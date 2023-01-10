//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// Chat mode
//


#include <string.h>
#include <ctype.h>
#include <SDL_scancode.h>
#include "hr_local.h"
#include "doomkeys.h"
#include "deh_str.h"
#include "i_input.h"
#include "m_misc.h"
#include "rd_keybinds.h"
#include "s_sound.h"
#include "v_video.h"

#define QUEUESIZE      128
#define MESSAGESIZE    128
#define MESSAGELEN     265
#define CT_PLR_GREEN   1
#define CT_PLR_YELLOW  2
#define CT_PLR_RED     3
#define CT_PLR_BLUE    4
#define CT_PLR_ALL     5
#define CT_ESCAPE      6

// Public data

boolean  chatmodeon;
char    *chat_macros[10];

// Private data

static void CT_queueChatChar (const char ch);
static void CT_ClearChatMessage (const int player);
static void CT_AddChar (const int player, const char c);
static void CT_BackSpace (const int player);

static int FontABaseLump;
static int head, tail;
static int chat_dest[MAXPLAYERS];
static int msgptr[MAXPLAYERS];
static int msglen[MAXPLAYERS];

static byte ChatQueue[QUEUESIZE];
static char chat_msg[MAXPLAYERS][MESSAGESIZE];
// add in the length of the pre-string
static char plr_lastmsg[MAXPLAYERS][MESSAGESIZE + 9];

static const char *CT_FromPlrText[MAXPLAYERS] = {
    "GREEN:  ",
    "YELLOW:  ",
    "RED:  ",
    "BLUE:  "
};

static boolean altdown;
static boolean shiftdown;


/*
================================================================================
=
= CT_Init
=
= Initialize chat mode data.
=
================================================================================
*/

void CT_Init (void)
{
    head = 0;  //initialize the queue index
    tail = 0;
    chatmodeon = false;
    memset(ChatQueue, 0, QUEUESIZE);

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        chat_dest[i] = 0;
        msgptr[i] = 0;
        memset(plr_lastmsg[i], 0, MESSAGESIZE);
        memset(chat_msg[i], 0, MESSAGESIZE);
    }

    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
}

/*
================================================================================
=
= CT_Stop
=
= Initialize chat mode data.
=
================================================================================
*/

static void CT_Stop (void)
{
    chatmodeon = false;
    I_StopTextInput();
}

// These keys are allowed by Vanilla Heretic:

static const boolean ValidChatChar (const char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '!' || c == '?' || c == ' ' || c == '-' || c == '='
		// [JN] Added support for extra English characters.
        || c == '(' || c == ')' || c == '+' || c == '/' || c == '\\'
        || c == '*' || c == '_' || c == '<' || c == '>' || c == ':' || c == '"'
        || c == ']' || c == '[' || c == ';' || c == '\''|| c == ',' || c == '.';
}

/*
================================================================================
=
= CT_Responder
=
= Initialize chat mode data.
=
================================================================================
*/

const boolean CT_Responder (event_t *ev)
{
    if (!netgame)
    {
        return false;
    }

    if (ev->data1 == SDL_SCANCODE_LALT || ev->data2 == SDL_SCANCODE_RALT)
    {
        altdown = (ev->type == ev_keydown);
        return false;
    }

    if (ev->data1 == SDL_SCANCODE_RSHIFT || ev->data1 == SDL_SCANCODE_LSHIFT)
    {
        shiftdown = (ev->type == ev_keydown);
        return false;
    }

    if (!chatmodeon)
    {
        int sendto = 0;

        if (BK_isKeyDown(ev, bk_multi_msg))
        {
            sendto = CT_PLR_ALL;
        }
        else if (BK_isKeyDown(ev, bk_multi_msg_player_0))
        {
            sendto = CT_PLR_GREEN;
        }
        else if (BK_isKeyDown(ev, bk_multi_msg_player_1))
        {
            sendto = CT_PLR_YELLOW;
        }
        else if (BK_isKeyDown(ev, bk_multi_msg_player_2))
        {
            sendto = CT_PLR_RED;
        }
        else if (BK_isKeyDown(ev, bk_multi_msg_player_3))
        {
            sendto = CT_PLR_BLUE;
        }

        if (sendto == 0 || (sendto != CT_PLR_ALL && !playeringame[sendto - 1])
        ||  sendto == consoleplayer + 1)
        {
            return false;
        }

        CT_queueChatChar(sendto);
        chatmodeon = true;
        I_StartTextInput(25, 10, screenwidth, 18);

        return true;
    }
    else if (ev->type == ev_keydown)
    {
        if (altdown)
        {
            if (ev->data1 >= SDL_SCANCODE_1 && ev->data1 <= SDL_SCANCODE_0)
            {
                char *macro;

                if (ev->data1 == SDL_SCANCODE_0)
                {
                    // macro 0 comes after macro 9
                    ev->data1 = SDL_SCANCODE_Z;
                }

                macro = chat_macros[ev->data1 - SDL_SCANCODE_Z];
                CT_queueChatChar(KEY_ENTER);  // send old message
                CT_queueChatChar(chat_dest[consoleplayer]);  // chose the dest.

                while (*macro)
                {
                    CT_queueChatChar(toupper(*macro++));
                }

                CT_queueChatChar(KEY_ENTER);  //send it off...
                CT_Stop();

                return true;
            }
        }

        if (BK_isKeyDown(ev, bk_menu_select))
        {
            CT_queueChatChar(KEY_ENTER);
            CT_Stop();
            return true;
        }
        else if (BK_isKeyDown(ev, bk_menu_activate))
        {
            CT_queueChatChar(CT_ESCAPE);
            CT_Stop();
            return true;
        }
        else if (BK_isKeyDown(ev, bk_menu_back))
        {
            CT_queueChatChar(KEY_BACKSPACE);
            return true;
        }
        else if (ValidChatChar(ev->data3))
        {
            CT_queueChatChar(toupper(ev->data3));
            return true;
        }
    }
    return false;
}

/*
================================================================================
=
= CT_Ticker
=
================================================================================
*/

void CT_Ticker (void)
{
    char c;
    int numplayers;

    for (int i = 0 ; i < MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }

        if ((c = players[i].cmd.chatchar) != 0)
        {
            if (c <= 5)
            {
                chat_dest[i] = c;
                continue;
            }
            else if (c == CT_ESCAPE)
            {
                CT_ClearChatMessage(i);
            }
            else if (c == KEY_ENTER)
            {
                numplayers = 0;

                for (int j = 0 ; j < MAXPLAYERS ; j++)
                {
                    numplayers += playeringame[j];
                }

                CT_AddChar(i, 0);  // set the end of message character

                if (numplayers > 2)
                {
                    M_StringCopy(plr_lastmsg[i], DEH_String(CT_FromPlrText[i]),
                                 sizeof(plr_lastmsg[i]));
                    M_StringConcat(plr_lastmsg[i], chat_msg[i],
                                   sizeof(plr_lastmsg[i]));
                }
                else
                {
                    M_StringCopy(plr_lastmsg[i], chat_msg[i],
                                 sizeof(plr_lastmsg[i]));
                }

                if (i != consoleplayer && (chat_dest[i] == consoleplayer + 1
                ||  chat_dest[i] == CT_PLR_ALL) && *chat_msg[i])
                {
                    P_SetMessage(&players[consoleplayer], plr_lastmsg[i],
                                 msg_chat, true);
                    S_StartSound(NULL, sfx_chat);
                }
                else if (i == consoleplayer && (*chat_msg[i]))
                {
                    if (numplayers > 1)
                    {
                        // [JN] Replicate sended message locally.
                        P_SetMessage(&players[consoleplayer], plr_lastmsg[i],
                                     msg_chat, true);
                        S_StartSound(NULL, sfx_chat);
                    }
                    else
                    {
                        P_SetMessage(&players[consoleplayer], txt_noplayers,
                                     msg_chat, true);
                        S_StartSound(NULL, sfx_chat);
                    }
                }
                CT_ClearChatMessage(i);
            }
            else if (c == KEY_BACKSPACE)
            {
                CT_BackSpace(i);
            }
            else
            {
                CT_AddChar(i, c);
            }
        }
    }
    return;
}

/*
================================================================================
=
= CT_Drawer
=
================================================================================
*/

void CT_Drawer (void)
{
    int x = 2;

    // [JN] Colorize text while typing.
    dp_translation = messages_chat_color_set == CR_NONE ?
                    NULL : cr[messages_chat_color_set];

    for (int i = 0 ; i < msgptr[consoleplayer] ; i++)
    {
        if (chat_msg[consoleplayer][i] < 33)
        {
            x += 6;
        }
        else
        {
            patch_t *patch = W_CacheLumpNum(FontABaseLump
                           + chat_msg[consoleplayer][i] - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, 10, patch);
            x += patch->width;
        }
    }
    V_DrawShadowedPatchRaven(x, 10, W_CacheLumpName 
                            (DEH_String("FONTA59"), PU_CACHE)); 
    BorderTopRefresh = true;
    UpdateState |= I_MESSAGES;
    dp_translation = NULL;
}

/*
================================================================================
=
= CT_queueChatChar
=
================================================================================
*/

static void CT_queueChatChar (const char ch)
{
    if (((tail + 1) & (QUEUESIZE - 1)) == head)
    {
        return;  // the queue is full
    }

    ChatQueue[tail] = ch;
    tail = (tail + 1) & (QUEUESIZE - 1);
}

/*
================================================================================
=
= CT_dequeueChatChar
=
================================================================================
*/

const char CT_dequeueChatChar (void)
{
    byte temp;

    if (head == tail)
    {
        return 0;  // queue is empty
    }
    temp = ChatQueue[head];
    head = (head + 1) & (QUEUESIZE - 1);
    return temp;
}

/*
================================================================================
=
= CT_AddChar
=
================================================================================
*/

static void CT_AddChar (const int player, const char c)
{
    if (msgptr[player] + 1 >= MESSAGESIZE || msglen[player] >= MESSAGELEN)
    {
        return;  // full.
    }

    chat_msg[player][msgptr[player]] = c;
    msgptr[player]++;

    if (c < 33)
    {
        msglen[player] += 6;
    }
    else
    {
        patch_t *patch = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
        msglen[player] += patch->width;
    }
}

/*
================================================================================
=
= CT_BackSpace
=
= Backs up a space, when the user hits (obviously) backspace.
=
================================================================================
*/

static void CT_BackSpace (const int player)
{
    char c;

    if (msgptr[player] == 0)
    {
        return;  // message is already blank
    }

    msgptr[player]--;
    c = chat_msg[player][msgptr[player]];

    if (c < 33)
    {
        msglen[player] -= 6;
    }
    else
    {
        patch_t *patch = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
        msglen[player] -= patch->width;
    }

    chat_msg[player][msgptr[player]] = 0;
}

/*
================================================================================
=
= CT_ClearChatMessage
=
= Clears out the data for the chat message, but the player's message
= is still saved in plrmsg.
=
================================================================================
*/

static void CT_ClearChatMessage (const int player)
{
    memset(chat_msg[player], 0, MESSAGESIZE);
    msgptr[player] = 0;
    msglen[player] = 0;
}

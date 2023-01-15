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



#include <ctype.h>

#include "h2def.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "p_local.h"
#include "v_video.h"
#include "i_swap.h"
#include "rd_rushexen.h"

// MACROS ------------------------------------------------------------------

#define	TEXTSPEED 3
#define	TEXTWAIT 140

// TYPES -------------------------------------------------------------------

typedef enum
{
    SINGLE,
    COOPERATIVE,
    DEATHMATCH
} gametype_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void WaitStop(void);
static void Stop(void);
static void LoadPics(void);
static void UnloadPics(void);
static void CheckForSkip(void);
static void CheckForSkipSingle(void);
static void InitStats(void);
static void DrDeathTally(void);
static void DrNumber(int val, int x, int y, int wrapThresh);
static void DrNumberBold(int val, int x, int y, int wrapThresh);
static void DrawHubText(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

boolean intermission;
char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean skipintermission;
static int interstate = 0;
static int intertime = -1;
static gametype_t gametype;
static int cnt;
static int slaughterboy;        // in DM, the player with the most kills
static patch_t *patchINTERPIC;
static patch_t *FontBNumbers[10];
static patch_t *FontBNegative;
static patch_t *FontBSlash;
static patch_t *FontBPercent;
static int FontABaseLump;
static int FontFBaseLump; // [JN] Small Russian font
static int FontBLump;
static int FontBLumpBase;

static signed int totalFrags[MAXPLAYERS];

static int HubCount;
static char *HubText;

// CODE --------------------------------------------------------------------

//========================================================================
//
// IN_Start
//
//========================================================================

extern void AM_Stop(void);

void IN_Start(void)
{
    int i;
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
    InitStats();
    LoadPics();
    intermission = true;
    interstate = 0;
    skipintermission = false;
    intertime = 0;
    AM_Stop();
    for (i = 0; i < maxplayers; i++)
    {
        players[i].messageTics = 0;
        players[i].message[0] = 0;
    }
    SN_StopAllSequences();
}

//========================================================================
//
// WaitStop
//
//========================================================================

void WaitStop(void)
{
    if (!--cnt)
    {
        Stop();
//              gamestate = GS_LEVEL;
//              G_DoLoadLevel();
        gameaction = ga_leavemap;
//              G_WorldDone();
    }
}

//========================================================================
//
// Stop
//
//========================================================================

static void Stop(void)
{
    intermission = false;
    UnloadPics();
    SB_state = -1;
    BorderNeedRefresh = true;
}

//========================================================================
//
// InitStats
//
//      Initializes the stats for single player mode
//========================================================================

static char *ClusMsgLumpNames[] = {
    "clus1msg",
    "clus2msg",
    "clus3msg",
    "clus4msg",
    "clus5msg"
};

// [JN] Custom Russian intermission texts for Beyond Heretic
static char *ClusMsgLumpNames_Rus[] = {
    "CLUS1_BH",
    "CLUS2_BH",
    "CLUS3_BH",
    "CLUS4_BH",
    "CLUS5_BH"
};

// [JN] Custom Russian intermission texts for Death Kings
static char *ClusMsgLumpNames_DK_Rus[] = {
    "CLUS1_DK",
    "CLUS2_DK",
    "CLUS3_DK",
    "CLUS4_DK",
    "CLUS5_DK"
};

static void InitStats(void)
{
    int i;
    int j;
    int oldCluster;
    signed int slaughterfrags;
    int posnum;
    int slaughtercount;
    int playercount;
    char *msgLumpName;
    int msgSize;
    int msgLump;

    extern int LeaveMap;

    if (!deathmatch)
    {
        gametype = SINGLE;
        HubCount = 0;
        oldCluster = P_GetMapCluster(gamemap);
        if (oldCluster != P_GetMapCluster(LeaveMap))
        {
            if (oldCluster >= 1 && oldCluster <= 5)
            {
                // [JN] Use standard CLUS*MSG for English language
                if (english_language)
                {
                    msgLumpName = ClusMsgLumpNames[oldCluster - 1];
                }
                else
                {
                    if (isDK)
                    {
                        // [JN] Use custom CLUS*_DK for Russian language in Death Kings
                        msgLumpName = ClusMsgLumpNames_DK_Rus[oldCluster - 1];
                    }
                    else
                    {
                        // [JN] Use custom CLUS*_BH for Russian language in Beyond Heretic
                        msgLumpName = ClusMsgLumpNames_Rus[oldCluster - 1];
                    }
                }
                msgLump = W_GetNumForName(msgLumpName);
                msgSize = W_LumpLength(msgLump);
                if (msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
                {
                    I_Error(english_language ?
                    "Cluster message too long (%s)" :
                    "Превышена допустимая длина текста (%s)",
                    msgLumpName);
                }
                W_ReadLump(msgLump, ClusterMessage);
                ClusterMessage[msgSize] = 0;    // Append terminator
                HubText = ClusterMessage;
                HubCount = strlen(HubText) * TEXTSPEED + TEXTWAIT;
                S_StartSongName("hub", true);
            }
        }
    }
    else
    {
        gametype = DEATHMATCH;
        slaughterboy = 0;
        slaughterfrags = -9999;
        posnum = 0;
        playercount = 0;
        slaughtercount = 0;
        for (i = 0; i < maxplayers; i++)
        {
            totalFrags[i] = 0;
            if (playeringame[i])
            {
                playercount++;
                for (j = 0; j < maxplayers; j++)
                {
                    if (playeringame[j])
                    {
                        totalFrags[i] += players[i].frags[j];
                    }
                }
                posnum++;
            }
            if (totalFrags[i] > slaughterfrags)
            {
                slaughterboy = 1 << i;
                slaughterfrags = totalFrags[i];
                slaughtercount = 1;
            }
            else if (totalFrags[i] == slaughterfrags)
            {
                slaughterboy |= 1 << i;
                slaughtercount++;
            }
        }
        if (playercount == slaughtercount)
        {                       // don't do the slaughter stuff if everyone is equal
            slaughterboy = 0;
        }
        S_StartSongName("hub", true);
    }
}

//========================================================================
//
// LoadPics
//
//========================================================================

static void LoadPics(void)
{
    int i;

    if (HubCount || gametype == DEATHMATCH)
    {
        patchINTERPIC = W_CacheLumpName("INTERPIC", PU_STATIC);
        FontBLumpBase = W_GetNumForName("FONTB16");
        for (i = 0; i < 10; i++)
        {
            FontBNumbers[i] = W_CacheLumpNum(FontBLumpBase + i, PU_STATIC);
        }
        FontBLump = W_GetNumForName("FONTB_S") + 1;
        FontBNegative = W_CacheLumpName("FONTB13", PU_STATIC);
        FontABaseLump = W_GetNumForName("FONTA_S") + 1;
        FontFBaseLump = W_GetNumForName("FONTF_S") + 1;

        FontBSlash = W_CacheLumpName("FONTB15", PU_STATIC);
        FontBPercent = W_CacheLumpName("FONTB05", PU_STATIC);
    }
}

//========================================================================
//
// UnloadPics
//
//========================================================================

static void UnloadPics(void)
{
    int i;

    if (HubCount || gametype == DEATHMATCH)
    {
        W_ReleaseLumpName("INTERPIC");

        patchINTERPIC = W_CacheLumpName("INTERPIC", PU_STATIC);
        FontBLumpBase = W_GetNumForName("FONTB16");
        for (i = 0; i < 10; i++)
        {
            W_ReleaseLumpNum(FontBLumpBase + i);
        }
        W_ReleaseLumpName("FONTB13");
        W_ReleaseLumpName("FONTB15");
        W_ReleaseLumpName("FONTB05");
    }
}

//========================================================================
//
// IN_Ticker
//
//========================================================================

void IN_Ticker(void)
{
    // [JN] Make PAUSE working properly on intermission screen
    if (paused)
    {
        return;
    }

    if (!intermission)
    {
        return;
    }
    if (interstate)
    {
        WaitStop();
        return;
    }
    skipintermission = false;

    // [JN] Allow double press text skipping if we are in single player.
    if (singleplayer)
    {
        CheckForSkipSingle();
    }
    else
    {
        CheckForSkip();
    }


    intertime++;
    if (skipintermission || (gametype == SINGLE && !HubCount))
    {
        interstate = 1;
        cnt = 10;
        skipintermission = false;
        //S_StartSound(NULL, sfx_dorcls);
    }
}

//========================================================================
//
// CheckForSkip
//
//      Check to see if any player hit a key
//========================================================================

static void CheckForSkip(void)
{
    int i;
    player_t *player;
    static boolean triedToSkip;

    for (i = 0, player = players; i < maxplayers; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
            {
                if (!player->attackdown)
                {
                    skipintermission = 1;
                }
                player->attackdown = true;
            }
            else
            {
                player->attackdown = false;
            }
            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                {
                    skipintermission = 1;
                }
                player->usedown = true;
            }
            else
            {
                player->usedown = false;
            }
        }
    }
    if (deathmatch && intertime < 140)
    {                           // wait for 4 seconds before allowing a skip
        if (skipintermission == 1)
        {
            triedToSkip = true;
            skipintermission = 0;
        }
    }
    else
    {
        if (triedToSkip)
        {
            skipintermission = 1;
            triedToSkip = false;
        }
    }
}

/*
================================================================================
=
= CheckForSkipSingle
=
= [JN] Allow double skipping by pressing ATTACK / USE keys:
= 1) First press printing all interlude text.
= 2) Second press advances to next stage.
=      
================================================================================
*/

static void CheckForSkipSingle (void)
{
    // [JN] Don't allow to skip by pressing "pause" button.
    if (players[consoleplayer].cmd.buttons == (BT_SPECIAL | BTS_PAUSE))
    {
        return;
    }

    // [JN] Double-skip by pressing "attack" button.
    if (players[consoleplayer].cmd.buttons & BT_ATTACK)
    {
        if (!players[consoleplayer].attackdown)
        {
            if (intertime >= 5003)
            {
                skipintermission = 1;
                return;
            }

            intertime += 5000;
            players[consoleplayer].attackdown = true;
        }
        players[consoleplayer].attackdown = true;
    }
    else
    {
        players[consoleplayer].attackdown = false;
    }

    // [JN] Double-skip by pressing "use" button.
    if (players[consoleplayer].cmd.buttons & BT_USE)
    {
        if (!players[consoleplayer].usedown)
        {
            if (intertime >= 5003)
            {
                skipintermission = 1;
                return;
            }

            intertime += 5000;
            players[consoleplayer].usedown = true;
        }
        players[consoleplayer].usedown = true;
    }
    else
    {
        players[consoleplayer].usedown = false;
    }
}

//========================================================================
//
// IN_Drawer
//
//========================================================================

void IN_Drawer(void)
{
    const patch_t *interpic_gfx = W_CacheLumpName("INTERPIC", PU_CACHE);

    if (!intermission)
    {
        return;
    }
    if (interstate)
    {
        return;
    }

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    UpdateState |= I_FULLSCRN;

    // [JN] For checking of modified fullscreen graphics.
    if (interpic_gfx->width == 560)
    {
        V_DrawPatchFullScreen(W_CacheLumpName("INTERPIC", PU_CACHE), false);
    }
    else
    {
        V_CopyScaledBuffer(I_VideoBuffer, (byte *) patchINTERPIC, ORIGWIDTH * ORIGHEIGHT);
    }

    if (gametype == SINGLE)
    {
        if (HubCount)
        {
            DrawHubText();
        }
    }
    else
    {
        DrDeathTally();
    }
}

//========================================================================
//
// DrDeathTally
//
//========================================================================

#define TALLY_EFFECT_TICKS 20
#define TALLY_FINAL_X_DELTA (23*FRACUNIT)
#define TALLY_FINAL_Y_DELTA (13*FRACUNIT)
#define TALLY_START_XPOS (178*FRACUNIT)
#define TALLY_STOP_XPOS (90*FRACUNIT)
#define TALLY_START_YPOS (132*FRACUNIT)
#define TALLY_STOP_YPOS (83*FRACUNIT)
#define TALLY_TOP_X 85
#define TALLY_TOP_Y 9
#define TALLY_LEFT_X 7
#define TALLY_LEFT_Y 71
#define TALLY_TOTALS_X 291

static void DrDeathTally(void)
{
    int i, j;
    fixed_t xPos, yPos;
    fixed_t xDelta, yDelta;
    fixed_t xStart, scale;
    int x, y;
    boolean bold;
    static boolean showTotals;
    int temp;

    if (english_language)
    {
        V_DrawPatch(TALLY_TOP_X, TALLY_TOP_Y,
                    W_CacheLumpName("tallytop", PU_CACHE), NULL);
        V_DrawPatch(TALLY_LEFT_X, TALLY_LEFT_Y,
                    W_CacheLumpName("tallylft", PU_CACHE), NULL);
    }
    else
    {
        V_DrawPatch(TALLY_TOP_X, TALLY_TOP_Y,
                    W_CacheLumpName("RD_TLTOP", PU_CACHE), NULL);
        V_DrawPatch(TALLY_LEFT_X, TALLY_LEFT_Y,
                    W_CacheLumpName("RD_TLLFT", PU_CACHE), NULL);
    }

    if (intertime < TALLY_EFFECT_TICKS)
    {
        showTotals = false;
        scale = (intertime * FRACUNIT) / TALLY_EFFECT_TICKS;
        xDelta = FixedMul(scale, TALLY_FINAL_X_DELTA);
        yDelta = FixedMul(scale, TALLY_FINAL_Y_DELTA);
        xStart = TALLY_START_XPOS - FixedMul(scale,
                                             TALLY_START_XPOS -
                                             TALLY_STOP_XPOS);
        yPos =
            TALLY_START_YPOS - FixedMul(scale,
                                        TALLY_START_YPOS - TALLY_STOP_YPOS);
    }
    else
    {
        xDelta = TALLY_FINAL_X_DELTA;
        yDelta = TALLY_FINAL_Y_DELTA;
        xStart = TALLY_STOP_XPOS;
        yPos = TALLY_STOP_YPOS;
    }
    if (intertime >= TALLY_EFFECT_TICKS && showTotals == false)
    {
        showTotals = true;
        S_StartSound(NULL, SFX_PLATFORM_STOP);
    }
    y = yPos >> FRACBITS;
    for (i = 0; i < maxplayers; i++)
    {
        xPos = xStart;
        for (j = 0; j < maxplayers; j++, xPos += xDelta)
        {
            x = xPos >> FRACBITS;
            bold = (i == consoleplayer || j == consoleplayer);
            if (playeringame[i] && playeringame[j])
            {
                if (bold)
                {
                    DrNumberBold(players[i].frags[j], x, y, 100);
                }
                else
                {
                    DrNumber(players[i].frags[j], x, y, 100);
                }
            }
            else
            {
                temp = RD_M_TextAWidth("--") / 2;
                if (bold)
                {
                    MN_DrTextAYellow("--", x - temp, y);
                }
                else
                {
                    RD_M_DrawTextA("--", x - temp, y);
                }
            }
        }
        if (showTotals && playeringame[i]
            && !((slaughterboy & (1 << i)) && !(intertime & 16)))
        {
            DrNumber(totalFrags[i], TALLY_TOTALS_X, y, 1000);
        }
        yPos += yDelta;
        y = yPos >> FRACBITS;
    }
}

//==========================================================================
//
// DrNumber
//
//==========================================================================

static void DrNumber(int val, int x, int y, int wrapThresh)
{
    char buff[8] = "XX";

    if (!(val < -9 && wrapThresh < 1000))
    {
        M_snprintf(buff, sizeof(buff), "%d",
                   val >= wrapThresh ? val % wrapThresh : val);
    }
    RD_M_DrawTextA(buff, x - RD_M_TextAWidth(buff) / 2, y);
}

//==========================================================================
//
// DrNumberBold
//
//==========================================================================

static void DrNumberBold(int val, int x, int y, int wrapThresh)
{
    char buff[8] = "XX";

    if (!(val < -9 && wrapThresh < 1000))
    {
        M_snprintf(buff, sizeof(buff), "%d",
                   val >= wrapThresh ? val % wrapThresh : val);
    }
    MN_DrTextAYellow(buff, x - RD_M_TextAWidth(buff) / 2, y);
}

//===========================================================================
//
// DrawHubText
//
//===========================================================================

static void DrawHubText(void)
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    patch_t *w;

    cy = 5;
    cx = 10;
    ch = HubText;
    count = (intertime - 10) / TEXTSPEED;
    if (count < 0)
    {
        count = 0;
    }
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
        {
            break;
        }
        if (c == '\n')
        {
            cx = 10;
            cy += 9;
            continue;
        }
        if (c < 32)
        {
            continue;
        }
        c = toupper(c);
        if (c == 32)
        {
            cx += 5;
            continue;
        }
        w = W_CacheLumpNum((english_language ? FontABaseLump : FontFBaseLump)
                                             + c - 33, PU_CACHE);
        if (cx + SHORT(w->width) > screenwidth)
        {
            break;
        }
        V_DrawShadowedPatchRaven(cx + wide_delta, cy, w);
        cx += SHORT(w->width);
    }
}

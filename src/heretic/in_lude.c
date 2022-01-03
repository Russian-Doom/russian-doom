//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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



/*
========================
=
= IN_lude.c
=
========================
*/

#include "am_map.h"
#include "doomdef.h"
#include "deh_str.h"
#include "p_local.h"
#include "s_sound.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "jn.h"


typedef enum
{
    SINGLE,
    COOPERATIVE,
    DEATHMATCH
} gametype_t;

// Public functions

boolean intermission;

// Private functions

void IN_WaitStop(void);
void IN_Stop(void);
void IN_LoadPics(void);
void IN_UnloadPics(void);
void IN_CheckForSkip(void);
void IN_InitStats(void);
void IN_InitDeathmatchStats(void);
void IN_InitNetgameStats(void);
void IN_DrawOldLevel(void);
void IN_DrawYAH(void);
void IN_DrawSingleStats(void);
void IN_DrawCoopStats(void);
void IN_DrawDMStats(void);
void IN_DrawNumber(int val, int x, int y, int digits);
void IN_DrawTime(int x, int y, int h, int m, int s);

static boolean skipintermission;
static int interstate = 0;
static int intertime = -1;
static int oldintertime = 0;
static gametype_t gametype;

static int cnt;

static int hours;
static int minutes;
static int seconds;

static int slaughterboy;        // in DM, the player with the most kills

static int killPercent[MAXPLAYERS];
static int bonusPercent[MAXPLAYERS];
static int secretPercent[MAXPLAYERS];

static patch_t *patchINTERPIC;
static patch_t *patchBEENTHERE;
static patch_t *patchGOINGTHERE;
static patch_t *FontBNumbers[10];
static patch_t *FontBNegative;
static patch_t *FontBSlash;
static patch_t *FontBPercent;

static int FontBLump;
static int FontBLumpBase;
static int FontGLump;
static int FontGLumpBase;
static int patchFaceOkayBase;
static int patchFaceDeadBase;

static signed int totalFrags[MAXPLAYERS];
static fixed_t dSlideX[MAXPLAYERS];
static fixed_t dSlideY[MAXPLAYERS];

static char *KillersText[] = { "K", "I", "L", "L", "E", "R", "S" };
//                                 "У", "Б", "И", "Й", "Ц", "Ы", ""
static char *KillersText_Rus[] = { "E", ",", "B", "Q", "W", "S", "" };

typedef struct
{
    int x;
    int y;
} yahpt_t;

static yahpt_t YAHspot[3][9] = {
    {
        {172,  78},
        {86,   90},
        {73,   66},
        {159,  95},
        {148, 126},
        {132,  54},
        {131,  74},
        {208, 138},
        {52,  101}
    },
    {
        {218,  57},
        {137,  81},
        {155, 124},
        {171,  68},
        {250,  86},
        {136,  98},
        {203,  90},
        {220, 140},
        {279, 106}
    },
    {
        {86,   99},
        {124, 103},
        {154,  79},
        {202,  83},
        {178,  59},
        {142,  58},
        {219,  66},
        {247,  57},
        {107 , 80}
    }
};

//========================================================================
//
// IN_Start
//
//========================================================================

void IN_Start(void)
{
    I_SetPalette(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    IN_LoadPics();
    IN_InitStats();
    intermission = true;
    interstate = -1;
    skipintermission = false;
    intertime = 0;
    oldintertime = 0;
    AM_Stop();
    S_StartSong(mus_intr, true, false);
}

//========================================================================
//
// IN_WaitStop
//
//========================================================================

void IN_WaitStop(void)
{
    if (!--cnt)
    {
        IN_Stop();
        G_WorldDone();
    }
}

//========================================================================
//
// IN_Stop
//
//========================================================================

void IN_Stop(void)
{
    intermission = false;
    IN_UnloadPics();
    BorderNeedRefresh = true;
}

//========================================================================
//
// IN_InitStats
//
//      Initializes the stats for single player mode
//========================================================================

void IN_InitStats(void)
{
    int i;
    int j;
    signed int slaughterfrags;
    int posnum;
    int slaughtercount;
    int playercount;
    int count;

    if (!netgame)
    {
        gametype = SINGLE;
        count = leveltime / 35;
        hours = count / 3600;
        count -= hours * 3600;
        minutes = count / 60;
        count -= minutes * 60;
        seconds = count;
    }
    else if (netgame && !deathmatch)
    {
        gametype = COOPERATIVE;
        memset(killPercent, 0, MAXPLAYERS * sizeof(int));
        memset(bonusPercent, 0, MAXPLAYERS * sizeof(int));
        memset(secretPercent, 0, MAXPLAYERS * sizeof(int));
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {
                if (totalkills)
                {
                    killPercent[i] = players[i].killcount * 100 / totalkills;
                }
                if (totalitems)
                {
                    bonusPercent[i] = players[i].itemcount * 100 / totalitems;
                }
                if (totalsecret)
                {
                    secretPercent[i] =
                        players[i].secretcount * 100 / totalsecret;
                }
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
        for (i = 0; i < MAXPLAYERS; i++)
        {
            totalFrags[i] = 0;
            if (playeringame[i])
            {
                playercount++;
                for (j = 0; j < MAXPLAYERS; j++)
                {
                    if (playeringame[j])
                    {
                        totalFrags[i] += players[i].frags[j];
                    }
                }
                dSlideX[i] = (43 * posnum * FRACUNIT) / 20;
                dSlideY[i] = (36 * posnum * FRACUNIT) / 20;
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
        {   // don't do the slaughter stuff if everyone is equal
            slaughterboy = 0;
        }
    }
}

static void IN_LoadUnloadPics(void (*callback)(char *lumpname,
                                               int lumpnum,
                                               patch_t **ptr))
{
    int i;

    switch (gameepisode)
    {
        case 1:
            callback(DEH_String("MAPE1"), 0, &patchINTERPIC);
            break;
        case 2:
            callback(DEH_String("MAPE2"), 0, &patchINTERPIC);
            break;
        case 3:
            callback(DEH_String("MAPE3"), 0, &patchINTERPIC);
            break;
        default:
            break;
    }

    callback(DEH_String("IN_X"), 0, &patchBEENTHERE);
    callback(DEH_String("IN_YAH"), 0, &patchGOINGTHERE);
    callback(DEH_String("FONTB13"), 0, &FontBNegative);

    callback(DEH_String("FONTB15"), 0, &FontBSlash);
    callback(DEH_String("FONTB05"), 0, &FontBPercent);

    FontBLumpBase = W_GetNumForName(DEH_String("FONTB16"));
    FontGLumpBase = W_GetNumForName(DEH_String("FONTG16"));

    for (i = 0; i < 10; i++)
    {
        callback(NULL, FontBLumpBase + i, &FontBNumbers[i]);
        callback(NULL, FontGLumpBase + i, &FontBNumbers[i]);
    }
}

//========================================================================
//
// IN_LoadPics
//
//========================================================================

static void LoadLumpCallback(char *lumpname, int lumpnum, patch_t **ptr)
{
    if (lumpname != NULL)
    {
        lumpnum = W_GetNumForName(lumpname);
    }

    // Cache the lump

    *ptr = W_CacheLumpNum(lumpnum, PU_STATIC);
}

void IN_LoadPics(void)
{
    FontBLump = W_GetNumForName(DEH_String("FONTB_S")) + 1;
    FontGLump = W_GetNumForName(DEH_String("FONTG_S")) + 1;
    patchFaceOkayBase = W_GetNumForName(DEH_String("FACEA0"));
    patchFaceDeadBase = W_GetNumForName(DEH_String("FACEB0"));

    IN_LoadUnloadPics(LoadLumpCallback);
}

//========================================================================
//
// IN_UnloadPics
//
//========================================================================

static void UnloadLumpCallback(char *lumpname, int lumpnum, patch_t **ptr)
{
    if (lumpname != NULL)
    {
        W_ReleaseLumpName(lumpname);
    }
    else
    {
        W_ReleaseLumpNum(lumpnum);
    }
}

void IN_UnloadPics(void)
{
    IN_LoadUnloadPics(UnloadLumpCallback);
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
    if (interstate == 3)
    {
        IN_WaitStop();
        return;
    }
    IN_CheckForSkip();
    intertime++;
    if (oldintertime < intertime)
    {
        interstate++;
        if (gameepisode > 3 && interstate >= 1)
        {   // Extended Wad levels:  skip directly to the next level
            interstate = 3;
        }
        switch (interstate)
        {
            case 0:
                oldintertime = intertime + 300;
                if (gameepisode > 3)
                {
                    oldintertime = intertime + 1200;
                }
                break;
            case 1:
                oldintertime = intertime + 200;
                break;
            case 2:
                oldintertime = INT_MAX;
                break;
            case 3:
                cnt = 10;
                break;
            default:
                break;
        }
    }
    if (skipintermission)
    {
        if (interstate == 0 && intertime < 150)
        {
            intertime = 150;
            skipintermission = false;
            return;
        }
        else if (interstate < 2 && gameepisode < 4)
        {
            interstate = 2;
            skipintermission = false;
            S_StartSound(NULL, sfx_dorcls);
            return;
        }
        interstate = 3;
        cnt = 10;
        skipintermission = false;
        S_StartSound(NULL, sfx_dorcls);
    }
}

//========================================================================
//
// IN_CheckForSkip
//
//      Check to see if any player hit a key
//========================================================================

void IN_CheckForSkip(void)
{
    int i;
    player_t *player;

    // [crispy] display tally screen after ExM8
    // [JN] Don't allow to skip ExM8 tally screen
    if (prevmap == 8)
    return;

    for (i = 0, player = players; i < MAXPLAYERS; i++, player++)
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
            // [JN] Pressing PAUSE should not skip intermission screen
            if (player->cmd.buttons & BTS_PAUSE)
            {
                skipintermission = 0;
            }
        }
    }
}

//========================================================================
//
// IN_Drawer
//
//========================================================================

void IN_Drawer(void)
{
    static int oldinterstate;

    if (!intermission)
    {
        return;
    }
    if (interstate == 3)
    {
        return;
    }
    UpdateState |= I_FULLSCRN;
    if (oldinterstate != 2 && interstate == 2)
    {
        S_StartSound(NULL, sfx_pstop);
    }
    oldinterstate = interstate;

    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: clean up wide screen remainings before drawing.
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    switch (interstate)
    {
        case 0:                // draw stats
            // [JN] Use unified function.
            V_FillFlat("FLOOR16");
            switch (gametype)
            {
                case SINGLE:
                    IN_DrawSingleStats();
                    break;
                case COOPERATIVE:
                    IN_DrawCoopStats();
                    break;
                case DEATHMATCH:
                    IN_DrawDMStats();
                    break;
            }
            break;
        case 1:                // leaving old level
            if (gameepisode < 4)
            {
                V_DrawPatchFullScreen(patchINTERPIC, false);
                IN_DrawOldLevel();
            }
            break;
        case 2:                // going to the next level
            if (gameepisode < 4)
            {
                V_DrawPatchFullScreen(patchINTERPIC, false);
                IN_DrawYAH();
            }
            break;
        case 3:                // waiting before going to the next level
            if (gameepisode < 4)
            {
                V_DrawPatchFullScreen(patchINTERPIC, false);
            }
            break;
        default:
            if (english_language)
            I_Error("IN_lude:  Intermission state out of range.\n");
            else
            I_Error("IN_lude: Ошибка последовательности в межмиссионном экране.\n");
            break;
    }
}

//========================================================================
//
// IN_DrawOldLevel
//
//========================================================================

void IN_DrawOldLevel(void)
{
    int i;
    int x;
	
	// boolean secret;
	// secret = false;

    if (english_language)
    {
        x = 160 - RD_M_TextBWidth(LevelNames[(gameepisode - 1)
                * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1]
                       + 7, x + wide_delta, 3);

        x = 160 - RD_M_TextAWidth(DEH_String("FINISHED")) / 2;
        RD_M_DrawTextA(DEH_String("FINISHED"), x + wide_delta, 25);
    }
    else
    {
        x = 160 - RD_M_TextBigRUSWidth(LevelNames_Rus[(gameepisode - 1)
            * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextBigRUS(LevelNames_Rus[(gameepisode - 1) * 9 + prevmap - 1]
                                                         + 7, x + wide_delta, 3);

        // УРОВЕНЬ ЗАВЕРШЕН
        x = 160 - RD_M_TextSmallRUSWidth(DEH_String("EHJDTYM PFDTHITY")) / 2;
        RD_M_DrawTextSmallRUS(DEH_String("EHJDTYM PFDTHITY"), x + wide_delta, 25, CR_NONE);
    }

    if (prevmap == 9)
    {
        for (i = 0; i < gamemap - 1; i++)
        {
            V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][i].x + wide_delta,
                        YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
        }
        if (!(intertime & 16))
        {
            V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][8].x + wide_delta,
                        YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
        }
    }
    else
    {
        for (i = 0; i < prevmap - 1; i++)
        {
            V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][i].x + wide_delta,
                        YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
        }
        if (players[consoleplayer].didsecret)
        {
            V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][8].x + wide_delta,
                        YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
        }
        if (!(intertime & 16))
        {
            V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][prevmap - 1].x + wide_delta,
                        YAHspot[gameepisode - 1][prevmap - 1].y,
                        patchBEENTHERE);
        }
    }
}

//========================================================================
//
// IN_DrawYAH
//
//========================================================================

void IN_DrawYAH(void)
{
    int i;
    int x;

    if (english_language)
    {
        x = 160 - RD_M_TextAWidth(DEH_String("NOW ENTERING:")) / 2;
        RD_M_DrawTextA(DEH_String("NOW ENTERING:"), x + wide_delta, 10);

        x = 160 - RD_M_TextBWidth(LevelNames[(gameepisode - 1)
                * 9 + gamemap - 1] + 7) / 2;
        RD_M_DrawTextB(LevelNames[(gameepisode - 1) * 9 + gamemap - 1]
                       + 7, x + wide_delta, 20);
    }
    else
    {
        // СЛЕДУЮЩИЙ УРОВЕНЬ:
        x = 160 - RD_M_TextSmallRUSWidth(DEH_String("CKTLE.OBQ EHJDTYM:")) / 2;
        RD_M_DrawTextSmallRUS(DEH_String("CKTLE.OBQ EHJDTYM:"), x + wide_delta, 10, CR_NONE);

        x = 160 - RD_M_TextBigRUSWidth(LevelNames_Rus[(gameepisode - 1)
            * 9 + gamemap - 1] + 7) / 2;
        RD_M_DrawTextBigRUS(LevelNames_Rus[(gameepisode - 1) * 9 + gamemap - 1]
                                                         + 7, x + wide_delta, 20);
    }

    if (prevmap == 9)
    {
        prevmap = gamemap - 1;
    }
    for (i = 0; i < prevmap; i++)
    {
        V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][i].x + wide_delta,
                    YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
    }
    if (players[consoleplayer].didsecret)
    {
        V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][8].x + wide_delta,
                    YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
    }
    if (!(intertime & 16) || interstate == 3)
    {                           // draw the destination 'X'
        V_DrawShadowedPatchRaven(YAHspot[gameepisode - 1][gamemap - 1].x + wide_delta,
                    YAHspot[gameepisode - 1][gamemap - 1].y, patchGOINGTHERE);
    }
}

//========================================================================
//
// IN_DrawSingleStats
//
//========================================================================

void IN_DrawSingleStats(void)
{
    int x;
    static int sounds;
    const int ttime = totaltimes / 35;  // [crispy] total time after level time
	
    // [JN] There is no intermission background in episodes 4-5, and thus,
    // all the stats including "Now Enterering" are placed in one screen.
    // I need a slightly different aligment to have enough space for
    // placing traditional K/I/S stats and also level time and total time.
    int classic_stats = (gamemode != retail || gameepisode <= 3);

	// int lastlevel = -1, lastepisode = -1;

    if (english_language)
    {
        RD_M_DrawTextB(DEH_String("KILLS"), 50 + wide_delta, classic_stats ? 65 : 44);
        RD_M_DrawTextB(DEH_String("ITEMS"), 50 + wide_delta, classic_stats ? 90 : 66);
        RD_M_DrawTextB(DEH_String("SECRETS"), 50 + wide_delta, classic_stats ? 115 : 88);

        x = 160 - RD_M_TextBWidth(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7, x + wide_delta, 3);

        x = 160 - RD_M_TextAWidth(DEH_String("FINISHED")) / 2;
        RD_M_DrawTextA(DEH_String("FINISHED"), x + wide_delta, 25);
    }
    else
    {
        RD_M_DrawTextBigRUS(DEH_String("DHFUB"), 50 + wide_delta, classic_stats ? 65 : 44);    // ВРАГИ
        RD_M_DrawTextBigRUS(DEH_String("GHTLVTNS"), 50 + wide_delta, classic_stats ? 90 : 66); // ПРЕДМЕТЫ
        RD_M_DrawTextBigRUS(DEH_String("NFQYBRB"), 50 + wide_delta, classic_stats ? 115 : 88); // ТАЙНИКИ

        x = 160 - RD_M_TextBigRUSWidth(LevelNames_Rus[(gameepisode - 1) * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextBigRUS(LevelNames_Rus[(gameepisode - 1) * 9 + prevmap - 1] + 7, x + wide_delta, 3);

        x = 160 - RD_M_TextSmallRUSWidth(DEH_String("EHJDTYM PFDTHITY")) / 2;         // УРОВЕНЬ ЗАВЕРШЕН
        RD_M_DrawTextSmallRUS(DEH_String("EHJDTYM PFDTHITY"), x + wide_delta, 25, CR_NONE); // УРОВЕНЬ ЗАВЕРШЕН
    }

    if (intertime < 30)
    {
        sounds = 0;
        return;
    }
    if (sounds < 1 && intertime >= 30)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }

    IN_DrawNumber(players[consoleplayer].killcount, 200 + wide_delta, classic_stats ? 65 : 44, 3);
    V_DrawShadowedPatch(237 + wide_delta, classic_stats ? 65 : 44, FontBSlash);
    IN_DrawNumber(totalkills, 248 + wide_delta, classic_stats ? 65 : 44, 3);

    if (intertime < 60)
    {
        return;
    }
    if (sounds < 2 && intertime >= 60)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }

    IN_DrawNumber(players[consoleplayer].itemcount, 200 + wide_delta, classic_stats ? 90 : 66, 3);
    V_DrawShadowedPatch(237 + wide_delta, classic_stats ? 90 : 66, FontBSlash);
    IN_DrawNumber(totalitems, 248 + wide_delta, classic_stats ? 90 : 66, 3);

    if (intertime < 90)
    {
        return;
    }
    if (sounds < 3 && intertime >= 90)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }

    IN_DrawNumber(players[consoleplayer].secretcount, 200 + wide_delta, classic_stats ? 115 : 88, 3);
    V_DrawShadowedPatch(237 + wide_delta, classic_stats ? 115 : 88, FontBSlash);
    IN_DrawNumber(totalsecret, 248 + wide_delta, classic_stats ? 115 : 88, 3);

    if (intertime < 150)
    {
        return;
    }
    if (sounds < 4 && intertime >= 150)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }

    // [JN] Draw level time and total time even for 4 and 5 episodes!
    if (english_language)
    {
        RD_M_DrawTextB(DEH_String("TIME"), 50 + wide_delta, classic_stats ? 145 : 114);
        IN_DrawTime(192 + wide_delta, classic_stats ? 145 : 114, hours, minutes, seconds);

        RD_M_DrawTextB(DEH_String("TOTAL"), 50 + wide_delta, classic_stats ? 165 : 134);
        IN_DrawTime(192 + wide_delta, classic_stats ? 165 : 134, ttime/3600, (ttime%3600)/60, ttime%60);
    }
    else
    {
        RD_M_DrawTextBigRUS(DEH_String("DHTVZ"), 50 + wide_delta, classic_stats ? 145 : 114);   // ВРЕМЯ
        IN_DrawTime(192 + wide_delta, classic_stats ? 145 : 114, hours, minutes, seconds);

        RD_M_DrawTextBigRUS(DEH_String("J,OTT DHTVZ"), 50 + wide_delta, classic_stats ? 165 : 134); // ОБЩЕЕ ВРЕМЯ
        IN_DrawTime(192 + wide_delta, classic_stats ? 165 : 134, ttime/3600, (ttime%3600)/60, ttime%60);
    }

    // [JN] Do not display "Now entering" after finishing ExM8
    if (!classic_stats && prevmap != 8)
    {
        if (english_language)
        {
            x = 160 - RD_M_TextAWidth(DEH_String("NOW ENTERING:")) / 2;
            RD_M_DrawTextA(DEH_String("NOW ENTERING:"), x + wide_delta, 160);

            x = 160 - RD_M_TextBWidth(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] + 7) / 2;
            RD_M_DrawTextB(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] + 7, x + wide_delta, 170);
        }
        else
        {
            x = 160 - RD_M_TextSmallRUSWidth(DEH_String("CKTLE.OBQ EHJDTYM:")) / 2;    // СЛЕДУЮЩИЙ УРОВЕНЬ:
            RD_M_DrawTextSmallRUS(DEH_String("CKTLE.OBQ EHJDTYM:"), x + wide_delta, 160, CR_NONE);  // СЛЕДУЮЩИЙ УРОВЕНЬ:

            x = 160 - RD_M_TextBigRUSWidth(LevelNames_Rus[(gameepisode - 1) * 9 + gamemap - 1] + 7) / 2;
            RD_M_DrawTextBigRUS(LevelNames_Rus[(gameepisode - 1) * 9 + gamemap - 1] + 7, x + wide_delta, 170);
        }
        skipintermission = false;
    }

    // [crispy] display tally screen after ExM8
    // [JN] Perfected timing for changing music
    if (intertime >= 260 && prevmap == 8)
    {
        F_StartFinale();
        return;
    }
}

//========================================================================
//
// IN_DrawCoopStats
//
//========================================================================

void IN_DrawCoopStats(void)
{
    int i;
    int x;
    int ypos;

    static int sounds;

    if (english_language)
    {
        RD_M_DrawTextB(DEH_String("KILLS"), 95 + wide_delta, 35);
        RD_M_DrawTextB(DEH_String("BONUS"), 155 + wide_delta, 35);
        RD_M_DrawTextB(DEH_String("SECRET"), 232 + wide_delta, 35);
    }
    else
    {
        RD_M_DrawTextB(DEH_String("DHFUB"), 81 + wide_delta, 35);    // ВРАГИ (95, 35)
        RD_M_DrawTextB(DEH_String(",JYECS"), 150 + wide_delta, 35);  // БОНУСЫ (155, 35)
        RD_M_DrawTextB(DEH_String("NFQYBRB"), 232 + wide_delta, 35); // ТАЙНИКИ (232, 35)
    }

    if (english_language)
    {
        x = 160 - RD_M_TextBWidth(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7, x + wide_delta, 3);

        x = 160 - RD_M_TextAWidth(DEH_String("FINISHED")) / 2;
        RD_M_DrawTextA(DEH_String("FINISHED"), x + wide_delta, 25);
    }
    else
    {
        x = 160 - RD_M_TextBigRUSWidth(LevelNames_Rus[(gameepisode - 1) * 9 + prevmap - 1] + 7) / 2;
        RD_M_DrawTextBigRUS(LevelNames_Rus[(gameepisode - 1) * 9 + prevmap - 1] + 7, x + wide_delta, 3);

        x = 160 - RD_M_TextSmallRUSWidth(DEH_String("EHJDTYM PFDTHITY")) / 2;   // УРОВЕНЬ ЗАВЕРШЕН
        RD_M_DrawTextSmallRUS(DEH_String("EHJDTYM PFDTHITY"), x + wide_delta, 25, CR_NONE);  // УРОВЕНЬ ЗАВЕРШЕН
    }

    ypos = 50;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            V_DrawShadowedPatch(25 + wide_delta, ypos,
                                W_CacheLumpNum(patchFaceOkayBase + i,
                                               PU_CACHE));
            if (intertime < 40)
            {
                sounds = 0;
                ypos += 37;
                continue;
            }
            else if (intertime >= 40 && sounds < 1)
            {
                S_StartSound(NULL, sfx_dorcls);
                sounds++;
            }

            if (english_language)
            {
                IN_DrawNumber(killPercent[i], 85 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(121 + wide_delta, ypos + 10, FontBPercent);
                IN_DrawNumber(bonusPercent[i], 160 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(196 + wide_delta, ypos + 10, FontBPercent);
                IN_DrawNumber(secretPercent[i], 237 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(273 + wide_delta, ypos + 10, FontBPercent);
            }
            else
            {
                // [JN] Coords changed to fit Russian words.
                IN_DrawNumber(killPercent[i], 84 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(120 + wide_delta, ypos + 10, FontBPercent);
                IN_DrawNumber(bonusPercent[i], 160 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(196 + wide_delta, ypos + 10, FontBPercent);
                IN_DrawNumber(secretPercent[i], 247 + wide_delta, ypos + 10, 3);
                V_DrawShadowedPatch(283 + wide_delta, ypos + 10, FontBPercent);
            }

            ypos += 37;
        }
    }
}

//========================================================================
//
// IN_DrawDMStats
//
//========================================================================

void IN_DrawDMStats(void)
{
    int i;
    int j;
    int ypos;
    int xpos;
    int kpos;

    static int sounds;

    xpos = 90;
    ypos = 55;

    if (english_language)
    {
        RD_M_DrawTextB(DEH_String("TOTAL"), 265 + wide_delta, 30);
        RD_M_DrawTextA(DEH_String("VICTIMS"), 140 + wide_delta, 8);
    }
    else
    {
        RD_M_DrawTextBigRUS(DEH_String("BNJU"), 265 + wide_delta, 30);    // ИТОГ
        RD_M_DrawTextSmallRUS(DEH_String(";THNDS"), 140 + wide_delta, 8, CR_NONE); // ЖЕРТВЫ
    }

    for (i = 0; i < 7; i++)
    {
        if (english_language)
            RD_M_DrawTextA(DEH_String(KillersText[i]), 10 + wide_delta, 80 + 9 * i);
        else
            RD_M_DrawTextSmallRUS(DEH_String(KillersText_Rus[i]), 10 + wide_delta, 80 + 9 * i, CR_NONE);
    }
    if (intertime < 20)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {
                V_DrawShadowedPatch(40 + wide_delta,
                                    ((ypos << FRACBITS) +
                                     dSlideY[i] * intertime) >> FRACBITS,
                                    W_CacheLumpNum(patchFaceOkayBase + i,
                                                   PU_CACHE));
                V_DrawShadowedPatch((((xpos << FRACBITS) + 
                                     dSlideX[i] * intertime) >> FRACBITS) + wide_delta, 18,
                                    W_CacheLumpNum(patchFaceDeadBase + i,
                                                   PU_CACHE));
            }
        }
        sounds = 0;
        return;
    }
    if (intertime >= 20 && sounds < 1)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }
    if (intertime >= 100 && slaughterboy && sounds < 2)
    {
        S_StartSound(NULL, sfx_wpnup);
        sounds++;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            if (intertime < 100 || i == consoleplayer)
            {
                V_DrawShadowedPatch(40 + wide_delta, ypos,
                                    W_CacheLumpNum(patchFaceOkayBase + i,
                                                   PU_CACHE));
                V_DrawShadowedPatch(xpos + wide_delta, 18,
                                    W_CacheLumpNum(patchFaceDeadBase + i,
                                                   PU_CACHE));
            }
            else
            {
                V_DrawTLPatch(40 + wide_delta, ypos,
                              W_CacheLumpNum(patchFaceOkayBase + i,
                                             PU_CACHE));
                V_DrawTLPatch(xpos + wide_delta, 18,
                              W_CacheLumpNum(patchFaceDeadBase + i,
                                             PU_CACHE));
            }
            kpos = 86;
            for (j = 0; j < MAXPLAYERS; j++)
            {
                if (playeringame[j])
                {
                    IN_DrawNumber(players[i].frags[j], kpos + wide_delta, ypos + 10, 3);
                    kpos += 43;
                }
            }
            if (slaughterboy & (1 << i))
            {
                if (!(intertime & 16))
                {
                    IN_DrawNumber(totalFrags[i], 263 + wide_delta, ypos + 10, 3);
                }
            }
            else
            {
                IN_DrawNumber(totalFrags[i], 263 + wide_delta, ypos + 10, 3);
            }
            ypos += 36;
            xpos += 43;
        }
    }
}

//========================================================================
//
// IN_DrawTime
//
//========================================================================

void IN_DrawTime(int x, int y, int h, int m, int s)
{
    if (h)
    {
        IN_DrawNumber(h, x, y, 2);
        RD_M_DrawTextB(DEH_String(":"), x + 26, y);
    }
    x += 34;
    if (m || h)
    {
        IN_DrawNumber(m, x, y, 2);
    }
    x += 34;
    if (s)
    {
        RD_M_DrawTextB(DEH_String(":"), x - 8, y);
        IN_DrawNumber(s, x, y, 2);
    }

    // [JN] If seconds = 0 (i.e. 23:00), draw it as "23:00", not as "23:".
    if (s == 0)
    {
        RD_M_DrawTextB(DEH_String(":"), x - 8, y);
        RD_M_DrawTextB(DEH_String("00"), x, y);
    }
}

//========================================================================
//
// IN_DrawNumber
//
//========================================================================

void IN_DrawNumber(int val, int x, int y, int digits)
{
    patch_t *patch;
    int xpos;
    int oldval;
    int realdigits;
    boolean neg;

    oldval = val;
    xpos = x;
    neg = false;
    realdigits = 1;

    if (val < 0)
    {                           //...this should reflect negative frags
        val = -val;
        neg = true;
        if (val > 99)
        {
            val = 99;
        }
    }
    if (val > 9)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 9;
        }
    }
    if (val > 99)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 99;
        }
    }
    if (val > 999)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 999;
        }
    }
    if (digits == 4)
    {
        patch = FontBNumbers[val / 1000];
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2 - 12, y, patch);
    }
    if (digits > 2)
    {
        if (realdigits > 2)
        {
            patch = FontBNumbers[val / 100];
            V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
        }
        xpos += 12;
    }
    val = val % 100;
    if (digits > 1)
    {
        if (val > 9)
        {
            patch = FontBNumbers[val / 10];
            V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
        }
        else if (digits == 2 || oldval > 99)
        {
            V_DrawShadowedPatch(xpos, y, FontBNumbers[0]);
        }
        xpos += 12;
    }
    val = val % 10;
    patch = FontBNumbers[val];
    V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
    if (neg)
    {
        patch = FontBNegative;
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2 - 12 * (realdigits),
                            y, patch);
    }
}


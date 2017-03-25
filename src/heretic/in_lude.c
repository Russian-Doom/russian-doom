//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// Russian DOOM (C) 2016-2017 Julian Nechaevsky


/*
========================
=
= IN_lude.c
=
========================
*/

#include "doomdef.h"
#include "deh_str.h"
#include "p_local.h"
#include "s_sound.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

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
void IN_DrawStatBack(void);
void IN_DrawSingleStats(void);
void IN_DrawCoopStats(void);
void IN_DrawDMStats(void);
void IN_DrawNumber(int val, int x, int y, int digits);
void IN_DrawTime(int x, int y, int h, int m, int s);
void IN_DrTextB(char *text, int x, int y);

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
static int patchFaceOkayBase;
static int patchFaceDeadBase;

static signed int totalFrags[MAXPLAYERS];
static fixed_t dSlideX[MAXPLAYERS];
static fixed_t dSlideY[MAXPLAYERS];

static char *KillersText[] = { "E", "<", "B", "Q", "W", "S", "" }; // "У", "Б", "И", "Й", "Ц", "Ы", ""

extern char *LevelNames[];

extern int lcd_gamma_fix;

typedef struct
{
    int x;
    int y;
} yahpt_t;

static yahpt_t YAHspot[3][9] = {
    {
     {172, 78},
     {86, 90},
     {73, 66},
     {159, 95},
     {148, 126},
     {132, 54},
     {131, 74},
     {208, 138},
     {52, 101}
     },
    {
     {218, 57},
     {137, 81},
     {155, 124},
     {171, 68},
     {250, 86},
     {136, 98},
     {203, 90},
     {220, 140},
     {279, 106}
     },
    {
     {86, 99},
     {124, 103},
     {154, 79},
     {202, 83},
     {178, 59},
     {142, 58},
     {219, 66},
     {247, 57},
     {107, 80}
     }
};

//========================================================================
//
// IN_Start
//
//========================================================================

extern void AM_Stop(void);

void IN_Start(void)
{
    if (lcd_gamma_fix)
        I_SetPalette(W_CacheLumpName(DEH_String("PALFIX"), PU_CACHE));
    else
        I_SetPalette(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    IN_LoadPics();
    IN_InitStats();
    intermission = true;
    interstate = -1;
    skipintermission = false;
    intertime = 0;
    oldintertime = 0;
    AM_Stop();
    S_StartSong(mus_intr, true);
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
    SB_state = -1;
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
        {                       // don't do the slaughter stuff if everyone is equal
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

    for (i = 0; i < 10; i++)
    {
        callback(NULL, FontBLumpBase + i, &FontBNumbers[i]);
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
        {                       // Extended Wad levels:  skip directly to the next level
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
    switch (interstate)
    {
        case 0:                // draw stats
            IN_DrawStatBack();
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
                V_DrawPatch(0, 0, patchINTERPIC);
                IN_DrawOldLevel();
            }
            break;
        case 2:                // going to the next level
            if (gameepisode < 4)
            {
                V_DrawPatch(0, 0, patchINTERPIC);
                IN_DrawYAH();
            }
            break;
        case 3:                // waiting before going to the next level
            if (gameepisode < 4)
            {
                V_DrawPatch(0, 0, patchINTERPIC);
            }
            break;
        default:
            I_Error("IN_lude:  Intermission state out of range.\n");
            break;
    }
}

//========================================================================
//
// IN_DrawStatBack
//
//========================================================================

void IN_DrawStatBack(void)
{
    int x;
    int y;

    byte *src;
    byte *dest;

    src = W_CacheLumpName(DEH_String("FLOOR16"), PU_CACHE);
    dest = I_VideoBuffer;

    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < SCREENWIDTH / 64; x++)
        {
            memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (SCREENWIDTH & 63)
        {
            memcpy(dest, src + ((y & 63) << 6), SCREENWIDTH & 63);
            dest += (SCREENWIDTH & 63);
        }
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
	
	boolean secret;
	secret = false;

    x = 160 - MN_TextBWidth(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] +
                            7) / 2;
    IN_DrTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7, x, 3);

	// [JN] Задаём разные тексты "завершен/завершена/завершено/завершены" для всех уровней игры. 
	// Экран №2, при загрузке нового уровня.
	if (gameepisode == 1) //ГОРОД ПРОКЛЯТЫХ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M1: ДОКИ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M2: ТЕМНИЦЫ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M3: ПРИВРАТНИЦКАЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M4: СТОРОЖЕВАЯ БАШНЯ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M5: ЦИТАДЕЛЬ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),   x, 25); }	// E1M6: КАФЕДРАЛЬНЫЙ СОБОР
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M7: СКЛЕПЫ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M8: АДСКАЯ УТРОБА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"),  x, 25); }	// E1M9: КЛАДБИЩЕ
	}
	
	if (gameepisode == 2) // АДСКАЯ УТРОБА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M1: КРАТЕР
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M2: ЛАВОВЫЕ ОЧАГИ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E2M3: РЕКА ОГНЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M4: ЛЕДЯНОЙ ГРОТ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M5: КАТАКОМБЫ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M6: ЛАБИРИНТ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M7: БОЛЬШОЙ ЗАЛ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M8: ПОРТАЛЫ ХАОСА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M9: ЛЕДНИК 
	}

	if (gameepisode == 3) // КУПОЛ Д'СПАРИЛА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M1: КЛАДОВАЯ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M2: СТОЧНЫЙ КОЛОДЕЦ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M3: СЛИЯНИЕ 
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M5: ЛОГОВО ОФИДИАНОВ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E3M6: ЗАЛЫ СТРАХА
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M7: ПРОПАСТЬ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M8: КРЕПОСТЬ Д'СПАРИЛА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M9: ВОДОНОСНЫЙ СЛОЙ
	}

	if (gameepisode == 4) // СКЛЕП
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M1: КАТАФАЛК
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E4M2: УКРЫТИЕ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M4: ГРОБНИЦА
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M6: ЗАЛЫ ОТСТУПНИКОВ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M8: РАЗРУШЕННЫЙ МОСТ
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M9: МАВЗОЛЕЙ
	}

	if (gameepisode == 5) // ЗАСТОЙНЫЕ ВЛАДЕНИЯ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M1: ОХРОВЫЕ УТЕСЫ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M2: СТРЕМНИНА
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M3: ПРИЧАЛ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M4: ВНУТРЕННИЙ ДВОР
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M5: ГИДРОТИР
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M6: КОЛОННАДА 
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M7: ЗЛОВОННЫЙ ОСОБНЯК
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M8: ПОЛЕ ВЫСШЕГО СУДА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M9: НЕРАЗБЕРИХА Д'СПАРИЛА
	}
	
//   x = 160 - MN_TextAWidth(DEH_String("FINISHED")) / 2;
//   MN_DrTextA(DEH_String("FINISHED"), x, 25);

    if (prevmap == 9)
    {
        for (i = 0; i < gamemap - 1; i++)
        {
            V_DrawPatch(YAHspot[gameepisode - 1][i].x,
                        YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
        }
        if (!(intertime & 16))
        {
            V_DrawPatch(YAHspot[gameepisode - 1][8].x,
                        YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
        }
    }
    else
    {
        for (i = 0; i < prevmap - 1; i++)
        {
            V_DrawPatch(YAHspot[gameepisode - 1][i].x,
                        YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
        }
        if (players[consoleplayer].didsecret)
        {
            V_DrawPatch(YAHspot[gameepisode - 1][8].x,
                        YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
        }
        if (!(intertime & 16))
        {
            V_DrawPatch(YAHspot[gameepisode - 1][prevmap - 1].x,
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
	
	// [JN] Задаём разные тексты "загружается/загружаются" для эпизодов 1, 2 и 3.	
	if (gameepisode == 1) // ГОРОД ПРОКЛЯТЫХ
	{
		if (gamemap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E1M1: ДОКИ
		if (gamemap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E1M2: ТЕМНИЦЫ
		if (gamemap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M3: ПРИВРАТНИЦКАЯ
		if (gamemap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M4: СТОРОЖЕВАЯ БАШНЯ
		if (gamemap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M5: ЦИТАДЕЛЬ
		if (gamemap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M6: КАФЕДРАЛЬНЫЙ СОБОР
		if (gamemap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E1M7: СКЛЕПЫ
		if (gamemap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M8: АДСКАЯ УТРОБА
		if (gamemap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E1M9: КЛАДБИЩЕ
	}
			
	if (gameepisode == 2) // ГОРОД ПРОКЛЯТЫХ
	{
		if (gamemap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M1: КРАТЕР
		if (gamemap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E2M2: ЛАВОВЫЕ ОЧАГИ
		if (gamemap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M3: РЕКА ОГНЯ
		if (gamemap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M4: ЛЕДЯНОЙ ГРОТ
		if (gamemap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E2M5: КАТАКОМБЫ
		if (gamemap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M6: ЛАБИРИНТ
		if (gamemap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M7: БОЛЬШОЙ ЗАЛ
		if (gamemap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E2M8: ПОРТАЛЫ ХАОСА
		if (gamemap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E2M9: ЛЕДНИК 
	}
	
	if (gameepisode == 3) // КУПОЛ Д'СПАРИЛА
	{
		if (gamemap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M1: КЛАДОВАЯ
		if (gamemap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M2: СТОЧНЫЙ КОЛОДЕЦ
		if (gamemap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M3: СЛИЯНИЕ 
		if (gamemap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
		if (gamemap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M5: ЛОГОВО ОФИДИАНОВ
		if (gamemap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 10); }	// E3M6: ЗАЛЫ СТРАХА
		if (gamemap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M7: ПРОПАСТЬ
		if (gamemap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M8: КРЕПОСТЬ Д'СПАРИЛА
		if (gamemap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10); }	// E3M9: ВОДОНОСНЫЙ СЛОЙ
	}
	
    // x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2;		// ЗАГРУЖАЕТСЯ
    // MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 10);				// ЗАГРУЖАЕТСЯ
	
    x = 160 - MN_TextBWidth(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] +
                            7) / 2;
    IN_DrTextB(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] + 7, x, 20);

    if (prevmap == 9)
    {
        prevmap = gamemap - 1;
    }
    for (i = 0; i < prevmap; i++)
    {
        V_DrawPatch(YAHspot[gameepisode - 1][i].x,
                    YAHspot[gameepisode - 1][i].y, patchBEENTHERE);
    }
    if (players[consoleplayer].didsecret)
    {
        V_DrawPatch(YAHspot[gameepisode - 1][8].x,
                    YAHspot[gameepisode - 1][8].y, patchBEENTHERE);
    }
    if (!(intertime & 16) || interstate == 3)
    {                           // draw the destination 'X'
        V_DrawPatch(YAHspot[gameepisode - 1][gamemap - 1].x,
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
	
	int lastlevel = -1, lastepisode = -1;

    IN_DrTextB(DEH_String("DHFUB"), 50, 65);    // ВРАГИ
    IN_DrTextB(DEH_String("GHTLVTNS"), 50, 90); // ПРЕДМЕТЫ
    IN_DrTextB(DEH_String("NFQYBRB"), 50, 115); // ТАЙНИКИ

    x = 160 - MN_TextBWidth(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] +
                            7) / 2;
    IN_DrTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7, x, 3);

	// [JN] Задаём разные тексты "завершен/завершена/завершено/завершены" для всех уровней игры. 
	// Экран №1, со статистикой.
	if (gameepisode == 1) //ГОРОД ПРОКЛЯТЫХ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M1: ДОКИ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M2: ТЕМНИЦЫ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M3: ПРИВРАТНИЦКАЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M4: СТОРОЖЕВАЯ БАШНЯ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M5: ЦИТАДЕЛЬ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),   x, 25); }	// E1M6: КАФЕДРАЛЬНЫЙ СОБОР
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M7: СКЛЕПЫ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M8: АДСКАЯ УТРОБА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"),  x, 25); }	// E1M9: КЛАДБИЩЕ
	}
	
	if (gameepisode == 2) // АДСКАЯ УТРОБА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M1: КРАТЕР
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M2: ЛАВОВЫЕ ОЧАГИ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E2M3: РЕКА ОГНЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M4: ЛЕДЯНОЙ ГРОТ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M5: КАТАКОМБЫ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M6: ЛАБИРИНТ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M7: БОЛЬШОЙ ЗАЛ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M8: ПОРТАЛЫ ХАОСА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M9: ЛЕДНИК 
	}

	if (gameepisode == 3) // КУПОЛ Д'СПАРИЛА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M1: КЛАДОВАЯ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M2: СТОЧНЫЙ КОЛОДЕЦ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M3: СЛИЯНИЕ 
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M5: ЛОГОВО ОФИДИАНОВ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E3M6: ЗАЛЫ СТРАХА
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M7: ПРОПАСТЬ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M8: КРЕПОСТЬ Д'СПАРИЛА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M9: ВОДОНОСНЫЙ СЛОЙ
	}

	if (gameepisode == 4) // СКЛЕП
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M1: КАТАФАЛК
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E4M2: УКРЫТИЕ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M4: ГРОБНИЦА
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M6: ЗАЛЫ ОТСТУПНИКОВ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M8: РАЗРУШЕННЫЙ МОСТ
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M9: МАВЗОЛЕЙ
	}

	if (gameepisode == 5) // ЗАСТОЙНЫЕ ВЛАДЕНИЯ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M1: ОХРОВЫЕ УТЕСЫ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M2: СТРЕМНИНА
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M3: ПРИЧАЛ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M4: ВНУТРЕННИЙ ДВОР
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M5: ГИДРОТИР
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M6: КОЛОННАДА 
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M7: ЗЛОВОННЫЙ ОСОБНЯК
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M8: ПОЛЕ ВЫСШЕГО СУДА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M9: НЕРАЗБЕРИХА Д'СПАРИЛА
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
    IN_DrawNumber(players[consoleplayer].killcount, 200, 65, 3);
    V_DrawShadowedPatch(237, 65, FontBSlash);
    IN_DrawNumber(totalkills, 248, 65, 3);
    if (intertime < 60)
    {
        return;
    }
    if (sounds < 2 && intertime >= 60)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }
    IN_DrawNumber(players[consoleplayer].itemcount, 200, 90, 3);
    V_DrawShadowedPatch(237, 90, FontBSlash);
    IN_DrawNumber(totalitems, 248, 90, 3);
    if (intertime < 90)
    {
        return;
    }
    if (sounds < 3 && intertime >= 90)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }
    IN_DrawNumber(players[consoleplayer].secretcount, 200, 115, 3);
    V_DrawShadowedPatch(237, 115, FontBSlash);
    IN_DrawNumber(totalsecret, 248, 115, 3);
    if (intertime < 150)
    {
        return;
    }
    if (sounds < 4 && intertime >= 150)
    {
        S_StartSound(NULL, sfx_dorcls);
        sounds++;
    }

    if (gamemode != retail || gameepisode <= 3)
    {
        IN_DrTextB(DEH_String("DHTVZ"), 85, 160);	// ВРЕМЯ
        IN_DrawTime(155, 160, hours, minutes, seconds);
    }
    else
    {
	
	// [JN] Задаём разные тексты "загружается/загружаются" для эпизодов 4 и 5.	
	if (gameepisode == 4) // СКЛЕП
	{
		if (gamemap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M1: КАТАФАЛК
		if (gamemap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M2: УКРЫТИЕ
		if (gamemap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
		if (gamemap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M4: ГРОБНИЦА
		if (gamemap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
		if (gamemap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 160); }	// E4M6: ЗАЛЫ ОТСТУПНИКОВ
		if (gamemap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 160); }	// E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
		if (gamemap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M8: РАЗРУШЕННЫЙ МОСТ
		if (gamemap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E4M9: МАВЗОЛЕЙ
	}
	
	if (gameepisode == 5) // ЗАСТОЙНЫЕ ВЛАДЕНИЯ
	{
		if (gamemap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:F>NCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:F>NCZ"), x, 160); }	// E5M1: ОХРОВЫЕ УТЕСЫ
		if (gamemap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M2: СТРЕМНИНА
		if (gamemap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M3: ПРИЧАЛ
		if (gamemap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M4: ВНУТРЕННИЙ ДВОР
		if (gamemap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M5: ГИДРОТИР
		if (gamemap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M6: КОЛОННАДА 
		if (gamemap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M7: ЗЛОВОННЫЙ ОСОБНЯК
		if (gamemap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M8: ПОЛЕ ВЫСШЕГО СУДА
		if (gamemap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2; MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160); }	// E5M9: НЕРАЗБЕРИХА Д'СПАРИЛА
	}
	
//        x = 160 - MN_TextAWidth(DEH_String("PFUHE:FTNCZ")) / 2;	// ЗАГРУЖАЕТСЯ
//        MN_DrTextA(DEH_String("PFUHE:FTNCZ"), x, 160);			// ЗАГРУЖАЕТСЯ

        x = 160 -
            MN_TextBWidth(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] +
                          7) / 2;
        IN_DrTextB(LevelNames[(gameepisode - 1) * 9 + gamemap - 1] + 7, x,
                   170);
        skipintermission = false;
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

    IN_DrTextB(DEH_String("DHFUB"), 81, 35);   // ВРАГИ (95, 35)
    IN_DrTextB(DEH_String("<JYECS"), 150, 35);  // БОНУСЫ (155, 35)
    IN_DrTextB(DEH_String("CTRHTNS"), 232, 35); // СЕКРЕТЫ (232, 35)
    x = 160 - MN_TextBWidth(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] +
                            7) / 2;
    IN_DrTextB(LevelNames[(gameepisode - 1) * 9 + prevmap - 1] + 7, x, 3);
    
	// [JN] Задаём разные тексты "завершен/завершена/завершено/завершены" для всех уровней игры. 
	// Экран совместного прохождения.
	if (gameepisode == 1) //ГОРОД ПРОКЛЯТЫХ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M1: ДОКИ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M2: ТЕМНИЦЫ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M3: ПРИВРАТНИЦКАЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M4: СТОРОЖЕВАЯ БАШНЯ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M5: ЦИТАДЕЛЬ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),   x, 25); }	// E1M6: КАФЕДРАЛЬНЫЙ СОБОР
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"),  x, 25); }	// E1M7: СКЛЕПЫ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"),  x, 25); }	// E1M8: АДСКАЯ УТРОБА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"),  x, 25); }	// E1M9: КЛАДБИЩЕ
	}
	
	if (gameepisode == 2) // АДСКАЯ УТРОБА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M1: КРАТЕР
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M2: ЛАВОВЫЕ ОЧАГИ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E2M3: РЕКА ОГНЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M4: ЛЕДЯНОЙ ГРОТ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M5: КАТАКОМБЫ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M6: ЛАБИРИНТ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M7: БОЛЬШОЙ ЗАЛ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E2M8: ПОРТАЛЫ ХАОСА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E2M9: ЛЕДНИК 
	}

	if (gameepisode == 3) // КУПОЛ Д'СПАРИЛА
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M1: КЛАДОВАЯ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M2: СТОЧНЫЙ КОЛОДЕЦ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M3: СЛИЯНИЕ 
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E3M5: ЛОГОВО ОФИДИАНОВ
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E3M6: ЗАЛЫ СТРАХА
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M7: ПРОПАСТЬ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E3M8: КРЕПОСТЬ Д'СПАРИЛА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E3M9: ВОДОНОСНЫЙ СЛОЙ
	}

	if (gameepisode == 4) // СКЛЕП
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M1: КАТАФАЛК
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYJ")) / 2; MN_DrTextA(DEH_String("PFDTHITYJ"), x, 25); }	// E4M2: УКРЫТИЕ
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M4: ГРОБНИЦА
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M6: ЗАЛЫ ОТСТУПНИКОВ
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M8: РАЗРУШЕННЫЙ МОСТ
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E4M9: МАВЗОЛЕЙ
	}

	if (gameepisode == 5) // ЗАСТОЙНЫЕ ВЛАДЕНИЯ
	{
		if (prevmap == 1 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M1: ОХРОВЫЕ УТЕСЫ
		if (prevmap == 2 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M2: СТРЕМНИНА
		if (prevmap == 3 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M3: ПРИЧАЛ
		if (prevmap == 4 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M4: ВНУТРЕННИЙ ДВОР
		if (prevmap == 5 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M5: ГИДРОТИР
		if (prevmap == 6 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M6: КОЛОННАДА 
		if (prevmap == 7 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITY"))  / 2; MN_DrTextA(DEH_String("PFDTHITY"),  x, 25); }	// E5M7: ЗЛОВОННЫЙ ОСОБНЯК
		if (prevmap == 8 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYS")) / 2; MN_DrTextA(DEH_String("PFDTHITYS"), x, 25); }	// E5M8: ПОЛЕ ВЫСШЕГО СУДА
		if (prevmap == 9 ) { x = 160 - MN_TextAWidth(DEH_String("PFDTHITYF")) / 2; MN_DrTextA(DEH_String("PFDTHITYF"), x, 25); }	// E5M9: НЕРАЗБЕРИХА Д'СПАРИЛА
	}
	
//	x = 160 - MN_TextAWidth(DEH_String("FINISHED")) / 2; MN_DrTextA(DEH_String("FINISHED"), x, 25);

    ypos = 50;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            V_DrawShadowedPatch(25, ypos,
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
			// [JN] Координаты скорректированы в 
			// соответствии с длинной русских слов.
            IN_DrawNumber(killPercent[i], 84, ypos + 10, 3);
            V_DrawShadowedPatch(120, ypos + 10, FontBPercent);
            IN_DrawNumber(bonusPercent[i], 160, ypos + 10, 3);
            V_DrawShadowedPatch(196, ypos + 10, FontBPercent);
            IN_DrawNumber(secretPercent[i], 247, ypos + 10, 3);
            V_DrawShadowedPatch(283, ypos + 10, FontBPercent);
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

    IN_DrTextB(DEH_String("BNJU"), 265, 30);	// ИТОГ
    MN_DrTextA(DEH_String(":THNDS"), 140, 8);	// ЖЕРТВЫ
    for (i = 0; i < 7; i++)
    {
        MN_DrTextA(DEH_String(KillersText[i]), 10, 80 + 9 * i);
    }
    if (intertime < 20)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {
                V_DrawShadowedPatch(40,
                                    ((ypos << FRACBITS) +
                                     dSlideY[i] * intertime) >> FRACBITS,
                                    W_CacheLumpNum(patchFaceOkayBase + i,
                                                   PU_CACHE));
                V_DrawShadowedPatch(((xpos << FRACBITS) +
                                     dSlideX[i] * intertime) >> FRACBITS, 18,
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
                V_DrawShadowedPatch(40, ypos,
                                    W_CacheLumpNum(patchFaceOkayBase + i,
                                                   PU_CACHE));
                V_DrawShadowedPatch(xpos, 18,
                                    W_CacheLumpNum(patchFaceDeadBase + i,
                                                   PU_CACHE));
            }
            else
            {
                V_DrawTLPatch(40, ypos,
                              W_CacheLumpNum(patchFaceOkayBase + i,
                                             PU_CACHE));
                V_DrawTLPatch(xpos, 18,
                              W_CacheLumpNum(patchFaceDeadBase + i,
                                             PU_CACHE));
            }
            kpos = 86;
            for (j = 0; j < MAXPLAYERS; j++)
            {
                if (playeringame[j])
                {
                    IN_DrawNumber(players[i].frags[j], kpos, ypos + 10, 3);
                    kpos += 43;
                }
            }
            if (slaughterboy & (1 << i))
            {
                if (!(intertime & 16))
                {
                    IN_DrawNumber(totalFrags[i], 263, ypos + 10, 3);
                }
            }
            else
            {
                IN_DrawNumber(totalFrags[i], 263, ypos + 10, 3);
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
        IN_DrTextB(DEH_String(";"), x + 26, y); // [JN] Ранее ":", но теперь двоеточие заменяет букву "Ж"
    }
    x += 34;
    if (m || h)
    {
        IN_DrawNumber(m, x, y, 2);
    }
    x += 34;
    if (s)
    {
        IN_DrTextB(DEH_String(";"), x - 8, y); // [JN] Ранее ":", но теперь двоеточие заменяет букву "Ж"
        IN_DrawNumber(s, x, y, 2);
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

//========================================================================
//
// IN_DrTextB
//
//========================================================================

void IN_DrTextB(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(FontBLump + c - 33, PU_CACHE);
            V_DrawShadowedPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

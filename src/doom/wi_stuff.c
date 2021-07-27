//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
// DESCRIPTION:
//	Intermission screens.
//



#include <stdio.h>

#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"
#include "w_wad.h"
#include "g_game.h"
#include "r_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "d_main.h"     // [JN] D_StartTitle

#include "rd_text.h"
// Needs access to LFB.
#include "v_video.h"
#include "st_stuff.h"
#include "wi_stuff.h"

#include "crispy.h"
#include "jn.h"

// [JN] Jaguar: prototypes
void WI_drawStatsJaguar(void);
void WI_updateStatsJaguar(void);

extern void M_WriteTextBigCentered_ENG();
extern void M_WriteTextBigCentered_RUS();
extern void HU_DemoProgressBar(void);

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES 4
#define NUMMAPS     9


// in tics
//U #define PAUSELEN		(TICRATE*2) 
//U #define SCORESTEP		100
//U #define ANIMPERIOD		32
// pixel distance from "(YOU)" to "PLAYER N"
//U #define STARDIST		10 
//U #define WK 1


// GLOBAL LOCATIONS
#define WI_TITLEY       2
#define WI_SPACINGY     33

// SINGPLE-PLAYER STUFF
#define SP_STATSX       50
#define SP_STATSY       50

#define SP_TIMEX        16
#define SP_TIMEY        (ORIGHEIGHT-32)

// NET GAME STUFF
#define NG_STATSY       50
#define NG_STATSX       (32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX     64


// DEATHMATCH STUFF
#define DM_MATRIXX      42
#define DM_MATRIXY      68

#define DM_SPACINGX     40

#define DM_TOTALSX      269

#define DM_KILLERSX     10
#define DM_KILLERSY     100
#define DM_VICTIMSX     5
#define DM_VICTIMSY     50


typedef enum
{
    ANIM_ALWAYS,
    ANIM_RANDOM,
    ANIM_LEVEL
} animenum_t;

typedef struct
{
    int x;
    int y;
} point_t;


//
// Animation.
// There is another anim_t used in p_spec.
//
typedef struct
{
    animenum_t type;

    int period;  // period in tics between animations
    int nanims;  // number of animation frames
    point_t	loc; // location of animation

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int data2; 

    // actual graphics for frames of animations
    patch_t* p[3]; 

    // following must be initialized to zero before use!

    // next value of bcnt (used in conjunction with period)
    int nexttic;

    // last drawn animation frame
    int lastdrawn;

    // next frame number to animate
    int ctr;
    
    // used by RANDOM and LEVEL when animating
    int state;  
} anim_t;


static point_t lnodes[NUMEPISODES][NUMMAPS] =
{
    // Episode 0 World Map
    {
    { 185, 164 },   // location of level 0 (CJ)
    { 148, 143 },   // location of level 1 (CJ)
    { 69,  122 },   // location of level 2 (CJ)
    { 209, 102 },   // location of level 3 (CJ)
    { 116, 89 },    // location of level 4 (CJ)
    { 166, 55 },    // location of level 5 (CJ)
    { 71,  56 },    // location of level 6 (CJ)
    { 135, 29 },    // location of level 7 (CJ)
    { 71,  24 }     // location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    {
    { 254, 25 },    // location of level 0 (CJ)
    { 97,  50 },    // location of level 1 (CJ)
    { 188, 64 },    // location of level 2 (CJ)
    { 128, 78 },    // location of level 3 (CJ)
    { 214, 92 },    // location of level 4 (CJ)
    { 133, 130 },   // location of level 5 (CJ)
    { 208, 136 },   // location of level 6 (CJ)
    { 148, 140 },   // location of level 7 (CJ)
    { 235, 158 }    // location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    {
	{ 156, 168 },   // location of level 0 (CJ)
	{ 48,  154 },   // location of level 1 (CJ)
	{ 174, 95 },    // location of level 2 (CJ)
	{ 265, 75 },    // location of level 3 (CJ)
	{ 130, 48 },    // location of level 4 (CJ)
	{ 279, 23 },    // location of level 5 (CJ)
	{ 198, 48 },    // location of level 6 (CJ)
	{ 140, 25 },    // location of level 7 (CJ)
	{ 281, 136 }    // location of level 8 (CJ)
    }
};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//

#define ANIM(type, period, nanims, x, y, nexttic)            \
   { (type), (period), (nanims), { (x), (y) }, (nexttic),    \
     0, { NULL, NULL, NULL }, 0, 0, 0, 0 }


static anim_t epsd0animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 224, 104, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 184, 160, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 112, 136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 72,  112, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 88,  96,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64,  48,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 192, 40,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 136, 16,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 80,  16,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64,  24,  0),
};

static anim_t epsd1animinfo[] =
{
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 1),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 2),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 3),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 4),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 5),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 6),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 7),
    ANIM(ANIM_LEVEL, TICRATE/3, 3, 192, 144, 8),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 8),
};

static anim_t epsd2animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 168, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 40,  136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 160, 96,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 80,  0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 120, 32,  0),
    ANIM(ANIM_ALWAYS, TICRATE/4, 3, 40,  0,   0),
};

static int NUMANIMS[NUMEPISODES] =
{
    arrlen(epsd0animinfo),
    arrlen(epsd1animinfo),
    arrlen(epsd2animinfo),
};

static anim_t *anims[NUMEPISODES] =
{
    epsd0animinfo,
    epsd1animinfo,
    epsd2animinfo
};


//
// GENERAL DATA
//

//
// Locally used stuff.
//

// States for single-player
#define SP_KILLS        0
#define SP_ITEMS        2
#define SP_SECRET       4
#define SP_FRAGS        6 
#define SP_TIME         8 
#define SP_PAR          ST_TIME

#define SP_PAUSE        1

// in seconds
#define SHOWNEXTLOCDELAY    4
//#define SHOWLASTLOCDELAY	SHOWNEXTLOCDELAY


// used to accelerate or skip a stage
static int acceleratestage;

// wbs->pnum
static int me;

 // specifies current state
static stateenum_t state;

// contains information passed into intermission
static wbstartstruct_t*  wbs;
static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int cnt;  

// used for timing of background animation
static int bcnt;

// signals to refresh everything for one frame
static int firstrefresh; 

static int cnt_kills[MAXPLAYERS];
static int cnt_items[MAXPLAYERS];
static int cnt_secret[MAXPLAYERS];
static int cnt_time;
static int cnt_par;
static int cnt_pause;

// # of commercial levels
static int NUMCMAPS; 


//
//	GRAPHICS
//

// [JN] You Are Here graphic
static patch_t*     yah[3] = { NULL, NULL, NULL }; 
static patch_t*     yah_rus[3] = { NULL, NULL, NULL }; 

// splat
static patch_t*     splat[2] = { NULL, NULL };

// %, : graphics
static patch_t*     percent;
static patch_t*     colon;

// 0-9 graphic
static patch_t*     num[10];

// minus sign
static patch_t*     wiminus;

// [JN] English patches:
static patch_t*     finished;       // "Finished"
static patch_t*     entering;       // "Entering"
static patch_t*     mp_items;       // "Items" (multiplayer)
static patch_t*     sp_secret;      // "Secret"
static patch_t*     kills;          // "Kills"
static patch_t*     secret;         // "Scrt" (multiplayer)
static patch_t*     items;          // "Items"
static patch_t*     frags;          // "Frags"
static patch_t*     mp_kills;       // "Kills" (multiplayer)
static patch_t*     timepatch;      // "Time"
static patch_t*     par;            // "Par"
static patch_t*     sucks;          // "Sucks"
static patch_t*     killers;        // "killers" (multiplayer)
static patch_t*     victims;        // "victims" (multiplayer)
static patch_t*     total;          // "Total"

// [JN] Russian patches:
static patch_t*     finished_rus;   // "Уровень завершен"
static patch_t*     entering_rus;   // "Загружается уровень"
static patch_t*     mp_items_rus;   // "Пред." (multiplayer)
static patch_t*     sp_secret_rus;  // "Тайники"
static patch_t*     kills_rus;      // "Враги"
static patch_t*     secret_rus;     // "Тайн." (multiplayer)
static patch_t*     items_rus;      // "Предметы"
static patch_t*     frags_rus;      // "Фраги"
static patch_t*     mp_kills_rus;   // "Враг." (multiplayer)
static patch_t*     timepatch_rus;  // "Время"
static patch_t*     par_rus;        // "Рекорд"
static patch_t*     sucks_rus;      // ":Ужас"
static patch_t*     killers_rus;    // "убийцы" (multiplayer)
static patch_t*     victims_rus;    // "жертвы" (multiplayer)
static patch_t*     total_rus;      // "Итог"
static patch_t*     overtime;       // "Общее время:"


// your face, your dead face
static patch_t*     star;
static patch_t*     bstar;

// "red P[1..MAXPLAYERS]"
static patch_t*     p[MAXPLAYERS];

// "gray P[1..MAXPLAYERS]"
static patch_t*     bp[MAXPLAYERS];

 // Name graphics of each level (centered)
static patch_t**    lnames;

// [JN] Russian level names
static patch_t**    lnames_d1_rus;
static patch_t**    lnames_d2_rus;
static patch_t**    lnames_plut_rus;
static patch_t**    lnames_tnt_rus;

// [JN] Unreplaceble Sigil compat Russian level names
static patch_t*     sglv20;
static patch_t*     sglv21;
static patch_t*     sglv22;
static patch_t*     sglv23;
static patch_t*     sglv24;
static patch_t*     sglv25;
static patch_t*     sglv26;
static patch_t*     sglv27;
static patch_t*     sglv28;

// [JN] Russian MAP31 and MAP32 names in BFG Edition
static patch_t*     rd_idkfa;
static patch_t*     rd_keen;

// Buffer storing the backdrop
static patch_t*     background;

//
// CODE
//

// slam background
void WI_slamBackground(void)
{
    V_DrawPatchFullScreen(background, false);
}


// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}


// Draws "<Levelname> Finished!"
void WI_drawLF(void)
{
    int y = WI_TITLEY;

    if (gamemode != commercial || wbs->last < NUMCMAPS)
    {
        // draw <LevelName> 
        if (english_language)
        {
            V_DrawShadowedPatchDoom((origwidth - SHORT(lnames[wbs->last]->width))/2, y, lnames[wbs->last]);
        }
        else
        {
            if (logical_gamemission == doom)
            {
                // [JN] Unreplaceble Sigil compat Russian level names
                if (sgl_compat_loaded && gameepisode == 3 && !english_language)
                {
                    if (gamemap == 1)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv20->width))/2, y, sglv20);
                    else if (gamemap == 2)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv21->width))/2, y, sglv21);
                    else if (gamemap == 3)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv22->width))/2, y, sglv22);
                    else if (gamemap == 4)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv23->width))/2, y, sglv23);
                    else if (gamemap == 5)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv24->width))/2, y, sglv24);
                    else if (gamemap == 6)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv25->width))/2, y, sglv25);
                    else if (gamemap == 7)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv26->width))/2, y, sglv26);
                    else if (gamemap == 8)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv27->width))/2, y, sglv27);
                    else if (gamemap == 9)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv28->width))/2, y, sglv28);
                }
                else
                {
                    V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d1_rus[wbs->last]->width))/2, y, lnames_d1_rus[wbs->last]);
                }
            }
            else if (logical_gamemission == doom2 || gamemission == pack_nerve)
            {
                // [JN] Special case for Russian level names in BFG Edition
                if (gamevariant == bfgedition)
                {
                    if (gamemap == 31)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(rd_idkfa->width))/2, y, rd_idkfa);
                    else if (gamemap == 32)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(rd_keen->width))/2, y, rd_keen);
                    else
                    V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d2_rus[wbs->last]->width))/2, y, lnames_d2_rus[wbs->last]);
                }
                else
                {
                    V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d2_rus[wbs->last]->width))/2, y, lnames_d2_rus[wbs->last]);
                }
            }
            else if (logical_gamemission == pack_plut)
            V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_plut_rus[wbs->last]->width))/2, y, lnames_plut_rus[wbs->last]);
            else if (logical_gamemission == pack_tnt)
            V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_tnt_rus[wbs->last]->width))/2, y, lnames_tnt_rus[wbs->last]);
            else
            V_DrawShadowedPatchDoom((origwidth - SHORT(lnames[wbs->last]->width))/2, y, lnames[wbs->last]);
        }
    
        // draw "Finished!"
        y += (5*SHORT(lnames[wbs->last]->height))/4;
        V_DrawShadowedPatchDoom((origwidth - SHORT
                                (english_language ? finished->width : finished_rus->width)) / 2, y, 
                                 english_language ? finished : finished_rus);
    }
    else if (wbs->last == NUMCMAPS)
    {
        // MAP33 - nothing is displayed!
    }
    else if (wbs->last > NUMCMAPS)
    {
        // > MAP33.  Doom bombs out here with a Bad V_DrawPatch error.
        // I'm pretty sure that doom2.exe is just reading into random
        // bits of memory at this point, but let's try to be accurate
        // anyway.  This deliberately triggers a V_DrawPatch error.

        patch_t tmp = { origwidth, ORIGHEIGHT, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0 } };

        V_DrawPatch(0, y, &tmp);
    }
}


// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // [JN] Draw "Entering" / "Загружается уровень"
    V_DrawShadowedPatchDoom((origwidth - SHORT
                            (english_language ? entering->width : entering_rus->width)) / 2, y,
                             english_language ? entering : entering_rus);

    // draw level
    y += (5*SHORT(lnames[wbs->next]->height))/4;

    if (english_language)
    {
        V_DrawShadowedPatchDoom((origwidth - SHORT(lnames[wbs->next]->width))/2, y, lnames[wbs->next]);
    }
    else
    {
        if (logical_gamemission == doom)
        {
            // [JN] Unreplaceble Sigil compat Russian level names
            if (sgl_compat_loaded && gameepisode == 3 && !english_language)
            {
                if (gamemap == 1)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv21->width))/2, y, sglv21);
                else if (gamemap == 2)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv22->width))/2, y, sglv22);
                else if (gamemap == 3)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv23->width))/2, y, sglv23);
                else if (gamemap == 4)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv24->width))/2, y, sglv24);
                else if (gamemap == 5)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv25->width))/2, y, sglv25);
                else if (gamemap == 6)
                {
                    if (wbs->next == 8)
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv28->width))/2, y, sglv28);
                    else
                    V_DrawShadowedPatchDoom((origwidth - SHORT(sglv26->width))/2, y, sglv26);
                }
                else if (gamemap == 7)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv27->width))/2, y, sglv27);
                else if (gamemap == 9)
                V_DrawShadowedPatchDoom((origwidth - SHORT(sglv26->width))/2, y, sglv26);
            }
            else
            {
                V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d1_rus[wbs->next]->width))/2, y, lnames_d1_rus[wbs->next]);
            }
        }
        else if (logical_gamemission == doom2 || gamemission == pack_nerve)
        {
            // [JN] Special case for Russian level names in BFG Edition
            if (gamevariant == bfgedition)
            {
                if (gamemap == 31)
                V_DrawShadowedPatchDoom((origwidth - SHORT(rd_keen->width))/2, y, rd_keen);
                else
                V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d2_rus[wbs->next]->width))/2, y, lnames_d2_rus[wbs->next]);
            }
            else
            {
                V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_d2_rus[wbs->next]->width))/2, y, lnames_d2_rus[wbs->next]);
            }
        }
        else if (logical_gamemission == pack_plut)
        V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_plut_rus[wbs->next]->width))/2, y, lnames_plut_rus[wbs->next]);
        else if (logical_gamemission == pack_tnt)
        V_DrawShadowedPatchDoom((origwidth - SHORT(lnames_tnt_rus[wbs->next]->width))/2, y, lnames_tnt_rus[wbs->next]);
        else
        V_DrawShadowedPatchDoom((origwidth - SHORT(lnames[wbs->next]->width))/2, y, lnames[wbs->next]);
    }
}

void WI_drawOnLnode (int n, patch_t* c[])
{
    int i;
    int left;
    int top;
    int right;
    int bottom;
    boolean fits = false;

    i = 0;
    do
    {
        left = lnodes[wbs->epsd][n].x - SHORT(c[i]->leftoffset);
        top = lnodes[wbs->epsd][n].y - SHORT(c[i]->topoffset);
        right = left + SHORT(c[i]->width);
        bottom = top + SHORT(c[i]->height);

        if (left >= 0 && right < origwidth && top >= 0 && bottom < ORIGHEIGHT)
        {
            fits = true;
        }
        else
        {
            i++;
        }
    } while (!fits && i!=2 && c[i] != NULL);

    // [JN] Кровавое пятно и надпись "ВЫ ЗДЕСЬ"
    if (fits && i<2)
    {
        V_DrawShadowedPatchDoom((lnodes[wbs->epsd][n].x) + wide_delta, lnodes[wbs->epsd][n].y, c[i]);
    }
    else
    {
    // DEBUG
    printf(english_language ?
           "Could not place patch on level %d" :
           "Невозможно поместить патч на уровень %d",
           n+1); 
    }
}


void WI_initAnimatedBack(void)
{
    int     i;
    anim_t* a;

    if (gamemode == commercial)
    return;

    if (wbs->epsd > 2)
    return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
        a = &anims[wbs->epsd][i];

        // init variables
        a->ctr = -1;

        // specify the next time to draw it
        if (a->type == ANIM_ALWAYS)
            a->nexttic = bcnt + 1 + (M_Random()%a->period);
        else if (a->type == ANIM_RANDOM)
            a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
        else if (a->type == ANIM_LEVEL)
            a->nexttic = bcnt + 1;
    }
}


void WI_updateAnimatedBack(void)
{
    int     i;
    anim_t* a;

    if (gamemode == commercial)
    return;

    if (wbs->epsd > 2)
    return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
        a = &anims[wbs->epsd][i];

        if (bcnt == a->nexttic)
        {
            switch (a->type)
            {
                case ANIM_ALWAYS:
                if (++a->ctr >= a->nanims) a->ctr = 0;
                a->nexttic = bcnt + a->period;
                break;

                case ANIM_RANDOM:
                a->ctr++;
                if (a->ctr == a->nanims)
                {
                    a->ctr = -1;
                    a->nexttic = bcnt+a->data2+(M_Random()%a->data1);
                }
                else a->nexttic = bcnt + a->period;
                break;

                case ANIM_LEVEL:
                // gawd-awful hack for level anims
                if (!(state == StatCount && i == 7) && wbs->next == a->data1)
                {
                    a->ctr++;
                    if (a->ctr == a->nanims) a->ctr--;
                    a->nexttic = bcnt + a->period;
                }
                break;
            }
        }
    }
}


void WI_drawAnimatedBack(void)
{
    int     i;
    anim_t* a;

    if (gamemode == commercial)
    return;

    if (wbs->epsd > 2)
    return;

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
    {
        a = &anims[wbs->epsd][i];

        if (a->ctr >= 0)
        V_DrawPatch((a->loc.x) + wide_delta, a->loc.y, a->p[a->ctr]);
    }

    // [crispy] show Fortress of Mystery if it has been completed
    if (wbs->epsd == 1 && wbs->didsecret)
    {
        a = &anims[1][7];
        V_DrawPatch((a->loc.x) + wide_delta, a->loc.y, a->p[2]);
    }
}


//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int WI_drawNum (int x, int y, int n, int digits)
{
    int fontwidth = SHORT(num[0]->width);
    int neg;
    int temp;

    if (digits < 0)
    {
        if (!n)
        {
            // make variable-length zeros 1 digit long
            digits = 1;
        }
        else
        {
            // figure out # of digits in #
            digits = 0;
            temp = n;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    neg = n < 0;
    if (neg)
    n = -n;

    // if non-number, do not draw it
    if (n == 1994)
    return 0;

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        V_DrawShadowedPatchDoom(x - wide_delta, y, num[ n % 10 ]);
        n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    V_DrawShadowedPatchDoom((x-=8) - wide_delta, y, wiminus);

    return x;
}


void WI_drawPercent (int x, int y, int p)
{
    if (p < 0)
    return;

    V_DrawShadowedPatchDoom(x - wide_delta, y, percent);

    WI_drawNum(x, y, p, -1);
}


//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
void WI_drawTime (int x, int y, int t, boolean suck)
{
    int div;
    int n;

    if (t<0)
    return;

    if (t <= 61*59 || !suck)
    {
        div = 1;

        do
        {
            n = (t / div) % 60;
            x = WI_drawNum(x, y, n, 2) - SHORT(colon->width);
            div *= 60;

            // draw
            if (div==60 || t / div)
            {
                V_DrawShadowedPatchDoom(x - wide_delta, y, colon);
            }
        } while (t / div && div < 3600);

        // [crispy] print at most in hhhh:mm:ss format
        if ((n = (t / div)))
        {
            x = WI_drawNum(x, y, n, -1);
        }
    }
    else
    {
        // "sucks"
        if (english_language)
        V_DrawShadowedPatchDoom((x - SHORT(sucks->width)) - wide_delta, y, sucks);
        else
        V_DrawShadowedPatchDoom((x - SHORT(sucks_rus->width)) - wide_delta, y, sucks_rus);
    }
}


void WI_End(void)
{
    void WI_unloadData(void);
    WI_unloadData();
}


void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}


void WI_updateNoState(void) {

    WI_updateAnimatedBack();

    if (!--cnt)
    {
        // Don't call WI_End yet.  G_WorldDone doesnt immediately 
        // change gamestate, so WI_Drawer is still going to get
        // run until that happens.  If we do that after WI_End
        // (which unloads all the graphics), we're in trouble.

        //WI_End();
        G_WorldDone();
    }
}

static boolean  snl_pointeron = false;


void WI_initShowNextLoc (void)
{
    // [crispy] display tally screen after ExM8
    if ((gamemode != commercial) && (gamemap == 8))
    {
        G_WorldDone();
        return;
    }

    state = ShowNextLoc;
    acceleratestage = 0;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    WI_initAnimatedBack();
}


void WI_updateShowNextLoc (void)
{
    WI_updateAnimatedBack();

    if (!--cnt || acceleratestage)
    WI_initNoState();
    else
    snl_pointeron = (cnt & 31) < 20;
}


void WI_drawShowNextLoc(void)
{
    int i;
    int last;

    // [JN] Press Beta: end the game after finishing level
    if (gamemode == pressbeta)
    {
        D_StartTitle ();    // [JN] back to the Main Menu
        return;             // ... and don't go any farther.
    }
    
    // [JN] Jaguar: do not erase stats, draw everything in one screen.
    if (gamemission == jaguar)
    {
        return;
    }

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    if ( gamemode != commercial)
    {
        if (wbs->epsd > 2)
        {
            WI_drawEL();
            return;
        }

        last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

        // draw a splat on taken cities.
        for (i=0 ; i<=last ; i++)
        WI_drawOnLnode(i, splat);

        // splat the secret level?
        if (wbs->didsecret)
        WI_drawOnLnode(8, splat);

        // draw flashing ptr
        if (snl_pointeron)
        WI_drawOnLnode(wbs->next, english_language ? yah : yah_rus); 
    }

    if (gamemission == pack_nerve && wbs->last == 7) 
    return;

    // draws which level you are entering..
    if ( (gamemode != commercial) || wbs->next != 30)
    WI_drawEL();  

    // [crispy] demo timer widget
    if (((demoplayback && (demotimer == 1 || demotimer == 3))
    ||   (demorecording && (demotimer == 2 || demotimer == 3))) && !vanillaparm)
    {
        ST_DrawDemoTimer(leveltime);
    }

    // [crispy] demo progress bar
    if (demoplayback && demobar && !vanillaparm)
    {
        HU_DemoProgressBar();
    }
}


void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}


int WI_fragSum(int playernum)
{
    int i;
    int frags = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i] && i!=playernum)
        {
            frags += plrs[playernum].frags[i];
        }
    }
	
    // JDC hack - negative frags.
    frags -= plrs[playernum].frags[playernum];
    // UNUSED if (frags < 0)
    // 	frags = 0;

    return frags;
}


static int dm_state;
static int dm_frags[MAXPLAYERS][MAXPLAYERS];
static int dm_totals[MAXPLAYERS];


void WI_initDeathmatchStats(void)
{
    int i;
    int j;

    state = StatCount;
    acceleratestage = 0;
    dm_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            for (j=0 ; j<MAXPLAYERS ; j++)
            if (playeringame[j])
            dm_frags[i][j] = 0;

            dm_totals[i] = 0;
        }
    }

    WI_initAnimatedBack();
}


void WI_updateDeathmatchStats(void)
{
    int i;
    int j;

    boolean stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && dm_state != 4)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
            {
                for (j=0 ; j<MAXPLAYERS ; j++)
                if (playeringame[j])
                dm_frags[i][j] = plrs[i].frags[j];

                dm_totals[i] = WI_fragSum(i);
            }
        }

        S_StartSound(0, sfx_barexp);
        dm_state = 4;
    }

    if (dm_state == 2)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
            {
                for (j=0 ; j<MAXPLAYERS ; j++)
                {
                    if (playeringame[j] && dm_frags[i][j] != plrs[i].frags[j])
                    {
                        if (plrs[i].frags[j] < 0)
                            dm_frags[i][j]--;
                        else
                            dm_frags[i][j]++;
                    
                        if (dm_frags[i][j] > 99)
                            dm_frags[i][j] = 99;
                    
                        if (dm_frags[i][j] < -99)
                            dm_frags[i][j] = -99;

                    stillticking = true;
                    }
                }
                dm_totals[i] = WI_fragSum(i);

                if (dm_totals[i] > 99)
                    dm_totals[i] = 99;

                if (dm_totals[i] < -99)
                    dm_totals[i] = -99;
            }
        }

        if (!stillticking)
        {
            S_StartSound(0, sfx_barexp);
            dm_state++;
        }
    }
    else if (dm_state == 4)
    {
        if (acceleratestage)
        {
            S_StartSound(0, sfx_slop);

            if ( gamemode == commercial)
            WI_initNoState();
            else
            WI_initShowNextLoc();
        }
    }
    else if (dm_state & 1)
    {
        if (!--cnt_pause)
        {
            dm_state++;
            cnt_pause = TICRATE;
        }
    }
}


void WI_drawDeathmatchStats(void)
{
    int i;
    int j;
    int x;
    int y;
    int w;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 
    WI_drawLF();

    // draw stat titles (top line)
    if (english_language)
    {
        V_DrawShadowedPatchDoom(DM_TOTALSX-SHORT(total->width)/2 + wide_delta, DM_MATRIXY-WI_SPACINGY+10, total);
        V_DrawShadowedPatchDoom(DM_KILLERSX + wide_delta, DM_KILLERSY, killers);
        V_DrawShadowedPatchDoom(DM_VICTIMSX + wide_delta, DM_VICTIMSY, victims);
    }
    else
    {
        V_DrawShadowedPatchDoom(DM_TOTALSX-SHORT(total_rus->width)/2 + wide_delta, DM_MATRIXY-WI_SPACINGY+10, total_rus);
        V_DrawShadowedPatchDoom(DM_KILLERSX + wide_delta, DM_KILLERSY, killers_rus);
        V_DrawShadowedPatchDoom(DM_VICTIMSX + wide_delta, DM_VICTIMSY, victims_rus);        
    }

    // draw P?
    x = DM_MATRIXX + DM_SPACINGX;
    y = DM_MATRIXY;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // [JN] Портрет игровка в режиме Дефматч
        if (playeringame[i])
        {
            V_DrawPatch((x-SHORT(p[i]->width)/2)+wide_delta, DM_MATRIXY - WI_SPACINGY, p[i]);
            V_DrawPatch((DM_MATRIXX-SHORT(p[i]->width)/2)+wide_delta, y, p[i]);

            if (i == me)
            {
                V_DrawPatch((x-SHORT(p[i]->width)/2)+wide_delta, DM_MATRIXY - WI_SPACINGY, bstar);
                V_DrawPatch((DM_MATRIXX-SHORT(p[i]->width)/2)+wide_delta, y, star);
            }
        }
        else
        {
            // V_DrawPatch(x-SHORT(bp[i]->width)/2,
            //   DM_MATRIXY - WI_SPACINGY, bp[i]);
            // V_DrawPatch(DM_MATRIXX-SHORT(bp[i]->width)/2,
            //   y, bp[i]);
        }
        x += DM_SPACINGX;
        y += WI_SPACINGY;
    }

    // draw stats
    y = DM_MATRIXY+10;
    w = SHORT(num[0]->width);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        x = DM_MATRIXX + DM_SPACINGX;

        if (playeringame[i])
        {
            for (j=0 ; j<MAXPLAYERS ; j++)
            {
            if (playeringame[j])
            WI_drawNum(x+w+wide_delta*2, y, dm_frags[i][j], 2);

            x += DM_SPACINGX;
            }
            WI_drawNum(DM_TOTALSX+w+wide_delta*2, y, dm_totals[i], 2);
        }
        y += WI_SPACINGY;
    }
}


static int  cnt_frags[MAXPLAYERS];
static int  dofrags;
static int  ng_state;

void WI_initNetgameStats(void)
{
    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
        continue;

        cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

        dofrags += WI_fragSum(i);
    }

    dofrags = !!dofrags;

    WI_initAnimatedBack();
}


void WI_updateNetgameStats(void)
{
    int i;
    int fsum;

    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && ng_state != 10)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            continue;

            cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;

            if (dofrags)
            cnt_frags[i] = WI_fragSum(i);
        }
        S_StartSound(0, sfx_barexp);
        ng_state = 10;
    }

    if (ng_state == 2)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            continue;

            cnt_kills[i] += 2;

            if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
            cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            else
            stillticking = true;
        }

        if (!stillticking)
        {
            S_StartSound(0, sfx_barexp);
            ng_state++;
        }
    }
    else if (ng_state == 4)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            continue;

            cnt_items[i] += 2;
            if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
            cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            else
            stillticking = true;
        }
        if (!stillticking)
        {
            S_StartSound(0, sfx_barexp);
            ng_state++;
        }
    }
    else if (ng_state == 6)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            continue;

            cnt_secret[i] += 2;

            if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
            cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
            else
            stillticking = true;
        }

        if (!stillticking)
        {
            S_StartSound(0, sfx_barexp);
            ng_state += 1 + 2*!dofrags;
        }
    }
    else if (ng_state == 8)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            continue;

            cnt_frags[i] += 1;

            if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
            cnt_frags[i] = fsum;
            else
            stillticking = true;
        }

        if (!stillticking)
        {
            S_StartSound(0, sfx_pldeth);
            ng_state++;
        }
    }
    else if (ng_state == 10)
    {
        if (acceleratestage)
        {
            S_StartSound(0, sfx_sgcock);
            if ( gamemode == commercial )
            WI_initNoState();
            else
            WI_initShowNextLoc();
        }
    }
    else if (ng_state & 1)
    {
        if (!--cnt_pause)
        {
            ng_state++;
            cnt_pause = TICRATE;
        }
    }
}


void WI_drawNetgameStats(void)
{
    int i;
    int x;
    int y;
    int pwidth = SHORT(percent->width);

    WI_slamBackground();
    
    // draw animated background
    WI_drawAnimatedBack(); 

    WI_drawLF();

    // draw stat titles (top line)
    // [JN] Using a slightly shortened patches for 
    // Russian language, so they can fit perfectly.
    if (english_language)
    {
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + NG_SPACINGX-SHORT(kills->width),   NG_STATSY, kills);
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 2*NG_SPACINGX-SHORT(items->width), NG_STATSY, items);
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 3*NG_SPACINGX-SHORT(secret->width),NG_STATSY, secret);
    }
    else
    {
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + NG_SPACINGX-SHORT(mp_kills_rus->width),  NG_STATSY, mp_kills_rus);
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 2*NG_SPACINGX-SHORT(mp_items_rus->width),NG_STATSY, mp_items_rus);
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 3*NG_SPACINGX-SHORT(secret_rus->width),  NG_STATSY, secret_rus);
    }

    if (dofrags)
    {
        if (english_language)
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 4*NG_SPACINGX-SHORT(frags->width), NG_STATSY, frags);
        else
        V_DrawShadowedPatchDoom(NG_STATSX + wide_delta + 4*NG_SPACINGX-SHORT(frags_rus->width), NG_STATSY, frags_rus);
    }

    // draw stats
    y = NG_STATSY + SHORT(kills->height);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
        continue;

        x = NG_STATSX;

        // [JN] Портрет игрока в режиме совместного прохождения
        V_DrawPatch((x-SHORT(p[i]->width))+wide_delta, y, p[i]);

        if (i == me)
        V_DrawPatch((x-SHORT(p[i]->width))+wide_delta, y, star);

        x += NG_SPACINGX + wide_delta*2;
        WI_drawPercent(x-pwidth, y+10, cnt_kills[i]);	x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_items[i]);	x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_secret[i]);	x += NG_SPACINGX;

        if (dofrags)
        WI_drawNum(x, y+10, cnt_frags[i], -1);

        y += WI_SPACINGY;
    }
}


static int sp_state;

void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();
}


void WI_updateStats(void)
{
    if (gamemission == jaguar)
    {
        WI_updateStatsJaguar();
        return;
    }

    WI_updateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = 0;
        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
        cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
        cnt_time = plrs[me].stime / TICRATE;
        cnt_par = wbs->partime / TICRATE;
        S_StartSound(0, sfx_barexp);
        sp_state = 10;
    }

    if (sp_state == 2)
    {
        cnt_kills[0] += 2;

        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
        {
            cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
            S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 4)
    {
        cnt_items[0] += 2;

        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
        {
            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
            S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 6)
    {
        cnt_secret[0] += 2;

        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
        {
            cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
            S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }

    else if (sp_state == 8)
    {
        if (!(bcnt&3))
        S_StartSound(0, sfx_pistol);

        cnt_time += 3;

        if (cnt_time >= plrs[me].stime / TICRATE)
            cnt_time = plrs[me].stime / TICRATE;

        cnt_par += 3;

        if (cnt_par >= wbs->partime / TICRATE)
        {
            cnt_par = wbs->partime / TICRATE;

            if (cnt_time >= plrs[me].stime / TICRATE)
            {
                S_StartSound(0, sfx_barexp);
                sp_state++;
            }
        }
    }
    else if (sp_state == 10)
    {
        if (acceleratestage)
        {
            S_StartSound(0, sfx_sgcock);

            if (gamemode == commercial)
            WI_initNoState();
            else
            WI_initShowNextLoc();
        }
    }
    else if (sp_state & 1)
    {
        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = TICRATE;
        }
    }
}


void WI_drawStats(void)
{
    int lh;	// line height

    if (gamemission == jaguar)
    {
        WI_drawStatsJaguar();
        return;
    }

    lh = (3*SHORT(num[0]->height))/2;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();
    
    WI_drawLF();

    V_DrawShadowedPatchDoom(SP_STATSX+wide_delta, SP_STATSY, 
                            english_language ? kills : kills_rus);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawShadowedPatchDoom(SP_STATSX+wide_delta, SP_STATSY+lh,
                            english_language ? items : items_rus);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    // [JN] Show secrets counter if only map have a secrets.
    // Adaptaken from Doom Retro, thanks Brad Harding!
    if (totalsecret || vanillaparm)
    {
    V_DrawShadowedPatchDoom(SP_STATSX+wide_delta, SP_STATSY+2*lh,
                            english_language ? sp_secret : sp_secret_rus);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);
    }

    V_DrawShadowedPatchDoom(SP_TIMEX+wide_delta, SP_TIMEY, 
                            english_language ? timepatch : timepatch_rus);
    
    // [JN] Press Beta: draw additional "Artifacts" counter
    if (gamemode == pressbeta)
    {
        // [JN] Draw "Arfifacts"/"Артефакты" title
        V_DrawShadowedPatchDoom(SP_STATSX+wide_delta, SP_STATSY+3*lh, 
                                W_CacheLumpName(DEH_String(english_language ?
                                                           "WIARTIF" : "RD_ARTIF"),
                                                           PU_CACHE));
        
        // [JN] Don't start counting immediately
        if (sp_state > 7)
        {
            // [JN] Number of collected artifacts
            WI_drawNum(origwidth - 78, SP_STATSY+3*lh, artifactcount, -1);

            // [JN] Draw "из" patch ("of")
            V_DrawShadowedPatchDoom((origwidth - 76)-wide_delta, SP_STATSY+3*lh, W_CacheLumpName(DEH_String("WIARTOF"), PU_CACHE));
            // [JN] Overall amount of artifacts, different for each level
            if (gameepisode == 1 && gamemap == 1)
            WI_drawNum(origwidth - 39, SP_STATSY+3*lh, 36, 2);  // Map 1: 36 artifacts
            else if (gameepisode == 2 && gamemap == 1)
            WI_drawNum(origwidth - 39, SP_STATSY+3*lh, 29, 2);  // Map 2: 29 artifacts
            else if (gameepisode == 3 && gamemap == 1)
            WI_drawNum(origwidth - 39, SP_STATSY+3*lh, 25, 2);  // Map 3: 25 artifacts
        }
    }

    WI_drawTime((origwidth/2 - SP_TIMEX)+wide_delta, SP_TIMEY, cnt_time, true);

    if (wbs->epsd < 4+1) // [JN] Sigil: extra episode
    {
        V_DrawShadowedPatchDoom(origwidth/2 + SP_TIMEX, SP_TIMEY, 
                                english_language ? par : par_rus);

        WI_drawTime(origwidth - SP_TIMEX, SP_TIMEY, cnt_par, true);
    }

    // [crispy] draw total time after level time and par time
    // [JN] Also don't show for Press Beta, there's one level per episode.
    if (!vanillaparm && gamemode != pressbeta)
    {
        if (sp_state > 8)
        {
            const int ttime = wbs->totaltimes / TICRATE;
            const boolean wide = (ttime > 61*59) || (SP_TIMEX + SHORT(total->width) >= origwidth/4);

            // [JN] Perfected positioning for both languages
            if (english_language)
            {
                V_DrawShadowedPatchDoom((SP_TIMEX)+wide_delta, SP_TIMEY + 16, total);
                // [crispy] choose x-position depending on width of time string
                WI_drawTime((wide ? origwidth : origwidth/2) - SP_TIMEX + (wide ? 0 : wide_delta), SP_TIMEY + 16, ttime, false);
            }
            else
            {
                // [JN] Choose x-position for long Russian "Общее время"
                V_DrawShadowedPatchDoom(SP_TIMEX + (wide ? 0 : 24) + wide_delta,
                                        SP_TIMEY + 16, overtime);
                // [crispy] choose x-position depending on width of time string
                WI_drawTime(SP_TIMEX + (wide ? 288 : 264) + (wide_delta * 2),
                            SP_TIMEY + 16, ttime, false);
            }
        }
    }

    // [crispy] demo timer widget
    if (((demoplayback && (demotimer == 1 || demotimer == 3))
    ||   (demorecording && (demotimer == 2 || demotimer == 3))) && !vanillaparm)
    {
        ST_DrawDemoTimer(leveltime);
    }

    // [crispy] demo progress bar
    if (demoplayback && demobar && !vanillaparm)
    {
        HU_DemoProgressBar();
    }
}


void WI_checkForAccelerate(void)
{
    int       i;
    player_t* player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
        if (playeringame[i])
        {
            // [JN] Don't allow to skip by pressing "pause" button.
            if (player->cmd.buttons == (BT_SPECIAL | BTS_PAUSE))
            continue;

            // [JN] Don't allow to skip intermission stats while active menu
            if (player->cmd.buttons & BT_ATTACK && !menuactive)
            {
                if (!player->attackdown)
                {
                    acceleratestage = 1;
                }
                player->attackdown = true;
            }
            else
            {
                player->attackdown = false;
            }

            // [JN] Don't allow to skip intermission stats while active menu
            if (player->cmd.buttons & BT_USE && !menuactive)
            {
                if (!player->usedown)
                {
                    acceleratestage = 1;
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


// Updates stuff each tick
void WI_Ticker(void)
{
    // [JN] Make PAUSE working properly on intermission screen
    if (paused && singleplayer)
    {
        return;
    }

    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
        // intermission music
        if ( gamemode == commercial )
        S_ChangeMusic(mus_dm2int, true);
        else
        S_ChangeMusic(mus_inter, true); 
    }

    WI_checkForAccelerate();

    switch (state)
    {
        case StatCount:
        if (deathmatch) 
        WI_updateDeathmatchStats();
        else if (netgame) 
        WI_updateNetgameStats();
        else 
        WI_updateStats();
        break;

        case ShowNextLoc:
        WI_updateShowNextLoc();
        break;
	
        case NoState:
        WI_updateNoState();
        break;
    }
}


typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

// Common load/unload function.  Iterates over all the graphics
// lumps to be loaded/unloaded into memory.

static void WI_loadUnloadData(load_callback_t callback)
{
    int     i, j;
    int     k, l, m, n;    // [JN] For language hot-swapping
    char    name[9];
    anim_t *a;

    if (gamemode == commercial)
    {
        // [JN] Load standard English Doom 2 level names
        for (i=0 ; i<NUMCMAPS ; i++)
        {
            DEH_snprintf(name, 9, "CWILV%2.2d", i);
                callback(name, &lnames[i]);
        }

        // [JN] Load Russian Doom 2 (only) level names
        for (l=0 ; l<NUMCMAPS ; l++)
        {
            DEH_snprintf(name, 9, "R2ILV%2.2d", l);
                callback(name, &lnames_d2_rus[l]);
        }

        // [JN] Russian MAP31 and MAP32 names in BFG Edition
        callback(DEH_String("R2_IDKFA"), &rd_idkfa);
        callback(DEH_String("R2_KEEN"),  &rd_keen);

        // [JN] Load Russian Plutonia (only) level names
        for (m=0 ; m<NUMCMAPS ; m++)
        {
            DEH_snprintf(name, 9, "RPILV%2.2d", m);
                callback(name, &lnames_plut_rus[m]);
        }

        // [JN] Load Russian TNT (only) level names
        for (n=0 ; n<NUMCMAPS ; n++)
        {
            DEH_snprintf(name, 9, "RTILV%2.2d", n);
                callback(name, &lnames_tnt_rus[n]);
        }
    }
    else
    {
        // [JN] Load standard English Doom 1 level names
        for (i=0 ; i<NUMMAPS ; i++)
        {
            DEH_snprintf(name, 9, "WILV%d%d", wbs->epsd, i);
                callback(name, &lnames[i]);
        }

        // [JN] Load Russian Doom 1 level names
        for (k=0 ; k<NUMMAPS ; k++)
        {
            DEH_snprintf(name, 9, "R1LV%d%d", wbs->epsd, k);
                callback(name, &lnames_d1_rus[k]);
        }

        // [JN] Load unreplaceble Sigil compat Russian level names
        if (sgl_compat_loaded)
        {
            callback(DEH_String("SGLV20"), &sglv20);
            callback(DEH_String("SGLV21"), &sglv21);
            callback(DEH_String("SGLV22"), &sglv22);
            callback(DEH_String("SGLV23"), &sglv23);
            callback(DEH_String("SGLV24"), &sglv24);
            callback(DEH_String("SGLV25"), &sglv25);
            callback(DEH_String("SGLV26"), &sglv26);
            callback(DEH_String("SGLV27"), &sglv27);
            callback(DEH_String("SGLV28"), &sglv28);
        }

        // [JN] English patches:
        callback(DEH_String("WIURH0"), &yah[0]);        // you are here
        callback(DEH_String("WIURH1"), &yah[1]);        // you are here (alt.)

        // [JN] Russian patches:
        callback(DEH_String("RD_URH0"), &yah_rus[0]);   // вы здесь
        callback(DEH_String("RD_URH1"), &yah_rus[1]);   // вы здесь (альт.)

        // splat
        callback(DEH_String("WISPLAT"), &splat[0]);

        if (wbs->epsd < 3)
        {
            for (j=0;j<NUMANIMS[wbs->epsd];j++)
            {
                a = &anims[wbs->epsd][j];
                for (i=0;i<a->nanims;i++)
                {
                    // MONDO HACK!
                    if (wbs->epsd != 1 || j != 8)
                    {
                        // animations
                        DEH_snprintf(name, 9, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                        callback(name, &a->p[i]);
                    }
                    else
                    {
                        // HACK ALERT!
                        a->p[i] = anims[1][4].p[i];
                    }
                }
            }
        }
    }

    // More hacks on minus sign.
    callback(DEH_String("WIMINUS"), &wiminus);

    for (i=0;i<10;i++)
    {
        // numbers 0-9
        DEH_snprintf(name, 9, "WINUM%d", i);
        callback(name, &num[i]);
    }

    // percent sign
    callback(DEH_String("WIPCNT"), &percent);

    // [JN] English patches:
    callback(DEH_String("WIF"),     &finished);         // "Finished"
    callback(DEH_String("WIENTER"), &entering);         // "Entering"
    callback(DEH_String("WIOSTIS"), &mp_items);         // "Items" (multiplayer)
    callback(DEH_String("WISCRT2"), &sp_secret);        // "Secret"
    callback(DEH_String("WIOSTK"),  &kills);            // "Kills"
    callback(DEH_String("WIOSTS"),  &secret);           // "Scrt" (multiplayer)
    callback(DEH_String("WIOSTI"),  &items);            // "Items"
    callback(DEH_String("WIFRGS"),  &frags);            // "Frags"
    callback(DEH_String("WIOSTKS"), &mp_kills);         // "Kills" (multiplayer)
    callback(DEH_String("WITIME"),  &timepatch);        // "Time"
    callback(DEH_String("WIPAR"),   &par);              // "Par"
    callback(DEH_String("WISUCKS"), &sucks);            // "Sucks"
    callback(DEH_String("WIKILRS"), &killers);          // "killers" (multiplayer)
    callback(DEH_String("WIVCTMS"), &victims);          // "victims" (multiplayer)
    callback(DEH_String("WIMSTT"),  &total);            // "Total"

    // [JN] Russian patches:
    callback(DEH_String("RD_WFIN"),  &finished_rus);    // "Уровень завершен"
    callback(DEH_String("RD_WENTR"), &entering_rus);    // "Загружается уровень"
    callback(DEH_String("RD_WITM"),  &mp_items_rus);    // "Пред." (multiplayer) 
    callback(DEH_String("RD_WSCRT"), &sp_secret_rus);   // "Тайники"
    callback(DEH_String("RD_WKILL"), &kills_rus);       // "Враги"
    callback(DEH_String("RD_WSCR"),  &secret_rus);      // "Тайн." (multiplayer)
    callback(DEH_String("RD_WITMS"), &items_rus);       // "Предметы"
    callback(DEH_String("RD_WFRGS"), &frags_rus);       // "Фраги"
    callback(DEH_String("RD_WKLL"),  &mp_kills_rus);    // "Враг." (multiplayer)
    callback(DEH_String("RD_WTIME"), &timepatch_rus);   // "Время"
    callback(DEH_String("RD_WPAR"),  &par_rus);         // "Рекорд"
    callback(DEH_String("RD_WSUCK"), &sucks_rus);       // ":Ужас"
    callback(DEH_String("RD_WKLRS"), &killers_rus);     // "убийцы" (multiplayer)
    callback(DEH_String("RD_WVCTM"), &victims_rus);     // "жертвы" (multiplayer)
    callback(DEH_String("RD_WTOT"),  &total_rus);       // "Итог"
    callback(DEH_String("WIOVTIME"), &overtime);        // "Общее время:"

    // french wad uses WIOBJ (?)
    // [JN] WIOBJ ("obj.") is a short version of WIOSTI ("objets")
    if (W_CheckNumForName(DEH_String("WIOBJ")) >= 0)
    {
        // "items"
        if (netgame && !deathmatch)
        callback(DEH_String("WIOBJ"), &items);
        else
        callback(DEH_String("WIOSTI"), &items);
    } 
    else
    {
        callback(DEH_String("WIOSTI"), &items);
    }

    // ":"
    callback(DEH_String("WICOLON"), &colon);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // "1,2,3,4"
        DEH_snprintf(name, 9, "STPB%d", i);
        callback(name, &p[i]);

        // "1,2,3,4"
        DEH_snprintf(name, 9, "WIBP%d", i+1);
        callback(name, &bp[i]);
    }

    // Background image

    if (gamemode == commercial)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else if (gamemode == retail && wbs->epsd == 3)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else if (sgl_loaded && wbs->epsd == 4 && W_CheckNumForName(DEH_String("SIGILINT")) != -1) // [crispy] Sigil
    {
        M_StringCopy(name, DEH_String("SIGILINT"), sizeof(name));
    }
    else
    {
        DEH_snprintf(name, sizeof(name), "WIMAP%d", wbs->epsd);
    }

    // Draw backdrop and save to a temporary buffer

    callback(name, &background);
}


static void WI_loadCallback(char *name, patch_t **variable)
{
    *variable = W_CacheLumpName(name, PU_STATIC);
}

void WI_loadData(void)
{
    if (gamemode == commercial)
    {
        NUMCMAPS = 32;
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, NULL);
        lnames_d2_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, NULL);
        lnames_plut_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, NULL);
        lnames_tnt_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, NULL);
    }
    else
    {
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS, PU_STATIC, NULL);
        lnames_d1_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS, PU_STATIC, NULL);
    }

    WI_loadUnloadData(WI_loadCallback);

    // These two graphics are special cased because we're sharing
    // them with the status bar code

    // your face
    star = W_CacheLumpName(DEH_String("STFST01"), PU_STATIC);

    // dead face
    bstar = W_CacheLumpName(DEH_String("STFDEAD0"), PU_STATIC);
}


static void WI_unloadCallback(char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}


void WI_unloadData(void)
{
    WI_loadUnloadData(WI_unloadCallback);

    // We do not free these lumps as they are shared with the status
    // bar code.

    // W_ReleaseLumpName("STFST01");
    // W_ReleaseLumpName("STFDEAD0");
}


void WI_Drawer (void)
{
    switch (state)
    {
        case StatCount:
        if (deathmatch)
        WI_drawDeathmatchStats();
        else if (netgame)
        WI_drawNetgameStats();
        else
        WI_drawStats();
        break;
	
        case ShowNextLoc:
        WI_drawShowNextLoc();
        break;
	
        case NoState:
        WI_drawNoState();
        break;
    }
}


void WI_initVariables (wbstartstruct_t* wbstartstruct)
{
    wbs = wbstartstruct;

#ifdef RANGECHECKING
    if (gamemode != commercial)
    {
        if ( gamemode == retail )
        RNGCHECK(wbs->epsd, 0, 3);
        else
        RNGCHECK(wbs->epsd, 0, 2);
    }
    else
    {
        RNGCHECK(wbs->last, 0, 8);
        RNGCHECK(wbs->next, 0, 8);
    }
        RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
        RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
    wbs->maxkills = 1;

    if (!wbs->maxitems)
    wbs->maxitems = 1;

    if (!wbs->maxsecret)
    wbs->maxsecret = 1;

    if (gamemode != retail && (!sgl_loaded || wbs->epsd == 3))
        if (wbs->epsd > 2)
            wbs->epsd -= 3;
}


void WI_Start(wbstartstruct_t* wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();

    if (deathmatch)
    WI_initDeathmatchStats();
    else if (netgame)
    WI_initNetgameStats();
    else
    WI_initStats();
}


// =============================================================================
//
// [JN] Jaguar Doom code
//
// =============================================================================

// 
// Level names for Jaguar, used on intermission screen
// 

char *LevelNamesJaguar[] = {
    "",
    "Hangar",
    "Plant",
    "Toxin Refinery",
    "Command Control",
    "Phobos Lab",
    "Central Processing",
    "Computer Station",
    "Phobos Anomaly",
    "Deimos Anomaly",
    "Containment Area",
    "Refinery",
    "Deimos Lab",
    "Command Center",
    "Halls of the Damned",
    "Spawning Vats",
    "Tower of Babel",
    "Hell Keep",
    "Pandemonium",
    "House of Pain",
    "Unholy Cathedral",
    "Mt. Erebus",
    "Limbo",
    "Dis",
    "Military Base",
    "Extra Map",
    "Warrens",
};

char *LevelNamesJaguar_Russian[] = {
    "",
    "Fyufh",                        // Ангар
    "\"ktrnhjcnfywbz",              // Электростанция
    "Pfdjl gj gththf,jnrt",         // Завод по переработке
    "Rjvfylysq geyrn",              // Командный пункт
    "Kf,jhfnjhbz yf Aj,jct",        // Лаборатория на Фобосе
    "Geyrn j,hf,jnrb",              // Пункт обработки
    "Dsxbckbntkmysq wtynh",         // Вычислительный центр
    "Fyjvfkbz yf Aj,jct",           // Аномалия на Фобосе
    "Fyjvfkbz yf Ltqvjct",          // Аномалия на Деймосе
    "{hfybkbot",                    // Хранилище
    "Jxbcnbntkmysq pfdjl",          // Очистительный завод
    "Kf,-hbz yf Ltqvjct",           // Лаб-рия на Деймосе
    "Rjvfylysq wtynh",              // Командный центр
    "Pfks ghjrkzns[",               // Залы проклятых
    "Ythtcnbkbot",                  // Нерестилище
    "Dfdbkjycrfz ,fiyz",            // Вавилонская башня
    "Rhtgjcnm Flf",                 // Крепость Ада
    "Gfyltvjybq",                   // Пандемоний
    "Ljv ,jkb",                     // Дом боли
    "Ytxtcnbdsq cj,jh",             // Нечестивый собор
    "Ujhf \"ht,",                   // Гора Эреб
    "Kbv,",                         // Лимб
    "Lbn",                          // Дит
    "Djtyyfz ,fpf",                 // Военная база
    "\"rcnhf-htfkmyjcnm",           // Экстра-реальность
    "Rhjkbxbq cfl",                 // Кроличий сад
};


// -----------------------------------------------------------------------------
// [JN] WI_updateStatsJaguar
// Emulates Jaguar intermission screen
// -----------------------------------------------------------------------------

void WI_updateStatsJaguar(void)
{
    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = 0;
        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
        cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
        cnt_time = plrs[me].stime / TICRATE;
        sp_state = 10;
    }

    // Count everything simultaneously:
    if (sp_state == 2)
    {
        cnt_kills[0] += 2;
        cnt_items[0] += 2;
        cnt_secret[0] += 2;
        cnt_time += 4;  // Count time 2x faster

        // Don't go higher than 100%
        if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
            cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;

        if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
            cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;

        if (cnt_time >= plrs[me].stime / TICRATE)
            cnt_time = plrs[me].stime / TICRATE;

        // Now, if all countings performed, ready to go to next level.
        // If not performed, i.e. still counting, pressing 'use' will
        // finish counting and allows to go to next level. Any questions?
        if (cnt_kills[0] == (plrs[me].skills * 100) / wbs->maxkills
        &&  cnt_items[0] == (plrs[me].sitems * 100) / wbs->maxitems
        &&  cnt_secret[0] == (plrs[me].ssecret * 100) / wbs->maxsecret
        &&  cnt_time == plrs[me].stime / TICRATE)
        {
            sp_state = 10;
        }
    }

    else if (sp_state == 10)
    {
        if (acceleratestage)
        {
            // Feedback sound
            S_StartSound(0, sfx_sgcock);
            WI_initNoState();
        }
    }
    else if (sp_state & 1)
    {
        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = 1;
        }
    }
}


// -----------------------------------------------------------------------------
// [JN] WI_drawStatsJaguar
// Emulates Jaguar intermission screen
// -----------------------------------------------------------------------------

void WI_drawStatsJaguar(void)
{
    int lh = (3*SHORT(num[0]->height))/2;   // line height

    WI_slamBackground();

    // Finished level stuff
    if (wbs->last < NUMCMAPS)
    {
        if (english_language)
        {
            M_WriteTextBigCentered_ENG(2, LevelNamesJaguar[gamemap]);
            M_WriteTextBigCentered_ENG (20, "Finished");
        }
        else
        {
            M_WriteTextBigCentered_RUS(2,LevelNamesJaguar_Russian[gamemap]);
            M_WriteTextBigCentered_RUS (20, "ehjdtym pfdthity");
        }
    }

    // Kills | Враги
    if (english_language)
        RD_M_DrawTextB("Kills", 71 + wide_delta, 50);
    else
        RD_M_DrawTextBigRUS("Dhfub", 80 + wide_delta, 50);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY, cnt_kills[0]);

    // Items | Предметы
    if (english_language)
        RD_M_DrawTextB("Items", 66 + wide_delta, 68);
    else
        RD_M_DrawTextBigRUS("Ghtlvtns", 32 + wide_delta, 68);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    // Secrets | Тайники
    if (english_language)
        RD_M_DrawTextB("Secrets", 30 + wide_delta, 86);
    else
        RD_M_DrawTextBigRUS("Nfqybrb", 45 + wide_delta, 86);

    WI_drawPercent(origwidth - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

    // Time
    if (english_language)
        RD_M_DrawTextB("Time", 74 + wide_delta, 113);
    else
        RD_M_DrawTextBigRUS("Dhtvz", 77 + wide_delta, 113);

    WI_drawTime(origwidth - SP_STATSX, SP_STATSY+4*lh-8+2, cnt_time, true);

    // Draw total times only after finishing last level
    if (gamemap == 23)
    {
        // [crispy] draw total time after level time and par time
        const int ttime = wbs->totaltimes / TICRATE;

        // Total | Итог
        if (english_language)
            RD_M_DrawTextB("Total", 59 + wide_delta, 131);
        else
            RD_M_DrawTextBigRUS("Bnju", 95 + wide_delta, 131);
        
        // Show total time only after level time is counted
        if (cnt_time == plrs[me].stime / TICRATE)
        WI_drawTime(origwidth - SP_STATSX, SP_STATSY+5*lh-8+2, ttime, false);
    }

    // Draws which level you are entering...
    // Note: do not draw "Entering Military Base" after finishing map 23
    if (gamemap != 23)
    {
        if (english_language)
        {
            M_WriteTextBigCentered_ENG (146, "Entering");
            M_WriteTextBigCentered_ENG (164, LevelNamesJaguar[wminfo.next+1]);
        }
        else
        {
            M_WriteTextBigCentered_RUS (146, "pfuhe;ftncz ehjdtym");
            M_WriteTextBigCentered_RUS (164, LevelNamesJaguar_Russian[wminfo.next+1]);
        }

    }
}

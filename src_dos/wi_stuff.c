//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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

#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "m_misc.h"
#include "r_local.h"
#include "s_sound.h"
#include "sounds.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"

#include "jn.h"


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
#define NUMEPISODES     4
#define NUMMAPS         9

// GLOBAL LOCATIONS
#define WI_TITLEY       2
#define WI_SPACINGY     33

// SINGPLE-PLAYER STUFF
#define SP_STATSX       50
#define SP_STATSY       50
#define SP_TIMEX        16
#define SP_TIMEY        (SCREENHEIGHT-32)

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
    animenum_t	type;

    
    int     period;     // period in tics between animations
    int     nanims;     // number of animation frames
    point_t loc;        // location of animation

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int     data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int     data2; 

    patch_t *p[3]; // actual graphics for frames of animations

    // following must be initialized to zero before use!

    int     nexttic;    // next value of bcnt (used in conjunction with period)
    int     lastdrawn;  // last drawn animation frame
    int	    ctr;        // next frame number to animate
    int     state;      // used by RANDOM and LEVEL when animating
} anim_t;


static point_t lnodes[NUMEPISODES][NUMMAPS] =
{
    // Episode 0 World Map
    {
    { 185, 164 },   // location of level 0 (CJ)
    { 148, 143 },   // location of level 1 (CJ)
    {  69, 122 },   // location of level 2 (CJ)
    { 209, 102 },   // location of level 3 (CJ)
    { 116,  89 },   // location of level 4 (CJ)
    { 166,  55 },   // location of level 5 (CJ)
    {  71,  56 },   // location of level 6 (CJ)
    { 135,  29 },   // location of level 7 (CJ)
    {  71,  24 }    // location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    {
    { 254,  25 },   // location of level 0 (CJ)
    {  97,  50 },   // location of level 1 (CJ)
    { 188,  64 },   // location of level 2 (CJ)
    { 128,  78 },   // location of level 3 (CJ)
    { 214,  92 },   // location of level 4 (CJ)
    { 133, 130 },   // location of level 5 (CJ)
    { 208, 136 },   // location of level 6 (CJ)
    { 148, 140 },   // location of level 7 (CJ)
    { 235, 158 }    // location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    {
    { 156, 168 },   // location of level 0 (CJ)
    {  48, 154 },   // location of level 1 (CJ)
    { 174,  95 },   // location of level 2 (CJ)
    { 265,  75 },   // location of level 3 (CJ)
    { 130,  48 },   // location of level 4 (CJ)
    { 279,  23 },   // location of level 5 (CJ)
    { 198,  48 },   // location of level 6 (CJ)
    { 140,  25 },   // location of level 7 (CJ)
    { 281, 136 }    // location of level 8 (CJ)
    }
};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//
static anim_t epsd0animinfo[] =
{
    { ANIM_ALWAYS, TICRATE/3, 3, { 224, 104 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 184, 160 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 112, 136 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  72, 112 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  88,  96 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  64,  48 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 192,  40 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 136,  16 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  80,  16 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  64,  24 } }
};

static anim_t epsd1animinfo[] =
{
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 1 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 2 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 3 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 4 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 5 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 6 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 7 },
    { ANIM_LEVEL, TICRATE/3, 3, { 192, 144 }, 8 },
    { ANIM_LEVEL, TICRATE/3, 1, { 128, 136 }, 8 }
};

static anim_t epsd2animinfo[] =
{
    { ANIM_ALWAYS, TICRATE/3, 3, { 104, 168 } },
    { ANIM_ALWAYS, TICRATE/3, 3, {  40, 136 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 160,  96 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 104,  80 } },
    { ANIM_ALWAYS, TICRATE/3, 3, { 120,  32 } },
    { ANIM_ALWAYS, TICRATE/4, 3, {  40,   0 } }
};

static int NUMANIMS[NUMEPISODES] =
{
    sizeof(epsd0animinfo)/sizeof(anim_t),
    sizeof(epsd1animinfo)/sizeof(anim_t),
    sizeof(epsd2animinfo)/sizeof(anim_t)
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
#define FB                  0

// States for single-player
#define SP_KILLS            0
#define SP_ITEMS            2
#define SP_SECRET           4
#define SP_FRAGS            6 
#define SP_TIME             8 
#define SP_PAR              ST_TIME

#define SP_PAUSE            1

// in seconds
#define SHOWNEXTLOCDELAY    4

static int      acceleratestage;    // used to accelerate or skip a stage
static int      me;                 // wbs->pnum
static int      cnt;                // used for general timing
static int      bcnt;               // used for timing of background animation

// signals to refresh everything for one frame
static int      firstrefresh; 
static int      cnt_kills[MAXPLAYERS];
static int      cnt_items[MAXPLAYERS];
static int      cnt_secret[MAXPLAYERS];
static int      cnt_time;
static int      cnt_par;
static int      cnt_pause;

// # of commercial levels
static int      NUMCMAPS;

// [JN] Intermission screen stuff
static int		dm_state;
static int		dm_frags[MAXPLAYERS][MAXPLAYERS];
static int		dm_totals[MAXPLAYERS];
static int      cnt_frags[MAXPLAYERS];
static int      dofrags;
static int      ng_state;
static int      sp_state;

static boolean  snl_pointeron = false;

static stateenum_t        state;    // specifies current state
static wbstartstruct_t   *wbs;      // contains information passed into intermission
static wbplayerstruct_t  *plrs;     // wbs->plyr[]


//
//	GRAPHICS
//

static patch_t  *bg;            // background (map of levels)
static patch_t  *yah[2];        // You Are Here graphic
static patch_t  *yah_rus[2];    // [JN] You Are Here graphic (Russian)
static patch_t  *splat;         // splat
static patch_t  *percent;       // %
static patch_t  *colon;         // :
static patch_t  *num[10];       // 0-9 graphic
static patch_t  *wiminus;       // minus sign

// [JN] English intermission screen patches:
static patch_t  *finished;      // "Finished"
static patch_t  *entering;      // "Entering"
static patch_t  *sp_secret;     // "Secret" 
static patch_t  *kills;         // "Kills"
static patch_t  *secret;        // "Scrt" (multiplayer)
static patch_t  *items;         // "Items"
static patch_t  *frags;         // "Frags"
static patch_t  *time;          // "Time"
static patch_t  *par;           // "Par"
static patch_t  *sucks;         // "Sucks"
static patch_t  *killers;       // "killers" (multiplayer)
static patch_t  *victims;       // "victims" (multiplayer)
static patch_t  *total;         // "Total"

// [JN] Russian intermission screen patches:
static patch_t  *finished_rus;  // "Уровень завершен"
static patch_t  *entering_rus;  // "Загружается уровень"
static patch_t  *mp_items_rus;  // "Загружается уровень"
static patch_t  *sp_secret_rus; // "Тайники"
static patch_t  *kills_rus;     // "Враги"
static patch_t  *secret_rus;    // "Тайн." (multiplayer)
static patch_t  *items_rus;     // "Предметы"
static patch_t  *frags_rus;     // "Фраги"
static patch_t  *time_rus;      // "Время"
static patch_t  *par_rus;       // "Рекорд"
static patch_t  *sucks_rus;     // ":Ужас"
static patch_t  *killers_rus;   // "убийцы" (multiplayer)
static patch_t  *victims_rus;   // "жертвы" (multiplayer)
static patch_t  *total_rus;     // "Итог"
static patch_t  *overtime;      // "Общее время:"

static patch_t  *star;           // your face
static patch_t  *bstar;          // your dead face
static patch_t  *p[MAXPLAYERS];  // "red P[1..MAXPLAYERS]"
static patch_t  *bp[MAXPLAYERS]; // "gray P[1..MAXPLAYERS]"

 // Name graphics of each level (centered)
static patch_t  **lnames;

// [JN] Russian level names
static patch_t  **lnames_d1_rus;
static patch_t  **lnames_d2_rus;
static patch_t  **lnames_plut_rus;
static patch_t  **lnames_tnt_rus;


//
// CODE
//

void WI_slamBackground(void)
{
    V_CopyRect(0, 0, 1, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0);  // killough 11/98
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

    // draw <LevelName> 
    if (english_language)
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames[wbs->last]->width))/2,
                    y, FB, lnames[wbs->last]);
    }
    else
    {
        if (shareware || registered || retail)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_d1_rus[wbs->last]->width))/2,
                        y, FB, lnames_d1_rus[wbs->last]);
        }
        else if (plutonia)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_plut_rus[wbs->last]->width))/2,
                        y, FB, lnames_plut_rus[wbs->last]);
        }
        else if (tnt)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_tnt_rus[wbs->last]->width))/2,
                         y, FB, lnames_tnt_rus[wbs->last]);
        }
        else
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_d2_rus[wbs->last]->width))/2,
                        y, FB, lnames_d2_rus[wbs->last]);
        }
    }

    // draw "Finished!"
    y += (5*SHORT(lnames[wbs->last]->height))/4;

    if (english_language)
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(finished->width))/2, y, FB, finished);
    }
    else
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(finished_rus->width))/2, y, FB, finished_rus);
    }
}


// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // draw "Entering"
    if (english_language)
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(entering->width))/2, y, FB, entering);
    }
    else
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(entering_rus->width))/2, y, FB, entering_rus);
    }

    // draw level
    y += (5*SHORT(lnames[wbs->next]->height))/4;

    if (english_language)
    {
        V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames[wbs->next]->width))/2, y, FB, lnames[wbs->next]);
    }
    else
    {
        if (shareware || registered || retail)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_d1_rus[wbs->next]->width))/2,
                        y, FB, lnames_d1_rus[wbs->next]);
        }
        else if (plutonia)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_plut_rus[wbs->next]->width))/2,
                        y, FB, lnames_plut_rus[wbs->next]);
        }
        else if (tnt)
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_tnt_rus[wbs->next]->width))/2,
                        y, FB, lnames_tnt_rus[wbs->next]);
        }
        else
        {
            V_DrawShadowedPatch((SCREENWIDTH - SHORT(lnames_d2_rus[wbs->next]->width))/2,
                        y, FB, lnames_d2_rus[wbs->next]);
        }
    }
}


void WI_drawOnLnode (int n, patch_t *c[])
{
    int     i;
    int     left;
    int     top;
    int     right;
    int     bottom;
    boolean fits = false;

    i = 0;

    do
    {
        left = lnodes[wbs->epsd][n].x - SHORT(c[i]->leftoffset);
        top = lnodes[wbs->epsd][n].y - SHORT(c[i]->topoffset);
        right = left + SHORT(c[i]->width);
        bottom = top + SHORT(c[i]->height);

        if (left >= 0 && right < SCREENWIDTH && top >= 0 && bottom < SCREENHEIGHT)
        {
            fits = true;
        }
        else
        {
            i++;
        }
    } while (!fits && i!=2);

    if (fits && i<2)
    {
        V_DrawShadowedPatch(lnodes[wbs->epsd][n].x, lnodes[wbs->epsd][n].y, FB, c[i]);
    }
    else
    {
        // DEBUG
        printf(english_language ?
        "Could not place patch on level %d" :
        "Невозможно разместить патч на уровне %d", n+1); 
    }
}


void WI_initAnimatedBack (void)
{
    int     i;
    anim_t *a;

    if (commercial)
    {
        return;
    }

    if (wbs->epsd > 2)
    {
        return;
    }

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
    {
        a = &anims[wbs->epsd][i];

        // init variables
        a->ctr = -1;

        // specify the next time to draw it
        if (a->type == ANIM_ALWAYS)
        {
            a->nexttic = bcnt + 1 + (M_Random()%a->period);
        }
        else if (a->type == ANIM_RANDOM)
        {
            a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
        }
        else if (a->type == ANIM_LEVEL)
        {
            a->nexttic = bcnt + 1;
        }
    }
}


void WI_updateAnimatedBack (void)
{
    int      i;
    anim_t  *a;

    if (commercial)
    {
        return;
    }

    if (wbs->epsd > 2)
    {
        return;
    }

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
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
                else
                {
                    a->nexttic = bcnt + a->period;
                }
                break;

                case ANIM_LEVEL:
                // gawd-awful hack for level anims
                if (!(state == StatCount && i == 7) && wbs->next == a->data1)
                {
                    a->ctr++;
                    if (a->ctr == a->nanims)
                    {
                        a->ctr--;
                    }
                    a->nexttic = bcnt + a->period;
                }
                break;
            }
        }
    }
}


void WI_drawAnimatedBack (void)
{
    int      i;
    anim_t  *a;

    if (commercial)
    {
        return;
    }

    if (wbs->epsd > 2)
    {
        return;
    }

    // [JN] Pre-draw Tower of Babel patch to prevent animation blinking.
    if (wbs->epsd == 1 && gamemap != 8)
    {
        a = &anims[1][gamemap-1];
        V_DrawPatch(a->loc.x, a->loc.y, FB, a->p[0]);
    }

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
    {
        a = &anims[wbs->epsd][i];

        if (a->ctr >= 0)
        {
            V_DrawPatch(a->loc.x, a->loc.y, FB, a->p[a->ctr]);
        }
    }

    // [crispy] show Fortress of Mystery if it has been completed
    if (wbs->epsd == 1 && wbs->didsecret)
    {
        a = &anims[1][7];
        V_DrawPatch(a->loc.x, a->loc.y, 0, a->p[2]);
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
    {
        n = -n;
    }

    // if non-number, do not draw it
    if (n == 1994)
    {
        return 0;
    }

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        V_DrawShadowedPatch(x, y, FB, num[ n % 10 ]);
        n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    {
        V_DrawShadowedPatch(x-=8, y, FB, wiminus);
    }

    return x;
}


void WI_drawPercent (int x, int y, int p)
{
    if (p < 0)
    {
        return;
    }

    V_DrawShadowedPatch(x, y, FB, percent);
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
    {
        return;
    }

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
                V_DrawShadowedPatch(x, y, FB, colon);
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
        {
            V_DrawShadowedPatch(x - SHORT(sucks->width), y, FB, sucks);
        }
        else
        {
            V_DrawShadowedPatch(x - SHORT(sucks_rus->width), y, FB, sucks_rus);
        }
    }
}


void WI_End (void)
{
    void WI_unloadData(void);
    WI_unloadData();
}


void WI_initNoState (void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}

void WI_updateNoState (void)
{
    WI_updateAnimatedBack();

    if (!--cnt)
    {
        WI_End();
        G_WorldDone();
    }

}


void WI_initShowNextLoc (void)
{
    // [crispy] display tally screen after ExM8
    if (!commercial && gamemap == 8)
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
    {
        WI_initNoState();
    }
    else
    {
        snl_pointeron = (cnt & 31) < 20;
    }
}


void WI_drawShowNextLoc (void)
{
    int i;
    int last;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    if (!commercial)
    {
        if (wbs->epsd > 2)
        {
            WI_drawEL();
            return;
        }

        last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

        // draw a splat on taken cities.
        for (i=0 ; i<=last ; i++)
        {
            WI_drawOnLnode(i, &splat);
        }

        // splat the secret level?
        if (wbs->didsecret)
        {
            WI_drawOnLnode(8, &splat);
        }

        // draw flashing ptr
        if (snl_pointeron)
        {
            WI_drawOnLnode(wbs->next, english_language ? yah : yah_rus);
        }
    }

    // draws which level you are entering..
    if ( (!commercial) || wbs->next != 30)
    {
        WI_drawEL();
    }
}


void WI_drawNoState (void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}


int WI_fragSum (int playernum)
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

    return frags;
}


void WI_initDeathmatchStats (void)
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


void WI_updateDeathmatchStats (void)
{
    int     i;
    int     j;
    boolean	stillticking;

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
        {
            S_StartSound(0, sfx_pistol);
        }

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
                        {
                            dm_frags[i][j]--;
                        }
                        else
                        {
                            dm_frags[i][j]++;
                        }

                        if (dm_frags[i][j] > 99)
                        {
                            dm_frags[i][j] = 99;
                        }
                        if (dm_frags[i][j] < -99)
                        {
                            dm_frags[i][j] = -99;
                        }

                        stillticking = true;
                    }
                }

                dm_totals[i] = WI_fragSum(i);

                if (dm_totals[i] > 99)
                {
                    dm_totals[i] = 99;
                }
            
                if (dm_totals[i] < -99)
                {
                    dm_totals[i] = -99;
                }
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

            if (commercial)
            {
                WI_initNoState();
            }
            else
            {
                WI_initShowNextLoc();
            }
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


void WI_drawDeathmatchStats (void)
{
    int i;
    int j;
    int x;
    int y;
    int w;
    int lh;	// line height

    lh = WI_SPACINGY;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 
    WI_drawLF();

    // draw stat titles (top line)
    if (english_language)
    {
        V_DrawShadowedPatch(DM_TOTALSX-SHORT(total->width)/2,
                            DM_MATRIXY-WI_SPACINGY+10, FB, total);

        V_DrawShadowedPatch(DM_KILLERSX, DM_KILLERSY, FB, killers);

        V_DrawShadowedPatch(DM_VICTIMSX, DM_VICTIMSY, FB, victims);
    }
    else
    {
        V_DrawShadowedPatch(DM_TOTALSX-SHORT(total_rus->width)/2,
                            DM_MATRIXY-WI_SPACINGY+10, FB, total_rus);

        V_DrawShadowedPatch(DM_KILLERSX, DM_KILLERSY, FB, killers_rus);

        V_DrawShadowedPatch(DM_VICTIMSX, DM_VICTIMSY, FB, victims_rus);
    }

    // draw P?
    x = DM_MATRIXX + DM_SPACINGX;
    y = DM_MATRIXY;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            V_DrawShadowedPatch(x-SHORT(p[i]->width)/2, DM_MATRIXY - WI_SPACINGY, FB, p[i]);

            V_DrawShadowedPatch(DM_MATRIXX-SHORT(p[i]->width)/2, y, FB, p[i]);

            if (i == me)
            {
                V_DrawPatch(x-SHORT(p[i]->width)/2, 
                            DM_MATRIXY - WI_SPACINGY,
                            FB,
                            bstar);

                V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)/2,
                            y,
                            FB,
                            star);
            }
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
                {
                    WI_drawNum(x+w, y, dm_frags[i][j], 2);
                }
        
                x += DM_SPACINGX;
            }

            WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
        }

        y += WI_SPACINGY;
    }
}


void WI_initNetgameStats (void)
{
    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }

        cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;
        dofrags += WI_fragSum(i);
    }

    dofrags = !!dofrags;

    WI_initAnimatedBack();
}


void WI_updateNetgameStats (void)
{
    int     i;
    int     fsum;
    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && ng_state != 10)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }

            cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;

            if (dofrags)
            {
                cnt_frags[i] = WI_fragSum(i);
            }
        }

        S_StartSound(0, sfx_barexp);
        ng_state = 10;
    }

    if (ng_state == 2)
    {
        if (!(bcnt&3))
        {
            S_StartSound(0, sfx_pistol);
        }

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }

            cnt_kills[i] += 2;
        
            if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
            {
                cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            }
            else
            {
                stillticking = true;
            }
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
        {
            S_StartSound(0, sfx_pistol);
        }

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }
        
            cnt_items[i] += 2;

            if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
            {
                cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            }
            else
            {
                stillticking = true;
            }
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
        {
            S_StartSound(0, sfx_pistol);
        }

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }
    
            cnt_secret[i] += 2;
    
            if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
            {
                cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
            }
            else
            {
                stillticking = true;
            }
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
        {
            S_StartSound(0, sfx_pistol);
        }

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
            {
                continue;
            }

            cnt_frags[i] += 1;
        
            if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
            {
                cnt_frags[i] = fsum;
            }
            else
            {
                stillticking = true;
            }
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

            if (commercial)
            {
                WI_initNoState();
            }
            else
            {
                WI_initShowNextLoc();
            }
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


void WI_drawNetgameStats (void)
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
    if (english_language)
    {
        V_DrawShadowedPatch(NG_STATSX+NG_SPACINGX-SHORT(kills->width),
                            NG_STATSY, FB, kills);

        V_DrawShadowedPatch(NG_STATSX+2*NG_SPACINGX-SHORT(items->width),
                            NG_STATSY, FB, items);

        V_DrawShadowedPatch(NG_STATSX+3*NG_SPACINGX-SHORT(secret->width),
                            NG_STATSY, FB, secret);

        if (dofrags)
        {
            V_DrawShadowedPatch(NG_STATSX+4*NG_SPACINGX-SHORT(frags->width),
                                NG_STATSY, FB, frags);
        }
    }
    else
    {
        V_DrawShadowedPatch(NG_STATSX+NG_SPACINGX-SHORT(kills_rus->width),
                            NG_STATSY, FB, kills_rus);

        V_DrawShadowedPatch(NG_STATSX+2*NG_SPACINGX-SHORT(mp_items_rus->width),
                            NG_STATSY, FB, mp_items_rus);

        V_DrawShadowedPatch(NG_STATSX+3*NG_SPACINGX-SHORT(secret_rus->width),
                            NG_STATSY, FB, secret_rus);

        if (dofrags)
        {
            V_DrawShadowedPatch(NG_STATSX+4*NG_SPACINGX-SHORT(frags_rus->width),
                                NG_STATSY, FB, frags_rus);
        }
    }

    // draw stats
    y = NG_STATSY + SHORT(kills->height);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }

        x = NG_STATSX;

        V_DrawShadowedPatch(x-SHORT(p[i]->width), y, FB, p[i]);

        if (i == me)
        {
            V_DrawShadowedPatch(x-SHORT(p[i]->width), y, FB, star);
        }

        x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_kills[i]);	x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_items[i]);	x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_secret[i]);	x += NG_SPACINGX;

        if (dofrags)
        {
            WI_drawNum(x, y+10, cnt_frags[i], -1);
        }

        y += WI_SPACINGY;
    }
}


void WI_initStats (void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();
}


void WI_updateStats (void)
{
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
        {
            S_StartSound(0, sfx_pistol);
        }

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
        {
            S_StartSound(0, sfx_pistol);
        }

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
        {
            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
            S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 6)
    {
        // [JN] If no secrets on the map, skip counting immediately.
        if (!totalsecret && singleplayer && !vanilla)
        {
            sp_state += 2;
        }
        else
        {
            cnt_secret[0] += 2;

            if (!(bcnt&3))
            {
                S_StartSound(0, sfx_pistol);
            }

            if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
            {
                cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
                S_StartSound(0, sfx_barexp);
                sp_state++;
            }
        }
    }
    else if (sp_state == 8)
    {
        if (!(bcnt&3))
        {
            S_StartSound(0, sfx_pistol);
        }

        cnt_time += 3;

        if (cnt_time >= plrs[me].stime / TICRATE)
        {
            cnt_time = plrs[me].stime / TICRATE;
        }

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

            if (commercial)
            {
                WI_initNoState();
            }
            else
            {
                WI_initShowNextLoc();
            }
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


void WI_drawStats (void)
{
    int lh;	// line height

    lh = (3*SHORT(num[0]->height))/2;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();

    WI_drawLF();

    V_DrawShadowedPatch(SP_STATSX, SP_STATSY, FB,
                        english_language ? kills : kills_rus);

    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawShadowedPatch(SP_STATSX, SP_STATSY+lh, FB,
                        english_language ? items : items_rus);

    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    // [JN] Show secrets counter if only map have a secrets.
    // Adaptaken from DOOM Retro, thanks Brad Harding!
    if (totalsecret || vanilla)
    {
        V_DrawShadowedPatch(SP_STATSX, SP_STATSY+2*lh, FB,
                            english_language ? sp_secret : sp_secret_rus);

        WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);
    }

    V_DrawShadowedPatch(SP_TIMEX, SP_TIMEY, FB,
                        english_language ? time : time_rus);

    WI_drawTime(SCREENWIDTH/2 - SP_TIMEX, SP_TIMEY, cnt_time, true);

    if (wbs->epsd < 4+1) // [JN] Sigil: extra episode
    {
        V_DrawShadowedPatch(SCREENWIDTH/2 + SP_TIMEX, SP_TIMEY, FB,
                            english_language ? par : par_rus);

        WI_drawTime(SCREENWIDTH - SP_TIMEX, SP_TIMEY, cnt_par, true);
    }

    // [crispy] draw total time after level time and par time
    if (sp_state > 8 && !vanilla)
    {
        const int ttime = wbs->totaltimes / TICRATE;
        const boolean wide = (ttime > 61*59) || (SP_TIMEX + SHORT(total->width) >= SCREENWIDTH/4);

        if (english_language)
        {
            V_DrawShadowedPatch(SP_TIMEX, SP_TIMEY + 16, FB, total);

            // [crispy] choose x-position depending on width of time string
            WI_drawTime((wide ? 320 : 320/2) - SP_TIMEX, SP_TIMEY + 16, ttime, false);
        }
        else
        {
            // [JN] Choose x-position for long Russian "Общее время"
            V_DrawShadowedPatch(SP_TIMEX + (wide ? 0 : 24),
                                SP_TIMEY + 16, FB, overtime);

            // [crispy] choose x-position depending on width of time string
            WI_drawTime(SP_TIMEX + (wide ? 288 : 264),
                        SP_TIMEY + 16, ttime, false);
        }
    }
}


void WI_checkForAccelerate (void)
{
    int        i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
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

            if (player->cmd.buttons & BT_USE)
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

            // [JN] Pressing PAUSE should not accelerate intermission screen
            if (player->cmd.buttons & BTS_PAUSE)
            {
                acceleratestage = 0;
            }
        }
    }
}


// Updates stuff each tick
void WI_Ticker (void)
{
    // [JN] Make PAUSE working properly on intermission screen
    if (paused)
    {
	    return;
    }

    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
        // intermission music
        if (commercial)
        {
            S_ChangeMusic(mus_dm2int, true);
        }
        else
        {
            S_ChangeMusic(mus_inter, true); 
        }
    }

    WI_checkForAccelerate();

    switch (state)
    {
        case StatCount:
        if (deathmatch) WI_updateDeathmatchStats();
        else if (netgame) WI_updateNetgameStats();
        else WI_updateStats();
        break;

        case ShowNextLoc:
        WI_updateShowNextLoc();
        break;

        case NoState:
        WI_updateNoState();
        break;
    }
}


void WI_loadData (void)
{
    int		 i;
    int		 j;
    int		 k, l, m, n;    // [JN] For language hot-swapping
    char	 name[9];
    anim_t  *a;

    if (commercial)
    {
        strcpy(name, "INTERPIC");
    }
    else
    {
        sprintf(name, "WIMAP%d", wbs->epsd);
    }
    
    if (wbs->epsd == 3)
    {
        strcpy(name,"INTERPIC");
    }
    else if (sigil && wbs->epsd == 4 && W_GetNumForName("SIGILINT") != -1) // [crispy] Sigil
    {
        strcpy(name, "SIGILINT");
    }

    // background
    bg = W_CacheLumpName(name, PU_CACHE);    
    V_DrawPatch(0, 0, 1, bg);

    if (commercial)
    {
        NUMCMAPS = 32;

        // [JN] Load standard English Doom 2 level names
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, 0);
        for (i=0 ; i<NUMCMAPS ; i++)
        {
            sprintf(name, "CWILV%2.2d", i);
            lnames[i] = W_CacheLumpName(name, PU_STATIC);
        }

        // [JN] Load Russian Doom 2 (only) level names
        lnames_d2_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, 0);
        for (k=0 ; k<NUMCMAPS ; k++)
        {
            if (commercial)
            sprintf(name, "R2ILV%2.2d", k);
            lnames_d2_rus[k] = W_CacheLumpName(name, PU_STATIC);
        }

        // [JN] Load Russian Plutonia (only) level names
        lnames_plut_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, 0);
        for (l=0 ; l<NUMCMAPS ; l++)
        {
            if (plutonia)
            sprintf(name, "RPILV%2.2d", l);
            lnames_plut_rus[l] = W_CacheLumpName(name, PU_STATIC);
        }

        // [JN] Load Russian TNT (only) level names
        lnames_tnt_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS, PU_STATIC, 0);
        for (m=0 ; m<NUMCMAPS ; m++)
        {
            if (tnt)
            sprintf(name, "RTILV%2.2d", m);
            lnames_tnt_rus[m] = W_CacheLumpName(name, PU_STATIC);
        }
    }
    else
    {
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS, PU_STATIC, 0);
        for (i=0 ; i<NUMMAPS ; i++)
        {
            sprintf(name, "WILV%d%d", wbs->epsd, i);
            lnames[i] = W_CacheLumpName(name, PU_STATIC);
        }

        lnames_d1_rus = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS, PU_STATIC, 0);
        for (n=0 ; n<NUMMAPS ; n++)
        {
            sprintf(name, "R1LV%d%d", wbs->epsd, n);
            lnames_d1_rus[n] = W_CacheLumpName(name, PU_STATIC);
        }

        // you are here
        yah[0] = W_CacheLumpName("WIURH0", PU_STATIC);
        yah_rus[0] = W_CacheLumpName("RD_URH0", PU_STATIC);

        // you are here (alt.)
        yah[1] = W_CacheLumpName("WIURH1", PU_STATIC);
        yah_rus[1] = W_CacheLumpName("RD_URH1", PU_STATIC);

        // splat
        splat = W_CacheLumpName("WISPLAT", PU_STATIC); 

        if (wbs->epsd < 3)
        {
            for (j=0 ; j<NUMANIMS[wbs->epsd] ; j++)
            {
                a = &anims[wbs->epsd][j];

                for (i=0 ; i<a->nanims ; i++)
                {
                    // MONDO HACK!
                    if (wbs->epsd != 1 || j != 8) 
                    {
                        // animations
                        sprintf(name, "WIA%d%.2d%.2d", wbs->epsd, j, i);  
                        a->p[i] = W_CacheLumpName(name, PU_STATIC);
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

    for (i=0;i<10;i++)
    {
        // numbers 0-9
        sprintf(name, "WINUM%d", i);     
        num[i] = W_CacheLumpName(name, PU_STATIC);
    }

    // [JN] English patches:
    finished = W_CacheLumpName("WIF", PU_STATIC);           // "finished"
    entering = W_CacheLumpName("WIENTER", PU_STATIC);       // "entering"
    sp_secret = W_CacheLumpName("WISCRT2", PU_STATIC);      // "secret"
    kills = W_CacheLumpName("WIOSTK", PU_STATIC);           // "kills"
    secret = W_CacheLumpName("WIOSTS", PU_STATIC);          // "scrt" (multiplayer)
	items = W_CacheLumpName("WIOSTI", PU_STATIC);           // "items"
    frags = W_CacheLumpName("WIFRGS", PU_STATIC);           // "frgs"
    time = W_CacheLumpName("WITIME", PU_STATIC);            // "time"
    par = W_CacheLumpName("WIPAR", PU_STATIC);              // "par"
    sucks = W_CacheLumpName("WISUCKS", PU_STATIC);          // "sucks"
    killers = W_CacheLumpName("WIKILRS", PU_STATIC);        // "killers" (vertical)
    victims = W_CacheLumpName("WIVCTMS", PU_STATIC);        // "victims" (horiz)
    total = W_CacheLumpName("WIMSTT", PU_STATIC);           // "total"

    // [JN] Russian patches:
    finished_rus = W_CacheLumpName("RD_WFIN", PU_STATIC);   // "Уровень завершен"
    entering_rus = W_CacheLumpName("RD_WENTR", PU_STATIC);  // "Загружается уровень"
    mp_items_rus = W_CacheLumpName("RD_WITM", PU_STATIC);  // "Загружается уровень"
    sp_secret_rus = W_CacheLumpName("RD_WSCRT", PU_STATIC); // "Тайники"
    kills_rus = W_CacheLumpName("RD_WKILL", PU_STATIC);     // "Враги"
    secret_rus = W_CacheLumpName("RD_WSCR", PU_STATIC);     // "Тайн." (multiplayer)
    items_rus = W_CacheLumpName("RD_WITMS", PU_STATIC);     // "Предметы"
    frags_rus = W_CacheLumpName("RD_WFRGS", PU_STATIC);     // "Фраги"
    time_rus = W_CacheLumpName("RD_WTIME", PU_STATIC);      // "Время"
    par_rus = W_CacheLumpName("RD_WPAR", PU_STATIC);        // "Рекорд"
    sucks_rus = W_CacheLumpName("RD_WSUCK", PU_STATIC);     // ":Ужас"
    killers_rus = W_CacheLumpName("RD_WKLRS", PU_STATIC);   // "убийцы" (multiplayer)
    victims_rus = W_CacheLumpName("RD_WVCTM", PU_STATIC);   // "жертвы" (multiplayer)
    total_rus = W_CacheLumpName("RD_WIMST", PU_STATIC);     // "Итог"
    overtime = W_CacheLumpName("WIOVTIME", PU_STATIC);      // "Общее время"    

    wiminus = W_CacheLumpName("WIMINUS", PU_STATIC);        // -
    percent = W_CacheLumpName("WIPCNT", PU_STATIC);         // %
    colon = W_CacheLumpName("WICOLON", PU_STATIC);          // :
    star = W_CacheLumpName("STFST01", PU_STATIC);           // your face
    bstar = W_CacheLumpName("STFDEAD0", PU_STATIC);         // dead face

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // "1,2,3,4"
        sprintf(name, "STPB%d", i);      
        p[i] = W_CacheLumpName(name, PU_STATIC);

        // "1,2,3,4"
        sprintf(name, "WIBP%d", i+1);     
        bp[i] = W_CacheLumpName(name, PU_STATIC);
    }
}


void WI_unloadData (void)
{
    int i;
    int j;

    Z_ChangeTag(wiminus, PU_CACHE);

    for (i=0 ; i<10 ; i++)
    {
        Z_ChangeTag(num[i], PU_CACHE);
    }

    if (commercial)
    {
        for (i=0 ; i<NUMCMAPS ; i++)
        {
            Z_ChangeTag(lnames[i], PU_CACHE);
        }
    }
    else
    {
        Z_ChangeTag(yah[0], PU_CACHE);
        Z_ChangeTag(yah[1], PU_CACHE);
        Z_ChangeTag(yah_rus[0], PU_CACHE);
        Z_ChangeTag(yah_rus[1], PU_CACHE);

        Z_ChangeTag(splat, PU_CACHE);

        for (i=0 ; i<NUMMAPS ; i++)
        {
            Z_ChangeTag(lnames[i], PU_CACHE);
        }
	
        if (wbs->epsd < 3)
        {
            for (j=0;j<NUMANIMS[wbs->epsd];j++)
            {
            if (wbs->epsd != 1 || j != 8)
                for (i=0;i<anims[wbs->epsd][j].nanims;i++)
                Z_ChangeTag(anims[wbs->epsd][j].p[i], PU_CACHE);
            }
        }
    }

    Z_Free(lnames);

    Z_ChangeTag(percent, PU_CACHE);
    Z_ChangeTag(colon, PU_CACHE);
    Z_ChangeTag(finished, PU_CACHE);
    Z_ChangeTag(entering, PU_CACHE);
    Z_ChangeTag(kills, PU_CACHE);
    Z_ChangeTag(secret, PU_CACHE);
    Z_ChangeTag(sp_secret, PU_CACHE);
    Z_ChangeTag(items, PU_CACHE);
    Z_ChangeTag(frags, PU_CACHE);
    Z_ChangeTag(time, PU_CACHE);
    Z_ChangeTag(sucks, PU_CACHE);
    Z_ChangeTag(par, PU_CACHE);
    Z_ChangeTag(victims, PU_CACHE);
    Z_ChangeTag(killers, PU_CACHE);
    Z_ChangeTag(total, PU_CACHE);
    
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        Z_ChangeTag(p[i], PU_CACHE);
    }

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        Z_ChangeTag(bp[i], PU_CACHE);
    }
}

void WI_Drawer (void)
{
    switch (state)
    {
        case StatCount:
        if (deathmatch)
        {
            WI_drawDeathmatchStats();
        }
        else if (netgame)
        {
            WI_drawNetgameStats();
        }
        else
        {
            WI_drawStats();
        }
        break;

        case ShowNextLoc:
        WI_drawShowNextLoc();
        break;

        case NoState:
        WI_drawNoState();
        break;
    }
}


#define RANGECHECKING
#define RNGCHECK(v,l,h) \
{ \
    if((v) < (l) || (v) > (h)) \
    { \
        I_Error("%s=%d in %s:%d", #v, (v), __FILE__, __LINE__); \
    } \
};


void WI_initVariables (wbstartstruct_t *wbstartstruct)
{
    wbs = wbstartstruct;

#ifdef RANGECHECKING
    if (!commercial)
    {
        RNGCHECK(wbs->epsd, 0, 3+1); // [JN] Sigil
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
    {
        wbs->maxkills = 1;
    }

    if (!wbs->maxitems)
    {
        wbs->maxitems = 1;
    }

    if (!wbs->maxsecret)
    {
        wbs->maxsecret = 1;
    }
}


void WI_Start(wbstartstruct_t *wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();

    if (deathmatch)
    {
        WI_initDeathmatchStats();
    }
    else if (netgame)
    {
        WI_initNetgameStats();
    }
    else
    {
        WI_initStats();
    }
}

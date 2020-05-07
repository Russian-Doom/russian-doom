//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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
// AM_map.c



#include <stdio.h>

#include "doomdef.h"
#include "deh_str.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_controls.h"
#include "p_local.h"
#include "am_map.h"
#include "am_data.h"
#include "m_misc.h"
#include "doomkeys.h"
#include "v_video.h"
#include "jn.h"


vertex_t KeyPoints[NUMKEYS];

char *LevelNames[] = {
    // EPISODE 1 - THE CITY OF THE DAMNED
    "E1M1:  THE DOCKS",
    "E1M2:  THE DUNGEONS",
    "E1M3:  THE GATEHOUSE",
    "E1M4:  THE GUARD TOWER",
    "E1M5:  THE CITADEL",
    "E1M6:  THE CATHEDRAL",
    "E1M7:  THE CRYPTS",
    "E1M8:  HELL'S MAW",
    "E1M9:  THE GRAVEYARD",
    // EPISODE 2 - HELL'S MAW
    "E2M1:  THE CRATER",
    "E2M2:  THE LAVA PITS",
    "E2M3:  THE RIVER OF FIRE",
    "E2M4:  THE ICE GROTTO",
    "E2M5:  THE CATACOMBS",
    "E2M6:  THE LABYRINTH",
    "E2M7:  THE GREAT HALL",
    "E2M8:  THE PORTALS OF CHAOS",
    "E2M9:  THE GLACIER",
    // EPISODE 3 - THE DOME OF D'SPARIL
    "E3M1:  THE STOREHOUSE",
    "E3M2:  THE CESSPOOL",
    "E3M3:  THE CONFLUENCE",
    "E3M4:  THE AZURE FORTRESS",
    "E3M5:  THE OPHIDIAN LAIR",
    "E3M6:  THE HALLS OF FEAR",
    "E3M7:  THE CHASM",
    "E3M8:  D'SPARIL'S KEEP",
    "E3M9:  THE AQUIFER",
    // EPISODE 4: THE OSSUARY
    "E4M1:  CATAFALQUE",
    "E4M2:  BLOCKHOUSE",
    "E4M3:  AMBULATORY",
    "E4M4:  SEPULCHER",
    "E4M5:  GREAT STAIR",
    "E4M6:  HALLS OF THE APOSTATE",
    "E4M7:  RAMPARTS OF PERDITION",
    "E4M8:  SHATTERED BRIDGE",
    "E4M9:  MAUSOLEUM",
    // EPISODE 5: THE STAGNANT DEMESNE
    "E5M1:  OCHRE CLIFFS",
    "E5M2:  RAPIDS",
    "E5M3:  QUAY",
    "E5M4:  COURTYARD",
    "E5M5:  HYDRATYR",
    "E5M6:  COLONNADE",
    "E5M7:  FOETID MANSE",
    "E5M8:  FIELD OF JUDGEMENT",
    "E5M9:  SKEIN OF D'SPARIL",
    // EPISODE 6: unnamed
    "E6M1:  ",
    "E6M2:  ",
    "E6M3:  ",
};

char *LevelNames_Rus[] = {
    // ЭПИЗОД 1 - ГОРОД ПРОКЛЯТЫХ
    "T1V1:  LJRB",                  // E1M1: ДОКИ
    "T1V2:  NTVYBWS",               // E1M2: ТЕМНИЦЫ
    "T1V3:  GHBDHFNYBWRFZ",         // E1M3: ПРИВРАТНИЦКАЯ
    "T1V4:  CNJHJ;TDFZ ,FIYZ",      // E1M4: СТОРОЖЕВАЯ БАШНЯ
    "T1V5:  WBNFLTKM",              // E1M5: ЦИТАДЕЛЬ
    "T1V6:  RFATLTHFKMYSQ CJ,JH",   // E1M6: КАФЕДРАЛЬНЫЙ СОБОР
    "T1V7:  CRKTGS",                // E1M7: СКЛЕПЫ
    "T1V8:  FLCRFZ ENHJ,F",         // E1M8: АДСКАЯ УТРОБА
    "T1V9:  RKFL,BOT",              // E1M9: КЛАДБИЩЕ
    // ЭПИЗОД 2 - АДСКАЯ УТРОБА
    "T2V1:  RHFNTH",                // E2M1: КРАТЕР
    "T2V2:  KFDJDST JXFUB",         // E2M2: ЛАВОВЫЕ ОЧАГИ
    "T2V3:  HTRF JUYZ",             // E2M3: РЕКА ОГНЯ
    "T2V4:  KTLZYJQ UHJN",          // E2M4: ЛЕДЯНОЙ ГРОТ
    "T2V5:  RFNFRJV,S",             // E2M5: КАТАКОМБЫ
    "T2V6:  KF,BHBYN",              // E2M6: ЛАБИРИНТ
    "T2V7:  ,JKMIJQ PFK",           // E2M7: БОЛЬШОЙ ЗАЛ
    "T2V8:  GJHNFKS [FJCF",         // E2M8: ПОРТАЛЫ ХАОСА
    "T2V9:  KTLYBR",                // E2M9: ЛЕДНИК 
    // ЭПИЗОД 3 - КУПОЛ Д'СПАРИЛА
    "T3V1:  RKFLJDFZ",              // E3M1: КЛАДОВАЯ
    "T3V2:  CNJXYSQ RJKJLTW",       // E3M2: СТОЧНЫЙ КОЛОДЕЦ
    "T3V3:  CKBZYBT",               // E3M3: СЛИЯНИЕ 
    "T3V4:  KFPEHYFZ RHTGJCNM",     // E3M4: ЛАЗУРНАЯ КРЕПОСТЬ
    "T3V5:  KJUJDJ JABLBFYJD",      // E3M5: ЛОГОВО ОФИДИАНОВ
    "T3V6:  PFKS CNHF[F",           // E3M6: ЗАЛЫ СТРАХА
    "T3V7:  GHJGFCNM",              // E3M7: ПРОПАСТЬ
    "T3V8:  RHTGJCNM L\"CGFHBKF",   // E3M8: КРЕПОСТЬ Д'СПАРИЛА
    "T3V9:  DJLJYJCYSQ CKJQ",       // E3M9: ВОДОНОСНЫЙ СЛОЙ
    // ЭПИЗОД 4: СКЛЕП
    "T4V1:  RFNFAFKR",              // E4M1: КАТАФАЛК
    "T4V2:  ERHSNBT",               // E4M2: УКРЫТИЕ
    "T4V3:  VJYFCNSHCRFZ UFKTHTZ",  // E4M3: МОНАСТЫРСКАЯ ГАЛЕРЕЯ
    "T4V4:  UHJ,YBWF",              // E4M4: ГРОБНИЦА
    "T4V5:  DTKBRFZ KTCNYBWF",      // E4M5: ВЕЛИКАЯ ЛЕСТНИЦА
    "T4V6:  PFKS JNCNEGYBRJD",      // E4M6: ЗАЛЫ ОТСТУПНИКОВ
    "T4V7:  NDTHLSYB GJUB,TKB",     // E4M7: ТВЕРДЫНИ ПОГИБЕЛИ
    "T4V8:  HFPHEITYYSQ VJCN",      // E4M8: РАЗРУШЕННЫЙ МОСТ
    "T4V9:  VFDPJKTQ",              // E4M9: МАВЗОЛЕЙ
    // ЭПИЗОД 5: ЗАСТОЙНЫЕ ВЛАДЕНИЯ  
    "T5V1:  J[HJDST ENTCS",         // E5M1: ОХРОВЫЕ УТЕСЫ
    "T5V2:  CNHTVYBYF",             // E5M2: СТРЕМНИНА
    "T5V3:  GHBXFK",                // E5M3: ПРИЧАЛ
    "T5V4:  DYENHTYYBQ LDJH",       // E5M4: ВНУТРЕННИЙ ДВОР
    "T5V5:  UBLHJNBH",              // E5M5: ГИДРОТИР
    "T5V6:  RJKJYYFLF",             // E5M6: КОЛОННАДА 
    "T5V7:  PKJDJYYSQ JCJ,YZR",     // E5M7: ЗЛОВОННЫЙ ОСОБНЯК
    "T5V8:  GJKT DSCITUJ CELF",     // E5M8: ПОЛЕ ВЫСШЕГО СУДА
    "T5V9:  GENFYBWF L\"CGFHBKF",   // E5M9: ПУТАНИЦА Д'СПАРИЛА
    // EPISODE 6: unnamed
    "T6V1:  ",
    "T6V2:  ",
    "T6V3:  ",
};

static int cheating = 0;

static int leveljuststarted = 1;        // kluge until AM_LevelInit() is called

boolean automapactive = false;
static int finit_height = SCREENHEIGHT - (42 << hires);
static int f_x, f_y;            // location of window on screen
static int f_w, f_h;            // size of window on screen
static int lightlev;            // used for funky strobing effect
static byte *fb;                // pseudo-frame buffer
static int amclock;

static mpoint_t m_paninc;       // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul;    // how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul;    // how far the window zooms in each tic (fb coords)

static int64_t m_x, m_y;        // LL x,y where the window is on the map (map coords)
static int64_t m_x2, m_y2;      // UR x,y where the window is on the map (map coords)

// width/height of window on map (map coords)
static int64_t m_w, m_h;
static fixed_t min_x, min_y;    // based on level size
static fixed_t max_x, max_y;    // based on level size
static fixed_t max_w, max_h;    // max_x-min_x, max_y-min_y
static fixed_t min_w, min_h;    // based on player size
static fixed_t min_scale_mtof;  // used to tell when to stop zooming out
static fixed_t max_scale_mtof;  // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr;           // the player represented by an arrow
static vertex_t oldplr;

static char cheat_amap[] = { 'r', 'a', 'v', 'm', 'a', 'p' };

static byte cheatcount = 0;

extern boolean viewactive;

#define NUMALIAS 11              // Number of antialiased lines.

static byte antialias[NUMALIAS][8] = {
    { 96,  97,  98,  99, 100, 101, 102, 103},
    {110, 109, 108, 107, 106, 105, 104, 103},
    { 75,  76,  77,  78,  79,  80,  81, 103},
    {197, 197, 196, 196, 195, 195, 194, 194},   // BLUEKEY
    {143, 143, 142, 142, 141, 141, 140, 140},   // YELLOWKEY
    {220, 220, 219, 219, 218, 218, 217, 217},   // GREENKEY
    { 43,  43,  43,  42,  42,  42,  41,  41},   // GRAYS + 3 (unrevealed walls)
    { 32,  31,  30,  29,  28,  27,  26,  25},   // WHITE
    {  8,   9,  10,  11,  12,  13,  14,  15},   // GREENS
    { 40,  40,  41,  41,  42,  42,  43,  43},   // GRAYS
    {  0,   2,   4,   6,   8,  10,  12,  14}    // BLACK
};

// [JN] Use iverted colors for automap overlay mode (softly faded to darken).
static byte antialias_overlay[NUMALIAS][8] = {
    { 99,  99,  98,  98,  97,  97,  96,  96},
    {106, 105, 104, 103, 102, 101, 100,  99},
    { 75,  75,  74,  74,  73,  73,  72,  72},
    {197, 197, 196, 196, 195, 195, 194, 194},   // BLUEKEY
    {143, 143, 142, 142, 141, 141, 140, 140},   // YELLOWKEY
    {220, 219, 218, 217, 216, 215, 214, 213},   // GREENKEY
    { 43,  42,  41,  40,  39,  38,  37,  36},   // GRAYS + 3 (unrevealed walls)
    { 32,  30,  28,  26,  24,  22,  20,  18},   // WHITE
    {  8,   7,   6,   5,   4,   3,   2,   1},   // GREENS
    { 40,  39,  39,  38,  38,  37,  37,  36},   // GRAYS
    {  0,   0,   1,   1,   2,   2,   3,   4}    // BLACK
};

static byte *maplump;           // pointer to the raw data for the automap background.
static short mapystart = 0;     // y-value for the start of the map bitmap...used in the paralax stuff.
static short mapxstart = 0;     //x-value for the bitmap.

// [crispy] automap rotate mode ...
// ... needs these early on
void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

// Functions

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits);

void AM_activateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w / 2;
    m_y -= m_h / 2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

void AM_restoreScaleAndLoc(void)
{

    m_w = old_m_w;
    m_h = old_m_h;
    if (!automap_follow)
    {
        m_x = old_m_x;
        m_y = old_m_y;
    }
    else
    {
        m_x = plr->mo->x - m_w / 2;
        m_y = plr->mo->y - m_h / 2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w << FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

void AM_findMinMaxBoundaries(void)
{
    int i;
    fixed_t a, b;

    min_x = min_y = INT_MAX;
    max_x = max_y = -INT_MAX;
    for (i = 0; i < numvertexes; i++)
    {
        if (vertexes[i].x < min_x)
            min_x = vertexes[i].x;
        else if (vertexes[i].x > max_x)
            max_x = vertexes[i].x;
        if (vertexes[i].y < min_y)
            min_y = vertexes[i].y;
        else if (vertexes[i].y > max_y)
            max_y = vertexes[i].y;
    }
    // [crispy] cope with huge level dimensions which span the entire INT range
    max_w = max_x/2 - min_x/2;
    max_h = max_y/2 - min_y/2;

    min_w = 2 * PLAYERRADIUS;
    min_h = 2 * PLAYERRADIUS;

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);

    min_scale_mtof = a < b ? a/2 : b/2;
    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);

}

void AM_changeWindowLoc(void)
{
    int64_t incx, incy;

    if (m_paninc.x || m_paninc.y)
    {
        automap_follow = 0;
        f_oldloc.x = INT_MAX;
    }

    incx = m_paninc.x;
    incy = m_paninc.y;
    if (automap_rotate)
    {
        AM_rotate(&incx, &incy, -mapangle);
    }
    m_x += incx;
    m_y += incy;

    if (m_x + m_w / 2 > max_x)
    {
        m_x = max_x - m_w / 2;
        m_paninc.x = 0;
    }
    else if (m_x + m_w / 2 < min_x)
    {
        m_x = min_x - m_w / 2;
        m_paninc.x = 0;
    }
    if (m_y + m_h / 2 > max_y)
    {
        m_y = max_y - m_h / 2;
        m_paninc.y = 0;
    }
    else if (m_y + m_h / 2 < min_y)
    {
        m_y = min_y - m_h / 2;
        m_paninc.y = 0;
    }

    // The following code was commented out in the released Heretic source,
    // but I believe we need to do this here to stop the background moving
    // when we reach the map boundaries. (In the released source it's done
    // in AM_clearFB).
    mapxstart += MTOF(m_paninc.x+FRACUNIT/2);
    mapystart -= MTOF(m_paninc.y+FRACUNIT/2);
    if(mapxstart >= screenwidth)
       mapxstart -= screenwidth;
    if(mapxstart < 0)
       mapxstart += screenwidth;
    if(mapystart >= finit_height)
       mapystart -= finit_height;
    if(mapystart < 0)
       mapystart += finit_height;
    // - end of code that was commented-out

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

void AM_initVariables(void)
{
    int pnum;
    thinker_t *think;
    mobj_t *mo;

    automapactive = true;
    fb = I_VideoBuffer;

    f_oldloc.x = INT_MAX;
    amclock = 0;
    lightlev = 0;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (!playeringame[pnum = consoleplayer])
        for (pnum = 0; pnum < MAXPLAYERS; pnum++)
            if (playeringame[pnum])
                break;
    plr = &players[pnum];
    oldplr.x = plr->mo->x;
    oldplr.y = plr->mo->y;
    m_x = plr->mo->x - m_w / 2;
    m_y = plr->mo->y - m_h / 2;
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // load in the location of keys, if in baby mode

    memset(KeyPoints, 0, sizeof(vertex_t) * 3);
    if (gameskill == sk_baby)
    {
        for (think = thinkercap.next; think != &thinkercap;
             think = think->next)
        {
            if (think->function != P_MobjThinker)
            {                   //not a mobj
                continue;
            }
            mo = (mobj_t *) think;
            if (mo->type == MT_CKEY)
            {
                KeyPoints[0].x = mo->x;
                KeyPoints[0].y = mo->y;
            }
            else if (mo->type == MT_AKYY)
            {
                KeyPoints[1].x = mo->x;
                KeyPoints[1].y = mo->y;
            }
            else if (mo->type == MT_BKYY)
            {
                KeyPoints[2].x = mo->x;
                KeyPoints[2].y = mo->y;
            }
        }
    }
}

void AM_loadPics(void)
{
    maplump = W_CacheLumpName(DEH_String("AUTOPAGE"), PU_STATIC);
}

// should be called at the start of every level
// right now, i figure it out myself

void AM_LevelInit(void)
{
    fixed_t a, b;
    leveljuststarted = 0;

    f_x = f_y = 0;
    f_w = screenwidth;
    f_h = finit_height;
    mapxstart = mapystart = 0;

    AM_findMinMaxBoundaries();
    
    // [crispy] initialize zoomlevel on all maps so that a 4096 units
    // square map would just fit in (MAP01 is 3376x3648 units)
    a = FixedDiv(f_w, (max_w>>FRACBITS < 2048) ? 2*(max_w>>FRACBITS) : 4096);
    b = FixedDiv(f_h, (max_h>>FRACBITS < 2048) ? 2*(max_h>>FRACBITS) : 4096);
    scale_mtof = FixedDiv(a < b ? a : b, (int) (0.7*FRACUNIT));

    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

static boolean stopped = true;

void AM_Stop(void)
{
    automapactive = false;
    stopped = true;
    BorderNeedRefresh = true;
}

void AM_Start(void)
{
    static int lastlevel = -1, lastepisode = -1;

    if (!stopped)
    {
        AM_Stop();
    }
    stopped = false;
    if (gamestate != GS_LEVEL)
    {
        return;                 // don't show automap if we aren't in a game!
    }
    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
}

// set the window scale to the maximum size

void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// set the window scale to the minimum size

void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

boolean AM_Responder(event_t * ev)
{
    int rc;
    int key;
    static int bigstate = 0;
    static int joywait = 0;

    key = ev->data1;
    rc = false;

    if (ev->type == ev_joystick && joybautomap >= 0
    && (ev->data1 & (1 << joybautomap)) != 0 && joywait < I_GetTime())
    {
        joywait = I_GetTime() + 5;

        if (!automapactive)
        {
            AM_Start ();
            viewactive = false;
        }
        else
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }
    }

    if (!automapactive)
    {

        if (ev->type == ev_keydown && key == key_map_toggle
         && gamestate == GS_LEVEL)
        {
            AM_Start();
            viewactive = false;
            rc = true;
        }
    }
    else if (ev->type == ev_keydown)
    {
        rc = true;

        if (key == key_map_east)                 // pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? -FTOM(F_PANINC) : FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_west)            // pan left
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? FTOM(F_PANINC) : -FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_north)           // pan up
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.y = FTOM(F_PANINC);
            }
            else
                rc = false;
        }
        else if (key == key_map_south)           // pan down
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.y = -FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_zoomout)         // zoom out
        {
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
        }
        else if (key == key_map_zoomin)          // zoom in
        {
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
        }
        else if (key == key_map_toggle)          // toggle map (tab)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop();
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else
                AM_restoreScaleAndLoc();
        }
        else if (key == key_map_follow)
        {
            automap_follow = !automap_follow;
            f_oldloc.x = INT_MAX;
            P_SetMessage(plr, automap_follow ?
                              amstr_followon : amstr_followoff, true);
        }
        else if (key == key_map_overlay)
        {
            automap_overlay = !automap_overlay;
            P_SetMessage(plr, automap_overlay ?
                              amstr_overlayon : amstr_overlayoff, true);
        }
        else if (key == key_map_rotate)
        {
            automap_rotate = !automap_rotate;
            P_SetMessage(plr, automap_rotate ?
                              amstr_rotateon : amstr_rotateoff, true);
        }
        else if (key == key_map_grid)
        {
            automap_grid = !automap_grid;
            P_SetMessage(plr, automap_grid ?
                              amstr_gridon : amstr_gridoff, true);
        }
        else
        {
            rc = false;
        }

        if (cheat_amap[cheatcount] == ev->data1 && !netgame)
        {
            cheatcount++;
        }
        else
        {
            cheatcount = 0;
        }
        if (cheatcount == 6)
        {
            cheatcount = 0;
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }

    else if (ev->type == ev_keyup)
    {
        rc = false;

        if (key == key_map_east)
        {
            if (!automap_follow)
                m_paninc.x = 0;
        }
        else if (key == key_map_west)
        {
            if (!automap_follow)
                m_paninc.x = 0;
        }
        else if (key == key_map_north)
        {
            if (!automap_follow)
                m_paninc.y = 0;
        }
        else if (key == key_map_south)
        {
            if (!automap_follow)
                m_paninc.y = 0;
        }
        else if (key == key_map_zoomout || key == key_map_zoomin)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }
    }

    return rc;
}

void AM_changeWindowScale(void)
{

    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if (scale_mtof < min_scale_mtof)
    {
        AM_minOutWindowScale();
    }
    else if (scale_mtof > max_scale_mtof)
    {
        AM_maxOutWindowScale();
    }
    else
    {
        AM_activateNewScale();
    }
}

void AM_doFollowPlayer(void)
{
    if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
        m_x = FTOM(MTOF(plr->mo->x)) - m_w / 2;
        m_y = FTOM(MTOF(plr->mo->y)) - m_h / 2;
        m_x2 = m_x + m_w;
        m_y2 = m_y + m_h;

        // do the parallax parchment scrolling.

        f_oldloc.x = plr->mo->x;
        f_oldloc.y = plr->mo->y;
    }
}

void AM_Ticker(void)
{

    if (!automapactive)
        return;

    amclock++;

    if (automap_follow)
    {
        AM_doFollowPlayer();
    }

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
    {
        AM_changeWindowScale();
    }

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
    {
        AM_changeWindowLoc();
    }

    // [crispy] required for AM_rotatePoint()
    if (automap_rotate)
    {
        mapcenter.x = m_x + m_w / 2;
        mapcenter.y = m_y + m_h / 2;
        // [crispy] keep the map static in overlay mode
        // if not following the player
        if (!(!automap_follow && automap_overlay))
        mapangle = ANG90 - viewangle;
    }
}

void AM_clearFB(int color)
{
    int i, j;

    int dmapx;
    int dmapy;
    int x, y;
    byte *src = W_CacheLumpName(DEH_String("AUTOPAGE"), PU_CACHE);
    byte *dest = I_VideoBuffer;

    if (automap_follow)
    {
        dmapx = (MTOF(plr->mo->x) - MTOF(oldplr.x));    //fixed point
        dmapy = (MTOF(oldplr.y) - MTOF(plr->mo->y));

        oldplr.x = plr->mo->x;
        oldplr.y = plr->mo->y;

        mapxstart += dmapx >> 1;
        mapystart += dmapy >> 1;

        while (mapxstart >= (screenwidth >> hires))
            mapxstart -= (screenwidth >> hires);
        while (mapxstart < 0)
            mapxstart += (screenwidth >> hires);
        while (mapystart >= (finit_height >> hires))
            mapystart -= (finit_height >> hires);
        while (mapystart < 0)
            mapystart += (finit_height >> hires);
    }
    else
    {
        mapxstart += (MTOF(m_paninc.x) >> 1);
        mapystart -= (MTOF(m_paninc.y) >> 1);

        if (mapxstart >= (screenwidth >> hires))
            mapxstart -= (screenwidth >> hires);
        if (mapxstart < 0)
            mapxstart += (screenwidth >> hires);
        if (mapystart >= (finit_height >> hires))
            mapystart -= (finit_height >> hires);
        if (mapystart < 0)
            mapystart += (finit_height >> hires);
    }

    if (widescreen == 1)
    {
        // [JN] Use static automap background for automap
        // because of parallax problem.
        for (y = 0; y < SCREENHEIGHT-28; y++)
        {
            for (x = 0; x < WIDESCREENWIDTH / 320; x++)
            {
                memcpy(dest, src + ((y & 127) << 6), 320);
                dest += 320;
            }
            if (WIDESCREENWIDTH & 127)
            {
                memcpy(dest, src + ((y & 127) << 6), WIDESCREENWIDTH & 127);
                dest += (WIDESCREENWIDTH & 127);
            }
        }
    }
    if (widescreen == 2)
    {
        // [JN] Use static automap background for automap
        // because of parallax problem.
        for (y = 0; y < SCREENHEIGHT - 21; y++)
        {
            for (x = 0; x < (WIDESCREENWIDTH - (42 << hires)) / 320; x++)
            {
                memcpy(dest, src + ((y & 127) << 6), 320);
                dest += 320;
            }
            if ((WIDESCREENWIDTH - (42 << hires)) & 127)
            {
                memcpy(dest, src + ((y & 127) << 6), (WIDESCREENWIDTH - (42 << hires)) & 127);
                dest += ((WIDESCREENWIDTH - (42 << hires)) & 127);
            }
        }
    }
    else
    {
        //blit the automap background to the screen.
        j = (mapystart & ~hires) * (SCREENWIDTH >> hires);
        for (i = 0; i < finit_height; i++)
        {
            memcpy(I_VideoBuffer + i * SCREENWIDTH, maplump + j + mapxstart,
                   SCREENWIDTH - mapxstart);
            memcpy(I_VideoBuffer + i * SCREENWIDTH + SCREENWIDTH - mapxstart,
                   maplump + j, mapxstart);
            j += SCREENWIDTH;
            if (j >= (finit_height >> hires) * (SCREENWIDTH >> hires))
                j = 0;
        }
    }
}

// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If I need the speed, will
// hash algorithm to the common cases.

boolean AM_clipMline(mline_t * ml, fline_t * fl)
{
    enum
    { LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };
    int outcode1 = 0, outcode2 = 0, outside;
    fpoint_t tmp = { 0, 0 };
    int dx, dy;

#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_w) (oc) |= RIGHT

    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
        outcode1 = TOP;
    else if (ml->a.y < m_y)
        outcode1 = BOTTOM;
    if (ml->b.y > m_y2)
        outcode2 = TOP;
    else if (ml->b.y < m_y)
        outcode2 = BOTTOM;
    if (outcode1 & outcode2)
        return false;           // trivially outside

    if (ml->a.x < m_x)
        outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
        outcode1 |= RIGHT;
    if (ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;
    if (outcode1 & outcode2)
        return false;           // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);
    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    if (outcode1 & outcode2)
        return false;

    while (outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if (outcode1)
            outside = outcode1;
        else
            outside = outcode2;
        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx * (fl->a.y)) / dy;
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx * (fl->a.y - f_h)) / dy;
            tmp.y = f_h - 1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy * (f_w - 1 - fl->a.x)) / dx;
            tmp.x = f_w - 1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy * (-fl->a.x)) / dx;
            tmp.x = 0;
        }
        if (outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y);
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y);
        }
        if (outcode1 & outcode2)
            return false;       // trivially outside
    }

    return true;
}

#undef DOOUTCODE

// Classic Bresenham w/ whatever optimizations I need for speed

void AM_drawFline(fline_t * fl, int color)
{

    register int x, y, dx, dy, sx, sy, ax, ay, d;
    static int fuck = 0;

    switch (color)
    {
        case WALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[0][0] : &antialias[0][0]), 8, 3);
            break;
        case FDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ? 
                       &antialias_overlay[1][0] : &antialias[1][0]), 8, 3);
            break;
        case CDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[2][0] : &antialias[2][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to key-locked doors
        case BLUEKEY:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[3][0] : &antialias[3][0]), 8, 3);
            break;
        case YELLOWKEY:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[4][0] : &antialias[4][0]), 8, 3);
            break;
        case GREENKEY:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[5][0] : &antialias[5][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to unrevealed walls
        case GRAYS + 3:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[6][0] : &antialias[6][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to player arrow
        case WHITE:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[7][0] : &antialias[7][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to thing triangles (GREENS = 8, not 264)
        case GREENS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[8][0] : &antialias[8][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to hidden 2-sided lines
        case GRAYS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[9][0] : &antialias[9][0]), 8, 3);
            break;
        // [JN] Apply antialiasing to hidden 2-sided lines (secret doors)
        case BLACK:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[10][0] : &antialias[10][0]), 8, 3);
            break;
        default:
            {
                // For debugging only
                if (fl->a.x < 0 || fl->a.x >= f_w
                    || fl->a.y < 0 || fl->a.y >= f_h
                    || fl->b.x < 0 || fl->b.x >= f_w
                    || fl->b.y < 0 || fl->b.y >= f_h)
                {
                    fprintf(stderr, "fuck %d \r", fuck++);
                    return;
                }

#define DOT(xx,yy,cc) fb[(yy)*f_w+(flipwidth[xx])]=(cc)    //the MACRO!

                dx = fl->b.x - fl->a.x;
                ax = 2 * (dx < 0 ? -dx : dx);
                sx = dx < 0 ? -1 : 1;

                dy = fl->b.y - fl->a.y;
                ay = 2 * (dy < 0 ? -dy : dy);
                sy = dy < 0 ? -1 : 1;

                x = fl->a.x;
                y = fl->a.y;

                if (ax > ay)
                {
                    d = ay - ax / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (x == fl->b.x)
                            return;
                        if (d >= 0)
                        {
                            y += sy;
                            d -= ax;
                        }
                        x += sx;
                        d += ay;
                    }
                }
                else
                {
                    d = ax - ay / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (y == fl->b.y)
                            return;
                        if (d >= 0)
                        {
                            x += sx;
                            d -= ay;
                        }
                        y += sy;
                        d += ax;
                    }
                }
            }
    }
}

/* Wu antialiased line drawer.
 * (X0,Y0),(X1,Y1) = line to draw
 * BaseColor = color # of first color in block used for antialiasing, the
 *          100% intensity version of the drawing color
 * NumLevels = size of color block, with BaseColor+NumLevels-1 being the
 *          0% intensity version of the drawing color
 * IntensityBits = log base 2 of NumLevels; the # of bits used to describe
 *          the intensity of the drawing color. 2**IntensityBits==NumLevels
 */
void PUTDOT(short xx, short yy, byte * cc, byte * cm)
{
    static int oldyy;
    static int oldyyshifted;
    byte *oldcc = cc;

    if (xx < 32)
    {
        cc += 7 - (xx >> 2);
    }
    else if (xx > (screenwidth - 32))
    {
        cc += 7 - ((screenwidth - xx) >> 2);
    }
    if (yy < 32)
    {
        cc += 7 - (yy >> 2);
    }
    else if (yy > (finit_height - 32))
    {
        cc += 7 - ((finit_height - yy) >> 2);
    }
    if (cc > cm && cm != NULL)
    {
        cc = cm;
    }
    else if (cc > oldcc + 6)    // don't let the color escape from the fade table...
    {
        cc = oldcc + 6;
    }
    if (yy == oldyy + 1)
    {
        oldyy++;
        oldyyshifted += (origwidth << hires);
    }
    else if (yy == oldyy - 1)
    {
        oldyy--;
        oldyyshifted -= (origwidth << hires);
    }
    else if (yy != oldyy)
    {
        oldyy = yy;
        oldyyshifted = yy * (origwidth << hires);
    }
    fb[oldyyshifted + flipwidth[xx]] = *(cc);
}

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits)
{
    unsigned short IntensityShift, ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
    short DeltaX, DeltaY, Temp, XDir;

    /* Make sure the line runs top to bottom */
    if (Y0 > Y1)
    {
        Temp = Y0;
        Y0 = Y1;
        Y1 = Temp;
        Temp = X0;
        X0 = X1;
        X1 = Temp;
    }
    /* Draw the initial pixel, which is always exactly intersected by
       the line and so needs no weighting */
    PUTDOT(X0, Y0, &BaseColor[0], NULL);

    if ((DeltaX = X1 - X0) >= 0)
    {
        XDir = 1;
    }
    else
    {
        XDir = -1;
        DeltaX = -DeltaX;       /* make DeltaX positive */
    }
    /* Special-case horizontal, vertical, and diagonal lines, which
       require no weighting because they go right through the center of
       every pixel */
    if ((DeltaY = Y1 - Y0) == 0)
    {
        /* Horizontal line */
        while (DeltaX-- != 0)
        {
            X0 += XDir;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        return;
    }
    if (DeltaX == 0)
    {
        /* Vertical line */
        do
        {
            Y0++;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        while (--DeltaY != 0);
        return;
    }
    //diagonal line.
    if (DeltaX == DeltaY)
    {
        do
        {
            X0 += XDir;
            Y0++;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        while (--DeltaY != 0);
        return;
    }
    /* Line is not horizontal, diagonal, or vertical */
    ErrorAcc = 0;               /* initialize the line error accumulator to 0 */
    /* # of bits by which to shift ErrorAcc to get intensity level */
    IntensityShift = 16 - IntensityBits;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NumLevels - 1;
    /* Is this an X-major or Y-major line? */
    if (DeltaY > DeltaX)
    {
        /* Y-major line; calculate 16-bit fixed-point fractional part of a
           pixel that X advances each time Y advances 1 pixel, truncating the
           result so that we won't overrun the endpoint along the X axis */
        ErrorAdj = ((unsigned int) DeltaX << 16) / (unsigned int) DeltaY;
        /* Draw all pixels other than the first and last */
        while (--DeltaY)
        {
            ErrorAccTemp = ErrorAcc;    /* remember currrent accumulated error */
            ErrorAcc += ErrorAdj;       /* calculate error for next pixel */
            if (ErrorAcc <= ErrorAccTemp)
            {
                /* The error accumulator turned over, so advance the X coord */
                X0 += XDir;
            }
            Y0++;               /* Y-major, so always advance Y */
            /* The IntensityBits most significant bits of ErrorAcc give us the
               intensity weighting for this pixel, and the complement of the
               weighting for the paired pixel */
            Weighting = ErrorAcc >> IntensityShift;
            PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
            PUTDOT(X0 + XDir, Y0,
                   &BaseColor[(Weighting ^ WeightingComplementMask)],
                   &BaseColor[7]);
        }
        /* Draw the final pixel, which is always exactly intersected by the line
           and so needs no weighting */
        PUTDOT(X1, Y1, &BaseColor[0], NULL);
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
       pixel that Y advances each time X advances 1 pixel, truncating the
       result to avoid overrunning the endpoint along the X axis */
    ErrorAdj = ((unsigned int) DeltaY << 16) / (unsigned int) DeltaX;
    /* Draw all pixels other than the first and last */
    while (--DeltaX)
    {
        ErrorAccTemp = ErrorAcc;        /* remember currrent accumulated error */
        ErrorAcc += ErrorAdj;   /* calculate error for next pixel */
        if (ErrorAcc <= ErrorAccTemp)
        {
            /* The error accumulator turned over, so advance the Y coord */
            Y0++;
        }
        X0 += XDir;             /* X-major, so always advance X */
        /* The IntensityBits most significant bits of ErrorAcc give us the
           intensity weighting for this pixel, and the complement of the
           weighting for the paired pixel */
        Weighting = ErrorAcc >> IntensityShift;
        PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
        PUTDOT(X0, Y0 + 1,
               &BaseColor[(Weighting ^ WeightingComplementMask)],
               &BaseColor[7]);

    }
    /* Draw the final pixel, which is always exactly intersected by the line
       and so needs no weighting */
    PUTDOT(X1, Y1, &BaseColor[0], NULL);
}

void AM_drawMline(mline_t * ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
        AM_drawFline(&fl, color);       // draws it on frame buffer using fb coords

}

void AM_drawGrid(int color)
{
    int64_t x, y;
    int64_t start, end;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if (automap_rotate)
    {
        start -= m_h / 2;
    }

    if ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS))
    {
        start += (MAPBLOCKUNITS<<FRACBITS) - 
      ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS));
    }

    end = m_x + m_w;
    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.x = x;
        ml.b.x = x;
        // [crispy] moved here
        ml.a.y = m_y;
        ml.b.y = m_y + m_h;
        if (automap_rotate)
        {
            ml.a.y -= m_w / 2;
            ml.b.y += m_w / 2;
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if (automap_rotate)
    {
        start -= m_w / 2;
    }

    if ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS))
    {
        start += (MAPBLOCKUNITS<<FRACBITS) - 
      ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS));
    }
    
    end = m_y + m_h;
    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y = start; y < end; y += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.y = y;
        ml.b.y = y;
        // [crispy] moved here
        ml.a.x = m_x;
        ml.b.x = m_x + m_w;
        if (automap_rotate)
        {
            ml.a.x -= m_h / 2;
            ml.b.x += m_h / 2;
            AM_rotatePoint(&ml.a);
            AM_rotatePoint(&ml.b);
        }
        AM_drawMline(&ml, color);
    }
}

void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i = 0; i < numlines; i++)
    {
        l.a.x = lines[i].v1->x;
        l.a.y = lines[i].v1->y;
        l.b.x = lines[i].v2->x;
        l.b.y = lines[i].v2->y;
        if (automap_rotate)
        {
            AM_rotatePoint(&l.a);
            AM_rotatePoint(&l.b);
        }
        if (cheating || (lines[i].flags & ML_MAPPED))
        {
            if ((lines[i].flags & LINE_NEVERSEE) && !cheating)
                continue;
            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS + lightlev);
            }
            else
            {
                if (lines[i].special == 39)
                {               // teleporters
                    AM_drawMline(&l, WALLCOLORS + WALLRANGE / 2);
                }
                else if (lines[i].flags & ML_SECRET)    // secret door
                {
                    if (cheating)
                        AM_drawMline(&l, 0);
                    else
                        AM_drawMline(&l, WALLCOLORS + lightlev);
                }
                else if (lines[i].special > 25 && lines[i].special < 35)
                {
                    switch (lines[i].special)
                    {
                        case 26:
                        case 32:
                            AM_drawMline(&l, BLUEKEY);
                            break;
                        case 27:
                        case 34:
                            AM_drawMline(&l, YELLOWKEY);
                            break;
                        case 28:
                        case 33:
                            AM_drawMline(&l, GREENKEY);
                            break;
                        default:
                            break;
                    }
                }
                else if (lines[i].backsector->floorheight
                         != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS + lightlev);  // floor level change
                }
                else if (lines[i].backsector->ceilingheight
                         != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS + lightlev);  // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS + lightlev);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE))
                AM_drawMline(&l, GRAYS + 3);
        }
    }

}

void AM_rotate (int64_t* x, int64_t* y, angle_t a)
{
    int64_t tmpx;

    tmpx = FixedMul(*x, finecosine[a >> ANGLETOFINESHIFT])
        - FixedMul(*y, finesine[a >> ANGLETOFINESHIFT]);
    *y = FixedMul(*x, finesine[a >> ANGLETOFINESHIFT])
        + FixedMul(*y, finecosine[a >> ANGLETOFINESHIFT]);
    *x = tmpx;
}

// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
static void AM_rotatePoint (mpoint_t *pt)
{
    int64_t tmpx;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[mapangle>>ANGLETOFINESHIFT])
         - (int64_t)FixedMul(pt->y, finesine[mapangle>>ANGLETOFINESHIFT])
         + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[mapangle>>ANGLETOFINESHIFT])
          + (int64_t)FixedMul(pt->y, finecosine[mapangle>>ANGLETOFINESHIFT])
          + mapcenter.y;

    pt->x = tmpx;
}

void AM_drawLineCharacter(mline_t * lineguy, int lineguylines, fixed_t scale,
                          angle_t angle, int color, fixed_t x, fixed_t y)
{
    int i;
    mline_t l;

    if (automap_rotate)
    {
        angle += mapangle;
    }

    for (i = 0; i < lineguylines; i++)
    {
        l.a.x = lineguy[i].a.x;
        l.a.y = lineguy[i].a.y;
        if (scale)
        {
            l.a.x = FixedMul(scale, l.a.x);
            l.a.y = FixedMul(scale, l.a.y);
        }
        if (angle)
            AM_rotate(&l.a.x, &l.a.y, angle);
        l.a.x += x;
        l.a.y += y;

        l.b.x = lineguy[i].b.x;
        l.b.y = lineguy[i].b.y;
        if (scale)
        {
            l.b.x = FixedMul(scale, l.b.x);
            l.b.y = FixedMul(scale, l.b.y);
        }
        if (angle)
            AM_rotate(&l.b.x, &l.b.y, angle);
        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, color);
    }

}

void AM_drawPlayers(void)
{

    int i;
    player_t *p;
    static int their_colors[] = { GREENKEY, YELLOWKEY, BLOODRED, BLUEKEY };
    int their_color = -1;
    int color;
    mpoint_t pt;

    if (!netgame)
    {
        /*
           if (cheating) AM_drawLineCharacter(cheat_player_arrow, NUMCHEATPLYRLINES, 0,
           plr->mo->angle, WHITE, plr->mo->x, plr->mo->y);
         *///cheat key player pointer is the same as non-cheat pointer..

        pt.x = plr->mo->x;
        pt.y = plr->mo->y;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, plr->mo->angle,
                             WHITE, pt.x, pt.y);
        return;
    }

    for (i = 0; i < MAXPLAYERS; i++)
    {
        their_color++;
        p = &players[i];

        if (deathmatch && !singledemo && p != plr)
        {
            continue;
        }
        if (!playeringame[i])
        {
            continue;
        }
        if (p->powers[pw_invisibility])
        {
            color = 102;        // *close* to the automap color
        }
        else
        {
            color = their_colors[their_color];
        }

        pt.x = p->mo->x;
        pt.y = p->mo->y;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, p->mo->angle,
                             color, pt.x, pt.y);
    }
}

void AM_drawThings(int colors, int colorrange)
{
    int i;
    mobj_t *t;
    mpoint_t pt;

    for (i = 0; i < numsectors; i++)
    {
        t = sectors[i].thinglist;
        while (t)
        {
            // [crispy] do not draw an extra triangle for the player
            if (t == plr->mo)
            {
                t = t->snext;
                continue;
            }

            pt.x = t->x;
            pt.y = t->y;
            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                 16 << FRACBITS, t->angle, colors + lightlev,
                                 pt.x, pt.y);
            t = t->snext;
        }
    }
}

void AM_drawkeys(void)
{
    mpoint_t pt0;
    mpoint_t pt1;
    mpoint_t pt2;

    // [JN] Support for keys rotation in automap rotate mode.
    pt0.x = KeyPoints[0].x; pt0.y = KeyPoints[0].y;
    pt1.x = KeyPoints[1].x; pt1.y = KeyPoints[1].y;
    pt2.x = KeyPoints[2].x; pt2.y = KeyPoints[2].y;

    if (automap_rotate)
    {
        AM_rotatePoint(&pt0);
        AM_rotatePoint(&pt1);
        AM_rotatePoint(&pt2);
    }

    if (KeyPoints[0].x != 0 || KeyPoints[0].y != 0)
    {
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, YELLOWKEY, 
                             pt0.x, pt0.y);
    }
    if (KeyPoints[1].x != 0 || KeyPoints[1].y != 0)
    {
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, GREENKEY,
                             pt1.x, pt1.y);
    }
    if (KeyPoints[2].x != 0 || KeyPoints[2].y != 0)
    {
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, BLUEKEY,
                             pt2.x, pt2.y);
    }
}

void AM_drawCrosshair(int color)
{
    fb[(f_w * (f_h + 1)) / 2] = color;  // single point for now
}

void AM_Drawer(void)
{
    char *level_name;
    int numepisodes;
    boolean wide_4_3 = (widescreen > 0 && screenblocks == 9);

    if (!automapactive)
    {
        return;
    }

    UpdateState |= I_FULLSCRN;
    if (!automap_overlay)
    {
        AM_clearFB(BACKGROUND);
    }
    else
    {
        BorderNeedRefresh = true;
    }
    if (automap_grid)
    {
        AM_drawGrid(GRIDCOLORS);
    }
    AM_drawWalls();
    AM_drawPlayers();
    if (cheating == 2)
    {
        AM_drawThings(THINGCOLORS, THINGRANGE);
    }

    if (gameskill == sk_baby)
    {
        AM_drawkeys();
    }

    if (gamemode == retail)
    {
        numepisodes = 5;
    }
    else
    {
        numepisodes = 3;
    }

    if (gameepisode <= numepisodes && gamemap < 10)
    {
        level_name = english_language ?
                     LevelNames[(gameepisode - 1) * 9 + gamemap - 1] :
                     LevelNames_Rus[(gameepisode - 1) * 9 + gamemap - 1];

        // [JN] Wide screen: place level name higher in wide screen,
        // do not place it under the status bar gargoyle's horn.
        if (widescreen > 0)
        {
            if (english_language)
            {
                MN_DrTextA(DEH_String(level_name), 20 + 
                          (wide_4_3 ?wide_delta : 0), 136);
            }
            else
            {
                MN_DrTextSmallRUS(DEH_String(level_name), 20 + 
                                 (wide_4_3 ? wide_delta : 0), 136);
            }
        }
        else
        {
            if (english_language)
            {
                MN_DrTextA(DEH_String(level_name), 20, 146);
            }
            else
            {
                MN_DrTextSmallRUS(DEH_String(level_name), 20, 146);
            }
        }
    }

    // [JN] Show level stats in automap
    if (!vanillaparm && automap_stats)
    {
        char text[32];
        int time = leveltime / TICRATE;

        M_snprintf(text, sizeof(text),
                   english_language ?
                   "KILLS: %d/ %d" :
                   "DHFUB: %d/ %d",
                   players[consoleplayer].killcount,
                   totalkills);
        if (english_language)
        {
            MN_DrTextA(text, 20 + (wide_4_3 ? wide_delta : 0), 16);
        }
        else
        {
            MN_DrTextSmallRUS(text, 20 + (wide_4_3 ? wide_delta : 0), 16);
        }

        M_snprintf(text, sizeof(text),
                   english_language ?
                   "ITEMS: %d/ %d" :
                   "GHTLVTNS: %d/ %d",
                   players[consoleplayer].itemcount,
                   totalitems);
        if (english_language)
        {
            MN_DrTextA(text, 20 + (wide_4_3 ? wide_delta : 0), 26);
        }
        else
        {
            MN_DrTextSmallRUS(text, 20 + (wide_4_3 ? wide_delta : 0), 26);
        }

        M_snprintf(text, sizeof(text),
                   english_language ?
                   "SECRETS: %d/ %d" :
                   "NFQYBRB: %d/ %d",
                   players[consoleplayer].secretcount,
                   totalsecret);
        if (english_language)
        {
            MN_DrTextA(text, 20 + (wide_4_3 ? wide_delta : 0), 36);
        }
        else
        {
            MN_DrTextSmallRUS(text, 20 + (wide_4_3 ? wide_delta : 0), 36);
        }

        M_snprintf(text, sizeof(text), 
                   english_language ?
                   "SKILL: %d" :
                   "CKJ;YJCNM: %d", 
                   gameskill +1);
        if (english_language)
        {
            MN_DrTextA(text, 20 + (wide_4_3 ? wide_delta : 0), 46);
        }
        else
        {
            MN_DrTextSmallRUS(text, 20 + (wide_4_3 ? wide_delta : 0), 46);
        }

        M_snprintf(text, sizeof(text),
                   "%02d:%02d:%02d",
                   time/3600,
                   (time%3600)/60,
                   time%60);
        MN_DrTextA(text, 20 + (wide_4_3 ? wide_delta : 0), 63);
    }
}

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
// AM_map.c


#include <stdlib.h>
#include "i_system.h"
#include "hr_local.h"
#include "deh_str.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "v_video.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "jn.h"


// For use if I do walls with outsides/insides
#define REDS        12*8
#define REDRANGE    1
#define BLUES       (256-4*16+8)
#define BLUERANGE   1
#define GREENS      (33*8)
#define GREENRANGE  1
#define GRAYS       (5*8)
#define GRAYSRANGE  1
#define BROWNS      (14*8)
#define BROWNRANGE  1
#define YELLOWS     10*8
#define YELLOWRANGE 1
#define BLACK       0
#define WHITE       4*8
#define BLOODRED    150
#define BLUEKEY     197
#define YELLOWKEY   144
#define GREENKEY    220

// Automap colors
#define YOURCOLORS	      WHITE
#define YOURRANGE	      0
#define WALLCOLORS	      REDS
#define WALLRANGE	      REDRANGE
#define TSWALLCOLORS	  GRAYS
#define TSWALLRANGE	      GRAYSRANGE
#define FDWALLCOLORS	  BROWNS
#define FDWALLRANGE	      BROWNRANGE
#define CDWALLCOLORS	  YELLOWS
#define CDWALLRANGE	      YELLOWRANGE
#define THINGCOLORS	      GREENS
#define THINGRANGE	      GREENRANGE
#define SECRETWALLCOLORS  WALLCOLORS
#define SECRETWALLRANGE   WALLRANGE
#define GRIDCOLORS	      (GRAYS + GRAYSRANGE/2)
#define GRIDRANGE	      0

// [JN] RAVMAP extended colors
#define REDS_RAVMAP       157
#define YELLOWS_RAVMAP    129
#define GREENS_RAVMAP     GREENKEY
#define GRAYS_RAVMAP      GRAYS

//the MACRO!
#define DOT(xx,yy,cc) I_VideoBuffer[(yy)*f_w+(flipscreenwidth[xx])]=(cc)

// [JN] FRACTOMAPBITS: overflow-safe coordinate system.
// Written by Andrey Budko (entryway), adapted from prboom-plus/src/am_map.*
#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// Scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)

// [JN] How much the automap moves window per tic in frame-buffer coordinates.
static int f_paninc;
#define F_PANINC_SLOW 8   // 280 map units in 1 second.
#define F_PANINC_FAST 16  // 560 map units in 1 second.

// [JN] How much zoom-in per tic goes to 2x in 1 second.
static int m_zoomin;
#define M_ZOOMIN_SLOW ((int) ((float)FRACUNIT * (1.04f + f_paninc / 200.0f)))
#define M_ZOOMIN_FAST ((int) ((float)FRACUNIT * (1.08f + f_paninc / 200.0f)))

// [JN] How much zoom-out per tic pulls out to 0.5x in 1 second.
static int m_zoomout;
#define M_ZOOMOUT_SLOW ((int) ((float)FRACUNIT / (1.04f + f_paninc / 200.0f)))
#define M_ZOOMOUT_FAST ((int) ((float)FRACUNIT / (1.08f + f_paninc / 200.0f)))

// Translates between frame-buffer and map distances
// [crispy] fix int overflow that causes map and grid lines to disappear
#define FTOM(x) (((int64_t)((x)<<FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>FRACBITS)

// Translates between frame-buffer and map coordinates
#define CXMTOF(x) (f_x + MTOF((x)-m_x))
#define CYMTOF(y) (f_y + (f_h - MTOF((y)-m_y)))

// [crispy] Used for automap background tiling
#define MAPBGROUNDWIDTH   (ORIGWIDTH)
#define MAPBGROUNDHEIGHT  (ORIGHEIGHT - (42 << quadres))

// [JN] Pointer to background drawing functions.
static void (*AM_drawBackground) (void);
static void AM_drawBackgroundHigh (void);
static void AM_drawBackgroundQuad (void);

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;

// A line drawing of the player pointing right, starting from the middle.
#define R ((8*FRACUNIT)/7)
static const mline_t player_arrow[] =
{
    { { -R+R/4,    0 }, {      0,    0 } },  // center line
    { { -R+R/4,  R/8 }, {      R,    0 } },  // blade
    { { -R+R/4, -R/8 }, {      R,    0 } },
    { { -R+R/4, -R/4 }, { -R+R/4,  R/4 } },  // crosspiece
    { { -R+R/8, -R/4 }, { -R+R/8,  R/4 } },
    { { -R+R/8, -R/4 }, { -R+R/4, -R/4 } },  // crosspiece connectors
    { { -R+R/8,  R/4 }, { -R+R/4,  R/4 } },
    { { -R-R/4,  R/8 }, { -R-R/4, -R/8 } },  // pommel
    { { -R-R/4,  R/8 }, { -R+R/8,  R/8 } },
    { { -R-R/4, -R/8 }, { -R+R/8, -R/8 } }
};

static const mline_t keysquare[] =
{
	{ {      0,    0 }, {    R/4, -R/2 } },
	{ {    R/4, -R/2 }, {    R/2, -R/2 } },
	{ {    R/2, -R/2 }, {    R/2,  R/2 } },
	{ {    R/2,  R/2 }, {    R/4,  R/2 } },
	{ {    R/4,  R/2 }, {      0,    0 } },  // handle part type thing
	{ {      0,    0 }, {     -R,    0 } },  // stem
	{ {     -R,    0 }, {     -R, -R/2 } },  // end lockpick part
	{ { -3*R/4,    0 }, { -3*R/4, -R/4 } }
};
#undef R

#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))
#define NUMKEYSQUARELINES (sizeof(keysquare)/sizeof(mline_t))

#define R (FRACUNIT)
static const mline_t thintriangle_guy[] = {
  { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
  { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
  { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R

#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))


static int cheating = 0;

// [JN] Choosen mark color.
static Translation_CR_t automap_mark_color_set;

static int finit_height;
static int f_x, f_y;  // location of window on screen
static int f_w, f_h;  // size of window on screen

static mpoint_t m_paninc;          // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul;       // how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul;       // how far the window zooms in each tic (fb coords)
static fixed_t curr_mtof_zoommul;  // [JN] Zooming interpolation.

static int64_t m_x, m_y;            // LL x,y where the window is on the map (map coords)
static int64_t m_x2, m_y2;          // UR x,y where the window is on the map (map coords)
static fixed_t prev_m_x, prev_m_y;  // [JN] Panning interpolation.

// Width/height of window on map (map coords)
static int64_t m_w, m_h;
static fixed_t min_x, min_y;    // based on level size
static fixed_t max_x, max_y;    // based on level size
static fixed_t max_w, max_h;    // max_x-min_x, max_y-min_y
static fixed_t min_scale_mtof;  // used to tell when to stop zooming out
static fixed_t max_scale_mtof;  // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
static fixed_t prev_scale_mtof = (fixed_t)INITSCALEMTOF; // [JN] Panning interpolation.
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

// the player represented by an arrow
static player_t *plr;

static byte    *maplump;      // Automap background patch.
static patch_t *am_crosshair; // [JN] Crosshair patch in non-follow mode.
static patch_t *marknums[10]; // numbers used for marking by the automap

// [JN] killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.
mpoint_t *markpoints = NULL;     // where the points are
int       markpointnum = 0;      // next point to be assigned (also number of points now)
int       markpointnum_max = 0;  // killough 2/22/98

static cheatseq_t cheat_amap = CHEAT("ravmap", 0);

boolean automapactive = false;
static boolean stopped = true;
vertex_t KeyPoints[NUMKEYS];

// Number of antialiased lines
#define NUMALIAS 13

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
    {  0,   2,   4,   6,   8,  10,  12,  14},   // BLACK
    {152, 153, 154, 155, 156, 157, 158, 159},   // REDS_RAVMAP
    {127, 127, 128, 128, 129, 129, 130, 130}    // YELLOWS_RAVMAP
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
    {  0,   0,   1,   1,   2,   2,   3,   4},   // BLACK
    {157, 157, 156, 156, 155, 155, 154, 154},   // REDS_RAVMAP
    {129, 129, 128, 128, 127, 127, 126, 126}    // YELLOWS_RAVMAP
};

static void DrawWuLine(int X0, int Y0, int X1, int Y1, byte *BaseColor,
                       int NumLevels, unsigned short IntensityBits);



// [crispy] automap rotate mode ...
// ... needs these early on
static void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

/*
================================================================================
=
= AM_activateNewScale
=
= Changes the map scale after zooming or translating.
=
================================================================================
*/

static void AM_activateNewScale (void)
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

/*
================================================================================
=
= AM_saveScaleAndLoc
=
= Saves the current center and zoom.
= Affects the variables that remember old scale and loc.
=
================================================================================
*/

static void AM_saveScaleAndLoc (void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

/*
================================================================================
=
= AM_restoreScaleAndLoc
=
= Restores the center and zoom from locally saved values.
= Affects global variables for location and scale.
=
================================================================================
*/

static void AM_restoreScaleAndLoc (void)
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
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w << FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

/*
================================================================================
=
= AM_findMinMaxBoundaries
=
= Determines bounding box of all vertices, 
= sets global variables controlling zoom range.
=
================================================================================
*/

static void AM_findMinMaxBoundaries (void)
{
    fixed_t a, b;

    min_x = min_y = INT_MAX;
    max_x = max_y = -INT_MAX;

    for (int i = 0 ; i < numvertexes ; i++)
    {
        if (vertexes[i].x < min_x)
        {
            min_x = vertexes[i].x;
        }
        else if (vertexes[i].x > max_x)
        {
            max_x = vertexes[i].x;
        }

        if (vertexes[i].y < min_y)
        {
            min_y = vertexes[i].y;
        }
        else if (vertexes[i].y > max_y)
        {
            max_y = vertexes[i].y;
        }
    }

    // [crispy] cope with huge level dimensions which span the entire INT range
    max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
    max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * FRACUNIT);
}

/*
================================================================================
=
= AM_changeWindowLoc
=
= Moves the map window by the global variables m_paninc.x, m_paninc.y
=
================================================================================
*/

static void AM_changeWindowLoc (void)
{
    int64_t incx, incy;

    if (m_paninc.x || m_paninc.y)
    {
        automap_follow = 0;
    }

    if (uncapped_fps && leveltime > oldleveltime)
    {
        incx = FixedMul(m_paninc.x, fractionaltic);
        incy = FixedMul(m_paninc.y, fractionaltic);
    }
    else
    {
        incx = m_paninc.x;
        incy = m_paninc.y;
    }

    if (automap_rotate)
    {
        AM_rotate(&incx, &incy, -mapangle);
    }

    m_x = prev_m_x + incx;
    m_y = prev_m_y + incy;

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

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

/*
================================================================================
=
= AM_addMark
=
= Adds a marker at the current location.
=
================================================================================
*/

static void AM_addMark (void)
{
    // [JN] killough 2/22/98: remove limit on automap marks
    if (markpointnum >= markpointnum_max)
    {
        markpoints = I_Realloc(markpoints,
                              (markpointnum_max = markpointnum_max ? 
                               markpointnum_max * 2 : 16) * sizeof(*markpoints));
    }

    // [crispy] keep the map static in overlay mode if not following the player
    if (!automap_follow)
    {
        markpoints[markpointnum].x = m_x + m_w / 2;
        markpoints[markpointnum].y = m_y + m_h / 2;
    }
    else
    {
        markpoints[markpointnum].x = plr->mo->x >> FRACTOMAPBITS;
        markpoints[markpointnum].y = plr->mo->y >> FRACTOMAPBITS;
    }
    markpointnum++;
}

/*
================================================================================
=
= AM_initMarksColor
=
================================================================================
*/

void AM_initMarksColor (const int color)
{
    Translation_CR_t *colorVar = &automap_mark_color_set;

    switch (color)
    {
        case 1:   *colorVar = CR_GRAY;          break;
        case 2:   *colorVar = CR_DARKGRAY;      break;
        case 3:   *colorVar = CR_RED;           break;
        case 4:   *colorVar = CR_DARKRED;       break;
        case 5:   *colorVar = CR_GREEN;         break;
        case 6:   *colorVar = CR_DARKGREEN;     break;
        case 7:   *colorVar = CR_OLIVE;         break;
        case 8:   *colorVar = CR_BLUE2;         break;
        case 9:   *colorVar = CR_DARKBLUE;      break;
        case 10:  *colorVar = CR_PURPLE;        break;
        case 11:  *colorVar = CR_NIAGARA;       break;
        case 12:  *colorVar = CR_AZURE;         break;
        case 13:  *colorVar = CR_BRIGHTYELLOW;  break;
        case 14:  *colorVar = CR_YELLOW;        break;
        case 15:  *colorVar = CR_DARKGOLD;      break;
        case 16:  *colorVar = CR_TAN;           break;
        case 17:  *colorVar = CR_BROWN;         break;
        default:  *colorVar = CR_WHITE;         break;
    }
}

/*
================================================================================
=
= AM_clearMarks
=
================================================================================
*/

void AM_clearMarks (void)
{
    markpointnum = 0;
}

/*
================================================================================
=
= AM_initVariables
=
================================================================================
*/

void AM_initVariables (void)
{
    thinker_t *think;
    mobj_t    *mo;

    automapactive = true;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // [JN] Find player to center.
    plr = &players[displayplayer];

    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;

    AM_Ticker();
    AM_changeWindowLoc();

    // For saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // Load in the location of keys, if in baby mode
    memset(KeyPoints, 0, sizeof(vertex_t) * 3);

    if (gameskill == sk_baby)
    {
        for (think = thinkercap.next; think != &thinkercap; think = think->next)
        {
            if (think->function != P_MobjThinker)
            {
                continue;  // Not a mobj
            }

            mo = (mobj_t *)think;

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

/*
================================================================================
=
= AM_initPics
=
= [JN] Preload automap graphics once at game startup.
=
================================================================================
*/

void AM_initPics (void)
{
    char namebuf[9];

    maplump = W_CacheLumpName(DEH_String("AUTOPAGE"), PU_STATIC);

    // Load crosshair patch.
    am_crosshair = W_CacheLumpName("XHAIR_1", PU_STATIC);

    for (int i = 0 ; i < 10 ; i++)
    {
        // Load custom, precise versions of automap marks.
        DEH_snprintf(namebuf, 9, "MARKNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }

    AM_drawBackground = quadres ? AM_drawBackgroundQuad : AM_drawBackgroundHigh;
}

/*
================================================================================
=
= AM_LevelInit
=
= should be called at the start of every level
= right now, i figure it out myself
=
================================================================================
*/

static void AM_LevelInit (void)
{
    finit_height = SCREENHEIGHT - (42 << hires);
    f_x = f_y = 0;
    f_w = screenwidth;
    f_h = finit_height;

    AM_findMinMaxBoundaries();
    
    scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7 * FRACUNIT));

    if (scale_mtof > max_scale_mtof)
    {
        scale_mtof = min_scale_mtof;
    }

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

/*
================================================================================
=
= AM_Stop
=
================================================================================
*/

void AM_Stop (void)
{
    automapactive = false;
    stopped = true;
    BorderNeedRefresh = true;
}

/*
================================================================================
=
= AM_Start
=
================================================================================
*/

void AM_Start (void)
{
    static int lastlevel = -1, lastepisode = -1;

    if (!stopped)
    {
        AM_Stop();
    }

    stopped = false;

    if (gamestate != GS_LEVEL)
    {
        return;  // Don't show automap if we aren't in a game!
    }

    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }

    AM_initVariables();
}

/*
================================================================================
=
= AM_minOutWindowScale
=
= Set the window scale to the maximum size.
=
================================================================================
*/

static void AM_minOutWindowScale (void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

/*
================================================================================
=
= AM_maxOutWindowScale
=
= Set the window scale to the minimum size.
=
================================================================================
*/

static void AM_maxOutWindowScale (void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

/*
================================================================================
=
= AM_Responder
=
= Handle events (user inputs) in automap mode.
=
================================================================================
*/

const boolean AM_Responder (const event_t *ev)
{
    boolean     rc;
    static int  bigstate = 0;
    static char buffer[32];

    // [JN] If run button is hold, pan/zoom Automap faster.    
    if (BK_isKeyPressed(bk_speed))
    {
        f_paninc = F_PANINC_FAST;
        m_zoomin = M_ZOOMIN_FAST;
        m_zoomout = M_ZOOMOUT_FAST;
    }
    else
    {
        f_paninc = F_PANINC_SLOW;
        m_zoomin = M_ZOOMIN_SLOW;
        m_zoomout = M_ZOOMOUT_SLOW;
    }

    rc = false;

    if (!automapactive)
    {

        if (BK_isKeyDown(ev, bk_map_toggle) && gamestate == GS_LEVEL)
        {
            AM_Start();
            rc = true;
        }
    }
    else
    {
        rc = true;

        if (BK_isKeyDown(ev, bk_right))     // Pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow)
            {
                m_paninc.x = flip_levels ? -FTOM(f_paninc) : FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_left)) // Pan left
        {
            if (!automap_follow)
            {
                m_paninc.x = flip_levels ? FTOM(f_paninc) : -FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_up))   // Pan up
        {
            if (!automap_follow)
            {
                m_paninc.y = FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_down)) // Pan down
        {
            if (!automap_follow)
            {
                m_paninc.y = -FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_out)) // Zoom out
        {
            mtof_zoommul = m_zoomout;
            ftom_zoommul = m_zoomin;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_in))  // Zoom in
        {
            mtof_zoommul = m_zoomin;
            ftom_zoommul = m_zoomout;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_toggle))   // Toggle map (tab)
        {
            bigstate = 0;
            AM_Stop();
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_max)) // Max zoom
        {
            bigstate = !bigstate;

            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else
            {
                AM_restoreScaleAndLoc();
            }
        }
        else if (BK_isKeyDown(ev, bk_map_follow))
        {
            automap_follow = !automap_follow;
            P_SetMessage(plr, automap_follow ?
                              amstr_followon : amstr_followoff, msg_uncolored, false);
        }
        else if (BK_isKeyDown(ev, bk_map_overlay))
        {
            automap_overlay = !automap_overlay;
            P_SetMessage(plr, automap_overlay ?
                              amstr_overlayon : amstr_overlayoff, msg_uncolored, false);
        }
        else if (BK_isKeyDown(ev, bk_map_rotate))
        {
            automap_rotate = !automap_rotate;
            P_SetMessage(plr, automap_rotate ?
                              amstr_rotateon : amstr_rotateoff, msg_uncolored, false);
        }
        else if (BK_isKeyDown(ev, bk_map_grid))
        {
            automap_grid = !automap_grid;
            P_SetMessage(plr, automap_grid ?
                              amstr_gridon : amstr_gridoff, msg_uncolored, false);
        }
        else if (BK_isKeyDown(ev, bk_map_mark))
        {
            // [JN] "Mark № added" / "Отметка № добавлена".
            M_snprintf(buffer, sizeof(buffer), "%s %d %s",
                       amstr_mark, markpointnum, amstr_added);
            P_SetMessage(plr, buffer, msg_uncolored, false);
            AM_addMark();
        }
        else if (BK_isKeyPressed(bk_speed) && BK_isKeyDown(ev, bk_map_clearmark))
        {
            // [JN] Clear all mark by holding "run" button and pressing "clear mark".
            if (markpointnum > 0)
            {
                P_SetMessage(plr, amstr_markscleared, msg_uncolored, false);
                AM_clearMarks();
            }
        }
        else if (BK_isKeyDown(ev, bk_map_clearmark))
        {
            if (markpointnum > 0)
            {
                // [JN] "Mark № cleared" / "Отметка № удалена".
                markpointnum--;
                M_snprintf(buffer, sizeof(buffer), "%s %d %s",
                           amstr_mark, markpointnum, amstr_cleared);
                P_SetMessage(plr, buffer, msg_uncolored, false);
            }
        }
        else
        {
            if (BK_isKeyUp(ev, bk_left))
            {
                if (!automap_follow)
                {
                    m_paninc.x = 0;
                }
            }
            else if (BK_isKeyUp(ev, bk_right))
            {
                if (!automap_follow)
                {
                    m_paninc.x = 0;
                }
            }
            else if (BK_isKeyUp(ev, bk_up))
            {
                if (!automap_follow)
                {
                    m_paninc.y = 0;
                }
            }
            else if (BK_isKeyUp(ev, bk_down))
            {
                if (!automap_follow)
                {
                    m_paninc.y = 0;
                }
            }
            else if (BK_isKeyUp(ev, bk_map_zoom_out) || BK_isKeyUp(ev, bk_map_zoom_in))
            {
                mtof_zoommul = FRACUNIT;
                ftom_zoommul = FRACUNIT;
            }
            rc = false;
        }

        if (ev->type == ev_keydown && cht_CheckCheat(&cheat_amap, ev->data2) && !netgame)
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }

    return rc;
}

/*
================================================================================
=
= AM_changeWindowScale
=
= Automap zooming.
=
================================================================================
*/

static void AM_changeWindowScale (void)
{
    if (uncapped_fps && leveltime > oldleveltime)
    {
        float f_paninc_smooth = (float)f_paninc / (float)FRACUNIT * (float)fractionaltic;

        if (f_paninc_smooth < 0.01f)
        {
            f_paninc_smooth = 0.01f;
        }
    
        scale_mtof = prev_scale_mtof;

        if (curr_mtof_zoommul == m_zoomin)
        {
            mtof_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_paninc_smooth / 200.0f)));
            ftom_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_paninc_smooth / 200.0f)));
        }
        if (curr_mtof_zoommul == m_zoomout)
        {
            mtof_zoommul = ((int) ((float)FRACUNIT / (1.00f + f_paninc_smooth / 200.0f)));
            ftom_zoommul = ((int) ((float)FRACUNIT * (1.00f + f_paninc_smooth / 200.0f)));
        }
    }

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

/*
================================================================================
=
= AM_doFollowPlayer
=
= Turn on follow mode - the map scrolls opposite to player motion.
=
================================================================================
*/

static void AM_doFollowPlayer (void)
{
    // [JN] Use interpolated player coords for smooth
    // scrolling and static player arrow position.
    m_x = (viewx >> FRACTOMAPBITS) - m_w/2;
    m_y = (viewy >> FRACTOMAPBITS) - m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

/*
================================================================================
=
= AM_Ticker
=
= Updates on Game Tick.
=
================================================================================
*/

void AM_Ticker (void)
{
    if (!automapactive)
    {
        return;
    }

    prev_scale_mtof = scale_mtof;
    prev_m_x = m_x;
    prev_m_y = m_y;
}

/*
================================================================================
=
= AM_drawBackgroundHigh
=
= Blit the automap background to the screen.
=
= [crispy] To support widescreen, increase the number of possible background
= tiles from 2 to 3. To support rendering at 2x resolution, treat original
= 320 x 158 tile image as 640 x 79.
=
================================================================================
*/

static void AM_drawBackgroundHigh (void)
{
    const int mapbgwidth_hires = ORIGWIDTH << hires;
    int j = mapbgwidth_hires;
    int x2 = screenwidth;
    int x3;
    
    if (x2 > mapbgwidth_hires)
    {
        x2 = mapbgwidth_hires;
    }

    x3 = screenwidth - x2;

    for (int i = 0 ; i < finit_height ; i++)
    {
        memcpy(I_VideoBuffer + i * screenwidth,
               maplump + j + mapbgwidth_hires - x3, x3);

        memcpy(I_VideoBuffer + i * screenwidth + x3,
               maplump + j + mapbgwidth_hires - x2, x2);

        memcpy(I_VideoBuffer + i * screenwidth + x2 + x3,
               maplump + j, 0);

        j += mapbgwidth_hires;

        if (j >= MAPBGROUNDHEIGHT * MAPBGROUNDWIDTH)
        {
            j = 0;
        }
    }
}

/*
================================================================================
=
= AM_drawBackgroundQuad
=
= [JN] Blit the automap background to the screen, quad resolution version.
=
================================================================================
*/

static void AM_drawBackgroundQuad (void)
{
    int j = ORIGWIDTH;

    for (int i = 0 ; i < finit_height ; i++)
    {
        memcpy(I_VideoBuffer + i * screenwidth, maplump + j + ORIGWIDTH, screenwidth);

        j += ORIGWIDTH;

        if (j >= MAPBGROUNDHEIGHT * MAPBGROUNDWIDTH)
        {
            j = 0;
        }
    }
}

/*
================================================================================
=
= AM_clipMline
=
= Automap clipping of lines.
=
= Based on Cohen-Sutherland clipping algorithm but with a slightly
= faster reject and precalculated slopes.  If the speed is needed,
= use a hash algorithm to handle  the common cases.
=
================================================================================
*/

static boolean AM_clipMline (const mline_t *ml, fline_t *fl)
{
    enum
    {
        LEFT   = 1,
        RIGHT  = 2,
        BOTTOM = 4,
        TOP    = 8
    };

    int outcode1 = 0;
    int outcode2 = 0;
    int outside;

    int      dx;
    int      dy;
    fpoint_t tmp;

#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_w) (oc) |= RIGHT

    // Do trivial rejects and outcodes
    if (ml->a.y > m_y2)
    {
        outcode1 = TOP;
    }
    else if (ml->a.y < m_y)
    {
        outcode1 = BOTTOM;
    }

    if (ml->b.y > m_y2)
    {
        outcode2 = TOP;
    }
    else if (ml->b.y < m_y)
    {
        outcode2 = BOTTOM;
    }
    if (outcode1 & outcode2)
    {
        return false;  // Trivially outside
    }

    if (ml->a.x < m_x)
    {
        outcode1 |= LEFT;
    }
    else if (ml->a.x > m_x2)
    {
        outcode1 |= RIGHT;
    }

    if (ml->b.x < m_x)
    {
        outcode2 |= LEFT;
    }
    else if (ml->b.x > m_x2)
    {
        outcode2 |= RIGHT;
    }

    if (outcode1 & outcode2)
    {
        return false;   // Trivially outside
    }

    // Transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
    {
        return false;
    }

    while (outcode1 | outcode2)
    {
        // May be partially inside box. Find an outside point.
        if (outcode1)
        {
            outside = outcode1;
        }
        else
        {
            outside = outcode2;
        }

        // Clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            // [JN] 'int64_t' math to avoid overflows on long lines.
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y)) / dy);
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h))) / dy);
            tmp.y = f_h - 1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x)) / dx);
            tmp.x = f_w - 1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x)) / dx);
            tmp.x = 0;
        }
        else
        {
            tmp.x = 0;
            tmp.y = 0;
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
        {
            return false;   // Trivially outside
        }
    }

    return true;
}
#undef DOOUTCODE

/*
================================================================================
=
= AM_drawFline
=
= Classic Bresenham w/ whatever optimizations needed for speed.
=
================================================================================
*/

static void AM_drawFline (const fline_t *fl, const int color)
{
    int x, y, dx, dy, sx, sy, ax, ay, d;

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
        // [JN] RAVMAP extended colors
        case REDS_RAVMAP:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[11][0] : &antialias[11][0]), 8, 3);
            break;
        case YELLOWS_RAVMAP:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (automap_overlay ?
                       &antialias_overlay[12][0] : &antialias[12][0]), 8, 3);
            break;
        default:
            {
                // For debugging only
                if (fl->a.x < 0 || fl->a.x >= f_w
                ||  fl->a.y < 0 || fl->a.y >= f_h
                ||  fl->b.x < 0 || fl->b.x >= f_w
                ||  fl->b.y < 0 || fl->b.y >= f_h)
                {
                    return;
                }

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
                        {
                            return;
                        }
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
                        {
                            return;
                        }
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

/*
================================================================================
=
= PUTDOT
=
= Wu antialiased line drawer.
= (X0,Y0),(X1,Y1) = line to draw
= BaseColor = color # of first color in block used for antialiasing, the
=          100% intensity version of the drawing color
= NumLevels = size of color block, with BaseColor+NumLevels-1 being the
=          0% intensity version of the drawing color
= IntensityBits = log base 2 of NumLevels; the # of bits used to describe
=          the intensity of the drawing color. 2**IntensityBits==NumLevels
=
================================================================================
*/

static void PUTDOT (const short xx, const short yy, byte *cc, byte *cm)
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
    else if (cc > oldcc + 6)  // Don't let the color escape from the fade table...
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
    I_VideoBuffer[oldyyshifted + flipscreenwidth[xx]] = *(cc);
}

/*
================================================================================
=
= DrawWuLine
=
================================================================================
*/

static void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                       const int NumLevels, unsigned const short IntensityBits)
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

/*
================================================================================
=
= AM_drawMline
=
= Clip lines, draw visible parts of lines.
=
================================================================================
*/

static void AM_drawMline (const mline_t *ml, const int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // Draws it on frame buffer using fb coords
        AM_drawFline(&fl, color);
    }

}

/*
================================================================================
=
= AM_drawGrid
=
= Draws flat (floor/ceiling tile) aligned grid lines.
=
================================================================================
*/

static void AM_drawGrid (const int color)
{
    int64_t x, y;
    int64_t start, end;
    const fixed_t gridsize = automap_grid_size << MAPBITS;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if (automap_rotate)
    {
        start -= m_h / 2;
    }

    if ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize)
    {
        start -= ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize);
    }

    end = m_x + m_w;
    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // Draw vertical gridlines
    for (x = start ; x < end ; x += gridsize)
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

    if ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize)
    {
        start -= ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize);
    }
    
    end = m_y + m_h;
    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // Draw horizontal gridlines
    for (y = start ; y < end ; y += gridsize)
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

/*
================================================================================
=
= AM_drawWalls
=
= Determines visible lines, draws them. 
= This is LineDef based, not LineSeg based.
=
================================================================================
*/

static void AM_drawWalls (void)
{
    static mline_t l;

    for (int i = 0 ; i < numlines ; i++)
    {
        l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
        l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
        l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
        l.b.y = lines[i].v2->y >> FRACTOMAPBITS;

        if (automap_rotate)
        {
            AM_rotatePoint(&l.a);
            AM_rotatePoint(&l.b);
        }

        if (cheating || (lines[i].flags & ML_MAPPED))
        {
            if ((lines[i].flags & ML_DONTDRAW) && !cheating)
            {
                continue;
            }

            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS);
            }
            else
            {
                // Teleporters
                if (lines[i].special == 39)
                {
                    AM_drawMline(&l, WALLCOLORS + WALLRANGE / 2);
                }
                else  // Secret door
                if (lines[i].flags & ML_SECRET)
                {
                    if (cheating)
                        AM_drawMline(&l, 0);
                    else
                        AM_drawMline(&l, WALLCOLORS);
                }
                else  // Key-locked doors
                if (lines[i].special > 25 && lines[i].special < 35)
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
                else  // Floor level change
                if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS);
                }
                else  // Ceiling level change
                if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS);
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & ML_DONTDRAW))
            {
                AM_drawMline(&l, GRAYS + 3);
            }
        }
    }

}

/*
================================================================================
=
= AM_rotate
=
= Rotation in 2D. Used to rotate player arrow line character.
=
================================================================================
*/

static void AM_rotate (int64_t *x, int64_t *y, angle_t a)
{
    int64_t tmpx;

    a >>= ANGLETOFINESHIFT;

    tmpx = FixedMul(*x, finecosine[a]) - FixedMul(*y, finesine[a]);
    *y = FixedMul(*x, finesine[a]) + FixedMul(*y, finecosine[a]);
    *x = tmpx;
}

/*
================================================================================
=
= AM_rotatePoint
=
= [crispy] rotate point around map center
= adapted from prboom-plus/src/am_map.c:898-920
=
================================================================================
*/

static void AM_rotatePoint (mpoint_t *pt)
{
    int64_t tmpx;
    const angle_t actualangle = ((!(!automap_follow && automap_overlay)) ?
                                 ANG90 - viewangle : mapangle) >> ANGLETOFINESHIFT;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[actualangle])
         - (int64_t)FixedMul(pt->y, finesine[actualangle])
         + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[actualangle])
          + (int64_t)FixedMul(pt->y, finecosine[actualangle])
          + mapcenter.y;

    pt->x = tmpx;
}

/*
================================================================================
=
= AM_drawLineCharacter
=
= Draws a vector graphic according to numerous parameters.
=
================================================================================
*/

static void AM_drawLineCharacter (const mline_t *lineguy, const int lineguylines, const fixed_t scale, 
                                  angle_t angle, const int color, const fixed_t x, const fixed_t y)
{
    mline_t l;

    if (automap_rotate)
    {
        angle += mapangle;
    }

    for (int i = 0 ; i < lineguylines ; i++)
    {
        l.a.x = lineguy[i].a.x;
        l.a.y = lineguy[i].a.y;

        if (scale)
        {
            l.a.x = FixedMul(scale, l.a.x);
            l.a.y = FixedMul(scale, l.a.y);
        }

        if (angle)
        {
            AM_rotate(&l.a.x, &l.a.y, angle);
        }

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
        {
            AM_rotate(&l.b.x, &l.b.y, angle);
        }

        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, color);
    }
}

/*
================================================================================
=
= AM_drawPlayers
=
= Draws the player arrow in single player, 
= or all the player arrows in a netgame.
=
================================================================================
*/

static void AM_drawPlayers (void)
{
    int        i;
    int        color;
    int        their_color = -1;
    const int  their_colors[] = { GREENKEY, YELLOWKEY, BLOODRED, BLUEKEY };
    mpoint_t   pt;
    player_t  *p;

    if (!netgame)
    {
        // [JN] Smooth player arrow rotation.
        const angle_t smoothangle = automap_rotate ? plr->mo->angle : viewangle;

        // [JN] Interpolate player arrow.
        pt.x = viewx >> FRACTOMAPBITS;
        pt.y = viewy >> FRACTOMAPBITS;

        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, smoothangle, 
                             WHITE, pt.x, pt.y);
        return;
    }

    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
        // [JN] Interpolate other player arrows angle.
        angle_t smoothangle;

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
            color = 102;  // *close* to the automap color
        }
        else
        {
            color = their_colors[their_color];
        }

        // [JN] Interpolate other player arrows.
        if (uncapped_fps && leveltime > oldleveltime)
        {
            pt.x = (p->mo->oldx + FixedMul(p->mo->x - p->mo->oldx, fractionaltic)) >> FRACTOMAPBITS;
            pt.y = (p->mo->oldy + FixedMul(p->mo->y - p->mo->oldy, fractionaltic)) >> FRACTOMAPBITS;
        }
        else
        {
            pt.x = p->mo->x >> FRACTOMAPBITS;
            pt.y = p->mo->y >> FRACTOMAPBITS;
        }

        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
            smoothangle = p->mo->angle;
        }
        else
        {
            smoothangle = R_InterpolateAngle(p->mo->oldangle, p->mo->angle, fractionaltic);
        }

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, smoothangle,
                             color, pt.x, pt.y);
    }
}

/*
================================================================================
=
= AM_drawThings
=
= Draws the things on the automap in double RAVMAP cheat mode.
=
================================================================================
*/

static void AM_drawThings (const int colors, const int colorrange)
{
    mobj_t   *t;
    angle_t   actualangle;
    mpoint_t  pt;

    for (int i = 0 ; i < numsectors ; i++)
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

            // [JN] Interpolate things if possible.
            if (uncapped_fps && leveltime > oldleveltime)
            {
                pt.x = (t->oldx + FixedMul(t->x - t->oldx, fractionaltic)) >> FRACTOMAPBITS;
                pt.y = (t->oldy + FixedMul(t->y - t->oldy, fractionaltic)) >> FRACTOMAPBITS;
                actualangle = R_InterpolateAngle(t->oldangle, t->angle, fractionaltic);
            }
            else
            {
                pt.x = t->x >> FRACTOMAPBITS;
                pt.y = t->y >> FRACTOMAPBITS;
                actualangle = t->angle;
            }

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            // [JN] RAVMAP extended colors:
            if (!vanillaparm)
            {
                // [JN] Use actual radius for things drawing.
                const fixed_t actualradius = t->radius >> FRACTOMAPBITS;

                // [crispy] draw blood splats and puffs as small squares
                if (t->type == MT_BLOOD
                ||  t->type == MT_BEAKPUFF
                ||  t->type == MT_STAFFPUFF     || t->type == MT_STAFFPUFF2
                ||  t->type == MT_GAUNTLETPUFF1 || t->type == MT_GAUNTLETPUFF2
                ||  t->type == MT_GOLDWANDPUFF1 || t->type == MT_GOLDWANDPUFF2
                ||  t->type == MT_BLASTERPUFF1  || t->type == MT_BLASTERPUFF2)
                {
                    AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy),
                                         actualradius >> 2, actualangle, GRAYS_RAVMAP, pt.x, pt.y);
                }
                else
                {
                    AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy), 
                                         actualradius, actualangle, 
                                         // Monsters
                                         t->flags & MF_COUNTKILL ? (t->health > 0 ? REDS_RAVMAP : GRAYS_RAVMAP) :
                                         // Gasbags
                                         t->type == MT_POD ? YELLOWS_RAVMAP :
                                         // Pickups
                                         t->flags & MF_SPECIAL ? GREENS_RAVMAP :
                                         // Everything else
                                         GRAYS_RAVMAP,
                                         pt.x, pt.y);
                }
            }
            else
            {
                AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                     FRACUNIT, actualangle, colors, pt.x, pt.y);
            }

            t = t->snext;
        }
    }
}

/*
================================================================================
=
= AM_drawMarks
=
= Draw the marked locations on the automap.
=
================================================================================
*/

#define MARK_W      (5 << hires)
#define MARK_FLIP_1 (1 << hires)
#define MARK_FLIP_2 (9 << hires)

static void AM_drawMarks (void)
{
    int       i;
    mpoint_t  pt;

    // [JN] killough 2/22/98: remove automap mark limit
    for (i = 0 ; i < markpointnum ; i++)
    {
        if (markpoints[i].x != -1)
        {
            int fx, fy;
            int j = i;

            // [crispy] center marks around player
            pt.x = markpoints[i].x;
            pt.y = markpoints[i].y;

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            fx = CXMTOF(pt.x);
            fy = CYMTOF(pt.y);

            do
            {
                int d = j % 10;

                // killough 2/22/98: less spacing for '1'
                if (d == 1)
                {
                    fx += (flip_levels ? -MARK_FLIP_1 : MARK_FLIP_1);
                }

                if (fx >= f_x + 5 && fx <= f_w - 5
                &&  fy >= f_y + 6 && fy <= f_h - 6)
                {
                    // [JN] Use custom, precise patch versions and do coloring.
                    dp_translation = cr[automap_mark_color_set];
                    V_DrawPatchUnscaled((flip_levels ? - fx : fx) >> quadres,
                                         fy >> quadres, marknums[d], NULL);
                    dp_translation = NULL;
                }

                // killough 2/22/98: 1 space backwards
                fx -= MARK_W - (flip_levels ? MARK_FLIP_2 : MARK_FLIP_1);

                j /= 10;
            } while (j > 0);
        }
    }
}

/*
================================================================================
=
= AM_drawkeys
=
================================================================================
*/

static void AM_drawkeys (void)
{
    mpoint_t pt; // [crispy]

    if (KeyPoints[0].x != 0 || KeyPoints[0].y != 0)
    {
        pt.x = KeyPoints[0].x >> FRACTOMAPBITS;
        pt.y = KeyPoints[0].y >> FRACTOMAPBITS;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, YELLOWKEY,
                             pt.x, pt.y);
    }
    if (KeyPoints[1].x != 0 || KeyPoints[1].y != 0)
    {
        pt.x = KeyPoints[1].x >> FRACTOMAPBITS;
        pt.y = KeyPoints[1].y >> FRACTOMAPBITS;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, GREENKEY,
                             pt.x, pt.y);
    }
    if (KeyPoints[2].x != 0 || KeyPoints[2].y != 0)
    {
        pt.x = KeyPoints[2].x >> FRACTOMAPBITS;
        pt.y = KeyPoints[2].y >> FRACTOMAPBITS;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }
        AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, BLUEKEY,
                             pt.x, pt.y);
    }
}

/*
================================================================================
=
= AM_drawCrosshair
=
= [JN] Draw crosshair which is representing the center of the map.
=
================================================================================
*/

static void AM_drawCrosshair (void)
{
    V_DrawPatchUnscaled(origwidth, 162, am_crosshair, NULL);
}

/*
================================================================================
=
= AM_drawCrosshair
=
= Draws the entire automap.
=
================================================================================
*/

void AM_Drawer (void)
{
    if (!automapactive)
    {
        return;
    }

    // [JN] Moved from AM_Ticker for drawing interpolation.
    if (automap_follow)
    {
        AM_doFollowPlayer();
    }

    // Change the zoom if necessary.
    // [JN] Moved from AM_Ticker for zooming interpolation.
    if (ftom_zoommul != FRACUNIT)
    {
        AM_changeWindowScale();
    }

    // Change X and Y location.
    // [JN] Moved from AM_Ticker for paning interpolation.
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
        {
            mapangle = ANG90 - plr->mo->angle;
        }
    }

    UpdateState |= I_FULLSCRN;

    if (!automap_overlay)
    {
        AM_drawBackground();
        skippsprinterp = true;
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

    if (!automap_follow && !vanillaparm)
    {
        AM_drawCrosshair();
    }

    AM_drawMarks();

    if (gameskill == sk_baby)
    {
        AM_drawkeys();
    }
}

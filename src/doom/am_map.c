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
//
// DESCRIPTION:  the automap code
//


#include <stdlib.h>
#include "i_system.h"
#include "st_bar.h"
#include "p_local.h"
#include "m_misc.h"
#include "v_video.h"
#include "doomstat.h"
#include "rd_keybinds.h"
#include "id_lang.h"
#include "am_map.h"
#include "jn.h"

// For use if I do walls with outsides/insides
#define REDS             176
#define REDRANGE         16
#define GREENS           112
#define GREENRANGE       16
#define GRAYS            96
#define BROWNS           64
#define YELLOWS          231
#define BLACK            0
#define WHITE            209

// Automap colors
#define BACKGROUND       BLACK
#define YOURCOLORS       WHITE
#define WALLCOLORS       REDS
#define WALLRANGE        REDRANGE
#define TSWALLCOLORS     GRAYS
#define FDWALLCOLORS     BROWNS
#define CDWALLCOLORS     YELLOWS
#define THINGCOLORS      GREENS
#define THINGRANGE       GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define GRIDCOLORS       104
#define XHAIRCOLORS      GRAYS

// The MACRO!
#define DOT(xx,yy,cc) I_VideoBuffer[(yy)*f_w+(flipscreenwidth[xx])]=(cc)

// [JN] Jaguar Doom automap colors
#define RED_JAGUAR       32
#define GREEN_JAGUAR     120
#define YELLOW_JAGUAR    163
#define MAGENTA_JAGUAR   254

// [JN] IDDT extended colors
#define REDS_IDDT        REDS
#define YELLOWS_IDDT     YELLOWS
#define GREENS_IDDT      GREENS
#define GRAYS_IDDT       GRAYS

// [JN] FRACTOMAPBITS: overflow-safe coordinate system.
// Written by Andrey Budko (entryway), adapted from prboom-plus/src/am_map.*
#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)

// [JN] How much the automap moves window per tic in frame-buffer coordinates.
static int f_paninc;
#define F_PANINC_SLOW 8   // 280 map units in 1 second.
#define F_PANINC_FAST 16  // 560 map units in 1 second.

// [JN] How much zoom-in per tic goes to 2x in 1 second.
static int m_zoomin;
#define M_ZOOMIN_SLOW ((int) (1.04*FRACUNIT))
#define M_ZOOMIN_FAST ((int) (1.08*FRACUNIT))

// [JN] How much zoom-out per tic pulls out to 0.5x in 1 second.
static int m_zoomout;
#define M_ZOOMOUT_SLOW ((int) (FRACUNIT/1.04))
#define M_ZOOMOUT_FAST ((int) (FRACUNIT/1.08))

// translates between frame-buffer and map distances
#define FTOM(x) (((int64_t)((x)<<16) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>16)

// translates between frame-buffer and map coordinates
#define CXMTOF(x) (f_x + MTOF((x)-m_x))
#define CYMTOF(y) (f_y + (f_h - MTOF((y)-m_y)))


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

// -----------------------------------------------------------------------------
// The vector graphics for the automap.
// A line drawing of the player pointing right, starting from the middle.
// -----------------------------------------------------------------------------

#define R ((8*FRACUNIT)/7)
static const mline_t player_arrow[] = {
    { { -R+R/8,   0 }, {  R,      0   } }, // -----
    { {  R,       0 }, {  R-R/2,  R/4 } }, // ----->
    { {  R,       0 }, {  R-R/2, -R/4 } },
    { { -R+R/8,   0 }, { -R-R/8,  R/4 } }, // >---->
    { { -R+R/8,   0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8,  R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};

static const mline_t cheat_player_arrow[] = {
    { { -R+R/8,     0        }, {  R,           0 } }, // -----
    { {  R,         0        }, {  R-R/2,     R/4 } }, // ----->
    { {  R,         0        }, {  R-R/2,    -R/4 } },
    { { -R+R/8,     0        }, { -R-R/8,     R/4 } }, // >----->
    { { -R+R/8,     0        }, { -R-R/8,    -R/4 } },
    { { -R+3*R/8,   0        }, { -R+R/8,     R/4 } }, // >>----->
    { { -R+3*R/8,   0        }, { -R+R/8,    -R/4 } },
    { { -R/2,       0        }, { -R/2,      -R/6 } }, // >>-d--->
    { { -R/2,      -R/6      }, { -R/2+R/6,  -R/6 } },
    { { -R/2+R/6,  -R/6      }, { -R/2+R/6,   R/4 } },
    { { -R/6,       0        }, { -R/6,      -R/6 } }, // >>-dd-->
    { { -R/6,      -R/6      }, {  0,        -R/6 } },
    { {  0,        -R/6      }, {  0,         R/4 } },
    { {  R/6,       R/4      }, {  R/6,      -R/7 } }, // >>-ddt->
    { {  R/6,      -R/7      }, {  R/6+R/32, -R/7-R/32 } },
    { {  R/6+R/32, -R/7-R/32 }, {  R/6+R/10, -R/7 } }
};
#undef R

#define R (FRACUNIT)
static const mline_t thintriangle_guy[] = {
    { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
    { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
    { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R


static int cheating = 0;

// [JN] Choosen color scheme, used in AM_initColors() and AM_drawFline().
static int automap_color_set;

// [JN] Choosen mark color.
static Translation_CR_t automap_mark_color_set;

// location of window on screen
static int f_x;
static int f_y;

// size of window on screen
static int f_w;
static int f_h;

static mpoint_t m_paninc;     // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)
static fixed_t  curr_mtof_zoommul; // [JN] Zooming interpolation.

static int64_t m_x, m_y;   // LL x,y where the window is on the map (map coords)
static int64_t m_x2, m_y2; // UR x,y where the window is on the map (map coords)
static fixed_t prev_m_x, prev_m_y; // [JN] Panning interpolation.

// width/height of window on map (map coords)
static int64_t m_w;
static int64_t m_h;

// based on level size
static fixed_t min_x;
static fixed_t min_y; 
static fixed_t max_x;
static fixed_t max_y;
static fixed_t max_w;
static fixed_t max_h;

static fixed_t min_scale_mtof; // used to tell when to stop zooming out
static fixed_t max_scale_mtof; // used to tell when to stop zooming in

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

static patch_t *am_crosshair; // [JN] Crosshair patch in non-follow mode.
static patch_t *marknums[10]; // numbers used for marking by the automap

// [JN] killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.
mpoint_t *markpoints = NULL;     // where the points are
int       markpointnum = 0;      // next point to be assigned (also number of points now)
int       markpointnum_max = 0;  // killough 2/22/98

cheatseq_t cheat_amap = CHEAT("iddt", 0);
cheatseq_t cheat_amap_beta = CHEAT("eek", 0);  // [JN] Press Beta cheat code

boolean automapactive = false;
static boolean stopped = true;

// [crispy] Antialiased lines from Heretic with more colors
#define NUMSHADES 8
#define NUMSHADES_BITS 3 // log2(NUMSHADES)
static void DrawWuLine (const fline_t *fl, const int color);
static byte color_shades[NUMSHADES * 256];


// [crispy] automap rotate mode needs these early on
static void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

// -----------------------------------------------------------------------------
// AM_activateNewScale
// Changes the map scale after zooming or translating.
// -----------------------------------------------------------------------------

static void AM_activateNewScale (void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w  = FTOM(f_w);
    m_h  = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_saveScaleAndLoc
// Saves the current center and zoom.
// Affects the variables that remember old scale and loc.
// -----------------------------------------------------------------------------

static void AM_saveScaleAndLoc (void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

// -----------------------------------------------------------------------------
// AM_restoreScaleAndLoc
// Restores the center and zoom from locally saved values.
// Affects global variables for location and scale.
// -----------------------------------------------------------------------------

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
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

// -----------------------------------------------------------------------------
// AM_addMark
// Adds a marker at the current location.
// -----------------------------------------------------------------------------

static void AM_addMark (void)
{
    // [JN] killough 2/22/98: remove limit on automap marks
    if (markpointnum >= markpointnum_max)
    {
        markpoints = I_Realloc(markpoints,
                              (markpointnum_max = markpointnum_max ? 
                               markpointnum_max*2 : 16) * sizeof(*markpoints));
    }

    // [crispy] keep the map static in overlay mode if not following the player
    if (!automap_follow)
    {
        markpoints[markpointnum].x = m_x + m_w/2;
        markpoints[markpointnum].y = m_y + m_h/2;
    }
    else
    {
        markpoints[markpointnum].x = plr->mo->x >> FRACTOMAPBITS;
        markpoints[markpointnum].y = plr->mo->y >> FRACTOMAPBITS;
    }
    markpointnum++;
}

// -----------------------------------------------------------------------------
// AM_findMinMaxBoundaries
// Determines bounding box of all vertices, 
// sets global variables controlling zoom range.
// -----------------------------------------------------------------------------

static void AM_findMinMaxBoundaries (void)
{
    int     i;
    fixed_t a, b;

    min_x = min_y =  INT_MAX;
    max_x = max_y = -INT_MAX;

    for (i = 0 ; i < numvertexes ; i++)
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

    a = FixedDiv(f_w<<FRACBITS, max_w);
    b = FixedDiv(f_h<<FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*FRACUNIT);
}

// -----------------------------------------------------------------------------
// AM_changeWindowLoc
// Moves the map window by the global variables m_paninc.x, m_paninc.y
// -----------------------------------------------------------------------------

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

    if (m_x + m_w/2 > max_x)
    {
        m_x = max_x - m_w/2;
    }
    else if (m_x + m_w/2 < min_x)
    {
        m_x = min_x - m_w/2;
    }

    if (m_y + m_h/2 > max_y)
    {
        m_y = max_y - m_h/2;
    }
    else if (m_y + m_h/2 < min_y)
    {
        m_y = min_y - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_initColors
// -----------------------------------------------------------------------------

void AM_initColors (void)
{
    // [JN] Define choosen color scheme.
    // It's always same for Jaguar Doom and vanilla mode.
    automap_color_set = gamemission == jaguar ? 2 :
                                  vanillaparm ? 0 : 
                                 automap_color;
}

// -----------------------------------------------------------------------------
// AM_initShades
// [crispy] Precalculate color lookup tables for antialised line drawing using COLORMAP
// -----------------------------------------------------------------------------

void AM_initShades (void)
{
    for (int color = 0 ; color < 256 ; ++color)
    {
#define REINDEX(I) (color + I * 256)
        // Pick a range of shades for a steep gradient to keep lines thin
        int shade_index[NUMSHADES] =
        {
            REINDEX(0), REINDEX(1), REINDEX(2), REINDEX(3), REINDEX(7), REINDEX(15), REINDEX(23), REINDEX(31),
        };
#undef REINDEX
        for (int shade = 0 ; shade < NUMSHADES ; ++shade)
        {
            color_shades[color * NUMSHADES + shade] = colormaps[shade_index[shade]];
        }
    }
}

// -----------------------------------------------------------------------------
// AM_initMarksColor
// -----------------------------------------------------------------------------

void AM_initMarksColor (const int color)
{
    Translation_CR_t *colorVar = &automap_mark_color_set;
    
    // [JN] Strictly original gray color in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        *colorVar = CR_GRAY;
    }
    else
    {
        switch (color)
        {
            case 1:   *colorVar = CR_DARKRED;    break;
            case 2:   *colorVar = CR_GREEN;      break;
            case 3:   *colorVar = CR_DARKGREEN;  break;
            case 4:   *colorVar = CR_OLIVE;      break;
            case 5:   *colorVar = CR_BLUE2;      break;
            case 6:   *colorVar = CR_DARKBLUE;   break;
            case 7:   *colorVar = CR_YELLOW;     break;
            case 8:   *colorVar = CR_ORANGE;     break;
            case 9:   *colorVar = CR_WHITE;      break;
            case 10:  *colorVar = CR_GRAY;       break;
            case 11:  *colorVar = CR_DARKGRAY;   break;
            case 12:  *colorVar = CR_TAN;        break;
            case 13:  *colorVar = CR_BROWN;      break;
            case 14:  *colorVar = CR_ALMOND;     break;
            case 15:  *colorVar = CR_KHAKI;      break;
            case 16:  *colorVar = CR_PINK;       break;
            case 17:  *colorVar = CR_BURGUNDY;   break;
            default:  *colorVar = CR_RED;        break;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_initVariables
// -----------------------------------------------------------------------------

void AM_initVariables (void)
{
    automapactive = true;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // [JN] Find player to center.
    plr = &players[displayplayer];

    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;

    AM_Ticker();
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

// -----------------------------------------------------------------------------
// AM_initPics
// [JN] Preload automap graphics once at game startup.
// -----------------------------------------------------------------------------

void AM_initPics (void)
{
    int i;
    char namebuf[9];

    // Load crosshair patch.
    am_crosshair = W_CacheLumpName("XHAIR_1", PU_STATIC);

    // Load custom, precise versions of automap marks.
    for (i = 0 ; i < 10 ; i++)
    {
        
        DEH_snprintf(namebuf, 9, "MARKNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }
}

// -----------------------------------------------------------------------------
// AM_clearMarks
// -----------------------------------------------------------------------------

void AM_clearMarks (void)
{
    markpointnum = 0;
}

// -----------------------------------------------------------------------------
// AM_LevelInit
// Should be called at the start of every level.
// Right now, i figure it out myself.
// -----------------------------------------------------------------------------

static void AM_LevelInit (void)
{
    f_x = f_y = 0;
    f_w = screenwidth;
    f_h = SCREENHEIGHT - (st_height << hires);

    AM_findMinMaxBoundaries();

    scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7*FRACUNIT));

    if (scale_mtof > max_scale_mtof)
    {
        scale_mtof = min_scale_mtof;
    }

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    // [JN] If running Deathmatch mode, mark all automap lines as mapped
    // so they will appear initially. DM mode is not about map reveal.
    if (deathmatch)
    {
        for (int i = 0 ; i < numlines ; i++)
        {
            lines[i].flags |= ML_MAPPED;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_Stop
// -----------------------------------------------------------------------------

void AM_Stop (void)
{
    automapactive = false;
    stopped = true;
}

// -----------------------------------------------------------------------------
// AM_Start
// -----------------------------------------------------------------------------

void AM_Start (void)
{
    static int lastlevel = -1, lastepisode = -1;

    if (!stopped)
    {
        AM_Stop();
    }

    stopped = false;

    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }

    AM_initVariables();
}

// -----------------------------------------------------------------------------
// AM_minOutWindowScale
// Set the window scale to the maximum size.
// -----------------------------------------------------------------------------

static void AM_minOutWindowScale (void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// -----------------------------------------------------------------------------
// AM_maxOutWindowScale
// Set the window scale to the minimum size.
// -----------------------------------------------------------------------------

static void AM_maxOutWindowScale (void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// -----------------------------------------------------------------------------
// AM_Responder
// Handle events (user inputs) in automap mode.
// -----------------------------------------------------------------------------

const boolean AM_Responder (event_t *ev)
{
    boolean rc;
    static int bigstate=0;
    static char buffer[32];
    const boolean speed_toggler = BK_isKeyPressed(bk_speed);

    // [JN] If run button is hold, pan/zoom Automap faster.    
    if (speed_toggler)
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
        if (BK_isKeyDown(ev, bk_map_toggle))
        {
            AM_Start ();
            rc = true;
        }
    }
    else
    {
        rc = true;

        if (BK_isKeyDown(ev, bk_right))        // pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow)
            {
                m_paninc.x = flip_levels ? -FTOM(f_paninc): FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_left))   // pan left
        {
            if (!automap_follow)
            {
                m_paninc.x = flip_levels ? FTOM(f_paninc): -FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_up))  // pan up
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
        else if (BK_isKeyDown(ev, bk_down))  // pan down
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
        else if (BK_isKeyDown(ev, bk_map_zoom_out))  // zoom out
        {
            mtof_zoommul = m_zoomout;
            ftom_zoommul = m_zoomin;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_in))   // zoom in
        {
            mtof_zoommul = m_zoomin;
            ftom_zoommul = m_zoomout;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_toggle))
        {
            bigstate = 0;
            AM_Stop ();
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_max))
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else AM_restoreScaleAndLoc();
        }
        else if (BK_isKeyDown(ev, bk_map_follow))
        {
            automap_follow = !automap_follow;
            P_SetMessage(plr, DEH_String(automap_follow ?
                         amstr_followon : amstr_followoff), msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_grid))
        {
            automap_grid = !automap_grid;
            P_SetMessage(plr, DEH_String(automap_grid ?
                         amstr_gridon : amstr_gridoff), msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_mark))
        {
            // [JN] "Mark № added" / "Отметка № добавлена".
            M_snprintf(buffer, sizeof(buffer), "%s %d %s",
                       DEH_String(amstr_mark), markpointnum, DEH_String(amstr_added));
            P_SetMessage(plr, buffer, msg_system, false);
            AM_addMark();
        }
        else if (BK_isKeyPressed(bk_speed) && BK_isKeyDown(ev, bk_map_clearmark))
        {
            // [JN] Clear all mark by holding "run" button and pressing "clear mark".
            if (markpointnum > 0)
            {
                P_SetMessage(plr, DEH_String(amstr_markscleared), msg_system, false);
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
                        DEH_String(amstr_mark), markpointnum, DEH_String(amstr_cleared));
                P_SetMessage(plr, buffer, msg_system, false);
                
            }
        }
        else if (BK_isKeyDown(ev, bk_map_overlay))
        {
            automap_overlay = !automap_overlay;
            P_SetMessage(plr, DEH_String(automap_overlay ?
                         amstr_overlayon : amstr_overlayoff), msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_rotate))
        {
            automap_rotate = !automap_rotate;
            P_SetMessage(plr, DEH_String(automap_rotate ?
                         amstr_rotateon : amstr_rotateoff), msg_system, false);
        }

        else
        {
            if (BK_isKeyUp(ev, bk_left))
            {
                if (!automap_follow) m_paninc.x = 0;
            }
            else if (BK_isKeyUp(ev, bk_right))
            {
                if (!automap_follow) m_paninc.x = 0;
            }
            else if (BK_isKeyUp(ev, bk_up))
            {
                if (!automap_follow) m_paninc.y = 0;
            }
            else if (BK_isKeyUp(ev, bk_down))
            {
                if (!automap_follow) m_paninc.y = 0;
            }
            else if (BK_isKeyUp(ev, bk_map_zoom_out) || BK_isKeyUp(ev, bk_map_zoom_in))
            {
                mtof_zoommul = FRACUNIT;
                ftom_zoommul = FRACUNIT;
            }
            rc = false;
        }

        // [JN] Press Beta: 'EEK' instead of 'IDDT'
        if ((!deathmatch || gameversion <= exe_doom_1_8) && ev->type == ev_keydown
        && cht_CheckCheat(gamemode == pressbeta ? &cheat_amap_beta : &cheat_amap, ev->data2))
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }
    return rc;
}

// -----------------------------------------------------------------------------
// AM_changeWindowScale
// Automap zooming.
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// AM_doFollowPlayer
// Turn on follow mode - the map scrolls opposite to player motion.
// -----------------------------------------------------------------------------

static void AM_doFollowPlayer (void)
{
    // [JN] Use interpolated player coords for smooth
    // scrolling and static player arrow position.
    if (!vanillaparm)
    {
        m_x = (viewx >> FRACTOMAPBITS) - m_w/2;
        m_y = (viewy >> FRACTOMAPBITS) - m_h/2;
    }
    else
    {
        m_x = FTOM(MTOF(plr->mo->x >> FRACTOMAPBITS)) - m_w/2;
        m_y = FTOM(MTOF(plr->mo->y >> FRACTOMAPBITS)) - m_h/2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

// -----------------------------------------------------------------------------
// AM_Ticker
// Updates on Game Tick.
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// AM_clearFB
// Clear automap frame buffer.
// -----------------------------------------------------------------------------

static void AM_clearFB (const int color)
{
    memset(I_VideoBuffer, color, f_w*f_h*sizeof(*I_VideoBuffer));
}

// -----------------------------------------------------------------------------
// AM_clipMline
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
// -----------------------------------------------------------------------------

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
    else if ((mx) >= f_w) (oc) |= RIGHT;

    // do trivial rejects and outcodes
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
        return false; // trivially outside
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
        return false; // trivially outside
    }

    // transform to frame-buffer coordinates.
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
        // may be partially inside box
        // find an outside point
        if (outcode1)
        {
            outside = outcode1;
        }
        else
        {
            outside = outcode2;
        }

        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            // [JN] 'int64_t' math to avoid overflows on long lines.
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y))/dy);
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h)))/dy);
            tmp.y = f_h-1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x))/dx);
            tmp.x = f_w-1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x))/dx);
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
            return false; // trivially outside
        }
    }

    return true;
}
#undef DOOUTCODE

// -----------------------------------------------------------------------------
// AM_drawFline
// Classic Bresenham w/ whatever optimizations needed for speed.
// -----------------------------------------------------------------------------

static void AM_drawFline (const fline_t *fl, const int color, const int automap_color_set)
{
    int x, y, dx, dy, sx, sy, ax, ay, d;

    // [JN] Apply line antialiasing
    if (automap_antialias && !vanillaparm)
    {
        switch (automap_color_set)
        {
            //
            // BOOM colors
            //
            case 1:
            {
                if (color == 23)    // Wall color
                    DrawWuLine(fl, 23);
                if (color == 119)   // Various teleporters
                    DrawWuLine(fl, 119);
                if (color == 204)   // BLUE locked doors
                    DrawWuLine(fl, 204);
                if (color == 175)   // RED locked doors
                    DrawWuLine(fl, 175);
                if (color == 231)   // YELLOW locked doors (using CDWALLCOLORS)
                    DrawWuLine(fl, 231);
                if (color == 208)   // non-secret closed door
                    DrawWuLine(fl, 208);
                if (color == 55)    // floor level change
                    DrawWuLine(fl, 55);
                if (color == 215)   // ceiling level change
                    DrawWuLine(fl, 215);
                if (color == 88)    // 2S lines that appear only in IDDT
                    DrawWuLine(fl, 88);
                break;
            }
            //
            // Jaguar Doom colors
            //
            case 2:
            {
                if (color == RED_JAGUAR)
                    DrawWuLine(fl, RED_JAGUAR);
                if (color == GREEN_JAGUAR)
                    DrawWuLine(fl, GREEN_JAGUAR);
                if (color == MAGENTA_JAGUAR)
                    DrawWuLine(fl, MAGENTA_JAGUAR);
                if (color == YELLOW_JAGUAR)
                    DrawWuLine(fl, YELLOW_JAGUAR);
                if (color == TSWALLCOLORS)  // Hidden gray walls
                    DrawWuLine(fl, TSWALLCOLORS);
                if (color == GRAYS+3)       // computermap visible lines
                    DrawWuLine(fl, GRAYS+3);
                break;
            }
            //
            // Raven colors
            //
            case 3:
            {
                if (color == 151)   // One-sided wall
                    DrawWuLine(fl, 151);
                if (color == 116)   // Various teleporters
                    DrawWuLine(fl, 116);
                if (color == 108)   // Secret door
                    DrawWuLine(fl, 108);
                if (color == 199)   // BLUE locked doors
                    DrawWuLine(fl, 199);
                if (color == 178)   // RED locked doors
                    DrawWuLine(fl, 178);
                if (color == 161)   // YELLOW locked doors
                    DrawWuLine(fl, 161);
                if (color == 239)   // floor level change
                    DrawWuLine(fl, 239);
                if (color == 133)   // ceiling level change
                    DrawWuLine(fl, 133);
                if (color == 99)    // ceiling level change, cheating (using GRAYS+3 (ML_DONTDRAW))
                    DrawWuLine(fl, 99);
                break;
            }
            //
            // Strife colors
            //
            case 4:
            {
                if (color == 119)   // Exit lines (using "Various teleporters")
                    DrawWuLine(fl, 119);
                if (color == 86)    // One-sided wall
                    DrawWuLine(fl, 86);
                if (color == 135)   // Various teleporters and secret door
                    DrawWuLine(fl, 135);
                if (color == 203)   // floor level change
                    DrawWuLine(fl, 203);
                if (color == 195)   // ceiling level change
                    DrawWuLine(fl, 195);
                if (color == 98)    // One-sided wall (cheating)
                    DrawWuLine(fl, 98);
                if (color == 102)   // One-sided wall (ML_DONTDRAW)
                    DrawWuLine(fl, 102);
                break;
            }
            //
            // Unity colors
            //
            case 5:
            {
                if (color == 184)   // [JN] One sided wall / secret door
                    DrawWuLine(fl, 184);
                if (color == 81)    // [JN] Various doors
                    DrawWuLine(fl, 81);
                if (color == 120)   // [JN] Various teleporters
                    DrawWuLine(fl, 120);
                if (color == 200)   // [JN] BLUE locked doors
                    DrawWuLine(fl, 200);
                if (color == 176)   // [JN] RED locked doors
                    DrawWuLine(fl, 176);
                if (color == 160)   // [JN] YELLOW locked doors
                    DrawWuLine(fl, 161);
                if (color == 72)    // [JN] Floor level change
                    DrawWuLine(fl, 72);
                if (color == 64)    // [JN] Ceiling level change
                    DrawWuLine(fl, 64);
                if (color == 96)    // [JN] IDDT visible lines
                    DrawWuLine(fl, 96);
                if (color == 252)   // [JN] Exit
                    DrawWuLine(fl, 252);
                break;
            }
            //
            // DOOM colors
            //
            default:
            {
                if (color == WALLCOLORS)
                    DrawWuLine(fl, WALLCOLORS);
                if (color == FDWALLCOLORS)
                    DrawWuLine(fl, FDWALLCOLORS);
                if (color == TSWALLCOLORS)
                    DrawWuLine(fl, TSWALLCOLORS);
                if (color == CDWALLCOLORS)
                    DrawWuLine(fl, CDWALLCOLORS);
                if (color == SECRETWALLCOLORS)
                    DrawWuLine(fl, SECRETWALLCOLORS);
                if (color == GRAYS+3)
                    DrawWuLine(fl, GRAYS+3);
                break;
            }
        }
        //
        // Common colors:
        //
        if (color == YOURCOLORS)    // White player arrow
            DrawWuLine(fl, YOURCOLORS);
        if (color == THINGCOLORS)   // Green triangles (IDDT cheat)
            DrawWuLine(fl, THINGCOLORS);
        if (color == 104)           // computermap visible lines
            DrawWuLine(fl, 104);

        //
        // IDDT extended colors:
        //
        if (color == REDS_IDDT)     // Alive monster
            DrawWuLine(fl, REDS_IDDT);
        if (color == GRAYS_IDDT)    // Dead monster or decorations
            DrawWuLine(fl, GRAYS_IDDT);
        if (color == YELLOWS_IDDT)  // Lost Soul or Explosive Barrel
            DrawWuLine(fl, YELLOWS_IDDT);
        if (color == GREENS_IDDT)   // Pickups
            DrawWuLine(fl, GREENS_IDDT);
    }
    else
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
        ax = 2 * (dx<0 ? -dx : dx);
        sx = dx<0 ? -1 : 1;

        dy = fl->b.y - fl->a.y;
        ay = 2 * (dy<0 ? -dy : dy);
        sy = dy<0 ? -1 : 1;

        x = fl->a.x;
        y = fl->a.y;

        if (ax > ay)
        {
            d = ay - ax/2;
            while (1)
            {
                DOT(x,y,color);

                if (x == fl->b.x)
                {
                    return;
                }
                if (d>=0)
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
            d = ax - ay/2;
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

// -----------------------------------------------------------------------------
// PUTDOT
// [JN] Line antialiasing. Adapted from Heretic source code. (src/am_map.c)
// Wu antialiased line drawer.
// (X0,Y0),(X1,Y1) = line to draw
// BaseColor = color # of first color in block used for antialiasing, the
//          100% intensity version of the drawing color
// NumLevels = size of color block, with BaseColor+NumLevels-1 being the
//          0% intensity version of the drawing color
// IntensityBits = log base 2 of NumLevels; the # of bits used to describe
//          the intensity of the drawing color. 2**IntensityBits==NumLevels
// -----------------------------------------------------------------------------

static void PUTDOT (const short xx, const short yy, const byte *cc, const byte *cm)
{
    static int oldyy;
    static int oldyyshifted;
    const byte *oldcc = cc;

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
    I_VideoBuffer[oldyyshifted + flipscreenwidth[xx]] = *(cc);
}

// -----------------------------------------------------------------------------
// DrawWuLine
// -----------------------------------------------------------------------------

static void DrawWuLine (const fline_t *fl, const int color)
{
    int X0 = fl->a.x, Y0 = fl->a.y, X1 = fl->b.x, Y1 = fl->b.y;
    const byte *BaseColor = &color_shades[color * NUMSHADES];
    
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
    IntensityShift = 16 - NUMSHADES_BITS;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NUMSHADES - 1;
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

// -----------------------------------------------------------------------------
// AM_drawMline
// Clip lines, draw visible parts of lines.
// -----------------------------------------------------------------------------

static void AM_drawMline (const mline_t *ml, const int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // draws it on frame buffer using fb coords
        AM_drawFline(&fl, color, automap_color_set);
    }
}

// -----------------------------------------------------------------------------
// AM_drawGrid
// Draws flat (floor/ceiling tile) aligned grid lines.
// -----------------------------------------------------------------------------

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

    // draw vertical gridlines
    for (x = start ; x < end ; x += gridsize)
    {
        ml.a.x = x;
        ml.b.x = x;
        // [crispy] moved here
        ml.a.y = m_y;
        ml.b.y = m_y+m_h;
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

    // draw horizontal gridlines
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

// -----------------------------------------------------------------------------
// AM_drawWalls
// Determines visible lines, draws them. 
// This is LineDef based, not LineSeg based.
// -----------------------------------------------------------------------------

static void AM_drawWalls (const int automap_color_set)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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

        switch (automap_color_set)
        {
            //
            // BOOM colors
            //
            case 1:
            {
                if (cheating || (lines[i].flags & ML_MAPPED))
                {
                    if (!lines[i].backsector)
                    {
                        AM_drawMline(&l, 23);
                    }
                    else
                    {
                        // Various teleporters
                        if (lines[i].special == 39  || lines[i].special == 97
                        ||  lines[i].special == 125 || lines[i].special == 126)
                        {
                            AM_drawMline(&l, 119);
                        }
                        // Secret door
                        else if (lines[i].flags & ML_SECRET)
                        {
                            AM_drawMline(&l, 23);      // wall color
                        }
                        // BLUE locked doors
                        else
                        if (lines[i].special == 26 || lines[i].special == 32
                        ||  lines[i].special == 99 || lines[i].special == 133)
                        {
                            AM_drawMline(&l, 204);
                        }
                        // RED locked doors
                        else
                        if (lines[i].special == 28  || lines[i].special == 33
                        ||  lines[i].special == 134 || lines[i].special == 135)
                        {
                            AM_drawMline(&l, 175);
                        }
                        // YELLOW locked doors
                        else
                        if (lines[i].special == 27  || lines[i].special == 34
                        ||  lines[i].special == 136 || lines[i].special == 137)
                        {
                            AM_drawMline(&l, 231);
                        }
                        // non-secret closed door
                        else
                        if (!(lines[i].flags & ML_SECRET) &&
                        ((lines[i].backsector->floorheight == lines[i].backsector->ceilingheight) ||
                        (lines[i].frontsector->floorheight == lines[i].frontsector->ceilingheight)))
                        {
                            AM_drawMline(&l, 208);      // non-secret closed door
                        } //jff 1/6/98 show secret sector 2S lines
                        // floor level change
                        else
                        if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                        {
                            AM_drawMline(&l, 55);
                        }
                        // ceiling level change
                        else
                        if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                        {
                            AM_drawMline(&l, 215);
                        }
                        //2S lines that appear only in IDDT
                        else if (cheating)
                        {
                            AM_drawMline(&l, 88);
                        }
                    }
                }
                // computermap visible lines
                else if (plr->powers[pw_allmap])
                {
                    // invisible flag lines do not show
                    if (!(lines[i].flags & ML_DONTDRAW))
                    {
                        if (!lines[i].backsector 
                        || lines[i].backsector->floorheight != lines[i].frontsector->floorheight
                        || lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                        {
                            AM_drawMline(&l, 104);
                        }
                    }
                }
                break;
            }
            //
            // Jaguar Doom colors
            //
            case 2:
            {
                if (cheating || (lines[i].flags & ML_MAPPED))
                {
                    if (!lines[i].backsector)
                    {
                        AM_drawMline(&l, RED_JAGUAR);
                    }
                    else
                    {
                        // Teleport line
                        if (lines[i].special == 39 || lines[i].special == 97)
                        {
                            AM_drawMline(&l, GREEN_JAGUAR);
                        }
                        // Secret door
                        else if (lines[i].flags & ML_SECRET)
                        {
                            AM_drawMline(&l, RED_JAGUAR);
                        }
                        // Any special linedef
                        else if (lines[i].special)
                        {
                            AM_drawMline(&l, MAGENTA_JAGUAR);
                        }
                        // Floor level change
                        else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                        {
                            AM_drawMline(&l, YELLOW_JAGUAR);
                        }
                        // Ceiling level change
                        else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                        {
                            AM_drawMline(&l, YELLOW_JAGUAR);
                        }
                        // Hidden gray walls
                        else if (cheating)
                        {
                            AM_drawMline(&l, TSWALLCOLORS);
                        }
                    }
                }
                else if (plr->powers[pw_allmap])
                {
                    if (!(lines[i].flags & ML_DONTDRAW)) AM_drawMline(&l, GRAYS+3);
                }
                break;
            }
            //
            // Raven colors
            //
            case 3:
            {
                if (cheating || (lines[i].flags & ML_MAPPED))
                {
                    if (!lines[i].backsector)
                    {
                        AM_drawMline(&l, 151);
                    }
                    else
                    {
                        // [JN] Various teleporters
                        if (lines[i].special == 39  || lines[i].special == 97
                        ||  lines[i].special == 125 || lines[i].special == 126)
                        {
                            AM_drawMline(&l, 116);
                        }
                        // [JN] Secret door
                        else
                        if (lines[i].flags & ML_SECRET)
                        {
                                AM_drawMline(&l, cheating ? 0 : 108);
                        }
                        // [JN] BLUE locked doors
                        else
                        if (lines[i].special == 26 || lines[i].special == 32
                        ||  lines[i].special == 99 || lines[i].special == 133)
                        {
                            AM_drawMline(&l, 199);
                        }
                        // [JN] RED locked doors
                        else
                        if (lines[i].special == 28  || lines[i].special == 33
                        ||  lines[i].special == 134 || lines[i].special == 135)
                        {
                            AM_drawMline(&l, 178);
                        }
                        // [JN] YELLOW locked doors
                        else
                        if (lines[i].special == 27  || lines[i].special == 34
                        ||  lines[i].special == 136 || lines[i].special == 137)
                        {
                            AM_drawMline(&l, 161);
                        }
                        // [JN] floor level change
                        else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                        {
                            AM_drawMline(&l, 239);
                        }
                        // [JN] ceiling level change
                        else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                        {
                            AM_drawMline(&l, 133);
                        }
                        else if (cheating)
                        {
                            AM_drawMline(&l, 99);
                        }
                    }
                }
                else if (plr->powers[pw_allmap])
                {
                    if (!(lines[i].flags & ML_DONTDRAW))
                    {
                        AM_drawMline(&l, 99);
                    }
                }
                break;
            }
            //
            // Strife colors
            //
            case 4:
            {
                if (cheating || (lines[i].flags & ML_MAPPED))
                {
                    // villsa [STRIFE]
                    // [JN] Changed to Doom exit lines
                    if (lines[i].special == 11 || lines[i].special == 51
                    ||  lines[i].special == 52 || lines[i].special == 124)
                    {
                        AM_drawMline(&l, 119);
                    }
                    // villsa [STRIFE] lightlev is unused here
                    else if (!lines[i].backsector)
                    {
                        AM_drawMline(&l, 86);
                    }
                    else
                    {
                        // [JN] Various teleporters
                        if (lines[i].special == 39  || lines[i].special == 97
                        ||  lines[i].special == 125 || lines[i].special == 126)
                        {
                            AM_drawMline(&l, 135);
                        }
                        // secret door
                        else if (lines[i].flags & ML_SECRET)
                        {
                            // villsa [STRIFE] just draw the wall as is!
                            AM_drawMline(&l, 86);
                        }
                        // floor level change
                        else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                        {
                            AM_drawMline(&l, 203);
                        }
                        // ceiling level change
                        else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                        {
                            AM_drawMline(&l, 195);
                        }
                        else if (cheating)
                        {
                            AM_drawMline(&l, 98);
                        }
                    }
                }
                else if (plr->powers[pw_allmap])
                {
                    if (!(lines[i].flags & ML_DONTDRAW))
                    {
                        AM_drawMline(&l, 102);
                    }
                }
                break;
            }
            //
            // Unity colors
            //
            case 5:
            {
                if (cheating || (lines[i].flags & ML_MAPPED))
                {
                    if ((lines[i].flags & ML_DONTDRAW) && !cheating)
                    {
                        continue;
                    }
            
                    // [JN] One sided wall
                    if (!lines[i].backsector)
                    {
                        AM_drawMline(&l, 184);
                    }
                    else
                    {
                        // [JN] Secret door
                        if (lines[i].flags & ML_SECRET)
                        {
                            AM_drawMline(&l, 184);
                        }
                        // [JN] Various Doors
                        else
                        if (lines[i].special == 1   || lines[i].special == 31
                        ||  lines[i].special == 117 || lines[i].special == 118)
                        {
                            AM_drawMline(&l, 81);
                        }
                        // [JN] Various teleporters
                        else
                        if (lines[i].special == 39  || lines[i].special == 97
                        ||  lines[i].special == 125 || lines[i].special == 126)
                        {
                            AM_drawMline(&l, 120);
                        }
                        // [JN] BLUE locked doors
                        else
                        if (lines[i].special == 26 || lines[i].special == 32
                        ||  lines[i].special == 99 || lines[i].special == 133)
                        {
                            AM_drawMline(&l, 200);
                        }
                        // [JN] RED locked doors
                        else
                        if (lines[i].special == 28  || lines[i].special == 33
                        ||  lines[i].special == 134 || lines[i].special == 135)
                        {
                            AM_drawMline(&l, 176);
                        }
                        // [JN] YELLOW locked doors
                        else
                        if (lines[i].special == 27  || lines[i].special == 34
                        ||  lines[i].special == 136 || lines[i].special == 137)
                        {
                            AM_drawMline(&l, 160);
                        }
                        // [JN] Floor level change
                        else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                        {
                            AM_drawMline(&l, 72);
                        }
                        // [JN] Ceiling level change
                        else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                        {
                            AM_drawMline(&l, 64);
                        }
                        // [JN] IDDT visible lines
                        else if (cheating)
                        {
                            AM_drawMline(&l, 96);
                        }
                    }
                    // [JN] Exit (can be one-sided or two-sided)
                    if (lines[i].special == 11 || lines[i].special == 51
                    ||  lines[i].special == 52 || lines[i].special == 124)
                    {
                        AM_drawMline(&l, 252);
                    }
                }
                // [JN] Computermap visible lines
                else if (plr->powers[pw_allmap])
                {
                    if (!(lines[i].flags & ML_DONTDRAW)) AM_drawMline(&l, 104);
                }
                break;
            }
            //
            // DOOM colors
            //
            default:
            {
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
                        // teleporters
                        if (lines[i].special == 39)
                        {
                            AM_drawMline(&l, WALLCOLORS+WALLRANGE/2);
                        }
                        // secret door
                        else if (lines[i].flags & ML_SECRET)
                        {
                            AM_drawMline(&l, WALLCOLORS);
                        }
                        // floor level change
                        else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                        {
                            AM_drawMline(&l, FDWALLCOLORS);
                        }
                        // ceiling level change
                        else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
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
                    if (!(lines[i].flags & ML_DONTDRAW)) AM_drawMline(&l, GRAYS+3);
                }
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// AM_rotate
// Rotation in 2D. Used to rotate player arrow line character.
// -----------------------------------------------------------------------------

static void AM_rotate (int64_t *x, int64_t *y, angle_t a)
{
    int64_t tmpx;

    a >>= ANGLETOFINESHIFT;

    tmpx = FixedMul(*x, finecosine[a])
         - FixedMul(*y, finesine[a]);

    *y = FixedMul(*x, finesine[a])
       + FixedMul(*y, finecosine[a]);

    *x = tmpx;
}

// -----------------------------------------------------------------------------
// AM_rotatePoint
// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// AM_drawLineCharacter
// Draws a vector graphic according to numerous parameters.
// -----------------------------------------------------------------------------

static void AM_drawLineCharacter (const mline_t *lineguy, const int lineguylines, const fixed_t scale, 
                                  angle_t angle, const int color, const fixed_t x, const fixed_t y)
{
    int     i;
    mline_t l;

    if (automap_rotate)
    {
        angle += mapangle;
    }

    for (i = 0 ; i < lineguylines ; i++)
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

// -----------------------------------------------------------------------------
// AM_drawPlayers
// Draws the player arrow in single player, 
// or all the player arrows in a netgame.
// -----------------------------------------------------------------------------

static void AM_drawPlayers (void)
{
    int         i;
    int         color;
    int         their_color = -1;
    const int   their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
    mpoint_t    pt;
    player_t   *p;

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
    
        if (gamemission == jaguar)
        {
            // [JN] Jaguar Doom: draw dark green, blinking arrow.
            if (leveltime & 8)
            AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0, 
                                 smoothangle, GREEN_JAGUAR, pt.x, pt.y);
        }
        else
        {
            if (cheating)
            {
                AM_drawLineCharacter(cheat_player_arrow, arrlen(cheat_player_arrow), 0,
                                     smoothangle, WHITE, pt.x, pt.y);
            }
            else
            {
                AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0,
                                     smoothangle, WHITE, pt.x, pt.y);
            }
        }
        
        return;
    }

    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
        // [JN] Interpolate other player arrows angle.
        angle_t smoothangle;

        their_color++;
        p = &players[i];

        if ((deathmatch && !singledemo) && p != plr)
        {
            continue;
        }

        if (!playeringame[i])
        {
            continue;
        }

        if (p->powers[pw_invisibility])
        {
            color = 246; // *close* to black
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

        AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0,
                             smoothangle, color,pt.x, pt.y);
    }
}

// -----------------------------------------------------------------------------
// AM_drawThings
// Draws the things on the automap in double IDDT cheat mode.
// -----------------------------------------------------------------------------

static void AM_drawThings (const int colors, const int colorrange)
{
    int       i;
    mpoint_t  pt;
    mobj_t   *t;
    angle_t   actualangle;

    for (i = 0 ; i < numsectors ; i++)
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

            // [JN] IDDT extended colors:
            if (!vanillaparm)
            {
                // [JN] Use actual radius for things drawing.
                const fixed_t actualradius = t->radius >> FRACTOMAPBITS;

                // [crispy] draw blood splats and puffs as small squares
                if (t->type == MT_BLOOD || t->type == MT_PUFF)
                {
                    AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy),
                                         actualradius >> 2, actualangle, GRAYS_IDDT, pt.x, pt.y);
                }
                else
                {
                    AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy), 
                                         actualradius, actualangle, 
                                         // Monsters
                                         t->flags & MF_COUNTKILL ? (t->health > 0 ? REDS_IDDT : GRAYS_IDDT) :
                                         // Lost Souls and Explosive barrels (does not have a MF_COUNTKILL flag)
                                         t->type == MT_SKULL || t->type == MT_BARREL ? YELLOWS_IDDT :
                                         // Pickups
                                         t->flags & MF_SPECIAL ? GREENS_IDDT :
                                         // Everything else
                                         GRAYS_IDDT,
                                         pt.x, pt.y);
                }
            }
            else
            {
                AM_drawLineCharacter(thintriangle_guy, arrlen(thintriangle_guy),
                                     FRACUNIT, actualangle, colors, pt.x, pt.y);
            }

            t = t->snext;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawThings
// Draw the marked locations on the automap.
// -----------------------------------------------------------------------------

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
                    V_DrawPatchUnscaled(flip_levels ? - fx : fx, fy, marknums[d], NULL);
                    dp_translation = NULL;
                }

                // killough 2/22/98: 1 space backwards
                fx -= MARK_W - (flip_levels ? MARK_FLIP_2 : MARK_FLIP_1);

                j /= 10;
            } while (j > 0);
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawCrosshair
// [JN] Draw crosshair representing map center, or single point in vanilla mode.
// -----------------------------------------------------------------------------

static void AM_drawCrosshair (const int color)
{
    if (vanillaparm)
    {
        I_VideoBuffer[(f_w*(f_h+1))/2] = color; // single point for now
    }
    else
    {
        // [JN] Draw small graphical crosshair,
        // if automap isn't following player.
        if (!automap_follow)
        {
            dp_translation = cr[CR_GRAY];
            V_DrawPatchUnscaled(origwidth, 168, am_crosshair, NULL);
            dp_translation = NULL;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_Drawer
// Draws the entire automap.
// -----------------------------------------------------------------------------

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

    if (!automap_overlay)
    {
        AM_clearFB(BACKGROUND);
        skippsprinterp = true;
    }

    if (automap_grid)
    {
        AM_drawGrid(GRIDCOLORS);
    }

    AM_drawWalls(automap_color_set);

    AM_drawPlayers();

    if (cheating == 2)
    {
        AM_drawThings(THINGCOLORS, THINGRANGE);
    }

    // [JN] Jaguar Doom: don't draw single dot or crosshair
    if (gamemission != jaguar)
    {
        AM_drawCrosshair(XHAIRCOLORS);
    }

    AM_drawMarks();

    V_MarkRect(f_x, f_y, f_w, f_h);
}

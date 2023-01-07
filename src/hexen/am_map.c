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


#include "i_system.h"
#include "h2def.h"
#include "i_video.h"
#include "i_swap.h"
#include "m_misc.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "am_map.h"
#include "am_data.h"
#include "v_video.h"
#include "rd_lang.h"


//the MACRO!
#define DOT(xx,yy,cc) I_VideoBuffer[(yy)*f_w+(flipscreenwidth[xx])]=(cc)


int cheating = 0;
static int leveljuststarted = 1;    // kluge until AM_LevelInit() is called

boolean automapactive = false;

static int finit_height;
static int f_x, f_y;            // location of window on screen
static int f_w, f_h;            // size of window on screen

static mpoint_t m_paninc;       // how far the window pans each tic (map coords)
static fixed_t mtof_zoommul;    // how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul;    // how far the window zooms in each tic (fb coords)
static fixed_t curr_mtof_zoommul;  // [JN] Zooming interpolation.

static int64_t m_x, m_y;        // LL x,y where the window is on the map (map coords)
static int64_t m_x2, m_y2;      // UR x,y where the window is on the map (map coords)
static fixed_t prev_m_x, prev_m_y; // [JN] Panning interpolation.

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

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
static fixed_t prev_scale_mtof = (fixed_t)INITSCALEMTOF; // [JN] Panning interpolation.
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

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

// [JN] Pointer to background drawing functions.
static void (*AM_drawBackground) (void);
static void AM_drawBackgroundHigh (void);
static void AM_drawBackgroundQuad (void);

static player_t *plr;           // the player represented by an arrow
static vertex_t oldplr;

// [JN] Choosen mark color.
static Translation_CR_t automap_mark_color_set;
static patch_t *marknums[10]; // numbers used for marking by the automap

// [JN] killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.
mpoint_t *markpoints = NULL;     // where the points are
int       markpointnum = 0;      // next point to be assigned (also number of points now)
int       markpointnum_max = 0;  // killough 2/22/98

static char cheat_kills[] = { 'k', 'i', 'l', 'l', 's' };
static boolean ShowKills = 0;
static unsigned ShowKillsCount = 0;

extern boolean viewactive;

// [JN] Apply line antialiasing for all types of lines,
// apply inversive (faded to dark) colors in automap overlay mode.
static byte antialias[8][8] = {
    { 83,  84,  85,  86,  87,  88,  89,  90},   // WALLCOLORS
    { 96,  96,  95,  94,  93,  92,  91,  90},   // FDWALLCOLORS
    {107, 108, 109, 110, 111, 112,  89,  90},   // CDWALLCOLORS
    { 40,  40,  40,  41,  41,  42,  42,  43},   // TSWALLCOLORS (also GRAYS)
    {198, 198, 198, 198, 199, 199, 199, 199},   // GREENKEY
    {157, 157, 157, 158, 158, 158, 159, 159},   // BLUEKEY
    {177, 177, 178, 178, 179, 179, 180, 180},   // BLOODRED
    { 32,  32,  32,  32,  32,  32,  32,  32}    // WHITE (no antialiasing)
};

static byte antialias_overlay[8][8] = {
    { 88,  87,  86,  85,  84,  83,  82,  81},   // WALLCOLORS
    { 95,  93,  91,  89,  87,  85,  84,  83},   // FDWALLCOLORS
    {107, 106, 105, 104, 103, 102, 101, 100},   // CDWALLCOLORS
    { 40,  39,  38,  37,  36,  35,  34,  33},   // TSWALLCOLORS (also GRAYS)
    {198, 198, 197, 197, 196, 196, 195, 194},   // GREENKEY
    {157, 157, 156, 156, 155, 155, 154, 154},   // BLUEKEY
    {177, 177, 176, 176, 175, 175, 174, 173},   // BLOODRED
    { 32,  31,  30,  29,  28,  27,  26,  25}    // WHITE
};

static byte *maplump;        // Automap background patch.
static short mapystart = 0;  // y-value for the start of the map bitmap...used in the parallax stuff.
static short mapxstart = 0;  // x-value for the bitmap.


// [crispy] automap rotate mode ...
// ... needs these early on
static void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

// Functions

static void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                       int NumLevels, unsigned short IntensityBits);

static void AM_DrawDeathmatchStats(void);
static void DrawWorldTimer(void);


static void AM_activateNewScale(void)
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

static void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

static void AM_restoreScaleAndLoc(void)
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

static void AM_findMinMaxBoundaries(void)
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
    max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
    max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

    min_w = 2 * PLAYERRADIUS;
    min_h = 2 * PLAYERRADIUS;

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * MAPPLAYERRADIUS);

}

static void AM_changeWindowLoc(void)
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

    // The following code was commented out in the released Hexen source,
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

// -----------------------------------------------------------------------------
// AM_initMarksColor
// -----------------------------------------------------------------------------

void AM_initMarksColor (int color)
{
    Translation_CR_t *colorVar = &automap_mark_color_set;

    switch (color)
    {
        case 1:   *colorVar = CR_GRAY;             break;
        case 2:   *colorVar = CR_DARKGRAY;         break;
        case 3:   *colorVar = CR_RED;              break;
        case 4:   *colorVar = CR_DARKRED;          break;
        case 5:   *colorVar = CR_GREEN;            break;
        case 6:   *colorVar = CR_DARKGREEN;        break;
        case 7:   *colorVar = CR_OLIVE;            break;
        case 8:   *colorVar = CR_BLUE2;            break;
        case 9:   *colorVar = CR_DARKBLUE;         break;
        case 10:  *colorVar = CR_NIAGARA;          break;
        case 11:  *colorVar = CR_BRIGHTYELLOW;     break;
        case 12:  *colorVar = CR_YELLOW;           break;
        case 13:  *colorVar = CR_TAN;              break;
        case 14:  *colorVar = CR_BROWN;            break;
        default:  *colorVar = CR_WHITE;            break;
    }
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

static void AM_initVariables(void)
{
    int pnum;
    thinker_t *think;

    automapactive = true;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (!playeringame[pnum = consoleplayer])
        for (pnum = 0; pnum < maxplayers; pnum++)
            if (playeringame[pnum])
                break;
    plr = &players[pnum];
    oldplr.x = plr->mo->x;
    oldplr.y = plr->mo->y;
    m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
    m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;

    AM_Ticker();
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // load in the location of keys, if in baby mode
    if (gameskill == sk_baby)
    {
        for (think = thinkercap.next; think != &thinkercap;
             think = think->next)
        {
            if (think->function != P_MobjThinker)
            {                   //not a mobj
                continue;
            }
        }
    }
}

static void AM_loadPics(void)
{
    int  i;
    char namebuf[9];

    maplump = W_CacheLumpName("AUTOPAGE", PU_STATIC);

    for (i = 0 ; i < 10 ; i++)
    {
        // [JN] Use custom, precise versions of automap marks.
        M_snprintf(namebuf, 9, "MARKNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }

    AM_drawBackground = quadres ? AM_drawBackgroundQuad : AM_drawBackgroundHigh;
}

// should be called at the start of every level
// right now, i figure it out myself

static void AM_LevelInit(void)
{
    leveljuststarted = 0;

    finit_height = SCREENHEIGHT - SBARHEIGHT;
    f_x = f_y = 0;
    f_w = screenwidth;
    f_h = finit_height;
    mapxstart = mapystart = 0;

    AM_findMinMaxBoundaries();

    scale_mtof = FixedDiv(min_scale_mtof, (int) (0.7*FRACUNIT));

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
        // don't show automap if we aren't in a game!
        return;
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

static void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// set the window scale to the minimum size

static void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

boolean AM_Responder(event_t * ev)
{
    int rc;
    static int bigstate = 0;
    static char buffer[32];
    boolean speed_toggler = BK_isKeyPressed(bk_speed);

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
        if (BK_isKeyDown(ev, bk_map_toggle) && gamestate == GS_LEVEL)
        {
            AM_Start();
            SB_state = -1;
            viewactive = false;
            rc = true;
        }
    }
    else
    {
        rc = true;

        if (BK_isKeyDown(ev, bk_right))                 // pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow)
            {
                m_paninc.x = flip_levels ? -FTOM(f_paninc) : FTOM(f_paninc);
            }
            else
                rc = false;
        }
        else if (BK_isKeyDown(ev, bk_left))                   // pan left
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
        else if (BK_isKeyDown(ev, bk_up))             // pan up
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
        else if (BK_isKeyDown(ev, bk_down))                   // pan down
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
        else if (BK_isKeyDown(ev, bk_map_zoom_out))                   // zoom out
        {
            mtof_zoommul = m_zoomout;
            ftom_zoommul = m_zoomin;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_in))            // zoom in
        {
            mtof_zoommul = m_zoomin;
            ftom_zoommul = m_zoomout;
            curr_mtof_zoommul = mtof_zoommul;
        }
        else if (BK_isKeyDown(ev, bk_map_toggle))
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop();
            SB_state = -1;
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_max))
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
        else if (BK_isKeyDown(ev, bk_map_follow))
        {
            automap_follow = !automap_follow;
            P_SetMessage(plr, automap_follow ? amstr_followon : amstr_followoff, msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_overlay))
        {
            automap_overlay = !automap_overlay;
            P_SetMessage(plr, automap_overlay ? amstr_overlayon : amstr_overlayoff, msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_rotate))
        {
            automap_rotate = !automap_rotate;
            P_SetMessage(plr, automap_rotate ? amstr_rotateon : amstr_rotateoff, msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_grid))
        {
            automap_grid = !automap_grid;
            P_SetMessage(plr, automap_grid ? amstr_gridon : amstr_gridoff, msg_system, false);
        }
        else if (BK_isKeyDown(ev, bk_map_mark))
        {
            // [JN] "Mark № added" / "Отметка № добавлена".
            M_snprintf(buffer, sizeof(buffer), "%s %d %s",
                       amstr_mark, markpointnum, amstr_added);
            P_SetMessage(plr, buffer, msg_system, false);
            AM_addMark();
        }
        else if (BK_isKeyPressed(bk_speed) && BK_isKeyDown(ev, bk_map_clearmark))
        {
            // [JN] Clear all mark by holding "run" button and pressing "clear mark".
            if (markpointnum > 0)
            {
                P_SetMessage(plr, amstr_markscleared, msg_system, false);
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
                P_SetMessage(plr, buffer, msg_system, false);
            }
        }
        else
        {
            if (BK_isKeyUp(ev, bk_left))
            {
                if (!automap_follow)
                    m_paninc.x = 0;
            }
            else if (BK_isKeyUp(ev, bk_right))
            {
                if (!automap_follow)
                    m_paninc.x = 0;
            }
            else if (BK_isKeyUp(ev, bk_up))
            {
                if (!automap_follow)
                    m_paninc.y = 0;
            }
            else if (BK_isKeyUp(ev, bk_down))
            {
                if (!automap_follow)
                    m_paninc.y = 0;
            }
            else if (BK_isKeyUp(ev, bk_map_zoom_out) || BK_isKeyUp(ev, bk_map_zoom_in))
            {
                mtof_zoommul = FRACUNIT;
                ftom_zoommul = FRACUNIT;
            }
            rc = false;
        }

        if (ev->type == ev_keydown && cheat_kills[ShowKillsCount] == ev->data1 && netgame && deathmatch)
        {
            ShowKillsCount++;
            if (ShowKillsCount == 5)
            {
                ShowKillsCount = 0;
                rc = false;
                ShowKills ^= 1;
            }
        }
        else
        {
            ShowKillsCount = 0;
        }
    }
    return rc;
}

static void AM_changeWindowScale(void)
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

static void AM_doautomap_follow (void)
{
    // [JN] Use interpolated player coords for smooth
    // scrolling and static player arrow position.
    m_x = (viewx >> FRACTOMAPBITS) - m_w/2;
    m_y = (viewy >> FRACTOMAPBITS) - m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

void AM_Ticker(void)
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
= AM_clearFB
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

// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If I need the speed, will
// hash algorithm to the common cases.

static boolean AM_clipMline(mline_t * ml, fline_t * fl)
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

static void AM_drawFline(fline_t * fl, int color)
{
    int x, y, dx, dy, sx, sy, ax, ay, d;

    switch (color)
    {
        case WALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[0][0] :
                                         &antialias[0][0], 8, 3);
            break;
        case FDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, 
                       automap_overlay ? &antialias_overlay[1][0] :
                                         &antialias[1][0], 8, 3);
            break;
        case CDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[2][0] :
                                         &antialias[2][0], 8, 3);
            break;
        // [JN] Apply antialiasing to the rest of the lines
        case TSWALLCOLORS: // [JN] ... and GRAYS
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[3][0] :
                                         &antialias[3][0], 8, 3);
            break;
        case GREENKEY:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[4][0] :
                                         &antialias[4][0], 8, 3);
            break;
        case BLUEKEY:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[5][0] :
                                         &antialias[5][0], 8, 3);
            break;
        case BLOODRED:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[6][0] :
                                         &antialias[6][0], 8, 3);
            break;
        case WHITE:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       automap_overlay ? &antialias_overlay[7][0] :
                                         &antialias[7][0], 8, 3);
            break;
        default:
            {
                // For debugging only
                if (fl->a.x < 0 || fl->a.x >= f_w
                    || fl->a.y < 0 || fl->a.y >= f_h
                    || fl->b.x < 0 || fl->b.x >= f_w
                    || fl->b.y < 0 || fl->b.y >= f_h)
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
static void PUTDOT(short xx, short yy, byte * cc, byte * cm)
{
    static int oldyy;
    static int oldyyshifted;
    byte *oldcc = cc;

    if (xx < 32)
        cc += 7 - (xx >> 2);
    else if (xx > (screenwidth - 32))
        cc += 7 - ((screenwidth - xx) >> 2);
    if (yy < 32)
        cc += 7 - (yy >> 2);
    else if (yy > (finit_height - 32))
        cc += 7 - ((finit_height - yy) >> 2);
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

static void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
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
        ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY;
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
    ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX;
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

static void AM_drawMline(mline_t * ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // draws it on frame buffer using fb coords
        AM_drawFline(&fl, color);
    }

}

static void AM_drawGrid(int color)
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
    for (x = start; x < end; x += gridsize)
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

    // draw horizontal gridlines
    for (y = start; y < end; y += gridsize)
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

static void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i = 0; i < numlines; i++)
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
                continue;
            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS);
            }
            else
            {
                if (lines[i].flags & ML_SECRET) // secret door
                {
                    if (cheating)
                        AM_drawMline(&l, 0);
                    else
                        AM_drawMline(&l, WALLCOLORS);
                }
                else if (lines[i].special == 13 || lines[i].special == 83)
                {               // Locked door line -- all locked doors are greed
                    AM_drawMline(&l, GREENKEY);
                }
                else if (lines[i].special == 70 || lines[i].special == 71)
                {               // intra-level teleports are blue
                    AM_drawMline(&l, BLUEKEY);
                }
                else if (lines[i].special == 74 || lines[i].special == 75)
                {               // inter-level teleport/game-winning exit -- both are red
                    AM_drawMline(&l, BLOODRED);
                }
                else if (lines[i].backsector->floorheight
                         != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS);  // floor level change
                }
                else if (lines[i].backsector->ceilingheight
                         != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS);  // ceiling level change
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
                AM_drawMline(&l, GRAYS + 3);
        }
    }

}

static void AM_rotate (int64_t* x, int64_t* y, angle_t a)
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
    const angle_t actualangle = (!(!automap_follow && automap_overlay)) ? ANG90 - viewangle :  mapangle;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[actualangle>>ANGLETOFINESHIFT])
         - (int64_t)FixedMul(pt->y, finesine[actualangle>>ANGLETOFINESHIFT])
         + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[actualangle>>ANGLETOFINESHIFT])
          + (int64_t)FixedMul(pt->y, finecosine[actualangle>>ANGLETOFINESHIFT])
          + mapcenter.y;

    pt->x = tmpx;
}

static void AM_drawLineCharacter(mline_t * lineguy, int lineguylines, fixed_t scale,
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

static void AM_drawPlayers(void)
{
    int i;
    player_t *p;
    mpoint_t pt;

    const int their_colors[] = {
        AM_PLR1_COLOR,
        AM_PLR2_COLOR,
        AM_PLR3_COLOR,
        AM_PLR4_COLOR,
        AM_PLR5_COLOR,
        AM_PLR6_COLOR,
        AM_PLR7_COLOR,
        AM_PLR8_COLOR
    };
    int their_color = -1;
    int color;
    // [JN] Smooth player arrow rotation:
    const angle_t smoothangle = automap_rotate ? plr->mo->angle : viewangle;

    if (!netgame)
    {
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

    for (i = 0; i < maxplayers; i++)
    {
        their_color++;
        p = &players[i];
        if (deathmatch && !singledemo && p != plr)
        {
            continue;
        }
        if (!playeringame[i])
            continue;
        color = their_colors[their_color];

        pt.x = p->mo->x >> FRACTOMAPBITS;
        pt.y = p->mo->y >> FRACTOMAPBITS;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, p->mo->angle,
                             color, pt.x, pt.y);
    }
}

static void AM_drawThings(int colors, int colorrange)
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

            // [JN] Interpolate things if possible.
            if (uncapped_fps && !vanillaparm && leveltime > oldleveltime)
            {
                pt.x = (t->oldx + FixedMul(t->x - t->oldx, fractionaltic)) >> FRACTOMAPBITS;
                pt.y = (t->oldy + FixedMul(t->y - t->oldy, fractionaltic)) >> FRACTOMAPBITS;
            }
            else
            {
                pt.x = t->x >> FRACTOMAPBITS;
                pt.y = t->y >> FRACTOMAPBITS;
            }

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                 16 << MAPBITS, t->angle, colors,
                                 pt.x, pt.y);
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
                    fx += (flip_levels ? -MARK_FLIP_1 : MARK_FLIP_1); // -1 : 1
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
                fx -= MARK_W - (flip_levels ? MARK_FLIP_2 : MARK_FLIP_1); // 9 : 1

                j /= 10;
            } while (j > 0);
        }
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
    V_DrawPatchUnscaled(origwidth, 162, 
                        W_CacheLumpName("XHAIR_1", PU_STATIC), NULL);
}

void AM_Drawer(void)
{
    if (!automapactive)
    {
        return;
    }

    // [JN] Moved from AM_Ticker for drawing interpolation.
    if (automap_follow)
    {
        AM_doautomap_follow();
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
    DrawWorldTimer();

    if (cheating == 2)
    {
        AM_drawThings(THINGCOLORS, THINGRANGE);
    }
    
    if (!automap_follow && !vanillaparm)
    {
        AM_drawCrosshair();
    }

    AM_drawMarks();

    if (english_language)
    {
        RD_M_DrawTextA(P_GetMapName(gamemap), 38 + wide_delta, 144);
    }
    else
    {
        RD_M_DrawTextSmallRUS(P_GetMapName(gamemap), 38 + wide_delta, 144, CR_NONE);
    }

    if (ShowKills && netgame && deathmatch)
    {
        AM_DrawDeathmatchStats();
    }
}

//===========================================================================
//
// AM_DrawDeathmatchStats
//
//===========================================================================

// 8-player note:  Proper player color names here, too

char *PlayerColorText[MAXPLAYERS] = {
    "BLUE:",
    "RED:",
    "YELLOW:",
    "GREEN:",
    "JADE:",
    "WHITE:",
    "HAZEL:",
    "PURPLE:"
};

static void AM_DrawDeathmatchStats(void)
{
    int i, j, k, m;
    int fragCount[MAXPLAYERS];
    int order[MAXPLAYERS];
    char textBuffer[80];
    int yPosition;

    for (i = 0; i < maxplayers; i++)
    {
        fragCount[i] = 0;
        order[i] = -1;
    }
    for (i = 0; i < maxplayers; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        else
        {
            for (j = 0; j < maxplayers; j++)
            {
                if (playeringame[j])
                {
                    fragCount[i] += players[i].frags[j];
                }
            }
            for (k = 0; k < maxplayers; k++)
            {
                if (order[k] == -1)
                {
                    order[k] = i;
                    break;
                }
                else if (fragCount[i] > fragCount[order[k]])
                {
                    for (m = maxplayers - 1; m > k; m--)
                    {
                        order[m] = order[m - 1];
                    }
                    order[k] = i;
                    break;
                }
            }
        }
    }
    yPosition = 15;
    for (i = 0; i < maxplayers; i++)
    {
        if (!playeringame[order[i]])
        {
            continue;
        }
        else
        {
            RD_M_DrawTextA(PlayerColorText[order[i]], 8, yPosition);
            M_snprintf(textBuffer, sizeof(textBuffer),
                       "%d", fragCount[order[i]]);
            RD_M_DrawTextA(textBuffer, 80, yPosition);
            yPosition += 10;
        }
    }
}

//===========================================================================
//
// DrawWorldTimer
//
//===========================================================================

static void DrawWorldTimer(void)
{
    int hours;
    int minutes;
    int seconds;
    int worldTimer = players[consoleplayer].worldTimer;
    char timeBuffer[32];
    char skill[15];
    boolean wide_4_3 = aspect_ratio >= 2 && screenblocks == 9;

    worldTimer /= 35;
    hours = worldTimer / 3600;
    worldTimer -= hours * 3600;
    minutes = worldTimer / 60;
    worldTimer -= minutes * 60;
    seconds = worldTimer;

    // [JN] Compose time to: HHHHH:MM:SS format
    M_snprintf(timeBuffer, sizeof(timeBuffer),
               "%5d : %.2d : %.2d", hours, minutes, seconds);
    RD_M_DrawTextA(timeBuffer, 224 + (wide_4_3 ? wide_delta : wide_delta*2), 8);

    // [JN] Draw skill level
    if (english_language)
    {
        M_snprintf(skill, sizeof(skill), "SKILL: %d", gameskill + 1);
        RD_M_DrawTextA(skill, 264 + (wide_4_3 ? wide_delta : wide_delta*2), 18);
    }
    else
    {
        // СЛОЖНОСТЬ:
        M_snprintf(skill, sizeof(skill), "CKJ;YJCNM: %d", gameskill + 1);
        RD_M_DrawTextSmallRUS(skill, 222 + (wide_4_3 ? wide_delta : wide_delta*2), 18, CR_NONE);
    }
}

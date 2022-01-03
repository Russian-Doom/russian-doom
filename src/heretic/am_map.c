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
// AM_map.c


#include <stdlib.h>
#include "doomdef.h"
#include "deh_str.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "am_map.h"
#include "am_data.h"
#include "v_video.h"
#include "m_misc.h"
#include "jn.h"


boolean automapactive = false;
vertex_t KeyPoints[NUMKEYS];

static int cheating = 0;
static int leveljuststarted = 1;  // kluge until AM_LevelInit() is called

static int finit_height = SCREENHEIGHT - (42 << hires);
static int f_x, f_y;            // location of window on screen
static int f_w, f_h;            // size of window on screen
static byte *fb;                // pseudo-frame buffer

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

static boolean stopped = true;

extern boolean viewactive;

// [JN] How much the automap moves window per tic in frame-buffer coordinates.
static int f_paninc;
static const int f_paninc_slow = 8;   // 280 map units in 1 second.
static const int f_paninc_fast = 16;  // 560 map units in 1 second.

// [JN] How much zoom-in per tic goes to 2x in 1 second.
static int m_zoomin;
static const int m_zoomin_slow = ((int) (1.04*FRACUNIT));
static const int m_zoomin_fast = ((int) (1.08*FRACUNIT));

// [JN] How much zoom-out per tic pulls out to 0.5x in 1 second.
static int m_zoomout;
static const int m_zoomout_slow = ((int) (FRACUNIT/1.04));
static const int m_zoomout_fast = ((int) (FRACUNIT/1.08));

// [JN] Choosen mark color.
static Translation_CR_t automap_mark_color_set;
static patch_t *marknums[10]; // numbers used for marking by the automap

// [JN] killough 2/22/98: Remove limit on automap marks,
// and make variables external for use in savegames.
mpoint_t *markpoints = NULL;     // where the points are
int       markpointnum = 0;      // next point to be assigned (also number of points now)
int       markpointnum_max = 0;  // killough 2/22/98

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

static patch_t *maplump;     // [JN] Pointer to the GFX patch for the automap background.
static short mapystart = 0;  // y-value for the start of the map bitmap...used in the paralax stuff.
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
        m_x = plr->mo->x - m_w / 2;
        m_y = plr->mo->y - m_h / 2;
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
    max_w = max_x/2 - min_x/2;
    max_h = max_y/2 - min_y/2;

    min_w = 2 * PLAYERRADIUS;
    min_h = 2 * PLAYERRADIUS;

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);

    min_scale_mtof = a < b ? a/2 : b/2;
    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);
}

static void AM_changeWindowLoc(void)
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

// -----------------------------------------------------------------------------
// AM_initMarksColor
// -----------------------------------------------------------------------------

void AM_initMarksColor (int color)
{
    Translation_CR_t *colorVar = &automap_mark_color_set;

    switch (color)
    {
        case 1:   *colorVar = CR_WHITE2GRAY_HERETIC;       break;
        case 2:   *colorVar = CR_WHITE2DARKGRAY_HERETIC;   break;
        case 3:   *colorVar = CR_WHITE2RED_HERETIC;        break;
        case 4:   *colorVar = CR_WHITE2DARKRED_HERETIC;    break;
        case 5:   *colorVar = CR_WHITE2GREEN_HERETIC;      break;
        case 6:   *colorVar = CR_WHITE2DARKGREEN_HERETIC;  break;
        case 7:   *colorVar = CR_WHITE2OLIVE_HERETIC;      break;
        case 8:   *colorVar = CR_WHITE2BLUE_HERETIC;       break;
        case 9:   *colorVar = CR_WHITE2DARKBLUE_HERETIC;   break;
        case 10:  *colorVar = CR_WHITE2PURPLE_HERETIC;     break;
        case 11:  *colorVar = CR_WHITE2NIAGARA_HERETIC;    break;
        case 12:  *colorVar = CR_WHITE2AZURE_HERETIC;      break;
        case 13:  *colorVar = CR_WHITE2YELLOW_HERETIC;     break;
        case 14:  *colorVar = CR_WHITE2GOLD_HERETIC;       break;
        case 15:  *colorVar = CR_WHITE2DARKGOLD_HERETIC;   break;
        case 16:  *colorVar = CR_WHITE2TAN_HERETIC;        break;
        case 17:  *colorVar = CR_WHITE2BROWN_HERETIC;      break;
        default:  *colorVar = CR_WHITE2DARKGREEN_HERETIC;  break;
    }
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
        markpoints = realloc(markpoints,
                            (markpointnum_max = markpointnum_max ? 
                             markpointnum_max*2 : 16) * sizeof(*markpoints));
    }

    // [crispy] keep the map static in overlay mode if not following the player
    if (!(!automap_follow && automap_overlay))
    {
        markpoints[markpointnum].x = m_x + m_w/2;
        markpoints[markpointnum].y = m_y + m_h/2;
    }
    else
    {
        markpoints[markpointnum].x = plr->mo->x;
        markpoints[markpointnum].y = plr->mo->y;
    }
    markpointnum++;
}

// -----------------------------------------------------------------------------
// AM_clearMarks
// -----------------------------------------------------------------------------

void AM_clearMarks (void)
{
    markpointnum = 0;
}

static void AM_initVariables(void)
{
    int pnum;
    thinker_t *think;
    mobj_t *mo;

    automapactive = true;
    fb = I_VideoBuffer;

    f_oldloc.x = INT_MAX;

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

static void AM_loadPics(void)
{
    int  i;
    char namebuf[9];

    // [JN] Parallax problem: AUTOPAGE changed to unreplacable MAPEPAGE.
    maplump = W_CacheLumpName(DEH_String("MAPEPAGE"), PU_STATIC);

    for (i = 0 ; i < 10 ; i++)
    {
        // [JN] Use custom, precise versions of automap marks.
        DEH_snprintf(namebuf, 9, "MARKNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }
}

// should be called at the start of every level
// right now, i figure it out myself

static void AM_LevelInit(void)
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

boolean AM_Responder(event_t *ev)
{
    int rc;
    static int bigstate = 0;
    static char buffer[32];
    boolean speed_toggler = BK_isKeyPressed(bk_speed);

    // [JN] If run button is hold, pan/zoom Automap faster.    
    if (speed_toggler)
    {
        f_paninc = f_paninc_fast;
        m_zoomin = m_zoomin_fast;
        m_zoomout = m_zoomout_fast;
    }
    else
    {
        f_paninc = f_paninc_slow;
        m_zoomin = m_zoomin_slow;
        m_zoomout = m_zoomout_slow;
    }

    rc = false;

    if (!automapactive)
    {

        if (BK_isKeyDown(ev, bk_map_toggle) && gamestate == GS_LEVEL)
        {
            AM_Start();
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
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? -FTOM(f_paninc) : FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_left))            // pan left
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? FTOM(f_paninc) : -FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_up))           // pan up
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.y = FTOM(f_paninc);
            }
            else
                rc = false;
        }
        else if (BK_isKeyDown(ev, bk_down))           // pan down
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.y = -FTOM(f_paninc);
            }
            else
            {
                rc = false;
            }
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_out))         // zoom out
        {
            mtof_zoommul = m_zoomout;
            ftom_zoommul = m_zoomin;
        }
        else if (BK_isKeyDown(ev, bk_map_zoom_in))          // zoom in
        {
            mtof_zoommul = m_zoomin;
            ftom_zoommul = m_zoomout;
        }
        else if (BK_isKeyDown(ev, bk_map_toggle))          // toggle map (tab)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop();
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
            f_oldloc.x = INT_MAX;
            P_SetMessage(plr, automap_follow ?
                              amstr_followon : amstr_followoff, msg_uncolored, true);
        }
        else if (BK_isKeyDown(ev, bk_map_overlay))
        {
            automap_overlay = !automap_overlay;
            P_SetMessage(plr, automap_overlay ?
                              amstr_overlayon : amstr_overlayoff, msg_uncolored, true);
        }
        else if (BK_isKeyDown(ev, bk_map_rotate))
        {
            automap_rotate = !automap_rotate;
            P_SetMessage(plr, automap_rotate ?
                              amstr_rotateon : amstr_rotateoff, msg_uncolored, true);
        }
        else if (BK_isKeyDown(ev, bk_map_grid))
        {
            automap_grid = !automap_grid;
            P_SetMessage(plr, automap_grid ?
                              amstr_gridon : amstr_gridoff, msg_uncolored, true);
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

        if (ev->type == ev_keydown && cheat_amap[cheatcount] == ev->data1 && !netgame)
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

    return rc;
}

static void AM_changeWindowScale(void)
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

static void AM_doFollowPlayer(void)
{
    if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
        // [JN] Use interpolated player coords for smooth
        // scrolling and static player arrow position.
        m_x = plr->mo->x - m_w/2;
        m_y = plr->mo->y - m_h/2;
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
    {
        return;
    }

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
}

static void AM_clearFB(int color)
{
    int dmapx;
    int dmapy;

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

    // [JN] Draw automap background as tiled GFX patches.
    V_DrawPatchUnscaled(0, 0, maplump, NULL);
    V_DrawPatchUnscaled(560, 0, maplump, NULL);
    V_DrawPatchUnscaled(0, 200, maplump, NULL);
    V_DrawPatchUnscaled(560, 200, maplump, NULL); 
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

static void AM_drawFline(fline_t * fl, int color)
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
static void PUTDOT(short xx, short yy, byte * cc, byte * cm)
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

static void AM_drawMline(mline_t * ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
        AM_drawFline(&fl, color);       // draws it on frame buffer using fb coords

}

static void AM_drawGrid(int color)
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

    if ((start-bmaporgx)%(automap_grid_size<<FRACBITS))
    {
        start -= ((start-bmaporgx)%(automap_grid_size<<FRACBITS));
    }

    end = m_x + m_w;
    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x = start; x < end; x += (automap_grid_size << FRACBITS))
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

    if ((start-bmaporgy)%(automap_grid_size<<FRACBITS))
    {
        start -= ((start-bmaporgy)%(automap_grid_size<<FRACBITS));
    }
    
    end = m_y + m_h;
    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y = start; y < end; y += (automap_grid_size << FRACBITS))
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
                AM_drawMline(&l, WALLCOLORS);
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
                        AM_drawMline(&l, WALLCOLORS);
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
            if (!(lines[i].flags & LINE_NEVERSEE))
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
    const angle_t actualangle = (!(!automap_follow && automap_overlay)) ? ANG90 - viewangle : mapangle;

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

            pt.x = t->x;
            pt.y = t->y;
            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                 16 << FRACBITS, t->angle, colors,
                                 pt.x, pt.y);
            t = t->snext;
        }
    }
}

// -----------------------------------------------------------------------------
// AM_drawThings
// Draw the marked locations on the automap.
// -----------------------------------------------------------------------------

static const int mark_w = 5 << hires;
static const int mark_flip_1 =  1 << hires;
static const int mark_flip_2 = -1 << hires;
static const int mark_flip_3 =  9 << hires;

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
                    fx += (flip_levels ? mark_flip_2 : mark_flip_1); // -1 : 1
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
                fx -= mark_w - (flip_levels ? mark_flip_3 : mark_flip_1); // 9 : 1

                j /= 10;
            } while (j > 0);
        }
    }
}

static void AM_drawkeys(void)
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

void AM_Drawer(void)
{
    char *level_name;
    int numepisodes;
    boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);

    if (!automapactive)
    {
        return;
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
    AM_drawMarks();
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
        if (aspect_ratio >= 2)
        {
            if (english_language)
            {
                RD_M_DrawTextA(DEH_String(level_name), 4 +
                              (wide_4_3 ? wide_delta : 0), 136);
            }
            else
            {
                RD_M_DrawTextSmallRUS(DEH_String(level_name), 4 +
                                     (wide_4_3 ? wide_delta : 0), 136, CR_NONE);
            }
        }
        else
        {
            if (english_language)
            {
                RD_M_DrawTextA(DEH_String(level_name), 20, 146);
            }
            else
            {
                RD_M_DrawTextSmallRUS(DEH_String(level_name), 20, 146, CR_NONE);
            }
        }
    }
}

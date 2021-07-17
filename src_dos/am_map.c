//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
// DESCRIPTION:  the automap code
//

#include <stdio.h>

#include "z_zone.h"
#include "doomdef.h"
#include "st_stuff.h"
#include "p_local.h"
#include "w_wad.h"
#include "dutils.h"
#include "i_system.h"
#include "v_video.h"
#include "doomstat.h"
#include "r_state.h"
#include "am_map.h"
#include "rd_lang.h"
#include "jn.h"


// For use if I do walls with outsides/insides
#define REDS                176
#define REDRANGE            16
#define BLUES               200
#define BLUERANGE           8
#define GREENS              112
#define GREENRANGE          16
#define GRAYS               96
#define GRAYSRANGE          16
#define BROWNS              64
#define BROWNRANGE          16
#define YELLOWS             231
#define YELLOWRANGE         1
#define BLACK               0
#define WHITE               209

// Automap colors
#define BACKGROUND          BLACK
#define YOURCOLORS          WHITE
#define YOURRANGE           0
#define WALLCOLORS          REDS
#define WALLRANGE           REDRANGE
#define TSWALLCOLORS        GRAYS
#define TSWALLRANGE         GRAYSRANGE
#define FDWALLCOLORS        BROWNS
#define FDWALLRANGE         BROWNRANGE
#define CDWALLCOLORS        YELLOWS
#define CDWALLRANGE         YELLOWRANGE
#define THINGCOLORS         GREENS
#define THINGRANGE          GREENRANGE
#define SECRETWALLCOLORS    WALLCOLORS
#define SECRETWALLRANGE     WALLRANGE
#define GRIDCOLORS          (GRAYS + GRAYSRANGE/2)
#define GRIDRANGE           0
#define XHAIRCOLORS         GRAYS

// [JN] Control keys
#define AM_PANDOWNKEY       KEY_DOWNARROW
#define AM_PANUPKEY         KEY_UPARROW
#define AM_PANRIGHTKEY      KEY_RIGHTARROW
#define AM_PANLEFTKEY       KEY_LEFTARROW
#define AM_ZOOMINKEY        '='
#define AM_ZOOMOUTKEY       '-'
#define AM_STARTKEY         KEY_TAB
#define AM_ENDKEY           KEY_TAB
#define AM_GOBIGKEY         '0'
#define AM_FOLLOWKEY        'f'
#define AM_GRIDKEY          'g'
#define AM_MARKKEY          'm'
#define AM_CLEARMARKKEY     'c'
#define AM_ROTATEKEY        'r'

#define AM_NUMMARKPOINTS 10

// scale on entry
#define INITSCALEMTOF   (.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC        4
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int)(1.02*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int)(FRACUNIT/1.02))

// translates between frame-buffer and map distances
#define FTOM(x) (((int64_t)((x)<<16) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)   (f_x + MTOF((x)-m_x))
#define CYMTOF(y)   (f_y + (f_h - MTOF((y)-m_y)))

// the following is crap
#define LINE_NEVERSEE ML_DONTDRAW

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
    int64_t x,y;
} mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;


//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/4 } },  // ----->
    { { R, 0 }, { R-R/2, -R/4 } },
    { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
    { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/6 } },  // ----->
    { { R, 0 }, { R-R/2, -R/6 } },
    { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
    { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
    { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
    { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
    { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
    { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
    { { -R/6, -R/6 }, { 0, -R/6 } },
    { { 0, -R/6 }, { 0, R/4 } },
    { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
    { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
    { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t triangle_guy[] = {
    { { -.867*R, -.5*R }, { .867*R, -.5*R } },
    { { .867*R, -.5*R } , { 0, R } },
    { { 0, R }, { -.867*R, -.5*R } }
};
#undef R
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
    { { -.5*R, -.7*R }, { R, 0 } },
    { { R, 0 }, { -.5*R, .7*R } },
    { { -.5*R, .7*R }, { -.5*R, -.7*R } }
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))


static int  cheating = 0;

static int  leveljuststarted = 1; // kluge until AM_LevelInit() is called

boolean     automapactive = false;
static int  finit_height = SCREENHEIGHT - 32;

// location of window on screen
static int  f_x;
static int  f_y;

// size of window on screen
static int  f_w;
static int  f_h;

static int      lightlev; // used for funky strobing effect
static byte*    fb;       // pseudo-frame buffer
static int      amclock;

static mpoint_t m_paninc;     // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)

static int64_t  m_x, m_y;     // LL x,y where the window is on the map (map coords)
static int64_t  m_x2, m_y2;   // UR x,y where the window is on the map (map coords)

//
// width/height of window on map (map coords)
//
static int64_t  m_w;
static int64_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y; 
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w; // max_x-min_x,
static fixed_t  max_h; // max_y-min_y

// based on player size
static fixed_t  min_w;
static fixed_t  min_h;

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t  old_m_w, old_m_h;
static int64_t  old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t  scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t  scale_ftom;

static player_t *plr; // the player represented by an arrow

static patch_t *marknums[10]; // numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS]; // where the points are
static int markpointnum = 0; // next point to be assigned

static unsigned char cheat_amap_seq[] = { 0xb2, 0x26, 0x26, 0x2e, 0xff };
static cheatseq_t cheat_amap = { cheat_amap_seq, 0 };

static boolean stopped = true;

extern boolean viewactive;

// [crispy] automap rotate mode ...
// ... needs these early on
void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;


// [JN] Automap line antialiasing:
#define NUMALIAS 35                             // Number of antialiased lines

static byte antialias[NUMALIAS][8] = {
    // Doom colors:
    {176, 178, 180, 182, 184, 186, 188, 190},   // 1.  WALLCOLORS
    { 64,  66,  68,  70,  72,  74,  76,  78},   // 2.  FDWALLCOLORS
    { 96,  98, 100, 102, 104, 106, 108, 101},   // 3.  TSWALLCOLORS
    {231, 160, 161, 162, 163, 164, 165, 166},   // 4.  CDWALLCOLORS
    {176, 178, 180, 182, 184, 186, 188, 190},   // 5.  SECRETWALLCOLORS
    { 99, 101, 103, 105, 107, 109, 110, 111},   // 6.  GRAYS+3 (LINE_NEVERSEE)
    // Boom colors:
    { 23,  24,  25,  26,  27,  28,  29,  30},   // 7.  One-sided wall
    {119, 120, 121, 122, 123, 124, 125, 126},   // 8.  Various teleporters
    {204, 204, 205, 205, 206, 206, 207, 207},   // 9.  BLUE locked doors
    {175, 176, 177, 178, 179, 180, 181, 182},   // 10. RED locked doors
    {208,  80,  81,  82,  83,  84,  85,  86},   // 11. non-secret closed door
    { 55,  56,  57,  58,  59,  60,  61,  62},   // 12. floor level change
    {215, 216, 217, 218, 219, 220, 221, 222},   // 13. ceiling level change
    { 88,  89,  90,  91,  92,  93,  94,  95},   // 14. 2S lines that appear only in IDDT
    // Jaguar colors:
    { 32,  34,  36,  38,  40,  42,  44,  46},   // 15. RED_JAGUAR
    {120, 121, 122, 123, 124, 125, 126, 127},   // 16. GREEN_JAGUAR
    {254, 254, 254, 254, 254, 254, 254, 254},   // 17. MAGENTA_JAGUAR
    {163, 163, 164, 164, 165, 165, 166, 167},   // 18. YELLOW_JAGUAR
    // Raven colors:
    {151, 151, 236, 236, 237, 237, 238, 239},   // 19. One-sided wall
    {116, 118, 120, 122, 124, 125, 126, 127},   // 20. Various teleporters
    {108, 109, 110, 111,   5,   6,   7,   8},   // 21. Secret door
    {199, 200, 201, 202, 203, 204, 205, 206},   // 22. BLUE locked doors
    {178, 180, 182, 184, 186, 188, 190, 191},   // 23. RED locked doors
    {161, 162, 163, 164, 165, 166, 167, 167},   // 24. YELLOW locked doors
    {236, 236, 237, 237, 238, 238, 239, 239},   // 25. floor level change
    {133, 135, 137, 139, 140, 142, 144, 146},   // 26. ceiling level change
    // Strife colors:
    { 86,  88,  90,  92,  94,  96,  98, 100},   // 27. One-sided wall
    {135, 137, 139, 141, 143, 145, 147, 149},   // 28. Various teleporters
    {203, 204, 205, 206, 207, 240, 241, 242},   // 29. floor level change
    {195, 196, 197, 198, 199, 200, 201, 202},   // 30. ceiling level change
    { 98, 100, 102, 104, 106, 108, 110, 111},   // 31. One-sided wall (cheating)
    {102, 103, 104, 105, 106, 107, 108, 109},   // 32. One-sided wall (LINE_NEVERSEE)
    // Common colors:
    {209,  86,  88,  90,  92,  94,  96,  98},   // 33. YOURCOLORS: White player arrow
    {112, 114, 116, 118, 120, 122, 124, 126},   // 34. THINGCOLORS: Green triangles (IDDT cheat)
    {104, 105, 106, 107, 108, 109, 110, 111}    // 35. GRIDCOLORS: Automap grid
};

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits);

void V_MarkRect (int x, int y, int width, int height);


//
//
//
void AM_activateNewScale(void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}


//
//
//
void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}


//
//
//
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
        m_x = plr->mo->x - m_w/2;
        m_y = plr->mo->y - m_h/2;
    }

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}


//
// adds a marker at the current location
//
void AM_addMark(void)
{
    // [crispy] keep the map static in overlay mode
    // if not following the player
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
    markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
}


//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
void AM_findMinMaxBoundaries(void)
{
    int     i;
    fixed_t a;
    fixed_t b;

    min_x = min_y =  MAXINT;
    max_x = max_y = -MAXINT;

    for (i=0;i<numvertexes;i++)
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

    min_w = 2*PLAYERRADIUS; // const? never changed?
    min_h = 2*PLAYERRADIUS;

    a = FixedDiv(f_w<<FRACBITS, max_w);
    b = FixedDiv(f_h<<FRACBITS, max_h);

    min_scale_mtof = a < b ? a/2 : b/2;
    max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*PLAYERRADIUS);
}


//
//
//
void AM_changeWindowLoc(void)
{
    int64_t incx, incy;

    if (m_paninc.x || m_paninc.y)
    {
        automap_follow = 0;
        f_oldloc.x = MAXINT;
    }

    incx = m_paninc.x;
    incy = m_paninc.y;
    if (automap_rotate)
    {
        AM_rotate(&incx, &incy, -mapangle);
    }
    m_x += incx;
    m_y += incy;

    if (m_x + m_w/2 > max_x)
    m_x = max_x - m_w/2;
    else if (m_x + m_w/2 < min_x)
    m_x = min_x - m_w/2;

    if (m_y + m_h/2 > max_y)
    m_y = max_y - m_h/2;
    else if (m_y + m_h/2 < min_y)
    m_y = min_y - m_h/2;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}


//
//
//
void AM_initVariables(void)
{
    int     pnum;
    static  event_t st_notify = { ev_keyup, AM_MSGENTERED };

    automapactive = true;
    fb = screens[0];

    f_oldloc.x = MAXINT;
    amclock = 0;
    lightlev = 0;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (!playeringame[pnum = consoleplayer])
    for (pnum=0;pnum<MAXPLAYERS;pnum++)
        if (playeringame[pnum])
        break;

    plr = &players[pnum];
    m_x = plr->mo->x - m_w/2;
    m_y = plr->mo->y - m_h/2;
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // inform the status bar of the change
    ST_Responder(&st_notify);
}


//
// 
//
void AM_loadPics(void)
{
    int i;
    char namebuf[9];

    for (i=0;i<10;i++)
    {
        sprintf(namebuf, "AMMNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }
}

void AM_unloadPics(void)
{
    int i;

    for (i=0;i<10;i++)
    Z_ChangeTag(marknums[i], PU_CACHE);
}

void AM_clearMarks(void)
{
    int i;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
    markpoints[i].x = -1; // means empty
    markpointnum = 0;
}


//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit(void)
{
    fixed_t a, b;
    leveljuststarted = 0;

    f_x = f_y = 0;
    f_w = SCREENWIDTH;
    f_h = finit_height;

    AM_clearMarks();

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


//
//
//
void AM_Stop (void)
{
    static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED };

    AM_unloadPics();
    automapactive = false;
    ST_Responder(&st_notify);
    stopped = true;
}


//
//
//
void AM_Start (void)
{
    static int  lastlevel = -1, lastepisode = -1;

    if (!stopped) AM_Stop();
    stopped = false;

    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit();
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }
    
    AM_initVariables();
    AM_loadPics();
}


//
// set the window scale to the maximum size
//
void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}


//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}


//
// Handle events (user inputs) in automap mode
//
boolean AM_Responder (event_t* ev)
{
    int         rc;
    static int  cheatstate=0;
    static int  bigstate=0;
    static char buffer[20];

    rc = false;

    if (!automapactive)
    {
        if (ev->type == ev_keydown && ev->data1 == AM_STARTKEY)
        {
            AM_Start ();
            viewactive = false;
            rc = true;
        }
    }

    else if (ev->type == ev_keydown)
    {
        rc = true;
        switch(ev->data1)
        {
            case AM_PANRIGHTKEY: // pan right
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow && !automap_overlay) m_paninc.x = FTOM(F_PANINC);
            else rc = false;
            break;

            case AM_PANLEFTKEY: // pan left
            if (!automap_follow && !automap_overlay) m_paninc.x = -FTOM(F_PANINC);
            else rc = false;
            break;

            case AM_PANUPKEY: // pan up
            if (!automap_follow && !automap_overlay) m_paninc.y = FTOM(F_PANINC);
            else rc = false;
            break;

            case AM_PANDOWNKEY: // pan down
            if (!automap_follow && !automap_overlay) m_paninc.y = -FTOM(F_PANINC);
            else rc = false;
            break;

            case AM_ZOOMOUTKEY: // zoom out
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
            break;

            case AM_ZOOMINKEY: // zoom in
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
            break;

            case AM_ENDKEY:
            bigstate = 0;
            viewactive = true;
            AM_Stop();
            break;

            case AM_GOBIGKEY:
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else AM_restoreScaleAndLoc();
            break;

            case AM_FOLLOWKEY:
            automap_follow = !automap_follow;
            f_oldloc.x = MAXINT;
            plr->message_system = automap_follow ? amstr_followon : amstr_followoff;
            break;
    
            case AM_GRIDKEY:
            automap_grid = !automap_grid;
            plr->message_system = automap_grid ? amstr_gridon : amstr_gridoff;
            break;

            case AM_MARKKEY:
            sprintf(buffer, "%s %d", amstr_markedspot, markpointnum);
            plr->message_system = buffer;
            AM_addMark();
            break;

            case AM_CLEARMARKKEY:
            AM_clearMarks();
            plr->message_system = amstr_markscleared;
            break;

            case AM_ROTATEKEY:
            automap_rotate ^= 1;
            plr->message_system = automap_rotate ? amstr_rotateon : amstr_rotateoff;
            break;

            default:
            cheatstate=0;
            rc = false;
        }

        if (!deathmatch && cht_CheckCheat(&cheat_amap, ev->data1))
        {
            rc = false;
            cheating = (cheating+1) % 3;
        }
    }

    else if (ev->type == ev_keyup)
    {
        rc = false;
        switch (ev->data1)
        {
            case AM_PANRIGHTKEY:
            if (!automap_follow) m_paninc.x = 0;
            break;

            case AM_PANLEFTKEY:
            if (!automap_follow) m_paninc.x = 0;
            break;

            case AM_PANUPKEY:
            if (!automap_follow) m_paninc.y = 0;
            break;

            case AM_PANDOWNKEY:
            if (!automap_follow) m_paninc.y = 0;
            break;

            case AM_ZOOMOUTKEY:
            case AM_ZOOMINKEY:
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
            break;
        }
    }

    return rc;
}


//
// Zooming
//
void AM_changeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    if (scale_mtof < min_scale_mtof)
    AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
    AM_maxOutWindowScale();
    else
    AM_activateNewScale();
}


//
//
//
void AM_doFollowPlayer(void)
{
    if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
        // [JN] Use interpolated player coords for smooth
        // scrolling and static player arrow position.
        m_x = plr->mo->x - m_w/2;
        m_y = plr->mo->y - m_h/2;
        m_x2 = m_x + m_w;
        m_y2 = m_y + m_h;
        f_oldloc.x = plr->mo->x;
        f_oldloc.y = plr->mo->y;
    }
}


//
// Updates on Game Tick
//
void AM_Ticker (void)
{
    if (!automapactive)
    return;

    amclock++;

    if (automap_follow)
    AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
    AM_changeWindowScale();

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
    AM_changeWindowLoc();

    // [crispy] required for AM_rotatePoint()
    if (automap_rotate)
    {
        mapcenter.x = m_x + m_w / 2;
        mapcenter.y = m_y + m_h / 2;
        // [crispy] keep the map static in overlay mode
        // if not following the player
        if (!(!automap_follow && automap_overlay))
        mapangle = ANG90 - plr->mo->angle;
    }
}


//
// Clear automap frame buffer.
//
void AM_clearFB(int color)
{
    memset(fb, color, f_w*f_h);
}


//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
boolean AM_clipMline (mline_t* ml, fline_t* fl)
{
    enum
    {
        LEFT   = 1,
        RIGHT  = 2,
        BOTTOM = 4,
        TOP    = 8
    };

    register int outcode1 = 0;
    register int outcode2 = 0;
    register int outside;

    fpoint_t    tmp;
    int         dx;
    int         dy;

#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;
    
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
    return false; // trivially outside

    if (ml->a.x < m_x)
    outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
    outcode1 |= RIGHT;

    if (ml->b.x < m_x)
    outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
    outcode2 |= RIGHT;

    if (outcode1 & outcode2)
    return false; // trivially outside

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
            tmp.x = fl->a.x + (dx*(fl->a.y))/dy;
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx*(fl->a.y-f_h))/dy;
            tmp.y = f_h-1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(f_w-1 - fl->a.x))/dx;
            tmp.x = f_w-1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(-fl->a.x))/dx;
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
            return false; // trivially outside
        }

    return true;
}
#undef DOOUTCODE


//
// Classic Bresenham w/ whatever optimizations needed for speed
//
#define DOT(xx,yy,cc) fb[(yy)*f_w+(xx)]=(cc) //the MACRO!

//
// AM_drawFline
// [JN] Draw non-antialiased lines
//
void AM_drawFline (fline_t* fl, int color)
{
    int x;
    int y;
    int dx;
    int dy;
    int sx;
    int sy;
    int ax;
    int ay;
    int d;

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
            if (x == fl->b.x) return;
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
            if (y == fl->b.y) return;
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


//
// AM_drawFlineAntialiased
// [JN] Draw antialiased lines
//
void AM_drawFlineAntialiased (fline_t* fl, int color)
{
    // Doom colors:
    if (automap_color == 0)
    {
        if (color == WALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[0][0]), 8, 3);
        if (color == FDWALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[1][0]), 8, 3);
        if (color == TSWALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[2][0]), 8, 3);
        if (color == CDWALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[3][0]), 8, 3);
        if (color == SECRETWALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[4][0]), 8, 3);
        if (color == GRAYS+3)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[5][0]), 8, 3);
    }
    // Boom colors:
    if (automap_color == 1)
    {
        if (color == 23)    // Wall color
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[6][0]), 8, 3);
        if (color == 119)   // Various teleporters
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[7][0]), 8, 3);
        if (color == 204)   // BLUE locked doors
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[8][0]), 8, 3);
        if (color == 175)   // RED locked doors
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[9][0]), 8, 3);
        if (color == 231)   // YELLOW locked doors (using CDWALLCOLORS)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[3][0]), 8, 3);
        if (color == 208)   // non-secret closed door
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[10][0]), 8, 3);
        if (color == 55)    // floor level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[11][0]), 8, 3);
        if (color == 215)   // ceiling level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[12][0]), 8, 3);
        if (color == 88)    // 2S lines that appear only in IDDT
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[13][0]), 8, 3);
    }
    // Jaguar colors:
    if (automap_color == 2)
    {
        if (color == 32)    // dark red
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[14][0]), 8, 3);
        if (color == 120)   // dark green
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[15][0]), 8, 3);
        if (color == 254)   // dark magenta
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[16][0]), 8, 3);
        if (color == 163)   // dark yellow
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[17][0]), 8, 3);
        if (color == TSWALLCOLORS)  // Hidden gray walls
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[2][0]), 8, 3);
        if (color == GRAYS+3)       // computermap visible lines
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[5][0]), 8, 3);
    }
    // Raven colors:
    if (automap_color == 3)
    {
        if (color == 151)   // One-sided wall
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[18][0]), 8, 3);
        if (color == 116)   // Various teleporters
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[19][0]), 8, 3);
        if (color == 108)   // Secret door
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[20][0]), 8, 3);
        if (color == 199)   // BLUE locked doors
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[21][0]), 8, 3);
        if (color == 178)   // RED locked doors
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[22][0]), 8, 3);
        if (color == 161)   // YELLOW locked doors
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[23][0]), 8, 3);
        if (color == 239)   // floor level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[24][0]), 8, 3);
        if (color == 133)   // ceiling level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[25][0]), 8, 3);
        if (color == 99)    // ceiling level change, cheating (using GRAYS+3 (LINE_NEVERSEE))
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[5][0]), 8, 3);
    }
    // Strife colors:
    if (automap_color == 4)
    {
        if (color == 119)   // Exit lines (using "Various teleporters")
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[7][0]), 8, 3);
        if (color == 86)    // One-sided wall
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[26][0]), 8, 3);
        if (color == 135)   // Various teleporters and secret door
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[27][0]), 8, 3);
        if (color == 203)   // floor level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[28][0]), 8, 3);
        if (color == 195)   // ceiling level change
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[29][0]), 8, 3);
        if (color == 98)    // One-sided wall (cheating)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[30][0]), 8, 3);
        if (color == 102)   // One-sided wall (LINE_NEVERSEE)
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[31][0]), 8, 3);
    }

    // Common colors:
    if (color == YOURCOLORS)    // White player arrow
        DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[32][0]), 8, 3);
    if (color == THINGCOLORS)   // Green triangles (IDDT cheat)
        DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[33][0]), 8, 3);
    if (color == 104)           // computermap visible lines
        DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[34][0]), 8, 3);
}


// [JN] Line antialiasing. Adapted from Heretic source code. (src/am_map.c)
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
        cc += 7 - (xx >> 2);
    else if (xx > (SCREENWIDTH - 32))
        cc += 7 - ((SCREENWIDTH - xx) >> 2);
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
        oldyyshifted += SCREENWIDTH;
    }
    else if (yy == oldyy - 1)
    {
        oldyy--;
        oldyyshifted -= SCREENWIDTH;
    }
    else if (yy != oldyy)
    {
        oldyy = yy;
        oldyyshifted = yy * SCREENWIDTH;
    }
    // [JN] TODO - for flipped levels
    // fb[oldyyshifted + flipwidth[xx]] = *(cc); 
    fb[oldyyshifted+xx] = *(cc);
}


void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits)
{
    unsigned short IntensityShift, ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
    short DeltaX, DeltaY, XDir;

    /* Make sure the line runs top to bottom */
    if (Y0 > Y1)
    {
        short Temp;

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


//
// Clip lines, draw visible part sof lines.
//
void AM_drawMline (mline_t* ml, int	color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // draws it on frame buffer using fb coords    
        if (automap_antialias && !vanilla)
        {
            AM_drawFlineAntialiased(&fl, color);
        }
        else
        {
            AM_drawFline(&fl, color);
        }
    }
}


//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
void AM_drawGrid (int color)
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
    start -= ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS));
    end = m_x + m_w;
    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x=start; x<end; x+=(MAPBLOCKUNITS<<FRACBITS))
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
    if ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS))
    start -= ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS));
    end = m_y + m_h;
    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y=start; y<end; y+=(MAPBLOCKUNITS<<FRACBITS))
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


//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
void AM_drawWalls (void)
{
    int     i;
    static  mline_t l;

    for (i=0;i<numlines;i++)
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
            AM_drawMline(&l, WALLCOLORS+lightlev);
            }
            else
            {
                if (lines[i].special == 39)
                { // teleporters
                    AM_drawMline(&l, WALLCOLORS+WALLRANGE/2);
                }
                else if (lines[i].flags & ML_SECRET) // secret door
                {
                    if (cheating) AM_drawMline(&l, SECRETWALLCOLORS + lightlev);
                    else AM_drawMline(&l, WALLCOLORS+lightlev);
                }
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS + lightlev); // floor level change
                }
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS+lightlev); // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS+lightlev);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS+3);
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] Automap colors: Boom
// -----------------------------------------------------------------------------
void AM_drawWallsBoom(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
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
                AM_drawMline(&l, 23);
            }
            else
            {
                // [JN] Various teleporters
                if (lines[i].special == 39
                ||  lines[i].special == 97
                ||  lines[i].special == 125
                ||  lines[i].special == 126)
                {
                    AM_drawMline(&l, 119);
                }
                // [JN] Secret door
                else if (lines[i].flags & ML_SECRET)
                {
                    AM_drawMline(&l, 23);      // wall color
                }

                // [JN] BLUE locked doors
                else if (lines[i].special == 26 || lines[i].special == 32
                     ||  lines[i].special == 99 || lines[i].special == 133)
                {
                    AM_drawMline(&l, 204);
                }
                // [JN] RED locked doors
                else if (lines[i].special == 28 || lines[i].special == 33
                     ||  lines[i].special == 134 || lines[i].special == 135)
                {
                    AM_drawMline(&l, 175);
                }
                // [JN] YELLOW locked doors
                else if (lines[i].special == 27 || lines[i].special == 34
                     ||  lines[i].special == 136 || lines[i].special == 137)
                {
                    AM_drawMline(&l, 231);
                }

                // non-secret closed door
                else if
                (!(lines[i].flags & ML_SECRET) &&
                ((lines[i].backsector->floorheight==lines[i].backsector->ceilingheight) ||
                (lines[i].frontsector->floorheight==lines[i].frontsector->ceilingheight)))
                {
                    AM_drawMline(&l, 208);      // non-secret closed door
                } //jff 1/6/98 show secret sector 2S lines
                // floor level change
                else if (lines[i].backsector->floorheight !=
                         lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, 55);
                }
                // ceiling level change
                else if (lines[i].backsector->ceilingheight !=
                         lines[i].frontsector->ceilingheight)
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
            if (!(lines[i].flags & ML_DONTDRAW)) // invisible flag lines do not show
            {
                if
                (
                !lines[i].backsector
                ||
                lines[i].backsector->floorheight
                != lines[i].frontsector->floorheight
                ||
                lines[i].backsector->ceilingheight
                != lines[i].frontsector->ceilingheight
                )
                    AM_drawMline(&l, 104);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] Atari Jaguar: draw walls using Jaguar colors.
// Adapted from original Jaguar Doom source, AM_Drawer.
// -----------------------------------------------------------------------------
void AM_drawWallsJaguar(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
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
                AM_drawMline(&l, 32);
            }
            else
            {
                // Teleport line
                if (lines[i].special == 39 || lines[i].special == 97)
                AM_drawMline(&l, 120);

                // Secret door
                else if (lines[i].flags & ML_SECRET)
                AM_drawMline(&l, 32);

                // Any special linedef
                else if (lines[i].special)
                AM_drawMline(&l, 254);

                // Floor level change
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                AM_drawMline(&l, 163);

                // Ceiling level change
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                AM_drawMline(&l, 163);

                // Hidden gray walls
                else if (cheating)
                AM_drawMline(&l, TSWALLCOLORS);

            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS+3);
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] Automap colors: Raven (Heretic)
// -----------------------------------------------------------------------------
void AM_drawWallsRaven(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
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
                AM_drawMline(&l, 151+lightlev);
            }
            else
            {
                // [JN] Various teleporters
                if (lines[i].special == 39
                ||  lines[i].special == 97
                ||  lines[i].special == 125
                ||  lines[i].special == 126)
                {
                    AM_drawMline(&l, 116);
                }
                // [JN] Secret door
                else if (lines[i].flags & ML_SECRET)
                {
                    if (cheating) 
                        AM_drawMline(&l, 0);
                    else 
                        AM_drawMline(&l, 108+lightlev);
                }
                // [JN] BLUE locked doors
                else if (lines[i].special == 26 || lines[i].special == 32
                     ||  lines[i].special == 99 || lines[i].special == 133)
                {
                    AM_drawMline(&l, 199);
                }
                // [JN] RED locked doors
                else if (lines[i].special == 28 || lines[i].special == 33
                     ||  lines[i].special == 134 || lines[i].special == 135)
                {
                    AM_drawMline(&l, 178);
                }
                // [JN] YELLOW locked doors
                else if (lines[i].special == 27 || lines[i].special == 34
                     ||  lines[i].special == 136 || lines[i].special == 137)
                {
                    AM_drawMline(&l, 161);
                }
                // [JN] floor level change
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                {
                    AM_drawMline(&l, 239 + lightlev);
                }
                // [JN] ceiling level change
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                {
                    AM_drawMline(&l, 133+lightlev);
                }
                else if (cheating)
                {
                    AM_drawMline(&l, 99+lightlev);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) 
                AM_drawMline(&l, 99);
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] Automap colors: Strife
// -----------------------------------------------------------------------------
void AM_drawWallsStrife(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
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

            // villsa [STRIFE]
            // [JN] Changed to Doom exit lines
            if (lines[i].special == 11
            ||  lines[i].special == 51
            ||  lines[i].special == 52
            ||  lines[i].special == 124)
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
                if (lines[i].special == 39
                ||  lines[i].special == 97
                ||  lines[i].special == 125
                ||  lines[i].special == 126)
                {
                    AM_drawMline(&l, 135);
                }
                else if (lines[i].flags & ML_SECRET) // secret door
                {
                    // villsa [STRIFE] just draw the wall as is!
                    AM_drawMline(&l, 86);
                }
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                {
                    AM_drawMline(&l, 203); // floor level change
                }
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                {
                    AM_drawMline(&l, 195); // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, 98);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) 
                AM_drawMline(&l, 102);
        }
    }
}

//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void AM_rotate (int64_t* x, int64_t* y, angle_t a)
{
    int64_t tmpx;

    tmpx = 	FixedMul(*x,finecosine[a>>ANGLETOFINESHIFT]) - FixedMul(*y,finesine[a>>ANGLETOFINESHIFT]);

    *y = FixedMul(*x,finesine[a>>ANGLETOFINESHIFT]) + FixedMul(*y,finecosine[a>>ANGLETOFINESHIFT]);
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

void AM_drawLineCharacter
( mline_t*  lineguy,
  int       lineguylines,
  fixed_t   scale,
  angle_t   angle,
  int       color,
  fixed_t   x,
  fixed_t   y )
{
    int     i;
    mline_t l;

    if (automap_rotate)
    {
        angle += mapangle;
    }

    for (i=0;i<lineguylines;i++)
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
    int         i;
    player_t*   p;
    static int  their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
    int         their_color = -1;
    int         color;
    mpoint_t    pt;

    if (!netgame)
    {
        pt.x = plr->mo->x;
        pt.y = plr->mo->y;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        if (cheating)
        AM_drawLineCharacter (cheat_player_arrow, NUMCHEATPLYRLINES, 0, plr->mo->angle, WHITE, pt.x, pt.y);
        else
        AM_drawLineCharacter (player_arrow, NUMPLYRLINES, 0, plr->mo->angle, WHITE, pt.x, pt.y);

        return;
    }

    for (i=0;i<MAXPLAYERS;i++)
    {
        their_color++;
        p = &players[i];

        if ( (deathmatch && !singledemo) && p != plr)
        continue;

        if (!playeringame[i])
        continue;

        if (p->powers[pw_invisibility])
        color = 246; // *close* to black

        else
        color = their_colors[their_color];

        pt.x = p->mo->x;
        pt.y = p->mo->y;
        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter (player_arrow, NUMPLYRLINES, 0, p->mo->angle, color, pt.x, pt.y);
    }
}

void AM_drawThings (int colors, int colorrange)
{
    int     i;
    mobj_t* t;
    mpoint_t	pt;

    for (i=0;i<numsectors;i++)
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

            AM_drawLineCharacter (thintriangle_guy, NUMTHINTRIANGLEGUYLINES, 16<<FRACBITS, t->angle, colors+lightlev, pt.x, pt.y);
            t = t->snext;
        }
    }
}

void AM_drawMarks(void)
{
    int i, fx, fy, w, h;
    mpoint_t pt;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
    {
        if (markpoints[i].x != -1)
        {
            //      w = SHORT(marknums[i]->width);
            //      h = SHORT(marknums[i]->height);
            w = 5; // because something's wrong with the wad, i guess
            h = 6; // because something's wrong with the wad, i guess
            // [crispy] center marks around player
            pt.x = markpoints[i].x;
            pt.y = markpoints[i].y;
            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }
            fx = (CXMTOF(pt.x)) - 1;
            fy = (CYMTOF(pt.y)) - 2;
            if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
            V_DrawPatch(fx, fy, 0, marknums[i]);
        }
    }
}

void AM_drawCrosshair(int color)
{
    fb[(f_w*(f_h+1))/2] = color; // single point for now
}

void AM_Drawer (void)
{
    if (!automapactive)
    return;

    // [JN] Don't draw Automap while in help screens.
    if (inhelpscreens)
    return;

    AM_clearFB(BACKGROUND);

    if (automap_grid)
	AM_drawGrid(GRIDCOLORS);

    if (automap_color == 0 || vanilla)  // Doom
    AM_drawWalls();
    else if (automap_color == 1)        // Boom
    AM_drawWallsBoom();
    else if (automap_color == 2)        // Jaguar
    AM_drawWallsJaguar();
    else if (automap_color == 3)        // Raven (Heretic)
    AM_drawWallsRaven();
    else                                // Strife
    AM_drawWallsStrife();

    AM_drawPlayers();

    if (cheating==2)
	AM_drawThings(THINGCOLORS, THINGRANGE);

    // [JN] Do not draw in following mode.
    if (!automap_follow)
    AM_drawCrosshair(XHAIRCOLORS);

    AM_drawMarks();

    V_MarkRect(f_x, f_y, f_w, f_h);
}


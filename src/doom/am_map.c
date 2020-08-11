//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2020 Julian Nechaevsky
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


#include <stdio.h>

#include "deh_main.h"
#include "z_zone.h"
#include "doomkeys.h"
#include "doomdef.h"
#include "st_stuff.h"
#include "p_local.h"
#include "w_wad.h"
#include "m_cheat.h"
#include "m_controls.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_timer.h"
#include "v_trans.h"
#include "v_video.h"
#include "doomstat.h"
#include "r_state.h"
#include "rd_lang.h"
#include "am_map.h"
#include "st_stuff.h"
#include "jn.h"


// For use if I do walls with outsides/insides
#define REDS             176
#define REDRANGE         16
#define BLUES            200
#define BLUERANGE        8
#define GREENS           112
#define GREENRANGE       16
#define GRAYS            96
#define GRAYSRANGE       16
#define BROWNS           64
#define BROWNRANGE       16
#define YELLOWS          231
#define YELLOWRANGE      1
#define BLACK            0
#define WHITE            209

// Automap colors
#define BACKGROUND       BLACK
#define YOURCOLORS       WHITE
#define YOURRANGE        0
#define WALLCOLORS       REDS
#define WALLRANGE        REDRANGE
#define TSWALLCOLORS     GRAYS
#define TSWALLRANGE      GRAYSRANGE
#define FDWALLCOLORS     BROWNS
#define FDWALLRANGE      BROWNRANGE
#define CDWALLCOLORS     YELLOWS
#define CDWALLRANGE      YELLOWRANGE
#define THINGCOLORS      GREENS
#define THINGRANGE       GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE  WALLRANGE
#define GRIDCOLORS       104
#define GRIDRANGE        0
#define XHAIRCOLORS      GRAYS

// The MACRO!
#define DOT(xx,yy,cc) fb[(yy)*f_w+(flipwidth[xx])]=(cc)

// [JN] Jaguar Doom automap colors
#define RED_JAGUAR       32
#define GREEN_JAGUAR     120
#define YELLOW_JAGUAR    163
#define MAGENTA_JAGUAR   254

// drawing stuff
#define AM_NUMMARKPOINTS 10

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)

// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC 4

// how much zoom-in per tic goes to 2x in 1 second
#define M_ZOOMIN ((int) (1.02*FRACUNIT))

// how much zoom-out per tic pulls out to 0.5x in 1 second
#define M_ZOOMOUT ((int) (FRACUNIT/1.02))

// translates between frame-buffer and map distances
#define FTOM(x) (((int64_t)((x)<<16) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>16)

// translates between frame-buffer and map coordinates
#define CXMTOF(x) (f_x + MTOF((x)-m_x))
#define CYMTOF(y) (f_y + (f_h - MTOF((y)-m_y)))

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

typedef struct
{
    fixed_t slp, islp;
} islope_t;


//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] = {
    { { -R+R/8,   0 }, {  R,      0   } }, // -----
    { {  R,       0 }, {  R-R/2,  R/4 } }, // ----->
    { {  R,       0 }, {  R-R/2, -R/4 } },
    { { -R+R/8,   0 }, { -R-R/8,  R/4 } }, // >---->
    { { -R+R/8,   0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8,  R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
    { { -R+R/8,     0        }, {  R,           0 } }, // -----
    { {  R,         0        }, {  R-R/2,     R/6 } }, // ----->
    { {  R,         0        }, {  R-R/2,    -R/6 } },
    { { -R+R/8,     0        }, { -R-R/8,     R/6 } }, // >----->
    { { -R+R/8,     0        }, { -R-R/8,    -R/6 } },
    { { -R+3*R/8,   0        }, { -R+R/8,     R/6 } }, // >>----->
    { { -R+3*R/8,   0        }, { -R+R/8,    -R/6 } },
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
mline_t triangle_guy[] = {
    { { (fixed_t)(-.867*R), (fixed_t)(-.5*R) }, { (fixed_t)(.867*R ), (fixed_t)(-.5*R) } },
    { { (fixed_t)(.867*R ), (fixed_t)(-.5*R) }, { (fixed_t)(0      ), (fixed_t)(R    ) } },
    { { (fixed_t)(0      ), (fixed_t)(R    ) }, { (fixed_t)(-.867*R), (fixed_t)(-.5*R) } }
};
#undef R

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
    { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
    { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
    { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R


static int cheating = 0;
static int leveljuststarted = 1; // kluge until AM_LevelInit() is called

boolean automapactive = false;

// location of window on screen
static int f_x;
static int f_y;

// size of window on screen
static int f_w;
static int f_h;

static byte* fb;     // pseudo-frame buffer
static int amclock;

static mpoint_t m_paninc;     // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)

static int64_t m_x, m_y;   // LL x,y where the window is on the map (map coords)
static int64_t m_x2, m_y2; // UR x,y where the window is on the map (map coords)


//
// width/height of window on map (map coords)
//
static int64_t m_w;
static int64_t m_h;

// based on level size
static fixed_t min_x;
static fixed_t min_y; 
static fixed_t max_x;
static fixed_t max_y;

static fixed_t max_w;
static fixed_t max_h;

// based on player size
static fixed_t min_w;
static fixed_t min_h;

static fixed_t min_scale_mtof; // used to tell when to stop zooming out
static fixed_t max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr; // the player represented by an arrow

static patch_t *marknums[10]; // numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS]; // where the points are
static int markpointnum = 0; // next point to be assigned


cheatseq_t cheat_amap = CHEAT("iddt", 0);
cheatseq_t cheat_amap_beta = CHEAT("eek", 0);   // [JN] Press Beta cheat code

static boolean stopped = true;



// [JN] Automap line antialiasing:
static byte antialias[42][8] = {
    // Doom colors:
    {176, 177, 178, 179, 180, 181, 182, 183},   //  0.  WALLCOLORS
    { 64,  65,  66,  67,  68,  69,  70,  71},   //  1.  FDWALLCOLORS
    { 96,  97,  98,  99, 100, 101, 102, 103},   //  2.  TSWALLCOLORS
    {231, 160, 161, 162, 163, 164, 165, 166},   //  3.  CDWALLCOLORS
    {176, 177, 178, 179, 180, 181, 182, 183},   //  4.  SECRETWALLCOLORS
    { 99, 100, 101, 102, 103, 104, 105, 106},   //  5.  GRAYS+3 (LINE_NEVERSEE)
    // Boom colors:
    { 23,  24,  25,  26,  27,  28,  29,  30},   //  6.  One-sided wall
    {119, 120, 121, 122, 123, 124, 125, 126},   //  7.  Various teleporters
    {204, 204, 205, 205, 206, 206, 207, 207},   //  8.  BLUE locked doors
    {175, 176, 177, 178, 179, 180, 181, 182},   //  9. RED locked doors
    {208,  80,  81,  82,  83,  84,  85,  86},   // 10. non-secret closed door
    { 55,  56,  57,  58,  59,  60,  61,  62},   // 11. floor level change
    {215, 216, 217, 218, 219, 220, 221, 222},   // 12. ceiling level change
    { 88,  89,  90,  91,  92,  93,  94,  95},   // 13. 2S lines that appear only in IDDT
    // Jaguar colors:
    { 32,  33,  34,  35,  36,  37,  38,  39},   // 14. RED_JAGUAR
    {120, 121, 122, 123, 124, 125, 126, 127},   // 15. GREEN_JAGUAR
    {254, 254, 254, 254, 254, 254, 254, 254},   // 16. MAGENTA_JAGUAR
    {163, 163, 164, 164, 165, 165, 166, 167},   // 17. YELLOW_JAGUAR
    // Raven colors:
    {151, 151, 236, 236, 237, 237, 238, 239},   // 18. One-sided wall
    {116, 117, 118, 119, 120, 121, 122, 123},   // 19. Various teleporters
    {108, 109, 110, 111,   5,   6,   7,   8},   // 20. Secret door
    {199, 200, 201, 202, 203, 204, 205, 206},   // 21. BLUE locked doors
    {178, 179, 180, 181, 182, 183, 184, 185},   // 22. RED locked doors
    {161, 162, 163, 164, 165, 166, 167, 167},   // 23. YELLOW locked doors
    {239, 239, 239, 239, 239, 239, 239, 239},   // 24. floor level change
    {133, 134, 135, 136, 137, 138, 139, 140},   // 25. ceiling level change
    // Strife colors:
    { 86,  87,  88,  89,  90,  91,  92,  93},   // 26. One-sided wall
    {135, 136, 137, 138, 139, 140, 141, 142},   // 27. Various teleporters
    {203, 204, 205, 206, 207, 240, 241, 242},   // 28. floor level change
    {195, 196, 197, 198, 199, 200, 201, 202},   // 29. ceiling level change
    { 98,  99, 100, 101, 102, 103, 104, 105},   // 30. One-sided wall (cheating)
    {102, 103, 104, 105, 106, 107, 108, 109},   // 31. One-sided wall (LINE_NEVERSEE)
    // Unity colors:
    {184, 185, 186, 187, 188, 189, 190, 191},   // 32. One sided wall / secret door
    { 81,  83,  85,  87,  88,  90,  92,  94},   // 33. Various doors
    {120, 121, 122, 123, 124, 125, 126, 127},   // 34. Various teleporters
    {200, 201, 202, 203, 204, 205, 206, 207},   // 35. Blue locked door
    {160, 160, 161, 161, 162, 162, 163, 164},   // 36. Yellow locked door
    { 72,  73,  74,  75,  76,  77,  78,  78},   // 37. Floor level change
    {252, 252, 253, 253, 254, 254, 254, 254},   // 38. Floor level change
    // Common colors:
    {209,  84,  85,  86,  87,  88,  89,  90},   // 39. YOURCOLORS: White player arrow
    {112, 113, 114, 115, 116, 117, 118, 119},   // 40. THINGCOLORS: Green triangles (IDDT cheat)
    {104, 105, 106, 107, 108, 109, 110, 111},   // 41. GRIDCOLORS: Automap grid
};

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits);

// [crispy] automap rotate mode ...
// ... needs these early on
void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;


// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.
void AM_getIslope (mline_t *ml, islope_t *is)
{
    int dx, dy;

    dy = ml->a.y - ml->b.y;
    dx = ml->b.x - ml->a.x;

    if (!dy)
    {
        is->islp = (dx<0?-INT_MAX:INT_MAX);
    }
    else
    {
        is->islp = FixedDiv(dx, dy);
    }

    if (!dx)
    {    
        is->slp = (dy<0?-INT_MAX:INT_MAX);
    }
    else
    {
        is->slp = FixedDiv(dy, dx);
    }
}


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
    max_w = max_x/2 - min_x/2;
    max_h = max_y/2 - min_y/2;

    min_w = 2*PLAYERRADIUS; // const? never changed?
    min_h = 2*PLAYERRADIUS;

    a = FixedDiv(f_w<<FRACBITS, max_w);
    b = FixedDiv(f_h<<FRACBITS, max_h);

    min_scale_mtof = a < b ? a/2 : b/2;
    max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*PLAYERRADIUS);
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


void AM_initVariables(void)
{
    int pnum;
    static event_t st_notify = { ev_keyup, AM_MSGENTERED, 0, 0 };

    automapactive = true;
    fb = I_VideoBuffer;

    f_oldloc.x = INT_MAX;
    amclock = 0;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (playeringame[consoleplayer])
    {
        plr = &players[consoleplayer];
    }
    else
    {
        plr = &players[0];

        for (pnum=0;pnum<MAXPLAYERS;pnum++)
        {
            if (playeringame[pnum])
            {
                plr = &players[pnum];
                break;
            }
        }
    }

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


void AM_loadPics(void)
{
    int i;
    char namebuf[9];

    for (i = 0 ; i < 10 ; i++)
    {
        DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }
}


void AM_unloadPics(void)
{
    int i;
    char namebuf[9];

    for (i = 0 ; i < 10 ; i++)
    {
        DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
        W_ReleaseLumpName(namebuf);
    }
}


void AM_clearMarks(void)
{
    int i;

    for (i = 0 ; i < AM_NUMMARKPOINTS ; i++)
    {
        markpoints[i].x = -1; // means empty
    }
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
    f_w = screenwidth;
    
    f_h = SCREENHEIGHT - ((gamemission == jaguar ? ST_HEIGHT_JAG : ST_HEIGHT) << hires);

    AM_clearMarks();

    AM_findMinMaxBoundaries();
    // [crispy] initialize zoomlevel on all maps so that a 4096 units
    // square map would just fit in (MAP01 is 3376x3648 units)
    a = FixedDiv(f_w, (max_w>>FRACBITS < 2048) ? 2*(max_w>>FRACBITS) : 4096);
    b = FixedDiv(f_h, (max_h>>FRACBITS < 2048) ? 2*(max_h>>FRACBITS) : 4096);
    scale_mtof = FixedDiv(a < b ? a : b, (int) (0.7*FRACUNIT));

    if (scale_mtof > max_scale_mtof)
    {
        scale_mtof = min_scale_mtof;
    }

    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}


void AM_Stop (void)
{
    static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED, 0 };

    AM_unloadPics();
    automapactive = false;
    ST_Responder(&st_notify);
    stopped = true;

    // [JN] Press Beta: immediately update all widgets and background.
    // Needed for correct redwaring of arms/artifacts section.
    if (gamemode == pressbeta)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}


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
boolean AM_Responder (event_t *ev)
{
    int rc;
    static int bigstate=0;
    static int joywait = 0;
    static char buffer[20];
    int key;

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

        return true;
    }

    if (!automapactive)
    {
        if (ev->type == ev_keydown && ev->data1 == key_map_toggle)
        {
            AM_Start ();
            viewactive = false;
            rc = true;
        }
    }
    else if (ev->type == ev_keydown)
    {
        rc = true;
        key = ev->data1;

        if (key == key_map_east)        // pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? -FTOM(F_PANINC): FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_west)   // pan left
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.x = flip_levels ? FTOM(F_PANINC): -FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_north)  // pan up
        {
            if (!automap_follow && !automap_overlay)
            {
                m_paninc.y = FTOM(F_PANINC);
            }
            else
            {
                rc = false;
            }
        }
        else if (key == key_map_south)  // pan down
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
        else if (key == key_map_zoomout)  // zoom out
        {
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
        }
        else if (key == key_map_zoomin)   // zoom in
        {
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
        }
        else if (key == key_map_toggle)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else AM_restoreScaleAndLoc();
        }
        else if (key == key_map_follow)
        {
            automap_follow = !automap_follow;
            f_oldloc.x = INT_MAX;
            if (automap_follow)
            {
                plr->message_system = DEH_String(amstr_followon);
            }
            else
            {
                plr->message_system = DEH_String(amstr_followoff);
            }
        }
        else if (key == key_map_grid)
        {
            automap_grid = !automap_grid;
            if (automap_grid)
            {
                plr->message_system = DEH_String(amstr_gridon);
            }
            else
            {
                plr->message_system = DEH_String(amstr_gridoff);
            }
        }
        else if (key == key_map_mark)
        {
            M_snprintf(buffer, sizeof(buffer), "%s %d",
                    DEH_String(amstr_markedspot), markpointnum);
            plr->message_system = buffer;
            AM_addMark();
        }
        else if (key == key_map_clearmark)
        {
            AM_clearMarks();
            plr->message_system = DEH_String(amstr_markscleared);
        }
        else if (key == key_map_overlay)
        {
            // [crispy] force redraw status bar
            extern boolean inhelpscreens;
            inhelpscreens = true;
            
            automap_overlay = !automap_overlay;
            if (automap_overlay)
            {
                plr->message_system = DEH_String(amstr_overlayon);
            }
            else
            {
                plr->message_system = DEH_String(amstr_overlayoff);
            }
        }
        else if (key == key_map_rotate)
        {
            automap_rotate = !automap_rotate;
            if (automap_rotate)
            {
                plr->message_system = DEH_String(amstr_rotateon);
            }
            else
            {
                plr->message_system = DEH_String(amstr_rotateoff);
            }
        }
        else
        {
            rc = false;
        }

        if ((!deathmatch || gameversion <= exe_doom_1_8) && gamemode != pressbeta
        && cht_CheckCheat(&cheat_amap, ev->data2))
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }

        // [JN] Press Beta: 'EEK' instead of 'IDDT'
        if (gamemode == pressbeta && cht_CheckCheat(&cheat_amap_beta, ev->data2))
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;
        key = ev->data1;

        if (key == key_map_east)
        {
            if (!automap_follow) m_paninc.x = 0;
        }
        else if (key == key_map_west)
        {
            if (!automap_follow) m_paninc.x = 0;
        }
        else if (key == key_map_north)
        {
            if (!automap_follow) m_paninc.y = 0;
        }
        else if (key == key_map_south)
        {
            if (!automap_follow) m_paninc.y = 0;
        }
        else if (key == key_map_zoomout || key == key_map_zoomin)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
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
        m_x = FTOM(MTOF(plr->mo->x)) - m_w/2;
        m_y = FTOM(MTOF(plr->mo->y)) - m_h/2;
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
    {
        return;
    }

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
        {
            mapangle = ANG90 - plr->mo->angle;
        }
    }
}


//
// Clear automap frame buffer.
//
void AM_clearFB(int color)
{
    memset(fb, color, f_w*f_h*sizeof(*fb));
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


//
// Classic Bresenham w/ whatever optimizations needed for speed
//
void AM_drawFline (fline_t* fl, int color)
{
    int x, y, dx, dy, sx, sy, ax, ay, d;

    // [JN] Apply line antialiasing
    if (automap_antialias && !vanillaparm)
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
        if (automap_color == 2 || gamemission == jaguar)
        {
            if (color == RED_JAGUAR)
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[14][0]), 8, 3);
            if (color == GREEN_JAGUAR)
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[15][0]), 8, 3);
            if (color == MAGENTA_JAGUAR)
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[16][0]), 8, 3);
            if (color == YELLOW_JAGUAR)
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
        // Unity colors:
        if (automap_color == 5)
        {
            // [JN] One sided wall / secret door
            if (color == 184)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[32][0]), 8, 3);
            }
            // [JN] Various doors
            if (color == 81)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[33][0]), 8, 3);
            }
            // [JN] Various teleporters
            if (color == 120)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[34][0]), 8, 3);
            }
            // [JN] BLUE locked doors
            if (color == 200)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[35][0]), 8, 3);
            }
            // [JN] RED locked doors
            if (color == 176)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[0][0]), 8, 3);
            }
            // [JN] YELLOW locked doors
            if (color == 160)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[36][0]), 8, 3);
            }
            // [JN] Floor level change
            if (color == 72)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[37][0]), 8, 3);
            }
            // [JN] Ceiling level change
            if (color == 64)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[1][0]), 8, 3);
            }
            // [JN] IDDT visible lines
            if (color == 96)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[2][0]), 8, 3);
            }
            // [JN] Exit
            if (color == 252)
            {
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[38][0]), 8, 3);
            }
        }
        // Common colors:
        {
            if (color == YOURCOLORS)    // White player arrow
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[39][0]), 8, 3);
            if (color == THINGCOLORS)   // Green triangles (IDDT cheat)
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[40][0]), 8, 3);
            if (color == 104)           // computermap visible lines
                DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, (&antialias[41][0]), 8, 3);

        }
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
    static int finit_height; // [JN] For different height between games.
    byte *oldcc = cc;

    if (gamemission == jaguar)
    finit_height = SCREENHEIGHT - (ST_HEIGHT_JAG << hires);
    else
    finit_height = SCREENHEIGHT - (ST_HEIGHT << hires);

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


//
// Clip lines, draw visible part sof lines.
//
void AM_drawMline (mline_t* ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
    {
        // draws it on frame buffer using fb coords
        AM_drawFline(&fl, color);
    }
}


//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
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
        start -= ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS));
    }

    end = m_x + m_w;

    if (automap_rotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x = start ; x < end ; x += (MAPBLOCKUNITS<<FRACBITS))
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
    {
        start -= ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS));
    }

    end = m_y + m_h;

    if (automap_rotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y = start ; y < end ; y += (MAPBLOCKUNITS<<FRACBITS))
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
void AM_drawWalls(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
            {
                continue;
            }

            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS);
            }
            else
            {
                if (lines[i].special == 39)          // teleporters
                {
                    AM_drawMline(&l, WALLCOLORS+WALLRANGE/2);
                }
                else if (lines[i].flags & ML_SECRET) // secret door
                {
                    if (cheating)
                    {
                        AM_drawMline(&l, SECRETWALLCOLORS);
                    }
                    else
                    {
                        AM_drawMline(&l, WALLCOLORS);
                    }
                }
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight) 
                {
                    AM_drawMline(&l, FDWALLCOLORS); // floor level change
                }
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight) 
                {
                    AM_drawMline(&l, CDWALLCOLORS); // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS+3);
        }
    }
}


//
// [JN] Automap colors: Boom
//
void AM_drawWallsBoom(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
            {
                continue;
            }

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
                else
                if (lines[i].flags & ML_SECRET)
                {
                    AM_drawMline(&l, 23);      // wall color
                }

                // [JN] BLUE locked doors
                else
                if (lines[i].special == 26
                ||  lines[i].special == 32
                ||  lines[i].special == 99
                ||  lines[i].special == 133)
                {
                    AM_drawMline(&l, 204);
                }
                // [JN] RED locked doors
                else
                if (lines[i].special == 28
                ||  lines[i].special == 33
                ||  lines[i].special == 134
                ||  lines[i].special == 135)
                {
                    AM_drawMline(&l, 175);
                }
                // [JN] YELLOW locked doors
                else
                if (lines[i].special == 27
                ||  lines[i].special == 34
                ||  lines[i].special == 136
                ||  lines[i].special == 137)
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


//
// [JN] Atari Jaguar: draw walls using Jaguar colors.
// Adapted from original Jaguar Doom source, AM_Drawer.
//
void AM_drawWallsJaguar(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
            {
                continue;
            }

            if (!lines[i].backsector)
            {
                AM_drawMline(&l, RED_JAGUAR);
            }
            else
            {
                // Teleport line
                if (lines[i].special == 39
                ||  lines[i].special == 97)
                AM_drawMline(&l, GREEN_JAGUAR);

                // Secret door
                else if (lines[i].flags & ML_SECRET)
                AM_drawMline(&l, RED_JAGUAR);

                // Any special linedef
                else if (lines[i].special)
                AM_drawMline(&l, MAGENTA_JAGUAR);

                // Floor level change
                else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
                AM_drawMline(&l, YELLOW_JAGUAR);

                // Ceiling level change
                else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
                AM_drawMline(&l, YELLOW_JAGUAR);

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

//
// [JN] Automap colors: Raven (Heretic)
//
void AM_drawWallsRaven(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
            {
                continue;
            }

            if (!lines[i].backsector)
            {
                AM_drawMline(&l, 151);
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
                        AM_drawMline(&l, 108);
                }
                // [JN] BLUE locked doors
                else 
                if (lines[i].special == 26
                ||  lines[i].special == 32
                ||  lines[i].special == 99
                ||  lines[i].special == 133)
                {
                    AM_drawMline(&l, 199);
                }
                // [JN] RED locked doors
                else
                if (lines[i].special == 28
                ||  lines[i].special == 33
                ||  lines[i].special == 134
                ||  lines[i].special == 135)
                {
                    AM_drawMline(&l, 178);
                }
                // [JN] YELLOW locked doors
                else
                if (lines[i].special == 27
                ||  lines[i].special == 34
                ||  lines[i].special == 136
                ||  lines[i].special == 137)
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
            if (!(lines[i].flags & LINE_NEVERSEE))
            {
                AM_drawMline(&l, 99);
            }
        }
    }
}


//
// [JN] Automap colors: Strife
//
void AM_drawWallsStrife(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
            {
                continue;
            }

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
            {
                AM_drawMline(&l, 102);
            }
        }
    }
}


void AM_drawWallsUnity(void)
{
    int    i;
    static mline_t l;

    for (i = 0 ; i < numlines ; i++)
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
                if (lines[i].special == 1
                ||  lines[i].special == 31
                ||  lines[i].special == 117
                ||  lines[i].special == 118)
                {
                    AM_drawMline(&l, 81);
                }
                // [JN] Various teleporters
                else
                if (lines[i].special == 39
                ||  lines[i].special == 97
                ||  lines[i].special == 125
                ||  lines[i].special == 126)
                {
                    AM_drawMline(&l, 120);
                }
                // [JN] BLUE locked doors
                else
                if (lines[i].special == 26
                ||  lines[i].special == 32
                ||  lines[i].special == 99
                ||  lines[i].special == 133)
                {
                    AM_drawMline(&l, 200);
                }
                // [JN] RED locked doors
                else
                if (lines[i].special == 28
                ||  lines[i].special == 33
                ||  lines[i].special == 134
                ||  lines[i].special == 135)
                {
                    AM_drawMline(&l, 176);
                }
                // [JN] YELLOW locked doors
                else
                if (lines[i].special == 27
                ||  lines[i].special == 34
                ||  lines[i].special == 136
                ||  lines[i].special == 137)
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
            if (lines[i].special == 11
            ||  lines[i].special == 51
            ||  lines[i].special == 52
            ||  lines[i].special == 124)
            {
                AM_drawMline(&l, 252);
            }
        }
        // [JN] Computermap visible lines
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, 104);
        }
    }
}


//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void AM_rotate (int64_t *x, int64_t *y, angle_t a)
{
    int64_t tmpx;

    tmpx = FixedMul(*x,finecosine[a>>ANGLETOFINESHIFT]) 
         - FixedMul(*y,finesine[a>>ANGLETOFINESHIFT]);
    *y = FixedMul(*x,finesine[a>>ANGLETOFINESHIFT])
       + FixedMul(*y,finecosine[a>>ANGLETOFINESHIFT]);
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


void AM_drawLineCharacter (mline_t *lineguy, int lineguylines, fixed_t scale, 
                           angle_t angle, int color, fixed_t x, fixed_t y)
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


void AM_drawPlayers(void)
{
    int         i;
    int         color;
    int         their_color = -1;
    static int  their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
    mpoint_t    pt;
    player_t   *p;

    if (!netgame)
    {
        pt.x = plr->mo->x;
        pt.y = plr->mo->y;

        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }
    
        if (gamemission == jaguar)
        {
            // [JN] Jaguar Doom: draw dark green, blinking arrow.
            if (leveltime & 8)
            AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0, 
                                 plr->mo->angle, GREEN_JAGUAR, pt.x, pt.y);
        }
        else
        {
            if (cheating)
            {
                AM_drawLineCharacter(cheat_player_arrow, arrlen(cheat_player_arrow), 0,
                                     plr->mo->angle, WHITE, pt.x, pt.y);
            }
            else
            {
                AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0,
                                     plr->mo->angle, WHITE, pt.x, pt.y);
            }
        }
        
        return;
    }

    for (i = 0 ; i < MAXPLAYERS ; i++)
    {
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

        pt.x = p->mo->x;
        pt.y = p->mo->y;

        if (automap_rotate)
        {
            AM_rotatePoint(&pt);
        }

        AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0,
                             p->mo->angle, color,pt.x, pt.y);
    }
}


void AM_drawThings (int colors, int colorrange)
{
    int       i;
    mpoint_t  pt;
    mobj_t   *t;

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

            pt.x = t->x;
            pt.y = t->y;

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            // [JN] Jaguar Doom: use dark green for things drawing
            AM_drawLineCharacter(thintriangle_guy, 
                                 arrlen(thintriangle_guy),
                                 16 << FRACBITS, t->angle, 
                                 gamemission == jaguar ? GREEN_JAGUAR : colors,
                                 pt.x, pt.y);
            t = t->snext;
        }
    }
}


void AM_drawMarks(void)
{
    int       i, fx, fy;
    mpoint_t  pt;

    for (i = 0 ; i < AM_NUMMARKPOINTS ; i++)
    {
        if (markpoints[i].x != -1)
        {
            // [crispy] center marks around player
            pt.x = markpoints[i].x;
            pt.y = markpoints[i].y;

            if (automap_rotate)
            {
                AM_rotatePoint(&pt);
            }

            fx = (flipwidth[CXMTOF(pt.x)] >> hires) - 1;
            fy = (CYMTOF(pt.y) >> hires) - 2;

            if (fx >= f_x && fx <= (f_w >> hires) - 5 
            &&  fy >= f_y && fy <= (f_h >> hires) - 6)
            {
                V_DrawPatch(fx, fy, marknums[i]);
            }
        }
    }
}


void AM_drawCrosshair(int color)
{
    if (vanillaparm)
    {
        fb[(f_w*(f_h+1))/2] = color; // single point for now
    }
    else
    {
        // [JN] Draw small graphical crosshair,
        // if automap isn't following player.
        if (!automap_follow)
        {
            dp_translation = cr[CR_GRAY];
            V_DrawPatchUnscaled(screenwidth/2, (SCREENHEIGHT-ST_HEIGHT-26)/2,
                                W_CacheLumpName(DEH_String("XHAIR_1S"), PU_CACHE));
            dp_translation = NULL;
        }
    }
}


void AM_Drawer (void)
{
    if (!automapactive)
    {
        return;
    }

    if (!automap_overlay)
    {
        AM_clearFB(BACKGROUND);
    }

    if (automap_grid)
    {
        AM_drawGrid(GRIDCOLORS);
    }

    // [JN] Jaguar Doom: always use Jaguar automap colors
    if (gamemission == jaguar)
    {
        AM_drawWallsJaguar();
    }
    else
    {
        if (automap_color == 0 || vanillaparm) // Doom
        AM_drawWalls();
        else if (automap_color == 1)           // Boom
        AM_drawWallsBoom();
        else if (automap_color == 2)           // Jaguar
        AM_drawWallsJaguar();
        else if (automap_color == 3)           // Raven (Heretic)
        AM_drawWallsRaven();
        else if (automap_color == 4)           // Strife
        AM_drawWallsStrife();
        else                                   // Unity
        AM_drawWallsUnity();
    }

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


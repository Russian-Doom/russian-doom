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



#ifndef __AMMAP_H__
#define __AMMAP_H__


#include <stdint.h>
#include "m_fixed.h"

// For use if I do walls with outsides/insides
#define REDS		12*8
#define REDRANGE	1       //16
#define BLUES		(256-4*16+8)
#define BLUERANGE	1       //8
#define GREENS		(33*8)
#define GREENRANGE	1       //16
#define GRAYS		(5*8)
#define GRAYSRANGE	1       //16
#define BROWNS		(14*8)
#define BROWNRANGE	1       //16
#define YELLOWS		10*8
#define YELLOWRANGE	1
#define BLACK		0
#define WHITE		4*8
#define PARCH		13*8-1
#define BLOODRED  150
#define BLUEKEY 	197
#define YELLOWKEY 144
#define GREENKEY  220

// Automap colors
#define BACKGROUND	PARCH
#define YOURCOLORS	WHITE
#define YOURRANGE	0
#define WALLCOLORS	REDS
#define WALLRANGE	REDRANGE
#define TSWALLCOLORS	GRAYS
#define TSWALLRANGE	GRAYSRANGE
#define FDWALLCOLORS	BROWNS
#define FDWALLRANGE	BROWNRANGE
#define CDWALLCOLORS	YELLOWS
#define CDWALLRANGE	YELLOWRANGE
#define THINGCOLORS	GREENS
#define THINGRANGE	GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS	(GRAYS + GRAYSRANGE/2)
#define GRIDRANGE	0
#define XHAIRCOLORS	GRAYS

#define AM_MSGHEADER (('a'<<24)+('m'<<16))
#define AM_MSGENTERED (AM_MSGHEADER | ('e'<<8))
#define AM_MSGEXITED (AM_MSGHEADER | ('x'<<8))

#define INITSCALEMTOF (.2*FRACUNIT)     // scale on entry

// translates between frame-buffer and map distances
// [crispy] fix int overflow that causes map and grid lines to disappear
#define FTOM(x) (((int64_t)((x)<<FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>FRACBITS)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

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


// [JN] Now global, since marks preserved in saved games.
extern mpoint_t *markpoints;
extern int       markpointnum, markpointnum_max;
void AM_Start (void);
void AM_Stop (void);
void AM_clearMarks (void);
void AM_initMarksColor (int color);

#endif

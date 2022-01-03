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
//	System specific interface stuff.
//


#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"


//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants. Now why not 32 levels here?
#define LIGHTLEVELS         16
#define LIGHTSEGSHIFT       4

#define MAXLIGHTSCALE       48
#define LIGHTSCALESHIFT     12

// [crispy] & [JN] smoother diminished lighting
#define MAXLIGHTZ           1024
#define LIGHTZSHIFT         17

// [JN] Vanilla values
#define MAXLIGHTZ_VANILLA   128
#define LIGHTZSHIFT_VANILLA 20

// [JN] Define, which diminished lighting to use
extern int lightzshift, maxlightz; 

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS        32

//
// POV related.
//

extern int viewwindowx, viewwindowy;
extern int centerx, centery;
extern int validcount;

extern fixed_t viewcos, viewsin;
extern fixed_t centerxfrac, centeryfrac;
extern fixed_t projection;

extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];
extern lighttable_t *fixedcolormap;

// [JN] Floor brightmaps
extern lighttable_t *fullbright_notgrayorbrown_floor[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Wall and thing brightmaps
extern lighttable_t *fullbright_redonly[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_notgray[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_notgrayorbrown[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_greenonly1[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_greenonly2[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_greenonly3[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_orangeyellow[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_dimmeditems[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_brighttan[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_redonly1[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_explosivebarrel[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_alllights[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_candles[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_pileofskulls[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *fullbright_redonly2[LIGHTLEVELS][MAXLIGHTSCALE];

extern int extralight;

//
// Sky rendering
//

#define SKYFLATNAME     "F_SKY1"    // SKY, store the number for name.
#define ANGLETOSKYSHIFT 22          // The sky map is 256*128*4 maps.

extern int skyflatnum, skytexture, skytexturemid;

//
// Called whenever the view size changes.
//
extern  int     screenblocks;
extern	int		detailshift;  // Blocky/low detail mode (0 = high, 1 = low).
extern  int     detailLevel;
extern  boolean setsizeneeded;

//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void (*colfunc) (void);
extern void (*transcolfunc) (void);
extern void (*basecolfunc) (void);
extern void (*fuzzcolfunc) (void);
extern void (*tlcolfunc) (void);
extern void (*spanfunc) (void);

//
// Utility functions.
//
int R_PointOnSide (fixed_t x, fixed_t y, node_t *node);
int R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line);

angle_t R_PointToAngle (fixed_t x, fixed_t y);
angle_t R_PointToAngleCrispy (fixed_t x, fixed_t y);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

fixed_t R_PointToDist (fixed_t x, fixed_t y);

subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);

//
// REFRESH - the actual rendering functions.
//

void R_RenderPlayerView (player_t *player);
void R_Init (void);
void R_SetViewSize (int blocks, int detail);
void R_ExecuteSetViewSize (void);
void R_InitBrightmaps (void);

#endif

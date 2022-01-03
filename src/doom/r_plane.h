//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Refresh, visplane stuff (floor, ceilings).
//


#ifndef __R_PLANE__
#define __R_PLANE__


#include "r_data.h"


// Visplane related.
extern int* lastopening; // [crispy] 32-bit integer math

// [JN] e6y: resolution limitation is removed
extern int *floorclip, *ceilingclip; // dropoff overflow

extern fixed_t *yslope, *distscale;
extern fixed_t  yslopes[LOOKDIRS][SCREENHEIGHT];


void R_InitPlanesRes (void);
void R_InitVisplanesRes (void);
void R_ClearPlanes (void);
void R_DrawPlanes (void);

visplane_t *R_FindPlane (fixed_t height, int picnum, int lightlevel);
visplane_t *R_CheckPlane (visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane (const visplane_t *pl, int start, int stop);


#endif


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
//	Refresh module, BSP traversal and handling.
//


#ifndef __R_BSP__
#define __R_BSP__

#include "limits.h"

extern seg_t       *curline;
extern side_t      *sidedef;
extern line_t      *linedef;
extern sector_t    *frontsector;
extern sector_t    *backsector;
extern drawseg_t   *ds_p;

// [JN] killough: New code which removes 2s linedef limit
extern drawseg_t *drawsegs;
extern unsigned   maxdrawsegs;

// [JN] Improved column clipping.
extern byte *solidcol;
void R_InitClipSegs (void);

typedef void (*drawfunc_t) (int start, int stop);


// BSP?
void R_ClearClipSegs (void);
void R_ClearDrawSegs (void);
void R_RenderBSPNode (int bspnum);


#endif

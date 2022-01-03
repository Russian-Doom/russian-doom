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
//	Refresh/render internal state variables (global).
//


#ifndef __R_STATE__
#define __R_STATE__

// Need data structure definitions.
#include "d_player.h"
#include "r_data.h"


// -----------------------------------------------------------------------------
// Refresh internal data structures, for rendering.
// -----------------------------------------------------------------------------

// needed for texture pegging 
extern fixed_t *textureheight;

// needed for pre rendering (fracs)
extern fixed_t *spritewidth, *spriteoffset, *spritetopoffset;

// colormaps
extern lighttable_t *colormaps;
extern lighttable_t *colormaps_rd;

// [JN] Brightmaps
extern lighttable_t *brightmaps_notgray;
extern lighttable_t *brightmaps_notgrayorbrown;
extern lighttable_t *brightmaps_redonly;
extern lighttable_t *brightmaps_greenonly1;
extern lighttable_t *brightmaps_greenonly2;
extern lighttable_t *brightmaps_greenonly3;
extern lighttable_t *brightmaps_orangeyellow;
extern lighttable_t *brightmaps_dimmeditems;
extern lighttable_t *brightmaps_brighttan;
extern lighttable_t *brightmaps_redonly1;
extern lighttable_t *brightmaps_explosivebarrel;
extern lighttable_t *brightmaps_alllights;
extern lighttable_t *brightmaps_candles;
extern lighttable_t *brightmaps_pileofskulls;
extern lighttable_t *brightmaps_redonly2;

extern int viewwidth, viewheight;
extern int scaledviewwidth, scaledviewheight;


// [crispy] lookup table for horizontal screen coordinates
extern int flipwidth[WIDESCREENWIDTH];

extern int firstflat;

// for global animation
extern int* flattranslation, *texturetranslation;


// Sprite....
extern int firstspritelump, lastspritelump, numspritelumps;


// -----------------------------------------------------------------------------
// Lookup tables for map data.
// -----------------------------------------------------------------------------

extern int          numsprites;
extern spritedef_t *sprites;
extern int          numvertexes;
extern vertex_t    *vertexes;
extern int          numsegs;
extern seg_t       *segs;
extern int          numsectors;
extern sector_t    *sectors;
extern int          numsubsectors;
extern subsector_t *subsectors;
extern int          numnodes;
extern node_t      *nodes;
extern int          numlines;
extern line_t      *lines;
extern int          numsides;
extern side_t      *sides;


// -----------------------------------------------------------------------------
// POV data.
// -----------------------------------------------------------------------------

extern fixed_t viewx, viewy, viewz;

extern angle_t   viewangle;
extern player_t *viewplayer;

// ?
extern int      viewangletox[FINEANGLES/2];
extern angle_t  clipangle;
extern angle_t *xtoviewangle;
extern angle_t *linearskyangle;

// angle to line origin
extern int rw_angle1;

extern visplane_t *floorplane;
extern visplane_t *ceilingplane;


#endif

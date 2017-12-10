//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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

// Russian Doom (C) 2016-2017 Julian Nechaevsky


#ifndef __R_STATE__
#define __R_STATE__

// Need data structure definitions.
#include "d_player.h"
#include "r_data.h"


//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t* textureheight;

// needed for pre rendering (fracs)
extern fixed_t* spritewidth;

extern fixed_t* spriteoffset;
extern fixed_t* spritetopoffset;

extern lighttable_t* colormaps;

// [JN] Brightmaps
extern lighttable_t* brightmaps_notgray;
extern lighttable_t* brightmaps_notgrayorbrown;
extern lighttable_t* brightmaps_redonly;
extern lighttable_t* brightmaps_greenonly1;
extern lighttable_t* brightmaps_greenonly2;
extern lighttable_t* brightmaps_greenonly3;
extern lighttable_t* brightmaps_orangeyellow;
extern lighttable_t* brightmaps_dimmeditems;
extern lighttable_t* brightmaps_poss_attack;
extern lighttable_t* brightmaps_spos_attack;
extern lighttable_t* brightmaps_cpos_attack_1;
extern lighttable_t* brightmaps_cpos_attack_2;
extern lighttable_t* brightmaps_caco_attack_1;
extern lighttable_t* brightmaps_caco_attack_2;
extern lighttable_t* brightmaps_pain_elemental;
extern lighttable_t* brightmaps_lost_soul_1;
extern lighttable_t* brightmaps_lost_soul_2;
extern lighttable_t* brightmaps_bspi_attack_1;
extern lighttable_t* brightmaps_bspi_attack_2;
extern lighttable_t* brightmaps_skel_attack;
extern lighttable_t* brightmaps_fatt_attack;
extern lighttable_t* brightmaps_sswv_attack;
extern lighttable_t* brightmaps_torches;
extern lighttable_t* brightmaps_floor_lamp;
extern lighttable_t* brightmaps_tech_lamps;
extern lighttable_t* brightmaps_flam_barrel;
extern lighttable_t* brightmaps_expl_barrel;

extern int viewwidth;
extern int scaledviewwidth;
extern int viewheight;
extern int scaledviewheight;

extern int firstflat;

// for global animation
extern int* flattranslation;	
extern int* texturetranslation;	


// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;


//
// Lookup tables for map data.
//
extern int          numsprites;
extern spritedef_t* sprites;

extern int       numvertexes;
extern vertex_t* vertexes;

extern int    numsegs;
extern seg_t* segs;

extern int       numsectors;
extern sector_t* sectors;

extern int          numsubsectors;
extern subsector_t* subsectors;

extern int     numnodes;
extern node_t* nodes;

extern int     numlines;
extern line_t* lines;

extern int     numsides;
extern side_t* sides;


//
// POV data.
//
extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;

extern angle_t   viewangle;
extern player_t* viewplayer;


// ?
extern angle_t clipangle;

extern int     viewangletox[FINEANGLES/2];
extern angle_t xtoviewangle[SCREENWIDTH+1];
//extern fixed_t		finetangent[FINEANGLES/2];

extern fixed_t rw_distance;
extern angle_t rw_normalangle;


// angle to line origin
extern int rw_angle1;

// Segs count?
extern int sscount;

extern visplane_t* floorplane;
extern visplane_t* ceilingplane;


#endif


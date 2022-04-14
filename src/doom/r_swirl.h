//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2000, 2005-2014 Simon Howard
// Copyright(C) 2019 Fabian Greffrath
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
//	[crispy] add support for SMMU swirling flats
//

#pragma once

void  R_InitDistortedFlats (void);
char *R_DistortedFlat (int flatnum);

extern void R_FlowPlane (int flow);
extern fixed_t FlowFactor_X;
extern fixed_t FlowFactor_Y;
extern fixed_t FlowDelta_X;
extern fixed_t FlowDelta_Y;

void R_FallLinedef (void);
extern fixed_t FallFactor_100, FallFactor_100_old;
extern fixed_t FallFactor_101, FallFactor_101_old;
extern fixed_t FallFactor_102, FallFactor_102_old;
extern fixed_t FallFactor_103, FallFactor_103_old;
extern fixed_t FallFactor_104, FallFactor_104_old;

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     sector;
    short   flow;
} flow_t;

extern flow_t flow[];

typedef struct
{
    int     mission;
    int     epsiode;
    int     map;
    int     linedef;
    short   fall;
} fall_t;

extern fall_t fall[];

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


#pragma once

#include <stdint.h>
#include "m_fixed.h"


typedef struct
{
    int64_t x,y;
} mpoint_t;

extern mpoint_t *markpoints;
extern int       markpointnum, markpointnum_max;

void AM_initVariables (void);
// [JN] Preload automap graphics once at game startup.
void AM_initPics (void);

void AM_Start (void);
void AM_Stop (void);
void AM_clearMarks (void);
void AM_initMarksColor (int color);

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
//  AutoMap module.
//


#pragma once

#include "d_event.h"
#include "m_cheat.h"


typedef struct
{
    int64_t x,y;
} mpoint_t;

extern mpoint_t *markpoints;
extern int markpointnum, markpointnum_max;

// Called by main loop.
const boolean AM_Responder (event_t *ev);

// Called by main loop.
void AM_Ticker (void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer (void);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Start (void);
void AM_Stop (void);

// [JN] Automap color scheme init.
// Called at game startup or when scheme is changed.
void AM_initColors (void);

// [JN] Automap mark color scheme init.
// Called at game startup or when color is changed.
void AM_initMarksColor (int color);

// [JN] Make global, since mark preserved in saved games.
void AM_clearMarks (void);

extern cheatseq_t cheat_amap;

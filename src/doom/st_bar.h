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


#pragma once

#include "doomtype.h"
#include "d_event.h"
#include "m_cheat.h"
#include "v_patch.h"


// Sizes of statusbar.
#define ST_HEIGHT       32
#define ST_Y            (ORIGHEIGHT - ST_HEIGHT)
#define ST_HEIGHT_JAG   40
#define ST_Y_JAG        (ORIGHEIGHT - ST_HEIGHT_JAG)

// [JN] Variable status bar height between common Doom and Jaguar Doom.
extern int st_height;

extern int st_palette;

// [crispy] Demo Timer widget
extern void ST_DrawDemoTimer (const int time);
extern int defdemotics, deftotaldemotics;

// Cheat codes
extern cheatseq_t cheat_mus;
extern cheatseq_t cheat_god;
extern cheatseq_t cheat_ammo;
extern cheatseq_t cheat_ammonokey;
extern cheatseq_t cheat_noclip;
extern cheatseq_t cheat_commercial_noclip;
extern cheatseq_t cheat_powerup[7];
extern cheatseq_t cheat_choppers;
extern cheatseq_t cheat_clev;
extern cheatseq_t cheat_mypos;

// [JN] Colored status bar widgets
extern byte *stbar_color_high_set;
extern byte *stbar_color_normal_set;
extern byte *stbar_color_low_set;
extern byte *stbar_color_critical_set;
extern byte *stbar_color_armor_1_set;
extern byte *stbar_color_armor_2_set;
extern byte *stbar_color_armor_0_set;

// [JN] Crosshair stuff.
extern patch_t *CrosshairPatch;
extern byte    *CrosshairOpacity;
extern int  CrosshairShowcaseTimeout;
extern void Crosshair_DefinePatch (void);
extern void Crosshair_DefineOpacity (void);
extern void Crosshair_DefineDrawingFunc (void);
extern void Crosshair_Colorize_inMenu (void);
extern void Crosshair_Draw (void);

// Main functions
boolean ST_Responder (event_t* ev);
void ST_Ticker (void);
void ST_Drawer (void);
void ST_WidgetsDrawer (void);
void ST_MapNameDrawer (void);
void ST_Start (void);
void ST_Init (void);

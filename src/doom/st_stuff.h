//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//



#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "m_cheat.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT	        32
#define ST_Y		        (ORIGHEIGHT - ST_HEIGHT)

// [JN] Jaguar status bar is 40 px tall
#define ST_HEIGHT_JAG   40
#define ST_Y_JAG        (ORIGHEIGHT - ST_HEIGHT_JAG)

// [crispy] Demo Timer widget
extern void ST_DrawDemoTimer (const int time);
extern int defdemotics, deftotaldemotics;


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);

// [JN] Called in options menu
void ST_refreshBackground (void);
void ST_drawWidgets (boolean refresh);
void ST_doRefresh (void);
void ST_createWidgets (void);


// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
    
} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
    
} st_chatstateenum_t;



extern byte *st_backing_screen;
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
extern int st_palette;

// [JN] Colored status bar widgets
extern byte *sbar_color_high_set;
extern byte *sbar_color_normal_set;
extern byte *sbar_color_low_set;
extern byte *sbar_color_critical_set;
extern byte *sbar_color_armor_1_set;
extern byte *sbar_color_armor_2_set;
extern byte *sbar_color_armor_0_set;


#endif

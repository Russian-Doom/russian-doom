//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
// DESCRIPTION:  Head up display
//

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"


//
// Globally visible constants.
//
#define HU_FONTSTART    '!' // the first font characters
#define HU_FONTEND      '_' // the last font characters

// [JN] Big STCFB font
#define HU_FONTSTART2   '!'	// the first font characters
#define HU_FONTEND2     '{'	// the last font characters
#define HU_FONTSIZE2    (HU_FONTEND2 - HU_FONTSTART2 + 1)

// Calculate # of glyphs in font.
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)	

#define HU_BROADCAST    5

#define HU_MSGREFRESH   KEY_ENTER
#define HU_MSGX         0
#define HU_MSGY         0
#define HU_MSGWIDTH     64  // in characters
#define HU_MSGHEIGHT    1   // in lines

#define HU_MSGTIMEOUT   (4*TICRATE)


//
// HEADS UP TEXT
//

void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);

#endif


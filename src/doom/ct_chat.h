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

void CT_Init (void);
void CT_Ticker (void);
void CT_Drawer (void);
char CT_dequeueChatChar (void);

boolean CT_Responder(event_t * ev);

extern boolean chatmodeon;
extern char *CT_FromPlrText[MAXPLAYERS];
extern char *chat_macros[10];

//
// FONTS
//

#define HU_FONTSTART    '!' // the first font characters
#define HU_FONTEND      '_' // the last font characters
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)	

// [JN] Big STCFB font
#define HU_FONTSTART2   '!'	// the first font characters
#define HU_FONTEND2     '}'	// the last font characters
#define HU_FONTSIZE2    (HU_FONTEND2 - HU_FONTSTART2 + 1)

// Global font definitions
extern patch_t *hu_font[HU_FONTSIZE];
extern patch_t *hu_font_small_eng[HU_FONTSIZE];
extern patch_t *hu_font_small_rus[HU_FONTSIZE];
extern patch_t *hu_font_big_eng[HU_FONTSIZE2];
extern patch_t *hu_font_big_rus[HU_FONTSIZE2];

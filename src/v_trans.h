// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: v_video.h,v 1.9 1998/05/06 11:12:54 jim Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//  Gamma correction LUT.
//  Color range translation support
//  Functions to draw patches (by post) directly to screen.
//  Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------


#ifndef __V_TRANS__
#define __V_TRANS__

#include "doomtype.h"


typedef enum
{
    CR_RED,
    CR_DARKRED,
    CR_GREEN,
    CR_DARKGREEN,
    CR_OLIVE,
    CR_BLUE,
    CR_BLUE2,
    CR_DARKBLUE,
    CR_YELLOW,
    CR_ORANGE,
    CR_WHITE,
    CR_GRAY,
    CR_DARKGRAY,
    CR_TAN,
    CR_BROWN,
    CR_ALMOND,
    CR_KHAKI,
    CR_PINK,
    CR_BURGUNDY,
    CR_RED2BLUE,
    CR_RED2GREEN,
    // Heretic (big font)
    CR_GREEN2GRAY_HERETIC,
    CR_GREEN2RED_HERETIC,
    CR_GREEN2GOLD_HERETIC,
    CR_GREEN2ORANGE_HERETIC,
    CR_GREEN2BLUE_HERETIC,
    // Heretic (hud digits)
    CR_GOLD2GREEN_HERETIC,
    CR_GOLD2RED_HERETIC,
    CR_GOLD2BLUE_HERETIC,
    CR_GOLD2GRAY_HERETIC,
    CR_GOLD2ORANGE_HERETIC,
    // Heretic (small font)
    CR_WHITE2GRAY_HERETIC,
    CR_WHITE2DARKGRAY_HERETIC,
    CR_WHITE2RED_HERETIC,
    CR_WHITE2DARKRED_HERETIC,
    CR_WHITE2GREEN_HERETIC,
    CR_WHITE2DARKGREEN_HERETIC,
    CR_WHITE2OLIVE_HERETIC,
    CR_WHITE2BLUE_HERETIC,
    CR_WHITE2DARKBLUE_HERETIC,
    CR_WHITE2PURPLE_HERETIC,
    CR_WHITE2NIAGARA_HERETIC,
    CR_WHITE2AZURE_HERETIC,
    CR_WHITE2YELLOW_HERETIC,
    CR_WHITE2GOLD_HERETIC,
    CR_WHITE2DARKGOLD_HERETIC,
    CR_WHITE2TAN_HERETIC,
    CR_WHITE2BROWN_HERETIC,
    // Heretic (colored blood and health gem)
    CR_RED2DARKRED_HERETIC,
    CR_RED2GREEN_HERETIC,
    CR_RED2MIDGREEN_HERETIC,
    CR_RED2DARKGREEN_HERETIC,
    CR_RED2YELLOW_HERETIC,
    CR_RED2GOLD_HERETIC,
    CR_RED2BROWN_HERETIC,
    CR_RED2MAGENTA_HERETIC,
    CR_RED2GRAY_HERETIC,
    CR_RED2WHITE_HERETIC,
    CR_RED2BLACK_HERETIC,
    // Heretic (all-to-gray)
    CR_MONOCHROME_HERETIC,
    // Hexen (big font and health gem)
    CR_RED2DARKRED_HEXEN,
    CR_RED2GREEN_HEXEN,
    CR_RED2DARKGREEN_HEXEN,
    CR_RED2YELLOW_HEXEN,
    CR_RED2GOLD_HEXEN,
    CR_RED2GRAY_HEXEN,
    CR_RED2DARKGRAY_HEXEN,
    // Hexen (small font)
    CR_GRAY2GDARKGRAY_HEXEN,
    CR_GRAY2DARKGOLD_HEXEN,
    CR_GRAY2GREEN_HEXEN,
    CR_GRAY2DARKGREEN_HEXEN,
    CR_GRAY2RED_HEXEN,
    CR_GRAY2BLUE_HEXEN,
    CR_GREEN2GRAY_HEXEN, // remove me
    CR_GREEN2RED_HEXEN,  // remove me
    // Hexen (status bar font)
    CR_YELLOW2GRAY_HEXEN,
    CR_YELLOW2GREEN_HEXEN,
    CR_YELLOW2RED_HEXEN,
    // Strife
    CR_GOLD2GRAY_STRIFE,
    CR_GOLD2GREEN_STRIFE,
    CR_GOLD2RED_STRIFE,
    CR_GOLD2DARKGOLD_STRIFE,
    CRMAX,
    CR_NONE
} Translation_CR_t;

extern byte  *cr[CRMAX];

extern byte  *tintmap;
extern byte  *shademap;
extern byte  *fuzzmap;

int V_GetPaletteIndex(byte *palette, int r, int g, int b);

#endif // __V_TRANS__ 

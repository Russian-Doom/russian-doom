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


#pragma once

#include "doomtype.h"


typedef enum
{
    CR_BLACK,
    CR_DARK,
    CR_THIRDSATURTION,
    CR_MONOCHROME,

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

    // Heretic-specific colors
    CR_BRIGHTYELLOW,
    CR_DARKGOLD,
    CR_PURPLE,
    CR_NIAGARA,
    CR_AZURE,
    CR_FLAME,

    // Doom (colored blood and slider gem)
    CR_RED2BLUE, // Terminator - don't override tablified colors starting from this line
    CR_RED2GREEN,
    CR_RED2GRAY,

    // Heretic (slider gem)
    CR_GREEN2GRAY_HERETIC,
    CR_GREEN2RED_HERETIC,
    // Heretic (health gem)
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

    // Hexen (slider gem)
    CR_GREEN2GRAY_HEXEN,
    CR_GREEN2RED_HEXEN,
    // Hexen (health gem)
    CR_RED2GREEN_HEXEN,
    CR_RED2DARKGREEN_HEXEN,
    CR_RED2YELLOW_HEXEN,
    CR_RED2GOLD_HEXEN,
    CR_RED2GRAY_HEXEN,
    CR_RED2DARKGRAY_HEXEN,

    CRMAX,
    CR_NONE,
    CR__STOP = CR_RED2BLUE
} Translation_CR_t;

extern byte  *cr[CRMAX];
extern char **crstr;

#define cr_esc '~'

int V_GetPaletteIndex(byte *palette, int r, int g, int b);
byte V_Colorize (byte *playpal, Translation_CR_t cr, byte source, boolean keepgray109);

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


enum
{
    CR_RED,
    CR_DARKRED,
    CR_GREEN,
    CR_DARKGREEN,
    CR_OLIVE,
    CR_BLUE,
    CR_BLUE2,
    CR_DARKBLUE,
    CR_GOLD,
    CR_ORANGE,
    CR_GRAY,
    CR_DARKGRAY,
    CR_TAN,
    CR_DARKTAN,
    CR_BROWN,
    CR_DARKBROWN,
    CR_BRICK,
    CR_DARKBRICK,
    CR_RED2BLUE,
    CR_RED2GREEN,
    // Heretic (big font)
    CR_GREEN2GRAY_HERETIC,
    CR_GREEN2RED_HERETIC,
    CR_GREEN2GOLD_HERETIC,
    CR_GREEN2BLUE_HERETIC,
    // Heretic (hud digits)
    CR_GOLD2GREEN_HERETIC,
    CR_GOLD2RED_HERETIC,
    CR_GOLD2BLUE_HERETIC,
    CR_GOLD2GRAY_HERETIC,
    // Heretic (small font)
    CR_GRAY2GDARKGRAY_HERETIC,  // Menu digits and inactive items
    CR_GRAY2DARKGOLD_HERETIC,   // Gameplay features headers
    CR_GRAY2GREEN_HERETIC,      // Gameplay features "ON"
    CR_GRAY2RED_HERETIC,        // Gameplay features "OFF"
    // Heretic (colored blood)
    CR_RED2MAGENTA_HERETIC,     // Magenta blood for Wizards
    CR_RED2GRAY_HERETIC,        // Gray blood for Iron Liches
    // Hexen (small font)
    CR_GRAY2GDARKGRAY_HEXEN,    // Menu digits and inactive items
    CR_GRAY2DARKGOLD_HEXEN,     // Gameplay features headers
    CR_GRAY2GREEN_HEXEN,        // Gameplay features "ON"
    CR_GRAY2RED_HEXEN,          // Gameplay features "OFF"
    // Strife
    CR_GOLD2GRAY_STRIFE,
    CR_GOLD2GREEN_STRIFE,
    CR_GOLD2RED_STRIFE,
    CR_GOLD2DARKGOLD_STRIFE,
    CRMAX
};

extern byte  *cr[CRMAX];

extern byte  *tintmap;
extern byte  *shademap;
extern byte  *fuzzmap;

int V_GetPaletteIndex(byte *palette, int r, int g, int b);

#endif // __V_TRANS__ 

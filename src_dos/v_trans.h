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
//  palette color ranges for translation.
//
//-----------------------------------------------------------------------------

#ifndef __V_TRANS__
#define __V_TRANS__

#include "doomtype.h"

enum
{
    CR_BRICK,
    CR_TAN,
    CR_GRAY,
    CR_DARKRED,
    CR_GREEN,
    CR_BROWN,
    CR_GOLD,
    CR_BLUE,
    CR_BLUE2,
    CR_RED,
    CR_RED2BLUE,
    CR_RED2GREEN,
    CRMAX
};

extern byte  *cr[CRMAX];

#endif // __V_TRANS__

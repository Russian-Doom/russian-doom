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
//	Mission start screen wipe/melt, special effects.
//	


#ifndef __F_WIPE_H__
#define __F_WIPE_H__


enum
{
    wipe_ColorXForm, // simple gradual pixel change for 8-bit only
    wipe_Melt,       // weird screen melt
    wipe_NUMWIPES
};


int wipe_StartScreen (int x, int y, int width, int height);
int wipe_EndScreen (int x, int y, int width, int height);
int wipe_ScreenWipe (int wipeno, int x, int y, int width, int height, int ticks);


#endif


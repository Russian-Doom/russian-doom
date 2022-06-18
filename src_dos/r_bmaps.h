//
// Copyright(C) 2017-2018 Julian Nechaevsky
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
//	Brightmap textures and flats lookup routine.
//


#pragma once


#define MINBRIGHT   24  // [JN] Minimal COLORMAP level for half-brights.

extern void R_InitBrightmaps ();

extern const byte *R_BrightmapForTexName (const char *texname);
extern const byte *R_BrightmapForSprite (const int type);
extern const byte *R_BrightmapForFlatNum (const int num);
extern const byte *R_BrightmapForState (const int state);
extern const byte **texturebrightmap;

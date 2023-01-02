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
// DESCRIPTION:
//	Gamma correction LUT.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//


#pragma once

#include "doomtype.h"

// Needed because we are refering to patches.
#include "v_patch.h"


//
// VIDEO
//

#define CENTERY			(SCREENHEIGHT/2)


extern int dirtybox[4];

extern byte *tinttable;
extern byte *transtable90;
extern byte *transtable80;
extern byte *transtable70;
extern byte *transtable60;
extern byte *transtable50;
extern byte *transtable40;
extern byte *transtable30;
extern byte *transtable20;
extern byte *transtable10;

extern byte *dp_translation;

extern int x_hires;

// haleyjd 08/28/10: implemented for Strife support
// haleyjd 08/28/10: Patch clipping callback, implemented to support Choco
// Strife.
typedef boolean (*vpatchclipfunc_t)(const patch_t *, int, int);
void V_SetPatchClipCallback(vpatchclipfunc_t func);


// Allocates buffer screens, call before R_Init.
void V_Init (void);

// Draw a block from the specified source screen to the screen.

void V_CopyRect(int srcx, int srcy, byte *source,
                int width, int height,
                int destx, int desty);

void V_DrawPatch (int x, int y, const patch_t *patch, const byte *table);
void V_DrawPatchFullScreen (patch_t *patch, const boolean flipped);
void V_DrawPatchFlipped (int x, int y, const patch_t *patch);
void V_DrawTLPatch (int x, int y, const patch_t *patch);
void V_DrawAltTLPatch (int x, int y, const patch_t *patch);
void V_DrawFadePatch (int x, int y, const patch_t *patch, const byte *table);
void V_DrawShadowedPatch (int x, int y, const patch_t *patch);
void V_DrawShadowedPatchDoom (int x, int y, const patch_t *patch);
void V_DrawShadowedPatchRaven (int x, int y, const patch_t *patch);
void V_DrawShadowedPatchStrife (int x, int y, const patch_t *patch);
void V_DrawXlaPatch (int x, int y, const patch_t *patch);     // villsa [STRIFE]
void V_DrawPatchUnscaled (int x, int y, const patch_t *patch, const byte *table);
void V_DrawPatchFinale (int x, int y, const patch_t *patch);
void V_FillFlat (char *lump);

// Draw a linear block of pixels into the view buffer.

void V_DrawBlock (const int x, const int y, const int width, int height, const byte *src);
void V_DrawScaledBlock (const int x, const int y, int width, int height, const byte *src);

void V_MarkRect (const int x, const int y, const int width, const int height);

void V_DrawFilledBox (const int x, const int y, const int w, const int h, const int c);
void V_DrawHorizLine (const int x, const int y, const int w, const int c);
void V_DrawVertLine (const int x, const int y, const int h, const int c);
void V_DrawBox (const int x, const int y, const int w, const int h, const int c);
void V_CopyScaledBuffer (byte *dest, const byte *src, const size_t size);

// Draw a raw screen lump

void V_DrawRawScreen (const byte *raw);

// Temporarily switch to using a different buffer to draw graphics, etc.

void V_UseBuffer (byte *buffer);

// Return to using the normal screen buffer to draw graphics.

void V_RestoreBuffer(void);

// Save a screenshot of the current screen to a file, named in the 
// format described in the string passed to the function, eg.
// "DOOM%02i.pcx"

void V_ScreenShot(const char *format);

// [JN] Load the lookup table for shadowed text from the TINTMAP lump.

void V_LoadTintMap(void);

// Load the lookup table for translucency calculations from the TINTTAB
// lump.

void V_LoadTintTable(void);

// villsa [STRIFE]
// Load the lookup table for translucency calculations from the XLATAB
// lump.

void V_LoadXlaTable(void);

void V_DrawMouseSpeedBox (const int speed);

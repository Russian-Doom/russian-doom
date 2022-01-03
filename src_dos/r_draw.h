//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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
//	System specific interface stuff.
//


#ifndef __R_DRAW__
#define __R_DRAW__


extern int           dc_x;
extern int           dc_yl;
extern int           dc_yh;
extern int           dc_texheight;
extern byte         *dc_source;  // first pixel in a column
extern fixed_t       dc_iscale;
extern fixed_t       dc_texturemid;
extern lighttable_t *dc_colormap;

extern int           ds_y;
extern int           ds_x1;
extern int           ds_x2;
extern byte         *ds_source;		    // start of a 64*64 tile image
extern lighttable_t *ds_colormap;
extern fixed_t       ds_xfrac;
extern fixed_t       ds_yfrac;
extern fixed_t       ds_xstep;
extern fixed_t       ds_ystep;

extern byte         *translationtables;
extern byte         *dc_translation;

// The span blitting interface. Hook in assembler or system specific BLT here.
void R_DrawColumn (void);
void R_DrawColumnLow (void);

// The Spectre/Invisibility effect.
void R_DrawFuzzColumn (void);
void R_DrawFuzzColumnLow (void);

// Draw with color translation tables,
//  for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void R_DrawTranslatedColumn (void);
void R_DrawTranslatedColumnLow (void);

// [JN] Translucent column
void R_DrawTLColumn (void);
void R_DrawTLColumnLow (void);

// [crispy] draw fuzz effect independent of rendering frame rate
void R_SetFuzzPosTic (void);
void R_SetFuzzPosDraw (void);


void R_VideoErase (unsigned ofs, int count);

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void R_DrawSpan (void);

// Low resolution mode, 160x200?
void R_DrawSpanLow (void);

// [JN] Draws the actual span as single color.
void R_DrawSpanNoTexture (void);
void R_DrawSpanLowNoTexture (void);

void R_InitBuffer (int width, int height);

// Rendering function.
void R_FillBackScreen (void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder (void);


#endif

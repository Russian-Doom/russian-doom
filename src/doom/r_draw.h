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
//	System specific interface stuff.
//


#ifndef __R_DRAW__
#define __R_DRAW__


//
// R_DrawColumn.
//

// The span blitting interface.
// Hook in assembler or system specific BLT here.
void R_DrawColumn (void);
void R_DrawColumnLow (void);


extern int           dc_x, dc_yl, dc_yh; 
extern int           dc_texheight;
extern fixed_t       dc_iscale;
extern fixed_t       dc_texturemid;
extern lighttable_t *dc_colormap; 
extern byte         *dc_source;

//
// The Spectre/Invisibility effect.
//

void R_DrawFuzzColumn (void);
void R_DrawFuzzColumnBW (void);
void R_DrawFuzzColumnImproved (void);
void R_DrawFuzzColumnImprovedBW (void);
void R_DrawFuzzColumnLow (void);
void R_DrawFuzzColumnLowBW (void);
void R_DrawFuzzColumnLowImproved (void);
void R_DrawFuzzColumnLowImprovedBW (void);
void R_DrawFuzzColumnTranslucent (void);
void R_DrawFuzzColumnTranslucentLow (void);

// [crispy] draw fuzz effect independent of rendering frame rate
void R_SetFuzzPosTic (void);
void R_SetFuzzPosDraw (void);

//
// Translated columns.
//

// Draw with color translation tables, for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void R_DrawTranslatedColumn (void);
void R_DrawTranslatedColumnLow (void);
void R_DrawTLColumn (void);
void R_DrawTLColumnLow (void);

extern byte *translationtables;
extern byte *dc_translation;

//
// Spans.
//

// Span blitting for rows, floor/ceiling.  No Sepctre effect needed.
void R_DrawSpan (void);
// Low resolution mode, 160x200?
void R_DrawSpanLow (void);

extern int           ds_y, ds_x1, ds_x2;
extern fixed_t       ds_xfrac, ds_yfrac;
extern fixed_t       ds_xstep, ds_ystep;
extern byte         *ds_source;
extern lighttable_t *ds_colormap;

//
// Video buffering stuff.
//

void R_InitBuffer (int width, int height);
// Rendering function.
void R_FillBackScreen (void);
// Copy a screen buffer.
void R_VideoErase (unsigned ofs, int count);
// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder (void);

#endif

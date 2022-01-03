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
//	Rendering of moving objects, sprites.
//

#include <stdint.h>

#ifndef __R_THINGS__
#define __R_THINGS__

#include "limits.h"


// Constant arrays used for psprite clipping and initializing clipping.
extern int *negonearray;       // [JN] killough 2/8/98: // dropoff overflow
extern int *screenheightarray; //      change to MAX_*  // dropoff overflow

// vars for R_DrawMaskedColumn
extern int          *mfloorclip;    // [crispy] 32-bit integer math
extern int          *mceilingclip;  // [crispy] 32-bit integer math
extern int64_t       sprtopscreen;
extern fixed_t       spryscale;
extern fixed_t       pspritescale;
extern fixed_t       pspriteiscale;


void R_DrawMaskedColumn (column_t *column);
void R_SortVisSprites (void);
void R_AddSprites (sector_t* sec);
void R_AddPSprites (void);
void R_DrawSprites (void);
void R_InitSpritesRes (void);
void R_InitSprites (char** namelist);
void R_ClearSprites (void);
void R_DrawMasked (void);
void R_ClipVisSprite (vissprite_t *vis, int xl, int xh);


#endif

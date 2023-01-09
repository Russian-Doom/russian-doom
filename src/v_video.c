//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//



#include "rd_io.h"
#include <string.h>
#include <math.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_ZLIB_APIS
#include "miniz.h"

#include "i_system.h"
#include "doomtype.h"
#include "deh_str.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "v_trans.h"
#include "jn.h"


// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK

// [JN] Blending tables used for translucent sprites and text fading/shadow.
byte *tinttable = NULL;
byte *transtable90 = NULL;
byte *transtable80 = NULL;
byte *transtable70 = NULL;
byte *transtable60 = NULL;
byte *transtable50 = NULL;
byte *transtable40 = NULL;
byte *transtable30 = NULL;
byte *transtable20 = NULL;
byte *transtable10 = NULL;

// [JN] Color translation
byte *dp_translation = NULL;

// villsa [STRIFE] Blending table used for Strife
byte *xlatab = NULL;

// The screen buffer that the v_video.c code draws to.

static byte *dest_screen = NULL;

int dirtybox[4]; 

// haleyjd 08/28/10: clipping callback function for patches.
// This is needed for Chocolate Strife, which clips patches to the screen.
static vpatchclipfunc_t patchclip_callback = NULL;


//
// [JN] Extra resolution variables:
//

int rendering_resolution = 1;
int rendering_resolution_temp;
int detailshift = 0;

// Main variable, defining high resolution.
// 0 =  320x200 (emulated)
// 1 =  640x400
// 2 = 1280x800
int hires;

// Addendum for high resolution rendering.
// Equals 1 for high detail, otherwise equals 0.
int quadres;

// Variable screen width and height values,
// shifted by hires variable.
int SCREENWIDTH;
int SCREENHEIGHT;

// Shortcut for patch drawing functions.
// Equals screenwidth * hires.
static int fullscreenwidth;

// Pointer to the function of drawing unscaled patches.
void (*V_DrawPatchUnscaled) (int x, int y, const patch_t *patch, const byte *table);


// -----------------------------------------------------------------------------
// V_MarkRect 
// -----------------------------------------------------------------------------

void V_MarkRect (const int x, const int y, const int width, const int height) 
{ 
    // If we are temporarily using an alternate screen, do not 
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y); 
        M_AddToBox (dirtybox, x + width-1, y + height-1); 
    }
} 
 
// -----------------------------------------------------------------------------
// V_CopyRect 
// -----------------------------------------------------------------------------

void V_CopyRect (int srcx, int srcy, byte *source,
                 int width, int height,
                 int destx, int desty)
{ 
    byte *src;
    byte *dest; 

    srcx <<= hires;
    srcy <<= hires;
    width <<= hires;
    height <<= hires;
    destx <<= hires;
    desty <<= hires;
 
#ifdef RANGECHECK 
    if (srcx < 0
     || srcx + width > screenwidth
     || srcy < 0
     || srcy + height > SCREENHEIGHT 
     || destx < 0
     || destx + width > screenwidth
     || desty < 0
     || desty + height > SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "Bad V_CopyRect" :
                 "Ошибка V_CopyRect");
    }
#endif 

    V_MarkRect(destx, desty, width, height); 
 
    src = source + screenwidth * srcy + srcx; 
    dest = dest_screen + screenwidth * desty + destx; 

    for ( ; height>0 ; height--) 
    { 
        memcpy(dest, src, width * sizeof(*dest));
        src += screenwidth; 
        dest += screenwidth; 
    } 
} 
 
// -----------------------------------------------------------------------------
// V_SetPatchClipCallback
//
// haleyjd 08/28/10: Added for Strife support.
// By calling this function, you can setup runtime error checking for patch 
// clipping. Strife never caused errors by drawing patches partway off-screen.
// Some versions of vanilla DOOM also behaved differently than the default
// implementation, so this could possibly be extended to those as well for
// accurate emulation.
// -----------------------------------------------------------------------------

void V_SetPatchClipCallback(vpatchclipfunc_t func)
{
    patchclip_callback = func;
}

// -----------------------------------------------------------------------------
// V_DrawPatch
// Masks a column based masked pic to the screen. 
// -----------------------------------------------------------------------------

void V_DrawPatch (int x, int y, const patch_t *patch, const byte *table)
{ 
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution
    byte *dest2, *desttop2; // High resolution
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if (patchclip_callback)
    {
        if (!patchclip_callback(patch, x, y))
        {
            return;
        }
    }

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop1++, desttop2++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
            const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

            source = sourcetrans = (byte *)column + 3;
            dest1 = desttop1 + column_post;
            dest2 = desttop2 + column_post;
            count = column->length;

            // [crispy] prevent framebuffer overflows
            {
                int tmpy = y + column->topdelta;

                // [crispy] too far left
                if (x < 0)
                {
                    continue;
                }

                // [crispy] too far right / width
                if (x >= origwidth)
                {
                    break;
                }

                // [crispy] too high
                while (tmpy < 0)
                {
                    count--;
                    source++;
                    sourcetrans++; // [Dasperal] Increment sourcetrans as well in case dp_translation is NULL
                    dest1 += (screenwidth << hires);
                    if (quadres)
                    dest2 += (screenwidth << hires);
                    tmpy++;
                }

                // [crispy] too low / height
                while (tmpy + count > ORIGHEIGHT)
                {
                    count--;
                }

                // [crispy] nothing left to draw?
                if (count < 1)
                {
                    continue;
                }
            }

            while (count--)
            {
                if (dp_translation)
                sourcetrans = &dp_translation[*source++];

                // [JN] If given table is a NULL, draw opaque patch.
                if (table != NULL)
                {
                    if (quadres)
                    {
                        *dest2 = *dest1 = table[((*dest1) << 8) + *sourcetrans];
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;

                        *dest2 = *dest1 = table[((*dest1) << 8) + *sourcetrans++];
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;
                    }
                    else
                    {
                        *dest1 = table[((*dest1) << 8) + *sourcetrans];
                        dest1 += fullscreenwidth;
                        *dest1 = table[((*dest1) << 8) + *sourcetrans++];
                        dest1 += fullscreenwidth;
                    }
                }
                else
                {
                    if (quadres)
                    {
                        *dest2 = *dest1 = *sourcetrans;
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;

                        *dest2 = *dest1 = *sourcetrans++;
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;
                    }
                    else
                    {
                        *dest1 = *sourcetrans;
                        dest1 += fullscreenwidth;
                        
                        *dest1 = *sourcetrans++;
                        dest1 += fullscreenwidth;
                    }
                }
            }
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawPatchFullScreen
// -----------------------------------------------------------------------------

void V_DrawPatchFullScreen (patch_t *patch, const boolean flipped)
{
    int x = ((screenwidth >> hires) - SHORT(patch->width)) / 2;

    patch->leftoffset = 0;
    patch->topoffset = 0;

    // [crispy] fill pillarboxes in widescreen mode
    if (screenwidth != ORIGWIDTH)
    {
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    if (flipped)
    {
        V_DrawPatchFlipped(x, 0, patch);
    }
    else
    {
        V_DrawPatch(x, 0, patch, NULL);
    }
}

// -----------------------------------------------------------------------------
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
// -----------------------------------------------------------------------------

void V_DrawPatchFlipped (int x, int y, const patch_t *patch)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution
    byte *dest2, *desttop2; // High resolution
    byte *source, *sourcetrans;
    int w, f;
 
    y -= SHORT(patch->topoffset); 
    x -= SHORT(patch->leftoffset); 

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

    V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop1++, desttop2++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));

        // step through the posts in a column
        while (column->topdelta != 0xff )
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
            const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

            source = sourcetrans = (byte *)column + 3;
            dest1 = desttop1 + column_post;
            dest2 = desttop2 + column_post;
            count = column->length;

            // [crispy] prevent framebuffer overflows
            {
                int tmpy = y + column->topdelta;

                // [crispy] too far left
                if (x < 0)
                {
                    continue;
                }

                // [crispy] too far right / width
                if (x >= origwidth)
                {
                    break;
                }

                // [crispy] too high
                while (tmpy < 0)
                {
                    count--;
                    source++;
                    dest1 += (screenwidth << hires);
                    if (quadres)
                    dest2 += (screenwidth << hires);
                    tmpy++;
                }

                // [crispy] too low / height
                while (tmpy + count > ORIGHEIGHT)
                {
                    count--;
                }

                // [crispy] nothing left to draw?
                if (count < 1)
                {
                    continue;
                }
            }

            while (count--)
            {
                if (dp_translation)
                sourcetrans = &dp_translation[*source++];

                if (quadres)
                {
                    *dest2 = *dest1 = *sourcetrans;
                    dest1 += fullscreenwidth;
                    dest2 += fullscreenwidth;

                    *dest2 = *dest1 = *sourcetrans++;
                    dest1 += fullscreenwidth;
                    dest2 += fullscreenwidth;
                }
                else
                {
                    *dest1 = *sourcetrans;
                    dest1 += fullscreenwidth;

                    *dest1 = *sourcetrans++;
                    dest1 += fullscreenwidth;
                }
            }
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawTLPatch
// Masks a column based translucent masked pic to the screen.
// -----------------------------------------------------------------------------

void V_DrawTLPatch (int x, int y, const patch_t *patch)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution (main patch)
    byte *dest2, *desttop2; // High resolution (main patch)
    byte *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;

    w = SHORT(patch->width);

    for (; col < w; x++, col++, desttop1++, desttop2++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
            const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

            source = (byte *) column + 3;
            dest1 = desttop1 + column_post;
            dest2 = desttop2 + column_post;
            count = column->length;

            while (count--)
            {
                if (quadres)
                {
                    *dest2 = *dest1 = tinttable[((*dest1) << 8) + *source];
                    dest1 += fullscreenwidth;
                    dest2 += fullscreenwidth;

                    *dest2 = *dest1 = tinttable[((*dest1) << 8) + *source++];
                    dest1 += fullscreenwidth;
                    dest2 += fullscreenwidth;
                }
                else
                {
                    *dest1 = tinttable[((*dest1) << 8) + *source];
                    dest1 += fullscreenwidth;
                    
                    *dest1 = tinttable[((*dest1) << 8) + *source++];
                    dest1 += fullscreenwidth;
                }
            }
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawXlaPatch
// villsa [STRIFE] Masks a column based translucent masked pic to the screen.
// -----------------------------------------------------------------------------

void V_DrawXlaPatch (int x, int y, const patch_t *patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

    col = 0;
    desttop = dest_screen + (y << hires) * screenwidth + x;

    w = SHORT(patch->width);
    for(; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while(column->topdelta != 0xff)
        {
            for (f = 0; f <= hires; f++)
            {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (screenwidth << hires) + (x * hires) + f;
            count = column->length;

            while(count--)
            {
                if (hires)
                {
                    *dest = xlatab[*dest + ((*source) << 8)];
                    dest += screenwidth;
                }
                *dest = xlatab[*dest + ((*source) << 8)];
                source++;
                dest += screenwidth;
            }
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawAltTLPatch
// Masks a column based translucent masked pic to the screen.
// -----------------------------------------------------------------------------

void V_DrawAltTLPatch (int x, int y, const patch_t *patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop = dest_screen + (y << hires) * screenwidth + x;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= hires; f++)
            {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (screenwidth << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest = tinttable[((*dest) << 8) + *source];
                    dest += screenwidth;
                }
                *dest = tinttable[((*dest) << 8) + *source++];
                dest += screenwidth;
            }
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawFadePatch
// [JN] Draws colorized with variable transluceny, with given tinting table.
// -----------------------------------------------------------------------------

void V_DrawFadePatch (int x, int y, const patch_t *patch, const byte *table)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution (main patch)
    byte *dest2, *desttop2; // High resolution (main patch)
    byte *dest3, *desttop3; // Middle resolution (shadow)
    byte *dest4, *desttop4; // High resolution (shadow)
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    w  = SHORT(patch->width);

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;
    
    if (draw_shadowed_text && !vanillaparm)
    {
        desttop3 = dest_screen + ((y + 1) << hires) * screenwidth + x + (2 << quadres);
        desttop4 = dest_screen + (((y + 1) << hires) + quadres) * screenwidth + x + (2 << quadres);
    }
    else
    {
        desttop3 = NULL;
        desttop4 = NULL;
    }
    
    for (; col < w; x++, col++, desttop1++, desttop2++, desttop3++, desttop4++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
                const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

                source = sourcetrans = (byte *) column + 3;
                dest1 = desttop1 + column_post;
                dest2 = desttop2 + column_post;

                if (draw_shadowed_text && !vanillaparm)
                {
                    dest3 = desttop3 + column_post;
                    dest4 = desttop4 + column_post;
                }
                else
                {
                    dest3 = NULL;
                    dest4 = NULL;
                }

                count = column->length;

                while (count--)
                {
                    if (dp_translation)
                    sourcetrans = &dp_translation[*source++];
    
                    if (quadres)
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            if (table == transtable90 || table == transtable80 || table == transtable70)
                            *dest4 = *dest3 = transtable30[((*dest3) << 8)];
                            if (table == transtable60 || table == transtable50 || table == transtable40)
                            *dest4 = *dest3 = transtable20[((*dest3) << 8)];
                            else
                            *dest4 = *dest3 = transtable10[((*dest3) << 8)];
                        
                            dest3 += fullscreenwidth;
                            dest4 += fullscreenwidth;
                        }
                        *dest2 = *dest1 = table[((*dest1) << 8) + *sourcetrans];
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            if (table == transtable90 || table == transtable80 || table == transtable70)
                            *dest4 = *dest3 = transtable30[((*dest3) << 8)];
                            if (table == transtable60 || table == transtable50 || table == transtable40)
                            *dest4 = *dest3 = transtable20[((*dest3) << 8)];
                            else
                            *dest4 = *dest3 = transtable10[((*dest3) << 8)];

                            dest3 += fullscreenwidth;
                            dest4 += fullscreenwidth;
                        }

                        *dest2 = *dest1 = table[((*dest1) << 8) + *sourcetrans++];
                        dest1 += fullscreenwidth;
                        dest2 += fullscreenwidth;
                    }
                    else
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            if (table == transtable90 || table == transtable80 || table == transtable70)
                            *dest3 = transtable30[((*dest3) << 8)];
                            if (table == transtable60 || table == transtable50 || table == transtable40)
                            *dest3 = transtable20[((*dest3) << 8)];
                            else
                            *dest3 = transtable10[((*dest3) << 8)];
                        
                            dest3 += fullscreenwidth;
                        }
                        *dest1 = table[((*dest1) << 8) + *sourcetrans];
                        dest1 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            if (table == transtable90 || table == transtable80 || table == transtable70)
                            *dest3 = transtable30[((*dest3) << 8)];
                            if (table == transtable60 || table == transtable50 || table == transtable40)
                            *dest3 = transtable20[((*dest3) << 8)];
                            else
                            *dest3 = transtable10[((*dest3) << 8)];

                            dest3 += fullscreenwidth;
                        }

                        *dest1 = table[((*dest1) << 8) + *sourcetrans++];
                        dest1 += fullscreenwidth;
                    }
                }
            }

            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawShadowedPatch
// Masks a column based masked pic to the screen.
// -----------------------------------------------------------------------------

void V_DrawShadowedPatch (int x, int y, const patch_t *patch)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution (main patch)
    byte *dest2, *desttop2; // High resolution (main patch)
    byte *dest3, *desttop3; // Middle resolution (shadow)
    byte *dest4, *desttop4; // High resolution (shadow)
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;
    desttop3 = dest_screen + ((y + 2) << hires) * screenwidth + x + (2 << quadres);
    desttop4 = dest_screen + (((y + 2) << hires) + quadres) * screenwidth + x + (2 << quadres);

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop1++, desttop2++, desttop3++, desttop4++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
                const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

                source = sourcetrans = (byte *) column + 3;
                dest1 = desttop1 + column_post;
                dest2 = desttop2 + column_post;
                dest3 = desttop3 + column_post;
                dest4 = desttop4 + column_post;

                count = column->length;

                // [crispy] prevent framebuffer overflows
                {
                    int tmpy = y + column->topdelta;

                    // [crispy] too far left
                    if (x < 0)
                    {
                        continue;
                    }

                    // [crispy] too far right / width
                    if (x >= origwidth)
                    {
                        break;
                    }

                    // [crispy] too high
                    while (tmpy < 0)
                    {
                        count--;
                        source++;
                        dest1 += (screenwidth << hires);
                        dest2 += (screenwidth << hires);
                        dest3 += (screenwidth << hires);
                        dest4 += (screenwidth << hires);
                        tmpy++;
                    }

                    // [crispy] too low / height
                    while (tmpy + count > ORIGHEIGHT)
                    {
                        count--;
                    }

                    // [crispy] nothing left to draw?
                    if (count < 1)
                    {
                        continue;
                    }
                }

                while (count--)
                {
                    if (dp_translation)
                    sourcetrans = &dp_translation[*source++];

                    if (quadres)
                    {
                        *dest4 = *dest3 = tinttable[((*dest3) << 8)];
                        dest4 += fullscreenwidth;
                        dest3 += fullscreenwidth;

                        *dest2 = *dest1 = *sourcetrans;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;

                        *dest4 = *dest3 = tinttable[((*dest3) << 8)];
                        dest4 += fullscreenwidth;
                        dest3 += fullscreenwidth;

                        *dest2 = *dest1 = *sourcetrans++;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;
                    }
                    else
                    {
                        *dest3 = tinttable[((*dest3) << 8)];
                        dest3 += fullscreenwidth;

                        *dest1 = *sourcetrans;
                        dest1 += fullscreenwidth;

                        *dest3 = tinttable[((*dest3) << 8)];
                        dest3 += fullscreenwidth;

                        *dest1 = *sourcetrans++;
                        dest1 += fullscreenwidth;
                    }
                }
            }

            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawShadowedPatchDoom
// [JN] Separated function for Doom, shadow size decreased to one pixel.
// -----------------------------------------------------------------------------

void V_DrawShadowedPatchDoom (int x, int y, const patch_t *patch)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution (main patch)
    byte *dest2, *desttop2; // High resolution (main patch)
    byte *dest3, *desttop3; // Middle resolution (shadow)
    byte *dest4, *desttop4; // High resolution (shadow)
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;
    
    if (draw_shadowed_text && !vanillaparm)
    {
        desttop3 = dest_screen + ((y + 1) << hires) * screenwidth + x + (2 << quadres);
        desttop4 = dest_screen + (((y + 1) << hires) + quadres) * screenwidth + x + (2 << quadres);
    }
    else
    {
        desttop3 = NULL;
        desttop4 = NULL;
    }

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop1++, desttop2++, desttop3++, desttop4++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
                const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

                source = sourcetrans = (byte *) column + 3;
                dest1 = desttop1 + column_post;
                dest2 = desttop2 + column_post;

                if (draw_shadowed_text && !vanillaparm)
                {
                    dest3 = desttop3 + column_post;
                    dest4 = desttop4 + column_post;
                }
                else 
                {
                    dest3 = NULL;
                    dest4 = NULL;
                }

                count = column->length;

                // [crispy] prevent framebuffer overflows
                {
                    int tmpy = y + column->topdelta;

                    // [crispy] too far left
                    if (x < 0)
                    {
                        continue;
                    }

                    // [crispy] too far right / width
                    if (x >= origwidth)
                    {
                        break;
                    }

                    // [crispy] too high
                    while (tmpy < 0)
                    {
                        count--;
                        source++;
                        dest1 += (screenwidth << hires);
                        dest2 += (screenwidth << hires);
                        dest3 += (screenwidth << hires);
                        dest4 += (screenwidth << hires);
                        tmpy++;
                    }

                    // [crispy] too low / height
                    while (tmpy + count > ORIGHEIGHT)
                    {
                        count--;
                    }

                    // [crispy] nothing left to draw?
                    if (count < 1)
                    {
                        continue;
                    }
                }

                while (count--)
                {
                    if (dp_translation)
                    sourcetrans = &dp_translation[*source++];

                    if (quadres)
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest4 = *dest3 = transtable60[((*dest3) << 8)];
                            dest4 += fullscreenwidth;
                            dest3 += fullscreenwidth;
                        }
                        *dest2 = *dest1 = *sourcetrans;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest4 = *dest3 = transtable60[((*dest3) << 8)];
                            dest4 += fullscreenwidth;
                            dest3 += fullscreenwidth;
                        }
                        *dest2 = *dest1 = *sourcetrans++;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;
                    }
                    else
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest3 = transtable60[((*dest3) << 8)];
                            dest3 += fullscreenwidth;
                        }
                        *dest1 = *sourcetrans;
                        dest1 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest3 = transtable60[((*dest3) << 8)];
                            dest3 += fullscreenwidth;
                        }
                        *dest1 = *sourcetrans++;
                        dest1 += fullscreenwidth;
                    }
                }
            }

            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawShadowedPatchRaven
// [JN] Separated function for Heretic and Hexen, 
// shadow size decreased to one pixel.
// -----------------------------------------------------------------------------

void V_DrawShadowedPatchRaven (int x, int y, const patch_t *patch)
{
    int col, count;
    column_t *column;
    byte *dest1, *desttop1; // Middle resolution (main patch)
    byte *dest2, *desttop2; // High resolution (main patch)
    byte *dest3, *desttop3; // Middle resolution (shadow)
    byte *dest4, *desttop4; // High resolution (shadow)
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop1 = dest_screen + (y << hires) * screenwidth + x;
    desttop2 = dest_screen + ((y << hires) + quadres) * screenwidth + x;
    
    if (draw_shadowed_text && !vanillaparm)
    {
        desttop3 = dest_screen + ((y + 1) << hires) * screenwidth + x + (2 << quadres);
        desttop4 = dest_screen + (((y + 1) << hires) + quadres) * screenwidth + x + (2 << quadres);
    }
    else
    {
        desttop3 = NULL;
        desttop4 = NULL;
    }

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop1++, desttop2++, desttop3++, desttop4++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= (hires + quadres); f++)
            {
                const int column_post = column->topdelta * (screenwidth << hires) + (x * (hires + quadres)) + f;

                source = sourcetrans = (byte *) column + 3;
                dest1 = desttop1 + column_post;
                dest2 = desttop2 + column_post;

                if (draw_shadowed_text && !vanillaparm)
                {
                    dest3 = desttop3 + column_post;
                    dest4 = desttop4 + column_post;
                }
                else 
                {
                    dest3 = NULL;
                    dest4 = NULL;
                }

                count = column->length;

                // [crispy] prevent framebuffer overflows
                {
                    int tmpy = y + column->topdelta;

                    // [crispy] too far left
                    if (x < 0)
                    {
                        continue;
                    }

                    // [crispy] too far right / width
                    if (x >= origwidth)
                    {
                        break;
                    }

                    // [crispy] too high
                    while (tmpy < 0)
                    {
                        count--;
                        source++;
                        dest1 += (screenwidth << hires);
                        dest2 += (screenwidth << hires);
                        dest3 += (screenwidth << hires);
                        dest4 += (screenwidth << hires);
                        tmpy++;
                    }

                    // [crispy] too low / height
                    while (tmpy + count > ORIGHEIGHT)
                    {
                        count--;
                    }

                    // [crispy] nothing left to draw?
                    if (count < 1)
                    {
                        continue;
                    }
                }

                while (count--)
                {
                    if (dp_translation)
                    sourcetrans = &dp_translation[*source++];

                    if (quadres)
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest4 = *dest3 = tinttable[((*dest3) << 8)];
                            dest4 += fullscreenwidth;
                            dest3 += fullscreenwidth;
                        }
                        *dest2 = *dest1 = *sourcetrans;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest4 = *dest3 = tinttable[((*dest3) << 8)];
                            dest4 += fullscreenwidth;
                            dest3 += fullscreenwidth;
                        }
                        *dest2 = *dest1 = *sourcetrans++;
                        dest2 += fullscreenwidth;
                        dest1 += fullscreenwidth;
                    }
                    else
                    {
                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest3 = tinttable[((*dest3) << 8)];
                            dest3 += fullscreenwidth;
                        }
                        *dest1 = *sourcetrans;
                        dest1 += fullscreenwidth;

                        if (draw_shadowed_text && !vanillaparm)
                        {
                            *dest3 = tinttable[((*dest3) << 8)];
                            dest3 += fullscreenwidth;
                        }
                        *dest1 = *sourcetrans++;
                        dest1 += fullscreenwidth;
                    }
                }
            }

            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawShadowedPatchStrife
// [JN] Separated function for Strife, shadow size decreased to one pixel.
// -----------------------------------------------------------------------------

void V_DrawShadowedPatchStrife (int x, int y, const patch_t *patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source, *sourcetrans;
    byte *desttop2, *dest2;
    int w, f;

    tinttable = W_CacheLumpName("XLATAB", PU_STATIC);

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;
    desttop2 = dest_screen + ((y + 1) << hires) * SCREENWIDTH + x + 2;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++, desttop2++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = sourcetrans = (byte *) column + 3;
            dest = desttop + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            dest2 = desttop2 + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (dp_translation)
                sourcetrans = &dp_translation[*source++];

                if (hires)
                {
                    *dest2 = tinttable[((*dest2) << 8)];
                    dest2 += SCREENWIDTH;
                    *dest = *sourcetrans;
                    dest += SCREENWIDTH;
                }
                *dest2 = tinttable[((*dest2) << 8)];
                dest2 += SCREENWIDTH;
                *dest = *sourcetrans++;
                dest += SCREENWIDTH;

            }
          }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// V_DrawPatchUnscaled
// [JN] hires independent version of V_DrawPatch.
// -----------------------------------------------------------------------------

void V_DrawPatchUnscaledHigh (int x, int y, const patch_t *patch, const byte *table)
{
    int count;
    int col;
    column_t *column;
    byte *desttop;
    byte *dest;
    byte *source;
    byte *sourcetrans;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    w  = SHORT(patch->width);

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + y * screenwidth + x;

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column->topdelta*screenwidth;
            count = column->length;

            // [crispy] prevent framebuffer overflows
            {
                int tmpy = y + column->topdelta;

                // [crispy] too far left
                if (x < 0)
                {
                    continue;
                }

                // [crispy] too far right / width
                if (x >= screenwidth)
                {
                    break;
                }

                // [crispy] too high
                while (tmpy < 0)
                {
                    count--;
                    source++;
                    sourcetrans++; // [Dasperal] Increment sourcetrans as well in case dp_translation is NULL
                    dest += screenwidth;
                    tmpy++;
                }

                // [crispy] too low / height
                while (tmpy + count > screenwidth)
                {
                    count--;
                }

                // [crispy] nothing left to draw?
                if (count < 1)
                {
                    continue;
                }
            }


            while (count--)
            {
                if (dp_translation)
                sourcetrans = &dp_translation[*source++];

                // [JN] If given table is a NULL, draw opaque patch.
                if (table != NULL)
                *dest = table[((*dest) << 8) + *sourcetrans++];
                else
                *dest = *sourcetrans++;    

                dest += fullscreenwidth >> quadres;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchUnscaledQuad (int x, int y, const patch_t *patch, const byte *table)
{
    int col, count;
    column_t *column;
    byte *dest, *desttop;
    byte *source, *sourcetrans;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + (y << quadres) * screenwidth + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= quadres; f++)
            {
            const int column_post = column->topdelta * (screenwidth << quadres) + x + f;

            source = sourcetrans = (byte *)column + 3;
            dest = desttop + column_post;
            count = column->length;

            // [crispy] prevent framebuffer overflows
            {
                int tmpy = y + column->topdelta;

                // [crispy] too far left
                if (x < 0)
                {
                    continue;
                }

                // [crispy] too far right / width
                if (x >= screenwidth)
                {
                    break;
                }

                // [crispy] too high
                while (tmpy < 0)
                {
                    count--;
                    source++;
                    sourcetrans++; // [Dasperal] Increment sourcetrans as well in case dp_translation is NULL
                    dest += (screenwidth << hires);
                    tmpy++;
                }

                // [crispy] too low / height
                while (tmpy + count > SCREENHEIGHT)
                {
                    count--;
                }

                // [crispy] nothing left to draw?
                if (count < 1)
                {
                    continue;
                }
            }

            while (count--)
            {
                if (dp_translation)
                sourcetrans = &dp_translation[*source++];

                // [JN] If given table is a NULL, draw opaque patch.
                if (table != NULL)
                {
                    *dest = table[((*dest) << 8) + *sourcetrans];
                    dest += screenwidth;

                    *dest = table[((*dest) << 8) + *sourcetrans++];
                    dest += screenwidth;

                }
                else
                {
                    *dest = *sourcetrans;
                    dest += screenwidth;

                    *dest = *sourcetrans++;
                    dest += screenwidth;
                }
            }
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] V_DrawPatchFinale
// Draws pixel-doubled sprite. Used exclusively on casting sequence screen.
// Written with extensive support of Fabian Greffrath, thanks! (16.01.2019)
// -----------------------------------------------------------------------------

void V_DrawPatchFinale (int x, int y, const patch_t *patch)
{ 
    int       count, col, w, f;
    column_t *column;
    byte     *desttop;
    byte     *dest;
    byte     *source;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen 
            + (y * 4)                   // Proper vertical offset for Y * 4 factor
            * screenwidth + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            for (f = 0; f <= 3; f++)    // Make X scale factor * 4 (0, 1, 2, 3)
            {
            source = (byte *)column + 3;

            dest = desttop 
                 + column->topdelta 
                 * (screenwidth * 4)    // Scale Y by 4
                 + (x * 3)              // Scale X by 4
                 + f;

            count = column->length;

            while (count--)
            {
                int g;

                for (g = 0; g <= 3; g++)
                {
                    *dest = *source;
                    dest += screenwidth;
                }
                source++;
            }

            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

// -----------------------------------------------------------------------------
// [JN] V_FillFlat
// Fills background with given flat, with support for high/low detail mode.
// -----------------------------------------------------------------------------

void V_FillFlat (char *lump)
{
    int x, y;
    const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;
    byte *src = W_CacheLumpName (DEH_String(lump), PU_CACHE);
    byte *dest = I_VideoBuffer;

    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < screenwidth; x++)
        {
            *dest++ = src[(((y >> shift_allowed) & 63) << 6) 
                         + ((x >> shift_allowed) & 63)];
        }
    }
}

// -----------------------------------------------------------------------------
// Load tint table from TINTTAB lump.
// -----------------------------------------------------------------------------

void V_LoadTintTable (void)
{
    tinttable = W_CacheLumpName("TINTTAB", PU_STATIC);
}

// -----------------------------------------------------------------------------
// V_LoadXlaTable
// villsa [STRIFE] Load xla table from XLATAB lump.
// -----------------------------------------------------------------------------

void V_LoadXlaTable (void)
{
    xlatab = W_CacheLumpName("XLATAB", PU_STATIC);
}

// -----------------------------------------------------------------------------
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
// -----------------------------------------------------------------------------

void V_DrawBlock (const int x, const int y, const int width, int height, const byte *src)
{ 
    byte *dest; 
 
#ifdef RANGECHECK 
    if (x < 0 || x + width > screenwidth || y < 0 || y + height > SCREENHEIGHT)
    {
        I_Error (english_language ?
                "Bad V_DrawBlock" :
                "Ошибка V_DrawBlock");
    }
#endif 

    V_MarkRect (x, y, width, height); 

    dest = dest_screen + (y << hires) * screenwidth + x;

    while (height--) 
    { 
        memcpy (dest, src, width * sizeof(*dest));
        src += width; 
        dest += screenwidth; 
    } 
} 

void V_DrawScaledBlock (const int x, const int y, int width, int height, const byte *src)
{
    byte *dest;
    int i, j;

#ifdef RANGECHECK
    if (x < 0
     || x + width > origwidth
     || y < 0
     || y + height > ORIGHEIGHT)
    {
	I_Error (english_language ?
             "Bad V_DrawScaledBlock" :
             "Ошибка V_DrawScaledBlock");
    }
#endif

    V_MarkRect (x, y, width, height);

    dest = dest_screen + (y << hires) * screenwidth + (x << hires);

    for (i = 0; i < (height << hires); i++)
    {
        for (j = 0; j < (width << hires); j++)
        {
            *(dest + i * screenwidth + j) = *(src + (i >> hires) * width + (j >> hires));
        }
    }
}

void V_DrawFilledBox (const int x, const int y, const int w, const int h, const int c)
{
    uint8_t *buf, *buf1;
    int x1, y1;

    buf = I_VideoBuffer + screenwidth * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        buf1 = buf;

        for (x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = c;
        }

        buf += screenwidth;
    }
}

void V_DrawHorizLine (const int x, const int y, const int w, const int c)
{
    uint8_t *buf;
    int x1;

    buf = I_VideoBuffer + screenwidth * y + x;

    for (x1 = 0; x1 < w; ++x1)
    {
        *buf++ = c;
    }
}

void V_DrawVertLine (const int x, const int y, const int h, const int c)
{
    uint8_t *buf;
    int y1;

    buf = I_VideoBuffer + screenwidth * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        *buf = c;
        buf += screenwidth;
    }
}

void V_DrawBox (const int x, const int y, const int w, const int h, const int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y+h-1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x+w-1, y, h, c);
}

// -----------------------------------------------------------------------------
// V_CopyScaledBuffer
// Draw a "raw" screen (lump containing raw data to blit directly to the screen)
// -----------------------------------------------------------------------------

void V_CopyScaledBuffer (byte *dest, const byte *src, const size_t size)
{
    int i, j, k;

#ifdef RANGECHECK
    if (size < 0
     || size > origwidth * ORIGHEIGHT)
    {
        I_Error(english_language ?
                "Bad V_CopyScaledBuffer" :
                "Ошибка V_CopyScaledBuffer");
    }
#endif

    for (k = 0; k < size; k++)
    {
        const int l = k / ORIGWIDTH; // current line in the source screen
        const int p = k - l * ORIGWIDTH; // current pixel in this line
        for (i = 0; i <= (hires + quadres); i++)
        {
            for (j = 0; j <= (hires + quadres); j++)
            {
                *(dest + (p << hires) + ((l << hires) + i) * screenwidth + j 
                       + (wide_delta << hires)) = *(src + k);
            }
        }
    }
}
 
void V_DrawRawScreen (const byte *raw)
{
    V_CopyScaledBuffer(dest_screen, raw, ORIGWIDTH * ORIGHEIGHT);
}

// -----------------------------------------------------------------------------
// V_Init
// [JN] Used for setting aspect ratio variables: width, height and deltas.
// -----------------------------------------------------------------------------

void V_Init (void) 
{ 
    if (rendering_resolution)
    {
        // If running in middle or high resolution,
        // use actial hires value and don't use low detail mode.
        hires = rendering_resolution;
        detailshift = 0;
        
        if (hires == 2)
        {
            quadres = 1;
            V_DrawPatchUnscaled = V_DrawPatchUnscaledQuad;
        }
        else
        {
            V_DrawPatchUnscaled = V_DrawPatchUnscaledHigh;
        }
    }
    else
    {
        // Else, emitate low resolution by using middle resolution
        // and low detail mode.
        hires = 1;
        detailshift = 1;
        V_DrawPatchUnscaled = V_DrawPatchUnscaledHigh;
    }

    SCREENWIDTH = ORIGWIDTH << hires;
    SCREENHEIGHT = ORIGHEIGHT << hires;

    if (aspect_ratio == 0)
    {
        // 4:3
        origwidth = ORIGWIDTH;
        screenwidth = SCREENWIDTH;
        wide_delta = 0;
    }
    else if (aspect_ratio == 1)
    {
        // 5:4
        origwidth = ORIGWIDTH;
        screenwidth = SCREENWIDTH;
        wide_delta = 0;
        actualheight = 256 << hires;
    }
    else if (aspect_ratio == 2)
    {
        // 16:9
        origwidth = 426;
        screenwidth = 426 << hires;
        wide_delta = 53;
    }
    else if (aspect_ratio == 3)
    {
        // 16:10
        origwidth = 384;
        screenwidth = 384 << hires;
        wide_delta = 32;
    }
    else if (aspect_ratio == 4)
    {
        // 21:9
        origwidth = 560;
        screenwidth = 560 << hires;
        wide_delta = 120;
    }

    screenwidth_low = screenwidth << hires;

    if (preserve_window_aspect_ratio)
    {
        if (aspect_ratio == 1)
        actualheight = 256 << hires;
        else
        actualheight = 240 << hires;
    }
    else
    {
        actualheight = SCREENHEIGHT;
    }

    fullscreenwidth = screenwidth * hires;
}

// Set the buffer that the code draws to.

void V_UseBuffer (byte *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer (void)
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//

typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} PACKEDATTR pcx_t;


//
// WritePCXfile
//

void WritePCXfile (const char *filename, const byte *data,
                   const int width, const int height,
                   const byte *palette)
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->reserved = 0;                  // PCX spec: reserved byte must be zero
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}

//
// WritePNGfile
//

void WritePNGfile(char *filename)
{
    extern void I_RenderReadPixels(byte **data, int *w, int *h);

    byte* data;
    int width, height;

    I_RenderReadPixels(&data, &width, &height);

    size_t png_data_size = 0;
    void *pPNG_data = tdefl_write_image_to_png_file_in_memory(data, width, height, 4, &png_data_size);
    if(!pPNG_data)
    {
        return;
    }
    else
    {
        FILE *handle = fopen(filename, "wb");
        fwrite(pPNG_data, 1, png_data_size, handle);
        fclose(handle);
        mz_free(pPNG_data);
    }
    free(data);
}

//
// V_ScreenShot
//

void V_ScreenShot (const char *format)
{
    int i;
    char lbmname[32]; // haleyjd 20110213: BUG FIX - 12 is too small!
    char *ext;        // [JN] lbmname extended from 16 to 32
    
    // find a file name to save it to

    if (png_screenshots)
    {
        ext = "png";
    }
    else
    {
        ext = "pcx";
    }

    for (i=0; i<=9999; i++) // [crispy] increase screenshot filename limit
    {
        M_snprintf(lbmname, sizeof(lbmname), format, i, ext);

        if (!M_FileExists(lbmname))
        {
            break;      // file doesn't exist
        }
    }

    if (i == 10000) // [crispy] increase screenshot filename limit
    {
        if (png_screenshots)
        {
            I_Error (english_language ? 
                     "V_ScreenShot: Couldn't create a PNG" :
                     "V_ScreenShot: ошибка сохранения PNG скриншота");
        }
        else
        {
            I_Error (english_language ?
            "V_ScreenShot: Couldn't create a PCX" :
            "V_ScreenShot: ошибка сохранения PCX скриншота");
        }
    }

    if(png_screenshots)
    {
        WritePNGfile(lbmname);
    }
    else
    {
    // save the pcx file
    WritePCXfile(lbmname, I_VideoBuffer,
                screenwidth, SCREENHEIGHT,
                W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
    }
}

// -----------------------------------------------------------------------------
// V_DrawMouseSpeedBox
//
// If box is only to calibrate speed, testing relative speed (as a measure
// of game pixels to movement units) is important whether physical mouse DPI
// is high or low. Line resolution starts at 1 pixel per 1 move-unit: if
// line maxes out, resolution becomes 1 pixel per 2 move-units, then per
// 3 move-units, etc.
// -----------------------------------------------------------------------------

#define MOUSE_SPEED_BOX_WIDTH  120
#define MOUSE_SPEED_BOX_HEIGHT 9

static int linelen_multiplier = 1;

void V_DrawMouseSpeedBox (const int speed)
{
    extern int usemouse;
    int bgcolor, bordercolor, red, black, white, yellow;
    int box_x, box_y;
    int original_speed;
    int redline_x;
    int linelen;
    int i;
    boolean draw_acceleration = false;

    // Get palette indices for colors for widget. These depend on the
    // palette of the game being played.

    bgcolor = I_GetPaletteIndex(0x77, 0x77, 0x77);
    bordercolor = I_GetPaletteIndex(0x55, 0x55, 0x55);
    red = I_GetPaletteIndex(0xff, 0x00, 0x00);
    black = I_GetPaletteIndex(0x00, 0x00, 0x00);
    yellow = I_GetPaletteIndex(0xff, 0xff, 0x00);
    white = I_GetPaletteIndex(0xff, 0xff, 0xff);

    // If the mouse is turned off, don't draw the box at all.
    if (!usemouse)
    {
        return;
    }

    // If acceleration is used, draw a box that helps to calibrate the
    // threshold point.
    if (mouse_threshold > 0 && fabs(mouse_acceleration - 1) > 0.01)
    {
        draw_acceleration = true;
    }

    // Calculate box position

    box_x = screenwidth - MOUSE_SPEED_BOX_WIDTH - 10;
    box_y = 15;

    V_DrawFilledBox(box_x, box_y,
                    MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bgcolor);
    V_DrawBox(box_x, box_y,
              MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bordercolor);

    // Calculate the position of the red threshold line when calibrating
    // acceleration.  This is 1/3 of the way along the box.

    redline_x = MOUSE_SPEED_BOX_WIDTH / 3;

    // Calculate line length

    if (draw_acceleration && speed >= mouse_threshold)
    {
        // Undo acceleration and get back the original mouse speed
        original_speed = speed - mouse_threshold;
        original_speed = (int) (original_speed / mouse_acceleration);
        original_speed += mouse_threshold;

        linelen = (original_speed * redline_x) / mouse_threshold;
    }
    else
    {
        linelen = speed / linelen_multiplier;
    }

    // Draw horizontal "thermometer" 

    if (linelen > MOUSE_SPEED_BOX_WIDTH - 1)
    {
        linelen = MOUSE_SPEED_BOX_WIDTH - 1;
        if (!draw_acceleration)
        {
            linelen_multiplier++;
        }
    }

    V_DrawHorizLine(box_x + 1, box_y + 4, MOUSE_SPEED_BOX_WIDTH - 2, black);

    if (!draw_acceleration || linelen < redline_x)
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen, white);
    }
    else
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        redline_x, white);
        V_DrawHorizLine(box_x + redline_x, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen - redline_x, yellow);
    }

    if (draw_acceleration)
    {
        // Draw acceleration threshold line
        V_DrawVertLine(box_x + redline_x, box_y + 1,
                       MOUSE_SPEED_BOX_HEIGHT - 2, red);
    }
    else
    {
        // Draw multiplier lines to indicate current resolution
        for (i = 1; i < linelen_multiplier; i++)
        {
            V_DrawVertLine(
                box_x + (i * MOUSE_SPEED_BOX_WIDTH / linelen_multiplier),
                box_y + 1, MOUSE_SPEED_BOX_HEIGHT - 2, yellow);
        }
    }
}


//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
// R_draw.c



#include "doomdef.h"
#include "deh_str.h"
#include "r_local.h"
#include "i_video.h"
#include "v_video.h"
#include "jn.h"

/*

All drawing to the view buffer is accomplished in this file.  The other refresh
files only know about ccordinates, not the architecture of the frame buffer.

*/

byte *viewimage;
int viewwidth, scaledviewwidth, viewheight, scaledviewheight, viewwindowx, viewwindowy;
byte *ylookup[SCREENHEIGHT];
int columnofs[WIDESCREENWIDTH];
byte translations[3][256];      // color tables for different players

/*
==================
=
= R_DrawColumn
=
= Source is the top of the column to scale
=
==================
*/

lighttable_t *dc_colormap;
int dc_x;
int dc_yl;
int dc_yh;
fixed_t dc_iscale;
fixed_t dc_texturemid;
int dc_texheight;
byte *dc_source;                // first pixel in a column (possibly virtual)

int dccount;                    // just for profiling

void R_DrawColumn(void)
{
    int      count;
    byte    *dest;   // killough
    fixed_t  frac;   // killough
    fixed_t  fracstep;

    count = dc_yh - dc_yl + 1;

    if (count <= 0)    // Zero length, column does not exceed a pixel.
    return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error (english_language ?
                 "R_DrawColumn: %i to %i at %i" :
                 "R_DrawColumn: %i к %i в %i",
                 dc_yl, dc_yh, dc_x);
#endif

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?

    dest = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];

    // Determine scaling, which is the only mapping to be done.

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.       (Yeah, right!!! -- killough)
    //
    // killough 2/1/98: more performance tuning

    {
        const byte *source = dc_source;
        const lighttable_t *colormap = dc_colormap;
        int heightmask = dc_texheight-1;
        if (dc_texheight & heightmask)   // not a power of 2 -- killough
        {
            heightmask++;
            heightmask <<= FRACBITS;

            if (frac < 0)
                while ((frac += heightmask) < 0);
            else
                while (frac >= heightmask)
                frac -= heightmask;

            do
            {
                // Re-map color indices from wall texture column
                //  using a lighting/special effects LUT.

                // heightmask is the Tutti-Frutti fix -- killough

                *dest = colormap[source[frac>>FRACBITS]];
                dest += screenwidth;                     // killough 11/98
                if ((frac += fracstep) >= heightmask)
                    frac -= heightmask;
            }
            while (--count);
        }
        else
        {
            while ((count-=2)>=0)   // texture height is a power of 2 -- killough
            {
                *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
                dest += screenwidth;   // killough 11/98
                frac += fracstep;
                *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
                dest += screenwidth;   // killough 11/98
                frac += fracstep;
            }
            if (count & 1)
                *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
        }
    }
}

/*
================================================================================
=
= R_DrawSkyColumn
=
= [JN] Draws sky texture column.
=
================================================================================
*/

void R_DrawSkyColumn(void)
{
    int   count = dc_yh - dc_yl + 1;
    int   fracstep = FRACUNIT >> hires;
    int   frac  = skytexturemid + (dc_yl - centery) * fracstep;
    byte *dest  = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];
    // [JN] Tutti-Frutti fix for sky texture.
    // Use actual texture height (200), instead of dc_texheight (128).
    const int sky_texheight = 200;
    int       heightmask = sky_texheight-1; 

    if (count <= 0)
    return;

    if (sky_texheight & heightmask)   // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
            frac -= heightmask;

        do
        {
           *dest = dc_colormap[dc_source[frac>>FRACBITS]];
            dest += screenwidth;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        }
        while (--count);
    }
    else
    {
        while ((count-=2)>=0)   // texture height is a power of 2 -- killough
        {
           *dest = dc_colormap[dc_source[(frac>>FRACBITS) & heightmask]];
            dest += screenwidth;
            frac += fracstep;
           *dest = dc_colormap[dc_source[(frac>>FRACBITS) & heightmask]];
            dest += screenwidth;
            frac += fracstep;
        }
        if (count & 1)
           *dest = dc_colormap[dc_source[(frac>>FRACBITS) & heightmask]];
    }
}

/*
================================================================================
=
= R_DrawSkyColumn
=
= [JN] Low detail version of R_DrawSkyColumn.
=
================================================================================
*/

void R_DrawSkyColumnLow(void)
{
    int   count = dc_yh - dc_yl + 1;
    int   fracstep = FRACUNIT >> (hires && !detailshift);
    int   frac  = skytexturemid + (dc_yl - centery) * fracstep;
    byte *dest  = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(dc_x << hires)]];
    byte *dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(dc_x << hires) + 1]];
    byte *dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(dc_x << hires)]];
    byte *dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(dc_x << hires) + 1]];
    // [JN] Tutti-Frutti fix for sky texture.
    // Use actual texture height (200), instead of dc_texheight (128).
    const int sky_texheight = 200;
    int       heightmask = sky_texheight-1; 

    if (count <= 0)
    return;

    if (sky_texheight & heightmask)   // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
            frac -= heightmask;

        do
        {
           *dest4 = *dest3 = *dest2 = *dest = dc_colormap[dc_source[frac >> FRACBITS]];
            dest  += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        }
        while (--count);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
           *dest4 = *dest3 = *dest2 = *dest = dc_colormap[dc_source[(frac >> FRACBITS) & heightmask]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;
            frac += fracstep; 
        } while (count--);
    }
}

/*
================================================================================
=
= R_DrawColumnLow
=
= [JN] Low detail version of R_DrawColumn.
=
================================================================================
*/

void R_DrawColumnLow(void)
{
    int      count; 
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      heightmask = dc_texheight - 1;
    byte    *dest = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x]];
    byte    *dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x+1]];
    byte    *dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x]];
    byte    *dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x+1]];
    fixed_t  fracstep = dc_iscale; 
    fixed_t  frac = dc_texturemid + (dc_yl-centery)*fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumnLow: %i to %i at %i" :
                "R_DrawColumnLow: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest2 = *dest = dc_colormap[dc_source[frac>>FRACBITS]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest4 = *dest3 = dc_colormap[dc_source[frac>>FRACBITS]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            *dest2 = *dest = dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest4 = *dest3 = dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            frac += fracstep; 

        } while (count--);
    }
}

// Translucent column draw - blended with background using tinttable.

void R_DrawTLColumn(void)
{
    int count;
    byte*   dest;
    fixed_t frac;
    fixed_t fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumn: %i to %i at %i" :
                 "R_DrawTLColumn: %i к %i у %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    do
    {
        *dest = tinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
        dest += screenwidth;

        frac += fracstep;
    } while (count--);
}

/*
================================================================================
=
= R_DrawColumnLow
=
= [JN] Draw translucent column, low-resolution version.
=
================================================================================
*/

void R_DrawTLColumnLow (void)
{
    int     count;
    int     x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int     heightmask = dc_texheight - 1;
    byte*   dest = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x]];
    byte*   dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x+1]];
    byte*   dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x]];
    byte*   dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x+1]];
    fixed_t fracstep = dc_iscale;
    fixed_t frac = dc_texturemid + (dc_yl-centery)*fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumnLow: %i to %i at %i" :
                 "R_DrawTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = tinttable[(*dest3<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
                *dest4 = tinttable[(*dest4<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = tinttable[(*dest3<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
                *dest4 = tinttable[(*dest4<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            frac += fracstep; 

        } while (count--);
    }
}

/*
================================================================================
=
= R_DrawExtraTLColumn
=
= [JN] Draw extra translucent column. With Tutti-Frutti fix by Lee Killough.
=
================================================================================
*/

void R_DrawExtraTLColumn(void)
{
    int      count, heightmask = dc_texheight-1;
    fixed_t  fracstep = dc_iscale;
    fixed_t  frac = dc_texturemid + (dc_yl-centery)*fracstep;
    byte    *dest = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];

    count = dc_yh - dc_yl + 1;

    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumn: %i to %i at %i" :
                 "R_DrawTLColumn: %i к %i у %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif

    if (dc_texheight & heightmask)   // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            dest += screenwidth;
            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        }
        while (--count);
    }
    else    // texture height is a power of 2 -- killough
    {
        while ((count-=2)>=0)
        {
            *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS & heightmask]]];
            dest += screenwidth;
            frac += fracstep;
            *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS & heightmask]]];
            dest += screenwidth;
            frac += fracstep;
        }
        if (count & 1)
        *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS & heightmask]]];
    }
}

/*
================================================================================
=
= R_DrawExtraTLColumn
=
= [JN] Draw extra translucent column, low-resolution version. 
=
================================================================================
*/

void R_DrawExtraTLColumnLow (void)
{
    int      count;
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      heightmask = dc_texheight - 1;
    byte    *dest  = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x]];
    byte    *dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x+1]];
    byte    *dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x]];
    byte    *dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x+1]];
    fixed_t  fracstep = dc_iscale;
    fixed_t  frac = dc_texturemid + (dc_yl-centery)*fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawExtraTLColumnLow: %i to %i at %i" :
                 "R_DrawTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            *dest2 = extratinttable[(*dest2<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = extratinttable[(*dest3<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
                *dest4 = extratinttable[(*dest4<<8)+dc_colormap[dc_source[frac>>FRACBITS]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            *dest = extratinttable[(*dest<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest2 = extratinttable[(*dest2<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = extratinttable[(*dest3<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
                *dest4 = extratinttable[(*dest4<<8)+dc_colormap[dc_source[(frac>>FRACBITS)&heightmask]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            frac += fracstep; 

        } while (count--);
    }
}

/*
========================
=
= R_DrawTranslatedColumn
=
========================
*/

byte *dc_translation;
byte *translationtables;

void R_DrawTranslatedColumn(void)
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumn: %i to %i at %i" :
                "R_DrawColumn: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]];
        dest += screenwidth;
        frac += fracstep;
    }
    while (count--);
}

/*
================================================================================
=
= R_DrawTranslatedColumnLow
=
= [JN] Draw translated column, low-resolution version.
=
================================================================================
*/

void R_DrawTranslatedColumnLow (void) 
{ 
    int      count; 
    int      x = dc_x << 1;  // low detail, need to scale by 2
    byte    *dest  = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x]];
    byte    *dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x+1]];
    byte    *dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x]];
    byte    *dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x+1]];
    fixed_t  fracstep = dc_iscale;
    fixed_t  frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    count = dc_yh - dc_yl; 
    if (count < 0) 
    return; 

#ifdef RANGECHECK 
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTranslatedColumnLow: %i to %i at %i" :
                 "R_DrawTranslatedColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif 

    do 
    {
        *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        *dest2 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        dest += screenwidth << hires;
        dest2 += screenwidth << hires;
        if (hires)
        {
            *dest3 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
            *dest4 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;
        }
        frac += fracstep; 
    } while (count--); 
}

void R_DrawTranslatedTLColumn(void)
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumn: %i to %i at %i" :
                "R_DrawColumn: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[flipwidth[dc_x]];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = tinttable[((*dest) << 8)
                          +
                          dc_colormap[dc_translation
                                      [dc_source[frac >> FRACBITS]]]];
        dest += screenwidth;
        frac += fracstep;
    }
    while (count--);
}

/*
================================================================================
=
= R_DrawTranslatedTLColumnLow
=
= [JN] Draw translucent, translated column, low-resolution version.
=
================================================================================
*/

void R_DrawTranslatedTLColumnLow(void)
{
    int      count;
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      heightmask = dc_texheight - 1;
    byte    *dest  = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x]];
    byte    *dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[x+1]];
    byte    *dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x]];
    byte    *dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[x+1]];
    fixed_t  fracstep = dc_iscale;
    fixed_t  frac = dc_texturemid + (dc_yl-centery)*fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTranslatedTLColumnLow: %i to %i at %i" :
                 "R_DrawTranslatedTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        while ((frac += heightmask) < 0);
        else
        while (frac >= heightmask)
        frac -= heightmask;

        do
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = tinttable[(*dest3<<8)+dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]];
                *dest4 = tinttable[(*dest4<<8)+dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            if ((frac += fracstep) >= heightmask)
            frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
            dest += screenwidth << hires;
            dest2 += screenwidth << hires;

            if (hires)
            {
                *dest3 = tinttable[(*dest3<<8)+dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
                *dest4 = tinttable[(*dest4<<8)+dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
            }

            frac += fracstep; 

        } while (count--);
    }
}

//--------------------------------------------------------------------------
//
// PROC R_InitTranslationTables
//
//--------------------------------------------------------------------------

void R_InitTranslationTables(void)
{
    int i;

    V_LoadTintTable();

    // Allocate translation tables
    translationtables = Z_Malloc(256 * 3, PU_STATIC, 0);

    // Fill out the translation tables
    for (i = 0; i < 256; i++)
    {
        if (i >= 225 && i <= 240)
        {
            translationtables[i] = 114 + (i - 225);       // yellow
            translationtables[i + 256] = 145 + (i - 225); // red
            translationtables[i + 512] = 190 + (i - 225); // blue
        }
        else
        {
            translationtables[i] = translationtables[i + 256]
                = translationtables[i + 512] = i;
        }
    }
}

/*
================
=
= R_DrawSpan
=
================
*/

int ds_y;
int ds_x1;
int ds_x2;
lighttable_t *ds_colormap;
fixed_t ds_xfrac;
fixed_t ds_yfrac;
fixed_t ds_xstep;
fixed_t ds_ystep;
byte *ds_source;                // start of a 64*64 tile image

int dscount;                    // just for profiling

void R_DrawSpan(void)
{
    fixed_t xfrac, yfrac;
    byte *dest;
    int count, spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= screenwidth
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawSpan: %i to %i at %i" :
                "R_DrawSpan: %i к %i в %i",
                ds_x1, ds_x2, ds_y);
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    count = ds_x2 - ds_x1;
    do
    {
        spot = ((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63);
        dest = ylookup[ds_y] + columnofs[flipwidth[ds_x1++]];
        *dest = ds_colormap[ds_source[spot]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }
    while (count--);
}

void R_DrawSpanLow(void)
{
    unsigned int xtemp, ytemp;
    byte    *dest, *dest2;
    int     count;
    int     spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=screenwidth || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error(english_language ?
                "R_DrawSpan: %i to %i at %i" :
                "R_DrawSpan: %i к %i у %i",
                ds_x1,ds_x2,ds_y);
    }
#endif

    count = (ds_x2 - ds_x1);

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    // dest = ylookup[(ds_y << hires)] + columnofs[ds_x1];
    // dest2 = ylookup[(ds_y << hires) + 1] + columnofs[ds_x1];

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

        // Lowres/blocky mode does it twice,
        //  while scale is adjusted appropriately.
        dest = ylookup[(ds_y << hires)] + columnofs[flipwidth[ds_x1]];
        *dest = ds_colormap[ds_source[spot]];
        dest2 = ylookup[(ds_y << hires) + 1] + columnofs[flipwidth[ds_x1++]];
        *dest2 = ds_colormap[ds_source[spot]];
        dest = ylookup[(ds_y << hires)] + columnofs[flipwidth[ds_x1]];
        *dest = ds_colormap[ds_source[spot]];
        dest2 = ylookup[(ds_y << hires) + 1] + columnofs[flipwidth[ds_x1++]];
        *dest2 = ds_colormap[ds_source[spot]];

    // position += step;
    ds_xfrac += ds_xstep;
    ds_yfrac += ds_ystep;

    } while (count--);
}



/*
================
=
= R_InitBuffer
=
=================
*/

void R_InitBuffer(int width, int height)
{
    int i;

    viewwindowx = (screenwidth - width) >> 1;
    for (i = 0; i < width; i++)
        columnofs[i] = viewwindowx + i;
    if (width == screenwidth)
        viewwindowy = 0;
    else
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;
    for (i = 0; i < height; i++)
        ylookup[i] = I_VideoBuffer + (i + viewwindowy) * screenwidth;
}


/*
==================
=
= R_DrawViewBorder
=
= Draws the border around the view for different size windows
==================
*/

boolean BorderNeedRefresh;

void R_DrawViewBorder(void)
{
    byte *src, *dest;
    int x, y;
    const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

    if (scaledviewwidth == screenwidth)
        return;

    if (gamemode == shareware)
    {
        src = W_CacheLumpName(DEH_String("FLOOR04"), PU_CACHE);
    }
    else
    {
        src = W_CacheLumpName(DEH_String("FLAT513"), PU_CACHE);
    }
    dest = I_VideoBuffer;

    // [JN] Simplified, same to V_FillFlat now.
    for (y = 0; y < SCREENHEIGHT - SBARHEIGHT; y++)
    {
        for (x = 0; x < screenwidth; x++)
        {
            *dest++ = src[(((y >> shift_allowed) & 63) << 6) 
                         + ((x >> shift_allowed) & 63)];
        }
    }
    for (x = (viewwindowx >> hires); x < ((viewwindowx >> hires) + (scaledviewwidth >> hires)); x += 16)
    {
        V_DrawPatch(x, (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordt"), PU_CACHE));
        V_DrawPatch(x, (viewwindowy >> hires) + (scaledviewheight >> hires),
                    W_CacheLumpName(DEH_String("bordb"), PU_CACHE));
    }
    for (y = (viewwindowy >> hires); y < ((viewwindowy >> hires) + (scaledviewheight >> hires)); y += 16)
    {
        V_DrawPatch((viewwindowx >> hires) - 4, y,
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE));
        V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), y,
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE));
    }
    V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) - 4,
                W_CacheLumpName(DEH_String("bordtl"), PU_CACHE));
    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), (viewwindowy >> hires) - 4,
                W_CacheLumpName(DEH_String("bordtr"), PU_CACHE));
    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), (viewwindowy >> hires) + (scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("bordbr"), PU_CACHE));
    V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) + (scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("bordbl"), PU_CACHE));
}

/*
==================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows
==================
*/

boolean BorderTopRefresh;

void R_DrawTopBorder(void)
{
    byte *src, *dest;
    int x, y;

    if (scaledviewwidth == screenwidth)
        return;

    if (gamemode == shareware)
    {
        src = W_CacheLumpName(DEH_String("FLOOR04"), PU_CACHE);
    }
    else
    {
        src = W_CacheLumpName(DEH_String("FLAT513"), PU_CACHE);
    }
    dest = I_VideoBuffer;

    for (y = 0; y < (30 << hires); y++)
    {
        for (x = 0; x < screenwidth / 64; x++)
        {
            memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (screenwidth & 63)
        {
            memcpy(dest, src + ((y & 63) << 6), screenwidth & 63);
            dest += (screenwidth & 63);
        }
    }
    if ((viewwindowy >> hires) < 25)
    {
        for (x = (viewwindowx >> hires); x < ((viewwindowx >> hires) + (viewwidth >> hires)); x += 16)
        {
            V_DrawPatch(x, (viewwindowy >> hires) - 4,
                        W_CacheLumpName(DEH_String("bordt"), PU_CACHE));
        }
        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires),
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE));
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires),
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE));
        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) + 16,
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE));
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires) + 16,
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE));

        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordtl"), PU_CACHE));
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordtr"), PU_CACHE));
    }
}

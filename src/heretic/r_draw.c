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
// R_draw.c


#include "doomdef.h"
#include "deh_str.h"
#include "r_local.h"
#include "i_video.h"
#include "v_video.h"
#include "jn.h"


// All drawing to the view buffer is accomplished in this file.  The other refresh
// files only know about ccordinates, not the architecture of the frame buffer.
int   viewwidth, scaledviewwidth;
int   viewheight, scaledviewheight;
int   viewwindowx, viewwindowy;

static byte *ylookup[SCREENHEIGHT]; 
static int   columnofs[WIDESCREENWIDTH]; 

// R_DrawColumn. Source is the top of the column to scale.
const lighttable_t *dc_colormap[2];  // [crispy] brightmaps
const byte         *dc_source;       // First pixel in a column (possibly virtual).
fixed_t dc_x, dc_yl, dc_yh;
fixed_t dc_iscale;
fixed_t dc_texturemid;
fixed_t dc_texheight;

// Translated columns.
byte *dc_translation;
byte *translationtables;

// Spans.
int     ds_y, ds_x1, ds_x2;
fixed_t ds_xfrac, ds_yfrac;
fixed_t ds_xstep, ds_ystep;

const lighttable_t *ds_colormap[2];
const byte         *ds_source;  // start of a 64*64 tile image 
const byte         *ds_brightmap;


/*
================================================================================
=
= R_DrawColumn
=
================================================================================
*/

void R_DrawColumn (void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac, fracstep;

    // Zero length, column does not exceed a pixel.
    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error (english_language ?
                 "R_DrawColumn: %i to %i at %i" :
                 "R_DrawColumn: %i к %i в %i",
                 dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery) * fracstep;

    // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    // [JN] killough 2/1/98: more performance tuning
    {
        const byte *source = dc_source;
        const byte *brightmap = dc_brightmap;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

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
                // [JN] heightmask is the Tutti-Frutti fix -- killough
                // [crispy] brightmaps
                const byte src = source[frac>>FRACBITS];

                *dest = colormap[brightmap[src]][src];
                dest += screenwidth;
                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do
            {
                // [crispy] brightmaps
                const byte src = source[(frac>>FRACBITS)&heightmask];

                *dest = colormap[brightmap[src]][src];
                dest += screenwidth; 
                frac += fracstep;
            } while (count--); 
        }
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

void R_DrawColumnLow (void)
{
    const int x = dc_x << 1;  // Blocky mode, need to multiply by 2.
    int       count = dc_yh - dc_yl; 
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac, fracstep;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumnLow: %i to %i at %i" :
                "R_DrawColumnLow: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery) * fracstep;

    // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
    {
        const byte *source = dc_source;
        const byte *brightmap = dc_brightmap;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

        if (dc_texheight & heightmask)  // not a power of 2 -- killough
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
                // [crispy] brightmaps
                const byte src = source[frac>>FRACBITS];
                *dest4 = *dest3 = *dest2 = *dest1 = colormap[brightmap[src]][src];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                if ((frac += dc_iscale) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do 
            {
                // [crispy] brightmaps
                const byte src = source[(frac>>FRACBITS)&heightmask];
                *dest4 = *dest3 = *dest2 = *dest1 = colormap[brightmap[src]][src];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                frac += fracstep; 

            } while (count--);
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

void R_DrawSkyColumn (void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac, fracstep;

    if (count < 0)
    {
        return;
    }

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    fracstep = skyiscale;
    frac = skytexturemid + (dc_yl - centery) * fracstep;

    {
        const byte *source = dc_source;
        const byte *brightmap = dc_brightmap;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = skytextureheight - 1;

        if (skytextureheight & heightmask)  // not a power of 2 -- killough
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
                const byte src = source[frac>>FRACBITS];

                *dest = colormap[brightmap[0]][src];
                dest += screenwidth;
                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do 
            {
                const byte src = source[(frac>>FRACBITS)&heightmask];

                *dest = colormap[brightmap[0]][src];
                dest += screenwidth;
                frac += fracstep;
            } while (count--);
        }
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
    const int x = dc_x << 1;  // Blocky mode, need to multiply by 2.
    int       count = dc_yh - dc_yl;
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac, fracstep;

    if (count < 0)
    {
        return;
    }

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    fracstep = skyiscale_low; 
    frac  = skytexturemid + (dc_yl - centery) * fracstep;

    {
        const byte *source = dc_source;
        const byte *brightmap = dc_brightmap;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = skytextureheight - 1;

        if (skytextureheight & heightmask) // not a power of 2 -- killough
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
                const byte src = source[frac>>FRACBITS];
                *dest4 = *dest3 = *dest2 = *dest1 = colormap[brightmap[0]][src];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do 
            {
                // [crispy] brightmaps
                const byte src = source[(frac>>FRACBITS)&heightmask];
                *dest4 = *dest3 = *dest2 = *dest1 = colormap[brightmap[0]][src];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                frac += fracstep; 

            } while (count--);
        }
    }
}

/*
================================================================================
=
= R_DrawTLColumn
=
= Translucent column draw - blended with background using tinttable.
=
================================================================================
*/

void R_DrawTLColumn (void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac, fracstep;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumn: %i to %i at %i" :
                 "R_DrawTLColumn: %i к %i у %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte *source = dc_source;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

        if (dc_texheight & heightmask)  // not a power of 2 -- killough
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
                *dest = tinttable[(*dest << 8) + colormap[0][source[frac >> FRACBITS]]];
                dest += screenwidth;
                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do
            {
                *dest = tinttable[(*dest << 8) + colormap[0][source[(frac >> FRACBITS) & heightmask]]];
                dest += screenwidth;
                frac += fracstep;
            } while (count--);
        }
    }
}

/*
================================================================================
=
= R_DrawTLColumnLow
=
= [JN] Draw translucent column, low-resolution version.
=
================================================================================
*/

void R_DrawTLColumnLow (void)
{
    const int x = dc_x << 1;  // Blocky mode, need to multiply by 2.
    int       count = dc_yh - dc_yl;
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac, fracstep;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumnLow: %i to %i at %i" :
                 "R_DrawTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    fracstep = dc_iscale; 
    frac  = dc_texturemid + (dc_yl-centery)*fracstep;

    {
        const byte *source = dc_source;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

        if (dc_texheight & heightmask)  // not a power of 2 -- killough
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
                const byte src = source[frac >> FRACBITS];

                *dest1 = tinttable[(*dest1 << 8) + colormap[0][src]];
                *dest2 = tinttable[(*dest2 << 8) + colormap[0][src]];
                *dest3 = tinttable[(*dest3 << 8) + colormap[0][src]];
                *dest4 = tinttable[(*dest4 << 8) + colormap[0][src]];
                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
    
                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do 
            {
                const byte src = source[(frac >> FRACBITS) & heightmask];

                *dest1 = tinttable[(*dest1 << 8) +colormap[0][src]];
                *dest2 = tinttable[(*dest2 << 8) +colormap[0][src]];
                *dest3 = tinttable[(*dest3 << 8) +colormap[0][src]];
                *dest4 = tinttable[(*dest4 << 8) +colormap[0][src]];
    
                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
    
                frac += fracstep; 

            } while (count--);
        }
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

void R_DrawExtraTLColumn (void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac, fracstep;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTLColumn: %i to %i at %i" :
                 "R_DrawTLColumn: %i к %i у %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    {
        const byte *source = dc_source;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

        if (dc_texheight & heightmask)  // not a power of 2 -- killough
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
                *dest = transtable80[(*dest << 8) + colormap[0][source[frac >> FRACBITS]]];
                dest += screenwidth;
                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do
            {
                *dest = transtable80[(*dest << 8) + colormap[0][source[(frac >> FRACBITS) & heightmask]]];
                dest += screenwidth;
                frac += fracstep;
            } while (count--);
        }
    }
}

/*
================================================================================
=
= R_DrawExtraTLColumnLow
=
= [JN] Draw extra translucent column, low-resolution version. 
=
================================================================================
*/

void R_DrawExtraTLColumnLow (void)
{
    const int x = dc_x << 1;  // Blocky mode, need to multiply by 2.
    int       count = dc_yh - dc_yl;
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac, fracstep;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawExtraTLColumnLow: %i to %i at %i" :
                 "R_DrawTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    fracstep = dc_iscale; 
    frac  = dc_texturemid + (dc_yl-centery)*fracstep;

    {
        const byte *source = dc_source;
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

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
                const byte src = source[frac >> FRACBITS];

                *dest1 = transtable80[(*dest1 << 8) + colormap[0][src]];
                *dest2 = transtable80[(*dest2 << 8) + colormap[0][src]];
                *dest4 = transtable80[(*dest4 << 8) + colormap[0][src]];
                *dest3 = transtable80[(*dest3 << 8) + colormap[0][src]];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                if ((frac += fracstep) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else // texture height is a power of 2 -- killough
        {
            do 
            {
                const byte src = source[(frac>>FRACBITS)&heightmask];

                *dest1 = transtable80[(*dest1 << 8) + colormap[0][src]];
                *dest2 = transtable80[(*dest2 << 8) + colormap[0][src]];
                *dest3 = transtable80[(*dest3 << 8) + colormap[0][src]];
                *dest4 = transtable80[(*dest4 << 8) + colormap[0][src]];

                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;

                frac += fracstep; 

            } while (count--);
        }
    }
}

/*
================================================================================
=
= R_DrawTranslatedColumn
=
================================================================================
*/

void R_DrawTranslatedColumn (void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumn: %i to %i at %i" :
                "R_DrawColumn: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl - centery) * dc_iscale; 

    do
    {
        *dest = dc_colormap[0][dc_translation[dc_source[frac >> FRACBITS]]];
        dest += screenwidth;
        frac += dc_iscale;
    } while (count--);
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
    const int x = dc_x << 1;  // low detail, need to scale by 2
    int       count = dc_yh - dc_yl; 
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac; 

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK 
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTranslatedColumnLow: %i to %i at %i" :
                 "R_DrawTranslatedColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif 

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale; 

    do 
    {
        *dest4 = *dest3 = *dest2 = *dest1 = dc_colormap[0][dc_translation[dc_source[frac >> FRACBITS]]];

        dest1 += screenwidth << hires;
        dest2 += screenwidth << hires;
        dest3 += screenwidth << hires;
        dest4 += screenwidth << hires;

        frac += dc_iscale; 
    } while (count--); 
}

void R_DrawTranslatedTLColumn(void)
{
    int      count = dc_yh - dc_yl;
    byte    *dest;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawColumn: %i to %i at %i" :
                "R_DrawColumn: %i к %i в %i",
                dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl - centery) * dc_iscale;

    do
    {
        *dest = tinttable[(*dest << 8) + dc_colormap[0][dc_translation[frac >> FRACBITS]]];
        dest += screenwidth;
        frac += dc_iscale;
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

void R_DrawTranslatedTLColumnLow (void)
{
    const int x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int       count = dc_yh - dc_yl;
    byte     *dest1, *dest2, *dest3, *dest4;
    fixed_t   frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error (english_language ?
                 "R_DrawTranslatedTLColumnLow: %i to %i at %i" :
                 "R_DrawTranslatedTLColumnLow: %i к %i у %i",
                 dc_yl, dc_yh, x);
    }
#endif

    dest1 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x + 1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x + 1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

    {
        const lighttable_t *const *colormap = dc_colormap;
        int heightmask = dc_texheight - 1;

        if (dc_texheight & heightmask)  // not a power of 2 -- killough
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
                *dest1 = tinttable[(*dest1 << 8) + colormap[0][dc_translation[frac >> FRACBITS]]];
                *dest2 = tinttable[(*dest2 << 8) + colormap[0][dc_translation[frac >> FRACBITS]]];
                *dest3 = tinttable[(*dest3 << 8) + colormap[0][dc_translation[frac >> FRACBITS]]];
                *dest4 = tinttable[(*dest4 << 8) + colormap[0][dc_translation[frac >> FRACBITS]]];
        
                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
        
                if ((frac += dc_iscale) >= heightmask)
                {
                    frac -= heightmask;
                }
            } while (count--);
        }
        else  // texture height is a power of 2 -- killough
        {
            do 
            {
                *dest1 = tinttable[(*dest1 << 8) + colormap[0][dc_translation[(frac >> FRACBITS) & heightmask]]];
                *dest2 = tinttable[(*dest2 << 8) + colormap[0][dc_translation[(frac >> FRACBITS) & heightmask]]];
                *dest3 = tinttable[(*dest3 << 8) + colormap[0][dc_translation[(frac >> FRACBITS) & heightmask]]];
                *dest4 = tinttable[(*dest4 << 8) + colormap[0][dc_translation[(frac >> FRACBITS) & heightmask]]];
        
                dest1 += screenwidth << hires;
                dest2 += screenwidth << hires;
                dest3 += screenwidth << hires;
                dest4 += screenwidth << hires;
        
                frac += dc_iscale; 
        
            } while (count--);
        }
    }
}

/*
================================================================================
=
= R_DrawSpan
=
= With Heretic style restrictions on view orientation, the floors and ceilings
= consist of horizontal slices or spans with constant z depth. However,
= rotation around the world z axis is possible, thus this mapping, while
= simpler and faster than perspective correct texture mapping, has to traverse
= the texture at an angle in all but a few cases. In consequence, flats are
= not stored by column (like walls), and the inner loop has to step in 
= texture space u and v.
=
================================================================================
*/

void R_DrawSpan (void)
{
    unsigned int count = ds_x2 - ds_x1;  // We do not check for zero spans here.
    const byte  *source = ds_source;
    const byte  *brightmap = ds_brightmap;
    const byte **colormap = ds_colormap;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= screenwidth
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error(english_language ?
                "R_DrawSpan: %i to %i at %i" :
                "R_DrawSpan: %i к %i в %i",
                ds_x1, ds_x2, ds_y);
#endif

    // Pack position and step variables into a single 32-bit integer,
    // with x in the top 16 bits and y in the bottom 16 bits.  For
    // each 16-bit part, the top 6 bits are the integer part and the
    // bottom 10 bits are the fractional part of the pixel position.
    // dest = ylookup[ds_y] + columnofs[ds_x1];

    do
    {
        byte *dest = ylookup[ds_y] + columnofs[flipviewwidth[ds_x1++]];

        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        unsigned const int ytemp = (ds_yfrac >> 10) & 0x0fc0;
        unsigned const int xtemp = (ds_xfrac >> 16) & 0x3f;
        unsigned const int spot = xtemp | ytemp;

        // Lookup pixel from flat texture tile, re-index using light/colormap.
        *dest = colormap[brightmap[source[spot]]][source[spot]];

        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    } while (count--);
}

/*
================================================================================
=
= R_DrawSpanLow
=
= Again...
=
================================================================================
*/

void R_DrawSpanLow (void)
{
    unsigned int count = ds_x2 - ds_x1;  // We do not check for zero spans here.
    const int    ds_y_low = ds_y << hires;
    const byte  *source = ds_source;
    const byte  *brightmap = ds_brightmap;
    const byte **colormap = ds_colormap;
    byte        *dest1, *dest2;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=screenwidth || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error(english_language ?
                "R_DrawSpan: %i to %i at %i" :
                "R_DrawSpan: %i к %i у %i",
                ds_x1,ds_x2,ds_y);
    }
#endif

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    // Pack position and step variables into a single 32-bit integer,
    // with x in the top 16 bits and y in the bottom 16 bits.  For
    // each 16-bit part, the top 6 bits are the integer part and the
    // bottom 10 bits are the fractional part of the pixel position.

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        unsigned const int ytemp = (ds_yfrac >> 10) & 0x0fc0;
        unsigned const int xtemp = (ds_xfrac >> 16) & 0x3f;
        unsigned const int spot = xtemp | ytemp;

        // Lowres/blocky mode does it twice, while scale is adjusted appropriately.
         dest1 = ylookup[ds_y_low] + columnofs[flipviewwidth[ds_x1]];
        *dest1 = colormap[brightmap[source[spot]]][source[spot]];
         dest2 = ylookup[ds_y_low + 1] + columnofs[flipviewwidth[ds_x1++]];
        *dest2 = colormap[brightmap[source[spot]]][source[spot]];
         dest1 = ylookup[ds_y_low] + columnofs[flipviewwidth[ds_x1]];
        *dest1 = colormap[brightmap[source[spot]]][source[spot]];
         dest2 = ylookup[ds_y_low + 1] + columnofs[flipviewwidth[ds_x1++]];
        *dest2 = colormap[brightmap[source[spot]]][source[spot]];

        // position += step;
        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;

    } while (count--);
}

/*
================================================================================
=
= R_InitBuffer
=
================================================================================
*/

void R_InitBuffer (const int width, const int height)
{
    int i;

    // Handle resize, e.g. smaller view windows with border and/or status bar.
    viewwindowx = (screenwidth - width) >> 1;

    for (i = 0 ; i < width ; i++)
    {
        columnofs[i] = viewwindowx + i;
    }

    // Same with base row offset.
    if (width == screenwidth)
    {
        viewwindowy = 0;
    }
    else
    {
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;
    }

    // Preclaculate all row offsets.
    for (i = 0; i < height; i++)
    {
        ylookup[i] = I_VideoBuffer + (i + viewwindowy) * screenwidth;
    }
}

/*
================================================================================
=
= R_DrawViewBorder
=
= Draws the border around the view for different size windows
=
================================================================================
*/

boolean BorderNeedRefresh;

void R_DrawViewBorder(void)
{
    byte *src, *dest;
    int x, y;
    const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

    if (scaledviewwidth == screenwidth)
    {
        return;
    }

    // [JN] TODO -- predefine background flats at strtup
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
    for (y = 0 ; y < SCREENHEIGHT - SBARHEIGHT ; y++)
    {
        for (x = 0 ; x < screenwidth ; x++)
        {
            *dest++ = src[(((y >> shift_allowed) & 63) << 6) 
                         + ((x >> shift_allowed) & 63)];
        }
    }

    for (x = (viewwindowx >> hires) ; x < ((viewwindowx >> hires) + (scaledviewwidth >> hires)) ; x += 16)
    {
        V_DrawPatch(x, (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordt"), PU_CACHE), NULL);
        V_DrawPatch(x, (viewwindowy >> hires) + (scaledviewheight >> hires),
                    W_CacheLumpName(DEH_String("bordb"), PU_CACHE), NULL);
    }
    for (y = (viewwindowy >> hires) ; y < ((viewwindowy >> hires) + (scaledviewheight >> hires)) ; y += 16)
    {
        V_DrawPatch((viewwindowx >> hires) - 4, y,
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), y,
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE), NULL);
    }
    V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) - 4,
                W_CacheLumpName(DEH_String("bordtl"), PU_CACHE), NULL);
    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), (viewwindowy >> hires) - 4,
                W_CacheLumpName(DEH_String("bordtr"), PU_CACHE), NULL);
    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), (viewwindowy >> hires) + (scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("bordbr"), PU_CACHE), NULL);
    V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) + (scaledviewheight >> hires),
                W_CacheLumpName(DEH_String("bordbl"), PU_CACHE), NULL);
}

/*
================================================================================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows
=
================================================================================
*/


boolean BorderTopRefresh;

void R_DrawTopBorder (void)
{
    byte *src, *dest;
    int x, y;

    if (scaledviewwidth == screenwidth)
    {
        return;
    }

    // [JN] TODO -- predefine background flats at strtup
    if (gamemode == shareware)
    {
        src = W_CacheLumpName(DEH_String("FLOOR04"), PU_CACHE);
    }
    else
    {
        src = W_CacheLumpName(DEH_String("FLAT513"), PU_CACHE);
    }

    dest = I_VideoBuffer;

    for (y = 0 ; y < (30 << hires) ; y++)
    {
        for (x = 0 ; x < screenwidth / 64 ; x++)
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
                        W_CacheLumpName(DEH_String("bordt"), PU_CACHE), NULL);
        }
        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires),
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires),
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) + 16,
                    W_CacheLumpName(DEH_String("bordl"), PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires) + 16,
                    W_CacheLumpName(DEH_String("bordr"), PU_CACHE), NULL);

        V_DrawPatch((viewwindowx >> hires) - 4, (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordtl"), PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires), (viewwindowy >> hires) - 4,
                    W_CacheLumpName(DEH_String("bordtr"), PU_CACHE), NULL);
    }
}

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


#include "h2def.h"
#include "i_system.h"
#include "r_local.h"
#include "v_video.h"


// All drawing to the view buffer is accomplished in this file.  The other refresh
// files only know about ccordinates, not the architecture of the frame buffer.
int   viewwidth, scaledviewwidth;
int   viewheight, scaledviewheight;
int   viewwindowx, viewwindowy;

byte *ylookup[SCREENHEIGHT];
int   columnofs[WIDESCREENWIDTH];

// R_DrawColumn. Source is the top of the column to scale.
lighttable_t *dc_colormap[2]; // [crispy] brightmaps
int           dc_x, dc_yl, dc_yh;
fixed_t       dc_iscale, dc_texturemid, dc_texheight;
byte         *dc_source;  // first pixel in a column (possibly virtual)

// Translated columns.
byte *dc_translation;
byte *translationtables;

// Spans.
int           ds_y, ds_x1, ds_x2;
fixed_t       ds_xfrac, ds_yfrac;
fixed_t       ds_xstep, ds_ystep;
byte         *ds_source;  // start of a 64*64 tile image
lighttable_t *ds_colormap;

// Border drawing.
boolean BorderNeedRefresh;
boolean BorderTopRefresh;


/*
================================================================================
=
= R_DrawColumn
=
= Source is the top of the column to scale
=
================================================================================
*/

void R_DrawColumn (void)
{
    int      count = dc_yh - dc_yl;
	int      heightmask = dc_texheight-1;
    byte    *dest;
    fixed_t  frac;

	// Zero length, column does not exceed a pixel.
    if (count < 0)
	{
		return;
	}

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        // [JN] Some widescreen assets (like CMCEB0) are notably
        // wider and placed outside of screen bounds farther than
        // this condition is checking, which may lead to unnecessary
        // I_Error calling. If no action is taken, an overflow may
        // happen as well as drawing column from opposite side of
        // the screen. Thus, instead of I_Error just dont draw such column.
        return;
    }
#endif

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];

    // Determine scaling, which is the only mapping to be done.

    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

    // Inner loop that does the actual texture mapping,
    // e.g. a DDA-lile scaling. This is as fast as it gets.
    // [JN] killough 2/1/98: more performance tuning
	//
	// not a power of 2 -- killough
	if (dc_texheight & heightmask)   
	{
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
		{
            while ((frac += heightmask) < 0);
		}
        else
		{
            while (frac >= heightmask)
				   frac -= heightmask;
		}

        do
        {
            // Re-map color indices from wall texture column
            //  using a lighting/special effects LUT.
            // heightmask is the Tutti-Frutti fix -- killough
            // [crispy] brightmaps
            const byte source = dc_source[frac>>FRACBITS];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += screenwidth;
            if ((frac += dc_iscale) >= heightmask)
			{
                frac -= heightmask;
			}
        } while (count--);
	}
	// texture height is a power of 2 -- killough
	else
    {
        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[(frac>>FRACBITS)&heightmask];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += screenwidth;
            frac += dc_iscale;
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

void R_DrawColumnLow (void)
{
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      count = dc_yh - dc_yl;
    int      heightmask = dc_texheight - 1;
    byte    *dest, *dest2, *dest3, *dest4;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        // [JN] Some widescreen assets (like CMCEB0) are notably
        // wider and placed outside of screen bounds farther than
        // this condition is checking, which may lead to unnecessary
        // I_Error calling. If no action is taken, an overflow may
        // happen as well as drawing column from opposite side of
        // the screen. Thus, instead of I_Error just dont draw such column.
        return;
    }
#endif

    dest = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            const byte source = dc_source[frac>>FRACBITS];
            *dest4 = *dest3 = *dest2 = *dest = dc_colormap[dc_brightmap[source]][source];

            dest += screenwidth << hires;
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
            const byte source = dc_source[(frac>>FRACBITS)&heightmask];
            *dest4 = *dest3 = *dest2 = *dest = dc_colormap[dc_brightmap[source]][source];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            frac += dc_iscale; 

        } while (count--);
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
    int      heightmask = dc_texheight-1;
    byte    *dest;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            dest += screenwidth;
            if ((frac += dc_iscale) >= heightmask)
            {
                frac -= heightmask;
            }
        }
        while (count--);
    }
    else  // texture height is a power of 2 -- killough
    {
        do
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS & heightmask]]];
            dest += screenwidth;
            frac += dc_iscale;
        } while (count--);
    }
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
    int     count = dc_yh - dc_yl;
    int     x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int     heightmask = dc_texheight - 1;
    byte   *dest, *dest2, *dest3, *dest4;
    fixed_t frac;

    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];

            dest += screenwidth << hires;
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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            frac += dc_iscale; 

        } while (count--);
    }
}

/*
================================================================================
=
= R_DrawAltTLColumn
=
================================================================================
*/

void R_DrawAltTLColumn(void)
{
    int      count = dc_yh - dc_yl;
    int      heightmask = dc_texheight-1;
    byte    *dest;
    fixed_t  frac;

    if (!dc_yl)
    {
        dc_yl = 1;
    }
    if (dc_yh == viewheight - 1)
    {
        dc_yh = viewheight - 2;
    }

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl - centery) * dc_iscale;

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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            dest += screenwidth;
            if ((frac += dc_iscale) >= heightmask)
            {
                frac -= heightmask;
            }
        }
        while (count--);
    }
    else  // texture height is a power of 2 -- killough
    {
        do
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS & heightmask]]];
            dest += screenwidth;
            frac += dc_iscale;
        } while (count--);
    }
}

/*
================================================================================
=
= R_DrawAltTLColumnLow
=
================================================================================
*/

void R_DrawAltTLColumnLow(void)
{
    int      count = dc_yh - dc_yl;
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      heightmask = dc_texheight-1;
    byte    *dest, *dest2, *dest3, *dest4;
    fixed_t  frac;

    if (!dc_yl)
    {
        dc_yl = 1;
    }
    if (dc_yh == viewheight - 1)
    {
        dc_yh = viewheight - 2;
    }

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned) dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl - centery) * dc_iscale;

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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            if ((frac += dc_iscale) >= heightmask)
            {
                frac -= heightmask;
            }
        }
        while (count--);
    }
    else  // texture height is a power of 2 -- killough
    {
        do
        {
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            frac += dc_iscale; 
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
    int      count = dc_yh - dc_yl;
    int      heightmask = dc_texheight-1;
    byte    *dest;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)dc_x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            *dest = transtable80[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            dest += screenwidth;
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
            *dest = transtable80[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS & heightmask]]];
            dest += screenwidth;
            frac += dc_iscale;
        } while (count--);
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
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      count = dc_yh - dc_yl;
    int      heightmask = dc_texheight - 1;
    byte    *dest, *dest2, *dest3, *dest4;
    fixed_t  frac;

    if (count < 0)
    return;

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest  = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            *dest = transtable80[(*dest<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest2 = transtable80[(*dest2<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest3 = transtable80[(*dest3<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];
            *dest4 = transtable80[(*dest4<<8)+dc_colormap[0][dc_source[frac>>FRACBITS]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            if ((frac += dc_iscale) >= heightmask)
            {
                frac -= heightmask;
            }
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do 
        {
            *dest = transtable80[(*dest<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest2 = transtable80[(*dest2<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest3 = transtable80[(*dest3<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];
            *dest4 = transtable80[(*dest4<<8)+dc_colormap[0][dc_source[(frac>>FRACBITS)&heightmask]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            frac += dc_iscale; 

        } while (count--);
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
    {
        return;
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale; 

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
    int      x = dc_x << 1;  // low detail, need to scale by 2
    int      count = dc_yh - dc_yl; 
    byte    *dest, *dest2, *dest3, *dest4;
    fixed_t  frac; 

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK 
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif 

    dest  = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale; 

    do 
    {
        *dest4 = *dest3 = *dest2 = *dest = dc_colormap[0][dc_translation[dc_source[frac>>FRACBITS]]];

        dest += screenwidth << hires;
        dest2 += screenwidth << hires;
        dest3 += screenwidth << hires;
        dest4 += screenwidth << hires;

        frac += dc_iscale; 
    } while (count--); 
}

//============================================================================
//
// R_DrawTranslatedTLColumn
//
//============================================================================

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
    {
        return;
    }
#endif

    dest = ylookup[dc_yl] + columnofs[flipviewwidth[dc_x]];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = tinttable[((*dest) << 8)
                          +
                          dc_colormap[0][dc_translation
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
    int      x = dc_x << 1; // Blocky mode, need to multiply by 2.
    int      count = dc_yh - dc_yl;
    int      heightmask = dc_texheight - 1;
    byte    *dest, *dest2, *dest3, *dest4;
    fixed_t  frac;

    if (count < 0)
    {
        return;
    }

#ifdef RANGECHECK
    if ((unsigned)x >= screenwidth || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        return;
    }
#endif

    dest  = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x]];
    dest2 = ylookup[(dc_yl << hires)] + columnofs[flipviewwidth[x+1]];
    dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x]];
    dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipviewwidth[x+1]];
    frac = dc_texturemid + (dc_yl-centery)*dc_iscale;

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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_translation[dc_source[frac>>FRACBITS]]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_translation[dc_source[frac>>FRACBITS]]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_translation[dc_source[frac>>FRACBITS]]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_translation[dc_source[frac>>FRACBITS]]]];

            dest += screenwidth << hires;
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
            *dest = tinttable[(*dest<<8)+dc_colormap[0][dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
            *dest2 = tinttable[(*dest2<<8)+dc_colormap[0][dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
            *dest3 = tinttable[(*dest3<<8)+dc_colormap[0][dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];
            *dest4 = tinttable[(*dest4<<8)+dc_colormap[0][dc_translation[dc_source[(frac>>FRACBITS)&heightmask]]]];

            dest += screenwidth << hires;
            dest2 += screenwidth << hires;
            dest3 += screenwidth << hires;
            dest4 += screenwidth << hires;

            frac += dc_iscale; 

        } while (count--);
    }
}

/*
================================================================================
=
= R_InitTranslationTables
=
================================================================================
*/

void R_InitTranslationTables(void)
{
    int i, lumpnum;
    byte *transLump;

    V_LoadTintTable();

    // Allocate translation tables
    translationtables = Z_Malloc(256 * 3 * (maxplayers - 1), PU_STATIC, 0);

    for (i = 0; i < 3 * (maxplayers - 1); i++)
    {
        lumpnum = W_GetNumForName("trantbl0") + i;
        transLump = W_CacheLumpNum(lumpnum, PU_STATIC);
        memcpy(translationtables + i * 256, transLump, 256);
        W_ReleaseLumpNum(lumpnum);
    }
}

/*
================================================================================
=
= R_DrawSpan
=
= With Hexen style restrictions on view orientation, the floors and ceilings
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
    byte  *dest;
    int    count, spot;
    unsigned int xtemp, ytemp;

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

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        dest = ylookup[ds_y] + columnofs[flipviewwidth[ds_x1++]];
        *dest = ds_colormap[ds_source[spot]];

        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;
    } while (count--);
}

/*
================================================================================
=
= R_DrawSpan
=
= Again...
=
================================================================================
*/

void R_DrawSpanLow(void)
{
    byte    *dest, *dest2;
    int      count = (ds_x2 - ds_x1);
    int      spot;
    unsigned int xtemp, ytemp;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0
	||  ds_x2 >= screenwidth || (unsigned)ds_y > SCREENHEIGHT)
    {
        I_Error(english_language ?
                "R_DrawSpan: %i to %i at %i" :
                "R_DrawSpan: %i к %i у %i",
                ds_x1, ds_x2, ds_y);
    }
#endif

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    do
    {
        // Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

        // Lowres/blocky mode does it twice, while scale is adjusted appropriately.
        dest = ylookup[(ds_y << hires)] + columnofs[flipviewwidth[ds_x1]];
        *dest = ds_colormap[ds_source[spot]];
        dest2 = ylookup[(ds_y << hires) + 1] + columnofs[flipviewwidth[ds_x1++]];
        *dest2 = ds_colormap[ds_source[spot]];
        dest = ylookup[(ds_y << hires)] + columnofs[flipviewwidth[ds_x1]];
        *dest = ds_colormap[ds_source[spot]];
        dest2 = ylookup[(ds_y << hires) + 1] + columnofs[flipviewwidth[ds_x1++]];
        *dest2 = ds_colormap[ds_source[spot]];

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

void R_InitBuffer (int width, int height)
{
    int i;

    viewwindowx = (screenwidth - width) >> 1;

    for (i = 0; i < width; i++)
	{
        columnofs[i] = viewwindowx + i;
	}

    if (width == screenwidth)
	{
        viewwindowy = 0;
	}
    else
	{
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;
	}

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
= Draws the border around the view for different size windows.
=
================================================================================
*/

void R_DrawViewBorder (void)
{
    int x, y;
    byte *dest = I_VideoBuffer;
    const byte *src = W_CacheLumpName("F_022", PU_CACHE);
    const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

    if (scaledviewwidth == screenwidth)
    {
        return;
    }

    // [JN] Simplified, same to V_FillFlat now.
    for (y = 0; y < SCREENHEIGHT - SBARHEIGHT; y++)
    {
        for (x = 0; x < screenwidth; x++)
        {
            *dest++ = src[(((y >> shift_allowed) & 63) << 6) 
                         + ((x >> shift_allowed) & 63)];
        }
    }

    for (x = (viewwindowx >> hires); x < (viewwindowx >> hires) + (scaledviewwidth >> hires); x += 16)
    {
        V_DrawPatch(x, (viewwindowy >> hires) - 4,
                    W_CacheLumpName("bordt", PU_CACHE), NULL);
        V_DrawPatch(x, (viewwindowy >> hires) + (scaledviewheight >> hires),
                    W_CacheLumpName("bordb", PU_CACHE), NULL);
    }
    for (y = (viewwindowy >> hires); y < (viewwindowy >> hires) + (scaledviewheight >> hires); y += 16)
    {
        V_DrawPatch((viewwindowx >> hires) - 4, y,
                     W_CacheLumpName("bordl", PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires), y,
                     W_CacheLumpName("bordr", PU_CACHE), NULL);
    }
    V_DrawPatch((viewwindowx >> hires) - 4,
                (viewwindowy >> hires) - 4, W_CacheLumpName("bordtl", PU_CACHE), NULL);
    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires),
                (viewwindowy >> hires) - 4, W_CacheLumpName("bordtr", PU_CACHE), NULL);

    V_DrawPatch((viewwindowx >> hires) + (scaledviewwidth >> hires),
                (viewwindowy >> hires) + (scaledviewheight >> hires), W_CacheLumpName("bordbr", PU_CACHE), NULL);
    V_DrawPatch((viewwindowx >> hires) - 4,
                (viewwindowy >> hires) + (scaledviewheight >> hires), W_CacheLumpName("bordbl", PU_CACHE), NULL);
}

/*
================================================================================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows.
=
================================================================================
*/

void R_DrawTopBorder (void)
{
    int x, y;
    const byte *src = W_CacheLumpName("F_022", PU_CACHE);
    byte *dest = I_VideoBuffer;
    const int shift_allowed = vanillaparm ? 1 : hud_detaillevel;

    if (scaledviewwidth == screenwidth)
    {
        return;
    }

    // [JN] Simplified, same to V_FillFlat now.
    for (y = 0; y < (34 << hires); y++)
    {
        for (x = 0; x < screenwidth; x++)
        {
            *dest++ = src[(((y >> shift_allowed) & 63) << 6) 
                         + ((x >> shift_allowed) & 63)];
        }
    }
    if (viewwindowy < 35)
    {
        for (x = (viewwindowx >> hires); x < (viewwindowx >> hires) + (viewwidth >> hires); x += 16)
        {
            V_DrawPatch(x, (viewwindowy >> hires) - 4, W_CacheLumpName("bordt", PU_CACHE), NULL);
        }

        V_DrawPatch((viewwindowx >> hires) - 4,
                    (viewwindowy >> hires), W_CacheLumpName("bordl", PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires),
                    (viewwindowy >> hires), W_CacheLumpName("bordr", PU_CACHE), NULL);

        V_DrawPatch((viewwindowx >> hires) - 4,
                    (viewwindowy >> hires) + 16, W_CacheLumpName("bordl", PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires),
                    (viewwindowy >> hires) + 16, W_CacheLumpName("bordr", PU_CACHE), NULL);

        V_DrawPatch((viewwindowx >> hires) - 4,
                    (viewwindowy >> hires) - 4, W_CacheLumpName("bordtl", PU_CACHE), NULL);
        V_DrawPatch((viewwindowx >> hires) + (viewwidth >> hires),
                    (viewwindowy >> hires) - 4, W_CacheLumpName("bordtr", PU_CACHE), NULL);
    }
}

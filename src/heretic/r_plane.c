//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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
// R_planes.c

// Russian Doom (C) 2016-2017 Julian Nechaevsky


#include <stdlib.h>
#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "r_local.h"

planefunction_t floorfunc, ceilingfunc;

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;
fixed_t skyiscale;

// [JN] For brightmaps initialization.
// Floors and ceilings:
int bmapflatnum1, bmapflatnum2, bmapflatnum3, bmapflatnum4, bmapflatnum5;
// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05, bmaptexture06, bmaptexture07;
int bmap_terminator;

//
// opening
//

visplane_t *visplanes = NULL;
visplane_t *lastvisplane;
visplane_t *floorplane, *ceilingplane;
static int numvisplanes;

int  openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int* lastopening;           // [crispy] 32-bit integer math

//
// clip values are the solid pixel bounding the range
// floorclip starts out SCREENHEIGHT
// ceilingclip starts out -1
//
int  floorclip[SCREENWIDTH];   // [crispy] 32-bit integer math
int  ceilingclip[SCREENWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t **planezlight;
fixed_t planeheight;

fixed_t yslope[SCREENHEIGHT];
fixed_t distscale[SCREENWIDTH];
fixed_t basexscale, baseyscale;

fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];


/*
================
=
= R_InitSkyMap
=
= Called whenever the view size changes
=
================
*/

void R_InitSkyMap(void)
{
    skyflatnum = R_FlatNumForName(DEH_String("F_SKY1"));
    skytexturemid = 200 * FRACUNIT;
    skyiscale = FRACUNIT >> hires;
}


// ===========================================================
// =                                                         =
// = [JN] Lookup and init all the textures for brightmapping =
// =                                                         =
// ===========================================================
void R_InitBrightmaps(void)
{
    // Print informative message while startup
    printf("\nR_Init: Инициализация брайтмаппинга.");

    // -------------------------------------------------------
    //  Flats and ceilings
    // -------------------------------------------------------

    // blue_only_floor
    bmapflatnum1 = R_FlatNumForName(DEH_String("FLOOR21"));
    bmapflatnum2 = R_FlatNumForName(DEH_String("FLOOR22"));
    bmapflatnum3 = R_FlatNumForName(DEH_String("FLOOR23"));
    bmapflatnum4 = R_FlatNumForName(DEH_String("FLOOR24"));
    bmapflatnum5 = R_FlatNumForName(DEH_String("FLOOR26"));    

    // -------------------------------------------------------
    //  Textures
    // -------------------------------------------------------

    // brightmap_redonly
    bmaptexture01 = R_TextureNumForName(DEH_String("GRSKULL3"));
    // brightmap_blueonly
    bmaptexture02 = R_TextureNumForName(DEH_String("DOOREXIT"));
    bmaptexture03 = R_TextureNumForName(DEH_String("SW2ON"));
    bmaptexture04 = R_TextureNumForName(DEH_String("SW2OFF"));
    bmaptexture05 = R_TextureNumForName(DEH_String("SW2OFF"));
    // brightmap_notbronze
    bmaptexture06 = R_TextureNumForName(DEH_String("SW1ON"));
    bmaptexture07 = R_TextureNumForName(DEH_String("SW1OFF"));
    
    // We need to declare a "terminator" - standard game texture,
    // presented in all Heretic series and using standard light formula.
    // Otherwise, non-defined textures will use latest brightmap.
    bmap_terminator = R_TextureNumForName(DEH_String("WOODWL"));
}


/*
====================
=
= R_InitPlanes
=
= Only at game startup
====================
*/

void R_InitPlanes(void)
{
}


/*
================
=
= R_MapPlane
=
global vars:

planeheight
ds_source
basexscale
baseyscale
viewx
viewy

BASIC PRIMITIVE
================
*/

void R_MapPlane(int y, int x1, int x2)
{
    // [crispy] see below
    // angle_t angle;
    fixed_t distance;
    // [JN] also see below
    // fixed_t length;
    unsigned index;
    int dx, dy;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned) y > viewheight)
        I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
#endif

    // [crispy] visplanes with the same flats now match up far better than before
    // adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math

    if (!(dy = abs(centery - y)))
    {
        return;
    }

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul (viewsin, planeheight) / dy;
        ds_ystep = cachedystep[y] = FixedMul (viewcos, planeheight) / dy;
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;

    ds_xfrac = viewx + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy - FixedMul(viewsin, distance) + dx * ds_ystep;
    
    if (fixedcolormap)
        ds_colormap = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;
        if (index >= MAXLIGHTZ)
            index = MAXLIGHTZ - 1;
        ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    spanfunc();                 // high or low detail
}

//=============================================================================

/*
====================
=
= R_ClearPlanes
=
= At begining of frame
====================
*/

void R_ClearPlanes(void)
{
    int i;
    angle_t angle;

//
// opening / clipping determination
//      
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

//
// texture calculation
//
    memset(cachedheight, 0, sizeof(cachedheight));
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;    // left to right mapping

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}

// [crispy] remove MAXVISPLANES Vanilla limit
void R_RaiseVisplanes (visplane_t** vp)
{
    if (lastvisplane - visplanes == numvisplanes)
    {
	int numvisplanes_old = numvisplanes;
	visplane_t* visplanes_old = visplanes;

	if (numvisplanes_old == MAXVISPLANES)
	    printf("R_FindPlane: Hit MAXVISPLANES (%d) Vanilla limit.\n", MAXVISPLANES);

	numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
	visplanes = realloc(visplanes, numvisplanes * sizeof(*visplanes));
	lastvisplane = visplanes + numvisplanes_old;
	floorplane = visplanes + (floorplane - visplanes_old);
	ceilingplane = visplanes + (ceilingplane - visplanes_old);

	if (vp)
	    *vp = visplanes + (*vp - visplanes_old);
    }
}

/*
===============
=
= R_FindPlane
=
===============
*/

visplane_t *R_FindPlane(fixed_t height, int picnum,
                        int lightlevel, int special)
{
    visplane_t *check;

    if (picnum == skyflatnum)
    {
        // all skies map together
        height = 0;
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel && special == check->special)
            break;
    }

    if (check < lastvisplane)
    {
        return (check);
    }

    R_RaiseVisplanes(&check);

    lastvisplane++;
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->special = special;
    check->minx = SCREENWIDTH;
    check->maxx = -1;
    memset(check->top, 0xff, sizeof(check->top));
    return (check);
}

/*
===============
=
= R_CheckPlane
=
===============
*/

visplane_t *R_CheckPlane(visplane_t * pl, int start, int stop)
{
    int intrl, intrh;
    int unionl, unionh;
    int x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
        if (pl->top[x] != 0xffffffffu) // [crispy] hires / 32-bit integer math
            break;

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;
        return pl;              // use the same one
    }

// make a new visplane

    R_RaiseVisplanes(&pl);
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    lastvisplane->special = pl->special;
    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;
    memset(pl->top, 0xff, sizeof(pl->top));

    return pl;
}



//=============================================================================

/*
================
=
= R_MakeSpans
=
================
*/

void R_MakeSpans(int x, 
 unsigned int t1, // [crispy] 32-bit integer math
 unsigned int b1, // [crispy] 32-bit integer math
 unsigned int t2, // [crispy] 32-bit integer math
 unsigned int b2) // [crispy] 32-bit integer math
{
    while (t1 < t2 && t1 <= b1)
    {
        R_MapPlane(t1, spanstart[t1], x - 1);
        t1++;
    }
    while (b1 > b2 && b1 >= t1)
    {
        R_MapPlane(b1, spanstart[b1], x - 1);
        b1--;
    }

    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



/*
================
=
= R_DrawPlanes
=
= At the end of each frame
================
*/

void R_DrawPlanes(void)
{
    visplane_t *pl;
    int light;
    int x, stop;
    int lumpnum;
    int angle;
    byte *tempSource;

    byte *dest;
    int count;
    fixed_t frac, fracstep;

    extern byte *ylookup[MAXHEIGHT];
    extern int columnofs[MAXWIDTH];

#ifdef RANGECHECK
    if (ds_p - drawsegs > numdrawsegs)
        I_Error("R_DrawPlanes: drawsegs overflow (%i)", ds_p - drawsegs);
    if (lastvisplane - visplanes > numvisplanes)
        I_Error("R_DrawPlanes: visplane overflow (%i)",
                lastvisplane - visplanes);
    if (lastopening - openings > MAXOPENINGS)
        I_Error("R_DrawPlanes: opening overflow (%i)",
                lastopening - openings);
#endif

    for (pl = visplanes; pl < lastvisplane; pl++)
    {
        if (pl->minx > pl->maxx)
            continue;
        //
        // sky flat
        //
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = skyiscale;

            // [JN] Invulnerability effect will colorize sky texture
            if (invul_sky && !vanillaparm)
            dc_colormap = (fixedcolormap ? fixedcolormap : colormaps);
            else
            dc_colormap = colormaps;    // sky is allways drawn full bright

            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture]>>FRACBITS;
            for (x = pl->minx; x <= pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];
                if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                {
                    angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);

                    count = dc_yh - dc_yl;
                    if (count < 0)
                        return;

#ifdef RANGECHECK
                    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0
                        || dc_yh >= SCREENHEIGHT)
                        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh,
                                dc_x);
#endif

                    dest = ylookup[dc_yl] + columnofs[dc_x];

                    fracstep = dc_iscale;
                    frac = dc_texturemid + (dc_yl - centery) * fracstep;
                    do
                    {
                        *dest = dc_source[frac >> FRACBITS];
                        dest += SCREENWIDTH;
                        frac += fracstep;
                    }
                    while (count--);

                    // [JN] Initially it was commented out. Colfunc() calls R_DrawColumn()
                    // which, originally, doesn't know how to draw textures taller than 128 pixels.
                    // I'm using R_DrawColumn with enhancements by Lee Killough, so now it actually
                    // can draw 128++ tall textures, but height itself must be defined in TEXTURE lump.
                    colfunc ();
                }
            }
            continue;
        }

        //
        // regular flat
        //
        lumpnum = firstflat + flattranslation[pl->picnum];

        tempSource = W_CacheLumpNum(lumpnum, PU_STATIC);

        switch (pl->special)
        {
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:           // Scroll_North
                ds_source = tempSource;
                break;
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:           // Scroll_East
                ds_source = tempSource + ((63 - ((leveltime >> 1) & 63)) <<
                                          (pl->special - 20) & 63);
                //ds_source = tempSource+((leveltime>>1)&63);
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:           // Scroll_South
                ds_source = tempSource;
                break;
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:           // Scroll_West
                ds_source = tempSource;
                break;
            case 4:            // Scroll_EastLavaDamage
                ds_source =
                    tempSource + (((63 - ((leveltime >> 1) & 63)) << 3) & 63);
                break;
            default:
                ds_source = tempSource;
        }
        planeheight = abs(pl->height - viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS - 1;
        if (light < 0)
            light = 0;
        planezlight = zlight[light];

        // [JN] Apply brightmaps to floor/ceiling...
        if (brightmaps && !vanillaparm &&
        (pl->picnum == bmapflatnum1     // FLOOR21
        || pl->picnum == bmapflatnum2   // FLOOR22
        || pl->picnum == bmapflatnum3   // FLOOR23
        || pl->picnum == bmapflatnum4   // FLOOR24
        || pl->picnum == bmapflatnum5)) // FLOOR26
        planezlight = fullbright_blueonly_floor[light];

        pl->top[pl->maxx+1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
        pl->top[pl->minx-1] = 0xffffffffu; // [crispy] hires / 32-bit integer math

        stop = pl->maxx + 1;
        for (x = pl->minx; x <= stop; x++)
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x],
                        pl->bottom[x]);

        W_ReleaseLumpNum(lumpnum);
    }
}

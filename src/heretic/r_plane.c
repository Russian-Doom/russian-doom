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
// R_planes.c


#include <stdlib.h>
#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "r_local.h"
#include "r_bmaps.h"
#include "r_swirl.h"
#include "jn.h"


/*
================================================================================
=
= MAXVISPLANES is no longer a limit on the number of visplanes,
= but a limit on the number of hash slots; larger numbers mean
= better performance usually but after a point they are wasted,
= and memory and time overheads creep in.
=
= Lee Killough
=
================================================================================
*/

#define MAXVISPLANES	128                  // must be a power of 2

static visplane_t  *visplanes[MAXVISPLANES]; // [JN] killough
static visplane_t  *freetail;                // [JN] killough
static visplane_t **freehead = &freetail;    // [JN] killough
visplane_t         *floorplane, *ceilingplane;

// [JN] killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// [JN] killough 8/1/98: set static number of openings to be large enough
// (a static limit is okay in this case and avoids difficulties in r_segs.c)

size_t  maxopenings;
int     *openings, *lastopening;  // [crispy] 32-bit integer math    
    
// clip values are the solid pixel bounding the range
// floorclip starts out SCREENHEIGHT
// ceilingclip starts out -1
//
// [JN] e6y: resolution limitation is removed

int *floorclip = NULL;    // dropoff overflow
int *ceilingclip = NULL;  // dropoff overflow

// spanstart holds the start of a plane span
// initialized to 0 at start
//
// [JN] e6y: resolution limitation is removed

static int *spanstart = NULL;  // killough 2/8/98

//
// texture mapping
//

static lighttable_t **planezlight;
static fixed_t planeheight;
static fixed_t cachedheight[SCREENHEIGHT];
static fixed_t cacheddistance[SCREENHEIGHT];
static fixed_t cachedxstep[SCREENHEIGHT];
static fixed_t cachedystep[SCREENHEIGHT];

//
// sky mapping
//
int     skyflatnum;
int     skytexture;
int     skytexturemid;
fixed_t skyiscale;
fixed_t skyiscale_low;
fixed_t skytextureheight;

// [JN] e6y: resolution limitation is removed
fixed_t *yslope = NULL;
fixed_t *distscale = NULL;
fixed_t yslopes[LOOKDIRS][SCREENHEIGHT];


/*
================================================================================
=
= R_InitPlanesRes
=
================================================================================
*/

void R_InitPlanesRes (void)
{
    if (floorclip)
    {
        free(floorclip);
    }
    if (ceilingclip)
    {
        free(ceilingclip);
    }
    if (spanstart)
    {
        free(spanstart);
    }
    if (yslope)
    {
        free(yslope);
    }
    if (distscale)
    {
        free(distscale);
    }

    floorclip = calloc(1, screenwidth * sizeof(*floorclip));
    ceilingclip = calloc(1, screenwidth * sizeof(*ceilingclip));
    spanstart = calloc(1, screenwidth * sizeof(*spanstart));
    yslope = calloc(1, screenwidth * sizeof(*yslope));
    distscale = calloc(1, screenwidth * sizeof(*distscale));
}

/*
================================================================================
=
= R_InitVisplanesRes
=
================================================================================
*/

void R_InitVisplanesRes (void)
{
    freetail = NULL;
    freehead = &freetail;

    for (int i = 0; i < MAXVISPLANES; i++)
    {
        visplanes[i] = 0;
    }
}

/*
================================================================================
=
= R_InitSkyMap
=
= Called whenever the view size changes.
=
================================================================================
*/

void R_InitSkyMap (void)
{
    skyflatnum = R_FlatNumForName(DEH_String("F_SKY1"));
    skytexturemid = 200 * FRACUNIT;
    // [JN] Tutti-Frutti fix for sky texture.
    // Use actual texture height (200), instead of dc_texheight (128).
    skyiscale = FRACUNIT >> hires;
    skyiscale_low = FRACUNIT;
    skytextureheight = 200;
}

/*
================================================================================
=
= R_MapPlane
=
= global vars:
= 
= planeheight
= ds_source
= viewx
= viewy
= 
= BASIC PRIMITIVE
================================================================================
*/

void R_MapPlane (int y, int x1, int x2)
{
    fixed_t   distance;
    unsigned  index;
    int       dx, dy;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned) y > viewheight)
        I_Error(english_language ?
                "R_MapPlane: %i, %i at %i" :
                "R_MapPlane: %i, %i в %i",
                x1, x2, y);
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
    {
        ds_colormap = fixedcolormap;
    }
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

/*
================================================================================
=
= R_ClearPlanes
=
= At begining of frame.
=
================================================================================
*/

void R_ClearPlanes (void)
{
    int i;

    // opening / clipping determination
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));
}

/*
================================================================================
=
= new_visplane
=
= [crispy] remove MAXVISPLANES Vanilla limit
= New function, by Lee Killough
=
================================================================================
*/

static visplane_t *new_visplane(unsigned int hash)
{
    visplane_t *check = freetail;

    if (!check)
    {
        check = calloc(1, sizeof(*check));
    }
    else if (!(freetail = freetail->next))
    {
        freehead = &freetail;
    }
    check->next = visplanes[hash];
    visplanes[hash] = check;
    return check;
}

/*
================================================================================
=
= R_FindPlane
=
================================================================================
*/

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, int special)
{
    visplane_t   *check;
    unsigned int  hash;

    if (picnum == skyflatnum)
    {
        // all skies map together
        height = 0;
        lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);

    for (check = visplanes[hash]; check; check = check->next)
        if (height == check->height && picnum == check->picnum
        && lightlevel == check->lightlevel && special == check->special)
            return check;

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->special = special;
    check->minx = screenwidth;
    check->maxx = -1;

    memset(check->top, 0xff, sizeof(check->top));

    return (check);
}

/*
================================================================================
=
= R_DupPlane
=
================================================================================
*/

visplane_t *R_DupPlane (const visplane_t *pl, int start, int stop)
{
    unsigned int  hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
    visplane_t   *new_pl = new_visplane(hash);

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->special = pl->special;
    new_pl->minx = start;
    new_pl->maxx = stop;

    for (int i = 0; i != screenwidth; i++)
    {
        new_pl->top[i] = SHRT_MAX;
    }

    return new_pl;
}

/*
================================================================================
=
= R_CheckPlane
=
================================================================================
*/

visplane_t *R_CheckPlane (visplane_t *pl, int start, int stop)
{
    int intrl, intrh, unionl, unionh, x;

    if (start < pl->minx)
    {
        intrl = pl->minx, unionl = start;
    }
    else
    {
        unionl = pl->minx, intrl = start;
    }

    if (stop  > pl->maxx)
    {
        intrh = pl->maxx, unionh = stop;
    }
    else
    {
        unionh = pl->maxx, intrh  = stop;
    }

    for (x=intrl ; x <= intrh && pl->top[x] == SHRT_MAX; x++); // [crispy] hires / 32-bit integer math
    if (x > intrh)
    {
        // Can use existing plane; extend range
        pl->minx = unionl, pl->maxx = unionh;
        return pl;
    }
    else
    {
        // Cannot use existing plane; create a new one
        return R_DupPlane(pl, start, stop);
    }
}

/*
================================================================================
=
= R_MakeSpans
=
================================================================================
*/

static void
R_MakeSpans (int x, unsigned int t1, unsigned int b1, // [crispy] 32-bit integer math
                    unsigned int t2, unsigned int b2) // [crispy] 32-bit integer math
{
    for ( ; t1 < t2 && t1 <= b1 ; t1++)
    {
        R_MapPlane(t1, spanstart[t1], x-1);
    }
    for ( ; b1 > b2 && b1 >= t1 ; b1--)
    {
        R_MapPlane(b1, spanstart[b1], x-1);
    }
    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2++] = x;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2--] = x;
    }
}

/*
================================================================================
=
= R_DrawPlanes
=
= At the end of each frame.
=
================================================================================
*/

void R_DrawPlanes (void)
{
    visplane_t  *pl;
    int          i;
    int          light;
    int          x, stop;
    int          lumpnum;
    int          angle;
    byte        *tempSource;

    for (i = 0 ; i < MAXVISPLANES ; i++)
    for (pl = visplanes[i] ; pl ; pl = pl->next, rendered_visplanes++)
    if (pl->minx <= pl->maxx)
    {
        //
        // sky flat
        //
        if (pl->picnum == skyflatnum)
        {
            for (x = pl->minx; x <= pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                {


                    // [crispy] Optionally draw skies horizontally linear.
                    angle = ((viewangle + (linear_sky && !vanillaparm ? linearskyangle[x] : 
                                           xtoviewangle[x])) ^ flip_levels) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle , false);

                    if (invul_sky && !vanillaparm)
                    {
                        if (players[consoleplayer].powers[pw_invulnerability] > BLINKTHRESHOLD
                        || (players[consoleplayer].powers[pw_invulnerability] & 8))
                        {
                            // [JN] Invulnerability effect will colorize sky texture,
                            // with out any changes in sky texture light level.
                            dc_colormap = fixedcolormap;
                        }
                        else
                        {
                            // [JN] Otherwise, sky is allways drawn full bright.
                            dc_colormap = colormaps;
                        }
                    }
                    else
                    {
                        // sky is allways drawn full bright
                        dc_colormap = colormaps;
                    }

                    skycolfunc ();
                }
            }
            continue;
        }
        //
        // regular flat
        //
        else
        {
            const int scrollOffset = leveltime >> 1 & 63;
            lumpnum = firstflat + flattranslation[pl->picnum];
            // [crispy] add support for SMMU swirling flats
            tempSource = (flattranslation[pl->picnum] == -1) ?
                          R_DistortedFlat(pl->picnum) :
                          W_CacheLumpNum(lumpnum, PU_STATIC);


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
                    ds_source = tempSource + ((63 - scrollOffset) << (pl->special - 20) & 63);
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
                    ds_source = tempSource + (((63 - scrollOffset) << 3) & 63);
                break;
                //
                // [JN] Custom sector effects for scrolling in any direction:
                //
                case 250: case 260: case 270: case 280:  // Scroll_North
                    ds_source = tempSource + ((scrollOffset << ((pl->special/10) - 25) & 63) << 6);
                break;
                case 200: case 210: case 220: case 230:  // Scroll_East
                    ds_source = tempSource + ((63 - scrollOffset) << ((pl->special/10) - 20) & 63);
                break;
                case 300: case 310: case 320: case 330:  // Scroll_South
                    ds_source = tempSource + (((63 - scrollOffset) << ((pl->special/10) - 30) & 63) << 6);
                break;
                case 350: case 360: case 370: case 380:  // Scroll_West
                    ds_source = tempSource + (scrollOffset << ((pl->special/10) - 35) & 63);
                break;
                //
                // [JN] Extra cases to fix reverse scrolling bug for fastest scrollers.
                // Use speed from "fast", not from "fastest" types.
                //
                case 290:  ds_source = tempSource + ((scrollOffset << 3 & 63) << 6);         break;  // Scroll_North
                case 240:  ds_source = tempSource + ((63 - scrollOffset) << 3 & 63);         break;  // Scroll_East
                case 340:  ds_source = tempSource + (((63 - scrollOffset) << 3 & 63) << 6);  break;  // Scroll_South
                case 390:  ds_source = tempSource + (scrollOffset << 3 & 63);                break;  // Scroll_West

                default:
                    ds_source = tempSource;
                break;
            }

            planeheight = abs(pl->height - viewz);
            light = ((pl->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;

            if (light >= LIGHTLEVELS)
            {
                light = LIGHTLEVELS - 1;
            }
            if (light < 0)
            {
                light = 0;
            }

            stop = pl->maxx + 1;
            planezlight = zlight[light];
            pl->top[pl->minx-1] = pl->top[stop] = UINT_MAX; // [crispy] 32-bit integer math

            // [JN] Apply brightmaps to floor/ceiling...
            if (brightmaps && !vanillaparm
            &&(pl->picnum == bmapflatnum1   // FLOOR21
            || pl->picnum == bmapflatnum2   // FLOOR22
            || pl->picnum == bmapflatnum3   // FLOOR23
            || pl->picnum == bmapflatnum4   // FLOOR24
            || pl->picnum == bmapflatnum5)) // FLOOR26
            {
                planezlight = fullbright_blueonly_floor[light];
            }

            for (x = pl->minx ; x <= stop ; x++)
            {
                R_MakeSpans(x,pl->top[x-1], pl->bottom[x-1], pl->top[x], pl->bottom[x]);
            }

            // [crispy] add support for SMMU swirling flats
            if (flattranslation[pl->picnum] != -1)
            {
                W_ReleaseLumpNum(lumpnum);
            }
        }
    }
}

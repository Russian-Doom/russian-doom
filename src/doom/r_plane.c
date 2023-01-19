//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//


#include <string.h>
#include <stdlib.h>
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "r_local.h"
#include "jn.h"


// -----------------------------------------------------------------------------
// MAXVISPLANES is no longer a limit on the number of visplanes,
// but a limit on the number of hash slots; larger numbers mean
// better performance usually but after a point they are wasted,
// and memory and time overheads creep in.
//
// Lee Killough
// -----------------------------------------------------------------------------

#define MAXVISPLANES	128                  // must be a power of 2

static visplane_t *visplanes[MAXVISPLANES];  // [JN] killough
static visplane_t *freetail;                 // [JN] killough
static visplane_t **freehead = &freetail;    // [JN] killough
visplane_t *floorplane, *ceilingplane;

// [JN] killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// [JN] killough 8/1/98: set static number of openings to be large enough
// (a static limit is okay in this case and avoids difficulties in r_segs.c)

size_t maxopenings;
int *openings, *lastopening; // [crispy] 32-bit integer math

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//

// [JN] Andrey Budko: resolution limitation is removed
int *floorclip = NULL;    // dropoff overflow
int *ceilingclip = NULL;  // dropoff overflow

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//

// [JN] Andrey Budko: resolution limitation is removed
static int *spanstart = NULL;  // killough 2/8/98


//
// texture mapping
//

static lighttable_t **planezlight;
static fixed_t planeheight;
static fixed_t cachedheight[MAXHEIGHT];
static fixed_t cacheddistance[MAXHEIGHT];
static fixed_t cachedxstep[MAXHEIGHT];
static fixed_t cachedystep[MAXHEIGHT];

// [JN] Andrey Budko: resolution limitation is removed
fixed_t *yslope = NULL;
fixed_t *distscale = NULL;
fixed_t  yslopes[MAXHEIGHT][MAXHEIGHT];


// -----------------------------------------------------------------------------
// R_InitPlanesRes
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// R_InitPlanesRes
// -----------------------------------------------------------------------------

void R_InitVisplanesRes (void)
{
    freetail = NULL;
    freehead = &freetail;

    for (int i = 0; i < MAXVISPLANES; i++)
    {
        visplanes[i] = 0;
    }
}

// -----------------------------------------------------------------------------
// R_MapPlane
//
// Uses global vars:
//  - planeheight
//  - ds_source
//  - viewx
//  - viewy
//
// BASIC PRIMITIVE
// -----------------------------------------------------------------------------

static void R_MapPlane (const int y, const int x1, const int x2)
{
    unsigned index;
    int      dx, dy;
    fixed_t	 distance;
    fixed_t	 ds_xfrac, ds_yfrac;
    fixed_t	 ds_xstep, ds_ystep;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || y > viewheight)
    {
        I_Error (english_language ? 
                 "R_MapPlane: %i, %i at %i" :
                 "R_MapPlane: %i, %i у %i", x1, x2, y);
    }
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

    // [JN] Add deltas to flow effect of swirling liquids.
    ds_xfrac =  viewx + FlowDelta_X + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy + FlowDelta_Y - FixedMul(viewsin, distance) + dx * ds_ystep;

    if (fixedcolormap)
    {
        ds_colormap[0] = ds_colormap[1] = fixedcolormap;
    }
    else
    {
        // [JN] Note: no smoother diminished lighting in -vanilla mode
        index = distance >> lightzshift;

        if (index >= maxlightz)
            index = maxlightz-1;

        ds_colormap[0] = planezlight[index];
        ds_colormap[1] = colormaps;
    }

    // high or low detail
    spanfunc (x1, x2, y, ds_xfrac, ds_xstep, ds_yfrac, ds_ystep);
}

// -----------------------------------------------------------------------------
// R_ClearPlanes
// At begining of frame.
// -----------------------------------------------------------------------------

void R_ClearPlanes (void)
{
    int i;

    // opening / clipping determination
    for (i = 0 ; i < viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (i = 0; i < MAXVISPLANES; i++)  // [JN] new code -- killough
        for (*freehead = visplanes[i], visplanes[i] = NULL ; *freehead ; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));
}

// -----------------------------------------------------------------------------
// [crispy] remove MAXVISPLANES Vanilla limit
// New function, by Lee Killough
// -----------------------------------------------------------------------------

static visplane_t *new_visplane (unsigned const int hash)
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

// -----------------------------------------------------------------------------
// R_FindPlane
// -----------------------------------------------------------------------------

visplane_t *R_FindPlane (fixed_t height, const int picnum, const int lightlevel, const int flow)
{
    visplane_t *check;
    unsigned int hash;

    if (picnum == skyflatnum)
    {
        height = 0; // all skys map together
        // [JN] Don't modify lightlevel parameter of visplane with sky texture.
        // Otherwise hash function will consider it as unique one, forcing 
        // creation of new visplane.
        // lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);

    for (check = visplanes[hash]; check; check = check->next)
        if (height == check->height && picnum == check->picnum 
        && lightlevel == check->lightlevel && flow == check->flow)
            return check;

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->flow = flow;
    check->minx = screenwidth;
    check->maxx = -1;

    memset(check->top, UINT_MAX, sizeof(check->top));

    return check;
}

// -----------------------------------------------------------------------------
// R_DupPlane
// -----------------------------------------------------------------------------

visplane_t *R_DupPlane(const visplane_t *pl, const int start, const int stop)
{
    visplane_t  *new_pl = new_visplane(visplane_hash(pl->picnum, pl->lightlevel, pl->height));

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->flow = pl->flow;
    new_pl->minx = start;
    new_pl->maxx = stop;

    memset(new_pl->top, UINT_MAX, sizeof(new_pl->top));

    return new_pl;
}

// -----------------------------------------------------------------------------
// R_CheckPlane
// -----------------------------------------------------------------------------

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

    for (x=intrl ; x <= intrh && pl->top[x] == UINT_MAX; x++); // [crispy] hires / 32-bit integer math
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

// -----------------------------------------------------------------------------
// R_MakeSpans
// -----------------------------------------------------------------------------

static void
R_MakeSpans (const int x, unsigned int t1, unsigned int b1, // [crispy] 32-bit integer math
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

// -----------------------------------------------------------------------------
// R_DrawPlanes
// At the end of each frame.
// -----------------------------------------------------------------------------

void R_DrawPlanes (void) 
{
    for (int i = 0 ; i < MAXVISPLANES ; i++)
    for (visplane_t *pl = visplanes[i] ; pl ; pl = pl->next, rendered_visplanes++)
    if (pl->minx <= pl->maxx)
    {
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = pspriteiscale>>(detailshift && !hires);
            
            // [JN] Scale sky texture if appropriate.
            if (mlook && scaled_sky)
            {
                dc_iscale /= 2;
            }

            // Sky is allways drawn full bright, i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected by INVUL inverse mapping.
            // [JN] Make optional, "Invulnerability affects sky" feature.
            
            if (invul_sky && !vanillaparm)
            {
                dc_colormap[0] = dc_colormap[1] = fixedcolormap ? fixedcolormap : colormaps;
            }
            else
            {
                dc_colormap[0] = dc_colormap[1] = colormaps;
            }

            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture]>>FRACBITS;

            for (int x = pl->minx ; x <= pl->maxx ; x++)
            {
                if ((dc_yl = pl->top[x]) != UINT_MAX && dc_yl <= (dc_yh = pl->bottom[x])) // [crispy] 32-bit integer math
                {
                    // [crispy] Optionally draw skies horizontally linear.
                    const int angle = ((viewangle + (linear_sky && !vanillaparm ?
                                        linearskyangle[x] : xtoviewangle[x]))^flip_levels)>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    colfunc ();
                }
            }
        }
        else  // regular flat
        {
            int light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
            const int stop = pl->maxx + 1;
            const int lumpnum = firstflat + flattranslation[pl->picnum];

            // [crispy] add support for SMMU swirling flats
            ds_source = (flattranslation[pl->picnum] == -1) ?
                         R_DistortedFlat(pl->picnum) : W_CacheLumpNum(lumpnum, PU_STATIC);
            ds_brightmap = R_BrightmapForFlatNum(lumpnum-firstflat);

            // [JN] Apply flow effect to swirling liquids.
            if (swirling_liquids && flattranslation[pl->picnum] == -1 && !vanillaparm)
            {
                R_FlowPlane(pl->flow);
            }
            else
            {
                FlowDelta_X = 0;
                FlowDelta_Y = 0;
            }

            planeheight = abs(pl->height-viewz);
            if (light >= LIGHTLEVELS)
            {
                light = LIGHTLEVELS-1;
            }
            if (light < 0)
            {
                light = 0;
            }
            planezlight = zlight[light];
            pl->top[pl->minx-1] = pl->top[stop] = UINT_MAX; // [crispy] 32-bit integer math

            for (int x = pl->minx ; x <= stop ; x++)
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

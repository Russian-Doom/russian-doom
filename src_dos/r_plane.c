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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//

#include <stdlib.h>
#include <limits.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "r_bmaps.h"
#include "r_main.h"
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

// [JN] e6y: resolution limitation is removed
int *floorclip = NULL;    // dropoff overflow
int *ceilingclip = NULL;  // dropoff overflow

//
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

// [JN] e6y: resolution limitation is removed
fixed_t *yslope = NULL;
fixed_t *distscale = NULL;
fixed_t  yslopes[LOOKDIRS][SCREENHEIGHT];


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

    floorclip = calloc(1, SCREENWIDTH * sizeof(*floorclip));
    ceilingclip = calloc(1, SCREENWIDTH * sizeof(*ceilingclip));
    spanstart = calloc(1, SCREENWIDTH * sizeof(*spanstart));
    yslope = calloc(1, SCREENWIDTH * sizeof(*yslope));
    distscale = calloc(1, SCREENWIDTH * sizeof(*distscale));
}

// -----------------------------------------------------------------------------
// R_InitPlanesRes
// -----------------------------------------------------------------------------

void R_InitVisplanesRes (void)
{
    int i;

    freetail = NULL;
    freehead = &freetail;

    for (i = 0; i < MAXVISPLANES; i++)
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

static void R_MapPlane (int y, int x1, int x2)
{
    int         dx, dy;
    unsigned    index;
    fixed_t     distance;

#ifdef RANGECHECK
    if (x2 < x1 ||  x1 < 0 || x2 >=viewwidth || (unsigned)y > viewheight)
    {
        I_Error (english_language ?
                "R_MapPlane: %i, %i at %i" :
                "R_MapPlane: %i, %i ¢ %i", x1,x2,y);
    }
#endif

    // [crispy] visplanes with the same flats now match up far better than before
    // adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math

    // [crispy] avoid division by zero if (y == centery)
    if (!(dy = abs(centery - y)))
    {
        return;
    }

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);

        // [JN] Blocky mode, need to multiply by 2. Sounds familiar? :)
        ds_xstep = cachedxstep[y] 
                 = FixedMul (viewsin, planeheight) / dy * (detailshift ? 2 : 1);
        ds_ystep = cachedystep[y] 
                 = FixedMul (viewcos, planeheight) / dy * (detailshift ? 2 : 1);
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
        // [JN] Note: no smooth diminished lighting in -vanilla mode
        index = distance >> lightzshift;

        if (index >= maxlightz)
            index = maxlightz-1;

        ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc ();	
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

static visplane_t *new_visplane (unsigned int hash)
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

visplane_t *R_FindPlane (fixed_t height, int picnum, int lightlevel)
{
    int i;
    unsigned int  hash;
    visplane_t   *check;

    if (picnum == skyflatnum)
    {
        height = 0; // all skys map together
        lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);
    
    for (check = visplanes[hash]; check; check = check->next)
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel)
            return check;

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;

    for (i = 0; i != SCREENWIDTH; i++)
    {
        check->top[i] = SHRT_MAX;
    }

    return check;
}

// -----------------------------------------------------------------------------
// R_DupPlane
// -----------------------------------------------------------------------------

visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{
    int i;
    visplane_t  *new_pl = new_visplane(visplane_hash(pl->picnum, pl->lightlevel, pl->height));

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->minx = start;
    new_pl->maxx = stop;

    for (i = 0; i != SCREENWIDTH; i++)
    {
        new_pl->top[i] = SHRT_MAX;
    }

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

// -----------------------------------------------------------------------------
// R_MakeSpans
// -----------------------------------------------------------------------------

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


// [crispy] add support for SMMU swirling flats
// adapted from smmu/r_ripple.c, by Simon Howard
static char *R_DistortedFlat (int flatnum)
{
    const int swirlfactor = 8192 / 64;
    const int swirlfactor2 = 8192 / 32;
    const int amp = 2;
    const int amp2 = 2;
    const int speed = 40;

    static int swirltic;
    static int offset[4096];

    static char distortedflat[4096];
    char *normalflat;
    int i;

    if (swirltic != gametic)
    {
    int x, y;

    for (x = 0; x < 64; x++)
    {
        for (y = 0; y < 64; y++)
        {
        int x1, y1;
        int sinvalue, sinvalue2;

        sinvalue = (y * swirlfactor + leveltime * speed * 5 + 900) & 8191;
        sinvalue2 = (x * swirlfactor2 + leveltime * speed * 4 + 300) & 8191;
        x1 = x + 128
           + ((finesine[sinvalue] * amp) >> FRACBITS)
           + ((finesine[sinvalue2] * amp2) >> FRACBITS);

        sinvalue = (x * swirlfactor + leveltime * speed * 3 + 700) & 8191;
        sinvalue2 = (y * swirlfactor2 + leveltime * speed * 4 + 1200) & 8191;
        y1 = y + 128
           + ((finesine[sinvalue] * amp) >> FRACBITS)
           + ((finesine[sinvalue2] * amp2) >> FRACBITS);

        x1 &= 63;
        y1 &= 63;

        offset[(y << 6) + x] = (y1 << 6) + x1;
        }
    }

    swirltic = gametic;
    }

    // [JN] Use only defined flat
    normalflat = W_CacheLumpNum(firstflat + flatnum, PU_LEVEL);

    for (i = 0; i < 4096; i++)
    {
    distortedflat[i] = normalflat[offset[i]];
    }

    Z_ChangeTag(normalflat, PU_CACHE);

    return distortedflat;
}

// -----------------------------------------------------------------------------
// R_DrawPlanes
// At the end of each frame.
// -----------------------------------------------------------------------------

void R_DrawPlanes (void)
{
    int          light;
    int          i, x;
    int          stop;
    int          angle;
    visplane_t  *pl;

    for (i = 0 ; i < MAXVISPLANES ; i++)
    {
    for (pl = visplanes[i] ; pl ; pl = pl->next)
    {
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            // [JN] Original:
            dc_iscale = pspriteiscale>>detailshift;
            // [JN] Mouselook addition:
            if (mlook)
            {
                dc_iscale = dc_iscale / 2;
            }

            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.

            // [JN] INVUL sphere now affects the sky
            if (invul_sky && !vanilla)
            {
                dc_colormap = (fixedcolormap ? fixedcolormap : colormaps);
            }
            else
            {
                dc_colormap = colormaps;
            }

            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture]>>FRACBITS;

            for (x=pl->minx ; x <= pl->maxx ; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if ((dc_yl = pl->top[x]) != SHRT_MAX && dc_yl <= (dc_yh = pl->bottom[x])) // [crispy] 32-bit integer math
                {
                    angle = (viewangle + (linear_sky && !vanilla ? 
                                          linearskyangle[x] : xtoviewangle[x])) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle, false);
                    colfunc ();
                }
            }
            continue;
        }

        // regular flat
        // [crispy] add support for SMMU swirling flats
        ds_source = (flattranslation[pl->picnum] == -1) ?
                    R_DistortedFlat(pl->picnum) :
                    W_CacheLumpNum(firstflat + flattranslation[pl->picnum], PU_STATIC);

        planeheight = abs(pl->height-viewz);
        light = ((pl->lightlevel + level_brightness)
                >> LIGHTSEGSHIFT) + extralight;

        if (light >= LIGHTLEVELS)
        {
            light = LIGHTLEVELS-1;
        }
        if (light < 0)
        {
            light = 0;
        }

        planezlight = zlight[light];

        // [JN] Applying brightmaps to floor/ceiling...
        if (brightmaps && !vanilla
        && (pl->picnum == bmapflatnum1      // CONS1_1
        ||  pl->picnum == bmapflatnum2      // CONS1_5
        ||  pl->picnum == bmapflatnum3))    // CONS1_7
        {
            planezlight = fullbright_notgrayorbrown_floor[light];
        }

        pl->top[pl->maxx+1] = SHRT_MAX; // [crispy] hires / 32-bit integer math
        pl->top[pl->minx-1] = SHRT_MAX; // [crispy] hires / 32-bit integer math

        stop = pl->maxx + 1;

        for (x=pl->minx ; x<= stop ; x++)
        {
            R_MakeSpans(x,pl->top[x-1],
                        pl->bottom[x-1],
                        pl->top[x],
                        pl->bottom[x]);
        }

        // [crispy] add support for SMMU swirling flats
        if (flattranslation[pl->picnum] != -1)
        {
            Z_ChangeTag (ds_source, PU_CACHE);
        }
    }
    }
}

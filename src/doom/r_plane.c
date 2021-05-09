//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//



#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_data.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_swirl.h"
#include "r_bmaps.h"

#include "jn.h"

extern boolean scaled_sky;

planefunction_t floorfunc;
planefunction_t ceilingfunc;


//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES	128
static visplane_t   *visplanes[MAXVISPLANES];   // killough
static visplane_t   *freetail;                  // killough
static visplane_t   **freehead = &freetail;     // killough
visplane_t*     floorplane;
visplane_t*     ceilingplane;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// ?
#define MAXOPENINGS WIDESCREENWIDTH*64*4 
int     openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int*    lastopening;           // [crispy] 32-bit integer math


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int floorclip[WIDESCREENWIDTH];   // [crispy] 32-bit integer math
int ceilingclip[WIDESCREENWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t**		planezlight;
fixed_t			planeheight;

fixed_t* yslope;
fixed_t yslopes[LOOKDIRS][SCREENHEIGHT];
fixed_t distscale[WIDESCREENWIDTH];
fixed_t basexscale;
fixed_t baseyscale;

fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];



//
// R_InitPlanes
// Only at game startup.
//
// void R_InitPlanes (void)
// {
//     // Doh!
// }


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
void R_MapPlane (int y, int x1, int x2)
{
    // [crispy] see below
    //  angle_t	angle;
    fixed_t	distance;
    //  fixed_t	length;
    unsigned index;
    int      dx, dy;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || y > viewheight)
    {
        I_Error (english_language ?
                 "R_MapPlane: %i, %i at %i" :
                 "R_MapPlane: %i, %i у %i",
                 x1,x2,y);
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

    ds_xfrac = viewx + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy - FixedMul(viewsin, distance) + dx * ds_ystep;

    if (fixedcolormap)
    {
        ds_colormap = fixedcolormap;
    }
    else
    {
        // [JN] Note: no smoother diminished lighting in -vanilla mode
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


//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes (void)
{
    int     i;
    const angle_t angle = (viewangle-ANG90)>>ANGLETOFINESHIFT; // left to right mapping

    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (int i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}


// [crispy] remove MAXVISPLANES Vanilla limit
// New function, by Lee Killough
static visplane_t *new_visplane(unsigned int hash)
{
    visplane_t  *check = freetail;

    if (!check)
        check = calloc(1, sizeof(*check));
    else if (!(freetail = freetail->next))
        freehead = &freetail;

    check->next = visplanes[hash];
    visplanes[hash] = check;
    return check;
}


//
// R_FindPlane
//
visplane_t*
R_FindPlane (fixed_t height, int picnum, int lightlevel)
{
    visplane_t* check;
    unsigned int hash;

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
    check->minx = WIDESCREENWIDTH;
    check->maxx = -1;

    memset (check->top,0xff,sizeof(check->top));

    return check;
}


//
// R_DupPlane
//
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{
    unsigned int    hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
    visplane_t      *new_pl = new_visplane(hash);

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->minx = start;
    new_pl->maxx = stop;

    memset(new_pl->top, USHRT_MAX, sizeof(new_pl->top));

    return new_pl;
}


//
// R_CheckPlane
//
visplane_t*
R_CheckPlane (visplane_t* pl, int start, int stop)
{
    int intrl;
    int intrh;
    int unionl;
    int unionh;
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

    for (x=intrl ; x <= intrh && pl->top[x] == 0xffffffffu; x++); // [crispy] hires / 32-bit integer math
    // [crispy] fix HOM if ceilingplane and floorplane are the same
    // visplane (e.g. both are skies)
    if (!(pl == floorplane && markceiling && floorplane == ceilingplane) && x > intrh)
    {
        // Can use existing plane; extend range
        pl->minx = unionl, pl->maxx = unionh;
    }
    else
    {
        // Cannot use existing plane; create a new one
        return R_DupPlane(pl,start,stop);
    }

    return pl;
}


//
// R_MakeSpans
//
void
R_MakeSpans (int x, 
unsigned int		t1, // [crispy] 32-bit integer math
unsigned int		b1, // [crispy] 32-bit integer math
unsigned int		t2, // [crispy] 32-bit integer math
unsigned int		b2  // [crispy] 32-bit integer math
)
{
    while (t1 < t2 && t1<=b1)
    {
        R_MapPlane (t1,spanstart[t1],x-1);
        t1++;
    }
    while (b1 > b2 && b1>=t1)
    {
        R_MapPlane (b1,spanstart[b1],x-1);
        b1--;
    }

    while (t2 < t1 && t2<=b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2>=t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}


//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes (void) 
{
    visplane_t* pl;
    int         i;
    int         light;
    int         x;
    int         stop;
    int         angle;
    int         lumpnum;


    for (i = 0 ; i < MAXVISPLANES ; i++)
    for (pl = visplanes[i] ; pl ; pl = pl->next)
    if (pl->minx <= pl->maxx)
    {
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            // [JN] Original:
            dc_iscale = pspriteiscale>>(detailshift && !hires);
            
            // [JN] Mouselook addition
            if (mlook && scaled_sky)
            dc_iscale = dc_iscale / 2;

            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.

            // [JN] Окрашивание неба при неузязвимости.
            if (invul_sky && !vanillaparm)
            dc_colormap = (fixedcolormap ? fixedcolormap : colormaps);
            else
            dc_colormap = colormaps;

            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture]>>FRACBITS;

            for (x=pl->minx ; x <= pl->maxx ; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                {
                    // [crispy] Optionally draw skies horizontally linear.
                    angle = ((viewangle + (linear_sky && !vanillaparm ? linearskyangle[x] : 
                                           xtoviewangle[x]))^flip_levels)>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle, false);
                    colfunc ();
                }
            }
        }

        // regular flat
        else
        {
        lumpnum = firstflat + flattranslation[pl->picnum];
        // [crispy] add support for SMMU swirling flats
        ds_source = (flattranslation[pl->picnum] == -1) ?
                    R_DistortedFlat(pl->picnum) :
                    W_CacheLumpNum(lumpnum, PU_STATIC);

        planeheight = abs(pl->height-viewz);
        light = ((pl->lightlevel+level_brightness) >> LIGHTSEGSHIFT)+extralight;

        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS-1;

        if (light < 0)
            light = 0;

        planezlight = zlight[light];

        // [JN] Apply brightmaps to floor/ceiling...
        if (brightmaps && brightmaps_allowed)
        {
            if (pl->picnum == bmapflatnum1  // CONS1_1
            ||  pl->picnum == bmapflatnum2  // CONS1_5
            ||  pl->picnum == bmapflatnum3) // CONS1_7
            {
                planezlight = fullbright_notgrayorbrown_floor[light];
            }

            if (pl->picnum == bmapflatnum4) // GATE6
            {
                planezlight = fullbright_orangeyellow_floor[light];
            }
        }

        pl->top[pl->maxx+1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
        pl->top[pl->minx-1] = 0xffffffffu; // [crispy] hires / 32-bit integer math

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
            W_ReleaseLumpNum(lumpnum);
        }
        }
    }
}

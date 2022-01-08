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


// Sky mapping
int Sky1Texture;
int Sky2Texture;
fixed_t Sky1ColumnOffset;
fixed_t Sky2ColumnOffset;
int skyflatnum;
static int skytexturemid;
static boolean DoubleSky;
extern fixed_t Sky1ScrollDelta;
extern fixed_t Sky2ScrollDelta;
#define SKYTEXTUREMIDSHIFTED 200

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

static visplane_t  *visplanes[MAXVISPLANES];   // [JN] killough
static visplane_t  *freetail;                  // [JN] killough
static visplane_t **freehead = &freetail;      // [JN] killough
visplane_t         *floorplane, *ceilingplane;

// [JN] killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// [JN] killough 8/1/98: set static number of openings to be large enough
// (a static limit is okay in this case and avoids difficulties in r_segs.c)

size_t  maxopenings;
int    *openings, *lastopening;  // [crispy] 32-bit integer math   

// Clip values are the solid pixel bounding the range.
// floorclip start out SCREENHEIGHT, ceilingclip starts out -1
// [JN] e6y: resolution limitation is removed

int *floorclip = NULL;    // dropoff overflow
int *ceilingclip = NULL;  // dropoff overflow

// Spanstart holds the start of a plane span, initialized to 0.
// [JN] e6y: resolution limitation is removed

static int *spanstart = NULL;  // killough 2/8/98

// Texture mapping

static lighttable_t **planezlight;
static fixed_t planeheight;
static fixed_t cachedheight[SCREENHEIGHT];
static fixed_t cacheddistance[SCREENHEIGHT];
static fixed_t cachedxstep[SCREENHEIGHT];
static fixed_t cachedystep[SCREENHEIGHT];

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
= R_InitSky
=
= Called at level load.
=
================================================================================
*/

void R_InitSky (int map)
{
    Sky1Texture = P_GetMapSky1Texture(map);
    Sky2Texture = P_GetMapSky2Texture(map);
    Sky1ScrollDelta = P_GetMapSky1ScrollDelta(map);
    Sky2ScrollDelta = P_GetMapSky2ScrollDelta(map);
    Sky1ColumnOffset = 0;
    Sky2ColumnOffset = 0;
    DoubleSky = P_GetMapDoubleSky(map);
}

/*
================================================================================
=
= R_InitSkyMap
=
= [JN] Called at game startup.
=
================================================================================
*/

void R_InitSkyMap (void)
{
    skyflatnum = R_FlatNumForName("F_SKY");
    skytexturemid = 200 * FRACUNIT;
}

/*
================================================================================
//
// R_MapPlane
//
// Globals used: planeheight, ds_source, viewx, viewy.
//
================================================================================
*/

void R_MapPlane (int y, int x1, int x2)
{
    fixed_t   distance;
    unsigned  index;
    int       dx, dy;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned) y > viewheight)
    {
        I_Error(english_language ?
                "R_MapPlane: %i, %i at %i" :
                "R_MapPlane: %i, %i у %i",
                x1, x2, y);
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
        index = distance >> LIGHTZSHIFT;
        if (index >= MAXLIGHTZ)
        {
            index = MAXLIGHTZ - 1;
        }
        ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // High or low detail
    spanfunc();
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

    memset(new_pl->top, SHRT_MAX, sizeof(new_pl->top));

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
        return R_DupPlane(pl,start,stop);
    }

    return pl;
}

/*
================================================================================
=
= R_MakeSpans
=
================================================================================
*/

static void R_MakeSpans (int x, unsigned int t1, unsigned int b1, // [crispy] 32-bit integer math
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

void R_DrawPlanes(void)
{
    visplane_t *pl;
    int         i, x, stop;
    int         light, angle;
    int         offset, skyTexture, offset2, skyTexture2;
    int         scrollOffset;
    int         heightmask;
    int         count, frac, fracstep = FRACUNIT >> !detailshift;
    byte       *source, *source2, *tempSource;
    byte       *dest, *dest1, *dest2, *dest3, *dest4;

    extern byte *ylookup[SCREENHEIGHT];
    extern int columnofs[WIDESCREENWIDTH];

    for (i = 0 ; i < MAXVISPLANES ; i++)
    for (pl = visplanes[i] ; pl ; pl = pl->next)
    if (pl->minx <= pl->maxx)
    {
        //
        // Sky flat
        //
        if (pl->picnum == skyflatnum)
        {
            // Render 2 layers, sky 1 in front
            if (DoubleSky)
            {
                offset = Sky1ColumnOffset >> 16;
                skyTexture = texturetranslation[Sky1Texture];
                offset2 = Sky2ColumnOffset >> 16;
                skyTexture2 = texturetranslation[Sky2Texture];

                for (x = pl->minx; x <= pl->maxx; x++)
                {
                    dc_yl = pl->top[x];
                    dc_yh = pl->bottom[x];
                    if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                    {
                        count = dc_yh - dc_yl;
                        if (count < 0)
                        {
                            return;
                        }
                        // [crispy] Optionally draw skies horizontally linear.
                        angle = ((viewangle + (linear_sky && !vanillaparm ?
                                  linearskyangle[x] : xtoviewangle[x])) ^ flip_levels) >> ANGLETOSKYSHIFT;
                        source = R_GetColumn(skyTexture, angle + offset, false);
                        source2 = R_GetColumn(skyTexture2, angle + offset2, false);
                        frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
                        heightmask = SKYTEXTUREMIDSHIFTED-1;

                        // [JN] Double sky drawing - LOW detail
                        if (detailshift)
                        {
                            dest1 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(x << hires)]];
                            dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(x << hires) + 1]];
                            dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(x << hires)]];
                            dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(x << hires) + 1]];

                            // not a power of 2 -- killough
                            if (SKYTEXTUREMIDSHIFTED & heightmask)
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
                                    if (source[frac >> FRACBITS])
                                    {
                                        *dest4 = *dest3 = *dest2 = *dest1 = source[frac >> FRACBITS];
                                    }
                                    else
                                    {
                                        *dest4 = *dest3 = *dest2 = *dest1 = source2[frac >> FRACBITS];
                                    }

                                    dest1 += screenwidth << hires;
                                    dest2 += screenwidth << hires;
                                    dest3 += screenwidth << hires;
                                    dest4 += screenwidth << hires;
                                    frac += fracstep/2; // [JN] Blocky mode, multiple vertically by 2.

                                    if ((frac += (FRACUNIT >> hires)) >= heightmask)
                                    {
                                        frac -= heightmask;
                                    }
                                }
                                while (count--);
                            }
                            // texture height is a power of 2 -- killough
                            else
                            {
                                do
                                {
                                    if (source[(frac >> FRACBITS) & heightmask])
                                    {
                                        *dest4 = *dest3 = *dest2 = *dest1 = source[(frac >> FRACBITS) & heightmask];
                                    }
                                    else
                                    {
                                        *dest4 = *dest3 = *dest2 = *dest1 = source2[(frac >> FRACBITS) & heightmask];
                                    }
    
                                dest1 += screenwidth << hires;
                                dest2 += screenwidth << hires;
                                dest3 += screenwidth << hires;
                                dest4 += screenwidth << hires;
                                frac += fracstep;
    
                                } while (count--);
                            }
                        }
                        // [JN] Double sky drawing - HIGH detail
                        else
                        {
                            dest = ylookup[dc_yl] + columnofs[flipwidth[x]];

                            // not a power of 2 -- killough
                            if (SKYTEXTUREMIDSHIFTED & heightmask)
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
                                    if (source[frac >> FRACBITS])
                                    {
                                        *dest = source[frac >> FRACBITS];
                                    }
                                    else
                                    {
                                        *dest = source2[frac >> FRACBITS];
                                    }
                                    dest += screenwidth;
                                    if ((frac += fracstep) >= heightmask)
                                    {
                                        frac -= heightmask;
                                    }
                                }
                                while (count--);
                            }
                            // texture height is a power of 2 -- killough
                            else
                            {
                                do
                                {
                                    if (source[(frac >> FRACBITS) & heightmask])
                                    {
                                        *dest = source[(frac >> FRACBITS) & heightmask];
                                    }
                                    else
                                    {
                                        *dest = source2[(frac >> FRACBITS) & heightmask];
                                    }
    
                                    dest += screenwidth;
                                    frac += fracstep;
    
                                } while (count--);
                            }
                        }
                    }
                }
                // Next visplane
                continue;
            }
            //
            // Render single layer sky
            //
            else
            {
                if (pl->special == 200)
                {
                    // Use sky 2
                    offset = Sky2ColumnOffset >> 16;
                    skyTexture = texturetranslation[Sky2Texture];
                }
                else
                {
                    // Use sky 1
                    offset = Sky1ColumnOffset >> 16;
                    skyTexture = texturetranslation[Sky1Texture];
                }

                for (x = pl->minx; x <= pl->maxx; x++)
                {
                    dc_yl = pl->top[x];
                    dc_yh = pl->bottom[x];
                    if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                    {
                        count = dc_yh - dc_yl;
                        if (count < 0)
                        {
                            return;
                        }
                        // [crispy] Optionally draw skies horizontally linear.
                        angle = ((viewangle + (linear_sky && !vanillaparm ?
                                  linearskyangle[x] : xtoviewangle[x])) ^ flip_levels) >> ANGLETOSKYSHIFT;
                        source = R_GetColumn(skyTexture, angle + offset, false);
                        frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
                        heightmask = SKYTEXTUREMIDSHIFTED-1;

                        // [JN] Single sky drawing - LOW detail
                        if (detailshift)
                        {
                            dest1 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(x << hires)]];
                            dest2 = ylookup[(dc_yl << hires)] + columnofs[flipwidth[(x << hires) + 1]];
                            dest3 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(x << hires)]];
                            dest4 = ylookup[(dc_yl << hires) + 1] + columnofs[flipwidth[(x << hires) + 1]];

                            // not a power of 2 -- killough
                            if (SKYTEXTUREMIDSHIFTED & heightmask)
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
                                    *dest4 = *dest3 = *dest2 = *dest1 = source[frac >> FRACBITS];

                                    dest1 += screenwidth << hires;
                                    dest2 += screenwidth << hires;
                                    dest3 += screenwidth << hires;
                                    dest4 += screenwidth << hires;
                                    frac += fracstep/2; // [JN] Blocky mode, multiple vertically by 2.

                                    if ((frac += (FRACUNIT >> hires)) >= heightmask)
                                    {
                                        frac -= heightmask;
                                    }
                                }
                                while (count--);
                            }
                            // texture height is a power of 2 -- killough
                            else
                            {
                                do
                                {
                                    *dest4 = *dest3 = *dest2 = *dest1 = source2[(frac >> FRACBITS) & heightmask];
                                        dest1 += screenwidth << hires;
                                    dest2 += screenwidth << hires;
                                    dest3 += screenwidth << hires;
                                    dest4 += screenwidth << hires;
                                    frac += fracstep;
    
                                } while (count--);
                            }
                        }
                        // [JN] Single sky drawing - HIGH detail
                        else
                        {
                            dest = ylookup[dc_yl] + columnofs[flipwidth[x]];

                            // not a power of 2 -- killough
                            if (SKYTEXTUREMIDSHIFTED & heightmask)
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
                                   *dest = source[frac >> FRACBITS];
                                    dest += screenwidth;

                                    if ((frac += fracstep) >= heightmask)
                                    {
                                        frac -= heightmask;
                                    }
                                }
                                while (count--);
                            }
                            // texture height is a power of 2 -- killough
                            else
                            {
                                do 
                                {
                                     *dest = source[(frac >> FRACBITS) & heightmask];
                                    dest += screenwidth;
                                    frac += fracstep;
                                } while (count--);
                            }
                        }
                    }
                }
                // Next visplane
                continue;
            }
        }
        //
        // Regular flat
        //
        else
        {
            tempSource = W_CacheLumpNum(firstflat + flattranslation[pl->picnum], PU_STATIC);
            scrollOffset = leveltime >> 1 & 63;

            // Handle scrolling flats
            switch (pl->special)
            {
                // Scroll_North_xxx
                case 201:
                case 202:
                case 203:
                ds_source = tempSource + ((scrollOffset << (pl->special - 201) & 63) << 6);
                break;

                // Scroll_East_xxx
                case 204:
                case 205:
                case 206:
                ds_source = tempSource + ((63 - scrollOffset) << (pl->special - 204) & 63);
                break;

                // Scroll_South_xxx
                case 207:
                case 208:
                case 209:
                ds_source = tempSource + (((63 - scrollOffset) << (pl->special - 207) & 63) << 6);
                break;

                // Scroll_West_xxx
                case 210:
                case 211:
                case 212:          
                ds_source = tempSource + (scrollOffset << (pl->special - 210) & 63);
                break;

                // Scroll_NorthWest_xxx
                case 213:
                case 214:
                case 215:
                ds_source = tempSource + (scrollOffset << (pl->special - 213) & 63)
                                       + ((scrollOffset << (pl->special - 213) & 63) << 6);
                break;

                // Scroll_NorthEast_xxx
                case 216:
                case 217:
                case 218:
                ds_source = tempSource + ((63 - scrollOffset) << (pl->special - 216) & 63)
                                       + ((scrollOffset << (pl->special - 216) & 63) << 6);
                break;

                // Scroll_SouthEast_xxx
                case 219:
                case 220:
                case 221:
                ds_source = tempSource + ((63 - scrollOffset) << (pl->special - 219) & 63)
                                       + (((63 - scrollOffset) << (pl->special - 219) & 63) << 6);
                break;

                // Scroll_SouthWest_xxx
                case 222:
                case 223:
                case 224:          
                ds_source = tempSource + (scrollOffset << (pl->special - 222) & 63)
                                       + (((63 - scrollOffset) << (pl->special - 222) & 63) << 6);
                break;

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
            
            for (x = pl->minx ; x <= stop ; x++)
            {
                R_MakeSpans(x,pl->top[x-1], pl->bottom[x-1], pl->top[x], pl->bottom[x]);
            }

            W_ReleaseLumpNum(firstflat + flattranslation[pl->picnum]);
        }
    }
}

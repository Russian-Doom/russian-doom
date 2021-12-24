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


#include "h2def.h"
#include "i_system.h"
#include "r_local.h"


// Sky mapping
int Sky1Texture;
int Sky2Texture;
fixed_t Sky1ColumnOffset;
fixed_t Sky2ColumnOffset;
int skyflatnum;
int skytexturemid;
fixed_t skyiscale;
boolean DoubleSky;
extern fixed_t Sky1ScrollDelta;
extern fixed_t Sky2ScrollDelta;

// Opening
static visplane_t *visplanes[MAXVISPLANES]; // [JN] killough
static visplane_t *freetail;                // [JN] killough
static visplane_t **freehead = &freetail;   // [JN] killough
visplane_t *floorplane, *ceilingplane;
planefunction_t floorfunc, ceilingfunc;

// [JN] killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

int  openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int* lastopening;           // [crispy] 32-bit integer math

// Clip values are the solid pixel bounding the range.
// floorclip start out SCREENHEIGHT
// ceilingclip starts out -1
int  floorclip[WIDESCREENWIDTH];   // [crispy] 32-bit integer math
int  ceilingclip[WIDESCREENWIDTH]; // [crispy] 32-bit integer math

// spanstart holds the start of a plane span, initialized to 0
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

// Texture mapping
lighttable_t **planezlight;
fixed_t planeheight;
fixed_t *yslope;
fixed_t yslopes[LOOKDIRS][SCREENHEIGHT];
fixed_t distscale[WIDESCREENWIDTH];
fixed_t basexscale, baseyscale;
fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];


//==========================================================================
//
// R_InitSky
//
// Called at level load.
//
//==========================================================================

void R_InitSky(int map)
{
    Sky1Texture = P_GetMapSky1Texture(map);
    Sky2Texture = P_GetMapSky2Texture(map);
    Sky1ScrollDelta = P_GetMapSky1ScrollDelta(map);
    Sky2ScrollDelta = P_GetMapSky2ScrollDelta(map);
    Sky1ColumnOffset = 0;
    Sky2ColumnOffset = 0;
    DoubleSky = P_GetMapDoubleSky(map);
}

//==========================================================================
//
// R_InitSkyMap
//
// Called whenever the view size changes.
//
//==========================================================================

void R_InitSkyMap(void)
{
    skyflatnum = R_FlatNumForName("F_SKY");
    skytexturemid = 200 * FRACUNIT;
    skyiscale = FRACUNIT;
}

//==========================================================================
//
// R_InitPlanes
//
// Called at game startup.
//
//==========================================================================

void R_InitPlanes(void)
{
}

//==========================================================================
//
// R_MapPlane
//
// Globals used: planeheight, ds_source, basexscale, baseyscale,
// viewx, viewy.
//
//==========================================================================

void R_MapPlane(int y, int x1, int x2)
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

    spanfunc();                 // High or low detail
}

//==========================================================================
//
// R_ClearPlanes
//
// Called at the beginning of each frame.
//
//==========================================================================

void R_ClearPlanes(void)
{
    int i;
    const angle_t angle = (viewangle - ANG90) >> ANGLETOFINESHIFT; // left to right mapping

    // opening / clipping determination
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (int i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
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

//==========================================================================
//
// R_FindPlane
//
//==========================================================================

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

    memset(new_pl->top, USHRT_MAX, sizeof(new_pl->top));

    return new_pl;
}

//==========================================================================
//
// R_CheckPlane
//
//==========================================================================

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

    for (x=intrl ; x <= intrh && pl->top[x] == 0xffffffffu; x++); // [crispy] hires / 32-bit integer math
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

//==========================================================================
//
// R_MakeSpans
//
//==========================================================================

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

//==========================================================================
//
// R_DrawPlanes
//
//==========================================================================

#define SKYTEXTUREMIDSHIFTED 200

void R_DrawPlanes(void)
{
    visplane_t *pl;
    int i;
    int light;
    int x, stop;
    int angle;
    byte *tempSource;
    byte *source;
    byte *source2;
    byte *dest;
    int count;
    int offset;
    int skyTexture;
    int offset2;
    int skyTexture2;
    int scrollOffset;
    int frac;
    int fracstep = FRACUNIT >> hires;

    extern byte *ylookup[SCREENHEIGHT];
    extern int columnofs[WIDESCREENWIDTH];

    for (i = 0 ; i < MAXVISPLANES ; i++)
    for (pl = visplanes[i] ; pl ; pl = pl->next)
    if (pl->minx <= pl->maxx)
    {
        if (pl->picnum == skyflatnum)
        {                       // Sky flat
            if (DoubleSky)
            {                   // Render 2 layers, sky 1 in front
                offset = Sky1ColumnOffset >> 16;
                skyTexture = texturetranslation[Sky1Texture];
                offset2 = Sky2ColumnOffset >> 16;
                skyTexture2 = texturetranslation[Sky2Texture];
                // [JN] Tutti-Frutti: how it should be applied?
                // dc_texheight = textureheight[skytexture]>>FRACBITS;
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
                        dest = ylookup[dc_yl] + columnofs[flipwidth[x]];
                        frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
                        do
                        {
                            if (source[frac >> FRACBITS])
                            {
                                *dest = source[frac >> FRACBITS];
                                frac += fracstep;
                            }
                            else
                            {
                                *dest = source2[frac >> FRACBITS];
                                frac += fracstep;
                            }
                            dest += screenwidth;
                        }
                        while (count--);
                    }
                }
                continue;       // Next visplane
            }
            else
            {                   // Render single layer
                if (pl->special == 200)
                {               // Use sky 2
                    offset = Sky2ColumnOffset >> 16;
                    skyTexture = texturetranslation[Sky2Texture];
                }
                else
                {               // Use sky 1
                    offset = Sky1ColumnOffset >> 16;
                    skyTexture = texturetranslation[Sky1Texture];
                }
                // [JN] Tutti-Frutti: how it should be applied?
                // dc_texheight = textureheight[skytexture]>>FRACBITS;
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
                        dest = ylookup[dc_yl] + columnofs[flipwidth[x]];
                        frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
                        do
                        {
                            *dest = source[frac >> FRACBITS];
                            dest += screenwidth;
                            frac += fracstep;
                        }
                        while (count--);
                    }
                }
                continue;       // Next visplane
            }
        }
        // Regular flat
        tempSource = W_CacheLumpNum(firstflat +
                                    flattranslation[pl->picnum], PU_STATIC);
        scrollOffset = leveltime >> 1 & 63;
        switch (pl->special)
        {                       // Handle scrolling flats
            case 201:
            case 202:
            case 203:          // Scroll_North_xxx
                ds_source = tempSource + ((scrollOffset
                                           << (pl->special - 201) & 63) << 6);
                break;
            case 204:
            case 205:
            case 206:          // Scroll_East_xxx
                ds_source = tempSource + ((63 - scrollOffset)
                                          << (pl->special - 204) & 63);
                break;
            case 207:
            case 208:
            case 209:          // Scroll_South_xxx
                ds_source = tempSource + (((63 - scrollOffset)
                                           << (pl->special - 207) & 63) << 6);
                break;
            case 210:
            case 211:
            case 212:          // Scroll_West_xxx
                ds_source = tempSource + (scrollOffset
                                          << (pl->special - 210) & 63);
                break;
            case 213:
            case 214:
            case 215:          // Scroll_NorthWest_xxx
                ds_source = tempSource + (scrollOffset
                                          << (pl->special - 213) & 63) +
                    ((scrollOffset << (pl->special - 213) & 63) << 6);
                break;
            case 216:
            case 217:
            case 218:          // Scroll_NorthEast_xxx
                ds_source = tempSource + ((63 - scrollOffset)
                                          << (pl->special - 216) & 63) +
                    ((scrollOffset << (pl->special - 216) & 63) << 6);
                break;
            case 219:
            case 220:
            case 221:          // Scroll_SouthEast_xxx
                ds_source = tempSource + ((63 - scrollOffset)
                                          << (pl->special - 219) & 63) +
                    (((63 - scrollOffset) << (pl->special - 219) & 63) << 6);
                break;
            case 222:
            case 223:
            case 224:          // Scroll_SouthWest_xxx
                ds_source = tempSource + (scrollOffset
                                          << (pl->special - 222) & 63) +
                    (((63 - scrollOffset) << (pl->special - 222) & 63) << 6);
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
        planezlight = zlight[light];

        pl->top[pl->maxx+1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
        pl->top[pl->minx-1] = 0xffffffffu; // [crispy] hires / 32-bit integer math

        stop = pl->maxx + 1;
        for (x = pl->minx; x <= stop; x++)
        {
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1],
                        pl->top[x], pl->bottom[x]);
        }
        W_ReleaseLumpNum(firstflat + flattranslation[pl->picnum]);
    }
}

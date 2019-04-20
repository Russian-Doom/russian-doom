//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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



// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "i_system.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern fixed_t Sky1ScrollDelta;
extern fixed_t Sky2ScrollDelta;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int Sky1Texture;
int Sky2Texture;
fixed_t Sky1ColumnOffset;
fixed_t Sky2ColumnOffset;
int skyflatnum;
int skytexturemid;
fixed_t skyiscale;
boolean DoubleSky;
planefunction_t floorfunc, ceilingfunc;

// [JN] For brightmaps initialization.
// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05;
int bmaptexture06, bmaptexture07, bmaptexture08, bmaptexture09, bmaptexture10;
int bmaptexture11, bmaptexture12, bmaptexture13, bmaptexture14, bmaptexture15;
int bmaptexture16, bmaptexture17, bmaptexture18, bmaptexture19, bmaptexture20;
int bmaptexture21, bmaptexture22, bmaptexture23;

int bmap_terminator;

// Opening
visplane_t *visplanes = NULL;
visplane_t *lastvisplane;
visplane_t *floorplane, *ceilingplane;
static int numvisplanes;
short openings[MAXOPENINGS], *lastopening;

// Clip values are the solid pixel bounding the range.
// floorclip start out SCREENHEIGHT
// ceilingclip starts out -1
short floorclip[SCREENWIDTH];
short ceilingclip[SCREENWIDTH];

// spanstart holds the start of a plane span, initialized to 0
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

// Texture mapping
lighttable_t **planezlight;
fixed_t planeheight;
fixed_t* yslope;
fixed_t yslopes[LOOKDIRS][SCREENHEIGHT];
fixed_t distscale[SCREENWIDTH];
fixed_t basexscale, baseyscale;
fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

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

// ===========================================================
// =                                                         =
// = [JN] Lookup and init all the textures for brightmapping =
// =                                                         =
// ===========================================================
void R_InitBrightmaps(void)
{
    // Print informative message while startup
    printf(english_language ?
           "\nR_Init: Brightmapping initialization." :
           "\nR_Init: Инициализация брайтмаппинга.");

    // -------------------------------------------------------
    //  Textures
    // -------------------------------------------------------

    // brightmap_greenonly
    bmaptexture01 = R_TextureNumForName("SW_1_MD");
    
    // brightmap_redonly
    bmaptexture02 = R_TextureNumForName("SW_2_DN");
    
    // brightmap_blueonly
    bmaptexture03 = R_TextureNumForName("SW_1_DN");
    bmaptexture04 = R_TextureNumForName("SW_2_MD");
    
    // brightmap_flame
    bmaptexture05 = R_TextureNumForName("SPAWN03");
    bmaptexture06 = R_TextureNumForName("SPAWN12");
    bmaptexture08 = R_TextureNumForName("SW52_ON");
    bmaptexture11 = R_TextureNumForName("X_FAC01"); // Korax
    bmaptexture12 = R_TextureNumForName("X_FAC02"); // Korax
    bmaptexture13 = R_TextureNumForName("X_FAC03"); // Korax
    bmaptexture14 = R_TextureNumForName("X_FAC04"); // Korax
    bmaptexture15 = R_TextureNumForName("X_FAC05"); // Korax
    bmaptexture16 = R_TextureNumForName("X_FAC06"); // Korax
    bmaptexture17 = R_TextureNumForName("X_FAC07"); // Korax
    bmaptexture18 = R_TextureNumForName("X_FAC08"); // Korax
    bmaptexture19 = R_TextureNumForName("X_FAC09"); // Korax
    bmaptexture20 = R_TextureNumForName("X_FAC10"); // Korax
    bmaptexture21 = R_TextureNumForName("X_FAC11"); // Korax
    bmaptexture22 = R_TextureNumForName("X_FAC12"); // Korax
    bmaptexture23 = R_TextureNumForName("TPORTX");  // Gate's frame

    if (gamemode != shareware)
    bmaptexture07 = R_TextureNumForName("SW51_ON");
    
    // brightmap_yellowred
    bmaptexture09 = R_TextureNumForName("SPAWN09");
    bmaptexture10 = R_TextureNumForName("SPAWN10");
    
    // We need to declare a "terminator" - standard game texture,
    // presented in all Hexen series and using standard light formula.
    // Otherwise, non-defined textures will use latest brightmap.
    bmap_terminator = R_TextureNumForName(("FOREST01"));    
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
    // [crispy] see below
    // angle_t angle;
    fixed_t distance;
    // length;
    unsigned index;
    int dx, dy;

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

    // [crispy] avoid division by zero if (y == centery)
    if (y == centery)
    {
        return;
    }

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
    angle_t angle;

    // Opening / clipping determination
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

    // Texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;    // left to right mapping
    // Scale will be unit scale at SCREENWIDTH/2 distance
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
            printf(english_language ?
                   "R_FindPlane: Hit MAXVISPLANES (%d) Vanilla limit.\n" :
                   "R_FindPlane: достигнут лимиn MAXVISPLANES (%d).\n",
                   MAXVISPLANES);
    
        numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
        visplanes = realloc(visplanes, numvisplanes * sizeof(*visplanes));
        lastvisplane = visplanes + numvisplanes_old;
        floorplane = visplanes + (floorplane - visplanes_old);
        ceilingplane = visplanes + (ceilingplane - visplanes_old);
    
        if (vp)
        *vp = visplanes + (*vp - visplanes_old);
    }
}

//==========================================================================
//
// R_FindPlane
//
//==========================================================================

visplane_t *R_FindPlane(fixed_t height, int picnum,
                        int lightlevel, int special)
{
    visplane_t *check;

    if (special < 150)
    {                           // Don't let low specials affect search
        special = 0;
    }

    if (picnum == skyflatnum)
    {                           // All skies map together
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

//==========================================================================
//
// R_CheckPlane
//
//==========================================================================

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
    {
        if (pl->top[x] != 0xffff)
        {
            break;
        }
    }

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;
        return pl;              // use the same visplane
    }

    // Make a new visplane
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

//==========================================================================
//
// R_MakeSpans
//
//==========================================================================

void R_MakeSpans(int x, int t1, int b1, int t2, int b2)
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

    extern byte *ylookup[MAXHEIGHT];
    extern int columnofs[MAXWIDTH];

#ifdef RANGECHECK
    if (ds_p - drawsegs > numdrawsegs)
    {
        I_Error(english_language ?
                "R_DrawPlanes: drawsegs overflow (%i)" :
                "R_DrawPlanes: превышен лимит drawsegs (%i)",
                ds_p - drawsegs);
    }
    if (lastvisplane - visplanes > numvisplanes)
    {
        I_Error(english_language ?
                "R_DrawPlanes: visplane overflow (%i)" :
                "R_DrawPlanes: превышен лимит visplane (%i)",
                lastvisplane - visplanes);
    }
    if (lastopening - openings > MAXOPENINGS)
    {
        I_Error(english_language ?
                "R_DrawPlanes: opening overflow (%i)" :
                "R_DrawPlanes: превышен лимит opening (%i)",
                lastopening - openings);
    }
#endif

    for (pl = visplanes; pl < lastvisplane; pl++)
    {
        if (pl->minx > pl->maxx)
        {
            continue;
        }
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
                    if (dc_yl <= dc_yh)
                    {
                        count = dc_yh - dc_yl;
                        if (count < 0)
                        {
                            return;
                        }
                        angle = (viewangle + xtoviewangle[x])
                            >> ANGLETOSKYSHIFT;
                        source = R_GetColumn(skyTexture, angle + offset);
                        source2 = R_GetColumn(skyTexture2, angle + offset2);
                        dest = ylookup[dc_yl] + columnofs[x];
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
                            dest += SCREENWIDTH;
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
                    if (dc_yl <= dc_yh)
                    {
                        count = dc_yh - dc_yl;
                        if (count < 0)
                        {
                            return;
                        }
                        angle = (viewangle + xtoviewangle[x])
                            >> ANGLETOSKYSHIFT;
                        source = R_GetColumn(skyTexture, angle + offset);
                        dest = ylookup[dc_yl] + columnofs[x];
                        frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
                        do
                        {
                            *dest = source[frac >> FRACBITS];
                            dest += SCREENWIDTH;
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
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
        if (light >= LIGHTLEVELS)
        {
            light = LIGHTLEVELS - 1;
        }
        if (light < 0)
        {
            light = 0;
        }
        planezlight = zlight[light];

        pl->top[pl->maxx + 1] = 0xffff;
        pl->top[pl->minx - 1] = 0xffff;

        stop = pl->maxx + 1;
        for (x = pl->minx; x <= stop; x++)
        {
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1],
                        pl->top[x], pl->bottom[x]);
        }
        W_ReleaseLumpNum(firstflat + flattranslation[pl->picnum]);
    }
}

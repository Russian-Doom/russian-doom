//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//

// Russian Doom (C) 2016-2017 Julian Nechaevsky


#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "r_sky.h"

extern int brightmaps;
extern int invul_sky;   // [JN] Неуязвимость окрашивает небо
extern int mlook;
extern boolean scaled_sky;

// [JN] For brightmaps initialization
// Floors and ceilings:
int bmapflatnum1, bmapflatnum2, bmapflatnum3;
// Walls:
int bmaptexture01, bmaptexture02, bmaptexture03, bmaptexture04, bmaptexture05;
int bmaptexture06, bmaptexture07, bmaptexture08, bmaptexture09, bmaptexture10;
int bmaptexture11, bmaptexture12, bmaptexture13, bmaptexture14, bmaptexture15;
int bmaptexture16, bmaptexture17, bmaptexture18, bmaptexture19, bmaptexture20;
int bmaptexture21, bmaptexture22, bmaptexture23, bmaptexture24, bmaptexture25;
int bmaptexture26, bmaptexture27, bmaptexture28, bmaptexture29, bmaptexture30;
int bmaptexture31, bmaptexture32, bmaptexture33, bmaptexture34, bmaptexture35;
int bmaptexture36, bmaptexture37, bmaptexture38, bmaptexture39, bmaptexture40;
int bmaptexture41, bmaptexture42, bmaptexture43, bmaptexture44, bmaptexture45;
int bmaptexture46, bmaptexture47, bmaptexture48, bmaptexture49, bmaptexture50;
int bmaptexture51, bmaptexture52, bmaptexture53, bmaptexture54, bmaptexture55;
int bmaptexture56, bmaptexture57, bmaptexture58, bmaptexture59, bmaptexture60;
int bmaptexture61, bmaptexture62, bmaptexture63, bmaptexture64, bmaptexture65;
int bmaptexture66, bmaptexture67, bmaptexture68, bmaptexture69, bmaptexture70;
int bmaptexture71, bmaptexture72, bmaptexture73, bmaptexture74, bmaptexture75;
int bmaptexture76, bmaptexture77, bmaptexture78, bmaptexture79, bmaptexture80;
int bmaptexture81, bmaptexture82, bmaptexture83, bmaptexture84;


int bmap_terminator;

planefunction_t floorfunc;
planefunction_t ceilingfunc;


//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES	128
visplane_t*     visplanes = NULL;
visplane_t*     lastvisplane;
visplane_t*     floorplane;
visplane_t*     ceilingplane;
static int	    numvisplanes;

// ?
#define MAXOPENINGS SCREENWIDTH*64*4 
int     openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int*    lastopening;           // [crispy] 32-bit integer math


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int floorclip[SCREENWIDTH];   // [crispy] 32-bit integer math
int ceilingclip[SCREENWIDTH]; // [crispy] 32-bit integer math

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

fixed_t yslope[SCREENHEIGHT];
fixed_t distscale[SCREENWIDTH];
fixed_t basexscale;
fixed_t baseyscale;

fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];

int detailLevel; // [JN] & [crispy] Необходимо для R_MapPlane


//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
    // Doh!
}


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
        I_Error ("R_MapPlane: %i, %i у %i",x1,x2,y);
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

        if (index >= MAXLIGHTZ )
            index = MAXLIGHTZ-1;

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
    angle_t angle;

    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
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


//
// R_FindPlane
//
visplane_t*
R_FindPlane (fixed_t height, int picnum, int lightlevel)
{
    visplane_t* check;

    if (picnum == skyflatnum)
    {
        height = 0; // all skys map together
        lightlevel = 0;
    }

    for (check=visplanes; check<lastvisplane; check++)
    {
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel)
        {
            break;
        }
    }

    if (check < lastvisplane)
    return check;

    R_RaiseVisplanes(&check);

    lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;

    memset (check->top,0xff,sizeof(check->top));

    return check;
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

    for (x=intrl ; x<= intrh ; x++)
    if (pl->top[x] != 0xffffffffu) // [crispy] hires / 32-bit integer math
        break;

    // [crispy] fix HOM if ceilingplane and floorplane are the same
    // visplane (e.g. both are skies)
    if (!(pl == floorplane && markceiling && floorplane == ceilingplane))
    {
        if (x > intrh)
        {
            pl->minx = unionl;
            pl->maxx = unionh;

            // use the same one
            return pl;		
        }
    }

    // make a new visplane
    R_RaiseVisplanes(&pl);
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;

    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    memset (pl->top,0xff,sizeof(pl->top));

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
                x1 = x + 128 + ((finesine[sinvalue] * amp) >> FRACBITS) + ((finesine[sinvalue2] * amp2) >> FRACBITS);

                sinvalue = (x * swirlfactor + leveltime * speed * 3 + 700) & 8191;
                sinvalue2 = (y * swirlfactor2 + leveltime * speed * 4 + 1200) & 8191;
                y1 = y + 128 + ((finesine[sinvalue] * amp) >> FRACBITS) + ((finesine[sinvalue2] * amp2) >> FRACBITS);

                x1 &= 63;
                y1 &= 63;

                offset[(y << 6) + x] = (y1 << 6) + x1;
            }
        }

    swirltic = gametic;
    }

    // [JN] Использовать конкретную поверхность
    normalflat = W_CacheLumpNum(firstflat + flatnum, PU_LEVEL);

    for (i = 0; i < 4096; i++)
    {
        distortedflat[i] = normalflat[offset[i]];
    }

    Z_ChangeTag(normalflat, PU_CACHE);

    return distortedflat;
}


//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes (void) 
{
    visplane_t* pl;
    int         light;
    int         x;
    int         stop;
    int         angle;
    int         lumpnum;

#ifdef RANGECHECK
    if (ds_p - drawsegs > numdrawsegs)
    I_Error ("R_DrawPlanes: переполнение 'drawsegs' (%i)", ds_p - drawsegs);

    if (lastvisplane - visplanes > numvisplanes)
    I_Error ("R_DrawPlanes: переполнение 'visplane' (%i)", lastvisplane - visplanes);

    if (lastopening - openings > MAXOPENINGS)
    I_Error ("R_DrawPlanes: переполнение 'opening' (%i)", lastopening - openings);
#endif

    for (pl = visplanes ; pl < lastvisplane ; pl++)
    {
        if (pl->minx > pl->maxx)
        continue;

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
                    angle = (viewangle + xtoviewangle[x])>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle, false);
                    colfunc ();
                }
            }
        continue;
        }

        // regular flat
        lumpnum = firstflat + flattranslation[pl->picnum];
        // [crispy] add support for SMMU swirling flats
        ds_source = (flattranslation[pl->picnum] == -1) ?
                    R_DistortedFlat(pl->picnum) :
                    W_CacheLumpNum(lumpnum, PU_STATIC);

        planeheight = abs(pl->height-viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;

        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS-1;

        if (light < 0)
            light = 0;

        planezlight = zlight[light];

        // [JN] Apply brightmaps to floor/ceiling...
        if (brightmaps && !vanillaparm &&
        (pl->picnum == bmapflatnum1     // CONS1_1
        || pl->picnum == bmapflatnum2   // CONS1_5
        || pl->picnum == bmapflatnum3)) // CONS1_7
        planezlight = fullbright_notgrayorbrown_floor[light];

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

// ===========================================================
// =                                                         =
// = [JN] Lookup and init all the textures for brightmapping =
// =                                                         =
// ===========================================================
void R_InitBrightmaps(void)
{
    // Print informative message while startup
    printf("\nR_Init: Инициализация брайтмаппинга.");

    // Texture lookup. There are many strict definitions,
    // for example, no need to lookup Doom 1 textures in TNT.
    
    // -------------------------------------------------------
    //  Flats and ceilings
    // -------------------------------------------------------

    // All games
    bmapflatnum1 = R_FlatNumForName("CONS1_1");
    bmapflatnum2 = R_FlatNumForName("CONS1_5");
    bmapflatnum3 = R_FlatNumForName("CONS1_7");

    // -------------------------------------------------------
    //  Red only
    // -------------------------------------------------------
    
    // Not in Shareware
    if (gamemode != shareware)
    {
        bmaptexture08 = R_TextureNumForName(("SW2WOOD"));
        bmaptexture09 = R_TextureNumForName(("WOOD4"));
        bmaptexture11 = R_TextureNumForName(("SLADSKUL"));
        bmaptexture16 = R_TextureNumForName(("SW2BLUE"));
        bmaptexture17 = R_TextureNumForName(("SW2GSTON"));
        bmaptexture23 = R_TextureNumForName(("WOODGARG"));
        bmaptexture34 = R_TextureNumForName(("EXITSTON"));
    }

    // In both games - Doom 1: red only, Doom 2: green only
    bmaptexture24 = R_TextureNumForName(("SW2STON2"));

    // Doom 1 only, not in Shareware
    if (gamemode == registered || gamemode == retail)
    {
        bmaptexture10 = R_TextureNumForName(("WOODSKUL"));
    }

    // Not in Doom 1
    if (gamemode == commercial)
    {
        bmaptexture01 = R_TextureNumForName(("SW1STARG"));
        bmaptexture02 = R_TextureNumForName(("SW2MARB"));
        bmaptexture06 = R_TextureNumForName(("SW2PANEL"));
        bmaptexture12 = R_TextureNumForName(("SW1BRIK"));
        bmaptexture14 = R_TextureNumForName(("SW1MET2"));
        bmaptexture18 = R_TextureNumForName(("SW2ROCK"));
        bmaptexture19 = R_TextureNumForName(("SW2STON6"));
        bmaptexture20 = R_TextureNumForName(("SW2ZIM"));
        bmaptexture25 = R_TextureNumForName(("SW1BRN1"));
        bmaptexture26 = R_TextureNumForName(("SW1STON2"));
    }

    // TNT Evilution only
    if (gamemission == pack_tnt)
    {
        bmaptexture27 = R_TextureNumForName(("LITERED2"));
        bmaptexture28 = R_TextureNumForName(("PNK4EXIT"));
    }

    // All games
    {
        bmaptexture03 = R_TextureNumForName(("SW1BRCOM"));
        bmaptexture04 = R_TextureNumForName(("SW1DIRT"));
        bmaptexture05 = R_TextureNumForName(("SW1STRTN"));
        bmaptexture07 = R_TextureNumForName(("SW2SLAD"));
        bmaptexture13 = R_TextureNumForName(("SW1COMM"));
        bmaptexture15 = R_TextureNumForName(("SW1STON1"));
        bmaptexture21 = R_TextureNumForName(("SW2COMP"));
        bmaptexture22 = R_TextureNumForName(("SW1STONE"));
        bmaptexture39 = R_TextureNumForName(("EXITSIGN"));
    }

    // -------------------------------------------------------
    //  Not gray
    // -------------------------------------------------------
    
    // Doom 1 only
    if (gamemode == shareware || gamemode == registered || gamemode == retail)
    {
        bmaptexture30 = R_TextureNumForName(("PLANET1"));
        bmaptexture38 = R_TextureNumForName(("LITEBLU2"));
    }

    // All games
    {
        bmaptexture29 = R_TextureNumForName(("COMPSTA2"));
        bmaptexture31 = R_TextureNumForName(("SW2EXIT"));
        bmaptexture32 = R_TextureNumForName(("SW2GRAY1"));
        bmaptexture33 = R_TextureNumForName(("COMPSTA1"));
        bmaptexture36 = R_TextureNumForName(("LITEBLU1"));
        bmaptexture37 = R_TextureNumForName(("SW2GRAY"));
    }

    // -------------------------------------------------------
    //  Not gray or brown
    // -------------------------------------------------------

    // Doom 1 only
    if (gamemode == shareware || gamemode == registered || gamemode == retail)
    {
        bmaptexture40 = R_TextureNumForName(("COMP2"));
        bmaptexture41 = R_TextureNumForName(("COMPUTE2"));
        bmaptexture43 = R_TextureNumForName(("COMPUTE1"));
        bmaptexture44 = R_TextureNumForName(("COMPUTE3"));
    }

    // Not in Doom 1
    if (gamemode == commercial)
    {
        bmaptexture35 = R_TextureNumForName(("SILVER2"));
        bmaptexture42 = R_TextureNumForName(("SILVER3"));
    }

    // TNT Evilution only
    if (gamemission == pack_tnt)
    {
        bmaptexture46 = R_TextureNumForName(("BTNTMETL"));
        bmaptexture47 = R_TextureNumForName(("BTNTSLVR"));
        bmaptexture48 = R_TextureNumForName(("SLAD2"));
        bmaptexture49 = R_TextureNumForName(("SLAD3"));
        bmaptexture50 = R_TextureNumForName(("SLAD4"));
        bmaptexture51 = R_TextureNumForName(("SLAD5"));
        bmaptexture52 = R_TextureNumForName(("SLAD6"));
        bmaptexture53 = R_TextureNumForName(("SLAD7"));
        bmaptexture54 = R_TextureNumForName(("SLAD8"));
        bmaptexture55 = R_TextureNumForName(("SLAD9"));
        bmaptexture56 = R_TextureNumForName(("SLAD10"));
        bmaptexture57 = R_TextureNumForName(("SLAD11"));
        bmaptexture59 = R_TextureNumForName(("SLADRIP1"));
        bmaptexture60 = R_TextureNumForName(("SLADRIP3"));
    }

    // -------------------------------------------------------
    //  Green only 1
    // -------------------------------------------------------
    
    // Not in Shareware
    if (gamemode != shareware)
    {
        bmaptexture73 = R_TextureNumForName(("SW2VINE"));
    }

    // Not in Doom 1
    if (gamemode == commercial)
    {
        bmaptexture45 = R_TextureNumForName(("SW2MOD1"));
        bmaptexture58 = R_TextureNumForName(("SPCDOOR3"));
        bmaptexture66 = R_TextureNumForName(("SW2TEK"));
        bmaptexture67 = R_TextureNumForName(("SW2BRIK"));
        bmaptexture71 = R_TextureNumForName(("SW2MET2"));
        bmaptexture74 = R_TextureNumForName(("PIPEWAL1"));
        bmaptexture75 = R_TextureNumForName(("TEKLITE2"));
    }

    // All games
    {
        bmaptexture68 = R_TextureNumForName(("SW2BRN2"));
        bmaptexture69 = R_TextureNumForName(("SW2COMM"));
        bmaptexture72 = R_TextureNumForName(("SW2STRTN"));
    }

    // -------------------------------------------------------
    //  Green only 2
    // -------------------------------------------------------

    // Doom 2 only
    if (gamemission == doom2)
    {
        bmaptexture78 = R_TextureNumForName(("SW2SKULL"));
    }    

    // Not in Doom 1
    if (gamemode == commercial)
    {
        bmaptexture61 = R_TextureNumForName(("SW2STARG"));
        bmaptexture62 = R_TextureNumForName(("SW2BRN1"));
    }

    // TNT Evilution only
    if (gamemission == pack_tnt)
    {
        bmaptexture79 = R_TextureNumForName(("M_TEC"));
    }

    // All games
    {
        bmaptexture63 = R_TextureNumForName(("SW2BRCOM"));
        bmaptexture64 = R_TextureNumForName(("SW2STON1"));
        bmaptexture65 = R_TextureNumForName(("SW2STONE"));
        bmaptexture70 = R_TextureNumForName(("SW2DIRT"));
    }

    // -------------------------------------------------------
    //  Green only 3
    // -------------------------------------------------------

    // All games
    {
        bmaptexture77 = R_TextureNumForName(("SW2BRNGN"));
        bmaptexture80 = R_TextureNumForName(("SW2METAL"));
    }

    // -------------------------------------------------------
    //  Orange and yellow
    // -------------------------------------------------------
    
    // Not in Doom 1
    if (gamemode == commercial)
    {
        bmaptexture81 = R_TextureNumForName(("TEKBRON2"));
    }

    // TNT Evilution only
    if (gamemission == pack_tnt)
    {
        bmaptexture82 = R_TextureNumForName(("LITEYEL2"));
        bmaptexture83 = R_TextureNumForName(("LITEYEL3"));
        bmaptexture84 = R_TextureNumForName(("YELMETAL"));
    }

    // -------------------------------------------------------
    //  Brightmap terminator
    // -------------------------------------------------------
    
    // We need to declare a "terminator" - standard game texture,
    // presented in all Doom series and using standard light formula.
    // Otherwise, non-defined textures will use latest brightmap.
    {
        bmap_terminator = R_TextureNumForName(("BIGDOOR2"));
    }
}

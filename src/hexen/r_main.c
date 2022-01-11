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


#include <stdlib.h>
#include <math.h>
#include "m_random.h"
#include "h2def.h"
#include "m_bbox.h"
#include "r_local.h"
#include "p_local.h"
#include "i_timer.h"
#include "jn.h"

int viewangleoffset;

// haleyjd: removed WATCOMC

int validcount = 1;             // increment every time a check is made

lighttable_t *fixedcolormap;
extern lighttable_t **walllights;

int centerx, centery;
fixed_t centerxfrac, centeryfrac;
fixed_t projection;

int sscount, linecount, loopcount;

fixed_t viewx, viewy, viewz;
angle_t viewangle;
fixed_t viewcos, viewsin;
player_t *viewplayer;

int detailshift;                // 0 = high, 1 = low

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup maps the visible view
// angles  to screen X coordinates, flattening the arc to a flat projection
// plane.  There will be many angles mapped to the same X.
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel to the lowest viewangle
// that maps back to x ranges from clipangle to -clipangle
// [JN] e6y: resolution limitation is removed
angle_t *xtoviewangle;  // killough 2/8/98

// [crispy] calculate the linear sky angle component here
angle_t *linearskyangle;

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// [JN] Brightmaps
lighttable_t *fullbright_greenonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_redonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_blueonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_purpleonly[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_flame[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_yellowred[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *fullbright_firebull[LIGHTLEVELS][MAXLIGHTSCALE];


int extralight;                 // bumped light from gun blasts

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*tlcolfunc) (void);
void (*transcolfunc) (void);
void (*transtlcolfunc) (void);
void (*spanfunc) (void);

/*
================================================================================
=
= R_PointOnSide
=
= Traverse BSP (sub) tree, check point against partition plane.
= Returns side 0 (front) or 1 (back).
=
= [JN] killough 5/2/98: reformatted
=
================================================================================
*/

int R_PointOnSide(fixed_t x, fixed_t y, node_t * node)
{
    if (!node->dx)
    return x <= node->x ? node->dy > 0 : node->dy < 0;

    if (!node->dy)
    return y <= node->y ? node->dx < 0 : node->dx > 0;

    x -= node->x;
    y -= node->y;

    // Try to quickly decide by looking at sign bits.
    if ((node->dy ^ node->dx ^ x ^ y) < 0)
    return (node->dy ^ x) < 0;  // (left is negative)

    return FixedMul(y, node->dx>>FRACBITS) >= FixedMul(node->dy>>FRACBITS, x);		
}

/*
================================================================================
=
= R_PointOnSegSide
=
= [JN] killough 5/2/98: reformatted
=
================================================================================
*/

int R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line)
{
    fixed_t lx = line->v1->x;
    fixed_t ly = line->v1->y;
    fixed_t ldx = line->v2->x - lx;
    fixed_t ldy = line->v2->y - ly;

    if (!ldx)
    return x <= lx ? ldy > 0 : ldy < 0;

    if (!ldy)
    return y <= ly ? ldx < 0 : ldx > 0;

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
    return (ldy ^ x) < 0;   // (left is negative)

    return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);	
}

/*
================================================================================
=
= R_PointToAngleSlope
=
= To get a global angle from cartesian coordinates, the coordinates are flipped
= until they are in the first octant of the coordinate system, then the y (<=x)
= is scaled and divided by x to get a tangent (slope) value which is looked up 
= in the tantoangle[] table.

// [crispy] turned into a general R_PointToAngle() flavor
// called with either slope_div = SlopeDivCrispy() from R_PointToAngleCrispy()
// or slope_div = SlopeDiv() else
================================================================================
*/

angle_t R_PointToAngleSlope (fixed_t x, fixed_t y, int (*slope_div) (unsigned int num, unsigned int den))
{	
    x -= viewx;
    y -= viewy;
    
    if ( (!x) && (!y) )
	return 0;

    if (x>= 0)
    {
	// x >=0
	if (y>= 0)
	{
	    // y>= 0

	    if (x>y)
	    {
		// octant 0
		return tantoangle[slope_div(y,x)];
	    }
	    else
	    {
		// octant 1
		return ANG90-1-tantoangle[slope_div(x,y)];
	    }
	}
	else
	{
	    // y<0
	    y = -y;

	    if (x>y)
	    {
		// octant 8
		return -tantoangle[slope_div(y,x)];
	    }
	    else
	    {
		// octant 7
		return ANG270+tantoangle[slope_div(x,y)];
	    }
	}
    }
    else
    {
	// x<0
	x = -x;

	if (y>= 0)
	{
	    // y>= 0
	    if (x>y)
	    {
		// octant 3
		return ANG180-1-tantoangle[slope_div(y,x)];
	    }
	    else
	    {
		// octant 2
		return ANG90+ tantoangle[slope_div(x,y)];
	    }
	}
	else
	{
	    // y<0
	    y = -y;

	    if (x>y)
	    {
		// octant 4
		return ANG180+tantoangle[slope_div(y,x)];
	    }
	    else
	    {
		 // octant 5
		return ANG270-1-tantoangle[slope_div(x,y)];
	    }
	}
    }
    return 0;
}

/*
================================================================================
=
= R_PointToAngle
=
================================================================================
*/

angle_t R_PointToAngle (fixed_t x, fixed_t y)
{
    return R_PointToAngleSlope (x, y, SlopeDiv);
}

/*
================================================================================
=
= R_PointToAngle
=
= [crispy] overflow-safe R_PointToAngle() flavor
= called only from R_CheckBBox(), R_AddLine() and P_SegLengths()
=
================================================================================
*/

angle_t R_PointToAngleCrispy (fixed_t x, fixed_t y)
{
    // [crispy] fix overflows for very long distances
    int64_t y_viewy = (int64_t)y - viewy;
    int64_t x_viewx = (int64_t)x - viewx;

    // [crispy] the worst that could happen is e.g. INT_MIN-INT_MAX = 2*INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX ||
        y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
	// [crispy] preserving the angle by halfing the distance in both directions
	x = x_viewx / 2 + viewx;
	y = y_viewy / 2 + viewy;
    }

    return R_PointToAngleSlope (x, y, SlopeDivCrispy);
}

/*
================================================================================
=
= R_PointToAngle2
=
================================================================================
*/

angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    viewx = x1;
    viewy = y1;

    // [crispy] R_PointToAngle2() is never called during rendering
    return R_PointToAngleSlope (x2, y2, SlopeDiv);
}

fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
    int angle;
    fixed_t dx, dy, temp;
    fixed_t dist, frac;

    dx = abs(x - viewx);
    dy = abs(y - viewy);

    if (dy > dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    // [JN] Fix crashes in udm1.wad
    if (dx != 0)
    {
        frac = FixedDiv(dy, dx);
    }
    else
    {
        frac = 0;
    }

    angle = (tantoangle[frac>>DBITS]+ANG90) >> ANGLETOFINESHIFT;

    dist = FixedDiv(dx, finesine[angle]);       // use as cosine

    return dist;
}



/*
=================
=
= R_InitPointToAngle
=
=================
*/

void R_InitPointToAngle(void)
{
// now getting from tables.c
#if 0
    int i;
    long t;
    float f;
//
// slope (tangent) to angle lookup
//
    for (i = 0; i <= SLOPERANGE; i++)
    {
        f = atan((float) i / SLOPERANGE) / (3.141592657 * 2);
        t = 0xffffffff * f;
        tantoangle[i] = t;
    }
#endif
}

//=============================================================================

/*
================
=
= R_ScaleFromGlobalAngle
=
= Returns the texture mapping scale for the current line at the given angle
= rw_distance must be calculated first
================

// [crispy] WiggleFix: move R_ScaleFromGlobalAngle function to r_segs.c,
// above R_StoreWallRange

fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
    fixed_t scale;
    int anglea, angleb;
    int sinea, sineb;
    fixed_t num, den;

#if 0
    {
        fixed_t dist, z;
        fixed_t sinv, cosv;

        sinv = finesine[(visangle - rw_normalangle) >> ANGLETOFINESHIFT];
        dist = FixedDiv(rw_distance, sinv);
        cosv = finecosine[(viewangle - visangle) >> ANGLETOFINESHIFT];
        z = abs(FixedMul(dist, cosv));
        scale = FixedDiv(projection, z);
        return scale;
    }
#endif

    anglea = ANG90 + (visangle - viewangle);
    angleb = ANG90 + (visangle - rw_normalangle);
// bothe sines are allways positive
    sinea = finesine[anglea >> ANGLETOFINESHIFT];
    sineb = finesine[angleb >> ANGLETOFINESHIFT];
    num = FixedMul(projection, sineb) << detailshift;
    den = FixedMul(rw_distance, sinea);
    if (den > num >> 16)
    {
        scale = FixedDiv(num, den);
        if (scale > 64 * FRACUNIT)
            scale = 64 * FRACUNIT;
        else if (scale < 256)
            scale = 256;
    }
    else
        scale = 64 * FRACUNIT;

    return scale;
}
*/


// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale)
{
    if (nangle == oangle)
        return nangle;
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
    }
    else // nangle < oangle
    {
        if (oangle - nangle < ANG270)
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
    }
}


/*
=================
=
= R_InitTables
=
=================
*/

void R_InitTables(void)
{
// now getting from tables.c
#if 0
    int i;
    float a, fv;
    int t;

//
// viewangle tangent table
//
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        a = (i - FINEANGLES / 4 + 0.5) * PI * 2 / FINEANGLES;
        fv = FRACUNIT * tan(a);
        t = fv;
        finetangent[i] = t;
    }

//
// finesine table
//
    for (i = 0; i < 5 * FINEANGLES / 4; i++)
    {
// OPTIMIZE: mirror...
        a = (i + 0.5) * PI * 2 / FINEANGLES;
        t = FRACUNIT * sin(a);
        finesine[i] = t;
    }
#endif

}


/*
=================
=
= R_InitTextureMapping
=
=================
*/

void R_InitTextureMapping(void)
{
    int i;
    int x;
    int t;
    fixed_t focallength;


//
// use tangent table to generate viewangletox
// viewangletox will give the next greatest x after the view angle
//
    // calc focallength so FIELDOFVIEW angles covers SCREENWIDTH

    // [crispy] in widescreen mode, make sure the same number of horizontal
    // pixels shows the same part of the game scene as in regular rendering mode
    fixed_t focalwidth;
    focalwidth = (((320 << hires)>>detailshift)/2)<<FRACBITS;

    focallength =
        FixedDiv(aspect_ratio >= 2 ? focalwidth : centerxfrac, finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);


    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (finetangent[i] > FRACUNIT * 2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT * 2)
            t = viewwidth + 1;
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;
            if (t < -1)
                t = -1;
            else if (t > viewwidth + 1)
                t = viewwidth + 1;
        }
        viewangletox[i] = t;
    }

//
// scan viewangletox[] to generate xtoviewangleangle[]
//
// xtoviewangle will give the smallest view angle that maps to x
    for (x = 0; x <= viewwidth; x++)
    {
        i = 0;
        while (viewangletox[i] > x)
            i++;
        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
        // [crispy] calculate sky angle for drawing horizontally linear skies.
        // Taken from GZDoom and refactored for integer math.
        linearskyangle[x] = ((viewwidth / 2 - x) * ((screenwidth << 6) / viewwidth))
                                                 * (ANG90 / (SCREENWIDTH << 6));
    }

//
// take out the fencepost cases from viewangletox
//
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        t = FixedMul(finetangent[i], focallength);
        t = centerx - t;
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth + 1)
            viewangletox[i] = viewwidth;
    }

    clipangle = xtoviewangle[0];
}

//=============================================================================

/*
====================
=
= R_InitLightTables
=
= Only inits the zlight table, because the scalelight table changes
= with view size
=
====================
*/

#define		DISTMAP	2

void R_InitLightTables(void)
{
    int i, j, level, startmap;
    int scale;

//
// Calculate the light levels to use for each level / distance combination
//
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTZ; j++)
        {
            scale =
                FixedDiv((320 / 2 * FRACUNIT),
                         (j + 1) << LIGHTZSHIFT);
            scale >>= LIGHTSCALESHIFT;
            level = startmap - scale / DISTMAP;
            if (level < 0)
                level = 0;
            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;
            zlight[i][j] = colormaps + level * 256;
        }
    }
}


/*
==============
=
= R_SetViewSize
=
= Don't really change anything here, because i might be in the middle of
= a refresh.  The change will take effect next refresh.
=
==============
*/

boolean setsizeneeded;
int setblocks, setdetail;

// [crispy] lookup table for horizontal screen coordinates
int		flipwidth[WIDESCREENWIDTH];

void R_SetViewSize(int blocks, int detail)
{
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}

/*
==============
=
= R_ExecuteSetViewSize
=
==============
*/

void R_ExecuteSetViewSize(void)
{
    fixed_t cosadj, dy;
    int i, j, level, startmap;

    setsizeneeded = false;

    // [JN] Set screen variables depending on aspect ratio.

    scaledviewwidth = screenwidth;

    if (aspect_ratio == 0 || aspect_ratio == 1)
    {
        // [JN] 4:3 and 5:4
        if (setblocks >= 11)
        {
            scaledviewheight = SCREENHEIGHT;
        }
        else
        {
            scaledviewwidth = (setblocks * 32) << hires;
            scaledviewheight = (setblocks * 161 / 10) << hires;
        }
    }
    else
    {
        // [JN] 16:9, 16:10 and 21:9
        if (setblocks == 9 || setblocks == 10)
        {
            scaledviewheight = SCREENHEIGHT - SBARHEIGHT;
        }
        else
        {
            scaledviewheight = SCREENHEIGHT;
        }
    }

    detailshift = setdetail;
    viewwidth = scaledviewwidth >> detailshift;
    viewheight = scaledviewheight >> (detailshift && hires);

    centery = viewheight / 2;
    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    if (aspect_ratio >= 2)
    {
        projection = MIN(centerxfrac, (((320 << hires)>>detailshift)/2)<<FRACBITS);
    }
    else
    {
        projection = centerxfrac;
    }

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        tlcolfunc = R_DrawTLColumn;
        transcolfunc = R_DrawTranslatedColumn;
        transtlcolfunc = R_DrawTranslatedTLColumn;
        spanfunc = R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        tlcolfunc = R_DrawTLColumnLow;
        transcolfunc = R_DrawTranslatedColumnLow;
        transtlcolfunc = R_DrawTranslatedTLColumnLow;
        spanfunc = R_DrawSpanLow;
    }

    R_InitBuffer(scaledviewwidth, scaledviewheight);

    R_InitTextureMapping();

//
// psprite scales
//
    pspritescale = FRACUNIT * viewwidth / origwidth;
    pspriteiscale = FRACUNIT * origwidth / viewwidth;

//
// thing clipping
//
    for (i = 0; i < viewwidth; i++)
        screenheightarray[i] = viewheight;

//
// planes
//
    for (i = 0; i < viewheight; i++)
    {

        const fixed_t num_wide = MIN(viewwidth<<detailshift, 320 << !detailshift)/2*FRACUNIT;
        const fixed_t num = (viewwidth<<(detailshift && !hires))/2*FRACUNIT;

        for (j = 0; j < LOOKDIRS; j++)
        {
            if (aspect_ratio >= 2)
            {
                dy = ((i-(viewheight/2 + ((j-LOOKDIRMIN) << (hires && !detailshift)) * (screenblocks < 9 ? screenblocks : 9) / 10))<<FRACBITS)+FRACUNIT/2;
            }
            else
            {
                dy = ((i-(viewheight/2 + ((j-LOOKDIRMIN) << (hires && !detailshift)) * (screenblocks < 11 ? screenblocks : 11) / 10))<<FRACBITS)+FRACUNIT/2;
            }

        dy = abs(dy);
        yslopes[j][i] = FixedDiv (aspect_ratio >= 2 ? num_wide : num, dy);
        }
    }
    yslope = yslopes[LOOKDIRMIN];

    for (i = 0; i < viewwidth; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

//
// Calculate the light levels to use for each level / scale combination
//
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTSCALE; j++)
        {
            level =
                startmap -
                j * screenwidth / (viewwidth << detailshift) / DISTMAP;
            if (level < 0)
                level = 0;
            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;
            scalelight[i][j] = colormaps + level * 256;

            // [JN] Brightmaps
            fullbright_greenonly[i][j] = brightmaps_greenonly + level * 256;
            fullbright_redonly[i][j] = brightmaps_redonly + level * 256;
            fullbright_blueonly[i][j] = brightmaps_blueonly + level * 256;
            fullbright_purpleonly[i][j] = brightmaps_purpleonly + level * 256;
            fullbright_flame[i][j] = brightmaps_flame + level * 256;
            fullbright_yellowred[i][j] = brightmaps_yellowred + level * 256;
            fullbright_firebull[i][j] = brightmaps_firebull + level * 256;
        }
    }

    // [crispy] lookup table for horizontal screen coordinates
    for (i = 0, j = scaledviewwidth - 1; i < scaledviewwidth; i++, j--)
    {
        flipwidth[i] = flip_levels ? j : i;
    }

    // [JN] Skip weapon bobbing interpolation for next frame.
    skippsprinterp = true;

//
// draw the border
//
    R_DrawViewBorder();         // erase old menu stuff
}


/*
==============
=
= R_Init
=
==============
*/

int screenblocks = 10;

void R_Init(void)
{
    // [JN] Wide screen: don't allow unsupported view modes at startup
    if (aspect_ratio >= 2)
    {
        if (screenblocks < 9)
            screenblocks = 9;
        if (screenblocks > 12)
            screenblocks = 12;
    }

    R_InitSpritesRes ();
    R_InitPlanesRes ();
    R_InitVisplanesRes ();

    R_InitData();
    R_InitPointToAngle();
    R_InitTables();
    // viewwidth / viewheight / detailLevel are set by the defaults
    R_SetViewSize(screenblocks, detailLevel);
    R_InitLightTables();
    R_InitSkyMap();
    R_InitTranslationTables();
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    node_t *node;
    int side, nodenum;

    if (!numnodes)              // single subsector is a special case
        return subsectors;

    nodenum = numnodes - 1;

    while (!(nodenum & NF_SUBSECTOR))
    {
        node = &nodes[nodenum];
        side = R_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];

}

//----------------------------------------------------------------------------
//
// PROC R_SetupFrame
//
//----------------------------------------------------------------------------

void R_SetupFrame(player_t * player)
{
    int i;
    int tempCentery;
    int intensity;
    int pitch;

    //drawbsp = 1;
    viewplayer = player;
    // haleyjd: removed WATCOMC
    // haleyjd FIXME: viewangleoffset handling?

    // [AM] Interpolate the player camera if the feature is enabled.

    if (uncapped_fps && !vanillaparm &&
        // Don't interpolate on the first tic of a level,
        // otherwise oldviewz might be garbage.
        leveltime > 1 &&
        // Don't interpolate if the player did something
        // that would necessitate turning it off for a tic.
        player->mo->interp == true &&
        // Don't interpolate during a paused state
        !paused && (!menuactive || demoplayback || netgame))
    {
        // Interpolate player camera from their old position to their current one.
        // [JN] Also for earthquake effect.
        if (localQuakeHappening[displayplayer] && !paused)
        {
            intensity = localQuakeHappening[displayplayer];
        
            viewx = player->mo->oldx + FixedMul(player->mo->x - player->mo->oldx, fractionaltic);
            viewx += ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;
            viewy = player->mo->oldy + FixedMul(player->mo->y - player->mo->oldy, fractionaltic);
            viewy += ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;
        }
        else
        {
            viewx = player->mo->oldx + FixedMul(player->mo->x - player->mo->oldx, fractionaltic);
            viewy = player->mo->oldy + FixedMul(player->mo->y - player->mo->oldy, fractionaltic);
        }
        viewz = player->oldviewz + FixedMul(player->viewz - player->oldviewz, fractionaltic);
        viewangle = R_InterpolateAngle(player->mo->oldangle, player->mo->angle, fractionaltic) + viewangleoffset;

        pitch = (player->oldlookdir + (player->lookdir - player->oldlookdir) * FIXED2DOUBLE(fractionaltic)) / MLOOKUNIT;
    }
    else
    {
        viewx = player->mo->x;
        viewy = player->mo->y;
        viewz = player->viewz;
        viewangle = player->mo->angle + viewangleoffset;

        // [crispy] pitch is actual lookdir /*and weapon pitch*/
        pitch = player->lookdir / MLOOKUNIT;
    }

    extralight = player->extralight;

    if (pitch > LOOKDIRMAX)
    pitch = LOOKDIRMAX;
    else
    if (pitch < -LOOKDIRMIN)
    pitch = -LOOKDIRMIN;

    // apply new yslope[] whenever "lookdir", "detailshift" or "screenblocks" change
    if (aspect_ratio >= 2)
    {
        tempCentery = viewheight/2 + (pitch << (hires && !detailshift)) * (screenblocks < 9 ? screenblocks : 9) / 10;
    }
    else
    {
        tempCentery = viewheight/2 + (pitch << (hires && !detailshift)) * (screenblocks < 11 ? screenblocks : 11) / 10;
    }

    if (centery != tempCentery)
    {
        centery = tempCentery;
        centeryfrac = centery << FRACBITS;
        yslope = yslopes[LOOKDIRMIN + pitch];
    }
    viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];
    sscount = 0;
    if (player->fixedcolormap)
    {
        fixedcolormap = colormaps + player->fixedcolormap
            * 256 * sizeof(lighttable_t);
        walllights = scalelightfixed;
        for (i = 0; i < MAXLIGHTSCALE; i++)
        {
            scalelightfixed[i] = fixedcolormap;
        }
    }
    else
    {
        fixedcolormap = 0;
    }
    validcount++;
    if (BorderNeedRefresh)
    {
        if (setblocks < 10)
        {
            R_DrawViewBorder();
        }
        BorderNeedRefresh = false;
        BorderTopRefresh = false;
        UpdateState |= I_FULLSCRN;
    }
    if (BorderTopRefresh)
    {
        if (setblocks < 10)
        {
            R_DrawTopBorder();
        }
        BorderTopRefresh = false;
        UpdateState |= I_MESSAGES;
    }

#if 0
    {
        static int frame;
        memset(screen, frame, screenwidth * SCREENHEIGHT);
        frame++;
    }
#endif
}

/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView(player_t * player)
{
    extern void V_DrawFilledBox (int x, int y, int w, int h, int c);

    R_SetupFrame(player);
    R_ClearClipSegs();
    R_ClearDrawSegs();
    if (automapactive && !automap_overlay)
    {
        R_RenderBSPNode (numnodes-1);
        return;
    }

    // [JN] Fill level's "out of bounds" with black color
    V_DrawFilledBox(viewwindowx, viewwindowy, scaledviewwidth, viewheight, 0);

    R_ClearPlanes();
    R_ClearSprites();
    NetUpdate();                // check for new console commands

    // Make displayed player invisible locally
    if (localQuakeHappening[displayplayer] && gamestate == GS_LEVEL)
    {
        players[displayplayer].mo->flags2 |= MF2_DONTDRAW;
        R_RenderBSPNode(numnodes - 1);  // head node is the last node output
        players[displayplayer].mo->flags2 &= ~MF2_DONTDRAW;
    }
    else
    {
        R_RenderBSPNode(numnodes - 1);  // head node is the last node output
    }

    NetUpdate();                // check for new console commands
    R_DrawPlanes();
    NetUpdate();                // check for new console commands
    R_DrawMasked();
    NetUpdate();                // check for new console commands
}

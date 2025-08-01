//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2025 Leonid Murin (Dasperal)
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
#include "v_video.h"
#include "jn.h"


// [JN] Used by perfomance counter.
int rendered_segs, rendered_visplanes, rendered_vissprites;

int       viewangleoffset;
int       validcount = 1;             // increment every time a check is made
int       centerx, centery;
fixed_t   centerxfrac, centeryfrac;
fixed_t   projection;
fixed_t   viewx, viewy, viewz;
angle_t   viewangle;
fixed_t   viewcos, viewsin;
player_t *viewplayer;

int setblocks;
boolean setsizeneeded;

// [crispy] lookup table for horizontal screen coordinates
// [JN] Resolution limitation is removed.
int *flipscreenwidth;
int *flipviewwidth;

// [JN] LOOKDIR variables for high/quad resolution, used only for rendering.
static fixed_t lookdirmin, lookdirmax, lookdirs;

// Bumped light from gun blasts.
int extralight;

// [JN] Smooth and vanilla diminished lighting
int lightzshift, maxlightz;

// Precalculated math tables.
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel to the lowest viewangle
// that maps back to x ranges from clipangle to -clipangle
// [JN] e6y: resolution limitation is removed
angle_t *xtoviewangle;  // killough 2/8/98

// [crispy] calculate the linear sky angle component here
// [JN] resolution limitation is removed
angle_t *linearskyangle;

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

lighttable_t *fixedcolormap;
extern lighttable_t **walllights;

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*tlcolfunc) (void);
void (*alttlcolfunc) (void);
void (*extratlcolfunc) (void);
void (*transcolfunc) (void);
void (*transtlcolfunc) (void);
void (*spanfunc) (fixed_t x1, fixed_t x2, const fixed_t y,
                  fixed_t ds_xfrac, const fixed_t ds_xstep,
                  fixed_t ds_yfrac, const fixed_t ds_ystep);

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

int R_PointOnSide (fixed_t x, fixed_t y, const node_t *node)
{
    if (!node->dx)
    {
        return x <= node->x ? node->dy > 0 : node->dy < 0;
    }

    if (!node->dy)
    {
        return y <= node->y ? node->dx < 0 : node->dx > 0;
    }

    x -= node->x;
    y -= node->y;

    // Try to quickly decide by looking at sign bits.
    if ((node->dy ^ node->dx ^ x ^ y) < 0)
    {
        return (node->dy ^ x) < 0;  // (left is negative)
    }

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

int R_PointOnSegSide (fixed_t x, fixed_t y, const seg_t *line)
{
    fixed_t lx = line->v1->x;
    fixed_t ly = line->v1->y;
    fixed_t ldx = line->v2->x - lx;
    fixed_t ldy = line->v2->y - ly;

    if (!ldx)
    {
        return x <= lx ? ldy > 0 : ldy < 0;
    }

    if (!ldy)
    {
        return y <= ly ? ldx < 0 : ldx > 0;
    }

    x -= lx;
    y -= ly;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ x ^ y) < 0)
    {
        return (ldy ^ x) < 0;   // (left is negative)
    }

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
=
= [crispy] turned into a general R_PointToAngle() flavor
= called with either slope_div = SlopeDivCrispy() from R_PointToAngleCrispy()
= or slope_div = SlopeDiv() else
================================================================================
*/

angle_t R_PointToAngleSlope (fixed_t x, fixed_t y,
                            int (*slope_div) (unsigned int num, unsigned int den))
{
    x -= viewx;
    y -= viewy;
    
    if ((!x) && (!y))
    {
        return 0;
    }

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
                return ANG90 + tantoangle[slope_div(x,y)];
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
    if (x_viewx < INT_MIN || x_viewx > INT_MAX
    ||  y_viewy < INT_MIN || y_viewy > INT_MAX)
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

/*
================================================================================
=
= R_PointToDist
=
================================================================================
*/

fixed_t R_PointToDist (fixed_t x, fixed_t y)
{
    int     angle;
    fixed_t dx, dy;
    fixed_t temp, dist, frac;

    dx = abs(x - viewx);
    dy = abs(y - viewy);

    if (dy>dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    // Fix crashes in udm1.wad

    if (dx != 0)
    {
        frac = FixedDiv(dy, dx);
    }
    else
    {
        frac = 0;
    }

    angle = (tantoangle[frac>>DBITS]+ANG90) >> ANGLETOFINESHIFT;

    // use as cosine
    dist = FixedDiv (dx, finesine[angle] );	

    return dist;
}

/*
================================================================================
=
= R_InterpolateAngle
=
= [AM] Interpolate between two angles.
=
================================================================================
*/

angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale)
{
    if (nangle == oangle)
    {
        return nangle;
    }
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
        {
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
        }
        else // Wrapped around
        {
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
        }
    }
    else // nangle < oangle
    {
        if (oangle - nangle < ANG270)
        {
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
        }
        else // Wrapped around
        {
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
        }
    }
}

/*
================================================================================
=
= R_InitTextureMapping
=
================================================================================
*/

static void R_InitTextureMapping (void)
{
    int     i, x, t;
    fixed_t focallength;

    // Use tangent table to generate viewangletox:
    // viewangletox will give the next greatest x after the view angle.
    //
    // Calc focallength so FIELDOFVIEW angles covers SCREENWIDTH.

    // [crispy] in widescreen mode, make sure the same number of horizontal
    // pixels shows the same part of the game scene as in regular rendering mode
    fixed_t focalwidth;
    focalwidth = (((320 << hires)>>detailshift)/2)<<FRACBITS;
    focallength = FixedDiv(aspect_ratio >= 2 ? focalwidth : 
                           centerxfrac, finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);

    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (finetangent[i] > FRACUNIT * 2)
        {
            t = -1;
        }
        else if (finetangent[i] < -FRACUNIT * 2)
        {
            t = viewwidth + 1;
        }
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;

            if (t < -1)
            {
                t = -1;
            }
            else if (t > viewwidth + 1)
            {
                t = viewwidth + 1;
            }
        }
        viewangletox[i] = t;
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.	
    for (x = 0 ; x <= viewwidth ; x++)
    {
        i = 0;

        while (viewangletox[i] > x)
        {
            i++;
        }

        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
        // [crispy] calculate sky angle for drawing horizontally linear skies.
        // Taken from GZDoom and refactored for integer math.
        linearskyangle[x] = ((viewwidth / 2 - x) * ((screenwidth << 6) / viewwidth))
                                                 * (ANG90 / (SCREENWIDTH << 6));
    }

    // Take out the fencepost cases from viewangletox.
    for (i = 0 ; i < FINEANGLES / 2 ; i++)
    {
        t = FixedMul(finetangent[i], focallength);
        t = centerx - t;

        if (viewangletox[i] == -1)
        {
            viewangletox[i] = 0;
        }
        else if (viewangletox[i] == viewwidth + 1)
        {
            viewangletox[i] = viewwidth;
        }
    }

    clipangle = xtoviewangle[0];
}

/*
================================================================================
=
= R_InitLightTables
=
= Only inits the zlight table, 
= because the scalelight table changes with view size.
=
================================================================================
*/

#define		DISTMAP	2

void R_InitLightTables (void)
{
    int level;
    int scale;

    // [JN] Define, which diminished lighting to use
    if (smoothlight && !vanillaparm)
    {
        lightzshift = LIGHTZSHIFT;
        maxlightz = MAXLIGHTZ;
    }
    else
    {
        lightzshift = LIGHTZSHIFT_VANILLA;
        maxlightz = MAXLIGHTZ_VANILLA;
    }

    // Calculate the light levels to use for each level / distance combination.
    for (int i = 0 ; i< LIGHTLEVELS ; i++)
    {
        const int firstmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;

        for (int j = 0 ; j < maxlightz ; j++)
        {
            scale = FixedDiv((320 / 2 * FRACUNIT), ((j + 1) << lightzshift));

            scale >>= LIGHTSCALESHIFT;
            level = firstmap - scale / DISTMAP;

            if (level < 0)
            {
                level = 0;
            }
            if (level >= NUMCOLORMAPS)
            {
                level = NUMCOLORMAPS - 1;
            }

            zlight[i][j] = colormaps + level * 256;
        }
    }
}

/*
================================================================================
=
= R_SetViewSize
=
= Don't really change anything here, because i might be in the middle of
= a refresh. The change will take effect next refresh.
=
================================================================================
*/

void R_SetViewSize (int blocks)
{
    setsizeneeded = true;
    setblocks = blocks;
}

/*
================================================================================
=
= R_ExecuteSetViewSize
=
================================================================================
*/

void R_ExecuteSetViewSize (void)
{
    int     i, j;
    int     level;
    fixed_t cosadj;
    fixed_t dy;

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

    viewwidth = scaledviewwidth >> detailshift;
    viewheight = scaledviewheight >> (detailshift && hires);

    centery = viewheight / 2;
    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    if (aspect_ratio >= 2)
    {
        projection = MIN(centerxfrac, (((320 << hires) >> detailshift) / 2) << FRACBITS);
    }
    else
    {
        projection = centerxfrac;
    }

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        tlcolfunc = R_DrawTLColumn;
        alttlcolfunc = R_DrawAltTLColumn;
        extratlcolfunc = R_DrawExtraTLColumn;
        transcolfunc = R_DrawTranslatedColumn;
        transtlcolfunc = R_DrawTranslatedTLColumn;
        spanfunc = R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        tlcolfunc = R_DrawTLColumnLow;
        alttlcolfunc = R_DrawAltTLColumnLow;
        extratlcolfunc = R_DrawExtraTLColumnLow;
        transcolfunc = R_DrawTranslatedColumnLow;
        transtlcolfunc = R_DrawTranslatedTLColumnLow;
        spanfunc = R_DrawSpanLow;
    }

    R_InitBuffer(scaledviewwidth, scaledviewheight);
    R_InitTextureMapping();

    // psprite scales
    pspritescale = FRACUNIT * viewwidth / origwidth;
    pspriteiscale = FRACUNIT * origwidth / viewwidth;

    // thing clipping
    for (i = 0 ; i < viewwidth ; i++)
    {
        screenheightarray[i] = viewheight;
    }

    // planes
    for (i = 0; i < viewheight; i++)
    {
        const fixed_t num = (viewwidth<<(detailshift && !hires))/2*FRACUNIT;
        const fixed_t num_wide = MIN(viewwidth<<detailshift, ORIGWIDTH << !detailshift)/2*FRACUNIT;

        for (j = 0; j < lookdirs; j++)
        {
            if (aspect_ratio >= 2)
            {
                dy = ((i-(viewheight/2 + ((j-lookdirmin) << (hires && !detailshift))
                   * (screenblocks < 9 ? screenblocks : 9) / 10))<<FRACBITS)+FRACUNIT/2;

                dy = abs(dy / hires);
            }
            else
            {
                dy = ((i-(viewheight/2 + ((j-lookdirmin) << (hires && !detailshift))
                   * (screenblocks < 11 ? screenblocks : 11) / 10))<<FRACBITS)+FRACUNIT/2;

                dy = abs(dy);
            }

            yslopes[j][i] = FixedDiv (aspect_ratio >= 2 ? num_wide : num, dy);
        }
    }

    yslope = yslopes[lookdirmin];

    for (i = 0 ; i < viewwidth ; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

    // Calculate the light levels to use for each level / scale combination.
    for (i = 0 ; i < LIGHTLEVELS ; i++)
    {
        const int firstmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (j = 0 ; j < MAXLIGHTSCALE ; j++)
        {
            level = firstmap - j * screenwidth / (viewwidth << detailshift) / DISTMAP;

            if (level < 0)
            {
                level = 0;
            }
            if (level >= NUMCOLORMAPS)
            {
                level = NUMCOLORMAPS - 1;
            }

            scalelight[i][j] = colormaps + level * 256;
        }
    }

    // [crispy] lookup table for horizontal screen coordinates
    for (i = 0, j = screenwidth - 1; i < screenwidth; i++, j--)
    {
        flipscreenwidth[i] = flip_levels ? j : i;
    }

    flipviewwidth = flipscreenwidth + (flip_levels ? (screenwidth - scaledviewwidth) : 0);

    // [JN] Skip weapon bobbing interpolation for next frame.
    skippsprinterp = true;

    // Draw the border, erase old menu stuff.
    R_DrawViewBorder();
}

/*
================================================================================
=
= R_Init
=
================================================================================
*/

void R_Init (void)
{
    // [JN] Wide screen: don't allow unsupported view modes at startup
    if (aspect_ratio >= 2)
    {
        if (screenblocks < 9)
            screenblocks = 9;
        if (screenblocks > 12)
            screenblocks = 12;
    }

    if (quadres)
    {
        lookdirmin = LOOKDIRMIN2;
        lookdirmax = LOOKDIRMAX2;
        lookdirs = LOOKDIRS2;
    }
    else
    {
        lookdirmin = LOOKDIRMIN;
        lookdirmax = LOOKDIRMAX;
        lookdirs = LOOKDIRS;
    }

    R_InitClipSegs ();
    R_InitSpritesRes ();
    R_InitPlanesRes ();
    R_InitVisplanesRes ();

    R_InitData();
    // viewwidth / viewheight  are set by the defaults
    R_SetViewSize(screenblocks);
    R_InitLightTables();
    R_InitSkyMap();
    R_InitTranslationTables();
}

/*
================================================================================
=
= R_PointInSubsector
=
= [JN] killough 5/2/98: reformatted, cleaned up
=
================================================================================
*/

subsector_t *R_PointInSubsector (fixed_t x, fixed_t y)
{
    int nodenum = numnodes-1;

    // special case for trivial maps (single subsector, no nodes)
    if (numnodes == 0)
    {
        return subsectors;
    }

    while (!(nodenum & NF_SUBSECTOR))
    {
        nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

/*
================================================================================
=
= R_SetupFrame
=
================================================================================
*/

static void R_SetupFrame (player_t *player)
{
    int tempCentery;
    int pitch;
    static int x_quake, y_quake, quaketime; // [crispy]

    viewplayer = player;

    // [AM] Interpolate the player camera if the feature is enabled.
    if (uncapped_fps &&
        // Don't interpolate on the first tic of a level,
        // otherwise oldviewz might be garbage.
        leveltime > 1 &&
        // Don't interpolate if the player did something
        // that would necessitate turning it off for a tic.
        player->mo->interp == true &&
        // Don't interpolate during a paused state
        leveltime > oldleveltime)
    {
        viewx = player->mo->oldx + FixedMul(player->mo->x - player->mo->oldx, fractionaltic);
        viewy = player->mo->oldy + FixedMul(player->mo->y - player->mo->oldy, fractionaltic);
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
        pitch = player->lookdir / MLOOKUNIT;
    }

    if (localQuakeHappening[displayplayer] && !paused)
    {
        // [crispy] only get new quake values once every gametic
        if (leveltime > quaketime)
        {
            int intensity = localQuakeHappening[displayplayer];
            x_quake = ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;
            y_quake = ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;
            quaketime = leveltime;
        }

        if (uncapped_fps)
        {
            viewx += FixedMul(x_quake, fractionaltic);
            viewy += FixedMul(y_quake, fractionaltic);
        }
        else
        {
            viewx += x_quake;
            viewy += y_quake;
        }
    }
    else if (!localQuakeHappening[displayplayer])
    {
        quaketime = 0;
    }

    extralight = player->extralight;
    extralight += extra_level_brightness; // [JN] Level Brightness feature.

    if (pitch > lookdirmax)
    {
        pitch = lookdirmax;
    }
    else if (pitch < -lookdirmin)
    {
        pitch = -lookdirmin;
    }

    // [JN] Extend pitch range in quad resolution.
    if (quadres)
    {
        pitch <<= quadres;
    }

    // apply new yslope[] whenever "lookdir", "detailshift" or "screenblocks" change
    if (aspect_ratio >= 2)
    {
        tempCentery = viewheight/2 + (pitch << (hires && !detailshift))
                    * (screenblocks < 9 ? screenblocks : 9) / 10;
    }
    else
    {
        tempCentery = viewheight/2 + (pitch << (hires && !detailshift))
                    * (screenblocks < 11 ? screenblocks : 11) / 10;
    }

    if (centery != tempCentery)
    {
        centery = tempCentery;
        centeryfrac = centery << FRACBITS;
        yslope = yslopes[lookdirmin + pitch];
    }

    viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];

    if (player->fixedcolormap)
    {
        fixedcolormap = colormaps + player->fixedcolormap * 256 * sizeof(lighttable_t);
        walllights = scalelightfixed;

        for (int i = 0 ; i < MAXLIGHTSCALE ; i++)
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
}

/*
================================================================================
=
= R_ClearStats
=
================================================================================
*/

void R_ClearStats (void)
{
    rendered_segs = 0;
    rendered_visplanes = 0;
    rendered_vissprites = 0;
}

/*
================================================================================
=
= R_RenderView
=
================================================================================
*/

void R_RenderPlayerView (player_t *player)
{
    R_SetupFrame(player);

    // Clear buffers.
    R_ClearClipSegs();
    R_ClearDrawSegs();
    if (automapactive && !automap_overlay)
    {
        R_RenderBSPNode (numnodes-1);
        return;
    }

    // [JN] Fill level's "out of bounds" with black color.
    V_DrawFilledBox(viewwindowx, viewwindowy, scaledviewwidth, scaledviewheight, 0);

    R_ClearPlanes();
    R_ClearSprites();

    // [crispy] Interpolate polyobjects here
    PO_InterpolatePolyObjects();

    // Check for new console commands.
    NetUpdate();

    // [JN] Smooth texture scrolling.
    if (!paused && uncapped_fps)
    {
        R_SmoothTextureScrolling();      
    }

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

    // Check for new console commands.
    NetUpdate();
    R_DrawPlanes();
    
    // Check for new console commands.
    NetUpdate();
    R_DrawMasked();
    
    // Check for new console commands.
    NetUpdate();
}

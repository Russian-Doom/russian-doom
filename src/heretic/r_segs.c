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
#include "i_system.h"
#include "doomdef.h"
#include "r_local.h"
#include "r_bmaps.h"
#include "jn.h"


/*
================================================================================
=
= R_SEGS.C
=
= This version has the tall-sector-crossing-precision-bug fixed.
=
================================================================================
*/


static int toptexture, bottomtexture, midtexture;
static int rw_x, rw_stopx;
static int worldtop, worldbottom, worldhigh, worldlow;

// True if any of the segs textures might be visible
static boolean segtextured;

// False if the back side is the same plane.
boolean markfloor, markceiling, maskedtexture;

// Wall related.
int rw_angle1;  // angle to line origin
static angle_t rw_normalangle, rw_centerangle;
static fixed_t rw_offset;
static fixed_t rw_distance;
static fixed_t rw_scale;
static fixed_t rw_scalestep;
static fixed_t rw_midtexturemid;
static fixed_t rw_toptexturemid;
static fixed_t rw_bottomtexturemid;

int64_t pixhigh, pixlow;  // [crispy] WiggleFix
fixed_t pixhighstep, pixlowstep;

int64_t	topfrac, bottomfrac;  // [crispy] WiggleFix
fixed_t topstep, bottomstep;


lighttable_t **walllights;
lighttable_t **walllights_top;     // 
lighttable_t **walllights_middle;  // [JN] Additional tables for brightmaps
lighttable_t **walllights_bottom;  //

static int *maskedtexturecol; // [crispy] 32-bit integer math


/*
================================================================================
=
= [crispy] WiggleFix: add this code block near the top of r_segs.c
=
= R_FixWiggle()
= Dynamic wall/texture rescaler, AKA "WiggleHack II"
=  by Kurt "kb1" Baumgardner ("kb") and Andrey "Entryway" Budko ("e6y")
=
=  [kb] When the rendered view is positioned, such that the viewer is
=   looking almost parallel down a wall, the result of the scale
=   calculation in R_ScaleFromGlobalAngle becomes very large. And, the
=   taller the wall, the larger that value becomes. If these large
=   values were used as-is, subsequent calculations would overflow,
=   causing full-screen HOM, and possible program crashes.
=
=  Therefore, vanilla Doom clamps this scale calculation, preventing it
=   from becoming larger than 0x400000 (64*FRACUNIT). This number was
=   chosen carefully, to allow reasonably-tight angles, with reasonably
=   tall sectors to be rendered, within the limits of the fixed-point
=   math system being used. When the scale gets clamped, Doom cannot
=   properly render the wall, causing an undesirable wall-bending
=   effect that I call "floor wiggle". Not a crash, but still ugly.
=
=  Modern source ports offer higher video resolutions, which worsens
=   the issue. And, Doom is simply not adjusted for the taller walls
=   found in many PWADs.
=
=  This code attempts to correct these issues, by dynamically
=   adjusting the fixed-point math, and the maximum scale clamp,
=   on a wall-by-wall basis. This has 2 effects:
=
=  1. Floor wiggle is greatly reduced and/or eliminated.
=  2. Overflow is no longer possible, even in levels with maximum
=     height sectors (65535 is the theoretical height, though Doom
=     cannot handle sectors > 32767 units in height.
=
=  The code is not perfect across all situations. Some floor wiggle can
=   still be seen, and some texture strips may be slightly misaligned in
=   extreme cases. These effects cannot be corrected further, without
=   increasing the precision of various renderer variables, and,
=   possibly, creating a noticable performance penalty.
=
================================================================================
*/

static int max_rwscale = 64 * FRACUNIT;
static int heightbits = 12;
static int heightunit = (1 << 12);
static int invhgtbits = 4;

static const struct
{
    int clamp;
    int heightbits;
} scale_values[9] = {
    {2048 * FRACUNIT, 12},
    {1024 * FRACUNIT, 12},
    {1024 * FRACUNIT, 11},
    { 512 * FRACUNIT, 11},
    { 512 * FRACUNIT, 10},
    { 256 * FRACUNIT, 10},
    { 256 * FRACUNIT,  9},
    { 128 * FRACUNIT,  9},
    {  64 * FRACUNIT,  9}
};

/*
================================================================================
=
= R_FixWiggle
=
================================================================================
*/

void R_FixWiggle (sector_t *sector)
{
    static int lastheight = 0;
    int height = (sector->interpceilingheight - sector->interpfloorheight) >> FRACBITS;

    // disallow negative heights. using 1 forces cache initialization
    if (height < 1)
    {
        height = 1;
    }

    // early out?
    if (height != lastheight)
    {
        lastheight = height;

        // initialize, or handle moving sector
        if (height != sector->cachedheight)
        {
            sector->cachedheight = height;
            sector->scaleindex = 0;
            height >>= 7;

            // calculate adjustment
            while (height >>= 1)
            sector->scaleindex++;
        }

        // fine-tune renderer for this wall
        max_rwscale = scale_values[sector->scaleindex].clamp;
        heightbits = scale_values[sector->scaleindex].heightbits;
        heightunit = (1 << heightbits);
        invhgtbits = FRACBITS - heightbits;
    }
}

/*
================================================================================
=
= R_RenderMaskedSegRange
=
================================================================================
*/

void R_RenderMaskedSegRange (drawseg_t *ds, int x1, int x2)
{
    unsigned  index;
    int       lightnum;
    int       texnum;
    column_t *col;

    // Calculate light table.
    // Use different light tables for horizontal / vertical / diagonal.
    // OPTIMIZE: get rid of LIGHTSEGSHIFT globally.
    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];

    lightnum = ((frontsector->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;

    // [JN] Fake contrast: make optional
    if (fake_contrast)
    {
        if (curline->v1->y == curline->v2->y)
        {
            lightnum--;
        }
        else if (curline->v1->x == curline->v2->x)
        {
            lightnum++;
        }
    }

    if (lightnum < 0)
    {
        walllights = scalelight[0];
    }
    else if (lightnum >= LIGHTLEVELS)
    {
        walllights = scalelight[LIGHTLEVELS - 1];
    }
    else
    {
        walllights = scalelight[lightnum];
    }

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;
    spryscale = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // Find positioning.
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
    {
        dc_texturemid = frontsector->interpfloorheight > backsector->interpfloorheight ?
                        frontsector->interpfloorheight : backsector->interpfloorheight;
        dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
    }
    else
    {
        dc_texturemid = frontsector->interpceilingheight < backsector->interpceilingheight ?
                        frontsector->interpceilingheight : backsector->interpceilingheight;
        dc_texturemid = dc_texturemid - viewz;
    }
    dc_texturemid += curline->sidedef->rowoffset;

    if (fixedcolormap)
    {
        dc_colormap = fixedcolormap;
    }

    // Draw the columns.
    for (dc_x = x1 ; dc_x <= x2 ; dc_x++)
    {
        // calculate lighting
        if (maskedtexturecol[dc_x] != INT_MAX) // [crispy] 32-bit integer math
        {
            if (!fixedcolormap)
            {
                index = spryscale >> (LIGHTSCALESHIFT - (detailshift && hires) + hires);

                if (index >= MAXLIGHTSCALE)
                {
                    index = MAXLIGHTSCALE - 1;
                }

                dc_colormap = walllights[index];
            }

            // [crispy] apply Killough's int64 sprtopscreen overflow fix
            // from winmbf/Source/r_segs.c:174-191
            // killough 3/2/98:
            //
            // This calculation used to overflow and cause crashes in Doom:
            //
            // sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
            //
            // This code fixes it, by using double-precision intermediate
            // arithmetic and by skipping the drawing of 2s normals whose
            // mapping to screen coordinates is totally out of range:
            {
                int64_t t = ((int64_t) centeryfrac << FRACBITS) 
                          - (int64_t) dc_texturemid * spryscale;

                if (t + (int64_t) textureheight[texnum] * spryscale < 0 
                ||  t > (int64_t) SCREENHEIGHT << FRACBITS*2)
                {
                    spryscale += rw_scalestep; // [crispy] MBF had this in the for-loop iterator
                    continue; // skip if the texture is out of screen's range
                }
                sprtopscreen = (int64_t)(t >> FRACBITS); // [crispy] WiggleFix
            }

            dc_iscale = 0xffffffffu / (unsigned)spryscale;

            // draw the texture
            col = (column_t *)((byte *)R_GetColumn(texnum, maskedtexturecol[dc_x], false) - 3);

            R_DrawMaskedColumn(col, -1);
            maskedtexturecol[dc_x] = INT_MAX; // [crispy] 32-bit integer math
        }
        spryscale += rw_scalestep;
    }
}

/*
================================================================================
=
= R_RenderSegLoop
=
= Draws zero, one, or two textures (and possibly a masked texture) for walls
= Can draw or mark the starting pixel of floor and ceiling textures.
=
= CALLED: CORE LOOPING ROUTINE
=
= [JN] Note: SPARKLEFIX has been taken from DOOM Retro.
= Many thanks to Brad Harding for his research and fixing this bug!
=
================================================================================
*/

static int didsolidcol;  // True if at least one column was marked solid

static void R_RenderSegLoop(void)
{
    angle_t     angle;
    unsigned    index;
    int         mid;
    fixed_t     texturecolumn = 0; // [JN] Purely to shut up the compiler.

    rendered_segs++;

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
        // mark floor / ceiling areas
        int yl = (int)((topfrac + heightunit - 1) >> heightbits);  // [crispy] WiggleFix
        int yh = (int)(bottomfrac >> heightbits);  // [crispy] WiggleFix

        // no space above wall?
        int bottom, top = ceilingclip[rw_x]+1;
      
        if (yl < top)
        {
            yl = top;
        }

        if (markceiling)
        {
            bottom = yl-1;

            if (bottom >= floorclip[rw_x])
            {
                bottom = floorclip[rw_x]-1;
            }
            if (top <= bottom)
            {
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
            }

            ceilingclip[rw_x] = bottom;
        }

        bottom = floorclip[rw_x]-1;

        if (yh > bottom)
        {
            yh = bottom;
        }

        if (markfloor)
        {
            top = yh < ceilingclip[rw_x] ? ceilingclip[rw_x] : yh;

            if (++top <= bottom)
            {
                floorplane->top[rw_x] = top;
                floorplane->bottom[rw_x] = bottom;
            }

            floorclip[rw_x] = top;
        }
        
        // [JN] Moved outside "segtextured"
        index = rw_scale >> (LIGHTSCALESHIFT - (detailshift && hires) + hires);

        // texturecolumn and lighting are independent of wall tiers
        if (segtextured)
        {
            // calculate texture offset
            angle = (rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT;
            texturecolumn = rw_offset - FixedMul(finetangent[angle], rw_distance);
            texturecolumn >>= FRACBITS;

            // calculate lighting
            if (index >= MAXLIGHTSCALE)
            {
                index = MAXLIGHTSCALE-1;
            }

            // [JN] All wall segments (top/middle/bottom) now using own lights
            dc_x = rw_x;
            dc_iscale = 0xffffffffu / (unsigned)rw_scale - SPARKLEFIX; // [JN] Sparkle fix
        }

        // draw the wall tiers
        if (midtexture)
        {
            // single sided line
            dc_yl = yl;
            dc_yh = yh;
            dc_texturemid = rw_midtexturemid;
            dc_source = R_GetColumn(midtexture, texturecolumn,true);
            dc_texheight = textureheight[midtexture] >> FRACBITS;

            // [JN] Account fixed colormap
            if (fixedcolormap)
            {
                dc_colormap = fixedcolormap;
            }
            else
            {
                dc_colormap = walllights_middle[index];
            }

            colfunc ();
            ceilingclip[rw_x] = viewheight;
            floorclip[rw_x] = -1;
        }
        else
        {
            // two sided line
            if (toptexture)
            {
                // top wall
                mid = (int)(pixhigh>>heightbits); // [crispy] WiggleFix
                pixhigh += pixhighstep;

                if (mid >= floorclip[rw_x])
                {
                    mid = floorclip[rw_x]-1;
                }

                if (mid >= yl)
                {
                    dc_yl = yl;
                    dc_yh = mid;
                    dc_texturemid = rw_toptexturemid + (dc_yl - centery + 1) * SPARKLEFIX; // [JN] Sparkle fix
                    dc_source = R_GetColumn(toptexture,texturecolumn,true);
                    dc_texheight = textureheight[toptexture]>>FRACBITS;

                    // [JN] Account fixed colormap
                    if (fixedcolormap)
                    {
                        dc_colormap = fixedcolormap;
                    }
                    else
                    {
                        dc_colormap = walllights_top[index];
                    }

                    colfunc ();
                    ceilingclip[rw_x] = mid;
                }
                else
                {
                    ceilingclip[rw_x] = yl-1;
                }
            }
            else
            {
                // no top wall
                if (markceiling)
                {
                    ceilingclip[rw_x] = yl-1;
                }
            }

            if (bottomtexture)
            {
                // bottom wall
                mid = (int)((pixlow+heightunit-1) >> heightbits); // [crispy] WiggleFix
                pixlow += pixlowstep;

                // no space above wall?
                if (mid <= ceilingclip[rw_x])
                {
                    mid = ceilingclip[rw_x]+1;
                }

                if (mid <= yh)
                {
                    dc_yl = mid;
                    dc_yh = yh;
                    dc_texturemid = rw_bottomtexturemid + (dc_yl - centery + 1) * SPARKLEFIX; // [JN] Sparkle fix
                    dc_source = R_GetColumn(bottomtexture,texturecolumn,true);
                    dc_texheight = textureheight[bottomtexture]>>FRACBITS;

                    // [JN] Account fixed colormap
                    if (fixedcolormap)
                    {
                        dc_colormap = fixedcolormap;
                    }
                    else
                    {
                        dc_colormap = walllights_bottom[index];
                    }

                    colfunc ();
                    floorclip[rw_x] = mid;
                }
                else
                {
                    floorclip[rw_x] = yh+1;
                }
            }
            else
            {
                // no bottom wall
                if (markfloor)
                {
                    floorclip[rw_x] = yh+1;
                }
            }

            // [JN] cph - if we completely blocked further sight through this column,
            // add this info to the solid columns array for r_bsp.c
            if ((markceiling || markfloor) && (floorclip[rw_x] <= ceilingclip[rw_x] + 1)) 
            {
                solidcol[rw_x] = 1; 
                didsolidcol = 1;
            }

            if (maskedtexture)
            {
                // save texturecol for backdrawing of masked mid texture
                maskedtexturecol[rw_x] = texturecolumn;
            }
        }

        rw_scale += rw_scalestep;
        topfrac += topstep;
        bottomfrac += bottomstep;
    }
}

/*
================================================================================
=
= R_ScaleFromGlobalAngle
=
= [crispy] WiggleFix: move R_ScaleFromGlobalAngle function to r_segs.c,
= above R_StoreWallRange
=
================================================================================
*/

static fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
    int     anglea = ANG90 + (visangle - viewangle);
    int     angleb = ANG90 + (visangle - rw_normalangle);
    int     den = FixedMul(rw_distance, finesine[anglea >> ANGLETOFINESHIFT]);
    fixed_t	num = FixedMul(projection, finesine[angleb >> ANGLETOFINESHIFT]);
    fixed_t scale;

    if (den > (num >> 16))
    {
        scale = FixedDiv(num, den);

        // [kb] When this evaluates True, the scale is clamped,
        //  and there will be some wiggling.
        if (scale > max_rwscale)
        {
            scale = max_rwscale;
        }
        else if (scale < 256)
        {
            scale = 256;
        }
    }
    else
    {
        scale = max_rwscale;
    }

    return scale;
}

/*
================================================================================
=
= R_StoreWallRange
=
= A wall segment will be drawn between start and stop pixels (inclusive).
=
================================================================================
*/

void R_StoreWallRange (int start, int stop)
{
    fixed_t     vtop;
    int         lightnum;
    int64_t     dx, dy, dx1, dy1, dist; // [crispy] fix long wall wobble
    const uint32_t len = curline->length;

    // [crispy] remove MAXDRAWSEGS Vanilla limit
    if (ds_p == drawsegs+maxdrawsegs)
    {
        unsigned newmax = maxdrawsegs ? maxdrawsegs*2 : 128; // killough
        drawsegs = I_Realloc(drawsegs,newmax*sizeof(*drawsegs));
        ds_p = drawsegs+maxdrawsegs;
        maxdrawsegs = newmax;
    }

#ifdef RANGECHECK
    if (start >= viewwidth || start > stop)
        I_Error(english_language ?
                "Bad R_RenderWallRange: %i to %i" :
                "Bad R_RenderWallRange: %i ? %i",
                start, stop);
#endif

    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for auto map
    linedef->flags |= ML_MAPPED;

    // [crispy] (flags & ML_MAPPED) is all we need to know for automap
    if (automapactive && !automap_overlay)
    {
        return;
    }

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->r_angle + ANG90;

    
    // [crispy] fix long wall wobble
    // thank you very much Linguica, e6y and kb1
    // http://www.doomworld.com/vb/post/1340718
    // shift right to avoid possibility of int64 overflow in rw_distance calculation
    dx = ((int64_t)curline->v2->px - curline->v1->px) >> 1;
    dy = ((int64_t)curline->v2->py - curline->v1->py) >> 1;
    dx1 = ((int64_t)viewx - curline->v1->px) >> 1;
    dy1 = ((int64_t)viewy - curline->v1->py) >> 1;
    dist = ((dy * dx1 - dx * dy1) / len) << 1;
    rw_distance = (fixed_t)BETWEEN(INT_MIN, INT_MAX, dist);

    ds_p->x1 = rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop + 1;

    // [JN] killough 1/6/98, 2/1/98: remove limit on openings
    {     
        extern int *openings; // dropoff overflow
        extern size_t maxopenings;
        size_t pos = lastopening - openings;
        size_t need = (rw_stopx - start)*sizeof(*lastopening) + pos;

        if (need > maxopenings)
        {
            drawseg_t *ds;                // jff 8/9/98 needed for fix from ZDoom
            int *oldopenings = openings;  // dropoff overflow
            int *oldlast = lastopening;   // dropoff overflow

            do
            {
                maxopenings = maxopenings ? maxopenings*2 : 16384;
            } while (need > maxopenings);

            openings = I_Realloc(openings, maxopenings * sizeof(*openings));
            lastopening = openings + pos;

            // jff 8/9/98 borrowed fix for openings from ZDOOM1.14
            // [RH] We also need to adjust the openings pointers that
            //    were already stored in drawsegs.
            for (ds = drawsegs; ds < ds_p; ds++)
            {
#define ADJUST(p) if (ds->p + ds->x1 >= oldopenings && ds->p + ds->x1 <= oldlast)\
            ds->p = ds->p - oldopenings + openings;
                ADJUST (maskedtexturecol);
                ADJUST (sprtopclip);
                ADJUST (sprbottomclip);
            }
#undef ADJUST
        }
    }

    // [crispy] WiggleFix: add this line, in r_segs.c:R_StoreWallRange,
    // right before calls to R_ScaleFromGlobalAngle:
    R_FixWiggle(frontsector);

    // calculate scale at both ends and step    
    ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);

    if (stop > start)
    {
        ds_p->scale2 = R_ScaleFromGlobalAngle(viewangle + xtoviewangle[stop]);
        ds_p->scalestep = rw_scalestep = (ds_p->scale2 - rw_scale) / (stop - start);
    }
    else
    {
        ds_p->scale2 = ds_p->scale1;
    }

    // calculate texture boundaries and decide if floor / ceiling marks are needed
    worldtop = frontsector->interpceilingheight - viewz;
    worldbottom = frontsector->interpfloorheight - viewz;

    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;
        if (linedef->flags & ML_DONTPEGBOTTOM)
        {
            vtop = frontsector->interpfloorheight + textureheight[sidedef->midtexture];
            // bottom of texture at bottom
            rw_midtexturemid = vtop - viewz;
        }
        else
        {
            // top of texture at top
            rw_midtexturemid = worldtop;
        }
        rw_midtexturemid += sidedef->rowoffset;
        ds_p->silhouette = SIL_BOTH;
        ds_p->sprtopclip = screenheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->bsilheight = INT_MAX;
        ds_p->tsilheight = INT_MIN;
    }
    else
    {
        // two sided line
        ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
        ds_p->silhouette = 0;

        // [JN] cph - closed 2S line e.g. door
        if (linedef->r_flags & RF_CLOSED)
        {
            // cph - killough's (outdated) comment follows - this deals with both 
            // "automap fixes", his and mine
            // killough 1/17/98: this test is required if the fix
            // for the automap bug (r_bsp.c) is used, or else some
            // sprites will be displayed behind closed doors. That
            // fix prevents lines behind closed doors with dropoffs
            // from being displayed on the automap.

            ds_p->silhouette = SIL_BOTH;
            ds_p->sprbottomclip = negonearray;
            ds_p->bsilheight = INT_MAX;
            ds_p->sprtopclip = screenheightarray;
            ds_p->tsilheight = INT_MIN;
        }
        else 
        {
            // [JN] Not solid - old code.
            if (frontsector->interpfloorheight > backsector->interpfloorheight)
            {
                ds_p->silhouette = SIL_BOTTOM;
                ds_p->bsilheight = frontsector->interpfloorheight;
            }
            else if (backsector->interpfloorheight > viewz)
            {
                ds_p->silhouette = SIL_BOTTOM;
                ds_p->bsilheight = INT_MAX;
            }

            if (frontsector->interpceilingheight < backsector->interpceilingheight)
            {
                ds_p->silhouette |= SIL_TOP;
                ds_p->tsilheight = frontsector->interpceilingheight;
            }
            else if (backsector->interpceilingheight < viewz)
            {
                ds_p->silhouette |= SIL_TOP;
                ds_p->tsilheight = INT_MIN;
    
            }
        }

        worldhigh = backsector->interpceilingheight - viewz;
        worldlow = backsector->interpfloorheight - viewz;

        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == skyflatnum
        &&  backsector->ceilingpic == skyflatnum)
        {
            worldtop = worldhigh;
        }

        if (worldlow != worldbottom
        ||  backsector->floorpic != frontsector->floorpic
        ||  backsector->lightlevel != frontsector->lightlevel
        ||  backsector->special != frontsector->special)
        {
            markfloor = true;
        }
        else
        {
            // same plane on both sides
            markfloor = false;
        }

        if (worldhigh != worldtop
        ||  backsector->ceilingpic != frontsector->ceilingpic
        ||  backsector->lightlevel != frontsector->lightlevel)
        {
            markceiling = true;
        }
        else
        {
            // same plane on both sides
            markceiling = false;
        }

        if (backsector->interpceilingheight <= frontsector->interpfloorheight
        ||  backsector->interpfloorheight >= frontsector->interpceilingheight)
        {
            // closed door
            markceiling = markfloor = true;
        }

        if (worldhigh < worldtop)
        {
            // top texture
            toptexture = texturetranslation[sidedef->toptexture];
            if (linedef->flags & ML_DONTPEGTOP)
            {
                // top of texture at top
                rw_toptexturemid = worldtop;
            }
            else
            {
                vtop = backsector->interpceilingheight
                     + textureheight[sidedef->toptexture];
                // bottom of texture
                rw_toptexturemid = vtop - viewz;
            }
        }
        if (worldlow > worldbottom)
        {
            // bottom texture
            bottomtexture = texturetranslation[sidedef->bottomtexture];

            if (linedef->flags & ML_DONTPEGBOTTOM)
            {
                // bottom of texture at bottom
                // top of texture at top
                rw_bottomtexturemid = worldtop;
            }
            else
            {
                // top of texture at top
                rw_bottomtexturemid = worldlow;
            }
        }

        rw_toptexturemid += sidedef->rowoffset;
        rw_bottomtexturemid += sidedef->rowoffset;

        // allocate space for masked texture tables
        if (sidedef->midtexture)
        {
            // masked midtexture
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += rw_stopx - rw_x;
        }
    }

    // calculate rw_offset (only needed for textured lines)
    segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

    if (segtextured)
    {
        // [crispy] fix long wall wobble
        rw_offset = (fixed_t)(((dx*dx1 + dy*dy1) / len) << 1);
        rw_offset += sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;

        // calculate light table
        // use different light tables for horizontal / vertical / diagonal
        // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
        if (!fixedcolormap)
        {
            lightnum = ((frontsector->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;

            // [JN] Fake contrast: make optional
            if (fake_contrast || vanillaparm)
            {
                if (curline->v1->y == curline->v2->y)
                {
                    lightnum--;
                }
                else if (curline->v1->x == curline->v2->x)
                {
                    lightnum++;
                }

                // [JN] Brightmapped line can't have lightlevel 0,
                // otherwise brightmap will not work at all.
                if (brightmaps && frontsector->lightlevel == 0)
                {
                    lightnum++;
                }
            }

            if (lightnum < 0)
            {
                walllights = scalelight[0];

                // [JN] If sector brightness = 0
                walllights_top = scalelight[0];
                walllights_middle = scalelight[0];
                walllights_bottom = scalelight[0];
            }
            else if (lightnum >= LIGHTLEVELS)
            {
                walllights = scalelight[LIGHTLEVELS - 1];

                // [JN] If sector brightness = 256
                walllights_top = scalelight[LIGHTLEVELS-1];
                walllights_middle = scalelight[LIGHTLEVELS-1];
                walllights_bottom = scalelight[LIGHTLEVELS-1];
            }
            else
            {
                // [JN] Standard formulas first
                walllights = scalelight[lightnum];
                walllights_top = scalelight[lightnum];
                walllights_middle = scalelight[lightnum];
                walllights_bottom = scalelight[lightnum];

                // [JN] Apply brightmaps to walls...
                if (brightmaps && !vanillaparm)
                {
                    //
                    // [JN] Middle segment
                    //
                    if (midtexture)
                    {
                        //  Red only
                        if (midtexture == bmaptexture01)
                        {
                            walllights_middle = fullbright_redonly[lightnum];
                        }
                        //  Blue only
                        if (midtexture == bmaptexture02 || midtexture == bmaptexture03
                        ||  midtexture == bmaptexture04 || midtexture == bmaptexture05)
                        {
                            walllights_middle = fullbright_blueonly[lightnum];
                        }
                        //  Not bronze
                        if (midtexture == bmaptexture06 || midtexture == bmaptexture07)
                        {
                            walllights_middle = fullbright_notbronze[lightnum];
                        }
                    }
                    //
                    // [JN] Top segment
                    //
                    if (toptexture)
                    {
                        //  Red only
                        if (toptexture == bmaptexture01)
                        {
                            walllights_top = fullbright_redonly[lightnum];
                        }
                        //  Blue only
                        if (toptexture == bmaptexture02 || toptexture == bmaptexture03
                        ||  toptexture == bmaptexture04 || toptexture == bmaptexture05)
                        {
                            walllights_top = fullbright_blueonly[lightnum];
                        }
                        //  Not bronze
                        if (toptexture == bmaptexture06 || toptexture == bmaptexture07)
                        {
                            walllights_top = fullbright_notbronze[lightnum];
                        }
                    }
                    //
                    // [JN] Bottom segment
                    //
                    if (bottomtexture)
                    {
                        //  Red only
                        if (bottomtexture == bmaptexture01)
                        {
                            walllights_bottom = fullbright_redonly[lightnum];
                        }
                        //  Blue only
                        if (bottomtexture == bmaptexture02 || bottomtexture == bmaptexture03
                        ||  bottomtexture == bmaptexture04 || bottomtexture == bmaptexture05)
                        {
                            walllights_bottom = fullbright_blueonly[lightnum];
                        }
                        //  Not bronze
                        if (bottomtexture == bmaptexture06 || bottomtexture == bmaptexture07)
                        {
                            walllights_bottom = fullbright_notbronze[lightnum];
                        }
                    }
                }
            }
        }
    }

    // if a floor / ceiling plane is on the wrong side of the view plane
    // it is definately invisible and doesn't need to be marked

    if (frontsector->interpfloorheight >= viewz)
    {
        // above view plane
        markfloor = false;
    }
    if (frontsector->interpceilingheight <= viewz
    &&  frontsector->ceilingpic != skyflatnum)
    {
        // below view plane
        markceiling = false;
    }

    // calculate incremental stepping values for texture edges
    worldtop >>= invhgtbits;
    worldbottom >>= invhgtbits;

    topstep = -FixedMul(rw_scalestep, worldtop);
    topfrac = ((int64_t)centeryfrac>>invhgtbits) 
            - (((int64_t)worldtop * rw_scale)>>FRACBITS); // [crispy] WiggleFix

    bottomstep = -FixedMul(rw_scalestep, worldbottom);
    bottomfrac = ((int64_t)centeryfrac>>invhgtbits)
               - (((int64_t)worldbottom * rw_scale)>>FRACBITS); // [crispy] WiggleFix

    if (backsector)
    {
        worldhigh >>= invhgtbits;
        worldlow >>= invhgtbits;

        if (worldhigh < worldtop)
        {
            pixhigh = ((int64_t)centeryfrac>>invhgtbits)
                    - (((int64_t)worldhigh * rw_scale)>>FRACBITS); // [crispy] WiggleFix
            pixhighstep = -FixedMul(rw_scalestep, worldhigh);
        }
        if (worldlow > worldbottom)
        {
            pixlow = ((int64_t)centeryfrac>>invhgtbits)
                   - (((int64_t)worldlow * rw_scale)>>FRACBITS); // [crispy] WiggleFix
            pixlowstep = -FixedMul(rw_scalestep, worldlow);
        }
    }

    // render it
    if (markceiling)
    {
        if (ceilingplane)  // [JN] killough 4/11/98: add NULL ptr checks
        {
            ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
        }
        else
        {
            markceiling = 0;
        }
    }

    if (markfloor) 
    {
        if (floorplane)  // [JN] killough 4/11/98: add NULL ptr checks
        // [JN] cph 2003/04/18  - ceilingplane and floorplane might be the same
        // visplane (e.g. if both skies); R_CheckPlane doesn't know about
        // modifications to the plane that might happen in parallel with the check
        // being made, so we have to override it and split them anyway if that is
        // a possibility, otherwise the floor marking would overwrite the ceiling
        // marking, resulting in HOM.
        if (markceiling && ceilingplane == floorplane)
        {
            floorplane = R_DupPlane (floorplane, rw_x, rw_stopx-1);
        }
        else
        {
            floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
        }
        else
        {
            markfloor = 0;
        }
    }

    didsolidcol = 0;
    R_RenderSegLoop();

    // [JN] cph - if a column was made solid by this wall, 
    // we _must_ save full clipping info.
    if (backsector && didsolidcol)
    {
        if (!(ds_p->silhouette & SIL_BOTTOM))
        {
            ds_p->silhouette |= SIL_BOTTOM;
            ds_p->bsilheight = backsector->floorheight;
        }
        if (!(ds_p->silhouette & SIL_TOP))
        {
            ds_p->silhouette |= SIL_TOP;
            ds_p->tsilheight = backsector->ceilingheight;
        }
    }

    // save sprite clipping info
    if (((ds_p->silhouette & SIL_TOP) || maskedtexture) && !ds_p->sprtopclip)
    {
        memcpy (lastopening, ceilingclip+start, sizeof(*lastopening)*(rw_stopx-start)); // [crispy] 32-bit integer math
        ds_p->sprtopclip = lastopening - start;
        lastopening += rw_stopx - start;
    }
    if (((ds_p->silhouette & SIL_BOTTOM) || maskedtexture) && !ds_p->sprbottomclip)
    {
        memcpy (lastopening, floorclip+start, sizeof(*lastopening)*(rw_stopx-start)); // [crispy] 32-bit integer math
        ds_p->sprbottomclip = lastopening - start;
        lastopening += rw_stopx - start;
    }
    if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT_MIN;
    }
    if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
    {
        ds_p->silhouette |= SIL_BOTTOM;
        ds_p->bsilheight = INT_MAX;
    }
    ds_p++;
}

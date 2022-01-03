//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	BSP traversal, handling of LineSegs for rendering.
//


#include <stdlib.h>
#include "doomdef.h"
#include "m_bbox.h"
#include "i_system.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_things.h"
#include "doomstat.h"
#include "r_state.h"
#include "r_bsp.h"
#include "jn.h"


seg_t     *curline;
side_t    *sidedef;
line_t    *linedef;
sector_t  *frontsector;
sector_t  *backsector;
drawseg_t *ds_p;

// [JN] killough: New code which removes 2s linedef limit
drawseg_t *drawsegs;
unsigned   maxdrawsegs;

// [JN] CPhipps - 
// Instead of clipsegs, let's try using an array with one entry for each column, 
// indicating whether it's blocked by a solid wall yet or not.

byte *solidcol;

// -----------------------------------------------------------------------------
// R_InitClipSegs
// -----------------------------------------------------------------------------

void R_InitClipSegs(void)
{
    solidcol = calloc(screenwidth, sizeof(*solidcol));
}

// -----------------------------------------------------------------------------
// R_ClearDrawSegs
// -----------------------------------------------------------------------------

void R_ClearDrawSegs (void)
{
    ds_p = drawsegs;
}

// -----------------------------------------------------------------------------
// CPhipps - 
// R_ClipWallSegment
//
// Replaces the old R_Clip*WallSegment functions. It draws bits of walls in those 
// columns which aren't solid, and updates the solidcol[] array appropriately
// -----------------------------------------------------------------------------

static void R_ClipWallSegment (int first, int last, boolean solid)
{
    byte *p;

    while (first < last)
    {
        if (solidcol[first])
        {
            if (!(p = memchr(solidcol+first, 0, last-first)))
            {
                return; // All solid
            }
            first = p - solidcol;
        }
        else
        {
            int to;

            if (!(p = memchr(solidcol+first, 1, last-first)))
            {
                to = last;
            }
            else
            {
                to = p - solidcol;
            }

            R_StoreWallRange(first, to-1);

            if (solid)
            {
                memset(solidcol+first,1,to-first);
            }
            first = to;
        }
    }
}

// -----------------------------------------------------------------------------
// R_RecalcLineFlags
// -----------------------------------------------------------------------------

static void R_RecalcLineFlags (line_t *linedef)
{
    linedef->r_validcount = gametic;

    // First decide if the line is closed, normal, or invisible */
    if (!(linedef->flags & ML_TWOSIDED)
    || backsector->interpceilingheight <= frontsector->interpfloorheight
    || backsector->interpfloorheight >= frontsector->interpceilingheight
    ||
    // if door is closed because back is shut:
    (backsector->interpceilingheight <= backsector->interpfloorheight
 
    // preserve a kind of transparent door/lift special effect:
    && (backsector->interpceilingheight >= frontsector->interpceilingheight
    || curline->sidedef->toptexture)

    && (backsector->interpfloorheight <= frontsector->interpfloorheight
    || curline->sidedef->bottomtexture)

    // properly render skies (consider door "open" if both ceilings are sky):
    && (backsector->ceilingpic !=skyflatnum || frontsector->ceilingpic!=skyflatnum)))
    {
        linedef->r_flags = RF_CLOSED;
    }
    else
    {
        // Reject empty lines used for triggers
        //  and special events.
        // Identical floor and ceiling on both sides,
        // identical light levels on both sides,
        // and no middle texture.
        // CPhipps - recode for speed, not certain if this is portable though
        if (backsector->interpceilingheight != frontsector->interpceilingheight
        || backsector->interpfloorheight != frontsector->interpfloorheight
        || curline->sidedef->midtexture
        || memcmp(&backsector->floor_xoffs, &frontsector->floor_xoffs,
        sizeof(frontsector->floor_xoffs) + sizeof(frontsector->floor_yoffs) +
        sizeof(frontsector->ceiling_xoffs) + sizeof(frontsector->ceiling_yoffs) +
        sizeof(frontsector->ceilingpic) + sizeof(frontsector->floorpic) +
        sizeof(frontsector->lightlevel) + sizeof(frontsector->floorlightsec) +
        sizeof(frontsector->ceilinglightsec)))
        {
            linedef->r_flags = 0;
            return;
        }
        else
        {
            linedef->r_flags = RF_IGNORE;
        }
    }

    // cph - I'm too lazy to try and work with offsets in this
    if (curline->sidedef->rowoffset) return;

    // Now decide on texture tiling
    if (linedef->flags & ML_TWOSIDED)
    {
        int c;

        // Does top texture need tiling
        if ((c = frontsector->interpceilingheight - backsector->interpceilingheight) > 0
        && (textureheight[texturetranslation[curline->sidedef->toptexture]] > c))
        {
            linedef->r_flags |= RF_TOP_TILE;
        }

        // Does bottom texture need tiling
        if ((c = frontsector->interpfloorheight - backsector->interpfloorheight) > 0
        && (textureheight[texturetranslation[curline->sidedef->bottomtexture]] > c))
        {
            linedef->r_flags |= RF_BOT_TILE;
        }
    }
    else
    {
        int c;

        // Does middle texture need tiling
        if ((c = frontsector->interpceilingheight - frontsector->interpfloorheight) > 0
        && (textureheight[texturetranslation[curline->sidedef->midtexture]] > c))
        {
            linedef->r_flags |= RF_MID_TILE;
        }
    }
}

// -----------------------------------------------------------------------------
// R_ClearClipSegs
// -----------------------------------------------------------------------------

void R_ClearClipSegs (void)
{
    memset(solidcol, 0, screenwidth);
}

// -----------------------------------------------------------------------------
// R_MaybeInterpolateSector
// [AM] Interpolate the passed sector, if prudent.
// -----------------------------------------------------------------------------

void R_MaybeInterpolateSector (sector_t *sector)
{
    if (uncapped_fps && !vanillaparm &&
        // Only if we moved the sector last tic.
        sector->oldgametic == gametic - 1)
    {
        // Interpolate between current and last floor/ceiling position.
        if (sector->floorheight != sector->oldfloorheight)
            sector->interpfloorheight = sector->oldfloorheight + FixedMul(sector->floorheight - sector->oldfloorheight, fractionaltic);
        else
            sector->interpfloorheight = sector->floorheight;
        if (sector->ceilingheight != sector->oldceilingheight)
            sector->interpceilingheight = sector->oldceilingheight + FixedMul(sector->ceilingheight - sector->oldceilingheight, fractionaltic);
        else
            sector->interpceilingheight = sector->ceilingheight;
    }
    else
    {
        sector->interpfloorheight = sector->floorheight;
        sector->interpceilingheight = sector->ceilingheight;
    }
}

// -----------------------------------------------------------------------------
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
// -----------------------------------------------------------------------------

static void R_AddLine (seg_t *line)
{
    int      x1, x2;
    angle_t  angle1, angle2;
    angle_t  span, tspan;

    curline = line;

    // [crispy] remove slime trails
    angle1 = R_PointToAngleCrispy (line->v1->px, line->v1->py);
    angle2 = R_PointToAngleCrispy (line->v2->px, line->v2->py);

    // Clip to view edges.
    span = angle1 - angle2;

    // Back side? I.e. backface culling?
    if (span >= ANG180)
    {
        return;
    }

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
        {
            return;
        }
	    angle1 = clipangle;
    }

    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
        {
            return;
        }
        angle2 = -clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 >= x2)  // [JN] killough 1/31/98 -- change == to >= for robustness
    {
        return;
    }

    backsector = line->backsector;

    // Single sided line?
    if (backsector)
    {
        // [AM] Interpolate sector movement before
        //      running clipping tests.  Frontsector
        //      should already be interpolated.
        R_MaybeInterpolateSector(backsector);
    }
    else
    {
        // [JN] If no backsector is present, 
        // just clip the line as a solid segment.
        R_ClipWallSegment (x1, x2, true);
        return;
    }

    // [JN] cph - roll up linedef properties in flags
    if ((linedef = curline->linedef)->r_validcount != gametic) 
    {
        R_RecalcLineFlags(linedef);
    }

    if (linedef->r_flags & RF_IGNORE)
    {
        return;
    }
    else
    {
        R_ClipWallSegment (x1, x2, linedef->r_flags & RF_CLOSED);
    }
}

// -----------------------------------------------------------------------------
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true if some part of the bbox might be visible.
// -----------------------------------------------------------------------------

static const int checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},
    {2,0,2,1},
    {0,0,0,0},
    {3,1,3,0},
    {0},
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};

// -----------------------------------------------------------------------------
// R_CheckBBox
// -----------------------------------------------------------------------------

static boolean R_CheckBBox (const fixed_t *bspcoord)
{
    angle_t    angle1, angle2;
    int        boxpos;
    const int *check;

    // Find the corners of the box that define the edges from current viewpoint.
    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT ] ? 1 : 2) +
             (viewy >= bspcoord[BOXTOP ] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);

    if (boxpos == 5)
    {
        return true;
    }

    check = checkcoord[boxpos];

    angle1 = R_PointToAngleCrispy (bspcoord[check[0]], bspcoord[check[1]]) - viewangle;
    angle2 = R_PointToAngleCrispy (bspcoord[check[2]], bspcoord[check[3]]) - viewangle;

    // [JN] cph - replaced old code, which was unclear and badly commented
    // Much more efficient code now
    if ((signed)angle1 < (signed)angle2)
    { 
        // it's "behind" us
        // Either angle1 or angle2 is behind us, so it doesn't matter if we 
        // change it to the corect sign
        if ((angle1 >= ANG180) && (angle1 < ANG270))
        {
            angle1 = INT_MAX; // which is ANG180-1
        }
        else
        {
            angle2 = INT_MIN;
        }
    }

    if ((signed)angle2 >= (signed)clipangle) return false; // Both off left edge
    if ((signed)angle1 <= -(signed)clipangle) return false; // Both off right edge
    if ((signed)angle1 >= (signed)clipangle) angle1 = clipangle; // Clip at left edge
    if ((signed)angle2 <= -(signed)clipangle) angle2 = -clipangle; // Clip at right edge

    // Find the first clippost that touches the 
    // source post (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    {
        int sx1 = viewangletox[angle1];
        int sx2 = viewangletox[angle2];

        // Does not cross a pixel.
        if (sx1 == sx2)
        {
            return false;
        }

        if (!memchr(solidcol+sx1, 0, sx2-sx1)) return false; 
        // All columns it covers are already solidly covered
    }

    return true;
}

// -----------------------------------------------------------------------------
// R_Subsector
// Determine floor/ceiling planes. Add sprites of things in sector.
// Draw one or more line segments.
//
// [JN] killough 1/31/98 -- made static, polished
// -----------------------------------------------------------------------------

static void R_Subsector (int num)
{
    subsector_t *sub = &subsectors[num];
    seg_t       *line = &segs[sub->firstline];
    int          count = sub->numlines;

#ifdef RANGECHECK
    if (num>=numsubsectors)
	I_Error (english_language ?
             "R_Subsector: ss %i with numss = %i" :
             "R_Subsector: ss %i с numss = %i", num, numsubsectors);
#endif

    frontsector = sub->sector;

    // [AM] Interpolate sector movement.  Usually only needed
    //      when you're standing inside the sector.
    R_MaybeInterpolateSector(frontsector);

    floorplane = frontsector->interpfloorheight < viewz ?
                 R_FindPlane (frontsector->interpfloorheight,
                              frontsector->floorpic,
                              frontsector->lightlevel) : NULL;

    ceilingplane = frontsector->interpceilingheight > viewz ||
                   frontsector->ceilingpic == skyflatnum ?
                   R_FindPlane (frontsector->interpceilingheight,
                                frontsector->ceilingpic,
                                frontsector->lightlevel) : NULL;

    // BSP is traversed by subsector.
    // A sector might have been split into several 
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sub->sector->validcount != validcount && (!automapactive || automap_overlay))
    {
        sub->sector->validcount = validcount;
        R_AddSprites (frontsector);
    }

    while (count--)
    {
        R_AddLine (line++);
    }
}

// -----------------------------------------------------------------------------
// RenderBSPNode
// Renders all subsectors below a given node, traversing subtree recursively.
// Just call with BSP root.
//
// [JN] killough 5/2/98: reformatted, removed tail recursion
// -----------------------------------------------------------------------------

void R_RenderBSPNode (int bspnum)
{
    while (!(bspnum & NF_SUBSECTOR))  // Found a subsector?
    {
        const node_t *bsp = &nodes[bspnum];

        // Decide which side the view point is on.
        int side = R_PointOnSide(viewx, viewy, bsp);

        // Recursively divide front space.
        R_RenderBSPNode(bsp->children[side]);

        // Possibly divide back space.
        if (!R_CheckBBox(bsp->bbox[side^1]))
        {
            return;
        }

        bspnum = bsp->children[side^1];
    }

    R_Subsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}

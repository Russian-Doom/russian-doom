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
//	LineOfSight/Visibility checks, uses REJECT Lookup Table.
//


#include "doomstat.h"
#include "i_system.h"
#include "p_local.h"
#include "jn.h"


static fixed_t   sightzstart;            // eye z of looker
static fixed_t   topslope, bottomslope;  // slopes to top and bottom of target
static fixed_t   t2x, t2y;
static divline_t strace;                 // from t1 to t2

static int sightcounts[2];


// -----------------------------------------------------------------------------
// P_DivlineSide
// Returns side 0 (front), 1 (back), or 2 (on).
// [JN] killough 4/19/98: made static, cleaned up
// -----------------------------------------------------------------------------

static int P_DivlineSide (const fixed_t x, const fixed_t y, const divline_t *node)
{
    fixed_t left, right;

    return
        !node->dx ? x == node->x ? 2 : x <= node->x ? node->dy > 0 : node->dy < 0 :
        // [JN] Fix https://doomwiki.org/wiki/Sleeping_shotgun_guy_in_MAP02_(Doom_II)
        !node->dy ? (singleplayer && !strict_mode && !vanillaparm ? y : x)  
                      == node->y ? 2 : y <= node->y ? node->dx < 0 : node->dx > 0 :
        (right = ((y - node->y) >> FRACBITS) * (node->dx >> FRACBITS)) <
        (left  = ((x - node->x) >> FRACBITS) * (node->dy >> FRACBITS)) ? 0 :
        right == left ? 2 : 1;
}

// -----------------------------------------------------------------------------
// P_InterceptVector2
// Returns the fractional intercept point along the first divline.
// This is only called by the addthings and addlines traversers.
// -----------------------------------------------------------------------------

static const fixed_t P_InterceptVector2 (const divline_t *v2, const divline_t *v1)
{
    fixed_t	frac, num, den;

    den = FixedMul (v1->dy>>8,v2->dx) - FixedMul(v1->dx>>8,v2->dy);

    if (den == 0)
    {
        return 0;
    }
    
    num = FixedMul ( (v1->x - v2->x)>>8 ,v1->dy) + 
    FixedMul ( (v2->y - v1->y)>>8 , v1->dx);
    frac = FixedDiv (num , den);

    return frac;
}

// -----------------------------------------------------------------------------
// P_CrossSubsector
// Returns true if strace crosses the given subsector successfully.
// -----------------------------------------------------------------------------

static const boolean P_CrossSubsector (const int num)
{
    seg_t        *seg;
    line_t       *line;
    int           s1, s2, count;
    subsector_t  *sub;
    sector_t     *front, *back;
    fixed_t       opentop, openbottom;
    divline_t     divl;
    vertex_t     *v1, *v2;
    fixed_t       frac, slope;

#ifdef RANGECHECK
    if (num>=numsubsectors)
    {
        I_Error (english_language ? "P_CrossSubsector: ss %i with numss = %i" :
                                    "P_CrossSubsector: ss %i с numss = %i",
                                    num, numsubsectors);
    }
#endif

    sub = &subsectors[num];

    // check lines
    count = sub->numlines;
    seg = &segs[sub->firstline];

    for ( ; count ; seg++, count--)
    {
        line = seg->linedef;

        // allready checked other side?
        if (line->validcount == validcount)
        {
            continue;
        }

        line->validcount = validcount;

        v1 = line->v1;
        v2 = line->v2;
        s1 = P_DivlineSide (v1->x,v1->y, &strace);
        s2 = P_DivlineSide (v2->x, v2->y, &strace);

        // line isn't crossed?
        if (s1 == s2)
        {
            continue;
        }

        divl.x = v1->x;
        divl.y = v1->y;
        divl.dx = v2->x - v1->x;
        divl.dy = v2->y - v1->y;
        s1 = P_DivlineSide (strace.x, strace.y, &divl);
        s2 = P_DivlineSide (t2x, t2y, &divl);

        // line isn't crossed?
        if (s1 == s2)
        {
            continue;
        }

        // Backsector may be NULL if this is an "impassible
        // glass" hack line.
        if (line->backsector == NULL)
        {
            return false;
        }

        // stop because it is not two sided anyway
        // might do this after updating validcount?
        if (!(line->flags & ML_TWOSIDED))
        {
            return false;
        }

        // crosses a two sided line
        front = seg->frontsector;
        back = seg->backsector;

        // no wall to block sight with?
        if (front->floorheight == back->floorheight
        &&  front->ceilingheight == back->ceilingheight)
        {
            continue;
        }

        // possible occluder
        // because of ceiling height differences
        if (front->ceilingheight < back->ceilingheight)
        {
            opentop = front->ceilingheight;
        }
        else
        {
            opentop = back->ceilingheight;
        }

        // because of ceiling height differences
        if (front->floorheight > back->floorheight)
        {
            openbottom = front->floorheight;
        }
        else
        {
            openbottom = back->floorheight;
        }

        // quick test for totally closed doors
        if (openbottom >= opentop)
        {
            return false;  // stop
        }

        frac = P_InterceptVector2 (&strace, &divl);

        if (front->floorheight != back->floorheight)
        {
            slope = FixedDiv (openbottom - sightzstart , frac);
            if (slope > bottomslope)
            {
                bottomslope = slope;
            }
        }

        if (front->ceilingheight != back->ceilingheight)
        {
            slope = FixedDiv (opentop - sightzstart , frac);
            if (slope < topslope)
            {
                topslope = slope;
            }
        }

        if (topslope <= bottomslope)
        {
            return false;  // stop
        }
    }

    // passed the subsector ok
    return true;		
}

// -----------------------------------------------------------------------------
// P_CrossBSPNode
// Returns true if strace crosses the given node successfully.
// -----------------------------------------------------------------------------

static const boolean P_CrossBSPNode (const int bspnum)
{
    node_t *bsp;
    int     side;

    if (bspnum & NF_SUBSECTOR)
    {
        return P_CrossSubsector (bspnum == -1 ? 0 : bspnum&(~NF_SUBSECTOR));
    }

    bsp = &nodes[bspnum];

    // decide which side the start point is on
    side = P_DivlineSide (strace.x, strace.y, (divline_t *)bsp);
    if (side == 2)
    {
        side = 0;  // an "on" should cross both sides
    }

    // cross the starting side
    if (!P_CrossBSPNode(bsp->children[side]))
    {
        return false;
    }

    // the partition plane is crossed here
    if (side == P_DivlineSide (t2x, t2y,(divline_t *)bsp))
    {
        // the line doesn't touch the other side
        return true;
    }

    // cross the ending side		
    return P_CrossBSPNode (bsp->children[side^1]);
}

// -----------------------------------------------------------------------------
// P_CheckSight
// Returns true if a straight line between t1 and t2 is unobstructed.
// Uses REJECT.
// -----------------------------------------------------------------------------

const boolean P_CheckSight (const mobj_t *t1, const mobj_t *t2)
{
    // Determine subsector entries in REJECT table.
    const int s1 = (t1->subsector->sector - sectors);
    const int s2 = (t2->subsector->sector - sectors);
    const int pnum = s1*numsectors + s2;

    // Check for trivial rejection in REJECT table.
    if (rejectmatrix[pnum>>3] & (1 << (pnum&7)))
    {
        sightcounts[0]++;

        // can't possibly be connected
        return false;	
    }

    // An unobstructed LOS is possible.
    // Now look from eyes of t1 to any part of t2.
    sightcounts[1]++;

    validcount++;

    sightzstart = t1->z + t1->height - (t1->height>>2);
    topslope = (t2->z+t2->height) - sightzstart;
    bottomslope = (t2->z) - sightzstart;

    strace.x = t1->x;
    strace.y = t1->y;
    t2x = t2->x;
    t2y = t2->y;
    strace.dx = t2->x - t1->x;
    strace.dy = t2->y - t1->y;

    // the head node is the last node output
    return P_CrossBSPNode (numnodes-1);	
}

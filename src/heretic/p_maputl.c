//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
// P_maputl.c


#include <stdlib.h>
#include <math.h>
#include "i_system.h"  // [crispy] I_Realloc()
#include "hr_local.h"
#include "m_bbox.h"
#include "p_local.h"
#include "jn.h"


/*
================================================================================
=
= P_AproxDistance
=
= Gives an estimation of distance (not exact)
=
================================================================================
*/

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy)
{
    dx = abs(dx);
    dy = abs(dy);
    if (dx < dy)
        return dx + dy - (dx >> 1);
    return dx + dy - (dy >> 1);
}

/*
================================================================================
=
= P_ApproxDistanceZ
=
= [JN] Gives an estimation of distance using three axises.
= Adapted from EDGE, converted to fixed point math.
=
================================================================================
*/

const int64_t P_ApproxDistanceZ (int64_t dx, int64_t dy, int64_t dz)
{
	int64_t dxy;

	dx = llabs(dx);
	dy = llabs(dy);
	dz = llabs(dz);

	dxy = (dy > dx) ? dy + dx/2 : dx + dy/2;

	return (dz > dxy) ? dz + dxy/2 : dxy + dz/2;
}

/*
================================================================================
=
= P_PointOnLineSide
=
= Returns 0 or 1
= [JN] killough 5/3/98: reformatted, cleaned up.
=
================================================================================
*/

int P_PointOnLineSide (fixed_t x, fixed_t y, const line_t *line)
{
    return
    !line->dx ? x <= line->v1->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->v1->y ? line->dx < 0 : line->dx > 0 :
    FixedMul(y-line->v1->y, line->dx>>FRACBITS) >=
    FixedMul(line->dy>>FRACBITS, x-line->v1->x);
}

/*
================================================================================
=
= P_BoxOnLineSide
=
= Considers the line to be infinite.
= Returns side 0 or 1, -1 if box crosses the line.
=
= [JN] killough 5/3/98: reformatted, cleaned up
=
================================================================================
*/

int P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld)
{
    switch (ld->slopetype)
    {
        int p;

        default: // shut up compiler warnings -- killough
        case ST_HORIZONTAL:
        return
        (tmbox[BOXBOTTOM] > ld->v1->y) == (p = tmbox[BOXTOP] > ld->v1->y) ?
        p ^ (ld->dx < 0) : -1;

        case ST_VERTICAL:
        return
        (tmbox[BOXLEFT] < ld->v1->x) == (p = tmbox[BOXRIGHT] < ld->v1->x) ?
        p ^ (ld->dy < 0) : -1;

        case ST_POSITIVE:
        return
        P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) ==
        (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;

        case ST_NEGATIVE:
        return
        (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) ==
        (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
}

/*
================================================================================
=
= P_PointOnDivlineSide
=
= Returns 0 or 1
=
= [JN] killough 5/3/98: reformatted, cleaned up
=
================================================================================
*/

int P_PointOnDivlineSide (fixed_t x, fixed_t y, const divline_t *line)
{
    return
    !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
    (line->dy^line->dx^(x -= line->x)^(y -= line->y)) < 0 ? (line->dy^x) < 0 :
    FixedMul(y>>8, line->dx>>8) >= FixedMul(line->dy>>8, x>>8);
}

/*
================================================================================
=
= P_MakeDivline
=
================================================================================
*/

void P_MakeDivline (line_t *li, divline_t *dl)
{
    dl->x = li->v1->x;
    dl->y = li->v1->y;
    dl->dx = li->dx;
    dl->dy = li->dy;
}

/*
================================================================================
=
= P_InterceptVector
=
= Returns the fractional intercept point along the first divline.
= This is only called by the addthings and addlines traversers.
=
= [JN] killough 5/3/98: reformatted, cleaned up
=
================================================================================
*/

fixed_t P_InterceptVector(divline_t *v2, divline_t *v1)
{
    if (singleplayer && !strict_mode)
    {
        // [JN] cph - no precision/overflow problems
        int64_t den = (int64_t)v1->dy * v2->dx - (int64_t)v1->dx * v2->dy;
        den >>= 16;
        
        if (!den)
        {
            return 0;
        }

        return (fixed_t)(((int64_t)(v1->x - v2->x) * v1->dy
                        - (int64_t)(v1->y - v2->y) * v1->dx) / den);
    }
    else
    {
        // [JN] Original demo-safe code.
        fixed_t den = FixedMul(v1->dy >> 8, v2->dx) - FixedMul(v1->dx >> 8, v2->dy);

        return den ? FixedDiv((FixedMul((v1->x-v2->x) >> 8, v1->dy)
                   + FixedMul((v2->y-v1->y) >> 8, v1->dx)), den) : 0;
    }
}

/*
================================================================================
=
= P_LineOpening
=
= Sets opentop and openbottom to the window through a two sided line.
= OPTIMIZE: keep this precalculated.
================================================================================
*/

fixed_t opentop, openbottom, openrange;
fixed_t lowfloor;

void P_LineOpening(line_t *linedef)
{
    sector_t *front, *back;

    if (linedef->sidenum[1] == NO_INDEX)
    {
        // single sided line
        openrange = 0;
        return;
    }

    front = linedef->frontsector;
    back = linedef->backsector;

    if (front->ceilingheight < back->ceilingheight)
    {
        opentop = front->ceilingheight;
    }
    else
    {
        opentop = back->ceilingheight;
    }

    if (front->floorheight > back->floorheight)
    {
        openbottom = front->floorheight;
        lowfloor = back->floorheight;
    }
    else
    {
        openbottom = back->floorheight;
        lowfloor = front->floorheight;
    }

    openrange = opentop - openbottom;
}


/*
================================================================================

						THING POSITION SETTING

================================================================================
*/

/*
================================================================================
=
= P_UnsetThingPosition 
=
= Unlinks a thing from block map and sectors.
=
================================================================================
*/

void P_UnsetThingPosition(mobj_t *thing)
{
    int blockx, blocky;

    if (!(thing->flags & MF_NOSECTOR))
    {
        // inert things don't need to be in blockmap
        // unlink from subsector
        if (thing->snext)
        {
            thing->snext->sprev = thing->sprev;
        }
        if (thing->sprev)
        {
            thing->sprev->snext = thing->snext;
        }
        else
        {
            thing->subsector->sector->thinglist = thing->snext;
        }
    }

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        // unlink from block map
        if (thing->bnext)
        {
            thing->bnext->bprev = thing->bprev;
        }
        if (thing->bprev)
        {
            thing->bprev->bnext = thing->bnext;
        }
        else
        {
            blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
            blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;
            if (blockx >= 0 && blockx < bmapwidth
            &&  blocky >= 0 && blocky < bmapheight)
            {
                blocklinks[blocky * bmapwidth + blockx] = thing->bnext;
            }
        }
    }
}

/*
================================================================================
=
= P_SetThingPosition 
=
= Links a thing into both a block and a subsector based on it's x y.
= Sets thing->subsector properly.
=
================================================================================
*/

void P_SetThingPosition (mobj_t *thing)
{
    int           blockx, blocky;
    mobj_t      **link;
    sector_t     *sec;
    subsector_t  *ss;

    // link into subsector
    ss = R_PointInSubsector(thing->x, thing->y);
    thing->subsector = ss;

    if (!(thing->flags & MF_NOSECTOR))
    {
        // invisible things don't go into the sector links
        sec = ss->sector;

        thing->sprev = NULL;
        thing->snext = sec->thinglist;

        if (sec->thinglist)
        {
            sec->thinglist->sprev = thing;
        }
        sec->thinglist = thing;
    }

    // link into blockmap
    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap            
        blockx = (thing->x - bmaporgx) >> MAPBLOCKSHIFT;
        blocky = (thing->y - bmaporgy) >> MAPBLOCKSHIFT;
        if (blockx >= 0 && blockx < bmapwidth && blocky >= 0
        &&  blocky < bmapheight)
        {
            link = &blocklinks[blocky * bmapwidth + blockx];
            thing->bprev = NULL;
            thing->bnext = *link;
            if (*link)
            {
                (*link)->bprev = thing;
            }
            *link = thing;
        }
        else
        {
            // thing is off the map
            thing->bnext = thing->bprev = NULL;
        }
    }
}



/*
================================================================================

						BLOCK MAP ITERATORS

For each line/thing in the given mapblock, call the passed function.
If the function returns false, exit with false without checking anything else.

================================================================================
*/

/*
================================================================================
=
= P_BlockLinesIterator
=
= The validcount flags are used to avoid checking lines
= that are marked in multiple mapblocks, so increment validcount before
= the first call to P_BlockLinesIterator, then make one or more calls to it.
=
= [JN] killough 5/3/98: reformatted, cleaned up
=
================================================================================
*/

boolean P_BlockLinesIterator (int x, int y, boolean(*func) (line_t *))
{
    int offset;
    int32_t *list;  // [crispy] BLOCKMAP limit

    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
    {
        return true;
    }

    offset = y*bmapwidth+x;
    offset = *(blockmap+offset);
    list = blockmaplump+offset;

    for ( ; *list != -1 ; list++)
    {
        line_t *ld = &lines[*list];

        if (ld->validcount == validcount)
        {
            continue;  // line has already been checked
        }

        ld->validcount = validcount;

        if (!func(ld))
        {
            return false;
        }
    }

    return true;  // everything was checked
}

/*
================================================================================
=
= P_BlockThingsIterator
=
================================================================================
*/

boolean P_BlockThingsIterator (int x, int y, boolean(*func) (mobj_t *))
{
    mobj_t *mobj;

    if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
    {
        return true;
    }

    for (mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext)
        if (!func(mobj))
            return false;

    // [JN] Do not apply following BLOCKMAP fix for explosion radius damage.
    // Otherwise, explosion damage will be multiplied on ammount of BLOCKMAP 
    // blocks object placed in.
    // Also do not apply for crusher damage, same reason as above.
    if (func == PIT_RadiusAttack || func == PIT_ChangeSector)
    {
        return true;
    }

    // [JN] Blockmap bug fix - add other mobjs from surrounding blocks that overlap this one.
    // The fix is written by Terry Hearst, thank you very much!
    // Fixes: http://doom2.net/doom2/research/things.html
    if (singleplayer && !strict_mode && !vanillaparm && improved_collision)
    {
        // Unwrapped for least number of bounding box checks
        // (-1, -1)
        if (x > 0 && y > 0)
        {
            for (mobj = blocklinks[(y-1)*bmapwidth+(x-1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x + mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                int yy = (mobj->y + mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (xx == x && yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (0, -1)
        if (y > 0)
        {
            for (mobj = blocklinks[(y-1)*bmapwidth+x] ; mobj ; mobj = mobj->bnext)
            {
                int yy = (mobj->y + mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (1, -1)
        if (x < (bmapwidth-1) && y > 0)
        {
            for (mobj = blocklinks[(y-1)*bmapwidth+(x+1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x - mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                int yy = (mobj->y + mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (xx == x && yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (1, 0)
        if (x < (bmapwidth-1))
        {
            for (mobj = blocklinks[y*bmapwidth+(x+1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x - mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                if (xx == x)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (1, 1)
        if (x < (bmapwidth-1) && y < (bmapheight-1))
        {
            for (mobj = blocklinks[(y+1)*bmapwidth+(x+1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x - mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                int yy = (mobj->y - mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (xx == x && yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (0, 1)
        if (y < (bmapheight-1))
        {
            for (mobj = blocklinks[(y+1)*bmapwidth+x] ; mobj ; mobj = mobj->bnext)
            {
                int yy = (mobj->y - mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (-1, 1)
        if (x > 0 && y < (bmapheight-1))
        {
            for (mobj = blocklinks[(y+1)*bmapwidth+(x-1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x + mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                int yy = (mobj->y - mobj->radius - bmaporgy)>>MAPBLOCKSHIFT;
                if (xx == x && yy == y)
                    if (!func( mobj ))
                        return false;
            }
        }
        // (-1, 0)
        if (x > 0)
        {
            for (mobj = blocklinks[y*bmapwidth+(x-1)] ; mobj ; mobj = mobj->bnext)
            {
                int xx = (mobj->x + mobj->radius - bmaporgx)>>MAPBLOCKSHIFT;
                if (xx == x)
                    if (!func( mobj ))
                        return false;
            }
        }
    }

    return true;
}


/*
================================================================================

					INTERCEPT ROUTINES

================================================================================
*/

// [crispy] remove INTERCEPTS limit
intercept_t *intercepts, *intercept_p; 

// [JN] Slightly extended - report which funcion is triggered limit:
//  0: PIT_AddLineIntercepts
//  1: PIT_AddThingIntercepts
//  2: P_SightBlockLinesIterator

void check_intercept (const int func)
{
	static size_t num_intercepts;
	const size_t offset = intercept_p - intercepts;

	if (offset >= num_intercepts)
	{
		num_intercepts = num_intercepts ? num_intercepts * 2 : MAXINTERCEPTS;
		intercepts = I_Realloc(intercepts, sizeof(*intercepts) * num_intercepts);
		intercept_p = intercepts + offset;

		if (num_intercepts == 2 * MAXINTERCEPTS)
		{
            printf(func == 0 ? "PIT_AddLineIntercepts: " :
                   func == 1 ? "PIT_AddThingIntercepts: " :
                               "P_SightBlockLinesIterator: ");
            printf(english_language ? "Hit INTERCEPTS limit!\n" :
                                      "превышен лимит INTERCEPTS!\n");
		}
	}
}

divline_t trace;
boolean earlyout;
int ptflags;

/*
================================================================================
=
= PIT_AddLineIntercepts
=
= Looks for lines in the given block that intercept the given trace
= to add to the intercepts list.
= A line is crossed if its endpoints are on opposite sides of the trace
= Returns true if earlyout and a solid line hit.
=
================================================================================
*/

boolean PIT_AddLineIntercepts (line_t *ld)
{
    int        s1, s2;
    fixed_t    frac;
    divline_t  dl;

    // avoid precision problems with two routines
    if (trace.dx >  FRACUNIT * 16
    ||  trace.dy >  FRACUNIT * 16
    ||  trace.dx < -FRACUNIT * 16
    ||  trace.dy < -FRACUNIT * 16)
    {
        s1 = P_PointOnDivlineSide(ld->v1->x, ld->v1->y, &trace);
        s2 = P_PointOnDivlineSide(ld->v2->x, ld->v2->y, &trace);
    }
    else
    {
        s1 = P_PointOnLineSide(trace.x, trace.y, ld);
        s2 = P_PointOnLineSide(trace.x + trace.dx, trace.y + trace.dy, ld);
    }

    if (s1 == s2)
    {
        // line isn't crossed
        return true;
    }

    // hit the line
    P_MakeDivline(ld, &dl);
    frac = P_InterceptVector(&trace, &dl);

    if (frac < 0)
    {
        // behind source
        return true;
    }

    // try to early out the check
    if (earlyout && frac < FRACUNIT && !ld->backsector)
    {
        // stop checking
        return false;
    }

    check_intercept(0); // [crispy] remove INTERCEPTS limit
    intercept_p->frac = frac;
    intercept_p->isaline = true;
    intercept_p->d.line = ld;
    intercept_p++;

    // continue
    return true;
}

/*
================================================================================
=
= PIT_AddThingIntercepts
=
================================================================================
*/

boolean PIT_AddThingIntercepts (mobj_t *thing)
{
    int        s1, s2;
    fixed_t    x1, y1, x2, y2;
    boolean    tracepositive;
    divline_t  dl;
    fixed_t    frac;

    tracepositive = (trace.dx ^ trace.dy) > 0;

    // check a corner to corner crossection for hit
    if (tracepositive)
    {
        x1 = thing->x - thing->radius;
        y1 = thing->y + thing->radius;
        x2 = thing->x + thing->radius;
        y2 = thing->y - thing->radius;
    }
    else
    {
        x1 = thing->x - thing->radius;
        y1 = thing->y - thing->radius;
        x2 = thing->x + thing->radius;
        y2 = thing->y + thing->radius;
    }

    s1 = P_PointOnDivlineSide(x1, y1, &trace);
    s2 = P_PointOnDivlineSide(x2, y2, &trace);

    if (s1 == s2)
    {
        // line isn't crossed
        return true;
    }

    dl.x = x1;
    dl.y = y1;
    dl.dx = x2 - x1;
    dl.dy = y2 - y1;

    frac = P_InterceptVector(&trace, &dl);

    if (frac < 0)
    {
        // behind source
        return true;
    }

    check_intercept(1); // [crispy] remove INTERCEPTS limit
    intercept_p->frac = frac;
    intercept_p->isaline = false;
    intercept_p->d.thing = thing;
    intercept_p++;

    // keep going
    return true;
}

/*
================================================================================
=
= P_TraverseIntercepts
=
= Returns true if the traverser function returns true for all lines.
=
= [JN] killough 5/3/98: reformatted, cleaned up
=
================================================================================
*/

boolean P_TraverseIntercepts (traverser_t func, fixed_t maxfrac)
{
    intercept_t *in = NULL;
    int count = intercept_p - intercepts;

    while (count--)
    {
        fixed_t dist = INT_MAX;
        intercept_t *scan;

        for (scan = intercepts; scan < intercept_p; scan++)
            if (scan->frac < dist)
                dist = (in=scan)->frac;

        if (dist > maxfrac)
            return true;    // checked everything in range

        if (!func(in))
            return false;   // don't bother going farther

        in->frac = INT_MAX;
    }
    return true;            // everything was traversed
}

/*
================================================================================
=
= P_PathTraverse
=
= Traces a line from x1,y1 to x2,y2, calling the traverser function for each
= Returns true if the traverser function returns true for all lines.
================================================================================
*/

boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int flags, boolean(*trav) (intercept_t *))
{
    fixed_t xt1, yt1, xt2, yt2;
    fixed_t xstep, ystep;
    fixed_t partial;
    fixed_t xintercept, yintercept;
    int mapx, mapy, mapxstep, mapystep;
    int count;

    earlyout = (flags & PT_EARLYOUT) != 0;

    validcount++;
    intercept_p = intercepts;

    if (((x1 - bmaporgx) & (MAPBLOCKSIZE - 1)) == 0)
    {
        // don't side exactly on a line
        x1 += FRACUNIT;
    }
    if (((y1 - bmaporgy) & (MAPBLOCKSIZE - 1)) == 0)
    {
        // don't side exactly on a line
        y1 += FRACUNIT;
    }

    trace.x = x1;
    trace.y = y1;
    trace.dx = x2 - x1;
    trace.dy = y2 - y1;

    x1 -= bmaporgx;
    y1 -= bmaporgy;
    xt1 = x1 >> MAPBLOCKSHIFT;
    yt1 = y1 >> MAPBLOCKSHIFT;

    x2 -= bmaporgx;
    y2 -= bmaporgy;
    xt2 = x2 >> MAPBLOCKSHIFT;
    yt2 = y2 >> MAPBLOCKSHIFT;

    if (xt2 > xt1)
    {
        mapxstep = 1;
        partial = FRACUNIT - ((x1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        ystep = FixedDiv(y2 - y1, abs(x2 - x1));
    }
    else if (xt2 < xt1)
    {
        mapxstep = -1;
        partial = (x1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        ystep = FixedDiv(y2 - y1, abs(x2 - x1));
    }
    else
    {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256 * FRACUNIT;
    }

    yintercept = (y1 >> MAPBTOFRAC) + FixedMul(partial, ystep);

    if (yt2 > yt1)
    {
        mapystep = 1;
        partial = FRACUNIT - ((y1 >> MAPBTOFRAC) & (FRACUNIT - 1));
        xstep = FixedDiv(x2 - x1, abs(y2 - y1));
    }
    else if (yt2 < yt1)
    {
        mapystep = -1;
        partial = (y1 >> MAPBTOFRAC) & (FRACUNIT - 1);
        xstep = FixedDiv(x2 - x1, abs(y2 - y1));
    }
    else
    {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256 * FRACUNIT;
    }

    xintercept = (x1 >> MAPBTOFRAC) + FixedMul(partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error from skipping the break.

    mapx = xt1;
    mapy = yt1;

    for (count = 0; count < 64; count++)
    {
        if (flags & PT_ADDLINES)
        {
            if (!P_BlockLinesIterator(mapx, mapy, PIT_AddLineIntercepts))
            {
                // early out
                return false;
            }
        }
        if (flags & PT_ADDTHINGS)
        {
            if (!P_BlockThingsIterator(mapx, mapy, PIT_AddThingIntercepts))
            {
                // early out
                return false;
            }
        }

        if (mapx == xt2 && mapy == yt2)
        {
            break;
        }

        if ((yintercept >> FRACBITS) == mapy)
        {
            yintercept += ystep;
            mapx += mapxstep;
        }
        else if ((xintercept >> FRACBITS) == mapx)
        {
            xintercept += xstep;
            mapy += mapystep;
        }
    }

    // go through the sorted list
    return P_TraverseIntercepts(trav, FRACUNIT);
}

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard, Andrey Budko
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
// DESCRIPTION:
//	Movement, collision handling.
//	Shooting and aiming.
//


#include "deh_misc.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "m_random.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "jn.h"


static fixed_t  tmbbox[4];
static mobj_t  *tmthing;
static int      tmflags;
static fixed_t	tmx, tmy;

// [JN] Rand Phares:
static int pe_x, pe_y;     // Pain Elemental position for Lost Soul checks
static int ls_x, ls_y;     // Lost Soul position for Lost Soul checks

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean floatok;
fixed_t tmfloorz;
fixed_t tmceilingz;
static fixed_t tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t *ceilingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
// [JN] 1/11/98 killough: removed limit on special lines crossed
line_t     **spechit;
static int   spechit_max;
int          numspechit;

#define MAXSPECIALCROSS             20
#define MAXSPECIALCROSS_ORIGINAL    8
#define DEFAULT_SPECHIT_MAGIC       0x01C09C98

static void SpechitOverrun (line_t *ld);


// =============================================================================
// TELEPORT MOVE
// =============================================================================

// -----------------------------------------------------------------------------
// PIT_StompThing
// -----------------------------------------------------------------------------

static boolean PIT_StompThing (mobj_t *thing)
{
    fixed_t	blockdist;

    if (!(thing->flags & MF_SHOOTABLE))
    {
        return true;
    }

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;
    }

    // don't clip against self
    if (thing == tmthing)
    {
        return true;
    }

    // monsters don't stomp things except on boss level
    if (!tmthing->player && gamemap != 30)
    {
        return false;	
    }

    P_DamageMobj (thing, tmthing, tmthing, 10000);

    return true;
}

// -----------------------------------------------------------------------------
// P_TeleportMove
// -----------------------------------------------------------------------------

const boolean P_TeleportMove (mobj_t *thing, const fixed_t x, const fixed_t y)
{
    int          xl, xh;
    int          yl, yh;
    int          bx, by;
    subsector_t *newsubsec;

    // kill anything occupying the position
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;

    // The base floor/ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    // stomp on any things contacted
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;        

    for (bx = xl ; bx <= xh ; bx++)
    {
        for (by = yl ; by <= yh ; by++)
        {
            if (!P_BlockThingsIterator(bx, by, PIT_StompThing))
            {
                return false;
            }
        }
    }

    // The move is ok, so link the thing into its new position
    P_UnsetThingPosition (thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;	
    thing->x = x;
    thing->y = y;

    // [AM] Don't interpolate mobjs that pass
    //      through teleporters
    thing->interp = false;

    P_SetThingPosition (thing);

    return true;
}


// =============================================================================
// MOVEMENT ITERATOR FUNCTIONS
// =============================================================================

// -----------------------------------------------------------------------------
// PIT_CrossLine
// Checks to see if a PE->LS trajectory line crosses a blocking
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//
//
// killough 11/98: reformatted
//
// [BH] Allow pain elementals to shoot lost souls through 2-sided walls with an
// ML_BLOCKMONSTERS flag. This is a compromise between BOOM and Vanilla DOOM
// behaviors, and allows pain elementals at the end of REQUIEM.WAD's MAP04 to do
// their thing.
// -----------------------------------------------------------------------------

static boolean PIT_CrossLine(line_t *ld)
{
    return (!((ld->flags ^ ML_TWOSIDED) & (ML_TWOSIDED | ML_BLOCKING/* | ML_BLOCKMONSTERS*/))
        || tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT] || tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT]
        || tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]
        || P_PointOnLineSide(pe_x, pe_y, ld) == P_PointOnLineSide(ls_x, ls_y, ld));
}

// -----------------------------------------------------------------------------
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
// -----------------------------------------------------------------------------

static boolean PIT_CheckLine (line_t *ld)
{
    if (tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]
    ||  tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]
    ||  tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM]
    ||  tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    {
        return true;
    }

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
    {
        return true;
    }

    // A line has been hit
    //
    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    if (!ld->backsector)
    {
        // one sided line
        return false;
    }

    if (!(tmthing->flags & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)
        {
            // explicitly blocking everything
            return false;
        }

        if (!tmthing->player && ld->flags & ML_BLOCKMONSTERS)
        {
            // block monsters only
            return false;
        }
    }

    // set openrange, opentop, openbottom
    P_LineOpening(ld);	

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
    }

    if (openbottom > tmfloorz)
    {
        tmfloorz = openbottom;	
    }

    if (lowfloor < tmdropoffz)
    {
        tmdropoffz = lowfloor;
    }

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        // [crispy] remove SPECHIT limit
        if (numspechit >= spechit_max)
        {
            spechit_max = spechit_max ? spechit_max * 2 : MAXSPECIALCROSS;
            spechit = I_Realloc(spechit, sizeof(*spechit) * spechit_max);
        }

        spechit[numspechit] = ld;
        numspechit++;

        // fraggle: spechits overrun emulation code from prboom-plus
        if (numspechit > MAXSPECIALCROSS_ORIGINAL)
        {
            // [crispy] print a warning
            if (numspechit == MAXSPECIALCROSS_ORIGINAL + 1)
            {
                printf("PIT_CheckLine: Triggered SPECHITS overflow!\n");
            }
            SpechitOverrun(ld);
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// PIT_CheckThing
// -----------------------------------------------------------------------------

static boolean PIT_CheckThing (mobj_t *thing)
{
    fixed_t blockdist;
    fixed_t newdist = P_AproxDistance(thing->x - tmx, thing->y - tmy);
    fixed_t olddist = P_AproxDistance(thing->x - tmthing->x, thing->y - tmthing->y);
    boolean solid;
    boolean unblocking = false;
    int     damage;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
    {
        return true;
    }

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;	
    }

    // don't clip against self
    if (thing == tmthing)
    {
        return true;
    }

    // check for skulls slamming into things
    if (tmthing->flags & MF_SKULLFLY)
    {
        // [crispy] check if attacking skull flies over player
        if (singleplayer && over_under && thing->player && !strict_mode && !vanillaparm)
        {
            if (tmthing->z > thing->z + thing->height)
            {
            return true;
            }
        }

        damage = ((P_Random()%8)+1)*tmthing->info->damage;

        P_DamageMobj (thing, tmthing, tmthing, damage);

        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = tmthing->momy = tmthing->momz = 0;

        P_SetMobjState(tmthing, tmthing->info->spawnstate);

        return false;		// stop moving
    }
    
    // [JN] Torque: make sliding corpses passable
    if (singleplayer && torque && tmthing->intflags & MIF_FALLING && !strict_mode && !vanillaparm)
    {
        return true;
    }

    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE)
    {
        // see if it went over / under
        if (tmthing->z > thing->z + thing->height)
        {
            // overhead
            return true;
        }
        if (tmthing->z+tmthing->height < thing->z)
        {
            // underneath
            return true;
        }

        if (tmthing->target && (tmthing->target->type == thing->type
        || (tmthing->target->type == MT_KNIGHT && thing->type == MT_BRUISER)
        || (tmthing->target->type == MT_BRUISER && thing->type == MT_KNIGHT)))
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
            {
                return true;
            }

            // sdh: Add deh_species_infighting here.  We can override the
            // "monsters of the same species cant hurt each other" behavior
            // through dehacked patches

            if (thing->type != MT_PLAYER && !deh_species_infighting)
            {
                // Explode, but do no damage.
                // Let players missile other players.
            return false;
            }
        }

        if (! (thing->flags & MF_SHOOTABLE) )
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);	
        }

        // damage / explode
        damage = ((P_Random()%8)+1)*tmthing->info->damage;
        P_DamageMobj (thing, tmthing, tmthing->target, damage);

        // don't traverse any more
        return false;				
    }

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = (thing->flags & MF_SOLID) != 0;

        if (tmflags&MF_PICKUP)
        {
            // can remove thing
            P_TouchSpecialThing (thing, tmthing);
        }

        return !solid;
    }

    if (singleplayer && !strict_mode && !vanillaparm)
    {
        if (over_under)
        {
            // [crispy] a solid hanging body will allow sufficiently small things underneath it
            if ((thing->flags & (MF_SOLID | MF_SPAWNCEILING)) == (MF_SOLID | MF_SPAWNCEILING) 
            && tmthing->z + tmthing->height <= thing->z)
            {
                tmceilingz = thing->z;
                return true;
            }

            // [crispy] allow players to walk over/under shootable objects
            if (tmthing->player && thing->flags & MF_SHOOTABLE)
            {
                if (tmthing->z >= thing->z + thing->height)
                {
                    // player walks over object
                    tmfloorz = thing->z + thing->height;
                    thing->ceilingz = tmthing->z;
                    return true;
                }
                else
                if (tmthing->z + tmthing->height <= thing->z)
                {
                    // player walks underneath object
                    tmceilingz = thing->z;
                    thing->floorz = tmthing->z + tmthing->height;
                    return true;
                }
            }
        }
        // [crispy] check if things are stuck and allow them to move further apart
        // taken from doomretro/src/p_map.c:319-332
        if (improved_collision && !thing->player 
        && (thing->flags & MF_SHOOTABLE && thing->type != MT_BARREL))
        {
            if (tmx == tmthing->x && tmy == tmthing->y)
            {
                unblocking = true;
            }
            else
            {
                if (newdist > olddist)
                {
                    unblocking = (tmthing->z < thing->z + thing->height &&
                                  tmthing->z + tmthing->height > thing->z);
                }
            }
        }
    }

    return !(thing->flags & MF_SOLID) || unblocking;
}

// -----------------------------------------------------------------------------
// P_CheckLineSide
//
// This routine checks for Lost Souls trying to be spawned
// across 1-sided lines, impassible lines, or "monsters can't
// cross" lines. Draw an imaginary line between the PE
// and the new Lost Soul spawn spot. If that line crosses
// a 'blocking' line, then disallow the spawn. Only search
// lines in the blocks of the blockmap where the bounding box
// of the trajectory line resides. Then check bounding box
// of the trajectory vs. the bounding box of each blocking
// line to see if the trajectory and the blocking line cross.
// Then check the PE and LS to see if they're on different
// sides of the blocking line. If so, return true, otherwise
// false.
// -----------------------------------------------------------------------------

const boolean P_CheckLineSide(mobj_t *actor, const fixed_t x, const fixed_t y)
{
    int xl, xh;
    int yl, yh;

    pe_x = actor->x;
    pe_y = actor->y;
    ls_x = x;
    ls_y = y;

    // here is the bounding box of the trajectory
    tmbbox[BOXLEFT] = MIN(pe_x, x);
    tmbbox[BOXRIGHT] = MAX(pe_x, x);
    tmbbox[BOXTOP] = MAX(pe_y, y);
    tmbbox[BOXBOTTOM] = MIN(pe_y, y);

    // determine which blocks to look in for blocking lines
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    validcount++;               // prevents checking same line twice

    for (int bx = xl; bx <= xh; bx++)
    {
        for (int by = yl; by <= yh; by++)
        {
            if (!P_BlockLinesIterator(bx, by, PIT_CrossLine))
            {
                return true;
            }
        }
    }

    return false;
}


// =============================================================================
// MOVEMENT CLIPPING
// =============================================================================

// -----------------------------------------------------------------------------
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
// -----------------------------------------------------------------------------

const boolean P_CheckPosition (mobj_t *thing, const fixed_t x, const fixed_t y)
{
    int xl, xh;
    int yl, yh;
    int bx, by;
    subsector_t *newsubsec;

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;

    // The base floor / ceiling is from the subsector that contains the point.
    // Any contacted lines the step closer together will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
    {
        return true;
    }
    
    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for (bx = xl ; bx <= xh ; bx++)
    {
        for (by = yl ; by <= yh ; by++)
        {
            if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
            {
                return false;
            }
        }
    }

    // check lines

    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx = xl ; bx <= xh ; bx++)
    {
        for (by = yl ; by <= yh ; by++)
        {
            if (!P_BlockLinesIterator (bx, by, PIT_CheckLine))
            {
                return false;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
// -----------------------------------------------------------------------------

const boolean P_TryMove (mobj_t *thing, const fixed_t x, const fixed_t y)
{
    fixed_t	oldx, oldy;

    floatok = false;

    if (!P_CheckPosition (thing, x, y))
    {
        // solid wall or thing
        return false;
    }

    if (!(thing->flags & MF_NOCLIP))
    {
        if (tmceilingz - tmfloorz < thing->height)
        {
            // doesn't fit
            return false;
        }

        floatok = true;

        if (!(thing->flags & MF_TELEPORT)
        && tmceilingz - thing->z < thing->height)
        {
            // mobj must lower itself to fit
            return false;
        }

        if (!(thing->flags&MF_TELEPORT)
        && tmfloorz - thing->z > 24*FRACUNIT)
        {
            // too big a step up
            return false;
        }

        if (!(thing->flags&(MF_DROPOFF|MF_FLOAT))
        && tmfloorz - tmdropoffz > 24*FRACUNIT )
        {
            // don't stand over a dropoff
            return false;
        }
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;	
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

    // if any special lines were hit, do the effect
    if (! (thing->flags&(MF_TELEPORT|MF_NOCLIP)) )
    {
        while (numspechit--)
        {
            // see if the line was crossed
            line_t *ld = spechit[numspechit];
            int side = P_PointOnLineSide (thing->x, thing->y, ld);
            int oldside = P_PointOnLineSide (oldx, oldy, ld);

            if (side != oldside)
            {
                if (ld->special)
                {
                    P_CrossSpecialLine (ld-lines, oldside, thing);
                }
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// killough 9/12/98:
//
// Apply "torque" to objects hanging off of ledges, so that they
// fall off. It's not really torque, since Doom has no concept of
// rotation, but it's a convincing effect which avoids anomalies
// such as lifeless objects hanging more than halfway off of ledges,
// and allows objects to roll off of the edges of moving lifts, or
// to slide up and then back down stairs, or to fall into a ditch.
// If more than one linedef is contacted, the effects are cumulative,
// so balancing is possible.
// -----------------------------------------------------------------------------

static boolean PIT_ApplyTorque (line_t *ld)
{
    if (ld->backsector &&       // If thing touches two-sided pivot linedef
    tmbbox[BOXRIGHT]  > ld->bbox[BOXLEFT]  &&
    tmbbox[BOXLEFT]   < ld->bbox[BOXRIGHT] &&
    tmbbox[BOXTOP]    > ld->bbox[BOXBOTTOM] &&
    tmbbox[BOXBOTTOM] < ld->bbox[BOXTOP] &&
    P_BoxOnLineSide(tmbbox, ld) == -1)
    {
        mobj_t *mo = tmthing;

        fixed_t dist =                               // lever arm
        + (ld->dx >> FRACBITS) * (mo->y >> FRACBITS)
        - (ld->dy >> FRACBITS) * (mo->x >> FRACBITS) 
        - (ld->dx >> FRACBITS) * (ld->v1->y >> FRACBITS)
        + (ld->dy >> FRACBITS) * (ld->v1->x >> FRACBITS);

        if (dist < 0 ?                               // dropoff direction
        ld->frontsector->floorheight < mo->z &&
        ld->backsector->floorheight >= mo->z :
        ld->backsector->floorheight < mo->z &&
        ld->frontsector->floorheight >= mo->z)
        {
            // At this point, we know that the object straddles a two-sided
            // linedef, and that the object's center of mass is above-ground.

            fixed_t x = abs(ld->dx), y = abs(ld->dy);

            if (y > x)
            {
                fixed_t t = x;
                x = y;
                y = t;
            }

            y = finesine[(tantoangle[FixedDiv(y,x)>>DBITS] + ANG90) >> ANGLETOFINESHIFT];

            // Momentum is proportional to distance between the
            // object's center of mass and the pivot linedef.
            //
            // It is scaled by 2^(OVERDRIVE - gear). When gear is
            // increased, the momentum gradually decreases to 0 for
            // the same amount of pseudotorque, so that oscillations
            // are prevented, yet it has a chance to reach equilibrium.

            dist = FixedDiv(FixedMul(dist, (mo->gear < OVERDRIVE) ?
                y << -(mo->gear - OVERDRIVE) :
                y >> +(mo->gear - OVERDRIVE)), x);

            // Apply momentum away from the pivot linedef.

            x = FixedMul(ld->dy, dist);
            y = FixedMul(ld->dx, dist);

            // Avoid moving too fast all of a sudden (step into "overdrive")

            dist = FixedMul(x,x) + FixedMul(y,y);

            while (dist > FRACUNIT*4 && mo->gear < MAXGEAR)
            ++mo->gear, x >>= 1, y >>= 1, dist >>= 1;

            mo->momx -= x;
            mo->momy += y;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// killough 9/12/98
//
// Applies "torque" to objects, based on all contacted linedefs
// -----------------------------------------------------------------------------

void P_ApplyTorque (mobj_t *mo)
{
    int xl = ((tmbbox[BOXLEFT] = mo->x - mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int xh = ((tmbbox[BOXRIGHT] = mo->x + mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int yl = ((tmbbox[BOXBOTTOM] = mo->y - mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int yh = ((tmbbox[BOXTOP] = mo->y + mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int bx,by,flags = mo->intflags;  // Remember the current state, for gear-change

    tmthing = mo;
    validcount++;  // prevents checking same line twice

    for (bx = xl ; bx <= xh ; bx++)
    {
        for (by = yl ; by <= yh ; by++)
        {
            P_BlockLinesIterator(bx, by, PIT_ApplyTorque);
        }
    }

    // If any momentum, mark object as 'falling' using engine-internal flags
    if (mo->momx | mo->momy)
    {
        mo->intflags |= MIF_FALLING;
    }
    else  // Clear the engine-internal flag indicating falling object.
    {
        mo->intflags &= ~MIF_FALLING;
    }

    // If the object has been moving, step up the gear.
    // This helps reach equilibrium and avoid oscillations.
    //
    // Doom has no concept of potential energy, much less
    // of rotation, so we have to creatively simulate these 
    // systems somehow :)

    if (!((mo->intflags | flags) & MIF_FALLING))    // If not falling for a while,
        mo->gear = 0;                               // Reset it to full strength
    else if (mo->gear < MAXGEAR)                    // Else if not at max gear,
        mo->gear++;                                 // move up a gear

    // [JN] Reduce torque tics while torque is applied, don't go negative.
    if (mo->intflags & MIF_FALLING && mo->geartics > 0)
    {
        mo->geartics--;
    }
}

// -----------------------------------------------------------------------------
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz, thing->ceilingz,
// and possibly thing->z.This is called for all nearby monsters whenever
// a sector changes height. If the thing doesn't fit, the z will be set to
// the lowest value and false will be returned.
// -----------------------------------------------------------------------------

static const sector_t *movingsector;

static const boolean P_ThingHeightClip (mobj_t* thing)
{
    const boolean onfloor = (thing->z == thing->floorz);

    P_CheckPosition (thing, thing->x, thing->y);	
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;

        // [JN] Update player's view when on moving platform.
        // Idea by Brad Harding, code by Fabian Greffrath.
        // Thanks, colleagues! (02.04.2018)
        if (singleplayer && !strict_mode && !vanillaparm && thing->player
        && thing->subsector->sector == movingsector)
        {
            P_CalcHeight (thing->player, true);
        }
    }
    else
    {
        // don't adjust a floating monster unless forced to
        if (thing->z+thing->height > thing->ceilingz)
        {
            thing->z = thing->ceilingz - thing->height;
        }
    }

    if (thing->ceilingz - thing->floorz < thing->height)
    {
        return false;
    }

    return true;
}


// =============================================================================
// SLIDE MOVE
// Allows the player to slide along any angled walls.
// =============================================================================

static fixed_t  bestslidefrac, secondslidefrac;
static line_t  *bestslideline, *secondslideline;
static fixed_t  tmxmove, tmymove;
static mobj_t  *slidemo;

// -----------------------------------------------------------------------------
// P_HitSlideLine
// Adjusts the xmove / ymove so that the next move will slide along the wall.
// -----------------------------------------------------------------------------

static void P_HitSlideLine (line_t* ld)
{
    int     side;
    angle_t	lineangle, moveangle, deltaangle;
    fixed_t movelen, newlen;

    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);
    lineangle = R_PointToAngle2 (0, 0, ld->dx, ld->dy);

    if (side == 1)
    {
        lineangle += ANG180;
    }

    moveangle = R_PointToAngle2 (0, 0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
    {
        deltaangle += ANG180;
    }

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);	
    tmymove = FixedMul (newlen, finesine[lineangle]);	
}

// -----------------------------------------------------------------------------
// PTR_SlideTraverse
// -----------------------------------------------------------------------------

static boolean PTR_SlideTraverse (intercept_t *in)
{
    line_t *li;

    if (!in->isaline)
    {
        I_QuitWithError(english_language ?
                        "PTR_SlideTraverse: not a line?" :
                        "PTR_SlideTraverse: не является линией?");
    }

    li = in->d.line;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
        {
            // don't hit the back side
            return true;		
        }

        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening (li);

    if (openrange < slidemo->height)
    {
        // doesn't fit
        goto isblocking;
    }

    if (opentop - slidemo->z < slidemo->height)
    {
        // mobj is too high
        goto isblocking;
    }

    if (openbottom - slidemo->z > 24*FRACUNIT )
    {
        // too big a step up
        goto isblocking;
    }

    // this line doesn't block movement
    return true;		

    // the line does block movement,
    // see if it is closer than best so far
  isblocking:		
    if (in->frac < bestslidefrac)
    {
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;  // stop
}

// -----------------------------------------------------------------------------
// P_SlideMove
// The momx / momy move is bad, so try to slide along a wall.
// Find the first line hit, move flush to it, and slide along it.
//
// This is a kludgy mess.
// -----------------------------------------------------------------------------

void P_SlideMove (mobj_t *mo)
{
    fixed_t leadx, leady;
    fixed_t trailx, traily;
    fixed_t newx, newy;
    int     hitcount;

    slidemo = mo;
    hitcount = 0;

  retry:
    if (++hitcount == 3)
    {
        goto stairstep;  // don't loop forever
    }
    
    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx+mo->momx, leady+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx+mo->momx, traily+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);

    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
        stairstep:
        if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
        {
            P_TryMove (mo, mo->x + mo->momx, mo->y);
        }

        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;	

    if (bestslidefrac > 0)
    {
        newx = FixedMul (mo->momx, bestslidefrac);
        newy = FixedMul (mo->momy, bestslidefrac);

        if (!P_TryMove (mo, mo->x+newx, mo->y+newy))
        {
            goto stairstep;
        }
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

    if (bestslidefrac > FRACUNIT)
    {
        bestslidefrac = FRACUNIT;
    }

    if (bestslidefrac <= 0)
    {
        return;
    }

    tmxmove = FixedMul (mo->momx, bestslidefrac);
    tmymove = FixedMul (mo->momy, bestslidefrac);

    P_HitSlideLine (bestslideline);  // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove))
    {
        goto retry;
    }
}

// -----------------------------------------------------------------------------
// P_LineAttack
// -----------------------------------------------------------------------------

mobj_t        *linetarget;  // who got hit (or NULL)
static mobj_t *shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
static fixed_t shootz;	

static int la_damage;
fixed_t    attackrange;
fixed_t    aimslope;

// slopes to top and bottom of target
// killough 4/20/98: make static instead of using ones in p_sight.c

static fixed_t  topslope;
static fixed_t  bottomslope;

// -----------------------------------------------------------------------------
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
// -----------------------------------------------------------------------------

static boolean PTR_AimTraverse (intercept_t *in)
{
    mobj_t  *th;
    fixed_t  slope;
    fixed_t  thingtopslope;
    fixed_t  thingbottomslope;
    fixed_t  dist;

    if (in->isaline)
    {
        line_t *li = in->d.line;

        if (!(li->flags & ML_TWOSIDED))
        {
            // stop
            return false;
        }

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
        P_LineOpening (li);

        if (openbottom >= opentop)
        {
            // stop
            return false;
        }

        dist = FixedMul (attackrange, in->frac);

        if (li->backsector == NULL
        ||  li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > bottomslope)
            {
                bottomslope = slope;
            }
        }

        if (li->backsector == NULL
        ||  li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv (opentop - shootz , dist);
            if (slope < topslope)
            {
                topslope = slope;
            }
        }

        if (topslope <= bottomslope)
        {
            // stop
            return false;
        }

        return true;  // shot continues
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
    {
        // can't shoot self
        return true;
    }

    if (!(th->flags & MF_SHOOTABLE))
    {
        // corpse or something
        return true;
    }

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < bottomslope)
    {
        // shot over the thing
        return true;
    }

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
    {
        // shot under the thing
        return true;
    }

    // this thing can be hit!
    if (thingtopslope > topslope)
    {
        thingtopslope = topslope;
    }

    if (thingbottomslope < bottomslope)
    {
        thingbottomslope = bottomslope;
    }

    aimslope = (thingtopslope+thingbottomslope)/2;
    linetarget = th;

    return false;  // don't go any farther
}

// -----------------------------------------------------------------------------
// PTR_ShootTraverse
// -----------------------------------------------------------------------------

static boolean PTR_ShootTraverse (intercept_t* in)
{
    line_t  *li;
    mobj_t  *th;
    fixed_t  dist;
    fixed_t  x, y, z, frac;
    fixed_t  slope, thingtopslope, thingbottomslope;

    if (in->isaline)
    {
        boolean safe = false;
        li = in->d.line;

        if (li->special)
        {
            P_ShootSpecialLine (shootthing, li);
        }

        if (!(li->flags & ML_TWOSIDED))
        {
            goto hitline;
        }

        // crosses a two sided line
        P_LineOpening (li);

        dist = FixedMul (attackrange, in->frac);

        // Andrey Budko: emulation of missed back side on two-sided lines.
        // backsector can be NULL when emulating missing back side.
        if (li->backsector == NULL)
        {
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > aimslope)
            {
                goto hitline;
            }

            slope = FixedDiv (opentop - shootz , dist);
            if (slope < aimslope)
            {
                goto hitline;
            }
        }
        else
        {
            if (li->frontsector->floorheight != li->backsector->floorheight)
            {
                slope = FixedDiv (openbottom - shootz , dist);
                if (slope > aimslope)
                {
                    goto hitline;
                }
            }

            if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
            {
                slope = FixedDiv (opentop - shootz , dist);
                if (slope < aimslope)
                {
                    goto hitline;
                }
            }
        }

        // shot continues
        return true;
	
        // hit line
        hitline:
        // position a bit closer
        frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
        x = trace.x + FixedMul (trace.dx, frac);
        y = trace.y + FixedMul (trace.dy, frac);
        z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->ceilingheight)
            {
                return false;
            }

            // it's a sky hack wall
            if	(li->backsector && li->backsector->ceilingpic == skyflatnum)
            {
                // [crispy] fix bullet puffs and laser spot not appearing in outdoor areas
                if (li->backsector->ceilingheight < z)
                {
                    return false;
                }
                else
                {
                    safe = true;
                }
            }
        }

        // [crispy] check if the pullet puff's z-coordinate is below of above
        // its spawning sector's floor or ceiling, respectively, and move its
        // coordinates to the point where the trajectory hits the plane
        if (aimslope && !vanillaparm)
        {
            const int lineside = P_PointOnLineSide(x, y, li);
            int side;

            if ((side = li->sidenum[lineside]) != NO_INDEX)
            {
                const sector_t *const sector = sides[side].sector;

                if (z < sector->floorheight
                || (z > sector->ceilingheight && sector->ceilingpic != skyflatnum))
                {
                    z = BETWEEN(sector->floorheight, sector->ceilingheight, z);
                    frac = FixedDiv(z - shootz, FixedMul(aimslope, attackrange));
                    // [JN] Shift X/Y coords only in non-strict mode
                    // for preventing puff slippering to closest line.
                    if (!strict_mode)
                    {
                        x = trace.x + FixedMul (trace.dx, frac);
                        y = trace.y + FixedMul (trace.dy, frac);
                    }
                }
            }
        }

        // Spawn bullet puffs.
        P_SpawnPuffSafe (x, y, z, safe);

        // don't go any farther
        return false;	
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
    {
        // can't shoot self
        return true;
    }

    if (!(th->flags&MF_SHOOTABLE))
    {
        // corpse or something
        return true;
    }

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < aimslope)
    {
        // shot over the thing
        return true;
    }

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > aimslope)
    {
        // shot under the thing
        return true;
    }

    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv (10 * FRACUNIT, attackrange);

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    // [JN] Don't damage mobjs and don't spawn blood splats
    // upon shooting them from extended range.
    if (dist > MISSILERANGE)
    {
        return false;
    }

    // Spawn bullet puffs or blod spots,
    // depending on target type.
    if (in->d.thing->flags & MF_NOBLOOD)
    {
        P_SpawnPuff (x,y,z);
    }
    else
    {
        P_SpawnBlood (x,y,z, la_damage, th);  // [crispy] pass thing type
    }

    if (la_damage)
    {
        P_DamageMobj (th, shootthing, shootthing, la_damage);
    }

    // don't go any farther
    return false;
}

// -----------------------------------------------------------------------------
// P_AimLineAttack
// -----------------------------------------------------------------------------

const fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, const fixed_t distance)
{
    fixed_t	x2, y2;

    t1 = P_SubstNullMobj(t1);

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
    y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

    if (!t1->player || vanilla_gameplay_feature(autoaim_vertical))
    {
        // can't shoot outside view angles
        // [JN] Widescreen support: SCREENWIDTH is replaced with (ORIGWIDTH << hires),
        // this value must not be changed for demo compatibily and other stuff.
        topslope = (SCREENHEIGHT/2)*FRACUNIT/((ORIGWIDTH << hires)/2);
        bottomslope = -(SCREENHEIGHT/2)*FRACUNIT/((ORIGWIDTH << hires)/2);
    }
    else
    {
        // Player and vertical autoaim disabled
        fixed_t factor;
        if(aspect_ratio >= 2)
        {
            // [JN] Wide screen: new magic number :(
            factor = 180;
        }
        else
        {
            factor = (screenblocks <= 10) ? 160 : 146;
        }
        topslope = ((t1->player->lookdir / MLOOKUNIT) << FRACBITS) / factor + 5;
        bottomslope = ((t1->player->lookdir / MLOOKUNIT) << FRACBITS) / factor - 5;
    }

    attackrange = distance;
    linetarget = NULL;

    P_PathTraverse ( t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_AimTraverse);

    if (linetarget)
    {
        return aimslope;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// P_LineAttack
// If damage == 0, it is just a test trace that will leave linetarget set.
// -----------------------------------------------------------------------------

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t distance, 
                   const fixed_t slope, const int damage)
{
    fixed_t	x2, y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
    y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse (t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_ShootTraverse);
}


// =============================================================================
// USE LINES
// =============================================================================

static mobj_t *usething;

static boolean PTR_UseTraverse (intercept_t *in)
{
    int side;

    if (!in->d.line->special)
    {
        P_LineOpening (in->d.line);

        if (openrange <= 0)
        {
            // [JN] Не прерывать звуки выстрелов при нажатии на стену
            S_StartSound (singleplayer ? NULL : usething, sfx_noway);

            // can't use through a wall
            return false;	
        }

        // not a special line, but keep checking
        return true;
    }

    side = 0;
    if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
    {
        side = 1;
    }
	
    P_UseSpecialLine (usething, in->d.line, side);

    // can't use for than one special line in a row
    return false;
}

// -----------------------------------------------------------------------------
// PTR_NoWayTraverse
//
// Returns false if a "oof" sound should be made because of a blocking
// linedef. Makes 2s middles which are impassable, as well as 2s uppers
// and lowers which block the player, cause the sound effect when the
// player tries to activate them. Specials are excluded, although it is
// assumed that all special linedefs within reach have been considered
// and rejected already (see P_UseLines).
//
// by Lee Killough
// -----------------------------------------------------------------------------

static boolean PTR_NoWayTraverse (intercept_t *in)
{
    line_t *ld = in->d.line;                          // This linedef

    return ld->special ||                             // Ignore specials
          (ld->flags & ML_TWOSIDED &&                 // [JN] Ignore two-sided...
           ld->flags & ML_BLOCKING) ||                // ...blocking linedefs
         !(ld->flags & ML_BLOCKING ||                 // Always blocking
          (P_LineOpening(ld),                         // Find openings
           openrange <= 0 ||                          // No opening
           openbottom > usething->z+24*FRACUNIT ||    // Too high it blocks
           opentop < usething->z+usething->height));  // Too low it blocks
}

// -----------------------------------------------------------------------------
// PTR_NoWayAudible
// [JN] Check if it is possible to play an "oof" sound,
// i.e. if usable wall segment is really visible and touchable.
// -----------------------------------------------------------------------------

boolean PTR_NoWayAudible (line_t *line)
{
    P_LineOpening(line);

    return (openrange <= 0 ||
            openbottom > usething->z + 24*FRACUNIT ||
            opentop < usething->z + usething->height);
}

// -----------------------------------------------------------------------------
// P_UseLines
// Looks for special lines in front of the player to activate.
// -----------------------------------------------------------------------------

void P_UseLines (const player_t *player) 
{
    const int angle = player->mo->angle >> ANGLETOFINESHIFT;
    fixed_t	x1, y1;
    fixed_t	x2, y2;

    usething = player->mo;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];
	
    // old code:
    //
    // P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
    //
    // This added test makes the "oof" sound work on 2s lines -- killough:
    // [JN] Do not break firing sounds by using walls, do not play in "vanilla".

    if (P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse))
    {
        if (!P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse) 
        && improved_collision && !vanillaparm)
        {
            S_StartSound (singleplayer ? NULL : usething, sfx_noway);
        }
    }
}


// =============================================================================
// RADIUS ATTACK
// =============================================================================

static mobj_t *bombsource;
static mobj_t *bombspot;
static int     bombdamage;

// -----------------------------------------------------------------------------
// PIT_RadiusAttack
// "bombsource" is the creature that caused the explosion at "bombspot".
// -----------------------------------------------------------------------------

boolean PIT_RadiusAttack (mobj_t *thing)
{
    fixed_t	dx, dy, dist;

    if (!(thing->flags & MF_SHOOTABLE))
    {
        return true;
    }

    // Boss spider and cyborg take no damage from concussion.
    if (thing->type == MT_CYBORG || thing->type == MT_SPIDER)
    {
        return true;
    }

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;

    if (dist < 0)
    {
        dist = 0;
    }

    if (dist >= bombdamage)
    {
        // out of range
        return true;
    }

    if ( P_CheckSight (thing, bombspot) )
    {
        // must be in direct path
        P_DamageMobj (thing, bombspot, bombsource, bombdamage - dist);
    }

    return true;
}

// -----------------------------------------------------------------------------
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
// -----------------------------------------------------------------------------

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage)
{
    int     x, y;
    int     xl, xh;
    int     yl, yh;
    fixed_t dist;

    dist = (damage+MAXRADIUS)<<FRACBITS;
    yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = spot;
    bombsource = source;
    bombdamage = damage;

    for (y = yl ; y <= yh ; y++)
    {
        for (x = xl ; x <= xh ; x++)
        {
            P_BlockThingsIterator (x, y, PIT_RadiusAttack);
        }
    }
}


// =============================================================================
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height, call this routine to 
// adjust the positions of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned. If crunch is true,
// they will take damage as they are being crushed. If Crunch is false,
// you should set the sector height back the way it was and call P_ChangeSector
// again to undo the changes.
// =============================================================================

static boolean		crushchange;
static boolean		nofit;

// -----------------------------------------------------------------------------
// PIT_ChangeSector
// -----------------------------------------------------------------------------

boolean PIT_ChangeSector (mobj_t *thing)
{
    if (P_ThingHeightClip (thing))
    {
        // keep checking
        return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        // [crispy] no blood, no giblets
        // S_GIBS should be a "safe" state, and so is S_NULL
        P_SetMobjState (thing, (thing->flags & MF_NOBLOOD) ? S_NULL : S_GIBS);

        // [JN] Initialize player's movement momentum for preventing
        // "flying through" solid walls in crushed state.
        if (singleplayer && thing->type == MT_PLAYER)
        {
            thing->momx = 0;
            thing->momy = 0;
        }

        if (gameversion > exe_doom_1_2)
        {
            thing->flags &= ~MF_SOLID;
        }
        thing->height = 0;
        thing->radius = 0;

        // [JN] Play extra crushing sound (D*SLOP2):
        if (crushed_corpses_sfx && !vanillaparm)
        {
            if (thing->type == MT_PLAYER)
            {
                // NULL-sourced for player for breaking pain/death sounds.
                S_StartSound(singleplayer ? NULL : thing, sfx_slop2);
            }
            if (thing->type != MT_BARREL)
            {
                // No sound for Explosive Barrels.
                S_StartSound(thing, sfx_slop2);
            }
        }

        // [crispy] connect giblet object with the crushed monster
        thing->target = thing;

        // keep checking
        return true;		
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj (thing);

        // keep checking
        return true;		
    }

    if (!(thing->flags & MF_SHOOTABLE))
    {
        // assume it is bloody gibs or something
        return true;			
    }

    nofit = true;

    if (crushchange && !(leveltime&3))
    {
        mobj_t *mo;

        P_DamageMobj(thing,NULL,NULL,10);

        // spray blood in a random direction

        if (singleplayer && thing->type == MT_BARREL && !strict_mode && !vanillaparm)
        {
            // [JN] Barrel should not bleed by crushed damage
            return true;
        }
        else
        {
            // [JN] Rest of monsters are bleeding
            mo = P_SpawnMobj (thing->x, thing->y, thing->z + thing->height/2, MT_BLOOD);
        }

        mo->momx = P_SubRandom() << 12;
        mo->momy = P_SubRandom() << 12;

        // [crispy] connect blood object with the monster that bleeds it
        mo->target = thing;

        // [crispy] Spectres bleed spectre blood
        if (colored_blood && !vanillaparm)
        {
            mo->flags |= (thing->flags & MF_SHADOW);
        }
    }

    // keep checking (crush other things)	
    return true;	
}

// -----------------------------------------------------------------------------
// P_ChangeSector
// -----------------------------------------------------------------------------

boolean P_ChangeSector (sector_t *sector, boolean crunch)
{
    int		x, y;

    nofit = false;
    crushchange = crunch;
    movingsector = sector;

    // re-check heights for all things near the moving sector
    for (x = sector->blockbox[BOXLEFT] ; x <= sector->blockbox[BOXRIGHT] ; x++)
    {
        for (y = sector->blockbox[BOXBOTTOM] ; y <= sector->blockbox[BOXTOP] ; y++)
        {
            P_BlockThingsIterator (x, y, PIT_ChangeSector);
        }
    }
	
    return nofit;
}

// -----------------------------------------------------------------------------
// SpechitOverrun
// Code to emulate the behavior of Vanilla Doom when encountering an overrun
// of the spechit array.  This is by Andrey Budko (entryway) and comes from his
// PrBoom plus port.  A big thanks to Andrey for this.
// -----------------------------------------------------------------------------

static void SpechitOverrun (line_t *ld)
{
    static unsigned int baseaddr = 0;
    unsigned int addr;
   
    if (baseaddr == 0)
    {
        int p;

        // This is the first time we have had an overrun.  Work out
        // what base address we are going to use.
        // Allow a spechit value to be specified on the command line.

        //!
        // @category compat
        // @arg <n>
        //
        // Use the specified magic value when emulating spechit overruns.
        //

        p = M_CheckParmWithArgs("-spechit", 1);
        
        if (p > 0)
        {
            M_StrToInt(myargv[p+1], (int *) &baseaddr);
        }
        else
        {
            baseaddr = DEFAULT_SPECHIT_MAGIC;
        }
    }
    
    // Calculate address used in doom2.exe

    addr = baseaddr + (ld - lines) * 0x3E;

    switch(numspechit)
    {
        case 9: 
        case 10:
        case 11:
        case 12:
            tmbbox[numspechit-9] = addr;
            break;
        case 13: 
            crushchange = addr; 
            break;
        case 14: 
            nofit = addr; 
            break;
        default:
            printf("SpechitOverrun: Warning: unable to emulate"
                            "an overrun where numspechit=%i\n",
                            numspechit);
            break;
    }
}

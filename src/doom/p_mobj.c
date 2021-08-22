//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Moving object handling. Spawn functions.
//


#include <stdio.h>
#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "p_local.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "doomstat.h"
#include "f_finale.h"
#include "crispy.h"
#include "jn.h"


#define MOBJ_CYCLE_LIMIT 1000000
#define STOPSPEED        0x1000
#define FRICTION         0xe800


mapthing_t itemrespawnque[ITEMQUESIZE];
int        itemrespawntime[ITEMQUESIZE];
int        iquehead;
int        iquetail;

void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_t*	mthing);

extern fixed_t attackrange;

// [JN] Floating amplitude LUT.
// Initial values are from Heretic, but divided by 3.
static const fixed_t FloatBobOffsets[64] = {
          0,   17129,   34094,   50730,
      66878,   82382,   97092,  110868,
     123575,  135093,  145309,  154126,
     161459,  167237,  171404,  173921,
     174762,  173921,  171404,  167237,
     161459,  154126,  145309,  135093,
     123575,  110868,   97092,   82382,
      66878,   50730,   34094,   17129,
         -1,  -17130,  -34094,  -50731,
     -66879,  -82382,  -97093, -110868,
    -123576, -135093, -145310, -154127,
    -161460, -167237, -171405, -173921,
    -174762, -173921, -171404, -167237,
    -161459, -154127, -145310, -135093,
    -123576, -110868,  -97093,  -82382,
     -66879,  -50731,  -34094,  -17129
};

// -----------------------------------------------------------------------------
// P_SetMobjState
// Returns true if the mobj is still present.
//
// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in P_SetMobjState() executes and exit with an error once
// an arbitrary very large limit is reached.
// -----------------------------------------------------------------------------

boolean P_SetMobjState (mobj_t *mobj, statenum_t state)
{
    state_t *st;
    int	cycle_counter = 0;

    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *) S_NULL;
            P_RemoveMobj (mobj);
            return false;
        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        // Modified handling.
        // Call action functions when the state is set
        if (st->action.acp3)
        {
            // [crispy] let pspr action pointers get called from mobj states
            st->action.acp3(mobj, NULL, NULL);
        }

        state = st->nextstate;

        if (cycle_counter++ > MOBJ_CYCLE_LIMIT)
        {
            I_Error(english_language ?
                    "P_SetMobjState: Infinite state cycle detected!" :
                    "P_SetMobjState: обнаружен бесконечный цикл!");
        }
    } while (!mobj->tics);

    return true;
}

// -----------------------------------------------------------------------------
// [crispy] return the latest "safe" state in a state sequence,
// so that no action pointer is ever called
// -----------------------------------------------------------------------------

static statenum_t P_LatestSafeState(statenum_t state)
{
    statenum_t safestate = S_NULL;
    static statenum_t laststate, lastsafestate;

    if (state == laststate)
    {
        return lastsafestate;
    }

    for (laststate = state ; state != S_NULL ; state = states[state].nextstate)
    {
        if (safestate == S_NULL)
        {
            safestate = state;
        }

        if (states[state].action.acp1)
        {
            safestate = S_NULL;
        }

        // [crispy] a state with -1 tics never changes
        if (states[state].tics == -1)
        {
            break;
        }
    }

    return lastsafestate = safestate;
}

// -----------------------------------------------------------------------------
// P_ExplodeMissile  
// -----------------------------------------------------------------------------

static void P_ExplodeMissileSafe (mobj_t *mo, boolean safe)
{
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, safe ? P_LatestSafeState(mobjinfo[mo->type].deathstate) : 
                                                 mobjinfo[mo->type].deathstate);

    mo->tics -= safe ? Crispy_Random()&3 : P_Random()&3;

    if (mo->tics < 1)
    {
        mo->tics = 1;
    }
    mo->flags &= ~MF_MISSILE;
    mo->flags |= MF_TRANSLUCENT;

    if (mo->info->deathsound)
    {
        S_StartSound (mo, mo->info->deathsound);
    }
}

// -----------------------------------------------------------------------------
// P_ExplodeMissile
// -----------------------------------------------------------------------------

void P_ExplodeMissile (mobj_t *mo)
{
    return P_ExplodeMissileSafe(mo, false);
}

// -----------------------------------------------------------------------------
// P_XYMovement  
// -----------------------------------------------------------------------------

void P_XYMovement (mobj_t* mo) 
{ 	
    player_t *player;
    fixed_t   xmove, ymove;

    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

            // [JN] Optionally fix bug: https://doomwiki.org/wiki/Lost_soul_target_amnesia
            if (singleplayer && agressive_lost_souls && !vanillaparm)
            {
                P_SetMobjState (mo, mo->info->seestate);
            }
            else
            {
                P_SetMobjState (mo, mo->info->spawnstate);
            }
        }

        return;
    }

    player = mo->player;

    if (mo->momx > MAXMOVE)
    {
        mo->momx = MAXMOVE;
    }
    else if (mo->momx < -MAXMOVE)
    {
        mo->momx = -MAXMOVE;
    }

    if (mo->momy > MAXMOVE)
    {
        mo->momy = MAXMOVE;
    }
    else if (mo->momy < -MAXMOVE)
    {
        mo->momy = -MAXMOVE;
    }

    xmove = mo->momx;
    ymove = mo->momy;

    do
    {
        fixed_t ptryx, ptryy;

        // killough 8/9/98: fix bug in original Doom source:
        // Large negative displacements were never considered.
        // This explains the tendency for Mancubus fireballs
        // to pass through walls.
        //
        // [JN] Fix: https://doomwiki.org/wiki/Mancubus_fireball_clipping
        // But keep this bug in vanilla mode, needed for 
        // a missile explosion in the code below.
        //
        // Thanks to Jeff Doggett for simplifying!
        //
        // Additionally, wallrunning bug is fixed (mo->player condition):
        // https://doomwiki.org/wiki/Wallrunning
        //
        // Thanks AXDOOMER and Brad Harding!

        if (singleplayer && !vanillaparm ? 
            mo->player ? ((xmove > MAXMOVE/2 || ymove > MAXMOVE/2) && (xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2)) 
                       : ((xmove > MAXMOVE/2 || ymove > MAXMOVE/2) || (xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2))
                       :  (xmove > MAXMOVE/2 || ymove > MAXMOVE/2))
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove (mo, ptryx, ptryy))
        {
            // blocked move
            if (mo->player)
            {	// try to slide along it
                if (BlockingMobj == NULL          // [JN] Mobj is not blocking.
                ||  BlockingMobj->health <= 0     // [JN] Allow to slightly bump into falling corpse.
                || !singleplayer || vanillaparm)  // [JN] Keep demo compatibility.
                {   
                    // [JN] Slide movement.
                    P_SlideMove(mo);
                }
                else
                {
                    // [JN] Slide against mobj.
                    // Remove X/Y momentum while moving on solid things.
                    if (P_TryMove(mo, mo->x, ptryy))
                    {
                        mo->momx = 0;
                    }
                    else if (P_TryMove(mo, ptryx, mo->y))
                    {
                        mo->momy = 0;
                    }
                    else
                    {
                        mo->momx = mo->momy = 0;
                    }
                }
            }
            else if (mo->flags & MF_MISSILE)
            {
                boolean safe = false;

                // explode a missile
                if (ceilingline 
                &&  ceilingline->backsector
                &&  ceilingline->backsector->ceilingpic == skyflatnum)
                {
                    if (mo->z > ceilingline->backsector->ceilingheight 
                    || vanillaparm) // [JN] Keep vanilla behaviour by removing missile object.
                    {
                        // Hack to prevent missiles exploding
                        // against the sky. Does not handle sky floors.
                        P_RemoveMobj (mo);
                        return;
                    }
                    else if (singleplayer)
                    {
                        // [JN] Make a full explosion with damage in single player.
                        safe = false;
                    }
                    else
                    {
                        safe = true;
                    }
                }
                P_ExplodeMissileSafe (mo, safe);
            }
            else
            {
                mo->momx = mo->momy = 0;
            }
        }
    } while (xmove || ymove);

    // slow down
    if (player && player->cheats & CF_NOMOMENTUM)
    {
        // debug option for no sliding at all
        mo->momx = mo->momy = 0;
        return;
    }

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
    {
        return;  // no friction for missiles ever
    }

    if (mo->z > mo->floorz)
    {
        return;  // no friction when airborne
    }

    if (mo->flags & MF_CORPSE)
    {
        // do not stop sliding
        //  if halfway off a step with some momentum
        if (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4
        ||  mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
            {
                return;
            }
        }
    }

    // [JN] Torque: needed?
    // killough 8/11/98: add bouncers
    // killough 9/15/98: add objects falling off ledges
    // killough 11/98: only include bouncers hanging off ledges
    if (singleplayer && (/*(mo->flags & MF_BOUNCES && mo->z > mo->dropoffz) ||*/
    mo->flags & MF_CORPSE || mo->intflags & MIF_FALLING) 
    && (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4
    ||  mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4) 
    &&  mo->floorz != mo->subsector->sector->floorheight)
    {
        return;  // do not stop sliding if halfway off a step with some momentum
    }

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
	&&  mo->momy > -STOPSPEED && mo->momy < STOPSPEED
	&& (!player || (player->cmd.forwardmove == 0
    &&  player->cmd.sidemove == 0 )))
    {
        // if in a walking frame, stop moving
        if (player&&(unsigned)((player->mo->state - states)- S_PLAY_RUN1) < 4)
        {
            P_SetMobjState (player->mo, S_PLAY);
        }

        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        mo->momx = FixedMul (mo->momx, FRICTION);
        mo->momy = FixedMul (mo->momy, FRICTION);
    }
}

// -----------------------------------------------------------------------------
// P_ZMovement
// -----------------------------------------------------------------------------

void P_ZMovement (mobj_t *mo)
{
    fixed_t	dist, delta;

    // check for smooth step up
    if (mo->player && mo->z < mo->floorz)
    {
        mo->player->viewheight -= mo->floorz-mo->z;
        mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight)>>3;
    }

    // adjust height
    mo->z += mo->momz;

    if ( mo->flags & MF_FLOAT && mo->target)
    {
        // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist = P_AproxDistance (mo->x - mo->target->x, mo->y - mo->target->y);
            delta = (mo->target->z + (mo->height>>1)) - mo->z;

            if (delta<0 && dist < -(delta*3))
            {
                mo->z -= FLOATSPEED;
            }
            else if (delta>0 && dist < (delta*3))
            {
                mo->z += FLOATSPEED;
            }
        }
    }

    // clip movement
    if (mo->z <= mo->floorz)
    {
        // hit the floor

        // Note (id):
        //  somebody left this after the setting momz to 0,
        //  kinda useless there.
        //
        // cph - This was the a bug in the linuxdoom-1.10 source which
        //  caused it not to sync Doom 2 v1.9 demos. Someone
        //  added the above comment and moved up the following code. So
        //  demos would desync in close lost soul fights.
        // Note that this only applies to original Doom 1 or Doom2 demos - not
        //  Final Doom and Ultimate Doom.  So we test demo_compatibility *and*
        //  gamemission. (Note we assume that Doom1 is always Ult Doom, which
        //  seems to hold for most published demos.)
            //  
            //  fraggle - cph got the logic here slightly wrong.  There are three
            //  versions of Doom 1.9:
            //
            //  * The version used in registered doom 1.9 + doom2 - no bounce
            //  * The version used in ultimate doom - has bounce
            //  * The version used in final doom - has bounce
            //
            // So we need to check that this is either retail or commercial
            // (but not doom2)
                //
                // JN - Doom 1.9 Special Edition has no bounce. Since "exe_doom_se" 
                // have a higher number than "exe_ultimate" for preventing too much 
                // code changes, it's declared here as an additional condition.
                // It's safe for Ultimate Doom IWAD internal demos.
                //

        int correct_lost_soul_bounce = gameversion >= exe_ultimate &&
                                       gameversion != exe_doom_se;

        if (correct_lost_soul_bounce && mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->momz = -mo->momz;
        }

        if (mo->momz < 0)
        {
            if (mo->player && mo->momz < -GRAVITY*8)
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                mo->player->deltaviewheight = mo->momz>>3;

                // [crispy] squat down weapon sprite as well
                // [JN] no squatting in vanilla mode
                if (!vanillaparm)
                {
                    // [JN] Suqat BFG9000 heavier and slower
                    if (mo->player->readyweapon == wp_bfg)
                    {
                        mo->player->psp_dy_max = mo->momz>>1;
                    }
                    else
                    {
                        mo->player->psp_dy_max = mo->momz>>2;
                    }
                }

                // [JN] Only alive player makes "oof" sound.
                if (mo->health > 0)
                {
                    // [JN] Не прерывать звуки выстрелов при падении с высоты
                    S_StartSound (singleplayer ? NULL : mo, sfx_oof);
                }
            }

            mo->momz = 0;
        }

        mo->z = mo->floorz;

        // cph 2001/05/26 -
        // See lost soul bouncing comment above. We need this here for bug
        // compatibility with original Doom2 v1.9 - if a soul is charging and
        // hit by a raising floor this incorrectly reverses its Y momentum.
        if (!correct_lost_soul_bounce && mo->flags & MF_SKULLFLY)
        {
            mo->momz = -mo->momz;
        }

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {
            P_ExplodeMissile (mo);
            return;
        }
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (mo->momz == 0)
        {
            mo->momz = -GRAVITY*2;
        }
        else
        {
            mo->momz -= GRAVITY;
        }
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        // hit the ceiling
        if (mo->momz > 0)
        {
            mo->momz = 0;
        }

	    mo->z = mo->ceilingz - mo->height;

        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->momz = -mo->momz;
        }

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {
            // [JN] Fix projectiles explode on impact with "sky" ceilings
            if (singleplayer && !vanillaparm && mo->subsector->sector->ceilingpic == skyflatnum)
            {
                P_RemoveMobj(mo);
            }
            else
            {
                P_ExplodeMissile (mo);
            }

            return;
        }
    }
}

// -----------------------------------------------------------------------------
// P_NightmareRespawn
// -----------------------------------------------------------------------------

void P_NightmareRespawn (mobj_t *mobj)
{
    fixed_t     x = mobj->spawnpoint.x << FRACBITS;
    fixed_t     y = mobj->spawnpoint.y << FRACBITS;
    fixed_t     z;
    subsector_t *ss;
    mobj_t      *mo;
    mapthing_t  *mthing = &mobj->spawnpoint;

    // [BH] Fix (0,0) respawning bug. See <http://doomwiki.org/wiki/(0,0)_respawning_bug>.
    if (singleplayer && !x && !y)
    {
        x = mobj->x;
        y = mobj->y;
    }

    // somthing is occupying it's position?
    if (!P_CheckPosition (mobj, x, y))
    {
        return;  // no respwan
    }

    // spawn a teleport fog at old spot
    // because of removal of the body?
    mo = P_SpawnMobj (mobj->x, mobj->y, mobj->subsector->sector->floorheight, MT_TFOG); 

    // initiate teleport sound
    S_StartSound (mo, sfx_telept);

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y); 
    mo = P_SpawnMobj (x, y, ss->sector->floorheight, MT_TFOG);
    S_StartSound (mo, sfx_telept);

    // spawn the new monster
    mthing = &mobj->spawnpoint;

    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else
    {
        z = ONFLOORZ;
    }

    // inherit attributes from deceased one
    mo = P_SpawnMobj (x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;	
    mo->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH)
    {
        mo->flags |= MF_AMBUSH;
    }

    mo->reactiontime = 18;

    // remove the old monster,
    P_RemoveMobj (mobj);
}

// -----------------------------------------------------------------------------
// P_MobjThinker
// -----------------------------------------------------------------------------

void P_MobjThinker (mobj_t *mobj)
{
    // [AM] Handle interpolation unless we're an active player.
    if (!(mobj->player != NULL && mobj == mobj->player->mo))
    {
        // Assume we can interpolate at the beginning
        // of the tic.
        mobj->interp = true;

        // Store starting position for mobj interpolation.
        mobj->oldx = mobj->x;
        mobj->oldy = mobj->y;
        mobj->oldz = mobj->z;
        mobj->oldangle = mobj->angle;
    }

    // momentum movement
    BlockingMobj = NULL;
    if (mobj->momx ||  mobj->momy || (mobj->flags&MF_SKULLFLY))
    {
        P_XYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v)(-1))
        {
            return;  // mobj was removed
        }
    }

    if ((mobj->z != mobj->floorz) || mobj->momz
    || (BlockingMobj && singleplayer && !vanillaparm))
    {
        P_ZMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v)(-1))
        {
            return;  // mobj was removed
        }
    }

    // [JN] Activation of floating items
    if (floating_powerups && !vanillaparm && singleplayer
    && (mobj->type == MT_MEGA       // Megasphere
    ||  mobj->type == MT_MISC12     // Supercharge
    ||  mobj->type == MT_INV        // Invulnerability
    ||  mobj->type == MT_INS))      // Partial invisibility
    {
        mobj->z = mobj->floorz + FloatBobOffsets[(mobj->health++) & 63];
    }

    // [JN] killough 9/12/98: objects fall off ledges if they are hanging off
    // slightly push off of ledge if hanging more than halfway off
    if (singleplayer && !vanillaparm && torque)
    {
        if (mobj->z > mobj->dropoffz        // Only objects contacting dropoff
        && !(mobj->flags & MF_NOGRAVITY)    // Only objects which fall
        &&  (mobj->flags & MF_CORPSE)       // [JN] And only for corpses
        &&   mobj->geartics > 0)            // [JN] And only if torque tics are available.
        {
            P_ApplyTorque(mobj);            // Apply torque
        }
        else
        {
            mobj->intflags &= ~MIF_FALLING, mobj->gear = 0;  // Reset torque
        }
    }

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics && !P_SetMobjState (mobj, mobj->state->nextstate))
        {
            return;  // freed itself
        }
    }
    else
    {
        // check for nightmare respawn
        if (!(mobj->flags & MF_COUNTKILL))
        {
            return;
        }
        if (!respawnmonsters)
        {
            return;
        }

        mobj->movecount++;

        if (mobj->movecount < 12*TICRATE)
        {
            return;
        }
        if (leveltime & 31)
        {
            return;
        }
        if (P_Random () > 4)
        {
            return;
        }

        P_NightmareRespawn (mobj);
    }
}

// -----------------------------------------------------------------------------
// P_SpawnMobj
// -----------------------------------------------------------------------------

static mobj_t *P_SpawnMobjSafe (fixed_t x, fixed_t y, fixed_t z, 
                                mobjtype_t type, boolean safe)
{
    mobj_t     *mobj;
    state_t    *st;
    mobjinfo_t *info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare && gameskill != sk_ultranm)
    {
        mobj->reactiontime = info->reactiontime;
    }

    mobj->lastlook = safe ? Crispy_Random () % MAXPLAYERS : P_Random () % MAXPLAYERS;
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[safe ? P_LatestSafeState(info->spawnstate) : info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // set subsector and/or block links
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->z = mobj->ceilingz - mobj->info->height;
    }
    else
    {
        mobj->z = z;
    }

    // [AM] Do not interpolate on spawn.
    mobj->interp = false;

    // [AM] Just in case interpolation is attempted...
    mobj->oldx = mobj->x;
    mobj->oldy = mobj->y;
    mobj->oldz = mobj->z;
    mobj->oldangle = mobj->angle;

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;

    P_AddThinker (&mobj->thinker);

    return mobj;
}

// -----------------------------------------------------------------------------
// P_SpawnMobj
// -----------------------------------------------------------------------------

mobj_t *P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    return P_SpawnMobjSafe(x, y, z, type, false);
} 

// -----------------------------------------------------------------------------
// P_RemoveMobj
// -----------------------------------------------------------------------------

void P_RemoveMobj (mobj_t *mobj)
{
    if ((mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED)
    && (mobj->type != MT_INV) && (mobj->type != MT_INS))
    {
        itemrespawnque[iquehead] = mobj->spawnpoint;
        itemrespawntime[iquehead] = leveltime;
        iquehead = (iquehead+1)&(ITEMQUESIZE-1);

        // lose one off the end?
        if (iquehead == iquetail)
        {
            iquetail = (iquetail+1)&(ITEMQUESIZE-1);
        }
    }

    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);

    // stop any playing sound
    S_StopSound (mobj);

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}

// -----------------------------------------------------------------------------
// P_FindDoomedNum
//
// Finds a mobj type with a matching doomednum
// killough 8/24/98: rewrote to use hashing
// -----------------------------------------------------------------------------

static int P_FindDoomedNum (unsigned type)
{
    static struct { int first, next; } *hash;
    int i;

    if (!hash)
    {
        hash = Z_Malloc(sizeof *hash * NUMMOBJTYPES, PU_CACHE, (void **) &hash);

        for (i = 0; i < NUMMOBJTYPES; i++)
        hash[i].first = NUMMOBJTYPES;

        for (i = 0; i < NUMMOBJTYPES; i++)
        if (mobjinfo[i].doomednum != -1)
        {
            unsigned h = (unsigned) mobjinfo[i].doomednum % NUMMOBJTYPES;
            hash[i].next = hash[h].first;
            hash[h].first = i;
        }
    }

    i = hash[type % NUMMOBJTYPES].first;

    while ((i < NUMMOBJTYPES) && ((unsigned)mobjinfo[i].doomednum != type))
    {
        i = hash[i].next;
    }

    return i;
}

// -----------------------------------------------------------------------------
// P_RespawnSpecials
// -----------------------------------------------------------------------------

void P_RespawnSpecials (void)
{
    int          i;
    fixed_t		 x, y, z;
    subsector_t *ss; 
    mobj_t      *mo;
    mapthing_t  *mthing;

    // only respawn items in deathmatch
    if (deathmatch != 2 && deathmatch != 3)
    {
        return;
    }

    // nothing left to respawn?
    if (iquehead == iquetail)
    {
        return;
    }

    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < 30*TICRATE)
    {
        return;
    }

    mthing = &itemrespawnque[iquetail];

    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y); 
    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG); 
    S_StartSound (mo, sfx_itmbk);

    // find which type to spawn
    // [JN] killough 8/23/98: use table for faster lookup
    i = P_FindDoomedNum(mthing->type);

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else
    {
        z = ONFLOORZ;
    }

    mo = P_SpawnMobj (x,y,z, i);
    mo->spawnpoint = *mthing;	
    mo->angle = ANG45 * (mthing->angle/45);

    // pull it from the que
    iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}

// -----------------------------------------------------------------------------
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged between levels.
// -----------------------------------------------------------------------------

void P_SpawnPlayer (mapthing_t *mthing)
{
    fixed_t   x, y, z;
    player_t *p;
    mobj_t   *mobj;

    if (mthing->type == 0)
    {
        return;
    }

    // not playing?
    if (!playeringame[mthing->type-1])
    {
        return;					
    }

    p = &players[mthing->type-1];

    if (p->playerstate == PST_REBORN)
    {
        G_PlayerReborn (mthing->type-1);
    }

    x    = mthing->x << FRACBITS;
    y    = mthing->y << FRACBITS;
    z    = ONFLOORZ;
    mobj = P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites
    if (mthing->type > 1)
    {
        mobj->flags |= (mthing->type-1)<<MF_TRANSSHIFT;
    }

    mobj->angle	= ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;	
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    p->deltaviewheight = 0;
    p->lookdir = 0;
    p->psp_dy = p->psp_dy_max = 0;
    skippsprinterp = true;
    finale_wipe_done = false;

    // setup gun psprite
    P_SetupPsprites (p);

    // give all cards in death match mode
    if (deathmatch)
        for (int i = 0 ; i < NUMCARDS ; i++)
            p->cards[i] = true;

    if (mthing->type-1 == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();		
    }
}

// -----------------------------------------------------------------------------
// P_SpawnMapThing
// The fields of the mapthing should already be in host byte order.
// -----------------------------------------------------------------------------

void P_SpawnMapThing (mapthing_t *mthing)
{
    int      i, bit;
    fixed_t  x, y, z;
    mobj_t  *mobj;

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[10])
        {
            memcpy (deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }

    if (mthing->type <= 0)
    {
        // Thing type 0 is actually "player -1 start".  
        // For some reason, Vanilla Doom accepts/ignores this.
        return;
    }

    // check for players specially
    if (mthing->type <= 4)
    {
        // save spots for respawning in network games
        playerstarts[mthing->type-1] = *mthing;
        playerstartsingame[mthing->type-1] = true;
        if (!deathmatch)
        {
            P_SpawnPlayer (mthing);
        }
        return;
    }

    // check for apropriate skill level
    if (!netgame && (mthing->options & 16))
    {
        return;
    }

    if (gameskill == sk_baby)
    {
        bit = 1;
    }
    else if (gameskill == sk_nightmare || gameskill == sk_ultranm)
    {
        bit = 4;
    }
    else
    {
        bit = 1<<(gameskill-1);
    }

    if (!(mthing->options & bit))
    {
        return;
    }

    // find which type to spawn
    // [JN] killough 8/23/98: use table for faster lookup
    i = P_FindDoomedNum(mthing->type);

    if (i == NUMMOBJTYPES)
    {
        // [crispy] ignore unknown map things
        printf (english_language ?
                "P_SpawnMapThing: Unknown type %i at (%i, %i)\n" :
                "P_SpawnMapThing: неизвестный предмет %i в (%i, %i)\n",
                mthing->type, mthing->x, mthing->y);
        return;
    }

    // don't spawn keycards and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
    {
        return;
    }

    // don't spawn any monsters if -nomonsters
    // [JN] Keep Commander Keen so player can exit MAP32.
    if (nomonsters && i != MT_KEEN 
    && (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL)))
    {
        return;
    }

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else
    {
        z = ONFLOORZ;
    }

    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
    {
        mobj->tics = 1 + (P_Random () % mobj->tics);
    }
    if (mobj->flags & MF_COUNTKILL)
    {
        totalkills++;
    }
    if (mobj->flags & MF_COUNTITEM)
    {
        totalitems++;
    }

    mobj->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH)
    {
        mobj->flags |= MF_AMBUSH;
    }

    // [crispy] randomly flip space marine corpse objects
    if (mobj->info->spawnstate == S_PLAY_DIE7
    ||  mobj->info->spawnstate == S_PLAY_XDIE9)
    {
        mobj->health -= Crispy_Random() & 1;
        // [crispy] randomly colorize space marine corpse objects
        if (!netgame && colored_blood && !vanillaparm)
        {
            mobj->flags |= (Crispy_Random() & 3) << MF_TRANSSHIFT;
        }
    }

    // [crispy] blinking key or skull in the status bar
    if (mobj->sprite == SPR_BSKU)
    {
        st_keyorskull[it_bluecard] = 3;
    }
    else if (mobj->sprite == SPR_RSKU)
    {
        st_keyorskull[it_redcard] = 3;
    }
    else if (mobj->sprite == SPR_YSKU)
    {
        st_keyorskull[it_yellowcard] = 3;
    }
}


// =============================================================================
// GAME SPAWN FUNCTIONS
// =============================================================================

// -----------------------------------------------------------------------------
// P_SpawnPuff
// -----------------------------------------------------------------------------

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z)
{
    return P_SpawnPuffSafe(x, y, z, false);
}

// -----------------------------------------------------------------------------
// P_SpawnPuffSafe
// -----------------------------------------------------------------------------

void P_SpawnPuffSafe (fixed_t x, fixed_t y, fixed_t z, boolean safe)
{
    mobj_t *th;

    z += safe ? (Crispy_SubRandom() << 10) : (P_SubRandom() << 10);

    th = P_SpawnMobjSafe (x,y,z, MT_PUFF, safe);
    th->momz = FRACUNIT;
    th->tics -= safe ? Crispy_Random()&3 : P_Random()&3;

    if (th->tics < 1)
    {
        th->tics = 1;
    }

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
    {
        P_SetMobjState (th, safe ? P_LatestSafeState(S_PUFF3) : S_PUFF3);
    }
}

// -----------------------------------------------------------------------------
// P_SpawnBlood
// [JN] Modified using a method by Fabian Greffrath (pass thing type)
// -----------------------------------------------------------------------------

void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t *target)
{
    mobj_t *th;

    z += (P_SubRandom() << 10);
    th = P_SpawnMobj (x,y,z, MT_BLOOD);
    th->momz = FRACUNIT*2;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
    {
        th->tics = 1;
    }

    if (damage <= 12 && damage >= 9)
    {
        P_SetMobjState (th,S_BLOOD2);
    }
    else if (damage < 9)
    {
        P_SetMobjState (th,S_BLOOD3);
    }

    // [crispy] connect blood object with the monster that bleeds it
    th->target = target;

    // [crispy] Spectres bleed spectre blood
    if (colored_blood && !vanillaparm)
    {
        th->flags |= (target->flags & MF_SHADOW);
    }
}

// -----------------------------------------------------------------------------
// P_CheckMissileSpawn
// Moves the missile forward a bit and possibly explodes it right there.
// -----------------------------------------------------------------------------

void P_CheckMissileSpawn (mobj_t *th)
{
    th->tics -= P_Random()&3;
    if (th->tics < 1)
    {
        th->tics = 1;
    }

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y))
    {
        P_ExplodeMissile (th);
    }
}

// -----------------------------------------------------------------------------
// P_SubstNullMobj
//
// Certain functions assume that a mobj_t pointer is non-NULL, causing a crash
// in some situations where it is NULL. Vanilla Doom did not crash because of
// the lack of proper memory protection. This function substitutes NULL 
// pointers for pointers to a dummy mobj, to avoid a crash.
// -----------------------------------------------------------------------------

mobj_t *P_SubstNullMobj(mobj_t *mobj)
{
    if (mobj == NULL)
    {
        static mobj_t dummy_mobj;

        dummy_mobj.x = 0;
        dummy_mobj.y = 0;
        dummy_mobj.z = 0;
        dummy_mobj.flags = 0;

        mobj = &dummy_mobj;
    }

    return mobj;
}

// -----------------------------------------------------------------------------
// P_SpawnMissile
// -----------------------------------------------------------------------------

mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type)
{
    int      dist;
    angle_t  an;
    mobj_t  *th;

    th = P_SpawnMobj (source->x, source->y, source->z + 4*8*FRACUNIT, type);

    if (th->info->seesound)
    {
        S_StartSound (th, th->info->seesound);
    }

    th->target = source;  // where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

    // fuzzy player
    if (dest->flags & MF_SHADOW)
    {
        an += P_SubRandom() << 20;
    }

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
    {
        dist = 1;
    }

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}

// -----------------------------------------------------------------------------
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster.
// -----------------------------------------------------------------------------

void P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type)
{
    mobj_t  *th;
    angle_t  an;
    fixed_t  x;
    fixed_t  y;
    fixed_t  z;
    fixed_t  slope;

    // see which target is to be aimed at
    an = source->angle;
    slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

        if (!linetarget)
        {
            an -= 2<<26;
            slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
        }

        if (!linetarget)
        {
            an = source->angle;
            slope = 0;
        }

        // [JN] Mouselook: also count vertical angles
        if (singleplayer && !linetarget && mlook)
        {
            an = source->angle;

            if (aspect_ratio >= 2)
            {
                // [JN] Wide screen: new magic number :(
                slope = ((source->player->lookdir / MLOOKUNIT) << FRACBITS) / 180;
            }
            else
            {
                slope = ((source->player->lookdir / MLOOKUNIT) << FRACBITS) / (screenblocks <= 10 ? 160 : 146);
            }
        }
    }

    x = source->x;
    y = source->y;
    z = source->z + 4*8*FRACUNIT;

    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
    {
        S_StartSound (th, th->info->seesound);
    }

    th->target = source;
    th->angle = an;
    th->momx = FixedMul( th->info->speed, finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul( th->info->speed, finesine[an>>ANGLETOFINESHIFT]);
    th->momz = FixedMul( th->info->speed, slope);

    P_CheckMissileSpawn (th);
}

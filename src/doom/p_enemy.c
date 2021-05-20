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
//	Enemy thinking, AI.
//	Action Pointer Functions
//	that are associated with states/frames. 
//



#include <stdio.h>
#include <stdlib.h>

#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "g_game.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "sounds.h"

#include "crispy.h"
#include "jn.h"


typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
    
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);


//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

mobj_t*		soundtarget;

void
P_RecursiveSound
( sector_t*	sec,
  int		soundblocks )
{
    int		i;
    sector_t*	other;
	
    // wake up all monsters in this sector
    if (sec->validcount == validcount
	&& sec->soundtraversed <= soundblocks+1)
    {
	return;		// already flooded
    }
    
    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;
	
    for (i=0 ;i<sec->linecount ; i++)
    {
	line_t* check = sec->lines[i];
	if (! (check->flags & ML_TWOSIDED) )
	    continue;
	
	P_LineOpening (check);

	if (openrange <= 0)
	    continue;	// closed door
	
	if ( sides[ check->sidenum[0] ].sector == sec)
	    other = sides[ check->sidenum[1] ] .sector;
	else
	    other = sides[ check->sidenum[0] ].sector;
	
	if (check->flags & ML_SOUNDBLOCK)
	{
	    if (!soundblocks)
		P_RecursiveSound (other, 1);
	}
	else
	    P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void
P_NoiseAlert
( mobj_t*	target,
  mobj_t*	emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}




//
// P_CheckMeleeRange
//
boolean P_CheckMeleeRange (mobj_t*	actor)
{
    mobj_t*	pl;
    fixed_t	dist;
	
    if (!actor->target)
	return false;
		
    pl = actor->target;
    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

    if (dist >= MELEERANGE-20*FRACUNIT+pl->info->radius)
	return false;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
							
    // [crispy] height check for melee attacks (from Hexen)
    if (singleplayer && !vanillaparm && over_under && pl->player)
    {
	// [crispy] Target is higher than the attacker
	if (pl->z > actor->z + actor->height)
	{
	    return false;
	}
	// [crispy] Attacker is higher
	else if (actor->z > pl->z + pl->height)
	{
	    return false;
	}
    }

    return true;		
}

//
// P_CheckMissileRange
//
boolean P_CheckMissileRange (mobj_t* actor)
{
    fixed_t	dist;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
	
    if ( actor->flags & MF_JUSTHIT )
    {
	// the target just hit the enemy,
	// so fight back!
	actor->flags &= ~MF_JUSTHIT;
	return true;
    }
	
    if (actor->reactiontime)
	return false;	// do not attack yet
		
    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance ( actor->x-actor->target->x,
			     actor->y-actor->target->y) - 64*FRACUNIT;
    
    if (!actor->info->meleestate)
	dist -= 128*FRACUNIT;	// no melee attack, so fire more

    dist >>= FRACBITS;

    if (actor->type == MT_VILE)
    {
        if (dist > 14*64)
        return false;   // too far away            
    }
	

    if (actor->type == MT_UNDEAD)
    {
	if (dist < 196)	
	    return false;	// close for fist attack
	dist >>= 1;
    }
	

    if (actor->type == MT_CYBORG
	|| actor->type == MT_SPIDER
	|| actor->type == MT_SKULL)
    {
	dist >>= 1;
    }
    
    if (dist > 200)
	dist = 200;
		
    if (actor->type == MT_CYBORG && dist > 160)
	dist = 160;
		
    if (P_Random () < dist)
	return false;
		
    return true;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

boolean P_Move (mobj_t*	actor)
{
    fixed_t	tryx;
    fixed_t	tryy;
    
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    boolean	try_ok;
    boolean	good;
		
    if (actor->movedir == DI_NODIR)
	return false;
		
    if ((unsigned)actor->movedir >= 8)
	I_Error (english_language ?
             "Weird actor->movedir!" :
             "Некорректный actor->movedir!");
		
    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

    try_ok = P_TryMove (actor, tryx, tryy);

    if (!try_ok)
    {
	// open any specials
	if (actor->flags & MF_FLOAT && floatok)
	{
	    // must adjust height
	    if (actor->z < tmfloorz)
		actor->z += FLOATSPEED;
	    else
		actor->z -= FLOATSPEED;

	    actor->flags |= MF_INFLOAT;
	    return true;
	}
		
	if (!numspechit)
	    return false;
			
	actor->movedir = DI_NODIR;
	good = false;

    // [JN] Исправление бага с "застревающими" монстрами в дверных проемах.
    // https://doomwiki.org/wiki/Monsters_stuck_in_doortracks,_walls_or_hanging_off_lifts
    // Вызывает рассинхронизацию внутренних демозаписей, поэтому строго для одиночной игры.
    // Адаптировано из Doom Retro (src/p_enemy.c), спасибо Брэду Хардингу!

    // if the special is not a door that can be opened, return false
    //
    // killough 8/9/98: this is what caused monsters to get stuck in
    // doortracks, because it thought that the monster freed itself
    // by opening a door, even if it was moving towards the doortrack,
    // and not the door itself.
    //
    // killough 9/9/98: If a line blocking the monster is activated,
    // return true 90% of the time. If a line blocking the monster is
    // not activated, but some other line is, return false 90% of the
    // time. A bit of randomness is needed to ensure it's free from
    // lockups, but for most cases, it returns the correct result.
    //
    // Do NOT simply return false 1/4th of the time (causes monsters to
    // back out when they shouldn't, and creates secondary stickiness).
    if (singleplayer && !vanillaparm)
    {
        line_t *blockline = spechit[numspechit];

        for (good = false; numspechit--;)
            if (P_UseSpecialLine(actor, spechit[numspechit], 0))
                good |= spechit[numspechit] == blockline ? 1 : 2;

        return (good && ((M_Random() >= 230) ^ (good & 1)));
    }
    
	while (numspechit--)
	{
	    line_t *ld = spechit[numspechit];
	    // if the special is not a door
	    // that can be opened,
	    // return false
	    if (P_UseSpecialLine (actor, ld,0))
		good = true;
	}
	return good;
    }
    else
    {
	actor->flags &= ~MF_INFLOAT;
    }
	
	
    if (! (actor->flags & MF_FLOAT) )	
	actor->z = actor->floorz;
    return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
boolean P_TryWalk (mobj_t* actor)
{	
    if (!P_Move (actor))
    {
	return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}




void P_NewChaseDir (mobj_t*	actor)
{
    fixed_t	deltax;
    fixed_t	deltay;
    
    dirtype_t	d[3];
    
    int		tdir;
    dirtype_t	olddir;
    
    dirtype_t	turnaround;

    if (!actor->target)
	I_Error (english_language ?
             "P_NewChaseDir: called with no target" :
             "P_NewChaseDir: вызов без определения цели");
		
    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
	d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
	d[1]= DI_WEST;
    else
	d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
	d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
	d[2]= DI_NORTH;
    else
	d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR
	&& d[2] != DI_NODIR)
    {
	actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
	if (actor->movedir != (int) turnaround && P_TryWalk(actor))
	    return;
    }

    // try other directions
    if (P_Random() > 200
	||  abs(deltay)>abs(deltax))
    {
	tdir=d[1];
	d[1]=d[2];
	d[2]=tdir;
    }

    if (d[1]==turnaround)
	d[1]=DI_NODIR;
    if (d[2]==turnaround)
	d[2]=DI_NODIR;
	
    if (d[1]!=DI_NODIR)
    {
	actor->movedir = d[1];
	if (P_TryWalk(actor))
	{
	    // either moved forward or attacked
	    return;
	}
    }

    if (d[2]!=DI_NODIR)
    {
	actor->movedir =d[2];

	if (P_TryWalk(actor))
	    return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
	actor->movedir =olddir;

	if (P_TryWalk(actor))
	    return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
	for ( tdir=DI_EAST;
	      tdir<=DI_SOUTHEAST;
	      tdir++ )
	{
	    if (tdir != (int) turnaround)
	    {
		actor->movedir =tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }
    else
    {
	for ( tdir=DI_SOUTHEAST;
	      tdir != (DI_EAST-1);
	      tdir-- )
	{
	    if (tdir != (int) turnaround)
	    {
		actor->movedir = tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }

    if (turnaround !=  DI_NODIR)
    {
	actor->movedir =turnaround;
	if ( P_TryWalk(actor) )
	    return;
    }

    actor->movedir = DI_NODIR;	// can not move
}



//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
boolean
P_LookForPlayers
( mobj_t*	actor,
  boolean	allaround )
{
    int		c;
    int		stop;
    player_t*	player;
    angle_t	an;
    fixed_t	dist;

    c = 0;
    stop = (actor->lastlook-1)&3;
	
    for ( ; ; actor->lastlook = (actor->lastlook+1)&3 )
    {
	if (!playeringame[actor->lastlook])
	    continue;
			
	if (c++ == 2
	    || actor->lastlook == stop)
	{
	    // done looking
	    return false;	
	}
	
	player = &players[actor->lastlook];

	if (player->health <= 0)
	    continue;		// dead

	if (!P_CheckSight (actor, player->mo))
	    continue;		// out of sight
			
	if (!allaround)
	{
	    an = R_PointToAngle2 (actor->x,
				  actor->y, 
				  player->mo->x,
				  player->mo->y)
		- actor->angle;
	    
	    if (an > ANG90 && an < ANG270)
	    {
		dist = P_AproxDistance (player->mo->x - actor->x,
					player->mo->y - actor->y);
		// if real close, react anyway
		if (dist > MELEERANGE)
		    continue;	// behind back
	    }
	}
		
	actor->target = player->mo;
	return true;
    }

    return false;
}


//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie (mobj_t* mo)
{
    thinker_t*	th;
    mobj_t*	mo2;
    line_t	junk;

    A_Fall (mo);
    
    // scan the remaining thinkers
    // to see if all Keens are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;

	mo2 = (mobj_t *)th;
	if (mo2 != mo
	    && mo2->type == mo->type
	    && mo2->health > 0)
	{
	    // other Keen not dead
	    return;		
	}
    }

    junk.tag = 666;
    EV_DoDoor(&junk, vld_open);
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look (mobj_t* actor)
{
    mobj_t*	targ;
	
    actor->threshold = 0;	// any shot will wake up
    targ = actor->subsector->sector->soundtarget;

    if (targ
	&& (targ->flags & MF_SHOOTABLE) )
    {
	actor->target = targ;

	if ( actor->flags & MF_AMBUSH )
	{
	    if (P_CheckSight (actor, actor->target))
		goto seeyou;
	}
	else
	    goto seeyou;
    }
	
	
    if (!P_LookForPlayers (actor, false) )
	return;
		
    // go into chase state
  seeyou:
    if (actor->info->seesound)
    {
	int		sound;
		
	switch (actor->info->seesound)
	{
	  case sfx_posit1:
	  case sfx_posit2:
	  case sfx_posit3:
	    sound = sfx_posit1+P_Random()%3;
	    break;

	  case sfx_bgsit1:
	  case sfx_bgsit2:
	    sound = sfx_bgsit1+P_Random()%2;
	    break;

	  default:
	    sound = actor->info->seesound;
	    break;
	}

	if (actor->type==MT_SPIDER
	    || actor->type == MT_CYBORG)
	{
	    // full volume
	    S_StartSound (NULL, sound);
	}
	else
	    S_StartSound (actor, sound);
    }

    // [JN] Original id Software's idea: 
    // If a monster yells at a player, it will 
    // alert other monsters to the player.
    if (singleplayer && !vanillaparm && noise_alert_sfx)
    {
        P_NoiseAlert (actor->target, actor);
    }
    
    P_SetMobjState (actor, actor->info->seestate);
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t*	actor)
{
    int		delta;

    if (actor->reactiontime)
	actor->reactiontime--;
				

    // modify target threshold
    if  (actor->threshold)
    {
	if (!actor->target
	    || actor->target->health <= 0)
	{
	    actor->threshold = 0;
	}
	else
	    actor->threshold--;
    }
    
    // [JN] Ultra Nightmare speed multipliers.
    // Adapted from Heretic, thanks Raven Software!
    if (gameskill == sk_ultranm 
    && actor->type != MT_SERGEANT
    && actor->type != MT_SHADOWS
    && actor->type != MT_UNDEAD
    && actor->type != MT_VILE)
    {
        actor->tics -= actor->tics / 2;
        if (actor->tics < 3)
        {
            actor->tics = 3;
        }
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
	actor->angle &= (7<<29);
	delta = actor->angle - (actor->movedir << 29);
	
	if (delta > 0)
	    actor->angle -= ANG90/2;
	else if (delta < 0)
	    actor->angle += ANG90/2;
    }

    if (!actor->target
	|| !(actor->target->flags&MF_SHOOTABLE))
    {
	// look for a new target
	if (P_LookForPlayers(actor,true))
	    return; 	// got a new target
	
	P_SetMobjState (actor, actor->info->spawnstate);
	return;
    }
    
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
	actor->flags &= ~MF_JUSTATTACKED;
	if (gameskill != sk_nightmare && gameskill != sk_ultranm && !fastparm)
	    P_NewChaseDir (actor);
	return;
    }
    
    // check for melee attack
    if (actor->info->meleestate
	&& P_CheckMeleeRange (actor))
    {
	if (actor->info->attacksound)
	    S_StartSound (actor, actor->info->attacksound);

	P_SetMobjState (actor, actor->info->meleestate);
	return;
    }
    
    // check for missile attack
    if (actor->info->missilestate)
    {
	if (gameskill < sk_nightmare
	    && !fastparm && actor->movecount)
	{
	    goto nomissile;
	}
	
	if (!P_CheckMissileRange (actor))
	    goto nomissile;
	
	P_SetMobjState (actor, actor->info->missilestate);
	actor->flags |= MF_JUSTATTACKED;
	return;
    }

    // ?
  nomissile:
    // possibly choose another target
    if (netgame
	&& !actor->threshold
	&& !P_CheckSight (actor, actor->target) )
    {
	if (P_LookForPlayers(actor,true))
	    return;	// got a new target
    }
    
    // chase towards player
    if (--actor->movecount<0
	|| !P_Move (actor))
    {
	P_NewChaseDir (actor);
    }
    
    // make active sound
    if (actor->info->activesound
	&& P_Random () < 3)
    {
	S_StartSound (actor, actor->info->activesound);
    }
}


//
// A_FaceTarget
//
void A_FaceTarget (mobj_t* actor)
{	
    if (!actor->target)
	return;
    
    actor->flags &= ~MF_AMBUSH;
	
    actor->angle = R_PointToAngle2 (actor->x,
				    actor->y,
				    actor->target->x,
				    actor->target->y);
    
    if (actor->target->flags & MF_SHADOW)
	actor->angle += P_SubRandom() << 21;
}


//
// A_PosAttack
//
void A_PosAttack (mobj_t* actor)
{
    int		angle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    angle = actor->angle;
    slope = P_AimLineAttack (actor, angle, MISSILERANGE);

    S_StartSound (actor, sfx_pistol);
    angle += P_SubRandom() << 20;
    
    if (gameskill == sk_ultranm)
    {
        damage = ((P_Random()%5)+1)*4;
    }
    else
    {
        damage = ((P_Random()%5)+1)*3;
    }

    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack (mobj_t* actor)
{
    int		i;
    int		bangle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;

    S_StartSound (actor, sfx_shotgn);
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

    for (i=0 ; i<3 ; i++)
    {
        int angle = bangle + (P_SubRandom() << 20);

        if (gameskill == sk_ultranm)
        {
            damage = ((P_Random()%5)+1)*4;
        }
        else
        {
            damage = ((P_Random()%5)+1)*3;
        }

        P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
    }        
}

void A_CPosAttack (mobj_t* actor)
{
    int		angle;
    int		bangle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;

    S_StartSound (actor, sfx_shotgn);
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

    angle = bangle + (P_SubRandom() << 20);

    if (gameskill == sk_ultranm)
    {
        damage = ((P_Random()%5)+1)*4;
    }
    else
    {
        damage = ((P_Random()%5)+1)*3;
    }

    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire (mobj_t* actor)
{	
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if (P_Random () < 40)
	return;

    if (!actor->target
	|| actor->target->health <= 0
	|| !P_CheckSight (actor, actor->target) )
    {
	P_SetMobjState (actor, actor->info->seestate);
    }
}


void A_SpidRefire (mobj_t* actor)
{	
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if (P_Random () < 10)
	return;

    if (!actor->target
	|| actor->target->health <= 0
	|| !P_CheckSight (actor, actor->target) )
    {
	P_SetMobjState (actor, actor->info->seestate);
    }
}

void A_BspiAttack (mobj_t *actor)
{	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);

    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_ARACHPLAZ);
}


//
// A_TroopAttack
//
void A_TroopAttack (mobj_t* actor)
{
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
    int damage = (P_Random()%8+1)*3;
    S_StartSound (actor, sfx_claw);
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }

    
    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


void A_SargAttack (mobj_t* actor)
{
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
	int damage = ((P_Random()%10)+1)*4;
	P_DamageMobj (actor->target, actor, actor, damage);
    }
}

void A_HeadAttack (mobj_t* actor)
{
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
	int damage = (P_Random()%6+1)*10;
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }
    
    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack (mobj_t* actor)
{	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    P_SpawnMissile (actor, actor->target, MT_ROCKET);
}


void A_BruisAttack (mobj_t* actor)
{
    if (!actor->target)
	return;

    // [JN] Исправление оригинального бага с отсутствующим A_FaceTarget
    // https://doomwiki.org/wiki/Baron_attacks_a_monster_behind_him
    if (singleplayer && !vanillaparm)
    {
        A_FaceTarget (actor);
    }
    
    if (P_CheckMeleeRange (actor))
    {
	int damage = (P_Random()%8+1)*10;
	S_StartSound (actor, sfx_claw);
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }
    
    // launch a missile
    P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}


//
// A_SkelMissile
//
void A_SkelMissile (mobj_t* actor)
{	
    mobj_t*	mo;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    actor->z += 16*FRACUNIT;	// so missile spawns higher
    mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
    actor->z -= 16*FRACUNIT;	// back to normal

    mo->x += mo->momx;
    mo->y += mo->momy;
    mo->tracer = actor->target;
}

int	TRACEANGLE = 0xc000000;

void A_Tracer (mobj_t* actor)
{
    angle_t	exact;
    fixed_t	dist;
    fixed_t	slope;
    mobj_t*	dest;
    mobj_t*	th;

    // killough 1/18/98: this is why some missiles do not have smoke
    // and some do. Also, internal demos start at random gametics, thus
    // the bug in which revenants cause internal demos to go out of sync.
    //
    // killough 3/6/98: fix revenant internal demo bug by subtracting
    // levelstarttic from gametic:

    if (singleplayer)
    {
        if (leveltime & 3)
        return;
    }
    else
    {
        if (gametic & 3)
        return;
    }

    // spawn a puff of smoke behind the rocket		
    P_SpawnPuff (actor->x, actor->y, actor->z);
	
    th = P_SpawnMobj (actor->x-actor->momx,
		      actor->y-actor->momy,
		      actor->z, MT_SMOKE);
    
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;
    if (th->tics < 1)
	th->tics = 1;
    
    // adjust direction
    dest = actor->tracer;
	
    if (!dest || dest->health <= 0)
	return;
    
    // change angle	
    exact = R_PointToAngle2 (actor->x,
			     actor->y,
			     dest->x,
			     dest->y);

    if (exact != actor->angle)
    {
	if (exact - actor->angle > 0x80000000)
	{
	    actor->angle -= TRACEANGLE;
	    if (exact - actor->angle < 0x80000000)
		actor->angle = exact;
	}
	else
	{
	    actor->angle += TRACEANGLE;
	    if (exact - actor->angle > 0x80000000)
		actor->angle = exact;
	}
    }
	
    exact = actor->angle>>ANGLETOFINESHIFT;
    actor->momx = FixedMul (actor->info->speed, finecosine[exact]);
    actor->momy = FixedMul (actor->info->speed, finesine[exact]);
    
    // change slope
    dist = P_AproxDistance (dest->x - actor->x,
			    dest->y - actor->y);
    
    dist = dist / actor->info->speed;

    if (dist < 1)
	dist = 1;
    slope = (dest->z+40*FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
	actor->momz -= FRACUNIT/8;
    else
	actor->momz += FRACUNIT/8;
}


void A_SkelWhoosh (mobj_t*	actor)
{
    if (!actor->target)
	return;
    A_FaceTarget (actor);
    S_StartSound (actor,sfx_skeswg);
}

void A_SkelFist (mobj_t*	actor)
{
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
	
    if (P_CheckMeleeRange (actor))
    {
	int damage = ((P_Random()%10)+1)*6;
	S_StartSound (actor, sfx_skepch);
	P_DamageMobj (actor->target, actor, actor, damage);
    }
}



//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t*		corpsehit;
mobj_t*		vileobj;
fixed_t		viletryx;
fixed_t		viletryy;

boolean PIT_VileCheck (mobj_t*	thing)
{
    int		maxdist;
    boolean	check;
	
    if (!(thing->flags & MF_CORPSE) )
	return true;	// not a monster
    
    if (thing->tics != -1)
	return true;	// not lying still yet
    
    if (thing->info->raisestate == S_NULL)
	return true;	// monster doesn't have a raise state
    
    maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;
	
    if ( abs(thing->x - viletryx) > maxdist
	 || abs(thing->y - viletryy) > maxdist )
	return true;		// not actually touching
		
    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;
    corpsehit->height <<= 2;
    check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);
    corpsehit->height >>= 2;

    if (!check)
	return true;		// doesn't fit here
		
    return false;		// got one, so stop checking
}



//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase (mobj_t* actor)
{
    mobjinfo_t*		info;
    mobj_t*		temp;
	
    if (actor->movedir != DI_NODIR)
    {
        int xl = (viletryx - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
        int xh = (viletryx - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
        int yl = (viletryy - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
        int yh = (viletryy - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;
        int bx, by;

	// check for corpses to raise
	viletryx =
	    actor->x + actor->info->speed*xspeed[actor->movedir];
	viletryy =
	    actor->y + actor->info->speed*yspeed[actor->movedir];

	vileobj = actor;
	for (bx=xl ; bx<=xh ; bx++)
	{
	    for (by=yl ; by<=yh ; by++)
	    {
		// Call PIT_VileCheck to check
		// whether object is a corpse
		// that canbe raised.
		if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
		{
		    // got one!
		    temp = actor->target;
		    actor->target = corpsehit;
		    A_FaceTarget (actor);
		    actor->target = temp;
					
		    P_SetMobjState (actor, S_VILE_HEAL1);
		    S_StartSound (corpsehit, sfx_slop);
		    info = corpsehit->info;
		    
		    P_SetMobjState (corpsehit,info->raisestate);
		    corpsehit->height <<= 2;
		    corpsehit->flags = info->flags;
		    corpsehit->health = info->spawnhealth;
		    corpsehit->target = NULL;
            
            if (corpsehit->height == 0 && corpsehit->radius == 0)
            corpsehit->flags |= MF_TRANSLUCENT;

		    return;
		}
	    }
	}
    }

    // Return to normal attack.
    A_Chase (actor);
}


//
// A_VileStart
//
void A_VileStart (mobj_t* actor)
{
    S_StartSound (actor, sfx_vilatk);
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire (mobj_t* actor);

void A_StartFire (mobj_t* actor)
{
    // [JN] Not called by the engine
    // S_StartSound(actor,sfx_flamst);
    A_Fire(actor);
}

void A_FireCrackle (mobj_t* actor)
{
    S_StartSound(actor,sfx_flame);
    A_Fire(actor);
}

void A_Fire (mobj_t* actor)
{
    mobj_t*	dest;
    mobj_t*     target;
    unsigned	an;
		
    dest = actor->tracer;
    if (!dest)
	return;

    target = P_SubstNullMobj(actor->target);
		
    // don't move it if the vile lost sight
    if (!P_CheckSight (target, dest) )
	return;

    an = dest->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition (actor);
    actor->x = dest->x + FixedMul (24*FRACUNIT, finecosine[an]);
    actor->y = dest->y + FixedMul (24*FRACUNIT, finesine[an]);
    actor->z = dest->z;
    P_SetThingPosition (actor);
    // [crispy] update the Archvile fire's floorz and ceilingz values
    // to prevent it from jumping back and forth between the floor heights
    // of its (faulty) spawn sector and the target's actual sector.
    // Thanks to Quasar for his excellent analysis at
    // https://www.doomworld.com/vb/post/1297952
    if (singleplayer && !vanillaparm)
    {
        actor->floorz = actor->subsector->sector->floorheight;
        actor->ceilingz = actor->subsector->sector->ceilingheight;
    }
}



//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget (mobj_t*	actor)
{
    mobj_t*	fog;
	
    if (!actor->target)
	return;

    A_FaceTarget (actor);

    fog = P_SpawnMobj (actor->target->x,
                       actor->target->x,
                       actor->target->z, MT_FIRE);
    
    actor->tracer = fog;
    fog->target = actor;
    fog->tracer = actor->target;

    // [F.G.B.H.J.N.] play D(S/P)FLAMST sound when Arch-Vile spawns fire attack
    if (!vanillaparm)
    S_StartSound(fog, sfx_flamst);

    A_Fire (fog);
}




//
// A_VileAttack
//
void A_VileAttack (mobj_t* actor)
{	
    mobj_t*	fire;
    int		an;
	
    if (!actor->target)
	return;
    
    A_FaceTarget (actor);

    if (!P_CheckSight (actor, actor->target) )
	return;

    S_StartSound (actor, sfx_barexp);
    P_DamageMobj (actor->target, actor, actor, 20);
    actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;
	
    an = actor->angle >> ANGLETOFINESHIFT;

    fire = actor->tracer;

    if (!fire)
	return;
		
    // move the fire between the vile and the player
    fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
    fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);	
    P_RadiusAttack (fire, actor, 70 );
}




//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it. 
//
#define	FATSPREAD	(ANG90/8)

void A_FatRaise (mobj_t *actor)
{
    A_FaceTarget (actor);
    S_StartSound (actor, sfx_manatk);
}


void A_FatAttack1 (mobj_t* actor)
{
    mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);

    // Change direction  to ...
    actor->angle += FATSPREAD;
    target = P_SubstNullMobj(actor->target);
    P_SpawnMissile (actor, target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);
}

void A_FatAttack2 (mobj_t* actor)
{
    mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);
    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    target = P_SubstNullMobj(actor->target);
    P_SpawnMissile (actor, target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD*2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);
}

void A_FatAttack3 (mobj_t*	actor)
{
    mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);

    target = P_SubstNullMobj(actor->target);
    
    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD/2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD/2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define	SKULLSPEED		(20*FRACUNIT)

// killough 10/98: this emulates the beta version's lost soul attacks
void A_SkullAttackBeta (mobj_t* actor)
{
    int		damage;

    if (!actor->target || actor->target->type == MT_SKULL)
	return;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    damage = (P_Random()%8+1)*actor->info->damage;
    P_DamageMobj(actor->target, actor, actor, damage);

    // [JN] Return to see state, otherwise Lost Sould will be freezed
    P_SetMobjState (actor, actor->info->seestate);
}

void A_SkullAttack (mobj_t* actor)
{
    mobj_t*		dest;
    angle_t		an;
    int			dist;

    if (gamemode == pressbeta)
	return A_SkullAttackBeta (actor);

    if (!actor->target)
	return;
		
    dest = actor->target;	
    actor->flags |= MF_SKULLFLY;

    S_StartSound (actor, actor->info->attacksound);
    A_FaceTarget (actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul (SKULLSPEED, finesine[an]);
    dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
    dist = dist / SKULLSPEED;
    
    if (dist < 1)
	dist = 1;
    actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}


//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void
A_PainShootSkull
( mobj_t*	actor,
  angle_t	angle )
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	z;
    
    mobj_t*	newmobj;
    angle_t	an;
    int		prestep;
    int		count;
    thinker_t*	currentthinker;

    // count total number of skull currently on the level
    count = 0;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
	if (   (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
	    && ((mobj_t *)currentthinker)->type == MT_SKULL)
	    count++;
	currentthinker = currentthinker->next;
    }

    // [JN] Optionally remove Lost Souls spawn limit.
    if (singleplayer && !vanillaparm)
    {
        if ((unlimited_lost_souls && count > 10240)
        || (!unlimited_lost_souls && count > 20))
        return;
    }
    else
    {
        // if there are allready 20 skulls on the level,
        // don't spit another one
        if (count > 20)
        return;
    }

    // okay, there's playe for another one
    an = angle >> ANGLETOFINESHIFT;
    
    prestep =
	4*FRACUNIT
	+ 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;
    
    x = actor->x + FixedMul (prestep, finecosine[an]);
    y = actor->y + FixedMul (prestep, finesine[an]);
    z = actor->z + 8*FRACUNIT;
		
    // [Doom Retro] Check whether the Lost Soul is being fired through a 1-sided
    // wall or an impassible line, or a "monsters can't cross" line.
    // If it is, then we don't allow the spawn.
    if (singleplayer)
    {
        if (P_CheckLineSide(actor, x, y))
        return;
    }

    newmobj = P_SpawnMobj (x , y, z, MT_SKULL);

    // Check for movements.
    if (!P_TryMove (newmobj, newmobj->x, newmobj->y)
    // [Doom Retro] Check to see if the new Lost Soul's z value is above the
    // ceiling of its new sector, or below the floor. If so, kill it.
    || ((newmobj->z > newmobj->subsector->sector->ceilingheight - newmobj->height
    || newmobj->z < newmobj->subsector->sector->floorheight) && singleplayer))
    {
        // kill it immediately
        P_DamageMobj (newmobj,actor,actor,10000);	
        return;
    }
		
    newmobj->target = actor->target;
    A_SkullAttack (newmobj);
}


//
// A_PainAttack
// Spawn a lost soul and launch it at the target
// 
void A_PainAttack (mobj_t* actor)
{
    if (!actor->target)
	return;

    A_FaceTarget (actor);
    A_PainShootSkull (actor, actor->angle);
}


void A_PainDie (mobj_t* actor)
{
    A_Fall (actor);
    A_PainShootSkull (actor, actor->angle+ANG90);
    A_PainShootSkull (actor, actor->angle+ANG180);
    A_PainShootSkull (actor, actor->angle+ANG270);
}






void A_Scream (mobj_t* actor)
{
    int		sound;
	
    switch (actor->info->deathsound)
    {
      case 0:
	return;
		
      case sfx_podth1:
      case sfx_podth2:
      case sfx_podth3:
	sound = sfx_podth1 + P_Random ()%3;
	break;
		
      case sfx_bgdth1:
      case sfx_bgdth2:
	sound = sfx_bgdth1 + P_Random ()%2;
	break;
	
      default:
	sound = actor->info->deathsound;
	break;
    }

    // Check for bosses.
    if (actor->type==MT_SPIDER
	|| actor->type == MT_CYBORG)
    {
	// full volume
	S_StartSound (NULL, sound);
    }
    else
	S_StartSound (actor, sound);
}


void A_XScream (mobj_t* actor)
{
    S_StartSound (actor, sfx_slop);	
}

void A_Pain (mobj_t* actor)
{
    // [JN] Не прерывать звуки выстрелов при получении ранения
    if (actor->info->doomednum == -1)
    S_StartSound (singleplayer ? NULL : actor, actor->info->painsound);

    else if (actor->info->painsound)
	S_StartSound (actor, actor->info->painsound);	
}



void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode (mobj_t* thingy)
{
    P_RadiusAttack(thingy, thingy->target, 128);
}

// Check whether the death of the specified monster type is allowed
// to trigger the end of episode special action.
//
// This behavior changed in v1.9, the most notable effect of which
// was to break uac_dead.wad

static boolean CheckBossEnd(mobjtype_t motype)
{
    if (gameversion < exe_ultimate)
    {
        if (gamemap != 8)
        {
            return false;
        }

        // Baron death on later episodes is nothing special.

        if (motype == MT_BRUISER && gameepisode != 1)
        {
            return false;
        }

        return true;
    }
    else
    {
        // New logic that appeared in Ultimate Doom.
        // Looks like the logic was overhauled while adding in the
        // episode 4 support.  Now bosses only trigger on their
        // specific episode.

	switch(gameepisode)
	{
            case 1:
                return gamemap == 8 && motype == MT_BRUISER;

            case 2:
                return gamemap == 8 && motype == MT_CYBORG;

            case 3:
                return gamemap == 8 && motype == MT_SPIDER;

	    case 4:
                return (gamemap == 6 && motype == MT_CYBORG)
                    || (gamemap == 8 && motype == MT_SPIDER);

            // [crispy] Sigil
            case 5:
                return false; 

            default:
                return gamemap == 8;
	}
    }
}

boolean    flag667;

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath (mobj_t* mo)
{
    thinker_t*	th;
    mobj_t*	mo2;
    line_t	junk;
    int		i;

    // [JN] Atari Jaguar special cases
    if (gamemission == jaguar)
    {
        if (gamemap != 8)
        return;

        if (mo->type != MT_BRUISER)
        return;        
    }
    // [JN] Doom 2, MAP07
    else if (gamemode == commercial)
    {
        if (gamemap != 7)
        return;

        if ((mo->type != MT_FATSO) && (mo->type != MT_BABY))
        return;
    }
    // [JN] Doom 1, ExM8
    else
    {
        if (!CheckBossEnd(mo->type))
        {
            return;
        }
    }

    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i] && players[i].health > 0)
	    break;
    
    if (i==MAXPLAYERS)
	return;	// no one left alive, so do not end game
    
    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;
	
	mo2 = (mobj_t *)th;
	if (mo2 != mo
	    && mo2->type == mo->type
	    && mo2->health > 0)
	{
	    // other boss not dead
	    return;
	}
    }
	
    // victory!
    if ( gamemode == commercial)
    {
	if (gamemap == 7)
	{
	    if (mo->type == MT_FATSO)
	    {
		junk.tag = 666;
		EV_DoFloor(&junk,lowerFloorToLowest);
		return;
	    }
	    
	    if (mo->type == MT_BABY)
	    {
            // [JN] (from DOOM Retro) Активировать подъем 
            // сектора 667 только один раз. Исправляет баг:
            // https://doomwiki.org/wiki/Tag_667#Bugs
	        if (!flag667)
	        {
	            junk.tag = 667;
	            EV_DoFloor(&junk,raiseToTexture);
	            flag667 = true;
	        }

	        return;
	    }
	}
    // [JN] Atari Jaguar: MAP08 special case
	if (gamemission == jaguar && gamemap == 8)
	{
	    if (mo->type == MT_BRUISER)
	    {
		junk.tag = 666;
		EV_DoFloor (&junk, lowerFloorToLowest);
		return;
	    }
	}
    }
    else
    {
	switch(gameepisode)
	{
	  case 1:
	    junk.tag = 666;
	    EV_DoFloor (&junk, lowerFloorToLowest);
	    return;
	    break;
	    
	  case 4:
	    switch(gamemap)
	    {
	      case 6:
		junk.tag = 666;
		EV_DoDoor (&junk, vld_blazeOpen);
		return;
		break;
		
	      case 8:
		junk.tag = 666;
		EV_DoFloor (&junk, lowerFloorToLowest);
		return;
		break;
	    }
	}
    }
	
    G_ExitLevel ();
}


void A_Hoof (mobj_t* mo)
{
    S_StartSound (mo, sfx_hoof);
    A_Chase (mo);
}

void A_Metal (mobj_t* mo)
{
    S_StartSound (mo, sfx_metal);
    A_Chase (mo);
}

void A_BabyMetal (mobj_t* mo)
{
    S_StartSound (mo, sfx_bspwlk);
    A_Chase (mo);
}

void
A_OpenShotgun2
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->mo, sfx_dbopn);
}

void
A_LoadShotgun2
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->mo, sfx_dbload);
}

void
A_ReFire
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp );

void
A_CloseShotgun2
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->mo, sfx_dbcls);
    A_ReFire(NULL,player,psp); // [crispy] let pspr action pointers get called from mobj states
}


mobj_t**		braintargets = NULL;
int		numbraintargets = 0; // [crispy] initialize
int		braintargeton = 0;
static int	maxbraintargets; // [crispy] remove braintargets limit

void A_BrainAwake (mobj_t* mo)
{
    thinker_t*	thinker;
    mobj_t*	m;
	
    // find all the target spots
    numbraintargets = 0;
    braintargeton = 0;
	
    for (thinker = thinkercap.next ;
	 thinker != &thinkercap ;
	 thinker = thinker->next)
    {
	if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;	// not a mobj

	m = (mobj_t *)thinker;

	if (m->type == MT_BOSSTARGET )
	{
        // [crispy] remove braintargets limit
	    if (numbraintargets == maxbraintargets)
	    {
		maxbraintargets = maxbraintargets ? 2 * maxbraintargets : 32;
		braintargets = I_Realloc(braintargets, maxbraintargets * sizeof(*braintargets));

		if (maxbraintargets > 32)
		    fprintf(stderr, english_language ?
                            "R_BrainAwake: Raised braintargets limit to %d.\n" :
                            "R_BrainAwake: увеличен лимит braintargets до (%d).\n",
                            maxbraintargets);
	    }
        
	    braintargets[numbraintargets] = m;
	    numbraintargets++;
	}
    }
	
    // [JN] Don't break Boss greeting sound by any others
    S_StartSoundNoBreak(sfx_bossit);
    
    // [crispy] no spawn spots available
    if (numbraintargets == 0)
	numbraintargets = -1;
}


void A_BrainPain (mobj_t*	mo)
{
    // [JN] Note: pain sound *shoud* have break.
    // Otherwise, player can get literally deaf by making SSG point blank blast.
    S_StartSound (NULL,sfx_bospn);
}


void A_BrainScream (mobj_t*	mo)
{
    int		x;

    // [JN] Взрывы Иконы греха отцентрированы. Исправление бага:
    // https://doomwiki.org/wiki/Lopsided_final_boss_explosions
    for (x=mo->x - 320*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
	int y = mo->y - 320*FRACUNIT;
	int z = 128 + P_Random()*2*FRACUNIT;
	mobj_t *th = P_SpawnMobj (x,y,z, MT_ROCKET);
	th->momz = P_Random()*512;

	P_SetMobjState (th, S_BRAINEXPLODE1);

	th->tics -= P_Random()&7;
	if (th->tics < 1)
	    th->tics = 1;
    
    th->flags |= MF_TRANSLUCENT;
    }
	
    // [JN] Don't break Boss death sound by any others
    S_StartSoundNoBreak(sfx_bosdth);
}



void A_BrainExplode (mobj_t* mo)
{
    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    x = mo->x +  P_SubRandom() * 2048;
    y = mo->y;
    z = 128 + P_Random()*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = P_Random()*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= P_Random()&7;
    if (th->tics < 1)
	th->tics = 1;
}


void A_BrainDie (mobj_t*	mo)
{
    G_ExitLevel ();
}

void A_BrainSpit (mobj_t*	mo)
{
    mobj_t*	targ;
    mobj_t*	newmobj;
    
    static int	easy = 0;
	
    easy ^= 1;
    if (gameskill <= sk_easy && (!easy))
	return;

    // [crispy] avoid division by zero by recalculating the number of spawn spots
    if (numbraintargets == 0)
	A_BrainAwake(NULL);

    // [crispy] still no spawn spots available
    if (numbraintargets == -1)
	return;
		
    // shoot a cube at current target
    targ = braintargets[braintargeton];
    braintargeton = (braintargeton+1)%numbraintargets;

    // spawn brain missile
    newmobj = P_SpawnMissile (mo, targ, MT_SPAWNSHOT);
    newmobj->target = targ;
    newmobj->reactiontime =
	((targ->y - mo->y)/newmobj->momy) / newmobj->state->tics;

    // [JN] Don't break Boss spitting sound by any others
    S_StartSoundNoBreak(sfx_bospit);
}



void A_SpawnFly (mobj_t* mo);

// travelling cube sound
void A_SpawnSound (mobj_t* mo)	
{
    S_StartSound (mo,sfx_boscub);
    A_SpawnFly(mo);
}

void A_SpawnFly (mobj_t* mo)
{
    mobj_t*	newmobj;
    mobj_t*	fog;
    mobj_t*	targ;
    int		r;
    mobjtype_t	type;
	
    if (--mo->reactiontime)
	return;	// still flying
	
    targ = P_SubstNullMobj(mo->target);

    // First spawn teleport fog.
    fog = P_SpawnMobj (targ->x, targ->y, targ->z, MT_SPAWNFIRE);
    S_StartSound (fog, sfx_telept);

    // Randomly select monster to spawn.
    r = P_Random ();

    // Probability distribution (kind of :),
    // decreasing likelihood.
    if ( r<50 )
	type = MT_TROOP;
    else if (r<90)
	type = MT_SERGEANT;
    else if (r<120)
	type = MT_SHADOWS;
    else if (r<130)
	type = MT_PAIN;
    else if (r<160)
	type = MT_HEAD;
    else if (r<162)
	type = MT_VILE;
    else if (r<172)
	type = MT_UNDEAD;
    else if (r<192)
	type = MT_BABY;
    else if (r<222)
	type = MT_FATSO;
    else if (r<246)
	type = MT_KNIGHT;
    else
	type = MT_BRUISER;		

    newmobj	= P_SpawnMobj (targ->x, targ->y, targ->z, type);
    if (P_LookForPlayers (newmobj, true) )
	P_SetMobjState (newmobj, newmobj->info->seestate);
	
    // telefrag anything in this spot
    P_TeleportMove (newmobj, newmobj->x, newmobj->y);

    // remove self (i.e., cube).
    P_RemoveMobj (mo);
}



void A_PlayerScream (mobj_t* mo)
{
    // Default death sound.
    int		sound = sfx_pldeth;

    // [JN] PDIEHI is also available in retail and newer versions of rigistered
    // and shareware. Hovewer, just in case of missing sound, use it only in
    // Ultimate Doom, Doom 2 and Final Doom.
    if (((gamemode == retail && !vanillaparm) || gamemode == commercial)
    &&  (gamemission != jaguar) // [JN] Sound not present in Jaguar Doom
    && 	(mo->health < -50))
    {
        // IF THE PLAYER DIES
        // LESS THAN -50% WITHOUT GIBBING
        sound = sfx_pdiehi;
    }
    
    S_StartSound (mo, sound);
}

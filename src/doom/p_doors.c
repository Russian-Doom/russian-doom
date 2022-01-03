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
// DESCRIPTION: Door animation code (opening/closing)
//


#include "z_zone.h"
#include "deh_main.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "rd_lang.h"
#include "sounds.h"
#include "jn.h"


// =============================================================================
// VERTICAL DOORS
// =============================================================================

// -----------------------------------------------------------------------------
// T_VerticalDoor
// -----------------------------------------------------------------------------
void T_VerticalDoor (vldoor_t *door)
{
    result_e res;

    switch(door->direction)
    {
        case 0:
        // WAITING
        if (!--door->topcountdown)
        {
            switch(door->type)
            {
                case vld_blazeRaise:
                door->direction = -1; // time to go back down
                // [JN] Z-axis sfx distance: sound invoked from the ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_bdcls);
                break;

                case vld_normal:
                door->direction = -1; // time to go back down
                // [JN] Z-axis sfx distance: sound invoked from the ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_dorcls);
                break;

                case vld_close30ThenOpen:
                door->direction = 1;
                // [JN] Z-axis sfx distance: sound invoked from the ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;

                default:
                break;
            }
        }
        break;

        case 2:
        //  INITIAL WAIT
        if (!--door->topcountdown)
        {
            switch(door->type)
            {
                case vld_raiseIn5Mins:
                door->direction = 1;
                door->type = vld_normal;
                // [JN] Z-axis sfx distance: sound invoked from the ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;

                default:
                break;
            }
        }
        break;

        case -1:
        // DOWN
        res = T_MovePlane(door->sector, 
			              door->speed,
			              door->sector->floorheight, false, 1,
                          door->direction);
        if (res == pastdest)
        {
            switch(door->type)
            {
                case vld_blazeRaise:
                case vld_blazeClose:
                door->sector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);  // unlink and free
                if (blazing_door_fix_sfx && !vanillaparm)
                {
                    // [crispy] fix "fast doors make two closing sounds"
                    break;
                }
                else
                {
                    S_StartSound(&door->sector->soundorg, sfx_bdcls);
                    break;
                }

                case vld_normal:
                case vld_close:
                door->sector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);  // unlink and free
                break;
		
                case vld_close30ThenOpen:
                door->direction = 0;
                door->topcountdown = TICRATE*30;
                break;

                default:
                break;
            }
        }
        else if (res == crushed)
        {
            switch(door->type)
            {
                case vld_blazeClose:
                case vld_close:  // DO NOT GO BACK UP!
                break;

                // [BH] play correct sound when raising fast doors
                case vld_blazeRaise:
                door->direction = 1;
                S_StartSound(&door->sector->soundorg, sfx_bdopn);
                break;

                default:
                door->direction = 1;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;
            }
        }
        break;

        case 1:
        // UP
        res = T_MovePlane(door->sector,
                          door->speed,
                          door->topheight, false, 1,
                          door->direction);

        if (res == pastdest)
        {
            switch(door->type)
            {
                case vld_blazeRaise:
                case vld_normal:
                door->direction = 0; // wait at top
                door->topcountdown = door->topwait;
                break;

                case vld_close30ThenOpen:
                case vld_blazeOpen:
                case vld_open:
                door->sector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);  // unlink and free
                // [JN] Don't play an opening sound if a door is aleady opened.
                S_StopDoorSound(&door->sector->soundorg);
                break;

                default:
                break;
            }
        }
        break;
    }
}

// -----------------------------------------------------------------------------
// EV_DoLockedDoor
// Move a locked door up/down
// -----------------------------------------------------------------------------

int EV_DoLockedDoor (line_t *line, vldoor_e type, mobj_t *thing)
{
    player_t *p = thing->player;

    if (!p)
    {
        return 0;
    }
		
    switch(line->special)
    {
        // Blue Lock
        case 99:
        case 133:
        if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
        {
            p->message_system = DEH_String(pd_blueo);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_bluecard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return 0;
        }
        break;

        // Red Lock
        case 134: 
        case 135:
        if (!p->cards[it_redcard] && !p->cards[it_redskull])
        {
            p->message_system = DEH_String(pd_redo);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_redcard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return 0;
        }
        break;

        // Yellow Lock
        case 136:	
        case 137:
        if (!p->cards[it_yellowcard] && !p->cards[it_yellowskull])
        {
            p->message_system = DEH_String(pd_yellowo);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_yellowcard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return 0;
        }
        break;	
    }

    return EV_DoDoor(line,type);
}

// -----------------------------------------------------------------------------
// EV_DoDoor
// -----------------------------------------------------------------------------

int EV_DoDoor (line_t *line, vldoor_e type)
{
    int       secnum, rtn;
    sector_t *sec;
    vldoor_t *door;

    secnum = -1;
    rtn = 0;

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];

        if (sec->specialdata)
        {
            continue;
        }

        // new door thinker
        rtn = 1;
        door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
        P_AddThinker (&door->thinker);
        sec->specialdata = door;

        door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
        door->sector = sec;
        door->type = type;
        door->topwait = VDOORWAIT;
        door->speed = VDOORSPEED;

        switch(type)
        {
            case vld_blazeClose:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->direction = -1;
            door->speed = VDOORSPEED * 4;
            // [JN] Z-axis sfx distance: sound invoked from the ceiling
            door->sector->soundorg.z = door->sector->ceilingheight;
            S_StartSound(&door->sector->soundorg, sfx_bdcls);
            break;

            case vld_close:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->direction = -1;
            // [JN] Z-axis sfx distance: sound invoked from the ceiling
            door->sector->soundorg.z = door->sector->ceilingheight;
            S_StartSound(&door->sector->soundorg, sfx_dorcls);
            break;

            case vld_close30ThenOpen:
            door->topheight = sec->ceilingheight;
            door->direction = -1;
            // [JN] Z-axis sfx distance: sound invoked from ceiling
            door->sector->soundorg.z = door->sector->ceilingheight;
            S_StartSound(&door->sector->soundorg, sfx_dorcls);
            break;

            case vld_blazeRaise:
            case vld_blazeOpen:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->speed = VDOORSPEED * 4;

            if (door->topheight != sec->ceilingheight)
            {
                // [JN] Z-axis sfx distance: sound invoked from ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_bdopn);
            }
            break;

            case vld_normal:
            case vld_open:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            if (door->topheight != sec->ceilingheight)
            {
                // [JN] Z-axis sfx distance: sound invoked from ceiling
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
            }
            break;

            default:
            break;
        }
    }
    return rtn;
}

// -----------------------------------------------------------------------------
// EV_VerticalDoor : open a door manually, no tag value
// -----------------------------------------------------------------------------

void EV_VerticalDoor (line_t *line, mobj_t *thing)
{
    int       side = 0;	              // only front sides can be used
    player_t *player = thing->player; // Check for locks
    sector_t *sec;
    vldoor_t *door;

    switch (line->special)
    {
        // Blue Lock
        case 26:
        case 32:
        if (!player)
        {
            return;
        }
        if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
        {
            player->message_system = DEH_String(pd_bluek);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_bluecard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return;
        }
        break;

        // Yellow Lock
        case 27:
        case 34:
        if (!player)
        {
            return;
        }
        if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
        {
            player->message_system = DEH_String(pd_yellowk);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_yellowcard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return;
        }
        break;

        // Red Lock
        case 28: 
        case 33:
        if (!player)
        {
            return;
        }
        if (!player->cards[it_redcard] && !player->cards[it_redskull])
        {
            player->message_system = DEH_String(pd_redk);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_redcard] = KEYBLINKTICS;
            if (PTR_NoWayAudible(line))
            {
                S_StartSound(NULL,sfx_oof);
            }
            return;
        }
        break;
    }

    // if the sector has an active thinker, use it

    if (line->sidenum[side^1] == NO_INDEX)
    {
        // [JN, from crispy] Фикс ошибки с дверью DR, назначенной 
        // односторонней линии. Не завершать работу с ошибкой, а
        // просто проигрывать звук "уфф".
        S_StartSound(player->mo,sfx_oof);
        return;
    }

    sec = sides[ line->sidenum[side^1]] .sector;

    if (sec->specialdata)
    {
        door = sec->specialdata;

        switch (line->special)
        {
            case 1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
            case 26:
            case 27:
            case 28:
            case 117:
            if (door->direction == -1)
            {
                door->direction = 1;  // go back up
                // [crispy] play sound effect when the door is opened again while going down
                if (!vanillaparm)
                {
                    // [JN] Z-axis sfx distance: sound invoked from the ceiling
                    door->sector->soundorg.z = door->sector->ceilingheight;
                    S_StartSound(&door->sector->soundorg,
                                 line->special == 117 ? sfx_bdopn : sfx_doropn);
                }
            }
            else
            {
                if (!thing->player)
                {
                    return;  // JDC: bad guys never close doors
                }

                // When is a door not a door?
                // In Vanilla, door->direction is set, even though
                // "specialdata" might not actually point at a door.

                if (door->thinker.function.acp1 == (actionf_p1) T_VerticalDoor)
                {
                    door->direction = -1;	// start going down immediately
                    // [crispy] play sound effect when the door is closed manually
                    if (!vanillaparm)
                    {
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg,
                                     line->special == 117 ? sfx_bdcls : sfx_dorcls);
                    }
                }
                else if (door->thinker.function.acp1 == (actionf_p1) T_PlatRaise)
                {
                    // Erm, this is a plat, not a door.
                    // This notably causes a problem in ep1-0500.lmp where
                    // a plat and a door are cross-referenced; the door
                    // doesn't open on 64-bit.
                    // The direction field in vldoor_t corresponds to the wait
                    // field in plat_t.  Let's set that to -1 instead.

                    plat_t *plat;

                    plat = (plat_t *) door;
                    plat->wait = -1;
                }
                else
                {
                    // This isn't a door OR a plat.  Now we're in trouble.

                    fprintf(stderr, english_language ?
                                    "EV_VerticalDoor: Tried to close something that wasn't a door.\n" :
                                    "EV_VerticalDoor: попытка закрыть что-то, что не является дверью.\n");

                    // Try closing it anyway. At least it will work on 32-bit
                    // machines.

                    door->direction = -1;
                }
            }
            return;
        }
    }

    // for proper sound
    switch (line->special)
    {
        case 117:  // BLAZING DOOR RAISE
        case 118:  // BLAZING DOOR OPEN
        // [JN] Z-axis sfx distance: sound invoked from the ceiling
        sec->soundorg.z = sec->ceilingheight;
        S_StartSound(&sec->soundorg,sfx_bdopn);
        break;

        case 1:  // NORMAL DOOR SOUND
        case 31:
        // [JN] Z-axis sfx distance: sound invoked from the ceiling
        sec->soundorg.z = sec->ceilingheight;
        S_StartSound(&sec->soundorg,sfx_doropn);
        break;

        default:  // LOCKED DOOR SOUND
        // [JN] Z-axis sfx distance: sound invoked from the ceiling
        sec->soundorg.z = sec->ceilingheight;
        S_StartSound(&sec->soundorg,sfx_doropn);
        break;
    }
    
    // new door thinker
    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker (&door->thinker);
    sec->specialdata = door;
    door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

    switch(line->special)
    {
        case 1:
        case 26:
        case 27:
        case 28:
        door->type = vld_normal;
        break;

        case 31:
        case 32:
        case 33:
        case 34:
        door->type = vld_open;
        // [JN] following line is not existing in Jaguar source code,
        // which explains an ability to press "use" multiple times on D1 door
        // types, speed it up and hear opening sound while every "use" action.
        if (gamemission != jaguar)
        {
            line->special = 0;
        }
        break;

        case 117:  // blazing door raise
        door->type = vld_blazeRaise;
        door->speed = VDOORSPEED*4;
        break;

        case 118:  // blazing door open
        door->type = vld_blazeOpen;
        line->special = 0;
        door->speed = VDOORSPEED*4;
        break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
}

// -----------------------------------------------------------------------------
// Spawn a door that closes after 30 seconds
// -----------------------------------------------------------------------------

void P_SpawnDoorCloseIn30 (sector_t *sec)
{
    vldoor_t *door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = vld_normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * TICRATE;
    door->topheight = sec->ceilingheight;
}

// -----------------------------------------------------------------------------
// Spawn a door that opens after 5 minutes
// -----------------------------------------------------------------------------

void P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum)
{
    vldoor_t *door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = vld_raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}

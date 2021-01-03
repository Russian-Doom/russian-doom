//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "sounds.h"
#include "rd_lang.h"
#include "jn.h"


//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
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
                case blazeRaise:
                door->direction = -1; // time to go back down
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);
                break;

                case normal:
                door->direction = -1; // time to go back down
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);
                break;

                case close30ThenOpen:
                door->direction = 1;
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
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
                case raiseIn5Mins:
                door->direction = 1;
                door->type = normal;
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
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
			              door->sector->floorheight,
			              false,1,door->direction);
        if (res == pastdest)
        {
            switch(door->type)
            {
                case blazeRaise:
                case blazeClose:
                door->sector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);  // unlink and free
                // [crispy] fix "fast doors make two closing sounds"
                // [JN] No fix for vanilla option
                if (!blazing_door_fix_sfx || vanilla)
                {
                    S_StartSound((mobj_t *)&door->sector->soundorg,sfx_bdcls);
                    break;
                }
                else
                {
                    break;
                }

            case normal:
            case close:
            door->sector->specialdata = NULL;
            P_RemoveThinker (&door->thinker);  // unlink and free
            break;

            case close30ThenOpen:
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
                case blazeClose:
                case close:     // DO NOT GO BACK UP!
                break;

                if (!vanilla)
                {
                    // [crispy] fix "fast doors reopening with wrong sound"
                    case blazeRaise:
                    door->direction = 1;
                    S_StartSound(&door->sector->soundorg, sfx_bdopn);
                    break;        
                }

                default:
                door->direction = 1;
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
                break;
            }
        }
        break;

        case 1:
        // UP
        res = T_MovePlane(door->sector,
                          door->speed,
                          door->topheight,
                          false,1,door->direction);

        if (res == pastdest)
        {
            switch(door->type)
            {
                case blazeRaise:
                case normal:
                door->direction = 0; // wait at top
                door->topcountdown = door->topwait;
                break;

                case close30ThenOpen:
                case blazeOpen:
                case open:
                door->sector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);  // unlink and free
                break;
		
                default:
                break;
            }
        }
        break;
    }
}


//
// EV_DoLockedDoor
// Move a locked door up/down
//
int EV_DoLockedDoor (line_t *line, vldoor_e type, mobj_t *thing)
{
    player_t *p;

    p = thing->player;

    if (!p)
    {
        return 0;
    }

    switch(line->special)
    {
        case 99:	// Blue Lock
        case 133:
        if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
        {
            p->message_system = pd_blueo;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_bluecard] = KEYBLINKTICS;
            return 0;
        }
        break;

        case 134: // Red Lock
        case 135:
        if (!p->cards[it_redcard] && !p->cards[it_redskull])
        {
            p->message_system = pd_redo;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_redcard] = KEYBLINKTICS;
            return 0;
        }
        break;

        case 136:	// Yellow Lock
        case 137:
        if (!p->cards[it_yellowcard] && !p->cards[it_yellowskull])
        {
            p->message_system = pd_yellowo;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            p->tryopen[it_yellowcard] = KEYBLINKTICS;
            return 0;
        }
        break;	
    }

    return EV_DoDoor(line,type);
}


int EV_DoDoor (line_t *line, vldoor_e type)
{
    int        secnum;
    int        rtn;
    sector_t  *sec;
    vldoor_t  *door;

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
            case blazeClose:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->direction = -1;
            door->speed = VDOORSPEED * 4;
            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);
            break;

            case close:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->direction = -1;
            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);
            break;

            case close30ThenOpen:
            door->topheight = sec->ceilingheight;
            door->direction = -1;
            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);
            break;

            case blazeRaise:
            case blazeOpen:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            door->speed = VDOORSPEED * 4;
            if (door->topheight != sec->ceilingheight)
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdopn);
            break;

            case normal:
            case open:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4*FRACUNIT;
            if (door->topheight != sec->ceilingheight) 
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
            break;

            default:
            break;
        }
    }
    return rtn;
}


//
// EV_VerticalDoor : open a door manually, no tag value
//
void EV_VerticalDoor (line_t *line, mobj_t *thing)
{
    int        secnum;
    int        side;
    player_t  *player;
    sector_t  *sec;
    vldoor_t  *door;

    side = 0;	// only front sides can be used

    //	Check for locks
    player = thing->player;

    switch(line->special)
    {
        case 26: // Blue Lock
        case 32:
        if (!player)
        {
            return;
        }

        if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
        {
            player->message_system = pd_bluek;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_bluecard] = KEYBLINKTICS;
            return;
        }
        break;

        case 27: // Yellow Lock
        case 34:
        if (!player)
        {
            return;
        }

        if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
        {
            player->message_system = pd_yellowk;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_yellowcard] = KEYBLINKTICS;
            return;
        }
        break;

        case 28: // Red Lock
        case 33:
        if (!player)
        {
            return;
        }

        if (!player->cards[it_redcard] && !player->cards[it_redskull])
        {
            player->message_system = pd_redk;
            S_StartSound(NULL,sfx_oof);
            // [crispy] blinking key or skull in the status bar
            player->tryopen[it_redcard] = KEYBLINKTICS;
            return;
        }
        break;
    }

    // if the door action is on a single-sided linedef, ignore
    if (line->sidenum[side^1] == -1)
    {
        // [JN] Use player->mo for sound source, don't play it globally.
        // This also fixes time lag on using this linedef.
        S_StartSound(player->mo,sfx_oof);
        return;
    }

    // if the sector has an active thinker, use it
    sec = sides[ line->sidenum[side^1]] .sector;
    secnum = sec-sectors;

    if (sec->specialdata)
    {
        door = sec->specialdata;

        switch(line->special)
        {
            case 1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
            case 26:
            case 27:
            case 28:
            case 117:
            if (door->direction == -1)
            {
                door->direction = 1;    // go back up
                // [crispy] play sound effect when the door
                // is opened again while going down
                if (!vanilla)
                S_StartSound(&door->sector->soundorg, line->special == 117 ?
                                                      sfx_bdopn : sfx_doropn);
            }
            else
            {
                if (!thing->player)
                {
                    return; // JDC: bad guys never close doors
                }

                door->direction = -1;   // start going down immediately
                // [crispy] play sound effect when the door is closed manually
                if (!vanilla)
                S_StartSound(&door->sector->soundorg, line->special == 117 ?
                                                      sfx_bdcls : sfx_dorcls);
            }
            return;
        }
    }

    // for proper sound
    switch(line->special)
    {
        case 117:   // BLAZING DOOR RAISE
        case 118:   // BLAZING DOOR OPEN
        S_StartSound((mobj_t *)&sec->soundorg,sfx_bdopn);
        break;

        case 1:     // NORMAL DOOR SOUND
        case 31:
        S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn);
        break;

        default:    // LOCKED DOOR SOUND
        S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn);
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
        door->type = normal;
        break;

        case 31:
        case 32:
        case 33:
        case 34:
        door->type = open;
        line->special = 0;
        break;

        case 117:   // blazing door raise
        door->type = blazeRaise;
        door->speed = VDOORSPEED*4;
        break;

        case 118:   // blazing door open
        door->type = blazeOpen;
        line->special = 0;
        door->speed = VDOORSPEED*4;
        break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
}


//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30 (sector_t *sec)
{
    vldoor_t *door;

    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * TICRATE;
    // [JN] Fixed infinite raising issue.
    // Thanks Jeff Doggett for this fix!
    door->topheight = sec->ceilingheight;
}


//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum)
{
    vldoor_t *door;

    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}

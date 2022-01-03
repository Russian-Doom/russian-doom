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
// P_doors.c



#include "doomdef.h"
#include "deh_str.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "jn.h"


//==================================================================
//==================================================================
//
//                                                      VERTICAL DOORS
//
//==================================================================
//==================================================================

//==================================================================
//
//      T_VerticalDoor
//
//==================================================================
void T_VerticalDoor(vldoor_t * door)
{
    result_e res;

    switch (door->direction)
    {
        case 0:                // WAITING
            if (!--door->topcountdown)
                switch (door->type)
                {
                    case vld_normal:
                        door->direction = -1;   // time to go back down
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;
                    case vld_close30ThenOpen:
                        door->direction = 1;
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;
                    default:
                        break;
                }
            break;
        case 2:                // INITIAL WAIT
            if (!--door->topcountdown)
            {
                switch (door->type)
                {
                    case vld_raiseIn5Mins:
                        door->direction = 1;
                        door->type = vld_normal;
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;
                    default:
                        break;
                }
            }
            break;
        case -1:               // DOWN
            res = T_MovePlane(door->sector, door->speed,
                              door->sector->floorheight, false, 1,
                              door->direction);
            if (res == pastdest)
            {
                switch (door->type)
                {
                    case vld_normal:
                    case vld_close:
                        door->sector->specialdata = NULL;
                        P_RemoveThinker(&door->thinker);    // unlink and free
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_dorcls);
                        break;
                    case vld_close30ThenOpen:
                        door->direction = 0;
                        door->topcountdown = 35 * 30;
                        break;
                    default:
                        break;
                }
            }
            else if (res == crushed)
            {
                switch (door->type)
                {
                    case vld_close:        // DON'T GO BACK UP!
                        break;
                    default:
                        door->direction = 1;
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_doropn);
                        break;
                }
            }
            break;
        case 1:                // UP
            res = T_MovePlane(door->sector, door->speed,
                              door->topheight, false, 1, door->direction);
            if (res == pastdest)
            {
                switch (door->type)
                {
                    case vld_normal:
                        door->direction = 0;    // wait at top
                        door->topcountdown = door->topwait;
                        break;
                    case vld_close30ThenOpen:
                    case vld_open:
                        door->sector->specialdata = NULL;
                        P_RemoveThinker(&door->thinker);    // unlink and free
                        S_StopSound(&door->sector->soundorg);
                        break;
                    default:
                        break;
                }
            }
            break;
    }
}

//----------------------------------------------------------------------------
//
// EV_DoDoor
//
// Move a door up/down
//
//----------------------------------------------------------------------------

int EV_DoDoor(line_t * line, vldoor_e type, fixed_t speed)
{
    int secnum;
    int retcode;
    sector_t *sec;
    vldoor_t *door;

    secnum = -1;
    retcode = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
        {
            continue;
        }
        // Add new door thinker
        retcode = 1;
        door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
        P_AddThinker(&door->thinker);
        sec->specialdata = door;
        door->thinker.function = T_VerticalDoor;
        door->sector = sec;
        switch (type)
        {
            case vld_close:
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                door->direction = -1;
                // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;
            case vld_close30ThenOpen:
                door->topheight = sec->ceilingheight;
                door->direction = -1;
                // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                door->sector->soundorg.z = door->sector->ceilingheight;
                S_StartSound(&door->sector->soundorg, sfx_doropn);
                break;
            case vld_normal:
            case vld_open:
                door->direction = 1;
                door->topheight = P_FindLowestCeilingSurrounding(sec);
                door->topheight -= 4 * FRACUNIT;
                if (door->topheight != sec->ceilingheight)
                {
                    // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                    door->sector->soundorg.z = door->sector->ceilingheight;
                    S_StartSound(&door->sector->soundorg, sfx_doropn);
                }
                break;
            default:
                break;
        }
        door->type = type;
        door->speed = speed;
        door->topwait = VDOORWAIT;
    }
    return (retcode);
}

//==================================================================
//
//      EV_VerticalDoor : open a door manually, no tag value
//
//==================================================================
void EV_VerticalDoor(line_t * line, mobj_t * thing)
{
    player_t *player;
    sector_t *sec;
    vldoor_t *door;
    int side;

    side = 0;                   // only front sides can be used
//
//      Check for locks
//
    player = thing->player;
    switch (line->special)
    {
        case 26:               // Blue Lock
        case 32:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_blue])
            {
                P_SetMessage(player, DEH_String(txt_needbluekey), msg_system, false);
                S_StartSound(NULL, sfx_plroof);
                // [JN] Set counter for missing key fading effect.
                player->bluekeyTics = 15;
                return;
            }
            break;
        case 27:               // Yellow Lock
        case 34:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_yellow])
            {
                P_SetMessage(player, DEH_String(txt_needyellowkey), msg_system, false);
                S_StartSound(NULL, sfx_plroof);
                // [JN] Set counter for missing key fading effect.
                player->yellowkeyTics = 15;
                return;
            }
            break;
        case 28:               // Green Lock
        case 33:
            if (!player)
            {
                return;
            }
            if (!player->keys[key_green])
            {
                P_SetMessage(player, DEH_String(txt_needgreenkey), msg_system, false);
                S_StartSound(NULL, sfx_plroof);
                // [JN] Set counter for missing key fading effect.
                player->greenkeyTics = 15;
                return;
            }
            break;
    }

    // if the sector has an active thinker, use it

    // [from-crispy] if the door action is on a single-sided linedef, ignore
    if (line->sidenum[side^1] == NO_INDEX)
    {
        // [JN] Use player->mo for sound source, don't play it globally.
        // This also fixes bombing out on using this linedef.
        S_StartSound(player->mo, sfx_plroof);
        return;
    }

    sec = sides[line->sidenum[side ^ 1]].sector;
    if (sec->specialdata)
    {
        door = sec->specialdata;
        switch (line->special)
        {
            case 1:            // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
            case 26:
            case 27:
            case 28:
                if (door->direction == -1)
                {
                    door->direction = 1;        // go back up

                    // [crispy] & [JN] play sound effect when the door
                    // is opened again while going down
                    if (!vanillaparm)
                    {
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_dormov);
                    }
                }
                else
                {
                    if (!thing->player)
                    {           // Monsters don't close doors
                        return;
                    }
                    door->direction = -1;       // start going down immediately

                    // [crispy] & [JN] play sound effect when the door is closed manually.
                    if (!vanillaparm)
                    {
                        // [JN] Z-axis sfx distance: sound invoked from the ceiling.
                        door->sector->soundorg.z = door->sector->ceilingheight;
                        S_StartSound(&door->sector->soundorg, sfx_dormov);
                    }
                }
                return;
        }
    }

    // for proper sound
    switch (line->special)
    {
        case 1:                // NORMAL DOOR SOUND
        case 31:
            // [JN] Z-axis sfx distance: sound invoked from the ceiling.
            sec->soundorg.z = sec->ceilingheight;
            S_StartSound(&sec->soundorg, sfx_doropn);
        break;
        default:               // LOCKED DOOR SOUND
            // [JN] Z-axis sfx distance: sound invoked from the ceiling.
            sec->soundorg.z = sec->ceilingheight;
            S_StartSound(&sec->soundorg, sfx_doropn);
        break;
    }

    //
    // new door thinker
    //
    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    switch (line->special)
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
            line->special = 0;
        break;
    }
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

    //
    // find the top and bottom of the movement range
    //
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
}

//==================================================================
//
//      Spawn a door that closes after 30 seconds
//
//==================================================================
void P_SpawnDoorCloseIn30(sector_t * sec)
{
    vldoor_t *door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = vld_normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * 35;
}

//==================================================================
//
//      Spawn a door that opens after 5 minutes
//
//==================================================================
void P_SpawnDoorRaiseIn5Mins(sector_t * sec, int secnum)
{
    vldoor_t *door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker(&door->thinker);
    sec->specialdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = vld_raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * 35;
}

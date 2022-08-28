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
//	Floor animation: raising stairs.
//


#include "z_zone.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "jn.h"


// =============================================================================
// FLOORS
// =============================================================================


// -----------------------------------------------------------------------------
// T_MovePlane
// Move a plane (floor or ceiling) and check for crushing
// -----------------------------------------------------------------------------

const result_e T_MovePlane (sector_t *sector, const fixed_t speed, fixed_t dest,
                            const boolean crush, const int floorOrCeiling, const int direction)
{
    boolean	flag;
    fixed_t	lastpos;

    // [AM] Store old sector heights for interpolation.
    sector->oldfloorheight = sector->floorheight;
    sector->oldceilingheight = sector->ceilingheight;
    sector->oldgametic = gametic;

    switch (floorOrCeiling)
    {
        case 0:
        // FLOOR
        switch (direction)
        {
            case -1:
            // DOWN
            if (sector->floorheight - speed < dest)
            {
                lastpos = sector->floorheight;
                sector->floorheight = dest;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->floorheight =lastpos;
                    P_ChangeSector(sector,crush);
                }
                return pastdest;
            }
            else
            {
                lastpos = sector->floorheight;
                sector->floorheight -= speed;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }
            }
            break;

            case 1:
            // [JN] Don't allow platform floor go through the ceiling.
            if (singleplayer && !strict_mode && !vanillaparm)
            {
                dest = dest < sector->ceilingheight ?
                       dest : sector->ceilingheight;
            }

            // UP
            if (sector->floorheight + speed > dest)
            {
                lastpos = sector->floorheight;
                sector->floorheight = dest;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    // [JN] Crush everyone to death. 
                    // Special fix for E2M4 and probably other maps.
                    if (singleplayer && !strict_mode && !vanillaparm)
                    return crushed;
                }
                return pastdest;
            }
            else
            {
                // COULD GET CRUSHED
                lastpos = sector->floorheight;
                sector->floorheight += speed;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    if (crush == true)
                    {
                        return crushed;
                    }
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }
            }
            break;
        }
        break;

        case 1:
        // CEILING
        switch (direction)
        {
            case -1:
            // DOWN
            if (sector->ceilingheight - speed < dest)
            {
                lastpos = sector->ceilingheight;
                sector->ceilingheight = dest;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                }
                return pastdest;
            }
            else
            {
                // COULD GET CRUSHED
                lastpos = sector->ceilingheight;
                sector->ceilingheight -= speed;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    if (crush == true)
                    {
                        return crushed;
                    }
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }
            }
            break;

            case 1:
            // UP
            if (sector->ceilingheight + speed > dest)
            {
                lastpos = sector->ceilingheight;
                sector->ceilingheight = dest;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                }
                return pastdest;
            }
            else
            {
                lastpos = sector->ceilingheight;
                sector->ceilingheight += speed;
                flag = P_ChangeSector(sector,crush);
            }
            break;
        }
        break;
    }

    return ok;
}

// -----------------------------------------------------------------------------
// T_MoveFloor
// Move a floor to it's destination (up or down)
// -----------------------------------------------------------------------------

void T_MoveFloor(floormove_t *floor)
{
    const result_e res = T_MovePlane(floor->sector,
                                     floor->speed,
                                     floor->floordestheight,
                                     floor->crush, 0, floor->direction);

    if (!(leveltime&7))
    {
        // [JN] Z-axis sfx distance: sound invoked from the floor
        floor->sector->soundorg.z = floor->sector->floorheight;
        S_StartSound(&floor->sector->soundorg, sfx_stnmov);
    }

    if (res == pastdest)
    {
        floor->sector->specialdata = NULL;

        if (floor->direction == 1)
        {
            switch (floor->type)
            {
                case donutRaise:
                floor->sector->special = floor->newspecial;
                floor->sector->floorpic = floor->texture;
                default:
                break;
            }
        }
        else if (floor->direction == -1)
        {
            switch (floor->type)
            {
                case lowerAndChange:
                floor->sector->special = floor->newspecial;
                floor->sector->floorpic = floor->texture;
                default:
                break;
            }
        }

        P_RemoveThinker(&floor->thinker);

        // [JN] Jaguar: no stop sound
        if (gamemission != jaguar)
        {
            // [JN] Z-axis sfx distance: sound invoked from the floor
            floor->sector->soundorg.z = floor->sector->floorheight;
            S_StartSound(&floor->sector->soundorg, sfx_pstop);
        }
    }
}

// -----------------------------------------------------------------------------
// EV_DoFloor
// Handle floor types
// -----------------------------------------------------------------------------

const int EV_DoFloor (line_t *line, const floor_e floortype)
{
    int          secnum, rtn, i;
    sector_t    *sec;
    floormove_t *floor;

    secnum = -1;
    rtn = 0;

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // Already moving? If so, keep going...
        if (sec->specialdata)
        {
            continue;
        }

        // new floor thinker
        rtn = 1;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;

        switch (floortype)
        {
            case lowerFloor:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
            P_FindHighestFloorSurrounding(sec);
            break;

            case lowerFloorToLowest:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
            P_FindLowestFloorSurrounding(sec);
            break;

            case turboLower:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = 
            P_FindHighestFloorSurrounding(sec);
            if (gameversion == exe_doom_1_2
            ||  floor->floordestheight != sec->floorheight)
            {
                floor->floordestheight += 8*FRACUNIT;
            }
            break;

            case raiseFloorCrush:
            floor->crush = true;
            case raiseFloor:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
            P_FindLowestCeilingSurrounding(sec);
            if (floor->floordestheight > sec->ceilingheight)
            {
                floor->floordestheight = sec->ceilingheight;
            }
            floor->floordestheight -= (8*FRACUNIT)*(floortype == raiseFloorCrush);
            break;

            case raiseFloorTurbo:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = 
            P_FindNextHighestFloor(sec,sec->floorheight);
            break;

            case raiseFloorToNearest:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
            P_FindNextHighestFloor(sec,sec->floorheight);
            break;

            case raiseFloor24:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
            break;

            case raiseFloor512:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 512 * FRACUNIT;
            break;

            case raiseFloor24AndChange:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            break;

            case raiseToTexture:
            {
                int minsize = INT_MAX;
                side_t *side;

                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;

                for (i = 0; i < sec->linecount; i++)
                {
                    if (twoSided (secnum, i))
                    {
                        side = getSide(secnum,i,0);
                        if (side->bottomtexture >= 0)
                        {
                            if (textureheight[side->bottomtexture] < minsize)
                            {
                                minsize = textureheight[side->bottomtexture];
                            }
                        }
                        side = getSide(secnum,i,1);

                        if (side->bottomtexture >= 0)
                        {
                            if (textureheight[side->bottomtexture] < minsize)
                            {
                                minsize = textureheight[side->bottomtexture];
                            }
                        }
                    }
                }

                floor->floordestheight =
                floor->sector->floorheight + minsize;
            }
            break;

            case lowerAndChange:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
            P_FindLowestFloorSurrounding(sec);
            floor->texture = sec->floorpic;

            for (i = 0; i < sec->linecount; i++)
            {
                if (twoSided(secnum, i))
                {
                    if (getSide(secnum,i,0)->sector-sectors == secnum)
                    {
                        sec = getSector(secnum,i,1);

                        if (sec->floorheight == floor->floordestheight)
                        {
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                    }
                    else
                    {
                        sec = getSector(secnum,i,0);

                        if (sec->floorheight == floor->floordestheight)
                        {
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                    }
                }
            }

            default:
            break;
        }
    }

    return rtn;
}

// -----------------------------------------------------------------------------
// BUILD A STAIRCASE!
// -----------------------------------------------------------------------------

#define STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE 10

const int EV_BuildStairs (line_t *line, const stair_e type)
{
    int secnum;
    int height;
    int i;
    int newsecnum;
    int texture;
    int ok;
    int rtn;
    sector_t *sec;
    sector_t *tsec;
    floormove_t *floor;
    fixed_t stairsize = 0;
    fixed_t speed = 0;

    secnum = -1;
    rtn = 0;

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // Already moving?  if so, keep going...
        if (sec->specialdata)
        {
            continue;
        }

        // new floor thinker
        rtn = 1;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;

        switch (type)
        {
            case build8:
            speed = FLOORSPEED/4;
            stairsize = 8*FRACUNIT;
            break;

            case turbo16:
            speed = FLOORSPEED*4;
            stairsize = 16*FRACUNIT;
            break;
        }

        floor->speed = speed;
        height = sec->floorheight + stairsize;
        floor->floordestheight = height;
        // Initialize
        floor->type = lowerFloor;
        // e6y
        // Uninitialized crush field will not be equal to 0 or 1 (true)
        // with high probability. So, initialize it with any other value
        floor->crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE;

        texture = sec->floorpic;

        // Find next sector to raise
        // 1.	Find 2-sided line with same sector side[0]
        // 2.	Other side is the next sector to raise
        do
        {
            ok = 0;

            for (i = 0 ; i < sec->linecount ; i++)
            {
                if (!((sec->lines[i])->flags & ML_TWOSIDED))
                {
                    continue;
                }

                tsec = (sec->lines[i])->frontsector;
                newsecnum = tsec-sectors;

                if (secnum != newsecnum)
                {
                    continue;
                }

                tsec = (sec->lines[i])->backsector;
                newsecnum = tsec - sectors;

                if (tsec->floorpic != texture)
                {
                    continue;
                }

                height += stairsize;

                if (tsec->specialdata)
                {
                    continue;
                }

                sec = tsec;
                secnum = newsecnum;
                floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);

                P_AddThinker (&floor->thinker);

                sec->specialdata = floor;
                floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                // Initialize
                floor->type = lowerFloor;
                // e6y
                // Uninitialized crush field will not be equal to 0 or 1 (true)
                // with high probability. So, initialize it with any other value
                floor->crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE;
                ok = 1;
                break;
            }
        } while (ok);
    }

    return rtn;
}

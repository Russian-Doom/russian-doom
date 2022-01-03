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
//	Teleportation.
//


#include "doomdef.h"
#include "doomstat.h"
#include "s_sound.h"
#include "p_local.h"
#include "sounds.h"
#include "r_state.h"
#include "jn.h"


// =============================================================================
// TELEPORTATION
// =============================================================================

int EV_Teleport (line_t *line, int side, mobj_t *thing)
{
    int        i, tag;
    unsigned   an;
    fixed_t    oldx, oldy, oldz;
    mobj_t    *m, *fog;
    thinker_t *thinker;
    sector_t  *sector;

    // Don't teleport missiles.
    if (thing->flags & MF_MISSILE)
    {
        return 0;
    }

    // Don't teleport if hit back of line, so you can get out of teleporter.
    if (side == 1)
    {
        return 0;
    }

    tag = line->tag;

    for (i = 0 ; i < numsectors ; i++)
    {
        if (sectors[i].tag == tag )
        {
            thinker = thinkercap.next;

            for (thinker = thinkercap.next ; thinker != &thinkercap ; thinker = thinker->next)
            {
                // Not a mobj.
                if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
                {
                    continue;
                }

                m = (mobj_t*)thinker;
		
                // Not a teleportman.
                if (m->type != MT_TELEPORTMAN)
                {
                    continue;
                }

                sector = m->subsector->sector;

                // Wrong sector.
                if (sector-sectors != i)
                {
                    continue;
                }

                oldx = thing->x;
                oldy = thing->y;
                oldz = thing->z;

                if (!P_TeleportMove (thing, m->x, m->y))
                {
                    return 0;
                }

                // The first Final Doom executable does not set thing->z
                // when teleporting. This quirk is unique to this
                // particular version; the later version included in
                // some versions of the Id Anthology fixed this.
                //
                // [JN] Fix behavior, safe for demos.
                // https://doomwiki.org/wiki/Final_Doom_teleporters_do_not_set_Z_coordinate
                if (gameversion != exe_final || (singleplayer && !vanillaparm))
                {
                    thing->z = thing->floorz;
                }

                if (thing->player)
                {
                    thing->player->viewz = thing->z+thing->player->viewheight;
                    thing->player->lookdir = 0;
                }

                // Spawn teleport fog at source and destination.
                fog = P_SpawnMobj (oldx, oldy, oldz, MT_TFOG);
                S_StartSound (fog, sfx_telept);
                an = m->angle >> ANGLETOFINESHIFT;
                fog = P_SpawnMobj (m->x+20*finecosine[an], m->y+20*finesine[an], thing->z, MT_TFOG);

                // Emit sound, where?
                S_StartSound (fog, sfx_telept);

                // Don't move for a bit.
                // [JN] Press Beta telepoters doesn't have this delay.
                if (thing->player && gamemode != pressbeta)
                {
                    thing->reactiontime = 18;
                }

                thing->angle = m->angle;
                thing->momx = thing->momy = thing->momz = 0;

                return 1;
            }
        }
    }

    return 0;
}

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
// P_tick.c



#include "doomdef.h"
#include "i_system.h"
#include "p_local.h"
#include "v_video.h"
#include "jn.h"

int leveltime;
int TimerGame;


/*
================================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.
The actual structures will vary in size, but the first element must be thinker_t.

================================================================================
*/

thinker_t thinkercap;  // both the head and tail of the thinker list

/*
================================================================================
=
= P_InitThinkers
=
================================================================================
*/

void P_InitThinkers (void)
{
    thinkercap.prev = thinkercap.next = &thinkercap;
}


/*
================================================================================
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
================================================================================
*/

void P_AddThinker(thinker_t *thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}

/*
================================================================================
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
================================================================================
*/

void P_RemoveThinker(thinker_t *thinker)
{
    thinker->function = (think_t) - 1;
}

/*
================================================================================
=
= P_RunThinkers
=
================================================================================
*/

static void P_RunThinkers (void)
{
    thinker_t *currentthinker, *nextthinker;

    // [JN] Prevent dropped item from jittering on moving platforms.
    // For single player only, really not safe for internal demos.
    // See: https://github.com/bradharding/doomretro/issues/501
    if (singleplayer)
    {
        currentthinker = thinkercap.next;
        while (currentthinker != &thinkercap)
        {
            if (currentthinker->function)
                if (currentthinker->function == P_MobjThinker)
                    currentthinker->function (currentthinker);
            
            nextthinker = currentthinker->next;
            currentthinker = nextthinker;
        }
    }

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        if (currentthinker->function == (think_t) - 1)
        {                       // time to remove it
            nextthinker = currentthinker->next;
            currentthinker->next->prev = currentthinker->prev;
            currentthinker->prev->next = currentthinker->next;
            Z_Free(currentthinker);
        }
        else
        {
            // [JN] Prevent dropped item from jittering on moving platforms.
            if (singleplayer)
            {
                if (currentthinker->function)
                    if (currentthinker->function != P_MobjThinker)
                        currentthinker->function (currentthinker);

                nextthinker = currentthinker->next;
            }
            else
            {
                if (currentthinker->function)
                    currentthinker->function(currentthinker);

                nextthinker = currentthinker->next;
            }
        }
        currentthinker = nextthinker;
    }
}

/*
================================================================================
=
= P_Ticker
=
================================================================================
*/

void P_Ticker (void)
{
    if (paused)
    {
        return;
    }

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            P_PlayerThink(&players[i]);
        }
    }

    if (TimerGame)
    {
        if (!--TimerGame)
        {
            G_ExitLevel();
        }
    }

    P_RunThinkers();
    P_UpdateSpecials();
    P_AmbientSound();
    leveltime++;
}

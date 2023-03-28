//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//



#include <stdlib.h>

#include "doomfeatures.h"
#include "d_main.h"
#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"
#include "w_checksum.h"
#include "w_wad.h"
#include "deh_main.h"
#include "d_loop.h"
#include "p_local.h"
#include "jn.h"


ticcmd_t *netcmds;

//
// Called when a player leaves the game
//
static void PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];
    unsigned int player_num;

    player_num = player - players;

    // Do this the same way as Vanilla Doom does, to allow dehacked
    // replacements of this message

    M_StringCopy(exitmsg, DEH_String(english_language ?
                                     "Player 1 left the game" :
                                     // [JN] игрок № отключился от сервера.
                                     "buhjr 1 jnrk.xbkcz jn cthdthf>"),
                                     sizeof(exitmsg));

    // [JN] Player's symbol number
    exitmsg[english_language ? 7 : 6] += player_num;

    playeringame[player_num] = false;
    P_SetMessage(&players[consoleplayer], exitmsg, msg_system, false);
    // [crispy] don't interpolate players who left the game
    player->mo->interp = false;

    // TODO: check if it is sensible to do this:

    if (demorecording) 
    {
        G_CheckDemoStatus ();
    }
}


static void RunTic(ticcmd_t *cmds, boolean *ingame)
{
    extern boolean advancedemo;
    unsigned int i;

    // Check for player quits.

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        if (!demoplayback && playeringame[i] && !ingame[i])
        {
            PlayerQuitGame(&players[i]);
        }
    }

    netcmds = cmds;

    // check that there are players in the game.  if not, we cannot
    // run a tic.

    if (advancedemo)
    {
        D_DoAdvanceDemo ();
    }

    G_Ticker ();
}

static loop_interface_t doom_loop_interface = {
    D_ProcessEvents,
    G_BuildTiccmd,
    RunTic,
    M_Ticker
};


// Load game settings from the specified structure and
// set global variables.

static void LoadGameSettings(net_gamesettings_t *settings)
{
    unsigned int i;

    deathmatch = settings->deathmatch;
    startepisode = settings->episode;
    startmap = settings->map;
    startskill = settings->skill;
    startloadgame = settings->loadgame;
    lowres_turn = settings->lowres_turn;
    nomonsters = settings->nomonsters;
    fastparm = settings->fast_monsters;
    respawnparm = settings->respawn_monsters;
    timelimit = settings->timelimit;
    consoleplayer = settings->consoleplayer;

    if (lowres_turn)
    {
        if (english_language)
        {
            printf("NOTE: Turning resolution is reduced; this is probably "
                   "because there is a client recording a Vanilla demo.\n");
        }
        else
        {
            printf("ВНИМАНИЕ: разрешение поворота камеры снижено. Вероятно, "
                   "это вызвано сохранением демозаписи в оригинальном формате.\n");
        }
    }

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        playeringame[i] = i < settings->num_players;
    }
}


// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t *settings)
{
    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->deathmatch = deathmatch;
    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    settings->loadgame = startloadgame;
    settings->gameversion = gameversion;
    settings->nomonsters = nomonsters;
    settings->fast_monsters = fastparm;
    settings->respawn_monsters = respawnparm;
    settings->timelimit = timelimit;

    settings->lowres_turn = (M_CheckParm("-record") > 0
                          && M_CheckParm("-longtics") == 0)
                          || M_CheckParm("-shorttics") > 0;
}


static void InitConnectData(net_connect_data_t *connect_data)
{
    boolean shorttics;

    connect_data->max_players = MAXPLAYERS;
    connect_data->drone = false;

    //!
    // @category net
    //
    // Run as the left screen in three screen mode.
    //

    if (M_CheckParm("-left") > 0)
    {
        viewangleoffset = ANG90;
        connect_data->drone = true;
    }

    //! 
    // @category net
    //
    // Run as the right screen in three screen mode.
    //

    if (M_CheckParm("-right") > 0)
    {
        viewangleoffset = ANG270;
        connect_data->drone = true;
    }

    //
    // Connect data
    //

    // Game type fields:

    connect_data->gamemode = gamemode;
    connect_data->gamemission = gamemission;

    //!
    // @category demo
    //
    // Play with low turning resolution to emulate demo recording.
    //

    shorttics = M_ParmExists("-shorttics");

    // Are we recording a demo? Possibly set lowres turn mode

    connect_data->lowres_turn = (M_ParmExists("-record")
                             && !M_ParmExists("-longtics"))
                              || shorttics;

    // Read checksums of our WAD directory and dehacked information

    W_Checksum(connect_data->wad_sha1sum);
    DEH_Checksum(connect_data->deh_sha1sum);

    // Are we playing with the Freedoom IWAD?

    connect_data->is_freedoom = W_CheckNumForName("FREEDOOM") >= 0;
}

void D_ConnectNetGame(void)
{
    net_connect_data_t connect_data;

    InitConnectData(&connect_data);
    netgame = D_InitNetGame(&connect_data);

    //!
    // @category net
    //
    // Start the game playing as though in a netgame with a single
    // player.  This can also be used to play back single player netgame
    // demos.
    //

    if (M_CheckParm("-solo-net") > 0)
    {
        netgame = true;
    }
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
void D_CheckNetGame (void)
{
    net_gamesettings_t settings;

    if (netgame)
    {
        autostart = true;
    }

    D_RegisterLoopCallbacks(&doom_loop_interface);

    SaveGameSettings(&settings);
    D_StartNetGame(&settings, NULL);
    LoadGameSettings(&settings);

    DEH_printf("    ");

    if (english_language)
    {
        if (gamemode != commercial)
        {
            DEH_printf("episode: %i, ", startepisode);
        }

        DEH_printf("level: %i, ", startmap);

        DEH_printf("skill: %i, mode: ", startskill+1);

        DEH_printf(netgame ? "cooperative"    :
           deathmatch == 1 ? "deathmatch"     : 
           deathmatch == 2 ? "deathmatch 2.0" : 
           deathmatch == 3 ? "deathmatch 3.0" : 
                             "single player");
                             
    }
    else
    {
        if (gamemode != commercial)
        {
            DEH_printf("эпизод: %i, ", startepisode);
        }

        DEH_printf("уровень: %i, ", startmap);

        DEH_printf("сложность: %i, режим: ", startskill+1);

        DEH_printf(netgame ? "совместная игра" :
           deathmatch == 1 ? "дефматч"         :
           deathmatch == 2 ? "дефматч 2.0"     :
           deathmatch == 3 ? "дефматч 3.0"     :
                             "одиночная игра");
    }

    DEH_printf("\n");

    if (netgame)
    {
        DEH_printf(english_language ?
                   "    player %i of %i (%i nodes)\n" :
                   "    игроки: %i из %i (узлов: %i)\n",
                   consoleplayer+1, settings.num_players, settings.num_players);
    }

    // Show players here; the server might have specified a time limit

    if (timelimit > 0 && deathmatch)
    {
        // Gross hack to work like Vanilla:

        if (timelimit == 20 && M_CheckParm("-avg"))
        {
            DEH_printf(english_language ?
                       "Austin Virtual Gaming: Levels will end after 20 minutes\n" :
                       "Austin Virtual Gaming: Уровни завершаются после 20 минут игры\n");
        }
        else
        {
            DEH_printf(english_language ?
                       "Levels will end after %d minute" :
                       "Уровни завершаются после %d минут игры",
                       timelimit);
            if (timelimit > 1)
                printf("s");
            printf(".\n");
        }
    }
}


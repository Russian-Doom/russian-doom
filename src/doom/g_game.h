//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//   Duh.
// 

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"


//
// GAME
//
void G_DeathMatchSpawnPlayer (int playernum);

void G_InitNew (skill_t skill, int episode, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew (skill_t skill, int episode, int map);

void G_DeferedPlayDemo (char* name);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (char* name);

void G_DoLoadGame (void);

// Called by M_Responder.
void G_SaveGame (int slot, char* description);

// Only called by startup code.
void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_PlayDemo (char* name);
void G_TimeDemo (char* name);
boolean G_CheckDemoStatus (void);

void G_ExitLevel (void);
void G_SecretExitLevel (void);

void G_WorldDone (void);

// Read current data from inputs and build a player movement command.

void G_BuildTiccmd (ticcmd_t *cmd, int maketic); 

void G_Ticker (void);
boolean G_Responder (event_t*	ev);

void G_ScreenShot (void);

void G_DrawMouseSpeedBox(void);
int G_VanillaVersionCode(void);

// [JN] Оригинальные ограничения отключены, т.к. в них больше нет смысла
// extern int vanilla_savegame_limit;
// extern int vanilla_demo_limit;

// [JN] Mouselook
extern int mlook;

// [JN] Дополнительные параметры игры

// Графика
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int swirling_liquids;
extern int invul_sky;
extern int colored_blood;
extern int red_resurrection_flash;
extern int draw_shadowed_text;
extern int show_diskicon;

// Звук
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int correct_endlevel_sfx;
extern int play_exit_sfx;

// Тактика
extern int automap_stats;
extern int secret_notification;
extern int negative_health;
extern int infragreen_visor;

// Физика
extern int over_under;
extern int weapon_bobbing;
extern int ssg_blast_enemies;
extern int randomly_flipcorpses;
extern int floating_powerups;

// Геймплей
extern int fix_map_errors;
extern int flip_levels;
extern int new_ouch_face;
extern int unlimited_lost_souls;
extern int agressive_lost_souls;
extern int fast_quickload;

// Прицел
extern int crosshair_draw;
extern int crosshair_health;
extern int crosshair_scale;

#endif


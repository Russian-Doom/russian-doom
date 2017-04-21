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

// Russian DOOM (C) 2016-2017 Julian Nechaevsky

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

void G_DeferedPlayDemo (char* demo);

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

// [JN] Дополнительные параметры игры

// - Интерфейс -
extern int draw_shadowed_text;      // Элементы меню и тексты отбрасывают тень
extern int fast_quickload;          // Не выводить запрос при быстрой загрузке
// - Графика -
extern int colored_blood;           // Кровь разных цветов
extern int randomly_flipcorpses;    // Произвольное зеркальное отражение трупов
extern int new_ouch_face;           // Корректная формула "Ouch face"
extern int invul_sky;               // Неуязвимость окрашивает небо
extern int swirling_liquids;        // Улучшенная анимация жидкостей
extern int red_resurrection_flash;  // Красная вспышка воскрешения монстров
extern int ssg_blast_enemies;       // Двуствольное ружье может разрывать врагов
extern int translucency;            // Прозрачность объектов
extern int no_pickup_flash;         // Не мигать экраном при получении предметов
// - Звук -
extern int crushed_corpses_sfx;     // Звук раздавливания трупов
extern int blazing_door_fix_sfx;    // Одиночный звук закрытия быстрой двери
extern int correct_endlevel_sfx;    // Корректный звук завершения уровня
extern int play_exit_sfx;           // Проигрывать звук при выходе из игры
// - Геймплей -
extern int negative_health;          // Отображать отрицательное здоровье
extern int secret_notification;      // Уведомление об обнаружении секрета
extern int show_total_time;          // Показывать общее время
extern int unlimited_lost_souls;     // Элементаль боли без ограничения душ
extern int unlimited_archvile_range; // Арчвайл без ограничения дистанции атаки

#endif


//
// Copyright(C) 2020-2022 Dasperal
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


#pragma once

#include <stdio.h>
#include "doomtype.h"
#include "d_event.h"


typedef enum
{
    // Movement
    bk_forward = 0,
    bk_backward,
    bk_turn_left,
    bk_turn_right,
    bk_strafe_left,
    bk_strafe_right,
    bk_fly_up,
    bk_fly_down,
    bk_fly_stop,
    bk_speed,
    bk_strafe,
    bk_jump,
    bk_toggle_autorun,

    bk_use,

    // Weapon
    bk_fire,
    bk_weapon_1,
    bk_weapon_2,
    bk_weapon_3,
    bk_weapon_4,
    bk_weapon_5,
    bk_weapon_6,
    bk_weapon_7,
    bk_weapon_8,
    bk_weapon_prev,
    bk_weapon_next,

    // Look
    bk_look_up,
    bk_look_down,
    bk_look_center,
    bk_toggle_mlook,

    // Inventory
    bk_inv_left,
    bk_inv_right,
    bk_inv_use_artifact,

    // Inventory: Strife
    bk_inv_use_health, // [Daspral] Reserved for safe addition of feature in future
//  bk_inv_drop, // [Daspral] Uncommenting these lines will break binds in config files
//  bk_inv_pop,
//  bk_inv_key,
//  bk_inv_home,
//  bk_inv_end,
//  bk_mission,

    // Artifacts: Heretic
    bk_arti_quartz,
    bk_arti_urn,
    bk_arti_bomb,
    bk_arti_tome,
    bk_arti_egg,
    bk_arti_shadowsphere,
    bk_arti_wings,
    bk_arti_torch,
    bk_arti_invulnerability,
    bk_arti_chaosdevice,

    // Artifacts: Hexen
    bk_arti_all,
    bk_arti_blastradius,
    bk_arti_teleportother,
    bk_arti_boostarmor,
    bk_arti_boostmana,
    bk_arti_summon,
    bk_arti_speed,
    bk_arti_healingradius,

    // Shortcuts
    bk_screen_inc, // [Dasperal] screen_inc/dec do not work on automap so they should be related to "Controls" section
    bk_screen_dec, // instead of "Shortcuts" section

    bk_menu_help, // [Dasperal] Bindings starting from this line are related to "Shortcuts" section
    bk_menu_save,
    bk_menu_load,
    bk_menu_volume,
    bk_qsave,
    bk_end_game,
    bk_qload,
    bk_quit,
    bk_gamma,
    bk_nextlevel,
    bk_reloadlevel,
    bk_screenshot,
    bk_pause,
    bk_finish_demo,

    // Toggles
    bk_toggle_crosshair,
    bk_messages,
    bk_detail,
    bk_toggle_fliplvls,

    // Multiplayer
    bk_spy,
    bk_multi_msg,
    bk_multi_msg_player_0,
    bk_multi_msg_player_1,
    bk_multi_msg_player_2,
    bk_multi_msg_player_3,
    bk_multi_msg_player_4,
    bk_multi_msg_player_5,
    bk_multi_msg_player_6,
    bk_multi_msg_player_7,

    // Map keys
    bk_map_toggle, // [Dasperal] Bindings starting from this line are related to "Map" section
    bk_map_zoom_in,
    bk_map_zoom_out,
    bk_map_zoom_max,
    bk_map_follow,
    bk_map_overlay,
    bk_map_rotate,
    bk_map_grid,
    bk_map_mark,
    bk_map_clearmark,

    // System keys
    bk_left, // [Dasperal] Bindings starting from this line are not saved to config file
    bk_right,
    bk_up,
    bk_down,

    bk_menu_activate,
    bk_menu_back,
    bk_menu_select,
    bk_menu_page_next,
    bk_menu_page_prev,

    bk_confirm,
    bk_abort,

    bk__size, // size of bound_key_t
    bk__null,
    bk__serializable = bk_left,
    bk__section_map = bk_map_toggle,
    bk__section_shortcuts = bk_menu_help
} bound_key_t;

typedef enum
{
    keyboard,
    mouse,
    controller
} device_t;

typedef struct bind_descriptor_s
{
    struct bind_descriptor_s* next;
    device_t device;
    int key;
} bind_descriptor_t;

extern bind_descriptor_t* bind_descriptor[bk__size];
extern boolean isBinding;
extern boolean isBindsLoaded;

void BK_ProcessKey(event_t* event);

boolean BK_isKeyPressed(bound_key_t key);

boolean BK_isKeyDown(event_t* event, bound_key_t key);

boolean BK_isKeyUp(event_t* event, bound_key_t key);

void BK_ReleaseKey(bound_key_t key);

void BK_ReleaseAllKeys();

void BK_StartBindingKey(bound_key_t key);

void BK_BindKey(event_t* event);

void BK_AddBind(bound_key_t boundKey, device_t device, int key);

/** Clears all binds for given bound_key */
void BK_ClearBinds(bound_key_t key);

void BK_AddBindingsToSystemKeys();

void BK_ApplyDefaultBindings();

void BK_ApplyVanilaBindings();

boolean KeybindsHandler_isHandling(char* sectionName);

void KeybindsHandler_HandleLine(char* keyName, char *value, size_t valueSize);

void KeybindsHandler_Save(FILE* file, char* sectionName);

void BK_TraverseBinds(void (*lambda)(bound_key_t key, bind_descriptor_t* bindDescriptor));

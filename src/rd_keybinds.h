//
// Copyright(C) 2020-2021 Dasperal
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

#ifndef RD_KEYBINDS_H
#define RD_KEYBINDS_H

#include "doomtype.h"
#include "d_event.h"

typedef enum
{
    //Movement
    bk_forward = 1,
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

    //Weapon
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

    //Look
    bk_look_up,
    bk_look_down,
    bk_look_center,
    bk_toggle_mlook,

    //Inventory
    bk_inv_left,
    bk_inv_right,
    bk_inv_use_artifact,

    //Inventory: Strife
    bk_inv_use_health, // [Daspral] Reserved for safe addition of feature in future
//  bk_inv_drop, // [Daspral] Uncommenting these lines will break binds in config files
//  bk_inv_pop,
//  bk_inv_key,
//  bk_inv_home,
//  bk_inv_end,
//  bk_mission,

    //Artifacts: Heretic
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

    //Artifacts: Hexen
    bk_arti_all,
    bk_arti_blastradius,
    bk_arti_teleportother,
    bk_arti_boostarmor,
    bk_arti_boostmana,
    bk_arti_summon,
    bk_arti_speed,
    bk_arti_healingradius,

    //Map keys
    bk_map_toggle,
    bk_map_zoom_in,
    bk_map_zoom_out,
    bk_map_zoom_max,
    bk_map_follow,
    bk_map_overlay,
    bk_map_rotate,
    bk_map_grid,
    bk_map_mark,
    bk_map_clearmark,

    //Shortcuts and toggles
    bk_qsave,
    bk_qload,
    bk_nextlevel,
    bk_reloadlevel,
    bk_screenshot,
    bk_finish_demo,
    bk_toggle_crosshair,
    bk_toggle_fliplvls,

    //Multiplayer
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

    //F Keys
    bk_menu_help,
    bk_menu_save,
    bk_menu_load,
    bk_menu_volume,
    bk_detail,
    bk_endgame,
    bk_messages,
    bk_quit,
    bk_gamma,
    bk_screen_inc,
    bk_screen_dec,

    bk_pause,

    //System keys
    bk__serializable, // [Dasperal] Bindings after this line are not saved to config file
    bk_left,
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

    bk__size, //size of bound_key_t
    bk__null
} bound_key_t;

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

/** Clears all binds for given bound_key */
void BK_ClearBinds(bound_key_t key);

void BK_AddBindingsToSystemKeys();

void BK_ApplyDefaultBindings();

void BK_LoadBindings(void* file);

void BK_SaveBindings(void* file);

#endif //RD_KEYBINDS_H

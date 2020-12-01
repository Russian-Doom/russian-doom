//
// Copyright(C) 2020 Dasperal
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

#include "doomtype.h"

#ifndef __RD_KEYBINDS_H__
#define __RD_KEYBINDS_H__

typedef enum
{
    //Movement
    bk_null = 0,
    bk_forward,
    bk_backward,
    bk_turn_left,
    bk_turn_right,
    bk_strafe_left,
    bk_strafe_right,
    bk_fly_up,
    bk_fly_down,
    bk_fly_center,
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
//  bk_inv_left,
//  bk_inv_right,
//  bk_inv_use_artifact,
//  bk_inv_use_health,
//  bk_inv_drop,
//  bk_inv_pop,
//  bk_inv_key,
//  bk_inv_home,
//  bk_inv_end,
//  bk_mission,

    //Artifacts: Heretic
//  bk_arti_all,
//  bk_arti_quartz,
//  bk_arti_urn,
//  bk_arti_bomb,
//  bk_arti_tome,
//  bk_arti_egg,
//  bk_arti_shadowsphere,
//  bk_arti_wings,
//  bk_arti_torch,
//  bk_arti_blastradius,
//  bk_arti_ring,
//  bk_arti_chaosdevice,

    //Artifacts: Hexen
//  bk_arti_poisonbag,
//  bk_arti_pig,
//  bk_arti_iconofdefender,
//  bk_arti_teleportother,
//  bk_arti_boostarmor,
//  bk_arti_boostmana,
//  bk_arti_summon,
//  bk_arti_speed,
//  bk_arti_healingradius,

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
    bk_save,
    bk_load,
    bk_nextlevel,
    bk_reloadlevel,
    bk_screenshot,
    bk_finish_demo,
    bk_toggle_crosshair,
    bk_toggle_fliplvls,

    bk_size //size of bound_key_t

} bound_key_t;

typedef struct
{
    int*    key_var;
    char    eng_name[128];
    char    rus_name[128];
    char    eng_HotKey;
    char    rus_HotKey;
} bound_key_descriptor; //TODO make descriptor private

bound_key_descriptor* BK_getKeyDescriptor(bound_key_t key); //TODO make descriptor private

/** Returns string of names for first 1 physical keys bound to given bound_key */
char* BK_getBoundKeysString(bound_key_t key);
/** Returns true if no keys have been bound to given bound_key */
boolean BK_KeyHasNoBinds(bound_key_t key);
/** Clears all binds for given bound_key */
void BK_ClearBinds(bound_key_t key);

#endif //__RD_KEYBINDS_H__

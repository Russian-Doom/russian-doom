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

#include <stddef.h>
#include "rd_keybinds.h"
#include "doomkeys.h"
#include "i_input.h"
#include "i_video.h"
#include "m_controls.h"
#include "m_misc.h"
#include "rd_menu.h"

boolean isBinding = false;
bound_key_t keyToBind = bk_null;

typedef struct
{
    int* key_var;
    int* mouse_var;
} bound_key_descriptor;

bound_key_descriptor bound_key_descriptors[bk_size] = {
    // Movement
    {NULL,               NULL}, // bk_null
    {&key_up,            &mousebforward}, // bk_forward
    {&key_down,          &mousebbackward}, // bk_backward
    {&key_left,          NULL}, // bk_turn_left
    {&key_right,         NULL}, // bk_turn_right
    {&key_strafeleft,    &mousebstrafeleft}, // bk_strafe_left
    {&key_straferight,   &mousebstraferight}, // bk_strafe_right
    {&key_flyup,         NULL}, // bk_fly_up
    {&key_flydown,       NULL}, // bk_fly_down
    {&key_flycenter,     NULL}, // bk_fly_center
    {&key_speed,         NULL}, // bk_speed
    {&key_strafe,        &mousebstrafe}, // bk_strafe
    {&key_jump,          &mousebjump}, // bk_jump
    {&key_toggleautorun, NULL}, // bk_toggle_autorun
    {&key_use,           &mousebuse}, // bk_use

    // Weapon
    {&key_fire,       &mousebfire}, // bk_fire
    {&key_weapon1,    NULL}, // bk_weapon_1
    {&key_weapon2,    NULL}, // bk_weapon_2
    {&key_weapon3,    NULL}, // bk_weapon_3
    {&key_weapon4,    NULL}, // bk_weapon_4
    {&key_weapon5,    NULL}, // bk_weapon_5
    {&key_weapon6,    NULL}, // bk_weapon_6
    {&key_weapon7,    NULL}, // bk_weapon_7
    {&key_weapon8,    NULL}, // bk_weapon_8
    {&key_prevweapon, &mousebprevweapon}, // bk_weapon_prev
    {&key_nextweapon, &mousebnextweapon}, // bk_weapon_next

    // Look
    {&key_lookup,      NULL}, // bk_look_up
    {&key_lookdown,    NULL}, // bk_look_down
    {&key_lookcenter,  NULL}, // bk_look_center
    {&key_togglemlook, NULL}, // bk_toggle_mlook

    // Inventory
//  // bk_inv_left
//  // bk_inv_right
//  // bk_inv_use_artifact
//  // bk_inv_use_health
//  // bk_inv_drop
//  // bk_inv_pop
//  // bk_inv_key
//  // bk_inv_home
//  // bk_inv_end
//  // bk_mission

    // Artifacts: Heretic
//  // bk_arti_all
//  // bk_arti_quartz
//  // bk_arti_urn
//  // bk_arti_bomb
//  // bk_arti_tome
//  // bk_arti_egg
//  // bk_arti_shadowsphere
//  // bk_arti_wings
//  // bk_arti_torch
//  // bk_arti_blastradius
//  // bk_arti_ring
//  // bk_arti_chaosdevice

    // Artifacts: Hexen
//  // bk_arti_poisonbag
//  // bk_arti_pig
//  // bk_arti_iconofdefender
//  // bk_arti_teleportother
//  // bk_arti_boostarmor
//  // bk_arti_boostmana
//  // bk_arti_summon
//  // bk_arti_speed
//  // bk_arti_healingradius

    // Map keys
    {&key_map_toggle,    NULL}, // bk_map_toggle
    {&key_map_zoomin,    NULL}, // bk_map_zoom_in
    {&key_map_zoomout,   NULL}, // bk_map_zoom_out
    {&key_map_maxzoom,   NULL}, // bk_map_zoom_max
    {&key_map_follow,    NULL}, // bk_map_follow
    {&key_map_overlay,   NULL}, // bk_map_overlay
    {&key_map_rotate,    NULL}, // bk_map_rotate
    {&key_map_grid,      NULL}, // bk_map_grid
    {&key_map_mark,      NULL}, // bk_map_mark
    {&key_map_clearmark, NULL}, // bk_map_clearmark

    // Shortcuts and toggles
    {&key_menu_qsave,       NULL}, // bk_qsave
    {&key_menu_qload,       NULL}, // bk_qload
    {&key_menu_nextlevel,   NULL}, // bk_nextlevel
    {&key_menu_reloadlevel, NULL}, // bk_reloadlevel
    {&key_menu_screenshot,  NULL}, // bk_screenshot
    {&key_demo_quit,        NULL}, // bk_finish_demo
    {&key_togglecrosshair,  NULL}, // bk_toggle_crosshair
    {&key_togglefliplvls,   NULL}, // bk_toggle_fliplvls

    //Multiplayer
    {&key_spy,                  NULL}, // bk_spy
    {&key_multi_msg,            NULL}, // bk_multi_msg
    {&(key_multi_msgplayer[0]), NULL}, // bk_multi_msg_player_0
    {&(key_multi_msgplayer[1]), NULL}, // bk_multi_msg_player_1
    {&(key_multi_msgplayer[2]), NULL}, // bk_multi_msg_player_2
    {&(key_multi_msgplayer[3]), NULL}, // bk_multi_msg_player_3
    {&(key_multi_msgplayer[4]), NULL}, // bk_multi_msg_player_4
    {&(key_multi_msgplayer[5]), NULL}, // bk_multi_msg_player_5
    {&(key_multi_msgplayer[6]), NULL}, // bk_multi_msg_player_6
    {&(key_multi_msgplayer[7]), NULL}, // bk_multi_msg_player_7

    //F & Special keys
    {&key_menu_activate,  NULL}, // bk_menu_activate
    {&key_menu_help,      NULL}, // bk_menu_help
    {&key_menu_save,      NULL}, // bk_menu_save
    {&key_menu_load,      NULL}, // bk_menu_load
    {&key_menu_volume,    NULL}, // bk_menu_volume
    {&key_menu_detail,    NULL}, // bk_detail
    {&key_menu_endgame,   NULL}, // bk_endgame
    {&key_menu_messages,  NULL}, // bk_messages
    {&key_menu_quit,      NULL}, // bk_quit
    {&key_menu_gamma,     NULL}, // bk_gamma
    {&key_menu_incscreen, NULL}, // bk_screen_inc
    {&key_menu_decscreen, NULL}, // bk_screen_dec
    {&key_pause,          NULL}, // bk_pause
    {&key_menu_back,      NULL}, // bk_menu_back
    {&key_menu_forward,   NULL}, // bk_menu_select

    //System keys
    {&key_menu_left,    NULL}, // bk_left
    {&key_menu_right,   NULL}, // bk_right
    {&key_menu_up,      NULL}, // bk_up
    {&key_menu_down,    NULL}, // bk_down
    {&key_menu_confirm, NULL}, // bk_confirm
    {&key_menu_abort,   NULL}  // bk_abort
};

boolean keyState[bk_size];

void BK_ProcessKey(event_t* event)
{
    for(int i = 0; i < bk_size; i++)
    {
        if((event->type == ev_keydown || event->type == ev_keyup) &&
           bound_key_descriptors[i].key_var &&
           *bound_key_descriptors[i].key_var == event->data1)
        {
            keyState[i] = event->type == ev_keydown;
        }

        if((event->type == ev_mouse_keydown || event->type == ev_mouse_keyup) &&
           bound_key_descriptors[i].mouse_var &&
           *bound_key_descriptors[i].mouse_var == event->data1)
        {
            keyState[i] = event->type == ev_mouse_keydown;
        }
    }
}

boolean BK_isKeyPressed(bound_key_t key)
{
    return keyState[key];
}

boolean BK_isKeyDown(event_t* event, bound_key_t key)
{
    return (event->type == ev_keydown && bound_key_descriptors[key].key_var && *bound_key_descriptors[key].key_var != 0 && *bound_key_descriptors[key].key_var == event->data1) ||
    (event->type == ev_mouse_keydown && bound_key_descriptors[key].mouse_var && *bound_key_descriptors[key].mouse_var != -1 && *bound_key_descriptors[key].mouse_var == event->data1);
}

boolean BK_isKeyUp(event_t* event, bound_key_t key)
{
    return (event->type == ev_keyup && bound_key_descriptors[key].key_var && *bound_key_descriptors[key].key_var != 0 && *bound_key_descriptors[key].key_var == event->data1) ||
    (event->type == ev_mouse_keyup && bound_key_descriptors[key].mouse_var && *bound_key_descriptors[key].mouse_var != -1 && *bound_key_descriptors[key].mouse_var == event->data1);
}

void BK_ReleaseKey(bound_key_t key)
{
    keyState[key] = false;
}

void BK_ReleaseAllKeys()
{
    memset(keyState, 0, sizeof(keyState));
}

// -----------------------------------------------------------------------------
// BK_KeyHasNoBinds
// Returns true if no keys have been bound to given bound_key
// -----------------------------------------------------------------------------
static boolean BK_KeyHasNoBinds(bound_key_t key)
{
    return *bound_key_descriptors[key].key_var == 0 &&
    (bound_key_descriptors[key].mouse_var == NULL || *bound_key_descriptors[key].mouse_var == -1);
}

static char* getKeyboardKeyName(int key)
{
    // [JN] Values are simple ASCII table:
    // https://upload.wikimedia.org/wikipedia/commons/7/7b/Ascii_Table-nocolor.svg
    switch(key)
    {
        case 0:     return "---";
        case 9:     return "TAB";
        case 13:    return "ENTER";
        case 32:    return "SPACE BAR";
        case 39:    return "'";
        case 42:    return "*";
        case 43:    return "+"; // [JN] NumPad +
        case 44:    return ",";
        case 45:    return "-";
        case 46:    return ".";
        case 47:    return "/";
        case 48:    return "0";
        case 49:    return "1";
        case 50:    return "2";
        case 51:    return "3";
        case 52:    return "4";
        case 53:    return "5";
        case 54:    return "6";
        case 55:    return "7";
        case 56:    return "8";
        case 57:    return "9";
        case 59:    return ";";
        case 61:    return "="; // [JN] Indicated as "+" in help screens
        case 91:    return "[";
        case 93:    return "]";
        case 92:    return "\\";
        case 96:    return "TILDE";
        case 97:    return "A";
        case 98:    return "B";
        case 99:    return "C";
        case 100:   return "D";
        case 101:   return "E";
        case 102:   return "F";
        case 103:   return "G";
        case 104:   return "H";
        case 105:   return "I";
        case 106:   return "J";
        case 107:   return "K";
        case 108:   return "L";
        case 109:   return "M";
        case 110:   return "N";
        case 111:   return "O";
        case 112:   return "P";
        case 113:   return "Q";
        case 114:   return "R";
        case 115:   return "S";
        case 116:   return "T";
        case 117:   return "U";
        case 118:   return "V";
        case 119:   return "W";
        case 120:   return "X";
        case 121:   return "Y";
        case 122:   return "Z";
        case 127:   return "BACKSPACE";
        case 157:   return "CTRL";
        case 172:   return "LEFT ARROW";
        case 173:   return "UP ARROW";
        case 174:   return "RIGHT ARROW";
        case 175:   return "DOWN ARROW";
        case 182:   return "SHIFT";
        case 184:   return "ALT";
        case 186:   return "CAPS LOCK";
        case 187:   return "F1";
        case 188:   return "F2";
        case 189:   return "F3";
        case 190:   return "F4";
        case 191:   return "F5";
        case 192:   return "F6";
        case 193:   return "F7";
        case 194:   return "F8";
        case 195:   return "F9";
        case 197:   return "NUM LOCK";
        case 198:   return "SCROLL LOCK";
        case 199:   return "HOME";
        case 201:   return "PAGE UP";
        case 204:   return "5"; // [JN] NumPad 5
        case 207:   return "END";
        case 209:   return "PAGE DOWN";
        case 210:   return "INSERT";
        case 211:   return "DELETE";
        case 215:   return "F11";
        case 216:   return "F12";
        case 217:   return "PRINT SCREEN";
        case 255:   return "PAUSE";
        default:    return "?"; // [JN] Unknown key
    }
}

static char* getMouseKeyName(int i)
{
    switch (i)
    {
        case -1:                  return "---";
        case  MOUSE_LEFT:         return "MOUSE 1";
        case  MOUSE_RIGHT:        return "MOUSE 2";
        case  MOUSE_MIDDLE:       return "MOUSE 3";
        case  MOUSE_4:            return "MOUSE 4";
        case  MOUSE_5:            return "MOUSE 5";
        case  MOUSE_SCROLL_UP:    return "SCROLL UP";
        case  MOUSE_SCROLL_DOWN:  return "SCROLL DOWN";
        case  MOUSE_SCROLL_RIGHT: return "SCROLL RIGHT";
        case  MOUSE_SCROLL_LEFT:  return "SCROLL LEFT";
        default:                  return "?"; // [JN] Unknown key
    }
}

static char* BK_getBoundKeysString(bound_key_t key)
{
    static char string[50];
    boolean mouseBindExists;

    memset(string, 0, sizeof(string));

    mouseBindExists = bound_key_descriptors[key].mouse_var && *bound_key_descriptors[key].mouse_var != -1;
    if(*bound_key_descriptors[key].key_var != 0)
    {
        M_StringConcat(string, getKeyboardKeyName(*bound_key_descriptors[key].key_var), 50);
        if(mouseBindExists)
            M_StringConcat(string, ", ", 50);
    }
    else if(!mouseBindExists)
    {
        return "---";
    }

    if(mouseBindExists)
        M_StringConcat(string, getMouseKeyName(*bound_key_descriptors[key].mouse_var), 50);
    
    return string;
}

void RD_Menu_Draw_Bindings(int x)
{
    for (int i = 0; i < CurrentMenu->itemCount; ++i)
    {
        if (CurrentMenu->items[i].option != 0)
        {
            boolean bindingThis = isBinding && i == CurrentItPos;

            RD_M_DrawTextSmallENG(bindingThis ? "?" : BK_getBoundKeysString(CurrentMenu->items[i].option),
                                  x + wide_delta, i * 10 + 25,
                                  bindingThis ? CR_WHITE : BK_KeyHasNoBinds(CurrentMenu->items[i].option) ?
                                  CR_DARKRED : CR_NONE);
        }
    }
}

void BK_StartBindingKey(bound_key_t key)
{
    isBinding = true;
    keyToBind = key;
}

void BK_BindKey(event_t* event)
{
    isBinding = false;

    if (event->type == ev_keydown && bound_key_descriptors[keyToBind].key_var)
    {
        if (event->data1 == *(bound_key_descriptors[keyToBind].key_var))
        {
            *bound_key_descriptors[keyToBind].key_var = 0;
        }
        else if (event->data1 != KEY_ESCAPE)
        {
            *bound_key_descriptors[keyToBind].key_var = event->data1;
        }
    }
    else if (event->type == ev_mouse_keydown && bound_key_descriptors[keyToBind].mouse_var)
    {
        if (event->data1 == *(bound_key_descriptors[keyToBind].mouse_var))
        {
            *bound_key_descriptors[keyToBind].mouse_var = -1;
        }
        else
        {
            *bound_key_descriptors[keyToBind].mouse_var = event->data1;
        }
    }

    keyToBind = bk_null;
}

// -----------------------------------------------------------------------------
// BK_ClearBinds
// Clears all binds for given bound_key
// -----------------------------------------------------------------------------
void BK_ClearBinds(bound_key_t key)
{
    *bound_key_descriptors[key].key_var = 0;
    if(bound_key_descriptors[key].mouse_var)
        *bound_key_descriptors[key].mouse_var = -1;
}

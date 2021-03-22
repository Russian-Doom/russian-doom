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
#include "m_controls.h"

bound_key_descriptor bound_key_descriptors[bk_size] = {
    // Movement
    {NULL, "", "", '\0', '\0'}, // bk_null
    {&key_up, "Move Forward", "ldb;tybt dgthtl", 'm', 'l'}, // bk_forward // Движение вперед
    {&key_down, "Move Backward", "ldb;tybt yfpfl", 'm', 'l'}, // bk_backward // Движение назад
    {&key_left, "Turn Left", "gjdjhjn yfktdj", 't', 'g'}, // bk_turn_left // Поворот налево
    {&key_right, "Turn Right", "gjdjhjn yfghfdj", 't', 'g'}, // bk_turn_right // Поворот направо
    {&key_strafeleft, "Strafe Left", ",jrjv dktdj", 's', ','}, // bk_strafe_left // Боком влево
    {&key_straferight, "Strafe Right", ",jrjv dghfdj", 's', ','}, // bk_strafe_right // Боком вправо
    {&key_flyup, "Fly up", "ktntnm ddth[", 'f', 'k'}, // bk_fly_up // Лететь вверх
    {&key_flydown, "Fly down", "ktntnm dybp", 'f', 'k'}, // bk_fly_down // Лететь вниз
    {&key_flycenter, "Drop", "ghbptvkbnmcz", 'd', 'g'}, // bk_fly_center // Приземлиться
    {&key_speed, "Speed On", ",tu", 's', ','}, // bk_speed // Бег
    {&key_strafe, "Strafe On", "ldb;tybt ,jrjv", 's', 'l'}, // bk_strafe // Движение боком
    {&key_use, "Jump", "ghs;jr", 'j', 'g'}, // bk_jump // Прыжок
    {&key_toggleautorun, "Always run", "gjcnjzyysq ,tu", 'a', 'g'}, // bk_toggle_autorun // Постоянный бег
    {&key_use, "Use", "bcgjkmpjdfnm", 'u', 'b'}, // bk_use // Использовать

    // Weapon
    {&key_fire, "Fire/Attack", "fnfrf*cnhtkm,f", 'f', 'f'}, // bk_fire // Атака/стрельба
    {&key_weapon1, "Weapon 1", "jhe;bt 1", 'w', 'j'}, // bk_weapon_1 // Оружие 1
    {&key_weapon2, "Weapon 2", "jhe;bt 2", 'w', 'j'}, // bk_weapon_2 // Оружие 2
    {&key_weapon3, "Weapon 3", "jhe;bt 3", 'w', 'j'}, // bk_weapon_3 // Оружие 3
    {&key_weapon4, "Weapon 4", "jhe;bt 4", 'w', 'j'}, // bk_weapon_4 // Оружие 4
    {&key_weapon5, "Weapon 5", "jhe;bt 5", 'w', 'j'}, // bk_weapon_5 // Оружие 5
    {&key_weapon6, "Weapon 6", "jhe;bt 6", 'w', 'j'}, // bk_weapon_6 // Оружие 6
    {&key_weapon7, "Weapon 7", "jhe;bt 7", 'w', 'j'}, // bk_weapon_7 // Оружие 7
    {&key_weapon8, "Weapon 8", "jhe;bt 8", 'w', 'j'}, // bk_weapon_8 // Оружие 8
    {&key_prevweapon, "Previous weapon", "ghtlsleott jhe;bt", 'p', 'g'}, // bk_weapon_prev // Предыдущее оружие
    {&key_nextweapon, "Next weapon", "cktle.ott jhe;bt", 'n', 'c'}, // bk_weapon_next // Следующее оружие

    // Look
    {&key_lookup, "Look up", "cvjnhtnm ddth[", 'l', 'c'}, // bk_look_up // Смотреть вверх
    {&key_lookdown, "Look down", "cvjnhtnm dybp", 'l', 'c'}, // bk_look_down // Смотреть вниз
    {&key_lookcenter, "Look forward", "wtynhbhjdfnm dpukzl", 'l', 'w'}, // bk_look_center // Центрировать взгляд
    {&key_togglemlook, "Mouse look", "j,pjh vsim.", 'm', 'j'}, // bk_toggle_mlook // Обзор мышью

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
    {&key_map_toggle, "Toggle automap", "jnrhsnm rfhne", 't', 'j'}, // bk_map_toggle // Открыть карту
    {&key_map_zoomin, "Zoom in", "ghb,kbpbnm", 'z', 'g'}, // bk_map_zoom_in // Приблизить
    {&key_map_zoomout, "Zoom out", "jnlfkbnm", 'z', 'j'}, // bk_map_zoom_out // Отдалить
    {&key_map_maxzoom, "Maximum zoom out", "gjkysq vfcinf,", 'm', 'g'}, // bk_map_zoom_max // Полный масштаб
    {&key_map_follow, "Follow mode", "ht;bv cktljdfybz", 'f', 'h'}, // bk_map_follow // Режим следования
    {&key_map_overlay, "Overlay mode", "ht;bv yfkj;tybz", 'o', 'h'}, // bk_map_overlay // Режим наложения
    {&key_map_rotate, "Rotate mode", "ht;bv dhfotybz", 'r', 'h'}, // bk_map_rotate // Режим вращения
    {&key_map_grid, "Toggle grid", "ctnrf", 't', 'c'}, // bk_map_grid // Сетка
    {&key_map_mark, "Mark location", "gjcnfdbnm jnvtnre", 'm', 'g'}, // bk_map_mark // Поставить отметку
    {&key_map_clearmark, "Clear all marks", "e,hfnm jnvtnrb", 'c', 'e'}, // bk_map_clearmark // Убрать отметки

    // Shortcuts and toggles
    {&key_menu_qsave, "Quick save", ",scnhjt cj[hfytybt", 'q', ','}, // bk_save // Быстрое сохранение
    {&key_menu_load, "Quick load", ",scnhfz pfuheprf", 'q', ','}, // bk_load // Быстрая загрузка
    {&key_menu_nextlevel, "Go to next level", "cktle.obq ehjdtym", 'g', 'c'}, // bk_nextlevel // Следующий уровень
    {&key_menu_reloadlevel, "Restart level/demo", "gthtpfgecr ehjdyz", 'r', 'g'}, // bk_reloadlevel // Перезапуск уровня
    {&key_menu_screenshot, "Save a screenshot", "crhbyijn", 's', 'c'}, // bk_screenshot // Скриншот
    {&key_demo_quit, "Finish demo recording", "pfrjyxbnm pfgbcm ltvj", 'f', 'p'}, // bk_finish_demo // Закончить запись демо
    {&key_togglecrosshair, "Crosshair", "ghbwtk", 'c', 'g'}, // bk_toggle_crosshair // Прицел
    {&key_togglefliplvls, "Level flipping", "pthrfkbhjdfybt ehjdyz", 'l', 'p'} // bk_toggle_fliplvls // Зеркалирование уровня
};

bound_key_descriptor* BK_getKeyDescriptor(bound_key_t key)
{
    return &bound_key_descriptors[key];
}

// -----------------------------------------------------------------------------
// BK_getBoundKeysString
// Returns string of names for first 1 physical keys bound to given bound_key
// -----------------------------------------------------------------------------
char* BK_getBoundKeysString(bound_key_t key)
{
    // [JN] Values are simple ASCII table:
    // https://upload.wikimedia.org/wikipedia/commons/7/7b/Ascii_Table-nocolor.svg
    switch(*bound_key_descriptors[key].key_var)
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

// -----------------------------------------------------------------------------
// BK_KeyHasNoBinds
// Returns true if no keys have been bound to given bound_key
// -----------------------------------------------------------------------------
boolean BK_KeyHasNoBinds(bound_key_t key)
{
    return *bound_key_descriptors[key].key_var == 0;
}

// -----------------------------------------------------------------------------
// BK_ClearBinds
// Clears all binds for given bound_key
// -----------------------------------------------------------------------------
void BK_ClearBinds(bound_key_t key)
{
    *bound_key_descriptors[key].key_var = 0;
}

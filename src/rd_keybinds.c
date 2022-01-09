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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_scancode.h>

#include "rd_keybinds.h"
#include "d_name.h"
#include "doomkeys.h"
#include "i_controller.h"
#include "i_input.h"
#include "i_video.h"
#include "jn.h"
#include "m_misc.h"
#include "rd_menu.h"

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

boolean isBinding = false;
bound_key_t keyToBind = bk__null;
boolean isBindsLoaded = false;

bind_descriptor_t* bind_descriptor[bk__size];
boolean keyState[bk__size];
boolean bindClearEnabled = true;

// [Dasperal] This array must be in sync with bound_key_t enum!
static const char* bkToName[] = {
    NULL,
    "Forward",
    "Backward",
    "Turn_left",
    "Turn_right",
    "Strafe_left",
    "Strafe_right",
    "Fly_up",
    "Fly_down",
    "Fly_stop",
    "Speed",
    "Strafe",
    "Jump",
    "Toggle_autorun",
    "Use",
    "Fire",
    "Weapon_1",
    "Weapon_2",
    "Weapon_3",
    "Weapon_4",
    "Weapon_5",
    "Weapon_6",
    "Weapon_7",
    "Weapon_8",
    "Weapon_prev",
    "Weapon_next",
    "Look_up",
    "Look_down",
    "Look_center",
    "Toggle_mlook",
    "Inv_left",
    "Inv_right",
    "Inv_use_artifact",
    "Inv_use_health",
    "Arti_quartz",
    "Arti_urn",
    "Arti_bomb",
    "Arti_tome",
    "Arti_egg",
    "Arti_shadowsphere",
    "Arti_wings",
    "Arti_torch",
    "Arti_invulnerability",
    "Arti_chaosdevice",
    "Arti_all",
    "Arti_blastradius",
    "Arti_teleportother",
    "Arti_boostarmor",
    "Arti_boostmana",
    "Arti_summon",
    "Arti_speed",
    "Arti_healingradius",
    "Map_toggle",
    "Map_zoom_in",
    "Map_zoom_out",
    "Map_zoom_max",
    "Map_follow",
    "Map_overlay",
    "Map_rotate",
    "Map_grid",
    "Map_mark",
    "Map_clearmark",
    "Qsave",
    "Qload",
    "Nextlevel",
    "Reloadlevel",
    "Screenshot",
    "Finish_demo",
    "Toggle_crosshair",
    "Toggle_fliplvls",
    "Spy",
    "Multi_msg",
    "Multi_msg_player_0",
    "Multi_msg_player_1",
    "Multi_msg_player_2",
    "Multi_msg_player_3",
    "Multi_msg_player_4",
    "Multi_msg_player_5",
    "Multi_msg_player_6",
    "Multi_msg_player_7"
};

static int nameToBk[arrlen(bkToName) - 1];
static boolean nameToBk_init = false;

// [Dasperal] This array must be in sync with SDL_Scancode enum!
// Strings in this array must not be changed only added, or it will break existing configs
static const char* kKToName[] = {
    "", "", "", "", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q",    // 0   - 20
    "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "ENTER", // 21  - 41
    "ESCAPE", "BACKSPACE", "TAB", "SPACE", "-", "=", "[", "]", "\\", "", ";", "\'", "`", ",", ".", "/",     // 42  - 56
    "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "SYS_RQ",        // 57  - 70
    "SCROLL_LOCK", "PAUSE", "INSERT", "HOME", "PAGE_UP", "DELETE", "END", "PAGE_DN", "RIGHT", "LEFT",       // 71  - 82
    "DOWN", "UP", "NUM_LOCK", "KP_/", "KP_*", "KP_-", "KP_+", "KP_ENTER", "KP_1", "KP_2", "KP_3", "KP_4",   // 83  - 92
    "KP_5", "KP_6", "KP_7", "KP_8", "KP_9", "KP_0", "KP_,", "", "APP", "", "KP_=", "F13", "F14", "F15",     // 93  - 106
    "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "", "", "",  // 107 - 125
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", // 126 - 151
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", // 152 - 177
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", // 178 - 203
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "LCTRL", "LSHIFT",      // 204 - 225
    "LALT", "", "RCTRL", "RSHIFT", "RALT", ""                                                               // 226 - 231
};

static int nameToKK[arrlen(kKToName)];
static boolean nameToKK_init = false;

// Index is MOUSE buttons enum
// [Dasperal] Strings in this array must not be changed only added, or it will break existing configs
static const char* mKToName[] = {
    "LEFT", "RIGHT", "MIDDLE", "MOUSE_4", "MOUSE_5", "SCROLL_UP", "SCROLL_DOWN", "SCROLL_RIGHT", "SCROLL_LEFT" // 0 - 8
};

static int nameToMK[arrlen(mKToName)];
static boolean nameToMK_init = false;

// Index is CONTROLLER buttons enum
// [Dasperal] Strings in this array must not be changed only added, or it will break existing configs
static const char* cKToName[] = {
    "A", "B", "X", "Y", "BACK", "GUIDE", "START", "LEFT_STICK", "RIGHT_STICK", "LEFT_SHOULDER",            // 0  - 9
    "RIGHT_SHOULDER", "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT", "MISC_1", "PADDLE_1", "PADDLE_2", // 10 - 17
    "PADDLE_3", "PADDLE_4", "TOUCHPAD", "LEFT_TRIGGER", "RIGHT_TRIGGER", "LEFT_TRIGGER_NEG",               // 18 - 23
    "RIGHT_TRIGGER_NEG", "LSX_POS", "LSX_NEG", "LSY_POS", "LSY_NEG", "RSX_POS", "RSX_NEG", "RSY_POS",      // 24 - 31
    "RSY_NEG"                                                                                              // 32
};

static int nameToCK[arrlen(cKToName)];
static boolean nameToCK_init = false;

static SDL_Scancode legacyToScancode(int legacyKey)
{
    // [JN] Values are simple ASCII table:
    // https://upload.wikimedia.org/wikipedia/commons/7/7b/Ascii_Table-nocolor.svg
    switch(legacyKey)
    {
        case KEY_TAB:        return SDL_SCANCODE_TAB;
        case KEY_ENTER:      return SDL_SCANCODE_RETURN;
        case KEY_ESCAPE:     return SDL_SCANCODE_ESCAPE;
        case ' ':            return SDL_SCANCODE_SPACE;
        case '\'':           return SDL_SCANCODE_APOSTROPHE;
        case '*':            return SDL_SCANCODE_8;
        case '+':            return SDL_SCANCODE_EQUALS;
        case ',':            return SDL_SCANCODE_COMMA;
        case KEY_MINUS:      return SDL_SCANCODE_MINUS;
        case '.':            return SDL_SCANCODE_PERIOD;
        case '/':            return SDL_SCANCODE_SLASH;
        case '0':            return SDL_SCANCODE_0;
        case '1':            return SDL_SCANCODE_1;
        case '2':            return SDL_SCANCODE_2;
        case '3':            return SDL_SCANCODE_3;
        case '4':            return SDL_SCANCODE_4;
        case '5':            return SDL_SCANCODE_5;
        case '6':            return SDL_SCANCODE_6;
        case '7':            return SDL_SCANCODE_7;
        case '8':            return SDL_SCANCODE_8;
        case '9':            return SDL_SCANCODE_9;
        case ';':            return SDL_SCANCODE_SEMICOLON;
        case KEY_EQUALS:     return SDL_SCANCODE_EQUALS; // [JN] Indicated as "+" in help screens
        case '[':            return SDL_SCANCODE_LEFTBRACKET;
        case ']':            return SDL_SCANCODE_RIGHTBRACKET;
        case '\\':           return SDL_SCANCODE_BACKSLASH;
        case '`':            return SDL_SCANCODE_GRAVE;
        case 'a':            return SDL_SCANCODE_A;
        case 'b':            return SDL_SCANCODE_B;
        case 'c':            return SDL_SCANCODE_C;
        case 'd':            return SDL_SCANCODE_D;
        case 'e':            return SDL_SCANCODE_E;
        case 'f':            return SDL_SCANCODE_F;
        case 'g':            return SDL_SCANCODE_G;
        case 'h':            return SDL_SCANCODE_H;
        case 'i':            return SDL_SCANCODE_I;
        case 'j':            return SDL_SCANCODE_J;
        case 'k':            return SDL_SCANCODE_K;
        case 'l':            return SDL_SCANCODE_L;
        case 'm':            return SDL_SCANCODE_M;
        case 'n':            return SDL_SCANCODE_N;
        case 'o':            return SDL_SCANCODE_O;
        case 'p':            return SDL_SCANCODE_P;
        case 'q':            return SDL_SCANCODE_Q;
        case 'r':            return SDL_SCANCODE_R;
        case 's':            return SDL_SCANCODE_S;
        case 't':            return SDL_SCANCODE_T;
        case 'u':            return SDL_SCANCODE_U;
        case 'v':            return SDL_SCANCODE_V;
        case 'w':            return SDL_SCANCODE_W;
        case 'x':            return SDL_SCANCODE_X;
        case 'y':            return SDL_SCANCODE_Y;
        case 'z':            return SDL_SCANCODE_Z;
        case KEY_BACKSPACE:  return SDL_SCANCODE_BACKSPACE;
        case KEY_RCTRL:      return SDL_SCANCODE_RCTRL;
        case KEYP_ENTER:     return SDL_SCANCODE_KP_ENTER;
        case KEYP_EQUALS:    return SDL_SCANCODE_KP_EQUALS;
        case KEYP_PERIOD:    return SDL_SCANCODE_KP_PERIOD;
        case KEY_LSHIFT:     return SDL_SCANCODE_LSHIFT;
        case KEY_LCTRL:      return SDL_SCANCODE_LCTRL;
        case KEY_LALT:       return SDL_SCANCODE_LALT;
        case KEY_LEFTARROW:  return SDL_SCANCODE_LEFT;
        case KEY_UPARROW:    return SDL_SCANCODE_UP;
        case KEY_RIGHTARROW: return SDL_SCANCODE_RIGHT;
        case KEY_DOWNARROW:  return SDL_SCANCODE_DOWN;
        case KEYP_MULTIPLY:  return SDL_SCANCODE_KP_MULTIPLY;
        case KEYP_MINUS:     return SDL_SCANCODE_KP_MINUS;
        case KEYP_PLUS:      return SDL_SCANCODE_KP_PLUS;
        case KEYP_DIVIDE:    return SDL_SCANCODE_KP_DIVIDE;
        case KEYP_9:         return SDL_SCANCODE_KP_9;
        case KEYP_8:         return SDL_SCANCODE_KP_8;
        case KEY_RSHIFT:     return SDL_SCANCODE_RSHIFT;
        case KEYP_0:         return SDL_SCANCODE_KP_0;
        case KEY_RALT:       return SDL_SCANCODE_RALT;
        case KEYP_1:         return SDL_SCANCODE_KP_1;
        case KEY_CAPSLOCK:   return SDL_SCANCODE_CAPSLOCK;
        case KEY_F1:         return SDL_SCANCODE_F1;
        case KEY_F2:         return SDL_SCANCODE_F2;
        case KEY_F3:         return SDL_SCANCODE_F3;
        case KEY_F4:         return SDL_SCANCODE_F4;
        case KEY_F5:         return SDL_SCANCODE_F5;
        case KEY_F6:         return SDL_SCANCODE_F6;
        case KEY_F7:         return SDL_SCANCODE_F7;
        case KEY_F8:         return SDL_SCANCODE_F8;
        case KEY_F9:         return SDL_SCANCODE_F9;
        case KEY_F10:        return SDL_SCANCODE_F10;
        case KEY_NUMLOCK:    return SDL_SCANCODE_NUMLOCKCLEAR;
        case KEY_SCRLCK:     return SDL_SCANCODE_SCROLLLOCK;
        case KEY_HOME:       return SDL_SCANCODE_HOME;
        case KEYP_2:         return SDL_SCANCODE_KP_2;
        case KEY_PGUP:       return SDL_SCANCODE_PAGEUP;
        case KEYP_3:         return SDL_SCANCODE_KP_3;
        case KEYP_4:         return SDL_SCANCODE_KP_4;
        case KEYP_5:         return SDL_SCANCODE_KP_5;
        case KEYP_6:         return SDL_SCANCODE_KP_6;
        case KEYP_7:         return SDL_SCANCODE_KP_7;
        case KEY_END:        return SDL_SCANCODE_END;
        case KEY_PGDN:       return SDL_SCANCODE_PAGEDOWN;
        case KEY_INS:        return SDL_SCANCODE_INSERT;
        case KEY_DEL:        return SDL_SCANCODE_DELETE;
        case KEY_F11:        return SDL_SCANCODE_F11;
        case KEY_F12:        return SDL_SCANCODE_F12;
        case KEY_PRTSCR:     return SDL_SCANCODE_PRINTSCREEN;
        case KEY_F13:        return SDL_SCANCODE_F13;
        case KEY_F14:        return SDL_SCANCODE_F14;
        case KEY_F15:        return SDL_SCANCODE_F15;
        case KEY_F16:        return SDL_SCANCODE_F16;
        case KEY_F17:        return SDL_SCANCODE_F17;
        case KEY_F18:        return SDL_SCANCODE_F18;
        case KEY_F19:        return SDL_SCANCODE_F19;
        case KEY_F20:        return SDL_SCANCODE_F20;
        case KEY_F21:        return SDL_SCANCODE_F21;
        case KEY_F22:        return SDL_SCANCODE_F22;
        case KEY_F23:        return SDL_SCANCODE_F23;
        case KEY_F24:        return SDL_SCANCODE_F24;
        case KEY_APP:        return SDL_SCANCODE_APPLICATION;
        case KEY_PAUSE:      return SDL_SCANCODE_PAUSE;
        default:             return SDL_SCANCODE_UNKNOWN; // [JN] Unknown key
    }
}

// Translates the SDL key to a value of the type found in doomkeys.h
static int scancodeToLegacy(SDL_Scancode scancode)
{
    static const int scancode_translate_table[] = SCANCODE_TO_KEYS_ARRAY;

    switch (scancode)
    {
        case SDL_SCANCODE_LCTRL:
            return KEY_LCTRL;
        case SDL_SCANCODE_RCTRL:
            return KEY_RCTRL;
        case SDL_SCANCODE_LSHIFT:
            return KEY_LSHIFT;
        case SDL_SCANCODE_RSHIFT:
            return KEY_RSHIFT;
        case SDL_SCANCODE_LALT:
            return KEY_LALT;
        case SDL_SCANCODE_RALT:
            return KEY_RALT;
        default:
            if (scancode >= 0 && scancode < arrlen(scancode_translate_table))
            {
                return scancode_translate_table[scancode];
            }
            else
            {
                return 0;
            }
    }
}

static device_t getEventDevice(event_t* event)
{
    switch(event->type)
    {
        case ev_keydown:
        case ev_keyup:
            return keyboard;
        case ev_mouse_keydown:
        case ev_mouse_keyup:
            return mouse;
        case ev_controller_keydown:
        case ev_controller_keyup:
            return controller;
        default:
            return -1;
    }
}

void BK_ProcessKey(event_t* event)
{
    device_t device = getEventDevice(event);

    for(int i = 0; i < bk__size; i++)
    {
        bind_descriptor_t* bind = bind_descriptor[i];
        while(bind)
        {
            if(bind->device == device && bind->key == event->data1)
            {
                keyState[i] = event->type == ev_keydown ||
                        event->type == ev_mouse_keydown ||
                        event->type == ev_controller_keydown;
                return;
            }
            bind = bind->next;
        }
    }
}

boolean BK_isKeyPressed(bound_key_t key)
{
    return keyState[key];
}

boolean BK_isKeyDown(event_t* event, bound_key_t key)
{
    if(event->type == ev_keydown
    || event->type == ev_mouse_keydown
    || event->type == ev_controller_keydown)
    {
        device_t device = getEventDevice(event);
        bind_descriptor_t* bind = bind_descriptor[key];
        while(bind)
        {
            if(bind->device == device && bind->key == event->data1)
                return true;
            bind = bind->next;
        }
    }
    return false;
}

boolean BK_isKeyUp(event_t* event, bound_key_t key)
{
    if(event->type == ev_keyup
    || event->type == ev_mouse_keyup
    || event->type == ev_controller_keyup)
    {
        device_t device = getEventDevice(event);
        bind_descriptor_t* bind = bind_descriptor[key];
        while(bind)
        {
            if(bind->device == device && bind->key == event->data1)
                return true;
            bind = bind->next;
        }
    }
    return false;
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
    return bind_descriptor[key] == NULL;
}

static const char* getKeyboardKeyName(int key)
{
    // [JN] Values are simple ASCII table:
    // https://upload.wikimedia.org/wikipedia/commons/7/7b/Ascii_Table-nocolor.svg
    switch(key)
    {
        case 0:              return "---";
        case KEY_TAB:        return "TAB";
        case KEY_ENTER:      return "ENTER";
        case ' ':            return "SPACE";
        case '\'':           return "'";
        case '*':            return "*";
        case '+':            return "+";
        case ',':            return ",";
        case KEY_MINUS:      return "-";
        case '.':            return ".";
        case '/':            return "/";
        case '0':            return "0";
        case '1':            return "1";
        case '2':            return "2";
        case '3':            return "3";
        case '4':            return "4";
        case '5':            return "5";
        case '6':            return "6";
        case '7':            return "7";
        case '8':            return "8";
        case '9':            return "9";
        case ';':            return ";";
        case KEY_EQUALS:     return "="; // [JN] Indicated as "+" in help screens
        case '[':            return "[";
        case ']':            return "]";
        case '\\':           return "\\";
        case '`':            return "TILDE";
        case 'a':            return "A";
        case 'b':            return "B";
        case 'c':            return "C";
        case 'd':            return "D";
        case 'e':            return "E";
        case 'f':            return "F";
        case 'g':            return "G";
        case 'h':            return "H";
        case 'i':            return "I";
        case 'j':            return "J";
        case 'k':            return "K";
        case 'l':            return "L";
        case 'm':            return "M";
        case 'n':            return "N";
        case 'o':            return "O";
        case 'p':            return "P";
        case 'q':            return "Q";
        case 'r':            return "R";
        case 's':            return "S";
        case 't':            return "T";
        case 'u':            return "U";
        case 'v':            return "V";
        case 'w':            return "W";
        case 'x':            return "X";
        case 'y':            return "Y";
        case 'z':            return "Z";
        case KEY_BACKSPACE:  return "BCKSP";
        case KEY_RCTRL:      return "RCTRL";
        case KEYP_ENTER:     return "KP_ENTER";
        case KEYP_EQUALS:    return "KP_=";
        case KEYP_PERIOD:    return "KP_.";
        case KEY_LSHIFT:     return "LSHIFT";
        case KEY_LCTRL:      return "LCTRL";
        case KEY_LALT:       return "LALT";
        case KEY_LEFTARROW:  return "LEFT";
        case KEY_UPARROW:    return "UP";
        case KEY_RIGHTARROW: return "RIGHT";
        case KEY_DOWNARROW:  return "DOWN";
        case KEYP_MULTIPLY:  return "KP_*";
        case KEYP_MINUS:     return "KP_-";
        case KEYP_PLUS:      return "KP_+";
        case KEYP_DIVIDE:    return "KP_/";
        case KEYP_9:         return "KP_9";
        case KEYP_8:         return "KP_8";
        case KEY_RSHIFT:     return "RSHIFT";
        case KEYP_0:         return "KP_0";
        case KEY_RALT:       return "RALT";
        case KEYP_1:         return "KP_1";
        case KEY_CAPSLOCK:   return "CAPS LOCK";
        case KEY_F1:         return "F1";
        case KEY_F2:         return "F2";
        case KEY_F3:         return "F3";
        case KEY_F4:         return "F4";
        case KEY_F5:         return "F5";
        case KEY_F6:         return "F6";
        case KEY_F7:         return "F7";
        case KEY_F8:         return "F8";
        case KEY_F9:         return "F9";
        case KEY_F10:        return "F10";
        case KEY_NUMLOCK:    return "NUM LOCK";
        case KEY_SCRLCK:     return "SCROLL";
        case KEY_HOME:       return "HOME";
        case KEYP_2:         return "KP_2";
        case KEY_PGUP:       return "PGEUP";
        case KEYP_3:         return "KP_3";
        case KEYP_4:         return "KP_4";
        case KEYP_5:         return "KP_5";
        case KEYP_6:         return "KP_6";
        case KEYP_7:         return "KP_7";
        case KEY_END:        return "END";
        case KEY_PGDN:       return "PGDN";
        case KEY_INS:        return "INS";
        case KEY_DEL:        return "DEL";
        case KEY_F11:        return "F11";
        case KEY_F12:        return "F12";
        case KEY_PRTSCR:     return "SYS RQ";
        case KEY_F13:        return "F13";
        case KEY_F14:        return "F14";
        case KEY_F15:        return "F15";
        case KEY_F16:        return "F16";
        case KEY_F17:        return "F17";
        case KEY_F18:        return "F18";
        case KEY_F19:        return "F19";
        case KEY_F20:        return "F20";
        case KEY_F21:        return "F21";
        case KEY_F22:        return "F22";
        case KEY_F23:        return "F23";
        case KEY_F24:        return "F24";
        case KEY_APP:        return "APP";
        case KEY_PAUSE:      return "PAUSE";
        default:             return "?"; // [JN] Unknown key
    }
}

static char* getMouseKeyName(int i)
{
    switch (i)
    {
        case  MOUSE_LEFT:         return "MOUSE 1";
        case  MOUSE_RIGHT:        return "MOUSE 2";
        case  MOUSE_MIDDLE:       return "MOUSE 3";
        case  MOUSE_4:            return "MOUSE 4";
        case  MOUSE_5:            return "MOUSE 5";
        case  MOUSE_SCROLL_UP:    return "MW_UP";
        case  MOUSE_SCROLL_DOWN:  return "MW_DOWN";
        case  MOUSE_SCROLL_RIGHT: return "MW_RIGHT";
        case  MOUSE_SCROLL_LEFT:  return "MW_LEFT";
        default:                  return "?"; // [JN] Unknown key
    }
}

static char* getControllerKeyName(int i)
{
    switch (i)
    {
        case CONTROLLER_A:                      return "PAD_A";
        case CONTROLLER_B:                      return "PAD_B";
        case CONTROLLER_X:                      return "PAD_X";
        case CONTROLLER_Y:                      return "PAD_Y";
        case CONTROLLER_BACK:                   return "PAD_BACK";
        case CONTROLLER_GUIDE:                  return "PAD_GUIDE";
        case CONTROLLER_START:                  return "PAD_START";
        case CONTROLLER_LEFT_STICK:             return "PAD_L3";
        case CONTROLLER_RIGHT_STICK:            return "PAD_R3";
        case CONTROLLER_LEFT_SHOULDER:          return "PAD_L1";
        case CONTROLLER_RIGHT_SHOULDER:         return "PAD_R1";
        case CONTROLLER_DPAD_UP:                return "DPAD_UP";
        case CONTROLLER_DPAD_DOWN:              return "DPAD_DOWN";
        case CONTROLLER_DPAD_LEFT:              return "DPAD_LEFT";
        case CONTROLLER_DPAD_RIGHT:             return "DPAD_RIGHT";
        case CONTROLLER_MISC1:                  return "PAD_MISC";
        case CONTROLLER_PADDLE1:                return "PADDLE 1";
        case CONTROLLER_PADDLE2:                return "PADDLE 2";
        case CONTROLLER_PADDLE3:                return "PADDLE 3";
        case CONTROLLER_PADDLE4:                return "PADDLE 4";
        case CONTROLLER_TOUCHPAD:               return "PAD_TOUCH";
        case CONTROLLER_LEFT_TRIGGER:           return "PAD_L2";
        case CONTROLLER_RIGHT_TRIGGER:          return "PAD_R2";
        case CONTROLLER_LEFT_TRIGGER_NEGATIVE:  return "PAD_L2_NEG";
        case CONTROLLER_RIGHT_TRIGGER_NEGATIVE: return "PAD_R2_NEG";
        case CONTROLLER_LSX_POSITIVE:           return "LSX_POS";
        case CONTROLLER_LSX_NEGATIVE:           return "LSX_NEG";
        case CONTROLLER_LSY_POSITIVE:           return "LSY_POS";
        case CONTROLLER_LSY_NEGATIVE:           return "LSY_NEG";
        case CONTROLLER_RSX_POSITIVE:           return "RSX_POS";
        case CONTROLLER_RSX_NEGATIVE:           return "RSX_NEG";
        case CONTROLLER_RSY_POSITIVE:           return "RSY_POS";
        case CONTROLLER_RSY_NEGATIVE:           return "RSY_NEG";
        default:                                return "?"; // [JN] Unknown key
    }
}

static char* BK_getBoundKeysString(bound_key_t key)
{
    static char string[50];
    bind_descriptor_t* bind;

    if(bind_descriptor[key] == NULL)
        return "---";

    memset(string, 0, sizeof(string));

    bind = bind_descriptor[key];
    while(bind)
    {
        switch(bind->device)
        {
            case keyboard:
                M_StringConcat(string, getKeyboardKeyName(bind->key), 50);
                break;
            case mouse:
                M_StringConcat(string, getMouseKeyName(bind->key), 50);
                break;
            case controller:
                M_StringConcat(string, getControllerKeyName(bind->key), 50);
                break;
            default:
                break;
        }

        if(bind->next)
        {
            M_StringConcat(string, ", ", 50);

            if(aspect_ratio < 2 || screenblocks  == 9)
            {
                M_StringConcat(string, "...", 50);
                break;
            }
        }

        bind = bind->next;
    }
    
    return string;
}

void RD_Menu_Draw_Bindings(int x)
{
    Translation_CR_t noBindTranslation;
    Translation_CR_t bindingTranslation;

    switch (RD_GameType)
    {
        default:
        case gt_Doom:
            noBindTranslation = CR_DARKRED;
            bindingTranslation = CR_WHITE;
            break;
        case gt_Heretic:
            noBindTranslation = CR_WHITE2GRAY_HERETIC;
            bindingTranslation = CR_WHITE2DARKGOLD_HERETIC;
            break;
        case gt_Hexen:
            noBindTranslation = CR_GRAY2GDARKGRAY_HEXEN;
            bindingTranslation = CR_GRAY2DARKGOLD_HEXEN;
            break;
    }

    for (int i = 0; i < CurrentMenu->itemCount; ++i)
    {
        if (CurrentMenu->items[i].option != 0)
        {
            boolean bindingThis = isBinding && i == CurrentItPos;

            RD_M_DrawTextSmallENG(bindingThis ? "?" : BK_getBoundKeysString(CurrentMenu->items[i].option),
                                  x + wide_delta, i * 10 + 25,
                                  bindingThis ? bindingTranslation : BK_KeyHasNoBinds(CurrentMenu->items[i].option) ?
                                  noBindTranslation : CR_NONE);
        }
    }
}

void BK_StartBindingKey(bound_key_t key)
{
    isBinding = true;
    keyToBind = key;
}

// -----------------------------------------------------------------------------
// RemoveKeyFromBinds
// Removes given key of device from all bound_keys
// -----------------------------------------------------------------------------
void RemoveKeyFromBinds(device_t device, int key)
{
    bind_descriptor_t *prevDescriptor, *tmp;

    for(int i = bk_forward; i < bk__serializable; ++i)
    {
        if(bind_descriptor[i] == NULL)
            continue;

        if(bind_descriptor[i]->device == device && bind_descriptor[i]->key == key)
        {
            tmp = bind_descriptor[i];
            bind_descriptor[i] = bind_descriptor[i]->next;
            free(tmp);
            continue;
        }

        prevDescriptor = bind_descriptor[i];
        tmp = prevDescriptor->next;
        while(tmp)
        {
            if(tmp->device == device && tmp->key == key)
            {
                prevDescriptor->next = tmp->next;
                free(tmp);
                break; // from while loop
            }
            else
            {
                prevDescriptor = tmp;
                tmp = prevDescriptor->next;
            }
        }
    }
}

static void AddBind(bound_key_t boundKey, device_t device, int key)
{
    bind_descriptor_t* bind = bind_descriptor[boundKey];

    if(bind == NULL)
    {
        if(bindClearEnabled)
            RemoveKeyFromBinds(device, key);
        bind = malloc(sizeof(bind_descriptor));
        bind->next = NULL;
        bind->device = device;
        bind->key = key;

        bind_descriptor[boundKey] = bind;
    }
    else
    {
        bind_descriptor_t* prevBind = NULL;

        // Iterate binds
        while(bind)
        {
            if(bind->device == device && bind->key == key)
            {
                if(bindClearEnabled)
                {
                    // Clear bind
                    if (prevBind)
                        prevBind->next = bind->next;
                    else
                        bind_descriptor[boundKey] = bind->next;
                    free(bind);
                }
                return;
            }
            prevBind = bind;
            bind = bind->next;
        }

        // Add new bind
        if(bindClearEnabled)
            RemoveKeyFromBinds(device, key);
        bind = malloc(sizeof(bind_descriptor));
        bind->next = NULL;
        bind->device = device;
        bind->key = key;

        prevBind->next = bind;
    }
}

void BK_BindKey(event_t* event)
{
    // [Dasperal] Prohibit binding of some keyboard keys and gamepad menu key
    if((event->type == ev_keydown
    && (event->data1 == KEY_F1
    || event->data1 == KEY_F2
    || event->data1 == KEY_F3
    || event->data1 == KEY_F4
    || event->data1 == KEY_F5
    || event->data1 == KEY_F7
    || event->data1 == KEY_F8
    || event->data1 == KEY_F10
    || event->data1 == KEY_F11
    || event->data1 == KEY_EQUALS
    || event->data1 == KEY_MINUS
    || event->data1 == KEY_PAUSE))
    || (event->type == ev_controller_keydown && event->data1 == CONTROLLER_START))
    {
        return;
    }

    isBinding = false;

    if(!BK_isKeyDown(event, bk_menu_activate))
    {
        AddBind(keyToBind, getEventDevice(event), event->data1);
    }

    keyToBind = bk__null;
}

// -----------------------------------------------------------------------------
// BK_ClearBinds
// Clears all binds for given bound_key
// -----------------------------------------------------------------------------
void BK_ClearBinds(bound_key_t key)
{
    bind_descriptor_t* tmp;

    while(bind_descriptor[key])
    {
        tmp = bind_descriptor[key];
        bind_descriptor[key] = bind_descriptor[key]->next;
        free(tmp);
    }
}

void BK_AddBindingsToSystemKeys()
{
    bindClearEnabled = false;

    // Keyboard
    AddBind(bk_left,  keyboard, KEY_LEFTARROW);
    AddBind(bk_right, keyboard, KEY_RIGHTARROW);
    AddBind(bk_up,    keyboard, KEY_UPARROW);
    AddBind(bk_down,  keyboard, KEY_DOWNARROW);

    AddBind(bk_menu_activate,  keyboard, KEY_ESCAPE);
    AddBind(bk_menu_back,      keyboard, KEY_BACKSPACE);
    AddBind(bk_menu_select,    keyboard, KEY_ENTER);
    AddBind(bk_menu_select,    keyboard, KEYP_ENTER);
    AddBind(bk_menu_page_next, keyboard, KEY_PGDN);
    AddBind(bk_menu_page_prev, keyboard, KEY_PGUP);

    AddBind(bk_confirm, keyboard, 'y');
    AddBind(bk_abort,   keyboard, 'n');
    AddBind(bk_abort,   keyboard, KEY_ESCAPE);

    AddBind(bk_menu_help,   keyboard, KEY_F1);
    AddBind(bk_menu_save,   keyboard, KEY_F2);
    AddBind(bk_menu_load,   keyboard, KEY_F3);
    AddBind(bk_menu_volume, keyboard, KEY_F4);
    AddBind(bk_detail,      keyboard, KEY_F5);
    AddBind(bk_endgame,     keyboard, KEY_F7);
    AddBind(bk_messages,    keyboard, KEY_F8);
    AddBind(bk_quit,        keyboard, KEY_F10);
    AddBind(bk_gamma,       keyboard, KEY_F11);
    AddBind(bk_screen_inc,  keyboard, KEY_EQUALS);
    AddBind(bk_screen_dec,  keyboard, KEY_MINUS);
    AddBind(bk_pause,       keyboard, KEY_PAUSE);

    // Mouse
    AddBind(bk_left,  mouse, MOUSE_SCROLL_LEFT);
    AddBind(bk_right, mouse, MOUSE_SCROLL_RIGHT);
    AddBind(bk_up,    mouse, MOUSE_SCROLL_UP);
    AddBind(bk_down,  mouse, MOUSE_SCROLL_DOWN);

    AddBind(bk_menu_back,      mouse, MOUSE_RIGHT);
    AddBind(bk_menu_select,    mouse, MOUSE_LEFT);
    AddBind(bk_menu_page_next, mouse, MOUSE_4);
    AddBind(bk_menu_page_prev, mouse, MOUSE_5);

    // Controller
    AddBind(bk_left,  controller, CONTROLLER_DPAD_LEFT);
    AddBind(bk_right, controller, CONTROLLER_DPAD_RIGHT);
    AddBind(bk_up,    controller, CONTROLLER_DPAD_UP);
    AddBind(bk_down,  controller, CONTROLLER_DPAD_DOWN);

    AddBind(bk_menu_activate,  controller, CONTROLLER_START);
    AddBind(bk_menu_back,      controller, CONTROLLER_B);
    AddBind(bk_menu_select,    controller, CONTROLLER_A);
    AddBind(bk_menu_page_next, controller, CONTROLLER_RIGHT_SHOULDER);
    AddBind(bk_menu_page_prev, controller, CONTROLLER_LEFT_SHOULDER);

    AddBind(bk_confirm, controller, CONTROLLER_A);
    AddBind(bk_abort,   controller, CONTROLLER_B);

    bindClearEnabled = true;
};

void BK_ApplyDefaultBindings()
{
    bindClearEnabled = false;

    // Keyboard
    AddBind(bk_forward,        keyboard, 'w');
    AddBind(bk_backward,       keyboard, 's');
    AddBind(bk_turn_left,      keyboard, KEY_LEFTARROW);
    AddBind(bk_turn_right,     keyboard, KEY_RIGHTARROW);
    AddBind(bk_strafe_left,    keyboard, 'a');
    AddBind(bk_strafe_right,   keyboard, 'd');
    AddBind(bk_speed,          keyboard, KEY_LSHIFT);
    AddBind(bk_speed,          keyboard, KEY_RSHIFT);
    AddBind(bk_strafe,         keyboard, KEY_LALT);
    AddBind(bk_strafe,         keyboard, KEY_RALT);
    AddBind(bk_toggle_autorun, keyboard, KEY_CAPSLOCK);

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        AddBind(bk_fly_up,   keyboard, KEY_PGUP);
        AddBind(bk_fly_down, keyboard, KEY_INS);
        AddBind(bk_fly_stop, keyboard, KEY_HOME);
    }

    AddBind(bk_use, keyboard, 'e');
    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        AddBind(bk_use,     keyboard, ' ');
    }
    else
    {
        AddBind(bk_jump,    keyboard, ' ');
    }

    AddBind(bk_fire, keyboard, KEY_LCTRL);

    AddBind(bk_weapon_1,    keyboard, '1');
    AddBind(bk_weapon_2,    keyboard, '2');
    AddBind(bk_weapon_3,    keyboard, '3');
    AddBind(bk_weapon_4,    keyboard, '4');

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        AddBind(bk_weapon_5, keyboard, '5');
        AddBind(bk_weapon_6, keyboard, '6');
        AddBind(bk_weapon_7, keyboard, '7');
    }
    if(RD_GameType == gt_Doom)
    {
        AddBind(bk_weapon_8, keyboard, '8');
    }

    AddBind(bk_toggle_mlook, keyboard, '`');

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        AddBind(bk_look_up,     keyboard, KEY_PGDN);
        AddBind(bk_look_down,   keyboard, KEY_DEL);
        AddBind(bk_look_center, keyboard, KEY_END);

        AddBind(bk_inv_left,         keyboard, '[');
        AddBind(bk_inv_right,        keyboard, ']');
        AddBind(bk_inv_use_artifact, keyboard, KEY_ENTER);
    }

    AddBind(bk_map_toggle,   keyboard, KEY_TAB);
    AddBind(bk_map_zoom_in,  keyboard, '=');
    AddBind(bk_map_zoom_out, keyboard, '-');
    AddBind(bk_map_zoom_max, keyboard, '0');
    AddBind(bk_map_follow,   keyboard, 'f');
    AddBind(bk_map_overlay,  keyboard, 'o');
    AddBind(bk_map_rotate,   keyboard, 'r');
    AddBind(bk_map_grid,     keyboard, 'g');

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        AddBind(bk_map_mark,      keyboard, 'm');
        AddBind(bk_map_clearmark, keyboard, 'c');
    }

    AddBind(bk_qsave,            keyboard, KEY_F6);
    AddBind(bk_qload,            keyboard, KEY_F9);
    AddBind(bk_screenshot,       keyboard, KEY_PRTSCR);
    AddBind(bk_finish_demo,      keyboard, 'q');
    AddBind(bk_toggle_crosshair, keyboard, 'x');

    AddBind(bk_spy,                keyboard, KEY_F12);
    AddBind(bk_multi_msg,          keyboard, 't');

    if(RD_GameType == gt_Doom)
    {
        AddBind(bk_multi_msg_player_0, keyboard, 'g');
        AddBind(bk_multi_msg_player_1, keyboard, 'i');
        AddBind(bk_multi_msg_player_2, keyboard, 'b');
        AddBind(bk_multi_msg_player_3, keyboard, 'r');
    }
    if(RD_GameType == gt_Heretic)
    {
        AddBind(bk_multi_msg_player_0, keyboard, 'g');
        AddBind(bk_multi_msg_player_1, keyboard, 'y');
        AddBind(bk_multi_msg_player_2, keyboard, 'r');
        AddBind(bk_multi_msg_player_3, keyboard, 'b');
    }
    if(RD_GameType == gt_Hexen)
    {
        AddBind(bk_multi_msg_player_0, keyboard, 'b');
        AddBind(bk_multi_msg_player_1, keyboard, 'r');
        AddBind(bk_multi_msg_player_2, keyboard, 'y');
        AddBind(bk_multi_msg_player_3, keyboard, 'g');
        AddBind(bk_multi_msg_player_4, keyboard, 'j');
        AddBind(bk_multi_msg_player_5, keyboard, 'w');
        AddBind(bk_multi_msg_player_6, keyboard, 'h');
        AddBind(bk_multi_msg_player_7, keyboard, 'p');
    }

    // Mouse
    AddBind(bk_fire,        mouse, MOUSE_LEFT);
    AddBind(bk_weapon_prev, mouse, MOUSE_SCROLL_UP);
    AddBind(bk_weapon_next, mouse, MOUSE_SCROLL_DOWN);
    AddBind(bk_forward,     mouse, MOUSE_MIDDLE);
    AddBind(bk_strafe,      mouse, MOUSE_RIGHT);

    // Controller
    AddBind(bk_look_center, controller, CONTROLLER_RIGHT_STICK);

    bindClearEnabled = true;
}

void BK_LoadBindings(void* file)
{
    int bind;
    char strparm[100];
    char* ptr;

    bindClearEnabled = false;
    while(!feof(file))
    {
        if(fscanf(file, "%3d %99[^\n]\n", &bind, strparm) != 2)
        {
            // end of key binds section
            break;
        }

        if(bind >= bk__serializable)
            continue;

        ptr = strparm;
        while(*ptr != '\0')
        {
            char deviceChar;
            int key;
            int charsToSkip;
            device_t device;

            if(sscanf(ptr, "%c_%3d%n", &deviceChar, &key, &charsToSkip) != 2)
            {
                ptr += charsToSkip + 1;
                continue;
            }
            ptr += charsToSkip + 1;

            switch(deviceChar)
            {
                case 'k':
                    device = keyboard;
                    break;
                case 'm':
                    device = mouse;
                    break;
                case 'c':
                    device = controller;
                    break;
                default:
                    device = -1;
            }

            AddBind(bind, device, key);
        }
    }
    bindClearEnabled = true;
    isBindsLoaded = true;
}

static int nameToBk_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, bkToName[*((int*) member)]);
}

static int nameToKK_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, kKToName[*((int*) member)]);
}

static int nameToMK_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, mKToName[*((int*) member)]);
}

static int nameToCK_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, cKToName[*((int*) member)]);
}

static int nameToBk_SortComparator(const void *one, const void *other)
{
    return strcmp(bkToName[*((int*) one)], bkToName[*((int*) other)]);
}

static int nameToKK_SortComparator(const void *one, const void *other)
{
    return strcmp(kKToName[*((int*) one)], kKToName[*((int*) other)]);
}

static int nameToMK_SortComparator(const void *one, const void *other)
{
    return strcmp(mKToName[*((int*) one)], mKToName[*((int*) other)]);
}

static int nameToCK_SortComparator(const void *one, const void *other)
{
    return strcmp(cKToName[*((int*) one)], cKToName[*((int*) other)]);
}

static void prepareIndex()
{
    if(!nameToBk_init)
    {
        for(int i = 0; i < arrlen(nameToBk); ++i)
        {
            nameToBk[i] = i + 1;
        }
        qsort(nameToBk, arrlen(nameToBk),
              sizeof(int), nameToBk_SortComparator);
        nameToBk_init = true;
    }

    if(!nameToKK_init)
    {
        for(int i = 0; i < arrlen(nameToKK); ++i)
        {
            nameToKK[i] = i;
        }
        qsort(nameToKK, arrlen(nameToKK),
              sizeof(int), nameToKK_SortComparator);
        nameToKK_init = true;
    }

    if(!nameToMK_init)
    {
        for(int i = 0; i < arrlen(nameToMK); ++i)
        {
            nameToMK[i] = i;
        }
        qsort(nameToMK, arrlen(nameToMK),
              sizeof(int), nameToMK_SortComparator);
        nameToMK_init = true;
    }

    if(!nameToCK_init)
    {
        for(int i = 0; i < arrlen(nameToCK); ++i)
        {
            nameToCK[i] = i;
        }
        qsort(nameToCK, arrlen(nameToCK),
              sizeof(int), nameToCK_SortComparator);
        nameToCK_init = true;
    }
}

boolean KeybindsHandler_Handles(char* sectionName)
{
    if(strcmp("Keybinds", sectionName) == 0)
    {
        prepareIndex();
        return true;
    }
    else
        return false;
}

void KeybindsHandler_HandleLine(char* keyName, char *value, size_t valueSize)
{
    int* bsearchResult;
    bound_key_t bind;

    bsearchResult = bsearch(keyName,
                            nameToBk, arrlen(bkToName) - 1, // bk_forward = 1
                            sizeof(int), nameToBk_Comparator);
    if(bsearchResult == NULL)
        return;

    bind = *bsearchResult;
    bindClearEnabled = false;
    while(*value != '\0')
    {
        char deviceChar;
        device_t device;
        int charsToSkip;
        char keyString[50];
        int key;

        if(sscanf(value, "\"%c_%50[^\"]\"%n", &deviceChar, keyString, &charsToSkip) != 2)
        {
            value += charsToSkip;
            if(*value != '\0')
                value += 2;
            continue;
        }
        value += charsToSkip;
        if(*value != '\0')
            value += 2;

        switch(deviceChar)
        {
            case 'k':
                device = keyboard;
                bsearchResult = bsearch(keyString,
                                        nameToKK, arrlen(kKToName),
                                        sizeof(int), nameToKK_Comparator);
                if(bsearchResult == NULL)
                    continue;
                key = scancodeToLegacy(*bsearchResult);
                break;
            case 'm':
                device = mouse;
                bsearchResult = bsearch(keyString,
                                        nameToMK, arrlen(mKToName),
                                        sizeof(int), nameToMK_Comparator);
                if(bsearchResult == NULL)
                    continue;
                key = *bsearchResult;
                break;
            case 'c':
                device = controller;
                bsearchResult = bsearch(keyString,
                                        nameToCK, arrlen(cKToName),
                                        sizeof(int), nameToCK_Comparator);
                if(bsearchResult == NULL)
                    continue;
                key = *bsearchResult;
                break;
            default:
                continue;
        }

        AddBind(bind, device, key);
        isBindsLoaded = true; // At least one bind have been loaded successfully
    }
    bindClearEnabled = true;
}

void KeybindsHandler_Save(FILE* file, char* sectionName)
{
    int i;
    bind_descriptor_t* bind;

    for(i = 0; i < bk__serializable; ++i)
    {
        if(bind_descriptor[i])
        {
            fprintf(file, "%s = ", bkToName[i]);
            bind = bind_descriptor[i];
            while(bind)
            {
                char deviceChar;
                const char *keyString;
                switch(bind->device)
                {
                    case keyboard:
                        deviceChar = 'k';
                        keyString = kKToName[legacyToScancode(bind->key)];
                        break;
                    case mouse:
                        deviceChar = 'm';
                        keyString = mKToName[bind->key];
                        break;
                    case controller:
                        deviceChar = 'c';
                        keyString = cKToName[bind->key];
                        break;
                    default:
                        bind = bind->next;
                        continue;
                }
                fprintf(file, "\"%c_%s\"", deviceChar, keyString);
                if(bind->next != NULL)
                    fprintf(file, ", ");
                bind = bind->next;
            }
            fprintf(file, "\n");
        }
    }
}

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
#include "i_controller.h"
#include "i_input.h"
#include "i_video.h"
#include "jn.h"
#include "m_misc.h"
#include "rd_menu.h"

typedef struct
{
    bound_key_t from;
    bound_key_t to;
} bind_section_t;

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

// [Dasperal] This array must be in sync with bound_key_t enum!
static const char* bkToName[] = {
    // Movement
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

    // Weapon
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

    // Look
    "Look_up",
    "Look_down",
    "Look_center",
    "Toggle_mlook",

    // Inventory
    "Inv_left",
    "Inv_right",
    "Inv_use_artifact",

    // Inventory: Strife
    "Inv_use_health",

    // Artifacts: Heretic
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

    // Artifacts: Hexen
    "Arti_all",
    "Arti_blastradius",
    "Arti_teleportother",
    "Arti_boostarmor",
    "Arti_boostmana",
    "Arti_summon",
    "Arti_speed",
    "Arti_healingradius",

    // Shortcuts
    "Screen_inc", // [Dasperal] screen_inc/dec do not work on automap so they should be related to "Controls" section
    "Screen_dec", // instead of "Shortcuts" section

    "-shortcuts-", // [Dasperal] Bindings after this line are related to "Shortcuts" section
    "Help",
    "Msave",
    "Mload",
    "Mvolume",
    "Qsave",
    "End_game",
    "Qload",
    "Quit",
    "Gamma",
    "Nextlevel",
    "Reloadlevel",
    "Screenshot",
    "Pause",
    "Finish_demo",

    // Toggles
    "Toggle_crosshair",
    "Messages",
    "Detail",
    "Toggle_fliplvls",

    // Multiplayer
    "Spy",
    "Multi_msg",
    "Multi_msg_player_0",
    "Multi_msg_player_1",
    "Multi_msg_player_2",
    "Multi_msg_player_3",
    "Multi_msg_player_4",
    "Multi_msg_player_5",
    "Multi_msg_player_6",
    "Multi_msg_player_7",

    // Map keys
    "-map-", // [Dasperal] Bindings after this line are related to "Map" section
    "Map_toggle",
    "Map_zoom_in",
    "Map_zoom_out",
    "Map_zoom_max",
    "Map_follow",
    "Map_overlay",
    "Map_rotate",
    "Map_grid",
    "Map_mark",
    "Map_clearmark"
};

static int nameToBk[arrlen(bkToName)];
static boolean nameToBk_init = false;

// [Dasperal] This array must be in sync with SDL_Scancode enum!
// Strings in this array must not be changed only added, or it will break existing configs
static const char* kKToName[] = {
    "?", "?", "?", "?", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", // 0   - 20
    "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "ENTER",  // 21  - 40
    "ESCAPE", "BACKSPACE", "TAB", "SPACE", "-", "=", "[", "]", "\\", "NON_US_#", ";", "\'", "`", ",", ".",   // 41  - 55
    "/", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "SYS_RQ",    // 56  - 70
    "SCROLL_LOCK", "PAUSE", "INSERT", "HOME", "PAGE_UP", "DELETE", "END", "PAGE_DN", "RIGHT", "LEFT",        // 71  - 80
    "DOWN", "UP", "NUM_LOCK", "KP_/", "KP_*", "KP_-", "KP_+", "KP_ENTER", "KP_1", "KP_2", "KP_3", "KP_4",    // 81  - 92
    "KP_5", "KP_6", "KP_7", "KP_8", "KP_9", "KP_0", "KP_.", "NON_US_\\", "APP", "POWER", "KP_=", "F13",      // 93  - 104
    "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "EXECUTE", "HELP", "MENU",  // 105 - 118
    "SELECT", "STOP", "AGAIN", "UNDO", "CUT", "COPY", "PASTE", "FIND", "MUTE", "VOLUME_UP", "VOLUME_DOWN",   // 119 - 129
    "?", "?", "?", "KP_,", "KP_=400", "INTER_1", "INTER_2", "INTER_3", "INTER_4", "INTER_5", "INTER_6",      // 130 - 140
    "INTER_7", "INTER_8", "INTER_9", "LANG_1", "LANG_2", "LANG_3", "LANG_4", "LANG_5", "LANG_6", "LANG_7",   // 141 - 150
    "LANG_8", "LANG_9", "ALT_ERASE", "SYS_REQ", "CANCEL", "CLEAR", "PRIOR", "RETURN2", "SEPARATOR", "OUT",   // 151 - 160
    "OPER", "CLEAR_AGAIN", "CRSEL", "EXSEL", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "KP_00", // 161 - 176
    "KP_000", "THOUSANDS_SEPARATOR", "DECIMAL_SEPARATOR", "CURRENCY_UNIT", "CURRENCY_SUBUNIT", "KP_(",       // 177 - 182
    "KP_)", "KP_[", "KP_]", "KP_TAB", "KP_BACKSPACE", "KP_A", "KP_B", "KP_C", "KP_D", "KP_E", "KP_F",        // 183 - 193
    "KP_XOR", "KP_POWER", "KP_%", "KP_<", "KP_>", "KP_&", "KP_&&", "KP_|", "KP_||", "KP_:", "KP_#",          // 194 - 204
    "KP_SPACE", "KP_@", "KP_!", "KP_MS", "KP_MR", "KP_MC", "KP_M+", "KP_M-", "KP_M*", "KP_M/", "KP_+-",      // 205 - 215
    "KP_CLEAR", "KP_CLEAR_ENTRY", "KP_BIN", "KP_OCT", "KP_DEC", "KP_HEX", "?", "?", "LCTRL", "LSHIFT",       // 216 - 225
    "LALT", "LGUI", "RCTRL", "RSHIFT", "RALT", "RGUI", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",// 226 - 242
    "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "MODE", "AUDIO_NEXT",              // 243 - 258
    "AUDIO_PREV", "AUDIO_STOP", "AUDIO_PLAY", "AUDIO_MUTE", "MEDIA", "WWW", "MAIL", "CALCULATOR", "COMPUTER",// 259 - 267
    "AC_SEARCH", "AC_HOME", "AC_BACK", "AC_FORWARD", "AC_STOP", "AC_REFRESH", "AC_BOOKMARKS",                // 268 - 274
    "BRIGHTNESS_DOWN", "BRIGHTNESS_UP", "DISPLAY_SWITCH", "ILLUM_TOGGLE", "ILLUM_DOWN", "ILLUM_UP", "EJECT", // 275 - 281
    "SLEEP", "APP1", "APP2", "AUDIO_REWIND", "AUDIO_FASTFORWARD"                                             // 282 - 286
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
// keyHasNoBinds
// Returns true if no keys have been bound to given bound_key
// -----------------------------------------------------------------------------
static boolean keyHasNoBinds(bound_key_t key)
{
    return bind_descriptor[key] == NULL;
}

static const char* getKeyboardKeyName(int key)
{
    switch(key)
    {
        case SDL_SCANCODE_BACKSPACE:          return "BCKSP";
        case SDL_SCANCODE_NONUSHASH:          return "NON US #";
        case SDL_SCANCODE_GRAVE:              return "TILDE";
        case SDL_SCANCODE_CAPSLOCK:           return "CAPS LOCK";
        case SDL_SCANCODE_PRINTSCREEN:        return "SYS RQ";
        case SDL_SCANCODE_SCROLLLOCK:         return "SCROLL LOCK";
        case SDL_SCANCODE_PAGEDOWN:           return "PGDN";
        case SDL_SCANCODE_PAGEUP:             return "PGEUP";
        case SDL_SCANCODE_NUMLOCKCLEAR:       return "NUM LOCK";
        case SDL_SCANCODE_NONUSBACKSLASH:     return "NON US \\";
        case SDL_SCANCODE_VOLUMEUP:           return "VOLUME UP";
        case SDL_SCANCODE_VOLUMEDOWN:         return "VOLUME DOWN";
        case SDL_SCANCODE_ALTERASE:           return "ALT ERASE";
        case SDL_SCANCODE_SYSREQ:             return "SYS REQ";
        case SDL_SCANCODE_THOUSANDSSEPARATOR: return "THOUSAND SEP";
        case SDL_SCANCODE_DECIMALSEPARATOR:   return "DECIMAL SEP";
        case SDL_SCANCODE_CURRENCYUNIT:       return "CURRENCY UNIT";
        case SDL_SCANCODE_CURRENCYSUBUNIT:    return "CURRENCY SUBUNIT";
        case SDL_SCANCODE_KP_BACKSPACE:       return "KP_BACKSP";
        case SDL_SCANCODE_BRIGHTNESSDOWN:     return "BRIGHTNESS DOWN";
        case SDL_SCANCODE_BRIGHTNESSUP:       return "BRIGHTNESS UP";
        case SDL_SCANCODE_DISPLAYSWITCH:      return "DISPLAY SWITCH";
        default:
            if(key > arrlen(kKToName))
            {
                return "?"; // [JN] Unknown key
            }
            return kKToName[key];
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

static char* getBoundKeysString(bound_key_t key)
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
        case gt_Hexen:
            noBindTranslation = CR_GRAY;
            bindingTranslation = CR_YELLOW;
            break;
    }

    for (int i = 0; i < CurrentMenu->itemCount; ++i)
    {
        if (CurrentMenu->items[i].pointer == BK_StartBindingKey)
        {
            boolean bindingThis = isBinding && i == CurrentItPos;

            RD_M_DrawTextSmallENG(bindingThis ? "?" : getBoundKeysString(CurrentMenu->items[i].option),
                                  x + wide_delta, i * 10 + 25,
                                  bindingThis ? bindingTranslation : keyHasNoBinds(CurrentMenu->items[i].option) ?
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
// removeKeyFromBinds
// Removes given key of device from all bound_keys
// -----------------------------------------------------------------------------
static void removeBindFromSection(const device_t device, const int key, const bind_section_t* section)
{
    bind_descriptor_t *prevDescriptor, *tmp;

    for(int i = section->from; i < section->to; ++i)
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

void BK_AddBind(bound_key_t boundKey, device_t device, int key)
{
    bind_section_t section;
    bind_descriptor_t *bind;

    if(boundKey > bk__section_map && boundKey < bk__serializable)
    {
        section.from = bk__section_shortcuts + 1;
        section.to = bk__serializable;
    }
    else if(boundKey > bk__section_shortcuts && boundKey < bk__section_map)
    {
        section.from = bk_forward;
        section.to = bk__serializable;
    }
    else // boundKey < bk__section_shortcuts
    {
        section.from = bk_forward;
        section.to = bk__section_map;
    }

    bind = bind_descriptor[boundKey];

    if(bind == NULL)
    {
        removeBindFromSection(device, key, &section);
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
                // Clear bind
                if (prevBind)
                    prevBind->next = bind->next;
                else
                    bind_descriptor[boundKey] = bind->next;
                free(bind);
                return;
            }
            prevBind = bind;
            bind = bind->next;
        }

        // Add new bind
        removeBindFromSection(device, key, &section);
        bind = malloc(sizeof(bind_descriptor));
        bind->next = NULL;
        bind->device = device;
        bind->key = key;

        prevBind->next = bind;
    }
}

void BK_BindKey(event_t* event)
{
    if(event->type == ev_keydown
    && *(kKToName[event->data1]) == '?')
    {
        printf(english_language ?
               "BK_BindKey: Attempt to bind unsupported scancode: %d" :
               "BK_BindKey: Попытка привязать неподдерживаемый сканкод: %d",
               event->data1);
        return;
    }

    // [Dasperal] Prohibit binding gamepad menu key
    if(event->type == ev_controller_keydown && event->data1 == CONTROLLER_START)
    {
        return;
    }

    isBinding = false;

    if(!BK_isKeyDown(event, bk_menu_activate))
    {
        BK_AddBind(keyToBind, getEventDevice(event), event->data1);
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
    // Keyboard
    BK_AddBind(bk_left, keyboard, SDL_SCANCODE_LEFT);
    BK_AddBind(bk_right, keyboard, SDL_SCANCODE_RIGHT);
    BK_AddBind(bk_up, keyboard, SDL_SCANCODE_UP);
    BK_AddBind(bk_down, keyboard, SDL_SCANCODE_DOWN);

    BK_AddBind(bk_menu_activate, keyboard, SDL_SCANCODE_ESCAPE);
    BK_AddBind(bk_menu_back, keyboard, SDL_SCANCODE_BACKSPACE);
    BK_AddBind(bk_menu_select, keyboard, SDL_SCANCODE_RETURN);
    BK_AddBind(bk_menu_select, keyboard, SDL_SCANCODE_KP_ENTER);
    BK_AddBind(bk_menu_page_next, keyboard, SDL_SCANCODE_PAGEDOWN);
    BK_AddBind(bk_menu_page_prev, keyboard, SDL_SCANCODE_PAGEUP);

    BK_AddBind(bk_confirm, keyboard, SDL_SCANCODE_Y);
    BK_AddBind(bk_abort, keyboard, SDL_SCANCODE_N);
    BK_AddBind(bk_abort, keyboard, SDL_SCANCODE_ESCAPE);

    // Mouse
    BK_AddBind(bk_left, mouse, MOUSE_SCROLL_LEFT);
    BK_AddBind(bk_right, mouse, MOUSE_SCROLL_RIGHT);
    BK_AddBind(bk_up, mouse, MOUSE_SCROLL_UP);
    BK_AddBind(bk_down, mouse, MOUSE_SCROLL_DOWN);

    BK_AddBind(bk_menu_back, mouse, MOUSE_RIGHT);
    BK_AddBind(bk_menu_select, mouse, MOUSE_LEFT);
    BK_AddBind(bk_menu_page_next, mouse, MOUSE_4);
    BK_AddBind(bk_menu_page_prev, mouse, MOUSE_5);

    // Controller
    BK_AddBind(bk_left, controller, CONTROLLER_DPAD_LEFT);
    BK_AddBind(bk_right, controller, CONTROLLER_DPAD_RIGHT);
    BK_AddBind(bk_up, controller, CONTROLLER_DPAD_UP);
    BK_AddBind(bk_down, controller, CONTROLLER_DPAD_DOWN);

    BK_AddBind(bk_menu_activate, controller, CONTROLLER_START);
    BK_AddBind(bk_menu_back, controller, CONTROLLER_B);
    BK_AddBind(bk_menu_select, controller, CONTROLLER_A);
    BK_AddBind(bk_menu_page_next, controller, CONTROLLER_RIGHT_SHOULDER);
    BK_AddBind(bk_menu_page_prev, controller, CONTROLLER_LEFT_SHOULDER);

    BK_AddBind(bk_confirm, controller, CONTROLLER_A);
    BK_AddBind(bk_abort,   controller, CONTROLLER_B);
};

void BK_ApplyDefaultBindings()
{
    // Keyboard
    BK_AddBind(bk_forward, keyboard, SDL_SCANCODE_W);
    BK_AddBind(bk_backward, keyboard, SDL_SCANCODE_S);
    BK_AddBind(bk_turn_left, keyboard, SDL_SCANCODE_LEFT);
    BK_AddBind(bk_turn_right, keyboard, SDL_SCANCODE_RIGHT);
    BK_AddBind(bk_strafe_left, keyboard, SDL_SCANCODE_A);
    BK_AddBind(bk_strafe_right, keyboard, SDL_SCANCODE_D);
    BK_AddBind(bk_speed, keyboard, SDL_SCANCODE_LSHIFT);
    BK_AddBind(bk_speed, keyboard, SDL_SCANCODE_RSHIFT);
    BK_AddBind(bk_strafe, keyboard, SDL_SCANCODE_LALT);
    BK_AddBind(bk_strafe, keyboard, SDL_SCANCODE_RALT);
    BK_AddBind(bk_toggle_autorun, keyboard, SDL_SCANCODE_CAPSLOCK);

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_fly_up, keyboard, SDL_SCANCODE_PAGEUP);
        BK_AddBind(bk_fly_down, keyboard, SDL_SCANCODE_INSERT);
        BK_AddBind(bk_fly_stop, keyboard, SDL_SCANCODE_HOME);
    }

    BK_AddBind(bk_use, keyboard, SDL_SCANCODE_E);
    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_use, keyboard, SDL_SCANCODE_SPACE);
    }
    else
    {
        BK_AddBind(bk_jump, keyboard, SDL_SCANCODE_SPACE);
    }

    BK_AddBind(bk_fire, keyboard, SDL_SCANCODE_LCTRL);
    BK_AddBind(bk_fire, keyboard, SDL_SCANCODE_RCTRL);

    BK_AddBind(bk_weapon_1, keyboard, SDL_SCANCODE_1);
    BK_AddBind(bk_weapon_2, keyboard, SDL_SCANCODE_2);
    BK_AddBind(bk_weapon_3, keyboard, SDL_SCANCODE_3);
    BK_AddBind(bk_weapon_4, keyboard, SDL_SCANCODE_4);

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_weapon_5, keyboard, SDL_SCANCODE_5);
        BK_AddBind(bk_weapon_6, keyboard, SDL_SCANCODE_6);
        BK_AddBind(bk_weapon_7, keyboard, SDL_SCANCODE_7);
    }
    if(RD_GameType == gt_Doom)
    {
        BK_AddBind(bk_weapon_8, keyboard, SDL_SCANCODE_8);
    }

    BK_AddBind(bk_toggle_mlook, keyboard, SDL_SCANCODE_GRAVE);

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_look_up, keyboard, SDL_SCANCODE_PAGEDOWN);
        BK_AddBind(bk_look_down, keyboard, SDL_SCANCODE_DELETE);
        BK_AddBind(bk_look_center, keyboard, SDL_SCANCODE_END);

        BK_AddBind(bk_inv_left, keyboard, SDL_SCANCODE_LEFTBRACKET);
        BK_AddBind(bk_inv_right, keyboard, SDL_SCANCODE_RIGHTBRACKET);
        BK_AddBind(bk_inv_use_artifact, keyboard, SDL_SCANCODE_RETURN);
    }

    if(RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_arti_tome, keyboard, SDL_SCANCODE_BACKSPACE);
    }

    BK_AddBind(bk_map_toggle, keyboard, SDL_SCANCODE_TAB);
    BK_AddBind(bk_map_zoom_in, keyboard, SDL_SCANCODE_EQUALS);
    BK_AddBind(bk_map_zoom_out, keyboard, SDL_SCANCODE_MINUS);
    BK_AddBind(bk_map_zoom_max, keyboard, SDL_SCANCODE_0);
    BK_AddBind(bk_map_follow, keyboard, SDL_SCANCODE_F);
    BK_AddBind(bk_map_overlay, keyboard, SDL_SCANCODE_O);
    BK_AddBind(bk_map_rotate, keyboard, SDL_SCANCODE_R);
    BK_AddBind(bk_map_grid, keyboard, SDL_SCANCODE_G);

    BK_AddBind(bk_map_clearmark, keyboard, SDL_SCANCODE_C);
    BK_AddBind(bk_map_mark, keyboard, SDL_SCANCODE_M);

    BK_AddBind(bk_menu_help, keyboard, SDL_SCANCODE_F1);
    BK_AddBind(bk_menu_save, keyboard, SDL_SCANCODE_F2);
    BK_AddBind(bk_menu_load, keyboard, SDL_SCANCODE_F3);
    BK_AddBind(bk_menu_volume, keyboard, SDL_SCANCODE_F4);

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_detail, keyboard, SDL_SCANCODE_F5);
    }

    BK_AddBind(bk_qsave, keyboard, SDL_SCANCODE_F6);
    BK_AddBind(bk_end_game, keyboard, SDL_SCANCODE_F7);
    BK_AddBind(bk_messages, keyboard, SDL_SCANCODE_F8);
    BK_AddBind(bk_qload, keyboard, SDL_SCANCODE_F9);
    BK_AddBind(bk_quit, keyboard, SDL_SCANCODE_F10);
    BK_AddBind(bk_gamma, keyboard, SDL_SCANCODE_F11);
    BK_AddBind(bk_screen_inc, keyboard, SDL_SCANCODE_EQUALS);
    BK_AddBind(bk_screen_dec, keyboard, SDL_SCANCODE_MINUS);
    BK_AddBind(bk_screenshot, keyboard, SDL_SCANCODE_PRINTSCREEN);
    BK_AddBind(bk_pause, keyboard, SDL_SCANCODE_PAUSE);
    BK_AddBind(bk_finish_demo, keyboard, SDL_SCANCODE_Q);
    BK_AddBind(bk_toggle_crosshair, keyboard, SDL_SCANCODE_X);

    BK_AddBind(bk_spy, keyboard, SDL_SCANCODE_F12);
    BK_AddBind(bk_multi_msg, keyboard, SDL_SCANCODE_T);

    if(RD_GameType == gt_Doom)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_I);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_B);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_R);
    }
    if(RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_Y);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_R);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_B);
    }
    if(RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_B);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_R);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_Y);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_4, keyboard, SDL_SCANCODE_J);
        BK_AddBind(bk_multi_msg_player_5, keyboard, SDL_SCANCODE_I);
        BK_AddBind(bk_multi_msg_player_6, keyboard, SDL_SCANCODE_H);
        BK_AddBind(bk_multi_msg_player_7, keyboard, SDL_SCANCODE_P);
    }

    // Mouse
    BK_AddBind(bk_fire, mouse, MOUSE_LEFT);
    BK_AddBind(bk_weapon_prev, mouse, MOUSE_SCROLL_UP);
    BK_AddBind(bk_weapon_next, mouse, MOUSE_SCROLL_DOWN);
    BK_AddBind(bk_forward, mouse, MOUSE_MIDDLE);
    BK_AddBind(bk_strafe, mouse, MOUSE_RIGHT);

    // Controller
    BK_AddBind(bk_look_center, controller, CONTROLLER_RIGHT_STICK);
}

void BK_ApplyVanilaBindings()
{
    // Keyboard
    BK_AddBind(bk_forward, keyboard, SDL_SCANCODE_UP);
    BK_AddBind(bk_backward, keyboard, SDL_SCANCODE_DOWN);
    BK_AddBind(bk_turn_left, keyboard, SDL_SCANCODE_LEFT);
    BK_AddBind(bk_turn_right, keyboard, SDL_SCANCODE_RIGHT);
    BK_AddBind(bk_strafe_left, keyboard, SDL_SCANCODE_COMMA);
    BK_AddBind(bk_strafe_right, keyboard, SDL_SCANCODE_PERIOD);
    BK_AddBind(bk_speed, keyboard, SDL_SCANCODE_LSHIFT);
    BK_AddBind(bk_speed, keyboard, SDL_SCANCODE_RSHIFT);
    BK_AddBind(bk_strafe, keyboard, SDL_SCANCODE_LALT);
    BK_AddBind(bk_strafe, keyboard, SDL_SCANCODE_RALT);

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_fly_up, keyboard, SDL_SCANCODE_PAGEUP);
        BK_AddBind(bk_fly_down, keyboard, SDL_SCANCODE_INSERT);
        BK_AddBind(bk_fly_stop, keyboard, SDL_SCANCODE_HOME);
    }

    BK_AddBind(bk_use, keyboard, SDL_SCANCODE_SPACE);

    if(RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_jump, keyboard, SDL_SCANCODE_SLASH);
    }

    BK_AddBind(bk_fire, keyboard, SDL_SCANCODE_LCTRL);
    BK_AddBind(bk_fire, keyboard, SDL_SCANCODE_RCTRL);

    BK_AddBind(bk_weapon_1, keyboard, SDL_SCANCODE_1);
    BK_AddBind(bk_weapon_2, keyboard, SDL_SCANCODE_2);
    BK_AddBind(bk_weapon_3, keyboard, SDL_SCANCODE_3);
    BK_AddBind(bk_weapon_4, keyboard, SDL_SCANCODE_4);

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_weapon_5, keyboard, SDL_SCANCODE_5);
        BK_AddBind(bk_weapon_6, keyboard, SDL_SCANCODE_6);
        BK_AddBind(bk_weapon_7, keyboard, SDL_SCANCODE_7);
    }
    if(RD_GameType == gt_Doom)
    {
        BK_AddBind(bk_weapon_8, keyboard, SDL_SCANCODE_8);
    }

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_look_up, keyboard, SDL_SCANCODE_PAGEDOWN);
        BK_AddBind(bk_look_down, keyboard, SDL_SCANCODE_DELETE);
        BK_AddBind(bk_look_center, keyboard, SDL_SCANCODE_END);

        BK_AddBind(bk_inv_left, keyboard, SDL_SCANCODE_LEFTBRACKET);
        BK_AddBind(bk_inv_right, keyboard, SDL_SCANCODE_RIGHTBRACKET);
        BK_AddBind(bk_inv_use_artifact, keyboard, SDL_SCANCODE_RETURN);
    }

    if(RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_arti_tome, keyboard, SDL_SCANCODE_BACKSPACE);
    }

    BK_AddBind(bk_map_toggle, keyboard, SDL_SCANCODE_TAB);
    BK_AddBind(bk_map_zoom_in, keyboard, SDL_SCANCODE_EQUALS);
    BK_AddBind(bk_map_zoom_out, keyboard, SDL_SCANCODE_MINUS);
    BK_AddBind(bk_map_zoom_max, keyboard, SDL_SCANCODE_0);
    BK_AddBind(bk_map_follow, keyboard, SDL_SCANCODE_F);

    BK_AddBind(bk_map_clearmark, keyboard, SDL_SCANCODE_C);
    BK_AddBind(bk_map_mark, keyboard, SDL_SCANCODE_M);

    BK_AddBind(bk_menu_help, keyboard, SDL_SCANCODE_F1);
    BK_AddBind(bk_menu_save, keyboard, SDL_SCANCODE_F2);
    BK_AddBind(bk_menu_load, keyboard, SDL_SCANCODE_F3);
    BK_AddBind(bk_menu_volume, keyboard, SDL_SCANCODE_F4);

    if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_detail, keyboard, SDL_SCANCODE_F5);
    }

    BK_AddBind(bk_qsave, keyboard, SDL_SCANCODE_F6);
    BK_AddBind(bk_end_game, keyboard, SDL_SCANCODE_F7);
    BK_AddBind(bk_messages, keyboard, SDL_SCANCODE_F8);
    BK_AddBind(bk_qload, keyboard, SDL_SCANCODE_F9);
    BK_AddBind(bk_quit, keyboard, SDL_SCANCODE_F10);
    BK_AddBind(bk_gamma, keyboard, SDL_SCANCODE_F11);
    BK_AddBind(bk_screen_inc, keyboard, SDL_SCANCODE_EQUALS);
    BK_AddBind(bk_screen_dec, keyboard, SDL_SCANCODE_MINUS);
    BK_AddBind(bk_screenshot, keyboard, SDL_SCANCODE_PRINTSCREEN);
    BK_AddBind(bk_pause, keyboard, SDL_SCANCODE_PAUSE);

    BK_AddBind(bk_spy, keyboard, SDL_SCANCODE_F12);
    BK_AddBind(bk_multi_msg, keyboard, SDL_SCANCODE_T);

    if(RD_GameType == gt_Doom)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_I);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_B);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_R);
    }
    if(RD_GameType == gt_Heretic)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_Y);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_R);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_B);
    }
    if(RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_multi_msg_player_0, keyboard, SDL_SCANCODE_B);
        BK_AddBind(bk_multi_msg_player_1, keyboard, SDL_SCANCODE_R);
        BK_AddBind(bk_multi_msg_player_2, keyboard, SDL_SCANCODE_Y);
        BK_AddBind(bk_multi_msg_player_3, keyboard, SDL_SCANCODE_G);
        BK_AddBind(bk_multi_msg_player_4, keyboard, SDL_SCANCODE_J);
        BK_AddBind(bk_multi_msg_player_5, keyboard, SDL_SCANCODE_W);
        BK_AddBind(bk_multi_msg_player_6, keyboard, SDL_SCANCODE_H);
        BK_AddBind(bk_multi_msg_player_7, keyboard, SDL_SCANCODE_P);
    }

    // Mouse
    BK_AddBind(bk_fire, mouse, MOUSE_LEFT);
    BK_AddBind(bk_forward, mouse, MOUSE_MIDDLE);
    BK_AddBind(bk_strafe, mouse, MOUSE_RIGHT);

    // Controller
    BK_AddBind(bk_look_center, controller, CONTROLLER_RIGHT_STICK);
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
            nameToBk[i] = i;
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
                            nameToBk, arrlen(bkToName),
                            sizeof(int), nameToBk_Comparator);
    if(bsearchResult == NULL)
        return;

    bind = *bsearchResult;
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
                if(bsearchResult == NULL || *(kKToName[*bsearchResult]) == '?')
                    continue;
                key = *bsearchResult;
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

        BK_AddBind(bind, device, key);
        isBindsLoaded = true; // At least one bind have been loaded successfully
    }
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
                        keyString = kKToName[bind->key];
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

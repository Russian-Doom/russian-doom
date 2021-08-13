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
#include <stdlib.h>
#include <string.h>

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

static char* getKeyboardKeyName(int key)
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
        case KEY_APP:        return "App";
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
        case CONTROLLER_A:              return "PAD_A";
        case CONTROLLER_B:              return "PAD_B";
        case CONTROLLER_X:              return "PAD_X";
        case CONTROLLER_Y:              return "PAD_Y";
        case CONTROLLER_BACK:           return "PAD_BACK";
        case CONTROLLER_GUIDE:          return "PAD_GUIDE";
        case CONTROLLER_START:          return "PAD_START";
        case CONTROLLER_LEFT_STICK:     return "PAD_L3";
        case CONTROLLER_RIGHT_STICK:    return "PAD_R3";
        case CONTROLLER_LEFT_SHOULDER:  return "PAD_L1";
        case CONTROLLER_RIGHT_SHOULDER: return "PAD_R1";
        case CONTROLLER_DPAD_UP:        return "DPAD_UP";
        case CONTROLLER_DPAD_DOWN:      return "DPAD_DOWN";
        case CONTROLLER_DPAD_LEFT:      return "DPAD_LEFT";
        case CONTROLLER_DPAD_RIGHT:     return "DPAD_RIGHT";
        case CONTROLLER_MISC1:          return "PAD_MISC";
        case CONTROLLER_PADDLE1:        return "PADDLE 1";
        case CONTROLLER_PADDLE2:        return "PADDLE 2";
        case CONTROLLER_PADDLE3:        return "PADDLE 3";
        case CONTROLLER_PADDLE4:        return "PADDLE 4";
        case CONTROLLER_TOUCHPAD:       return "PAD_TOUCH";
        case CONTROLLER_LEFT_TRIGGER:   return "PAD_L2";
        case CONTROLLER_RIGHT_TRIGGER:  return "PAD_R2";
        default:                        return "?"; // [JN] Unknown key
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

static void AddBind(bound_key_t boundKey, device_t device, int key)
{
    bind_descriptor_t* bind = bind_descriptor[boundKey];

    if(bind == NULL)
    {
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
        bind = malloc(sizeof(bind_descriptor));
        bind->next = NULL;
        bind->device = device;
        bind->key = key;

        prevBind->next = bind;
    }
}

void BK_BindKey(event_t* event)
{
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
};

void BK_ApplyDefaultBindings()
{
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
    AddBind(bk_jump,           keyboard, ' ');
    AddBind(bk_toggle_autorun, keyboard, KEY_CAPSLOCK);

    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        AddBind(bk_fly_up,   keyboard, KEY_PGUP);
        AddBind(bk_fly_down, keyboard, KEY_INS);
        AddBind(bk_fly_stop, keyboard, KEY_HOME);
    }

    AddBind(bk_use, keyboard, 'e');

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

    if(RD_GameType == gt_Doom)
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
    AddBind(bk_fire,        mouse, MOUSE_LEFT);
    AddBind(bk_weapon_prev, mouse, MOUSE_SCROLL_UP);
    AddBind(bk_weapon_next, mouse, MOUSE_SCROLL_DOWN);
    AddBind(bk_forward,     mouse, MOUSE_MIDDLE);
    AddBind(bk_strafe,      mouse, MOUSE_RIGHT);

    // Controller
    AddBind(bk_look_center, controller, CONTROLLER_RIGHT_STICK);
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

void BK_SaveBindings(void* file)
{
    int i;
    bind_descriptor_t* bind;

    fprintf(file, "%-30s%s\n", "Keybinds", "Start");

    for(i = 0; i < bk__serializable; ++i)
    {
        if(bind_descriptor[i])
        {
            fprintf(file, "%-30d", i);
            bind = bind_descriptor[i];
            while(bind)
            {
                char deviceChar;
                switch(bind->device)
                {
                    case keyboard:
                        deviceChar = 'k';
                        break;
                    case mouse:
                        deviceChar = 'm';
                        break;
                    case controller:
                        deviceChar = 'c';
                        break;
                    default:
                        deviceChar = ' ';
                }
                fprintf(file, "%c_%d ", deviceChar, bind->key);
                bind = bind->next;
            }
            fprintf(file, "\n");
        }
    }
    fprintf(file, "%-30s%s\n", "Keybinds", "End");
}

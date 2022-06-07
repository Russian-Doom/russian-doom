//
// Copyright(C) 2022 Dasperal
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


#include "rd_migration.h"

#include <locale.h>
#include <SDL_scancode.h>
#include <string.h>
#include "d_name.h"
#include "jn.h"
#include "m_misc.h"
#include "rd_keybinds.h"

static void RegisterTrackedDefault(const char* name, default_type_t type);
static void FreeTrackers();

int config_version = 0;
static defaultTracker_t* defaultTrackers;

void M_RegisterTrackedFields()
{
    if(config_version == CURRENT_CONFIG_VERSION)
        return;

    if(config_version < 2)
    {
        RegisterTrackedDefault("message_pickup_color", DEFAULT_INT);
        if(RD_GameType != gt_Hexen)
            RegisterTrackedDefault("message_secret_color", DEFAULT_INT);
        RegisterTrackedDefault("message_system_color", DEFAULT_INT);
        RegisterTrackedDefault("message_chat_color", DEFAULT_INT);
        if(RD_GameType == gt_Hexen)
            RegisterTrackedDefault("message_quest_color", DEFAULT_INT);

        if(RD_GameType == gt_Doom)
        {
            RegisterTrackedDefault("sbar_color_high", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_normal", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_low", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_critical", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_armor_1", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_armor_2", DEFAULT_INT);
            RegisterTrackedDefault("sbar_color_armor_0", DEFAULT_INT);
        }
    }
}

void M_ApplyMigration()
{
    if(config_version == CURRENT_CONFIG_VERSION)
        return;

    if(config_version < 1) // Made keys F[1, 2, 3, 4, 5, 7, 8, 10, 11], -, =, Pause not hardcoded and bindable
    {
        // Add missing(previously hardcoded) bindings
        BK_AddBind(bk_menu_help,   keyboard, SDL_SCANCODE_F1);
        BK_AddBind(bk_menu_save,   keyboard, SDL_SCANCODE_F2);
        BK_AddBind(bk_menu_load,   keyboard, SDL_SCANCODE_F3);
        BK_AddBind(bk_menu_volume, keyboard, SDL_SCANCODE_F4);
        BK_AddBind(bk_detail,      keyboard, SDL_SCANCODE_F5);
        BK_AddBind(bk_end_game,    keyboard, SDL_SCANCODE_F7);
        BK_AddBind(bk_messages,    keyboard, SDL_SCANCODE_F8);
        BK_AddBind(bk_quit,        keyboard, SDL_SCANCODE_F10);
        BK_AddBind(bk_gamma,       keyboard, SDL_SCANCODE_F11);
        BK_AddBind(bk_screen_inc,  keyboard, SDL_SCANCODE_EQUALS);
        BK_AddBind(bk_screen_dec,  keyboard, SDL_SCANCODE_MINUS);
        BK_AddBind(bk_pause,       keyboard, SDL_SCANCODE_PAUSE);
    }

    // Do not rebing following keys so they will not
    // be cleared after migration:
    BK_AddBind(bk_map_rotate,  keyboard, SDL_SCANCODE_R);
    BK_AddBind(bk_map_grid,    keyboard, SDL_SCANCODE_G);

    if(RD_GameType == gt_Hexen)
    {
        BK_AddBind(bk_forward, keyboard, SDL_SCANCODE_W);
    }

    if(config_version < 2) // Changed names of 'message_*_color' and 'sbar_color_*' config entries
    {
        defaultTracker_t* message_pickup_color = M_GetDefaultTracker("message_pickup_color");
        defaultTracker_t* message_system_color = M_GetDefaultTracker("message_system_color");
        defaultTracker_t* message_chat_color = M_GetDefaultTracker("message_chat_color");

        if(message_pickup_color->found)
            message_color_pickup = message_pickup_color->value.i != 0 ? message_pickup_color->value.i + 1 : 0;
        if(message_system_color->found)
            message_color_system = message_system_color->value.i != 0 ? message_system_color->value.i + 1 : 0;
        if(message_chat_color->found)
            message_color_chat = message_chat_color->value.i != 0 ? message_chat_color->value.i + 1 : 0;

        if(RD_GameType != gt_Hexen && JN_getNotCommonIntVarPointer(v_message_color_secret) != NULL)
        {
            defaultTracker_t* message_secret_color = M_GetDefaultTracker("message_secret_color");
            if(message_secret_color->found)
                *JN_getNotCommonIntVarPointer(v_message_color_secret) = message_secret_color->value.i != 0 ? message_secret_color->value.i + 1 : 0;
        }
        if(RD_GameType == gt_Hexen && JN_getNotCommonIntVarPointer(v_message_color_quest) != NULL)
        {
            defaultTracker_t* message_quest_color = M_GetDefaultTracker("message_quest_color");
            if(message_quest_color->found)
                *JN_getNotCommonIntVarPointer(v_message_color_quest) = message_quest_color->value.i != 0 ? message_quest_color->value.i + 1 : 0;
        }

        if(RD_GameType == gt_Doom)
        {
            defaultTracker_t* sbar_color_high = M_GetDefaultTracker("sbar_color_high");
            defaultTracker_t* sbar_color_normal = M_GetDefaultTracker("sbar_color_normal");
            defaultTracker_t* sbar_color_low = M_GetDefaultTracker("sbar_color_low");
            defaultTracker_t* sbar_color_critical = M_GetDefaultTracker("sbar_color_critical");
            defaultTracker_t* sbar_color_armor_1 = M_GetDefaultTracker("sbar_color_armor_1");
            defaultTracker_t* sbar_color_armor_2 = M_GetDefaultTracker("sbar_color_armor_2");
            defaultTracker_t* sbar_color_armor_0 = M_GetDefaultTracker("sbar_color_armor_0");

            if(JN_getNotCommonIntVarPointer(v_stbar_color_high) != NULL && sbar_color_high->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_high) = sbar_color_high->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_normal) != NULL && sbar_color_normal->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_normal) = sbar_color_normal->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_low) != NULL && sbar_color_low->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_low) = sbar_color_low->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_critical) != NULL && sbar_color_critical->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_critical) = sbar_color_critical->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_armor_1) != NULL && sbar_color_armor_1->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_armor_1) = sbar_color_armor_1->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_armor_2) != NULL && sbar_color_armor_2->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_armor_2) = sbar_color_armor_2->value.i + 1;
            if(JN_getNotCommonIntVarPointer(v_stbar_color_armor_0) != NULL && sbar_color_armor_0->found)
                *JN_getNotCommonIntVarPointer(v_stbar_color_armor_0) = sbar_color_armor_0->value.i + 1;
        }
    }

    FreeTrackers();
}

defaultTracker_t* M_GetDefaultTracker(const char* name)
{
    defaultTracker_t* tracker = defaultTrackers;
    while(tracker != NULL)
    {
        if(strcmp(tracker->name, name) == 0)
            return tracker;
        tracker = tracker->next;
    }
    return NULL;
}

void M_SetTrackedValue(defaultTracker_t *tracker, char *value)
{
    switch (tracker->type)
    {
        case DEFAULT_STRING:
            tracker->value.s = M_StringDuplicate(value);
            break;
        case DEFAULT_INT:
            tracker->value.i = strtol(value, NULL, 10);
            break;
        case DEFAULT_INT_HEX:
            tracker->value.i = strtol(value, NULL, 16);
            break;
        case DEFAULT_FLOAT:
        {
            // Different locales use different decimal separators.
            // However, the choice of the current locale isn't always
            // under our own control. If the strtof() function fails to
            // parse the string representing the floating point number
            // using the current locale's decimal separator, it will
            // return 0, resulting in silent sound effects. To
            // mitigate this, we replace the first non-digit,
            // non-minus character in the string with the current
            // locale's decimal separator before passing it to strtof().
            struct lconv *lc = localeconv();
            char dec, *str;
            int i = 0;

            dec = lc->decimal_point[0];
            str = M_StringDuplicate(value);

            // Skip sign indicators.
            if(str[i] == '-' || str[i] == '+')
            {
                i++;
            }

            for(; str[i] != '\0'; i++)
            {
                if(!isdigit(str[i]))
                {
                    str[i] = dec;
                    break;
                }
            }

            tracker->value.f = strtof(str, NULL);
            free(str);
            break;
        }
    }
    tracker->found = true;
}

static void RegisterTrackedDefault(const char* name, const default_type_t type)
{
    defaultTracker_t* tracker = malloc(sizeof(defaultTracker_t));
    tracker->name = name;
    tracker->type = type;
    tracker->value.s = NULL;
    tracker->found = false;
    tracker->next = NULL;

    if(defaultTrackers != NULL)
        tracker->next = defaultTrackers;

    defaultTrackers = tracker;
}

static void FreeTrackers()
{
    defaultTracker_t *tracker;

    while(defaultTrackers != NULL)
    {
        if(defaultTrackers->type == DEFAULT_STRING
        && defaultTrackers->found
        && defaultTrackers->value.s != NULL)
        {
            free(defaultTrackers->value.s);
        }

        tracker = defaultTrackers;
        defaultTrackers = tracker->next;
        free(tracker);
    }
}

//
// Copyright(C) 2022 Dasperal/Leonid Murin
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

#include "doomtype.h"
#include "m_config.h"
#include "rd_keybinds.h"

enum {
   /**
    * [Dasperal] This is only growing number that should be incremented when config structure changes
    * in a way that require use of migration mechanism\n
    *\n
    * Meaning of values:\n
    * 0 - Initial state\n
    * 1 - Made keys F[1, 2, 3, 4, 5, 7, 8, 10, 11], -, =, Pause not hardcoded and bindable.
    *     Changed text representation of NumPad 'Del' key from 'KP_,' to 'KP_.'.\n
    * 2 - Changed names of 'message_*_color' and 'sbar_color_*' config entries.
    *     Fixed broken bk_map_rotate, bk_map_rotate and bk_forward.\n
    */
    CURRENT_CONFIG_VERSION = 2
};

/**
 * Version of the read config
 */
extern int config_version;

typedef struct defaultTracker_s
{
    struct defaultTracker_s* next;
    const char* name;    // Name of the config variable
    default_type_t type; // Type of the variable
    union {
        int i;   // DEFAULT_INT and DEFAULT_INT_HEX types
        float f; // DEFAULT_FLOAT type
        char* s; // DEFAULT_STRING type
    } value;       // Value of the tracked config variable
    boolean found; // Is 'value' field contains valid data
} defaultTracker_t;

typedef struct keybindsTracker_s
{
    struct keybindsTracker_s* next;
    const char* keyName;
    bind_descriptor_t* descriptors;
}keybindsTracker_t;

defaultTracker_t* M_GetDefaultTracker(const char* name);
keybindsTracker_t* M_GetKeybindsTracker(const char* name);
void M_SetTrackedValue(defaultTracker_t *tracker, char *value);
void M_RegisterTrackedFields();
void M_ApplyMigration();

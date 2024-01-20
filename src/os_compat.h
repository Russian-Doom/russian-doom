//
// Copyright(C) 2021 Roman Fomin
// Copyright(C) 2021-2023 Julian Nechaevsky
// Copyright(C) 2021-2024 Leonid Murin (Dasperal)
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

#include "SDL_syswm.h"

/**
 * OS versions ordered from oldest to newest.
 * Each version represents a set of versions, starting with itself inclusively and ending with the next version (not inclusively).
 * The newest version represents all versions starting from itself, including future versions.
 */
typedef enum
{
   /**
    * The Unknown OS version is treated as the oldest because it can't guarantee the existence of any API
    */
    Unknown = 1,
#ifdef _WIN32
    Windows_XP,
    Windows_XP64,
    Windows_Vista,
    Windows_7,
    Windows_8,
    Windows_8_1,
    Windows_10,
    Windows_11
#endif
} os_version_t;

os_version_t OS_getVersion(void);

#ifdef _WIN32
void OS_DisableWindowCornersRounding(SDL_Window* window);
#endif

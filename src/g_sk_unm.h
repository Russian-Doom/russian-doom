//
// Copyright(C) 2023-2024 Leonid Murin (Dasperal)
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

#include "d_mode.h"

/**
 * Next, properly assign values according to selected skill level.
 * UNM definitions are might be not optimal for possible Dehacked / Hehacked mods,
 * but this skill itself is designed for original Doom, and UNM menu item
 * will be hidden if replaced M_ROUGH gfx is provided in PWAD.
 */
void UNM_Apply_Restore_Atters(skill_t skill);

/**
 * [JN] Keep original or given by Dehacked / Hehacked mobj info as separated values,
 * so they can be safely toggled between common and UNM skill levels.
 */
void UNM_Save_Atters(void);

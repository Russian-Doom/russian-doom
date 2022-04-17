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

#include <SDL_scancode.h>
#include "d_name.h"
#include "rd_keybinds.h"

int config_version = 0;

void RD_ApplyMigration()
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

        if(RD_GameType == gt_Doom || RD_GameType == gt_Heretic)
        {
            BK_AddBind(bk_detail, keyboard, SDL_SCANCODE_F5);
        }

        BK_AddBind(bk_end_game,    keyboard, SDL_SCANCODE_F7);
        BK_AddBind(bk_messages,    keyboard, SDL_SCANCODE_F8);
        BK_AddBind(bk_quit,        keyboard, SDL_SCANCODE_F10);
        BK_AddBind(bk_gamma,       keyboard, SDL_SCANCODE_F11);
        BK_AddBind(bk_screen_inc,  keyboard, SDL_SCANCODE_EQUALS);
        BK_AddBind(bk_screen_dec,  keyboard, SDL_SCANCODE_MINUS);
        BK_AddBind(bk_pause,       keyboard, SDL_SCANCODE_PAUSE);
    }
}

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2025 Leonid Murin (Dasperal)
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

#include <m_misc.h>
#include <SDL_render.h>

#include "m_config.h"

const char** available_render_drivers = NULL;
int8_t num_of_available_render_drivers = 0;
const char* render_driver_option = NULL;
int32_t render_driver_cursor = 0;
int8_t render_driver_index = 0;

void init_available_render_drivers()
{
    num_of_available_render_drivers = SDL_GetNumRenderDrivers();
    available_render_drivers = malloc(sizeof(char*) * num_of_available_render_drivers);
    for(int8_t index = 0; index < num_of_available_render_drivers; index++)
    {
        SDL_RendererInfo driver_info;
        SDL_GetRenderDriverInfo(index, &driver_info);
        available_render_drivers[index] = driver_info.name;
    }
}

int8_t get_render_driver_index(const char* driver_name)
{
    if(!driver_name)
    {
        return -1;
    }

    for(int8_t index = 0; index < num_of_available_render_drivers; index++)
    {
        if(strcmp(driver_name, available_render_drivers[index]) == 0)
        {
            return index;
        }
    }
    return -1;
}

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

#include <jn.h>
#include <m_misc.h>
#include <SDL_render.h>
#include <video_config.h>

#include "m_config.h"

render_driver_option_t _render_driver_options[9] = {
    {
        "Direct X 12",
        "direct3d12",
        false,
        -1
    },
    {
        "Direct X 11",
        "direct3d11",
        false,
        -1
    },
    {
       "Direct X",
       "direct3d",
       false,
       -1
   },
   {
       "Metal",
       "metal",
       false,
       -1
   },
   {
       "OpenGL",
       "opengl",
       false,
       -1
   },
   {
       "OpenGL ES 2",
       "opengles2",
       false,
       -1
   },
   {
       "Vulkan",
       "vulkan",
       false,
       -1
   },
   {
       "GPU",
       "gpu",
       false,
       -1
   },
   {
       "Software",
       "software",
       false,
       -1
   }
};

char* render_driver_option = NULL;
int32_t render_driver_cursor = 0;
int8_t render_driver_index = 0;

int8_t get_render_driver_option_index(const char* driver_name)
{
    if(!driver_name)
    {
        return -1;
    }

    for(int8_t i = 0; i < arrlen(_render_driver_options); i++)
    {
        if(strcmp(driver_name, _render_driver_options[i].driver_name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void init_available_render_drivers()
{
    const int8_t num_of_available_render_drivers = SDL_GetNumRenderDrivers();
    SDL_RendererInfo driver_info;
    for(int8_t index = 0; index < num_of_available_render_drivers; index++)
    {
        SDL_GetRenderDriverInfo(index, &driver_info);
        const int8_t option_index = get_render_driver_option_index(driver_info.name);
        printf(english_language ? "Available SDL render drivers:\n" :
                                  "Доступные драйверы рендера SDL:\n");
        if(option_index >= 0)
        {
            _render_driver_options[option_index].driver_index = index;
            printf("\t%s\n",
                   driver_info.name);
            if(option_index != software && option_index != gpu)
            {
                _render_driver_options[option_index].active = true;
            }
        }
    }
}

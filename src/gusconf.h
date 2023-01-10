//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
// DESCRIPTION:
//     GUS emulation code.
//


#pragma once

#include "doomtype.h"

/**
 * Dynamic version of SDL_MIXER_VERSION_ATLEAST macro
 */
#define LIB_VERSION_ATLEAST(lpVersionStruct, X, Y, Z) \
    (((lpVersionStruct)->major >= (X)) && \
     ((lpVersionStruct)->major > (X) || (lpVersionStruct)->minor >= (Y)) && \
     ((lpVersionStruct)->major > (X) || (lpVersionStruct)->minor > (Y) || (lpVersionStruct)->patch >= (Z)))

extern char *gus_patch_path;
extern int gus_ram_kb;

boolean GUS_WriteConfig(char *path);

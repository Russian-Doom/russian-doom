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
// DESCRIPTION:
//	Endianess handling, swapping 16bit and 32bit.
//


#pragma once

#include "SDL_endian.h"


// Endianess handling.
// WAD files are stored little endian.

// Just use SDL's endianness swapping functions.

// These are deliberately cast to signed values; this is the behaviour
// of the macros in the original source and some code relies on it.

#define SHORT(x)  ((signed short) SDL_SwapLE16(x))
#define LONG(x)   ((signed int) SDL_SwapLE32(x))

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define SYS_BIG_ENDIAN
#endif

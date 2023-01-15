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
//     Pseudo-random number generator for secure demos.
//


#pragma once

#include "doomtype.h"


// Nonce value used as random seed for secure demos.

typedef byte prng_seed_t[16];

void PRNG_Start(prng_seed_t seed);
void PRNG_Stop(void);
unsigned int PRNG_Random(void);

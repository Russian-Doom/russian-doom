//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//
//    


#pragma once

#include "doomtype.h"


// Returns a number from 0 to 255, from a lookup table.
const int M_Random (void);

// As M_Random, but used only by the play simulation.
const int P_Random (void);

// [crispy] our own private random function
const int Crispy_Random (void);

// Fix randoms for demos.
void M_ClearRandom (void);

// Defined version of P_Random() - P_Random()
const int P_SubRandom (void);
const int Crispy_SubRandom (void);

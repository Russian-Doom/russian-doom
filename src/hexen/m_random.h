//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#pragma once


// Most damage defined using HITDICE
#define HITDICE(a) ((1+(P_Random()&7))*a)

int M_Random(void);
// returns a number from 0 to 255
int P_Random(void);
// as M_Random, but used only by the play simulation

void M_ClearRandom(void);
// fix randoms for demos

extern int rndindex;

// Defined version of P_Random() - P_Random()
int P_SubRandom (void);

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
//  Nil.
//    


#pragma once

#include "doomtype.h"


//
// MISC
//
extern  int	myargc;
extern  char**	myargv;

// [Dasperal] exedir from chocolate
extern char* exedir;
extern char* packageResourcesDir;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm (char* check);

// Same as M_CheckParm, but checks that num_args arguments are available
// following the specified argument.
int M_CheckParmWithArgs(char *check, int num_args);

void M_FindResponseFile(void);
void M_AddLooseFiles(void);

// Parameter has been specified?

boolean M_ParmExists(char *check);

// Get name of executable used to run this program:

char *M_GetExecutableName(void);

void M_PrintHelp(void);

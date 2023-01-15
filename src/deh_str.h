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
//
// Dehacked string replacements
//


#pragma once

#include <stdio.h>
#include "doomfeatures.h"
#include "doomtype.h"


// Used to do dehacked text substitutions throughout the program

#ifdef FEATURE_DEHACKED

char *DEH_String(char *s);
void DEH_printf(char *fmt, ...);
void DEH_fprintf(FILE *fstream, char *fmt, ...);
void DEH_snprintf(char *buffer, size_t len, char *fmt, ...);
void DEH_AddStringReplacement(char *from_text, char *to_text);
boolean DEH_HasStringReplacement(char *s);


#else

#define DEH_String(x) (x)
#define DEH_printf printf
#define DEH_fprintf fprintf
#define DEH_snprintf snprintf

#endif

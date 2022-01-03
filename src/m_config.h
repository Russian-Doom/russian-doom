//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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
//      Configuration file interface.
//    



#ifndef __M_CONFIG__
#define __M_CONFIG__

#include <stdio.h>
#include "doomtype.h"

typedef struct
{
    /**
     * Returns true if section with a given name is handled by this handler.
     * This function also used to initialize handlers internal state if section is handled by this handler.
     */
    boolean (*handles) (char* sectionName);
    /**
     * Called for every line in the handled section.
     */
    void (*handleLine) (char* keyName, char *value, size_t valueSize);
    /**
     * Saves all data of the handled section to the config file.
     */
    void (*save) (FILE *file, char* sectionName);

    /**
     * Called in the end of section handling.
     * This function used to free handlers internal state.
     */
    void (*onFinishHandling) ();
} sectionHandler_t;

void M_LoadConfig(void);
void M_SaveConfig(void);
void M_AppendConfigSection(const char* sectionName, sectionHandler_t* handler);
void M_SaveDefaultAlternate(char *main);
void M_SetConfigDir(char *dir);
void M_BindIntVariable(char *name, int *variable);
void M_BindFloatVariable(char *name, float *variable);
void M_BindStringVariable(char *name, char **variable);
void M_SetConfigFilename(char *name);
char *M_GetSaveGameDir();

extern char *configdir;

#endif

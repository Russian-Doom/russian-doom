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
//	WAD I/O functions.
//


#pragma once

#include <stdio.h>
#include "doomtype.h"
#include "w_file.h"


//
// TYPES
//

//
// WADFILE I/O related stuff.
//

typedef struct lumpinfo_s lumpinfo_t;
typedef int lumpindex_t;

struct lumpinfo_s
{
    char	name[8];
    wad_file_t *wad_file;
    int		position;
    int		size;
    void       *cache;

    // Used for hash table lookups
    lumpindex_t next;
};

int W_CheckMultipleLumps(char *name);

extern lumpinfo_t **lumpinfo;
extern unsigned int numlumps;

wad_file_t *W_AddFile(char *filename);
void W_Reload(void);

lumpindex_t W_CheckNumForName(char *name);
lumpindex_t W_GetNumForName(char *name);
lumpindex_t W_CheckNumForNameFromTo(const char *name, int from, int to);

int W_LumpLength(lumpindex_t lump);
void W_ReadLump(lumpindex_t lump, void *dest);

void *W_CacheLumpNum(lumpindex_t lumpnum, int tag);
void *W_CacheLumpName(char *name, int tag);

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(lumpindex_t lumpnum);
void W_ReleaseLumpName(char *name);

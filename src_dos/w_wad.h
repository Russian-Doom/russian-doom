//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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


#ifndef __W_WAD__
#define __W_WAD__


//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char        identification[4];		
    int         numlumps;
    int         infotableofs;
} wadinfo_t;

typedef struct
{
    int         filepos;
    int         size;
    char        name[8];
} filelump_t;


//
// WADFILE I/O related stuff.
//
typedef struct
{
    char        name[8];
    int         handle;
    int         position;
    int         size;
    // Used for hash table lookups
    int next;

    // killough 4/17/98: namespace tags, to prevent conflicts between resources
    enum {
    ns_global=0,
    ns_sprites,
    ns_flats,
    ns_colormaps
    } namespace;
} lumpinfo_t;


void    W_InitMultipleFiles (char** filenames);
void    W_Reload (void);
void    W_ReadLump (int lump, void *dest);
void   *W_CacheLumpNum (int lump, int tag);
void   *W_CacheLumpName (char* name, int tag);

int     W_CheckNumForName (char* name);
int     W_GetNumForName (char* name);
int     W_LumpLength (int lump);

extern  void       **lumpcache;
extern  int          numlumps;
extern  lumpinfo_t  *lumpinfo;

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(char *s);

#endif


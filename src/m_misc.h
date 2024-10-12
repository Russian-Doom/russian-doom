//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2024 Leonid Murin (Dasperal)
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
//      Miscellaneous.
//    


#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#endif

#include "doomtype.h"

boolean M_WriteFile(const char *name, void *source, int length);
boolean M_WriteFileTimeout(const char *name, void *source, int length, int delay);
int M_ReadFile(char *name, byte **buffer);
void M_MakeDirectory(char *dir);
char *M_TempFile(char *s);
boolean M_FileExists(char *file);
char *M_FileCaseExists(char *file);
long M_FileLength(FILE *handle);
boolean M_PathWritable(const char* path);

/**
 * Cross platform version of fopen with UTF-8 support.
 * M_fopen must be used instead of stdio.h/fopen
 */
FILE* M_fopen(const char* filename, const char* mode);

/**
 * Cross platform version of remove with UTF-8 support.
 * M_remove must be used instead of stdio.h/remove
 */
int M_remove(const char* path);

/**
 * Cross platform version of rename with UTF-8 support.
 * M_rename must be used instead of stdio.h/rename
 */
int M_rename(const char* oldname, const char* newname);

/**
 * Cross platform version of stat with UTF-8 support.
 * M_stat must be used instead of stdio.h/stat
 */
int M_stat(const char* path, struct stat* buf);
boolean M_StrToInt(const char *str, int *result);
char *M_DirName(const char *path);
const char *M_FileName(const char *path);
void M_ExtractFileBase(const char* path, char* dest, int dest_size);
void M_ForceUppercase(char *text);
void M_ForceLowercase(char *text);
char *M_StrCaseStr(char *haystack, char *needle);
char *M_StringDuplicate(const char *orig);
boolean M_StringCopy(char *dest, const char *src, size_t dest_size);
boolean M_StringConcat(char *dest, const char *src, size_t dest_size);
char *M_StringReplace(const char *haystack, const char *needle,
                      const char *replacement);
char *M_StringJoin(const char *s, ...);
boolean M_StringStartsWith(const char *s, const char *prefix);
boolean M_StringEndsWith(const char *s, const char *suffix);
void M_FreeStringArray_NullTerminated(const char** array);
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...);
char *M_OEMToUTF8(const char *ansi);
void M_NormalizeSlashes(char *str);
char* RD_M_FindInternalResource(char* resourceName);

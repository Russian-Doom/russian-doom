//
//  Copyright(C) 2021 Roman Fomin
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// DESCRIPTION:
//      unicode paths for fopen() on Windows


#ifndef RD_WIN_IO_H
#define RD_WIN_IO_H

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <direct.h>

wchar_t* ConvertToUtf8(const char *str);
FILE* D_fopen(const char *filename, const char *mode);
int D_remove(const char *path);
int D_rename(const char *oldname, const char *newname);
int D_mkdir(const char *dirname);

#undef  fopen
#define fopen(n, m) D_fopen(n, m)

#undef  remove
#define remove(p) D_remove(p)

#undef  rename
#define rename(o, n) D_rename(o, n)

// [Dasperal] not used
// #undef  stat
// #define stat(p, b) D_stat(p, b)

// #undef  open
// #define open(n, of) D_open(n, of)

// #undef  access
// #define access(p, m) D_access(p, m)

#undef  mkdir
#define mkdir(d) D_mkdir(d)
#endif

#endif //RD_WIN_IO_H

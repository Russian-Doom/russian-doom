//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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
//	Main program, simply calls D_DoomMain high level loop.
//



#include "config.h"

#include <stdio.h>

#include "SDL.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"

// [JN] Vanilla game mode available for all three games in RD
boolean vanillaparm;

// [JN] Devparm available for all three games in RD
boolean devparm;


#ifdef _WIN32
// -----------------------------------------------------------------------------
// RD_CreateWindowsConsole
// [JN] Creates console output Window. For Windows OS only.
// -----------------------------------------------------------------------------
void RD_CreateWindowsConsole (void)
{
    // [JN] Allocate console.
    AllocConsole();

    // [JN] Head text outputs.
    freopen("CONIN$", "r",stdin); 
    freopen("CONOUT$","w",stdout); 
    freopen("CONOUT$","w",stderr); 

    // [JN] Set a proper codepage.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}
#endif

void M_SetExeDir(void)
{
#ifdef _WIN32
    TCHAR dirname[MAX_PATH + 1];
    DWORD dirname_len;
    TCHAR *fp = NULL;

    memset(dirname, 0, sizeof(dirname));
    dirname_len = GetModuleFileName(NULL, dirname, MAX_PATH);
    fp = &dirname[dirname_len];
    while (dirname <= fp && *fp != DIR_SEPARATOR)
    {
        fp--;
    }
    *(fp + 1) = '\0';

    exedir = M_StringDuplicate(dirname);
#else
    char *dirname;

    dirname = M_DirName(myargv[0]);
    exedir = M_StringJoin(dirname, DIR_SEPARATOR_S, NULL);
    free(dirname);
#endif
}

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);


int main(int argc, char **argv)
{
    // save arguments

    myargc = argc;
    myargv = argv;

    M_SetExeDir();

    M_FindResponseFile();

    // Check for -devparm being activated
    devparm = M_CheckParm ("-devparm");

#ifdef _WIN32
    // [JN] Create a console output on Windows for devparm mode.
    if (devparm)
    {
        RD_CreateWindowsConsole();
    }
#endif

    // [JN] Activate vanilla gameplay mode.
    // All optional enhancements will be disabled 
    // without modifying configuration files.
    vanillaparm = M_ParmExists("-vanilla");

    // start doom

    D_DoomMain ();

    return 0;
}


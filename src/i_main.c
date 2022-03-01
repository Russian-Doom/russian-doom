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
//	Main program, simply calls D_DoomMain high level loop.
//



#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>   // [JN] srand(time(0))

#include "SDL.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syslimits.h>
#else
#include <sys/stat.h>
#include <unistd.h>
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
#ifdef __APPLE__ //TODO [Dasperal] test this
    uint32_t buffSize = PATH_MAX+1;
    char *exenameRaw, *exename, *dirname;
    int result;

    exenameRaw = malloc(buffSize);
    result = _NSGetExecutablePath(exenameRaw, &buffSize);
    if(result == -1) // if needs larger buffer - retry
    {
        free(exenameRaw);
        exenameRaw =  malloc(buffSize);
        result = _NSGetExecutablePath(exenameRaw, &buffSize);
    }

    if(result == 0)
    {
        exename = malloc(buffSize); // [Dasperal] Use a buffer of the same size and hope that this is enough
        if(realpath(exenameRaw, exename) != NULL)
        {
            dirname = M_DirName(exename);
        }
        else
        {
            printf("I_MAIN: Error: Unable to resolve path to executable. Errored path:\n \t%s\n", exename);
            printf("I_MAIN: Trying to get path to executable from arg0\n \t%s\n", myargv[0]);
            dirname = M_DirName(myargv[0]);
        }
        free(exename);
    }
    else
    {
        printf("I_MAIN: Error: Unable to get path to executable from _NSGetExecutablePath\n");
        printf("I_MAIN: Trying to get path to executable from arg0\n \t%s\n", myargv[0]);
        dirname = M_DirName(myargv[0]);
    }
    free(exenameRaw);
    exedir = M_StringJoin(dirname, DIR_SEPARATOR_S, NULL);
    free(dirname);
#else // Windows & Linux
    exedir = SDL_GetBasePath();
    if(!exedir)
    {
        char* temp;
        printf("I_MAIN: Error: Unable to get path to executable from SDL_GetBasePath");
        printf("I_MAIN: Trying to get path to executable from arg0\n \t%s\n", myargv[0]);
        temp = M_DirName(myargv[0]);
        exedir = M_StringJoin(temp, DIR_SEPARATOR_S, NULL);
        free(temp);
    }
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
    myargv = malloc(argc * sizeof(char *));
    assert(myargv != NULL);

    for (int i = 0; i < argc; i++)
    {
        myargv[i] = M_StringDuplicate(argv[i]);
    }

    M_SetExeDir();
#ifdef __APPLE__
    packageResourcesDir = SDL_GetBasePath();
#endif

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

    // [JN] Use current time as seed for random generator.
    srand(time(0));

    // start doom

    D_DoomMain ();

    return 0;
}


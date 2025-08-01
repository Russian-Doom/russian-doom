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
//	Main program, simply calls D_DoomMain high level loop.
//


#include <assert.h>
#include <stdio.h>
#include <time.h>   // [JN] srand(time(0))

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_net.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdlib.h>
#include <io.h>

#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syslimits.h>
#include <unistd.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "doomtype.h"
#include "d_name.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "jn.h"

#include "git_info.h"

static void printVersion(void);

// [JN] Vanilla game mode available for all three games in RD
boolean vanillaparm;

// [JN] Devparm available for all three games in RD
boolean devparm;

#ifdef _WIN32
// -----------------------------------------------------------------------------
// RD_CreateWindowsConsole
// [JN] Creates a console output Window. For Windows OS only.
// -----------------------------------------------------------------------------
static boolean console_created = false;
boolean console_connected = false;

static void ReopenConsoleHandles(void)
{
    HANDLE handle;
    DWORD lpmode = 0;

    handle = GetStdHandle(STD_INPUT_HANDLE);
    if(GetConsoleMode(handle, &lpmode))
    {
        freopen("CONIN$", "rt", stdin);
    }

    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(GetConsoleMode(handle, &lpmode))
    {
        freopen("CONOUT$", "wt", stdout);
    }

    handle = GetStdHandle(STD_ERROR_HANDLE);
    if(GetConsoleMode(handle, &lpmode))
    {
        freopen("CONOUT$", "wt", stderr);
    }
}

void RD_CreateWindowsConsole(void)
{
    DWORD mode;

    // [JN] Console already created, don't try to create it again.
    if(console_connected || console_created)
    {
        return;
    }

    // [JN] Allocate console.
    AllocConsole();

    // [JN] Head text outputs.
    ReopenConsoleHandles();

    SetConsoleTitle("Console");

    // [JN] Set a proper codepage and mode
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(handle, &mode);
    SetConsoleMode(handle, mode | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);

    console_created = true;
}
#endif

int lang_param;

static void CheckLangParam()
{
    lang_param = M_CheckParmWithArgs("-lang", 1);
    if(!lang_param)
        return;

    if(strcmp(myargv[lang_param + 1], "en") == 0)
    {
        english_language = 1; // english
    }
    else if(strcmp(myargv[lang_param + 1], "ru") == 0)
    {
        english_language = 0; // russian
    }
    else
    {
        printf("-LANG: Unknown language");
        // default to english for readability because old Windows versions can't display UTF-8 chars in console
        english_language = 1;
    }
}

void M_SetExeDir(void)
{
    char *dirname;
#ifdef __APPLE__ //TODO [Dasperal] test this
    uint32_t buffSize = PATH_MAX+1;
    char *exenameRaw, *exename;
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
    dirname = SDL_GetBasePath();
    if(dirname)
    {
        exedir = M_StringDuplicate(dirname);
        SDL_free(dirname);
    }
    else
    {
        printf("I_MAIN: Error: Unable to get path to executable from SDL_GetBasePath");
        printf("I_MAIN: Trying to get path to executable from arg0\n \t%s\n", myargv[0]);
        dirname = M_DirName(myargv[0]);
        exedir = M_StringJoin(dirname, DIR_SEPARATOR_S, NULL);
        free(dirname);
    }
#endif
}

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

#ifdef _WIN32
__declspec(dllexport) int RussianDoom_Main(int argc, char** argv)
#else
int main(int argc, char **argv)
#endif
{
    // save arguments

    myargc = argc;
    myargv = malloc(argc * sizeof(char *));
    assert(myargv != NULL);

    for (int i = 0; i < argc; i++)
    {
        myargv[i] = M_StringDuplicate(argv[i]);
    }

    // Check for CLI params demanding console being activated
    devparm = M_CheckParm ("-devparm");
    const int version_param = M_CheckParm("--version");
    const boolean help_param = M_CheckParm("--help") // Standard Linux help
                            || M_CheckParm("-h")     // Standard Linux help (short CLI_Parameter)
                            || M_CheckParm("-help")  // Linux help
                            || M_CheckParm("/?")     // Standard Windows CMD help
                            || M_CheckParm("-?");    // Standard Windows PowerShell help

#ifdef _WIN32
    // [JN] if game language is not set yet (-1), and OS-preferred language
    // is appropriate for using Russian language in the game, use it.
    if(english_language == -1)
    {
        // [JN] Get system-preferred language
        DWORD lang_id = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));

        if(lang_id != LANG_RUSSIAN
        && lang_id != LANG_UKRAINIAN
        && lang_id != LANG_BELARUSIAN)
            english_language = 1;
        else
            english_language = 0;
    }

    // [JN] Create a console output window if any of CLI params demands it
    if(!console_connected && (devparm || version_param || help_param))
    {
        RD_CreateWindowsConsole();
    }
#endif
    // Route stderr messages to stdout
    dup2(1, 2);

    // Check for -lang param before loading response file to show potential errors in the correct language
    CheckLangParam();

    // compose a proper command line from loose file paths passed as arguments
    // to allow for loading WADs and DEHACKED patches by drag-and-drop and "open with"
    M_AddLooseFiles();

    M_FindResponseFile();

    // Check for -lang param again after loading response file to set correct language if -lang param was in response file
    CheckLangParam();

    if(version_param)
    {
        printVersion();
        CONSOLE_EPILOG
        return 0;
    }

    if(help_param)
    {
        M_PrintHelp();
        CONSOLE_EPILOG
        return 0;
    }

    M_SetExeDir();
#ifdef __APPLE__
    packageResourcesDir = SDL_GetBasePath();
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

static void printVersion(void)
{
    printf("%s\n", RD_Project_String);
    printf("Revision: %s (%s)\n", GIT_SHA, GIT_TIME);
    printf("SDL version\n\tCompiled: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    printf("\tRuntime: %d.%d.%d\n", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
    printf("SDL_mixer version\n\tCompiled: %d.%d.%d\n", SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL);
    const SDL_version* sdl_mixerVersion = Mix_Linked_Version();
    printf("\tRuntime: %d.%d.%d\n", sdl_mixerVersion->major, sdl_mixerVersion->minor, sdl_mixerVersion->patch);
    printf("SDL_net version\n\tCompiled: %d.%d.%d\n", SDL_NET_MAJOR_VERSION, SDL_NET_MINOR_VERSION, SDL_NET_PATCHLEVEL);
    const SDL_version* sdl_netVersion = SDLNet_Linked_Version();
    printf("\tRuntime: %d.%d.%d\n", sdl_netVersion->major, sdl_netVersion->minor, sdl_netVersion->patch);
}


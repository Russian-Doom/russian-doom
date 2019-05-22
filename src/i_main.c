//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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


// [JN] Devparm available for all three games in RD
boolean devparm;


//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

/* [JN] TODO - implement a proper return to cmd.exe
void I_RD_SendReturn (void)
{
    keybd_event(VK_RETURN,
                0x0D,
                KEYEVENTF_EXTENDEDKEY,
                0);
}
*/

int main(int argc, char **argv)
{
    // save arguments

    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

    // [JN] Make a console output for Windows. There two ways:
    //  1) In case of using -devparm, separate console window will appear.
    //  2) In case of starting game from Windows console (cmd.exe), all 
    //     prints will be made in existing console.

#ifdef _WIN32
    // Check for -devparm being activated
    devparm = M_CheckParm ("-devparm");

    if (devparm)
    {   // Create a separate console window
        AllocConsole();
    }
    /* [JN] TODO - implement a proper return to cmd.exe
    else
    {
        // Use an existing console window
        AttachConsole(-1);

        // Clear console contens to emulate vanilla
        // behaviour and for proper line breaking
        system("cls");

        // Send an 'ENTER' key after exiting the game
        // for proper return to command prompt
        I_AtExit(I_RD_SendReturn, false);
    }
    */

    // Head text outputs
    freopen("CONIN$", "r",stdin); 
    freopen("CONOUT$","w",stdout); 
    freopen("CONOUT$","w",stderr); 

    // Set a proper codepage
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // start doom

    D_DoomMain ();

    return 0;
}


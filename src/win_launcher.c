//
// Copyright(C) 2022-2024 Leonid Murin (Dasperal)
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

#ifdef HAVE_CONSOLE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "SDL.h"
#include "doomtype.h"

__declspec(dllimport) extern boolean console_connected;
__declspec(dllimport) extern int RussianDoom_Main(int argc, char** argv);

int main(int argc, char** argv)
{
#ifdef HAVE_CONSOLE
    console_connected = true;

    // [JN] Set a proper codepage and mode
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    return RussianDoom_Main(argc, argv);
}

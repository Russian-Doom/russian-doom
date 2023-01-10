//
// Copyright(C) 2022-2023 Leonid Murin (Dasperal)
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

#include "SDL.h"
#include "doomtype.h"

dll_import extern boolean console_connected;
dll_import extern int InterDoom_Main(int argc, char** argv);

int main(int argc, char** argv)
{
#ifdef HAVE_CONSOLE
    console_connected = true;
#endif
    return InterDoom_Main(argc, argv);
}

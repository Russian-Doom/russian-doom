//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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
//
// DESCRIPTION:
//  Main program, simply calls D_DoomMain high level loop.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "doomdef.h"
#include "m_misc.h"
#include "d_main.h"

int main (int argc, char** argv) 
{ 
    // [JN] Use current time as seed for random generator.
    srand(time(0));

    myargc = argc; 
    myargv = argv; 

    D_DoomMain (); 

    return 0;
}


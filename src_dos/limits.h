//
// Copyright (C) 2020 Julian Nechaevsky
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
//  Doom static limits, collected in one place.
//  Useful references:
//  - https://doomwiki.org/wiki/Static_limits
//  - https://doomwiki.org/wiki/Doom32
//


#ifndef __LIMITS__
#define __LIMITS__


// [JN] Uncommenting following definition will build Doom+ limits executable.
//
// MAXDRAWSEGS and MAXVISPLANES was slightliy increased from vanilla values 
// to the Doom Classic level. It's far than enough to run big maps like TNT
// MAP21 and MAP27 without crashing. SAVEGAMESIZE was doubled just in case.
//
// NOTE: Doom+ limits version requires 32 MiB of RAM *minimum*, while
// vanilla requires only 16 MiB. Default size in DOSBox is set to 32 MiB
// ("memsize=32" variable), so it should not be a problem. But back in 1994...

// #define DOOM_PLUS_LIMITS

#ifndef DOOM_PLUS_LIMITS     // Vanilla limits
  #define MAXDRAWSEGS        1280    // Increased from 256
  #define MAXLINEANIMS       64
  #define MAXOPENINGS        20480
  #define MAXPLATS           30
  #define MAXVISPLANES       384     // Increased from 128
  #define MAXVISSPRITES      128
  #define SAVEGAMESIZE       360448  // Increased from 180224
#else                        // Doom+ limits
  #define MAXDRAWSEGS        2048
  #define MAXLINEANIMS       16384
  #define MAXOPENINGS        65536
  #define MAXPLATS           7680
  #define MAXVISPLANES       1024
  #define MAXVISSPRITES      1024
  #define SAVEGAMESIZE       2883584
#endif


#endif

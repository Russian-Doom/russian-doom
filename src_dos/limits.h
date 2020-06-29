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


// [JN] Comment following definition to make a build with strict vanilla limits.
//
// NOTE: in Doom+ limits, SAVEGAMESIZE is smaller than actual Doom+:
// 524288 (512 KiB) instead of 2883584 (2,75 MiB), for keeping ability
// to run the game on 16 MiB of RAM.

#define DOOM_PLUS_LIMITS

#ifndef DOOM_PLUS_LIMITS     // Vanilla limits
  #define MAXDRAWSEGS        256
  #define MAXLINEANIMS       64
  #define MAXOPENINGS        20480
  #define MAXPLATS           30
  #define MAXVISPLANES       128
  #define MAXVISSPRITES      128
  #define MAXANIMS           32
  #define MAX_ADJOINING_SECTORS 20
  #define SAVEGAMESIZE       180224
#else                        // Doom+ limits
  #define MAXDRAWSEGS        2048
  #define MAXLINEANIMS       16384
  #define MAXOPENINGS        65536
  #define MAXPLATS           7680
  #define MAXVISPLANES       1024
  #define MAXVISSPRITES      1024
  #define MAXANIMS           32
  #define MAX_ADJOINING_SECTORS 20
  #define SAVEGAMESIZE       524288
#endif


#endif

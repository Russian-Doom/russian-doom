# Doom Vanille

Linux Doom ported to DOS. Aims at 100% compatibility with the original executables that came with the different games. 

This means that: 

1. This source port will work with the registered and shareware versions of Doom. 
2. This source port will fix the [boucing Lost Souls bug](https://doomwiki.org/wiki/Demo_desyncing_caused_by_bouncing_lost_souls) when playing The Ultimate Doom or Final Doom. 
3. This source port will emulate the [Final Doom teleportation bug](https://doomwiki.org/wiki/Final_Doom_teleporters_do_not_set_Z_coordinate) when playing Final Doom. 
4. This source port can emulate the alternative Final Doom executable that came with the id Anthology (use `-alt`)

## Supported games
* Doom shareware
* Doom registered
* The Ultimate Doom
* Doom II: Hell on Earth
* Final Doom - TNT: Evilution
* Final Doom - The Plutonia Experiment

You can see the supported IWAD names [here](https://github.com/AXDOOMER/doom-vanille/wiki/Supported-IWADs).

If you have multiple IWADs in your directory, you can use the `-iwad` parameter to specify which IWAD to use. 

## Source code

Doom Vanille is based on [PCDoom](https://github.com/nukeykt/pcdoom), but was updated to [PCDoom2](https://github.com/nukeykt/PCDoom-v2/). 

Unlike the latter, no other tool than the Watcom compiler is necessary to compile it. 

## Changes

Please visit the [bug fixes and improvements](https://github.com/AXDOOMER/doom-vanille/wiki/Changes) page on the [wiki](https://github.com/AXDOOMER/doom-vanille/wiki).

## Build instructions

1) Install Open Watcom C.

2) Open pcdoom.wpj in Watcom.

3) Build.

4) Copy `pcdoom.exe` to your Doom folder. 

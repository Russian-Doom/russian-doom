# Russian Doom for DOS

This is a sub-project of the main project Russian Doom. 

## Supported games
* Doom shareware
* Doom registered
* The Ultimate Doom
* Doom II: Hell on Earth
* Final Doom - TNT: Evilution
* Final Doom - The Plutonia Experiment
* SIGIL (five episodes version)

If you have multiple IWADs in your directory, you can use the `-iwad` parameter to specify which IWAD to use. 

## New command line parameters

`-vanilla`<br />
Enables "vanilla" mode, that will turn off almost all improvements like colored blood, shadowed text and other.
Note: it does affect increased engine limits and bug fixes.

`-mb`<br />
Define amount of memory for game to use. Minimum is 2 MB, maximum is 64 MB.

`-dm3`<br />
Enables newschool deathmatch (Deathmatch 3.0) mode. Weapons will stay after picking up, but items and powerups will dissapear, and will respawn after 30 seconds.


## Compiling

Only Open Watcom C is required. Steps to compile:

1. Install [Open Watcom C](http://www.openwatcom.org/) version 1.9 or 2.0 beta.
2. Open rusdoom.wpj in Watcom.
3. Build.
4. Copy rusdoom.exe to your Doom folder.

## Gravis UltraSound

Gravis UltraSound sound synth requires a GUS patches to be installed. Please follow this [instruction](https://github.com/JNechaevsky/russian-doom/blob/master/src_dos/ultrasnd/README.md) to setup them in DOSBox.


## Source code

DOS version is based on [PCDoom](https://github.com/nukeykt/PCDoom-v2) by Alexey Khokholov and [Doom Vanille](https://github.com/AXDOOMER/doom-vanille) by Alexandre-Xavier Labonté-Lamoureux, with significant additions from [Crispy Doom](https://www.chocolate-doom.org/wiki/index.php/Crispy_Doom) by Fabian Greffrath, and distibutes with their permission. 

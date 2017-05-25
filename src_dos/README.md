# Russian DOOM for DOS

This is a sub-project of main project Russian DOOM. 

## Supported games
* Doom shareware
* Doom registered
* The Ultimate Doom
* Doom II: Hell on Earth
* Final Doom - TNT: Evilution
* Final Doom - The Plutonia Experiment

If you have multiple IWADs in your directory, you can use the `-iwad` parameter to specify which IWAD to use. 

## New command line parameters

`-vanilla`<br />
Enables "vanilla" mode, that will turn off almost all improvements like colored blood, shadowed text and other.
Note: it does affect increased engine limits and bug fixes.

`-pitch`<br />
Activates sound pitching like in earlier versions of Doom.

`-dm3`<br />
Enables newschool deathmatch (Deathmatch 3.0) mode. Weapons will stay after picking up, but items and powerups will dissapear, and will respawn after 30 seconds.


## Compiling

Open Watcom C is required. For compiling instructions please refer to this [page](https://github.com/JNechaevsky/russian-doom/blob/master/COMPILING.md).

## Source code

DOS version is based on [PCDoom](https://github.com/nukeykt/PCDoom-v2) by Alexey Khokholov and [Doom Vanille](https://github.com/AXDOOMER/doom-vanille) by Alexandre-Xavier Labonté-Lamoureux, with significant additions from [Crispy Doom](https://www.chocolate-doom.org/wiki/index.php/Crispy_Doom) by Fabian Greffrath, and distibutes with their permission. 

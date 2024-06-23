//
// Copyright(C) 2021-2024 Leonid Murin (Dasperal)
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


#pragma once


typedef enum
{
    gt_Doom,
    gt_Heretic,
    gt_Hexen,
    gt_Strife
} GameType_t;

// [Dasperal] Those vars should be defined in game-specific code
extern char* RD_Project_Name; // Just project name
extern char* RD_Project_String; // Project name with the full version
extern char* RD_Project_Version; // Just full version with short sha and date (for 'version' cheat only)
extern char* RD_Project_TarName;
extern char* ID_Project_TarName; // ID config compatibility
extern GameType_t RD_GameType;
extern char* RD_Project_CDRom_Dir;

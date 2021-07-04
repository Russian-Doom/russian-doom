//
// Copyright(C) 2021 Dasperal
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

#ifndef RD_D_NAME_H
#define RD_D_NAME_H

typedef enum
{
    gt_Doom,
    gt_Heretic,
    gt_Hexen,
    gt_Strife
} GameType_t;

// [Dasperal] Those vars should be defined in game-specific code
extern char* RD_Project_Name;
extern char* RD_Project_String;
extern GameType_t RD_GameType;

#endif //RD_D_NAME_H

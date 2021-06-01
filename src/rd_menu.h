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

#ifndef RD_MENU_H
#define RD_MENU_H

#include <stdint.h>

typedef enum
{
    ITT_EMPTY,
    ITT_EFUNC,
    ITT_LRFUNC,
    ITT_SETMENU,
    ITT_INERT
} ItemType_t;

typedef struct
{
    const ItemType_t type;
    const char* const text_eng;
    const char* const text_rus;
    void (*func) (intptr_t option);
    const intptr_t option;
} MenuItem_t;

typedef struct Menu_s
{
    const int x_eng;
    int y;
    const int x_rus;
    int itemCount;
    const MenuItem_t* const items;
    void (*drawFunc) (void);
    const struct Menu_s* const prevMenu;

    int lastOn;
} Menu_t;

#endif //RD_MENU_H

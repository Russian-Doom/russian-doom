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
#include "rd_text.h"

#define LEFT_DIR 0
#define RIGHT_DIR 1

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

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_SpinInt(int* var, int minValue, int maxValue, int direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_SpinInt_Step(int* var, int minValue, int maxValue, int step, int direction);

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
extern void RD_Menu_SlideInt(int* var, int minValue, int maxValue, int direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
extern void RD_Menu_SlideInt_Step(int* var, int minValue, int maxValue, int step, int direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
extern void RD_Menu_SlideFloat_Step(float* var, float minValue, float maxValue, float step, int direction);

/**
 * Shifts value of 'var' by 2 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_ShiftSpinInt(int* var, int minValue, int maxValue, int direction);
/**
 * Shifts value of 'var' by 2 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
extern void RD_Menu_ShiftSlideInt(int* var, int minValue, int maxValue, int direction);

#endif //RD_MENU_H

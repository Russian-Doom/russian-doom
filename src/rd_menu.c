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

#include "rd_menu.h"

/**
 * Increments or decrements 'var' depending on 'direction'. LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of var exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
inline void RD_Menu_SpinInt(int* var, int minValue, int maxValue, int direction)
{
    RD_Menu_SpinInt_Step(var, minValue, maxValue, 1, direction);
}

/**
 * Increments or decrements 'var' depending on 'direction' by 'step'. LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of var exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
inline void RD_Menu_SpinInt_Step(int* var, int minValue, int maxValue, int step, int direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            (*var) -= step;
            if (*var < minValue)
                *var = maxValue;
            break;
        case RIGHT_DIR:
            (*var) += step;
            if (*var > maxValue)
                *var = minValue;
        default:
            break;
    }
}

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
inline void RD_Menu_SlideInt(int* var, int minValue, int maxValue, int direction)
{
    RD_Menu_SlideInt_Step(var, minValue, maxValue, 1, direction);
}

/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
inline void RD_Menu_SlideInt_Step(int* var, int minValue, int maxValue, int step, int direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
                (*var) -= step;
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
                (*var) += step;
        default:
            break;
    }
}

/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
inline void RD_Menu_SlideFloat_Step(float* var, float minValue, float maxValue, float step, int direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
                (*var) -= step;
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
                (*var) += step;
        default:
            break;
    }
}

/**
 * Shifts value of 'var' by 1 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
inline void RD_Menu_ShiftSpinInt(int* var, int minValue, int maxValue, int direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            (*var) >>= 1;
            if (*var < minValue)
                *var = maxValue;
            break;
        case RIGHT_DIR:
            (*var) <<= 1;
            if (*var > maxValue)
                *var = minValue;
        default:
            break;
    }
}

/**
 * Shifts value of 'var' by 1 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * Value can not exit range specified by 'minValue' and 'maxValue'
 */
inline void RD_Menu_ShiftSlideInt(int* var, int minValue, int maxValue, int direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
                (*var) >>= 1;
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
                (*var) <<= 1;
        default:
            break;
    }
}

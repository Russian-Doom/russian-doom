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

#include "i_video.h"
#include "jn.h"
#include "v_patch.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

static lumpindex_t bigSlider_left_patch;
static lumpindex_t bigSlider_middle1_patch;
static lumpindex_t bigSlider_middle2_patch;
static lumpindex_t bigSlider_right_patch;
static lumpindex_t bigSlider_gem_patch;

static lumpindex_t smallSlider_left_patch;
static lumpindex_t smallSlider_middle_patch;
static lumpindex_t smallSlider_right_patch;
static lumpindex_t smallSlider_gem_patch;

static Translation_CR_t gem_normal_translation;
static Translation_CR_t gem_zero_translation;
static Translation_CR_t gem_max_translation;

extern void (*drawShadowedPatch)(int x, int y, patch_t *patch);

void RD_Menu_InitSliders(char* BigSlider_left_patch,
                         char* BigSlider_middle1_patch,
                         char* BigSlider_middle2_patch,
                         char* BigSlider_right_patch,
                         char* BigSlider_gem_patch,
                         char* SmallSlider_left_patch,
                         char* SmallSlider_middle_patch,
                         char* SmallSlider_right_patch,
                         char* SmallSlider_gem_patch,
                         Translation_CR_t Gem_normal_translation,
                         Translation_CR_t Gem_zero_translation,
                         Translation_CR_t Gem_max_translation)
{
    bigSlider_left_patch = W_GetNumForName(BigSlider_left_patch);
    bigSlider_middle1_patch = W_GetNumForName(BigSlider_middle1_patch);
    if (BigSlider_middle2_patch)
        bigSlider_middle2_patch = W_GetNumForName(BigSlider_middle2_patch);
    else
        bigSlider_middle2_patch = -1;
    bigSlider_right_patch = W_GetNumForName(BigSlider_right_patch);
    bigSlider_gem_patch = W_GetNumForName(BigSlider_gem_patch);

    smallSlider_left_patch = W_GetNumForName(SmallSlider_left_patch);
    smallSlider_middle_patch = W_GetNumForName(SmallSlider_middle_patch);
    smallSlider_right_patch = W_GetNumForName(SmallSlider_right_patch);
    smallSlider_gem_patch = W_GetNumForName(SmallSlider_gem_patch);

    gem_normal_translation = Gem_normal_translation;
    gem_zero_translation = Gem_zero_translation;
    gem_max_translation = Gem_max_translation;
}

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

/** [Dasperal] y = menu->y + 2 + (item * ITEM_HEIGHT) */
void RD_Menu_DrawSlider(Menu_t * menu, int y, int width, int value)
{
    int x;
    int x2;
    int count;

    x = (english_language ? menu->x_eng : menu->x_rus) + 24;
    V_DrawPatch(x - 32 + wide_delta, y, W_CacheLumpNum(bigSlider_left_patch, PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawPatch(x2 + wide_delta, y,
                    W_CacheLumpNum(bigSlider_middle2_patch == -1 || (count & 1) ?
                                   bigSlider_middle1_patch :
                                   bigSlider_middle2_patch, PU_CACHE));
    }
    V_DrawPatch(x2 + wide_delta, y, W_CacheLumpNum(bigSlider_right_patch, PU_CACHE));

    if (value > width)
        value = width;

    V_DrawPatch(x + 4 + value * 8 + wide_delta, y + 7, W_CacheLumpNum(bigSlider_gem_patch, PU_CACHE));
}

/** [JN] Draw small slider*/
void RD_Menu_DrawSliderSmall(Menu_t * menu, int y, int width, int value)
{
    int x;
    int x2;
    int count;

    x = (english_language ? menu->x_eng : menu->x_rus) + 24;

    drawShadowedPatch(x - 32 + wide_delta, y, W_CacheLumpNum(smallSlider_left_patch, PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        drawShadowedPatch(x2 - 16 + wide_delta, y, W_CacheLumpNum(smallSlider_middle_patch, PU_CACHE));
    }
    drawShadowedPatch(x2 - 25 + wide_delta, y, W_CacheLumpNum(smallSlider_right_patch, PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (value == 0)
        dp_translation = gem_zero_translation == CR_NONE ? NULL : cr[gem_zero_translation];
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if (value > width)
    {
        value = width;
        dp_translation = gem_max_translation == CR_NONE ? NULL : cr[gem_max_translation];
    }
    // [JN] Standard function (green gem)
    else
        dp_translation = gem_normal_translation == CR_NONE ? NULL : cr[gem_normal_translation];

    V_DrawPatch(x + value * 8 + wide_delta, y + 7, W_CacheLumpNum(smallSlider_gem_patch, PU_CACHE));
    dp_translation = NULL;
}

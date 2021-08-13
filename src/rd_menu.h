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

#include "d_event.h"
#include "rd_text.h"
#include "rd_menu_control.h"

typedef enum
{
    LEFT_DIR,
    RIGHT_DIR
} Direction_t;

typedef enum
{
    MENU_SOUND_CURSOR_MOVE,
    MENU_SOUND_SLIDER_MOVE,
    MENU_SOUND_CLICK,
    MENU_SOUND_ACTIVATE,
    MENU_SOUND_DEACTIVATE,
    MENU_SOUND_BACK,
    MENU_SOUND_PAGE
} MenuSound_t;

typedef enum
{
    /** Empty menu item.
     *  Fields 'text_eng', 'text_rus' and 'pointer' must be NULL.
     *  Field 'option' must be 0
     */
    ITT_EMPTY,
    /** Subheader menu item. Can not be selected or clicked. Rendered in gold color.
     *  Field 'pointer' must be NULL.
     *  Field 'option' must be 0
     */
    ITT_TITLE,
    /** Clickable menu item.
     *  The type of field 'pointer' is void (*) (int option).
     *  Field 'option' is an argument for the function specified by 'pointer'
     */
    ITT_EFUNC,
    /** Slider or Spinner menu item. Sound handled in associated function.
     *  The type of field 'pointer' is void (*) (Direction_t direction).
     *  Field 'option' is not used and should be 0
     */
    ITT_LRFUNC,
    /** A menu item that represents a switch that can be activated with Use, Left, and Right keys.
     *  Plays MENU_SOUND_SLIDER_MOVE sound when clicked.
     *  The type of field 'pointer' is void (*) (void).
     *  Field 'option' is not used and should be 0
     */
    ITT_SWITCH,
    /** A menu item that changes the current menu to one specified by 'pointer'.
     *  The type of field 'pointer' is Menu_t*.
     *  Field 'option' is not used and should be 0
     */
    ITT_SETMENU,
    /** A menu item that changes the current menu to one specified by 'pointer'.
     *  In net game shows an error instead of a menu change.
     *  The type of field 'pointer' is Menu_t*.
     *  Field 'option' is the type of error
     */
    ITT_SETMENU_NONET
} ItemType_t;

typedef struct
{
    const ItemType_t type;
    const char* const text_eng;
    const char* const text_rus;
    /** Actual type and meaning of this field is determent by 'type' field. @see ItemType_t */
    void* pointer;
    /** Actual meaning of this field is determent by 'type' field. @see ItemType_t */
    const int option;
} MenuItem_t;

typedef struct
{
    const int pageCount;
    const struct Menu_s** const pagesArray;
    const int pageNumber_x;
    const int pageNumber_y;
    const Translation_CR_t translation;
} PageDescriptor_t;

typedef struct Menu_s
{
    int x_eng; //
    int x_rus; // [Dasperal] Those fields should be const but some menus of Doom dynamically change them.
    int y;     //
    const char* const title_eng;
    const char* const title_rus;
    const boolean replaceableBigFont;
    const int itemCount;
    const MenuItem_t* const items;
    const boolean bigFont;
    void (*drawFunc) (void);
    /** If the menu supports PageUp and PageDn this field must point to a pagination descriptor struct
     *  otherwise, this field must be NULL.
     */
    const PageDescriptor_t* const pageDescriptor;
    const struct Menu_s* prevMenu;

    /** The initial value of this field must be the index of the first clickable menu item */
    int lastOn;
} Menu_t;

extern Menu_t* MainMenu;
extern Menu_t* CurrentMenu;
extern int CurrentItPos;
extern int MenuTime;

void RD_Menu_InitMenu(int Item_Height, int Item_Height_Small,
                      void (*OnActivateMenu)(void), void (*OnDeactivateMenu)(void));

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
                         Translation_CR_t Gem_max_translation);

void RD_Menu_InitCursor(char* BigCursor1_patch,
                        char* BigCursor2_patch,
                        char* SmallCursor1_patch,
                        char* SmallCursor2_patch,
                        int Cursor_Y_Offset,
                        int Cursor_Y_Offset_Small,
                        int Cursor_X_Offset,
                        int Cursor_X_Offset_Small);

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_SpinInt(int* var, int minValue, int maxValue, Direction_t direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_SpinInt_Step(int* var, int minValue, int maxValue, int step, Direction_t direction);

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
extern void RD_Menu_SlideInt(int* var, int minValue, int maxValue, Direction_t direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
extern void RD_Menu_SlideInt_Step(int* var, int minValue, int maxValue, int step, Direction_t direction);
/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
extern void RD_Menu_SlideFloat_Step(float* var, float minValue, float maxValue, float step, Direction_t direction);

/**
 * Shifts value of 'var' by 2 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range
 */
extern void RD_Menu_ShiftSpinInt(int* var, int minValue, int maxValue, Direction_t direction);
/**
 * Shifts value of 'var' by 2 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
extern void RD_Menu_ShiftSlideInt(int* var, int minValue, int maxValue, Direction_t direction);

/** [Dasperal] y = menu->y + 2 + (item * ITEM_HEIGHT) */
void RD_Menu_DrawSlider(Menu_t * menu, int y, int width, int value);
/** [JN] Draw small slider*/
void RD_Menu_DrawSliderSmall(Menu_t * menu, int y, int width, int value);

void RD_Menu_Draw_Bindings(int x);

void RD_Menu_DrawMenu(Menu_t* menu, int menuTime, int currentItPos);

boolean RD_Menu_Responder(event_t* event);

void RD_Menu_SetMenu(const Menu_t* menu);

extern boolean SCNetCheck(int option);
extern void RD_Menu_StartSound(MenuSound_t sound);

#endif //RD_MENU_H

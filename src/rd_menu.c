//
// Copyright(C) 2021-2022 Dasperal
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

#include <ctype.h>
#include "rd_menu.h"

#include "deh_str.h"
#include "d_name.h"
#include "doomkeys.h"
#include "i_video.h"
#include "jn.h"
#include "m_misc.h"
#include "rd_keybinds.h"
#include "v_patch.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

static int item_Height;
static int item_Height_Small;
static int cursor_Y_Offset;
static int cursor_Y_Offset_Small;
static int cursor_X_Offset;
static int cursor_X_Offset_Small;

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

static lumpindex_t bigCursor1_patch;
static lumpindex_t bigCursor2_patch;

static lumpindex_t smallCursor1_patch;
static lumpindex_t smallCursor2_patch;

static void (*onActivateMenu)(void);
static void (*onDeactivateMenu)(void);

boolean menuactive;
Menu_t* MainMenu;
Menu_t* CurrentMenu;
int CurrentItPos;
int MenuTime;

extern void (*drawShadowedPatch)(int x, int y, patch_t *patch);

void RD_Menu_InitMenu(int Item_Height, int Item_Height_Small,
                      void (*OnActivateMenu)(void), void (*OnDeactivateMenu)(void))
{
    item_Height = Item_Height;
    item_Height_Small = Item_Height_Small;
    onActivateMenu = OnActivateMenu;
    onDeactivateMenu = OnDeactivateMenu;
}

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

void RD_Menu_InitCursor(char* BigCursor1_patch,
                        char* BigCursor2_patch,
                        char* SmallCursor1_patch,
                        char* SmallCursor2_patch,
                        int Cursor_Y_Offset,
                        int Cursor_Y_Offset_Small,
                        int Cursor_X_Offset,
                        int Cursor_X_Offset_Small)
{
    bigCursor1_patch = W_GetNumForName(BigCursor1_patch);
    bigCursor2_patch = W_GetNumForName(BigCursor2_patch);

    smallCursor1_patch = W_GetNumForName(SmallCursor1_patch);
    smallCursor2_patch = W_GetNumForName(SmallCursor2_patch);

    cursor_Y_Offset = Cursor_Y_Offset;
    cursor_Y_Offset_Small = Cursor_Y_Offset_Small;
    cursor_X_Offset = Cursor_X_Offset;
    cursor_X_Offset_Small = Cursor_X_Offset_Small;
}

/**
 * Increments or decrements 'var' depending on 'direction'. LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of var exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range.
 * Plays MENU_SOUND_SLIDER_MOVE sound
 */
inline void RD_Menu_SpinInt(int* var, int minValue, int maxValue, Direction_t direction)
{
    RD_Menu_SpinInt_Step(var, minValue, maxValue, 1, direction);
}

/**
 * Increments or decrements 'var' depending on 'direction' by 'step'. LEFT_DIR = decrement, RIGHT_DIR = increment.
 * If value of var exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range.
 * Plays MENU_SOUND_SLIDER_MOVE sound
 */
inline void RD_Menu_SpinInt_Step(int* var, int minValue, int maxValue, int step, Direction_t direction)
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
            break;
        default:
            return;
    }
    RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
}

/**
 * Increments or decrements 'var' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
inline void RD_Menu_SlideInt(int* var, int minValue, int maxValue, Direction_t direction)
{
    RD_Menu_SlideInt_Step(var, minValue, maxValue, 1, direction);
}

/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
inline void RD_Menu_SlideInt_Step(int* var, int minValue, int maxValue, int step, Direction_t direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
            {
                (*var) -= step;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
            {
                (*var) += step;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
        default:
            break;
    }
}

/**
 * Increments or decrements 'var' by 'step' depending on 'direction', LEFT_DIR = decrement, RIGHT_DIR = increment.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
inline void RD_Menu_SlideFloat_Step(float* var, float minValue, float maxValue, float step, Direction_t direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
            {
                (*var) -= step;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
            {
                (*var) += step;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
        default:
            break;
    }
}

/**
 * Shifts value of 'var' by 1 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * If value of 'var' exits range specified by 'minValue' and 'maxValue' then it will wrap to other end of the range.
 * Plays MENU_SOUND_SLIDER_MOVE sound
 */
inline void RD_Menu_ShiftSpinInt(int* var, int minValue, int maxValue, Direction_t direction)
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
            break;
        default:
            return;
    }
    RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
}

/**
 * Shifts value of 'var' by 1 in 'direction', LEFT_DIR = left, RIGHT_DIR = right.
 * Value can not exit range specified by 'minValue' and 'maxValue'.
 * Plays MENU_SOUND_SLIDER_MOVE sound if value actually changed
 */
inline void RD_Menu_ShiftSlideInt(int* var, int minValue, int maxValue, Direction_t direction)
{
    switch (direction)
    {
        case LEFT_DIR:
            if (*var > minValue)
            {
                (*var) >>= 1;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
            break;
        case RIGHT_DIR:
            if (*var < maxValue)
            {
                (*var) <<= 1;
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
        default:
            break;
    }
}

/** [Dasperal] y = menu->y + 2 + (item * ITEM_HEIGHT) */
void RD_Menu_DrawSlider(Menu_t* menu, int y, int width, int value)
{
    int x;
    int x2;
    int count;

    x = (english_language ? menu->x_eng : menu->x_rus) + (RD_GameType == gt_Doom ? 8 : 24);
    V_DrawPatch(x - (RD_GameType == gt_Doom ? 8 : 32) + wide_delta, y, W_CacheLumpNum(bigSlider_left_patch, PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawPatch(x2 + wide_delta, y,
                    W_CacheLumpNum(bigSlider_middle2_patch == -1 || (count & 1) ?
                                   bigSlider_middle1_patch :
                                   bigSlider_middle2_patch, PU_CACHE));
    }
    V_DrawPatch(x2 + wide_delta, y, W_CacheLumpNum(bigSlider_right_patch, PU_CACHE));

    if (value > width - 1)
        value = width - 1;

    V_DrawPatch(x + (RD_GameType == gt_Doom ? 0 : 4) + value * 8 + wide_delta, y + (RD_GameType == gt_Doom ? 0 : 7), W_CacheLumpNum(bigSlider_gem_patch, PU_CACHE));
}

/** [JN] Draw small slider*/
void RD_Menu_DrawSliderSmall(Menu_t* menu, int y, int width, int value)
{
    RD_Menu_DrawSliderSmallInline(english_language ? menu->x_eng : menu->x_rus, y, width, value);
}

void RD_Menu_DrawSliderSmallInline(int x, int y, int width, int value)
{
    int x2;
    int count;

    x += (RD_GameType == gt_Doom ? 8 : 24);

    drawShadowedPatch(x - (RD_GameType == gt_Doom ? 8 : 32) + wide_delta, y, W_CacheLumpNum(smallSlider_left_patch, PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        drawShadowedPatch(x2 - (RD_GameType == gt_Doom ? 0 : 16) + wide_delta, y, W_CacheLumpNum(smallSlider_middle_patch, PU_CACHE));
    }
    drawShadowedPatch(x2 - (RD_GameType == gt_Doom ? 0 : 25) + wide_delta, y, W_CacheLumpNum(smallSlider_right_patch, PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (value == 0)
        dp_translation = gem_zero_translation == CR_NONE ? NULL : cr[gem_zero_translation];
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if (value > width - 1)
    {
        value = width - 1;
        dp_translation = gem_max_translation == CR_NONE ? NULL : cr[gem_max_translation];
    }
    // [JN] Standard function (green gem)
    else
        dp_translation = gem_normal_translation == CR_NONE ? NULL : cr[gem_normal_translation];

    V_DrawPatch(x + value * 8 + wide_delta, y + (RD_GameType == gt_Doom ? 0 : 7), W_CacheLumpNum(smallSlider_gem_patch, PU_CACHE));
    dp_translation = NULL;
}

void RD_Menu_DrawMenu(Menu_t* menu, int menuTime, int currentItPos)
{
    int i;
    int x;
    int y;
    const MenuItem_t *item;
    Translation_CR_t subheaderTranslation;

    if (menu->drawFunc != NULL)
    {
        menu->drawFunc();
    }
    x = english_language ? menu->x_eng : menu->x_rus;
    y = menu->y;
    subheaderTranslation =
            RD_GameType == gt_Doom    ? CR_YELLOW :
            RD_GameType == gt_Heretic ? CR_WHITE2DARKGOLD_HERETIC :
            RD_GameType == gt_Hexen   ? CR_GRAY2DARKGOLD_HEXEN :
          /*RD_GameType == gt_Strife*/  CR_GOLD2GRAY_STRIFE;

    if(english_language)
    {
        if(menu->title_eng)
        {
            char* title_eng = (char *) menu->title_eng;
            if(menu->replaceableBigFont)
            {
                RD_M_DrawTextB(title_eng, 160 - RD_M_TextBWidth(title_eng) / 2 + wide_delta, 4);
            }
            else
            {
                RD_M_DrawTextBigENG(title_eng, 160 - RD_M_TextBigENGWidth(title_eng) / 2 + wide_delta, 4);
            }
        }
    }
    else
    {
        if(menu->title_rus)
        {
            char* title_rus = (char *) menu->title_rus;
            RD_M_DrawTextBigRUS(title_rus, 160 - RD_M_TextBigRUSWidth(title_rus) / 2 + wide_delta, 4);
        }
    }

    item = menu->items;
    for (i = 0; i < menu->itemCount; i++)
    {
        if (item->type != ITT_EMPTY && (english_language ? item->text_eng : item->text_rus))
        {
            // [JN] Define where to use big and where small fonts,
            // and where to use big or small vertical spacing.
            if (english_language)
            {
                if (menu->bigFont)
                {
                    if(menu->replaceableBigFont)
                    {
                        if(RD_GameType == gt_Doom)
                        {
                            char* text = (char *) item->text_eng;
                            text++;
                            drawShadowedPatch(x + wide_delta, y, W_CacheLumpName(DEH_String(text), PU_CACHE));
                        }
                        else
                            RD_M_DrawTextB((char *) item->text_eng, x + wide_delta, y);
                    }
                    else
                    {
                        RD_M_DrawTextBigENG((char *) item->text_eng, x + wide_delta, y);
                    }
                }
                else
                {
                    RD_M_DrawTextSmallENG((char *) item->text_eng, x + wide_delta, y,
                                          item->type == ITT_TITLE ? subheaderTranslation : CR_NONE);
                }
            }
            else
            {
                if (menu->bigFont)
                {
                    if(RD_GameType == gt_Doom && menu->replaceableBigFont)
                    {
                        char* text = (char *) item->text_rus;
                        text++;
                        drawShadowedPatch(x + wide_delta, y, W_CacheLumpName(DEH_String(text), PU_CACHE));
                    }
                    else
                        RD_M_DrawTextBigRUS((char *) item->text_rus, x + wide_delta, y);
                }
                else
                {
                    RD_M_DrawTextSmallRUS((char *) item->text_rus, x + wide_delta, y,
                                          item->type == ITT_TITLE ? subheaderTranslation : CR_NONE);
                }
            }
        }

        if (menu->bigFont)
        {
            y += item_Height;
        }
        else
        {
            y += item_Height_Small;
        }

        item++;
    }

    if(CurrentMenu->pageDescriptor != NULL)
    {
        char string[8];
        const PageDescriptor_t* descriptor = CurrentMenu->pageDescriptor;

        for(i = 0; i < descriptor->pageCount; i++)
            if(descriptor->pagesArray[i] == CurrentMenu)
                break;

        M_snprintf(string, 8, "(%d/%d)", i + 1, descriptor->pageCount);
        RD_M_DrawTextSmallENG(string, descriptor->pageNumber_x + wide_delta,
                              descriptor->pageNumber_y, descriptor->translation);
    }

    if (menu->bigFont)
    {
        y = menu->y + (currentItPos * item_Height) + cursor_Y_Offset;
        drawShadowedPatch(x + cursor_X_Offset + wide_delta, y,
            W_CacheLumpNum(menuTime & (RD_GameType == gt_Doom ? 8 : 16) ? 
                           bigCursor1_patch : bigCursor2_patch, PU_CACHE));
    }
    else
    {
        y = menu->y + (currentItPos * item_Height_Small) + cursor_Y_Offset_Small;
        drawShadowedPatch(x + cursor_X_Offset_Small + wide_delta, y,
            W_CacheLumpNum(menuTime & (RD_GameType == gt_Doom ? 8 : 16) ?
                           smallCursor1_patch : smallCursor2_patch, PU_CACHE));
    }
}

boolean RD_Menu_Responder(event_t* event)
{
    int i;
    MenuItem_t *item;

    item = (MenuItem_t *) &CurrentMenu->items[CurrentItPos];

    if (BK_isKeyDown(event, bk_down))            // Next menu item
    {
        do
        {
            if (CurrentItPos + 1 > CurrentMenu->itemCount - 1)
            {
                CurrentItPos = 0;
            }
            else
            {
                CurrentItPos++;
            }
        } while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY ||
                 CurrentMenu->items[CurrentItPos].type == ITT_TITLE);
        RD_Menu_StartSound(MENU_SOUND_CURSOR_MOVE);
        return true;
    }
    else if (BK_isKeyDown(event, bk_up))         // Previous menu item
    {
        do
        {
            if (CurrentItPos == 0)
            {
                CurrentItPos = CurrentMenu->itemCount - 1;
            }
            else
            {
                CurrentItPos--;
            }
        } while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY ||
                 CurrentMenu->items[CurrentItPos].type == ITT_TITLE);
        RD_Menu_StartSound(MENU_SOUND_CURSOR_MOVE);
        return true;
    }
    else if (BK_isKeyDown(event, bk_left))       // Slider left
    {
        if (item->type == ITT_LRFUNC && item->pointer != NULL)
        {
            ((void (*)(Direction_t)) item->pointer)(LEFT_DIR);
        }
        else if (item->type == ITT_SWITCH && item->pointer != NULL)
        {
            ((void (*)(void)) item->pointer)();
            RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
        }
        return true;
    }
    else if (BK_isKeyDown(event, bk_right))      // Slider right
    {
        if (item->type == ITT_LRFUNC && item->pointer != NULL)
        {
            ((void (*)(Direction_t)) item->pointer)(RIGHT_DIR);
        }
        else if (item->type == ITT_SWITCH && item->pointer != NULL)
        {
            ((void (*)(void)) item->pointer)();
            RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
        }
        return true;
    }
    else if (BK_isKeyDown(event, bk_menu_select))    // Activate item (enter)
    {
        if (item->type == ITT_SETMENU)
        {
            RD_Menu_SetMenu((Menu_t *) item->pointer);
            RD_Menu_StartSound(MENU_SOUND_CLICK);
        }
        if (item->type == ITT_SETMENU_NONET)
        {
            if (SCNetCheck(item->option))
            {
                RD_Menu_SetMenu((Menu_t *) item->pointer);
                RD_Menu_StartSound(MENU_SOUND_CLICK);
            }
        }
        else if (item->pointer != NULL)
        {
            CurrentMenu->lastOn = CurrentItPos;
            if (item->type == ITT_LRFUNC)
            {
                ((void (*)(Direction_t)) item->pointer)(RIGHT_DIR);
            }
            else if (item->type == ITT_EFUNC)
            {
                ((void (*)(int)) item->pointer)(item->option);
                RD_Menu_StartSound(MENU_SOUND_CLICK);
            }
            else if (item->type == ITT_SWITCH)
            {
                ((void (*)(void)) item->pointer)();
                RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            }
        }
        return true;
    }
    else if (BK_isKeyDown(event, bk_menu_activate))     // Toggle menu
    {
        RD_Menu_DeactivateMenu();
        return true;
    }
    else if (BK_isKeyDown(event, bk_menu_back))         // Go back to previous menu
    {
        RD_Menu_StartSound(MENU_SOUND_BACK);
        if (CurrentMenu->prevMenu == NULL)
        {
            RD_Menu_DeactivateMenu();
        }
        else
        {
            RD_Menu_SetMenu(CurrentMenu->prevMenu);
        }
        return true;
    }
    // [JN] Scroll menus by PgUp/PgDn keys
    else if (BK_isKeyDown(event, bk_menu_page_prev))
    {
        if (CurrentMenu->pageDescriptor != NULL)
        {
            int j;
            const PageDescriptor_t* descriptor = CurrentMenu->pageDescriptor;
            for (j = 0; j < descriptor->pageCount; ++j)
            {
                if(descriptor->pagesArray[j] == CurrentMenu)
                {
                    j--;
                    if(j < 0)
                        j = descriptor->pageCount - 1;
                    break;
                }
            }

            RD_Menu_SetMenu(descriptor->pagesArray[j]);
            RD_Menu_StartSound(MENU_SOUND_PAGE);
            return true;
        }
    }
    else if (BK_isKeyDown(event, bk_menu_page_next))
    {
        if (CurrentMenu->pageDescriptor != NULL)
        {
            int j;
            const PageDescriptor_t* descriptor = CurrentMenu->pageDescriptor;
            for (j = 0; j < descriptor->pageCount; ++j)
            {
                if(descriptor->pagesArray[j] == CurrentMenu)
                {
                    j++;
                    if(j >= descriptor->pageCount)
                        j = 0;
                    break;
                }
            }

            RD_Menu_SetMenu(descriptor->pagesArray[j]);
            RD_Menu_StartSound(MENU_SOUND_PAGE);
            return true;
        }
    }
    else if (event->type == ev_keydown && event->data2 != 0)
    {
        // Jump to menu item based on first letter:

        for (i = CurrentItPos + 1; i < CurrentMenu->itemCount; i++)
        {
            if (CurrentMenu->items[i].type != ITT_TITLE && CurrentMenu->items[i].type != ITT_EMPTY &&
                (english_language ? CurrentMenu->items[i].text_eng : CurrentMenu->items[i].text_rus))
            {
                const char *textString = english_language ? CurrentMenu->items[i].text_eng
                                                          : CurrentMenu->items[i].text_rus;
                if (textString)
                {
                    if (toupper(event->data2) == toupper(textString[0]))
                    {
                        CurrentItPos = i;
                        return true;
                    }
                }
            }
        }

        for (i = 0; i <= CurrentItPos; i++)
        {
            if (CurrentMenu->items[i].type != ITT_TITLE && CurrentMenu->items[i].type != ITT_EMPTY &&
                (english_language ? CurrentMenu->items[i].text_eng : CurrentMenu->items[i].text_rus))
            {
                const char *textString = english_language ? CurrentMenu->items[i].text_eng
                                                          : CurrentMenu->items[i].text_rus;
                if (textString)
                {
                    if (toupper(event->data2) == toupper(textString[0]))
                    {
                        CurrentItPos = i;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void RD_Menu_SetMenu(const Menu_t* menu)
{
    CurrentMenu->lastOn = CurrentItPos;
    CurrentMenu = (Menu_t*) menu;
    CurrentItPos = CurrentMenu->lastOn;
}

void RD_Menu_ActivateMenu(void)
{
    if (menuactive)
    {
        return;
    }
    menuactive = true;
    MenuTime = 0;

    CurrentMenu = MainMenu;
    CurrentItPos = CurrentMenu->lastOn;

    RD_Menu_StartSound(MENU_SOUND_ACTIVATE);

    if(onActivateMenu)
        onActivateMenu();
}

void RD_Menu_DeactivateMenu(void)
{
    if (CurrentMenu != NULL)
    {
        CurrentMenu->lastOn = CurrentItPos;
    }
    menuactive = false;

    RD_Menu_StartSound(MENU_SOUND_DEACTIVATE);

    if(onDeactivateMenu)
        onDeactivateMenu();
}

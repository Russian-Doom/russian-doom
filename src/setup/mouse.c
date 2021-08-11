//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
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

#include "textscreen.h"
#include "doomtype.h"
#include "m_config.h"

#include "execute.h"

#include "mode.h"
#include "mouse.h"

#define WINDOW_HELP_URL "https://jnechaevsky.github.io/projects/rusdoom/setup/index.html"

static int usemouse = 1;

static int mouseSensitivity = 5;
static float mouse_acceleration = 2.0F;
static int mouse_threshold = 10;
static int mlook = 0;
static int grabmouse = 1;

int novert = 1; // [JN] ¬ертикальное перемещение отключено по умолчанию.
int mouse_y_invert = 0; // [crispy]

void ConfigMouse(void)
{
    txt_window_t *window;

    window = TXT_NewWindow(english_language ?
                           "Mouse configuration" :
                           "Настройки мыши");

    TXT_SetTableColumns(window, 2);

    //
    // [JN] Create translated buttons
    //

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, english_language ?
                        TXT_NewWindowAbortAction(window) :
                        TXT_NewWindowAbortAction_Rus(window));
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, english_language ?
                        TXT_NewWindowSelectAction(window) :
                        TXT_NewWindowSelectAction_Rus(window));

    if (english_language)
        TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);
    else
        TXT_SetWindowHelpURL_RUS(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
        TXT_NewCheckBox(english_language ?
                        "Enable mouse" :
                        "Разрешить использование мыши",
                        &usemouse),
        TXT_TABLE_OVERFLOW_RIGHT,
            TXT_NewCheckBox(english_language ?
                            "Allow mouse look" :
                            "Обзор мышью",
                            &mlook),
        TXT_TABLE_OVERFLOW_RIGHT,
            TXT_NewCheckBox(english_language ?
                            "Invert Y axis" :
                            "Вертикальна€ инверси€",
                            &mouse_y_invert),
        TXT_TABLE_OVERFLOW_RIGHT,
            TXT_NewInvertedCheckBox(english_language ?
                                    "Allow vertical mouse movement" :
                                    "Вертикальное перемещение",
                                    &novert),
        TXT_TABLE_OVERFLOW_RIGHT,
        TXT_NewCheckBox(english_language ?
                        "Grab mouse in windowed mode" :
                        "Захват мыши в оконном режиме",
                        &grabmouse),
        TXT_TABLE_OVERFLOW_RIGHT,

        TXT_NewSeparator(english_language ?
                         "Mouse motion" :
                         "Настройка перемещени€"),
        TXT_NewLabel(english_language ?
                     "Speed" :
                     "Скорость"),
        TXT_NewSpinControl(&mouseSensitivity, 1, 256),
        TXT_NewLabel(english_language ?
                     "Acceleration" :
                     "Акселераци€"),
        TXT_NewFloatSpinControl(&mouse_acceleration, 1.0F, 5.0F),
        TXT_NewLabel(english_language ?
                     "Acceleration threshold" :
                     "Порог акселерации"),
        TXT_NewSpinControl(&mouse_threshold, 0, 32), NULL);
}

void BindMouseVariables(void)
{
    M_BindIntVariable("use_mouse",               &usemouse);
    M_BindIntVariable("mlook",                   &mlook);
    M_BindIntVariable("mouse_sensitivity",       &mouseSensitivity);
    M_BindIntVariable("novert",                  &novert);
    M_BindIntVariable("mouse_y_invert",          &mouse_y_invert);
    M_BindIntVariable("grabmouse",               &grabmouse);
    M_BindIntVariable("mouse_threshold",         &mouse_threshold);
    M_BindFloatVariable("mouse_acceleration",    &mouse_acceleration);
}


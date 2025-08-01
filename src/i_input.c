//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2025 Leonid Murin (Dasperal)
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
// DESCRIPTION:
//     SDL implementation of system-specific input interface.
//



#include "SDL.h"
#include "SDL_keycode.h"

#include "doomkeys.h"
#include "doomtype.h"
#include "d_event.h"
#include "i_input.h"
#include "i_timer.h" // [crispy]
#include "m_argv.h"
#include "m_config.h"
#include "m_fixed.h" // [crispy]
#include "jn.h"

static const int scancode_translate_table[] = SCANCODE_TO_KEYS_ARRAY;

// Lookup table for mapping ASCII characters to their equivalent when
// shift is pressed on a US layout keyboard. This is the original table
// as found in the Doom sources, comments and all.
static const char shiftxform[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

// If true, I_StartTextInput() has been called, and we are populating
// the data3 field of ev_keydown events.
static boolean text_input_enabled = true;

// Bit mask of mouse button state.
static unsigned int mouse_button_state = 0;

// Disallow mouse and joystick movement to cause forward/backward
// motion.  Specified with the '-novert' command line parameter.
// This is an int to allow saving to config file

// [JN] Вертикальное перемещение отключено по умолчанию.
int novert = 1;

// [Dasperal] Mouselook: Enabled by default.
int mlook = true;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.
float mouse_acceleration = 2.0;
int mouse_threshold = 10;
// [crispy]
int mouse_y_invert = 0;

// Translates the SDL key to a value of the type found in doomkeys.h
static int TranslateKey(SDL_Keysym *sym)
{
    int scancode = sym->scancode;

    switch (scancode)
    {
        case SDL_SCANCODE_LCTRL:
            return KEY_LCTRL;

        case SDL_SCANCODE_RCTRL:
            return KEY_RCTRL;

        case SDL_SCANCODE_LSHIFT:
            return KEY_LSHIFT;

        case SDL_SCANCODE_RSHIFT:
            return KEY_RSHIFT;

        case SDL_SCANCODE_LALT:
            return KEY_LALT;

        case SDL_SCANCODE_RALT:
            return KEY_RALT;

        default:
            if (scancode >= 0 && scancode < arrlen(scancode_translate_table))
            {
                return scancode_translate_table[scancode];
            }
            else
            {
                return 0;
            }
    }
}

// Get the localized version of the key press. This takes into account the
// keyboard layout, but does not apply any changes due to modifiers, (eg.
// shift-, alt-, etc.)
static int GetLocalizedKey(SDL_Keysym *sym)
{
    // When using Vanilla mapping, we just base everything off the scancode
    // and always pretend the user is using a US layout keyboard.
    if (vanilla_keyboard_mapping)
    {
        return TranslateKey(sym);
    }
    else
    {
        int result = sym->sym;

        switch (result)
        {
            case SDLK_KP_1:
                return SDLK_1;
            case SDLK_KP_2:
                return SDLK_2;
            case SDLK_KP_3:
                return SDLK_3;
            case SDLK_KP_4:
                return SDLK_4;
            case SDLK_KP_5:
                return SDLK_5;
            case SDLK_KP_6:
                return SDLK_6;
            case SDLK_KP_7:
                return SDLK_7;
            case SDLK_KP_8:
                return SDLK_8;
            case SDLK_KP_9:
                return SDLK_9;
            case SDLK_KP_0:
                return SDLK_0;
            case SDLK_KP_PERIOD:
                return SDLK_PERIOD;
            case SDLK_KP_DIVIDE:
                return SDLK_SLASH;
            case SDLK_KP_MULTIPLY:
                return SDLK_ASTERISK;
            case SDLK_KP_MINUS:
                return SDLK_MINUS;
            case SDLK_KP_PLUS:
                return SDLK_PLUS;
            case SDLK_KP_EQUALS:
                return SDLK_EQUALS;
            default:
                break;
        }

        if (result < 0 || result >= 128)
        {
            result = 0;
        }

        return result;
    }
}

// Get the equivalent ASCII (Unicode?) character for a keypress.
static int GetTypedChar(SDL_Keysym *sym)
{
    // We only return typed characters when entering text, after
    // I_StartTextInput() has been called. Otherwise we return nothing.
    if (!text_input_enabled)
    {
        return 0;
    }

    // If we're strictly emulating Vanilla, we should always act like
    // we're using a US layout keyboard (in ev_keydown, data1=data2).
    // Otherwise we should use the native key mapping.
    if (vanilla_keyboard_mapping)
    {
        int result = TranslateKey(sym);

        // If shift is held down, apply the original uppercase
        // translation table used under DOS.
        if ((SDL_GetModState() & KMOD_SHIFT) != 0
         && result >= 0 && result < arrlen(shiftxform))
        {
            result = shiftxform[result];
        }

        return result;
    }
    else
    {
        SDL_Event next_event;

        // Special cases, where we always return a fixed value.
        switch (sym->sym)
        {
            case SDLK_BACKSPACE: return KEY_BACKSPACE;
            case SDLK_RETURN:    return KEY_ENTER;
            default:
                break;
        }

        // The following is a gross hack, but I don't see an easier way
        // of doing this within the SDL2 API (in SDL1 it was easier).
        // We want to get the fully transformed input character associated
        // with this keypress - correct keyboard layout, appropriately
        // transformed by any modifier keys, etc. So peek ahead in the SDL
        // event queue and see if the key press is immediately followed by
        // an SDL_TEXTINPUT event. If it is, it's reasonable to assume the
        // key press and the text input are connected. Technically the SDL
        // API does not guarantee anything of the sort, but in practice this
        // is what happens and I've verified it through manual inspect of
        // the SDL source code.
        //
        // In an ideal world we'd split out ev_keydown into a separate
        // ev_textinput event, as SDL2 has done. But this doesn't work
        // (I experimented with the idea), because lots of Doom's code is
        // based around different responders "eating" events to stop them
        // being passed on to another responder. If code is listening for
        // a text input, it cannot block the corresponding keydown events
        // which can affect other responders.
        //
        // So we're stuck with this as a rather fragile alternative.

        if (SDL_PeepEvents(&next_event, 1, SDL_PEEKEVENT,
                           SDL_FIRSTEVENT, SDL_LASTEVENT) == 1
         && next_event.type == SDL_TEXTINPUT)
        {
            // If an SDL_TEXTINPUT event is found, we always assume it
            // matches the key press. The input text must be a single
            // ASCII character - if it isn't, it's possible the input
            // char is a Unicode value instead; better to send a null
            // character than the unshifted key.
            if (strlen(next_event.text.text) == 1
             && (next_event.text.text[0] & 0x80) == 0)
            {
                return next_event.text.text[0];
            }
        }

        // Failed to find anything :/
        return 0;
    }
}

void I_HandleKeyboardEvent(SDL_Event *sdlevent)
{
    // XXX: passing pointers to event for access after this function
    // has terminated is undefined behaviour
    event_t event;

    event.delayed = false;
    switch (sdlevent->type)
    {
        case SDL_KEYDOWN:
            event.type = ev_keydown;
            event.data1 = sdlevent->key.keysym.scancode;
            event.data2 = GetLocalizedKey(&sdlevent->key.keysym);
            event.data3 = GetTypedChar(&sdlevent->key.keysym);
            event.data4 = 0;

            if (event.data1 != 0)
            {
                D_PostEvent(&event);
            }
            break;

        case SDL_KEYUP:
            event.type = ev_keyup;
            event.data1 = sdlevent->key.keysym.scancode;

            // data2/data3 are initialized to zero for ev_keyup.
            // For ev_keydown it's the shifted Unicode character
            // that was typed, but if something wants to detect
            // key releases it should do so based on data1
            // (key ID), not the printable char.

            event.data2 = event.data3 = event.data4 = 0;

            if (event.data1 != 0)
            {
                D_PostEvent(&event);
            }
            break;

        default:
            break;
    }
}

void I_StartTextInput(int x1, int y1, int x2, int y2)
{
    text_input_enabled = true;

    if (!vanilla_keyboard_mapping)
    {
        // SDL2-TODO: SDL_SetTextInputRect(...);
        SDL_StartTextInput();
    }
}

void I_StopTextInput(void)
{
    text_input_enabled = false;

    if (!vanilla_keyboard_mapping)
    {
        SDL_StopTextInput();
    }
}

static void UpdateMouseButtonState(SDL_MouseButtonEvent *buttonEvent)
{
    event_t event;
    int button;

    if (buttonEvent->button < SDL_BUTTON_LEFT || buttonEvent->button > SDL_BUTTON_X2)
    {
        return;
    }

    // Note: button "0" is left, button "1" is right,
    // button "2" is middle for Doom.  This is different
    // to how SDL sees things.
    switch(buttonEvent->button)
    {
        case SDL_BUTTON_LEFT:
            button = MOUSE_LEFT;
            break;
        case SDL_BUTTON_RIGHT:
            button = MOUSE_RIGHT;
            break;
        case SDL_BUTTON_MIDDLE:
            button = MOUSE_MIDDLE;
            break;
        default:
            // SDL buttons are indexed from 1.
            button = buttonEvent->button - 1;
            break;
    }

    // Turn bit representing this button on or off
    if (buttonEvent->state)
    {
        mouse_button_state |= (1 << button);
        event.type = ev_mouse_keydown;
    }
    else
    {
        mouse_button_state &= ~(1 << button);
        event.type = ev_mouse_keyup;
    }

    // Post an event
    event.delayed = false;
    event.data1 = button;
    event.data2 = buttonEvent->x;
    event.data3 = buttonEvent->y;
    event.data4 = 0;
    D_PostEvent(&event);
}

static void MapMouseWheelToButtons(SDL_MouseWheelEvent *wheel)
{
    // SDL2 distinguishes button events from mouse wheel events.
    // We want to treat the mouse wheel as two buttons, as per
    // SDL1
    event_t up, down;
    int button;

    if(wheel->y)
    {
        button = MOUSE_SCROLL_UP + (wheel->y > 0 ? wheel->direction : !wheel->direction);

        // post a button down event
        down.type = ev_mouse_keydown;
        down.delayed = false;
        down.data1 = button;
        down.data2 = down.data3 = down.data4 = 0;
        D_PostEvent(&down);

        // post a button up event
        up.type = ev_mouse_keyup;
        up.delayed = true;
        up.data1 = button;
        up.data2 = up.data3 = up.data4 = 0;
        D_PostEvent(&up);
    }

    if(wheel->x)
    {
        button = MOUSE_SCROLL_RIGHT + (wheel->x > 0 ? wheel->direction : !wheel->direction);

        // post a button down event
        down.type = ev_mouse_keydown;
        down.delayed = false;
        down.data1 = button;
        down.data2 = down.data3 = down.data4 = 0;
        D_PostEvent(&down);

        // post a button up event
        up.type = ev_mouse_keyup;
        up.delayed = true;
        up.data1 = button;
        up.data2 = up.data3 = up.data4 = 0;
        D_PostEvent(&up);
    }
}

void I_HandleMouseEvent(SDL_Event *sdlevent)
{
    switch (sdlevent->type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            UpdateMouseButtonState(&(sdlevent->button));
            break;
        case SDL_MOUSEWHEEL:
            MapMouseWheelToButtons(&(sdlevent->wheel));
            break;
        default:
            break;
    }
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > mouse_threshold)
    {
        return (int)((val - mouse_threshold) * mouse_acceleration + mouse_threshold);
    }
    else
    {
        return val;
    }
}

// [crispy] Distribute the mouse movement between the current tic and the next
// based on how far we are into the current tic. Compensates for mouse sampling
// jitter.
static void SmoothMouse(int* x, int* y)
{
    static int x_remainder_old = 0;
    static int y_remainder_old = 0;
    int x_remainder, y_remainder;
    fixed_t correction_factor;
    fixed_t fractic;

    *x += x_remainder_old;
    *y += y_remainder_old;

    fractic = I_GetFracRealTime();
    correction_factor = FixedDiv(fractic, FRACUNIT + fractic);

    x_remainder = FixedMul(*x, correction_factor);
    *x -= x_remainder;
    x_remainder_old = x_remainder;

    y_remainder = FixedMul(*y, correction_factor);
    *y -= y_remainder;
    y_remainder_old = y_remainder;
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.
void I_ReadMouse(void)
{
    int x, y;
    event_t ev;

    SDL_GetRelativeMouseState(&x, &y);

    if (uncapped_fps)
    {
        SmoothMouse(&x, &y);
    }

    if (x != 0 || y != 0) 
    {
        ev.type = ev_mouse_move;
        ev.delayed = false;
        ev.data1 = mouse_button_state;
        ev.data2 = AccelerateMouse(x);

        // [JN] Mouselook: we need vertical mouse movement for mlook
        if (!novert || mlook)
        {
            ev.data3 = -AccelerateMouse(y);
        }
        else
        {
            ev.data3 = 0;
        }

        ev.data4 = 0;
        D_PostEvent(&ev);
    }
}

// Bind all variables controlling input options.
void I_BindInputVariables(void)
{
    M_BindFloatVariable("mouse_acceleration",      &mouse_acceleration);
    M_BindIntVariable("mouse_threshold",           &mouse_threshold);
    M_BindIntVariable("vanilla_keyboard_mapping",  &vanilla_keyboard_mapping);
    M_BindIntVariable("novert",                    &novert);
    M_BindIntVariable("mouse_y_invert",            &mouse_y_invert); // [crispy]
}

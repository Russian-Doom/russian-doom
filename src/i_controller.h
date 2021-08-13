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
// DESCRIPTION:
//      System-specific joystick interface.
//



#ifndef __I_JOYSTICK__
#define __I_JOYSTICK__

// DOOM Controller definition
enum
{
    CONTROLLER_A = 0, // SDL_CONTROLLER_BUTTON_A
    CONTROLLER_B,
    CONTROLLER_X,
    CONTROLLER_Y,
    CONTROLLER_BACK,
    CONTROLLER_GUIDE,
    CONTROLLER_START,
    CONTROLLER_LEFT_STICK,
    CONTROLLER_RIGHT_STICK,
    CONTROLLER_LEFT_SHOULDER,
    CONTROLLER_RIGHT_SHOULDER,
    CONTROLLER_DPAD_UP,
    CONTROLLER_DPAD_DOWN,
    CONTROLLER_DPAD_LEFT,
    CONTROLLER_DPAD_RIGHT,
    CONTROLLER_MISC1,
    CONTROLLER_PADDLE1,
    CONTROLLER_PADDLE2,
    CONTROLLER_PADDLE3,
    CONTROLLER_PADDLE4,
    CONTROLLER_TOUCHPAD,
    CONTROLLER_LEFT_TRIGGER, // SDL_CONTROLLER_BUTTON_MAX
    CONTROLLER_RIGHT_TRIGGER
};

typedef enum
{
    CONTROLLER_AXIS_NONE = -1,
    CONTROLLER_AXIS_MOVE = 0,
    CONTROLLER_AXIS_STRAFE,
    CONTROLLER_AXIS_TURN,
    CONTROLLER_AXIS_VLOOK,
    CONTROLLER_AXIS_MAX
} controller_axis_t;

extern int useController;
extern int invertAxis[CONTROLLER_AXIS_MAX];
extern int bindAxis[CONTROLLER_AXIS_MAX];

void I_InitController(void);
void I_ShutdownController(void);
void I_UpdateController(void);

void I_BindControllerVariables(void);

#endif /* #ifndef __I_JOYSTICK__ */


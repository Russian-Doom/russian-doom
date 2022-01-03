//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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

#include <SDL_gamecontroller.h>

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
    CONTROLLER_RIGHT_TRIGGER,
    CONTROLLER_LEFT_TRIGGER_NEGATIVE,
    CONTROLLER_RIGHT_TRIGGER_NEGATIVE,
    CONTROLLER_LSX_POSITIVE,
    CONTROLLER_LSX_NEGATIVE,
    CONTROLLER_LSY_POSITIVE,
    CONTROLLER_LSY_NEGATIVE,
    CONTROLLER_RSX_POSITIVE,
    CONTROLLER_RSX_NEGATIVE,
    CONTROLLER_RSY_POSITIVE,
    CONTROLLER_RSY_NEGATIVE
};

typedef enum
{
    CONTROLLER_AXIS_BUTTON = 0,
    CONTROLLER_AXIS_MOVE,
    CONTROLLER_AXIS_STRAFE,
    CONTROLLER_AXIS_TURN,
    CONTROLLER_AXIS_VLOOK
} controller_axis_t;

typedef struct controller_s
{
    struct controller_s* next;
    char guid[33];
    SDL_GameController* SDL_controller;
    char* name;
    SDL_JoystickID index;
    int invertAxis[SDL_CONTROLLER_AXIS_MAX];
    int bindAxis[SDL_CONTROLLER_AXIS_MAX];
    int axisDeadZone[SDL_CONTROLLER_AXIS_MAX];
    int axisSensitivity[SDL_CONTROLLER_AXIS_MAX];

    boolean axisButtonsPositiveState[SDL_CONTROLLER_AXIS_MAX];
    boolean axisButtonsNegativeState[SDL_CONTROLLER_AXIS_MAX];
} controller_t;

extern int useController;
#define ACTIVE_CONTROLLERS_SIZE 10
extern controller_t* activeControllers[ACTIVE_CONTROLLERS_SIZE];
extern controller_t* currentController;

void I_InitControllerModule(void);
void I_InitController(void);
void I_ShutdownController(void);
void I_UpdateController(void);

void I_BindControllerVariables(void);
boolean ControllerHandler_Handles(char* sectionName);
void ControllerHandler_HandleLine(char* keyName, char *value, size_t valueSize);
void ControllerHandler_Save(FILE* file, char* sectionName);
void ControllerHandler_onFinishHandling();

#endif /* #ifndef __I_JOYSTICK__ */


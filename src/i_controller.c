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
//       SDL Joystick code.
//



#include "SDL.h"
#include <stdio.h>

#include "doomtype.h"
#include "d_event.h"
#include "i_controller.h"
#include "i_system.h"
#include "m_config.h"
#include "jn.h"

// When an axis is within the dead zone, it is set to zero.
// This is 5% of the full range:

#define DEAD_ZONE (32768 / 3)

static SDL_GameController *controller = NULL;
static boolean LTriggerState = false, RTriggerState = false;

// Configuration variables:
int useController = 1;
int invertAxis[CONTROLLER_AXIS_MAX] = {0, 0, 0, 1};
int bindAxis[CONTROLLER_AXIS_MAX] = {
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY
};

void I_ShutdownController(void)
{
    if (controller != NULL)
    {
        SDL_GameControllerClose(controller);
        controller = NULL;
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }
}

void I_InitController(void)
{
    int i;

    if (!useController)
    {
        return;
    }

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
    {
        return;
    }

    // Open the GameController
    for (i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            controller = SDL_GameControllerOpen(i);
            break;
        }
    }

    if (controller == NULL)
    {
        printf(english_language ?
               "I_InitController: Failed to open joystick #%i\n" :
               "I_InitController: ошибка активизации джойстика #%i\n",
               i);
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        return;
    }

    SDL_GameControllerEventState(SDL_ENABLE);

    // Initialized okay!

    printf("I_InitController: %s\n", SDL_GameControllerName(controller));

    I_AtExit(I_ShutdownController, true);
}


static void UpdateControllerButtonState(SDL_ControllerButtonEvent* buttonEvent)
{
    event_t event;

    if(buttonEvent->button >= SDL_CONTROLLER_BUTTON_MAX)
        return;

    if (buttonEvent->state)
    {
        event.type = ev_controller_keydown;
    }
    else
    {
        event.type = ev_controller_keyup;
    }

    // Post an event
    event.data1 = buttonEvent->button;
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}

static void UpdateControllerAxisState(SDL_ControllerAxisEvent* axisEvent)
{
    event_t event;

    if(axisEvent->axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
    {
        if((LTriggerState == true && axisEvent->value > 0)
        || (LTriggerState == false && axisEvent->value == 0))
            return;

        if(LTriggerState == false && axisEvent->value > 0)
        {
            event.type = ev_controller_keydown;
            LTriggerState = true;
        }
        else
        {
            event.type = ev_controller_keyup;
            LTriggerState = false;
        }

        event.data1 = CONTROLLER_LEFT_TRIGGER;
    }
    else if(axisEvent->axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
    {
        if((RTriggerState == true && axisEvent->value > 0)
        || (RTriggerState == false && axisEvent->value == 0))
            return;

        if(RTriggerState == false && axisEvent->value > 0)
        {
            event.type = ev_controller_keydown;
            RTriggerState = true;
        }
        else
        {
            event.type = ev_controller_keyup;
            RTriggerState = false;
        }
        event.data1 = CONTROLLER_RIGHT_TRIGGER;
    }
    else
    {
        return;
    }

    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}

void I_HandleControllerEvent(SDL_Event *sdlevent)
{
    if(sdlevent->type == SDL_CONTROLLERAXISMOTION)
        UpdateControllerAxisState(&(sdlevent->caxis));
    else
        UpdateControllerButtonState(&(sdlevent->cbutton));
}

static int GetAxisState(controller_axis_t axis)
{
    int value;

    if(axis < 0)
        return 0;

    value = SDL_GameControllerGetAxis(controller, bindAxis[axis]);

    if (value < DEAD_ZONE && value > -DEAD_ZONE)
    {
        value = 0;
    }

    if(invertAxis[axis])
        value = -value;

    return value;
}

void I_UpdateController(void)
{
    if (controller != NULL)
    {
        event_t ev;

        ev.type = ev_controller_move;
        ev.data1 = GetAxisState(CONTROLLER_AXIS_MOVE);
        ev.data2 = GetAxisState(CONTROLLER_AXIS_STRAFE);
        ev.data3 = GetAxisState(CONTROLLER_AXIS_TURN);
        ev.data4 = GetAxisState(CONTROLLER_AXIS_VLOOK);

        D_PostEvent(&ev);
    }
}

void I_BindControllerVariables(void)
{
    M_BindIntVariable("use_controller",           &useController);
    M_BindIntVariable("controller_invert_move",   &invertAxis[CONTROLLER_AXIS_MOVE]);
    M_BindIntVariable("controller_invert_strafe", &invertAxis[CONTROLLER_AXIS_STRAFE]);
    M_BindIntVariable("controller_invert_turn",   &invertAxis[CONTROLLER_AXIS_TURN]);
    M_BindIntVariable("controller_invert_vlook",  &invertAxis[CONTROLLER_AXIS_VLOOK]);
    M_BindIntVariable("controller_bind_move",     &bindAxis[CONTROLLER_AXIS_MOVE]);
    M_BindIntVariable("controller_bind_strafe",   &bindAxis[CONTROLLER_AXIS_STRAFE]);
    M_BindIntVariable("controller_bind_turn",     &bindAxis[CONTROLLER_AXIS_TURN]);
    M_BindIntVariable("controller_bind_vlook",    &bindAxis[CONTROLLER_AXIS_VLOOK]);
}


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



#include <stdio.h>
#include <string.h>
#include <SDL.h>

#include "doomtype.h"
#include "d_event.h"
#include "i_controller.h"
#include "i_system.h"
#include "m_config.h"
#include "jn.h"

// When an axis is within the dead zone, it is set to zero.
// This is 5% of the full range:

#define DEAD_ZONE (32768 / 3)

static char* axesNames[] = {
    "LX",
    "LY",
    "RX",
    "RY"
};

static int nameToA[arrlen(axesNames)];
static boolean nameToA_init = false;

typedef enum
{
    AO_INVERT,
    AO_BIND
} axisOption_t;

static char* axesOptionsNames[] = {
    "Invert",
    "Binding"
};

static int nameToAO[arrlen(axesOptionsNames)];
static boolean nameToAO_init = false;

static char* axisBindsNames[] = {
    "NONE",
    "MOVE",
    "STRAFE",
    "TURN",
    "VLOOK"
};

static int nameToAB[arrlen(axisBindsNames)];
static boolean nameToAB_init = false;

controller_t* knownControllers;
controller_t* currentController;

static boolean LTriggerState = false, RTriggerState = false;

// Configuration variables:
int useController = 1;

static int nameToA_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, axesNames[*((int*) member)]);
}

static int nameToAO_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, axesOptionsNames[*((int*) member)]);
}

static int nameToAB_Comparator(const void *sample, const void *member)
{
    return strcmp(sample, axisBindsNames[*((int*) member)]);
}

static int nameToA_SortComparator(const void *one, const void *other)
{
    return strcmp(axesNames[*((int*) one)], axesNames[*((int*) other)]);
}

static int nameToAO_SortComparator(const void *one, const void *other)
{
    return strcmp(axesOptionsNames[*((int*) one)], axesOptionsNames[*((int*) other)]);
}

static int nameToAB_SortComparator(const void *one, const void *other)
{
    return strcmp(axisBindsNames[*((int*) one)], axisBindsNames[*((int*) other)]);
}

static void prepareIndex()
{
    if(!nameToA_init)
    {
        for(int i = 0; i < arrlen(nameToA); ++i)
        {
            nameToA[i] = i;
        }
        qsort(nameToA, arrlen(nameToA),
              sizeof(int), nameToA_SortComparator);
        nameToA_init = true;
    }

    if(!nameToAO_init)
    {
        for(int i = 0; i < arrlen(nameToAO); ++i)
        {
            nameToAO[i] = i;
        }
        qsort(nameToAO, arrlen(nameToAO),
              sizeof(int), nameToAO_SortComparator);
        nameToAO_init = true;
    }

    if(!nameToAB_init)
    {
        for(int i = 0; i < arrlen(nameToAB); ++i)
        {
            nameToAB[i] = i;
        }
        qsort(nameToAB, arrlen(nameToAB),
              sizeof(int), nameToAB_SortComparator);
        nameToAB_init = true;
    }
}

void I_ShutdownController(void)
{
    controller_t *temp;

    temp = knownControllers;
    while(temp)
    {
        if(temp->SDL_controller != NULL)
        {
            SDL_GameControllerClose(temp->SDL_controller);
            temp->SDL_controller = NULL;
        }
        temp = temp->next;
    }
    currentController = NULL;
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

static controller_t* registerNewController(char* guid, SDL_GameController* sdlController)
{
    controller_t* controller;
    char sectionName[11 + 33]; // "Controller_" + guid
    extern sectionHandler_t controllerHandler;

    controller = malloc(sizeof(controller_t));
    controller->next = NULL;
    memcpy(controller->guid, guid, 33);
    controller->SDL_controller = sdlController;

    controller->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] = 0;
    controller->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] = 1;
    controller->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] = 0;
    controller->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] = 1;

    controller->bindAxis[SDL_CONTROLLER_AXIS_LEFTX] = CONTROLLER_AXIS_STRAFE;
    controller->bindAxis[SDL_CONTROLLER_AXIS_LEFTY] = CONTROLLER_AXIS_MOVE;
    controller->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX] = CONTROLLER_AXIS_TURN;
    controller->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY] = CONTROLLER_AXIS_VLOOK;

    sprintf(sectionName, "Controller_%s", guid);
    M_AppendConfigSection(sectionName, &controllerHandler);

    return controller;
}

void I_InitController(void)
{
    int i;
    boolean foundController = false;
    SDL_GameController* controller;
    char guid[33];
    controller_t* temp;

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
            if(!controller)
                continue;

            SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(
                    SDL_GameControllerGetJoystick(controller)),
                guid, 33);
            if(knownControllers == NULL)
            {
                currentController = knownControllers = registerNewController(guid, controller);
            }
            else
            {
                temp = knownControllers;
                while(temp)
                {
                    if(strcmp(temp->guid, guid) == 0)
                    {
                        temp->SDL_controller = controller;
                        currentController = temp;
                        break;
                    }

                    if(temp->next)
                    {
                        temp = temp->next;
                    }
                    else
                    {
                        currentController = temp->next = registerNewController(guid, controller);
                        break;
                    }
                }
            }

            foundController = true;
            break;
        }
    }

    if (!foundController)
    {
        printf(english_language ?
               "I_InitController: Failed to open controller\n" :
               "I_InitController: ошибка активизации контроллера\n");
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        currentController = NULL;
        return;
    }

    SDL_GameControllerEventState(SDL_ENABLE);

    // Initialized okay!

    printf("I_InitController: %s\n", SDL_GameControllerName(currentController->SDL_controller));

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
    event.delayed = false;
    event.data1 = buttonEvent->button;
    event.data2 = event.data3 = event.data4 = 0;
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

    event.delayed = false;
    event.data2 = event.data3 = event.data4 = 0;
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
    int value = 0, axisValue;

    for(SDL_GameControllerAxis sdlAxis = SDL_CONTROLLER_AXIS_LEFTX; sdlAxis < SDL_CONTROLLER_AXIS_TRIGGERLEFT; sdlAxis++)
    {
        if(currentController->bindAxis[sdlAxis] == axis)
        {
            axisValue = SDL_GameControllerGetAxis(currentController->SDL_controller, sdlAxis);

            if(axisValue < DEAD_ZONE && axisValue > -DEAD_ZONE)
            {
                axisValue = 0;
            }

            if(currentController->invertAxis[sdlAxis])
                axisValue = -axisValue;

            value += axisValue;
        }
    }

    return value;
}

void I_UpdateController(void)
{
    if (currentController != NULL && currentController->SDL_controller != NULL)
    {
        event_t ev;

        ev.type = ev_controller_move;
        ev.delayed = false;
        ev.data1 = GetAxisState(CONTROLLER_AXIS_MOVE);
        ev.data2 = GetAxisState(CONTROLLER_AXIS_STRAFE);
        ev.data3 = GetAxisState(CONTROLLER_AXIS_TURN);
        ev.data4 = GetAxisState(CONTROLLER_AXIS_VLOOK);

        D_PostEvent(&ev);
    }
}

void I_BindControllerVariables(void)
{
    M_BindIntVariable("use_controller", &useController);
}

boolean ControllerHandler_Handles(char* sectionName)
{
    if(strstr(sectionName, "Controller_") != sectionName)
        return false;

    currentController = registerNewController(sectionName + 11, NULL);
    if(knownControllers == NULL)
    {
        knownControllers = currentController;
    }
    else
    {
        controller_t* temp = knownControllers;
        while(temp->next)
        {
            temp = temp->next;
        }
        temp->next = currentController;
    }
    prepareIndex();
    return true;
}

void ControllerHandler_HandleLine(char* keyName, char *value, size_t valueSize)
{
    char axisName[11];
    char axisOptionName[21];
    int* bsearchResult;
    SDL_GameControllerAxis axis;
    axisOption_t axisOption;

    if(sscanf(keyName, "%10[^_]_%20s", axisName, axisOptionName) != 2)
    {
        return;
    }

    bsearchResult = bsearch(axisName,
                            nameToA, arrlen(axesNames),
                            sizeof(int), nameToA_Comparator);
    if(bsearchResult == NULL)
        return;

    axis = *bsearchResult;
    bsearchResult = bsearch(axisOptionName,
                            nameToAO, arrlen(axesOptionsNames),
                            sizeof(int), nameToAO_Comparator);
    if(bsearchResult == NULL)
        return;

    axisOption = *bsearchResult;
    switch(axisOption)
    {
        case AO_INVERT:
            currentController->invertAxis[axis] = strtol(value, NULL, 0);
            break;
        case AO_BIND:
        {
            bsearchResult = bsearch(value,
                                    nameToAB, arrlen(axisBindsNames),
                                    sizeof(int), nameToAB_Comparator);
            if(bsearchResult == NULL)
                return;

            currentController->bindAxis[axis] = *bsearchResult;
            break;
        }
        default:
            return;
    }
}

void ControllerHandler_Save(FILE* file, char* sectionName)
{
    controller_t* temp = knownControllers;
    while(temp)
    {
        if(strcmp(temp->guid, sectionName + 11) == 0)
        {
            for(SDL_GameControllerAxis i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_TRIGGERLEFT; ++i)
            {
                fprintf(file, "%s_%s = %s\n",
                        axesNames[i], axesOptionsNames[AO_BIND],
                        axisBindsNames[temp->bindAxis[i]]);
                fprintf(file, "%s_%s = %d\n",
                        axesNames[i], axesOptionsNames[AO_INVERT],
                        temp->invertAxis[i]);
            }
        }
        temp = temp->next;
    }
}

void ControllerHandler_onFinishHandling()
{
    currentController = NULL;
}
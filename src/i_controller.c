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
#include "m_misc.h"
#include "m_fixed.h"
#include "jn.h"

// When an axis is within the dead zone, it is set to zero.
// This is 5% of the full range:

#define DEAD_ZONE 328
#define BASE_SENSITIVITY_BITS 3

static boolean isControllerInitialized = false;

// [Dasperal] This array must be in sync with SDL_GameControllerAxis enum!
static char* axesNames[] = {
    "LX",
    "LY",
    "RX",
    "RY",
    "LT",
    "RT"
};

static int nameToA[arrlen(axesNames)];
static boolean nameToA_init = false;

typedef enum
{
    AO_INVERT,
    AO_BIND,
    AO_DEADZONE,
    AO_SENSITIVITY
} axisOption_t;

// [Dasperal] This array must be in sync with axisOption_t enum!
static char* axesOptionsNames[] = {
    "Invert",
    "Binding",
    "DeadZone",
    "Sensitivity"
};

static int nameToAO[arrlen(axesOptionsNames)];
static boolean nameToAO_init = false;

// [Dasperal] This array must be in sync with controller_axis_t enum!
static char* axisBindsNames[] = {
    "NONE", // BUTTON
    "MOVE",
    "STRAFE",
    "TURN",
    "VLOOK"
};

static int nameToAB[arrlen(axisBindsNames)];
static boolean nameToAB_init = false;

static controller_t* knownControllers;
controller_t* activeControllers[ACTIVE_CONTROLLERS_SIZE];
controller_t* currentController;

// [Dasperal] This array must be in sync with SDL_GameControllerAxis enum!
static boolean axisButtonsPositive[SDL_CONTROLLER_AXIS_MAX] = {
    CONTROLLER_LSX_POSITIVE, CONTROLLER_LSY_POSITIVE, CONTROLLER_RSX_POSITIVE,
    CONTROLLER_RSY_POSITIVE, CONTROLLER_LEFT_TRIGGER, CONTROLLER_RIGHT_TRIGGER
};
// [Dasperal] This array must be in sync with SDL_GameControllerAxis enum!
static boolean axisButtonsNegative[SDL_CONTROLLER_AXIS_MAX] = {
    CONTROLLER_LSX_NEGATIVE, CONTROLLER_LSY_NEGATIVE, CONTROLLER_RSX_NEGATIVE,
    CONTROLLER_RSY_NEGATIVE, CONTROLLER_LEFT_TRIGGER_NEGATIVE, CONTROLLER_RIGHT_TRIGGER_NEGATIVE
};

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

    if(isControllerInitialized)
    {
        temp = knownControllers;
        while(temp)
        {
            if(temp->SDL_controller != NULL)
            {
                SDL_GameControllerClose(temp->SDL_controller);
                temp->SDL_controller = NULL;
                temp->index = -1;
                free(temp->name);
                temp->name = NULL;
            }
            temp = temp->next;
        }
        for(int i = 0; i < ACTIVE_CONTROLLERS_SIZE; ++i)
        {
            activeControllers[i] = NULL;
        }
        SDL_GameControllerEventState(SDL_IGNORE);
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        isControllerInitialized = false;
    }
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
    if(sdlController)
    {
        controller->name = M_StringDuplicate(SDL_GameControllerName(sdlController));
        controller->index = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(sdlController));
    }
    else
    {
        controller->name = NULL;
        controller->index = -1;
    }

    controller->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] = 0;
    controller->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] = 1;
    controller->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] = 0;
    controller->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] = 1;
    controller->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = 0;
    controller->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = 0;

    controller->bindAxis[SDL_CONTROLLER_AXIS_LEFTX] = CONTROLLER_AXIS_STRAFE;
    controller->bindAxis[SDL_CONTROLLER_AXIS_LEFTY] = CONTROLLER_AXIS_MOVE;
    controller->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX] = CONTROLLER_AXIS_TURN;
    controller->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY] = CONTROLLER_AXIS_VLOOK;
    controller->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = CONTROLLER_AXIS_BUTTON;
    controller->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = CONTROLLER_AXIS_BUTTON;

    controller->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX] = 16;
    controller->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY] = 16;
    controller->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX] = 16;
    controller->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY] = 16;
    controller->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = 1;
    controller->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = 1;

    controller->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX] = 8;
    controller->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY] = 8;
    controller->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX] = 8;
    controller->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY] = 6;
    controller->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = 8;
    controller->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = 8;

    sprintf(sectionName, "Controller_%s", guid);
    M_AppendConfigSection(sectionName, &controllerHandler);

    return controller;
}

static void ActivateController(SDL_GameController *controller)
{
    char guid[33];
    controller_t* temp;
    controller_t* activeController;

    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(
            SDL_GameControllerGetJoystick(controller)),
                      guid, 33);
    if(knownControllers == NULL)
    {
        activeController = knownControllers = registerNewController(guid, controller);
    }
    else
    {
        temp = knownControllers;
        while(temp)
        {
            if(strcmp(temp->guid, guid) == 0)
            {
                if(temp->SDL_controller == NULL)
                {
                    temp->SDL_controller = controller;
                    temp->name = M_StringDuplicate(SDL_GameControllerName(controller));
                    temp->index = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
                activeController = temp;
                break;}
                else
                {
                    printf("I_InitController: Found controllers with the same GUID \"%s\"\n\t%d: \"%s\"\n\t%d: \"%s\"\n"
                           "\tIf you experience problems with the simultaneous use of those controllers, report it as a bug\n",
                           guid, SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)),
                           SDL_GameControllerName(controller),
                           SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(temp->SDL_controller)),
                           SDL_GameControllerName(temp->SDL_controller));
                    SDL_GameControllerClose(controller);
                    return;
                }
            }

            if(temp->next)
            {
                temp = temp->next;
            }
            else
            {
                activeController = temp->next = registerNewController(guid, controller);
                break;
            }
        }
    }

    printf("I_InitController: Active controller \"%s\"\n", SDL_GameControllerName(controller));
    for (int i = 0; i < ACTIVE_CONTROLLERS_SIZE; ++i)
    {
        if(activeControllers[i] == NULL)
        {
            activeControllers[i] = activeController;
            break;
        }
    }
}

void I_InitControllerModule(void)
{
    int i;
    boolean foundController = false;
    SDL_GameController* controller;

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

            ActivateController(controller);
            foundController = true;
        }
    }

    if (!foundController)
    {
        printf(english_language ?
               "I_InitController: Failed to open controller\n" :
               "I_InitController: ошибка активизации контроллера\n");
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        return;
    }

    SDL_GameControllerEventState(SDL_ENABLE);

    // Initialized okay!
    isControllerInitialized = true;
}

void I_InitController(void)
{
    I_InitControllerModule();
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
    int axisValue;
    controller_t* controller = knownControllers;

    while(controller)
    {
        if(controller->index == axisEvent->which)
        {;
            break;
        }
        controller = controller->next;
    }

    if(!controller)
        return;

    if(controller->bindAxis[axisEvent->axis] != CONTROLLER_AXIS_BUTTON)
        return;

    axisValue = axisEvent->value;
    if(axisValue < controller->axisDeadZone[axisEvent->axis] * DEAD_ZONE
    && axisValue > -controller->axisDeadZone[axisEvent->axis] * DEAD_ZONE)
    {
        axisValue = 0;
    }

    if(controller->invertAxis[axisEvent->axis])
        axisValue = -axisValue;

    event.delayed = false;
    event.data2 = event.data3 = event.data4 = 0;
    if(controller->axisButtonsPositiveState[axisEvent->axis] == false && axisValue > 0)
    {
        event.type = ev_controller_keydown;
        event.data1 = axisButtonsPositive[axisEvent->axis];
        D_PostEvent(&event);
        controller->axisButtonsPositiveState[axisEvent->axis] = true;
    }
    if(controller->axisButtonsPositiveState[axisEvent->axis] == true && axisValue <= 0)
    {
        event.type = ev_controller_keyup;
        event.data1 = axisButtonsPositive[axisEvent->axis];
        D_PostEvent(&event);
        controller->axisButtonsPositiveState[axisEvent->axis] = false;
    }
    if(controller->axisButtonsNegativeState[axisEvent->axis] == false && axisValue < 0)
    {
        event.type = ev_controller_keydown;
        event.data1 = axisButtonsNegative[axisEvent->axis];
        D_PostEvent(&event);
        controller->axisButtonsNegativeState[axisEvent->axis] = true;
    }
    if(controller->axisButtonsNegativeState[axisEvent->axis] == true && axisValue >= 0)
    {
        event.type = ev_controller_keyup;
        event.data1 = axisButtonsNegative[axisEvent->axis];
        D_PostEvent(&event);
        controller->axisButtonsNegativeState[axisEvent->axis] = false;
    }
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
    fixed_t value = 0;
    int axisValue;
    controller_t* temp = knownControllers;

    while(temp)
    {
        if(temp->SDL_controller != NULL)
        {
            for(SDL_GameControllerAxis sdlAxis = SDL_CONTROLLER_AXIS_LEFTX;
                sdlAxis < SDL_CONTROLLER_AXIS_MAX; sdlAxis++)
            {
                if(temp->bindAxis[sdlAxis] == axis)
                {
                    axisValue = SDL_GameControllerGetAxis(temp->SDL_controller, sdlAxis);

                    if(axisValue < temp->axisDeadZone[sdlAxis] * DEAD_ZONE &&
                       axisValue > -temp->axisDeadZone[sdlAxis] * DEAD_ZONE)
                    {
                        axisValue = 0;
                    }

                    if(temp->invertAxis[sdlAxis])
                        axisValue = -axisValue;

                    if(axisValue > 32766)
                        axisValue = 32766;
                    if(axisValue < -32766)
                        axisValue = -32766;

                    value += FixedMul(FixedDiv(axisValue << FRACBITS, 32766 << FRACBITS),
                                      temp->axisSensitivity[sdlAxis] << (FRACBITS - BASE_SENSITIVITY_BITS));
                }
            }
        }
        temp = temp->next;
    }

    if(value > FRACUNIT)
        value = FRACUNIT;
    if(value < -FRACUNIT)
        value = -FRACUNIT;
    return value;
}

void I_UpdateController(void)
{
    if(activeControllers[0] != NULL)
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
        case AO_DEADZONE:
            currentController->axisDeadZone[axis] = strtol(value, NULL, 0);
            break;
        case AO_SENSITIVITY:
            currentController->axisSensitivity[axis] = strtol(value, NULL, 0);
            break;
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
            for(SDL_GameControllerAxis i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; ++i)
            {
                fprintf(file, "%s_%s = %s\n",
                        axesNames[i], axesOptionsNames[AO_BIND],
                        axisBindsNames[temp->bindAxis[i]]);
                fprintf(file, "%s_%s = %d\n",
                        axesNames[i], axesOptionsNames[AO_INVERT],
                        temp->invertAxis[i]);
                fprintf(file, "%s_%s = %d\n",
                        axesNames[i], axesOptionsNames[AO_DEADZONE],
                        temp->axisDeadZone[i]);
                fprintf(file, "%s_%s = %d\n",
                        axesNames[i], axesOptionsNames[AO_SENSITIVITY],
                        temp->axisSensitivity[i]);
            }
        }
        temp = temp->next;
    }
}

void ControllerHandler_onFinishHandling()
{
    currentController = NULL;
}
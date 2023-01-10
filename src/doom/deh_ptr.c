//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
//
// Parses Action Pointer entries in dehacked files
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "info.h"
#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "jn.h"

actionf_t codeptrs[NUMSTATES]; // [crispy] share with deh_bexptr.c

static int CodePointerIndex(actionf_t *ptr)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        if (!memcmp(&codeptrs[i], ptr, sizeof(actionf_t)))
        {
            return i;
        }
    }

    return -1;
}

static void DEH_PointerInit(void)
{
    int i;
    
    // Initialize list of dehacked pointers

    for (i=0; i<EXTRASTATES; ++i)
        codeptrs[i] = states[i].action;

    // [BH] Initialize extra dehacked states
    for (; i < NUMSTATES; i++)
    {
	states[i].sprite = SPR_TNT1;
	states[i].frame = 0;
	states[i].tics = -1;
	states[i].action.acv = (actionf_v) NULL;
	states[i].nextstate = i;
	states[i].misc1 = 0;
	states[i].misc2 = 0;
//	states[i].dehacked = false;
	codeptrs[i] = states[i].action;
    }
}

static void *DEH_PointerStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    
    // FIXME: can the third argument here be something other than "Frame"
    // or are we ok?

    if (sscanf(line, "Pointer %*i (%*s %i)", &frame_number) != 1)
    {
        DEH_Warning(context, english_language ?
                             "Parse error on section start" :
                             "Ошибка обработки на начальной стадии");
        return NULL;
    }

    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
        DEH_Warning(context, english_language ?
                             "Invalid frame number: %i" :
                             "Некорректный номер фрейма: %i",
                             frame_number);
        return NULL;
    }

    return &states[frame_number];
}

static void DEH_PointerParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t *state;
    char *variable_name, *value;
    int ivalue;
    
    if (tag == NULL)
       return;

    state = (state_t *) tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse
        DEH_Warning(context, english_language ?
        "Failed to parse assignment" :
        "Ошибка анализа назначения");
        return;
    }
    
//    printf("Set %s to %s for state\n", variable_name, value);

    // all values are integers

    ivalue = atoi(value);
    
    // set the appropriate field

    if (!strcasecmp(variable_name, "Codep frame"))
    {
        if (ivalue < 0 || ivalue >= NUMSTATES)
        {
            DEH_Warning(context, english_language ?
                                 "Invalid state '%i'" :
                                 "Некорректное положение '%i'",
                                 ivalue);
        }
        else
        {        
            state->action = codeptrs[ivalue];
        }
    }
    else
    {
        DEH_Warning(context, english_language ?
                             "Unknown variable name '%s'" :
                             "Неизвестная переменная '%s'",
                             variable_name);
    }
}

static void DEH_PointerSHA1Sum(sha1_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        SHA1_UpdateInt32(context, CodePointerIndex(&states[i].action));
    }
}

deh_section_t deh_section_pointer =
{
    "Pointer",
    DEH_PointerInit,
    DEH_PointerStart,
    DEH_PointerParseLine,
    NULL,
    DEH_PointerSHA1Sum,
};


//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	System specific interface stuff.
//


#pragma once

#include "d_ticcmd.h"
#include "d_event.h"


typedef void (*atexit_func_t)(void);

// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte*	I_ZoneBase (int *size);

boolean I_ConsoleStdout(void);


// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit (void);

void I_Error (char *error, ...);

void *I_Realloc(void *ptr, size_t size);

boolean I_GetMemoryValue(unsigned int offset, void *value, int size);

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)

void I_AtExit(atexit_func_t func, boolean run_if_error);

// Add all system-specific config file variable bindings.

void I_BindVariables(void);

#ifdef _WIN32
dll_export extern boolean console_connected;

void RD_CreateWindowsConsole(void);
void I_ConsolePause(void);

#define CONSOLE_PROLOG RD_CreateWindowsConsole();
#define CONSOLE_EPILOG if(!console_connected) I_ConsolePause();
#else
#define CONSOLE_PROLOG
#define CONSOLE_EPILOG
#endif

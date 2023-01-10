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

#include "doomdef.h"


// Read events from all input devices
void D_ProcessEvents (void); 


//
// BASE LEVEL
//
void D_PageTicker (void);
void D_PageDrawer (void);
void D_AdvanceDemo (void);
void D_DoAdvanceDemo (void);
void D_StartTitle (void);


//
// GLOBAL VARIABLES
//

extern gameaction_t gameaction;
extern boolean oldest_version;
extern boolean sgl_loaded, sgl_compat_loaded, mlvls_loaded, havemap33;

extern void EnableLoadingDisk (void);

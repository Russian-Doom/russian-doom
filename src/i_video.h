//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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



#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

// Screen width and height.

#define hires 1

// -----------------------------------------------------------------------------
// [JN] Wide screen executable building definition. In fact, it's not a real
// wide screen mode, it's just a way to get wide picture on resolutions like:
// - 1920x1080
// - 1366x768
// - 1600x900
//
// There is one simple step to turn building executable into wide mode -
// uncomment following macro to define some code to be built as wide.
// #define WIDESCREEN

// In case of building wide screen executable, extend screen width to new value.
#ifdef WIDESCREEN
#define ORIGWIDTH       426
#else
#define ORIGWIDTH       320
#endif

#define ORIGHEIGHT      200
#define SRCWIDTH        320

// Delta is a pixel width from left size of the screen to the standard 320
// pixel canvas. It stands for moving some elements to the right to get a
// proper centering in wide mode.
#ifdef WIDESCREEN
#define ORIGWIDTH_DELTA 53
#else
#define ORIGWIDTH_DELTA 0
#endif

// -----------------------------------------------------------------------------

#define SCREENWIDTH  (ORIGWIDTH << hires)
#define SCREENHEIGHT (ORIGHEIGHT << hires)

#define SCREENWIDTH_4_3 (256 << hires)

// Screen height used when aspect_ratio_correct=true.

#define SCREENHEIGHT_4_3 (240 << hires)

typedef boolean (*grabmouse_callback_t)(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);

// [crispy] re-initialize only the parts of the rendering stack that are really necessary
void I_ReInitGraphics (int reinit);

void I_GraphicsCheckCommandLine(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette);
int I_GetPaletteIndex(int r, int g, int b);

// void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);

void I_SetWindowTitle(char *title);

void I_CheckIsScreensaver(void);
void I_SetGrabMouseCallback(grabmouse_callback_t func);

void I_DisplayFPSDots(boolean dots_on);
void I_BindVideoVariables(void);

void I_InitWindowTitle(void);
void I_InitWindowIcon(void);

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

// void I_StartFrame (void);

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic (void);

// Enable the loading disk image displayed when reading from disk.

void I_EnableLoadingDisk(int xoffs, int yoffs);

extern char *video_driver;
extern boolean screenvisible;

extern int vanilla_keyboard_mapping;
extern boolean screensaver_mode;
extern int usegamma;
extern byte *I_VideoBuffer;

extern int screen_width;
extern int screen_height;
extern int fullscreen;
extern int aspect_ratio_correct;
extern int smoothing;
extern int vga_porch_flash;
extern int integer_scaling;
extern int force_software_renderer;

#endif

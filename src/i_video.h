//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	System specific interface stuff.
//


#pragma once

#include "doomtype.h"


// Screen width and height.

#define hires 1

#define ORIGWIDTH       320
#define ORIGHEIGHT      200

#define SCREENWIDTH     (ORIGWIDTH << hires)
#define SCREENHEIGHT    (ORIGHEIGHT << hires)

#define SCREENWIDTH_4_3 (256 << hires)

// [JN] Aspect ratio macroses and variables. Available ratios are:
// aspect_ratio = 0 (4:3)
// aspect_ratio = 1 (5:4)
// aspect_ratio = 2 (16:9)
// aspect_ratio = 3 (16:10)
// aspect_ratio = 4 (21:9)

#define WIDEORIGWIDTH   560
#define WIDESCREENWIDTH (WIDEORIGWIDTH << hires)
#define WIDE_DELTA      (WIDEORIGWIDTH - ORIGWIDTH) / 2
#define SCREENHEIGHT_5_4 (256 << hires)

extern int aspect_ratio;
extern int aspect_ratio_temp;
extern int wide_delta;
extern int screenwidth;
extern int origwidth;
extern int actualheight;

void I_DrawBlackBorders (void);

// Screen height used when aspect_ratio_correct=true.

#define SCREENHEIGHT_4_3 (240 << hires)

void *I_GetSDLWindow(void);
void *I_GetSDLRenderer(void);

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
extern const char *gammalevel_names[];
extern const char *gammalevel_names_rus[];
extern int usegamma;
extern byte *I_VideoBuffer;

extern int screen_width;
extern int screen_height;
extern int fullscreen;
extern int aspect_ratio_correct;
extern int smoothing;
extern int vga_porch_flash;
extern int force_software_renderer;

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

#include <SDL.h>
#include "doomtype.h"


// Screen width and height.

#define ORIGWIDTH     320
#define ORIGHEIGHT    200

#define MAXWIDTH      2240
#define MAXHEIGHT     800

extern int hires;
extern int detailshift;

extern int SCREENWIDTH;
extern int SCREENHEIGHT;

extern int rendering_resolution;
extern int rendering_resolution_temp;

// [JN] Aspect ratio macroses and variables. Available ratios are:
// aspect_ratio = 0 (4:3)
// aspect_ratio = 1 (5:4)
// aspect_ratio = 2 (16:9)
// aspect_ratio = 3 (16:10)
// aspect_ratio = 4 (21:9)

extern int aspect_ratio;
extern int aspect_ratio_temp;
extern int opengles_renderer;
extern int opengles_renderer_temp;
extern int wide_delta;
extern int screenwidth;
extern int screenwidth_low;
extern int origwidth;
extern int actualheight;

void I_DrawBlackBorders (void);

void *I_GetSDLWindow(void);
void *I_GetSDLRenderer(void);

typedef boolean (*grabmouse_callback_t)(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);

// [crispy] re-initialize only the parts of the rendering stack that are really necessary
void I_ReInitGraphics (const int reinit);

void I_GraphicsCheckCommandLine(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (const byte *palette);
const int I_GetPaletteIndex(const int r, const int g, const int b);

// void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);

void I_SetWindowTitle(const char *title);

void I_CheckIsScreensaver(void);
void I_SetGrabMouseCallback(grabmouse_callback_t func);

void I_DisplayFPSDots(boolean dots_on);
void I_BindVideoVariables(void);

void I_InitWindowTitle(void);
void I_InitWindowIcon(void);
void I_ToggleWindowBorder (void);
void I_KeepWindowOnTop (void);

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

// void I_StartFrame (void);

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic (void);

// Enable the loading disk image displayed when reading from disk.

void I_EnableLoadingDisk(int xoffs, int yoffs);

void AdjustWindowSize(void);

extern SDL_Window *screen;

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
extern int preserve_window_aspect_ratio;
extern int smoothing;
extern int vga_porch_flash;
extern int force_software_renderer;

extern int window_border;
extern int window_width;
extern int window_height;
extern int window_title_short;
extern int window_ontop;

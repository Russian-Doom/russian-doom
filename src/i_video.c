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
//	DOOM graphics stuff for SDL.
//



#include <stdlib.h>

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "icon.h"
#include "config.h"
#include "d_name.h"
#include "d_loop.h"
#include "deh_str.h"
#include "doomtype.h"
#include "i_controller.h"
#include "i_input.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "jn.h"


// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

static SDL_Window *screen;
static SDL_Renderer *renderer;

// Window title

static char *window_title = "";

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

static SDL_Surface *screenbuffer = NULL;
static SDL_Surface *argbbuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_upscaled = NULL;

static SDL_Rect blit_rect = {
    0,
    0,
    WIDESCREENWIDTH,
    SCREENHEIGHT
};

static uint32_t pixel_format;

// palette

static SDL_Color palette[256];
static boolean palette_to_set;

// display has been set up?

static boolean initialized = false;

// disable mouse?

static boolean nomouse = false;
int usemouse = 1;

// Save screenshots in PNG format.

int png_screenshots = 1; // [JN] Crispy!

// SDL video driver name

char *video_driver = "";

// [JN] Window X and Y position to save and restore.

int window_position_y = 0;
int window_position_x = 0;

// [JN] Use bordered or borderless window.

int window_border = 1;

// SDL display number on which to run.

int video_display = 0;

// [JN] Defines window title composition:
// true - only game name will appear.
// false - game name, port name and version will appear.

int window_title_short = true;

// Screen width and height, from configuration file.

int window_width = SCREENWIDTH;
int window_height = SCREENHEIGHT_4_3;

typedef enum
{
    cs_WIDTH_AND_HEIGHT = 0,
    cs_WIDTH,
    cs_HEIGHT
} changedSize_t;

static changedSize_t changedWindowSize = cs_WIDTH_AND_HEIGHT;

// Fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int fullscreen_width = 0, fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int max_scaling_buffer_pixels = 16000000;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// [JN] Aspect ratio variables.

int aspect_ratio = 2;
int aspect_ratio_temp; // used for in-game toggling
int wide_delta;
int screenwidth;
int origwidth;

// [JN] Vertical Sync
int vsync = true;

// Aspect ratio correction mode

int aspect_ratio_correct = true;
int actualheight;

// [JN] Show FPS counter

int show_fps = false;
int real_fps;

// [JN] Незначительное сглаживание текстур

int smoothing = false;

// VGA Porch palette change emulation

int vga_porch_flash = false;

// Force integer scales for resolution-independent rendering

int integer_scaling = false;

// Force software rendering, for systems which lack effective hardware
// acceleration

int force_software_renderer = false;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 35;  // [JN] Redused from 1000 to 35

// Grab the mouse? (int type for config code). nograbmouse_override allows
// this to be temporarily disabled via the command line.

static int grabmouse = true;
static boolean nograbmouse_override = false;

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible = true;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static boolean display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static boolean noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = NULL;

// Does the window currently have focus?

boolean window_focused = true;

// [JN] Does the sound volume needs to be updated?
// Used for "mute_inactive_window" feature.

boolean volume_needs_update = false;

// Window resize state.

static boolean need_resize = false;
static unsigned int last_resize_time;
static int resize_delay = 70; // [JN] Redused from 500 to 70

//
// [JN] Brightness, gamma, saturtion and color intensity.
//

float brightness = 1.0f;

int  usegamma = 7;  // default gamma level is 0.85
byte gammatable[18][256];

float color_saturation = 1.0f;

int show_palette = 1;

float r_color_factor  = 1.0f;
float g_color_factor  = 1.0f;
float b_color_factor  = 1.0f;

static const float gammalevels[18] =
{
    // Darker
    0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f,

    // No gamma correction
    1.0f,

    // Lighter
    1.125f, 1.25f, 1.375f, 1.5f, 1.625f, 1.75f, 1.875f, 2.0f,
};

// [JN] Text representation of values above.

const char *gammalevel_names[] =
{
    // Darker
    "0.50", "0.55", "0.60", "0.65", "0.70", "0.75", "0.80", "0.85", "0.90",

    // No gamma correction
    "1.00",

    // Lighter
    "1.125", "1.25", "1.375", "1.5", "1.625", "1.75", "1.875", "2.00",
};

const char *gammalevel_names_rus[] =
{
    // Darker
    "0>50", "0>55", "0>60", "0>65", "0>70", "0>75", "0>80", "0>85", "0>90",

    // No gamma correction
    "1>00",

    // Lighter
    "1>125", "1>25", "1>375", "1>5", "1>625", "1>75", "1>875", "2>00",
};


void *I_GetSDLWindow(void)
{
    return screen;
}

void *I_GetSDLRenderer(void)
{
    return renderer;
}

static boolean MouseShouldBeGrabbed()
{
    // never grab the mouse when in screensaver mode
   
    if (screensaver_mode)
        return false;

    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // if we specify not to grab the mouse, never grab

    if (nograbmouse_override || !grabmouse)
        return false;

    // Invoke the grabmouse callback function to determine whether
    // the mouse should be grabbed

    if (grabmouse_callback != NULL)
    {
        return grabmouse_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(boolean dots_on)
{
    display_fps_dots = dots_on;
}

static void SetShowCursor(boolean show)
{
    if (!screensaver_mode)
    {
        // When the cursor is hidden, grab the input.
        // Relative mode implicitly hides the cursor.
        SDL_SetRelativeMouseMode(!show);
        SDL_GetRelativeMouseState(NULL, NULL);
    }
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
        static int w, h;
        
        SetShowCursor(true);

        // [JN] Get screen width and height.
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // [JN] Place mouse cursor to the center of the screen.
        if (fullscreen)
        {
            SDL_WarpMouseGlobal(w / 2, h / 2);
        }

        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        initialized = false;
    }
}


// Adjust window_width / window_height variables to be an an aspect
// ratio consistent with the aspect_ratio_correct variable.
static void AdjustWindowSize(void)
{
    if (aspect_ratio_correct || integer_scaling)
    {
        switch(changedWindowSize)
        {
            case cs_WIDTH:
                window_height = (window_width * actualheight + screenwidth - 1) / screenwidth;
                break;
            case cs_HEIGHT:
                window_width = window_height * screenwidth / actualheight;
                break;
            default:
                if(window_width * actualheight <= window_height * screenwidth)
                {
                    // We round up window_height if the ratio is not exact; this leaves
                    // the result stable.
                    window_height = (window_width * actualheight + screenwidth - 1) / screenwidth;
                }
                else
                {
                    window_width = window_height * screenwidth / actualheight;
                }
        }
    }
    changedWindowSize = cs_WIDTH_AND_HEIGHT;
}

static void HandleWindowEvent(SDL_WindowEvent *event)
{
    int i;

    switch (event->event)
    {
#if 0 // SDL2-TODO
        case SDL_ACTIVEEVENT:
            // need to update our focus state
            UpdateFocus();
            break;
#endif
        case SDL_WINDOWEVENT_EXPOSED:
            palette_to_set = true;
            break;

        case SDL_WINDOWEVENT_RESIZED:
            need_resize = true;
            if(window_width == event->data1 && window_height != event->data2)
                changedWindowSize = cs_HEIGHT;
            else if(window_width != event->data1 && window_height == event->data2)
                changedWindowSize = cs_WIDTH;
            else
                changedWindowSize = cs_WIDTH_AND_HEIGHT;
            last_resize_time = SDL_GetTicks();
            break;

        // Don't render the screen when the window is minimized:

        case SDL_WINDOWEVENT_MINIMIZED:
            screenvisible = false;
            break;

        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
            screenvisible = true;
            break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.

        case SDL_WINDOWEVENT_FOCUS_GAINED:
            window_focused = true;
            volume_needs_update = true;
            break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
            window_focused = false;
            volume_needs_update = true;
            break;
            
        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the video_display config variable.

        case SDL_WINDOWEVENT_MOVED:
            i = SDL_GetWindowDisplayIndex(screen);
            if (i >= 0)
            {
                video_display = i;
            }
            // [JN] Get X and Y coordinates after moving a window.
            SDL_GetWindowPosition(screen, &window_position_x, &window_position_y);
            break;

        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// HandleWindowResize
// [JN] Updates window contents (SDL texture) on fly while resizing.
// SDL_WINDOWEVENT_RESIZED from above is still needed to get rid of 
// black borders after window size has been changed.
// -----------------------------------------------------------------------------

static int HandleWindowResize (void* data, SDL_Event *event) 
{
    if (event->type == SDL_WINDOWEVENT 
    &&  event->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        // Redraw window contents
        I_FinishUpdate();
    }
    return 0;
}

// [JN] Alt + Return (the Enter key on main keyboard)
static boolean ToggleFullScreenKeyShortcut(SDL_Keysym *sym)
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return (sym->scancode == SDL_SCANCODE_RETURN ||
            sym->scancode == SDL_SCANCODE_KP_ENTER) && (sym->mod & flags) != 0;
}

static void I_ToggleFullScreen(void)
{
    unsigned int flags = 0;

    // TODO: Consider implementing fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (fullscreen_width != 0 || fullscreen_height != 0)
    {
        return;
    }

    fullscreen = !fullscreen;

    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);

    if (!fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, window_width, window_height);
    }
}

void I_GetEvent(void)
{
    extern void I_HandleKeyboardEvent(SDL_Event *sdlevent);
    extern void I_HandleMouseEvent(SDL_Event *sdlevent);
    extern void I_HandleControllerEvent(SDL_Event *sdlevent);
    SDL_Event sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                if (ToggleFullScreenKeyShortcut(&sdlevent.key.keysym))
                {
                    I_ToggleFullScreen();
                    break;
                }
                // deliberate fall-though

            case SDL_KEYUP:
		        I_HandleKeyboardEvent(&sdlevent);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                if (usemouse && !nomouse && window_focused)
                {
                    I_HandleMouseEvent(&sdlevent);
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERAXISMOTION:
                I_HandleControllerEvent(&sdlevent);
                break;

            case SDL_QUIT:
                if (screensaver_mode)
                {
                    I_Quit();
                }
                else
                {
                    event_t event;
                    event.type = ev_quit;
                    event.delayed = false;
                    event.data1 = event.data2 = event.data3 = event.data4 = 0;
                    D_PostEvent(&event);
                }
                break;

            case SDL_WINDOWEVENT:
                if (sdlevent.window.windowID == SDL_GetWindowID(screen))
                {
                    HandleWindowEvent(&sdlevent.window);
                }
                break;

            default:
                break;
        }
    }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();

    if (usemouse && !nomouse && window_focused)
    {
        I_ReadMouse();
    }

    I_UpdateController();
}

static void UpdateGrab(void)
{
    static boolean currently_grabbed = false;
    boolean grab;

    grab = MouseShouldBeGrabbed();

    if (screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
    }
    else if (!grab && currently_grabbed)
    {
        int screen_w, screen_h;

        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_GetWindowSize(screen, &screen_w, &screen_h);
        SDL_WarpMouseInWindow(screen, screen_w - 16, screen_h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;

}

static void LimitTextureSize(int *w_upscale, int *h_upscale)
{
    SDL_RendererInfo rinfo;
    int orig_w, orig_h;

    orig_w = *w_upscale;
    orig_h = *h_upscale;

    // Query renderer and limit to maximum texture dimensions of hardware:
    if (SDL_GetRendererInfo(renderer, &rinfo) != 0)
    {
        I_Error("CreateUpscaledTexture: SDL_GetRendererInfo() call failed: %s",
                SDL_GetError());
    }

    while (*w_upscale * screenwidth > rinfo.max_texture_width)
    {
        --*w_upscale;
    }
    while (*h_upscale * SCREENHEIGHT > rinfo.max_texture_height)
    {
        --*h_upscale;
    }

    if ((*w_upscale < 1 && rinfo.max_texture_width > 0) ||
        (*h_upscale < 1 && rinfo.max_texture_height > 0))
    {
        I_Error("CreateUpscaledTexture: Can't create a texture big enough for "
                "the whole screen! Maximum texture size %dx%d",
                rinfo.max_texture_width, rinfo.max_texture_height);
    }

    // We limit the amount of texture memory used for the intermediate buffer,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very
    // huge textures, so the user can use this to reduce the maximum texture
    // size if desired.

    if (max_scaling_buffer_pixels < screenwidth * SCREENHEIGHT)
    {
        I_Error("CreateUpscaledTexture: max_scaling_buffer_pixels too small "
                "to create a texture buffer: %d < %d",
                max_scaling_buffer_pixels, screenwidth * SCREENHEIGHT);
    }

    while (*w_upscale * *h_upscale * screenwidth * SCREENHEIGHT
           > max_scaling_buffer_pixels)
    {
        if (*w_upscale > *h_upscale)
        {
            --*w_upscale;
        }
        else
        {
            --*h_upscale;
        }
    }

    if (*w_upscale != orig_w || *h_upscale != orig_h)
    {
        printf("CreateUpscaledTexture: Limited texture size to %dx%d "
               "(max %d pixels, max texture size %dx%d)\n",
               *w_upscale * screenwidth, *h_upscale * SCREENHEIGHT,
               max_scaling_buffer_pixels,
               rinfo.max_texture_width, rinfo.max_texture_height);
    }
}

static void CreateUpscaledTexture(boolean force)
{
    int w, h;
    int h_upscale, w_upscale;
    static int h_upscale_old, w_upscale_old;

    SDL_Texture *new_texture, *old_texture;

    // Get the size of the renderer output. The units this gives us will be
    // real world pixels, which are not necessarily equivalent to the screen's
    // window size (because of highdpi).
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        I_Error(english_language ?
                "Failed to get renderer output size: %s" :
                "Невозможно выполнить рендеринг размера: %s",
                SDL_GetError());
    }

    // When the screen or window dimensions do not match the aspect ratio
    // of the texture, the rendered area is scaled down to fit. Calculate
    // the actual dimensions of the rendered area.

    if (w * actualheight < h * screenwidth)
    {
        // Tall window.

        h = w * actualheight / screenwidth;
    }
    else
    {
        // Wide window.

        w = h * screenwidth / actualheight;
    }

    // Pick texture size the next integer multiple of the screen dimensions.
    // If one screen dimension matches an integer multiple of the original
    // resolution, there is no need to overscale in this direction.

    w_upscale = (w + screenwidth - 1) / screenwidth;
    h_upscale = (h + SCREENHEIGHT - 1) / SCREENHEIGHT;

    // Minimum texture dimensions of 320x200.

    if (w_upscale < 1)
    {
        w_upscale = 1;
    }
    if (h_upscale < 1)
    {
        h_upscale = 1;
    }

    LimitTextureSize(&w_upscale, &h_upscale);

    // Create a new texture only if the upscale factors have actually changed.

    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force)
    {
        return;
    }

    h_upscale_old = h_upscale;
    w_upscale_old = w_upscale;

    // Set the scaling quality for rendering the upscaled texture to "linear",
    // which looks much softer and smoother than "nearest" but does a better
    // job at downscaling from the upscaled texture to screen.

    // [JN] smooting - функция и переменная незначительного сглаживания текстур.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, smoothing ? "linear" : "nearest");

    new_texture = SDL_CreateTexture(renderer,
                                pixel_format,
                                SDL_TEXTUREACCESS_TARGET,
                                w_upscale*screenwidth,
                                h_upscale*SCREENHEIGHT);

    old_texture = texture_upscaled;
    texture_upscaled = new_texture;

    if (old_texture != NULL)
    {
        SDL_DestroyTexture(old_texture);
    }
}

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
fixed_t fractionaltic;

// -----------------------------------------------------------------------------
// [JN] I_DrawBlackBorders
// Filling side borders with black color in widescreen mode.
// Used in screensize 9 for emulating 4:3 display aspect ratio.
// -----------------------------------------------------------------------------

void I_DrawBlackBorders (void)
{
    SDL_Rect rectangle_left;
    SDL_Rect rectangle_right;

    if (vga_porch_flash)
    {
        // [JN] "flash" the pillars/letterboxes with palette 
        // changes, emulating VGA "porch" behaviour.
        SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g,
                                         palette[0].b, SDL_ALPHA_OPAQUE);
    }
    else
    {
        // [JN] Or else, draw them with unchangable black color.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }

    rectangle_left.x = 0;
    rectangle_left.y = 0;
    rectangle_left.w = wide_delta << hires;
    rectangle_left.h = actualheight;

    rectangle_right.x = (wide_delta << hires) + SCREENWIDTH;
    rectangle_right.y = 0;
    rectangle_right.w = WIDEORIGWIDTH;
    rectangle_right.h = actualheight;

    SDL_RenderFillRect(renderer, &rectangle_left);
    SDL_RenderFillRect(renderer, &rectangle_right);
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int lasttic;
    int tics;
    int i;

    if (!initialized)
        return;

    if (noblit)
        return;

    if (need_resize)
    {
        if (SDL_GetTicks() > last_resize_time + resize_delay)
        {
            int flags;
            // When the window is resized (we're not in fullscreen mode),
            // save the new window size.
            flags = SDL_GetWindowFlags(screen);
            if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0)
            {
                SDL_GetWindowSize(screen, &window_width, &window_height);

                // Adjust the window by resizing again so that the window
                // is the right aspect ratio.
                AdjustWindowSize();
                SDL_SetWindowSize(screen, window_width, window_height);
            }
            CreateUpscaledTexture(false);
            need_resize = false;
            palette_to_set = true;
        }
        else
        {
            return;
        }
    }

    UpdateGrab();

#if 0 // SDL2-TODO
    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;
#endif

    // [crispy] variable rendering framerate
    // [JN] Modified to have a variable FPS cap.
    if (uncapped_fps && !singletics)
    {
        static int halftics_old;
        int halftics;
        extern int GetAdjustedTimeN (const int N);

        while ((halftics = GetAdjustedTimeN(max_fps)) == halftics_old)
        {
            SDL_Delay(1);
        }

        halftics_old = halftics;
    }

    // draws little dots on the bottom of the screen

    if (display_fps_dots)
    {
	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*4 ; i+=4)
	    I_VideoBuffer[ (SCREENHEIGHT-1)*screenwidth + i] = 0xff;
	for ( ; i<20*4 ; i+=4)
	    I_VideoBuffer[ (SCREENHEIGHT-1)*screenwidth + i] = 0x0;
    }

	// [crispy] [AM] Real FPS counter
    if (show_fps)
	{
		static int lastmili;
		static int fpscount;
		int mili;

		fpscount++;

		i = SDL_GetTicks();
		mili = i - lastmili;

		// Update FPS counter every second
        // [JN] Update 10x time faster (1000 reduced to 100)
		if (mili >= 100)
		{
			real_fps = (fpscount * 1000) / mili;
			fpscount = 0;
			lastmili = i;
            
            if (real_fps >= 9999)
                real_fps  = 9999;
            if (real_fps <= 0)
                real_fps  = 0;
		}
	}

    // Draw disk icon before blit, if necessary.
    if (show_diskicon)
    {
        V_DrawDiskIcon();
    }

    if (palette_to_set)
    {
        SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
        palette_to_set = false;
    }

    if (vga_porch_flash && aspect_ratio <= 1)
    {
        // "flash" the pillars/letterboxes with palette changes, emulating
        // VGA "porch" behaviour (GitHub issue #832)
        SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g,
            palette[0].b, SDL_ALPHA_OPAQUE);
    }

    // Blit from the paletted 8-bit screen buffer to the intermediate
    // 32-bit RGBA buffer that we can load into the texture.

    SDL_LowerBlit(screenbuffer, &blit_rect, argbbuffer, &blit_rect);

    // Update the intermediate texture with the contents of the RGBA buffer.

    SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);

    // Make sure the pillarboxes are kept clear each frame.

    SDL_RenderClear(renderer);

    if (smoothing)
    {
    // Render this intermediate texture into the upscaled texture
    // using "nearest" integer scaling.

    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Finally, render this upscaled texture to screen using linear scaling.

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    }
    else
    {
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    }

    if (aspect_ratio >= 2 && screenblocks == 9)
    {
        I_DrawBlackBorders();
    }

    // Draw!

    SDL_RenderPresent(renderer);

    // [AM] Figure out how far into the current tic we're in as a fixed_t.
    if (uncapped_fps)
    {
        fractionaltic = I_GetFracRealTime();
    }
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy(scr, I_VideoBuffer, screenwidth*SCREENHEIGHT*sizeof(*scr));
}

// -----------------------------------------------------------------------------
// I_InitGammaTables
// [JN] Initialize and generate gamma-correction levels.
// Based on implementation from DOOM Retro.
// -----------------------------------------------------------------------------

static void I_InitGammaTables (void)
{
    for (int i = 0; i < 18; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            gammatable[i][j] = (byte)(pow(j / 255.0, 1.0 / gammalevels[i]) * 255.0 + 0.5);
        }
    }
}

//
// I_SetPalette
//
void I_SetPalette (byte *doompalette)
{
    int i;

    // [JN] Safe-guard conditions to fix incorrect values:
    if (brightness < 0) brightness = 0;
    if (brightness > 1) brightness = 1;
    
    if (usegamma < 0) usegamma = 0;
    if (usegamma > 17) usegamma = 17;
    
    if (color_saturation < 0) color_saturation = 0;
    if (color_saturation > 1) color_saturation = 1;
    
    if (r_color_factor < 0) r_color_factor = 0;
    if (r_color_factor > 1) r_color_factor = 1;
    if (g_color_factor < 0) g_color_factor = 0;
    if (g_color_factor > 1) g_color_factor = 1;
    if (b_color_factor < 0) b_color_factor = 0;
    if (b_color_factor > 1) b_color_factor = 1;

    for (i=0; i<256; ++i)
    {
        // [JN] Extended palette values generation routine.
        // Based on implementation from DOOM Retro.
        byte    *gamma = gammatable[usegamma];

        byte    r = gamma[*doompalette++];
        byte    g = gamma[*doompalette++];
        byte    b = gamma[*doompalette++];
        double  p = sqrt(r * r * 0.299 + g * g * 0.587 + b * b * 0.114);

        palette[i].r = (byte)((p + (r - p) * color_saturation) * r_color_factor ) * brightness;
        palette[i].g = (byte)((p + (g - p) * color_saturation) * g_color_factor ) * brightness;
        palette[i].b = (byte)((p + (b - p) * color_saturation) * b_color_factor ) * brightness;
    }

    palette_to_set = true;
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;

    best = 0; best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - palette[i].r) * (r - palette[i].r)
             + (g - palette[i].g) * (g - palette[i].g)
             + (b - palette[i].b) * (b - palette[i].b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

// 
// Set the window title
//

void I_SetWindowTitle(char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on 
// the title set with I_SetWindowTitle.
//

void I_InitWindowTitle(void)
{
    char *buf;

    buf = M_StringJoin(window_title, window_title_short ? 
                       NULL : " - ", RD_Project_String, NULL);
    SDL_SetWindowTitle(screen, buf);
    free(buf);
}

// Set the application icon

void I_InitWindowIcon(void)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) icon_data, icon_w, icon_h,
                                       32, icon_w * 4,
                                       0xff << 24, 0xff << 16,
                                       0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(screen, surface);
    SDL_FreeSurface(surface);
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static void SetScaleFactor(int factor)
{
    // Pick 320x200 or 320x240, depending on aspect ratio correct

    window_width = factor * screenwidth;
    window_height = factor * actualheight;
    fullscreen = false;
}

void I_GraphicsCheckCommandLine(void)
{
    int i;

    //!
    // @category video
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_CheckParm ("-noblit");

    //!
    // @category video 
    //
    // Don't grab the mouse when running in windowed mode.
    //

    nograbmouse_override = M_ParmExists("-nograbmouse");

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    //!
    // @category video 
    //
    // Run in a window.
    //

    if (M_CheckParm("-window") || M_CheckParm("-nofullscreen"))
    {
        fullscreen = false;
    }

    //!
    // @category video 
    //
    // Run in fullscreen mode.
    //

    if (M_CheckParm("-fullscreen"))
    {
        fullscreen = true;
    }

    //!
    // @category video 
    //
    // Disable the mouse.
    //

    nomouse = M_CheckParm("-nomouse") > 0;

    //!
    // @category video
    // @arg <x>
    //
    // Specify the screen width, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        window_width = atoi(myargv[i + 1]);
        window_height = window_width * 2;
        AdjustWindowSize();
        fullscreen = false;
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        window_height = atoi(myargv[i + 1]);
        window_width = window_height * 2;
        AdjustWindowSize();
        fullscreen = false;
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the dimensions of the window. Implies -window.
    //

    i = M_CheckParmWithArgs("-geometry", 1);

    if (i > 0)
    {
        int w, h, s;

        s = sscanf(myargv[i + 1], "%ix%i", &w, &h);
        if (s == 2)
        {
            window_width = w;
            window_height = h;
            fullscreen = false;
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen. Implies -window.
    //

    if (M_CheckParm("-1")) 
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size. Implies -window.
    //

    if (M_CheckParm("-2")) 
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size. Implies -window.
    //

    if (M_CheckParm("-3")) 
    {
        SetScaleFactor(3);
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
    char *env;

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

static void SetSDLVideoDriver(void)
{
    // Allow a default value for the SDL video driver to be specified
    // in the configuration file.

    if (strcmp(video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", video_driver, NULL);
        putenv(env_string);
        free(env_string);
    }
}

// Check the display bounds of the display referred to by 'video_display' and
// set x and y to a location that places the window in the center of that
// display.
static void CenterWindow(int *x, int *y, int w, int h)
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
                        "for display #%d!\n", video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

#ifdef WIN32
extern void DisableWinRound(SDL_Window* screen);
#endif

static void SetVideoMode(void)
{
    int w, h;
    int x = 0, y = 0;
    unsigned int rmask, gmask, bmask, amask;
    int bpp;
    int window_flags = 0, renderer_flags = 0;
    SDL_DisplayMode mode;

    w = window_width;
    h = window_height;

    // In windowed mode, the window can be resized while the game is
    // running.
    window_flags = SDL_WINDOW_RESIZABLE;

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

#ifdef _WIN32
    // [JN] Windows 11 idiocy. Indicate that window using OpenGL mode (while it's
    // a Direct3D in fact), so SDL texture will not be freezed upon vsync toggling.
    if (!force_software_renderer)
    {
        window_flags |= SDL_WINDOW_OPENGL;
    }
#endif

    if (fullscreen)
    {
        if (fullscreen_width == 0 && fullscreen_height == 0)
        {
            // This window_flags means "Never change the screen resolution!
            // Instead, draw to the entire screen by scaling the texture
            // appropriately".
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            w = fullscreen_width;
            h = fullscreen_height;
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    
    // in fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run fullscreen on.
    if (fullscreen)
    {
        CenterWindow(&x, &y, w, h);
    }

    // [JN] If window X and Y coords was not set,
    // place game window in the center of the screen.
    if (window_position_x == 0 || window_position_y == 0)
    {
        window_position_x = x/2 + w/2;
        window_position_y = y/2 + h/2;
    }

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If
    // "window_flags" contains the fullscreen flag (see above), then
    // w and h are ignored.

    if (screen == NULL)
    {
        // [JN] If we are running in borderless mode, 
        // place game window in the top-left corner of the screen.
        screen = SDL_CreateWindow(NULL, window_border == 0 ? 0 : window_position_x,
                                        window_border == 0 ? 0 : window_position_y,
                                        w, h, window_flags);

        if (screen == NULL)
        {
            I_Error(english_language ?
                    "Error creating window for video startup: %s" :
                    "Ошибка создания окна для видео-загрузки: %s",
            SDL_GetError());
        }

#ifdef WIN32
        DisableWinRound(screen);
#endif

        pixel_format = SDL_GetWindowPixelFormat(screen);

        // [JN] Allow game window to be downscaled to 1:1 pixel size.
        SDL_SetWindowMinimumSize(screen, screenwidth >> hires, actualheight >> hires);

        I_InitWindowTitle();
        I_InitWindowIcon();
    }

    // [JN] Disable window border if window_border = 0.
    if (!window_border)
    {
        SDL_SetWindowBordered(screen, SDL_FALSE);
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.
    renderer_flags = SDL_RENDERER_TARGETTEXTURE;

    if (SDL_GetCurrentDisplayMode(video_display, &mode) != 0)
    {
        I_Error("Could not get display mode for video display #%d: %s",
        video_display, SDL_GetError());
    }

    // Turn on vsync if we aren't in a -timedemo
    // In -timedemo mode it's always disabled to get a maximum possible fps.
    if (!singletics && mode.refresh_rate > 0)
    {
        if (vsync)
        {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
    }

    // [JN] Note: vsync is always disabled in software rendering mode.
    if (force_software_renderer)
    {
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        vsync = false;
    }

    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
        // all associated textures get destroyed
        texture = NULL;
        texture_upscaled = NULL;
    }

    renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

    // If we could not find a matching render driver,
    // try again without hardware acceleration.

    if (renderer == NULL && !force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

        // If this helped, save the setting for later.
        if (renderer != NULL)
        {
            force_software_renderer = 1;
        }
    }

    if (renderer == NULL)
    {
        I_Error(english_language ?
                "Error creating renderer for screen window: %s" :
                "Ошибка создания рендеринга для окна: %s",
                SDL_GetError());
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    if (aspect_ratio_correct || integer_scaling)
    {
        SDL_RenderSetLogicalSize(renderer,
                                screenwidth,
                                actualheight);
    }
                             
    // Force integer scales for resolution-independent rendering.
    
    SDL_RenderSetIntegerScale(renderer, integer_scaling);

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Create the 8-bit paletted and the 32-bit RGBA screenbuffer surfaces.

    if (screenbuffer != NULL)
    {
        SDL_FreeSurface(screenbuffer);
        screenbuffer = NULL;
    }

    if (screenbuffer == NULL)
    {
        screenbuffer = SDL_CreateRGBSurface(0,
                                            screenwidth, SCREENHEIGHT, 8,
                                            0, 0, 0, 0);
        SDL_FillRect(screenbuffer, NULL, 0);
    }

    // Format of rgbabuffer must match the screen pixel format because we
    // import the surface data into the texture.

    if (argbbuffer != NULL)
    {
        SDL_FreeSurface(argbbuffer);
        argbbuffer = NULL;
    }

    if (argbbuffer == NULL)
    {
        SDL_PixelFormatEnumToMasks(pixel_format, &bpp,
                                   &rmask, &gmask, &bmask, &amask);
        argbbuffer = SDL_CreateRGBSurface(0,
                                          screenwidth, SCREENHEIGHT, bpp,
                                          rmask, gmask, bmask, amask);
        SDL_FillRect(argbbuffer, NULL, 0);
    }

    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
                                pixel_format,
                                SDL_TEXTUREACCESS_STREAMING,
                                screenwidth, SCREENHEIGHT);

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture(true);
}

void I_InitGraphics(void)
{
    SDL_Event dummy;
    byte *doompal;
    char *env;

    // Pass through the XSCREENSAVER_WINDOW environment variable to 
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        char winenv[30];
        unsigned int winid;

        sscanf(env, "0x%x", &winid);
        M_snprintf(winenv, sizeof(winenv), "SDL_WINDOWID=%i", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();

    // [JN] Set an event watcher for window resize to allow
    // update window contents on fly.
    SDL_AddEventWatch(HandleWindowResize, screen);

    // [JN] Set correct capped/uncapped mode at startup.
    if (max_fps == 35)
    {
        uncapped_fps = 0;
    }
    else
    {
        uncapped_fps = 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error(english_language ?
                "Failed to initialize video: %s" :
                "Ошибка инициализации видео: %s",
        SDL_GetError());
    }

    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    if (screensaver_mode)
    {
        fullscreen = true;
    }

    // Create the game window; this may switch graphic modes depending
    // on configuration.
    AdjustWindowSize();
    SetVideoMode();

    // [JN] Set window hint with a high priority.
    // Fixes not working Win-key combinations on SDL 2.0.14.
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS,
                            "1", SDL_HINT_OVERRIDE);

    // [JN] Initialize and generate gamma-correction levels.

    I_InitGammaTables();

    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    SDL_FillRect(screenbuffer, NULL, 0);

    // Set the palette

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);
    SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);

    // SDL2-TODO UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (fullscreen && !screensaver_mode)
    {
        SDL_Delay(startup_delay);
    }

    // The actual 320x200 canvas that we draw to. This is the pixel buffer of
    // the 8-bit paletted screen buffer that gets blit on an intermediate
    // 32-bit RGBA screen buffer that gets loaded into a texture that gets
    // finally rendered into our window or full screen in I_FinishUpdate().

    I_VideoBuffer = screenbuffer->pixels;
    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, screenwidth * SCREENHEIGHT * sizeof(*I_VideoBuffer));

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));

    initialized = true;
}

// [crispy] re-initialize only the parts of the rendering stack that are really necessary

void I_ReInitGraphics (int reinit)
{
	// [crispy] re-set rendering resolution and re-create framebuffers
	if (reinit & REINIT_FRAMEBUFFERS)
	{
		unsigned int rmask, gmask, bmask, amask;
		int unused_bpp;

		// [crispy] re-initialize resolution-agnostic patch drawing
		V_Init();

		SDL_FreeSurface(screenbuffer);
		screenbuffer = SDL_CreateRGBSurface(0,
				                    screenwidth, SCREENHEIGHT, 8,
				                    0, 0, 0, 0);

		SDL_FreeSurface(argbbuffer);
		SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
		                           &rmask, &gmask, &bmask, &amask);
		argbbuffer = SDL_CreateRGBSurface(0,
		                                  screenwidth, SCREENHEIGHT, 32,
		                                  rmask, gmask, bmask, amask);

		I_VideoBuffer = screenbuffer->pixels;

		V_RestoreBuffer();

		// [crispy] it will get re-created below with the new resolution
		SDL_DestroyTexture(texture);
	}

	// [crispy] re-create renderer
	if (reinit & REINIT_RENDERER)
	{
		SDL_RendererInfo info = {0};
		int flags;

		SDL_GetRendererInfo(renderer, &info);
		flags = info.flags;

		if (vsync && !(flags & SDL_RENDERER_SOFTWARE))
		{
			flags |= SDL_RENDERER_PRESENTVSYNC;
		}
		else
		{
			flags &= ~SDL_RENDERER_PRESENTVSYNC;
		}

		SDL_DestroyRenderer(renderer);
		renderer = SDL_CreateRenderer(screen, -1, flags);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// [crispy] the texture gets destroyed in SDL_DestroyRenderer(), force its re-creation
		texture_upscaled = NULL;
	}

	// [crispy] re-create textures
	if (reinit & REINIT_TEXTURES)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

		texture = SDL_CreateTexture(renderer,
		                            pixel_format,
		                            SDL_TEXTUREACCESS_STREAMING,
		                            screenwidth, SCREENHEIGHT);

		// [crispy] force its re-creation
		CreateUpscaledTexture(true);
	}

	// [crispy] re-set logical rendering resolution
	if (reinit & REINIT_ASPECTRATIO)
	{
		if (aspect_ratio_correct == 1)
		{
 			if (aspect_ratio == 1)
 			actualheight = SCREENHEIGHT_5_4;
 			else
			actualheight = SCREENHEIGHT_4_3;
		}
		else
		{
			actualheight = SCREENHEIGHT;
		}

		if (aspect_ratio_correct || integer_scaling)
		{
			SDL_RenderSetLogicalSize(renderer,
			                         screenwidth,
			                         actualheight);
		}
		else
		{
			SDL_RenderSetLogicalSize(renderer, 0, 0);
		}

		#if SDL_VERSION_ATLEAST(2, 0, 5)
		SDL_RenderSetIntegerScale(renderer, integer_scaling);
		#endif
	}

	// [crispy] adjust the window size and re-set the palette
	need_resize = true;
}

void I_RenderReadPixels(byte **data, int *w, int *h, int *p)
{
	SDL_Rect rect;
	SDL_PixelFormat *format;
	int temp;
	uint32_t png_format;
	byte *pixels;

	// [crispy] adjust cropping rectangle if necessary
	rect.x = rect.y = 0;
	SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
	if (aspect_ratio_correct || integer_scaling)
	{
		if (integer_scaling)
		{
			int temp1, temp2, scale;
			temp1 = rect.w;
			temp2 = rect.h;
			scale = MIN(rect.w / screenwidth, rect.h / actualheight);

			rect.w = screenwidth * scale;
			rect.h = actualheight * scale;

			rect.x = (temp1 - rect.w) / 2;
			rect.y = (temp2 - rect.h) / 2;
		}
		else
		if (rect.w * actualheight > rect.h * screenwidth)
		{
			temp = rect.w;
			rect.w = rect.h * screenwidth / actualheight;
			rect.x = (temp - rect.w) / 2;
		}
		else
		if (rect.h * screenwidth > rect.w * actualheight)
		{
			temp = rect.h;
			rect.h = rect.w * actualheight / screenwidth;
			rect.y = (temp - rect.h) / 2;
		}
	}

	// [crispy] native PNG pixel format
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	png_format = SDL_PIXELFORMAT_ABGR8888;
#else
	png_format = SDL_PIXELFORMAT_RGBA8888;
#endif
	format = SDL_AllocFormat(png_format);
	temp = rect.w * format->BytesPerPixel; // [crispy] pitch

	// [crispy] allocate memory for screenshot image
	pixels = malloc(rect.h * temp);
	SDL_RenderReadPixels(renderer, &rect, format->format, pixels, temp);

	*data = pixels;
	*w = rect.w;
	*h = rect.h;
	*p = temp;

	SDL_FreeFormat(format);
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("use_mouse",                 &usemouse);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("aspect_ratio",              &aspect_ratio);
    M_BindIntVariable("video_display",             &video_display);
    M_BindIntVariable("vsync",                     &vsync);
    M_BindIntVariable("show_fps",                  &show_fps);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("smoothing",                 &smoothing);
    M_BindIntVariable("max_fps",                   &max_fps);
    M_BindIntVariable("vga_porch_flash",           &vga_porch_flash);
    M_BindIntVariable("integer_scaling",           &integer_scaling);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("resize_delay",              &resize_delay);
    M_BindIntVariable("fullscreen_width",          &fullscreen_width);
    M_BindIntVariable("fullscreen_height",         &fullscreen_height);
    M_BindIntVariable("window_title_short",        &window_title_short);
    M_BindIntVariable("window_width",              &window_width);
    M_BindIntVariable("window_height",             &window_height);
    M_BindIntVariable("window_border",             &window_border);
    M_BindIntVariable("grabmouse",                 &grabmouse);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindIntVariable("window_position_x",         &window_position_x);
    M_BindIntVariable("window_position_y",         &window_position_y);
    M_BindIntVariable("png_screenshots",           &png_screenshots);

    // Color options
    M_BindFloatVariable("brightness",              &brightness);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindFloatVariable("color_saturation",        &color_saturation);
    M_BindIntVariable("show_palette",              &show_palette);
    M_BindFloatVariable("r_color_factor",          &r_color_factor);
    M_BindFloatVariable("g_color_factor",          &g_color_factor);
    M_BindFloatVariable("b_color_factor",          &b_color_factor);
}


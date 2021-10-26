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



#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"
#include "mode.h"

#include "display.h"
#include "config.h"

#define WINDOW_HELP_URL "https://jnechaevsky.github.io/projects/rusdoom/setup/index.html"

extern void RestartTextscreen(void);

typedef struct
{
    int w, h;
} window_size_t;

// [JN] List of available aspect ratios
static char *aspect_ratios[] = 
{
    "4:3",
    "5:4",
    "16:9",
    "16:10",
    "21:9"
};

// List of aspect ratio-corrected window sizes:
static window_size_t window_sizes_scaled[] =
{
    { 640,  480 },
    { 800,  600 },
    { 960,  720 },
    { 1024, 768 },
    { 1280, 960 },
    { 1440, 1080 },
    { 1600, 1200 },
    { 1920, 1440 },
    { 0, 0},
};

// [JN] List of 5:4 aspect ratio window sizes:
static window_size_t window_sizes_5_4[] =
{
    { 640,  512 },
    { 800,  640 },
    { 960,  768 },
    { 1024, 820 },
    { 1280, 1024 },
    { 1440, 1152 },
    { 1600, 1280 },
    { 1920, 1536 },
    { 0, 0},
};

// [JN] List of 16:9 aspect ratio window sizes:
static window_size_t window_sizes_16_9[] =
{
    { 852,  480 },
    { 960,  540 },
    { 1024, 576 },
    { 1280, 720 },
    { 1600, 900 },
    { 1920, 1080 },
    { 0, 0},
};

// [JN] List of 16:10 aspect ratio window sizes:
static window_size_t window_sizes_16_10[] =
{
    { 768,  480 },
    { 864,  540 },
    { 920,  576 },
    { 1152, 720 },
    { 1440, 900 },
    { 1728, 1080 },
    { 0, 0},
};

// [JN] List of 21:9 aspect ratio window sizes:
static window_size_t window_sizes_21_9[] =
{
    { 1136,  480 },
    { 1276,  540 },
    { 1368,  576 },
    { 1702,  720 },
    { 2130,  900 },
    { 2556,  1080 },
    { 0, 0},
};

static char *video_driver = "";
static int aspect_ratio_correct = 1;
static int smoothing = 0;
static int vga_porch_flash = 0;
static int integer_scaling = 0;
static int fullscreen = 1;
static int aspect_ratio = 2;
static int fullscreen_width = 0, fullscreen_height = 0;
static int window_title_short = 1;
static int window_width = 640, window_height = 480;
static int window_border = 1;
static int startup_delay = 35;  // [JN] Redused from 1000 to 35
static int resize_delay = 70;   // [JN] Redused from 500 to 70
static int usegamma = 4; // [JN] Set default gamma to improved level 2.0.

int uncapped_fps = 1;
int vsync = 1;
int show_fps = 0;
int show_diskicon = 1;
int graphical_startup = 0; // [JN] Disabled by default
int screen_wiping = 1;
int show_endoom = 0;
int png_screenshots = 1;   // [JN] Crispy!
int flashing_hom = 0;

static int system_video_env_set;

// Set the SDL_VIDEODRIVER environment variable

void SetDisplayDriver(void)
{
    static int first_time = 1;

    if (first_time)
    {
        system_video_env_set = getenv("SDL_VIDEODRIVER") != NULL;

        first_time = 0;
    }

    // Don't override the command line environment, if it has been set.

    if (system_video_env_set)
    {
        return;
    }

    // Use the value from the configuration file, if it has been set.

    if (strcmp(video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", video_driver, NULL);
        putenv(env_string);
        free(env_string);
    }
}

static void WindowSizeSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(size))
{
    TXT_CAST_ARG(window_size_t, size);

    window_width = size->w;
    window_height = size->h;
}

static txt_radiobutton_t *SizeSelectButton(window_size_t *size)
{
    char buf[15];
    txt_radiobutton_t *result;

    M_snprintf(buf, sizeof(buf), "%ix%i", size->w, size->h);
    result = TXT_NewRadioButton(buf, &window_width, size->w);
    TXT_SignalConnect(result, "selected", WindowSizeSelected, size);

    return result;
}

static void GenerateSizesTable(TXT_UNCAST_ARG(widget),
                               TXT_UNCAST_ARG(sizes_table))
{
    TXT_CAST_ARG(txt_table_t, sizes_table);
    window_size_t *sizes;
    boolean have_size;
    int i;

    // Pick which window sizes list to use
    if (aspect_ratio == 1)
    {
        sizes = window_sizes_5_4;
    }
    else if (aspect_ratio == 2)
    {
        sizes = window_sizes_16_9;
    }
    else if (aspect_ratio == 3)
    {
        sizes = window_sizes_16_10;
    }
    else if (aspect_ratio == 4)
    {
        sizes = window_sizes_21_9;
    }
    else
    {
        sizes = window_sizes_scaled;
    }

    // Build the table
    TXT_ClearTable(sizes_table);
    TXT_SetColumnWidths(sizes_table, 14, 14, 14);

    TXT_AddWidget(sizes_table, TXT_NewSeparator(english_language ? 
                                                "Window size" :
                                                "������ ����"));

    have_size = false;

    for (i = 0; sizes[i].w != 0; ++i)
    {
        TXT_AddWidget(sizes_table, SizeSelectButton(&sizes[i]));
        have_size = have_size || window_width == sizes[i].w;
    }

    // Windows can be any arbitrary size. We key off the width of the
    // window in pixels. If the current size is not in the list of
    // standard (integer multiply) sizes, create a special button to
    // mean "the current window size".
    if (!have_size)
    {
        static window_size_t current_size;
        current_size.w = window_width;
        current_size.h = window_height;
        TXT_AddWidget(sizes_table, SizeSelectButton(&current_size));
    }
}

static void AdvancedDisplayConfig(TXT_UNCAST_ARG(widget),
                                  TXT_UNCAST_ARG(sizes_table))
{
    // TXT_CAST_ARG(txt_table_t, sizes_table);
    txt_window_t *window;
    txt_checkbox_t *ar_checkbox;

    window = TXT_NewWindow(english_language ?
                           "Advanced display options" :
                           "������ ���������");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 40);

    TXT_AddWidgets(window,

        ar_checkbox =
        TXT_NewCheckBox(english_language ?
                        "Draw window border" :
                        "���������� ����� ����",
                        &window_border),
        
        TXT_If(gamemission == heretic || gamemission == hexen,
        TXT_NewCheckBox(english_language ?
                        "Graphical startup" :
                        "����������� ��������",
                        &graphical_startup)),

        TXT_If(gamemission == doom || gamemission == heretic,
        TXT_NewCheckBox(english_language ?
                        "Show ENDOOM screen on exit" :
                        "���������� ����� ENDOOM ��� ������",
                        &show_endoom)),

/*
#if SDL_VERSION_ATLEAST(2, 0, 5)
        TXT_NewCheckBox("������������� ��������������� ����", &integer_scaling),
#endif
*/

        NULL);

    //
    // [JN] Create translated buttons
    //

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, english_language ?
                        TXT_NewWindowAbortAction(window) :
                        TXT_NewWindowAbortAction_Rus(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, english_language ?
                        TXT_NewWindowSelectAction(window) :
                        TXT_NewWindowSelectAction_Rus(window));
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *sizes_table;
    txt_window_action_t *advanced_button;
    txt_checkbox_t *ar_checkbox;
    txt_dropdown_list_t *cc_dropdown;

    // Open the window

    window = TXT_NewWindow(english_language ?
                           "Display Configuration" :
                           "��������� ������");

    if (english_language)
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);
    else
    TXT_SetWindowHelpURL_RUS(window, WINDOW_HELP_URL);

    // Build window:

    TXT_AddWidgets(window,

    TXT_NewSeparator(english_language ?
                     "Rendering" :
                     "���������"),

        TXT_NewHorizBox(TXT_NewStrut(4, 0),
                        TXT_NewLabel(english_language ?
                        "Display aspect ratio: ":
                        "����������� ������ ������: "),
                        cc_dropdown = TXT_NewDropdownList(&aspect_ratio,
                                                          aspect_ratios, 5),
                        NULL),

        ar_checkbox = 

        TXT_NewCheckBox(english_language ?
                        "Vertical sync" :
                        "������������ �������������",
                        &vsync),
        TXT_NewCheckBox(english_language ?
                        "Uncapped framerate" :
                        "����� ����������� � �������� �������",
                        &uncapped_fps),
        TXT_If(gamemission == doom,
                TXT_NewCheckBox(english_language ?
                        "Show fps counter" :
                        "������� �������� �������",
                        &show_fps)),
        TXT_NewCheckBox(english_language ?
                        "Smooth pixel scaling" :
                        "���������� �����������",
                        &smoothing),

  TXT_NewSeparator(english_language ?
                   "Extra" :
                   "�������������"),

      TXT_If(gamemission == doom,
              TXT_NewCheckBox(english_language ?
                      "Show Disk icon" :
                      "���������� ������ �������",
                      &show_diskicon)),
      TXT_If(gamemission == heretic || gamemission == hexen || gamemission == strife,
          TXT_NewCheckBox(english_language ?
                          "Graphical startup" :
                          "����������� ��������",
                          &graphical_startup)),

#ifdef HAVE_LIBPNG
        TXT_NewCheckBox(english_language ?
                        "Save screenshots in PNG format" :
                        "��������� ��������� � ������� PNG",
                        &png_screenshots),
#endif

    TXT_NewSeparator(english_language ?
                     "Video" :
                     "�����"),
            
        TXT_NewCheckBox(english_language ?
                        "Full screen" :
                        "������������� �����",
                        &fullscreen),
        TXT_NewConditional(&fullscreen, 0,
            sizes_table = TXT_NewTable(3)),
        NULL);

    TXT_SetColumnWidths(window, 42);    
    
    // The window is set at a fixed vertical position.  This keeps
    // the top of the window stationary when switching between
    // fullscreen and windowed mode (which causes the window's
    // height to change).
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 
                                  (gamemission == heretic || gamemission == hexen ? 
                                  2 : 1));

    GenerateSizesTable(NULL, sizes_table);

    TXT_SignalConnect(ar_checkbox, "changed", GenerateSizesTable, sizes_table);

    // [JN] Re-generate window sizes table after changing aspect ratio.
    TXT_SignalConnect(cc_dropdown, "changed", GenerateSizesTable, sizes_table);

    //    Button to open "advanced" window.
    // Need to pass a pointer to the window sizes table, as some of the options
    // in there trigger a rebuild of it.

    advanced_button = TXT_NewWindowAction('a', english_language ?
                                               "Advanced" :
                                               "������");

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, advanced_button);
    TXT_SignalConnect(advanced_button, "pressed",
                      AdvancedDisplayConfig, sizes_table);

    //
    // [JN] Create translated buttons
    //

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, english_language ?
                        TXT_NewWindowAbortAction(window) :
                        TXT_NewWindowAbortAction_Rus(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, english_language ?
                        TXT_NewWindowSelectAction(window) :
                        TXT_NewWindowSelectAction_Rus(window));
}

void BindDisplayVariables(void)
{
    M_BindIntVariable("vsync",                     &vsync);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("uncapped_fps",              &uncapped_fps);    
    M_BindIntVariable("show_fps",                  &show_fps);
    M_BindIntVariable("smoothing",                 &smoothing);
    M_BindIntVariable("show_diskicon",             &show_diskicon);
    M_BindIntVariable("screen_wiping",             &screen_wiping);
    M_BindIntVariable("png_screenshots",           &png_screenshots);
    M_BindIntVariable("flashing_hom",              &flashing_hom);

    M_BindIntVariable("vga_porch_flash",           &vga_porch_flash);
    M_BindIntVariable("integer_scaling",           &integer_scaling);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("aspect_ratio",              &aspect_ratio);
    M_BindIntVariable("fullscreen_width",          &fullscreen_width);
    M_BindIntVariable("fullscreen_height",         &fullscreen_height);
    M_BindIntVariable("window_border",             &window_border);
    M_BindIntVariable("window_title_short",        &window_title_short);
    M_BindIntVariable("window_width",              &window_width);
    M_BindIntVariable("window_height",             &window_height);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("resize_delay",              &resize_delay);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindIntVariable("usegamma",                  &usegamma);

    if (gamemission == doom || gamemission == heretic || gamemission == strife)
    {
        M_BindIntVariable("show_endoom",           &show_endoom);
    }

    if (gamemission == heretic || gamemission == hexen || gamemission == strife)
    {
        M_BindIntVariable("graphical_startup",     &graphical_startup);
    }
}


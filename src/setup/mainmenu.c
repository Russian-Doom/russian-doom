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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winnls.h> // [JN] GetSystemDefaultLCID()
#endif

#include "config.h"
#include "textscreen.h"

#include "execute.h"

#include "d_name.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "z_zone.h"

#include "icon.h"
#include "mode.h"

#include "compatibility.h"
#include "display.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://jnechaevsky.github.io/projects/rusdoom/setup/index.html"

// -----------------------------------------------------------------------------
// [Dasperal] d_name.h var definition
// -----------------------------------------------------------------------------
char* RD_Project_Name = PACKAGE_PREFIX " Setup";
char* RD_Project_String = PACKAGE_PREFIX " Setup " BUILD_DOOM_VERSION;
GameType_t RD_GameType = gt_Doom;

//
// [JN] Prototype for Main Menu
//
void MainMenu(void);

//
// [JN] Language switch
//
static void LanguageSelect(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    // Switch to next language on invoking
    english_language++;

    // More than 1 (English)? Jump back to 0 (Russian)
    if (english_language > 1)
        english_language = 0;

    // Redraw Main Menu
    MainMenu();
}

static const int cheat_sequence[] =
{
    KEY_UPARROW, KEY_UPARROW, KEY_DOWNARROW, KEY_DOWNARROW,
    KEY_LEFTARROW, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    'b', 'a', KEY_ENTER, 0
};

static unsigned int cheat_sequence_index = 0;

// I think these are good "sensible" defaults:

static void SensibleDefaults(void)
{
    snd_musicdevice = 3;
    graphical_startup = 0;
    show_endoom = 0;
    novert = 1;
    mouse_y_invert = 0; // [crispy]
    snd_dmxoption = "-opl3 -reverse";
    png_screenshots = 1;
    flashing_hom = 0;

    // Rendering
    vsync = 1;
    show_fps = 0;
    show_diskicon = 1;

    // Display
    level_brightness = 0;
    menu_shading = 0;
    hud_detaillevel = 0;

    // Automap
    automap_color = 0;
    automap_antialias = 1;
    automap_stats = 1;
    automap_level_time = 1;
    automap_total_time = 0;
    automap_coords = 0;
    automap_follow = 1;
    automap_overlay = 0;
    automap_rotate = 0;
    automap_grid = 0;
    automap_grid_size = 128;
    hud_widget_colors = 0;

    // Gameplay: Graphical
    brightmaps = 1;
    fake_contrast = 0;
    translucency = 1;
    improved_fuzz = 2;
    messages_alignment = 0;
    messages_timeout = 4;
    message_pickup_color = 0;
    message_secret_color = 7;
    message_system_color = 0;
    message_chat_color = 2;
    colored_blood = 1;
    swirling_liquids = 1;
    invul_sky = 1;
    linear_sky = 1;
    flip_weapons = 0;
    draw_shadowed_text = 1;

    // Gameplay: Status Bar
    extra_player_faces = 1;
    negative_health = 0;
    sbar_colored = 0;
    sbar_colored_gem = 0;
    sbar_color_high = 5;
    sbar_color_normal = 2;
    sbar_color_low = 7;
    sbar_color_critical = 0;
    sbar_color_armor_1 = 2;
    sbar_color_armor_2 = 5;
    sbar_color_armor_0 = 0;
    ammo_widget = 0;
    ammo_widget_colored = 1;

    // Gameplay: Audible
    z_axis_sfx = 0;
    play_exit_sfx = 0;
    crushed_corpses_sfx = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx = 0;
    correct_endlevel_sfx = 0;

    // Gameplay: Tactical
    secret_notification = 1;
    infragreen_visor = 0;
    show_all_artifacts = 0;
    show_artifacts_timer = 0;
    negative_health = 0;

    // Gameplay: Physical
    over_under = 0;
    torque = 1;
    weapon_bobbing = 1;
    ssg_blast_enemies = 1;
    randomly_flipcorpses = 1;
    floating_powerups = 1;
    toss_drop = 1;

    // Gameplay: Crosshair
    crosshair_draw = 0;
    crosshair_type = 1;
    crosshair_scale = 0;

    // Gameplay: Gameplay
    fix_map_errors = 1;
    flip_levels = 0;
    unlimited_lost_souls = 1;
    agressive_lost_souls = 0;
    pistol_start = 0;
    fast_quickload = 1;

    // Gameplay: Demos
    demotimer = 0;
    demotimerdir = 0;
    demobar = 0;
    no_internal_demos = 0;
}

static int MainMenuKeyPress(txt_window_t *window, int key, void *user_data)
{
    if (key == cheat_sequence[cheat_sequence_index])
    {
        ++cheat_sequence_index;

        if (cheat_sequence[cheat_sequence_index] == 0)
        {
            SensibleDefaults();
            cheat_sequence_index = 0;

            window = TXT_MessageBox(NULL, "    \x01    ");

            return 1;
        }
    }
    else
    {
        cheat_sequence_index = 0;
    }

    return 0;
}

static void DoQuit(void *widget, void *dosave)
{
    if (dosave != NULL)
    {
        M_SaveConfig();
    }

    TXT_Shutdown();

    exit(0);
}

static void QuitConfirm(void *unused1, void *unused2)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_button_t *yes_button;
    txt_button_t *no_button;

    window = TXT_NewWindow(NULL);

    TXT_AddWidgets(window, 
                   label = TXT_NewLabel(english_language ?
                                        "Exiting setup.\nSave settings?" :
                                        "Выход из программы настроек.\nСохранить внесенные изменени€?"),
                   TXT_NewStrut(24, 0),
                   yes_button = TXT_NewButton2(english_language ?
                                               "  Yes  " :
                                               " Сохранить    ",
                                               DoQuit, DoQuit),
                   no_button = TXT_NewButton2(english_language ?
                                              "  No   " :
                                              " Не сохран€ть ",
                                              DoQuit, NULL),
                   NULL);

    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button, TXT_HORIZ_CENTER);

    //
    // [JN] Create translated button ("Cancel" only)
    //

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, english_language ?
                        TXT_NewWindowAbortAction(window) :
                        TXT_NewWindowAbortAction_Rus(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
}

static void LaunchDoom(void *unused1, void *unused2)
{
    execute_context_t *exec;

    // Save configuration first

    M_SaveConfig();

    // Shut down textscreen GUI

    TXT_Shutdown();

    // Launch Doom

    exec = NewExecuteContext();
    PassThroughArguments(exec);
    ExecuteDoom(exec);

    exit(0);
}

static txt_button_t *GetLaunchButton(void)
{
    char *label;

    switch (gamemission)
        {
        case doom:
            label = english_language ?
                    "Save parameters and launch DOOM     " :
                    "Сохранить настройки и запустить DOOM";
            break;
        case heretic:
            label = english_language ?
                    "Save parameters and launch Heretic     " :
                    "Сохранить настройки и запустить Heretic";
            break;
        case hexen:
            label = english_language ?
                    "Save parameters and launch Hexen     " :
                    "Сохранить настройки и запустить Hexen";
            break;
        case strife:
            label = english_language ?
                    "Save parameters and launch STRIFE!     " :
                    "Сохранить настройки и запустить STRIFE!";
            break;
        default:
            label = english_language ?
                    "Save parameters and launch game     " :
                    "Сохранить настройки и запустить игру";
            break;
        }

    return TXT_NewButton2(label, LaunchDoom, NULL);
}

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_window_action_t *warp_action;
    txt_window_action_t *lang_action;

    window = TXT_NewWindow(english_language ?
                           "Main Menu" :
                           "Главное меню");

    if (english_language)
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);
    else
    TXT_SetWindowHelpURL_RUS(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
    TXT_NewButton2(english_language ?
                   "Configure Display" : "Настройки экрана",
                   (TxtWidgetSignalFunc) ConfigDisplay, NULL),
    TXT_NewButton2(english_language ?
                   "Configure Sound" :
                   "Настройки звука",
                   (TxtWidgetSignalFunc) ConfigSound, NULL),
    TXT_NewButton2(english_language ?
                   "Configure Mouse" :
                   "Настройки мыши",
                   (TxtWidgetSignalFunc) ConfigMouse, NULL),
    // [JN] No longer used, now using in-game menu
    /*
    TXT_NewButton2(english_language ?
                   "Optional Gameplay Enhacements" :
                   "Дополнительные параметры игры",
                   (TxtWidgetSignalFunc) CompatibilitySettings, NULL),
    */
    GetLaunchButton(),
    TXT_NewStrut(0, 1),
    TXT_NewButton2(english_language ?
                   "Start a Network Game" :
                   "Начать сетевую игру",
                   (TxtWidgetSignalFunc) StartMultiGame, NULL),
    TXT_NewButton2(english_language ?
                   "Join a Network Game" :
                   "Присоединитьс€ к сетевой игре",
                   (TxtWidgetSignalFunc) JoinMultiGame, NULL),
    TXT_NewButton2(english_language ?
                   "Multiplayer Configuration" :
                   "Настройки сетевой игры",
                   (TxtWidgetSignalFunc) MultiplayerConfig, NULL),
    TXT_NewStrut(0, 1),
    NULL);

    //
    // [JN] ESC = Quit, F2 = Warp, F4 = Language selection
    //

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, english_language ? "Quit" : "Выход");
    warp_action = TXT_NewWindowAction(KEY_F2,     english_language ? "Warp" : "Уровень");
    lang_action = TXT_NewWindowActionY(KEY_F4,    english_language ? "Русский" : "English");


    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, NULL);
    TXT_SignalConnect(warp_action, "pressed", (TxtWidgetSignalFunc) WarpMenu, NULL);
    TXT_SignalConnect(lang_action, "pressed", LanguageSelect, window);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, warp_action);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, lang_action);

    TXT_SetKeyListener(window, MainMenuKeyPress, NULL);
}

//
// Initialize all configuration variables, load config file, etc
//

static void InitConfig(void)
{
    M_SetConfigDir(NULL);
    InitBindings();

    SetChatMacroDefaults();
    SetPlayerNameDefault();

    M_LoadConfig();
}

//
// Application icon
//

static void SetIcon(void)
{
    extern SDL_Window *TXT_SDLWindow;
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) icon_data, icon_w,
                                       icon_h, 32, icon_w * 4,
                                       0xff << 24, 0xff << 16,
                                       0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(TXT_SDLWindow, surface);
    SDL_FreeSurface(surface);
}

static void SetWindowTitle(void)
{
    TXT_SetDesktopTitle("Setup.exe");
}

// Initialize the textscreen library.

static void InitTextscreen(void)
{
    SetDisplayDriver();

    if (!TXT_Init(0, 0))
        {
            fprintf(stderr, english_language ?
                            "Failed to initialize GUI\n" :
                            "Ќевозможно инициализировать интерфейс\n");
            exit(-1);
        }

    // [JN] Use different colors for different games.
    if (gamemission == doom)
    {
        TXT_SetColor(TXT_COLOR_BLUE, 0, 63, 148);           // Background
        TXT_SetColor(TXT_COLOR_BRIGHT_CYAN, 84, 206, 254);  // Borders
        TXT_SetColor(TXT_COLOR_BRIGHT_GREEN, 255, 232, 59); // FN buttons
        TXT_SetColor(TXT_COLOR_CYAN, 70, 176, 219);         // Highlighted item
        TXT_SetColor(TXT_COLOR_GREEN, 0, 255, 0);           // Language switching text
    }
    else if (gamemission == heretic)
    {
        TXT_SetColor(TXT_COLOR_BLUE, 0, 96, 0);             // Background
        TXT_SetColor(TXT_COLOR_BRIGHT_CYAN, 24, 217, 43);   // Borders
        TXT_SetColor(TXT_COLOR_BRIGHT_GREEN, 255, 232, 59); // FN buttons
        TXT_SetColor(TXT_COLOR_CYAN, 47, 187, 110);         // Highlighted item
        TXT_SetColor(TXT_COLOR_GREEN, 0, 255, 255);         // Language switching text
    }
    else if (gamemission == hexen)
    {
        TXT_SetColor(TXT_COLOR_BLUE, 96, 0, 0);             // Background
        TXT_SetColor(TXT_COLOR_BRIGHT_CYAN, 255, 36, 36);   // Borders
        TXT_SetColor(TXT_COLOR_BRIGHT_GREEN, 255, 232, 59); // FN buttons
        TXT_SetColor(TXT_COLOR_CYAN, 187, 87, 47);          // Highlighted item
        TXT_SetColor(TXT_COLOR_GREEN, 115, 255, 60);        // Language switching text
    }

    SetIcon();
    SetWindowTitle();
}

// Restart the textscreen library.  Used when the video_driver variable
// is changed.

void RestartTextscreen(void)
{
    TXT_Shutdown();
    InitTextscreen();
}

// 
// Initialize and run the textscreen GUI.
//

static void RunGUI(void)
{
    InitTextscreen();

    TXT_GUIMainLoop();
}

static void MissionSet(void)
{
#ifdef _WIN32
    // [JN] Get system preffed language...
    DWORD rd_lang_id = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));
    // ..if game language is not set yet (-1), and OS preffered language
    // is appropriate for using Russian language in the game, use it.
    if (english_language == -1)
    {
        if (rd_lang_id != LANG_RUSSIAN
        &&  rd_lang_id != LANG_UKRAINIAN
        &&  rd_lang_id != LANG_BELARUSIAN)
        english_language = 1;
        else
        english_language = 0;
    }
#endif

    SetWindowTitle();
    InitConfig();
    MainMenu();
}

void D_DoomMain(void)
{
    SetupMission(MissionSet);

    RunGUI();
}


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

#include "doomtype.h"

#include "config.h"
#include "textscreen.h"

#include "doomtype.h"
#include "d_mode.h"
#include "d_name.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"

#include "compatibility.h"
#include "display.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

#include "mode.h"

GameMission_t gamemission;
static const iwad_t **iwads;

typedef struct
{
    char *label;
    GameMission_t mission;
    int mask;
    char *name;
    //char *config_file;
    char *extra_config_file;
    char *executable;
} mission_config_t;

// Default mission to fall back on, if no IWADs are found at all:

#define DEFAULT_MISSION (&mission_configs[0])

static mission_config_t mission_configs[] =
{
    {
        "Doom",
        doom,
        IWAD_MASK_DOOM,
        "doom",
        // "default.ini",
        PROGRAM_PREFIX "doom.ini",
        PROGRAM_PREFIX "doom"
    },
    {
        "Heretic",
        heretic,
        IWAD_MASK_HERETIC,
        "heretic",
        // "heretic.ini",
        PROGRAM_PREFIX "heretic.ini",
        PROGRAM_PREFIX "heretic"
    },
    {
        "Hexen",
        hexen,
        IWAD_MASK_HEXEN,
        "hexen",
        // "hexen.ini",
        PROGRAM_PREFIX "hexen.ini",
        PROGRAM_PREFIX "hexen"
    },
	
    {
        "Strife",
        strife,
        IWAD_MASK_STRIFE,
        "strife",
        // "strife.ini",
        PROGRAM_PREFIX "strife.ini",
        PROGRAM_PREFIX "strife"
    }
};

static GameSelectCallback game_selected_callback;

// Miscellaneous variables that aren't used in setup.

// Windows OS only: do not set game language on first launch, 
// try to determine it automatically in D_DoomMain.
#ifdef _WIN32
int english_language = -1;
#else
int english_language = 0;
#endif

static int showMessages = 1;
static int screenblocks = 10;
static int detailLevel = 0;
static char *executable = NULL;
static char *game_title = "Doom";
static char *back_flat = "F_PAVE01";
static int comport = 0;
static char *nickname = NULL;

// -----------------------------------------------------------------------------
// [JN] PWAD autoloading. Initially all 4 values are empty.
// -----------------------------------------------------------------------------

static char *autoloadglobalpwad[10]    = { "", "", "", "" };
static char *autoloaddoom1pwad[10]     = { "", "", "", "" };
static char *autoloaddoom2pwad[10]     = { "", "", "", "" };
static char *autoloadplutoniapwad[10]  = { "", "", "", "" };
static char *autoloadtntpwad[10]       = { "", "", "", "" };
static char *autoloadregisteredpwad[10] = { "", "", "", "" };
static char *autoloadretailpwad[10]     = { "", "", "", "" };

static void BindMiscVariables(void)
{
    int i;
    char pwad[32];

    // [JN] Initialise language variable first
    M_BindIntVariable("english_language", &english_language);

    if (gamemission == doom)
    {
        // [JN] PWAD autoloading. Note that we are using variables 1..4, not 0...3.
        for (i = 1 ; i < 5 ; ++i)
        {
            M_snprintf(pwad, sizeof(pwad), "autoload_global_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadglobalpwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_doom1_pwad%i", i);
            M_BindStringVariable(pwad, &autoloaddoom1pwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_doom2_pwad%i", i);
            M_BindStringVariable(pwad, &autoloaddoom2pwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_plutonia_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadplutoniapwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_tnt_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadtntpwad[i]);

            // [JN] If autoloads have not been set, initialize with defaults.
            if (autoloadglobalpwad[i] == NULL)
                autoloadglobalpwad[i] = "";
            if (autoloaddoom1pwad[i] == NULL)
                autoloaddoom1pwad[i] = "";
            if (autoloaddoom2pwad[i] == NULL)
                autoloaddoom2pwad[i] = "";
            if (autoloadplutoniapwad[i] == NULL)
                autoloadplutoniapwad[i] = "";
            if (autoloadtntpwad[i] == NULL)
                autoloadtntpwad[i] = "";
        }

        M_BindIntVariable("detaillevel",   &detailLevel);
        M_BindIntVariable("show_messages", &showMessages);
    }

    if (gamemission == heretic)
    {
        // [JN] PWAD autoloading. Note that we are using variables 1..4, not 0...3.
        for (i = 1 ; i < 5 ; ++i)
        {
            M_snprintf(pwad, sizeof(pwad), "autoload_global_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadglobalpwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_registered_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadregisteredpwad[i]);

            M_snprintf(pwad, sizeof(pwad), "autoload_retail_pwad%i", i);
            M_BindStringVariable(pwad, &autoloadretailpwad[i]);

            // [JN] If autoloads have not been set, initialize with defaults.
            if (autoloadglobalpwad[i] == NULL)
                autoloadglobalpwad[i] = "";
            if (autoloadregisteredpwad[i] == NULL)
                autoloadregisteredpwad[i] = "";
            if (autoloadretailpwad[i] == NULL)
                autoloadretailpwad[i] = "";
        }
    }

    if (gamemission == hexen)
    {
        M_BindIntVariable("messageson", &showMessages);
    }

    if (gamemission == strife)
    {
        // Strife has a different default value than the other games
        screenblocks = 10;

        M_BindStringVariable("back_flat",   &back_flat);
        M_BindStringVariable("nickname",    &nickname);

        M_BindIntVariable("screensize",     &screenblocks);
        M_BindIntVariable("comport",        &comport);
    }
    else
    {
        M_BindIntVariable("screenblocks",   &screenblocks);
    }

}

//
// Initialise all configuration file bindings.
//

void InitBindings(void)
{
    BindCompatibilityVariables();
    BindDisplayVariables();
    BindMouseVariables();
    BindSoundVariables();
    BindMiscVariables();
    BindMultiplayerVariables();
}

// Set the name of the executable program to run the game:

static void SetExecutable(mission_config_t *config)
{
    char *extension;

    free(executable);

#ifdef _WIN32
    extension = ".exe";
#else
    extension = "";
#endif

    executable = M_StringJoin(config->executable, extension, NULL);
}

static void SetMission(mission_config_t *config)
{
    switch (config->mission)
    {
        case heretic:
            RD_Project_String = PACKAGE_PREFIX " Setup " BUILD_HERETIC_VERSION;
            break;
        case hexen:
            RD_Project_String = PACKAGE_PREFIX " Setup " BUILD_HEXEN_VERSION;
            break;
        case strife:
            RD_Project_String = PACKAGE_PREFIX " Setup " BUILD_STRIFE_VERSION;
            break;
        // [Dasperal] For everything else use default doom version
        default:
            break;
    }

    iwads = D_FindAllIWADs(config->mask);
    gamemission = config->mission;
    SetExecutable(config);
    game_title = config->label;
    M_SetConfigFilename(config->extra_config_file);
}

static mission_config_t *GetMissionForName(char *name)
{
    int i;

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        if (!strcmp(mission_configs[i].name, name))
        {
            return &mission_configs[i];
        }
    }

    return NULL;
}

// Check the name of the executable.  If it contains one of the game
// names (eg. chocolate-hexen-setup.exe) then use that game.

static boolean CheckExecutableName(GameSelectCallback callback)
{
    mission_config_t *config;
    char *exe_name;
    int i;

    exe_name = M_GetExecutableName();

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        config = &mission_configs[i];

        if (strstr(exe_name, config->name) != NULL)
        {
            SetMission(config);
            callback();
            return true;
        }
    }

    return false;
}

static void GameSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(config))
{
    TXT_CAST_ARG(mission_config_t, config);

    SetMission(config);
    game_selected_callback();
}

static void OpenGameSelectDialog(GameSelectCallback callback)
{
    mission_config_t *mission = NULL;
    txt_window_t *window;
    const iwad_t **iwads;
    int num_games;
    int i;

	// "Select game"
	window = TXT_NewWindow(PACKAGE_PREFIX " Doom");

    // TXT_AddWidget(window, TXT_NewLabel("Выберите желаемую игру:\n"));
    TXT_AddWidget(window, TXT_NewLabel(" "));

    num_games = 0;

    // Add a button for each game.

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        // Do we have any IWADs for this game installed?
        // If so, add a button.

        iwads = D_FindAllIWADs(mission_configs[i].mask);

        if (iwads[0] != NULL)
        {
            mission = &mission_configs[i];
            TXT_AddWidget(window, TXT_NewButton2(mission_configs[i].label,
                                                 GameSelected,
                                                 &mission_configs[i]));
            ++num_games;
        }

        free(iwads);
    }

    TXT_AddWidget(window, TXT_NewStrut(0, 1));

    // No IWADs found at all?  Fall back to doom, then.

    if (num_games == 0)
    {
        TXT_CloseWindow(window);
        SetMission(DEFAULT_MISSION);
        callback();
        return;
    }

    // Only one game? Use that game, and don't bother with a dialog.

    if (num_games == 1)
    {
        TXT_CloseWindow(window);
        SetMission(mission);
        callback();
        return;
    }

    game_selected_callback = callback;
}

void SetupMission(GameSelectCallback callback)
{
    mission_config_t *config;
    char *mission_name;
    int p;

    //!
    // @arg <game>
    //
    // Specify the game to configure the settings for.  Valid
    // values are 'doom', 'heretic', 'hexen' and 'strife'.
    //

    p = M_CheckParm("-game");

    if (p > 0)
    {
        mission_name = myargv[p + 1];

        config = GetMissionForName(mission_name);

        if (config == NULL)
        {
            I_Error("Invalid parameter - '%s'", mission_name);
        }

        SetMission(config);
        callback();
    }
    else if (!CheckExecutableName(callback))
    {
        OpenGameSelectDialog(callback);
    }
}

char *GetExecutableName(void)
{
    return executable;
}

char *GetGameTitle(void)
{
    return game_title;
}

const iwad_t **GetIwads(void)
{
    return iwads;
}


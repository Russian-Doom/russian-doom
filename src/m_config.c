//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2025 Leonid Murin (Dasperal)
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
//    Configuration file interface.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <locale.h>

#include "SDL_filesystem.h"

#include "d_name.h"
#include "doomtype.h"
#include "doomfeatures.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "jn.h"
#include "rd_migration.h"
#include "rd_keybinds.h"
#include "i_controller.h"

typedef struct
{
    char* loadPath;
    char* savePath;
} config_path_t;

typedef struct section_s
{
    struct section_s* next;
    char* name;
    sectionHandler_t* handler;
} section_t;

// Location where all configuration data is stored
config_path_t configPath;

static section_t* sections;

//
// DEFAULTS
//

typedef struct
{
    // Name of the variable
    char *name;

    // Pointer to the location in memory of the variable
    union {
        int *i;
        char **s;
        float *f;
    } location;

    // Type of the variable
    default_type_t type;

    // If true, this config variable has been bound to a variable
    // and is being used.
    boolean bound;
} default_t;

typedef struct
{
    default_t *defaults;
    int numdefaults;
} default_collection_t;

#define CONFIG_VARIABLE_GENERIC(name, type) \
    { #name, {NULL}, type, false }

#define CONFIG_VARIABLE_INT(name) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_INT)
#define CONFIG_VARIABLE_INT_HEX(name) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_INT_HEX)
#define CONFIG_VARIABLE_FLOAT(name) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_FLOAT)
#define CONFIG_VARIABLE_STRING(name) \
    CONFIG_VARIABLE_GENERIC(name, DEFAULT_STRING)

//! @begin_config_file default

static default_t defaults_list[] =
{
    //!
    // [JN] Support for switching to the English language.
    //

    CONFIG_VARIABLE_INT(english_language),

    //!
    // Enable directory based autoload.

    CONFIG_VARIABLE_INT(enable_autoload),

    //!
    // Full path to a directory in which WAD files and dehacked patches
    // can be placed to be automatically loaded on startup. A subdirectory
    // of this directory matching the IWAD name is checked to find the
    // files to load.

    CONFIG_VARIABLE_STRING(autoload_root),

    //!
    // @game strife
    //
    // If non-zero, dialogue text is displayed over characters' pictures
    // when engaging actors who have voices.
    //

    CONFIG_VARIABLE_INT(show_talk),

    //!
    // @game strife
    //
    // Volume of voice sound effects, range 0-15.
    //

    CONFIG_VARIABLE_INT(voice_volume),

    //!
    // If non-zero, mouse input is enabled.  If zero, mouse input is
    // disabled.
    //

    CONFIG_VARIABLE_INT(use_mouse),

    //!
    // If non-zero, joystick input is enabled.
    //

    CONFIG_VARIABLE_INT(use_controller),

    //!
    // @game strife
    //
    // Screen size, range 3-11.
    //
    // A value of 11 gives a full-screen view with the status bar not
    // displayed.  A value of 10 gives a full-screen view with the
    // status bar displayed.
    //

    CONFIG_VARIABLE_INT(screensize),

    //!
    // Music output device.  A non-zero value gives MIDI sound output,
    // while a value of zero disables music.
    //

    CONFIG_VARIABLE_INT(snd_musicdevice),

    //!
    // Sound effects device.  A value of zero disables in-game sound
    // effects, a value of 1 enables PC speaker sound effects, while
    // a value in the range 2-9 enables the "normal" digital sound
    // effects.
    //

    CONFIG_VARIABLE_INT(snd_sfxdevice),

    //!
    // @game strife
    //
    // Name of background flat used by view border.
    //

    CONFIG_VARIABLE_STRING(back_flat),

    //!
    // @game strife
    //
    // Multiplayer nickname (?).
    //

    CONFIG_VARIABLE_STRING(nickname),

    //!
    // Multiplayer chat macro: message to send when alt+0 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro0),

    //!
    // Multiplayer chat macro: message to send when alt+1 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro1),

    //!
    // Multiplayer chat macro: message to send when alt+2 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro2),

    //!
    // Multiplayer chat macro: message to send when alt+3 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro3),

    //!
    // Multiplayer chat macro: message to send when alt+4 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro4),

    //!
    // Multiplayer chat macro: message to send when alt+5 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro5),

    //!
    // Multiplayer chat macro: message to send when alt+6 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro6),

    //!
    // Multiplayer chat macro: message to send when alt+7 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro7),

    //!
    // Multiplayer chat macro: message to send when alt+8 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro8),

    //!
    // Multiplayer chat macro: message to send when alt+9 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro9),

    //!
    // @game strife
    //
    // Serial port number to use for SERSETUP.EXE (unused).
    //

    CONFIG_VARIABLE_INT(comport),

    //!
    // @game heretic hexen strife
    //
    // If non-zero, display the graphical startup screen.
    //

    CONFIG_VARIABLE_INT(graphical_startup),

    //!
    // If non-zero, the game will run in full screen mode.  If zero,
    // the game will run in a window.
    //

    CONFIG_VARIABLE_INT(fullscreen),

    //!
    // [JN] Rendering resolution.
    //

    CONFIG_VARIABLE_INT(rendering_resolution),

    //!
    // [JN] Aspect ratio.
    //

    CONFIG_VARIABLE_INT(aspect_ratio),

    //!
    // SDL render driver
    //

    CONFIG_VARIABLE_STRING(render_driver),

    //!
    // Index of the display on which the game should run. This has no
    // effect if running in windowed mode (fullscreen=0) and
    // window_position is not set to "center".
    //

    CONFIG_VARIABLE_INT(video_display),

    //!
    // If non-zero, any pillar/letter boxes drawn around the game area
    // will "flash" when the game palette changes, simulating the VGA
    // "porch"

    CONFIG_VARIABLE_INT(vga_porch_flash),

    //!
    // Number of milliseconds to wait on startup after the video mode
    // has been set, before the game will start.  This allows the
    // screen to settle on some monitors that do not display an image
    // for a brief interval after changing video modes.
    //

    CONFIG_VARIABLE_INT(startup_delay),
    CONFIG_VARIABLE_INT(resize_delay),

    //!
    // [JN] Defines short or long window title composition.
    //

    CONFIG_VARIABLE_INT(window_title_short),

    //!
    // [JN] If true, window will be pinned on top of other windows.
    //

    CONFIG_VARIABLE_INT(window_ontop),

    //!
    // Window width when running in windowed mode.
    //

    CONFIG_VARIABLE_INT(window_width),

    //!
    // Window height when running in windowed mode.
    //

    CONFIG_VARIABLE_INT(window_height),

    //!
    // [JN] Use bordered or borderless window.
    //

    CONFIG_VARIABLE_INT(window_border),

    //!
    // [Dasperal] UNDOCUMENTED FEATURE. Defines custom text in window title.
    //

    CONFIG_VARIABLE_STRING(window_title_custom_prefix),

    //!
    // Width for screen mode when running fullscreen.
    // If this and fullscreen_height are both set to zero, we run
    // fullscreen as a desktop window that covers the entire screen,
    // rather than ever switching screen modes. It should usually
    // be unnecessary to set this value.
    //
    CONFIG_VARIABLE_INT(fullscreen_width),

    //!
    // Height for screen mode when running fullscreen.
    // See documentation for fullscreen_width.
    //
    CONFIG_VARIABLE_INT(fullscreen_height),

    //!
    // If this is non-zero, the mouse will be "grabbed" when running
    // in windowed mode so that it can be used as an input device.
    // When running full screen, this has no effect.
    //

    CONFIG_VARIABLE_INT(grabmouse),

    //!
    // Mouse acceleration factor.  When the speed of mouse movement
    // exceeds the threshold value (mouse_threshold), the speed is
    // multiplied by this value.
    //

    CONFIG_VARIABLE_FLOAT(mouse_acceleration),

    //!
    // Mouse acceleration threshold.  When the speed of mouse movement
    // exceeds this threshold value, the speed is multiplied by an
    // acceleration factor (mouse_acceleration).
    //

    CONFIG_VARIABLE_INT(mouse_threshold),

    //!
    // Sound output sample rate, in Hz.
    // Typical values to use are
    // 11025, 22050, 44100 and 48000.
    //

    CONFIG_VARIABLE_INT(snd_samplerate),

    //!
    // Maximum number of bytes to allocate for caching converted sound
    // effects in memory. If set to zero, there is no limit applied.
    //

    CONFIG_VARIABLE_INT(snd_cachesize),

    //!
    // Maximum size of the output sound buffer size in milliseconds.
    // Sound output is generated periodically in slices. Higher values
    // might be more efficient but will introduce latency to the
    // sound output. The default is 28ms (one slice per tic with the
    // 35fps timer).

    CONFIG_VARIABLE_INT(snd_maxslicetime_ms),

    //!
    // External command to invoke to perform MIDI playback. If set to
    // the empty string, SDL_mixer's internal MIDI playback is used.
    // This only has any effect when snd_musicdevice is set to General
    // MIDI output.

    CONFIG_VARIABLE_STRING(snd_musiccmd),

    //!
    // Value to set for the DMXOPTION environment variable. If this contains
    // "-opl3", output for an OPL3 chip is generated when in OPL MIDI
    // playback mode.
    //
    CONFIG_VARIABLE_STRING(snd_dmxoption),

    //!
    // The I/O port to use to access the OPL chip.  Only relevant when
    // using native OPL music playback.
    //

    CONFIG_VARIABLE_INT_HEX(opl_io_port),

    //!
    // @game doom heretic strife
    //
    // If non-zero, the ENDOOM text screen is displayed when exiting the
    // game. If zero, the ENDOOM screen is not displayed.
    //

    CONFIG_VARIABLE_INT(show_endoom),

    //!
    // If non-zero, the game behaves like Vanilla Doom, always assuming
    // an American keyboard mapping.  If this has a value of zero, the
    // native keyboard mapping of the keyboard is used.
    //

    CONFIG_VARIABLE_INT(vanilla_keyboard_mapping),

    //!
    // Name of the SDL video driver to use.  If this is an empty string,
    // the default video driver is used.
    //

    CONFIG_VARIABLE_STRING(video_driver),

    //!
    // [JN] Window X and Y position to save and restore.
    //

    CONFIG_VARIABLE_INT(window_position_x),
    CONFIG_VARIABLE_INT(window_position_y),

#ifdef FEATURE_MULTIPLAYER

    //!
    // Name to use in network games for identification.  This is only
    // used on the "waiting" screen while waiting for the game to start.
    //

    CONFIG_VARIABLE_STRING(player_name),

#endif

#ifdef FEATURE_SOUND

    //!
    // Scaling factor used by libsamplerate. This is used when converting
    // sounds internally back into integer form; normally it should not
    // be necessary to change it from the default value. The only time
    // it might be needed is if a PWAD file is loaded that contains very
    // loud sounds, in which case the conversion may cause sound clipping
    // and the scale factor should be reduced. The lower the value, the
    // quieter the sound effects become, so it should be set as high as is
    // possible without clipping occurring.

    CONFIG_VARIABLE_FLOAT(libsamplerate_scale),

    //!
    // Full path to a soundfont file to use with FluidSynth MIDI playback.
    //

    CONFIG_VARIABLE_STRING(fluidsynth_sf_path),

    //!
    // Full path to a Timidity configuration file to use for MIDI
    // playback. The file will be evaluated from the directory where
    // it is evaluated, so there is no need to add "dir" commands
    // into it.
    //

    CONFIG_VARIABLE_STRING(timidity_cfg_path),

    //!
    // Path to GUS patch files to use when operating in GUS emulation
    // mode.
    //

    CONFIG_VARIABLE_STRING(gus_patch_path),

    //!
    // Number of kilobytes of RAM to use in GUS emulation mode. Valid
    // values are 256, 512, 768 or 1024.
    //

    CONFIG_VARIABLE_INT(gus_ram_kb),

#endif

    // Rendering
    CONFIG_VARIABLE_INT(vsync),
    CONFIG_VARIABLE_INT(preserve_window_aspect_ratio),
    CONFIG_VARIABLE_INT(uncapped_fps),
    CONFIG_VARIABLE_INT(show_fps),
    CONFIG_VARIABLE_INT(smoothing),
    CONFIG_VARIABLE_INT(max_fps),
    CONFIG_VARIABLE_INT(smoothlight),
    CONFIG_VARIABLE_INT(show_diskicon),
    CONFIG_VARIABLE_INT(screen_wiping),
    CONFIG_VARIABLE_INT(png_screenshots),
    CONFIG_VARIABLE_INT(flashing_hom),

    // Display
    CONFIG_VARIABLE_INT(screenblocks),
    CONFIG_VARIABLE_INT(extra_level_brightness),
    CONFIG_VARIABLE_INT(menu_shading),
    CONFIG_VARIABLE_INT(hud_detaillevel),

    // Color options
    CONFIG_VARIABLE_FLOAT(brightness),
    CONFIG_VARIABLE_INT(usegamma),
    CONFIG_VARIABLE_FLOAT(color_saturation),
    CONFIG_VARIABLE_INT(show_palette),
    CONFIG_VARIABLE_FLOAT(r_color_factor),
    CONFIG_VARIABLE_FLOAT(g_color_factor),
    CONFIG_VARIABLE_FLOAT(b_color_factor),
    
    CONFIG_VARIABLE_INT(local_time),
    CONFIG_VARIABLE_INT(show_messages),
    CONFIG_VARIABLE_INT(messages_alignment),
    CONFIG_VARIABLE_INT(messages_timeout),
    CONFIG_VARIABLE_INT(message_fade),
    CONFIG_VARIABLE_INT(message_color_pickup),
    CONFIG_VARIABLE_INT(message_color_secret),
    CONFIG_VARIABLE_INT(message_color_system),
    CONFIG_VARIABLE_INT(message_color_chat),
    CONFIG_VARIABLE_INT(message_color_quest),
    CONFIG_VARIABLE_INT(draw_shadowed_text),

    // Automap
    CONFIG_VARIABLE_INT(automap_color),
    CONFIG_VARIABLE_INT(automap_antialias),
    CONFIG_VARIABLE_INT(automap_rotate),
    CONFIG_VARIABLE_INT(automap_overlay),
    CONFIG_VARIABLE_INT(automap_overlay_bg),    
    CONFIG_VARIABLE_INT(automap_follow),
    CONFIG_VARIABLE_INT(automap_grid),
    CONFIG_VARIABLE_INT(automap_grid_size),
    CONFIG_VARIABLE_INT(automap_mark_color),
    CONFIG_VARIABLE_INT(automap_secrets),

    // Stats
    CONFIG_VARIABLE_INT(stats_placement),
    CONFIG_VARIABLE_INT(stats_kis),
    CONFIG_VARIABLE_INT(stats_skill),
    CONFIG_VARIABLE_INT(stats_level_time),
    CONFIG_VARIABLE_INT(stats_total_time),
    CONFIG_VARIABLE_INT(stats_coords),
    CONFIG_VARIABLE_INT(stats_level_name),
    CONFIG_VARIABLE_INT(stats_color),

    // Sound
    CONFIG_VARIABLE_INT(sfx_volume),
    CONFIG_VARIABLE_INT(music_volume),
    CONFIG_VARIABLE_INT(snd_channels),
    CONFIG_VARIABLE_INT(snd_monomode),
    CONFIG_VARIABLE_INT(snd_pitchshift),
    CONFIG_VARIABLE_INT(mute_inactive_window),

    // Controls
    CONFIG_VARIABLE_INT(always_run),
    CONFIG_VARIABLE_INT(mlook),
    CONFIG_VARIABLE_INT(mouse_sensitivity),
    CONFIG_VARIABLE_INT(novert),
    CONFIG_VARIABLE_INT(mouse_y_invert),
    CONFIG_VARIABLE_INT(artiskip),

    // Gameplay: Game Mechanics
    CONFIG_VARIABLE_INT(strict_mode),

    // Gameplay: Graphical
    CONFIG_VARIABLE_INT(brightmaps),
    CONFIG_VARIABLE_INT(fake_contrast),
    CONFIG_VARIABLE_INT(translucency),
    CONFIG_VARIABLE_INT(colored_blood),
    CONFIG_VARIABLE_INT(improved_fuzz),
    CONFIG_VARIABLE_INT(swirling_liquids),
    CONFIG_VARIABLE_INT(invul_sky),
    CONFIG_VARIABLE_INT(linear_sky),
    CONFIG_VARIABLE_INT(randomly_flipcorpses),
    CONFIG_VARIABLE_INT(flip_weapons),

    // Gameplay: Status Bar
    CONFIG_VARIABLE_INT(extra_player_faces),
    CONFIG_VARIABLE_INT(negative_health),
    CONFIG_VARIABLE_INT(sbar_colored),
    CONFIG_VARIABLE_INT(sbar_colored_gem),
    CONFIG_VARIABLE_INT(stbar_color_high),
    CONFIG_VARIABLE_INT(stbar_color_normal),
    CONFIG_VARIABLE_INT(stbar_color_low),
    CONFIG_VARIABLE_INT(stbar_color_critical),
    CONFIG_VARIABLE_INT(stbar_color_armor_1),
    CONFIG_VARIABLE_INT(stbar_color_armor_2),
    CONFIG_VARIABLE_INT(stbar_color_armor_0),
    CONFIG_VARIABLE_INT(ammo_widget),
    CONFIG_VARIABLE_INT(ammo_widget_colored),
    CONFIG_VARIABLE_INT(weapon_widget),
    CONFIG_VARIABLE_INT(center_inventory_cursor),

    // Gameplay: Audible
    CONFIG_VARIABLE_INT(z_axis_sfx),
    CONFIG_VARIABLE_INT(play_exit_sfx),
    CONFIG_VARIABLE_INT(crushed_corpses_sfx),
    CONFIG_VARIABLE_INT(blazing_door_fix_sfx),
    CONFIG_VARIABLE_INT(noise_alert_sfx),
    CONFIG_VARIABLE_INT(correct_endlevel_sfx),

    // Gameplay: Tactical
    CONFIG_VARIABLE_INT(secret_notification),
    CONFIG_VARIABLE_INT(infragreen_visor),
    CONFIG_VARIABLE_INT(autoaim_horizonal),
    CONFIG_VARIABLE_INT(autoaim_vertical),
    CONFIG_VARIABLE_INT(show_all_artifacts),
    CONFIG_VARIABLE_INT(show_artifacts_timer),

    // Gameplay: Physical
    CONFIG_VARIABLE_INT(improved_collision),
    CONFIG_VARIABLE_INT(over_under),
    CONFIG_VARIABLE_INT(torque),
    CONFIG_VARIABLE_INT(ssg_blast_enemies),
    CONFIG_VARIABLE_INT(floating_powerups),
    CONFIG_VARIABLE_INT(toss_drop),
    CONFIG_VARIABLE_INT(weapon_bobbing),

    // Gameplay: Crosshair
    CONFIG_VARIABLE_INT(crosshair_draw),
    CONFIG_VARIABLE_INT(crosshair_shape),
    CONFIG_VARIABLE_INT(crosshair_type),
    CONFIG_VARIABLE_INT(crosshair_scale),
    CONFIG_VARIABLE_INT(crosshair_opacity),

    // Gameplay: Gameplay
    CONFIG_VARIABLE_INT(default_skill),
    CONFIG_VARIABLE_INT(fix_map_errors),
    CONFIG_VARIABLE_INT(flip_levels),
    CONFIG_VARIABLE_INT(pistol_start),
    CONFIG_VARIABLE_INT(breathing),
    CONFIG_VARIABLE_INT(unlimited_lost_souls),
    CONFIG_VARIABLE_INT(fast_quickload),
    CONFIG_VARIABLE_INT(skip_unused_artifact),

    // [Dasperal] Vanila bugs fixes
    CONFIG_VARIABLE_INT(heresiarch_zero_cast_time_fix),

    // Gameplay: Demos
    CONFIG_VARIABLE_INT(demotimer),
    CONFIG_VARIABLE_INT(demotimerdir),
    CONFIG_VARIABLE_INT(demobar),
    CONFIG_VARIABLE_INT(no_internal_demos),
};

static default_collection_t default_collection =
{
    defaults_list,
    arrlen(defaults_list)
};

static void DefaultHandler_Save(FILE* file, char* sectionName);
static void DefaultHandler_HandleLine(char* keyName, char *value, size_t valueSize);
static boolean DefaultHandler_isHandling(char* sectionName)
{
    return strcmp("General", sectionName) == 0;
}

static sectionHandler_t defaultHandler = {
    DefaultHandler_isHandling,
    DefaultHandler_HandleLine,
    DefaultHandler_Save,
    NULL
};

static sectionHandler_t keybindsHandler = {
    KeybindsHandler_isHandling,
    KeybindsHandler_HandleLine,
    KeybindsHandler_Save,
    NULL
};

sectionHandler_t controllerHandler = {
    ControllerHandler_isHandling,
    ControllerHandler_HandleLine,
    ControllerHandler_Save,
    ControllerHandler_onFinishHandling
};

static sectionHandler_t* handlers[] = {
    &defaultHandler,
    &keybindsHandler,
    &controllerHandler

};

// Search a collection for a variable

static default_t *SearchCollection(default_collection_t *collection, char *name)
{
    int i;

    for (i=0; i<collection->numdefaults; ++i) 
    {
        if (!strcmp(name, collection->defaults[i].name))
        {
            return &collection->defaults[i];
        }
    }

    return NULL;
}

static void DefaultHandler_Save(FILE* file, char* sectionName)
{
    default_t *defaults;
    int i;

    defaults = default_collection.defaults;

    for (i=0 ; i < default_collection.numdefaults ; i++)
    {
        // Ignore unbound variables
        if (!defaults[i].bound)
        {
            continue;
        }

        // Print the name and the value
        switch (defaults[i].type) 
        {
            case DEFAULT_INT:
	            fprintf(file, "%s = %i\n", defaults[i].name, *defaults[i].location.i);
                break;

            case DEFAULT_INT_HEX:
	            fprintf(file, "%s = 0x%x\n", defaults[i].name, *defaults[i].location.i);
                break;

            case DEFAULT_FLOAT:
                fprintf(file, "%s = %f\n", defaults[i].name, *defaults[i].location.f);
                break;

            case DEFAULT_STRING:
	            fprintf(file, "%s = \"%s\"\n", defaults[i].name, *defaults[i].location.s);
                break;
        }
    }
}

static void SetVariable(default_t *def, char *value);

static void DefaultHandler_HandleLine(char* keyName, char *value, size_t valueSize)
{
    default_t *def;
    defaultTracker_t* tracker;

    tracker = M_GetDefaultTracker(keyName);
    def = SearchCollection(&default_collection, keyName);
    if((def == NULL || !def->bound)
    && tracker == NULL)
    {
        // Unknown variable?  Unbound variables are also treated as unknown.
        return;
    }

    if(lang_param && strcmp(keyName, "english_language") == 0)
    {
        // Ignore english_language entry in config with -lang mode
        return;
    }

    // Strip off trailing non-printable characters
    while(strlen(value) > 0 && !isprint(value[strlen(value)-1]))
    {
        value[strlen(value)-1] = '\0';
    }

    // Surrounded by quotes? If so, remove them.
    if(strlen(value) >= 2
    && value[0] == '"' && value[strlen(value) - 1] == '"')
    {
        value[strlen(value) - 1] = '\0';
        memmove(value, value + 1, valueSize - 1);
    }

    if(def != NULL && def->bound)
        SetVariable(def, value);

    if(tracker != NULL)
        M_SetTrackedValue(tracker, value);
}

// Parses integer values in the configuration file

static int ParseIntParameter(char *strparm)
{
    int parm;

    if (strparm[0] == '0' && strparm[1] == 'x')
        sscanf(strparm+2, "%x", &parm);
    else
        sscanf(strparm, "%i", &parm);

    return parm;
}

static void SetVariable(default_t *def, char *value)
{
    // parameter found

    switch (def->type)
    {
        case DEFAULT_STRING:
            *def->location.s = M_StringDuplicate(value);
            break;

        case DEFAULT_INT:
        case DEFAULT_INT_HEX:
            *def->location.i = ParseIntParameter(value);
            break;

        case DEFAULT_FLOAT:
        {
            // Different locales use different decimal separators.
            // However, the choice of the current locale isn't always
            // under our own control. If the atof() function fails to
            // parse the string representing the floating point number
            // using the current locale's decimal separator, it will
            // return 0, resulting in silent sound effects. To
            // mitigate this, we replace the first non-digit,
            // non-minus character in the string with the current
            // locale's decimal separator before passing it to atof().
            struct lconv *lc = localeconv();
            char dec, *str;
            int i = 0;

            dec = lc->decimal_point[0];
            str = M_StringDuplicate(value);

            // Skip sign indicators.
            if(str[i] == '-' || str[i] == '+')
            {
                i++;
            }

            for(; str[i] != '\0'; i++)
            {
                if(!isdigit(str[i]))
                {
                    str[i] = dec;
                    break;
                }
            }

            *def->location.f = (float) atof(str);
            free(str);
            break;
        }
    }
}

//
// M_SaveConfig
//

void M_SaveConfig (void)
{
    FILE *f;
    section_t* section;

    if(!configPath.savePath)
    {
        I_AddError(english_language ?
                   "No writable path to save configuration file\n" :
                   "Нет доступного для записи места для сохранения файла конфигурации\n");
        return;
    }

    printf(english_language ?
           "Saving configuration file:\n    %s\n" :
           "Сохранение файла конфигурации:\n    %s\n",
        configPath.savePath);

    const char* config_dir = M_DirName(configPath.savePath);
    M_MakeDirectory(config_dir);
    free(config_dir);

    f = M_fopen(configPath.savePath, "w");
    if(!f)
    {
        I_AddError(english_language ?
                   "Unable to open configuration file for writing\n" :
                   "Не удалось открыть файл конфигурации для записи\n");
        return;
    }

    fprintf(f, "config_version = %i\n\n", CURRENT_CONFIG_VERSION);
    section = sections;
    while(section)
    {
        fprintf(f, "[%s]\n", section->name);
        section->handler->save(f, section->name);
        fprintf(f, "\n");
        section = section->next;
    }

    fclose(f);
}

void M_AppendConfigSection(const char* sectionName, sectionHandler_t* handler)
{
    section_t* temp;

    if(sections == NULL)
    {
        sections = malloc(sizeof(section_t));
        sections->next = NULL;
        sections->name = M_StringDuplicate(sectionName);
        sections->handler = handler;
    }
    else
    {
        if(strcmp(sections->name, sectionName) == 0)
            return;

        temp = sections;
        while(temp->next != NULL)
        {
            if(strcmp(temp->next->name, sectionName) == 0)
                return;
            temp = temp->next;
        }
        temp->next = malloc(sizeof(section_t));
        temp->next->next = NULL;
        temp->next->name = M_StringDuplicate(sectionName);
        temp->next->handler = handler;
    }
}

static void LoadSections(FILE *file)
{
    int i;
    char sectionName[100];
    char keyName[100];
    char value[300];

    while(!feof(file))
    {
        if(fscanf(file, "[%99[^]]]%*1[\n]", sectionName) == 1)
        {
            for(i = 0; i < arrlen(handlers); ++i)
            {
                if(handlers[i]->isHandling(sectionName))
                {
                    while(!feof(file))
                    {
                        if(fscanf(file, "%99[^\n =] = %299[^\n]%*1[\n]", keyName, value) != 2)
                        {
                            // Section end (empty line) of error
                            break;
                        }

                        handlers[i]->handleLine(keyName, value, 300);
                    }
                    if(handlers[i]->onFinishHandling)
                        handlers[i]->onFinishHandling();
                    M_AppendConfigSection(sectionName, handlers[i]);
                    break;
                }
            }
            fscanf(file, "%*[^[]");
        }
        else
        {
            printf("    M_Config: Error: Failed to load config\n");
            break;
        }
    }
}

static void ApplyDefaults()
{
    M_AppendConfigSection("General", &defaultHandler);
    BK_ApplyDefaultBindings();
    M_AppendConfigSection("Keybinds", &keybindsHandler);
}

static const char** config_path_prefixes(const char* tarname)
{
    const char* sdl_pref_path = SDL_GetPrefPath(NULL, tarname);
    const char* path_prefixes[] = {
#ifdef _WIN32
        // Exe dir or -cdrom
        M_StringDuplicate(M_ParmExists("-cdrom") ? RD_Project_CDRom_Dir : exedir),
#else
        // Exe dir
        M_StringDuplicate(exedir),
#endif
#ifndef _WIN32
        /*
         * On Windows: %AppData%\Roaming\tarname\
         * On Linux: ~/.local/share/tarname/
         * On Max OS: ~/Library/Application Support/tarname/
         */
        M_StringDuplicate(sdl_pref_path),
#endif
        // Working dir
        M_StringJoin(".", DIR_SEPARATOR_S, NULL),
        NULL
    };

    const size_t size = sizeof(path_prefixes);
    const char** ret = malloc(size);

    memcpy(ret, path_prefixes, size);
    SDL_free((void*) sdl_pref_path);

    return ret;
};

static boolean isSupportedConfig(char* config_path)
{
    FILE* file = M_fopen(config_path, "r");
    if(file == NULL)
        return false;

    char buffer[14];
    if(fread(&buffer, 1, 14, file) != 14)
    {
        fclose(file);
        return false;
    }
    if(buffer[0] != '[' && strcmp(buffer, "config_version") != 0)
    {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static config_path_t findConfigfilePaths(void)
{
    config_path_t ret = {NULL, NULL};

    const char** RD_prefixes = config_path_prefixes(PACKAGE_TARNAME);
    const char** ID_prefixes = config_path_prefixes("inter-doom");

    const char** prefixes = RD_prefixes;
    while(*prefixes != NULL && !ret.savePath)
    {
        char* config_path = M_StringJoin(*prefixes, RD_Project_TarName, ".ini", NULL);
        boolean saved = false;
        if(M_PathWritable(*prefixes))
        {
            ret.savePath = config_path;
            saved = true;
            if(M_FileExists(config_path))
            {
                ret.loadPath = config_path;
            }
        }

        if(!saved)
            free(config_path);

        prefixes++;
    }

    // ID config compatibility
    if(!ret.loadPath)
    {
        prefixes = ID_prefixes;
        while(*prefixes != NULL)
        {
            char* config_path = M_StringJoin(*prefixes, ID_Project_TarName, ".ini", NULL);
            if(M_FileExists(config_path) && isSupportedConfig(config_path))
            {
                ret.loadPath = config_path;
                printf(english_language ?
                       "Found compatible International Doom config file\n" :
                       "Найден поддерживаемый файл конфигурации International Doom\n");
                break;
            }
            free(config_path);
            prefixes++;
        }
    }

    M_FreeStringArray_NullTerminated(RD_prefixes);
    M_FreeStringArray_NullTerminated(ID_prefixes);
    return ret;
}

//
// M_LoadConfig
//
void M_LoadConfig(void)
{
    int i;
    FILE* file;
    int firstChar, cfg_version = 0;
 
    // check for a custom default file

    //!
    // @arg <file>
    //
    // Load configuration from the specified file, instead of the default
    //
    i = M_CheckParmWithArgs("-config", 1);

    if(i)
    {
        configPath.loadPath = configPath.savePath = myargv[i + 1];
    }
    else
    {
        configPath = findConfigfilePaths();
    }

    file = M_fopen(configPath.loadPath, "r");
    if(file == NULL)
    {
        // File not opened, but don't complain.
        // It's probably just the first time they ran the game.
        ApplyDefaults();
        return;
    }
    firstChar = fgetc(file);
    fseek(file, -1, SEEK_CUR);
    if(firstChar == EOF)
    {
        // Empty file
        ApplyDefaults();
        fclose(file);
        return;
    }
    if(firstChar != '[')
    {
        if(fscanf(file, "config_version = %i\n\n", &cfg_version) != 1)
        {
            printf(english_language ?
                   "    M_Config: Error: Unsupported config format\n" :
                   "    M_Config: Ошибка: Формат конфига не поддерживается\n");
            ApplyDefaults();
            fclose(file);
            return;
        }
    }

    printf(english_language ?
           "Loading configuration file:\n    %s\n" :
           "Загрузка файла конфигурации:\n    %s\n",
        configPath.loadPath);

    config_version = cfg_version;
    M_RegisterTrackedFields();

    LoadSections(file);

    fclose(file);

    if(!isBindsLoaded)
    {
        BK_ApplyDefaultBindings();
        M_AppendConfigSection("Keybinds", &keybindsHandler);
    }

    M_ApplyMigration();
}

// Get a configuration file variable by its name

static default_t *GetDefaultForName(char *name)
{
    default_t *result;

    result = SearchCollection(&default_collection, name);

    // Not found? Internal error.

    if (result == NULL)
    {
        I_QuitWithError(english_language ?
                        "Unknown configuration variable: '%s'" :
                        "Неизвестная переменная в файле конфигурации: '%s'",
                        name);
    }

    return result;
}

//
// Bind a variable to a given configuration file variable, by name.
//

void M_BindIntVariable(char *name, int *location)
{
    default_t *variable;

    variable = GetDefaultForName(name);
    assert(variable->type == DEFAULT_INT
        || variable->type == DEFAULT_INT_HEX);

    variable->location.i = location;
    variable->bound = true;
}

void M_BindFloatVariable(char *name, float *location)
{
    default_t *variable;

    variable = GetDefaultForName(name);
    assert(variable->type == DEFAULT_FLOAT);

    variable->location.f = location;
    variable->bound = true;
}

void M_BindStringVariable(char *name, char **location)
{
    default_t *variable;

    variable = GetDefaultForName(name);
    assert(variable->type == DEFAULT_STRING);

    variable->location.s = location;
    variable->bound = true;
}

static const char** savegame_path_prefixes(void)
{
    const char* sdl_pref_path = SDL_GetPrefPath(NULL, PACKAGE_TARNAME);
    const char* path_prefixes[] = {
#ifdef _WIN32
        // Exe dir or -cdrom
        M_StringJoin(M_ParmExists("-cdrom") ? RD_Project_CDRom_Dir : exedir, "savegames", DIR_SEPARATOR_S, NULL),
        M_StringJoin(getenv("USERPROFILE"), DIR_SEPARATOR_S, "Saved Games", DIR_SEPARATOR_S, PACKAGE_TARNAME, DIR_SEPARATOR_S, NULL),
#else
        // Exe dir
        M_StringJoin(exedir, "savegames", DIR_SEPARATOR_S, NULL),
#endif
#ifndef _WIN32
        /*
         * On Windows: %AppData%\Roaming\russian-doom\
         * On Linux: ~/.local/share/russian-doom/
         * On Max OS: ~/Library/Application Support/russian-doom/
         */
        M_StringJoin(sdl_pref_path, "savegames", DIR_SEPARATOR_S, NULL),
#endif
        // Working dir
        M_StringJoin(".", DIR_SEPARATOR_S, "savegames", DIR_SEPARATOR_S, NULL),
        NULL
    };

    const size_t size = sizeof(path_prefixes);
    const char** ret = malloc(size);

    memcpy(ret, path_prefixes, size);
    SDL_free((void*) sdl_pref_path);

    return ret;
};

//
// Calculate the path to the directory to use to store save games.
// Creates the directory as necessary.
//

char* M_GetSaveGameDir(void)
{
    char* savegamedir = NULL;

    int p = M_CheckParmWithArgs("-savedir", 1);
    if(p)
    {
        savegamedir = M_StringJoin(myargv[p + 1], DIR_SEPARATOR_S, NULL);
    }
    else
    {
        const char** RD_prefixes = savegame_path_prefixes();

        const char** prefixes = RD_prefixes;
        while(*prefixes != NULL)
        {
            if(M_PathWritable(*prefixes))
            {
                savegamedir = M_StringDuplicate(*prefixes);
                break;
            }
            prefixes++;
        }

        M_FreeStringArray_NullTerminated(RD_prefixes);
    }

    if(!M_FileExists(savegamedir))
    {
        M_MakeDirectory(savegamedir);
    }
    printf(english_language ?
            "Savegames folder:\n    %s\n" :
            "Сохраненные игры будут расположены в папке:\n    %s\n",
            savegamedir);
    return savegamedir;
}

char* M_GetAutoloadDir(void)
{
    char* prefix = M_DirName(configPath.savePath);
    char* autoload_path = M_StringJoin(prefix, DIR_SEPARATOR_S, "autoload", NULL);
    free(prefix);
    return autoload_path;
}

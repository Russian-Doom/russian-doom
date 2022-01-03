//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//    Configuration file interface.
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "SDL_filesystem.h"

#include "config.h"
#include "doomtype.h"
#include "doomfeatures.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "jn.h"

#ifndef ___RD_TARGET_SETUP___
    #include "rd_keybinds.h"
    #include "i_controller.h"
#endif

typedef struct section_s
{
    struct section_s* next;
    char* name;
    sectionHandler_t* handler;
} section_t;

// Location where all configuration data is stored
char *configdir, *configPath;

static section_t* sections;

// Default filenames for configuration files.
static char *config_file_name;

//
// DEFAULTS
//

typedef enum
{
    DEFAULT_INT,
    DEFAULT_INT_HEX,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
} default_type_t;

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

    // If this is a key value, the original integer scancode we read from
    // the config file before translating it to the internal key value.
    // If zero, we didn't read this value from a config file.
    int untranslated;

    // The value we translated the scancode into when we read the 
    // config file on startup.  If the variable value is different from
    // this, it has been changed and needs to be converted; otherwise,
    // use the 'untranslated' value.
    int original_translated;

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
    { #name, {NULL}, type, 0, 0, false }

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
    // [JN] Doom PWAD autoloading
    //

    CONFIG_VARIABLE_STRING(autoload_global_pwad1),
    CONFIG_VARIABLE_STRING(autoload_global_pwad2),
    CONFIG_VARIABLE_STRING(autoload_global_pwad3),
    CONFIG_VARIABLE_STRING(autoload_global_pwad4),

    CONFIG_VARIABLE_STRING(autoload_doom1_pwad1),
    CONFIG_VARIABLE_STRING(autoload_doom1_pwad2),
    CONFIG_VARIABLE_STRING(autoload_doom1_pwad3),
    CONFIG_VARIABLE_STRING(autoload_doom1_pwad4),

    CONFIG_VARIABLE_STRING(autoload_doom2_pwad1),
    CONFIG_VARIABLE_STRING(autoload_doom2_pwad2),
    CONFIG_VARIABLE_STRING(autoload_doom2_pwad3),
    CONFIG_VARIABLE_STRING(autoload_doom2_pwad4),

    CONFIG_VARIABLE_STRING(autoload_plutonia_pwad1),
    CONFIG_VARIABLE_STRING(autoload_plutonia_pwad2),
    CONFIG_VARIABLE_STRING(autoload_plutonia_pwad3),
    CONFIG_VARIABLE_STRING(autoload_plutonia_pwad4),

    CONFIG_VARIABLE_STRING(autoload_tnt_pwad1),
    CONFIG_VARIABLE_STRING(autoload_tnt_pwad2),
    CONFIG_VARIABLE_STRING(autoload_tnt_pwad3),
    CONFIG_VARIABLE_STRING(autoload_tnt_pwad4),

    //!
    // [JN] Heretic PWAD autoloading
    //

    CONFIG_VARIABLE_STRING(autoload_registered_pwad1),
    CONFIG_VARIABLE_STRING(autoload_registered_pwad2),
    CONFIG_VARIABLE_STRING(autoload_registered_pwad3),
    CONFIG_VARIABLE_STRING(autoload_registered_pwad4),

    CONFIG_VARIABLE_STRING(autoload_retail_pwad1),
    CONFIG_VARIABLE_STRING(autoload_retail_pwad2),
    CONFIG_VARIABLE_STRING(autoload_retail_pwad3),
    CONFIG_VARIABLE_STRING(autoload_retail_pwad4),

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
    // [JN] Aspect ratio.
    //

    CONFIG_VARIABLE_INT(aspect_ratio),

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
    // If non-zero, forces integer scales for resolution-independent rendering.
    //

    CONFIG_VARIABLE_INT(integer_scaling),

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
    // Sound output sample rate, in Hz.  Typical values to use are
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

    //!
    // [JN] Russian Doom menu
    //

    // Rendering
    CONFIG_VARIABLE_INT(vsync),
    CONFIG_VARIABLE_INT(aspect_ratio_correct),
    CONFIG_VARIABLE_INT(uncapped_fps),
    CONFIG_VARIABLE_INT(show_fps),
    CONFIG_VARIABLE_INT(smoothing),
    CONFIG_VARIABLE_INT(max_fps),
    CONFIG_VARIABLE_INT(show_diskicon),
    CONFIG_VARIABLE_INT(screen_wiping),
    CONFIG_VARIABLE_INT(png_screenshots),
    CONFIG_VARIABLE_INT(flashing_hom),

    // Display
    CONFIG_VARIABLE_INT(screenblocks),
    CONFIG_VARIABLE_INT(level_brightness),
    CONFIG_VARIABLE_INT(menu_shading),
    CONFIG_VARIABLE_INT(detaillevel),
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
    CONFIG_VARIABLE_INT(message_pickup_color),
    CONFIG_VARIABLE_INT(message_secret_color),
    CONFIG_VARIABLE_INT(message_system_color),
    CONFIG_VARIABLE_INT(message_chat_color),
    CONFIG_VARIABLE_INT(draw_shadowed_text),

    // Automap specific variables
    CONFIG_VARIABLE_INT(automap_color),
    CONFIG_VARIABLE_INT(automap_mark_color),
    CONFIG_VARIABLE_INT(automap_antialias),
    CONFIG_VARIABLE_INT(automap_stats),
    CONFIG_VARIABLE_INT(automap_level_time),
    CONFIG_VARIABLE_INT(automap_total_time),
    CONFIG_VARIABLE_INT(automap_coords),
    CONFIG_VARIABLE_INT(automap_overlay),
    CONFIG_VARIABLE_INT(automap_rotate),
    CONFIG_VARIABLE_INT(automap_follow),
    CONFIG_VARIABLE_INT(automap_grid),
    CONFIG_VARIABLE_INT(automap_grid_size),
    CONFIG_VARIABLE_INT(hud_widget_colors),

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
    CONFIG_VARIABLE_INT(sbar_color_high),
    CONFIG_VARIABLE_INT(sbar_color_normal),
    CONFIG_VARIABLE_INT(sbar_color_low),
    CONFIG_VARIABLE_INT(sbar_color_critical),
    CONFIG_VARIABLE_INT(sbar_color_armor_1),
    CONFIG_VARIABLE_INT(sbar_color_armor_2),
    CONFIG_VARIABLE_INT(sbar_color_armor_0),
    CONFIG_VARIABLE_INT(ammo_widget),
    CONFIG_VARIABLE_INT(ammo_widget_colored),

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
    CONFIG_VARIABLE_INT(horizontal_autoaim),
    CONFIG_VARIABLE_INT(show_all_artifacts),
    CONFIG_VARIABLE_INT(show_artifacts_timer),

    // Gameplay: Physical
    CONFIG_VARIABLE_INT(improved_collision),
    CONFIG_VARIABLE_INT(over_under),
    CONFIG_VARIABLE_INT(torque),
    CONFIG_VARIABLE_INT(weapon_bobbing),
    CONFIG_VARIABLE_INT(ssg_blast_enemies),
    CONFIG_VARIABLE_INT(floating_powerups),
    CONFIG_VARIABLE_INT(toss_drop),

    // Gameplay: Crosshair
    CONFIG_VARIABLE_INT(crosshair_draw),
    CONFIG_VARIABLE_INT(crosshair_type),
    CONFIG_VARIABLE_INT(crosshair_scale),

    // Gameplay: Gameplay
    CONFIG_VARIABLE_INT(fix_map_errors),
    CONFIG_VARIABLE_INT(flip_levels),
    CONFIG_VARIABLE_INT(unlimited_lost_souls),
    CONFIG_VARIABLE_INT(agressive_lost_souls),
    CONFIG_VARIABLE_INT(pistol_start),
    CONFIG_VARIABLE_INT(fast_quickload),

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
static boolean DefaultHandler_Handles(char* sectionName)
{
    return strcmp("General", sectionName) == 0;
}

static sectionHandler_t defaultHandler = {
    DefaultHandler_Handles,
    DefaultHandler_HandleLine,
    DefaultHandler_Save,
    NULL
};
#ifndef ___RD_TARGET_SETUP___
static sectionHandler_t keybindsHandler = {
    KeybindsHandler_Handles,
    KeybindsHandler_HandleLine,
    KeybindsHandler_Save,
    NULL
};

sectionHandler_t controllerHandler = {
    ControllerHandler_Handles,
    ControllerHandler_HandleLine,
    ControllerHandler_Save,
    ControllerHandler_onFinishHandling
};
#endif
static sectionHandler_t* handlers[] = {
    &defaultHandler,
#ifndef ___RD_TARGET_SETUP___
    &keybindsHandler,
    &controllerHandler
#endif
};
#ifndef ___RD_TARGET_SETUP___
static int handlersSize = 3;
#else
static int handlersSize = 1;
#endif

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

    def = SearchCollection(&default_collection, keyName);
    if(def == NULL || !def->bound)
    {
        // Unknown variable?  Unbound variables are also treated as unknown.
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

    SetVariable(def, value);
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
            *def->location.f = (float) atof(value);
            break;
    }
}

static void LoadDefaultCollection(FILE *file)
{
    default_t *def;
    char defname[80];
    char strparm[100];

    while(!feof(file))
    {
        if(fscanf(file, "%79s %99[^\n]\n", defname, strparm) != 2)
        {
            // This line doesn't match
            continue;
        }

        // [Dasperal] Key binds section
#ifndef ___RD_TARGET_SETUP___
        if(strcmp("Keybinds", defname) == 0 && strcmp("Start", strparm) == 0)
        {
            BK_LoadBindings(file);
            continue;
        }
#endif

        // Find the setting in the list
        def = SearchCollection(&default_collection, defname);

        if(def == NULL || !def->bound)
        {
            // Unknown variable?  Unbound variables are also treated
            // as unknown.
            continue;
        }

        // Strip off trailing non-printable characters (\r characters
        // from DOS text files)
        while(strlen(strparm) > 0 && !isprint(strparm[strlen(strparm)-1]))
        {
            strparm[strlen(strparm)-1] = '\0';
        }

        // Surrounded by quotes? If so, remove them.
        if(strlen(strparm) >= 2
        && strparm[0] == '"' && strparm[strlen(strparm) - 1] == '"')
        {
            strparm[strlen(strparm) - 1] = '\0';
            memmove(strparm, strparm + 1, sizeof(strparm) - 1);
        }

        SetVariable(def, strparm);
    }
}

// Set the default filenames to use for configuration files.

void M_SetConfigFilename(char *name)
{
    config_file_name = name;
}

//
// M_SaveConfig
//

void M_SaveConfig (void)
{
    FILE *f;
    section_t* section;

    f = fopen(configPath, "w");
    if(!f)
        return; // can't write the file, but don't complain
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

//
// Save default_collection to alternate filenames
//

void M_SaveDefaultAlternate(char *main)
{
    char *orig_main;

    // Temporarily change the filenames

    orig_main = configPath;

    configPath = main;

    M_SaveConfig();

    // Restore normal filenames

    configPath = orig_main;
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
            for(i = 0; i < handlersSize; ++i)
            {
                if(handlers[i]->handles(sectionName))
                {
                    printf(english_language ?
                           "\tM_Config: Loading section \"%s\"\n" :
                           "\tM_Config: Загрузка секции \"%s\"\n", sectionName);
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
            printf("\tM_Config: Error: Failed to load config\n");
            break;
        }
    }
}

//
// M_LoadConfig
//
void M_LoadConfig(void)
{
    int i, c;
    FILE* file;
 
    // check for a custom default file

    //!
    // @arg <file>
    //
    // Load configuration from the specified file, instead of the default
    //
    i = M_CheckParmWithArgs("-config", 1);

    if(i)
    {
        configPath = myargv[i + 1];
    }
    else
    {
        configPath = M_StringJoin(configdir, config_file_name, NULL);
    }

    printf(english_language ?
           "Loading config form %s\n" :
           "Загрузка файла конфигурации:\n \t%s\n",
           configPath);

    file = fopen(configPath, "r");
    if(file == NULL)
    {
        // File not opened, but don't complain.
        // It's probably just the first time they ran the game.
        M_AppendConfigSection("General", &defaultHandler);
#ifndef ___RD_TARGET_SETUP___
        BK_ApplyDefaultBindings();
        M_AppendConfigSection("Keybinds", &keybindsHandler);
#endif
        return;
    }

    c = fgetc(file);
    fseek(file, -1, SEEK_CUR);
    if(c != '[')
    {
        LoadDefaultCollection(file);
        M_AppendConfigSection("General", &defaultHandler);
#ifndef ___RD_TARGET_SETUP___
        if(isBindsLoaded)
            M_AppendConfigSection("Keybinds", &keybindsHandler);
#endif
    }
    else
    {
        LoadSections(file);
    }

    fclose(file);

#ifndef ___RD_TARGET_SETUP___
    if(!isBindsLoaded)
    {
        BK_ApplyDefaultBindings();
        M_AppendConfigSection("Keybinds", &keybindsHandler);
    }
#endif
}

// Get a configuration file variable by its name

static default_t *GetDefaultForName(char *name)
{
    default_t *result;

    result = SearchCollection(&default_collection, name);

    // Not found? Internal error.

    if (result == NULL)
    {
        I_Error(english_language ?
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

// Get the path to the default configuration dir to use, if NULL
// is passed to M_SetConfigDir.

static char *GetDefaultConfigDir(void)
{
    char *result;
    char* tempResult;
#if defined(_WIN32) || defined(BUILD_PORTABLE)
    result = M_StringDuplicate(exedir);
    // [Dasperal] Try to check whether writing to exedir is possible by creating a savegames directory
    tempResult = M_StringJoin(result, "savegames", NULL);
    // If the savegames directory already exists, optimistically assume that writing is possible
    if (!M_FileExists(tempResult))
    {
        M_MakeDirectory(tempResult);
        if (!M_FileExists(tempResult))
        {
            free(result);
            result = M_StringDuplicate("");
        }
    }
    free(tempResult);
#else
    tempResult = SDL_GetPrefPath("", PACKAGE_TARNAME); // This might be ~/.local/share/russian-doom
    if (tempResult != NULL)
    {
        result = M_StringDuplicate(tempResult);
        SDL_free(tempResult);
    }
    else
        result = M_StringDuplicate("");
#endif
    return result;
}

// 
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - default.ini, chocolate-doom.ini, savegames, etc.
//

void M_SetConfigDir(char *dir)
{
    // Use the directory that was passed, or find the default.

    if (dir != NULL)
    {
        configdir = dir;
    }
    else
    {
        configdir = GetDefaultConfigDir();
    }

    if (strcmp(configdir, "") != 0)
    {
        printf(english_language ?
               "Using %s for configuration\n" :
               "Настройки программы будут расположены в папке:\n \t%s\n",
               configdir);
    }

    // Make the directory if it doesn't already exist:

    M_MakeDirectory(configdir);
}

//
// Calculate the path to the directory to use to store save games.
// Creates the directory as necessary.
//

char *M_GetSaveGameDir()
{
    char *savegamedir;

    int p = M_CheckParmWithArgs("-savedir", 1);
    if (p)
    {
        savegamedir = M_StringJoin(myargv[p + 1], DIR_SEPARATOR_S, NULL);
    }
    else
    {
#ifdef _WIN32
        // In -cdrom mode, we write savegames to a specific directory
        // in addition to configs.
        if (M_ParmExists("-cdrom"))
        {
            savegamedir = configdir;
        }
        else if (0 == strcmp(configdir, ""))
        {
            char* topdir = M_StringJoin(getenv("USERPROFILE"), DIR_SEPARATOR_S, "Saved Games", NULL);
            if (M_FileExists(topdir))
                savegamedir = M_StringJoin(topdir, DIR_SEPARATOR_S, PACKAGE_TARNAME, DIR_SEPARATOR_S, NULL);
            else
                savegamedir = M_StringJoin("savegames", DIR_SEPARATOR_S, NULL);

            free(topdir);
        }
        else
#endif
            savegamedir = M_StringJoin(configdir, "savegames", DIR_SEPARATOR_S, NULL);
    }

    if (!M_FileExists(savegamedir))
    {
        M_MakeDirectory(savegamedir);
    }
    printf(english_language ?
            "Savegames folder:\n \t%s\n" :
            "Сохраненные игры будут расположены в папке:\n \t%s\n",
            savegamedir);
    return savegamedir;
}


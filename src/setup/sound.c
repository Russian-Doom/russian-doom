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



// Sound control menu

#include <stdlib.h>

#include "SDL_mixer.h"

#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"

#include "mode.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://github.com/JNechaevsky/inter-doom/wiki"

typedef enum
{
    OPLMODE_OPL2,
    OPLMODE_OPL3,
    NUM_OPLMODES,
} oplmode_t;

static char *opltype_strings[] =
{
    "OPL2",
    "OPL3"
};

static char *cfg_extension[] = { "cfg", NULL };

// Config file variables:

int snd_sfxdevice = SNDDEVICE_SB;
int snd_musicdevice = SNDDEVICE_SB;
const int snd_samplerate = 44100;
int opl_io_port = 0x388;
int snd_cachesize = 64 * 1024 * 1024;
int snd_maxslicetime_ms = 28;
char *snd_musiccmd = "";
int snd_pitchshift = 0;
int mute_inactive_window = 0;
char *snd_dmxoption = "-opl3"; // [crispy] default to OPL3 emulation

static int numChannels = 32;
static int sfxVolume = 8;
static int musicVolume = 8;
static int snd_monomode = 0; // [JN] Mono SFX
static int voiceVolume = 15;
static int show_talk = 0;
// [JN] Disable "use_libsamplerate" config variable,
// always preffering a best sound quality.
// static int use_libsamplerate = 0;
// [JN] Using 1.0 makes sound volume much more original-like,
// but also will make sfx volume notably higher than music volume.
static float libsamplerate_scale = 0.65;

static char *timidity_cfg_path = NULL;
static char *gus_patch_path = "";
static int gus_ram_kb = 1024;

static int snd_oplmode;

static void UpdateSndDevices(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    switch (snd_oplmode)
    {
        default:
        case OPLMODE_OPL2:
            snd_dmxoption = "";
            break;

        case OPLMODE_OPL3:
            snd_dmxoption = "-opl3";
            break;
    }
}

static txt_dropdown_list_t *OPLTypeSelector(void)
{
    txt_dropdown_list_t *result;

    if (snd_dmxoption != NULL && strstr(snd_dmxoption, "-opl3") != NULL)
    {
        snd_oplmode = OPLMODE_OPL3;
    }
    else
    {
        snd_oplmode = OPLMODE_OPL2;
    }

    result = TXT_NewDropdownList(&snd_oplmode, opltype_strings, 2);

    TXT_SignalConnect(result, "changed", UpdateSndDevices, NULL);

    return result;
}

void ConfigSound(void)
{
    txt_window_t *window;

    // Build the window

    window = TXT_NewWindow(english_language ?
                           "Sound configuration" :
                           "��������� �����");

    if (english_language)
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);
    else
    TXT_SetWindowHelpURL_RUS(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 40);
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 3);

    TXT_AddWidgets(window,
        TXT_NewSeparator(english_language ?
                         "Sound effects" :
                         "�������� �������"),
        TXT_NewRadioButton(english_language ?
                           "Disabled" :
                           "���������",
                           &snd_sfxdevice, SNDDEVICE_NONE),
        TXT_If(gamemission == doom,
            TXT_NewRadioButton(english_language ?
                               "PC speaker effects" :
                               "������� ��",
                               &snd_sfxdevice, SNDDEVICE_PCSPEAKER)),
        TXT_NewRadioButton(english_language ?
                           "Digital sound effects" :
                           "��������",
                           &snd_sfxdevice,
                           SNDDEVICE_SB),
        TXT_If(gamemission == doom || gamemission == heretic
            || gamemission == hexen,
            TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
                TXT_NewHorizBox(
                    TXT_NewStrut(4, 0),
                    TXT_NewCheckBox(english_language ?
                                    "Pitch-shifted sounds" :
                                    "������������ ����-�������",
                                    &snd_pitchshift),
                                    NULL))),
        TXT_If(gamemission == doom || gamemission == heretic
            || gamemission == hexen,
            TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
                TXT_NewHorizBox(
                    TXT_NewStrut(4, 0),
                    TXT_NewCheckBox(english_language ?
                                    "Mono sounds" :
                                    "���� �����",
                                    &snd_monomode),
                                    NULL))),
        TXT_If(gamemission == strife,
            TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
                TXT_NewHorizBox(
                    TXT_NewStrut(4, 0),
                    TXT_NewCheckBox(english_language ?
                                    "Show text with voices" :
                                    "���������� ��������", &show_talk),
                                    NULL))),

        TXT_NewSeparator(english_language ?
                         "Music" :
                         "������"),
        TXT_NewRadioButton(english_language ?
                           "Disabled" :
                           "���������",
                           &snd_musicdevice, SNDDEVICE_NONE),

        TXT_NewRadioButton("OPL (Adlib/Soundblaster)", &snd_musicdevice,
                           SNDDEVICE_SB),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_SB,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewLabel(english_language ?
                "Chip type: " :
                "�����: "),
                OPLTypeSelector(),
                NULL)),

        TXT_NewRadioButton(english_language ?
                           "GUS (emulated)" :
                           "GUS (��������)",
                           &snd_musicdevice, SNDDEVICE_GUS),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GUS,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewLabel(english_language ?
                             "Select directory containing GUS patches" :
                             "������������ ������ GUS: "),
                             NULL)),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GUS,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&gus_patch_path, 34, english_language ?
                                    "Select directory containing GUS patches" :
                                    "������� ���� � ������ GUS",
                                    TXT_DIRECTORY),
                NULL)),

        TXT_NewRadioButton("MIDI/MP3/OGG/FLAC/Tracker", // [crispy] improve ambigious music backend name
                           &snd_musicdevice, SNDDEVICE_GENMIDI),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GENMIDI,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewLabel(english_language ?
                             "Timidity configuration file: " :
                             "���������������� ���� Timidity: "),
                             NULL)),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GENMIDI,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&timidity_cfg_path, 34, english_language ?
                                    "Select Timidity config file" :
                                    "�������� ���������������� ���� Timidity",
                                    cfg_extension),
                NULL)),

        TXT_NewSeparator(english_language ?
                         "Misc." :
                         "�������������"),
        TXT_NewCheckBox(english_language ?
                        "Mute inactive window" :
                        "���� � ���������� ����",
                        &mute_inactive_window),

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

void BindSoundVariables(void)
{
    M_BindIntVariable("snd_sfxdevice",            &snd_sfxdevice);
    M_BindIntVariable("snd_musicdevice",          &snd_musicdevice);
    M_BindIntVariable("snd_channels",             &numChannels);
    M_BindIntVariable("sfx_volume",               &sfxVolume);
    M_BindIntVariable("music_volume",             &musicVolume);
    M_BindIntVariable("snd_monomode",             &snd_monomode);

    // [JN] Disable "use_libsamplerate" config variable,
    // always preffering a best sound quality.
    // M_BindIntVariable("use_libsamplerate",        &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale",    &libsamplerate_scale);

    M_BindIntVariable("gus_ram_kb",               &gus_ram_kb);
    M_BindStringVariable("gus_patch_path",      &gus_patch_path);
    M_BindStringVariable("timidity_cfg_path",     &timidity_cfg_path);

    M_BindIntVariable("snd_maxslicetime_ms",      &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd",          &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption",         &snd_dmxoption);

    M_BindIntVariable("snd_cachesize",            &snd_cachesize);
    M_BindIntVariable("opl_io_port",              &opl_io_port);

    M_BindIntVariable("snd_pitchshift",           &snd_pitchshift);
    M_BindIntVariable("mute_inactive_window",     &mute_inactive_window);

    if (gamemission == strife)
    {
        M_BindIntVariable("voice_volume",         &voiceVolume);
        M_BindIntVariable("show_talk",            &show_talk);
    }

    timidity_cfg_path = M_StringDuplicate("");

    // All versions of Heretic and Hexen did pitch-shifting.
    // Most versions of Doom did not and Strife never did.
    snd_pitchshift = gamemission == heretic || gamemission == hexen;

    // Default sound volumes - different games use different values.

    switch (gamemission)
    {
        case doom:
        default:
            sfxVolume = 8;  musicVolume = 8;
            break;
        case heretic:
        case hexen:
            sfxVolume = 10; musicVolume = 10;
            break;
        case strife:
            sfxVolume = 8;  musicVolume = 13;
            break;
    }
}


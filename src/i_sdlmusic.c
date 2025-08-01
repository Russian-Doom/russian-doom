//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	System interface for music.
//


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDL.h"
#include "SDL_mixer.h"

#include "i_winmusic.h"
#include "doomtype.h"
#include "memio.h"
#include "mus2mid.h"
#include "gusconf.h"
#include "i_sound.h"
#include "i_swap.h"
#include "m_misc.h"
#include "z_zone.h"

#include "jn.h"

static boolean music_initialized = false;

// If this is true, this module initialized SDL sound and has the 
// responsibility to shut it down

static boolean sdl_was_initialized = false;
#ifdef _WIN32
static boolean win_midi_stream_opened = false;
#endif
static boolean win_midi_song_registered = false;

static boolean musicpaused = false;
static int current_music_volume;

char *fluidsynth_sf_path = "";
char *timidity_cfg_path = "";

static char *temp_timidity_cfg = NULL;

// [JN] Temporal solution for proper volume control between MIDI/digital music.
static boolean is_midi_file;

// If the temp_timidity_cfg config variable is set, generate a "wrapper"
// config file for Timidity to point to the actual config file. This
// is needed to inject a "dir" command so that the patches are read
// relative to the actual config file.

static boolean WriteWrapperTimidityConfig(char *write_path)
{
    char *path;
    FILE *fstream;
    const SDL_version* sdl_mixer_version = Mix_Linked_Version();

    if(!strcmp(timidity_cfg_path, ""))
    {
        return false;
    }

    printf(english_language ?
           "I_SDLMusic: Using Timidity config from:\n    %s\n" :
           "I_SDLMusic: Используется Timidity конфиг из файла:\n    %s\n",
           timidity_cfg_path);

    if(!LIB_VERSION_ATLEAST(sdl_mixer_version, 2, 5, 0) && strchr(timidity_cfg_path, ' '))
    {
        printf(english_language ?
               "\tError: The path contains spaces, which are not supported by your SDL_mixer library. Update SDL_mixer to at least 2.5.0.\n" :
               "\tОшибка: Путь содержит пробелы, что не поддерживается вашей версией библиотеки SDL_mixer. Обновите SDL_mixer хотя бы до версии 2.5.0.\n");
    }

    fstream = M_fopen(write_path, "w");

    if(fstream == NULL)
    {
        printf(english_language ?
               "Error: Could not write Timidity config.\n" :
               "Ошибка: Не удалось записать конфигурацию Timidity.\n");
        return false;
    }

    path = M_DirName(timidity_cfg_path);
    if(LIB_VERSION_ATLEAST(sdl_mixer_version, 2, 5, 0))
    {
        fprintf(fstream, "dir \"%s\"\n", path);
    }
    else
    {
        fprintf(fstream, "dir %s\n", path);
    }
    free(path);

    if(LIB_VERSION_ATLEAST(sdl_mixer_version, 2, 5, 0))
    {
        fprintf(fstream, "source \"%s\"\n", timidity_cfg_path);
    }
    else
    {
        fprintf(fstream, "source %s\n", timidity_cfg_path);
    }
    fclose(fstream);

    return true;
}

// putenv requires a non-const string whose lifetime is the whole program
// so can't use a string directly, have to do this silliness
static char sdl_mixer_disable_fluidsynth[] = "SDL_MIXER_DISABLE_FLUIDSYNTH=1";

void I_InitTimidityConfig(void)
{
    char *env_string;
    boolean success;

    temp_timidity_cfg = M_TempFile("timidity.cfg");

    if (snd_musicdevice == SNDDEVICE_GUS)
    {
        success = GUS_WriteConfig(temp_timidity_cfg);
    }
    else
    {
        success = WriteWrapperTimidityConfig(temp_timidity_cfg);
    }

    // Set the TIMIDITY_CFG environment variable to point to the temporary
    // config file.

    if (success)
    {
        env_string = M_StringJoin("TIMIDITY_CFG=", temp_timidity_cfg, NULL);
        putenv(env_string);
        // env_string deliberately not freed; see putenv manpage

        // If we're explicitly configured to use Timidity (either through
        // timidity_cfg_path or GUS mode), then disable Fluidsynth, because
        // SDL_mixer considers Fluidsynth a higher priority than Timidity and
        // therefore can end up circumventing Timidity entirely.
        putenv(sdl_mixer_disable_fluidsynth);
    }
    else
    {
        free(temp_timidity_cfg);
        temp_timidity_cfg = NULL;
    }
}

// Remove the temporary config file generated by I_InitTimidityConfig().

static void RemoveTimidityConfig(void)
{
    if (temp_timidity_cfg != NULL)
    {
        M_remove(temp_timidity_cfg);
        free(temp_timidity_cfg);
    }
}

// Shutdown music

static void I_SDL_ShutdownMusic(void)
{
    if (music_initialized)
    {
#if defined(_WIN32)
        if (win_midi_stream_opened)
        {
            I_WIN_ShutdownMusic();
			win_midi_stream_opened = false;
        }
#endif
        Mix_HaltMusic();
        music_initialized = false;

        if (sdl_was_initialized)
        {
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            sdl_was_initialized = false;
        }
    }
}

static boolean SDLIsInitialized(void)
{
    int freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels) != 0;
}

// Initialize music subsystem
static boolean I_SDL_InitMusic(void)
{
    // If SDL_mixer is not initialized, we have to initialize it
    // and have the responsibility to shut it down later on.

    if (SDLIsInitialized())
    {
        music_initialized = true;
    }
    else
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            printf("Unable to set up sound.\n");
        }
        else if (Mix_OpenAudioDevice(snd_samplerate, AUDIO_S16SYS, 2, 1024, NULL, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
        {
            printf("Error initializing SDL_mixer: \'%s\'.\n",
                    SDL_GetError());
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }
        else
        {
            SDL_PauseAudio(0);

            sdl_was_initialized = true;
            music_initialized = true;
        }
    }

#if defined(SDL_MIXER_VERSION_ATLEAST)
#if SDL_MIXER_VERSION_ATLEAST(2,0,2)
    // Initialize SDL_Mixer for MIDI music playback
    Mix_Init(MIX_INIT_MID | MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_MOD); // [crispy] initialize some more audio formats
#endif
#endif

    // Once initialization is complete, the temporary Timidity config
    // file can be removed.

    RemoveTimidityConfig();

    // When using FluidSynth, proceed to set the soundfont path via
    // Mix_SetSoundFonts if necessary.

    if (snd_musicdevice != SNDDEVICE_GUS && strlen(fluidsynth_sf_path) > 0 && strlen(timidity_cfg_path) == 0)
    {
        printf(english_language ?
               "I_SDLMusic: Using FluidSynth soundfont from:\n    %s\n" :
               "I_SDLMusic: Используется FluidSynth soundfont из файла:\n    %s\n",
               fluidsynth_sf_path);

        Mix_SetSoundFonts(fluidsynth_sf_path);
    }

    if(snd_musicdevice != SNDDEVICE_GUS && strlen(fluidsynth_sf_path) == 0 && strlen(timidity_cfg_path) == 0)
    {
        printf(english_language ?
               "I_SDLMusic: Using SDL-defined MIDI backend.\n" :
               "I_SDLMusic: Используется MIDI бэкэнд определённый SDL.\n");
    }

    // If snd_musiccmd is set, we need to call Mix_SetMusicCMD to
    // configure an external music playback program.

    if (strlen(snd_musiccmd) > 0)
    {
        Mix_SetMusicCMD(snd_musiccmd);
    }

#if defined(_WIN32)
    // [JN] Don't enable it for GUS, since it handles its own volume just fine.
    if (snd_musicdevice != SNDDEVICE_GUS && strlen(timidity_cfg_path) == 0)
    {
        win_midi_stream_opened = I_WIN_InitMusic();
    }
#endif

    return music_initialized;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//

static void UpdateMusicVolume(void)
{
    int vol;

    if (musicpaused)
    {
        vol = 0;
    }
    else
    {
        vol = (current_music_volume * MIX_MAX_VOLUME) / 127;
    }

#if defined(_WIN32)
    I_WIN_SetMusicVolume(vol);
#endif
    Mix_VolumeMusic(vol);
}

// Set music volume (0 - 15)

static void I_SDL_SetMusicVolume(int volume)
{
    // Internal state variable.
    if(volume == 0)
    {
        current_music_volume = 0;
    }
    else
    {
#ifdef _WIN32
        if(snd_musicdevice != SNDDEVICE_GUS && strlen(timidity_cfg_path) == 0)
        {
            current_music_volume = (is_midi_file ? 40 : 2) + volume * 5;
        }
        else
#endif
            current_music_volume = 2 + volume * 2;
    }
    UpdateMusicVolume();
}

// Start playing a mid

static void I_SDL_PlaySong(void *handle, boolean looping)
{
    int loops;

    if (!music_initialized)
    {
        return;
    }

    if (handle == NULL && !win_midi_song_registered)
    {
        return;
    }

    if (looping)
    {
        loops = -1;
    }
    else
    {
        loops = 1;
    }

#if defined(_WIN32)
    if (win_midi_song_registered)
    {
        I_WIN_PlaySong(looping);
    }
    else
#endif
    {
        Mix_PlayMusic((Mix_Music *) handle, loops);
    }
}

static void I_SDL_PauseSong(void)
{
    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (win_midi_stream_opened)
    {
        I_WIN_PauseSong();
    }
    else
#endif
    {
        musicpaused = true;

        UpdateMusicVolume();
    }
}

static void I_SDL_ResumeSong(void)
{
    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (win_midi_stream_opened)
    {
        I_WIN_ResumeSong();
    }
    else
#endif
    {
        musicpaused = false;

        UpdateMusicVolume();
    }
}

static void I_SDL_StopSong(void)
{
    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (win_midi_song_registered)
    {
        I_WIN_StopSong();
    }
    else
#endif
    {
        Mix_HaltMusic();
    }
}

static void I_SDL_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialized)
    {
        return;
    }

#if defined(_WIN32)
    if (win_midi_song_registered)
    {
        I_WIN_UnRegisterSong();
		win_midi_song_registered = false;
    }
    else
#endif
    {
        if (handle != NULL)
        {
            Mix_FreeMusic(music);
        }
    }
}

// Determine whether memory block is a .mid file 
#if defined(_WIN32)
static boolean IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}
#endif

static boolean IsMus(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MUS\x1a", 4);
}

static boolean ConvertMus(byte *musdata, int len, const char *filename)
{
    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outbuf_len;
    int result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

static void *I_SDL_RegisterSong(void *data, int len)
{
    char *filename;
    Mix_Music *music;

    if (!music_initialized)
    {
        return NULL;
    }

    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature

    filename = M_TempFile("doom"); // [crispy] generic filename

    // [crispy] Reverse Choco's logic from "if (MIDI)" to "if (not MUS)"
    // MUS is the only format that requires conversion,
    // let SDL_Mixer figure out the others
/*
    if (IsMid(data, len) && len < MAXMIDLENGTH)
*/
    if (!IsMus(data, len)) // [crispy] MUS_HEADER_MAGIC
    {
        M_WriteFile(filename, data, len);
        // [JN] Indicate it's not a MIDI file.
        is_midi_file = false;
    }
    else
    {
	// Assume a MUS file and try to convert

        ConvertMus(data, len, filename);
        // [JN] Indicate it is a MIDI file.
        is_midi_file = true;
    }

    // Load the MIDI. In an ideal world we'd be using Mix_LoadMUS_RW()
    // by now, but Mix_SetMusicCMD() only works with Mix_LoadMUS(), so
    // we have to generate a temporary file.

#if defined(_WIN32)
    // If we do not have an external music command defined, play
    // music with the Windows native MIDI.
    if (win_midi_stream_opened && (IsMus(data, len) || IsMid(data, len)))
    {
        if (I_WIN_RegisterSong(filename))
        {
            music = (void *) 1;
			win_midi_song_registered = true;
        }
        else
        {
            music = NULL;
            printf(english_language ?
                    "Error loading midi: Failed to register song.\n" :
                    "Ошибка загрузки при регистрации midi файла.\n");
        }
    }
    else
#endif
    {
        music = Mix_LoadMUS(filename);
        if (music == NULL)
        {
            // Failed to load
            printf(english_language ?
                    "Error loading midi: \'%s\'.\n" :
                    "Ошибка загрузки midi: \'%s\'.\n", SDL_GetError());
        }

        // Remove the temporary MIDI file; however, when using an external
        // MIDI program we can't delete the file. Otherwise, the program
        // won't find the file to play. This means we leave a mess on
        // disk :(

        if (strlen(snd_musiccmd) == 0)
        {
            M_remove(filename);
        }
    }

    free(filename);

    return music;
}

// Is the song playing?
static boolean I_SDL_MusicIsPlaying(void)
{
    if (!music_initialized)
    {
        return false;
    }

    return Mix_PlayingMusic();
}

static snddevice_t music_sdl_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

music_module_t music_sdl_module =
{
    music_sdl_devices,
    arrlen(music_sdl_devices),
    I_SDL_InitMusic,
    I_SDL_ShutdownMusic,
    I_SDL_SetMusicVolume,
    I_SDL_PauseSong,
    I_SDL_ResumeSong,
    I_SDL_RegisterSong,
    I_SDL_UnRegisterSong,
    I_SDL_PlaySong,
    I_SDL_StopSong,
    I_SDL_MusicIsPlaying,
    NULL,  // Poll
};


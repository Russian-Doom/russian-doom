//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  none
//



#include <stdio.h>
#include <stdlib.h>

#include "i_sound.h"
#include "i_system.h"

#include "doomfeatures.h"
#include "deh_str.h"

#include "doomstat.h"
#include "doomtype.h"

#include "sounds.h"
#include "s_sound.h"

#include "m_misc.h"
#include "m_random.h"
#include "m_argv.h"

#include "p_local.h"
#include "w_wad.h"
#include "z_zone.h"

#include "crispy.h"
#include "jn.h"

// when to clip out sounds
// Does not fit the large outdoor areas.

#define S_CLIPPING_DIST (1200 * FRACUNIT)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT).  Changed back to the
// Vanilla value of 200 (why was this changed?)

#define S_CLOSE_DIST (200 * FRACUNIT)

// The range over which sound attenuates

#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation

#define S_STEREO_SWING (96 * FRACUNIT)
static int stereo_swing;

#define NORM_PRIORITY 64
#define NORM_SEP 128

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t *sfxinfo;

    // origin of sound
    mobj_t *origin;

    // handle of the sound being played
    int handle;

    int pitch;

} channel_t;

// The set of channels available

static channel_t *channels;

// Internal volume level, ranging from 0-127

static int snd_SfxVolume;

// Whether songs are mus_paused

static boolean mus_paused;

// Music currently being played

static musicinfo_t *mus_playing = NULL;



// [JN] External music number, used for music system hot-swapping.

int music_num_rd;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
    int i;

    if (gameversion == exe_doom_1_666)
    {
        if (logical_gamemission == doom)
        {
            I_SetOPLDriverVer(opl_doom1_1_666);
        }
        else
        {
            I_SetOPLDriverVer(opl_doom2_1_666);
        }
    }
    else
    {
        I_SetOPLDriverVer(opl_doom_1_9);
    }

    I_PrecacheSounds(S_sfx, NUMSFX);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    // [JN] Make sound channels multiple by four:
    snd_channels -= snd_channels % 4;

    // [JN] Correct unacceptable values:
    if (snd_channels <= 4)  
        snd_channels = 4;
    else
    if (snd_channels >= 64)
        snd_channels = 64;

    // [JN] Cap sound channels to 8 in -vanilla game mode.
    if (vanillaparm)
    {
        snd_channels = 8;
    }

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    channels = I_Realloc(NULL, snd_channels*sizeof(channel_t));

    // Free all channels for use
    for (i=0 ; i<snd_channels ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }

    // Doom defaults to pitch-shifting off.
    if (snd_pitchshift == -1)
    {
        snd_pitchshift = 0;
    }

    I_AtExit(S_Shutdown, true);

    // [crispy] handle stereo separation for mono-sfx and flipped levels
    S_UpdateStereoSeparation();
}

// -----------------------------------------------------------------------------
// S_RD_Change_SoundDevice
// [JN] Routine for sfx device hot-swapping.
// -----------------------------------------------------------------------------

void S_RD_Change_SoundDevice (void)
{
    int i;

    // Regenerate cache of digital sounds
    I_PrecacheSounds(S_sfx, NUMSFX);

    // Free all channels for use
    for (i=0 ; i<snd_channels ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // Reinitialize sfx usefulness
    for (i=1 ; i<NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }
}

// -----------------------------------------------------------------------------
// S_ChannelsRealloc
// [JN] Reallocates sound channels, needed for hot-swapping.
// -----------------------------------------------------------------------------

void S_ChannelsRealloc(void)
{
    int i;

    // Safeguard conditions:
    if (snd_channels < 4)
        snd_channels = 4;
    if (snd_channels > 64)
        snd_channels = 64;

    channels = I_Realloc(channels, snd_channels * sizeof(channel_t));
    for (i=0 ; i<snd_channels ; i++)
    {
        channels[i].sfxinfo = 0;
    }
}

// -----------------------------------------------------------------------------
// S_UpdateStereoSeparation
// [JN] Defines stereo separtion for mono sfx mode and flipped levels.
// -----------------------------------------------------------------------------

void S_UpdateStereoSeparation(void)
{
	// [crispy] play all sound effects in mono
	if (snd_monomode)
	{
		stereo_swing = 0;
	}
	else
	if (flip_levels)
	{
		stereo_swing = -S_STEREO_SWING;
	}
	else
	{
		stereo_swing = S_STEREO_SWING;
	}
}

void S_Shutdown(void)
{
    I_ShutdownSound();
}

static void S_StopChannel(int cnum)
{
    int i;
    channel_t *c;

    c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing

        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

        // check to see if other channels are playing the sound

        for (i=0; i<snd_channels; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        // degrade usefulness of sound data

        c->sfxinfo->usefulness--;
        c->sfxinfo = NULL;
        c->origin = NULL;
    }
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start(void)
{
    int cnum;
    int mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo)
        {
            S_StopChannel(cnum);
        }
    }

    // start new music for the level
    if (musicVolume) // [crispy] do not reset pause state at zero music volume
    mus_paused = 0;

    // [JN] Добавена нумерация музыки для No Rest for the Living
    // Спасибо Фабиану Греффрату!
    if (gamemode == commercial)
    {
        const int nmus[] =
        {
            mus_messag,
            mus_ddtblu,
            mus_doom,
            mus_shawn,
            mus_in_cit,
            mus_the_da,
            mus_in_cit,
            mus_shawn2,
            mus_ddtbl2,
        };

        if (gamemission == pack_nerve && gamemap <= arrlen(nmus))
        {
            mnum = nmus[gamemap - 1];
        }
        else
        mnum = mus_runnin + gamemap - 1;
    } 
    else
    {
        int spmus[]=
        {
            // Song - Who? - Where?

            mus_e3m4,        // American     e4m1
            mus_e3m2,        // Romero       e4m2
            mus_e3m3,        // Shawn        e4m3
            mus_e1m5,        // American     e4m4
            mus_e2m7,        // Tim          e4m5
            mus_e2m4,        // Romero       e4m6
            mus_e2m6,        // J.Anderson   e4m7 CHIRON.WAD
            mus_e2m5,        // Shawn        e4m8
            mus_e1m9,        // Tim          e4m9
        };

        // [JN] Doom 1.9 Special Edition EP4 music
        const int spmus_se[]=
        {
            mus_e2m6,   // e4m1
            mus_e3m2,   // e4m2
            mus_e3m3,   // e4m3
            mus_e1m4,   // e4m4
            mus_e2m7,   // e4m5
            mus_e2m4,   // e4m6
            mus_e1m7,   // e4m7
            mus_e1m6,   // e4m8
            mus_e1m6,   // e4m9
        };

        if (gameepisode < 4 || gameepisode == 5) // [crispy] Sigil 
        {
            mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
        }
        else
        {
            mnum = gameversion == exe_doom_se ?
                                  spmus_se[gamemap-1] :
                                  spmus[gamemap-1];
        }
    }

    S_ChangeMusic(mnum, true);
}

void S_StopSound(mobj_t *origin)
{
    int cnum;

    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}

//
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int                cnum;

    channel_t*        c;

    // Find an open channel
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (!channels[cnum].sfxinfo)
        {
            break;
        }
        else if (origin && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == snd_channels)
    {
        // Look for lower priority
        for (cnum=0 ; cnum<snd_channels ; cnum++)
        {
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
            {
                break;
            }
        }

        if (cnum == snd_channels)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority.
            S_StopChannel(cnum);
        }
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}

//
// Changes volume and stereo-separation variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

static int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep)
{
    fixed_t        approx_dist;
    fixed_t        adx;
    fixed_t        ady;
    fixed_t        adz; // [JN] Z-axis sfx distance
    angle_t        angle;

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);
    adz = !z_axis_sfx || vanillaparm ? 0 :
          abs(listener->z - source->z);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady + adz - ((adx < ady ? adx : ady)>>1);

    // [crispy] proper sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8
    if ((gamemap != 8 || (!vanillaparm && (gamemode == commercial || gameepisode == 4))) && approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
                            listener->y,
                            source->x,
                            source->y);

    if (angle > listener->angle)
    {
        angle = angle - listener->angle;
    }
    else
    {
        angle = angle + (0xffffffff - listener->angle);
    }

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(stereo_swing, finesine[angle]) >> FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        *vol = snd_SfxVolume;
    }
    // [crispy] proper sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8
    else if (gamemap == 8 && ((gamemode != commercial && gameepisode != 4) || vanillaparm))
    {
        if (approx_dist > S_CLIPPING_DIST)
        {
            approx_dist = S_CLIPPING_DIST;
        }

        *vol = 15+ ((snd_SfxVolume-15)
                    *((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
            / S_ATTENUATOR;
    }
    else
    {
        // distance effect
        *vol = (snd_SfxVolume
                * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
            / S_ATTENUATOR;
    }

    return (*vol > 0);
}

// clamp supplied integer to the range 0 <= x <= 255.

static int Clamp(int x)
{
    if (x < 0)
    {
        return 0;
    }
    else if (x > 255)
    {
        return 255;
    }
    return x;
}

void S_StartSound(void *origin_p, int sfx_id)
{
    sfxinfo_t *sfx;
    mobj_t *origin;
    int rc;
    int sep;
    int pitch;
    int cnum;
    int volume;

    origin = (mobj_t *) origin_p;
    volume = snd_SfxVolume;

    // [crispy] make non-fatal, consider zero volume
    if (sfx_id == sfx_None || !snd_SfxVolume)
    {
        return;
    }

    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
    {
        I_Error(english_language ?
                "Bad sfx #: %d" :
                "Некорректный звук #: %d",
                sfx_id);
    }

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    pitch = NORM_PITCH;
    if (sfx->link)
    {
        volume += sfx->volume;
        pitch = sfx->pitch;

        if (volume < 1)
        {
            return;
        }

        if (volume > snd_SfxVolume)
        {
            volume = snd_SfxVolume;
        }
    }


    // Check to see if it is audible,
    //  and if not, modify the params
    if (origin && origin != players[consoleplayer].mo)
    {
        rc = S_AdjustSoundParams(players[consoleplayer].mo,
                                 origin,
                                 &volume,
                                 &sep);

        if (origin->x == players[consoleplayer].mo->x
         && origin->y == players[consoleplayer].mo->y)
        {
            sep = NORM_SEP;
        }

        if (!rc)
        {
            return;
        }
    }
    else
    {
        sep = NORM_SEP;
    }

    // hacks to vary the sfx pitches
    if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    {
        pitch += 8 - (M_Random()&15);
    }
    else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
    {
        pitch += 16 - (M_Random()&31);
    }
    pitch = Clamp(pitch);

    // kill old sound
    S_StopSound(origin);

    // try to find a channel
    cnum = S_GetChannel(origin, sfx);

    if (cnum < 0)
    {
        return;
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0)
    {
        sfx->usefulness = 1;
    }

    if (sfx->lumpnum < 0)
    {
        sfx->lumpnum = I_GetSfxLumpNum(sfx);
    }

    // [JN] Atari Jaguar have down-pitched sounds
    if (gamemission == jaguar)
    {
        pitch = 122;
    }

    channels[cnum].pitch = pitch;
    channels[cnum].handle = I_StartSound(sfx, cnum, volume, sep, channels[cnum].pitch);
}

void S_StartSoundOnce (void *origin_p, int sfx_id)
{
    int cnum;
    const sfxinfo_t *const sfx = &S_sfx[sfx_id];

    for (cnum = 0; cnum < snd_channels; cnum++)
    {
        if (channels[cnum].sfxinfo == sfx &&
            channels[cnum].origin == origin_p)
        {
            return;
        }
    }

    S_StartSound(origin_p, sfx_id);
}

// -----------------------------------------------------------------------------
// S_StartSoundNoBreak 
// [JN] Plays unbreakable sound on the last available sound channel.
// -----------------------------------------------------------------------------

void S_StartSoundNoBreak(int sfx_id)
{
    sfxinfo_t *sfx = &S_sfx[sfx_id];

    // [JN] Do not play sound if not audible.
    if (sfx_id < 1 || sfx_id > NUMSFX || !snd_SfxVolume)
    {
        return;
    }

    // [JN] Set poper lump number for playing.
    if (sfx->lumpnum < 0)
    {
        sfx->lumpnum = I_GetSfxLumpNum(sfx);
    }

    // [JN] Start sound with given parameters:
    channels[snd_channels-1].handle = 
    I_StartSound(sfx,            // SFX to play
                 snd_channels-1, // Use last available channel
                 snd_SfxVolume,  // Use maximum available volume
                 NORM_SEP,       // Normal stereo separation
                 NORM_PITCH);    // Normal pitch
}

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates music & sounds
//

void S_UpdateSounds(mobj_t *listener)
{
    int                audible;
    int                cnum;
    int                volume;
    int                sep;
    sfxinfo_t*        sfx;
    channel_t*        c;

    I_UpdateSound();

    for (cnum=0; cnum<snd_channels; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = snd_SfxVolume;
                sep = NORM_SEP;

                if (sfx->link)
                {
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                    else if (volume > snd_SfxVolume)
                    {
                        volume = snd_SfxVolume;
                    }
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin)
                {
                    audible = S_AdjustSoundParams(listener,
                                                  c->origin,
                                                  &volume,
                                                  &sep);

                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
                    {
                        I_UpdateSoundParams(c->handle, volume, sep);
                    }
                }
            }
            else
            {
                // if channel is allocated but sound has stopped,
                //  free it
                S_StopChannel(cnum);
            }
        }
    }
}

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error(english_language ?
                "Attempt to set music volume at %d" :
                "Попытка установить громкость музыки на %d",
                volume);
    }

    // [crispy] & [JN] Fixed bug when music was hearable with zero volume
    if (musicVolume == 0)
    {
        S_PauseSound();
    }
    else if (!paused)
    {
        S_ResumeSound();
    }

    I_SetMusicVolume(volume);
}

void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error(english_language ?
                "Attempt to set sfx volume at %d" :
                "Попытка установить громкость звука на %d", volume);
    }

    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//

void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}

void S_ChangeMusic(int musicnum, int looping)
{
    musicinfo_t *music = NULL;
    char namebuf[9];
    void *handle;

    // The Doom IWAD file has two versions of the intro music: d_intro
    // and d_introa.  The latter is used for OPL playback.
    //
    // [JN] Some older shareware version does not have d_introa music.
    // I don't have a definitions for them, so let's just don't play
    // d_introa in shareware mode at all.

    if (musicnum == mus_intro && gamemode != shareware
                              && (snd_musicdevice == SNDDEVICE_ADLIB
                              ||  snd_musicdevice == SNDDEVICE_SB))
    {
        musicnum = mus_introa;
    }

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    {
        I_Error(english_language ?
                "Bad music number %d" :
                "Некорректный номер музыки %d",
                musicnum);
    }
    else
    {
        music = &S_music[musicnum];
    }

    if (mus_playing == music)
    {
        return;
    }

    // [JN] After inner muscial number has been set, sync it with
    // external number, used in M_RD_Change_MusicDevice.
    music_num_rd = musicnum;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", DEH_String(music->name));
        music->lumpnum = W_GetNumForName(namebuf);
    }

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);

    handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
    music->handle = handle;
    I_PlaySong(handle, looping);

    mus_playing = music;
}

boolean S_MusicPlaying(void)
{
    return I_MusicIsPlaying();
}

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
        {
            I_ResumeSong();
        }

        I_StopSong();
        I_UnRegisterSong(mus_playing->handle);
        W_ReleaseLumpNum(mus_playing->lumpnum);
        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}

// -----------------------------------------------------------------------------
// S_MuteSound
// [JN] Sets sound and music volume to 0, stops all sounds in all channels.
// -----------------------------------------------------------------------------

void S_MuteSound(void)
{
    int i;

    S_SetMusicVolume(0);
    S_SetSfxVolume(0);
    for (i=0; i<snd_channels; i++)
    {
        S_StopChannel(i);
    }

    volume_needs_update = false;
}

// -----------------------------------------------------------------------------
// S_UnMuteSound
// [JN] Restores sound and music volume.
// -----------------------------------------------------------------------------

void S_UnMuteSound(void)
{
    S_SetMusicVolume(musicVolume * 8);
    S_SetSfxVolume(sfxVolume * 8);

    volume_needs_update = false;
}


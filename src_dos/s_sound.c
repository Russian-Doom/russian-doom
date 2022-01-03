//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2022 Julian Nechaevsky
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

#include "i_system.h"
#include "i_sound.h"
#include "sounds.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"
#include "p_local.h"
#include "doomstat.h"
#include "dmx.h"
#include "dpmiapi.h"
#include "jn.h"


#define S_MAX_VOLUME        127

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST     (1200 * FRACUNIT)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
#define S_CLOSE_DIST		(200 * FRACUNIT)

// The range over which sound attenuates.
#define S_ATTENUATOR        ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PITCH          128
#define NORM_PRIORITY       64
#define NORM_SEP            128

// Stereo separation.
#define S_STEREO_SWING      (96 * FRACUNIT)


typedef struct
{
    sfxinfo_t  *sfxinfo;    // sound information (if null, channel avail.)
    void       *origin;     // origin of sound
    int         handle;     // handle of the sound being played
} channel_t;


// the set of channels available
static channel_t *channels;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
static int snd_SfxVolume;

// Maximum volume of music. Useless so far.
static int snd_MusicVolume;

extern int sfxVolume;
extern int musicVolume;

// whether songs are mus_paused
static boolean      mus_paused;	

// music currently being played
static musicinfo_t *mus_playing = NULL;

// following is set
//  by the defaults code in M_misc:
// number of channels available
int         numChannels;	
static int  nextcleanup;

//
// Internals.
//
int S_getChannel (void *origin, sfxinfo_t *sfxinfo);
int S_AdjustSoundParams (mobj_t *listener, mobj_t *source, int *vol, int *sep, int *pitch);
void S_StopChannel (int cnum);

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error(english_language ?
                "Attempt to set music volume at %d" :
                "Попытка установить громкость музыки на значение %d",
                volume);
    }

    I_SetMusicVolume(volume);
    snd_MusicVolume = volume;

    // [crispy] & [JN] Fixed bug when music was hearable with zero volume
    if (musicVolume == 0)
    {
        S_PauseSound();
    }
    else if (!paused)
    {
        S_ResumeSound();
    }
}


void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
        {
            I_ResumeSong(mus_playing->handle);
        }

        I_StopSong(mus_playing->handle);
        I_UnRegisterSong(mus_playing->handle);
        Z_ChangeTag(mus_playing->data, PU_CACHE);

        _dpmi_unlockregion(mus_playing->data, lumpinfo[mus_playing->lumpnum].size);

        mus_playing->data = 0;
        mus_playing = 0;
    }
}


void S_ChangeMusic (int musicnum, int looping)
{
    char         namebuf[9];
    musicinfo_t *music;

    if (snd_MusicDevice == snd_Adlib && musicnum == mus_intro)
    {
        musicnum = mus_introa;
    }

    if (musicnum >= NUMMUSIC)
    {
        musicnum = mus_runnin + (musicnum % (NUMMUSIC - mus_runnin));
    }

    if ((musicnum <= mus_None) || (musicnum >= NUMMUSIC))
    {
	    I_Error(english_language ?
                "Bad music number %d" :
                "Некорректный номер музыки: %d", musicnum);
    }
    else
    {
        music = &S_music[musicnum];
    }

    if (mus_playing == music)
    {
        return;
    }

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        sprintf(namebuf, "d_%s", music->name);
        music->lumpnum = W_GetNumForName(namebuf);
    }

    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
    music->handle = I_RegisterSong(music->data);
    _dpmi_lockregion(music->data, lumpinfo[music->lumpnum].size);

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}


//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic (int m_id)
{
    S_ChangeMusic(m_id, false);
}


void S_StopChannel (int cnum)
{
    int        i;
    channel_t *c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing
        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

        // check to see
        //  if other channels are playing the sound
        for (i=0 ; i<numChannels ; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        // degrade usefulness of sound data
        c->sfxinfo->usefulness--;
        c->sfxinfo = 0;
    }
}


//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int S_AdjustSoundParams (mobj_t *listener, mobj_t *source, int *vol, int *sep, int *pitch)
{
    fixed_t approx_dist;
    fixed_t adx;
    fixed_t ady;
    angle_t angle;

    // calculate the distance to sound origin and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

    // [crispy] proper sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8
    if ((gamemap != 8 || (!vanilla && (!commercial || gameepisode == 4)))
    &&  approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

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
    if (snd_monomode)
    {
        *sep = 128;
    }
    else
    {
        *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);
    }

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        *vol = snd_SfxVolume;
    }
    // [crispy] proper sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8
    else if (gamemap == 8 && ((!commercial && gameepisode != 4) || vanilla))
    {
        if (approx_dist > S_CLIPPING_DIST)
        {
            approx_dist = S_CLIPPING_DIST;
        }

        *vol = 15+ ((snd_SfxVolume-15) 
             * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS)) / S_ATTENUATOR;
    }
    else
    {
        // distance effect
        *vol = (snd_SfxVolume
             * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS)) / S_ATTENUATOR; 
    }

    return (*vol > 0);
}


void S_SetSfxVolume (int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error(english_language ?
                "Attempt to set sfx volume at %d" :
                "Попытка установить громкость звука на значение %d", volume);
    }

    snd_SfxVolume = volume;
}


void S_StartSoundOnce (void *origin_p, int sfx_id)
{
    int cnum;
    const sfxinfo_t *const sfx = &S_sfx[sfx_id];

    for (cnum = 0; cnum < numChannels; cnum++)
    {
        if (channels[cnum].sfxinfo == sfx &&
            channels[cnum].origin == origin_p)
        {
            return;
        }
    }

    S_StartSound(origin_p, sfx_id);
}


//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound (void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong(mus_playing->handle);
        mus_paused = true;
    }
}


void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong(mus_playing->handle);
        mus_paused = false;
    }
}


void S_StopSound(void *origin)
{
    int cnum;

    for (cnum = 0 ; cnum < numChannels ; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}


//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int S_getChannel (void *origin, sfxinfo_t *sfxinfo)
{
    int		   cnum;   // channel number to use
    channel_t *c;

    // Find an open channel
    for (cnum = 0 ; cnum < numChannels ; cnum++)
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
    if (cnum == numChannels)
    {
        // Look for lower priority
        for (cnum = 0 ; cnum<numChannels ; cnum++)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;

        if (cnum == numChannels)
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


void S_StartSoundAtVolume (void *origin_p, int sfx_id, int volume)
{
    int         rc;
    int         sep;
    int         pitch;
    int         priority;
    int         cnum;
    mobj_t     *origin = (mobj_t *) origin_p;
    sfxinfo_t  *sfx;
  
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
                "Некорректный номер звука: %d", sfx_id);
    }

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    if (sfx->link)
    {
        pitch = sfx->pitch;
        priority = sfx->priority;
        volume += sfx->volume;

        if (volume < 1)
        {
            return;
        }

        if (volume > snd_SfxVolume)
        {
            volume = snd_SfxVolume;
        }
    }
    else
    {
        pitch = NORM_PITCH;
        priority = NORM_PRIORITY;
    }

    // Check to see if it is audible,
    //  and if not, modify the params
    if (origin && origin != players[consoleplayer].mo)
    {
        rc = S_AdjustSoundParams(players[consoleplayer].mo,
                                 origin,
                                 &volume,
                                 &sep,
                                 &pitch);
	
        if (origin->x == players[consoleplayer].mo->x
        &&  origin->y == players[consoleplayer].mo->y)
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

        if (pitch<0)
        pitch = 0;
        else if (pitch>255)
        pitch = 255;
    }
    else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
    {
        pitch += 16 - (M_Random()&31);

        if (pitch<0)
        pitch = 0;
        else if (pitch>255)
        pitch = 255;
    }

    // kill old sound
    S_StopSound(origin);

    // try to find a channel
    cnum = S_getChannel(origin, sfx);

    if (cnum<0)
    {
        return;
    }

    //
    // This is supposed to handle the loading/caching.
    // For some odd reason, the caching is done nearly
    //  each time the sound is needed?
    //

    // get lumpnum if necessary
    if (sfx->lumpnum < 0)
    {
        sfx->lumpnum = I_GetSfxLumpNum(sfx);
    }

    // cache data if necessary
    if (!sfx->data)
    {
        sfx->data = (void *) W_CacheLumpNum(sfx->lumpnum, PU_MUSIC);
    _dpmi_lockregion(sfx->data, lumpinfo[sfx->lumpnum].size);
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0)
    {
        sfx->usefulness = 1;
    }

    // Assigns the handle to one of the channels in the
    //  mix/output buffer.
    channels[cnum].handle = I_StartSound(sfx_id,
                                         sfx->data,
                                         volume,
                                         sep,
                                         pitch,
                                         priority);
}


void S_StartSound (void *origin, int sfx_id)
{
    S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume);
}


//
// [JN] Play unbreakable NULL-origin sound.
// Used by Icon of Sin for preventing it's sounds
// being breaked by player's "oof" and few others. 
//
void S_StartSoundNoBreak(int sfx_id)
{
    int sep;
    int pitch;
    int priority;
    int cnum;
    int volume;
    sfxinfo_t *sfx;
  
    sfx = &S_sfx[sfx_id];           // Sfx id to play
    cnum = S_getChannel(NULL, sfx); // Try to find a channel (always NULL origin)
    volume = snd_SfxVolume;         // Use maximum sound volume
    sep = NORM_SEP;                 // Don't use stereo separation (128)
    pitch = NORM_PITCH;             // Don't use pitch (127)
    priority = NORM_PRIORITY;       // Use standard priority (64)

    // Put non-breakable sound into last channel. Duh.
    cnum = 8;

    // Sfx to play
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

    // cache data if necessary
    if (!sfx->data)
    {
        sfx->data = (void *) W_CacheLumpNum(sfx->lumpnum, PU_MUSIC);
        _dpmi_lockregion(sfx->data, lumpinfo[sfx->lumpnum].size);
    }
  
    // Assigns the handle to one of the channels in the
    //  mix/output buffer.
    channels[cnum].handle = I_StartSound(sfx_id,
                                         sfx->data,
                                         volume,
                                         sep,
                                         pitch,
                                         priority);
}


//
// Updates music & sounds
//
void S_UpdateSounds(void* listener_p)
{
    int         audible;
    int         cnum;
    int         volume;
    int         sep;
    int         pitch;
    int         i;
    mobj_t     *listener = (mobj_t*)listener_p;
    sfxinfo_t  *sfx;
    channel_t  *c;

    // Clean up unused data.
    if (gametic > nextcleanup)
    {
        for (i = 1 ; i < NUMSFX ; i++)
        {
            if (S_sfx[i].usefulness < 1 && S_sfx[i].usefulness > -1)
            {
                if (--S_sfx[i].usefulness == -1)
                {
                    Z_ChangeTag(S_sfx[i].data, PU_CACHE);
                    _dpmi_unlockregion(S_sfx[i].data, lumpinfo[S_sfx[i].lumpnum].size);
                    S_sfx[i].data = 0;
                }
            }
        }

        nextcleanup = gametic + 15;
    }

    for (cnum = 0 ; cnum < numChannels ; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = snd_SfxVolume;
                pitch = NORM_PITCH;
                sep = NORM_SEP;

                if (sfx->link)
                {
                    pitch = sfx->pitch;
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
                if (c->origin && listener_p != c->origin)
                {
                    audible = S_AdjustSoundParams(listener,
                                                  c->origin,
                                                  &volume,
                                                  &sep,
                                                  &pitch);

                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
                    {
                        I_UpdateSoundParams(c->handle, volume, sep, pitch);
                    }
                }
            }
            else
            {
                // if channel is allocated but sound has stopped, free it
                S_StopChannel(cnum);
            }
        }
    }
}


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init (int sfxVolume, int musicVolume)
{
    int i;

    // Whatever these did with DMX, these are rather dummies now.
    I_SetChannels(numChannels);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    channels = (channel_t *) Z_Malloc(numChannels*sizeof(channel_t), PU_STATIC, 0);
  
    // Free all channels for use
    for (i = 0 ; i < numChannels ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    // Note that sounds have not been cached (yet).
    for (i = 1 ; i < NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }
}


// -----------------------------------------------------------------------------
// S_ChannelsRealloc
// [JN] Reallocates sound channels, needed for hot-swapping.
// -----------------------------------------------------------------------------

void S_ChannelsRealloc (void)
{
    int i;

    // Safeguard conditions:
    if (numChannels < 4)
        numChannels = 4;
    if (numChannels > 64)
        numChannels = 64;

    channels = (channel_t *) Z_Malloc(numChannels*sizeof(channel_t), PU_STATIC, 0);
    for (i=0 ; i<numChannels ; i++)
    {
        channels[i].sfxinfo = 0;
    }
}


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start (void)
{
    int cnum;
    int mnum;

    // kill all playing sounds at start of level (trust me - a good idea)
    for (cnum = 0 ; cnum < numChannels ; cnum++)
        if (channels[cnum].sfxinfo)
            S_StopChannel(cnum);

    // start new music for the level
    if (musicVolume) // [crispy] do not reset pause state at zero music volume
    {
        mus_paused = 0;
    }

    if (commercial)
    {
        mnum = mus_runnin + gamemap - 1;
    }
    else
    {
        int spmus[]=
        {
            // Song - Who? - Where?
            mus_e3m4,   // American    e4m1
            mus_e3m2,   // Romero      e4m2
            mus_e3m3,   // Shawn       e4m3
            mus_e1m5,   // American    e4m4
            mus_e2m7,   // Tim         e4m5
            mus_e2m4,   // Romero      e4m6
            mus_e2m6,   // J.Anderson  e4m7 CHIRON.WAD
            mus_e2m5,   // Shawn       e4m8
            mus_e1m9    // Tim         e4m9
        };

        if (gameepisode < 4 || gameepisode == 5) // [crispy] Sigil
        {
            mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
        }
        else
        {
            mnum = spmus[gamemap-1];
        }
    }

    S_ChangeMusic(mnum, true);

    nextcleanup = 15;
}

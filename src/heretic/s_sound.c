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


#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_sound.h"
#include "r_local.h"
#include "p_local.h"

#include "sounds.h"

#include "w_wad.h"
#include "z_zone.h"

#include "jn.h"

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

void S_ShutDown(void);
boolean S_StopSoundID(int sound_id, int priority);

static channel_t channel[MAX_CHANNELS];

static void *rs;          // Handle for the registered song
int mus_song = -1;
int mus_lumpnum;
void *mus_sndptr;
byte *soundCurve;
int idmusnum;  // [JN] jff 3/17/98 to keep track of last IDMUS specified music num
int snd_MaxVolume_tmp;  // [JN] Temp volume variable used for hot-muting.



int AmbChan;

void S_Start(void)
{
    int i;

    // [JN] If music was choosen by cheat code, play it.
    if (idmusnum != -1)
    {
        S_StartSong(idmusnum, true, false);
    }
    else
    {
        S_StartSong((gameepisode - 1) * 9 + gamemap - 1, true, false);
    }

    //stop all sounds
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].handle)
        {
            S_StopSound(channel[i].mo);
        }
    }
    memset(channel, 0, 8 * sizeof(channel_t));
}

void S_StartSong(int song, boolean loop, boolean replay)
{
    int mus_len;

    // [JN] Don't replay an old song, but
    // allow replay if music system is changed.
    //
    // Commented out because of music device hot-swapping disabled.
    // if (song == mus_song && !replay)
    // {
    //     return;
    // }

    if (rs != NULL)
    {
        I_StopSong();
        // [JN] No-op. Do not unregister song now, otherwise 
        // changed music system will be able to handle it.
        //
        // Uncommented out because of music device hot-swapping disabled.
        I_UnRegisterSong(rs);
    }

    if (song < mus_e1m1 || song > NUMMUSIC)
    {
        return;
    }
    // [crispy] support dedicated music tracks for each map
    if (S_music[song][1].name && W_CheckNumForName(S_music[song][1].name) > 0)
    {
        mus_lumpnum = (W_GetNumForName(S_music[song][1].name));
    }
    else
    {
        mus_lumpnum = (W_GetNumForName(S_music[song][0].name));
    }
    mus_sndptr = W_CacheLumpNum(mus_lumpnum, PU_MUSIC);
    mus_len = W_LumpLength(mus_lumpnum);
    rs = I_RegisterSong(mus_sndptr, mus_len);
    I_PlaySong(rs, loop);       //'true' denotes endless looping.
    mus_song = song;
}

void S_StopSong(void)
{
    I_StopSong();
    I_UnRegisterSong(rs);
    W_ReleaseLumpNum(mus_lumpnum);
}

static mobj_t *GetSoundListener(void)
{
    static degenmobj_t dummy_listener;

    // If we are at the title screen, the console player doesn't have an
    // object yet, so return a pointer to a static dummy listener instead.

    if (players[consoleplayer].mo != NULL)
    {
        return players[consoleplayer].mo;
    }
    else
    {
        dummy_listener.x = 0;
        dummy_listener.y = 0;
        dummy_listener.z = 0;

        return (mobj_t *) &dummy_listener;
    }
}

void S_StartSound(void *_origin, int sound_id)
{
    mobj_t *origin = _origin;
    mobj_t *listener;
    int dist, vol;
    int i;
    int priority;
    int sep;
    int angle;
    int absx;
    int absy;
    int absz;

    static int sndcount = 0;
    int chan;

    listener = GetSoundListener();

    if (sound_id == 0 || snd_MaxVolume == 0)
        return;
    if (origin == NULL)
    {
        origin = listener;
    }

// calculate the distance before other stuff so that we can throw out
// sounds that are beyond the hearing range.
    absx = abs(origin->x - listener->x);
    absy = abs(origin->y - listener->y);
    // [JN] Z-axis sfx distance
    absz = !z_axis_sfx || vanillaparm ? 0 : abs(origin->z - listener->z);
    dist = P_ApproxDistanceZ(absx, absy, absz);
    dist >>= FRACBITS;

    if (dist >= MAX_SND_DIST)
    {
        return;                 //sound is beyond the hearing range...
    }
    if (dist < 0)
    {
        dist = 0;
    }
    priority = S_sfx[sound_id].priority;
    priority *= (10 - (dist / 160));
    if (!S_StopSoundID(sound_id, priority))
    {
        return;                 // other sounds have greater priority
    }
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (origin->player)
        {
            i = snd_Channels_RD;
            break;              // let the player have more than one sound.
        }
        if (origin == channel[i].mo)
        {                       // only allow other mobjs one sound
            S_StopSound(channel[i].mo);
            break;
        }
    }
    if (i >= snd_Channels_RD)
    {
        if (sound_id >= sfx_wind)
        {
            if (AmbChan != -1 && S_sfx[sound_id].priority <=
                S_sfx[channel[AmbChan].sound_id].priority)
            {
                return;         //ambient channel already in use
            }
            else
            {
                AmbChan = -1;
            }
        }
        for (i = 0; i < snd_Channels_RD; i++)
        {
            if (channel[i].mo == NULL)
            {
                break;
            }
        }
        if (i >= snd_Channels_RD)
        {
            //look for a lower priority sound to replace.
            sndcount++;
            if (sndcount >= snd_Channels_RD)
            {
                sndcount = 0;
            }
            for (chan = 0; chan < snd_Channels_RD; chan++)
            {
                i = (sndcount + chan) % snd_Channels_RD;
                if (priority >= channel[i].priority)
                {
                    chan = -1;  //denote that sound should be replaced.
                    break;
                }
            }
            if (chan != -1)
            {
                return;         //no free channels.
            }
            else                //replace the lower priority sound.
            {
                if (channel[i].handle)
                {
                    if (I_SoundIsPlaying(channel[i].handle))
                    {
                        I_StopSound(channel[i].handle);
                    }
                    if (S_sfx[channel[i].sound_id].usefulness > 0)
                    {
                        S_sfx[channel[i].sound_id].usefulness--;
                    }

                    if (AmbChan == i)
                    {
                        AmbChan = -1;
                    }
                }
            }
        }
    }
    if (S_sfx[sound_id].lumpnum == 0)
    {
        S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
    }

    // calculate the volume based upon the distance from the sound origin.
    vol = soundCurve[dist];

    if (origin == listener)
    {
        sep = 128;
    }
    else
    {
        angle = R_PointToAngle2(listener->x, listener->y,
                                origin->x, origin->y);
        angle = (angle - viewangle) >> 24;
        sep = angle * 2 - 128;
        if (sep < 64)
            sep = -sep;
        if (sep > 192)
            sep = 512 - sep;
    }

    channel[i].pitch = (byte) (NORM_PITCH + (M_Random() & 7) - (M_Random() & 7));
    channel[i].handle = I_StartSound(&S_sfx[sound_id], i, vol, sep, channel[i].pitch);
    channel[i].mo = origin;
    channel[i].sound_id = sound_id;
    channel[i].priority = priority;
    if (sound_id >= sfx_wind)
    {
        AmbChan = i;
    }
    if (S_sfx[sound_id].usefulness == -1)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

void S_StartSoundAtVolume(void *_origin, int sound_id, int volume)
{
    mobj_t *origin = _origin;
    mobj_t *listener;
    int i;

    listener = GetSoundListener();

    if (sound_id == 0 || snd_MaxVolume == 0)
        return;
    if (origin == NULL)
    {
        origin = listener;
    }

    if (volume == 0)
    {
        return;
    }
    volume = (volume * (snd_MaxVolume + 1) * 8) >> 7;

// no priority checking, as ambient sounds would be the LOWEST.
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].mo == NULL)
        {
            break;
        }
    }
    if (i >= snd_Channels_RD)
    {
        return;
    }
    if (S_sfx[sound_id].lumpnum == 0)
    {
        S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
    }

    channel[i].pitch = (byte) (NORM_PITCH - (M_Random() & 3) + (M_Random() & 3));
    channel[i].handle = I_StartSound(&S_sfx[sound_id], i, volume, 128, channel[i].pitch);
    channel[i].mo = origin;
    channel[i].sound_id = sound_id;
    channel[i].priority = 1;    //super low priority.
    if (S_sfx[sound_id].usefulness == -1)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

// -----------------------------------------------------------------------------
// S_StartSoundAmbient
// [JN] Used for waterfall and wind sounds. Sets proper stereo separation and
// making sounds always playable (altrough not hearable if sound is too far).
// -----------------------------------------------------------------------------

void S_StartSoundAmbient(void *_origin, int sound_id)
{
    mobj_t *origin = _origin;
    mobj_t *listener;
    int dist, vol;
    int sep;
    int angle;
    int i;
    int priority;
    int absx, absy;

    // [JN] Player is always listener.
    listener = players[consoleplayer].mo;

    if (origin == NULL)
    {
        origin = listener;
    }

    if (origin == listener)
    {
        sep = 128;
    }
    else
    {
        angle = R_PointToAngle2(listener->x, listener->y,
                                origin->x, origin->y);
        angle = (angle - viewangle) >> 24;
        sep = angle * 2 - 128;
        if (sep < 64)
            sep = -sep;
        if (sep > 192)
            sep = 512 - sep;
    }

    // [JN] Calculate the distance.
    absx = abs(origin->x - listener->x);
    absy = abs(origin->y - listener->y);
    dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
    dist >>= FRACBITS;

    if (dist >= MAX_SND_DIST)
    {
        dist = MAX_SND_DIST - 1;
    }
    if (dist < 0)
    {
        dist = 0;
    }

    priority = S_sfx[sound_id].priority;
    priority *= (10 - (dist / 160));

    // [JN] Calculate the volume based upon the distance from the sound origin.
    vol = soundCurve[dist];

    // [JN] No priority checking.
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].mo == NULL)
        {
            break;
        }
    }

    if (i >= snd_Channels_RD)
    {
        return;
    }

    channel[i].pitch = (byte) (NORM_PITCH - (M_Random() & 7) + (M_Random() & 7));
    channel[i].handle = I_StartSound(&S_sfx[sound_id], i, vol, sep, channel[i].pitch);
    channel[i].mo = origin;
    channel[i].sound_id = sound_id;
    channel[i].priority = priority;
    if (S_sfx[sound_id].usefulness == -1)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

boolean S_StopSoundID(int sound_id, int priority)
{
    int i;
    int lp;                     //least priority
    int found;

    if (S_sfx[sound_id].numchannels == -1)
    {
        return (true);
    }
    lp = -1;                    //denote the argument sound_id
    found = 0;
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].sound_id == sound_id && channel[i].mo)
        {
            found++;            //found one.  Now, should we replace it??
            if (priority >= channel[i].priority)
            {                   // if we're gonna kill one, then this'll be it
                lp = i;
                priority = channel[i].priority;
            }
        }
    }
    if (found < S_sfx[sound_id].numchannels)
    {
        return (true);
    }
    else if (lp == -1)
    {
        return (false);         // don't replace any sounds
    }
    if (channel[lp].handle)
    {
        if (I_SoundIsPlaying(channel[lp].handle))
        {
            I_StopSound(channel[lp].handle);
        }
        if (S_sfx[channel[i].sound_id].usefulness > 0)
        {
            S_sfx[channel[i].sound_id].usefulness--;
        }
        channel[lp].mo = NULL;
    }
    return (true);
}

void S_StopSound(void *_origin)
{
    mobj_t *origin = _origin;
    int i;

    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].mo == origin)
        {
            I_StopSound(channel[i].handle);
            if (S_sfx[channel[i].sound_id].usefulness > 0)
            {
                S_sfx[channel[i].sound_id].usefulness--;
            }
            channel[i].handle = 0;
            channel[i].mo = NULL;
            if (AmbChan == i)
            {
                AmbChan = -1;
            }
        }
    }
}

void S_SoundLink(mobj_t * oldactor, mobj_t * newactor)
{
    int i;

    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].mo == oldactor)
            channel[i].mo = newactor;
    }
}

void S_PauseSound(void)
{
    I_PauseSong();
}

void S_ResumeSound(void)
{
    I_ResumeSong();
}

void S_UpdateSounds(mobj_t * listener)
{
    int i, dist, vol;
    int angle;
    int sep;
    int priority;
    int absx;
    int absy;
    int absz;

    I_UpdateSound();

    listener = GetSoundListener();
    if (snd_MaxVolume == 0)
    {
        return;
    }

    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (!channel[i].handle || S_sfx[channel[i].sound_id].usefulness == -1)
        {
            continue;
        }
        if (!I_SoundIsPlaying(channel[i].handle))
        {
            if (S_sfx[channel[i].sound_id].usefulness > 0)
            {
                S_sfx[channel[i].sound_id].usefulness--;
            }
            channel[i].handle = 0;
            channel[i].mo = NULL;
            channel[i].sound_id = 0;
            if (AmbChan == i)
            {
                AmbChan = -1;
            }
        }
        if (channel[i].mo == NULL || channel[i].sound_id == 0
         || channel[i].mo == listener || listener == NULL)
        {
            continue;
        }
        else
        {
            absx = abs(channel[i].mo->x - listener->x);
            absy = abs(channel[i].mo->y - listener->y);
            // [JN] Z-axis sfx distance.
            absz = !z_axis_sfx || vanillaparm ? 0 : abs(channel[i].mo->z - listener->z);
            dist = absx + absy + absz - (absx > absy ? absy >> 1 : absx >> 1);
            dist >>= FRACBITS;

            if (dist >= MAX_SND_DIST)
            {
                // [JN] Do not stop/break waterfall and wind sounds,
                // consider them playing at maximum distance.
                if (channel[i].sound_id == sfx_waterfl
                ||  channel[i].sound_id == sfx_wind)
                {
                    dist = MAX_SND_DIST - 1;
                }
                else
                {
                    S_StopSound(channel[i].mo);
                    continue;
                }
            }
            if (dist < 0)
                dist = 0;

// calculate the volume based upon the distance from the sound origin.
            vol = soundCurve[dist];

            angle = R_PointToAngle2(listener->x, listener->y,
                                    channel[i].mo->x, channel[i].mo->y);
            angle = ((flip_levels ? -angle : angle) - viewangle) >> 24;
            sep = angle * 2 - 128;

            // [JN] Support for mono sfx mode
            if (snd_monomode)
            {
                sep = 128;            
            }
            else
            {
            if (sep < 64)
                sep = -sep;
            if (sep > 192)
                sep = 512 - sep;
            }

            // TODO: Pitch shifting.
            I_UpdateSoundParams(channel[i].handle, vol, sep);
            priority = S_sfx[channel[i].sound_id].priority;
            priority *= (10 - (dist >> 8));
            channel[i].priority = priority;
        }
    }
}

void S_Init(void)
{
    idmusnum = -1; // [JN] jff 3/17/98 insure idmus number is blank

    I_SetOPLDriverVer(opl_doom2_1_666);
    soundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);

    // [JN] Make sound channels multiple by four:
    snd_Channels -= snd_Channels % 4;

    // [JN] Correct unacceptable values:
    if (snd_Channels <= 4)  
        snd_Channels = 4;
    else
    if (snd_Channels >= 64)
        snd_Channels = 64;

    // [JN] Cap sound channels to 8 in -vanilla game mode.
    if (vanillaparm)
    {
        snd_Channels_RD = 8;
    }
    else
    {
        snd_Channels_RD = snd_Channels;
    }

    I_SetMusicVolume(snd_MusicVolume);
    S_SetMaxVolume(true);

    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable.

    I_AtExit(S_ShutDown, true);

    // Heretic defaults to pitch-shifting on
    if (snd_pitchshift == -1)
    {
        snd_pitchshift = 1;
    }

    I_PrecacheSounds(S_sfx, NUMSFX);
}

/*
================================================================================
=
= S_RD_Change_SoundDevice
=
= [JN] Routine for sfx device hot-swapping.
=
================================================================================
*/

void S_RD_Change_SoundDevice (void)
{
    int i;

    // Regenerate cache of digital sounds
    I_PrecacheSounds(S_sfx, NUMSFX);

    // Free all channels for use
    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].handle)
        {
            S_StopSound(channel[i].mo);
        }
    }
    memset(channel, 0, 8 * sizeof(channel_t));

    // Reinitialize sfx usefulness
    for (i = 0 ; i < NUMSFX ; i++)
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
    if (snd_Channels < 4)
        snd_Channels = 4;
    if (snd_Channels > 64)
        snd_Channels = 64;

    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].handle)
        {
            S_StopSound(channel[i].mo);
        }
    }
    memset(channel, 0, 8 * sizeof(channel_t));
}

void S_GetChannelInfo(SoundInfo_t * s)
{
    int i;
    ChanInfo_t *c;

    s->channelCount = snd_Channels_RD;
    s->musicVolume = snd_MusicVolume;
    s->soundVolume = snd_MaxVolume;
    for (i = 0; i < snd_Channels_RD; i++)
    {
        c = &s->chan[i];
        c->id = channel[i].sound_id;
        c->priority = channel[i].priority;
        c->name = S_sfx[c->id].name;
        c->mo = channel[i].mo;

        if (c->mo != NULL)
        {
            c->distance = P_AproxDistance(c->mo->x - viewx, c->mo->y - viewy)
                >> FRACBITS;
        }
        else
        {
            c->distance = 0;
        }
    }
}

void S_SetMaxVolume(boolean fullprocess)
{
    int i;

    if (!fullprocess)
    {
        soundCurve[0] =
            (*((byte *) W_CacheLumpName("SNDCURVE", PU_CACHE)) *
             (snd_MaxVolume * 8)) >> 7;
    }
    else
    {
        for (i = 0; i < MAX_SND_DIST; i++)
        {
            soundCurve[i] =
                (*((byte *) W_CacheLumpName("SNDCURVE", PU_CACHE) + i) *
                 (snd_MaxVolume * 8)) >> 7;
        }
    }
}

static boolean musicPaused;
void S_SetMusicVolume(void)
{
    I_SetMusicVolume(snd_MusicVolume);
    if (snd_MusicVolume == 0)
    {
        I_PauseSong();
        musicPaused = true;
    }
    else if (musicPaused)
    {
        musicPaused = false;
        I_ResumeSong();
    }
}

void S_ShutDown(void)
{
    I_StopSong();
    I_UnRegisterSong(rs);
    I_ShutdownSound();
}

// -----------------------------------------------------------------------------
// S_MuteSound
// [JN] Sets sound and music volume to 0, stops all sounds in all channels.
// -----------------------------------------------------------------------------

void S_MuteSound(void)
{
    int i;

    I_SetMusicVolume(0);

    snd_MaxVolume = 0;
    S_SetMaxVolume(true);
    for (i = 0; i < snd_Channels_RD; i++)
    {
        if (channel[i].handle)
        {
            S_StopSound(channel[i].mo);
        }
    }

    volume_needs_update = false;
}

// -----------------------------------------------------------------------------
// S_UnMuteSound
// [JN] Restores sound and music volume.
// -----------------------------------------------------------------------------

void S_UnMuteSound(void)
{
    I_SetMusicVolume(snd_MusicVolume);

    snd_MaxVolume = snd_MaxVolume_tmp;
    S_SetMaxVolume(true);

    volume_needs_update = false;
}

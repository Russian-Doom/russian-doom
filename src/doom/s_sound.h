//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
//	The not so system specific sound interface.
//


#pragma once

#include "r_local.h"
#include "sounds.h"


// Initializes sound stuff, including volume
// Sets channels, SFX and music volume, 
//  allocates channel buffer, sets S_sfx lookup.
void S_Init (int sfxVolume, int musicVolume);


// Shut down sound 
void S_Shutdown(void);

// [JN] SFX device changing routine
void S_RD_Change_SoundDevice(void);

// [JN] Channels reallocation
void S_ChannelsRealloc(void);

// [JN] Stereo separation defining
void S_UpdateStereoSeparation(void);

// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
void S_Start(void);

// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
void S_StartSound(void *origin_p, int sfx_id);
void S_StartSoundOnce(void *origin, int sound_id);
void S_StartSoundNoBreak(int sfx_id);

// Stop sound for thing at <origin>
void S_StopSound(const mobj_t *origin);

// [Dasperal] & [JN] Prevents opening sound for door 
// which is already opened.
void S_StopDoorSound (degenmobj_t *origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int m_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int musicnum, int looping);

// query if music is playing
const boolean S_MusicPlaying(void);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);

// Updates music & sounds
void S_UpdateSounds(mobj_t *listener);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

extern int music_num_rd;
// [JN] jff 3/17/98 holds last IDMUS number, or -1
extern int idmusnum;

void S_MuteSound(void);
void S_UnMuteSound(void);

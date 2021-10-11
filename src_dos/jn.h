//
// Copyright(C) 2018-2019 Julian Nechaevsky
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
//	Russian Doom specific variables.
//


#ifndef __JN_H__
#define __JN_H__


// -----------------------------------------------------------------------------
// Language (0 = Russian, 1 = English)
// -----------------------------------------------------------------------------

extern int english_language;

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------

extern boolean vanilla;


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

extern int noflats;
extern int show_fps, fps;
extern int show_diskicon;
extern int screen_wiping;
extern int screenshot_pcx;


// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

extern int level_brightness;
extern int local_time;


// -----------------------------------------------------------------------------
// Messages
// -----------------------------------------------------------------------------

extern int messages_pickup_color;
extern int messages_secret_color;
extern int messages_system_color;
extern int messages_chat_color;


// -----------------------------------------------------------------------------
// Automap
// -----------------------------------------------------------------------------

extern int automap_color;
extern int automap_antialias;
extern int automap_stats;
extern int automap_overlay;
extern int automap_rotate;
extern int automap_follow;
extern int automap_grid;


// -----------------------------------------------------------------------------
// Controls
// -----------------------------------------------------------------------------

extern int joybspeed;
extern int mlook;
extern int mouse_y_invert;
extern int novert;


// -----------------------------------------------------------------------------
// Audio
// -----------------------------------------------------------------------------

extern int sfxVolume;
extern int musicVolume;
extern int numChannels;
extern int snd_SBport, snd_SBirq, snd_SBdma, snd_Mport;
extern int snd_samplerate;
extern int snd_monomode;
extern int snd_pitchshift;


// -----------------------------------------------------------------------------
// Optional gameplay features
// -----------------------------------------------------------------------------

// Gameplay: Graphical
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int improved_fuzz;
extern int colored_hud;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int linear_sky;
extern int flip_weapons;
extern int draw_shadowed_text;

// Gameplay: Audible
extern int play_exit_sfx;
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int noise_alert_sfx;

// Gameplay: Tactical
extern int automap_stats;
extern int secret_notification;
extern int negative_health;

// Gameplay: Physical
extern int over_under;
extern int torque;
extern int weapon_bobbing;
extern int ssg_blast_enemies;
extern int randomly_flipcorpses;
extern int floating_powerups;

// Gameplay: Crosshair
extern int crosshair_draw;
extern int crosshair_health;

// Gameplay: Gameplay
extern int extra_player_faces;
extern int unlimited_lost_souls;
extern int fast_quickload;
extern int no_internal_demos;


#endif

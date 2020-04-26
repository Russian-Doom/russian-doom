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
// Command line parameters
// -----------------------------------------------------------------------------

extern boolean vanillaparm;


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

extern int vsync;
// extern int aspect_ratio_correct;
extern int uncapped_fps;
extern int show_fps, real_fps;
// extern int smoothing;
// extern int force_software_renderer;
extern int show_diskicon;
extern int screen_wiping;
extern int png_screenshots;


// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

extern int level_brightness;
extern int local_time;
extern int draw_shadowed_text;
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
// Sound
// -----------------------------------------------------------------------------

extern int snd_monomode;
extern int mute_inactive_window;
extern boolean window_focused;
extern boolean volume_needs_update;


// -----------------------------------------------------------------------------
// Controls
// -----------------------------------------------------------------------------

extern int mlook;
extern int novert;
extern int mouse_y_invert;


// -----------------------------------------------------------------------------
// Gameplay feautures
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
extern int flip_weapons;

// Gameplay: Audible
extern int play_exit_sfx;
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int noise_alert_sfx;
extern int correct_endlevel_sfx;

// Gameplay: Tactical
extern int secret_notification;
extern int negative_health;
extern int infragreen_visor;

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
extern int crosshair_scale;

// Gameplay: Gameplay
extern int fix_map_errors;
extern int flip_levels;
extern int extra_player_faces;
extern int unlimited_lost_souls;
extern int agressive_lost_souls;
extern int fast_quickload;
extern int no_internal_demos;


// -----------------------------------------------------------------------------
// Doom Press Release Beta
// -----------------------------------------------------------------------------

extern int artifactcount;   // Amount of artifacts
extern int lifecount;       // Amount of lifes


#endif

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
// Command line parameters
// -----------------------------------------------------------------------------

extern boolean vanillaparm;
extern boolean flip_levels_cmdline;


// -----------------------------------------------------------------------------
// Controls
// -----------------------------------------------------------------------------

extern int mlook;
extern int local_time;


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

extern int uncapped_fps;


// -----------------------------------------------------------------------------
// Language (0 = Russian, 1 = English)
// -----------------------------------------------------------------------------

extern int english_language;


// -----------------------------------------------------------------------------
// Automap specific variables
// -----------------------------------------------------------------------------

extern int automap_follow;
extern int automap_overlay;
extern int automap_rotate;
extern int automap_grid;

// -----------------------------------------------------------------------------
// Optional gameplay features
// -----------------------------------------------------------------------------

// Rendering
extern int show_diskicon;
extern int screen_wiping;

// Sound
extern int snd_monomode;

// Gameplay: Graphical
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int colored_hud;
extern int colored_messages;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int red_resurrection_flash;
extern int draw_shadowed_text;

// Gameplay: Audible
extern int play_exit_sfx;
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int noise_alert_sfx;
extern int correct_endlevel_sfx;

// Gameplay: Tactical
extern int automap_stats;
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
extern int extra_player_faces;
extern int unlimited_lost_souls;
extern int agressive_lost_souls;
extern int fast_quickload;
extern int no_internal_demos;
extern int flip_levels;


// -----------------------------------------------------------------------------
// Doom Press Release Beta
// -----------------------------------------------------------------------------

extern int artifactcount;   // Amount of artifacts
extern int lifecount;       // Amount of lifes


#endif

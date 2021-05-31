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
extern int show_endoom;
extern int flashing_hom;


// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

extern int screenblocks, screenSize;
extern int level_brightness;
extern int menu_shading;
extern int local_time;
extern int detailLevel;
extern int hud_detaillevel;
extern int showMessages;
extern int draw_shadowed_text;
extern int messages_timeout;
extern int message_pickup_color;
extern int message_secret_color;
extern int message_system_color;
extern int message_chat_color;


// -----------------------------------------------------------------------------
// Automap
// -----------------------------------------------------------------------------

extern int automap_color;
extern int automap_antialias;
extern int automap_stats;
extern int automap_level_time;
extern int automap_total_time;
extern int automap_coords;
extern int automap_overlay;
extern int automap_rotate;
extern int automap_follow;
extern int automap_grid;
extern int automap_grid_size;

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------

extern int snd_channels_rd, snd_channels, snd_channels_vanilla;
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
// Selective game
// -----------------------------------------------------------------------------

extern int selective_skill;
extern int selective_episode;
extern int selective_map;

extern int selective_health;
extern int selective_armor;
extern int selective_armortype;

// Doom weapons
extern int selective_wp_chainsaw;
extern int selective_wp_shotgun;
extern int selective_wp_supershotgun;
extern int selective_wp_chaingun;
extern int selective_wp_missile;
extern int selective_wp_plasma;
extern int selective_wp_bfg;

// Heretic weapons
extern int selective_wp_gauntlets;
extern int selective_wp_crossbow;
extern int selective_wp_dragonclaw;
extern int selective_wp_hellstaff;
extern int selective_wp_phoenixrod;
extern int selective_wp_firemace;

extern int selective_backpack;

extern int selective_ammo_0;   // bullets | wand crystals
extern int selective_ammo_1;   // shells  | ethereal arrows
extern int selective_ammo_2;   // cells   | claw orbs
extern int selective_ammo_3;   // rockets | hellstaff runes
extern int selective_ammo_4;   //         | flame orbs
extern int selective_ammo_5;   //         | mace spheres

extern int selective_key_0;
extern int selective_key_1;
extern int selective_key_2;
extern int selective_key_3;
extern int selective_key_4;
extern int selective_key_5;

extern int selective_fast;
extern int selective_respawn;

// Heretic artifacts
extern int selective_arti_0;   // Quartz Flask
extern int selective_arti_1;   // Mystic Urn
extern int selective_arti_2;   // Timebomb
extern int selective_arti_3;   // Tome of Power
extern int selective_arti_4;   // Ring of Invincibility
extern int selective_arti_5;   // Morph Ovum
extern int selective_arti_6;   // Chaos Device
extern int selective_arti_7;   // Shadowsphere
extern int selective_arti_8;   // Wings of Wrath
extern int selective_arti_9;   // Torch

// -----------------------------------------------------------------------------
// Gameplay feautures
// -----------------------------------------------------------------------------

// Gameplay: Graphical
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int improved_fuzz;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int linear_sky;
extern int flip_weapons;

// Gameplay: Status Bar
extern int extra_player_faces;
extern int negative_health;
extern int sbar_colored;
extern int sbar_colored_gem;
extern int sbar_color_high;
extern int sbar_color_normal;
extern int sbar_color_low;
extern int sbar_color_critical;
extern int sbar_color_armor_1;
extern int sbar_color_armor_2;
extern int sbar_color_armor_0;

// Gameplay: Audible
extern int z_axis_sfx;
extern int play_exit_sfx;
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int noise_alert_sfx;
extern int correct_endlevel_sfx;

// Gameplay: Tactical
extern int secret_notification;
extern int infragreen_visor;

// Gameplay: Physical
extern int over_under;
extern int torque;
extern int weapon_bobbing;
extern int ssg_blast_enemies;
extern int randomly_flipcorpses;
extern int floating_powerups;
extern int toss_drop;

// Gameplay: Crosshair
extern int crosshair_draw;
extern int crosshair_type;
extern int crosshair_scale;

// Gameplay: Gameplay
extern int fix_map_errors;
extern int flip_levels;
extern int unlimited_lost_souls;
extern int agressive_lost_souls;
extern int pistol_start;
extern int fast_quickload;

// Gameplay: Demos
extern int demotimer;
extern int demotimerdir;
extern int demobar;
extern int no_internal_demos;


// -----------------------------------------------------------------------------
// Doom Press Release Beta
// -----------------------------------------------------------------------------

extern int artifactcount;   // Amount of artifacts
extern int lifecount;       // Amount of lifes


#endif

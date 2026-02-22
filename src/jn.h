//
// Copyright(C) 2014 Fabian Greffrath
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
//	Russian Doom specific variables.
//


#pragma once

#include "doomtype.h"


#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef BETWEEN
#define BETWEEN(l,u,x) (((l)>(x))?(l):((x)>(u))?(u):(x))
#endif

#define singleplayer (!demorecording && !demoplayback && !netgame)

/**
 * Macro for vanilla visual bug fix condition.
 * It accounts for '-vanilla' cli param.
 *
 * Vanilla visual bug fixes should be wrapped in if statement with this macro as condition.
 *
 * @param option - Name of the config option.
 *		  It should be true if the fix is enabled.
 *		  And it should be false if the fix is disabled, i.e., vanilla behavior.
 * @returns True - when fix should be enabled.
 *			False - when fix should be disabled, i.e., vanilla behavior.
 */
#define vanilla_visual_bug_fix(option) (!vanillaparm && !strict_mode && singleplayer && (option))

/**
 * Macro for vanilla gameplay bug fix condition.
 * It accounts for '-vanilla' cli param, the 'strict gameplay mode' option,
 * demo playback/record and netgame
 *
 * Vanilla gameplay bug fixes should be wrapped in if statement with this macro as condition.
 *
 * @param option - Name of the config option.
 *		  It should be true if the fix is enabled.
 *		  And it should be false if the fix is disabled, i.e., vanilla behavior.
 * @returns True - when fix should be enabled.
 *			False - when fix should be disabled, i.e., vanilla behavior.
 */
#define vanilla_gameplay_bug_fix(option) (!vanillaparm && !strict_mode && singleplayer && (option))

/**
 * Macro for vanilla gameplay feature condition.
 * It accounts for '-vanilla' cli param, the 'strict gameplay mode' option,
 * demo playback/record and netgame
 *
 * Vanilla gameplay features that can be disabled should be wrapped in if statement with this macro as condition.
 *
 * @param option - Name of the config option.
 *		  It should be true if the feature is enabled, i.e., vanilla behavior.
 *		  And it should be false if the feature is disabled.
 * @returns True - when feature should be enabled, i.e., vanilla behavior.
 *			False - when feature should be disabled.
 */
#define vanilla_gameplay_feature(option) (vanillaparm || strict_mode || !singleplayer || (option))

/**
 * Macro for additional visual feature condition.
 * It accounts for '-vanilla' cli param.
 *
 * Additional visual features should be wrapped in if statement with this macro as condition.
 *
 * @param option - Name of the config option.
 *		  It should be true if the feature is enabled.
 *		  And it should be false if the feature is disabled, i.e., vanilla behavior.
 * @returns True - when feature should be enabled.
 *			False - when feature should be disabled, i.e., vanilla behavior.
 */
#define visual_feature(option) (!vanillaparm && (option))

/**
 * Macro for additional gameplay feature condition.
 * It accounts for '-vanilla' cli param, the 'strict gameplay mode' option,
 * demo playback/record and netgame
 *
 * Additional gameplay features should be wrapped in if statement with this macro as condition.
 *
 * @param option - Name of the config option.
 *		  It should be true if the feature is enabled.
 *		  And it should be false if the feature is disabled, i.e., vanilla behavior.
 * @returns True - when feature should be enabled.
 *			False - when feature should be disabled, i.e., vanilla behavior.
 */
#define gameplay_feature(option) (!vanillaparm && !strict_mode && singleplayer && (option))

void *crispy_realloc (void *ptr, size_t size);

enum
{
	REINIT_FRAMEBUFFERS = 1,
	REINIT_RENDERER = 2,
	REINIT_TEXTURES = 4,
	REINIT_ASPECTRATIO = 8,
};

extern int lang_param;

// -----------------------------------------------------------------------------
// Language (0 = Russian, 1 = English)
// -----------------------------------------------------------------------------

extern int english_language;


// -----------------------------------------------------------------------------
// Command line parameters
// -----------------------------------------------------------------------------

extern boolean vanillaparm;
extern int demowarp; // [JN] Demo warp from Crispy Doom.


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

extern int vsync;
// extern int preserve_window_aspect_ratio;
extern int uncapped_fps;
extern int show_fps, real_fps;
extern int max_fps;
extern int smoothlight;
extern int show_diskicon;
extern int screen_wiping;
extern int png_screenshots;
extern int show_endoom;
extern int flashing_hom;


// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

extern int screenblocks, screenSize;
extern int extra_level_brightness;
extern int menu_shading;

extern float brightness;
extern float color_saturation;
extern int show_palette;
extern float r_color_factor;
extern float g_color_factor;
extern float b_color_factor;

extern int local_time;
extern int hud_detaillevel;
extern int showMessages;
extern int message_fade;
extern int draw_shadowed_text;
extern int messages_alignment;
extern int messages_timeout;
extern int message_color_pickup;
extern int message_color_secret;
extern int message_color_system;
extern int message_color_chat;
extern int message_color_quest;

// -----------------------------------------------------------------------------
// Automap
// -----------------------------------------------------------------------------

extern int automap_color;
extern int automap_antialias;
extern int automap_rotate;
extern int automap_overlay;
extern int automap_overlay_bg;
extern int automap_follow;
extern int automap_grid;
extern int automap_grid_size;
extern int automap_mark_color;
extern int automap_secrets;

// -----------------------------------------------------------------------------
// Stats
// -----------------------------------------------------------------------------

extern int stats_placement;
extern int stats_kis;
extern int stats_skill;
extern int stats_level_time;
extern int stats_total_time;
extern int stats_coords;
extern int stats_level_name;
extern int stats_color;

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

extern int selective_class; // Hexen player class
extern int selective_skill;
extern int selective_episode;
extern int selective_map;

extern int selective_health;
extern int selective_armor;
extern int selective_armortype;
extern int selective_armor_0; // Hexen armor: MESH ARMOR
extern int selective_armor_1; // Hexen armor: FALCON SHIELD
extern int selective_armor_2; // Hexen armor: PLATINUM HELMET
extern int selective_armor_3; // Hexen armor: AMULET OF WARDING

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

// Hexen weapons
extern int selective_wp_second;
extern int selective_wp_third;
extern int selective_wp_fourth;
extern int selective_wp_piece_0;
extern int selective_wp_piece_1;
extern int selective_wp_piece_2;

extern int selective_backpack;

extern int selective_ammo_0;   // bullets | wand crystals   | blue mana
extern int selective_ammo_1;   // shells  | ethereal arrows | green mana
extern int selective_ammo_2;   // cells   | claw orbs       |
extern int selective_ammo_3;   // rockets | hellstaff runes |
extern int selective_ammo_4;   //         | flame orbs      |
extern int selective_ammo_5;   //         | mace spheres    |

extern int selective_key_0;  // blue keycard     | yellow key | EMERALD KEY
extern int selective_key_1;  // yellow keycard   | green key  | SILVER KEY
extern int selective_key_2;  // red keycard      | blue key   | FIRE KEY
extern int selective_key_3;  // blue skull key   |            | STEEL KEY
extern int selective_key_4;  // yellow skull key |            | HORN KEY
extern int selective_key_5;  // red skull key    |            | CAVE KEY
extern int selective_key_6;  //                  |            | CASTLE KEY
extern int selective_key_7;  //                  |            | SWAMP KEY
extern int selective_key_8;  //                  |            | RUSTED KEY
extern int selective_key_9;  //                  |            | DUNGEON KEY
extern int selective_key_10; //                  |            | AXE KEY

extern int selective_fast;
extern int selective_respawn;

// Heretic artifacts
extern int selective_arti_0;   // Quartz Flask
extern int selective_arti_1;   // Mystic Urn
extern int selective_arti_2;   // Timebomb              | FLECHETTE
extern int selective_arti_3;   // Tome of Power         | DISC OF REPULSION
extern int selective_arti_4;   // Ring of Invincibility | ICON OF THE DEFENDER
extern int selective_arti_5;   // Morph Ovum            | PORKALATOR
extern int selective_arti_6;   // Chaos Device
extern int selective_arti_7;   // Shadowsphere          | BANISHMENT DEVICE
extern int selective_arti_8;   // Wings of Wrath
extern int selective_arti_9;   // Torch
extern int selective_arti_10;  //                       | KRATER OF MIGHT
extern int selective_arti_11;  //                       | DRAGONSKIN BRACERS
extern int selective_arti_12;  //                       | DARK SERVANT
extern int selective_arti_13;  //                       | BOOTS OF SPEED
extern int selective_arti_14;  //                       | MYSTIC AMBIT INCANT

extern int selective_puzzle_0;  // FLAME MASK
extern int selective_puzzle_1;  // HEART OF D'SPARIL
extern int selective_puzzle_2;  // RUBY PLANET
extern int selective_puzzle_3;  // EMERALD PLANET 1
extern int selective_puzzle_4;  // EMERALD PLANET 2
extern int selective_puzzle_5;  // SAPPHIRE PLANET 1
extern int selective_puzzle_6;  // SAPPHIRE PLANET 2
extern int selective_puzzle_7;  // CLOCK GEAR ((S)
extern int selective_puzzle_8;  // CLOCK GEAR ((B)
extern int selective_puzzle_9;  // CLOCK GEAR ((S B)
extern int selective_puzzle_10; // CLOCK GEAR ((B S)
extern int selective_puzzle_11; // DAEMON CODEX
extern int selective_puzzle_12; // LIBER OSCURA
extern int selective_puzzle_13; // YORICK'S SKULL
extern int selective_puzzle_14; // GLAIVE SEAL
extern int selective_puzzle_15; // HOlY RELIC
extern int selective_puzzle_16; // SIGIL OF THE MAGUS

// -----------------------------------------------------------------------------
// Gameplay feautures
// -----------------------------------------------------------------------------

// Gameplay: Game Mechanics
extern int strict_mode;

// Gameplay: Graphical
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int improved_fuzz;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int linear_sky;
extern int randomly_flipcorpses;
extern int flip_weapons;

// Gameplay: Status Bar
extern int extra_player_faces;
extern int negative_health;
extern int sbar_colored;
extern int sbar_colored_gem;
extern int stbar_color_high;
extern int stbar_color_normal;
extern int stbar_color_low;
extern int stbar_color_critical;
extern int stbar_color_armor_1;
extern int stbar_color_armor_2;
extern int stbar_color_armor_0;
extern int ammo_widget;
extern int ammo_widget_colored;
extern int weapon_widget;
extern int center_inventory_cursor;

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
extern int autoaim_horizonal;
extern int autoaim_vertical;
extern int show_all_artifacts;
extern int show_artifacts_timer;

// Gameplay: Physical
extern int improved_collision;
extern int over_under;
extern int torque;
extern int ssg_blast_enemies;
extern int floating_powerups;
extern int toss_drop;
extern int weapon_bobbing;

// Gameplay: Crosshair
extern int crosshair_draw;
extern int crosshair_shape;
extern int crosshair_type;
extern int crosshair_scale;
extern int crosshair_opacity;

// Gameplay: Gameplay
extern int default_skill;
extern int fix_map_errors;
extern int flip_levels;
extern int pistol_start;
extern int breathing;
extern int unlimited_lost_souls;
extern int fast_quickload;
extern int skip_unused_artifact;

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

typedef enum
{
    v_message_color_secret = 0,
    v_message_color_quest,
    v_stbar_color_high,
    v_stbar_color_normal,
    v_stbar_color_low,
    v_stbar_color_critical,
    v_stbar_color_armor_1,
    v_stbar_color_armor_2,
    v_stbar_color_armor_0
} notCommonVar_t;

/**
 * [Dasperal] This function is used to get a pointer to some of the previously declared variables.
 * It is needed because direct use of them leads to a linking error
 */
int* JN_getNotCommonIntVarPointer(notCommonVar_t var);

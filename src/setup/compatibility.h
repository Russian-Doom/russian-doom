//
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



#ifndef SETUP_COMPATIBILITY_H
#define SETUP_COMPATIBILITY_H

void CompatibilitySettings(void);
void BindCompatibilityVariables(void);


// [JN] Дополнительные параметры игры

// Rendering
extern int show_diskicon;

// Display
extern int level_brightness;
extern int menu_shading;
extern int hud_detaillevel;
extern int messages_alignment;
extern int messages_timeout;
extern int message_pickup_color;
extern int message_secret_color;
extern int message_system_color;
extern int message_chat_color;
extern int draw_shadowed_text;

// Automap
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
extern int hud_widget_colors;

// Графика
extern int brightmaps;
extern int fake_contrast;
extern int translucency;
extern int improved_fuzz;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int linear_sky;
extern int flip_weapons;

// Status Bar
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
extern int ammo_widget;
extern int ammo_widget_colored;

// Звук
extern int crushed_corpses_sfx;
extern int blazing_door_fix_sfx;
extern int correct_endlevel_sfx;
extern int z_axis_sfx;
extern int play_exit_sfx;
extern int noise_alert_sfx;

// Тактика
extern int secret_notification;
extern int infragreen_visor;
extern int show_all_artifacts;
extern int show_artifacts_timer;

// Физика
extern int over_under;
extern int torque;
extern int weapon_bobbing;
extern int ssg_blast_enemies;
extern int randomly_flipcorpses;
extern int floating_powerups;
extern int toss_drop;

// Геймплей
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

// Прицел
extern int crosshair_draw;
extern int crosshair_type;
extern int crosshair_scale;


#endif /* #ifndef SETUP_COMPATIBILITY_H */


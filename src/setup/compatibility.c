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


#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "https://jnechaevsky.github.io/projects/rusdoom/setup/index.html"


// Display
int level_brightness = 0;
int menu_shading = 0;
int hud_detaillevel = 0;
int messages_alignment = 0;
int messages_timeout = 4;
int message_pickup_color = 0;
int message_secret_color = 7;
int message_system_color = 0;
int message_chat_color = 2;
int draw_shadowed_text = 1;

// Automap
int automap_color = 0;
int automap_antialias = 1;
int automap_stats = 1;
int automap_level_time = 1;
int automap_total_time = 0;
int automap_coords = 0;
int automap_overlay = 0;
int automap_rotate = 0;
int automap_follow = 1;
int automap_grid = 0;
int automap_grid_size = 128;
int hud_widget_colors = 0;

// Gameplay: Graphical
int brightmaps = 1;
int fake_contrast = 0;
int translucency = 1;
int improved_fuzz = 2;
int colored_blood = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int linear_sky = 1;
int flip_weapons = 0;

// Gameplay: Status Bar
int extra_player_faces = 1;
int negative_health = 0;
int sbar_colored = 0;
int sbar_colored_gem = 0;
int sbar_color_high = 5;
int sbar_color_normal = 2;
int sbar_color_low = 7;
int sbar_color_critical = 0;
int sbar_color_armor_1 = 2;
int sbar_color_armor_2 = 5;
int sbar_color_armor_0 = 0;
int ammo_widget = 0;
int ammo_widget_colored = 1;

// Gameplay: Audible
int z_axis_sfx = 0;
int play_exit_sfx = 0;
int crushed_corpses_sfx = 1;
int blazing_door_fix_sfx = 1;
int noise_alert_sfx = 0;
int correct_endlevel_sfx = 0;

// Gameplay: Tactical
int secret_notification = 1;
int infragreen_visor = 0;
int show_all_artifacts = 0;
int show_artifacts_timer = 0;

// Gameplay: Physical
int over_under = 0;
int torque = 1;
int weapon_bobbing = 1;
int ssg_blast_enemies = 1;
int randomly_flipcorpses = 1;
int floating_powerups = 1;
int toss_drop = 1;

// Gameplay: Crosshair
int crosshair_draw = 0;
int crosshair_type = 1;
int crosshair_scale = 0;

// Gameplay: Gameplay
int fix_map_errors = 1;
int flip_levels = 0;
int unlimited_lost_souls = 1;
int agressive_lost_souls = 0;
int pistol_start = 0;
int fast_quickload = 1;

// Gameplay: Demos
int demotimer = 0;
int demotimerdir = 0;
int demobar = 0;
int no_internal_demos = 0;


// int show_exit_sequence = 1;  // [Strife]


// [JN] No longer used, now using in-game menu
/*
void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_table_t  *window_features;

    window = TXT_NewWindow(english_language ?
                           "Optional Gameplay Enhacements" :
                           "Дополнительные параметры игры");
    window_features = TXT_NewTable(1);

    if (english_language)
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);
    else
    TXT_SetWindowHelpURL_RUS(window, WINDOW_HELP_URL);

    //
    // [JN] Create translated buttons
    //

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, english_language ?
                        TXT_NewWindowAbortAction(window) :
                        TXT_NewWindowAbortAction_Rus(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, english_language ?
                        TXT_NewWindowSelectAction(window) :
                        TXT_NewWindowSelectAction_Rus(window));

    if (gamemission == doom)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 15, window_features));
    else if (gamemission == heretic)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 15, window_features));
    else if (gamemission == hexen)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 10, window_features));

    TXT_AddWidgets(window_features,

    TXT_NewSeparator(english_language ?
        "Graphical" :
        "Графика"),
        TXT_NewCheckBox(english_language ?
            "Brightmaps for textures and sprites" :
            "Брайтмаппинг текстур и спрайтов",
            &brightmaps),
        TXT_NewCheckBox(english_language ?
            "Apply fake contrast on walls" :
            "Имитаци€ контрастного освещени€ стен",
            &fake_contrast),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Translucency" :
            "Юффект прозрачности у некоторых объектов",
            &translucency)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Swirling liquids" :
            "Улучшенна€ анимаци€ жидкостей",
            &swirling_liquids)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Invulberability affects sky" :
            "Неу€звимость окрашивает небо",
            &invul_sky)),    
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Colored HUD elements" :
            "Разноцветные элементы HUD",
            &colored_hud)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Colored blood and corpses" :
            "Разноцветна€ кровь и трупы",
            &colored_blood)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Red resurrection flash" :
            "Красна€ вспышка воскрешени€ монстров",
            &red_resurrection_flash)),
        TXT_NewCheckBox(english_language ?
            "Texts are dropping shadow" :
            "Юлементы меню и тексты отбрасывают тень",
            &draw_shadowed_text),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Show disk icon" :
            "Показывать значок дискеты",
            &show_diskicon)),

    TXT_If(gamemission == doom, TXT_NewSeparator(english_language ?
        "Audible" :
        "Звук")), 
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Sound of crushing corpses" :
            "Звук раздавливани€ трупов",
            &crushed_corpses_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Single sound of closing blazing door" :
            "Одиночный звук закрыти€ быстрой двери",
            &blazing_door_fix_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Play exit sounds" :
            "Проигрывать звук при выходе из игры",
            &play_exit_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Correct sound of level ending" :
            "Корректный звук завершени€ уровн€",
            &correct_endlevel_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Monsters alert sound waking up others" :
            "Тревога монстра пробуждает других монстров",
            &noise_alert_sfx)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator(english_language ?
    "Tactical" :
    "Тактика")),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Show level stats on automap" :
            "Отображать статистику уровн€ на карте",
            &automap_stats)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Notification of revealed secters" :
            "Уведомление об обнаружении тайников",
            &secret_notification)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Show negative health" :
            "Отображать отрицательное здоровье",
            &negative_health)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Infragreen light amplification visor" :
            "Инфразеленый визор усилени€ освещени€",
            &infragreen_visor)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator(english_language ?
    "Physical" :
    "Физика")),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Walk over and under monsters" :
            "Игрок может проходить под и над монстрами",
            &over_under)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Corpses sliding from the ledges" :
            "Трупы соскальзывают с выступов и обрывов",
            &torque)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Weapon bobbing while firing" :
            "Покачивание оружи€ при стрельбе в движении",
            &weapon_bobbing)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Lethal pellet of a point-blank SSG" :
            "Двуствольное ружье может разрывать врагов",
            &ssg_blast_enemies)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Randomly mirrored corpses" :
            "Произвольное зеркальное отражение трупов",
            &randomly_flipcorpses)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Floating powerups" :
            "Левитирующие сферы-артефакты",
            &floating_powerups)),

    TXT_NewSeparator(english_language ?
    "Gameplay" :
    "Геймплей"),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Fix errors of vanilla maps" :
            "Исправл€ть ошибки оригинальных уровней",
            &fix_map_errors)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Flip game levels" :
            "Зеркальное отражение уровней",
            &flip_levels)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Extra player faces on the HUD" :
            "Дополнительные лица игрока в HUD",
            &extra_player_faces)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Pain Elemental without Souls limit" :
            "Юлементаль Боли без ограничени€ душ",
            &unlimited_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "More agressive Lost Souls" :
            "Повышенна€ агрессивность Потер€нных душ",
            &agressive_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Don't prompt for quick loading/saving" :
            "Не выводить запрос при быстрой загрузке",
            &fast_quickload)),
        TXT_NewCheckBox(english_language ?
            "Don't play internal demos" :
            "Не проигрывать внутренние демозаписи",
            &no_internal_demos),

    TXT_NewSeparator(english_language ?
    "Crosshair" :
    "Прицел"),
        TXT_NewCheckBox(english_language ?
            "Draw crosshair" :
            "Отображать прицел",
            &crosshair_draw),
        TXT_NewCheckBox(english_language ?
            "Health indication" :
            "Индикаци€ здоровь€",
            &crosshair_health),
        TXT_NewCheckBox(english_language ?
            "Increased size" :
            "Увеличенный размер",
            &crosshair_scale),

        // TXT_If(gamemission == strife, TXT_NewCheckBox("Показывать значок песочных часов",        &show_diskicon)),
        // TXT_If(gamemission == strife, TXT_NewCheckBox("Показывать заставку при выходе",          &show_exit_sequence)),

    NULL);
}
*/

void BindCompatibilityVariables(void)
{
    // Display
    M_BindIntVariable("level_brightness",       &level_brightness);
    M_BindIntVariable("menu_shading",           &menu_shading);
    M_BindIntVariable("hud_detaillevel",        &hud_detaillevel);
    M_BindIntVariable("messages_alignment",     &messages_alignment);
    M_BindIntVariable("messages_timeout",       &messages_timeout);
    M_BindIntVariable("message_pickup_color",   &message_pickup_color);
    M_BindIntVariable("message_secret_color",   &message_secret_color);
    M_BindIntVariable("message_system_color",   &message_system_color);
    M_BindIntVariable("message_chat_color",     &message_chat_color);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);
    
    // Automap
    M_BindIntVariable("automap_color",          &automap_color);
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("automap_level_time",     &automap_level_time);
    M_BindIntVariable("automap_total_time",     &automap_total_time);
    M_BindIntVariable("automap_coords",         &automap_coords);
    M_BindIntVariable("automap_overlay",        &automap_overlay);
    M_BindIntVariable("automap_rotate",         &automap_rotate);
    M_BindIntVariable("automap_follow",         &automap_follow);
    M_BindIntVariable("automap_grid",           &automap_grid);
    M_BindIntVariable("automap_grid_size",      &automap_grid_size);
    M_BindIntVariable("hud_widget_colors",      &hud_widget_colors);

    // Gameplay: Graphical
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("improved_fuzz",          &improved_fuzz);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("linear_sky",             &linear_sky);
    M_BindIntVariable("flip_weapons",           &flip_weapons);

    // Gameplay: Status Bar
    M_BindIntVariable("extra_player_faces",     &extra_player_faces);
    M_BindIntVariable("negative_health",        &negative_health);
    M_BindIntVariable("sbar_colored",           &sbar_colored);
    M_BindIntVariable("sbar_colored_gem",       &sbar_colored_gem);
    M_BindIntVariable("sbar_color_high",        &sbar_color_high);
    M_BindIntVariable("sbar_color_normal",      &sbar_color_normal);
    M_BindIntVariable("sbar_color_low",         &sbar_color_low);
    M_BindIntVariable("sbar_color_critical",    &sbar_color_critical);
    M_BindIntVariable("sbar_color_armor_1",     &sbar_color_armor_1);
    M_BindIntVariable("sbar_color_armor_2",     &sbar_color_armor_2);
    M_BindIntVariable("sbar_color_armor_0",     &sbar_color_armor_0);
    M_BindIntVariable("ammo_widget",             &ammo_widget);
    M_BindIntVariable("ammo_widget_colored",     &ammo_widget_colored);

    // Gameplay: Audible
    M_BindIntVariable("z_axis_sfx",             &z_axis_sfx);
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);
    M_BindIntVariable("noise_alert_sfx",        &noise_alert_sfx);
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);

    // Gameplay: Tactical
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("infragreen_visor",       &infragreen_visor);
    M_BindIntVariable("show_all_artifacts",     &show_all_artifacts);
    M_BindIntVariable("show_artifacts_timer",   &show_artifacts_timer);

    // Gameplay: Physical
    M_BindIntVariable("over_under",             &over_under);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("floating_powerups",      &floating_powerups);
    M_BindIntVariable("toss_drop",              &toss_drop);

    // Gameplay: Crosshair
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_type",         &crosshair_type);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);

    // Gameplay: Gameplay
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);
    M_BindIntVariable("fast_quickload",         &fast_quickload);

    // Gameplay: Demos
    M_BindIntVariable("demotimer",              &demotimer);
    M_BindIntVariable("demotimerdir",           &demotimerdir);
    M_BindIntVariable("demobar",                &demobar);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);

    // M_BindIntVariable("show_exit_sequence",  &show_exit_sequence);    // [Strife]
}

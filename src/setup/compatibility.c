//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2019 Julian Nechaevsky
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



// Sound control menu

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/gameplay.html"


// [JN] Дополнительные параметры игры

// Графика
int brightmaps = 1;
int fake_contrast = 0;
int translucency = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int colored_hud = 0;
int colored_blood = 1;
int red_resurrection_flash = 1;
int draw_shadowed_text = 1;
int show_diskicon = 1;

// Звук
int crushed_corpses_sfx = 1;
int blazing_door_fix_sfx = 1;
int correct_endlevel_sfx = 0;
int play_exit_sfx = 1;
int noise_alert_sfx = 0;

// Тактика
int automap_stats = 1;
int secret_notification = 1;
int negative_health = 0;
int infragreen_visor = 0;

// Физика
int over_under = 0;
int torque = 1;
int weapon_bobbing = 1;
int ssg_blast_enemies = 1;
int randomly_flipcorpses = 1;
int floating_powerups = 0;

// Геймплей
int fix_map_errors = 1;
int flip_levels = 0;
int extra_player_faces = 1;
int unlimited_lost_souls = 1;
int agressive_lost_souls = 0;
int fast_quickload = 1;
int no_internal_demos = 0;

// Прицел
int crosshair_draw = 0;
int crosshair_health = 1;
int crosshair_scale = 0;

// int show_exit_sequence = 1;  // [Strife]

void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_table_t  *window_features;

    window = TXT_NewWindow(english_language ?
                           "Optional Gameplay Enhacements" :
                           "„ополнительные параметры игры");
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
        "ѓрафика"),
        TXT_NewCheckBox(english_language ?
            "Brightmaps for textures and sprites" :
            "Ѓрайтмаппинг текстур и спрайтов",
            &brightmaps),
        TXT_NewCheckBox(english_language ?
            "Apply fake contrast on walls" :
            "€митация контрастного освещения стен",
            &fake_contrast),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Translucency" :
            "ћффект прозрачности у некоторых объектов",
            &translucency)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Swirling liquids" :
            "“лучшенная анимация жидкостей",
            &swirling_liquids)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Invulberability affects sky" :
            "Ќеуязвимость окрашивает небо",
            &invul_sky)),    
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Colored HUD elements" :
            "ђазноцветные элементы HUD",
            &colored_hud)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Colored blood and corpses" :
            "ђазноцветная кровь и трупы",
            &colored_blood)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Red resurrection flash" :
            "Љрасная вспышка воскрешения монстров",
            &red_resurrection_flash)),
        TXT_NewCheckBox(english_language ?
            "Texts are dropping shadow" :
            "ћлементы меню и тексты отбрасывают тень",
            &draw_shadowed_text),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Show disk icon" :
            "Џоказывать значок дискеты",
            &show_diskicon)),

    TXT_If(gamemission == doom, TXT_NewSeparator(english_language ?
        "Audible" :
        "‡вук")), 
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Sound of crushing corpses" :
            "‡вук раздавливания трупов",
            &crushed_corpses_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Single sound of closing blazing door" :
            "Ћдиночный звук закрытия быстрой двери",
            &blazing_door_fix_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Play exit sounds" :
            "Џроигрывать звук при выходе из игры",
            &play_exit_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Correct sound of level ending" :
            "Љорректный звук завершения уровня",
            &correct_endlevel_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Monsters alert sound waking up others" :
            "’ревога монстра пробуждает других монстров",
            &noise_alert_sfx)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator(english_language ?
    "Tactical" :
    "’актика")),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Show level stats on automap" :
            "Ћтображать статистику уровня на карте",
            &automap_stats)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Notification of revealed secters" :
            "“ведомление об обнаружении тайников",
            &secret_notification)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Show negative health" :
            "Ћтображать отрицательное здоровье",
            &negative_health)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Infragreen light amplification visor" :
            "€нфразеленый визор усиления освещения",
            &infragreen_visor)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator(english_language ?
    "Physical" :
    "”изика")),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Walk over and under monsters" :
            "€грок может проходить под и над монстрами",
            &over_under)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Corpses sliding from the ledges" :
            "’рупы соскальзывают с выступов и обрывов",
            &torque)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Weapon bobbing while firing" :
            "Џокачивание оружия при стрельбе в движении",
            &weapon_bobbing)),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Lethal pellet of a point-blank SSG" :
            "„вуствольное ружье может разрывать врагов",
            &ssg_blast_enemies)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Randomly mirrored corpses" :
            "Џроизвольное зеркальное отражение трупов",
            &randomly_flipcorpses)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Floating powerups" :
            "‹евитирующие сферы-артефакты",
            &floating_powerups)),

    TXT_NewSeparator(english_language ?
    "Gameplay" :
    "ѓеймплей"),
        TXT_If(gamemission == doom, TXT_NewCheckBox(english_language ?
            "Fix errors of vanilla maps" :
            "€справлять ошибки оригинальных уровней",
            &fix_map_errors)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox(english_language ?
            "Flip game levels" :
            "‡еркальное отражение уровней",
            &flip_levels)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Extra player faces on the HUD" :
            "„ополнительные лица игрока в HUD",
            &extra_player_faces)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Pain Elemental without Souls limit" :
            "ћлементаль Ѓоли без ограничения душ",
            &unlimited_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "More agressive Lost Souls" :
            "Џовышенная агрессивность Џотерянных душ",
            &agressive_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox(english_language ?
            "Don't prompt for quick loading/saving" :
            "Ќе выводить запрос при быстрой загрузке",
            &fast_quickload)),
        TXT_NewCheckBox(english_language ?
            "Don't play internal demos" :
            "Ќе проигрывать внутренние демозаписи",
            &no_internal_demos),

    TXT_NewSeparator(english_language ?
    "Crosshair" :
    "Џрицел"),
        TXT_NewCheckBox(english_language ?
            "Draw crosshair" :
            "Ћтображать прицел",
            &crosshair_draw),
        TXT_NewCheckBox(english_language ?
            "Health indication" :
            "€ндикация здоровья",
            &crosshair_health),
        TXT_NewCheckBox(english_language ?
            "Increased size" :
            "“величенный размер",
            &crosshair_scale),

        // TXT_If(gamemission == strife, TXT_NewCheckBox("Џоказывать значок песочных часов",        &show_diskicon)),
        // TXT_If(gamemission == strife, TXT_NewCheckBox("Џоказывать заставку при выходе",          &show_exit_sequence)),

    NULL);
}

void BindCompatibilityVariables(void)
{
    // Графика
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("colored_hud",            &colored_hud);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("red_resurrection_flash", &red_resurrection_flash);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);
    M_BindIntVariable("show_diskicon",          &show_diskicon);

    // Звук
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);
    M_BindIntVariable("noise_alert_sfx",        &noise_alert_sfx);

    // Тактика
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("negative_health",        &negative_health);
    M_BindIntVariable("infragreen_visor",       &infragreen_visor);

    // Физика
    M_BindIntVariable("over_under",             &over_under);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("floating_powerups",      &floating_powerups);

    // Геймплей
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("extra_player_faces",     &extra_player_faces);
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);
    M_BindIntVariable("fast_quickload",         &fast_quickload);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);

    // Прицел
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_health",       &crosshair_health);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);

    // M_BindIntVariable("show_exit_sequence",     &show_exit_sequence);    // [Strife]
}


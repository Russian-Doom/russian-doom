//
// Copyright(C) 2005-2014 Simon Howard
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

// Russian Doom (C) 2016-2018 Julian Nechaevsky


// Sound control menu

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/gameplay.html"


// [JN] ƒополнительные параметры игры

// √рафика
int uncapped_fps = 0;
int brightmaps = 1;
int fake_contrast = 0;
int translucency = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int colored_blood = 1;
int red_resurrection_flash = 1;
int draw_shadowed_text = 1;
int show_diskicon = 1;

// «вук
int crushed_corpses_sfx = 1;
int blazing_door_fix_sfx = 1;
int correct_endlevel_sfx = 0;
int play_exit_sfx = 1;

// “актика
int automap_stats = 1;
int secret_notification = 1;
int negative_health = 0;
int infragreen_visor = 0;

// ‘изика
int over_under = 0;
int torque = 1;
int weapon_bobbing = 1;
int ssg_blast_enemies = 1;
int randomly_flipcorpses = 1;
int floating_powerups = 0;

// √еймплей
int fix_map_errors = 1;
int flip_levels = 0;
int new_ouch_face = 1;
int unlimited_lost_souls = 1;
int agressive_lost_souls = 0;
int fast_quickload = 1;

// ѕрицел
int crosshair_draw = 0;
int crosshair_health = 0;
int crosshair_scale = 0;

// int show_exit_sequence = 1;  // [Strife]

void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_table_t  *window_features;

    window = TXT_NewWindow("Дополнительные параметры игры");
    window_features = TXT_NewTable(1);

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    if (gamemission == doom)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 15, window_features));
    else if (gamemission == heretic)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 15, window_features));
    else if (gamemission == hexen)
        TXT_AddWidget(window, TXT_NewScrollPane(47, 8, window_features));

    TXT_AddWidgets(window_features,

    TXT_NewSeparator("Графика"),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Сн€ть ограничение в 35 fps",                               &uncapped_fps)),
        TXT_NewCheckBox("Брайтмаппинг текстур и спрайтов",                                                      &brightmaps),
        TXT_NewCheckBox("Имитаци€ контрастного освещени€ стен",                                                 &fake_contrast),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Юффект прозрачности у некоторых объектов",                 &translucency)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Улучшенна€ анимаци€ жидкостей",                            &swirling_liquids)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Неу€звимость окрашивает небо",   &invul_sky)),    
        TXT_If(gamemission == doom, TXT_NewCheckBox("Разноцветна€ кровь и трупы",                               &colored_blood)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Красна€ вспышка воскрешени€ монстров",                     &red_resurrection_flash)),
        TXT_NewCheckBox("Юлементы меню и тексты отбрасывают тень",                                              &draw_shadowed_text),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Показывать значок дискеты",                                &show_diskicon)),

    TXT_If(gamemission == doom, TXT_NewSeparator("Звук")), 
        TXT_If(gamemission == doom, TXT_NewCheckBox("Звук раздавливани€ трупов",                &crushed_corpses_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Одиночный звук закрыти€ быстрой двери",    &blazing_door_fix_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Проигрывать звук при выходе из игры",      &play_exit_sfx)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Корректный звук завершени€ уровн€",        &correct_endlevel_sfx)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator("Тактика")),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Отображать статистику уровн€ на карте",  &automap_stats)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Уведомление об обнаружении тайников",    &secret_notification)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Отображать отрицательное здоровье",                                &negative_health)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Инфразеленый визор усилени€ освещени€",                            &infragreen_visor)),

    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator("Физика")),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Игрок может проходить под и над монстрами",                            &over_under)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Трупы соскальзывают с выступов и обрывов",   &torque)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Покачивание оружи€ при стрельбе в движении", &weapon_bobbing)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Двуствольное ружье может разрывать врагов",                            &ssg_blast_enemies)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Произвольное зеркальное отражение трупов",   &randomly_flipcorpses)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Левитирующие сферы-артефакты",                                         &floating_powerups)),
        
    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator("Геймплей")),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Исправл€ть ошибки оригинальных уровней",                   &fix_map_errors)),
        TXT_If(gamemission == doom || gamemission == heretic, TXT_NewCheckBox("Зеркальное отражение уровней",   &flip_levels)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Корректна€ формула \"Ouch face\"",                         &new_ouch_face)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Юлементаль Боли без ограничени€ душ",                      &unlimited_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Повышенна€ агрессивность Потер€нных душ",                  &agressive_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Не выводить запрос при быстрой загрузке",                  &fast_quickload)),

    TXT_NewSeparator("Прицел"),
        TXT_NewCheckBox("Отображать прицел",    &crosshair_draw),
        TXT_NewCheckBox("Индикаци€ здоровь€",   &crosshair_health),
        TXT_NewCheckBox("Увеличенный размер",   &crosshair_scale),

        // TXT_If(gamemission == strife, TXT_NewCheckBox("Показывать значок песочных часов",        &show_diskicon)),
        // TXT_If(gamemission == strife, TXT_NewCheckBox("Показывать заставку при выходе",          &show_exit_sequence)),

    NULL);
}

void BindCompatibilityVariables(void)
{
    // √рафика
    M_BindIntVariable("uncapped_fps",           &uncapped_fps);
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("red_resurrection_flash", &red_resurrection_flash);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);
    M_BindIntVariable("show_diskicon",          &show_diskicon);

    // «вук
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);

    // “актика
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("negative_health",        &negative_health);
    M_BindIntVariable("infragreen_visor",       &infragreen_visor);

    // ‘изика
    M_BindIntVariable("over_under",             &over_under);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("floating_powerups",      &floating_powerups);

    // √еймплей
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("new_ouch_face",          &new_ouch_face);
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);
    M_BindIntVariable("fast_quickload",         &fast_quickload);

    // ѕрицел
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_health",       &crosshair_health);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);

    // M_BindIntVariable("show_exit_sequence",     &show_exit_sequence);    // [Strife]
}


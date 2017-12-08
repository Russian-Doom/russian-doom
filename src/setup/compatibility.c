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

// Russian Doom (C) 2016-2017 Julian Nechaevsky


// Sound control menu

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/gameplay.html"

// [JN] Дополнительные параметры игры

// - Интерфейс -
int draw_shadowed_text = 1;      // Элементы меню и тексты отбрасывают тень
int fast_quickload = 1;          // Не выводить запрос при быстрой загрузке
int show_diskicon = 1;           // Значок дискеты / [Strife] песочных часов
int show_exit_sequence = 1;      // [Strife] Показывать заставку при выходе
// - Графика -
int brightmaps = 1;              // Брайтмаппинг текстур и спрайтов
int fake_contrast = 0;           // Имитация контрастного освещения стен
int colored_blood = 1;           // Кровь разных цветов
int randomly_flipcorpses = 1;    // Произвольное зеркальное отражение трупов
int new_ouch_face = 1;           // Корректная формула "Ouch face"
int invul_sky = 1;               // Неуязвимость окрашивает небо
int swirling_liquids = 1;        // Улучшенная анимация жидкостей
int red_resurrection_flash = 1;  // Красная вспышка воскрешения монстров
int ssg_blast_enemies = 1;       // Двуствольное ружье может разрывать врагов
int translucency = 1;            // Прозрачность объектов
int weapon_bobbing = 1;          // Покачивание оружия при стрельбе в движении
// - Звук -
int crushed_corpses_sfx = 1;     // Звук раздавливания трупов
int blazing_door_fix_sfx = 1;    // Одиночный звук закрытия быстрой двери
int correct_endlevel_sfx = 0;    // Корректный звук завершения уровня
int play_exit_sfx = 1;           // Проигрывать звук при выходе из игры
// - Геймплей -
int secret_notification = 1;      // Уведомление об обнаружении секрета
int show_total_time = 1;          // Показывать общее время
int unlimited_lost_souls = 1;     // Элементаль боли без ограничения
int agressive_lost_souls = 0;     // Повышенная агрессивность Потерянных Душ
int negative_health = 0;          // Отображать отрицательное здоровье
int flip_levels = 0;              // Зеркальное отображение уровней

void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_table_t  *window_features;

    window = TXT_NewWindow("„ополнительные параметры игры");
    window_features = TXT_NewTable(1);

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidget(window, TXT_NewScrollPane(47, 15, window_features));

    TXT_AddWidgets(window_features,
    TXT_If(gamemission == doom || gamemission == heretic, TXT_NewSeparator("€нтерфейс")),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("ћлементы меню и тексты отбрасывают тень",   &draw_shadowed_text)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Ќе выводить запрос при быстрой загрузке",   &fast_quickload)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Ћбщее время на межмиссионном экране",       &show_total_time)),
        TXT_If(gamemission == doom, TXT_NewCheckBox("Џоказывать значок дискеты",                 &show_diskicon)),
        
        // [JN] Элементы для Strife 
        TXT_If(gamemission == strife, TXT_NewCheckBox("Џоказывать значок песочных часов",        &show_diskicon)),
        TXT_If(gamemission == strife, TXT_NewCheckBox("Џоказывать заставку при выходе",          &show_exit_sequence)),
    
    TXT_NewSeparator("ѓрафика"),
        TXT_NewCheckBox("Ѓрайтмаппинг текстур и спрайтов",           &brightmaps),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("€митация контрастного освещения стен",      &fake_contrast)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("ћффект прозрачности у некоторых объектов",  &translucency)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("“лучшенная анимация жидкостей",             &swirling_liquids)),
        TXT_If(gamemission == doom || gamemission == heretic,   TXT_NewCheckBox("Џроизвольное зеркальное отражение трупов",  &randomly_flipcorpses)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("ђазноцветная кровь и трупы",                &colored_blood)),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("Ќеуязвимость окрашивает небо",              &invul_sky)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Љрасная вспышка воскрешения монстров",      &red_resurrection_flash)),

    TXT_If(gamemission == doom,    TXT_NewSeparator("‡вук")), 
        TXT_If(gamemission == doom,	TXT_NewCheckBox("‡вук раздавливания трупов",                 &crushed_corpses_sfx)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Ћдиночный звук закрытия быстрой двери",     &blazing_door_fix_sfx)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Џроигрывать звук при выходе из игры",       &play_exit_sfx)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Љорректный звук завершения уровня",         &correct_endlevel_sfx)),

    TXT_If(gamemission == doom || gamemission == heretic,    TXT_NewSeparator("ѓеймплей")),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("“ведомление об обнаружении тайников",       &secret_notification)),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("Џокачивание оружия при стрельбе в движении",&weapon_bobbing)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Љорректная формула \"Ouch face\"",          &new_ouch_face)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("„вуствольное ружье может разрывать врагов", &ssg_blast_enemies)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("ћлементаль Ѓоли без ограничения душ",       &unlimited_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Џовышенная агрессивность Џотерянных душ",   &agressive_lost_souls)),
        TXT_If(gamemission == doom,	TXT_NewCheckBox("Ћтображать отрицательное здоровье",         &negative_health)),
        TXT_If(gamemission == doom || gamemission == heretic,	TXT_NewCheckBox("‡еркальное отражение уровней",              &flip_levels)),
    NULL);
}

void BindCompatibilityVariables(void)
{
    // [JN] Дополнительные параметры игры

    // - Интерфейс -
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);       // Элементы меню и тексты отбрасывают тень
    M_BindIntVariable("fast_quickload",         &fast_quickload);           // Не выводить запрос при быстрой загрузке
    M_BindIntVariable("show_total_time",        &show_total_time);          // Показывать общее время
    M_BindIntVariable("show_exit_sequence",     &show_exit_sequence);       // [Strife] Показывать заставку при выходе
    M_BindIntVariable("show_diskicon",          &show_diskicon);            // Показывать значок дискеты
    // - Графика -
    M_BindIntVariable("brightmaps",             &brightmaps);               // Брайтмаппинг текстур и спрайтов
    M_BindIntVariable("fake_contrast",          &fake_contrast);            // Имитация контрастного освещения стен
    M_BindIntVariable("translucency",           &translucency);             // Прозрачность объектов
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);         // Улучшенная анимация жидкостей
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);     // Произвольное зеркальное отражение трупов
    M_BindIntVariable("colored_blood",          &colored_blood);            // Кровь разных цветов
    M_BindIntVariable("invul_sky",              &invul_sky);                // Неуязвимость окрашивает небо
    M_BindIntVariable("red_resurrection_flash", &red_resurrection_flash);   // Красная вспышка воскрешения монстров
    // - Звук -
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);      // Звук раздавливания трупов
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);     // Одиночный звук закрытия быстрой двери
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);            // Проигрывать звук при выходе из игры
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);     // Корректный звук завершения уровня
    // - Геймплей -
    M_BindIntVariable("secret_notification",    &secret_notification);      // Уведомление об обнаружении секрета
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);           // Покачивание оружия при стрельбе в движении
    M_BindIntVariable("new_ouch_face",          &new_ouch_face);            // Корректная формула "Ouch face"
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);        // Двуствольное ружье может разрывать врагов
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);     // Элементаль боли без ограничения
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);     // Повышенная агрессивность Потерянных душ
    M_BindIntVariable("negative_health",        &negative_health);          // Отображать отрицательное здоровье
    M_BindIntVariable("flip_levels",            &flip_levels);              // Зеркальное отражение уровней
}


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

// Russian DOOM (C) 2016-2017 Julian Nechaevsky

// Sound control menu

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/gameplay.html"

extern int english_setup;

// [JN] Оригинальные ограничения отключены, т.к. в них больше нет смысла
/*
int vanilla_savegame_limit = 1;
int vanilla_demo_limit = 1;
*/

// [JN] Дополнительные параметры игры

// - Графика -
int colored_blood = 1;           // Кровь разных цветов
int new_ouch_face = 1;           // Корректная формула "Ouch face"
int invul_sky = 1;               // Неуязвимость окрашивает небо
int red_resurrection_flash = 1;  // Красная вспышка воскрешения монстров
int ssg_blast_enemies = 1;       // Двуствольное ружье может разрывать врагов
int translucency = 1;            // Прозрачность объектов
int no_pickup_flash = 0;         // Не мигать экраном при получении предметов
// - Звук -
int crushed_corpses_sfx = 1;     // Звук раздавливания трупов
int blazing_door_fix_sfx = 1;    // Одиночный звук закрытия быстрой двери
int play_exit_sfx = 1;           // Проигрывать звук при выходе из игры
// - Геймплей -
int secret_notification = 1;     // Уведомление об обнаружении секрета
int show_total_time = 1;         // Показывать общее время
int unlimited_lost_souls = 1;    // Элементаль боли без ограничения

void CompatibilitySettings(void)
{
    txt_window_t *window;

    /* English language */
    if (english_setup)
    {
        window = TXT_NewWindow("Additional gameplay options");

        TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

        TXT_AddWidgets(window,
        TXT_If(gamemission == doom,    TXT_NewSeparator("Graphics")),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Colored blood and squished corpses",          &colored_blood)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Correct formula of \"Ouch face\"",            &new_ouch_face)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Invulnerability affects sky",                 &invul_sky)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Red flash of monsters resurrection",          &red_resurrection_flash)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Super Shotgun has a chance to blast enemies", &ssg_blast_enemies)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Low-key transparency on some objects",        &translucency)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("No yellow flash on picking up items",         &no_pickup_flash)),

        TXT_If(gamemission == doom,    TXT_NewSeparator("Sound")), 
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Sound of crushed corpses",           &crushed_corpses_sfx)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Single sound of fast closing doors", &blazing_door_fix_sfx)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Playing sound on exiting the game",  &play_exit_sfx)),

        TXT_If(gamemission == doom,    TXT_NewSeparator("Gameplay")),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Notification of discovered secrets",       &secret_notification)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Total level times on intermission screen", &show_total_time)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Pain Elemental without Lost Soul's limit", &unlimited_lost_souls)),

        NULL);
    }

    /* Русский язык */
    else
    {
        window = TXT_NewWindow("„ополнительные параметры игры");

        TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

        TXT_AddWidgets(window,
        TXT_If(gamemission == doom,    TXT_NewSeparator("ѓрафика")),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("ђазноцветная кровь и трупы монстров",       &colored_blood)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Љорректная формула \"Ouch face\"",          &new_ouch_face)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Ќеуязвимость окрашивает небо",              &invul_sky)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Љрасная вспышка воскрешения монстров",      &red_resurrection_flash)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("„вуствольное ружье может разрывать врагов", &ssg_blast_enemies)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("ћффект прозрачности у некоторых объектов",  &translucency)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Ќе мигать экраном при получении предметов", &no_pickup_flash)),

        TXT_If(gamemission == doom,    TXT_NewSeparator("‡вук")), 
            TXT_If(gamemission == doom,	TXT_NewCheckBox("‡вук раздавливания трупов",             &crushed_corpses_sfx)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Ћдиночный звук закрытия быстрой двери", &blazing_door_fix_sfx)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Џроигрывать звук при выходе из игры",   &play_exit_sfx)),

        TXT_If(gamemission == doom,    TXT_NewSeparator("ѓеймплей")),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("“ведомление об обнаружении тайников",   &secret_notification)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("Ћбщее время на межмиссионном экране",   &show_total_time)),
            TXT_If(gamemission == doom,	TXT_NewCheckBox("ћлементаль Ѓоли без ограничения душ",   &unlimited_lost_souls)),

        NULL);
    }
}

void BindCompatibilityVariables(void)
{
    /*
    M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    */

    // [JN] Дополнительные параметры геймплея

    // - Графика -
    M_BindIntVariable("colored_blood",          &colored_blood);            // Кровь разных цветов
    M_BindIntVariable("new_ouch_face",          &new_ouch_face);            // Корректная формула "Ouch face"
    M_BindIntVariable("invul_sky",              &invul_sky);                // Неуязвимость окрашивает небо
    M_BindIntVariable("red_resurrection_flash", &red_resurrection_flash);   // Красная вспышка воскрешения монстров
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);        // Двуствольное ружье может разрывать врагов
    M_BindIntVariable("translucency",           &translucency);             // Прозрачность объектов
    M_BindIntVariable("no_pickup_flash",        &no_pickup_flash);          // Не мигать экраном при получении предметов
    // - Звук -
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);      // Звук раздавливания трупов
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);     // Одиночный звук закрытия быстрой двери
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);            // Проигрывать звук при выходе из игры
    // - Геймплей -
    M_BindIntVariable("secret_notification",    &secret_notification);      // Уведомление об обнаружении секрета
    M_BindIntVariable("show_total_time",        &show_total_time);          // Показывать общее время
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);     // Элементаль боли без ограничения
}


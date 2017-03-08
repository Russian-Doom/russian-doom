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

#ifndef SETUP_COMPATIBILITY_H
#define SETUP_COMPATIBILITY_H

void CompatibilitySettings(void);
void BindCompatibilityVariables(void);

// extern int vanilla_savegame_limit;
// extern int vanilla_demo_limit;

// [JN] Дополнительные параметры игры

// - Графика -
extern int colored_blood;           // Кровь разных цветов
extern int new_ouch_face;           // Корректная формула "Ouch face"
extern int invul_sky;               // Неуязвимость окрашивает небо
extern int red_resurrection_flash;  // Красная вспышка воскрешения монстров
extern int ssg_blast_enemies;       // Двуствольное ружье может разрывать врагов
extern int translucency;            // Прозрачность объектов
extern int no_pickup_flash;         // Не мигать экраном при получении предметов
// - Звук -
extern int crushed_corpses_sfx;     // Звук раздавливания трупов
extern int blazing_door_fix_sfx;    // Одиночный звук закрытия быстрой двери
extern int play_exit_sfx;           // Проигрывать звук при выходе из игры
// - Геймплей -
extern int secret_notification;     // Уведомление об обнаружении секрета
extern int show_total_time;         // Показывать общее время
extern int unlimited_lost_souls;    // Элементаль боли без ограничения


#endif /* #ifndef SETUP_COMPATIBILITY_H */


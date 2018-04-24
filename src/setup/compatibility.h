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


#ifndef SETUP_COMPATIBILITY_H
#define SETUP_COMPATIBILITY_H

void CompatibilitySettings(void);
void BindCompatibilityVariables(void);

// extern int vanilla_savegame_limit;
// extern int vanilla_demo_limit;

// [JN] Дополнительные параметры игры

// - Интерфейс -
extern int draw_shadowed_text;      // Элементы меню и тексты отбрасывают тень
extern int fast_quickload;          // Не выводить запрос при быстрой загрузке
extern int show_total_time;         // Показывать общее время
extern int show_diskicon;           // Значок дискеты / [Strife] песочных часов
// extern int show_exit_sequence;      // [Strife] Показывать заставку при выходе
// - Графика -
extern int brightmaps;              // Брайтмаппинг текстур и спрайтов
extern int fake_contrast;           // Имитация контрастного освещения стен
extern int translucency;            // Прозрачность объектов
extern int infragreen_visor;        // Инфразеленый визор усиления освещения
extern int floating_powerups;       // Левитирующие сферы-артефакты
extern int swirling_liquids;        // Улучшенная анимация жидкостей
extern int randomly_flipcorpses;    // Произвольное зеркальное отражение трупов
extern int colored_blood;           // Кровь разных цветов
extern int invul_sky;               // Неуязвимость окрашивает небо
extern int red_resurrection_flash;  // Красная вспышка воскрешения монстров
// - Звук -
extern int crushed_corpses_sfx;     // Звук раздавливания трупов
extern int blazing_door_fix_sfx;    // Одиночный звук закрытия быстрой двери
extern int play_exit_sfx;           // Проигрывать звук при выходе из игры
extern int correct_endlevel_sfx;    // Корректный звук завершения уровня
// - Геймплей -
extern int fix_map_errors;          // Исправлять ошибки оригинальных уровней
extern int automap_stats;           // Отображать статистику уровня на карте
extern int secret_notification;     // Уведомление об обнаружении секрета
extern int weapon_bobbing;          // Покачивание оружия при стрельбе в движении
extern int new_ouch_face;           // Корректная формула "Ouch face"
extern int ssg_blast_enemies;       // Двуствольное ружье может разрывать врагов
extern int unlimited_lost_souls;    // Элементаль боли без ограничения душ
extern int agressive_lost_souls;    // Повышенная агрессивность Потерянных душ
extern int negative_health;         // Отображать отрицательное здоровье
extern int flip_levels;             // Зеркальное отражение уровней


#endif /* #ifndef SETUP_COMPATIBILITY_H */


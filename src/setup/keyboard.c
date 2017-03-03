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

// =--------------------------------------------------------------=
// Copyright(C) 2016-2017 Julian Nechaevsky
//
// ќписание:
// * ѕеревод строчек Setup.exe
// =--------------------------------------------------------------=

#include "textscreen.h"
#include "doomtype.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"

#include "execute.h"
#include "txt_keyinput.h"

#include "mode.h"
#include "joystick.h"
#include "keyboard.h"

#define WINDOW_HELP_URL "http://jnechaevsky.users.sourceforge.net/projects/rusdoom/setup/keyboard.html"

int vanilla_keyboard_mapping = 1;

static int always_run = 0;

// Keys within these groups cannot have the same value.

static int *controls[] = { &key_left, &key_right, &key_up, &key_down,
                           &key_toggleautorun,
                           &key_strafeleft, &key_straferight, &key_fire,
                           &key_use, &key_strafe, &key_speed, &key_jump,
                           &key_flyup, &key_flydown, &key_flycenter,
                           &key_lookup, &key_lookdown, &key_lookcenter,
                           &key_invleft, &key_invright, &key_invquery,
                           &key_invuse, &key_invpop, &key_mission, &key_invkey,
                           &key_invhome, &key_invend, &key_invdrop,
                           &key_useartifact, &key_pause, &key_usehealth,
                           &key_weapon1, &key_weapon2, &key_weapon3,
                           &key_weapon4, &key_weapon5, &key_weapon6,
                           &key_weapon7, &key_weapon8,
                           &key_arti_all, &key_arti_health, &key_arti_poisonbag,
                           &key_arti_blastradius, &key_arti_teleport,
                           &key_arti_teleportother, &key_arti_egg,
                           &key_arti_invulnerability,
                           &key_prevweapon, &key_nextweapon, NULL };

static int *menu_nav[] = { &key_menu_activate, &key_menu_up, &key_menu_down,
                           &key_menu_left, &key_menu_right, &key_menu_back,
                           &key_menu_forward, NULL };

static int *shortcuts[] = { &key_menu_help, &key_menu_save, &key_menu_load,
                            &key_menu_volume, &key_menu_detail, &key_menu_qsave,
                            &key_menu_endgame, &key_menu_messages, &key_spy,
                            &key_menu_qload, &key_menu_quit, &key_menu_gamma,
                            &key_menu_incscreen, &key_menu_decscreen, 
                            &key_menu_screenshot,
                            &key_message_refresh, &key_multi_msg,
                            &key_multi_msgplayer[0], &key_multi_msgplayer[1],
                            &key_multi_msgplayer[2], &key_multi_msgplayer[3] };

static int *map_keys[] = { &key_map_north, &key_map_south, &key_map_east,
                           &key_map_west, &key_map_zoomin, &key_map_zoomout,
                           &key_map_toggle, &key_map_maxzoom, &key_map_follow,
                           &key_map_grid, &key_map_mark, &key_map_clearmark,
                           NULL };

static void UpdateJoybSpeed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(var))
{
    if (always_run)
    {
        /*
         <Janizdreg> if you want to pick one for chocolate doom to use, 
                     pick 29, since that is the most universal one that 
                     also works with heretic, hexen and strife =P

         NB. This choice also works with original, ultimate and final exes.
        */

        joybspeed = 29;
    }
    else
    {
        joybspeed = 0;
    }
}

static int VarInGroup(int *variable, int **group)
{
    unsigned int i;

    for (i=0; group[i] != NULL; ++i)
    {
        if (group[i] == variable)
        {
            return 1;
        }
    }

    return 0;
}

static void CheckKeyGroup(int *variable, int **group)
{
    unsigned int i;

    // Don't check unless the variable is in this group.

    if (!VarInGroup(variable, group))
    {
        return;
    }

    // If another variable has the same value as the new value, reset it.

    for (i=0; group[i] != NULL; ++i)
    {
        if (*variable == *group[i] && group[i] != variable)
        {
            // A different key has the same value.  Clear the existing
            // value. This ensures that no two keys can have the same
            // value.

            *group[i] = 0;
        }
    }
}

// Callback invoked when a key control is set

static void KeySetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);

    CheckKeyGroup(variable, controls);
    CheckKeyGroup(variable, menu_nav);
    CheckKeyGroup(variable, shortcuts);
    CheckKeyGroup(variable, map_keys);
}

// Add a label and keyboard input to the specified table.

static void AddKeyControl(TXT_UNCAST_ARG(table), char *name, int *var)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_key_input_t *key_input;

    TXT_AddWidget(table, TXT_NewLabel(name));
    key_input = TXT_NewKeyInput(var);
    TXT_AddWidget(table, key_input);

    TXT_SignalConnect(key_input, "set", KeySetCallback, var);
}

static void AddSectionLabel(TXT_UNCAST_ARG(table), char *title,
                            boolean add_space)
{
    TXT_CAST_ARG(txt_table_t, table);
    char buf[64];

    if (add_space)
    {
        TXT_AddWidgets(table,
                       TXT_NewStrut(0, 1),
                       TXT_TABLE_EOL,
                       NULL);
    }

    M_snprintf(buf, sizeof(buf), " - %s - ", title);

    TXT_AddWidgets(table,
                   TXT_NewLabel(buf),
                   TXT_TABLE_EOL,
                   NULL);
}
static void ConfigExtraKeys(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_scrollpane_t *scrollpane;
    txt_table_t *table;
    boolean extra_keys = gamemission == heretic
                      || gamemission == hexen
                      || gamemission == strife;

	// "Extra keyboard controls"
    window = TXT_NewWindow("Дополнительное управление");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 21, 9);

    if (extra_keys)
    {
        // When we have extra controls, a scrollable pane must be used.

        scrollpane = TXT_NewScrollPane(0, 13, table);
        TXT_AddWidget(window, scrollpane);

		// "View"
        AddSectionLabel(table, "Взгл€д", false);

		// "Look up"
        AddKeyControl(table, "Смотреть вверх", &key_lookup);
		// "Look down"
        AddKeyControl(table, "Смотреть вниз", &key_lookdown);
		// "Center view"
        AddKeyControl(table, "Центрировать", &key_lookcenter);

        if (gamemission == heretic || gamemission == hexen)
        {
			// "Flying"
            AddSectionLabel(table, "Полет", true);

			// "Fly up"
            AddKeyControl(table, "Лететь вверх", &key_flyup);
			// "Fly down"
            AddKeyControl(table, "Лететь вниз", &key_flydown);
			// "Fly center"
            AddKeyControl(table, "Центрировать", &key_flycenter);
        }

		// "Inventory"
        AddSectionLabel(table, "Инвентарь", true);

		// "Inventory left"
        AddKeyControl(table, "Пролистать влево", &key_invleft);
		// "Inventory right"
        AddKeyControl(table, "Пролистать вправо", &key_invright);

		// [JN] Strife у нас не поддержимаетс€, пропускаем.
        if (gamemission == strife)
        {
            AddKeyControl(table, "Home", &key_invhome);
            AddKeyControl(table, "End", &key_invend);
            AddKeyControl(table, "Query", &key_invquery);
            AddKeyControl(table, "Drop", &key_invdrop);
            AddKeyControl(table, "Show weapons", &key_invpop);
            AddKeyControl(table, "Show mission", &key_mission);
            AddKeyControl(table, "Show keys", &key_invkey);
            AddKeyControl(table, "Use", &key_invuse);
            AddKeyControl(table, "Use health", &key_usehealth);
        }
        else
        {
			// "Use artifact"
            AddKeyControl(table, "Использовать артефакт ", &key_useartifact);
        }

        if (gamemission == hexen)
        {
			// "Artifacts"
            AddSectionLabel(table, "Артефакты", true);

			// "One of each"
            AddKeyControl(table, "Все однократно", &key_arti_all);
			// "Quartz Flask"
            AddKeyControl(table, "Кварцевый флакон", &key_arti_health);
			// "Flechette"
            AddKeyControl(table, "Зелье", &key_arti_poisonbag);
			// "Disc of Repulsion"
            AddKeyControl(table, "Диск отторжени€", &key_arti_blastradius);
			// "Chaos Device"
            AddKeyControl(table, "Юмблема Хаоса", &key_arti_teleport);
			// Banishment Device
            AddKeyControl(table, "Юмблема изгнани€", &key_arti_teleportother);
			// "Porkalator"
            AddKeyControl(table, "Свиноморфер", &key_arti_egg);
			// "Icon of the Defender"
            AddKeyControl(table, "Символ защитника",
                          &key_arti_invulnerability);
        }
    }
    else
    {
        TXT_AddWidget(window, table);
    }

    if (gamemission == doom)
    {
        AddSectionLabel(table, "Движение", false);
        AddKeyControl(table, "Режим посто€нного бега ", &key_toggleautorun);
    }
    
	// "Weapons"
    AddSectionLabel(table, "Оружие", extra_keys);

	// "Weapon N"
    AddKeyControl(table, "Оружие 1", &key_weapon1);
    AddKeyControl(table, "Оружие 2", &key_weapon2);
    AddKeyControl(table, "Оружие 3", &key_weapon3);
    AddKeyControl(table, "Оружие 4", &key_weapon4);
    AddKeyControl(table, "Оружие 5", &key_weapon5);
    AddKeyControl(table, "Оружие 6", &key_weapon6);
    AddKeyControl(table, "Оружие 7", &key_weapon7);
    AddKeyControl(table, "Оружие 8", &key_weapon8);
	// "Previous weapon"
    AddKeyControl(table, "Предыдущее оружие", &key_prevweapon);
	// "Next weapon"
    AddKeyControl(table, "Следующее оружие", &key_nextweapon);
}

static void OtherKeysDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_table_t *table;
    txt_scrollpane_t *scrollpane;

	// "Other keys"
    window = TXT_NewWindow("Другие клавиши");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 9);

	// "Menu navigation"
    AddSectionLabel(table, "Навигаци€ в меню", false);

	// "Activate menu"
    AddKeyControl(table, "Активировать меню",		&key_menu_activate);
	// "Move cursor up"
    AddKeyControl(table, "Курсор вверх",			&key_menu_up);
	// "Move cursor down"
    AddKeyControl(table, "Курсор вниз",				&key_menu_down);
	// "Move slider left"
    AddKeyControl(table, "Ползунок влево",			&key_menu_left);
	// "Move slider right"
    AddKeyControl(table, "Ползунок вправо",			&key_menu_right);
	// "Go to previous menu"
    AddKeyControl(table, "Предыдущий экран",		&key_menu_back);
	// "Activate menu item"
    AddKeyControl(table, "Активировать пункт меню",	&key_menu_forward);
	// "Confirm action"
    AddKeyControl(table, "Подтвердить действие",	&key_menu_confirm);
	// "Cancel action"
    AddKeyControl(table, "Отменить действие",		&key_menu_abort);

	// "Shortcut keys"
    AddSectionLabel(table, "Клавиши быстрого доступа", true);

	// "Pause game"
    AddKeyControl(table, "Пауза",					&key_pause);
	// "Help screen"
    AddKeyControl(table, "Помощь",					&key_menu_help);
	// "Save game"
    AddKeyControl(table, "Сохранение",				&key_menu_save);
	// "Load game"
    AddKeyControl(table, "Загрузка",				&key_menu_load);
	// "Sound volume"
    AddKeyControl(table, "Громкость",				&key_menu_volume);
	// "Toggle detail"
    AddKeyControl(table, "Детализаци€",				&key_menu_detail);
	// "Quick save"
    AddKeyControl(table, "Быстрое сохранение",		&key_menu_qsave);
	// "End game"
    AddKeyControl(table, "Закончить игру",			&key_menu_endgame);
	// "Toggle messages"
    AddKeyControl(table, "Сообщени€",				&key_menu_messages);
	// "Quick load"
    AddKeyControl(table, "Быстра€ загрузка",		&key_menu_qload);
	// "Quit game"
    AddKeyControl(table, "Выход из игры",			&key_menu_quit);
	// "Toggle gamma"
    AddKeyControl(table, "Гамма-коррекци€",			&key_menu_gamma);
	// "Multiplayer spy"
    AddKeyControl(table, "Вид другого игрока",		&key_spy);

	
	// Increase screen size
    AddKeyControl(table, "Увеличить экран",			&key_menu_incscreen);
	// Decrease screen size
    AddKeyControl(table, "Уменьшить экран",			&key_menu_decscreen);
	// Save a screenshot
    AddKeyControl(table, "Скриншот",				&key_menu_screenshot);

	// "Display last message"
    AddKeyControl(table, "Показать последнее сообщение ",	&key_message_refresh);
	// "Finish recording demo"
    AddKeyControl(table, "Закончить запись демо",			&key_demo_quit);

	// "Map"
    AddSectionLabel(table, "Карта", true);
	// "Toggle map"
    AddKeyControl(table, "Открыть карту",            &key_map_toggle);
	// "Zoom in"
    AddKeyControl(table, "Приблизить",               &key_map_zoomin);
	// "Zoom out"
    AddKeyControl(table, "Отдалить",              &key_map_zoomout);
	// "Maximum zoom out"
    AddKeyControl(table, "Максимальное отдаление",      &key_map_maxzoom);
	// "Follow mode"
    AddKeyControl(table, "Режим следовани€",           &key_map_follow);
	// "Pan north"
    AddKeyControl(table, "Прокрутить вверх",             &key_map_north);
	// "Pan south"
    AddKeyControl(table, "Прокрутить вниз",             &key_map_south);
	// "Pan east"
    AddKeyControl(table, "Прокрутить вправо",              &key_map_east);
	// "Pan west"
    AddKeyControl(table, "Прокрутить влево",              &key_map_west);
	// "Toggle grid"
    AddKeyControl(table, "Отобразить сетку",           &key_map_grid);
	// "Mark location"
    AddKeyControl(table, "Поставить отметку",         &key_map_mark);
	// "Clear all marks"
    AddKeyControl(table, "Убрать отметки",       &key_map_clearmark);

	// "Multiplayer"
    AddSectionLabel(table, "Сетева€ игра", true);

	// "Send message"
    AddKeyControl(table, "Отправить сообщение",          &key_multi_msg);
	// "- to player N"
    AddKeyControl(table, "- игроку 1",         &key_multi_msgplayer[0]);
    AddKeyControl(table, "- игроку 2",         &key_multi_msgplayer[1]);
    AddKeyControl(table, "- игроку 3",         &key_multi_msgplayer[2]);
    AddKeyControl(table, "- игроку 4",         &key_multi_msgplayer[3]);

    if (gamemission == hexen || gamemission == strife)
    {
        AddKeyControl(table, "- игроку 5",     &key_multi_msgplayer[4]);
        AddKeyControl(table, "- игроку 6",     &key_multi_msgplayer[5]);
        AddKeyControl(table, "- игроку 7",     &key_multi_msgplayer[6]);
        AddKeyControl(table, "- игроку 8",     &key_multi_msgplayer[7]);
    }

    scrollpane = TXT_NewScrollPane(0, 13, table);

    TXT_AddWidget(window, scrollpane);
}

void ConfigKeyboard(void)
{
    txt_window_t *window;
    txt_checkbox_t *run_control;

    always_run = joybspeed >= 20;

	// "Keyboard configuration"
    window = TXT_NewWindow("Настройки клавиатуры");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    // The window is on a 5-column grid layout that looks like:
    // Label | Control | | Label | Control
    // There is a small gap between the two conceptual "columns" of
    // controls, just for spacing.
    TXT_SetTableColumns(window, 5);
    TXT_SetColumnWidths(window, 16, 8, 2, 16, 8);

	// "Movement"
    TXT_AddWidget(window, TXT_NewSeparator("Движение"));
	// "Move Forward"
    AddKeyControl(window, "Движение вперед  ", &key_up);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
	// "Strafe Left"
    AddKeyControl(window, " Боком влево", &key_strafeleft);

	// "Move Backward"
    AddKeyControl(window, "Движение назад ", &key_down);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
	// "Strafe Right"
    AddKeyControl(window, " Боком вправо", &key_straferight);

	// "Turn Left"
    AddKeyControl(window, "Поворот налево ", &key_left);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
	// "Speed On"
    AddKeyControl(window, " Бег", &key_speed);

	// "Turn Right"
    AddKeyControl(window, "Поворот направо ", &key_right);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
	// "Strafe On"
    AddKeyControl(window, " Движение боком ", &key_strafe);

    if (gamemission == hexen || gamemission == strife)
    {
		// "Jump"
        AddKeyControl(window, "Прыжок", &key_jump);
    }

	// "Action"
    TXT_AddWidget(window, TXT_NewSeparator("Действи€"));
	// "Fire/Attack"
    AddKeyControl(window, "Атака/стрельба  ", &key_fire);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
	// "Use"
    AddKeyControl(window, " Использовать   ", &key_use);

    TXT_AddWidgets(window,
				   // "More controls..."
                   TXT_NewButton2("Дополнительно...", ConfigExtraKeys, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
				   // "Other keys..."
                   TXT_NewButton2(" Другие клавиши...", OtherKeysDialog, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,

				   // "Misc."
                   TXT_NewSeparator("Дополнительно"),
				   // "Always run"
                   run_control = TXT_NewCheckBox("Режим посто€нного бега", &always_run),
                   TXT_TABLE_EOL,
				   // "Use native keyboard mapping"
                   TXT_NewInvertedCheckBox("Использовать нативную раскладку",
                                           &vanilla_keyboard_mapping),
                   TXT_TABLE_EOL,
                   NULL);

    TXT_SignalConnect(run_control, "changed", UpdateJoybSpeed, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

void BindKeyboardVariables(void)
{
    M_BindIntVariable("vanilla_keyboard_mapping", &vanilla_keyboard_mapping);
}

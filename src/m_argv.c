//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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
//



#include <ctype.h>
#include "rd_io.h"
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "d_name.h"
#include "i_system.h"
#include "m_misc.h"
#include "m_argv.h"  // haleyjd 20110212: warning fix
#include "jn.h"

int		myargc;
char**		myargv;




//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
//

int M_CheckParmWithArgs(char *check, int num_args)
{
    int i;

    for (i = 1; i < myargc - num_args; i++)
    {
	if (!strcasecmp(check, myargv[i]))
	    return i;
    }

    return 0;
}

//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//

boolean M_ParmExists(char *check)
{
    return M_CheckParm(check) != 0;
}

int M_CheckParm(char *check)
{
    return M_CheckParmWithArgs(check, 0);
}

#define MAXARGVS        100

static void LoadResponseFile(int argv_index, const char *filename)
{
    FILE *handle;
    int size;
    char *infile;
    char *file;
    char **newargv;
    int newargc;
    int i, k;

    // Read the response file into memory
    handle = fopen(filename, "rb");

    if (handle == NULL)
    {
        printf (english_language ?
                "\nNo such response file!" :
                "\nОтветный файл не обнаружен!");
        exit(1);
    }

    printf(english_language ?
           "Found response file %s!\n" :
           "Ответный файл %s обнаружен!\n",
           filename);

    size = M_FileLength(handle);

    // Read in the entire file
    // Allocate one byte extra - this is in case there is an argument
    // at the end of the response file, in which case a '\0' will be
    // needed.

    file = malloc(size + 1);

    i = 0;

    while (i < size)
    {
        k = fread(file + i, 1, size - i, handle);

        if (k < 0)
        {
            I_Error(english_language ?
                    "Failed to read full contents of '%s'" :
                    "Невозможно прочитать содержимое \"%s\"",
                    filename);
        }

        i += k;
    }

    fclose(handle);

    // Create new arguments list array

    newargv = malloc(sizeof(char *) * MAXARGVS);
    newargc = 0;
    memset(newargv, 0, sizeof(char *) * MAXARGVS);

    // Copy all the arguments in the list up to the response file

    for (i=0; i<argv_index; ++i)
    {
        newargv[i] = myargv[i];
        myargv[i] = NULL;
        ++newargc;
    }

    infile = file;
    k = 0;

    while(k < size)
    {
        // Skip past space characters to the next argument

        while(k < size && isspace(infile[k]))
        {
            ++k;
        }

        if (k >= size)
        {
            break;
        }

        // If the next argument is enclosed in quote marks, treat
        // the contents as a single argument.  This allows long filenames
        // to be specified.

        if (infile[k] == '\"')
        {
            char *argstart;
            // Skip the first character(")
            ++k;

            argstart = &infile[k];

            // Read all characters between quotes

            while (k < size && infile[k] != '\"' && infile[k] != '\n')
            {
                ++k;
            }

            if (k >= size || infile[k] == '\n')
            {
                I_Error(english_language ?
                        "Quotes are not closed in the response file \"%s\"" :
                        "Кавычки не закрыты в ответном файле \"%s\"",
                        filename);
            }

            // Cut off the string at the closing quote

            infile[k] = '\0';
            ++k;
            newargv[newargc++] = M_StringDuplicate(argstart);
        }
        else
        {
            char *argstart;
            // Read in the next argument until a space is reached

            argstart = &infile[k];

            while(k < size && !isspace(infile[k]))
            {
                ++k;
            }

            // Cut off the end of the argument at the first space

            infile[k] = '\0';

            ++k;
            newargv[newargc++] = M_StringDuplicate(argstart);
        }
    }

    // Add arguments following the response file argument

    for (i=argv_index + 1; i<myargc; ++i)
    {
        newargv[newargc] = myargv[i];
        ++newargc;
    }

    // Free any old strings in myargv which were not moved to newargv
    for (i = 0; i < myargc; ++i)
    {
        if (myargv[i] != NULL)
        {
            free(myargv[i]);
            myargv[i] = NULL;
        }
    }

    free(myargv);
    myargv = newargv;
    myargc = newargc;

#if 0
    // Disabled - Vanilla Doom does not do this.
    // Display arguments

    printf("%d command-line args:\n", myargc);

    for (k=1; k<myargc; k++)
    {
        printf("'%s'\n", myargv[k]);
    }
#endif
}

//
// Find a Response File
//

void M_FindResponseFile(void)
{
    int i;

    for (i = 1; i < myargc; i++)
    {
        if (myargv[i][0] == '@')
        {
            LoadResponseFile(i, myargv[i] + 1);
        }
    }

    for (;;)
    {
        //!
        // @arg filename
        //
        // Load extra command line arguments from the given response file.
        // Arguments read from the file will be inserted into the command
        // line replacing this argument. A response file can also be loaded
        // using the abbreviated syntax '@filename.rsp'.
        //
        i = M_CheckParmWithArgs("-response", 1);
        if (i <= 0)
        {
            break;
        }
        // Replace the -response argument so that the next time through
        // the loop we'll ignore it. Since some parameters stop reading when
        // an argument beginning with a '-' is encountered, we keep something
        // that starts with a '-'.
        free(myargv[i]);
        myargv[i] = M_StringDuplicate("-_");
        LoadResponseFile(i + 1, myargv[i + 1]);
    }
}

// Return the name of the executable used to start the program:

char *M_GetExecutableName(void)
{
    char *sep;

    sep = strrchr(myargv[0], DIR_SEPARATOR);

    if (sep == NULL)
    {
        return myargv[0];
    }
    else
    {
        return sep + 1;
    }
}

char* exedir = NULL;
#ifdef __APPLE__
char* packageResourcesDir = NULL;
#endif

void M_PrintHelp(void)
{
/**
 * Each argument of the CLI_Parameter macros MUST be on a separate line.
 * Double quotes (") MUST NOT be used inside strings.
 */
#define CLI_Parameter(keys, description_eng, description_rus) \
printf("  %-34s  %s\n", (keys), english_language ? (description_eng) : (description_rus))

    const char* const game = RD_GameType == gt_Doom ? "doom" :
                             RD_GameType == gt_Heretic ? "heretic" :
                             RD_GameType == gt_Hexen ? "hexen" : "strife";

    printf("%s '%s%s --help -lang %s'\n\n",
           english_language ? "Чтобы увидеть это сообщение на русском запустите" : "To see this message in english run",
           PROGRAM_PREFIX,
           game,
           english_language ? "ru" : "en");

    printf("%s:\t%s%s [%s]\n%s:\n",
           english_language ? "Usage" : "Использование",
           PROGRAM_PREFIX,
           game,
           english_language ? "Options" : "параметры",
           english_language ? "Options" : "Параметры");

    CLI_Parameter("-h, --help",
                  "Show this help message",
                  "Показать это сообщение");
    CLI_Parameter("    --Version",
                  "Show version information",
                  "Показать информацию о версии");
    CLI_Parameter("-lang <lang>",
                  "Force specified language before config file is loaded and ignoring the value from it. Language can be changed later in the Options menu. Valid values are: 'en', 'ru'",
                  "Принудительно установить заданный язык до загрузки конфигурационного файла и игнорируя значение из него. Язык позже можно будет изменить в меню Настроек. Поддерживаемые значения: 'en', 'ru'");
    CLI_Parameter("-iwad <path>",
                  "Specify an IWAD file. <path> can be absolute or relative to one of the WAD search locations",
                  "Задать IWAD файл. <path> может быть абсолютным или относительным одному из мест поиска WAD'ов");
    CLI_Parameter("-file <path> ...",
                  "Load the given PWAD(s). <path> can be absolute or relative to one of the WAD search locations",
                  "Загрузить указанные PWAD'ы. <path> может быть абсолютным или относительным одному из мест поиска WAD'ов");
    if(RD_GameType != gt_Hexen)
    {
        CLI_Parameter("-deh <path> ...",
                      "Load the given dehacked patch(es). <path> can be absolute or relative to one of the WAD search locations",
                      "Загрузить указанные патчи dehacked. <path> может быть абсолютным или относительным одному из мест поиска WAD'ов");
        CLI_Parameter("-nocheats",
                      "Ignore cheats in dehacked files",
                      "Игнорировать читы в dehacked файлах");
    }
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-nodeh, -nodehlump",
                      "Don't load DEHACKED lumps from WADs",
                      "Не загружать DEHACKED лампы из вадов");
    }
    CLI_Parameter("-nwtmerge <path> ...",
                  "Simulates the behavior of NWT's -merge option. Multiple files may be specified",
                  "Эмулировать поведение опции -merge из NWT. Можно указать несколько файлов");
    CLI_Parameter("-af <path> ...",
                  "Simulates the behavior of NWT's -af option, merging flats into the main IWAD. Multiple files may be specified",
                  "Эмулировать поведение опции -af из NWT, объединяя текстуры полов с главным IWAD'ом. Можно указать несколько файлов");
    CLI_Parameter("-as <path> ...",
                  "Simulates the behavior of NWT's -as option, merging sprites into the main IWAD. Multiple files may be specified",
                  "Эмулировать поведение опции -as из NWT, объединяя спрайты с главным IWAD'ом. Можно указать несколько файлов");
    CLI_Parameter("-aa <path> ...",
                  "Equivalent to '-af <paths> -as <paths>'",
                  "Эквивалент '-af <paths> -as <paths>'");
    if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-scripts <path>",
                      "Specify <path> to level scripts. Development option",
                      "Задать путь к скриптам уровней. Опция для разработки");
    }
    CLI_Parameter("-response <path>",
                  "Load extra command line arguments from the given response file",
                  "Загрузить дополнительные параметры командной строки из данного файла ответов");
    CLI_Parameter("-gus_patches <dir>",
                  "Specify GUS patches location",
                  "Задать расположение патчей GUS");
    CLI_Parameter("-config <path>",
                  "Load configuration from the specified file, instead of the default",
                  "Загрузить конфигурацию из указанного файла вместо стандартного");
    CLI_Parameter("-savedir <dir>",
                  "Specify directory for savegames",
                  "Задать директорию для сохранений");
#ifdef _WIN32
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-cdrom",
                      "Save configuration data and savegames in C:\\doomdata, allowing play from CD",
                      "Сохранять конфигурацию и сохранённые игры в C:\\doomdata, позволяя играть с CD");
    }
    else if(RD_GameType == gt_Heretic)
    {
        CLI_Parameter("-cdrom",
                      "Save configuration data and savegames in C:\\heretic.cd, allowing play from CD",
                      "Сохранять конфигурацию и сохранённые игры в C:\\heretic.cd, позволяя играть с CD");
    }
    else if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-cdrom",
                      "Save configuration data and savegames in C:\\hexndata, allowing play from CD",
                      "Сохранять конфигурацию и сохранённые игры в C:\\hexndata, позволяя играть с CD");
    }
    else if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-cdrom",
                      "Save configuration data and savegames in C:\\strife.cd, allowing play from CD",
                      "Сохранять конфигурацию и сохранённые игры в C:\\strife.cd, позволяя играть с CD");
    }
#endif


// Game start and gameplay
    CLI_Parameter("-loadgame <slot>",
                  "Load the game in savegame <slot>",
                  "Загрузить сохранённую игру из слота <slot>");
    CLI_Parameter("-skill <skill>",
                  "Set the game skill, 1-6 (1: easiest, 6: hardest). A skill of 0 disables all monsters",
                  "Задать уровень сложности, 1-6 (1: самый лёгкий, 6: самый сложный). Сложность 0 отключает всех монстров");
    if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-class <n>",
                      "Specify player class: 0 = fighter, 1 = cleric, 2 = mage, 3 = pig",
                      "Задать класс игрока: 0 = воин, 1 = клирик, 2 = маг, 3 = свинья");
    }
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-map, -warp {<x> [<y>] | E<x>M<y>}",
                      "Start a game immediately, warping to level E<x>M<y> (Doom 1) (default y = 1)",
                      "Начать игру сразу же, переместившись на уровень E<x>M<y> (Doom 1) (по умолчанию y = 1)");
        CLI_Parameter("-map, -warp {<x> | MAP<x>}",
                      "Start a game immediately, warping to level MAP<x> (Doom 2)",
                      "Начать игру сразу же, переместившись на уровень MAP<x> (Doom 2)");
    }
    else if(RD_GameType == gt_Heretic)
    {
        CLI_Parameter("-map, -warp {<x> <y> | E<x>M<y>}",
                      "Start a game immediately, warping to level E<x>M<y>",
                      "Начать игру сразу же, переместившись на уровень E<x>M<y>");
    }
    else if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-map, -warp [<x>]",
                      "Start a game immediately, warping to level with hxvisit <x> (default '1')",
                      "Начать игру сразу же, переместившись на уровень с hxvisit <x> (по умолчанию '1')");
        CLI_Parameter("-map, -warp MAP<x>",
                      "Start a game immediately, warping to level with map number <x>",
                      "Начать игру сразу же, переместившись на уровень с номером карты <x>");
    }
    else if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-map, -warp <x>",
                      "Start a game immediately, warping to level <x>",
                      "Начать игру сразу же, переместившись на уровень <x>");
    }
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-episode <n>",
                      "Start playing on episode <n> (1-3, 1-4 for 'The Ultimate Doom', episode 5 available if non-compat version of SIGIL.WAD is loaded)",
                      "Начать игру с эпизода <n> (1-3, 1-4 для 'The Ultimate Doom', эпизод 5 доступен если загружена не compat версия SIGIL.WAD");
    }
    else if(RD_GameType == gt_Heretic)
    {
        CLI_Parameter("-episode <n>",
                      "Start playing on episode <n> (1-3, 1-5 for 'Shadow of the Serpent Riders')",
                      "Начать игру с эпизода <n> (1-3, 1-5 для 'Shadow of the Serpent Riders')");
    }
    CLI_Parameter("-nomonsters",
                  "Disable monsters",
                  "Отключить монстров");
    CLI_Parameter("-respawn",
                  "Monsters respawn after being killed",
                  "Монстры возрождаются после того как были убиты");
    CLI_Parameter("-fast",
                  "Monsters move faster",
                  "Монстры движутся быстрее");
    if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-work",
                      "Set Rogue playtesting mode (godmode, noclip toggled by backspace)",
                      "Режим тестирования Rogue (Режим бога, прохождение сквозь стены переключается по backspace)");
        CLI_Parameter("-random",
                      "Items respawn at random locations",
                      "Предметы респавнятся в случайных местах");
    }
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-turbo [<x>]",
                      "Turbo mode. The player's speed is multiplied by [x]%%. If unspecified, <x> defaults to 200. Values are rounded up to 10 and down to 400",
                      "Турбо режим. Скорость игрока умножается на [x]%%. Если не указано, то <x> по умолчанию 200. Значения округляются от 10 и до 400");
    }
    CLI_Parameter("-vanilla",
                  "Activate vanilla gameplay mode. All optional enhancements will be disabled without modifying configuration files",
                  "Активировать режим оригинального геймплея. Все опциональные улучшения будут отключены без изменения конфигурационного файла");
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-beta",
                      "Emulate Press Beta Doom version",
                      "Эмулировать бета-версию для прессы");
        CLI_Parameter("-jaguar",
                      "Emulate Atari Jaguar Doom version",
                      "Эмулировать версию Doom для Atari Jaguar");
        CLI_Parameter("-pack <pack>",
                      "Explicitly specify a Doom II 'mission pack' to run as, instead of detecting it based on the file name. Valid values are: 'doom2', 'tnt' and 'plutonia'",
                      "Явно задать 'mission pack' для Doom II, вместо того чтобы определять его по имени файла. Поддерживаемые значения: 'doom2', 'tnt' и 'plutonia'");
        CLI_Parameter("-gameversion <version>",
                      "Emulate a specific version of Doom. Valid values are '1.2', '1.666', '1.7', '1.8', '1.9', 'ultimate', 'doomse', 'final', 'final2'",
                      "Эмулировать конкретную версию Doom. Поддерживаемые значения: '1.2', '1.666', '1.7', '1.8', '1.9', 'ultimate', 'doomse', 'final', 'final2'");
        CLI_Parameter("-complevel <n>",
                      "Toggle various bug fixes and enhanced features for maximum compatibility with other (related) source ports. Valid values are: '0' - Doom v1.2, '1' - Doom v1.666, '2' - Doom v1.9, '3' - Ultimate Doom, '4' - Final Doom",
                      "Включает различные баг фиксы и улучшения для максимальной совместимости с другими (связанными) портами. В Поддерживаемые значения: '0' - Doom v1.2, '1' - Doom v1.666, '2' - Doom v1.9, '3' - Ultimate Doom, '4' - Final Doom");
    }
    else if(RD_GameType == gt_Hexen)
    {
#ifdef WHEN_ITS_DONE
        CLI_Parameter("-psx",
                      "Emulate Playstation 1 Hexen version",
                      "Эмулировать версию Hexen для Playstation 1");
#endif
        CLI_Parameter("-v10override",
                      "Disable the check for the v1.0 IWAD file, even though it will almost certainly cause the game to crash",
                      "Отключить проверку на IWAD версии 1.0, даже с учётом того что это практически однозначно приведёт к падению игры");
    }
    else if(RD_GameType == gt_Heretic)
    {
        CLI_Parameter("-hhever <version>",
                      "Select the Heretic version number that was used to generate the HHE patch to be loaded. Patches for each of the Vanilla Heretic versions ('1.0', '1.2', '1.3') can be loaded, but the correct version number must be specified",
                      "Задать номер версии Heretic которая использовалась для генерации загружаемых HHE патчей. Патчи для каждой оригинальной версии Heretic ('1.0', '1.2', '1.3') могут быть загружены, но корректный номер версии должен быть указан");
    }
    else if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-gameversion <version>",
                      "Emulate a specific version of Strife. Valid values are '1.2' and '1.31'",
                      "Эмулировать конкретную версию Strife. Поддерживаемые значения: '1.2' и '1.31'");
    }


// System
    CLI_Parameter("-mb <size>",
                  "Specify the heap <size>, in MiB (default '16')",
                  "Задать размера кучи в MiB (по умолчанию '16')");
    CLI_Parameter("-setmem <version>",
                  "Specify DOS version to emulate for NULL pointer dereference emulation. Supported versions are: 'dos622', 'dos71', 'dosbox'. The default is to emulate DOS 7.1 (Windows 98)",
                  "Задать эмулируемую версию DOS для случая эмуляции разыменования нулевого указателя. Поддерживаемые значения: 'dos622', 'dos71', 'dosbox'. По умолчанию эмулируется DOS 7.1 (Windows 98)");
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-donut <x> <y>",
                      "Use the specified magic values when emulating behavior caused by memory overruns from improperly constructed donuts. In Vanilla Doom this can differ depending on the operating system. The default is to emulate the behavior when running under Windows 98",
                      "Использовать заданные магические числа при эмуляции поведения вызванного переполнениями памяти из-за неправильно сконструированных donuts. В оригинальном Doom оно может отличаться в зависимости от операционной системы. По умолчанию эмулируется поведение в Windows 98");
    }
    if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-spechit <n>",
                      "Use the specified magic value when emulating spechit overruns",
                      "Использовать указанное магическое число для эмуляции переполнения spechit");
    }
//    CLI_Parameter("-zonezero", // Deliberately undocumented
//                  "Zone memory debugging flag. If set, memory is zeroed after it is freed to deliberately break any code that attempts to use it after free",
//                  "Флаг отладки зональной памяти. Если установлен, память зануляется после освобождения чтобы намеренно сломать любой код который пытается использовать её после освобождения");
//    CLI_Parameter("-zonescan", // Deliberately undocumented
//                  "Zone memory debugging flag. If set, each time memory is freed, the zone heap is scanned to look for remaining pointers to the freed block",
//                  "Флаг отладки зональной памяти. Если установлен, каждый раз когда память освобождается, куча сканируется в поисках оставшихся указателей на освобождённый блок");
    if(RD_GameType != gt_Hexen)
    {
        CLI_Parameter("-reject_pad_with_ff",
                      "Pad remaining space of the REJECT lump with 0xff",
                      "Заполнить оставшееся пространство лампа REJECT с помощью 0xff");
    }
    if(RD_GameType != gt_Strife)
    {
        CLI_Parameter("-blockmap",
                      "(re-)create BLOCKMAP if necessary",
                      "(пере-)создать BLOCKMAP если необходимо");
    }
    CLI_Parameter("-mmap",
                  "Use the OS's virtual memory subsystem to map WAD files directly into memory",
                  "Использовать подсистему виртуальной памяти операционной системы для отображения WAD файлов непосредственно в память");


// Window and features
    CLI_Parameter("-fullscreen",
                  "Run in fullscreen mode",
                  "Полноэкранный режим");
    CLI_Parameter("-window, -nofullscreen",
                  "Run in a window",
                  "Оконный режим");
    CLI_Parameter("-width <W>",
                  "Specify the screen width, in pixels. Height is determent automatically. Implies -window",
                  "Задать ширину экрана в пикселах. Высота определяется автоматически. Подразумевает -window");
    CLI_Parameter("-height <H>",
                  "Specify the screen height, in pixels. Width is determent automatically. Implies -window",
                  "Задать высоту экрана в пикселах. Ширина определяется автоматически. Подразумевает -window");
    CLI_Parameter("-geometry <W>x<H>",
                  "Specify the dimensions of the window. Implies -window",
                  "Задать размеры экрана. Подразумевает -window");
    CLI_Parameter("-1",
                  "Don't scale up the screen. Implies -window",
                  "Не масштабировать экран. Подразумевает -window");
    CLI_Parameter("-2",
                  "Double up the screen to 2x its normal size. Implies -window",
                  "Увеличить экран в два раза относительно нормальных размеров. Подразумевает -window");
    CLI_Parameter("-3",
                  "Double up the screen to 3x its normal size. Implies -window",
                  "Увеличить экран в три раза относительно нормальных размеров. Подразумевает -window");
    CLI_Parameter("-devparm",
                  "Show console window and log more debug info",
                  "Показать окно консоли и логировать больше отладочной информации");
    CLI_Parameter("-nogui",
                  "Don't show error message dialog box",
                  "Не показывать окно сообщения об ошибке");
    CLI_Parameter("-nosound",
                  "Disable all sound output",
                  "Отключить вывод всего аудио");
    CLI_Parameter("-nosfx",
                  "Disable sound effects",
                  "Отключить звуковые эффекты");
    CLI_Parameter("-nomusic",
                  "Disable music",
                  "Отключить музыку");
    if(RD_GameType == gt_Strife)
    {
        CLI_Parameter("-novoice",
                      "Disable voice dialog and show dialog as text instead, even if voices.wad can be found",
                      "Отключить озвучку диалогов и отображать их как текст даже если voices.wad обнаружен");
        CLI_Parameter("-nograph",
                      "Disable graphical introduction sequence",
                      "Отключить графическую последовательность введения");
        CLI_Parameter("-flip",
                      "Flip player gun sprites (broken)",
                      "Отразить спрайты оружия игрока (сломано)");
    }
    CLI_Parameter("-noblit",
                  "Disable blitting the screen",
                  "Отключить blitting экрана");
    CLI_Parameter("-nomouse",
                  "Disable the mouse",
                  "Отключить мышь");
    CLI_Parameter("-nograbmouse",
                  "Don't grab the mouse when running in windowed mode",
                  "Не захватывать мышь при игре в оконном режиме");
    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-ravpic",
                      "Take screenshots when F1 is pressed",
                      "Сохранять скриншоты при нажатии F1");
    }
//    CLI_Parameter("-testcontrols", // Undocumented
//                  "Invoked by setup to test the controls",
//                  "Вызывается setup'ом для проверки управления");

// Demo
    CLI_Parameter("-playdemo <demo>",
                  "Play back the demo named <demo>.lmp. Demo lumps in working directory have priority over ones from wad files",
                  "Проиграть демо <demo>.lmp. Демо лампы в рабочей директории имеют приоритет над лампами в вадах");
    CLI_Parameter("-timedemo <demo>",
                  "Play back the demo named <demo>.lmp with uncapped tic rate. Demo lumps in working directory have priority over ones from wad files",
                  "Проиграть демо <demo>.lmp с неограниченным тикрейтом. Демо лампы в рабочей директории имеют приоритет над лампами в вадах");
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-nodraw",
                      "Disable rendering the screen entirely in -timedemo mode",
                      "Полностью отключить рендер экрана в режиме -timedemo");
    }
    CLI_Parameter("-record <demo>",
                  "Record demo to file <demo>.lmp stored into working directory",
                  "Записать демо в файл <demo>.lmp расположенный в рабочей директории");
    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-recordfrom <slot> <demo>",
                      "Load the game in savegame <slot> and start recording demo to file <demo>.lmp stored into working directory. Equivalent to '-loadgame <slot> -record <demo>'",
                      "Загрузить сохранённую игру из слота <slot> и начать запись демо в файл <demo>.lmp расположенный в рабочей директории. Эквивалент '-loadgame <slot> -record <demo>'");
    }
    CLI_Parameter("-maxdemo <size>",
                  "Specify the demo buffer size (KiB)",
                  "Задать размер буфера для демо (KiB)");
    CLI_Parameter("-strictdemos",
                  "When recording or playing back demos, disable any extensions of the vanilla demo format - record demos as vanilla would do, and play back demos as vanilla would do",
                  "При записи или проигрывании демо отключить все расширения оригинального формата демо - записывать демо как это бы делал оригинал, и проигрывать демо как это бы делал оригинал");
    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-demoextend",
                      "Record or playback a demo without automatically quitting after either level exit or player respawn",
                      "Записать или проиграть демо без автоматического выхода после завершения уровня или возрождения игрока");
    }
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-statdump <file>",
                      "Dump statistics information of the levels that were played to the specified <file>. The output from this option matches the output from statdump.exe. If '-' provided as <file> argument, statistics will be printed to console",
                      "Вывести статистическую информацию об уровнях, которые были воспроизведены, в указанный <file>. Выходные данные этого параметра совпадают с выходными данными из statdump.exe. Если в качестве аргумента <file> передан '-', статистика будет выведена в консоль");
    }
    CLI_Parameter("-longtics",
                  "Record or playback a demo with high resolution turning",
                  "Записать или проиграть демо с высоким разрешением поворота");
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-shorttics",
                      "Play with low turning resolution to emulate demo recording",
                      "Играть с низким разрешением поворота, чтобы эмулировать запись демо");
    }
    if(RD_GameType == gt_Heretic || RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-shortticfix",
                      "Smooth out low resolution turning when recording a demo",
                      "Сглаживать повороты с низким разрешением при записи демо");
    }

// Net
    CLI_Parameter("-autojoin",
                  "Automatically search the local LAN for a multiplayer server and join it",
                  "Автоматически найти сервер сетевой игры в локальной сети и подключится к нему");
    CLI_Parameter("-connect <address>",
                  "Connect to a multiplayer server running on the given <address>",
                  "Подключится к серверу сетевой игры работающему на данном адресе");
    CLI_Parameter("-drone",
                  "Join the server as an observer client",
                  "Подключится к серверу в режиме наблюдателя");
    CLI_Parameter("-solo-net",
                  "Start the game playing as though in a netgame with a single player. This can also be used to play back single player netgame demos",
                  "Начать игру как сетевую с одним игроком. Это также может быть использовано для просмотра демо сетевой игры с одним игроком");
    CLI_Parameter("-server",
                  "Start a multiplayer server, listening for connections",
                  "Запустить сервер сетевой игры ожидающий соединений");
    CLI_Parameter("-privateserver",
                  "When running a server, don't register with the chocolate-doom global master server. Implies -server",
                  "При запуске сервера не регистрировать его в глобальном списке серверов на мастер-сервере chocolate-doom. Подразумевает -server");
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-dedicated",
                      "Start a dedicated server, routing packets but not participating in the game itself",
                      "Режим выделенного сервера, маршрутизирует пакеты, но не участвует в самой игре");
    }
    CLI_Parameter("-port <port>",
                  "Use the specified UDP <port> for communications (default '2342')",
                  "Использовать заданный UDP порт для связи (по умолчанию '2342')");
    CLI_Parameter("-servername <name>",
                  "When starting a network server, specify a <name> for the server",
                  "В режиме сервера установить его имя");
    CLI_Parameter("-nodes <n>",
                  "Autostart the netgame when <n> clients have joined the server",
                  "Автоматически начинать сетевую игру когда <n> клиентов подключились к серверу");
    CLI_Parameter("-ignoreversion",
                  "When running a netgame server, ignore version mismatches between the server and the client. Using this option may cause game desyncs to occur, or differences in protocol may mean the netgame will simply not function at all",
                  "В режиме сервера игнорировать разницу в версиях между сервером и клиентом. Использование этой опции может привести к рассинхрону, или разница в протоколах может быть причиной полной неработоспособности сетевой игры");
    CLI_Parameter("-newsync",
                  "Use new network client sync code rather than the classic sync code. This is disabled by default because it has some bugs",
                  "Использовать новый код синхронизации сетевых клиентов вместо классического кода синхронизации. Отключено по умолчанию так как содержит баги");
    CLI_Parameter("-extratics <n>",
                  "Send <n> extra tics in every packet as insurance against dropped packets",
                  "Отправлять <n> дополнительных тиков в каждом пакете на случай потери пакетов");
    CLI_Parameter("-dup <n>",
                  "Reduce the resolution of turning by a factor of <n>, reducing the amount of network bandwidth needed",
                  "Уменьшить разрешение поворота в <n> раз, уменьшая необходимую пропускную способность сети");
    if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-cmdfrag",
                      "Sent out a frag count packet on every kill",
                      "Отправлять пакет с количеством убийств после каждого убийства");
    }
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-avg",
                      "Austin Virtual Gaming: Levels will end after 20 minutes",
                      "Austin Virtual Gaming: Уровни завершаются спустя 20 минут");
    }
    if(RD_GameType != gt_Strife)
    {
        CLI_Parameter("-deathmatch",
                      "Start a deathmatch game",
                      "Начать игру в режиме deathmatch");
    }
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-altdeath",
                      "Start a deathmatch 2.0 game. Weapons do not stay in place and all items respawn after 30 seconds",
                      "Начать игру в режиме deathmatch 2.0. Оружие не остаётся на месте, и все предметы появляются вновь через 30 секунд");
    }
    if(RD_GameType == gt_Doom)
    {
        CLI_Parameter("-dm3",
                      "Start a deathmatch 3.0 game. Weapons stay in place and all items respawn after 30 seconds",
                      "Начать игру в режиме deathmatch 3.0. Оружие остаётся на месте, и все предметы появляются вновь через 30 секунд");
    }
    if(RD_GameType == gt_Hexen)
    {
        CLI_Parameter("-randclass",
                      "In deathmatch mode, change a player's class each time the player respawns",
                      "В режиме deathmatch класс игрока меняется при каждом возрождении");
    }
    CLI_Parameter("-timer <n>",
                  "For multiplayer games: exit each level after <n> minutes",
                  "Для мультиплеерных игр: каждый уровень завершается после <n> минут");
    if(RD_GameType == gt_Doom || RD_GameType == gt_Strife)
    {
        CLI_Parameter("-left",
                      "Run as the left screen in three screen mode",
                      "Левый экран в 3-х экранном режиме");
        CLI_Parameter("-right",
                      "Run as the right screen in three screen mode",
                      "Правый экран в 3-х экранном режиме");
        CLI_Parameter("-search",
                      "Query the chocolate-doom Internet master server for a global list of active servers",
                      "Запрос глобального списка действующих серверов у мастер-сервера chocolate-doom");
        CLI_Parameter("-query <address>",
                      "Query the status of the server running on the given IP <address>",
                      "Запрос статуса сервера по данному IP адресу");
        CLI_Parameter("-localsearch",
                      "Search the local LAN for running servers",
                      "Поиск действующих серверов в локальной сети");
    }
}

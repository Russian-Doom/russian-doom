//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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



// HEADER FILES ------------------------------------------------------------

// haleyjd: removed WATCOMC
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "config.h"
#include "doomfeatures.h"

#include "h2def.h"
#include "ct_chat.h"
#include "d_iwad.h"
#include "d_mode.h"
#include "m_misc.h"
#include "s_sound.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "net_client.h"
#include "p_local.h"
#include "v_video.h"
#include "w_main.h"
#include "w_merge.h"
#include "r_bmaps.h"
#include "rd_rushexen.h"

// MACROS ------------------------------------------------------------------

#define MAXWADFILES 20
#define CT_KEY_BLUE         'b'
#define CT_KEY_RED          'r'
#define CT_KEY_YELLOW       'y'
#define CT_KEY_GREEN        'g'
#define CT_KEY_PLAYER5      'j'     // Jade
#define CT_KEY_PLAYER6      'w'     // White
#define CT_KEY_PLAYER7      'h'     // Hazel
#define CT_KEY_PLAYER8      'p'     // Purple
#define CT_KEY_ALL          't'

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void D_ConnectNetGame(void);
void D_CheckNetGame(void);
boolean F_Responder(event_t * ev);
void I_StartupKeyboard(void);
void I_StartupJoystick(void);
void I_ShutdownKeyboard(void);
void S_InitScript(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void H2_ProcessEvents(void);
void H2_DoAdvanceDemo(void);
void H2_AdvanceDemo(void);
void H2_StartTitle(void);
void H2_PageTicker(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void DrawMessage(void);
static void PageDrawer(void);
static void HandleArgs(void);
static void CheckRecordFrom(void);
static void DrawAndBlit(void);
static void WarpCheck(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean automapactive;
extern boolean askforquit;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

GameMode_t gamemode;
char *gamedescription;
char *iwadfile;
static char demolumpname[9];    // Demo lump to start playing.
boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean randomclass;            // checkparm of -randclass
boolean debugmode;              // checkparm of -debug
boolean ravpic;                 // checkparm of -ravpic
boolean cdrom = false;          // true if cd-rom mode active
boolean cmdfrag;                // true if a CMD_FRAG packet should be sent out
boolean artiskip;               // whether shift-enter skips an artifact
int maxzone = 0x800000;         // Maximum allocated for zone heap (8meg default)
skill_t startskill;
int startepisode;
int startmap;
boolean autostart;
boolean advancedemo;
FILE *debugfile;
int UpdateState;
int maxplayers = MAXPLAYERS;
int demosequence;

boolean title_mus_played = false;

boolean hasUnknownPWads = false;
boolean isHexenDemo = false;
boolean isDK = false;

// [JN] Support for fallback to the English language.
// Windows OS only: do not set game language on first launch, 
// try to determine it automatically in D_DoomMain.
#ifdef _WIN32
int english_language = -1;
#else
int english_language = 0;
#endif

// Display
int level_brightness = 0;
int hud_detaillevel = 0;    // [JN] Blocky mode, 0 = high, 1 = normal
int local_time = 0; 

// Automap
int automap_follow = 1;
int automap_overlay = 0;
int automap_rotate = 0;
int automap_grid = 0;

// Sound
int snd_monomode = 0;

// Gameplay: Graphical
int brightmaps = 1;
int fake_contrast = 0;
int flip_levels = 0;
int draw_shadowed_text = 1;

// Gameplay: Crosshair
int crosshair_draw = 0;
int crosshair_type = 1;
int crosshair_scale = 0;

// Gameplay: Gameplay
int no_internal_demos = 0;


// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WarpMap;
static int pagetic;
static char *pagename;

// CODE --------------------------------------------------------------------

void D_BindVariables(void)
{
    int i;

    M_ApplyPlatformDefaults();

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindMapControls();
    M_BindMenuControls();
    M_BindWeaponControls();
    M_BindChatControls(MAXPLAYERS);
    M_BindHereticControls();
    M_BindHexenControls();

    key_multi_msgplayer[0] = CT_KEY_BLUE;
    key_multi_msgplayer[1] = CT_KEY_RED;
    key_multi_msgplayer[2] = CT_KEY_YELLOW;
    key_multi_msgplayer[3] = CT_KEY_GREEN;
    key_multi_msgplayer[4] = CT_KEY_PLAYER5;
    key_multi_msgplayer[5] = CT_KEY_PLAYER6;
    key_multi_msgplayer[6] = CT_KEY_PLAYER7;
    key_multi_msgplayer[7] = CT_KEY_PLAYER8;

#ifdef FEATURE_MULTIPLAYER
    NET_BindVariables();
#endif

    M_BindIntVariable("graphical_startup",      &graphical_startup);
    M_BindIntVariable("english_language",       &english_language);
    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindIntVariable("sfx_volume",             &snd_MaxVolume);
    M_BindIntVariable("music_volume",           &snd_MusicVolume);
    M_BindIntVariable("snd_monomode",           &snd_monomode);
    M_BindIntVariable("messageson",             &messageson);
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("snd_channels",           &snd_Channels);
    M_BindIntVariable("mlook",                  &mlook);
    M_BindIntVariable("level_brightness",       &level_brightness);
    M_BindIntVariable("local_time",             &local_time);

    // Automap
    M_BindIntVariable("automap_follow",         &automap_follow);
    M_BindIntVariable("automap_overlay",        &automap_overlay);
    M_BindIntVariable("automap_rotate",         &automap_rotate);
    M_BindIntVariable("automap_grid",           &automap_grid);

    // [JN] Дополнительные параметры игры

    // Графика
    M_BindIntVariable("uncapped_fps",           &uncapped_fps);
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);

    // Геймплей
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);

    // Прицел
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_type",         &crosshair_type);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);

    // Multiplayer chat macros
    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }
}

// The Mac version of the Hexen IWAD is different to the "normal" DOS
// version - it doesn't include lumps used by the DOS DMX library.
// This means that we can't do GUS or OPL emulation and need to apply
// a workaround.
static void AdjustForMacIWAD(void)
{
    boolean adjust_music = false;

    switch (snd_musicdevice)
    {
        case SNDDEVICE_ADLIB:
        case SNDDEVICE_SB:
            adjust_music = W_CheckNumForName("GENMIDI") < 0;
            break;

        case SNDDEVICE_GUS:
            adjust_music = W_CheckNumForName("DMXGUS") < 0;
            break;

        default:
            break;
    }

    if (adjust_music)
    {
        if (english_language)
        {
            printf("** Note: You appear to be using the Mac version of the Hexen\n"
                   "** IWAD file. This is missing the lumps required for OPL or\n"
                   "** GUS emulation. Your music configuration is being adjusted\n"
                   "** to a different setting that won't cause the game to "
                   "crash.\n");
        }
        else
        {
            printf("** Внимание: предположительно, вы используете IWAD версии для Mac.\n"
                   "** В нем отсутствует информация о проигрывании музыки в формате OPL\n"
                   "** или GUS. Применена настройка дополнительной совместимости, которая\n"
                   "** позволит проигрывать музыку корректно.\n");
        }
        snd_musicdevice = SNDDEVICE_GENMIDI;
    }
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

static boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse

    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !advancedemo && !demoplayback;
}

// Message displayed when quitting Hexen

static void D_HexenQuitMessage(void)
{
    printf("\nHexen: Beyond Heretic\n");
}

static void D_AddFile(char *filename)
{
    printf(english_language ?
    " adding: %s\n" :
    " добавление: %s\n", filename);

    W_AddFile(filename);
}

// Find out what version of Hexen is playing.

void D_IdentifyVersion(void)
{
    // The Hexen Shareware, ne 4 Level Demo Version, is missing the SKY1 lump
    // and uses the SKY2 lump instead. Let's use this fact and the missing
    // levels from MAP05 onward to identify it and set gamemode accordingly.

    if (W_CheckNumForName("SKY1") == -1 &&
        W_CheckNumForName("MAP05") == -1 )
    {
	gamemode = shareware;
	maxplayers = 4;
    }

    // The v1.0 IWAD file is missing a bunch of lumps that can cause the game
    // to crash, so we exit with an error if the user tries to play with it.
    // But we provide an override command line flag if they really want to
    // do it.

    //!
    // If provided, the check for the v1.0 IWAD file is disabled, even though
    // it will almost certainly cause the game to crash.
    //
    // @category compat
    //

    if (!M_ParmExists("-v10override")
     && gamemode != shareware && W_CheckNumForName("CLUS1MSG") < 0)
    {
        if (english_language)
        {
            I_Error(
            "You are trying to use the Hexen v1.0 IWAD. This isn't\n"
            "supported by Russian Hexen. Please upgrade to the v1.1\n"
            "IWAD file. See here for more information:\n"
            "  https://www.doomworld.com/classicdoom/info/patches.php");
        }
        else
        {
            I_Error(
            "Использование IWAD-файла версии 1.0 не рекоммендуется,\n"
            "так как он содержит ошибки в игре. Просьба обновить IWAD\n"
            "до версии 1.1. Дополнительная информация доступна по ссылке:\n"
            "  https://www.doomworld.com/classicdoom/info/patches.php");            
        }

    }
}

// Set the gamedescription string.

void D_SetGameDescription(void)
{
/*
    NB: The 4 Level Demo Version actually prints a four-lined banner
    (and indeed waits for a keypress):

    Hexen:  Beyond Heretic

    4 Level Demo Version
    Press any key to continue.
*/
    int newpwadfile;
    char* internalWadName = RD_M_FindInternalResource("hexen-common.wad");

    W_MergeFile(internalWadName);
    free(internalWadName);
    if (gamemode == shareware)
    {
        isHexenDemo = true;
        gamedescription = english_language ?
                          "Hexen: 4 Level Demo Version" :
                          "Hexen: Демоверсия четырех уровней";
    }
    else
    {
        gamedescription = "Hexen";
    }

    // [JN] Параметр "-file" перенесен из w_main.c
    // Необходимо для того, чтобы любые ресурсы из pwad-файлов
    // загружались после руссифицированных pwad-файлов.

    newpwadfile = M_CheckParmWithArgs ("-file", 1);
    if (newpwadfile)
    {
        while (++newpwadfile != myargc && myargv[newpwadfile][0] != '-')
        {
            char    *filename;

            filename = D_TryFindWADByName(myargv[newpwadfile]);
            printf(english_language ?
                   " adding: %s\n" :
                   " добавление: %s\n",
                   filename);
            W_MergeFile(filename);

            // [JN] Поддержка Hexen: Deathkings of the Dark Citadel
            // Больше спасибо CapnClever за оказанную помощь!
            // [Dasperal] Переписано на нормальный код
            if (M_StrCaseStr(myargv[newpwadfile], "hexdd.wad")) //Deathkings of the Dark Citadel
            {
                isDK = true;
                gamedescription = english_language ? 
                                  "Hexen: Deathkings of the Dark Citadel" :
                                  "Hexen: Короли Смерти Темной Цитадели";
            }
            else //Any unknown pwad
            {
                hasUnknownPWads = true;
            }
        }
    }
}

//==========================================================================
//
// H2_Main
//
//==========================================================================
void InitMapMusicInfo(void);

void D_DoomMain(void)
{
    GameMission_t gamemission;
    int p;

#ifdef _WIN32
    // [JN] Get system preffed language...
    DWORD rd_lang_id = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));
    // ..if game language is not set yet (-1), and OS preffered language
    // is appropriate for using Russian language in the game, use it.
    if (english_language == -1)
    {
        if (rd_lang_id != LANG_RUSSIAN
        &&  rd_lang_id != LANG_UKRAINIAN
        &&  rd_lang_id != LANG_BELARUSIAN)
        english_language = 1;
        else
        english_language = 0;
    }

    // [JN] Print colorized title
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED 
                                                           | FOREGROUND_RED
                                                           | FOREGROUND_GREEN
                                                           | FOREGROUND_BLUE
                                                           | FOREGROUND_INTENSITY);
    for (p = 0 ; p < 32 ; p++) ST_Message(" ");
    ST_Message("Russian Hexen " PACKAGE_VERSION);
    for (p = 0 ; p < 31 ; p++) ST_Message(" ");
    ST_Message("\n");

    // [JN] Fallback to standard console colos
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED
                                                           | FOREGROUND_GREEN
                                                           | FOREGROUND_BLUE);
#else
    // [JN] Just print an uncolored banner
    for (p = 0 ; p < 32 ; p++) ST_Message(" ");
    ST_Message("Russian Hexen " PACKAGE_VERSION);
    for (p = 0 ; p < 31 ; p++) ST_Message(" ");
    ST_Message("\n");
#endif

    I_AtExit(D_HexenQuitMessage, false);
    startepisode = 1;
    autostart = false;
    startskill = sk_medium;
    startmap = 1;
    gamemode = commercial;

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);

    // Load defaults before initing other systems
    ST_Message(english_language ?
               "M_LoadDefaults: Load system defaults.\n" :
               "M_LoadDefaults: Загрузка системных стандартов.\n");
    D_BindVariables();

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\hexndata,
    // allowing play from CD.
    //

    cdrom = M_ParmExists("-cdrom");
#endif

    if (cdrom)
    {
        M_SetConfigDir("c:\\hexndata\\");
    }
    else
    {
        M_SetConfigDir(NULL);
    }

    M_SetConfigFilenames(/*"hexen.ini", */PROGRAM_PREFIX "hexen.ini");
    M_LoadDefaults();

    // Initialize subsystems

    ST_Message(english_language ?
               "V_Init: allocate screens.\n" :
               "V_Init: Инициализация видео.\n");
    V_Init();

    I_AtExit(M_SaveDefaults, false);

    // Set the directory where hub savegames are saved.
    SavePath = M_GetSaveGameDir();

    ST_Message(english_language ?
               "Z_Init: Init zone memory allocation daemon.\n" :
               "Z_Init: Инициализация распределения памяти.\n");
    Z_Init();

    // haleyjd: removed WATCOMC

    ST_Message(english_language ?
               "W_Init: Init WADfiles.\n" :
               "W_Init: Инициализация WAD-файлов.\n");

    iwadfile = D_FindIWAD(IWAD_MASK_HEXEN, &gamemission);

    if (iwadfile == NULL)
    {
        if (english_language)
        {
            I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                    "one with the '-iwad' command line parameter.");
        }
        else
        {
            I_Error("Невозможно определить игру из за отсутствующего IWAD-файла.\n"
                    "Попробуйте указать IWAD-файл командой '-iwad'.\n");
        }
    }

    D_AddFile(iwadfile);
    W_CheckCorrectIWAD(hexen);
    D_IdentifyVersion();
    D_SetGameDescription();
    AdjustForMacIWAD();

    HandleArgs();

    ST_Message(english_language ?
               "MN_Init: Init menu system.\n" :
               "MN_Init: Инициализация системы меню.\n");
    MN_Init();

    ST_Message(english_language ?
               "CT_Init: Init chat mode data.\n" :
               "CT_Init: Инициализация данных режима чата.\n");
    CT_Init();

    InitMapMusicInfo();         // Init music fields in mapinfo

    ST_Message(english_language ?
               "S_InitScript: Init script system.\n" :
               "S_InitScript: Инициализация системы скриптов.\n");
    S_InitScript();

    ST_Message(english_language ?
               "SN_InitSequenceScript: Registering sound sequences.\n" :
               "SN_InitSequenceScript: Регистрация наборов звуковых событий.\n");
    SN_InitSequenceScript();
    ST_Message(english_language ?
               "I_Init: Setting up machine state.\n" :
               "I_Init: Инициализация состояния компьютера.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(false);
    I_InitMusic();

#ifdef FEATURE_MULTIPLAYER
    ST_Message(english_language ?
               "NET_Init: Init networking subsystem.\n" :
               "NET_Init: Инициализация сетевой подсистемы.\n");
    NET_Init();
#endif
    D_ConnectNetGame();

    S_Init();
    S_Start();

    ST_Message(english_language ?
               "ST_Init: Init startup screen.\n" :
               "ST_Init: Инициализация загрузочного экрана.\n");
    ST_Init();

    // Show version message now, so it's visible during R_Init()
    ST_Message(english_language ?
               "R_Init: Init Hexen refresh daemon" :
               "R_Init: Инициализация процесса запуска Hexen");
    R_Init();
    ST_Message("\n");

    //if (M_CheckParm("-net"))
    //    ST_NetProgress();       // Console player found

    ST_Message(english_language ?
               "P_Init: Init Playloop state.\n" :
               "P_Init: Инициализация игрового окружения.\n");
    P_Init();

    // Check for command line warping. Follows P_Init() because the
    // MAPINFO.TXT script must be already processed.
    WarpCheck();

    ST_Message(english_language ?
               "D_CheckNetGame: Checking network game status.\n" :
               "D_CheckNetGame: Проверка статуса сетевой игры.\n");
    D_CheckNetGame();

    ST_Message(english_language ?
               "SB_Init: Loading patches.\n" :
               "SB_Init: Загрузка патчей.\n");
    SB_Init();

    ST_Done();

    if (autostart)
    {
        ST_Message(english_language ?
                   "Warp to Map %d (\"%s\":%d), Skill %d\n" :
                   "Перемещение на уровень: %d (\"%s\":%d), сложность: %d\n",
                   WarpMap, P_GetMapName(startmap), startmap, startskill + 1);
    }

    CheckRecordFrom();

    p = M_CheckParm("-record");
    if (p && p < myargc - 1)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        H2_GameLoop();          // Never returns
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(demolumpname);
        H2_GameLoop();          // Never returns
    }

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo(demolumpname);
        H2_GameLoop();          // Never returns
    }

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in savegame slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    if (p)
    {
        G_LoadGame(atoi(myargv[p + 1]));
    }

    if (gameaction != ga_loadgame)
    {
        UpdateState |= I_FULLSCRN;
        BorderNeedRefresh = true;
        if (autostart || netgame)
        {
            G_StartNewInit();
            G_InitNew(startskill, startepisode, startmap);
        }
        else
        {
            H2_StartTitle();
        }
    }

    // [JN] Show the game we are playing
    ST_Message(english_language ? "Starting game: " : "Запуск игры: ");
    ST_Message("\"");
    ST_Message(gamedescription);
    ST_Message("\".");
    ST_Message("\n");

    H2_GameLoop();              // Never returns
}

//==========================================================================
//
// HandleArgs
//
//==========================================================================

static void HandleArgs(void)
{
    int p;

    //!
    // @vanilla
    //
    // Disable monsters.
    //

    nomonsters = M_ParmExists("-nomonsters");

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_ParmExists("-respawn");

    //!
    // @vanilla
    // @category net
    //
    // In deathmatch mode, change a player's class each time the
    // player respawns.
    //

    randomclass = M_ParmExists("-randclass");

    //!
    // @vanilla
    //
    // Take screenshots when F1 is pressed.
    //

    ravpic = M_ParmExists("-ravpic");

    //!
    // @vanilla
    //
    // Don't allow artifacts to be used when the run key is held down.
    //

    artiskip = M_ParmExists("-artiskip");

    debugmode = M_ParmExists("-debug");

    //!
    // @vanilla
    // @category net
    //
    // Start a deathmatch game.
    //

    deathmatch = M_ParmExists("-deathmatch");

    // currently broken or unused:
    cmdfrag = M_ParmExists("-cmdfrag");

    // Check WAD file command line options
    W_ParseCommandLine();

    //!
    // @vanilla
    // @arg <path>
    //
    // Development option to specify path to level scripts.
    //

    p = M_CheckParmWithArgs("-scripts", 1);

    if (p)
    {
        sc_FileScripts = true;
        sc_ScriptsDir = myargv[p+1];
    }

    //!
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //

    p = M_CheckParmWithArgs("-skill", 1);

    if (p)
    {
        startskill = myargv[p+1][0] - '1';
        autostart = true;
    }

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs("-playdemo", 1);

    if (!p)
    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //

        p = M_CheckParmWithArgs("-timedemo", 1);
    }

    if (p)
    {
        char *uc_filename;
        char file[256];

        M_StringCopy(file, myargv[p+1], sizeof(file));

        // With Vanilla Hexen you have to specify the file without
        // extension, but make that optional.
        uc_filename = strdup(myargv[p + 1]);
        M_ForceUppercase(uc_filename);

        if (!M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringConcat(file, ".lmp", sizeof(file));
        }

        free(uc_filename);

        if (W_AddFile(file) != NULL)
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name,
                         sizeof(demolumpname));
        }
        else
        {
            // The file failed to load, but copy the original arg as a
            // demo name to make tricks like -playdemo demo1 possible.
            M_StringCopy(demolumpname, myargv[p+1], sizeof(demolumpname));
        }

        ST_Message("Проигрывание демозаписи %s.\n", myargv[p+1]);	// "Playing demo %s.\n"
    }

    // [JN] Addition: also generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    //!
    // @category demo
    //
    // Record or playback a demo without automatically quitting
    // after either level exit or player respawn.
    //

    demoextend = M_ParmExists("-demoextend");

    if (M_ParmExists("-testcontrols"))
    {
        autostart = true;
        testcontrols = true;
    }
}

//==========================================================================
//
// WarpCheck
//
//==========================================================================

static void WarpCheck(void)
{
    int p;
    int map;

    p = M_CheckParm("-warp");
    if (p && p < myargc - 1)
    {
        WarpMap = atoi(myargv[p + 1]);
        map = P_TranslateMap(WarpMap);
        if (map == -1)
        {                       // Couldn't find real map number
            startmap = 1;
            ST_Message("-WARP: Некорректный номер уровня.\n");	// "-WARP: Invalid map number.\n"
        }
        else
        {                       // Found a valid startmap
            startmap = map;
            autostart = true;
        }
    }
    else
    {
        WarpMap = 1;
        startmap = P_TranslateMap(1);
        if (startmap == -1)
        {
            startmap = 1;
        }
    }
}

//==========================================================================
//
// H2_GameLoop
//
//==========================================================================

void H2_GameLoop(void)
{
    if (M_CheckParm("-debugfile"))
    {
        char filename[20];
        M_snprintf(filename, sizeof(filename), "debug%i.txt", consoleplayer);
        debugfile = fopen(filename, "w");
    }
    I_SetWindowTitle(gamedescription);
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_InitGraphics();

    while (1)
    {
        // [JN] Don't call empty function
        // Frame syncronous IO operations
        // I_StartFrame();

        // Process one or more tics
        // Will run at least one tic
        TryRunTics();

        // [JN] Mute and restore sound and music volume.
        if (mute_inactive_window && volume_needs_update)
        {
            if (!window_focused)
            {
                S_MuteSound();
            }
            else
            {
                S_UnMuteSound();
            }
        }

        // Move positional sounds
        S_UpdateSounds(players[displayplayer].mo);

        // Update display, next frame, with current state.
        if (screenvisible)
        DrawAndBlit();
    }
}

//==========================================================================
//
// H2_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//==========================================================================

void H2_ProcessEvents(void)
{
    event_t *ev;

    for (;;)
    {
        ev = D_PopEvent();

        if (ev == NULL)
        {
            break;
        }

        if (F_Responder(ev))
        {
            continue;
        }
        if (MN_Responder(ev))
        {
            continue;
        }
        G_Responder(ev);
    }
}

//==========================================================================
//
// DrawAndBlit
//
//==========================================================================

static void DrawAndBlit(void)
{
    // [JN] Set correct palette. Allow finale stages choose palettes.
    if (gamestate != GS_LEVEL && gamestate != GS_FINALE)
    {
        I_SetPalette(W_CacheLumpName(usegamma <= 8 ?
                                    "PALFIX" :
                                    "PLAYPAL",
                                    PU_CACHE));
    }

    // Change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
    }

    // Do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
            {
                break;
            }
            if (automapactive)
            {
                // [crispy] update automap while playing
                R_RenderPlayerView(&players[displayplayer]);
                AM_Drawer();
            }
            else
            {
                R_RenderPlayerView(&players[displayplayer]);
            }
            CT_Drawer();
            UpdateState |= I_FULLVIEW;
            SB_Drawer();
            break;
        case GS_INTERMISSION:
            IN_Drawer();
            break;
        case GS_FINALE:
            F_Drawer();
            break;
        case GS_DEMOSCREEN:
            PageDrawer();
            break;
    }

    if (testcontrols)
    {
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    if (paused && !menuactive && !askforquit)
    {
        if (!netgame)
        {
            V_DrawShadowedPatchRaven(160 + wide_delta, (viewwindowy >> hires) + 5,
                                     W_CacheLumpName(english_language ?
                                     "PAUSED" : "RD_PAUS", PU_CACHE));
        }
        else
        {
            V_DrawShadowedPatchRaven(160 + wide_delta, 70,
                                    W_CacheLumpName(english_language ?
                                    "PAUSED" : "RD_PAUS", PU_CACHE));
        }
    }

    // Draw current message
    DrawMessage();

    // Draw Menu
    MN_Drawer();

    // Send out any new accumulation
    NetUpdate();

    // Flush buffered stuff to screen
    I_FinishUpdate();
}

//==========================================================================
//
// DrawMessage
//
//==========================================================================

static void DrawMessage(void)
{
    player_t *player;

    player = &players[consoleplayer];
    if (player->messageTics <= 0)
    {                           // No message
        return;
    }
    if (player->yellowMessage)
    {
        if (english_language)
        {
            MN_DrTextAYellow(player->message,
                            160 - MN_TextAWidth(player->message) / 2 
                            + wide_delta, 1);
        }
        else
        {
            MN_DrTextSmallYellowRUS(player->message,
                                    160 - MN_DrTextSmallRUSWidth(player->message) / 2 
                                    + wide_delta, 1);
        }
    }
    else
    {
        if (english_language)
        {
            MN_DrTextA(player->message, 
                       160 - MN_TextAWidth(player->message) / 2
                       + wide_delta, 1);
        }
        else
        {
            MN_DrTextSmallRUS(player->message,
                              160 - MN_DrTextSmallRUSWidth(player->message) / 2
                              + wide_delta, 1);            
        }
    }
}

//==========================================================================
//
// H2_PageTicker
//
//==========================================================================

void H2_PageTicker(void)
{
    if (--pagetic < 0)
    {
        H2_AdvanceDemo();
    }
}

//==========================================================================
//
// PageDrawer
//
//==========================================================================

static void PageDrawer(void)
{
    boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, WIDESCREENWIDTH, SCREENHEIGHT, 0);
    }

    V_DrawRawScreen(W_CacheLumpName(pagename, PU_CACHE));
    if (demosequence == 1)
    {
        V_DrawShadowedPatchRaven(4 + (wide_4_3 ? wide_delta : 0), 160, 
                                 W_CacheLumpName( english_language ?
                                 "ADVISOR" : "RD_ADV" , PU_CACHE));
    }
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// H2_AdvanceDemo
//
// Called after each demo or intro demosequence finishes.
//
//==========================================================================

void H2_AdvanceDemo(void)
{
    advancedemo = true;
}

//==========================================================================
//
// H2_DoAdvanceDemo
//
//==========================================================================

void H2_DoAdvanceDemo(void)
{
    S_ResumeSound();    // [JN] Fix vanilla Hexen bug: resume music playing
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save/end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
    {
        case 0:
            pagetic = 280;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            {
                pagename = "TITLE";
            }
            else
            {
                pagename = isDK ? "TITLEDK" : "TITLE";
            }
            if (!title_mus_played)
            S_StartSongName("hexen", false);
            if (no_internal_demos)
            title_mus_played = true;
            break;
        case 1:
            pagetic = 210;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            {
                pagename = "TITLE";
            }
            else
            {
                pagename = isDK ? "TITLEDK" : "TITLE";
            }
            break;
        case 2:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo("demo1");
            break;
        case 3:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            {
                pagename = "CREDIT";
            }
            else
            {
                pagename = isDK ? "RD_CREDK" : "RD_CREDT";
            }
            break;
        case 4:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo("demo2");
            break;
        case 5:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            {
                pagename = "CREDIT";
            }
            else
            {
                pagename = isDK ? "RD_CREDK" : "RD_CREDT";
            }
            break;
        case 6:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo("demo3");
            break;
    }
}

//==========================================================================
//
// H2_StartTitle
//
//==========================================================================

void H2_StartTitle(void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    H2_AdvanceDemo();
}

//==========================================================================
//
// CheckRecordFrom
//
// -recordfrom <savegame num> <demoname>
//
//==========================================================================

static void CheckRecordFrom(void)
{
    int p;

    p = M_CheckParm("-recordfrom");
    if (!p || p > myargc - 2)
    {                           // Bad args
        return;
    }
    G_LoadGame(atoi(myargv[p + 1]));
    G_DoLoadGame();             // Load the gameskill etc info from savegame
    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);

    H2_GameLoop();              // Never returns
}

// haleyjd: removed WATCOMC
/*
void CleanExit(void)
{
	union REGS regs;

	I_ShutdownKeyboard();
	regs.x.eax = 0x3;
	int386(0x10, &regs, &regs);
	printf("Exited from HEXEN: Beyond Heretic.\n");
	exit(1);
}
*/

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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



//
// Памятка по символам: docs_russian/charmap/raven_charmap.xlsx
//


// MN_menu.c


#include <stdlib.h>
#include <ctype.h>

#include "deh_str.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "i_input.h"
#include "i_system.h"
#include "i_swap.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_trans.h"
#include "v_video.h"
#include "crispy.h"
#include "jn.h"

// Macros

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN     16
#define ASCII_CURSOR '_'

// [JN] Sizes of small font and small arrow for RD menu
#define ITEM_HEIGHT_SMALL 10
#define SELECTOR_XOFFSET_SMALL (-14)

// Types

typedef enum
{
    ITT_EMPTY,
    ITT_EFUNC,
    ITT_LRFUNC,
    ITT_SETMENU,
    ITT_INERT
} ItemType_t;

typedef enum
{
    MENU_MAIN,
    MENU_EPISODE,
    MENU_SKILL,
    MENU_OPTIONS,
    MENU_FILES,
    MENU_LOAD,
    MENU_SAVE,
    MENU_RENDERING,
    MENU_DISPLAY,
    MENU_SOUND,
    MENU_CONTROLS,
    MENU_GAMEPLAY1,
    MENU_GAMEPLAY2,
    MENU_NONE
} MenuType_t;

typedef struct
{
    ItemType_t type;
    char *text;
    boolean(*func) (int option);
    int option;
    MenuType_t menu;
} MenuItem_t;

typedef struct
{
    int x;
    int y;
    void (*drawFunc) (void);
    int itemCount;
    MenuItem_t *items;
    int oldItPos;
    MenuType_t prevMenu;
} Menu_t;

// Private Functions

static void InitFonts(void);
static void SetMenu(MenuType_t menu);
static boolean SCNetCheck(int option);
static boolean SCQuitGame(int option);
static boolean SCEpisode(int option);
static boolean SCSkill(int option);
static boolean SCLoadGame(int option);
static boolean SCSaveGame(int option);
static boolean SCMessages(int option);
static boolean SCInfo(int option);
static void DrawMainMenu(void);
static void DrawEpisodeMenu(void);
static void DrawSkillMenu(void);
static void DrawFileSlots(Menu_t * menu);
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawLoadMenu(void);
static void DrawSaveMenu(void);
static void DrawSlider(Menu_t * menu, int item, int width, int slot);
static void DrawSliderSmall(Menu_t * menu, int item, int width, int slot);
void MN_LoadSlotText(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static boolean M_RD_Change_VSync(int option);
static boolean M_RD_AspectRatio(int option);
static boolean M_RD_Uncapped(int option);
static boolean M_RD_FPScounter(int option);
static boolean M_RD_Smoothing(int option);
static boolean M_RD_Renderer(int option);
static boolean M_RD_Screenshots(int option);

// Display
static void DrawDisplayMenu(void);
static boolean M_RD_ScreenSize(int option);
static boolean M_RD_Gamma(int option);
static boolean M_RD_LocalTime(int option);
static boolean M_RD_Messages(int option);
static boolean M_RD_ShadowedText(int option);

// Sound
static void DrawSoundMenu(void);
static boolean M_RD_SfxVolume(int option);
static boolean M_RD_MusVolume(int option);
static boolean M_RD_SfxChannels(int option);
static boolean M_RD_SndMode(int option);
static boolean M_RD_PitchShifting(int option);

// Controls
static void DrawControlsMenu(void);
static boolean M_RD_AlwaysRun(int option);
static boolean M_RD_Sensitivity(int option);
static boolean M_RD_MouseLook(int option);
static boolean M_RD_Novert(int option);


// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static boolean M_RD_Brightmaps(int option);
static boolean M_RD_FakeContrast(int option);
static boolean M_RD_ColoredHUD(int option);
static boolean M_RD_ColoredBlood(int option);
static boolean M_RD_InvulSky(int option);
static boolean M_RD_AutoMapStats(int option);
static boolean M_RD_SecretNotify(int option);
static boolean M_RD_NegativeHealth(int option);

// Gameplay (page 2)
static void DrawGameplay2Menu(void);
static boolean M_RD_Torque(int option);
static boolean M_RD_Bobbing(int option);
static boolean M_RD_FlipCorpses(int option);
static boolean M_RD_CrossHairDraw(int option);
static boolean M_RD_CrossHairHealth(int option);
static boolean M_RD_CrossHairScale(int option);
static boolean M_RD_FlipLevels(int option);
static boolean M_RD_NoDemos(int option);

// End game
static boolean M_RD_EndGame(int option);

// Reset settings
static boolean M_RD_ResetSettings(int option);

// Change language
static boolean M_RD_ChangeLanguage(int option);

// External Data

extern int detailLevel;

// Public Data

boolean menuactive;
int InfoType;
boolean messageson;

// Private Data

// [JN] Original English fonts
static int FontABaseLump;   // small
static int FontBBaseLump;   // big
// [JN] Small special font used for time/fps widget
static int FontCBaseLump;
// [JN] Unchangable English fonts
static int FontDBaseLump;   // small
static int FontEBaseLump;   // big
// [JN] Unchangable Russian fonts
static int FontFBaseLump;   // small
static int FontGBaseLump;   // big

static int SkullBaseLump;
static Menu_t *CurrentMenu;
static int CurrentItPos;
static int MenuEpisode;
static int MenuTime;
static boolean soundchanged;

boolean askforquit;
static int typeofask;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static char SlotText[6][SLOTTEXTLEN + 2];
static char oldSlotText[SLOTTEXTLEN + 2];
static int SlotStatus[6];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;
extern int snd_Channels;

// [JN] Set default mouse sensitivity to 5, like in Doom
int mouseSensitivity = 5;

static MenuItem_t MainItems[] = {
    {ITT_EFUNC, "NEW GAME", SCNetCheck, 1, MENU_EPISODE},
    {ITT_SETMENU, "OPTIONS", NULL, 0, MENU_OPTIONS},
    {ITT_SETMENU, "GAME FILES", NULL, 0, MENU_FILES},
    {ITT_EFUNC, "INFO", SCInfo, 0, MENU_NONE},
    {ITT_EFUNC, "QUIT GAME", SCQuitGame, 0, MENU_NONE}
};

static MenuItem_t MainItems_Rus[] = {
    {ITT_EFUNC, "YJDFZ BUHF", SCNetCheck, 1, MENU_EPISODE},	// НОВАЯ ИГРА
    {ITT_SETMENU, "YFCNHJQRB", NULL, 0, MENU_OPTIONS},		// НАСТРОЙКИ
    {ITT_SETMENU, "AFQKS BUHS", NULL, 0, MENU_FILES},		// ФАЙЛЫ ИГРЫ
    {ITT_EFUNC, "BYAJHVFWBZ", SCInfo, 0, MENU_NONE},		// ИНФОРМАЦИЯ
    {ITT_EFUNC, "DS[JL", SCQuitGame, 0, MENU_NONE}			// ВЫХОД
};

static Menu_t MainMenu = {
    110 + ORIGWIDTH_DELTA, 56,
    DrawMainMenu,
    5, MainItems,
    0,
    MENU_NONE
};

static Menu_t MainMenu_Rus = {
    103 + ORIGWIDTH_DELTA, 56,
    DrawMainMenu,
    5, MainItems_Rus,
    0,
    MENU_NONE
};

static MenuItem_t EpisodeItems[] = {
    {ITT_EFUNC, "CITY OF THE DAMNED", SCEpisode, 1, MENU_NONE},
    {ITT_EFUNC, "HELL'S MAW", SCEpisode, 2, MENU_NONE},
    {ITT_EFUNC, "THE DOME OF D'SPARIL", SCEpisode, 3, MENU_NONE},
    {ITT_EFUNC, "THE OSSUARY", SCEpisode, 4, MENU_NONE},
    {ITT_EFUNC, "THE STAGNANT DEMESNE", SCEpisode, 5, MENU_NONE}
};

static MenuItem_t EpisodeItems_Rus[] = {
    {ITT_EFUNC, "UJHJL GHJRKZNS[", SCEpisode, 1, MENU_NONE},		// ГОРОД ПРОКЛЯТЫХ
    {ITT_EFUNC, "FLCRFZ ENHJ,F", SCEpisode, 2, MENU_NONE},			// АДСКАЯ УТРОБА
    {ITT_EFUNC, "REGJK L\"CGFHBKF", SCEpisode, 3, MENU_NONE},		// КУПОЛ Д'СПАРИЛА
    {ITT_EFUNC, "CRKTG", SCEpisode, 4, MENU_NONE},					// СКЛЕП
    {ITT_EFUNC, "PFCNJQYST DKFLTYBZ", SCEpisode, 5, MENU_NONE}		// ЗАСТОЙНЫЕ ВЛАДЕНИЯ 
};

static Menu_t EpisodeMenu = {
    80 + ORIGWIDTH_DELTA, 50,
    DrawEpisodeMenu,
    3, EpisodeItems,
    0,
    MENU_MAIN
};

static Menu_t EpisodeMenu_Rus = {
    55 + ORIGWIDTH_DELTA, 50,
    DrawEpisodeMenu,
    3, EpisodeItems_Rus,
    0,
    MENU_MAIN
};

static MenuItem_t FilesItems[] = {
    {ITT_EFUNC, "LOAD GAME", SCNetCheck, 2, MENU_LOAD},
    {ITT_SETMENU, "SAVE GAME", NULL, 0, MENU_SAVE}
};

static MenuItem_t FilesItems_Rus[] = {
    {ITT_EFUNC, "PFUHEPBNM BUHE", SCNetCheck, 2, MENU_LOAD},	// ЗАГРУЗИТЬ ИГРУ
    {ITT_SETMENU, "CJ[HFYBNM BUHE", NULL, 0, MENU_SAVE}			// СОХРАНИТЬ ИГРУ
};

static Menu_t FilesMenu = {
    110 + ORIGWIDTH_DELTA, 60,
    DrawFilesMenu,
    2, FilesItems,
    0,
    MENU_MAIN
};

static Menu_t FilesMenu_Rus = {
    90 + ORIGWIDTH_DELTA, 60,
    DrawFilesMenu,
    2, FilesItems_Rus,
    0,
    MENU_MAIN
};

static MenuItem_t LoadItems[] = {
    {ITT_EFUNC, NULL, SCLoadGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 5, MENU_NONE}
};

static Menu_t LoadMenu = {
    70 + ORIGWIDTH_DELTA, 30,
    DrawLoadMenu,
    6, LoadItems,
    0,
    MENU_FILES
};

static Menu_t LoadMenu_Rus = {
    70 + ORIGWIDTH_DELTA, 30,
    DrawLoadMenu,
    6, LoadItems,
    0,
    MENU_FILES
};

static MenuItem_t SaveItems[] = {
    {ITT_EFUNC, NULL, SCSaveGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 5, MENU_NONE}
};

static Menu_t SaveMenu = {
    70 + ORIGWIDTH_DELTA, 30,
    DrawSaveMenu,
    6, SaveItems,
    0,
    MENU_FILES
};

static Menu_t SaveMenu_Rus = {
    70 + ORIGWIDTH_DELTA, 30,
    DrawSaveMenu,
    6, SaveItems,
    0,
    MENU_FILES
};

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, "THOU NEEDETH A WET-NURSE", SCSkill, sk_baby, MENU_NONE},
    {ITT_EFUNC, "YELLOWBELLIES-R-US", SCSkill, sk_easy, MENU_NONE},
    {ITT_EFUNC, "BRINGEST THEM ONETH", SCSkill, sk_medium, MENU_NONE},
    {ITT_EFUNC, "THOU ART A SMITE-MEISTER", SCSkill, sk_hard, MENU_NONE},
    {ITT_EFUNC, "BLACK PLAGUE POSSESSES THEE", SCSkill, sk_nightmare, MENU_NONE},
    {ITT_EFUNC, "QUICKETH ART THEE, FOUL WRAITH", SCSkill, sk_ultranm, MENU_NONE} // [JN] Thanks to Jon Dowland for this :)
};

static MenuItem_t SkillItems_Rus[] = {
    {ITT_EFUNC, "YZYTXRF YFLJ,YF VYT", SCSkill, sk_baby, MENU_NONE},    // НЯНЕЧКА НАДОБНА МНЕ
    {ITT_EFUNC, "YT CNJKM VE;TCNDTYTY Z", SCSkill, sk_easy, MENU_NONE}, // НЕ СТОЛЬ МУЖЕСТВЕНЕН Я
    {ITT_EFUNC, "GJLFQNT VYT B[", SCSkill, sk_medium, MENU_NONE},       // ПОДАЙТЕ МНЕ ИХ
    {ITT_EFUNC, "BCREITY Z CHF;TYBZVB", SCSkill, sk_hard, MENU_NONE},   // ИСКУШЕН Я СРАЖЕНИЯМИ
    {ITT_EFUNC, "XEVF JDKFLTKF VYJQ", SCSkill, sk_nightmare, MENU_NONE},// ЧУМА ОВЛАДЕЛА МНОЙ
    {ITT_EFUNC, "RJIVFHJV BCGJKYTY Z", SCSkill, sk_ultranm, MENU_NONE}  // КОШМАРОМ ИСПОЛНЕН Я
};

static Menu_t SkillMenu = {
    38 + ORIGWIDTH_DELTA, 30,
    DrawSkillMenu,
    6, SkillItems,
    2,
    MENU_EPISODE
};

static Menu_t SkillMenu_Rus = {
    38 + ORIGWIDTH_DELTA, 30,
    DrawSkillMenu,
    6, SkillItems_Rus,
    2,
    MENU_EPISODE
};


// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "RENDERING",         NULL,                0, MENU_RENDERING},
    {ITT_SETMENU, "DISPLAY",           NULL,                0, MENU_DISPLAY  },
    {ITT_SETMENU, "SOUND",             NULL,                0, MENU_SOUND    },
    {ITT_SETMENU, "CONTROLS",          NULL,                0, MENU_CONTROLS },
    {ITT_SETMENU, "GAMEPLAY",          NULL,                0, MENU_GAMEPLAY1},
//  {ITT_EFUNC,   "END GAME",          M_RD_EndGame,        0, MENU_NONE     },
    {ITT_EFUNC,   "RESET SETTINGS",    M_RD_ResetSettings,  0, MENU_NONE     },
    {ITT_EFUNC,   "LANGUAGE: ENGLISH", M_RD_ChangeLanguage, 0, MENU_NONE     }
};

static MenuItem_t OptionsItems_Rus[] = {
    {ITT_SETMENU, "DBLTJ",             NULL,                0, MENU_RENDERING},  // ВИДЕО
    {ITT_SETMENU, "\'RHFY",            NULL,                0, MENU_DISPLAY  },  // ЭКРАН
    {ITT_SETMENU, "FELBJ",             NULL,                0, MENU_SOUND    },  // АУДИО
    {ITT_SETMENU, "EGHFDKTYBT",        NULL,                0, MENU_CONTROLS },  // УПРАВЛЕНИЕ
    {ITT_SETMENU, "UTQVGKTQ",          NULL,                0, MENU_GAMEPLAY1},  // ГЕЙМПЛЕЙ
//  {ITT_EFUNC,   "PFRJYXBNM BUHE",    M_RD_EndGame,        0, MENU_NONE     },  // ЗАКОНЧИТЬ ИГРУ
    {ITT_EFUNC,   "C,HJC YFCNHJTR",    M_RD_ResetSettings,  0, MENU_NONE     },  // СБРОС НАСТРОЕК
    {ITT_EFUNC,   "ZPSR: HECCRBQ",     M_RD_ChangeLanguage, 0, MENU_NONE     }   // ЯЗЫК: РУССКИЙ
};

static Menu_t OptionsMenu = {
    77 + ORIGWIDTH_DELTA, 16,
    NULL,
    7, OptionsItems,
    0,
    MENU_MAIN
};

static Menu_t OptionsMenu_Rus = {
    77 + ORIGWIDTH_DELTA, 16,
    NULL,
    7, OptionsItems_Rus,
    0,
    MENU_MAIN
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_LRFUNC, "VERTICAL SYNCHRONIZATION:",       M_RD_Change_VSync, 0, MENU_NONE},
    {ITT_LRFUNC, "FIX ASPECT RATIO:",               M_RD_AspectRatio,  0, MENU_NONE},
    {ITT_LRFUNC, "UNCAPPED FRAMERATE:",             M_RD_Uncapped,     0, MENU_NONE},
    {ITT_LRFUNC, "SHOW FPS COUNTER:",               M_RD_FPScounter,   0, MENU_NONE},
    {ITT_LRFUNC, "PIXEL SCALING:",                  M_RD_Smoothing,    0, MENU_NONE},
    {ITT_LRFUNC, "VIDEO RENDERER:",                 M_RD_Renderer,     0, MENU_NONE},
    {ITT_EMPTY,  NULL,                              NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "SCREENSHOT FORMAT:",              M_RD_Screenshots,  0, MENU_NONE}
};

static MenuItem_t RenderingItems_Rus[] = {
    {ITT_LRFUNC, "DTHNBRFKMYFZ CBY[HJYBPFWBZ:",     M_RD_Change_VSync, 0, MENU_NONE}, // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    {ITT_LRFUNC, "ABRCBHJDFNM CJJNYJITYBT CNJHJY:", M_RD_AspectRatio,  0, MENU_NONE}, // ФИКСИРОВАТЬ СООТНОШЕНИЕ СТОРОН
    {ITT_LRFUNC, "JUHFYBXTYBT RFLHJDJQ XFCNJNS:",   M_RD_Uncapped,     0, MENU_NONE}, // ОГРАНИЧЕНИЕ КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "CXTNXBR RFLHJDJQ XFCNJNS:",       M_RD_FPScounter,   0, MENU_NONE}, // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing,    0, MENU_NONE}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_LRFUNC, "J,HF,JNRF DBLTJ:",                M_RD_Renderer,     0, MENU_NONE}, // ОБРАБОТКА ВИДЕО
    {ITT_EMPTY,  NULL,                              NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "AJHVFN CRHBYIJNJD:",              M_RD_Screenshots,  0, MENU_NONE}  // ФОРМАТ СКРИНШОТОВ
};

static Menu_t RenderingMenu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawRenderingMenu,
    8, RenderingItems,
    0,
    MENU_OPTIONS
};

static Menu_t RenderingMenu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawRenderingMenu,
    8, RenderingItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_LRFUNC, "SCREEN SIZE",              M_RD_ScreenSize,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "GAMMA-CORRECTION",         M_RD_Gamma,        0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE},
    {ITT_EFUNC,  "LOCAL TIME:",              M_RD_LocalTime,    0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE}, 
    {ITT_EFUNC,  "MESSAGES:",                M_RD_Messages,     0, MENU_NONE},
    {ITT_EFUNC,  "TEXT CASTS SHADOWS:",      M_RD_ShadowedText, 0, MENU_NONE}
};

static MenuItem_t DisplayItems_Rus[] = {
    {ITT_LRFUNC, "HFPVTH BUHJDJUJ \'RHFYF",  M_RD_ScreenSize,   0, MENU_NONE}, // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE}, // 
    {ITT_LRFUNC, "EHJDTYM UFVVF-RJHHTRWBB",  M_RD_Gamma,        0, MENU_NONE}, // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE}, //
    {ITT_EFUNC,  "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,    0, MENU_NONE}, // СИСТЕМНОЕ ВРЕМЯ
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE}, //
    {ITT_EFUNC,  "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,     0, MENU_NONE}, // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_EFUNC,  "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText, 0, MENU_NONE}  // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    
};

static Menu_t DisplayMenu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawDisplayMenu,
    8, DisplayItems,
    0,
    MENU_OPTIONS
};

static Menu_t DisplayMenu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawDisplayMenu,
    8, DisplayItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_LRFUNC, "SFX VOLUME",                 M_RD_SfxVolume,     0, MENU_NONE},
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME",               M_RD_MusVolume,     0, MENU_NONE},
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE},
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE},
    {ITT_LRFUNC, "SFX CHANNELS",               M_RD_SfxChannels,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE},
    {ITT_EFUNC,  "SFX MODE:",                  M_RD_SndMode,       0, MENU_NONE},
    {ITT_EFUNC,  "PITCH-SHIFTED SOUNDS:",      M_RD_PitchShifting, 0, MENU_NONE}
};

static MenuItem_t SoundItems_Rus[] = {
    {ITT_LRFUNC, "UHJVRJCNM PDERF",            M_RD_SfxVolume,     0, MENU_NONE}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_LRFUNC, "UHJVRJCNM VEPSRB",           M_RD_MusVolume,     0, MENU_NONE}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_LRFUNC, "PDERJDST RFYFKS",            M_RD_SfxChannels,   0, MENU_NONE}, // ЗВУКОВЫЕ КАНАЛЫ
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_EFUNC,  "HT;BV PDERF:",               M_RD_SndMode,       0, MENU_NONE}, // РЕЖИМ ЗВУКА
    {ITT_EFUNC,  "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting, 0, MENU_NONE}  // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
};

static Menu_t SoundMenu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawSoundMenu,
    9, SoundItems,
    0,
    MENU_OPTIONS
};

static Menu_t SoundMenu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawSoundMenu,
    9, SoundItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_EFUNC,  "ALWAYS RUN:",              M_RD_AlwaysRun,    0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "MOUSE SENSIVITY",          M_RD_Sensitivity,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,              0, MENU_NONE},
    {ITT_EFUNC,  "MOUSE LOOK:",              M_RD_MouseLook,    0, MENU_NONE},
    {ITT_EFUNC,  "VERTICAL MOVEMENT:",       M_RD_Novert,       0, MENU_NONE}
};

static MenuItem_t ControlsItems_Rus[] = {
    {ITT_EFUNC,  "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,   0, MENU_NONE}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_EMPTY,  NULL,                        NULL,             0, MENU_NONE},
    {ITT_LRFUNC, "CRJHJCNM VSIB",             M_RD_Sensitivity, 0, MENU_NONE}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,  NULL,                        NULL,             0, MENU_NONE},
    {ITT_EFUNC,  "J,PJH VSIM.:",              M_RD_MouseLook,   0, MENU_NONE}, // ОБЗОР МЫШЬЮ
    {ITT_EFUNC,  "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,      0, MENU_NONE}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawControlsMenu,
    6, ControlsItems,
    0,
    MENU_OPTIONS
};

static Menu_t ControlsMenu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawControlsMenu,
    6, ControlsItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay1Items[] = {
    {ITT_EFUNC,  "BRIGHTMAPS:",                  M_RD_Brightmaps,     0,  MENU_NONE      },
    {ITT_EFUNC,  "FAKE CONTRAST:",               M_RD_FakeContrast,   0,  MENU_NONE      },
    {ITT_EFUNC,  "COLORED HUD:",                 M_RD_ColoredHUD,     0,  MENU_NONE      },
    {ITT_EFUNC,  "COLORED BLOOD:",               M_RD_ColoredBlood,   0,  MENU_NONE      },
    {ITT_EFUNC,  "INVULNERABILITY AFFECTS SKY:", M_RD_InvulSky,       0,  MENU_NONE      },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE      },
    {ITT_EFUNC,  "SHOW LEVEL STATS ON AUTOMAP:", M_RD_AutoMapStats,   0,  MENU_NONE      },
    {ITT_EFUNC,  "NOTIFY OF REVEALED SECRETS:",  M_RD_SecretNotify,   0,  MENU_NONE      },
    {ITT_EFUNC,  "SHOW NEGATIVE HEALTH:",        M_RD_NegativeHealth, 0,  MENU_NONE      },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE      },
    {ITT_SETMENU,"NEXT PAGE...",                 NULL,                0,  MENU_GAMEPLAY2 }
};

static MenuItem_t Gameplay1Items_Rus[] = {
    {ITT_EFUNC,  ",HFQNVFGGBYU:",                M_RD_Brightmaps,     0,  MENU_NONE      }, // БРАЙТМАППИНГ
    {ITT_EFUNC,  "BVBNFWBZ RJYNHFCNYJCNB:",      M_RD_FakeContrast,   0,  MENU_NONE      }, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_EFUNC,  "HFPYJWDTNYST \'KTVTYNS $:",    M_RD_ColoredHUD,     0,  MENU_NONE      }, // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ HUD
    {ITT_EFUNC,  "HFPYJWDTNYFZ RHJDM:",          M_RD_ColoredBlood,   0,  MENU_NONE      }, // РАЗНОЦВЕТНАЯ КРОВЬ
    {ITT_EFUNC,  "YTEZPDBVJCNM JRHFIBDFTN YT,J:",M_RD_InvulSky,       0,  MENU_NONE      }, // НЕУЯЗВИМОСТЬ ОКРАШИВАЕТ НЕБО
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE      }, //
    {ITT_EFUNC,  "CNFNBCNBRF EHJDYZ YF RFHNT:",  M_RD_AutoMapStats,   0,  MENU_NONE      }, // СТАТИСТИКА УРОВНЯ НА КАРТЕ
    {ITT_EFUNC,  "CJJ,OFNM J YFQLTYYJV NFQYBRT:",M_RD_SecretNotify,   0,  MENU_NONE      }, // СООБЩАТЬ О НАЙДЕННОМ ТАЙНИКЕ
    {ITT_EFUNC,  "JNHBWFNTKMYJT PLJHJDMT D $:",  M_RD_NegativeHealth, 0,  MENU_NONE      }, // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ В HUD
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE      }, //
    {ITT_SETMENU,"CKTLE.OFZ CNHFYBWF>>>",        NULL,                0,  MENU_GAMEPLAY2 }  // СЛЕДУЮЩАЯ СТРАНИЦА
};

static Menu_t Gameplay1Menu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawGameplay1Menu,
    11, Gameplay1Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay1Menu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawGameplay1Menu,
    11, Gameplay1Items_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features (2)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay2Items[] = {
    {ITT_EFUNC,  "CORPSES SLIDING FROM LEDGES:", M_RD_Torque,         0,  MENU_NONE   },
    {ITT_EFUNC,  "WEAPON BOBBING WHILE FIRING:", M_RD_Bobbing,        0,  MENU_NONE   },
    {ITT_EFUNC,  "RANDOMLY MIRRORED CORPSES:",   M_RD_FlipCorpses,    0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EFUNC,  "DRAW CROSSHAIR:",              M_RD_CrossHairDraw,  0,  MENU_NONE   },
    {ITT_EFUNC,  "HEALTH INDICATION:",           M_RD_CrossHairHealth,0,  MENU_NONE   },
    {ITT_EFUNC,  "INCREASED SIZE:",              M_RD_CrossHairScale, 0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EFUNC,  "FLIP GAME LEVELS:",            M_RD_FlipLevels,     0,  MENU_NONE   },
    {ITT_EFUNC,  "PLAY INTERNAL DEMOS:",         M_RD_NoDemos,        0,  MENU_NONE   },
    {ITT_SETMENU,"PREVIOUS PAGE...",             NULL,                0,  MENU_GAMEPLAY1 }
};

static MenuItem_t Gameplay2Items_Rus[] = {
    {ITT_EFUNC,  "NHEGS CGJKPF.N C DJPDSITYBQ:", M_RD_Torque,         0,  MENU_NONE   },    // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_EFUNC,  "EKEXITYYJT GJRFXBDFYBT JHE;BZ:",M_RD_Bobbing,       0,  MENU_NONE   },    // УЛУЧШЕННОЕ ПОКАЧИВАНИЕ ОРУЖИЯ
    {ITT_EFUNC,  "PTHRFKBHJDFYBT NHEGJD:",       M_RD_FlipCorpses,    0,  MENU_NONE   },    // ЗЕКРАЛИРОВАНИЕ ТРУПОВ
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EFUNC,  "JNJ,HF;FNM GHBWTK:",           M_RD_CrossHairDraw,  0,  MENU_NONE   },    // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_EFUNC,  "BYLBRFWBZ PLJHJDMZ:",          M_RD_CrossHairHealth,0,  MENU_NONE   },    // ИНДИКАЦИЯ ЗДОРОВЬЯ
    {ITT_EFUNC,  "EDTKBXTYYSQ HFPVTH:",          M_RD_CrossHairScale, 0,  MENU_NONE   },    // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EFUNC,  "PTHRFKMYJT JNHF;TYBT EHJDYTQ:",M_RD_FlipLevels,     0,  MENU_NONE   },    // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_EFUNC,  "GHJBUHSDFNM LTVJPFGBCB:",      M_RD_NoDemos,        0,  MENU_NONE   },    // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_SETMENU,"GHTLSLEOFZ CNHFYBWF>>>",       NULL,                0,  MENU_GAMEPLAY1 }  // ПРЕДЫДУЩАЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawGameplay2Menu,
    11, Gameplay2Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay2Menu_Rus = {
    36 + ORIGWIDTH_DELTA, 42,
    DrawGameplay2Menu,
    11, Gameplay2Items_Rus,
    0,
    MENU_OPTIONS
};

static Menu_t *Menus[] = {
    &MainMenu,
    &EpisodeMenu,
    &SkillMenu,
    &OptionsMenu,
    &FilesMenu,
    &LoadMenu,
    &SaveMenu,
    &RenderingMenu,
    &DisplayMenu,
    &SoundMenu,
    &ControlsMenu,
    &Gameplay1Menu,
    &Gameplay2Menu
};

static Menu_t *Menus_Rus[] = {
    &MainMenu_Rus,
    &EpisodeMenu_Rus,
    &SkillMenu_Rus,
    &OptionsMenu_Rus,
    &FilesMenu_Rus,
    &LoadMenu_Rus,
    &SaveMenu_Rus,
    &RenderingMenu_Rus,
    &DisplayMenu_Rus,
    &SoundMenu_Rus,
    &ControlsMenu_Rus,
    &Gameplay1Menu_Rus,
    &Gameplay2Menu_Rus
};

static char *GammaText[] = {
    TXT_GAMMA_IMPROVED_OFF,
    TXT_GAMMA_IMPROVED_05,
    TXT_GAMMA_IMPROVED_1,
    TXT_GAMMA_IMPROVED_15,
    TXT_GAMMA_IMPROVED_2,
    TXT_GAMMA_IMPROVED_25,
    TXT_GAMMA_IMPROVED_3,
    TXT_GAMMA_IMPROVED_35,
    TXT_GAMMA_IMPROVED_4,
    TXT_GAMMA_ORIGINAL_OFF,
    TXT_GAMMA_ORIGINAL_05,
    TXT_GAMMA_ORIGINAL_1,
    TXT_GAMMA_ORIGINAL_15,
    TXT_GAMMA_ORIGINAL_2,
    TXT_GAMMA_ORIGINAL_25,
    TXT_GAMMA_ORIGINAL_3,
    TXT_GAMMA_ORIGINAL_35,
    TXT_GAMMA_ORIGINAL_4
};

static char *GammaText_Rus[] = {
    TXT_GAMMA_IMPROVED_OFF_RUS,
    TXT_GAMMA_IMPROVED_05_RUS,
    TXT_GAMMA_IMPROVED_1_RUS,
    TXT_GAMMA_IMPROVED_15_RUS,
    TXT_GAMMA_IMPROVED_2_RUS,
    TXT_GAMMA_IMPROVED_25_RUS,
    TXT_GAMMA_IMPROVED_3_RUS,
    TXT_GAMMA_IMPROVED_35_RUS,
    TXT_GAMMA_IMPROVED_4_RUS,
    TXT_GAMMA_ORIGINAL_OFF_RUS,
    TXT_GAMMA_ORIGINAL_05_RUS,
    TXT_GAMMA_ORIGINAL_1_RUS,
    TXT_GAMMA_ORIGINAL_15_RUS,
    TXT_GAMMA_ORIGINAL_2_RUS,
    TXT_GAMMA_ORIGINAL_25_RUS,
    TXT_GAMMA_ORIGINAL_3_RUS,
    TXT_GAMMA_ORIGINAL_35_RUS,
    TXT_GAMMA_ORIGINAL_4_RUS
};


//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    InitFonts();
    menuactive = false;
    messageson = true;
    SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

    if (gamemode == retail)
    {                           // Add episodes 4 and 5 to the menu
        EpisodeMenu.itemCount = EpisodeMenu_Rus.itemCount = 5;
        EpisodeMenu.y = EpisodeMenu_Rus.y -= ITEM_HEIGHT;
    }
}

//---------------------------------------------------------------------------
//
// PROC InitFonts
//
//---------------------------------------------------------------------------

static void InitFonts(void)
{
    // [JN] Original English fonts
    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
    FontBBaseLump = W_GetNumForName(DEH_String("FONTB_S")) + 1;

    // [JN] Small special font used for time/fps widget
    FontCBaseLump = W_GetNumForName(DEH_String("FONTC_S")) + 1;

    // [JN] Unchangable English fonts
    FontDBaseLump = W_GetNumForName(DEH_String("FONTD_S")) + 1;
    FontEBaseLump = W_GetNumForName(DEH_String("FONTE_S")) + 1;

    // [JN] Unchangable Russian fonts
    FontFBaseLump = W_GetNumForName(DEH_String("FONTF_S")) + 1;
    FontGBaseLump = W_GetNumForName(DEH_String("FONTG_S")) + 1;    
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextA
//
// Draw text using font A.
//
//---------------------------------------------------------------------------

void MN_DrTextA(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextAWidth
//
// Returns the pixel width of a string using font A.
//
//---------------------------------------------------------------------------

int MN_TextAWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextB
//
// Draw text using font B.
//
//---------------------------------------------------------------------------

void MN_DrTextB(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum((english_language ? FontBBaseLump : FontEBaseLump)
                                                 + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextBWidth
//
// Returns the pixel width of a string using font B.
//
//---------------------------------------------------------------------------

int MN_TextBWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextC
//
// [JN] Draw small time digits using font C.
//
//---------------------------------------------------------------------------

void MN_DrTextC(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        p = W_CacheLumpNum(FontCBaseLump + c - 33, PU_CACHE);
        V_DrawPatch(x, y, p);
        x += SHORT(p->width);
    }
}

//---------------------------------------------------------------------------
//
// MN_DrTextSmallENG
//
// [JN] Draw text string with unreplacable English font.
//
//---------------------------------------------------------------------------

void MN_DrTextSmallENG(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontDBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_DrTextSmallENGWidth
//
// [JN] Returns the pixel width of a string using font D.
//
//---------------------------------------------------------------------------

int MN_DrTextSmallENGWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontDBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// MN_DrTextBigENG
//
// [JN] Draw text string with unreplacable English font.
//
//---------------------------------------------------------------------------

void MN_DrTextBigENG(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(FontEBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_DrTextBigENGWidth
//
// [JN] Returns the pixel width of a string using font E.
//
//---------------------------------------------------------------------------

int MN_DrTextBigENGWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontEBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// MN_DrTextSmallRUS
//
// [JN] Draw text string with unreplacable English font.
//
//---------------------------------------------------------------------------

void MN_DrTextSmallRUS(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontFBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_DrTextSmallRUSWidth
//
// [JN] Returns the pixel width of a string using font F.
//
//---------------------------------------------------------------------------

int MN_DrTextSmallRUSWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontFBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// MN_DrTextBigRUS
//
// [JN] Draw text string with unreplacable English font.
//
//---------------------------------------------------------------------------

void MN_DrTextBigRUS(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(FontGBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_DrTextBigRUSWidth
//
// [JN] Returns the pixel width of a string using font G.
//
//---------------------------------------------------------------------------

int MN_DrTextBigRUSWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontGBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_Ticker
//
//---------------------------------------------------------------------------

void MN_Ticker(void)
{
    if (menuactive == false)
    {
        return;
    }
    MenuTime++;
}

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

char *QuitEndMsg[] = {
    "ARE YOU SURE YOU WANT TO QUIT?",
    "ARE YOU SURE YOU WANT TO END THE GAME?",
    "DO YOU WANT TO QUICKSAVE THE GAME NAMED",
    "DO YOU WANT TO QUICKLOAD THE GAME NAMED",
    "RESET SETTINGS TO THEIR DEFAULTS?",
};

char *QuitEndMsg_Rus[] = {
    "DS LTQCNDBNTKMYJ ;TKFTNT DSQNB?",          // ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ВЫЙТИ?
    "DS LTQCNDBNTKMYJ ;TKFTNT PFRJYXBNM BUHE?", // ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ЗАКОНЧИТЬ ИГРУ?
    "DSGJKYBNM ,SCNHJT CJ[HFYTYBT BUHS:",       // ВЫПОЛНИТЬ БЫСТРОЕ СОХРАНЕНИЕ ИГРЫ:
    "DSGJKYBNM ,SCNHE. PFUHEPRE BUHS:",         // ВЫПОЛНИТЬ БЫСТРУЮ ЗАГРУЗКУ ИГРЫ:
    "C,HJCBNM YFCNHJQRB YF CNFYLFHNYST PYFXTYBZ?" // СБРОСИТЬ НАСТРОЙКИ НА СТАНДАРТНЫЕ ЗНАЧЕНИЯ?
};

void MN_Drawer(void)
{
    int i;
    int x;
    int y;
    MenuItem_t *item;
    char *message;
    char *selName;

    if (menuactive == false)
    {
        if (askforquit)
        {
            message = DEH_String(english_language ?
                                 QuitEndMsg[typeofask - 1] :
                                 QuitEndMsg_Rus[typeofask - 1]);

            if (english_language)
            MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2 + ORIGWIDTH_DELTA, 80);
            else
            MN_DrTextSmallRUS(message, 160 - MN_DrTextSmallRUSWidth(message) / 2 + ORIGWIDTH_DELTA, 80);

            if (typeofask == 3)
            {
                MN_DrTextA(SlotText[quicksave - 1], 160 -
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2 + ORIGWIDTH_DELTA, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2 + ORIGWIDTH_DELTA, 90);
            }
            if (typeofask == 4)
            {
                MN_DrTextA(SlotText[quickload - 1], 160 -
                           MN_TextAWidth(SlotText[quickload - 1]) / 2 + ORIGWIDTH_DELTA, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quickload - 1]) / 2 + ORIGWIDTH_DELTA, 90);
            }
            UpdateState |= I_FULLSCRN;
        }
        return;
    }
    else
    {
        UpdateState |= I_FULLSCRN;
        if (InfoType)
        {
            MN_DrawInfo();
            return;
        }
        if (screenblocks < 10)
        {
            BorderNeedRefresh = true;
        }
        if (CurrentMenu->drawFunc != NULL)
        {
            CurrentMenu->drawFunc();
        }
        x = CurrentMenu->x;
        y = CurrentMenu->y;
        item = CurrentMenu->items;
        for (i = 0; i < CurrentMenu->itemCount; i++)
        {
            if (item->type != ITT_EMPTY && item->text)
            {
                // [JN] Define where to use big and where small fonts,
                // and where to use big or small vertical spacing.
                if (english_language)
                {
                    if (CurrentMenu == &MainMenu
                    ||  CurrentMenu == &EpisodeMenu
                    ||  CurrentMenu == &FilesMenu
                    ||  CurrentMenu == &SkillMenu
                    ||  CurrentMenu == &OptionsMenu)
                    {
                        MN_DrTextBigENG(DEH_String(item->text), x, y);
                    }
                    else
                    {
                        MN_DrTextSmallENG(DEH_String(item->text), x, y);
                    }
                }
                else
                {
                    if (CurrentMenu == &MainMenu_Rus
                    ||  CurrentMenu == &EpisodeMenu_Rus
                    ||  CurrentMenu == &FilesMenu_Rus
                    ||  CurrentMenu == &SkillMenu_Rus
                    ||  CurrentMenu == &OptionsMenu_Rus)
                    {
                        MN_DrTextBigRUS(DEH_String(item->text), x, y);
                    }
                    else
                    {
                        MN_DrTextSmallRUS(DEH_String(item->text), x, y);
                    }
                }
            }

            if (CurrentMenu == &MainMenu
            ||  CurrentMenu == &EpisodeMenu
            ||  CurrentMenu == &FilesMenu
            ||  CurrentMenu == &SkillMenu
            ||  CurrentMenu == &OptionsMenu
            ||  CurrentMenu == &MainMenu_Rus
            ||  CurrentMenu == &EpisodeMenu_Rus
            ||  CurrentMenu == &FilesMenu_Rus
            ||  CurrentMenu == &SkillMenu_Rus
            ||  CurrentMenu == &OptionsMenu_Rus)
            {
                y += ITEM_HEIGHT;
            }
            else
            {
                y += ITEM_HEIGHT_SMALL;
            }

            item++;
        }

        // [JN] Define where to draw big arrow and where small arrow.
        if (CurrentMenu == &MainMenu
        ||  CurrentMenu == &EpisodeMenu
        ||  CurrentMenu == &FilesMenu
        ||  CurrentMenu == &LoadMenu
        ||  CurrentMenu == &SaveMenu
        ||  CurrentMenu == &SkillMenu
        ||  CurrentMenu == &OptionsMenu
        ||  CurrentMenu == &MainMenu_Rus
        ||  CurrentMenu == &EpisodeMenu_Rus
        ||  CurrentMenu == &FilesMenu_Rus
        ||  CurrentMenu == &SkillMenu_Rus
        ||  CurrentMenu == &LoadMenu_Rus
        ||  CurrentMenu == &SaveMenu_Rus
        ||  CurrentMenu == &OptionsMenu_Rus)        
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET, y,
                                     W_CacheLumpName(selName, PU_CACHE));
        }
        else
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT_SMALL) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 8 ? "INVGEMR1" : "INVGEMR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET_SMALL, y,
                                     W_CacheLumpName(selName, PU_CACHE));
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMainMenu
//
//---------------------------------------------------------------------------

static void DrawMainMenu(void)
{
    int frame;

    frame = (MenuTime / 3) % 18;
    V_DrawShadowedPatchRaven(88 + ORIGWIDTH_DELTA, 0, W_CacheLumpName(DEH_String("M_HTIC"), PU_CACHE));
    V_DrawShadowedPatchRaven(40 + ORIGWIDTH_DELTA, 10, W_CacheLumpNum(SkullBaseLump + (17 - frame),
                                       PU_CACHE));
    V_DrawShadowedPatchRaven(232 + ORIGWIDTH_DELTA, 10, W_CacheLumpNum(SkullBaseLump + frame, PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawEpisodeMenu
//
//---------------------------------------------------------------------------

static void DrawEpisodeMenu(void)
{
}

//---------------------------------------------------------------------------
//
// PROC DrawSkillMenu
//
//---------------------------------------------------------------------------

static void DrawSkillMenu(void)
{
}

//---------------------------------------------------------------------------
//
// PROC DrawFilesMenu
//
//---------------------------------------------------------------------------

static void DrawFilesMenu(void)
{
// clear out the quicksave/quickload stuff
    quicksave = 0;
    quickload = 0;
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageTics = 1;
}

//---------------------------------------------------------------------------
//
// PROC DrawLoadMenu
//
//---------------------------------------------------------------------------

static void DrawLoadMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("LOAD GAME");
    title_rus = DEH_String("PFUHEPBNM BUHE");   // ЗАГРУЗИТЬ ИГРУ

    if (english_language)
    MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + ORIGWIDTH_DELTA, 7);
    else
    MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + ORIGWIDTH_DELTA, 7);
    
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&LoadMenu);
}

//---------------------------------------------------------------------------
//
// PROC DrawSaveMenu
//
//---------------------------------------------------------------------------

static void DrawSaveMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("SAVE GAME");
    title_rus = DEH_String("CJ[HFYBNM BUHE");   // СОХРАНИТЬ ИГРУ

    if (english_language)
    MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + ORIGWIDTH_DELTA, 7);
    else
    MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + ORIGWIDTH_DELTA, 7);

    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&SaveMenu);
}

//===========================================================================
//
// MN_LoadSlotText
//
//              Loads in the text message for each slot
//===========================================================================

void MN_LoadSlotText(void)
{
    FILE *fp;
    int i;
    char *filename;

    for (i = 0; i < 6; i++)
    {
        int retval;
        filename = SV_Filename(i);
        fp = fopen(filename, "rb+");
	free(filename);

        if (!fp)
        {
            SlotText[i][0] = 0; // empty the string
            SlotStatus[i] = 0;
            continue;
        }
        retval = fread(&SlotText[i], SLOTTEXTLEN, 1, fp);
        fclose(fp);
        SlotStatus[i] = retval = 1;
    }
    slottextloaded = true;
}

//---------------------------------------------------------------------------
//
// PROC DrawFileSlots
//
//---------------------------------------------------------------------------

static void DrawFileSlots(Menu_t * menu)
{
    int i;
    int x;
    int y;

    x = menu->x;
    y = menu->y;
    for (i = 0; i < 6; i++)
    {
        V_DrawShadowedPatchRaven(x, y, W_CacheLumpName(DEH_String("M_FSLOT"), PU_CACHE));
        if (SlotStatus[i])
        {
            // [JN] Use only small English chars here
            MN_DrTextSmallENG(SlotText[i], x + 5, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}


// -----------------------------------------------------------------------------
// DrawRenderingMenu
// -----------------------------------------------------------------------------

static void DrawRenderingMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("RENDERING OPTIONS");
    title_rus = DEH_String("YFCNHJQRB DBLTJ");  // НАСТРОЙКИ ВИДЕО

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        // Subheaders
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("RENDERING"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("EXTRA"), 36 + ORIGWIDTH_DELTA, 102);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        // Subheaders
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("HTYLTHBYU"), 36 + ORIGWIDTH_DELTA, 32);      // РЕНДЕРИНГ
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + ORIGWIDTH_DELTA, 102);  // ДОПОЛНИТЕЛЬНО
        dp_translation = NULL;
    }


    // - Vertical sync ---------------------------------------------------------
    if (vsync)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 216 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 236 + ORIGWIDTH_DELTA, 42);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 216 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 236 + ORIGWIDTH_DELTA, 42);
    }

    // - Fix aspect ratio ------------------------------------------------------
    if (aspect_ratio_correct)
    {
#ifdef WIDESCREEN
        MN_DrTextSmallENG(DEH_String("16:9"), (english_language ? 156 : 270) 
                                            + ORIGWIDTH_DELTA, 52);
#else
        MN_DrTextSmallENG(DEH_String("4:3"), (english_language ? 156 : 270)
                                           + ORIGWIDTH_DELTA, 52);
#endif
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 156 + ORIGWIDTH_DELTA, 52);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 270 + ORIGWIDTH_DELTA, 52);
    }

    // - Uncapped FPS ----------------------------------------------------------
    if (uncapped_fps)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 184 + ORIGWIDTH_DELTA, 62);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 254 + ORIGWIDTH_DELTA, 62);
    }
    else
    {
        MN_DrTextSmallENG(DEH_String("35 FPS"), (english_language ? 184 : 254) 
                                              + ORIGWIDTH_DELTA, 62);
    }

    // - FPS counter -----------------------------------------------------------
    if (show_fps)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 165 + ORIGWIDTH_DELTA, 72);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 223 + ORIGWIDTH_DELTA, 72);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 165 + ORIGWIDTH_DELTA, 72);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 223 + ORIGWIDTH_DELTA, 72);
    }

    // - Pixel scaling ---------------------------------------------------------
    if (smoothing)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("SMOOTH"), 131 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 211 + ORIGWIDTH_DELTA, 82);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("SHARP"), 131 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 211 + ORIGWIDTH_DELTA, 82);
    }

    // - Video renderer --------------------------------------------------------
    if (force_software_renderer)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("SOFTWARE (CPU)"), 149 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("GHJUHFVVYFZ"), 159 + ORIGWIDTH_DELTA, 92);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("HARDWARE (GPU)"), 149 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("FGGFHFNYFZ"), 159 + ORIGWIDTH_DELTA, 92);
    }

    // - Screenshot format --------------------------------------------------------
    if (png_screenshots)
    {
        MN_DrTextSmallENG(DEH_String("PNG"), (english_language ? 175 : 176)
                                           + ORIGWIDTH_DELTA, 112);
    }
    else
    {
        MN_DrTextSmallENG(DEH_String("PCX"), (english_language ? 175 : 176)
                                           + ORIGWIDTH_DELTA, 112);
    }   
}

static boolean M_RD_Change_VSync(int option)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
    return false;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    return true;
}

static boolean M_RD_AspectRatio(int option)
{
    aspect_ratio_correct ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Update status bar
    SB_state = -1;

    return true;
}

static boolean M_RD_Uncapped(int option)
{
    uncapped_fps ^= 1;
    return true;
}

static boolean M_RD_FPScounter(int option)
{
    show_fps ^= 1;
    return true;
}

static boolean M_RD_Smoothing(int option)
{
    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Update status bar
    SB_state = -1;

    return true;
}

static boolean M_RD_Renderer(int option)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update status bar
    SB_state = -1;

    return true;
}

static boolean M_RD_Screenshots(int option)
{
    png_screenshots ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = DEH_String("DISPLAY OPTIONS");
    title_rus = DEH_String("YFCNHJQRB \'RHFYF");  // НАСТРОЙКИ ЭКРАНА

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("SCREEN"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("MESSAGES AND TEXTS"), 36 + ORIGWIDTH_DELTA, 92);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("\'RHFY"), 36 + ORIGWIDTH_DELTA, 32);              // ЭКРАН
        MN_DrTextSmallRUS(DEH_String("CJJ,OTYBZ B NTRCNS"), 36 + ORIGWIDTH_DELTA, 92);  // СООБЩЕНИЯ И ТЕКСТЫ
        dp_translation = NULL;
    }

    // Screen size
#ifdef WIDESCREEN
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 4, screenblocks - 9);
    M_snprintf(num, 4, "%3d", screenblocks);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 85 + ORIGWIDTH_DELTA, 52);
    dp_translation = NULL;
#else
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 10, screenblocks - 3);
    M_snprintf(num, 4, "%3d", screenblocks);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, (english_language ? 208 : 202) + ORIGWIDTH_DELTA, 41);
    dp_translation = NULL;
#endif

    // Gamma-correction
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 3, 18, usegamma);

    // Local time:
    if (local_time)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 110 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 157 + ORIGWIDTH_DELTA, 82);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 110 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 157 + ORIGWIDTH_DELTA, 82);
    }

    // Messages:
    if (messageson)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 108 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 208 + ORIGWIDTH_DELTA, 102);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 108 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 208 + ORIGWIDTH_DELTA, 102);
    }

    // Text casting shadows:
    if (draw_shadowed_text)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 179 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 220 + ORIGWIDTH_DELTA, 112);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 179 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 220 + ORIGWIDTH_DELTA, 112);
    }
}

static boolean M_RD_ScreenSize(int option)
{
    if (option == RIGHT_DIR)
    {
        if (screenblocks < 12) // [JN] Now we have 12 screen sizes
        {
            screenblocks++;
        }
    }
    else if (screenblocks > 3)
    {
        screenblocks--;
    }

#ifdef WIDESCREEN
    // [JN] Wide screen: don't allow unsupported (bordered) views
    // screenblocks - config file variable
    if (screenblocks < 9)
        screenblocks = 9;
    if (screenblocks > 12)
        screenblocks = 12;
#endif

    R_SetViewSize(screenblocks, detailLevel);
    return true;
}

static boolean M_RD_Gamma(int option)
{
    switch(option)
    {
        case 0:
        if (usegamma > 0) 
            usegamma--;
        break;

        case 1:
        if (usegamma < 17) 
            usegamma++;
        break;
    }

    I_SetPalette((byte *) W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE));

    P_SetMessage(&players[consoleplayer], english_language ? 
                                          GammaText[usegamma] :
                                          GammaText_Rus[usegamma],
                                          false);
    return true;
}

static boolean M_RD_LocalTime(int option)
{
    local_time ^= 1;
    return true;
}

static boolean M_RD_Messages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], DEH_String(english_language ?
                      "MESSAGES ON" :
                      "CJJ,OTYBZ DRK.XTYS"), // СООБЩЕНИЯ ВКЛЮЧЕНЫ
                      true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], DEH_String(english_language ?
                      "MESSAGES OFF" :
                      "CJJ,OTYBZ DSRK.XTYS"), // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
                      true);
    }
    S_StartSound(NULL, sfx_chat);
    return true;
}

static boolean M_RD_ShadowedText(int option)
{
    draw_shadowed_text ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = DEH_String("SOUND OPTIONS");
    title_rus = DEH_String("YFCNHJQRB PDERF");  // НАСТРОЙКИ ЗВУКА

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("VOLUME"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("EXTRA"), 36 + ORIGWIDTH_DELTA, 82);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHJVRJCNM"), 36 + ORIGWIDTH_DELTA, 32);       // ГРОМКОСТЬ
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + ORIGWIDTH_DELTA, 82);   // ДОПОЛНИТЕЛЬНО
        dp_translation = NULL;
    }

    // SFX Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 1, 16, snd_MaxVolume);
    M_snprintf(num, 4, "%3d", snd_MaxVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + ORIGWIDTH_DELTA, 53);
    dp_translation = NULL;

    // Music Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 3, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + ORIGWIDTH_DELTA, 73);
    dp_translation = NULL;

    // SFX Channels
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 6, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%3d", snd_Channels);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + ORIGWIDTH_DELTA, 103);
    dp_translation = NULL;

    // SFX Mode
    if (snd_monomode)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("MONO"), 105 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("VJYJ"), 128 + ORIGWIDTH_DELTA, 112);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("STEREO"), 105 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("CNTHTJ"), 128 + ORIGWIDTH_DELTA, 112);
    }    

    // Pitch-Shifted sounds
    if (snd_pitchshift)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 189 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 230 + ORIGWIDTH_DELTA, 122);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 189 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 230 + ORIGWIDTH_DELTA, 122);
    }
}

static boolean M_RD_SfxVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MaxVolume < 15)
        {
            snd_MaxVolume++;
        }
    }
    else if (snd_MaxVolume)
    {
        snd_MaxVolume--;
    }
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu

    return true;
}

static boolean M_RD_MusVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MusicVolume < 15)
        {
            snd_MusicVolume++;
        }
    }
    else if (snd_MusicVolume)
    {
        snd_MusicVolume--;
    }
    S_SetMusicVolume();

    return true;
}

static boolean M_RD_SfxChannels(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_Channels < 64)
        {
            snd_Channels += 4;
        }
    }
    else if (snd_Channels > 4)
    {
        snd_Channels -= 4;
    }

    // Reallocate sound channels
    S_ChannelsRealloc();

    return true;
}

static boolean M_RD_SndMode(int option)
{
    snd_monomode ^= 1;
    return true;
}

static boolean M_RD_PitchShifting(int option)
{
    snd_pitchshift ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = DEH_String("CONTROL SETTINGS");
    title_rus = DEH_String("EGHFDKTYBT");  // УПРАВЛЕНИЕ

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MOVEMENT"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("MOUSE"), 36 + ORIGWIDTH_DELTA, 52);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("GTHTLDB;TYBT"), 36 + ORIGWIDTH_DELTA, 32);    // ПЕРЕДВИЖЕНИЕ
        MN_DrTextSmallRUS(DEH_String("VSIM"), 36 + ORIGWIDTH_DELTA, 52);            // МЫШЬ
        dp_translation = NULL;
    }

    // Always run
    if (joybspeed >= 20)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 118 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 209 + ORIGWIDTH_DELTA, 42);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 118 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 209 + ORIGWIDTH_DELTA, 42);
    }

    // Mouse sensivity
    DrawSliderSmall((english_language ? &ControlsMenu : &ControlsMenu_Rus), 3, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + ORIGWIDTH_DELTA, 73);
    dp_translation = NULL;

    // Mouse look
    if (mlook)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 118 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 132 + ORIGWIDTH_DELTA, 82);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 118 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 132 + ORIGWIDTH_DELTA, 82);
    }

    // Novert
    if (!novert)
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 168 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 227 + ORIGWIDTH_DELTA, 92);
    }
    else
    {
        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 168 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 227 + ORIGWIDTH_DELTA, 92);
    }
}

static boolean M_RD_AlwaysRun(int option)
{
    static int joybspeed_old = 2;

    if (joybspeed >= 20)
    {
        joybspeed = joybspeed_old;
    }
    else
    {
        joybspeed_old = joybspeed;
        joybspeed = 29;
    }

    return true;
}

static boolean M_RD_Sensitivity(int option)
{
    if (option == RIGHT_DIR)
    {
        if (mouseSensitivity < 255) // [crispy] extended range
        {
            mouseSensitivity++;
        }
    }
    else if (mouseSensitivity)
    {
        mouseSensitivity--;
    }
    return true;
}

static boolean M_RD_MouseLook(int option)
{
    mlook ^= 1;
    if (!mlook)
    players[consoleplayer].centering = true;
    return true;
}

static boolean M_RD_Novert(int option)
{
    novert ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawGameplay1Menu
// -----------------------------------------------------------------------------

static void DrawGameplay1Menu(void)
{

    static char *title_eng, *title_rus;

    title_eng = DEH_String("GAMEPLAY FEATURES");
    title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("VISUAL"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("TACTICAL"), 36 + ORIGWIDTH_DELTA, 92);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHFABRF"), 36 + ORIGWIDTH_DELTA, 32); // ГРАФИКА
        MN_DrTextSmallRUS(DEH_String("NFRNBRF"), 36 + ORIGWIDTH_DELTA, 92); // ТАКТИКА
        dp_translation = NULL;
    }

    // Brightmaps
    if (brightmaps)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 119 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 133 + ORIGWIDTH_DELTA, 42);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 119 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 133 + ORIGWIDTH_DELTA, 42);

        dp_translation = NULL;
    }

    // Fake contrast
    if (fake_contrast)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 143 + ORIGWIDTH_DELTA, 52);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 205 + ORIGWIDTH_DELTA, 52);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 143 + ORIGWIDTH_DELTA, 52);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 205 + ORIGWIDTH_DELTA, 52);

        dp_translation = NULL;
    }

    // Colored HUD
    if (colored_hud)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 126 + ORIGWIDTH_DELTA, 62);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 235 + ORIGWIDTH_DELTA, 62);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 126 + ORIGWIDTH_DELTA, 62);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 235 + ORIGWIDTH_DELTA, 62);

        dp_translation = NULL;
    }


    // Colored blood
    if (colored_blood)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 139 + ORIGWIDTH_DELTA, 72);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 178 + ORIGWIDTH_DELTA, 72);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 139 + ORIGWIDTH_DELTA, 72);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 178 + ORIGWIDTH_DELTA, 72);

        dp_translation = NULL;
    }

    // Invulnerability affects sky
    if (invul_sky)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 235 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 253 + ORIGWIDTH_DELTA, 82);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 235 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 253 + ORIGWIDTH_DELTA, 82);

        dp_translation = NULL;
    }

    // Show level stats on automap
    if (automap_stats)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 240 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 231 + ORIGWIDTH_DELTA, 102);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 240 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 231 + ORIGWIDTH_DELTA, 102);

        dp_translation = NULL;
    }

    // Notify of revealed secrets
    if (secret_notification)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 235 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 251 + ORIGWIDTH_DELTA, 112);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 235 + ORIGWIDTH_DELTA, 112);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 251 + ORIGWIDTH_DELTA, 112);

        dp_translation = NULL;
    }

    // Negative health
    if (negative_health)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 190 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 253 + ORIGWIDTH_DELTA, 122);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 190 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 253 + ORIGWIDTH_DELTA, 122);

        dp_translation = NULL;
    }
}

static boolean M_RD_Brightmaps(int option)
{
    brightmaps ^= 1;
    return true;
}

static boolean M_RD_FakeContrast(int option)
{
    fake_contrast ^= 1;
    return true;
}

static boolean M_RD_ColoredHUD(int option)
{
    colored_hud ^= 1;
    SB_state = -1;      // Update status bar (JN: not sure is it needed)
    return true;
}

static boolean M_RD_ColoredBlood(int option)
{
    colored_blood ^= 1;
    return true;
}

static boolean M_RD_InvulSky(int option)
{
    invul_sky ^= 1;
    return true;
}

static boolean M_RD_AutoMapStats(int option)
{
    automap_stats ^= 1;
    return true;
}

static boolean M_RD_SecretNotify(int option)
{
    secret_notification ^= 1;
    return true;
}

static boolean M_RD_NegativeHealth(int option)
{
    negative_health ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawGameplay2Menu
// -----------------------------------------------------------------------------

static void DrawGameplay2Menu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("GAMEPLAY FEATURES");
    title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("PHYSICAL"), 36 + ORIGWIDTH_DELTA, 32);
        MN_DrTextSmallENG(DEH_String("CROSSHAIR"), 36 + ORIGWIDTH_DELTA, 72);
        MN_DrTextSmallENG(DEH_String("GAMEPLAY"), 36 + ORIGWIDTH_DELTA, 112);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + ORIGWIDTH_DELTA, 7);

        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHFABRF"), 36 + ORIGWIDTH_DELTA, 32);    // ФИЗИКА
        MN_DrTextSmallRUS(DEH_String("NFRNBRF"), 36 + ORIGWIDTH_DELTA, 72);    // ПРИЦЕЛ
        MN_DrTextSmallRUS(DEH_String("UTQVGKTQ"), 36 + ORIGWIDTH_DELTA, 112);  // ГЕЙМПЛЕЙ
        dp_translation = NULL;
    }

    // Corpses sliding from the ledges
    if (torque)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 238 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 253 + ORIGWIDTH_DELTA, 42);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 238 + ORIGWIDTH_DELTA, 42);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 253 + ORIGWIDTH_DELTA, 42);

        dp_translation = NULL;
    }

    // Weapon bobbing while firing
    if (weapon_bobbing)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 233 + ORIGWIDTH_DELTA, 52);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 260 + ORIGWIDTH_DELTA, 52);

        dp_translation = NULL;
    }
    else 
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 233 + ORIGWIDTH_DELTA, 52);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 260 + ORIGWIDTH_DELTA, 52);

        dp_translation = NULL;
    }

    // Randomly flipped corpses
    if (randomly_flipcorpses)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 232 + ORIGWIDTH_DELTA, 62);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 201 + ORIGWIDTH_DELTA, 62);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 232 + ORIGWIDTH_DELTA, 62);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 201 + ORIGWIDTH_DELTA, 62);

        dp_translation = NULL;
    }

    // Draw crosshair
    if (crosshair_draw)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 150 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 175 + ORIGWIDTH_DELTA, 82);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 150 + ORIGWIDTH_DELTA, 82);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 175 + ORIGWIDTH_DELTA, 82);

        dp_translation = NULL;
    }

    // Health indication
    if (crosshair_health)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 161 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 179 + ORIGWIDTH_DELTA, 92);

        dp_translation = NULL;
    }
    else 
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 161 + ORIGWIDTH_DELTA, 92);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 179 + ORIGWIDTH_DELTA, 92);

        dp_translation = NULL;
    }

    // Increased size
    if (crosshair_scale)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 146 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 181 + ORIGWIDTH_DELTA, 102);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 146 + ORIGWIDTH_DELTA, 102);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 181 + ORIGWIDTH_DELTA, 102);

        dp_translation = NULL;
    }

    // Flip game levels
    if (flip_levels)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 153 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 255 + ORIGWIDTH_DELTA, 122);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 153 + ORIGWIDTH_DELTA, 122);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 255 + ORIGWIDTH_DELTA, 122);

        dp_translation = NULL;
    }

    // Don't play internal demos
    if (no_internal_demos)
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("OFF"), 179 + ORIGWIDTH_DELTA, 132);
        else
        MN_DrTextSmallRUS(DEH_String("DSRK"), 211 + ORIGWIDTH_DELTA, 132);

        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];

        if (english_language)
        MN_DrTextSmallENG(DEH_String("ON"), 179 + ORIGWIDTH_DELTA, 132);
        else
        MN_DrTextSmallRUS(DEH_String("DRK"), 211 + ORIGWIDTH_DELTA, 132);

        dp_translation = NULL;
    }
}

static boolean M_RD_Torque(int option)
{
    torque ^= 1;
    return true;
}

static boolean M_RD_Bobbing(int option)
{
    weapon_bobbing ^= 1;
    return true;
}

static boolean M_RD_FlipCorpses(int option)
{
    randomly_flipcorpses ^= 1;
    return true;
}

static boolean M_RD_CrossHairDraw(int option)
{
    crosshair_draw ^= 1;
    return true;
}

static boolean M_RD_CrossHairHealth(int option)
{
    crosshair_health ^= 1;
    return true;
}

static boolean M_RD_CrossHairScale(int option)
{
    crosshair_scale ^= 1;
    return true;
}

static boolean M_RD_FlipLevels(int option)
{
    extern void R_ExecuteSetViewSize();

    flip_levels ^= 1;

    // [JN] Redraw game screen
    R_ExecuteSetViewSize();

    return true;
}

static boolean M_RD_NoDemos(int option)
{
    no_internal_demos ^= 1;
    return true;
}

//---------------------------------------------------------------------------
// M_RD_EndGame
//---------------------------------------------------------------------------

static boolean M_RD_EndGame(int option)
{
    if (demoplayback || netgame)
    {
        return false;
    }
    menuactive = false;
    askforquit = true;
    typeofask = 2;              //endgame
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

//---------------------------------------------------------------------------
// M_RD_ResetSettings
//---------------------------------------------------------------------------

static boolean M_RD_ResetSettings(int option)
{
    menuactive = false;
    askforquit = true;
    typeofask = 5;              // Reset settings to their defaults
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

void M_RD_DoResetSettings(void)
{
    // Rendering
    vsync                   = 1;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    show_fps                = 0;
    smoothing               = 0;
    force_software_renderer = 0;
    png_screenshots         = 1;

    // Display
    screenblocks        = 10;
    usegamma            = 0;
    local_time          = 0;
    messageson          = 1;
    draw_shadowed_text  = 1;

    // Audio
    snd_MaxVolume   = 8;
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu
    snd_MusicVolume = 8;
    S_SetMusicVolume();
    snd_Channels    = 32;
    S_ChannelsRealloc();
    snd_monomode    = 0;

    // Controls
    joybspeed           = 29;
    mouseSensitivity    = 5;
    mlook               = 0;
    players[consoleplayer].centering = true;
    novert              = 1;    

    // Gameplay (1)
    brightmaps          = 1;
    fake_contrast       = 0;
    colored_hud         = 0;
    colored_blood       = 1;
    invul_sky           = 1;
    automap_stats       = 1;
    secret_notification = 1;
    negative_health     = 0;

    // Gameplay (2)
    torque               = 1;
    weapon_bobbing       = 1;
    randomly_flipcorpses = 1;
    crosshair_draw       = 0;
    crosshair_health     = 1;
    crosshair_scale      = 0;
    flip_levels          = 0;
    no_internal_demos    = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();
    R_SetViewSize(screenblocks, detailLevel);

    // Update status bar
    SB_state = -1;
    BorderNeedRefresh = true;

    P_SetMessage(&players[consoleplayer], 
                  english_language ?
                  "SETTINGS RESET" :
                  "YFCNHJQRB C,HJITYS", // НАСТРОЙКИ СБРОШЕНЫ
                  false);
    S_StartSound(NULL, sfx_dorcls);
    menuactive = true;
}

//---------------------------------------------------------------------------
// M_RD_ChangeLanguage
//---------------------------------------------------------------------------

static boolean M_RD_ChangeLanguage(int option)
{
    extern void D_DoAdvanceDemo(void);
    extern int demosequence;

    english_language ^= 1;

    // Reset options menu
    CurrentMenu = english_language ? &OptionsMenu : &OptionsMenu_Rus;
    
    // Clear HUD message
    players[consoleplayer].message = NULL;

    // Update language strings
    RD_DefineLanguageStrings();

    // Update window title
    if (gamemode == retail)
    {
        I_SetWindowTitle(english_language ?
                        "Heretic: Shadow of the Serpent Riders" :
                        "Heretic: Тень Змеиных Всадников");
    }
    else if (gamemode == registered)
    {
        I_SetWindowTitle("Heretic");
    }
    else
    {
        I_SetWindowTitle(english_language ?
                         "Heretic (shareware)" :
                         "Heretic (демоверсия)");
    }
    I_InitWindowTitle();

    // Update TITLEPIC/CREDIT screens in live mode
    if (gamestate == GS_DEMOSCREEN)
    {
        if (demosequence == 0   // initial title screen
        ||  demosequence == 1   // title screen + advisor
        ||  demosequence == 3   // credits
        ||  demosequence == 5)  // ordering info / credits
        {
            demosequence--;
            D_DoAdvanceDemo();
        }
    }

    // Update game screen, borders and status bar
    UpdateState |= I_FULLSCRN;
    BorderNeedRefresh = true;
    SB_state = -1;

    // Restart finale text
    if (gamestate == GS_FINALE)
    {
        F_StartFinale();
    }

    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCNetCheck
//
//---------------------------------------------------------------------------

static boolean SCNetCheck(int option)
{
    if (!netgame)
    {                           // okay to go into the menu
        return true;
    }
    switch (option)
    {
        case 1:
            P_SetMessage(&players[consoleplayer], english_language ?
                         "YOU CAN'T START A NEW GAME IN NETPLAY!" :
                         "YTDJPVJ;YJ YFXFNM YJDE. BUHE D CTNTDJQ BUHT!", // НЕВОЗМОЖНО НАЧАТЬ НОВУЮ ИГРУ В СЕТЕВОЙ ИГРЕ!
                         true);
            break;
        case 2:
            P_SetMessage(&players[consoleplayer], english_language ?
                         "YOU CAN'T LOAD A GAME IN NETPLAY!" :
                         "YTDJPVJ;YJ PFUHEPBNMCZ D CTNTDJQ BUHT!", // НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ!
                         true);
            break;
        default:
            break;
    }
    menuactive = false;
    return false;
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static boolean SCQuitGame(int option)
{
    menuactive = false;
    askforquit = true;
    typeofask = 1;              //quit game
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}



//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static boolean SCMessages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], DEH_String(english_language ?
                      "MESSAGES ON" :
                      "CJJ,OTYBZ DRK.XTYS"), // СООБЩЕНИЯ ВКЛЮЧЕНЫ
                      true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], DEH_String(english_language ?
                      "MESSAGES OFF" :
                      "CJJ,OTYBZ DSRK.XTYS"), // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
                      true);
    }
    S_StartSound(NULL, sfx_chat);
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static boolean SCLoadGame(int option)
{
    char *filename;

    if (!SlotStatus[option])
    {                           // slot's empty...don't try and load
        return false;
    }

    filename = SV_Filename(option);
    G_LoadGame(filename);
    free(filename);

    MN_DeactivateMenu();
    BorderNeedRefresh = true;
    if (quickload == -1)
    {
        quickload = option + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static boolean SCSaveGame(int option)
{
    char *ptr;

    if (!FileMenuKeySteal)
    {
        int x, y;

        FileMenuKeySteal = true;
        // We need to activate the text input interface to type the save
        // game name:
        x = SaveMenu.x + 1;
        y = SaveMenu.y + 1 + option * ITEM_HEIGHT;
        I_StartTextInput(x, y, x + 190, y + ITEM_HEIGHT - 2);

        M_StringCopy(oldSlotText, SlotText[option], sizeof(oldSlotText));
        ptr = SlotText[option];
        while (*ptr)
        {
            ptr++;
        }
        *ptr = '_';
        *(ptr + 1) = 0;
        SlotStatus[option]++;
        currentSlot = option;
        slotptr = ptr - SlotText[option];
        return false;
    }
    else
    {
        G_SaveGame(option, SlotText[option]);
        FileMenuKeySteal = false;
        I_StopTextInput();
        MN_DeactivateMenu();
    }
    BorderNeedRefresh = true;
    if (quicksave == -1)
    {
        quicksave = option + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCEpisode
//
//---------------------------------------------------------------------------

static boolean SCEpisode(int option)
{
    if (gamemode == shareware && option > 1)
    {
        P_SetMessage(&players[consoleplayer], english_language ?
                     "ONLY AVAILABLE IN THE REGISTERED VERSION" :
                     "'GBPJL YTLJCNEGTY D LTVJDTHCBB", // ЭПИЗОД НЕДОСТУПЕН В ДЕМОВЕРСИИ
                     true);
    }
    else
    {
        MenuEpisode = option;
        SetMenu(MENU_SKILL);
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static boolean SCSkill(int option)
{
    G_DeferedInitNew(option, MenuEpisode, 1);
    MN_DeactivateMenu();
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static boolean SCInfo(int option)
{
    InfoType = 1;
    S_StartSound(NULL, sfx_dorcls);
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t * event)
{
    int charTyped;
    int key;
    int i;
    MenuItem_t *item;
    extern boolean automapactive;
    extern void D_StartTitle(void);
    extern void G_CheckDemoStatus(void);
    char *textBuffer;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (event->type == ev_quit
         || (event->type == ev_keydown
          && (event->data1 == key_menu_activate
           || event->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (event->type == ev_quit)
    {
        // First click on close = bring up quit confirm message.
        // Second click = confirm quit.

        if (!menuactive && askforquit && typeofask == 1)
        {
            G_CheckDemoStatus();
            I_Quit();
        }
        else
        {
            SCQuitGame(0);
            S_StartSound(NULL, sfx_chat);
        }
        return true;
    }

    // Allow the menu to be activated from a joystick button if a button
    // is bound for joybmenu.
    if (event->type == ev_joystick)
    {
        if (joybmenu >= 0 && (event->data1 & (1 << joybmenu)) != 0)
        {
            MN_ActivateMenu();
            return true;
        }
    }

    if (event->type != ev_keydown)
    {
        return false;
    }

    key = event->data1;
    charTyped = event->data2;

    if (InfoType)
    {
        if (gamemode == shareware)
        {
            InfoType = (InfoType + 1) % 5;
        }
        else
        {
            InfoType = (InfoType + 1) % 4;
        }
        if (key == KEY_ESCAPE)
        {
            InfoType = 0;
        }
        if (!InfoType)
        {
            paused = false;
            MN_DeactivateMenu();
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, sfx_dorcls);
        return (true);          //make the info screen eat the keypress
    }

    if ((ravpic && key == KEY_F1) ||
        (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot();
        return (true);
    }

    if (askforquit)
    {
        if (key == key_menu_confirm)
        {
            switch (typeofask)
            {
                case 1:
                    G_CheckDemoStatus();
                    I_Quit();
                    return false;

                case 2:
                    players[consoleplayer].messageTics = 0;
                    //set the msg to be cleared
                    players[consoleplayer].message = NULL;
                    paused = false;
                    I_SetPalette(W_CacheLumpName (usegamma <= 8 ?
                                                  "PALFIX" :
                                                  "PLAYPAL",
                                                  PU_CACHE));
                    D_StartTitle();     // go to intro/demo mode.
                    break;

                case 3:
                    P_SetMessage(&players[consoleplayer], english_language ?
                                  "QUICKSAVING...." :
                                  ",SCNHJT CJ[HFYTYBT>>>", // БЫСТРОЕ СОХРАНЕНИЕ...
                                  false);
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                    break;

                case 4:
                    P_SetMessage(&players[consoleplayer], english_language ?
                                  "QUICKLOADING...." :
                                  ",SCNHFZ PFUHEPRF>>>", // БЫСТРАЯ ЗАГРУЗКА...
                                  false);
                    SCLoadGame(quickload - 1);
                    BorderNeedRefresh = true;
                    break;

                case 5:
                    M_RD_DoResetSettings();
                    break;

                default:
                    break;
            }

            askforquit = false;
            typeofask = 0;

            return true;
        }
        else if (key == key_menu_abort || key == KEY_ESCAPE)
        {
            players[consoleplayer].messageTics = 1;  //set the msg to be cleared
            askforquit = false;
            typeofask = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            S_ResumeSound();    // [JN] Fix vanilla Heretic bug: resume music playing
            return true;
        }

        return false;           // don't let the keys filter thru
    }

    if (!menuactive && !chatmodeon)
    {
        if (key == key_menu_decscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            M_RD_ScreenSize(LEFT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_incscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            M_RD_ScreenSize(RIGHT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_help)           // F1
        {
            SCInfo(0);      // start up info screens
            menuactive = true;
            return (true);
        }
        else if (key == key_menu_save)           // F2 (save game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                menuactive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &SaveMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_load)           // F3 (load game)
        {
            if (SCNetCheck(2))
            {
                menuactive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_volume)         // F4 (volume)
        {
            menuactive = true;
            FileMenuKeySteal = false;
            MenuTime = 0;
            CurrentMenu = english_language ? 
                          &SoundMenu :
                          &SoundMenu_Rus;
            CurrentItPos = CurrentMenu->oldItPos;
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, sfx_dorcls);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (key == key_menu_detail)          // F5 (detail)
        {
            // F5 isn't used in Heretic. (detail level)
            return true;
        }
        else if (key == key_menu_qsave)           // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    menuactive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    CurrentMenu = &SaveMenu;
                    CurrentItPos = CurrentMenu->oldItPos;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_dorcls);
                    slottextloaded = false; //reload the slot text, when needed
                    quicksave = -1;
                    P_SetMessage(&players[consoleplayer], english_language ?
                                  "CHOOSE A QUICKSAVE SLOT" : 
                                  "DS,THBNT CKJN ,SCNHJUJ CJ[HFYTYBZ", // ВЫБЕРИТЕ СЛОТ БЫСТРОГО СОХРАНЕНИЯ
                                  true);
                }
                else
                {
                    askforquit = true;
                    typeofask = 3;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_chat);
                }
            }
            return true;
        }
        else if (key == key_menu_endgame)         // F7 (end game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                S_StartSound(NULL, sfx_chat);
                M_RD_EndGame(0);
            }
            return true;
        }
        else if (key == key_menu_messages)        // F8 (toggle messages)
        {
            SCMessages(0);
            return true;
        }
        else if (key == key_menu_qload)           // F9 (quickload)
        {
            if (!quickload || quickload == -1)
            {
                menuactive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
                quickload = -1;
                P_SetMessage(&players[consoleplayer], english_language ?
                             "CHOOSE A QUICKLOAD SLOT" :
                             "DS,THBNT CKJN ,SCNHJQ PFUHEPRB", // ВЫБЕРИТЕ СЛОТ БЫСТРОЙ ЗАГРУЗКИ
                             true);
            }
            else
            {
                askforquit = true;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                typeofask = 4;
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_quit)            // F10 (quit)
        {
            if (gamestate == GS_LEVEL)
            {
                SCQuitGame(0);
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_gamma)           // F11 (gamma correction)
        {
            usegamma++;
            if (usegamma > 17)
            {
                usegamma = 0;
            }
            I_SetPalette((byte *) W_CacheLumpName(usegamma <= 8 ?
                                                  "PALFIX" :
                                                  "PLAYPAL",
                                                  PU_CACHE));

            P_SetMessage(&players[consoleplayer], english_language ? 
                                                  GammaText[usegamma] :
                                                  GammaText_Rus[usegamma],
                                                  false);
            return true;
        }

    }

    if (!menuactive)
    {
        if (key == key_menu_activate || gamestate == GS_DEMOSCREEN || demoplayback)
        {
            MN_ActivateMenu();
            return (true);
        }
        return (false);
    }
    if (!FileMenuKeySteal)
    {
        item = &CurrentMenu->items[CurrentItPos];

        if (key == key_menu_down)            // Next menu item
        {
            do
            {
                if (CurrentItPos + 1 > CurrentMenu->itemCount - 1)
                {
                    CurrentItPos = 0;
                }
                else
                {
                    CurrentItPos++;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
        else if (key == key_menu_up)         // Previous menu item
        {
            do
            {
                if (CurrentItPos == 0)
                {
                    CurrentItPos = CurrentMenu->itemCount - 1;
                }
                else
                {
                    CurrentItPos--;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
        else if (key == key_menu_left)       // Slider left
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(LEFT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
        else if (key == key_menu_right)      // Slider right
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(RIGHT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
        else if (key == key_menu_forward)    // Activate item (enter)
        {
            if (item->type == ITT_SETMENU)
            {
                SetMenu(item->menu);
            }
            else if (item->func != NULL)
            {
                CurrentMenu->oldItPos = CurrentItPos;
                if (item->type == ITT_LRFUNC)
                {
                    item->func(RIGHT_DIR);
                }
                else if (item->type == ITT_EFUNC)
                {
                    if (item->func(item->option))
                    {
                        if (item->menu != MENU_NONE)
                        {
                            SetMenu(item->menu);
                        }
                    }
                }
            }
            S_StartSound(NULL, sfx_dorcls);
            return (true);
        }
        else if (key == key_menu_activate)     // Toggle menu
        {
            MN_DeactivateMenu();
            return (true);
        }
        else if (key == key_menu_back)         // Go back to previous menu
        {
            S_StartSound(NULL, sfx_switch);
            if (CurrentMenu->prevMenu == MENU_NONE)
            {
                MN_DeactivateMenu();
            }
            else
            {
                SetMenu(CurrentMenu->prevMenu);
            }
            return (true);
        }
        // [JN] Scroll Gameplay features menu by PgUp/PgDn keys
        else if (key == KEY_PGUP || key == KEY_PGDN)
        {
            if (CurrentMenu == &Gameplay1Menu || CurrentMenu == &Gameplay1Menu_Rus)
            {
                SetMenu(MENU_GAMEPLAY2);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &Gameplay2Menu || CurrentMenu == &Gameplay2Menu_Rus)
            {
                SetMenu(MENU_GAMEPLAY1);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
        }
        else if (charTyped != 0)
        {
            // Jump to menu item based on first letter:

            for (i = 0; i < CurrentMenu->itemCount; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(DEH_String(CurrentMenu->items[i].text)[0]))
                    {
                        CurrentItPos = i;
                        return (true);
                    }
                }
            }
        }

        return (false);
    }
    else
    {
        // Editing file names
        // When typing a savegame name, we use the fully shifted and
        // translated input value from event->data3.
        charTyped = event->data3;

        textBuffer = &SlotText[currentSlot][slotptr];
        if (key == KEY_BACKSPACE)
        {
            if (slotptr)
            {
                *textBuffer-- = 0;
                *textBuffer = ASCII_CURSOR;
                slotptr--;
            }
            return (true);
        }
        if (key == KEY_ESCAPE)
        {
            memset(SlotText[currentSlot], 0, SLOTTEXTLEN + 2);
            M_StringCopy(SlotText[currentSlot], oldSlotText,
                         sizeof(SlotText[currentSlot]));
            SlotStatus[currentSlot]--;
            MN_DeactivateMenu();
            return (true);
        }
        if (key == KEY_ENTER)
        {
            SlotText[currentSlot][slotptr] = 0; // clear the cursor
            item = &CurrentMenu->items[CurrentItPos];
            CurrentMenu->oldItPos = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                item->func(item->option);
                if (item->menu != MENU_NONE)
                {
                    SetMenu(item->menu);
                }
            }
            return (true);
        }
        if (slotptr < SLOTTEXTLEN && key != KEY_BACKSPACE)
        {
            if (isalpha(charTyped))
            {
                *textBuffer++ = toupper(charTyped);
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
            // [JN] Extended support of Russian alphabet input, including special symbols.
            if (isdigit(charTyped) 
            || charTyped == ' '
            || charTyped == '!'
            || charTyped == '('
            || charTyped == ')'
            || charTyped == '-'
            || charTyped == '+'
            || charTyped == '='
            || charTyped == '/'
            || charTyped == '\\'
            || charTyped == '*'
            || charTyped == '?'
            || charTyped == '_'
            || charTyped == '<'
            || charTyped == '>'
            || charTyped == ':'
            || charTyped == '"'
            || charTyped == ']'   // ъ
            || charTyped == '['   // х
            || charTyped == ';'   // ж
            || charTyped == '\''  // э
            || charTyped == ','   // б
            || charTyped == '.')  // ю
            {
                *textBuffer++ = charTyped;
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
        }
        return (true);
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
    if (menuactive)
    {
        return;
    }
    if (paused)
    {
        S_ResumeSound();
    }
    menuactive = true;
    FileMenuKeySteal = false;
    MenuTime = 0;
    CurrentMenu = english_language ? &MainMenu : &MainMenu_Rus;
    CurrentItPos = CurrentMenu->oldItPos;
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    S_StartSound(NULL, sfx_dorcls);
    slottextloaded = false;     //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
    if (CurrentMenu != NULL)
    {
        CurrentMenu->oldItPos = CurrentItPos;
    }
    S_ResumeSound();    // [JN] Fix vanilla Heretic bug: resume music playing
    menuactive = false;
    if (FileMenuKeySteal)
    {
        I_StopTextInput();
    }
    if (!netgame)
    {
        paused = false;
    }
    S_StartSound(NULL, sfx_dorcls);
    if (soundchanged)
    {
        S_SetMaxVolume(true);   //recalc the sound curve
        soundchanged = false;
    }
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageTics = 1;
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
#ifdef WIDESCREEN
    // [JN] Clean up remainings of the wide screen before 
    // drawing a HELP or TITLE screens.
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
#endif

    I_SetPalette(W_CacheLumpName(usegamma <= 8 ?
                                 "PALFIX" :
                                 "PLAYPAL",
                                 PU_CACHE));

    // [JN] Some complex mess to avoid using numerical identification of screens.
    // Note: in English version, Shareware using paletted screens instead of RAWs.
    if (english_language)
    {
        if (gamemode == shareware)
        {
            if (InfoType == 0)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE"), PU_CACHE));
            if (InfoType == 1)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP1"), PU_CACHE));
            if (InfoType == 2)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP2"), PU_CACHE));
            if (InfoType == 3)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("CREDIT"), PU_CACHE));
            if (InfoType == 4)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("ORDER"), PU_CACHE));
        }
        else
        {
            if (InfoType == 0)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE"), PU_CACHE));
            if (InfoType == 1)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP1"), PU_CACHE));
            if (InfoType == 2)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP2"), PU_CACHE));
            if (InfoType == 3)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("CREDIT"), PU_CACHE));
            if (InfoType == 4)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("ORDER"), PU_CACHE));
        }
    }
    else
    {
        if (InfoType == 0)
        {
            if (gamemode == retail)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE_RT"), PU_CACHE));
            else
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE"), PU_CACHE));
        }
        if (InfoType == 1)
        V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP1_R"), PU_CACHE));
        if (InfoType == 2)
        V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("HELP2_R"), PU_CACHE));
        if (InfoType == 3)
        {
            if (gamemode == retail)
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("CRED_RT"), PU_CACHE));
            else
            V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("CRED_RG"), PU_CACHE));
        }
        if (InfoType == 4)
        V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("ORDER_R"), PU_CACHE));
    }
}


//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

static void SetMenu(MenuType_t menu)
{
    CurrentMenu->oldItPos = CurrentItPos;
    CurrentMenu = english_language ? Menus[menu] : Menus_Rus[menu];
    CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//
//---------------------------------------------------------------------------

static void DrawSlider(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;

    x = menu->x + 24;
    y = menu->y + 2 + (item * ITEM_HEIGHT);
    V_DrawShadowedPatchRaven(x - 32, y, W_CacheLumpName(DEH_String("M_SLDLT"), PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2, y, W_CacheLumpName(DEH_String(count & 1 ? "M_SLDMD1"
                                           : "M_SLDMD2"), PU_CACHE));
    }
    V_DrawShadowedPatchRaven(x2, y, W_CacheLumpName(DEH_String("M_SLDRT"), PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        dp_translation = cr[CR_GREEN2GRAY_HERETIC];
        V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        dp_translation = cr[CR_GREEN2RED_HERETIC];
        V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawSliderSmall
//
// [JN] Draw small slider
//
//---------------------------------------------------------------------------

static void DrawSliderSmall(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;

    x = menu->x + 24;
    y = menu->y + (item * ITEM_HEIGHT_SMALL);

    V_DrawShadowedPatchRaven(x - 32, y, W_CacheLumpName(DEH_String("M_RDSLDL"), PU_CACHE));

    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2 - 16, y, W_CacheLumpName(DEH_String("M_RDSLD1"), PU_CACHE));
    }

    V_DrawShadowedPatchRaven(x2 - 25, y, W_CacheLumpName(DEH_String("M_RDSLDR"), PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        dp_translation = cr[CR_GREEN2GRAY_HERETIC];
        V_DrawPatch(x + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        dp_translation = cr[CR_GREEN2RED_HERETIC];
        V_DrawPatch(x + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
}

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
    MENU_AUTOMAP,
    MENU_SOUND,
    MENU_SOUND_SYS,
    MENU_CONTROLS,
    MENU_GAMEPLAY1,
    MENU_GAMEPLAY2,
    MENU_OPTIONS_VANILLA,
    MENU_OPTIONS2_VANILLA,
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
static void DrawOptionsMenu(void);
static void DrawSlider(Menu_t * menu, int item, int width, int slot);
static void DrawSliderSmall(Menu_t * menu, int item, int width, int slot);
void MN_LoadSlotText(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static boolean M_RD_Change_Widescreen(int option);
static boolean M_RD_Change_VSync(int option);
static boolean M_RD_Uncapped(int option);
static boolean M_RD_FPScounter(int option);
static boolean M_RD_Smoothing(int option);
static boolean M_RD_PorchFlashing(int option);
static boolean M_RD_Renderer(int option);
static boolean M_RD_Screenshots(int option);

// Display
static void DrawDisplayMenu(void);
static boolean M_RD_ScreenSize(int option);
static boolean M_RD_Gamma(int option);
static boolean M_RD_LevelBrightness(int option);
static boolean M_RD_LocalTime(int option);
static boolean M_RD_Messages(int option);
static boolean M_RD_ShadowedText(int option);

// Automap
static void DrawAutomapMenu(void);
static boolean M_RD_AutoMapStats(int option);
static boolean M_RD_AutoMapOverlay(int option);
static boolean M_RD_AutoMapRotate(int option);
static boolean M_RD_AutoMapFollow(int option);
static boolean M_RD_AutoMapGrid(int option);

// Sound
static void DrawSoundMenu(void);
static boolean M_RD_SfxVolume(int option);
static boolean M_RD_MusVolume(int option);
static boolean M_RD_SfxChannels(int option);

// Sound system
static void DrawSoundSystemMenu(void);
static boolean M_RD_SoundDevice(int option);
static boolean M_RD_MusicDevice(int option);
static boolean M_RD_Sampling(int option);
static boolean M_RD_SndMode(int option);
static boolean M_RD_PitchShifting(int option);
static boolean M_RD_MuteInactive(int option);

// Controls
static void DrawControlsMenu(void);
static boolean M_RD_AlwaysRun(int option);
static boolean M_RD_Sensitivity(int option);
static boolean M_RD_Acceleration(int option);
static boolean M_RD_Threshold(int option);
static boolean M_RD_MouseLook(int option);
static boolean M_RD_InvertY(int option);
static boolean M_RD_Novert(int option);


// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static boolean M_RD_Brightmaps(int option);
static boolean M_RD_FakeContrast(int option);
static boolean M_RD_ExtraTrans(int option);
static boolean M_RD_ColoredHUD(int option);
static boolean M_RD_ColoredBlood(int option);
static boolean M_RD_InvulSky(int option);
static boolean M_RD_SecretNotify(int option);
static boolean M_RD_NegativeHealth(int option);

// Gameplay (page 2)
static void DrawGameplay2Menu(void);
static boolean M_RD_Torque(int option);
static boolean M_RD_Bobbing(int option);
static boolean M_RD_FlipCorpses(int option);
static boolean M_RD_FloatAmplitude(int option);
static boolean M_RD_CrossHairDraw(int option);
static boolean M_RD_CrossHairType(int option);
static boolean M_RD_CrossHairScale(int option);
static boolean M_RD_FlipLevels(int option);
static boolean M_RD_NoDemos(int option);
static boolean M_RD_WandStart(int option);

// Vanilla Options menu
static void DrawOptionsMenu_Vanilla(void);
static void DrawOptions2Menu_Vanilla(void);

// End game
static boolean M_RD_EndGame(int option);

// Reset settings
static boolean M_RD_ResetSettings(int option);

// Change language
static boolean M_RD_ChangeLanguage(int option);

// Public Data

boolean menuactive;
int InfoType;

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

// [JN] Set default mouse sensitivity to 5, like in Doom
int mouseSensitivity = 5;

static MenuItem_t MainItems[] = {
    {ITT_EFUNC,   "NEW GAME",   SCNetCheck, 1, MENU_EPISODE},
    {ITT_SETMENU, "OPTIONS",    NULL,       0, MENU_OPTIONS},
    {ITT_SETMENU, "GAME FILES", NULL,       0, MENU_FILES},
    {ITT_EFUNC,   "INFO",       SCInfo,     0, MENU_NONE},
    {ITT_EFUNC,   "QUIT GAME",  SCQuitGame, 0, MENU_NONE}
};

static MenuItem_t MainItems_Rus[] = {
    {ITT_EFUNC,   "YJDFZ BUHF", SCNetCheck, 1, MENU_EPISODE}, // НОВАЯ ИГРА
    {ITT_SETMENU, "YFCNHJQRB",  NULL,       0, MENU_OPTIONS}, // НАСТРОЙКИ
    {ITT_SETMENU, "AFQKS BUHS", NULL,       0, MENU_FILES},   // ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,   "BYAJHVFWBZ", SCInfo,     0, MENU_NONE},    // ИНФОРМАЦИЯ
    {ITT_EFUNC,   "DS[JL",      SCQuitGame, 0, MENU_NONE}     // ВЫХОД
};

static Menu_t MainMenu = {
    110, 56,
    DrawMainMenu,
    5, MainItems,
    0,
    MENU_NONE
};

static Menu_t MainMenu_Rus = {
    103, 56,
    DrawMainMenu,
    5, MainItems_Rus,
    0,
    MENU_NONE
};

static MenuItem_t EpisodeItems[] = {
    {ITT_EFUNC, "CITY OF THE DAMNED",   SCEpisode, 1, MENU_NONE},
    {ITT_EFUNC, "HELL'S MAW",           SCEpisode, 2, MENU_NONE},
    {ITT_EFUNC, "THE DOME OF D'SPARIL", SCEpisode, 3, MENU_NONE},
    {ITT_EFUNC, "THE OSSUARY",          SCEpisode, 4, MENU_NONE},
    {ITT_EFUNC, "THE STAGNANT DEMESNE", SCEpisode, 5, MENU_NONE}
};

static MenuItem_t EpisodeItems_Rus[] = {
    {ITT_EFUNC, "UJHJL GHJRKZNS[",    SCEpisode, 1, MENU_NONE}, // ГОРОД ПРОКЛЯТЫХ
    {ITT_EFUNC, "FLCRFZ ENHJ,F",      SCEpisode, 2, MENU_NONE}, // АДСКАЯ УТРОБА
    {ITT_EFUNC, "REGJK L\"CGFHBKF",   SCEpisode, 3, MENU_NONE}, // КУПОЛ Д'СПАРИЛА
    {ITT_EFUNC, "CRKTG",              SCEpisode, 4, MENU_NONE}, // СКЛЕП
    {ITT_EFUNC, "PFCNJQYST DKFLTYBZ", SCEpisode, 5, MENU_NONE}  // ЗАСТОЙНЫЕ ВЛАДЕНИЯ 
};

static Menu_t EpisodeMenu = {
    80, 50,
    DrawEpisodeMenu,
    3, EpisodeItems,
    0,
    MENU_MAIN
};

static Menu_t EpisodeMenu_Rus = {
    55, 50,
    DrawEpisodeMenu,
    3, EpisodeItems_Rus,
    0,
    MENU_MAIN
};

static MenuItem_t FilesItems[] = {
    {ITT_EFUNC,   "LOAD GAME", SCNetCheck, 2, MENU_LOAD},
    {ITT_SETMENU, "SAVE GAME", NULL,       0, MENU_SAVE}
};

static MenuItem_t FilesItems_Rus[] = {
    {ITT_EFUNC,   "PFUHEPBNM BUHE", SCNetCheck, 2, MENU_LOAD}, // ЗАГРУЗИТЬ ИГРУ
    {ITT_SETMENU, "CJ[HFYBNM BUHE", NULL,       0, MENU_SAVE}  // СОХРАНИТЬ ИГРУ
};

static Menu_t FilesMenu = {
    110, 60,
    DrawFilesMenu,
    2, FilesItems,
    0,
    MENU_MAIN
};

static Menu_t FilesMenu_Rus = {
    90, 60,
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
    70, 30,
    DrawLoadMenu,
    6, LoadItems,
    0,
    MENU_FILES
};

static Menu_t LoadMenu_Rus = {
    70, 30,
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
    70, 30,
    DrawSaveMenu,
    6, SaveItems,
    0,
    MENU_FILES
};

static Menu_t SaveMenu_Rus = {
    70, 30,
    DrawSaveMenu,
    6, SaveItems,
    0,
    MENU_FILES
};

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, "THOU NEEDETH A WET-NURSE",       SCSkill, sk_baby,      MENU_NONE},
    {ITT_EFUNC, "YELLOWBELLIES-R-US",             SCSkill, sk_easy,      MENU_NONE},
    {ITT_EFUNC, "BRINGEST THEM ONETH",            SCSkill, sk_medium,    MENU_NONE},
    {ITT_EFUNC, "THOU ART A SMITE-MEISTER",       SCSkill, sk_hard,      MENU_NONE},
    {ITT_EFUNC, "BLACK PLAGUE POSSESSES THEE",    SCSkill, sk_nightmare, MENU_NONE},
    {ITT_EFUNC, "QUICKETH ART THEE, FOUL WRAITH", SCSkill, sk_ultranm,   MENU_NONE} // [JN] Thanks to Jon Dowland for this :)
};

static MenuItem_t SkillItems_Rus[] = {
    {ITT_EFUNC, "YZYTXRF YFLJ,YF VYT",    SCSkill, sk_baby,      MENU_NONE}, // НЯНЕЧКА НАДОБНА МНЕ
    {ITT_EFUNC, "YT CNJKM VE;TCNDTYTY Z", SCSkill, sk_easy,      MENU_NONE}, // НЕ СТОЛЬ МУЖЕСТВЕНЕН Я
    {ITT_EFUNC, "GJLFQNT VYT B[",         SCSkill, sk_medium,    MENU_NONE}, // ПОДАЙТЕ МНЕ ИХ
    {ITT_EFUNC, "BCREITY Z CHF;TYBZVB",   SCSkill, sk_hard,      MENU_NONE}, // ИСКУШЕН Я СРАЖЕНИЯМИ
    {ITT_EFUNC, "XEVF JDKFLTKF VYJQ",     SCSkill, sk_nightmare, MENU_NONE}, // ЧУМА ОВЛАДЕЛА МНОЙ
    {ITT_EFUNC, "RJIVFHJV BCGJKYTY Z",    SCSkill, sk_ultranm,   MENU_NONE}  // КОШМАРОМ ИСПОЛНЕН Я
};

static Menu_t SkillMenu = {
    38, 30,
    DrawSkillMenu,
    6, SkillItems,
    2,
    MENU_EPISODE
};

static Menu_t SkillMenu_Rus = {
    38, 30,
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
    {ITT_EFUNC,   "RESET SETTINGS",    M_RD_ResetSettings,  0, MENU_NONE     },
    {ITT_LRFUNC,  "LANGUAGE: ENGLISH", M_RD_ChangeLanguage, 0, MENU_NONE     }
};

static MenuItem_t OptionsItems_Rus[] = {
    {ITT_SETMENU, "DBLTJ",             NULL,                0, MENU_RENDERING},  // ВИДЕО
    {ITT_SETMENU, "\'RHFY",            NULL,                0, MENU_DISPLAY  },  // ЭКРАН
    {ITT_SETMENU, "FELBJ",             NULL,                0, MENU_SOUND    },  // АУДИО
    {ITT_SETMENU, "EGHFDKTYBT",        NULL,                0, MENU_CONTROLS },  // УПРАВЛЕНИЕ
    {ITT_SETMENU, "UTQVGKTQ",          NULL,                0, MENU_GAMEPLAY1},  // ГЕЙМПЛЕЙ
    {ITT_EFUNC,   "C,HJC YFCNHJTR",    M_RD_ResetSettings,  0, MENU_NONE     },  // СБРОС НАСТРОЕК
    {ITT_LRFUNC,  "ZPSR: HECCRBQ",     M_RD_ChangeLanguage, 0, MENU_NONE     }   // ЯЗЫК: РУССКИЙ
};

static Menu_t OptionsMenu = {
    81, 31,
    DrawOptionsMenu,
    7, OptionsItems,
    0,
    MENU_MAIN
};

static Menu_t OptionsMenu_Rus = {
    81, 31,
    DrawOptionsMenu,
    7, OptionsItems_Rus,
    0,
    MENU_MAIN
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_LRFUNC, "DISPLAY ASPECT RATIO:",     M_RD_Change_Widescreen, 0, MENU_NONE},
    {ITT_LRFUNC, "VERTICAL SYNCHRONIZATION:", M_RD_Change_VSync,      0, MENU_NONE},
    {ITT_LRFUNC, "FRAME RATE:",               M_RD_Uncapped,          0, MENU_NONE},
    {ITT_LRFUNC, "FPS COUNTER:",              M_RD_FPScounter,        0, MENU_NONE},
    {ITT_LRFUNC, "PIXEL SCALING:",            M_RD_Smoothing,         0, MENU_NONE},
    {ITT_LRFUNC, "PORCH PALETTE CHANGING:",   M_RD_PorchFlashing,     0, MENU_NONE},
    {ITT_LRFUNC, "VIDEO RENDERER:",           M_RD_Renderer,          0, MENU_NONE},
    {ITT_EMPTY,  NULL,                        NULL,                   0, MENU_NONE},
    {ITT_LRFUNC, "SCREENSHOT FORMAT:",        M_RD_Screenshots,       0, MENU_NONE}
};

static MenuItem_t RenderingItems_Rus[] = {
    {ITT_LRFUNC, "CJJNYJITYBT CNJHJY \'RHFYF:", M_RD_Change_Widescreen, 0, MENU_NONE}, // СООТНОШЕНИЕ СТОРОН ЭКРАНА
    {ITT_LRFUNC, "DTHNBRFKMYFZ CBY[HJYBPFWBZ:", M_RD_Change_VSync,      0, MENU_NONE}, // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    {ITT_LRFUNC, "RFLHJDFZ XFCNJNF:",           M_RD_Uncapped,          0, MENU_NONE}, // КАДРОВАЯ ЧАСТОТА
    {ITT_LRFUNC, "CXTNXBR RFLHJDJQ XFCNJNS:",   M_RD_FPScounter,        0, MENU_NONE}, // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "GBRCTKMYJT CUKF;BDFYBT:",     M_RD_Smoothing,         0, MENU_NONE}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_LRFUNC, "BPVTYTYBT GFKBNHS RHFTD 'RHFYF:", M_RD_PorchFlashing, 0, MENU_NONE}, // ИЗМЕНЕНИЕ ПАЛИТРЫ КРАЕВ ЭКРАНА
    {ITT_LRFUNC, "J,HF,JNRF DBLTJ:",            M_RD_Renderer,          0, MENU_NONE}, // ОБРАБОТКА ВИДЕО
    {ITT_EMPTY,  NULL,                          NULL,                   0, MENU_NONE}, //
    {ITT_LRFUNC, "AJHVFN CRHBYIJNJD:",          M_RD_Screenshots,       0, MENU_NONE}  // ФОРМАТ СКРИНШОТОВ
};

static Menu_t RenderingMenu = {
    36, 42,
    DrawRenderingMenu,
    9, RenderingItems,
    0,
    MENU_OPTIONS
};

static Menu_t RenderingMenu_Rus = {
    36, 42,
    DrawRenderingMenu,
    9, RenderingItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_LRFUNC, "SCREEN SIZE",         M_RD_ScreenSize,      0, MENU_NONE},
    {ITT_EMPTY,  NULL,                  NULL,                 0, MENU_NONE},
    {ITT_LRFUNC, "GAMMA-CORRECTION",    M_RD_Gamma,           0, MENU_NONE},
    {ITT_EMPTY,  NULL,                  NULL,                 0, MENU_NONE},
    {ITT_LRFUNC, "LEVEL BRIGHTNESS",    M_RD_LevelBrightness, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,                  NULL,                 0, MENU_NONE},
    {ITT_EMPTY,  NULL,                  NULL,                 0, MENU_NONE},
    {ITT_LRFUNC, "LOCAL TIME:",         M_RD_LocalTime,       0, MENU_NONE},
    {ITT_LRFUNC, "MESSAGES:",           M_RD_Messages,        0, MENU_NONE},
    {ITT_LRFUNC, "TEXT CASTS SHADOWS:", M_RD_ShadowedText,    0, MENU_NONE},
    {ITT_SETMENU,"AUTOMAP SETTINGS...", NULL,                 0, MENU_AUTOMAP}
};

static MenuItem_t DisplayItems_Rus[] = {
    {ITT_LRFUNC, "HFPVTH BUHJDJUJ \'RHFYF",  M_RD_ScreenSize,      0, MENU_NONE},   // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE},   // 
    {ITT_LRFUNC, "EHJDTYM UFVVF-RJHHTRWBB",  M_RD_Gamma,           0, MENU_NONE},   // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE},   //
    {ITT_LRFUNC, "EHJDTYM JCDTOTYYJCNB",     M_RD_LevelBrightness, 0, MENU_NONE},   // УРОВЕНЬ ОСВЕЩЕННОСТИ
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE},   //
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE},   //
    {ITT_LRFUNC, "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,       0, MENU_NONE},   // СИСТЕМНОЕ ВРЕМЯ
    {ITT_LRFUNC, "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,        0, MENU_NONE},   // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_LRFUNC, "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText,    0, MENU_NONE},   // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_SETMENU,"YFCNHJQRB RFHNS>>>",       NULL,                 0, MENU_AUTOMAP} // НАСТРОЙКИ КАРТЫ
};

static Menu_t DisplayMenu = {
    36, 42,
    DrawDisplayMenu,
    11, DisplayItems,
    0,
    MENU_OPTIONS
};

static Menu_t DisplayMenu_Rus = {
    36, 42,
    DrawDisplayMenu,
    11, DisplayItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_LRFUNC, "LEVEL STATS:",  M_RD_AutoMapStats,   0, MENU_NONE},
    {ITT_LRFUNC, "OVERLAY MODE:", M_RD_AutoMapOverlay, 0, MENU_NONE},
    {ITT_LRFUNC, "ROTATE MODE:",  M_RD_AutoMapRotate,  0, MENU_NONE},
    {ITT_LRFUNC, "FOLLOW MODE:",  M_RD_AutoMapFollow,  0, MENU_NONE},
    {ITT_LRFUNC, "GRID:",         M_RD_AutoMapGrid,    0, MENU_NONE}
};

static MenuItem_t AutomapItems_Rus[] = {
    {ITT_LRFUNC, "CNFNBCNBRF EHJDYZ:", M_RD_AutoMapStats,   0, MENU_NONE}, // СТАТИСТИКА УРОВНЯ
    {ITT_LRFUNC, "HT;BV YFKJ;TYBZ:",   M_RD_AutoMapOverlay, 0, MENU_NONE}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_LRFUNC, "HT;BV DHFOTYBZ:",    M_RD_AutoMapRotate,  0, MENU_NONE}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_LRFUNC, "HT;BV CKTLJDFYBZ:",  M_RD_AutoMapFollow,  0, MENU_NONE}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_LRFUNC, "CTNRF:",             M_RD_AutoMapGrid,    0, MENU_NONE}  // СЕТКА
};

static Menu_t AutomapMenu = {
    102, 32,
    DrawAutomapMenu,
    5, AutomapItems,
    0,
    MENU_DISPLAY
};

static Menu_t AutomapMenu_Rus = {
    82, 32,
    DrawAutomapMenu,
    5, AutomapItems_Rus,
    0,
    MENU_DISPLAY
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_LRFUNC, "SFX VOLUME",               M_RD_SfxVolume,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,             0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME",             M_RD_MusVolume,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,             0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,             0, MENU_NONE},
    {ITT_LRFUNC, "SFX CHANNELS",             M_RD_SfxChannels, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,             0, MENU_NONE},
    {ITT_EMPTY,  NULL,                       NULL,             0, MENU_NONE},
    {ITT_SETMENU,"SOUND SYSTEM SETTINGS...", NULL,             0, MENU_SOUND_SYS}
};

static MenuItem_t SoundItems_Rus[] = {
    {ITT_LRFUNC, "UHJVRJCNM PDERF",               M_RD_SfxVolume,   0, MENU_NONE},      // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,                            NULL,             0, MENU_NONE},      //
    {ITT_LRFUNC, "UHJVRJCNM VEPSRB",              M_RD_MusVolume,   0, MENU_NONE},      // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,                            NULL,             0, MENU_NONE},      //
    {ITT_EMPTY,  NULL,                            NULL,             0, MENU_NONE},      //
    {ITT_LRFUNC, "PDERJDST RFYFKS",               M_RD_SfxChannels, 0, MENU_NONE},      // ЗВУКОВЫЕ КАНАЛЫ
    {ITT_EMPTY,  NULL,                            NULL,             0, MENU_NONE},      //
    {ITT_EMPTY,  NULL,                            NULL,             0, MENU_NONE},      //
    {ITT_SETMENU,"YFCNHJQRB PDERJDJQ CBCNTVS>>>", NULL,             0, MENU_SOUND_SYS}, // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
};

static Menu_t SoundMenu = {
    36, 42,
    DrawSoundMenu,
    9, SoundItems,
    0,
    MENU_OPTIONS
};

static Menu_t SoundMenu_Rus = {
    36, 42,
    DrawSoundMenu,
    9, SoundItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    {ITT_LRFUNC, "SOUND EFFECTS:",        M_RD_SoundDevice,   0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC:",                M_RD_MusicDevice,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                    NULL,               0, MENU_NONE},
    {ITT_LRFUNC, "SAMPLING FREQUENCY:",   M_RD_Sampling,      0, MENU_NONE},
    {ITT_EMPTY,  NULL,                    NULL,               0, MENU_NONE},
    {ITT_LRFUNC, "SOUND EFFECTS MODE:",   M_RD_SndMode,       0, MENU_NONE},
    {ITT_LRFUNC, "PITCH-SHIFTED SOUNDS:", M_RD_PitchShifting, 0, MENU_NONE},
    {ITT_LRFUNC, "MUTE INACTIVE WINDOW:", M_RD_MuteInactive,  0, MENU_NONE},
};

static MenuItem_t SoundSysItems_Rus[] = {
    {ITT_LRFUNC, "PDERJDST \'AATRNS:",         M_RD_SoundDevice,   0, MENU_NONE}, // ЗВУКОВЫЕ ЭФФЕКТЫ:
    {ITT_LRFUNC, "VEPSRF:",                    M_RD_MusicDevice,   0, MENU_NONE}, // МУЗЫКА:
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_LRFUNC, "XFCNJNF LBCRHTNBPFWBB:",     M_RD_Sampling,      0, MENU_NONE}, // ЧАСТОТА ДИСКРЕТИЗАЦИИ:
    {ITT_EMPTY,  NULL,                         NULL,               0, MENU_NONE}, //
    {ITT_LRFUNC, "HT;BV PDERJDS[ \'AATRNJD:",  M_RD_SndMode,       0, MENU_NONE}, // РЕЖИМ ЗВУКОВЫХ ЭФФЕКТОВ
    {ITT_LRFUNC, "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting, 0, MENU_NONE}, // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
    {ITT_LRFUNC, "PDER D YTFRNBDYJV JRYT:",    M_RD_MuteInactive,  0, MENU_NONE}, // ЗВУК В НЕАКТИВНОМ ОКНЕ
};

static Menu_t SoundSysMenu = {
    36, 42,
    DrawSoundSystemMenu,
    8, SoundSysItems,
    0,
    MENU_SOUND
};

static Menu_t SoundSysMenu_Rus = {
    36, 42,
    DrawSoundSystemMenu,
    8, SoundSysItems_Rus,
    0,
    MENU_SOUND
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_LRFUNC, "ALWAYS RUN:",            M_RD_AlwaysRun,    0, MENU_NONE},
    {ITT_EMPTY,  NULL,                     NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "MOUSE SENSIVITY",        M_RD_Sensitivity,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,                     NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "ACCELERATION",           M_RD_Acceleration, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,                     NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "ACCELERATION THRESHOLD", M_RD_Threshold,    0, MENU_NONE},
    {ITT_EMPTY,  NULL,                     NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "MOUSE LOOK:",            M_RD_MouseLook,    0, MENU_NONE},
    {ITT_LRFUNC, "INVERT Y AXIS:",         M_RD_InvertY,      0, MENU_NONE},
    {ITT_LRFUNC, "VERTICAL MOVEMENT:",     M_RD_Novert,       0, MENU_NONE}
};

static MenuItem_t ControlsItems_Rus[] = {
    {ITT_LRFUNC, "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,    0, MENU_NONE}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_EMPTY,  NULL,                        NULL,              0, MENU_NONE}, //
    {ITT_LRFUNC, "CRJHJCNM VSIB",             M_RD_Sensitivity,  0, MENU_NONE}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,  NULL,                        NULL,              0, MENU_NONE}, //
    {ITT_LRFUNC, "FRCTKTHFWBZ",               M_RD_Acceleration, 0, MENU_NONE}, // АКСЕЛЕРАЦИЯ
    {ITT_EMPTY,  NULL,                        NULL,              0, MENU_NONE}, //
    {ITT_LRFUNC, "GJHJU FRCTKTHFWBB",         M_RD_Threshold,    0, MENU_NONE}, // ПОРОГ АКСЕЛЕРАЦИИ
    {ITT_EMPTY,  NULL,                        NULL,              0, MENU_NONE}, //
    {ITT_LRFUNC, "J,PJH VSIM.:",              M_RD_MouseLook,    0, MENU_NONE}, // ОБЗОР МЫШЬЮ
    {ITT_LRFUNC, "DTHNBRFKMYFZ BYDTHCBZ:",    M_RD_InvertY,      0, MENU_NONE}, // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    {ITT_LRFUNC, "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,       0, MENU_NONE}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36, 42,
    DrawControlsMenu,
    11, ControlsItems,
    0,
    MENU_OPTIONS
};

static Menu_t ControlsMenu_Rus = {
    36, 42,
    DrawControlsMenu,
    11, ControlsItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay1Items[] = {
    {ITT_LRFUNC, "BRIGHTMAPS:",                  M_RD_Brightmaps,   0, MENU_NONE},
    {ITT_LRFUNC, "FAKE CONTRAST:",               M_RD_FakeContrast, 0, MENU_NONE},
    {ITT_LRFUNC, "EXTRA TRANSLUCENCY:",          M_RD_ExtraTrans,   0, MENU_NONE},
    {ITT_LRFUNC, "COLORED HUD:",                 M_RD_ColoredHUD,   0, MENU_NONE},
    {ITT_LRFUNC, "COLORED BLOOD:",               M_RD_ColoredBlood, 0, MENU_NONE},
    {ITT_LRFUNC, "INVULNERABILITY AFFECTS SKY:", M_RD_InvulSky,     0, MENU_NONE},
    {ITT_EMPTY,  NULL,                           NULL,              0, MENU_NONE},
    {ITT_LRFUNC, "CORPSES SLIDING FROM LEDGES:", M_RD_Torque,       0, MENU_NONE},
    {ITT_LRFUNC, "WEAPON BOBBING WHILE FIRING:", M_RD_Bobbing,      0, MENU_NONE},
    {ITT_LRFUNC, "RANDOMLY MIRRORED CORPSES:",   M_RD_FlipCorpses,  0, MENU_NONE},
    {ITT_LRFUNC, "FLOATING ITEMS AMPLITUDE:" ,   M_RD_FloatAmplitude,0, MENU_NONE},
    {ITT_SETMENU,"NEXT PAGE...",                 NULL,              0, MENU_GAMEPLAY2}
};

static MenuItem_t Gameplay1Items_Rus[] = {
    {ITT_LRFUNC, ",HFQNVFGGBYU:",                  M_RD_Brightmaps,   0, MENU_NONE},      // БРАЙТМАППИНГ
    {ITT_LRFUNC, "BVBNFWBZ RJYNHFCNYJCNB:",        M_RD_FakeContrast, 0, MENU_NONE},      // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_LRFUNC, "LJGJKYBNTKMYFZ GHJPHFXYJCNM:",   M_RD_ExtraTrans,   0, MENU_NONE},      // ДОПОЛНИТЕЛЬНАЯ ПРОЗРАЧНОСТЬ
    {ITT_LRFUNC, "HFPYJWDTNYST \'KTVTYNS $:",      M_RD_ColoredHUD,   0, MENU_NONE},      // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ HUD
    {ITT_LRFUNC, "HFPYJWDTNYFZ RHJDM:",            M_RD_ColoredBlood, 0, MENU_NONE},      // РАЗНОЦВЕТНАЯ КРОВЬ
    {ITT_LRFUNC, "YTEZPDBVJCNM JRHFIBDFTN YT,J:",  M_RD_InvulSky,     0, MENU_NONE},      // НЕУЯЗВИМОСТЬ ОКРАШИВАЕТ НЕБО
    {ITT_EMPTY,  NULL,                             NULL,              0, MENU_NONE},      //
    {ITT_LRFUNC, "NHEGS CGJKPF.N C DJPDSITYBQ:",   M_RD_Torque,       0, MENU_NONE},      // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_LRFUNC, "EKEXITYYJT GJRFXBDFYBT JHE;BZ:", M_RD_Bobbing,      0, MENU_NONE},      // УЛУЧШЕННОЕ ПОКАЧИВАНИЕ ОРУЖИЯ
    {ITT_LRFUNC, "PTHRFKBHJDFYBT NHEGJD:",         M_RD_FlipCorpses,  0, MENU_NONE},      // ЗЕКРАЛИРОВАНИЕ ТРУПОВ
    {ITT_LRFUNC, "KTDBNFWBZ GHTLVTNJD:",           M_RD_FloatAmplitude,0, MENU_NONE},      // АМПЛИТУДА ЛЕВИТАЦИИ ПРЕДМЕТОВ
    {ITT_SETMENU,"CKTLE.OFZ CNHFYBWF>>>",          NULL,              0, MENU_GAMEPLAY2 } // СЛЕДУЮЩАЯ СТРАНИЦА
};

static Menu_t Gameplay1Menu = {
    36, 36,
    DrawGameplay1Menu,
    12, Gameplay1Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay1Menu_Rus = {
    36, 36,
    DrawGameplay1Menu,
    12, Gameplay1Items_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features (2)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay2Items[] = {
    {ITT_LRFUNC, "NOTIFY OF REVEALED SECRETS:", M_RD_SecretNotify,    0, MENU_NONE},
    {ITT_LRFUNC, "SHOW NEGATIVE HEALTH:",       M_RD_NegativeHealth,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,                          NULL,                 0, MENU_NONE},
    {ITT_LRFUNC, "DRAW CROSSHAIR:",             M_RD_CrossHairDraw,   0, MENU_NONE},
    {ITT_LRFUNC, "INDICATION:",                 M_RD_CrossHairType,   0, MENU_NONE},
    {ITT_LRFUNC, "INCREASED SIZE:",             M_RD_CrossHairScale,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,                          NULL,                 0, MENU_NONE},
    {ITT_LRFUNC, "FLIP GAME LEVELS:",           M_RD_FlipLevels,      0, MENU_NONE},
    {ITT_LRFUNC, "PLAY INTERNAL DEMOS:",        M_RD_NoDemos,         0, MENU_NONE},
    {ITT_LRFUNC, "WAND START GAME MODE:",       M_RD_WandStart,       0, MENU_NONE},
    {ITT_EMPTY,  NULL,                          NULL,                 0, MENU_NONE},
    {ITT_SETMENU,"PREVIOUS PAGE...",            NULL,                 0, MENU_GAMEPLAY1}
};

static MenuItem_t Gameplay2Items_Rus[] = {
    {ITT_LRFUNC, "CJJ,OFNM J YFQLTYYJV NFQYBRT:", M_RD_SecretNotify,    0, MENU_NONE},     // СООБЩАТЬ О НАЙДЕННОМ ТАЙНИКЕ
    {ITT_LRFUNC, "JNHBWFNTKMYJT PLJHJDMT D $:",   M_RD_NegativeHealth,  0, MENU_NONE},     // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ В HUD
    {ITT_EMPTY,  NULL,                            NULL,                 0, MENU_NONE},     //
    {ITT_LRFUNC, "JNJ,HF;FNM GHBWTK:",            M_RD_CrossHairDraw,   0, MENU_NONE},     // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_LRFUNC, "BYLBRFWBZ:",                    M_RD_CrossHairType,   0, MENU_NONE},     // ИНДИКАЦИЯ
    {ITT_LRFUNC, "EDTKBXTYYSQ HFPVTH:",           M_RD_CrossHairScale,  0, MENU_NONE},     // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,  NULL,                            NULL,                 0, MENU_NONE},     //
    {ITT_LRFUNC, "PTHRFKMYJT JNHF;TYBT EHJDYTQ:", M_RD_FlipLevels,      0, MENU_NONE},     // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_LRFUNC, "GHJBUHSDFNM LTVJPFGBCB:",       M_RD_NoDemos,         0, MENU_NONE},     // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_LRFUNC, " ",/* [JN] Joint EN/RU string*/ M_RD_WandStart,       0, MENU_NONE},     // РЕЖИМ ИГРЫ "WAND START"
    {ITT_EMPTY,  NULL,                            NULL,                 0, MENU_NONE},
    {ITT_SETMENU,"GHTLSLEOFZ CNHFYBWF>>>",        NULL,                 0, MENU_GAMEPLAY1} // ПРЕДЫДУЩАЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    36, 36,
    DrawGameplay2Menu,
    12, Gameplay2Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay2Menu_Rus = {
    36, 36,
    DrawGameplay2Menu,
    12, Gameplay2Items_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Vanilla options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems_Vanilla[] = {
    {ITT_EFUNC,   "END GAME",          M_RD_EndGame,     0, MENU_NONE},
    {ITT_EFUNC,   "MESSAGES : ",       SCMessages,       0, MENU_NONE},
    {ITT_LRFUNC,  "MOUSE SENSITIVITY", M_RD_Sensitivity, 0, MENU_NONE},
    {ITT_EMPTY,   NULL,                NULL,             0, MENU_NONE},
    {ITT_SETMENU, "MORE...",           NULL,             0, MENU_OPTIONS2_VANILLA}
};

static MenuItem_t OptionsItems_Rus_Vanilla[] = {
    {ITT_EFUNC,   "PFRJYXBNM BUHE",   M_RD_EndGame,     0, MENU_NONE},
    {ITT_EFUNC,   "CJJ,OTYBZ : ",     SCMessages,       0, MENU_NONE},
    {ITT_LRFUNC,  "CRJHJCNM VSIB",    M_RD_Sensitivity, 0, MENU_NONE},
    {ITT_EMPTY,   NULL,               NULL,             0, MENU_NONE},
    {ITT_SETMENU, "LJGJKYBNTKMYJ>>>", NULL,             0, MENU_OPTIONS2_VANILLA}
};

static Menu_t OptionsMenu_Vanilla = {
    88, 30,
    DrawOptionsMenu_Vanilla,
    5, OptionsItems_Vanilla,
    0,
    MENU_MAIN
};

static Menu_t OptionsMenu_Rus_Vanilla = {
    88, 30,
    DrawOptionsMenu_Vanilla,
    5, OptionsItems_Rus_Vanilla,
    0,
    MENU_MAIN
};

// -----------------------------------------------------------------------------
// Vanilla options menu (more...)
// -----------------------------------------------------------------------------

static MenuItem_t Options2Items_Vanilla[] = {
    {ITT_LRFUNC, "SCREEN SIZE",  M_RD_ScreenSize, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,           NULL,            0, MENU_NONE},
    {ITT_LRFUNC, "SFX VOLUME",   M_RD_SfxVolume,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,           NULL,            0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME", M_RD_MusVolume,  0, MENU_NONE},
    {ITT_EMPTY,  NULL,           NULL,            0, MENU_NONE}
};

static MenuItem_t Options2Items_Rus_Vanilla[] = {
    {ITT_LRFUNC, "HFPVTH 'RHFYF",   M_RD_ScreenSize, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,               NULL,           0, MENU_NONE},
    {ITT_LRFUNC, "UHJVRJCNM PDERF",  M_RD_SfxVolume, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,               NULL,           0, MENU_NONE},
    {ITT_LRFUNC, "UHJVRJCNM VEPSRB", M_RD_MusVolume, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,               NULL,           0, MENU_NONE}
};

static Menu_t Options2Menu_Vanilla = {
    90, 20,
    DrawOptions2Menu_Vanilla,
    6, Options2Items_Vanilla,
    0,
    MENU_OPTIONS_VANILLA
};

static Menu_t Options2Menu_Rus_Vanilla = {
    90, 20,
    DrawOptions2Menu_Vanilla,
    6, Options2Items_Rus_Vanilla,
    0,
    MENU_OPTIONS_VANILLA
};

// -----------------------------------------------------------------------------

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
    &AutomapMenu,
    &SoundMenu,
    &SoundSysMenu,
    &ControlsMenu,
    &Gameplay1Menu,
    &Gameplay2Menu,
    &OptionsMenu_Vanilla,
    &Options2Menu_Vanilla
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
    &AutomapMenu_Rus,
    &SoundMenu_Rus,
    &SoundSysMenu_Rus,
    &ControlsMenu_Rus,
    &Gameplay1Menu_Rus,
    &Gameplay2Menu_Rus,
    &OptionsMenu_Rus_Vanilla,
    &Options2Menu_Rus_Vanilla
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

// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
static int G_ReloadLevel(void)
{
    int result = false;

    if (gamestate == GS_LEVEL)
    {
        // [crispy] restart demos from the map they were started
        if (demorecording)
        {
            gamemap = startmap;
        }
        G_DeferedInitNew(gameskill, gameepisode, gamemap);
        result = true;
    }

    return result;
}

static int G_GotoNextLevel(void)
{
    byte heretic_next[6][9] = {
    {12, 13, 14, 15, 16, 19, 18, 21, 17},
    {22, 23, 24, 29, 26, 27, 28, 31, 25},
    {32, 33, 34, 39, 36, 37, 38, 41, 35},
    {42, 43, 44, 49, 46, 47, 48, 51, 45},
    {52, 53, 59, 55, 56, 57, 58, 61, 54},
    {62, 63, 11, 11, 11, 11, 11, 11, 11}, // E6M4-E6M9 shouldn't be accessible
    };

    int changed = false;

    if (gamemode == shareware)
    heretic_next[0][7] = 11;

    if (gamemode == registered)
    heretic_next[2][7] = 11;

    if (gamestate == GS_LEVEL)
    {
        int epsd, map;

        epsd = heretic_next[gameepisode-1][gamemap-1] / 10;
        map = heretic_next[gameepisode-1][gamemap-1] % 10;

        G_DeferedInitNew(gameskill, epsd, map);
        changed = true;
    }

    return changed;
}


//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    InitFonts();
    menuactive = false;
    SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;

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
        if (c < 33) // [JN] Means space symbol (" ").
        {
            x += 4;
        }
        else
        {
            p = W_CacheLumpNum(FontCBaseLump + c - 33, PU_CACHE);
            V_DrawPatch(x, y, p);
            x += SHORT(p->width);
        }
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
    "DS LTQCNDBNTKMYJ ;TKFTNT DSQNB?",            // ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ВЫЙТИ?
    "DS LTQCNDBNTKMYJ ;TKFTNT PFRJYXBNM BUHE?",   // ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ЗАКОНЧИТЬ ИГРУ?
    "DSGJKYBNM ,SCNHJT CJ[HFYTYBT BUHS:",         // ВЫПОЛНИТЬ БЫСТРОЕ СОХРАНЕНИЕ ИГРЫ:
    "DSGJKYBNM ,SCNHE. PFUHEPRE BUHS:",           // ВЫПОЛНИТЬ БЫСТРУЮ ЗАГРУЗКУ ИГРЫ:
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
            {
                MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2 
                                    + wide_delta, 80);
            }
            else
            {
                MN_DrTextSmallRUS(message, 160 - MN_DrTextSmallRUSWidth(message) / 2 
                                           + wide_delta, 80);
            }

            if (typeofask == 3)
            {
                MN_DrTextA(SlotText[quicksave - 1], 160 -
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2 + wide_delta, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2 + wide_delta, 90);
            }
            if (typeofask == 4)
            {
                MN_DrTextA(SlotText[quickload - 1], 160 -
                           MN_TextAWidth(SlotText[quickload - 1]) / 2 + wide_delta, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quickload - 1]) / 2 + wide_delta, 90);
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
                    ||  CurrentMenu == &OptionsMenu
                    ||  CurrentMenu == &OptionsMenu_Vanilla
                    ||  CurrentMenu == &Options2Menu_Vanilla)
                    {
                        MN_DrTextBigENG(DEH_String(item->text), x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextSmallENG(DEH_String(item->text), x + wide_delta, y);
                    }
                }
                else
                {
                    if (CurrentMenu == &MainMenu_Rus
                    ||  CurrentMenu == &EpisodeMenu_Rus
                    ||  CurrentMenu == &FilesMenu_Rus
                    ||  CurrentMenu == &SkillMenu_Rus
                    ||  CurrentMenu == &OptionsMenu_Rus
                    ||  CurrentMenu == &OptionsMenu_Rus_Vanilla
                    ||  CurrentMenu == &Options2Menu_Rus_Vanilla)
                    {
                        MN_DrTextBigRUS(DEH_String(item->text), x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextSmallRUS(DEH_String(item->text), x + wide_delta, y);
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
            ||  CurrentMenu == &OptionsMenu_Rus
            ||  CurrentMenu == &OptionsMenu_Vanilla
            ||  CurrentMenu == &OptionsMenu_Rus_Vanilla
            ||  CurrentMenu == &Options2Menu_Vanilla
            ||  CurrentMenu == &Options2Menu_Rus_Vanilla)
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
        ||  CurrentMenu == &OptionsMenu_Rus
        ||  CurrentMenu == &OptionsMenu_Vanilla
        ||  CurrentMenu == &OptionsMenu_Rus_Vanilla
        ||  CurrentMenu == &Options2Menu_Vanilla
        ||  CurrentMenu == &Options2Menu_Rus_Vanilla)
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET + wide_delta, y,
                                     W_CacheLumpName(selName, PU_CACHE));
        }
        else
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT_SMALL) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 8 ? "INVGEMR1" : "INVGEMR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET_SMALL + wide_delta, y,
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
    V_DrawShadowedPatchRaven(88 + wide_delta, 0,
                             W_CacheLumpName(DEH_String("M_HTIC"), PU_CACHE));
    V_DrawShadowedPatchRaven(40 + wide_delta, 10,
                             W_CacheLumpNum(SkullBaseLump + (17 - frame), PU_CACHE));
    V_DrawShadowedPatchRaven(232 + wide_delta, 10,
                             W_CacheLumpNum(SkullBaseLump + frame, PU_CACHE));
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
    {
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);
    }
    else
    {
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 7);
    }
    
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
    {
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);
    }
    else
    {
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 7);
    }

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
        V_DrawShadowedPatchRaven(x + wide_delta, y,
                                 W_CacheLumpName(DEH_String("M_FSLOT"), PU_CACHE));
        if (SlotStatus[i])
        {
            // [JN] Use only small English chars here
            MN_DrTextSmallENG(SlotText[i], x + 5 + wide_delta, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}

// -----------------------------------------------------------------------------
// DrawOptionsMenu
// -----------------------------------------------------------------------------

static void DrawOptionsMenu(void)
{
    char *title_eng = DEH_String("OPTIONS");
    char *title_rus = DEH_String("YFCNHJQRB");  // НАСТРОЙКИ
    
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    // Update status bar
    SB_state = -1;

    if (english_language)
    {
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);
    }
    else
    {
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);
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
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // RENDERING
        // 
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("RENDERING"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Display aspect ratio
        MN_DrTextSmallENG(DEH_String(aspect_ratio_temp == 1 ? "5:4" :
                                     aspect_ratio_temp == 2 ? "16:9" :
                                     aspect_ratio_temp == 3 ? "16:10" :
                                     aspect_ratio_temp == 4 ? "21:9" :
                                                              "4:3"),
                                     185 + wide_delta, 42);
        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY2GREEN_HERETIC];
            MN_DrTextSmallENG(DEH_String("THE PROGRAM MUST BE RESTARTED"),
                                         51 + wide_delta, 138);
            dp_translation = NULL;
        }

        // Vertical sync
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallENG(DEH_String("N/A"), 216 + wide_delta, 52);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallENG(DEH_String(vsync ? "ON" : "OFF"),
                                        216 + wide_delta, 52);
        }

        // Uncapped FPS
        MN_DrTextSmallENG(DEH_String(uncapped_fps ? "UNCAPPED" : "35 FPS"),
                                     120 + wide_delta, 62);

        // FPS counter
        MN_DrTextSmallENG(DEH_String(show_fps ? "ON" : "OFF"),
                                     129 + wide_delta, 72);

        // Pixel scaling
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallENG(DEH_String("N/A"), 131 + wide_delta, 82);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallENG(DEH_String(smoothing ? "SMOOTH" : "SHARP"),
                                        131 + wide_delta, 82);
        }

        // Porch palette changing
        MN_DrTextSmallENG(DEH_String(vga_porch_flash ? "ON" : "OFF"),
                                     205 + wide_delta, 92);

        // Video renderer
        MN_DrTextSmallENG(DEH_String(force_software_renderer ?
                                     "SOFTWARE (CPU)" : "HARDWARE (GPU)"),
                                     149 + wide_delta, 102);

        //
        // EXTRA
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("EXTRA"), 36 + wide_delta, 112);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // РЕНДЕРИНГ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("HTYLTHBYU"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Соотношение сторон экрана
        MN_DrTextSmallENG(DEH_String(aspect_ratio_temp == 1 ? "5:4" :
                                     aspect_ratio_temp == 2 ? "16:9" :
                                     aspect_ratio_temp == 3 ? "16:10" :
                                     aspect_ratio_temp == 4 ? "21:9" :
                                                              "4:3"),
                                     230 + wide_delta, 42);

        // Informative message: НЕОБХОДИМ ПЕРЕЗАПУСК ИГРЫ
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY2GREEN_HERETIC];
            MN_DrTextSmallRUS(DEH_String("YTJ,[JLBV GTHTPFGECR GHJUHFVVS"), 
                                         46 + wide_delta, 138);
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallRUS(DEH_String("Y/L"), 236 + wide_delta, 52);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallRUS(DEH_String(vsync ? "DRK" : "DSRK"),
                                        236 + wide_delta, 52);
        }

        // Кадровая частота
        if (uncapped_fps)
        MN_DrTextSmallRUS(DEH_String(",TP JUHFYBXTYBZ"), 165 + wide_delta, 62);
        else
        MN_DrTextSmallENG(DEH_String("35 FPS"), 165 + wide_delta, 62);

        // Счетчик кадровой частоты
        MN_DrTextSmallRUS(DEH_String(show_fps ? "DRK" : "DSRK"),
                                     223 + wide_delta, 72);

        // Пиксельное сглаживание
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallRUS(DEH_String("Y/L"), 211 + wide_delta, 82);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallRUS(DEH_String(smoothing ? "DRK" : "DSRK"),
                                        211 + wide_delta, 82);
        }

        // Изменение палитры краев экрана
        MN_DrTextSmallRUS(DEH_String(vga_porch_flash ? "DRK" : "DSRK"),
                                     265 + wide_delta, 92);

        // Обработка видео
        MN_DrTextSmallRUS(DEH_String(force_software_renderer ?
                                     "GHJUHFVVYFZ" : "FGGFHFNYFZ"),
                                     159 + wide_delta, 102);

        //
        // ДОПОЛНИТЕЛЬНО
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + wide_delta, 112);
        dp_translation = NULL;
    }

    // Screenshot format / Формат скриншотов (same english values)
    MN_DrTextSmallENG(DEH_String(png_screenshots ? "PNG" : "PCX"),
                                 175 + wide_delta, 122);

}

static boolean M_RD_Change_Widescreen(int option)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in MN_Init and stored into config file in M_QuitResponse.
    switch(option)
    {
        case 0:
        aspect_ratio_temp--;
        if (aspect_ratio_temp < 0)
            aspect_ratio_temp = 4;
        break;

        case 1:
        aspect_ratio_temp++;
        if (aspect_ratio_temp > 4)
            aspect_ratio_temp = 0;
        break;
    }
    return true;
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
    // [JN] Disable smoothing toggling in software renderer
    if (force_software_renderer == 1)
    return false;

    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Update status bar
    SB_state = -1;

    return true;
}

static boolean M_RD_PorchFlashing(int option)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();

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
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // SCREEN, INTERFACE
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("SCREEN"), 36 + wide_delta, 32);
        MN_DrTextSmallENG(DEH_String("INTERFACE"), 36 + wide_delta, 102);
        dp_translation = NULL;

        // Local time
        MN_DrTextSmallENG(DEH_String(
                          local_time == 1 ? "12-HOUR (HH:MM)" :
                          local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                          local_time == 3 ? "24-HOUR (HH:MM)" :
                          local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF"),
                          110 + wide_delta, 112);

        // Messages
        MN_DrTextSmallENG(DEH_String(show_messages ? "ON" : "OFF"),
                                     108 + wide_delta, 122);

        // Text casts shadows
        MN_DrTextSmallENG(DEH_String(draw_shadowed_text ? "ON" : "OFF"),
                                     179 + wide_delta, 132);
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // ЭКРАН, ИНТЕРФЕЙС
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("\'RHFY"), 36 + wide_delta, 32);
        MN_DrTextSmallRUS(DEH_String("BYNTHATQC"), 36 + wide_delta, 102);
        dp_translation = NULL;

        // Системное время
        MN_DrTextSmallRUS(DEH_String(
                          local_time == 1 ? "12-XFCJDJT (XX:VV)" :
                          local_time == 2 ? "12-XFCJDJT (XX:VV:CC)" :
                          local_time == 3 ? "24-XFCJDJT (XX:VV)" :
                          local_time == 4 ? "24-XFCJDJT (XX:VV:CC)" : "DSRK"),
                          157 + wide_delta, 112);

        // Отображение сообщений
        MN_DrTextSmallRUS(DEH_String(show_messages ? "DRK" : "DSRK"),
                                     208 + wide_delta, 122);

        // Тексты отбрасывают тень
        MN_DrTextSmallRUS(DEH_String(draw_shadowed_text ? "DRK" : "DSRK"),
                                     220 + wide_delta, 132);
    }

    //
    // Sliders
    //

    // Screen size
    if (aspect_ratio_temp >= 2)
    {
        DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 4, screenblocks - 9);
        M_snprintf(num, 4, "%3d", screenblocks);
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextSmallENG(num, 85 + wide_delta, 52);
        dp_translation = NULL;
    }
    else
    {
        DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 10, screenblocks - 3);
        M_snprintf(num, 4, "%3d", screenblocks);
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextA(num, 135 + wide_delta, 52);
        dp_translation = NULL;
    }

    // Gamma-correction
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 3, 18, usegamma);

    // Level brightness
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 5, 5, level_brightness / 16);
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

    if (aspect_ratio_temp >= 2)
    {
        // [JN] Wide screen: don't allow unsupported (bordered) views
        // screenblocks - config file variable
        if (screenblocks < 9)
            screenblocks = 9;
        if (screenblocks > 12)
            screenblocks = 12;
    }

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

static boolean M_RD_LevelBrightness(int option)
{
    switch(option)
    {
        case 0:
        if (level_brightness > 0)
            level_brightness -= 16;
        break;

        case 1:
        if (level_brightness < 64)
            level_brightness += 16;
        break;
    }

    return true;
}

static boolean M_RD_LocalTime(int option)
{
    switch(option)
    {
        case 0: 
        local_time--;
        if (local_time < 0) 
            local_time = 4;
        break;

        case 1:
        local_time++;
        if (local_time > 4)
            local_time = 0;
        break;
    }

    return true;
}

static boolean M_RD_Messages(int option)
{
    show_messages ^= 1;

    if (show_messages)
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
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("AUTOMAP SETTINGS");
    title_rus = DEH_String("YFCNHJQRB RFHNS");  // НАСТРОЙКИ КАРТЫ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        // Level stats
        MN_DrTextSmallENG(DEH_String(automap_stats ? "ON" : "OFF"),
                                     187 + wide_delta, 32);

        // Overlay mode
        MN_DrTextSmallENG(DEH_String(automap_overlay ? "ON" : "OFF"),
                                     200 + wide_delta, 42);

        // Rotate mode
        MN_DrTextSmallENG(DEH_String(automap_rotate ? "ON" : "OFF"),
                                     193 + wide_delta, 52);

        // Follow mode
        MN_DrTextSmallENG(DEH_String(automap_follow ? "ON" : "OFF"),
                                     189 + wide_delta, 62);

        // Grid
        MN_DrTextSmallENG(DEH_String(automap_grid ? "ON" : "OFF"),
                                     138 + wide_delta, 72);
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        // Статистика уровня
        MN_DrTextSmallRUS(DEH_String(automap_stats ? "DRK" : "DSRK"),
                                     214 + wide_delta, 32);

        // Режим наложения
        MN_DrTextSmallRUS(DEH_String(automap_overlay ? "DRK" : "DSRK"),
                                     208 + wide_delta, 42);

        // Режим вращения
        MN_DrTextSmallRUS(DEH_String(automap_rotate ? "DRK" : "DSRK"),
                                     200 + wide_delta, 52);

        // Режим следования
        MN_DrTextSmallRUS(DEH_String(automap_follow ? "DRK" : "DSRK"),
                                     215 + wide_delta, 62);

        // Сетка
        MN_DrTextSmallRUS(DEH_String(automap_grid ? "DRK" : "DSRK"),
                                     128 + wide_delta, 72);
    }
}

static boolean M_RD_AutoMapStats(int option)
{
    automap_stats ^= 1;
    return true;
}

static boolean M_RD_AutoMapOverlay(int option)
{
    automap_overlay ^= 1;
    return true;
}

static boolean M_RD_AutoMapRotate(int option)
{
    automap_rotate ^= 1;
    return true;
}

static boolean M_RD_AutoMapFollow(int option)
{
    automap_follow ^= 1;
    return true;
}

static boolean M_RD_AutoMapGrid(int option)
{
    automap_grid ^= 1;
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
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // VOLUME, CHANNELS, ADVANCED
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("VOLUME"), 36 + wide_delta, 32);
        MN_DrTextSmallENG(DEH_String("CHANNELS"), 36 + wide_delta, 82);
        MN_DrTextSmallENG(DEH_String("ADVANCED"), 36 + wide_delta, 112);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // ГРОМКОСТЬ, ВОСПРОИЗВЕДЕНИЕ, ДОПОЛНИТЕЛЬНО
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHJVRJCNM"), 36 + wide_delta, 32);
        MN_DrTextSmallRUS(DEH_String("DJCGHJBPDTLTYBT"), 36 + wide_delta, 82);
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + wide_delta, 112);
        dp_translation = NULL;
    }

    //
    // Sliders
    //

    // SFX Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 1, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%3d", snd_MaxVolume_tmp);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 53);
    dp_translation = NULL;

    // Music Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 3, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 73);
    dp_translation = NULL;

    // SFX Channels
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 6, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%3d", snd_Channels);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 103);
    dp_translation = NULL;
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

    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable.

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

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundSystemMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = DEH_String("SOUND SYSTEM SETTINGS");
    title_rus = DEH_String("YFCNHJQRB PDERJDJQ CBCNTVS");  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // SOUND SYSTEM
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("SOUND SYSTEM"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Sound effects
        if (snd_sfxdevice == 0)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallENG(DEH_String("DISABLED"), 144 + wide_delta, 42);
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 3)
        {
            MN_DrTextSmallENG(DEH_String("DIGITAL SFX"), 144 + wide_delta, 42);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallENG(DEH_String("DISABLED"), 80 + wide_delta, 52);
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            MN_DrTextSmallENG(DEH_String("OPL2 SYNTH"), 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            MN_DrTextSmallENG(DEH_String("OPL3 SYNTH"), 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 5)
        {
            MN_DrTextSmallENG(DEH_String("GUS EMULATION"), 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            MN_DrTextSmallENG(DEH_String("MIDI/MP3/OGG/FLAC"), 80 + wide_delta, 52);
        }

        //
        // QUALITY
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("QUALITY"), 36 + wide_delta, 62);
        dp_translation = NULL;

        // Sampling frequency (hz)
        if (snd_samplerate == 44100)
        {
            MN_DrTextSmallENG(DEH_String("44100 HZ"), 178 + wide_delta, 72);
        }
        else if (snd_samplerate == 22050)
        {
            MN_DrTextSmallENG(DEH_String("22050 HZ"), 178 + wide_delta, 72);
        }
        else if (snd_samplerate == 11025)
        {
            MN_DrTextSmallENG(DEH_String("11025 HZ"), 178 + wide_delta, 72);
        }

        //
        // MISCELLANEOUS
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MISCELLANEOUS"), 36 + wide_delta, 82);
        dp_translation = NULL;

        // SFX Mode
        MN_DrTextSmallENG(DEH_String(snd_monomode ? "MONO" : "STEREO"),
                                     181 + wide_delta, 92);

        // Pitch-Shifted sounds
        MN_DrTextSmallENG(DEH_String(snd_pitchshift ? "ON" : "OFF"),
                                     189 + wide_delta, 102);

        // Mute inactive window
        MN_DrTextSmallENG(DEH_String(mute_inactive_window ? "ON" : "OFF"),
                                     184 + wide_delta, 112);

        // Informative message:
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
        {
            dp_translation = cr[CR_GRAY2GREEN_HERETIC];
            MN_DrTextSmallENG(DEH_String("CHANGING WILL REQUIRE RESTART OF THE PROGRAM"),
                                         3 + wide_delta, 132);
            dp_translation = NULL;
        }
    }
    else
    {

        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // ЗВУКВАЯ СИСТЕМА
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("PDERJDFZ CBCNTVF"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallRUS(DEH_String("JNRK.XTYS"), 173 + wide_delta, 42);
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 3)
        {
            // ЦИФРОВЫЕ
            MN_DrTextSmallRUS(DEH_String("WBAHJDST"), 173 + wide_delta, 42);
        }

        // Музыка
        if (snd_musicdevice == 0)
        {   
            // ОТКЛЮЧЕНА
            dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
            MN_DrTextSmallRUS(DEH_String("JNRK.XTYF"), 91 + wide_delta, 52);
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // СИНТЕЗ OPL2
            MN_DrTextSmallRUS(DEH_String("CBYNTP J"), 91 + wide_delta, 52);
            MN_DrTextSmallENG(DEH_String("OPL2"), 140 + wide_delta, 52);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // СИНТЕЗ OPL3
            MN_DrTextSmallRUS(DEH_String("CBYNTP J"), 91 + wide_delta, 52);
            MN_DrTextSmallENG(DEH_String("OPL3"), 140 + wide_delta, 52);
        }
        else if (snd_musicdevice == 5)
        {
            // ЭМУЛЯЦИЯ GUS
            MN_DrTextSmallRUS(DEH_String("\'VEKZWBZ"), 91 + wide_delta, 52);
            MN_DrTextSmallENG(DEH_String("GUS"), 155 + wide_delta, 52);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            MN_DrTextSmallENG(DEH_String("MIDI/MP3/OGG/FLAC"), 91 + wide_delta, 52);
        }

        //
        // КАЧЕСТВО ЗВУЧАНИЯ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("RFXTCNDJ PDEXFYBZ"), 36 + wide_delta, 62);
        dp_translation = NULL;

        // Частота дискретизации (гц)
        if (snd_samplerate == 44100)
        {
            MN_DrTextSmallRUS(DEH_String("44100 UW"), 200 + wide_delta, 72);
        }
        else if (snd_samplerate == 22050)
        {
            MN_DrTextSmallRUS(DEH_String("22050 UW"), 200 + wide_delta, 72);
        }
        else if (snd_samplerate == 11025)
        {
            MN_DrTextSmallRUS(DEH_String("11025 UW"), 200 + wide_delta, 72);
        }

        //
        // РАЗНОЕ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("HFPYJT"), 36 + wide_delta, 82);
        dp_translation = NULL;

        // Режим звуковых эффектов
        MN_DrTextSmallRUS(DEH_String(snd_monomode ? "VJYJ" : "CNTHTJ"),
                                     226 + wide_delta, 92);

        // Произвольный питч-шифтинг
        MN_DrTextSmallRUS(DEH_String(snd_pitchshift ? "DRK" : "DSRK"),
                                     230 + wide_delta, 102);

        // Звук в неактивном окне
        MN_DrTextSmallRUS(DEH_String(mute_inactive_window ? "DSRK" : "DRK"),
                                     201 + wide_delta, 112);

        // Informative message: ИЗМЕНЕНИЕ ПОТРЕБУЕТ ПЕРЕЗАПУСК ПРОГРАММЫ
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
        {
            dp_translation = cr[CR_GRAY2GREEN_HERETIC];
            MN_DrTextSmallRUS(DEH_String("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS"), 
                                         11 + wide_delta, 132);
            dp_translation = NULL;
        }
    }
}

static boolean M_RD_SoundDevice(int option)
{
    switch(option)
    {
        case 0:
        case 1:
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 3;
        else 
        if (snd_sfxdevice == 3)
            snd_sfxdevice = 0;
        break;
    }
    return true;
}

static boolean M_RD_MusicDevice(int option)
{
    switch(option)
    {
        case 0:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice = 8;    // Set to MIDI/MP3/OGG/FLAC
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice = 5;    // Set to GUS
            }
            else if (snd_musicdevice == 5)
            {
                snd_musicdevice = 3;    // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3  && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice = 3;    // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice = 0;    // Disable
            }
        break;
        case 1:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice  = 3;   // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice  = 3;   // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice  = 5;
            }
            else if (snd_musicdevice == 5)
            {
                snd_musicdevice  = 8;   // Set to MIDI/MP3/OGG/FLAC
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice  = 0;   // Disable
            }
        break;

    }
    return true;
}

static boolean M_RD_Sampling(int option)
{
    switch(option)
    {
        case 0:
            if (snd_samplerate == 44100)
            {
                snd_samplerate = 22050;
            }
            else if (snd_samplerate == 22050)
            {
                snd_samplerate = 11025;
            }
            else if (snd_samplerate == 11025)
            {
                snd_samplerate  = 44100;
            }
        break;
        case 1:
            if (snd_samplerate == 11025)
            {
                snd_samplerate = 22050;
            }
            else if (snd_samplerate == 22050)
            {
                snd_samplerate = 44100;
            }
            else if (snd_samplerate == 44100)
            {
                snd_samplerate = 11025;
            }
        break;
    }
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

static boolean M_RD_MuteInactive(int option)
{
    mute_inactive_window ^= 1;
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
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // MOVEMENT
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MOVEMENT"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Always run
        MN_DrTextSmallENG(DEH_String(joybspeed >= 20 ? "ON" : "OFF"),
                                     118 + wide_delta, 42);

        //
        // MOUSE
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MOUSE"), 36 + wide_delta, 52);
        dp_translation = NULL;

        // Mouse look
        MN_DrTextSmallENG(DEH_String(mlook ? "ON" : "OFF"),
                                     118 + wide_delta, 122);

        // Invert Y axis
        if (!mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextSmallENG(DEH_String(mouse_y_invert ? "ON" : "OFF"),
                                     133 + wide_delta, 132);
        dp_translation = NULL;

        // Novert
        if (mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextSmallENG(DEH_String(!novert ? "ON" : "OFF"),
                                     168 + wide_delta, 142);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // ПЕРЕДВИЖЕНИЕ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("GTHTLDB;TYBT"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Режим постоянного бега
        MN_DrTextSmallRUS(DEH_String(joybspeed >= 20 ? "DRK" : "DSRK"),
                                     209 + wide_delta, 42);

        //
        // МЫШЬ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("VSIM"), 36 + wide_delta, 52);
        dp_translation = NULL;

        // Обзор мышью
        MN_DrTextSmallRUS(DEH_String(mlook ? "DRK" : "DSRK"),
                                     132 + wide_delta, 122);

        // Вертикальная инверсия
        if (!mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextSmallRUS(DEH_String(mouse_y_invert ? "DRK" : "DSRK"),
                                     199 + wide_delta, 132);
        dp_translation = NULL;

        // Вертикальное перемещение
        if (mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
        MN_DrTextSmallRUS(DEH_String(!novert ? "DRK" : "DSRK"),
                                     227 + wide_delta, 142);
        dp_translation = NULL;
    }

    //
    // Sliders
    //

    // Mouse sensivity
    DrawSliderSmall((english_language ? &ControlsMenu : &ControlsMenu_Rus), 3, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 73);
    dp_translation = NULL;

    // Acceleration
    DrawSliderSmall((english_language ? &ControlsMenu : &ControlsMenu_Rus), 5, 12, mouse_acceleration * 4 - 4);
    M_snprintf(num, 4, "%f", mouse_acceleration);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 93);
    dp_translation = NULL;

    // Threshold
    DrawSliderSmall((english_language ? &ControlsMenu : &ControlsMenu_Rus), 7, 12, mouse_threshold / 2);
    M_snprintf(num, 4, "%3d", mouse_threshold);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 113);
    dp_translation = NULL;
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

static boolean M_RD_Acceleration(int option)
{
    switch (option)
    {
        case 0:
        if (mouse_acceleration > 1.1)
            mouse_acceleration -= 0.1;
        break;

        case 1:
        if (mouse_acceleration < 5.0)
            mouse_acceleration += 0.1;
        break;
    }

    return true;
}

static boolean M_RD_Threshold(int option)
{
    switch (option)
    {
        case 0:
        if (mouse_threshold > 0)
            mouse_threshold--;
        break;

        case 1:
        if (mouse_threshold < 32)
            mouse_threshold++;
        break;
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

static boolean M_RD_InvertY(int option)
{
    mouse_y_invert ^= 1;
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
    char *title_eng = DEH_String("GAMEPLAY FEATURES");
    char *title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 + wide_delta, 4);

        //
        // VISUAL
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("VISUAL"), 36 + wide_delta, 26);
        dp_translation = NULL;

        // Brightmaps
        dp_translation = brightmaps ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(brightmaps ? "ON" : "OFF"), 119 + wide_delta, 36);
        dp_translation = NULL;

        // Fake contrast
        dp_translation = fake_contrast ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(fake_contrast ? "ON" : "OFF"), 143 + wide_delta, 46);
        dp_translation = NULL;

        // Extra translucency
        dp_translation = translucency ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(translucency ? "ON" : "OFF"), 180 + wide_delta, 56);
        dp_translation = NULL;

        // Colored HUD
        dp_translation = sbar_colored ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(sbar_colored ? "ON" : "OFF"), 126 + wide_delta, 66);
        dp_translation = NULL;

        // Colored blood
        dp_translation = colored_blood ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(colored_blood ? "ON" : "OFF"), 139 + wide_delta, 76);
        dp_translation = NULL;

        // Invulnerability affects sky
        dp_translation = invul_sky ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(invul_sky ? "ON" : "OFF"), 235 + wide_delta, 86);
        dp_translation = NULL;

        //
        // PHYSICAL
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("PHYSICAL"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Corpses sliding from the ledges
        dp_translation = torque ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(torque ? "ON" : "OFF"), 238 + wide_delta, 106);
        dp_translation = NULL;

        // Weapon bobbing while firing
        dp_translation = weapon_bobbing ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(weapon_bobbing ? "ON" : "OFF"), 233 + wide_delta, 116);
        dp_translation = NULL;

        // Randomly flipped corpses
        dp_translation = randomly_flipcorpses ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(randomly_flipcorpses ? "ON" : "OFF"), 232 + wide_delta, 126);
        dp_translation = NULL;

        // Floating items amplitude
        dp_translation = floating_powerups ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(floating_powerups == 1 ? "STANDARD" :
                                     floating_powerups == 2 ? "HALFED" : "OFF"),
                                     209 + wide_delta, 136);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 4);

        //
        // ГРАФИКА
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHFABRF"), 36 + wide_delta, 26);
        dp_translation = NULL;

        // Брайтмаппинг
        dp_translation = brightmaps ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(brightmaps ? "DRK" : "DSRK"), 133 + wide_delta, 36);
        dp_translation = NULL;

        // Имитация контрастности
        dp_translation = fake_contrast ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(fake_contrast ? "DRK" : "DSRK"), 205 + wide_delta, 46);
        dp_translation = NULL;

        // Дополнительная прозрачность
        dp_translation = translucency ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(translucency ? "DRK" : "DSRK"), 245 + wide_delta, 56);
        dp_translation = NULL;

        // Разноцветные элемен HUD
        dp_translation = sbar_colored ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(sbar_colored ? "DRK" : "DSRK"), 235 + wide_delta, 66);
        dp_translation = NULL;

        // Разноцветная кровь
        dp_translation = colored_blood ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(colored_blood ? "DRK" : "DSRK"), 178 + wide_delta, 76);          
        dp_translation = NULL;

        // Неуязвимость окрашивает небо
        dp_translation = invul_sky ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(invul_sky ? "DRK" : "DSRK"), 253 + wide_delta, 86);
        dp_translation = NULL;

        //
        // ФИЗИКА
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("ABPBRF"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Трупы сползают с возвышений
        dp_translation = torque ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(torque ? "DRK" : "DSRK"), 248 + wide_delta, 106);
        dp_translation = NULL;

        // Улучшенное покачивание оружия
        dp_translation = weapon_bobbing ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(weapon_bobbing ? "DRK" : "DSRK"), 260 + wide_delta, 116);
        dp_translation = NULL;

        // Зеркалирование трупов
        dp_translation = randomly_flipcorpses ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(randomly_flipcorpses ? "DRK" : "DSRK"), 201 + wide_delta, 126);
        dp_translation = NULL;

        // Амплитуда левитации предметов
        dp_translation = floating_powerups ? cr[CR_GRAY2GREEN_HERETIC] : cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(floating_powerups == 1 ? "CNFYLFHNYFZ" :
                                     floating_powerups == 2 ? "EGJKJDBYTYFZ" : "DSRK"),
                                     188 + wide_delta, 136);
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

static boolean M_RD_ExtraTrans(int option)
{
    translucency ^= 1;
    return true;
}

static boolean M_RD_ColoredHUD(int option)
{
    sbar_colored ^= 1;
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
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 4);

        //
        // TACTICAL
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("TACTICAL"), 36 + wide_delta, 26);
        dp_translation = NULL;

        // Notify of revealed secrets
        dp_translation = secret_notification ? cr[CR_GRAY2GREEN_HERETIC] :
                                               cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(secret_notification ? "ON" : "OFF"),
                                     235 + wide_delta, 36);
        dp_translation = NULL;

        // Negative health
        dp_translation = negative_health ? cr[CR_GRAY2GREEN_HERETIC] :
                                           cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(negative_health ? "ON" : "OFF"),
                                     190 + wide_delta, 46);
        dp_translation = NULL;

        //
        // CROSSHAIR
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("CROSSHAIR"), 36 + wide_delta, 56);
        dp_translation = NULL;

        // Draw crosshair
        dp_translation = crosshair_draw ? cr[CR_GRAY2GREEN_HERETIC] :
                                          cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_draw ? "ON" : "OFF"),
                                     150 + wide_delta, 66);
        dp_translation = NULL;

        // Indication
        dp_translation = crosshair_type ? cr[CR_GRAY2GREEN_HERETIC] :
                                          cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_type == 1 ? "HEALTH" :
                                     crosshair_type == 2 ? "TARGET HIGHLIGHTING" :
                                     crosshair_type == 3 ? "TARGET HIGHLIGHTING+HEALTH" :
                                                            "STATIC"),
                                     111 + wide_delta, 76);
        dp_translation = NULL;

        // Increased size
        dp_translation = crosshair_scale ? cr[CR_GRAY2GREEN_HERETIC] :
                                           cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_scale ? "ON" : "OFF"),
                                     146 + wide_delta, 86);
        dp_translation = NULL;

        //
        // GAMEPLAY
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("GAMEPLAY"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Flip game levels
        dp_translation = flip_levels ? cr[CR_GRAY2GREEN_HERETIC] :
                                       cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(flip_levels ? "ON" : "OFF"),
                                     153 + wide_delta, 106);
        dp_translation = NULL;

        // Play internal demos
        dp_translation = no_internal_demos ? cr[CR_GRAY2RED_HERETIC] :
                                             cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextSmallENG(DEH_String(no_internal_demos ? "OFF" : "ON"),
                                     179 + wide_delta, 116);
        dp_translation = NULL;

        // Wand start
        dp_translation = pistol_start ? cr[CR_GRAY2GREEN_HERETIC] :
                                        cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(pistol_start ? "ON" : "OFF"),
                                     193 + wide_delta, 126);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 4);

        //
        // ТАКТИКА
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("NFRNBRF"), 36 + wide_delta, 26);
        dp_translation = NULL;

        // Сообщать о найденном тайнике
        dp_translation = secret_notification ? cr[CR_GRAY2GREEN_HERETIC] :
                                               cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(secret_notification ? "DRK" : "DSRK"),
                                     251 + wide_delta, 36);
        dp_translation = NULL;

        // Отрицательное здоровье в HUD
        dp_translation = negative_health ? cr[CR_GRAY2GREEN_HERETIC] :
                                           cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(negative_health ? "DRK" : "DSRK"),
                                     253 + wide_delta, 46);
        dp_translation = NULL;

        //
        // ПРИЦЕЛ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("GHBWTK"), 36 + wide_delta, 56);
        dp_translation = NULL;

        // Отображать прицел
        dp_translation = crosshair_draw ? cr[CR_GRAY2GREEN_HERETIC] :
                                          cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_draw ? "DRK" : "DSRK"),
                                     175 + wide_delta, 66);
        dp_translation = NULL;

        // Индикация
        dp_translation = crosshair_type ? cr[CR_GRAY2GREEN_HERETIC] :
                                          cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_type == 1 ? "PLJHJDMT" :       // ЗДОРОВЬЕ
                                     crosshair_type == 2 ? "GJLCDTNRF WTKB" : // ПОДСВЕТКА ЦЕЛИ
                                     crosshair_type == 3 ? "GJLCDTNRF WTKB+PLJHJDMT" :
                                                           "CNFNBXYFZ"),      // СТАТИЧНАЯ
                                     111 + wide_delta, 76);
        dp_translation = NULL;

        // Увеличенный размер
        dp_translation = crosshair_scale ? cr[CR_GRAY2GREEN_HERETIC] :
                                           cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_scale ? "DRK" : "DSRK"),
                                     181 + wide_delta, 86);
        dp_translation = NULL;

        //
        // ГЕЙМПЛЕЙ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UTQVGKTQ"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Зеркальное отражение уровней
        dp_translation = flip_levels ? cr[CR_GRAY2GREEN_HERETIC] :
                                       cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(flip_levels ? "DRK" : "DSRK"),
                                     255 + wide_delta, 106);
        dp_translation = NULL;

        // Проигрывать демозаписи
        dp_translation = no_internal_demos ? cr[CR_GRAY2RED_HERETIC] :
                                             cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextSmallRUS(DEH_String(no_internal_demos ? "DSRK" : "DRK"),
                                     211 + wide_delta, 116);
        dp_translation = NULL;

        // Режим игры "Wand start"
        MN_DrTextSmallRUS(DEH_String("HT;BV BUHS"), 36 + wide_delta, 126);
        MN_DrTextSmallENG(DEH_String("\"WAND START\":"), 120 + wide_delta, 126);
        dp_translation = pistol_start ? cr[CR_GRAY2GREEN_HERETIC] :
                                        cr[CR_GRAY2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(pistol_start ? "DRK" : "DSRK"),
                                     217 + wide_delta, 126);
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

static boolean M_RD_FloatAmplitude(int option)
{
    switch(option)
    {
        case 0: 
        floating_powerups--;
        if (floating_powerups < 0) 
            floating_powerups = 2;
        break;
    
        case 1:
        floating_powerups++;
        if (floating_powerups > 2)
            floating_powerups = 0;
        break;
    }

    return true;
}

static boolean M_RD_CrossHairDraw(int option)
{
    crosshair_draw ^= 1;
    return true;
}

static boolean M_RD_CrossHairType(int option)
{
    switch(option)
    {
        case 0: 
        crosshair_type--;
        if (crosshair_type < 0) 
            crosshair_type = 3;
        break;
    
        case 1:
        crosshair_type++;
        if (crosshair_type > 3)
            crosshair_type = 0;
        break;
    }

    return true;
}

static boolean M_RD_CrossHairScale(int option)
{
    crosshair_scale ^= 1;
    return true;
}

static boolean M_RD_FlipLevels(int option)
{
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

static boolean M_RD_WandStart(int option)
{
    pistol_start ^= 1;
    return true;
}

//---------------------------------------------------------------------------
// [JN] Vanilla Options menu 1 and 2
//---------------------------------------------------------------------------

static void DrawOptionsMenu_Vanilla(void)
{
    if (english_language)
    {
        MN_DrTextB(DEH_String(show_messages ? "ON" : "OFF"), 196 + wide_delta, 50);
    }
    else
    {
        MN_DrTextBigRUS(DEH_String(show_messages ? "DRK" : "DSRK"), 223 + wide_delta, 50);
    }
    DrawSlider(&OptionsMenu_Vanilla, 3, 10, mouseSensitivity);
}

static void DrawOptions2Menu_Vanilla(void)
{
    if (aspect_ratio_temp >= 2)
    {
        DrawSlider(&Options2Menu_Vanilla, 1, 4, screenblocks - 9);
    }
    else
    {
        DrawSlider(&Options2Menu_Vanilla, 1, 10, screenblocks - 3);
    }
    DrawSlider(&Options2Menu_Vanilla, 3, 16, snd_MaxVolume);
    DrawSlider(&Options2Menu_Vanilla, 5, 16, snd_MusicVolume);
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
    vga_porch_flash         = 0;
    force_software_renderer = 0;
    png_screenshots         = 1;

    // Display
    screenblocks        = 10;
    usegamma            = 4;
    level_brightness    = 0;
    local_time          = 0;
    show_messages       = 1;
    draw_shadowed_text  = 1;

    // Automap
    automap_stats   = 1;
    automap_follow  = 1;
    automap_overlay = 0;
    automap_rotate  = 0;
    automap_grid    = 0;

    // Audio
    snd_MaxVolume   = 8;
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu
    snd_MusicVolume = 8;
    S_SetMusicVolume();
    snd_Channels    = 32;
    S_ChannelsRealloc();
    snd_monomode    = 0;
    snd_pitchshift  = 1;
    mute_inactive_window = 0;

    // Controls
    joybspeed           = 29;
    mouseSensitivity    = 5;
    mlook               = 0;
    players[consoleplayer].centering = true;
    novert              = 1;    

    // Gameplay (1)
    brightmaps           = 1;
    fake_contrast        = 0;
    translucency         = 1;
    sbar_colored         = 0;
    colored_blood        = 1;
    invul_sky            = 1;
    torque               = 1;
    weapon_bobbing       = 1;
    randomly_flipcorpses = 1;

    // Gameplay (2)
    secret_notification  = 1;
    negative_health      = 0;
    crosshair_draw       = 0;
    crosshair_type       = 1;
    crosshair_scale      = 0;
    flip_levels          = 0;
    no_internal_demos    = 0;
    pistol_start         = 0;

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
    show_messages ^= 1;

    if (show_messages)
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
            // [JN] Widescreen: remember choosen widescreen variable before quit.
            aspect_ratio = aspect_ratio_temp;

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

            // [JN] Widescreen: remember choosen widescreen variable before quit.
            aspect_ratio = aspect_ratio_temp;
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
                    // [JN] Widescreen: remember choosen widescreen variable before quit.
                    aspect_ratio = aspect_ratio_temp;
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
            // [JN] Force to use vanilla options 2 menu in -vanilla game mode.
            if (vanillaparm)
            {
                CurrentMenu = english_language ? 
                            &Options2Menu_Vanilla :
                            &Options2Menu_Rus_Vanilla;
            }
            else
            {
                CurrentMenu = english_language ? 
                            &SoundMenu :
                            &SoundMenu_Rus;
            }
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
        // [crispy] those two can be considered as shortcuts for the ENGAGE cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        else if (!netgame && key != 0 && key == key_menu_reloadlevel)
        {
            if (G_ReloadLevel())
            return true;
        }
        else if (!netgame && key != 0 && key == key_menu_nextlevel)
        {
            if (G_GotoNextLevel())
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
                *textBuffer = 0;
                slotptr--;
                textBuffer = &SlotText[currentSlot][slotptr];
                *textBuffer = ASCII_CURSOR;
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
    // [JN] For checking of modified fullscreen graphics.
    patch_t *page0_gfx = W_CacheLumpName("TITLE", PU_CACHE);
    patch_t *page1_gfx = W_CacheLumpName("HELP1", PU_CACHE);
    patch_t *page2_gfx = W_CacheLumpName("HELP2", PU_CACHE);
    patch_t *page3_gfx = W_CacheLumpName("CREDIT", PU_CACHE);

    if (aspect_ratio_temp >= 2)
    {
        // [JN] Clean up remainings of the wide screen before 
        // drawing a HELP or TITLE screens.
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    I_SetPalette(W_CacheLumpName(usegamma <= 8 ? "PALFIX" : "PLAYPAL", PU_CACHE));

    // [JN] Some complex mess to avoid using numerical identification of screens.
    // Note: older Shareware version using paletted screens instead of RAWs.
    //
    // Check if have a modified graphics:
    // - If we don't, we can draw a GFX wide version.
    // - If we do, draw it as a RAW screen instead.
    
    if (english_language)
    {
        switch (InfoType)
        {
            case 0:
            if (page0_gfx->width == 560)
            V_DrawPatchFullScreen(W_CacheLumpName("TITLE", PU_CACHE), false);
            else
            V_DrawRawScreen(W_CacheLumpName("TITLE", PU_CACHE));
            break;

            case 1:
            if (page1_gfx->width == 560)
            V_DrawPatchFullScreen(W_CacheLumpName("HELP1", PU_CACHE), false);
            else
            V_DrawRawScreen(W_CacheLumpName("HELP1", PU_CACHE));
            break;

            case 2:
            if (page2_gfx->width == 560)
            V_DrawPatchFullScreen(W_CacheLumpName("HELP2", PU_CACHE), false);
            else
            V_DrawRawScreen(W_CacheLumpName("HELP2", PU_CACHE));
            break;

            case 3:
            if (page3_gfx->width == 560)
            V_DrawPatchFullScreen(W_CacheLumpName("CREDIT", PU_CACHE), false);
            else
            V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
            break;

            case 4:
            // [JN] Available only in Shareware, can't be replaced.
            V_DrawPatchFullScreen(W_CacheLumpName("ORDER", PU_CACHE), false);
            break;
        }
    }
    else
    {
        V_DrawPatchFullScreen(W_CacheLumpNum
                       (W_GetNumForName
                       (InfoType == 0 ? 
                       (gamemode == retail ? "TITLE_RT" : "TITLE") :
                        InfoType == 1 ? "HELP1_R" : 
                        InfoType == 2 ? "HELP2_R" :
                        InfoType == 3 ?
                       (gamemode == retail ? "CRED_RT" : "CRED_RG") :
                                             "ORDER_R"), PU_CACHE), false);
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

    // [JN] Force to use vanilla options menu in -vanilla game mode.
    if (vanillaparm)
    {
        if (CurrentMenu == &OptionsMenu || CurrentMenu == &OptionsMenu_Rus)
        {
            CurrentMenu = english_language ? &OptionsMenu_Vanilla :
                                             &OptionsMenu_Rus_Vanilla;
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//---------------------------------------------------------------------------
static void DrawSlider(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;

    x = menu->x + 24;
    y = menu->y + 2 + (item * ITEM_HEIGHT);
    V_DrawShadowedPatchRaven(x - 32 + wide_delta, y,
                             W_CacheLumpName(DEH_String("M_SLDLT"), PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2 + wide_delta, y,
                                 W_CacheLumpName(DEH_String(count & 1 ? 
                                                           "M_SLDMD1" : "M_SLDMD2"), PU_CACHE));
    }
    V_DrawShadowedPatchRaven(x2 + wide_delta, y,
                             W_CacheLumpName(DEH_String("M_SLDRT"), PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        dp_translation = cr[CR_GREEN2GRAY_HERETIC];
        V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only mouse sensivity in vanilla options menu requires this trick.
    else if ((CurrentMenu == &OptionsMenu_Vanilla || CurrentMenu == &OptionsMenu_Rus_Vanilla) && slot > 9)
    {
        slot = 9;
        dp_translation = cr[CR_GREEN2RED_HERETIC];
        V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7,
                W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
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

    V_DrawShadowedPatchRaven(x - 32 + wide_delta, y,
                             W_CacheLumpName(DEH_String("M_RDSLDL"), PU_CACHE));

    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2 - 16 + wide_delta, y,
                                 W_CacheLumpName(DEH_String("M_RDSLD1"), PU_CACHE));
    }

    V_DrawShadowedPatchRaven(x2 - 25 + wide_delta, y,
                             W_CacheLumpName(DEH_String("M_RDSLDR"), PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        dp_translation = cr[CR_GREEN2GRAY_HERETIC];
        V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        dp_translation = cr[CR_GREEN2RED_HERETIC];
        V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                W_CacheLumpName(DEH_String("M_RDSLG"), PU_CACHE));
}

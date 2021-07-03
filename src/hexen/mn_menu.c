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


// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include "h2def.h"
#include "doomkeys.h"
#include "i_input.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_trans.h"
#include "v_video.h"
#include "rd_rushexen.h"
#include "crispy.h"
#include "jn.h"

// MACROS ------------------------------------------------------------------

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

// [JN] Sizes of small font and small arrow for RD menu
#define ITEM_HEIGHT_SMALL 10
#define SELECTOR_XOFFSET_SMALL (-14)

// TYPES -------------------------------------------------------------------

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
    MENU_CLASS,
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
    MENU_GAMEPLAY,
    MENU_NONE
} MenuType_t;

typedef struct
{
    ItemType_t type;
    char *text;
    void (*func) (int option);
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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void InitMapInfo(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void InitFonts(void);
static void SetMenu(MenuType_t menu);
static void SCQuitGame(int option);
static void SCClass(int option);
static void SCSkill(int option);
static boolean SCNetCheck(int option);
static void SCNetCheck2(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCMessages(int option);
static void SCInfo(int option);
static void DrawMainMenu(void);
static void DrawClassMenu(void);
static void DrawSkillMenu(void);
static void DrawFileSlots(Menu_t * menu);
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawLoadMenu(void);
static void DrawSaveMenu(void);
//static void DrawSlider(Menu_t * menu, int item, int width, int slot);
static void DrawSliderSmall(Menu_t * menu, int item, int width, int slot);
void MN_LoadSlotText(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static void M_RD_Change_Widescreen(int option);
static void M_RD_Change_VSync(int option);
static void M_RD_Uncapped(int option);
static void M_RD_FPScounter(int option);
static void M_RD_Smoothing(int option);
static void M_RD_PorchFlashing(int option);
static void M_RD_Renderer(int option);
static void M_RD_Screenshots(int option);

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(int option);
static void M_RD_Gamma(int option);
static void M_RD_LevelBrightness(int option);
static void M_RD_LocalTime(int option);
static void M_RD_Messages(int option);
static void M_RD_ShadowedText(int option);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapOverlay(int option);
static void M_RD_AutoMapRotate(int option);
static void M_RD_AutoMapFollow(int option);
static void M_RD_AutoMapGrid(int option);

// Sound
static void DrawSoundMenu(void);
static void M_RD_SfxVolume(int option);
static void M_RD_MusVolume(int option);
static void M_RD_SfxChannels(int option);

// Sound system
static void DrawSoundSystemMenu(void);
static void M_RD_SoundDevice(int option);
static void M_RD_MusicDevice(int option);
static void M_RD_Sampling(int option);
static void M_RD_SndMode(int option);
static void M_RD_PitchShifting(int option);
static void M_RD_MuteInactive(int option);

// Controls
static void DrawControlsMenu(void);
static void M_RD_AlwaysRun(int option);
static void M_RD_Sensitivity(int option);
static void M_RD_MouseLook(int option);
static void M_RD_InvertY(int option);
static void M_RD_Novert(int option);

// Gameplay
static void DrawGameplayMenu(void);
static void M_RD_Brightmaps(int option);
static void M_RD_FakeContrast(int option);
static void M_RD_CrossHairDraw(int option);
static void M_RD_CrossHairType(int option);
static void M_RD_CrossHairScale(int option);
static void M_RD_FlipLevels(int option);
static void M_RD_NoDemos(int option);

// End game
static void SCEndGame(int option);

// Reset settings
static void M_RD_ResetSettings(int option);

// Change language
static void M_RD_ChangeLanguage(int option);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean gamekeydown[256];        // The NUMKEYS macro is local to g_game

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean menuactive;
int InfoType;
int messageson = true;
boolean mn_SuicideConsole;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// [JN] Original English fonts
static int FontABaseLump;       // small
static int FontAYellowBaseLump; // small yellow
static int FontBBaseLump;       // big
// [JN] Small special font used for time/fps widget
static int FontCBaseLump;
// [JN] Unchangable Russian fonts
static int FontDBaseLump;       // small
static int FontDYellowBaseLump; // small yellow
static int FontEBaseLump;       // big

static int MauloBaseLump;
static Menu_t *CurrentMenu;
static int CurrentItPos;
static int MenuPClass;
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

static MenuItem_t MainItems[] = {
    {ITT_SETMENU, "NEW GAME", SCNetCheck2, 1, MENU_CLASS},
    {ITT_SETMENU, "OPTIONS", NULL, 0, MENU_OPTIONS},
    {ITT_SETMENU, "GAME FILES", NULL, 0, MENU_FILES},
    {ITT_EFUNC,   "INFO", SCInfo, 0, MENU_NONE},
    {ITT_EFUNC,   "QUIT GAME", SCQuitGame, 0, MENU_NONE}
};

static MenuItem_t MainItems_Rus[] = {
    {ITT_SETMENU, "YJDFZ BUHF", SCNetCheck2, 1, MENU_CLASS},	// НОВАЯ ИГРА
    {ITT_SETMENU, "YFCNHJQRB", NULL, 0, MENU_OPTIONS},			// НАСТРОЙКИ
    {ITT_SETMENU, "AFQKS BUHS", NULL, 0, MENU_FILES},			// ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,   "BYAJHVFWBZ", SCInfo, 0, MENU_NONE},			// ИНФОРМАЦИЯ
    {ITT_EFUNC,   "DS[JL", SCQuitGame, 0, MENU_NONE}			// ВЫХОД
};

static Menu_t MainMenu = {
    110, 56,
    DrawMainMenu,
    5, MainItems,
    0,
    MENU_NONE
};

static Menu_t MainMenu_Rus = {
    104, 56,
    DrawMainMenu,
    5, MainItems_Rus,
    0,
    MENU_NONE
};

static MenuItem_t ClassItems[] = {
    {ITT_EFUNC, "FIGHTER", SCClass, 0, MENU_NONE},
    {ITT_EFUNC, "CLERIC", SCClass, 1, MENU_NONE},
    {ITT_EFUNC, "MAGE", SCClass, 2, MENU_NONE}
};

static MenuItem_t ClassItems_Rus[] = {
    {ITT_EFUNC, "DJBY", SCClass, 0, MENU_NONE},		// ВОИН
    {ITT_EFUNC, "RKTHBR", SCClass, 1, MENU_NONE},	// КЛЕРИК
    {ITT_EFUNC, "VFU", SCClass, 2, MENU_NONE}		// МАГ
};

static Menu_t ClassMenu = {
    66, 66,
    DrawClassMenu,
    3, ClassItems,
    0,
    MENU_MAIN
};

static Menu_t ClassMenu_Rus = {
    66, 66,
    DrawClassMenu,
    3, ClassItems_Rus,
    0,
    MENU_MAIN
};

static MenuItem_t FilesItems[] = {
    {ITT_SETMENU, "LOAD GAME", SCNetCheck2, 2, MENU_LOAD},
    {ITT_SETMENU, "SAVE GAME", NULL, 0, MENU_SAVE}
};

static MenuItem_t FilesItems_Rus[] = {
    {ITT_SETMENU, "PFUHEPBNM BUHE", SCNetCheck2, 2, MENU_LOAD},		// ЗАГРУЗИТЬ ИГРУ
    {ITT_SETMENU, "CJ[HFYBNM BUHE", NULL, 0, MENU_SAVE}				// СОХРАНИТЬ ИГРУ
};

static Menu_t FilesMenu = {
    110, 60,
    DrawFilesMenu,
    2, FilesItems,
    0,
    MENU_MAIN
};

static Menu_t FilesMenu_Rus = {
    110, 60,
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

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, NULL, SCSkill, sk_baby, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_easy, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_medium, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_hard, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_nightmare, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_ultranm, MENU_NONE}
};

static Menu_t SkillMenu = {
    120, 44,
    DrawSkillMenu,
    6, SkillItems,
    2,
    MENU_CLASS
};

static Menu_t SkillMenu_Rus = {
    120, 44,
    DrawSkillMenu,
    6, SkillItems,
    2,
    MENU_CLASS
};

// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "RENDERING",      NULL,               0, MENU_RENDERING},
    {ITT_SETMENU, "DISPLAY",        NULL,               0, MENU_DISPLAY  },
    {ITT_SETMENU, "SOUND",          NULL,               0, MENU_SOUND    },
    {ITT_SETMENU, "CONTROLS",       NULL,               0, MENU_CONTROLS },
    {ITT_SETMENU, "GAMEPLAY",       NULL,               0, MENU_GAMEPLAY },
    {ITT_EFUNC,   "RESET SETTINGS", M_RD_ResetSettings, 0, MENU_NONE     },
    {ITT_EFUNC,   "LANGUAGE: ENGLISH", M_RD_ChangeLanguage, 0, MENU_NONE     }
};

static MenuItem_t OptionsItems_Rus[] = {
    {ITT_SETMENU, "DBLTJ",          NULL,               0, MENU_RENDERING}, // ВИДЕО
    {ITT_SETMENU, "\'RHFY",         NULL,               0, MENU_DISPLAY  }, // ЭКРАН
    {ITT_SETMENU, "FELBJ",          NULL,               0, MENU_SOUND    }, // АУДИО
    {ITT_SETMENU, "EGHFDKTYBT",     NULL,               0, MENU_CONTROLS }, // УПРАВЛЕНИЕ
    {ITT_SETMENU, "UTQVGKTQ",       NULL,               0, MENU_GAMEPLAY }, // ГЕЙМПЛЕЙ
    {ITT_EFUNC,   "C,HJC YFCNHJTR", M_RD_ResetSettings, 0, MENU_NONE     }, // СБРОС НАСТРОЕК
    {ITT_EFUNC,   "ZPSR: HECCRBQ", M_RD_ChangeLanguage, 0, MENU_NONE     }
    
};

static Menu_t OptionsMenu = {
    77, 16,
    NULL,
    7, OptionsItems,
    0,
    MENU_MAIN
};

static Menu_t OptionsMenu_Rus = {
    77, 16,
    NULL,
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
    {ITT_LRFUNC, "OVERLAY MODE:", M_RD_AutoMapOverlay, 0, MENU_NONE},
    {ITT_LRFUNC, "ROTATE MODE:",  M_RD_AutoMapRotate,  0, MENU_NONE},
    {ITT_LRFUNC, "FOLLOW MODE:",  M_RD_AutoMapFollow,  0, MENU_NONE},
    {ITT_LRFUNC, "GRID:",         M_RD_AutoMapGrid,    0, MENU_NONE}
};

static MenuItem_t AutomapItems_Rus[] = {
    {ITT_LRFUNC, "HT;BV YFKJ;TYBZ:",   M_RD_AutoMapOverlay, 0, MENU_NONE}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_LRFUNC, "HT;BV DHFOTYBZ:",    M_RD_AutoMapRotate,  0, MENU_NONE}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_LRFUNC, "HT;BV CKTLJDFYBZ:",  M_RD_AutoMapFollow,  0, MENU_NONE}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_LRFUNC, "CTNRF:",             M_RD_AutoMapGrid,    0, MENU_NONE}  // СЕТКА
};

static Menu_t AutomapMenu = {
    102, 32,
    DrawAutomapMenu,
    4, AutomapItems,
    0,
    MENU_DISPLAY
};

static Menu_t AutomapMenu_Rus = {
    82, 32,
    DrawAutomapMenu,
    4, AutomapItems_Rus,
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
    {ITT_LRFUNC, "ALWAYS RUN:",        M_RD_AlwaysRun,   0, MENU_NONE},
    {ITT_EMPTY,  NULL,                 NULL,             0, MENU_NONE},
    {ITT_LRFUNC, "MOUSE SENSIVITY",    M_RD_Sensitivity, 0, MENU_NONE},
    {ITT_EMPTY,  NULL,                 NULL,             0, MENU_NONE},
    {ITT_LRFUNC, "MOUSE LOOK:",        M_RD_MouseLook,   0, MENU_NONE},
    {ITT_LRFUNC, "INVERT Y AXIS:",     M_RD_InvertY,     0, MENU_NONE},
    {ITT_LRFUNC, "VERTICAL MOVEMENT:", M_RD_Novert,      0, MENU_NONE}
};

static MenuItem_t ControlsItems_Rus[] = {
    {ITT_LRFUNC, "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,   0, MENU_NONE}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_EMPTY,  NULL,                        NULL,             0, MENU_NONE}, //
    {ITT_LRFUNC, "CRJHJCNM VSIB",             M_RD_Sensitivity, 0, MENU_NONE}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,  NULL,                        NULL,             0, MENU_NONE}, //
    {ITT_LRFUNC, "J,PJH VSIM.:",              M_RD_MouseLook,   0, MENU_NONE}, // ОБЗОР МЫШЬЮ
    {ITT_LRFUNC, "DTHNBRFKMYFZ BYDTHCBZ:",    M_RD_InvertY,     0, MENU_NONE}, // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    {ITT_LRFUNC, "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,      0, MENU_NONE}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36, 42,
    DrawControlsMenu,
    7, ControlsItems,
    0,
    MENU_OPTIONS
};

static Menu_t ControlsMenu_Rus = {
    36, 42,
    DrawControlsMenu,
    7, ControlsItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features
// -----------------------------------------------------------------------------

static MenuItem_t GameplayItems[] = {
    {ITT_LRFUNC, "BRIGHTMAPS:",          M_RD_Brightmaps,      0, MENU_NONE   },
    {ITT_LRFUNC, "FAKE CONTRAST:",       M_RD_FakeContrast,    0, MENU_NONE   },
    {ITT_EMPTY,  NULL,                   NULL,                 0, MENU_NONE   },
    {ITT_LRFUNC, "DRAW CROSSHAIR:",      M_RD_CrossHairDraw,   0, MENU_NONE   },
    {ITT_LRFUNC, "INDICATION:",          M_RD_CrossHairType,   0, MENU_NONE   },
    {ITT_LRFUNC, "INCREASED SIZE:",      M_RD_CrossHairScale,  0, MENU_NONE   },
    {ITT_EMPTY,  NULL,                   NULL,                 0, MENU_NONE   },
    {ITT_LRFUNC, "FLIP GAME LEVELS:",    M_RD_FlipLevels,      0, MENU_NONE   },
    {ITT_LRFUNC, "PLAY INTERNAL DEMOS:", M_RD_NoDemos,         0, MENU_NONE   }
};

static MenuItem_t GameplayItems_Rus[] = {
    {ITT_LRFUNC, ",HFQNVFGGBYU:",            M_RD_Brightmaps,      0, MENU_NONE   }, // БРАЙТМАППИНГ
    {ITT_LRFUNC, "BVBNFWBZ RJYNHFCNYJCNB:",  M_RD_FakeContrast,    0, MENU_NONE   }, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE   }, //
    {ITT_LRFUNC, "JNJ,HF;FNM GHBWTK:",       M_RD_CrossHairDraw,   0, MENU_NONE   }, // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_LRFUNC, "BYLBRFWBZ:",               M_RD_CrossHairType,   0, MENU_NONE   }, // ИНДИКАЦИЯ
    {ITT_LRFUNC, "EDTKBXTYYSQ HFPVTH:",      M_RD_CrossHairScale,  0, MENU_NONE   }, // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,  NULL,                       NULL,                 0, MENU_NONE   }, //
    {ITT_LRFUNC, "PTHRFKMYJT JNHF;TYBT EHJDYTQ:",  M_RD_FlipLevels,0, MENU_NONE   }, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_LRFUNC, "GHJBUHSDFNM LTVJPFGBCB:",  M_RD_NoDemos,         0, MENU_NONE   }  // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
};

static Menu_t GameplayMenu = {
    36, 42,
    DrawGameplayMenu,
    9, GameplayItems,
    0,
    MENU_OPTIONS
};

static Menu_t GameplayMenu_Rus = {
    36, 42,
    DrawGameplayMenu,
    9, GameplayItems_Rus,
    0,
    MENU_OPTIONS
};

static Menu_t *Menus[] = {
    &MainMenu,
    &ClassMenu,
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
    &GameplayMenu
};

static Menu_t *Menus_Rus[] = {
    &MainMenu_Rus,
    &ClassMenu_Rus,
    &SkillMenu_Rus,
    &OptionsMenu_Rus,
    &FilesMenu_Rus,
    &LoadMenu,
    &SaveMenu,
    &RenderingMenu_Rus,
    &DisplayMenu_Rus,
    &AutomapMenu_Rus,
    &SoundMenu_Rus,
    &SoundSysMenu_Rus,
    &ControlsMenu_Rus,
    &GameplayMenu_Rus,
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

// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    InitFonts();
    menuactive = false;
//      messageson = true;              // Set by defaults in .CFG
    MauloBaseLump = W_GetNumForName("FBULA0");  // ("M_SKL00");

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;
}

//---------------------------------------------------------------------------
//
// PROC InitFonts
//
//---------------------------------------------------------------------------

static void InitFonts(void)
{
    // [JN] Original English fonts
    FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    FontAYellowBaseLump = W_GetNumForName("FONTAY_S") + 1;
    FontBBaseLump = W_GetNumForName("FONTB_S") + 1;

    // [JN] Small special font used for time/fps widget
    FontCBaseLump = W_GetNumForName("FONTC_S") + 1;

    // [JN] Unchangable Russian fonts
    FontDBaseLump = W_GetNumForName("FONTD_S") + 1;        // small
    FontDYellowBaseLump = W_GetNumForName("FONTDY_S") + 1; // small yellow
    FontEBaseLump = W_GetNumForName("FONTE_S") + 1;        // big
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

//==========================================================================
//
// MN_DrTextAYellow
//
//==========================================================================

void MN_DrTextAYellow(char *text, int x, int y)
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
            p = W_CacheLumpNum(FontAYellowBaseLump + c - 33, PU_CACHE);
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
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
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
// MN_DrTextSmallRUS
//
// [JN] Draw text string with unreplacable Russian font.
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
            p = W_CacheLumpNum(FontDBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// MN_DrTextSmallYellowRUS
//
// [JN] Draw text string with unreplacable Russian yellow font.
//
//---------------------------------------------------------------------------

void MN_DrTextSmallYellowRUS(char *text, int x, int y)
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
            p = W_CacheLumpNum(FontDYellowBaseLump + c - 33, PU_CACHE);
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
            p = W_CacheLumpNum(FontDBaseLump + c - 33, PU_CACHE);
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
            p = W_CacheLumpNum(FontEBaseLump + c - 33, PU_CACHE);
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
            p = W_CacheLumpNum(FontEBaseLump + c - 33, PU_CACHE);
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
    "ARE YOU SURE YOU WANT TO SUICIDE?",
    "RESET SETTINGS TO THEIR DEFAULTS?",
    "" // [JN] Placeholder for language changing
};

char *QuitEndMsg_Rus[] = {
    "DS LTQCNDBNTKMYJ ;TKFTNT DSQNB?",			// ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ВЫЙТИ?
    "DS LTQCNDBNTKMYJ ;TKFTNT PFRJYXBNM BUHE?",	// ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ЗАКОНЧИТЬ ИГРУ?
    "DSGJKYBNM ,SCNHJT CJ[HFYTYBT BUHS:",		// ВЫПОЛНИТЬ БЫСТРОЕ СОХРАНЕНИЕ ИГРЫ:
    "DSGJKYBNM ,SCNHE. PFUHEPRE BUHS:",			// ВЫПОЛНИТЬ БЫСТРУЮ ЗАГРУЗКУ ИГРЫ:
    "DS LTQCNDBNTKMYJ [JNBNT CJDTHIBNM CEBWBL?",  // ВЫ ДЕЙСТВИТЕЛЬНО ХОТИТЕ СОВЕРШИТЬ СУИЦИД?
    "C,HJCBNM YFCNHJQRB YF CNFYLFHNYST PYFXTYBZ?", // СБРОСИТЬ НАСТРОЙКИ НА СТАНДАРТНЫЕ ЗНАЧЕНИЯ?
    "" // [JN] Placeholder for language changing
};

void MN_Drawer(void)
{
    int i;
    int x;
    int y;
    MenuItem_t *item;
    char *selName;

    if (menuactive == false)
    {
        if (askforquit)
        {
            if (english_language)
            {
                MN_DrTextA(QuitEndMsg[typeofask - 1], 160 -
                           MN_TextAWidth(QuitEndMsg[typeofask - 1]) / 2
                           + wide_delta, 80);
            }
            else
            {
                MN_DrTextSmallRUS(QuitEndMsg_Rus[typeofask - 1], 160 -
                           MN_DrTextSmallRUSWidth(QuitEndMsg_Rus[typeofask - 1]) / 2
                           + wide_delta, 80);
            }

            if (typeofask == 3)
            {
                MN_DrTextA(SlotText[quicksave - 1], 160 -
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2
                           + wide_delta, 90);
                MN_DrTextA("?", 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2
                           + wide_delta, 90);
            }
            if (typeofask == 4)
            {
                MN_DrTextA(SlotText[quickload - 1], 160 -
                           MN_TextAWidth(SlotText[quickload - 1]) / 2
                           + wide_delta, 90);
                MN_DrTextA("?", 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2
                           + wide_delta, 90);
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
                    ||  CurrentMenu == &ClassMenu
                    ||  CurrentMenu == &SkillMenu
                    ||  CurrentMenu == &OptionsMenu
                    ||  CurrentMenu == &FilesMenu)
                    {
                        MN_DrTextB(item->text, x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextA(item->text, x + wide_delta, y);
                    }
                }
                else
                {
                    if (CurrentMenu == &MainMenu_Rus
                    ||  CurrentMenu == &ClassMenu_Rus
                    ||  CurrentMenu == &SkillMenu_Rus
                    ||  CurrentMenu == &OptionsMenu_Rus
                    ||  CurrentMenu == &FilesMenu_Rus)
                    {
                        MN_DrTextBigRUS(item->text, x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextSmallRUS(item->text, x + wide_delta, y);
                    }
                }
            }

            // [JN] Use a different font's vertical spacing in following menus:
            if (CurrentMenu == & RenderingMenu
            ||  CurrentMenu == & DisplayMenu
            ||  CurrentMenu == & AutomapMenu
            ||  CurrentMenu == & SoundMenu
            ||  CurrentMenu == & SoundSysMenu
            ||  CurrentMenu == & ControlsMenu
            ||  CurrentMenu == & GameplayMenu
            ||  CurrentMenu == & RenderingMenu_Rus
            ||  CurrentMenu == & DisplayMenu_Rus
            ||  CurrentMenu == & AutomapMenu_Rus
            ||  CurrentMenu == & SoundMenu_Rus
            ||  CurrentMenu == & SoundSysMenu_Rus
            ||  CurrentMenu == & ControlsMenu_Rus
            ||  CurrentMenu == & GameplayMenu_Rus)
            {
                y += ITEM_HEIGHT_SMALL;
            }
            else
            {
                y += ITEM_HEIGHT;
            }

            item++;
        }

        // [JN] Draw small arrow instead of big in following menus:
        if (CurrentMenu == & RenderingMenu
        ||  CurrentMenu == & DisplayMenu
        ||  CurrentMenu == & AutomapMenu
        ||  CurrentMenu == & SoundMenu
        ||  CurrentMenu == & SoundSysMenu
        ||  CurrentMenu == & ControlsMenu
        ||  CurrentMenu == & GameplayMenu
        ||  CurrentMenu == & RenderingMenu_Rus
        ||  CurrentMenu == & DisplayMenu_Rus
        ||  CurrentMenu == & AutomapMenu_Rus
        ||  CurrentMenu == & SoundMenu_Rus
        ||  CurrentMenu == & SoundSysMenu_Rus
        ||  CurrentMenu == & ControlsMenu_Rus
        ||  CurrentMenu == & GameplayMenu_Rus)
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT_SMALL) + SELECTOR_YOFFSET;
            selName = MenuTime & 8 ? "INVGEMR1" : "INVGEMR2";
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET_SMALL + wide_delta, y,
                                     W_CacheLumpName(selName, PU_CACHE));
        }
        else
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
            selName = MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2";
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET + wide_delta, y,
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

    frame = (MenuTime / 5) % 7;
    
    V_DrawShadowedPatchRaven(88 + wide_delta, 0, W_CacheLumpName("M_HTIC", PU_CACHE));
// Old Gold skull positions: (40, 10) and (232, 10)
    V_DrawShadowedPatchRaven(42 + wide_delta, 83, W_CacheLumpNum(MauloBaseLump + (frame + 2) % 7,
                                       PU_CACHE));
    V_DrawShadowedPatchRaven(273 + wide_delta, 83, W_CacheLumpNum(MauloBaseLump + frame, PU_CACHE));

    // [JN] Update Status bar.
    SB_state = -1;
}

//==========================================================================
//
// DrawClassMenu
//
//==========================================================================

static void DrawClassMenu(void)
{
    pclass_t class;
    static char *boxLumpName[3] = {
        "m_fbox",
        "m_cbox",
        "m_mbox"
    };
    static char *boxLumpName_Rus[3] = {
        "rd_fbox",
        "rd_cbox",
        "rd_mbox"
    };
    static char *walkLumpName[3] = {
        "m_fwalk1",
        "m_cwalk1",
        "m_mwalk1"
    };

    if (english_language)
    {
        MN_DrTextB("CHOOSE CLASS:", 34 + wide_delta, 24);
    }
    else
    {
        MN_DrTextBigRUS("DS,THBNT RKFCC:", 5 + wide_delta, 24);   // ВЫБЕРИТЕ КЛАСС:
    }
    class = (pclass_t) CurrentMenu->items[CurrentItPos].option;

    V_DrawShadowedPatchRaven(174 + wide_delta, 8, 
                             W_CacheLumpName(english_language ? 
                                             boxLumpName[class] :
                                             boxLumpName_Rus[class], PU_CACHE));

    V_DrawPatch(174 + 24 + wide_delta, 8 + 12,
                W_CacheLumpNum(W_GetNumForName(walkLumpName[class])
                               + ((MenuTime >> 3) & 3), PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawSkillMenu
//
//---------------------------------------------------------------------------

static void DrawSkillMenu(void)
{
    if (english_language)
    {
        MN_DrTextB("CHOOSE SKILL LEVEL:", 74 + wide_delta, 16);
    }
    else
    {
        MN_DrTextBigRUS("EHJDTYM CKJ;YJCNB:", 57 + wide_delta, 16);   // УРОВЕНЬ СЛОЖНОСТИ:
    }

    // [JN] Update Status bar.
    SB_state = -1;
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
    P_ClearMessage(&players[consoleplayer]);
}

//---------------------------------------------------------------------------
//
// PROC DrawLoadMenu
//
//---------------------------------------------------------------------------

static void DrawLoadMenu(void)
{
    if (english_language)
    {
        MN_DrTextB("LOAD GAME", 160 - MN_TextBWidth("LOAD GAME") / 2 + wide_delta, 10);
    }
    else
    {
        // ЗАГРУЗИТЬ ИГРУ
        MN_DrTextBigRUS("PFUHEPBNM BUHE", 160 - MN_DrTextBigRUSWidth("PFUHEPBNM BUHE")
                                              / 2 + wide_delta, 10);
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
    if (english_language)
    {
        MN_DrTextB("SAVE GAME", 160 - MN_TextBWidth("SAVE GAME") / 2 + wide_delta, 10);
    }
    else
    {
        // СОХРАНИТЬ ИГРУ
        MN_DrTextBigRUS("CJ[HFYBNM BUHE", 160 - MN_DrTextBigRUSWidth("CJ[HFYBNM BUHE")
                                              / 2 + wide_delta, 10);
    }
    
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&SaveMenu);
}

static boolean ReadDescriptionForSlot(int slot, char *description)
{
    FILE *fp;
    boolean found;
    char name[100];
    char versionText[HXS_VERSION_TEXT_LENGTH];

    M_snprintf(name, sizeof(name), "%shexen-save-%d.sav", SavePath, slot);

    fp = fopen(name, "rb");

    if (fp == NULL)
    {
        return false;
    }

    found = fread(description, HXS_DESCRIPTION_LENGTH, 1, fp) == 1
         && fread(versionText, HXS_VERSION_TEXT_LENGTH, 1, fp) == 1;

    found = found && strcmp(versionText, HXS_VERSION_TEXT) == 0;

    fclose(fp);

    return found;
}

//===========================================================================
//
// MN_LoadSlotText
//
// For each slot, looks for save games and reads the description field.
//
//===========================================================================

void MN_LoadSlotText(void)
{
    char description[HXS_DESCRIPTION_LENGTH];
    int slot;

    for (slot = 0; slot < 6; slot++)
    {
        if (ReadDescriptionForSlot(slot, description))
        {
            memcpy(SlotText[slot], description, SLOTTEXTLEN);
            SlotStatus[slot] = 1;
        }
        else
        {
            memset(SlotText[slot], 0, SLOTTEXTLEN);
            SlotStatus[slot] = 0;
        }
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
        V_DrawShadowedPatchRaven(x + wide_delta, y, W_CacheLumpName("M_FSLOT", PU_CACHE));
        if (SlotStatus[i])
        {
            MN_DrTextA(SlotText[i], x + 5 + wide_delta, y + 5);
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

    title_eng = "RENDERING OPTIONS";
    title_rus = "YFCNHJQRB DBLTJ";  // НАСТРОЙКИ ВИДЕО

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // RENDERING
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("RENDERING", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Display aspect ratio
        MN_DrTextA(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 185 + wide_delta, 42);

        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY2RED_HEXEN];
            MN_DrTextA("THE PROGRAM MUST BE RESTARTED", 51 + wide_delta, 135);
            dp_translation = NULL;
        }

        // Vertical sync
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextA("N/A", 216 + wide_delta, 52);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextA(vsync ? "ON" : "OFF", 216 + wide_delta, 52);
        }

        // Uncapped FPS
        MN_DrTextA(uncapped_fps ? "UNCAPPED" : "35 FPS", 120 + wide_delta, 62);

        // FPS counter
        MN_DrTextA(show_fps ? "ON" : "OFF", 129 + wide_delta, 72);

        // Pixel scaling
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextA("N/A", 131 + wide_delta, 82);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextA(smoothing ? "SMOOTH" : "SHARP", 131 + wide_delta, 82);
        }

        // Porch palette changing
        MN_DrTextA(vga_porch_flash ? "ON" : "OFF", 205 + wide_delta, 92);

        // Video renderer
        MN_DrTextA(force_software_renderer ? "SOFTWARE (CPU)" : "HARDWARE (GPU)",
                   149 + wide_delta, 102);

        //
        // EXTRA
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("EXTRA", 36 + wide_delta, 112);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 7);

        //
        // РЕНДЕРИНГ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("HTYLTHBYU", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Соотношение сторон экрана
        MN_DrTextA(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 230 + wide_delta, 42);

        // Informative message: НЕОБХОДИМ ПЕРЕЗАПУСК ИГРЫ
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY2RED_HEXEN];
            MN_DrTextSmallRUS("YTJ,[JLBV GTHTPFGECR GHJUHFVVS", 46 + wide_delta, 135);
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextSmallRUS("Y/L", 236 + wide_delta, 52);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallRUS(vsync ? "DRK" : "DSRK", 236 + wide_delta, 52);
        }

        // Кадровая частота
        if (uncapped_fps)
        MN_DrTextSmallRUS(",TP JUHFYBXTYBZ", 165 + wide_delta, 62);
        else
        MN_DrTextA("35 FPS", 165 + wide_delta, 62);

        // Счетчик кадровой частоты
        MN_DrTextSmallRUS(show_fps ? "DRK" : "DSRK", 223 + wide_delta, 72);

        // Пиксельное сглаживание
        if (force_software_renderer)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextSmallRUS("Y/L", 211 + wide_delta, 82);
            dp_translation = NULL;
        }
        else
        {
            MN_DrTextSmallRUS(smoothing ? "DRK" : "DSRK", 211 + wide_delta, 82);
        }

        // Изменение палитры краев экрана
        MN_DrTextSmallRUS(vga_porch_flash ? "DRK" : "DSRK", 265 + wide_delta, 92);

        // Обработка видео
        MN_DrTextSmallRUS(force_software_renderer ? "GHJUHFVVYFZ" : "FGGFHFNYFZ",
                          159 + wide_delta, 102);

        //
        // ДОПОЛНИТЕЛЬНО
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("LJGJKYBNTKMYJ", 36 + wide_delta, 112);
        dp_translation = NULL;
    }

    // Screenshot format / Формат скриншотов (same english values)
    MN_DrTextA(png_screenshots ? "PNG" : "PCX", 175 + wide_delta, 122);
}

static void M_RD_Change_Widescreen(int option)
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
}

static void M_RD_Change_VSync(int option)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
    return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

static void M_RD_Uncapped(int option)
{
    uncapped_fps ^= 1;
}

static void M_RD_FPScounter(int option)
{
    show_fps ^= 1;
}

static void M_RD_Smoothing(int option)
{
    // [JN] Disable smoothing toggling in software renderer
    if (force_software_renderer == 1)
    return;

    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Update status bar
    SB_state = -1;
}

static void M_RD_PorchFlashing(int option)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
}

static void M_RD_Renderer(int option)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update status bar
    SB_state = -1;
}

static void M_RD_Screenshots(int option)
{
    png_screenshots ^= 1;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = "DISPLAY OPTIONS";
    title_rus = "YFCNHJQRB \'RHFYF";  // НАСТРОЙКИ ЭКРАНА

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // SCREEN, INTERFACE
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("SCREEN", 36 + wide_delta, 32);
        MN_DrTextA("INTERFACE", 36 + wide_delta, 102);
        dp_translation = NULL;

        // Local time
        MN_DrTextA(local_time == 1 ? "12-HOUR (HH:MM)" :
                   local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                   local_time == 3 ? "24-HOUR (HH:MM)" :
                   local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF",
                   110 + wide_delta, 112);

        // Messages
        MN_DrTextA((messageson ? "ON" : "OFF"), 108 + wide_delta, 122);

        // Text casts shadows
        MN_DrTextA((draw_shadowed_text ? "ON" : "OFF"), 179 + wide_delta, 132);
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
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("\'RHFY", 36 + wide_delta, 32);
        MN_DrTextSmallRUS("BYNTHATQC", 36 + wide_delta, 102);
        dp_translation = NULL;

        // Системное время
        MN_DrTextSmallRUS(local_time == 1 ? "12-XFCJDJT (XX:VV)" :
                          local_time == 2 ? "12-XFCJDJT (XX:VV:CC)" :
                          local_time == 3 ? "24-XFCJDJT (XX:VV)" :
                          local_time == 4 ? "24-XFCJDJT (XX:VV:CC)" : "DSRK",
                          157 + wide_delta, 112);

        // Отображение сообщений
        MN_DrTextSmallRUS((messageson ? "DRK" : "DSRK"), 208 + wide_delta, 122);

        // Тексты отбрасывают тень
        MN_DrTextSmallRUS((draw_shadowed_text ? "DRK" : "DSRK"), 220 + wide_delta, 132);
    }

    // Screen size
    if (aspect_ratio >= 2)
    {
        DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 4, screenblocks - 9);
        M_snprintf(num, 4, "%3d", screenblocks);
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextA(num, 85 + wide_delta, 52);
        dp_translation = NULL;
    }
    else
    {
        DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 10, screenblocks - 3);
        M_snprintf(num, 4, "%3d", screenblocks);
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextA(num, 135 + wide_delta, 52);
        dp_translation = NULL;
    }

    // Gamma-correction
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 3, 18, usegamma);

    // Level brightness
    DrawSliderSmall((english_language ? &DisplayMenu : &DisplayMenu_Rus), 5, 5, level_brightness / 16);
}

static void M_RD_ScreenSize(int option)
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

    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: don't allow unsupported (bordered) views
        // screenblocks - config file variable
        if (screenblocks < 9)
            screenblocks = 9;
        if (screenblocks > 12)
            screenblocks = 12;
    }

    R_SetViewSize(screenblocks, detailLevel);
}

static void M_RD_Gamma(int option)
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
}

static void M_RD_Messages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], english_language ?
                      "MESSAGES ON" :
                      "CJJ,OTYBZ DRK.XTYS", // СООБЩЕНИЯ ВКЛЮЧЕНЫ
                      true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], english_language ?
                      "MESSAGES OFF" :
                      "CJJ,OTYBZ DSRK.XTYS", // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
                      true);
    }
    S_StartSound(NULL, SFX_CHAT);
}

static void M_RD_LevelBrightness(int option)
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
}

static void M_RD_LocalTime(int option)
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
}

// -----------------------------------------------------------------------------
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = "AUTOMAP SETTINGS";
    title_rus = "YFCNHJQRB RFHNS";  // НАСТРОЙКИ КАРТЫ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 
                                  + wide_delta, 7);

        // Overlay mode
        MN_DrTextA(automap_overlay ? "ON" : "OFF", 200 + wide_delta, 32);

        // Rotate mode
        MN_DrTextA(automap_rotate ? "ON" : "OFF", 193 + wide_delta, 42);

        // Follow mode
        MN_DrTextA(automap_follow ? "ON" : "OFF", 189 + wide_delta, 52);

        // Grid
        MN_DrTextA(automap_grid ? "ON" : "OFF", 138 + wide_delta, 62);
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        // Режим наложения
        MN_DrTextSmallRUS(automap_overlay ? "DRK" : "DSRK", 208 + wide_delta, 32);

        // Режим вращения
        MN_DrTextSmallRUS(automap_rotate ? "DRK" : "DSRK", 200 + wide_delta, 42);

        // Режим следования
        MN_DrTextSmallRUS(automap_follow ? "DRK" : "DSRK", 215 + wide_delta, 52);

        // Сетка
        MN_DrTextSmallRUS(automap_grid ? "DRK" : "DSRK", 128 + wide_delta, 62);
    }
}

static void M_RD_AutoMapOverlay(int option)
{
    automap_overlay ^= 1;
}

static void M_RD_AutoMapRotate(int option)
{
    automap_rotate ^= 1;
}

static void M_RD_AutoMapFollow(int option)
{
    automap_follow ^= 1;
}

static void M_RD_AutoMapGrid(int option)
{
    automap_grid ^= 1;
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = "SOUND OPTIONS";
    title_rus = "YFCNHJQRB PDERF";  // НАСТРОЙКИ ЗВУКА

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // VOLUME, CHANNELS, ADVANCED
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("VOLUME", 36 + wide_delta, 32);
        MN_DrTextA("CHANNELS", 36 + wide_delta, 82);
        MN_DrTextA("ADVANCED", 36 + wide_delta, 112);
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
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("UHJVRJCNM", 36 + wide_delta, 32);
        MN_DrTextSmallRUS("DJCGHJBPDTLTYBT", 36 + wide_delta, 82);
        MN_DrTextSmallRUS("LJGJKYBNTKMYJ", 36 + wide_delta, 112);
        dp_translation = NULL;
    }

    // SFX Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 1, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%3d", snd_MaxVolume_tmp);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
    MN_DrTextA(num, 184 + wide_delta, 53);
    dp_translation = NULL;

    // Music Volume
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 3, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
    MN_DrTextA(num, 184 + wide_delta, 73);
    dp_translation = NULL;

    // SFX Channels
    DrawSliderSmall((english_language ? &SoundMenu : &SoundMenu_Rus), 6, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%3d", snd_Channels);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
    MN_DrTextA(num, 184 + wide_delta, 103);
    dp_translation = NULL;
}

static void M_RD_SfxVolume(int option)
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
    soundchanged = true;        // we'll set it when we leave the menu
}

static void M_RD_MusVolume(int option)
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
}

static void M_RD_SfxChannels(int option)
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
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundSystemMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = "SOUND SYSTEM SETTINGS";
    title_rus = "YFCNHJQRB PDERJDJQ CBCNTVS";  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // SOUND SYSTEM
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("SOUND SYSTEM", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Sound effects
        if (snd_sfxdevice == 0)
        {
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextA("DISABLED", 144 + wide_delta, 42);
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 3)
        {
            MN_DrTextA("DIGITAL SFX", 144 + wide_delta, 42);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextA("DISABLED", 80 + wide_delta, 52);
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            MN_DrTextA("OPL2 SYNTH", 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            MN_DrTextA("OPL3 SYNTH", 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 5)
        {
            MN_DrTextA("GUS EMULATION", 80 + wide_delta, 52);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            MN_DrTextA("MIDI/MP3/OGG/FLAC/TRACKER", 80 + wide_delta, 52);
        }

        //
        // QUALITY
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("QUALITY", 36 + wide_delta, 62);
        dp_translation = NULL;

        // Sampling frequency (hz)
        if (snd_samplerate == 44100)
        {
            MN_DrTextA("44100 HZ", 178 + wide_delta, 72);
        }
        else if (snd_samplerate == 22050)
        {
            MN_DrTextA("22050 HZ", 178 + wide_delta, 72);
        }
        else if (snd_samplerate == 11025)
        {
            MN_DrTextA("11025 HZ", 178 + wide_delta, 72);
        }

        //
        // MISCELLANEOUS
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("MISCELLANEOUS", 36 + wide_delta, 82);
        dp_translation = NULL;

        // SFX Mode
        MN_DrTextA(snd_monomode ? "MONO" : "STEREO", 181 + wide_delta, 92);

        // Pitch-Shifted sounds
        MN_DrTextA(snd_pitchshift ? "ON" : "OFF", 189 + wide_delta, 102);

        // Mute inactive window
        MN_DrTextA(mute_inactive_window ? "ON" : "OFF", 184 + wide_delta, 112);

        // Informative message:
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
        {
            dp_translation = cr[CR_GRAY2RED_HEXEN];
            MN_DrTextA("CHANGING WILL REQUIRE RESTART OF THE PROGRAM", 3 + wide_delta, 132);
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
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("PDERJDFZ CBCNTVF", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextSmallRUS("JNRK.XTYS", 173 + wide_delta, 42);
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 3)
        {
            // ЦИФРОВЫЕ
            MN_DrTextSmallRUS("WBAHJDST", 173 + wide_delta, 42);
        }

        // Музыка
        if (snd_musicdevice == 0)
        {   
            // ОТКЛЮЧЕНА
            dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
            MN_DrTextSmallRUS("JNRK.XTYF", 91 + wide_delta, 52);
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // СИНТЕЗ OPL2
            MN_DrTextSmallRUS("CBYNTP", 91 + wide_delta, 52);
            MN_DrTextA("OPL2", 140 + wide_delta, 52);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // СИНТЕЗ OPL3
            MN_DrTextSmallRUS("CBYNTP", 91 + wide_delta, 52);
            MN_DrTextA("OPL3", 140 + wide_delta, 52);
        }
        else if (snd_musicdevice == 5)
        {
            // ЭМУЛЯЦИЯ GUS
            MN_DrTextSmallRUS("\'VEKZWBZ", 91 + wide_delta, 52);
            MN_DrTextA("GUS", 155 + wide_delta, 52);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            MN_DrTextA("MIDI/MP3/OGG/FLAC/TRACKER", 91 + wide_delta, 52);
        }

        //
        // КАЧЕСТВО ЗВУЧАНИЯ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("RFXTCNDJ PDEXFYBZ", 36 + wide_delta, 62);
        dp_translation = NULL;

        // Частота дискретизации (гц)
        if (snd_samplerate == 44100)
        {
            MN_DrTextSmallRUS("44100 UW", 200 + wide_delta, 72);
        }
        else if (snd_samplerate == 22050)
        {
            MN_DrTextSmallRUS("22050 UW", 200 + wide_delta, 72);
        }
        else if (snd_samplerate == 11025)
        {
            MN_DrTextSmallRUS("11025 UW", 200 + wide_delta, 72);
        }

        //
        // РАЗНОЕ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("HFPYJT", 36 + wide_delta, 82);
        dp_translation = NULL;

        // Режим звуковых эффектов
        MN_DrTextSmallRUS(snd_monomode ? "VJYJ" : "CNTHTJ", 226 + wide_delta, 92);

        // Произвольный питч-шифтинг
        MN_DrTextSmallRUS(snd_pitchshift ? "DRK" : "DSRK", 230 + wide_delta, 102);

        // Звук в неактивном окне
        MN_DrTextSmallRUS(mute_inactive_window ? "DSRK" : "DRK", 201 + wide_delta, 112);

        // Informative message: ИЗМЕНЕНИЕ ПОТРЕБУЕТ ПЕРЕЗАПУСК ПРОГРАММЫ
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
        {
            dp_translation = cr[CR_GRAY2RED_HEXEN];
            MN_DrTextSmallRUS("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS", 
                              11 + wide_delta, 132);
            dp_translation = NULL;
        }
    }
}

static void M_RD_SoundDevice(int option)
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
}

static void M_RD_MusicDevice(int option)
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
}

static void M_RD_Sampling(int option)
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
}

static void M_RD_SndMode(int option)
{
    snd_monomode ^= 1;
}

static void M_RD_PitchShifting(int option)
{
    snd_pitchshift ^= 1;
}

static void M_RD_MuteInactive(int option)
{
    mute_inactive_window ^= 1;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    static char *title_eng, *title_rus;
    static char num[4];

    title_eng = "CONTROL SETTINGS";
    title_rus = "EGHFDKTYBT";  // УПРАВЛЕНИЕ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // MOVEMENT
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("MOVEMENT", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Always run
        MN_DrTextA(joybspeed >= 20 ? "ON" : "OFF", 118 + wide_delta, 42);

        //
        // MOUSE
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("MOUSE", 36 + wide_delta, 52);
        dp_translation = NULL;

        // Mouse look
        MN_DrTextA(mlook ? "ON" : "OFF", 118 + wide_delta, 82);

        // Invert Y axis
        if (!mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextA(mouse_y_invert ? "ON" : "OFF", 133 + wide_delta, 92);
        dp_translation = NULL;

        // Novert
        if (mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextA(!novert ? "ON" : "OFF", 168 + wide_delta, 102);
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
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("GTHTLDB;TYBT", 36 + wide_delta, 32);
        dp_translation = NULL;

        // Режим постоянного бега
        MN_DrTextSmallRUS(joybspeed >= 20 ? "DRK" : "DSRK", 209 + wide_delta, 42);

        //
        // МЫШЬ
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("VSIM", 36 + wide_delta, 52);
        dp_translation = NULL;

        // Обзор мышью
        MN_DrTextSmallRUS(mlook ? "DRK" : "DSRK", 132 + wide_delta, 82);

        // Вертикальная инверсия
        if (!mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextSmallRUS(mouse_y_invert ? "DRK" : "DSRK", 199 + wide_delta, 92);
        dp_translation = NULL;

        // Вертикальное перемещение
        if (mlook)
        dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
        MN_DrTextSmallRUS(!novert ? "DRK" : "DSRK", 227 + wide_delta, 102);
        dp_translation = NULL;
    }

    // Mouse sensivity
    DrawSliderSmall((english_language ? &ControlsMenu : &ControlsMenu_Rus), 3, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HEXEN];
    MN_DrTextA(num, 152 + wide_delta, 73);
    dp_translation = NULL;
}

static void M_RD_AlwaysRun(int option)
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
}

static void M_RD_MouseLook(int option)
{
    mlook ^= 1;
    if (!mlook)
    players[consoleplayer].centering = true;
}

static void M_RD_Sensitivity(int option)
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
}

static void M_RD_InvertY(int option)
{
    mouse_y_invert ^= 1;
}

static void M_RD_Novert(int option)
{
    novert ^= 1;
}

// -----------------------------------------------------------------------------
// DrawGameplayMenu
// -----------------------------------------------------------------------------

static void DrawGameplayMenu(void)
{
    static char *title_eng, *title_rus;

    title_eng = "GAMEPLAY FEATURES";
    title_rus = "YFCNHJQRB UTQVGKTZ";  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextB(title_eng, 160 - MN_TextBWidth(title_eng) / 2 + wide_delta, 7);

        //
        // VISUAL, CROSSHAIR, GAMEPLAY
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextA("VISUAL", 36 + wide_delta, 32);
        MN_DrTextA("CROSSHAIR", 36 + wide_delta, 62);
        MN_DrTextA("GAMEPLAY", 36 + wide_delta, 102);
        dp_translation = NULL;

        // Brightmaps
        dp_translation = brightmaps ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(brightmaps ? "ON" : "OFF", 119 + wide_delta, 42);
        dp_translation = NULL;

        // Fake contrast
        dp_translation = fake_contrast ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(fake_contrast ? "ON" : "OFF", 143 + wide_delta, 52);
        dp_translation = NULL;

        // Draw crosshair
        dp_translation = crosshair_draw ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(crosshair_draw ? "ON" : "OFF", 150 + wide_delta, 72);
        dp_translation = NULL;

        // Indication
        dp_translation = crosshair_type ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(crosshair_type == 1 ? "HEALTH" : "STATIC",  111 + wide_delta, 82);
        dp_translation = NULL;

        // Increased size
        dp_translation = crosshair_scale ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(crosshair_scale ? "ON" : "OFF", 146 + wide_delta, 92);
        dp_translation = NULL;

        // Flip game levels
        dp_translation = flip_levels ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextA(flip_levels ? "ON" : "OFF", 153 + wide_delta, 112);
        dp_translation = NULL;

        // Play internal demos
        dp_translation = no_internal_demos ? cr[CR_GRAY2RED_HEXEN] : cr[CR_GRAY2GREEN_HEXEN];
        MN_DrTextA(no_internal_demos ? "OFF" : "ON", 179 + wide_delta, 122);
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
        // ГРАФИКА, CROSSHAIR, GAMEPLAY
        //
        dp_translation = cr[CR_GRAY2DARKGOLD_HEXEN];
        MN_DrTextSmallRUS("UHFABRF", 36 + wide_delta, 32);
        MN_DrTextSmallRUS("GHBWTK", 36 + wide_delta, 62);
        MN_DrTextSmallRUS("UTQVGKTQ", 36 + wide_delta, 102);
        dp_translation = NULL;

        // Брайтмаппинг
        dp_translation = brightmaps ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(brightmaps ? "DRK" : "DSRK", 133 + wide_delta, 42);
        dp_translation = NULL;

        // Имитация контрастности
        dp_translation = fake_contrast ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(fake_contrast ? "DRK" : "DSRK", 205 + wide_delta, 52);
        dp_translation = NULL;

        // Отображать прицел
        dp_translation = crosshair_draw ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(crosshair_draw ? "DRK" : "DSRK", 175 + wide_delta, 72);
        dp_translation = NULL;

        // Индикация
        dp_translation = crosshair_type ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(crosshair_type == 1 ? "PLJHJDMT" :       // ЗДОРОВЬЕ
                                                "CNFNBXYFZ",       // СТАТИЧНАЯ
                                                111 + wide_delta, 82);
        dp_translation = NULL;

        // Увеличенный размер
        dp_translation = crosshair_scale ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(crosshair_scale ? "DRK" : "DSRK", 181 + wide_delta, 92);
        dp_translation = NULL;

        // Зеркальное отражение уровней
        dp_translation = flip_levels ? cr[CR_GRAY2GREEN_HEXEN] : cr[CR_GRAY2RED_HEXEN];
        MN_DrTextSmallRUS(flip_levels ? "DRK" : "DSRK", 255 + wide_delta, 112);
        dp_translation = NULL;

        // Проигрывать демозаписи
        dp_translation = no_internal_demos ? cr[CR_GRAY2RED_HEXEN] : cr[CR_GRAY2GREEN_HEXEN];
        MN_DrTextSmallRUS(no_internal_demos ? "DRK" : "DSRK", 211 + wide_delta, 122);
        dp_translation = NULL;
    }
}

static void M_RD_Brightmaps(int option)
{
    brightmaps ^= 1;
}

static void M_RD_FakeContrast(int option)
{
    fake_contrast ^= 1;
}

static void M_RD_ShadowedText(int option)
{
    draw_shadowed_text ^= 1;
}

static void M_RD_CrossHairDraw(int option)
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairType(int option)
{
    crosshair_type ^= 1;
}

static void M_RD_CrossHairScale(int option)
{
    crosshair_scale ^= 1;
}

static void M_RD_FlipLevels(int option)
{
    flip_levels ^= 1;

    // [JN] Redraw game screen
    R_ExecuteSetViewSize();
}

static void M_RD_NoDemos(int option)
{
    no_internal_demos ^= 1;
}

//---------------------------------------------------------------------------
// M_RD_ResetSettings
//---------------------------------------------------------------------------

static void M_RD_ResetSettings(int option)
{
    menuactive = false;
    askforquit = true;
    typeofask = 6;              // Reset settings to their defaults
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

void M_RD_DoResetSettings(void)
{
    
    // Rendering
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    smoothing               = 0;
    vga_porch_flash         = 0;
    force_software_renderer = 0;

    // Display
    screenblocks    = 10;
    usegamma        = 0;
    messageson      = 1;
    level_brightness = 0;
    local_time      = 0;

    // Audio
    snd_MaxVolume   = 8;
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
    mlook               = 0;
    players[consoleplayer].centering = true;
    mouseSensitivity    = 5;

    // Gameplay
    brightmaps          = 1;
    fake_contrast       = 0;
    draw_shadowed_text  = 1;
    crosshair_draw      = 0;
    crosshair_type      = 1;
    crosshair_scale     = 0;
    no_internal_demos   = 0;

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
    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
    menuactive = true;
}


//---------------------------------------------------------------------------
// M_RD_ChangeLanguage
//---------------------------------------------------------------------------

static void M_RD_ChangeLanguage(int option)
{
    extern void H2_DoAdvanceDemo(void);
    extern int demosequence;

    english_language ^= 1;

    // Reset options menu
    CurrentMenu = english_language ? &OptionsMenu : &OptionsMenu_Rus;

    // Clear HUD messages
    players[consoleplayer].message[0] = 0;
    players[consoleplayer].yellowMessage = 0;

    // Update window title
    if (isDK)
    {
        I_SetWindowTitle(english_language ?
                        "Hexen: Deathkings of the Dark Citadel" :
                        "Hexen: Короли Смерти Темной Цитадели");
    }
    else if (isHexenDemo)
    {
        I_SetWindowTitle(english_language ?
                        "Hexen: 4 Level Demo Version" :
                        "Hexen: Демоверсия четырех уровней");
    }
    else
    {
        I_SetWindowTitle("Hexen");
    }
    I_InitWindowTitle();

    // Update TITLE/CREDIT screens
    if (gamestate == GS_DEMOSCREEN)
    {
        if (demosequence == 0   // initial title screen
        ||  demosequence == 1   // title screen + advisor
        ||  demosequence == 3   // credits
        ||  demosequence == 5)  // ordering info / credits
        {
            demosequence--;
            H2_DoAdvanceDemo();
        }
    }

    // Update game screen, borders and status bar
    UpdateState |= I_FULLSCRN;
    BorderNeedRefresh = true;
    SB_state = -1;

    // Re-init map info lump
    InitMapInfo();

    // Restart intermission text
    if (gamestate == GS_INTERMISSION)
    {
        IN_Start();
    }

    // Restart finale text
    if (gamestate == GS_FINALE)
    {
        InfoType = 0;
        menuactive = false;
        paused = false;
        F_StartFinale();
    }
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static void SCQuitGame(int option)
{
    menuactive = false;
    askforquit = true;
    typeofask = 1;              //quit game
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCEndGame
//
//---------------------------------------------------------------------------

static void SCEndGame(int option)
{
    if (demoplayback)
    {
        return;
    }
    if (SCNetCheck(3))
    {
        menuactive = false;
        askforquit = true;
        typeofask = 2;          //endgame
        if (!netgame && !demoplayback)
        {
            paused = true;
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static void SCMessages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], english_language ?
        "MESSAGES ON" : // СООБЩЕНИЯ ВКЛЮЧЕНЫ
        "CJJ,OTYBZ DRK.XTYS",
        true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], english_language ?
        "MESSAGES OFF" :
        "CJJ,OTYBZ DSRK.XTYS", // СООБЩЕНИЯ ВЫКЛЮЧЕНЫ
        true);
    }
    S_StartSound(NULL, SFX_CHAT);
}

//===========================================================================
//
// SCNetCheck
//
//===========================================================================

static boolean SCNetCheck(int option)
{
    if (!netgame)
    {
        return true;
    }
    switch (option)
    {
        case 1:                // new game
            P_SetMessage(&players[consoleplayer], english_language ?
                         "YOU CAN'T START A NEW GAME IN NETPLAY!" :
                         "YTDJPVJ;YJ YFXFNM YJDE. BUHE D CTNTDJQ BUHT!", // НЕВОЗМОЖНО НАЧАТЬ НОВУЮ ИГРУ В СЕТЕВОЙ ИГРЕ!
                         true);
            break;
        case 2:                // load game
            P_SetMessage(&players[consoleplayer], english_language ?
                         "YOU CAN'T LOAD A GAME IN NETPLAY!" :
                         "YTDJPVJ;YJ PFUHEPBNMCZ D CTNTDJQ BUHT!", // НЕВОЗМОЖНО ЗАГРУЗИТЬСЯ В СЕТЕВОЙ ИГРЕ!
                         true);
            break;
        case 3:                // end game
            P_SetMessage(&players[consoleplayer], english_language ?
                         "YOU CAN'T END A GAME IN NETPLAY!" :
                         "YTDJPVJ;YJ PFRJYXBNM CTNTDE. BUHE!", // НЕВОЗМОЖНО ЗАКОНЧИТЬ СЕТЕВУЮ ИГРУ!
                         true);
            break;
    }
    menuactive = false;
    S_StartSound(NULL, SFX_CHAT);
    return false;
}

//===========================================================================
//
// SCNetCheck2
//
//===========================================================================

static void SCNetCheck2(int option)
{
    SCNetCheck(option);
    return;
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void SCLoadGame(int option)
{
    if (demoplayback)
    {
        // deactivate playback, return control to player
        demoextend = false;
    }
    if (!SlotStatus[option])
    {                           // Don't try to load from an empty slot
        return;
    }
    G_LoadGame(option);
    MN_DeactivateMenu();
    BorderNeedRefresh = true;
    if (quickload == -1)
    {
        quickload = option + 1;
        P_ClearMessage(&players[consoleplayer]);
    }
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static void SCSaveGame(int option)
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
        *ptr = '[';
        *(ptr + 1) = 0;
        SlotStatus[option]++;
        currentSlot = option;
        slotptr = ptr - SlotText[option];
        return;
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
        P_ClearMessage(&players[consoleplayer]);
    }
}

//==========================================================================
//
// SCClass
//
//==========================================================================

static void SCClass(int option)
{
    if (netgame)
    {
        P_SetMessage(&players[consoleplayer], english_language ?
                     "YOU CAN'T START A NEW GAME FROM WITHIN A NETGAME!" :        
                     "YTDJPVJ;YJ YFXFNM YJDE. BUHE D CTNTDJQ BUHT!", // НЕВОЗМОЖНО НАЧАТЬ НОВУЮ ИГРУ В СЕТЕВОЙ ИГРЕ!
                     true);
        return;
    }
    MenuPClass = option;
    switch (MenuPClass)
    {
        case PCLASS_FIGHTER:
            SkillMenu.x = 120;
            SkillItems[0].text = english_language ? "SQUIRE" : "JHE;TYJCTW";    // ОРУЖЕНОСЕЦ
            SkillItems[1].text = english_language ? "KNIGHT" : "HSWFHM";        // РЫЦАРЬ
            SkillItems[2].text = english_language ? "WARRIOR" : "DJBNTKM";      // ВОИТЕЛЬ
            SkillItems[3].text = english_language ? "BERSERKER" : ",THCTHR";    // БЕРСЕРК
            SkillItems[4].text = english_language ? "TITAN" : "NBNFY";          // ТИТАН
            SkillItems[5].text = english_language ? "AVATAR" : "DTHIBNTKM";     // ВЕРШИТЕЛЬ
            break;
        case PCLASS_CLERIC:
            SkillMenu.x = 116;
            SkillItems[0].text = english_language ? "ALTAR BOY" : "FKNFHYBR";   // АЛТАРНИК
            SkillItems[1].text = english_language ? "ACOLYTE" : "CKE;BNTKM";    // СЛУЖИТЕЛЬ  
            SkillItems[2].text = english_language ? "PRIEST" : "CDZOTYYBR";     // СВЯЩЕННИК
            SkillItems[3].text = english_language ? "CARDINAL" : "RFHLBYFK";    // КАРДИНАЛ
            SkillItems[4].text = english_language ? "POPE" : "TGBCRJG";         // ЕПИСКОП
            SkillItems[5].text = english_language ? "APOSTLE" : "FGJCNJK";      // АПОСТОЛ
            break;
        case PCLASS_MAGE:
            SkillMenu.x = 112;
			SkillItems[0].text = english_language ? "APPRENTICE" : "EXTYBR";                // УЧЕНИК
            SkillItems[1].text = english_language ? "ENCHANTER" : "XFHJLTQ";                // ЧАРОДЕЙ
            SkillItems[2].text = english_language ? "SORCERER" : "RJKLEY";                  // КОЛДУН
            SkillItems[3].text = english_language ? "WARLOCK" : "XTHYJRYB;YBR";             // ЧЕРНОКНИЖНИК
            SkillItems[4].text = english_language ? "HIGHER MAGE" : "DTH[JDYSQ VFU";        // ВЕРХОВНЫЙ МАГ
            SkillItems[5].text = english_language ? "GREAT ARCHMAGE" : "DTKBRBQ FH[BVFU";   // ВЕЛИКИЙ АРХИМАГ
            break;
    }
    SetMenu(MENU_SKILL);
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static void SCSkill(int option)
{
    if (demoplayback)
    {
        // deactivate playback, return control to player
        demoextend = false;
    }

    PlayerClass[consoleplayer] = MenuPClass;
    G_DeferredNewGame(option);
    SB_SetClassData();
    SB_state = -1;
    MN_DeactivateMenu();
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static void SCInfo(int option)
{
    InfoType = 1;
    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t * event)
{
    int key;
    int charTyped;
    int i;
    MenuItem_t *item;
    extern boolean automapactive;
    extern void H2_StartTitle(void);
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

            // [JN] Widescreen: remember choosen widescreen variable before quit.
            aspect_ratio = aspect_ratio_temp;
            I_Quit();
        }
        else
        {
            SCQuitGame(0);
            S_StartSound(NULL, SFX_CHAT);
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

    // Only care about keypresses beyond this point.

    if (event->type != ev_keydown)
    {
        return false;
    }

    key = event->data1;
    charTyped = event->data2;

    if (InfoType)
    {
        /* The 4-Level Demo Version also has 3 Info pages
        if (gamemode == shareware)
        {
            InfoType = (InfoType + 1) % 5;
        }
        else
        */
        {
            InfoType = (InfoType + 1) % 4;
        }
        if (key == KEY_ESCAPE)
        {
            InfoType = 0;
        }
        if (!InfoType)
        {
            if (!netgame && !demoplayback)
            {
                paused = false;
            }
            MN_DeactivateMenu();
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
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
                    P_ClearMessage(&players[consoleplayer]);
                    askforquit = false;
                    typeofask = 0;
                    paused = false;
                    I_SetPalette(W_CacheLumpName(usegamma <= 8 ?
                                                 "PALFIX" :
                                                 "PLAYPAL",
                                                 PU_CACHE));
                    H2_StartTitle();    // go to intro/demo mode.
                    return false;
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
                    BorderNeedRefresh = true;
                    mn_SuicideConsole = true;
                    break;
                case 6:
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
            players[consoleplayer].messageTics = 0;
            askforquit = false;
            typeofask = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            S_ResumeSound();    // [JN] Fix vanilla Hexen bug: resume music playing
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
            S_StartSound(NULL, SFX_PICKUP_KEY);
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
            S_StartSound(NULL, SFX_PICKUP_KEY);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_help)           // F1 (help screen)
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
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
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
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
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
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (key == key_menu_detail)         // F5 (suicide)
        {
            menuactive = false;
            askforquit = true;
            typeofask = 5;  // suicide
            return true;
        }
        else if (key == key_menu_qsave)          // F6 (quicksave)
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
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                    slottextloaded = false; //reload the slot text
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
                    S_StartSound(NULL, SFX_CHAT);
                }
            }
            return true;
        }
        else if (key == key_menu_endgame)        // F7 (end game)
        {
            if (SCNetCheck(3))
            {
                if (gamestate == GS_LEVEL && !demoplayback)
                {
                    S_StartSound(NULL, SFX_CHAT);
                    SCEndGame(0);
                }
            }
            return true;
        }
        else if (key == key_menu_messages)       // F8 (toggle messages)
        {
            SCMessages(0);
            return true;
        }
        else if (key == key_menu_qload)          // F9 (quickload)
        {
            if (SCNetCheck(2))
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
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                    slottextloaded = false; // reload the slot text
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
                    S_StartSound(NULL, SFX_CHAT);
                }
            }
            return true;
        }
        else if (key == key_menu_quit)           // F10 (quit)
        {
            if (gamestate == GS_LEVEL || gamestate == GS_FINALE)
            {
                SCQuitGame(0);
                S_StartSound(NULL, SFX_CHAT);
            }
            return true;
        }
        else if (key == key_menu_gamma)          // F11 (gamma correction)
        {
            usegamma++;
            if (usegamma > 17)
            {
                usegamma = 0;
            }
            SB_PaletteFlash(true);  // force change
            P_SetMessage(&players[consoleplayer], english_language ?
                         GammaText[usegamma] :
                         GammaText_Rus[usegamma],
                         false);
            return true;
        }
        else if (key == KEY_F12)                 // F12 (???)
        {
            // F12 - reload current map (devmaps mode)
            // [JN] Allow only in devparm mode, see this comment:
            // https://github.com/JNechaevsky/russian-doom/issues/210#issuecomment-702321075

            if (netgame || !devparm)
            {
                return false;
            }
            if (gamekeydown[key_speed])
            {               // Monsters ON
                nomonsters = false;
            }
            if (gamekeydown[key_strafe])
            {               // Monsters OFF
                nomonsters = true;
            }
            G_DeferedInitNew(gameskill, gameepisode, gamemap);
            P_SetMessage(&players[consoleplayer], english_language ?
                                                  TXT_CHEATWARP : 
                                                  TXT_CHEATWARP_RUS,
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

        if (key == key_menu_down)                // Next menu item
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
            S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            return (true);
        }
        else if (key == key_menu_up)             // Previous menu item
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
            S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            return (true);
        }
        else if (key == key_menu_left)           // Slider left
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(LEFT_DIR);
                S_StartSound(NULL, SFX_PICKUP_KEY);
            }
            return (true);
        }
        else if (key == key_menu_right)          // Slider right
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(RIGHT_DIR);
                S_StartSound(NULL, SFX_PICKUP_KEY);
            }
            return (true);
        }
        else if (key == key_menu_forward)        // Activate item (enter)
        {
            if (item->type == ITT_SETMENU)
            {
                if (item->func != NULL)
                {
                    item->func(item->option);
                }
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
                    item->func(item->option);
                }
            }
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            return (true);
        }
        else if (key == key_menu_activate)
        {
            MN_DeactivateMenu();
            return (true);
        }
        else if (key == key_menu_back)
        {
            S_StartSound(NULL, SFX_PICKUP_KEY);

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
        else if (charTyped != 0)
        {
            for (i = 0; i < CurrentMenu->itemCount; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(CurrentMenu->items[i].text[0]))
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
    CurrentMenu = english_language ? 
                  &MainMenu :
                  &MainMenu_Rus;
    CurrentItPos = CurrentMenu->oldItPos;
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    S_StartSound(NULL, SFX_PLATFORM_STOP);
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
    S_ResumeSound();    // [JN] Fix vanilla Hexen bug: resume music playing
    menuactive = false;
    if (FileMenuKeySteal)
    {
        I_StopTextInput();
    }
    if (!netgame)
    {
        paused = false;
    }
    S_StartSound(NULL, SFX_PLATFORM_STOP);
    P_ClearMessage(&players[consoleplayer]);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, WIDESCREENWIDTH, SCREENHEIGHT, 0);
    }

    I_SetPalette(W_CacheLumpName(usegamma <= 8 ?
                                 "PALFIX" :
                                 "PLAYPAL",
                                 PU_CACHE));

    // [JN] Some complex mess to avoid using numerical identification of screens.
    if (english_language)
    {
        V_DrawRawScreen(W_CacheLumpNum
                       (W_GetNumForName
                       (InfoType == 0 ? "TITLE" :
                        InfoType == 1 ? "HELP1" :
                        InfoType == 2 ? "HELP2" :
                                        "CREDIT"), PU_CACHE));
    }
    else
    {
        V_DrawRawScreen(W_CacheLumpNum
                       (W_GetNumForName
                       (InfoType == 0 ? "TITLE" :
                        InfoType == 1 ? "RD_HELP1" :
                        InfoType == 2 ? "RD_HELP2" :
                                 isDK ? "RD_CREDK" :
                                        "RD_CREDT"), PU_CACHE));
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
    CurrentMenu = english_language ?
                  Menus[menu] :
                  Menus_Rus[menu];
    CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//
// [JN] Not used for now
//
//---------------------------------------------------------------------------
/*
static void DrawSlider(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;

    x = menu->x + 24;
    y = menu->y + 2 + (item * ITEM_HEIGHT);
    V_DrawShadowedPatchRaven(x - 32 + wide_delta, y, W_CacheLumpName("M_SLDLT", PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2 + wide_delta, y, W_CacheLumpName(count & 1 ? "M_SLDMD1"
                                           : "M_SLDMD2", PU_CACHE));
    }
    V_DrawShadowedPatchRaven(x2 + wide_delta, y, W_CacheLumpName("M_SLDRT", PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7, W_CacheLumpName("M_SLDKD", PU_CACHE));
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7, W_CacheLumpName("M_SLDKR", PU_CACHE));
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + 4 + slot * 8 + wide_delta, y + 7, W_CacheLumpName("M_SLDKB", PU_CACHE));
}
*/

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

    V_DrawShadowedPatchRaven(x - 32 + wide_delta, y, W_CacheLumpName("M_RDSLDL", PU_CACHE));

    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawShadowedPatchRaven(x2 - 16 + wide_delta, y,
                                 W_CacheLumpName("M_RDSLD1", PU_CACHE));
    }

    V_DrawShadowedPatchRaven(x2 - 25 + wide_delta, y,
                             W_CacheLumpName("M_RDSLDR", PU_CACHE));

    // [JN] Colorizing slider gem...
    // Most left position (dull green gem)
    if (slot == 0)
    {
        dp_translation = cr[CR_GREEN2GRAY_HERETIC];
        V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName("M_RDSLG", PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        dp_translation = cr[CR_GREEN2RED_HERETIC];
        V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                    W_CacheLumpName("M_RDSLG", PU_CACHE));
        dp_translation = NULL;
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + slot * 8 + wide_delta, y + 7,
                W_CacheLumpName("M_RDSLG", PU_CACHE));
}

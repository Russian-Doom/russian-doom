//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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



//
// Памятка по символам: docs_russian/charmap/raven_charmap.xlsx
//


// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include "h2def.h"
#include "doomkeys.h"
#include "i_controller.h"
#include "i_input.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h" // [JN] I_GetTime()
#include "i_video.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_trans.h"
#include "v_video.h"
#include "rd_keybinds.h"
#include "rd_menu.h"
#include "rd_rushexen.h"
#include "jn.h"

// MACROS ------------------------------------------------------------------
#define ITEM_HEIGHT 20
#define SLOTTEXTLEN	22
#define ASCII_CURSOR '['

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void InitMapInfo(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SCQuitGame(int option);
static void SCClass(int option);
static void SCSkill(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCInfo(int option);
static void DrawMainMenu(void);
static void DrawClassMenu(void);
static void DrawSkillMenu(void);
static void DrawOptionsMenu(void);
static void DrawOptionsMenu_Vanilla(void);
static void DrawOptions2Menu_Vanilla(void);
static void DrawFileSlots();
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawSaveLoadMenu(void);
void MN_LoadSlotText(void);
void OnActivateMenu(void);
void OnDeactivateMenu(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static void M_RD_Change_Widescreen(Direction_t direction);
static void M_RD_Change_VSync();
static void M_RD_MaxFPS(Direction_t direction);
static void M_RD_FPScounter();
static void M_RD_Smoothing();
static void M_RD_PorchFlashing();
static void M_RD_Screenshots();

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(Direction_t direction);
static void M_RD_LevelBrightness(Direction_t direction);
static void M_RD_Detail();

// Color
static void DrawColorMenu(void);
static void M_RD_Brightness(Direction_t direction);
static void M_RD_Gamma(Direction_t direction);
static void M_RD_Saturation(Direction_t direction);
static void M_RD_ShowPalette();
static void M_RD_RED_Color(Direction_t direction);
static void M_RD_GREEN_Color(Direction_t direction);
static void M_RD_BLUE_Color(Direction_t direction);

// Messages and Texts
static void DrawMessagesMenu(void);
static void M_RD_Messages(Direction_t direction);
static void M_RD_MessagesAlignment(Direction_t direction);
static void M_RD_MessagesTimeout(Direction_t direction);
static void M_RD_MessagesFade();
static void M_RD_ShadowedText();
static void M_RD_LocalTime(Direction_t direction);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapOverlay();
static void M_RD_AutoMapRotate();
static void M_RD_AutoMapFollow();
static void M_RD_AutoMapGrid();

// Sound
static void DrawSoundMenu(void);
static void M_RD_SfxVolume(Direction_t direction);
static void M_RD_MusVolume(Direction_t direction);
static void M_RD_SfxChannels(Direction_t direction);

// Sound system
static void DrawSoundSystemMenu(void);
static void M_RD_SoundDevice();
static void M_RD_MusicDevice(Direction_t direction);
static void M_RD_Sampling(Direction_t direction);
static void M_RD_SndMode();
static void M_RD_PitchShifting();
static void M_RD_MuteInactive();

// Controls
static void DrawControlsMenu(void);
static void M_RD_AlwaysRun();
static void M_RD_Sensitivity(Direction_t direction);
static void M_RD_MouseLook();
static void M_RD_InvertY();
static void M_RD_Novert();

// Key Bindings
void M_RD_Draw_Bindings();

// Gamepad
static void OpenControllerSelectMenu();

static void DrawGamepadSelectMenu();

static void M_RD_UseGamepad();

static void OpenControllerOptionsMenu(int controller);

static void DrawGamepadMenu_1();
static void DrawGamepadMenu_2();

static void M_RD_BindAxis_LY(Direction_t direction);
static void M_RD_SensitivityAxis_LY(Direction_t direction);
static void M_RD_InvertAxis_LY();
static void M_RD_DeadZoneAxis_LY(Direction_t direction);
static void M_RD_BindAxis_LX(Direction_t direction);
static void M_RD_SensitivityAxis_LX(Direction_t direction);
static void M_RD_InvertAxis_LX();
static void M_RD_DeadZoneAxis_LX(Direction_t direction);
static void M_RD_BindAxis_LT(Direction_t direction);
static void M_RD_SensitivityAxis_LT(Direction_t direction);
static void M_RD_InvertAxis_LT();
static void M_RD_DeadZoneAxis_LT(Direction_t direction);

static void M_RD_BindAxis_RY(Direction_t direction);
static void M_RD_SensitivityAxis_RY(Direction_t direction);
static void M_RD_InvertAxis_RY();
static void M_RD_DeadZoneAxis_RY(Direction_t direction);
static void M_RD_BindAxis_RX(Direction_t direction);
static void M_RD_SensitivityAxis_RX(Direction_t direction);
static void M_RD_InvertAxis_RX();
static void M_RD_DeadZoneAxis_RX(Direction_t direction);
static void M_RD_BindAxis_RT(Direction_t direction);
static void M_RD_SensitivityAxis_RT(Direction_t direction);
static void M_RD_InvertAxis_RT();
static void M_RD_DeadZoneAxis_RT(Direction_t direction);

// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static void M_RD_Brightmaps();
static void M_RD_FakeContrast();
static void M_RD_LinearSky();
static void M_RD_FlipCorpses();
static void M_RD_FlipWeapons();
static void M_RD_Collision();
static void M_RD_Torque();
static void M_RD_FloatAmplitude(Direction_t direction);

// Gameplay (page 2)
static void DrawGameplay2Menu(void);
static void M_RD_ColoredSBar();
static void M_RD_ColoredGem(Direction_t direction);
static void M_RD_NegativeHealth();
static void M_RD_CrossHairDraw();
static void M_RD_CrossHairType();
static void M_RD_CrossHairScale();
static void M_RD_FlipLevels();
static void M_RD_NoDemos();

// Level Select (1)
static void DrawLevelSelect1Menu(void);
static void M_RD_SelectiveClass(Direction_t direction);
static void M_RD_SelectiveSkill(Direction_t direction);
static void M_RD_SelectiveHub(Direction_t direction);
static void M_RD_SelectiveMap(Direction_t direction);
static void M_RD_SelectiveHealth(Direction_t direction);
static void M_RD_SelectiveArmor_0(Direction_t direction);
static void M_RD_SelectiveArmor_1(Direction_t direction);
static void M_RD_SelectiveArmor_2(Direction_t direction);
static void M_RD_SelectiveArmor_3(Direction_t direction);

// Level Select (2)
static void DrawLevelSelect2Menu(void);
static void M_RD_SelectiveWp_0();
static void M_RD_SelectiveWp_1();
static void M_RD_SelectiveWp_2();
static void M_RD_SelectiveWp_P_0();
static void M_RD_SelectiveWp_P_1();
static void M_RD_SelectiveWp_P_2();
static void M_RD_SelectiveAmmo_0(Direction_t direction);
static void M_RD_SelectiveAmmo_1(Direction_t direction);
static void M_RD_SelectiveArti_0(Direction_t direction);
static void M_RD_SelectiveArti_1(Direction_t direction);
static void M_RD_SelectiveArti_2(Direction_t direction);

// Level Select (3)
static void DrawLevelSelect3Menu(void);
static void M_RD_SelectiveArti_3(Direction_t direction);
static void M_RD_SelectiveArti_4(Direction_t direction);
static void M_RD_SelectiveArti_5(Direction_t direction);
static void M_RD_SelectiveArti_6(Direction_t direction);
static void M_RD_SelectiveArti_7(Direction_t direction);
static void M_RD_SelectiveArti_8(Direction_t direction);
static void M_RD_SelectiveArti_9(Direction_t direction);
static void M_RD_SelectiveArti_10(Direction_t direction);
static void M_RD_SelectiveArti_11(Direction_t direction);
static void M_RD_SelectiveArti_12(Direction_t direction);
static void M_RD_SelectiveArti_13(Direction_t direction);
static void M_RD_SelectiveArti_14(Direction_t direction);
static void M_RD_SelectiveKey_0();

// Level Select (4)
static void DrawLevelSelect4Menu(void);
static void M_RD_SelectiveKey_1();
static void M_RD_SelectiveKey_2();
static void M_RD_SelectiveKey_3();
static void M_RD_SelectiveKey_4();
static void M_RD_SelectiveKey_5();
static void M_RD_SelectiveKey_6();
static void M_RD_SelectiveKey_7();
static void M_RD_SelectiveKey_8();
static void M_RD_SelectiveKey_9();
static void M_RD_SelectiveKey_10();
static void M_RD_SelectivePuzzle_0(Direction_t direction);
static void M_RD_SelectivePuzzle_1(Direction_t direction);
static void M_RD_SelectivePuzzle_2(Direction_t direction);

// Level Select (5)
static void DrawLevelSelect5Menu(void);
static void M_RD_SelectivePuzzle_3(Direction_t direction);
static void M_RD_SelectivePuzzle_4(Direction_t direction);
static void M_RD_SelectivePuzzle_5(Direction_t direction);
static void M_RD_SelectivePuzzle_6(Direction_t direction);
static void M_RD_SelectivePuzzle_7(Direction_t direction);
static void M_RD_SelectivePuzzle_8(Direction_t direction);
static void M_RD_SelectivePuzzle_9(Direction_t direction);
static void M_RD_SelectivePuzzle_10(Direction_t direction);
static void M_RD_SelectivePuzzle_11(Direction_t direction);
static void M_RD_SelectivePuzzle_12(Direction_t direction);
static void M_RD_SelectivePuzzle_13(Direction_t direction);
static void M_RD_SelectivePuzzle_14(Direction_t direction);
static void M_RD_SelectivePuzzle_15(Direction_t direction);
static void M_RD_SelectivePuzzle_16(Direction_t direction);

// End game
static void SCEndGame(int option);

// Reset settings
static void M_RD_ResetSettings(int option);

// Change language
static void M_RD_ChangeLanguage(int option);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------
extern int MapCount;
extern boolean alwaysRun;

// PUBLIC DATA DEFINITIONS -------------------------------------------------
int InfoType;
boolean mn_SuicideConsole;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// [JN] Original English fonts
static int FontAYellowBaseLump; // small yellow
// [JN] Unchangable Russian fonts
static int FontFYellowBaseLump; // small yellow

static int MauloBaseLump;
static int MenuPClass;
static boolean soundchanged;

// [JN] Used as a timer for hiding menu background
// while changing screen size, gamma and level brightness.
static int menubgwait;

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

//[Dasperal] Predeclare menu variables to allow referencing them before they initialized
static Menu_t* OptionsMenu;
static Menu_t ClassMenu;
static Menu_t RDOptionsMenu;
static Menu_t RenderingMenu;
static Menu_t DisplayMenu;
static Menu_t ColorMenu;
static Menu_t MessagesMenu;
static Menu_t AutomapMenu;
static Menu_t SoundMenu;
static Menu_t SoundSysMenu;
static Menu_t ControlsMenu;
static Menu_t Bindings1Menu;
static Menu_t Bindings2Menu;
static Menu_t Bindings3Menu;
static Menu_t Bindings4Menu;
static Menu_t Bindings5Menu;
static Menu_t Bindings6Menu;
static Menu_t Bindings7Menu;
static const Menu_t* BindingsMenuPages[] = {&Bindings1Menu, &Bindings2Menu, &Bindings3Menu, &Bindings4Menu, &Bindings5Menu, &Bindings6Menu, &Bindings7Menu};
static Menu_t Gamepad1Menu;
static Menu_t Gamepad2Menu;
static const Menu_t* GamepadMenuPages[] = {&Gamepad1Menu, &Gamepad2Menu};
static Menu_t Gameplay1Menu;
static Menu_t Gameplay2Menu;
static const Menu_t* GameplayMenuPages[] = {&Gameplay1Menu, &Gameplay2Menu};
static Menu_t LevelSelectMenu1;
static Menu_t LevelSelectMenu2_F;
static Menu_t LevelSelectMenu2_C;
static Menu_t LevelSelectMenu2_M;
static Menu_t LevelSelectMenu3;
static Menu_t LevelSelectMenu4;
static Menu_t LevelSelectMenu5;
static const Menu_t* LevelSelectMenuPages[] = {&LevelSelectMenu1, &LevelSelectMenu2_F, &LevelSelectMenu3, &LevelSelectMenu4, &LevelSelectMenu5};
static Menu_t VanillaOptions2Menu;
static Menu_t FilesMenu;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

static MenuItem_t HMainItems[] = {
    {ITT_SETMENU_NONET, "NEW GAME",   "YJDFZ BUHF", &ClassMenu,     1}, // НОВАЯ ИГРА
    {ITT_SETMENU,       "OPTIONS",    "YFCNHJQRB",  &RDOptionsMenu, 0}, // НАСТРОЙКИ
    {ITT_SETMENU,       "GAME FILES", "AFQKS BUHS", &FilesMenu,     0}, // ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,         "INFO",       "BYAJHVFWBZ", SCInfo,         0}, // ИНФОРМАЦИЯ
    {ITT_EFUNC,         "QUIT GAME",  "DS[JL",      SCQuitGame,     0}  // ВЫХОД
};

static Menu_t HMainMenu = {
    110, 104,
    56,
    NULL, NULL, true,
    5, HMainItems, true,
    DrawMainMenu,
    NULL,
    NULL,
    0
};

static MenuItem_t ClassItems[] = {
    {ITT_EFUNC, "FIGHTER", "DJBY",   SCClass, 0}, // ВОИН
    {ITT_EFUNC, "CLERIC",  "RKTHBR", SCClass, 1}, // КЛЕРИК
    {ITT_EFUNC, "MAGE",    "VFU",    SCClass, 2}  // МАГ
};

static Menu_t ClassMenu = {
    66, 66,
    66,
    NULL, NULL, true,
    3, ClassItems, true,
    DrawClassMenu,
    NULL,
    &HMainMenu,
    0
};

static MenuItem_t SkillItems_F[] = {
    {ITT_EFUNC, "SQUIRE",    "JHE;TYJCTW", SCSkill, sk_baby},      // ОРУЖЕНОСЕЦ
    {ITT_EFUNC, "KNIGHT",    "HSWFHM",     SCSkill, sk_easy},      // РЫЦАРЬ
    {ITT_EFUNC, "WARRIOR",   "DJBNTKM",    SCSkill, sk_medium},    // ВОИТЕЛЬ
    {ITT_EFUNC, "BERSERKER", ",THCTHR",    SCSkill, sk_hard},      // БЕРСЕРК
    {ITT_EFUNC, "TITAN",     "NBNFY",      SCSkill, sk_nightmare}, // ТИТАН
    {ITT_EFUNC, "AVATAR",    "DTHIBNTKM",  SCSkill, sk_ultranm}    // ВЕРШИТЕЛЬ
};

static Menu_t SkillMenu_F = {
    120, 120,
    44,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    6, SkillItems_F, true,
    DrawSkillMenu,
    NULL,
    &ClassMenu,
    2
};

static MenuItem_t SkillItems_C[] = {
    {ITT_EFUNC, "ALTAR BOY", "FKNFHYBR",  SCSkill, sk_baby},      // АЛТАРНИК
    {ITT_EFUNC, "ACOLYTE",   "CKE;BNTKM", SCSkill, sk_easy},      // СЛУЖИТЕЛЬ
    {ITT_EFUNC, "PRIEST",    "CDZOTYYBR", SCSkill, sk_medium},    // СВЯЩЕННИК
    {ITT_EFUNC, "CARDINAL",  "RFHLBYFK",  SCSkill, sk_hard},      // КАРДИНАЛ
    {ITT_EFUNC, "POPE",      "TGBCRJG",   SCSkill, sk_nightmare}, // ЕПИСКОП
    {ITT_EFUNC, "APOSTLE",   "FGJCNJK",   SCSkill, sk_ultranm}    // АПОСТОЛ
};

static Menu_t SkillMenu_C = {
    116, 116,
    44,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    6, SkillItems_C, true,
    DrawSkillMenu,
    NULL,
    &ClassMenu,
    2
};

static MenuItem_t SkillItems_M[] = {
    {ITT_EFUNC, "APPRENTICE",     "EXTYBR",          SCSkill, sk_baby},      // УЧЕНИК
    {ITT_EFUNC, "ENCHANTER",      "XFHJLTQ",         SCSkill, sk_easy},      // ЧАРОДЕЙ
    {ITT_EFUNC, "SORCERER",       "RJKLEY",          SCSkill, sk_medium},    // КОЛДУН
    {ITT_EFUNC, "WARLOCK",        "XTHYJRYB;YBR",    SCSkill, sk_hard},      // ЧЕРНОКНИЖНИК
    {ITT_EFUNC, "HIGHER MAGE",    "DTH[JDYSQ VFU",   SCSkill, sk_nightmare}, // ВЕРХОВНЫЙ МАГ
    {ITT_EFUNC, "GREAT ARCHMAGE", "DTKBRBQ FH[BVFU", SCSkill, sk_ultranm}    // ВЕЛИКИЙ АРХИМАГ
};

static Menu_t SkillMenu_M = {
    112, 112,
    44,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    6, SkillItems_M, true,
    DrawSkillMenu,
    NULL,
    &ClassMenu,
    2
};

// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t RDOptionsItems[] = {
    {ITT_SETMENU, "RENDERING",         "DBLTJ",          &RenderingMenu,      0}, // ВИДЕО
    {ITT_SETMENU, "DISPLAY",           "\'RHFY",         &DisplayMenu,        0}, // ЭКРАН
    {ITT_SETMENU, "SOUND",             "FELBJ",          &SoundMenu,          0}, // АУДИО
    {ITT_SETMENU, "CONTROLS",          "EGHFDKTYBT",     &ControlsMenu,       0}, // УПРАВЛЕНИЕ
    {ITT_SETMENU, "GAMEPLAY",          "UTQVGKTQ",       &Gameplay1Menu,       0}, // ГЕЙМПЛЕЙ
    {ITT_SETMENU, "LEVEL SELECT",      "DS,JH EHJDYZ",   &LevelSelectMenu1,   0}, // ВЫБОР УРОВНЯ
    {ITT_EFUNC,   "RESET SETTINGS",    "C,HJC YFCNHJTR", M_RD_ResetSettings,  0}, // СБРОС НАСТРОЕК
    {ITT_EFUNC,   "LANGUAGE: ENGLISH", "ZPSR: HECCRBQ",  M_RD_ChangeLanguage, 0}  // ЯЗЫК: РУССКИЙ
};

static Menu_t RDOptionsMenu = {
    77, 77,
    31,
    "OPTIONS", "YFCNHJQRB", false,
    8, RDOptionsItems, true,
    DrawOptionsMenu,
    NULL,
    &HMainMenu,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_TITLE,  "RENDERING",                 "HTYLTHBYU",                       NULL,                   0}, // РЕНДЕРИНГ
    {ITT_LRFUNC, "DISPLAY ASPECT RATIO:",     "CJJNYJITYBT CNJHJY \'RHFYF:",     M_RD_Change_Widescreen, 0}, // СООТНОШЕНИЕ СТОРОН ЭКРАНА
    {ITT_SWITCH, "VERTICAL SYNCHRONIZATION:", "DTHNBRFKMYFZ CBY[HJYBPFWBZ:",     M_RD_Change_VSync,      0}, // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    {ITT_LRFUNC, "FPS LIMIT:",                "JUHFYBXTYBT",                     M_RD_MaxFPS,            0}, // ОГРАНИЧЕНИЕ FPS
    {ITT_SWITCH, "FPS COUNTER:",              "CXTNXBR RFLHJDJQ XFCNJNS:",       M_RD_FPScounter,        0}, // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ
    {ITT_SWITCH, "PIXEL SCALING:",            "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing,         0}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_SWITCH, "PORCH PALETTE CHANGING:",   "BPVTYTYBT GFKBNHS RHFTD 'RHFYF:", M_RD_PorchFlashing,     0}, // ИЗМЕНЕНИЕ ПАЛИТРЫ КРАЕВ ЭКРАНА
    {ITT_TITLE,  "EXTRA",                     "LJGJKYBNTKMYJ",                   NULL,                   0}, // ДОПОЛНИТЕЛЬНО
    {ITT_SWITCH, "SCREENSHOT FORMAT:",        "AJHVFN CRHBYIJNJD:",              M_RD_Screenshots,       0}  // ФОРМАТ СКРИНШОТОВ
};

static Menu_t RenderingMenu = {
    36, 36,
    32,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    9, RenderingItems, false,
    DrawRenderingMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_TITLE,  "SCREEN",              "\'RHFY",                   NULL,                 0}, // ЭКРАН
    {ITT_LRFUNC, "SCREEN SIZE",         "HFPVTH BUHJDJUJ \'RHFYF",  M_RD_ScreenSize,      0}, // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC, "LEVEL BRIGHTNESS",    "EHJDTYM JCDTOTYYJCNB",     M_RD_LevelBrightness, 0}, // УРОВЕНЬ ОСВЕЩЕННОСТИ
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                 0},
    {ITT_SWITCH, "GRAPHICS DETAIL:",    "LTNFKBPFWBZ UHFABRB:",     M_RD_Detail,          0}, // ДЕТАЛИЗАЦИЯ ГРАФИКИ
    {ITT_SETMENU, "COLOR OPTIONS...",   "YFCNHJQRB WDTNF>>>",       &ColorMenu,           0}, // НАСТРОЙКИ ЦВЕТА...
    {ITT_TITLE,  "INTERFACE",           "BYNTHATQC",                NULL,                 0}, // ИНТЕРФЕЙС
    {ITT_SETMENU, "MESSAGES AND TEXTS...", "CJJ,OTYBZ B NTRCNS>>>", &MessagesMenu,        0}, // СООБЩЕНИЯ И ТЕКСТЫ...
    {ITT_SETMENU,"AUTOMAP SETTINGS...", "YFCNHJQRB RFHNS>>>",       &AutomapMenu,         0}  // НАСТРОЙКИ КАРТЫ...
};

static Menu_t DisplayMenu = {
    36, 36,
    32,
    "DISPLAY OPTIONS", "YFCNHJQRB \'RHFYF", false, // НАСТРОЙКИ ЭКРАНА
    10, DisplayItems, false,
    DrawDisplayMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Color options
// -----------------------------------------------------------------------------

static MenuItem_t ColorItems[] = {
    {ITT_LRFUNC, "",  "", M_RD_Brightness,  0}, // Brightness | Яркость
    {ITT_LRFUNC, "",  "", M_RD_Gamma,       0}, // Gamma | Гамма
    {ITT_LRFUNC, "",  "", M_RD_Saturation,  0}, // Saturation | Насыщенность
    {ITT_SWITCH, "",  "", M_RD_ShowPalette, 0}, // Show palette | Отобразить палитру
    {ITT_TITLE,  "",  "", NULL,             0}, // Color intensity | Цветовая интенсивность
    {ITT_LRFUNC, "",  "", M_RD_RED_Color,   0},
    {ITT_LRFUNC, "",  "", M_RD_GREEN_Color, 0},
    {ITT_LRFUNC, "",  "", M_RD_BLUE_Color,  0}
};

static Menu_t ColorMenu = {
    164, 164,
    25,
    "COLOR OPTIONS", "YFCNHJQRF WDTNF", false,  // НАСТРОЙКИ ЦВЕТА
    8, ColorItems, false,
    DrawColorMenu,
    NULL,
    &DisplayMenu,
    0
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

static MenuItem_t MessagesItems[] = {
    {ITT_TITLE,  "GENERAL",             "JCYJDYJT",                 NULL,                         0}, // ОСНОВНОЕ
    {ITT_SWITCH, "MESSAGES:",           "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,                0}, // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_LRFUNC, "ALIGNMENT:",          "DSHFDYBDFYBT:",            M_RD_MessagesAlignment,       0}, // ВЫРАВНИВАНИЕ
    {ITT_LRFUNC, "MESSAGE TIMEOUT",     "NFQVFEN JNJ,HF;TYBZ",      M_RD_MessagesTimeout,         0}, // ТАЙМАУТ ОТОБРАЖЕНИЯ
    {ITT_EMPTY,   NULL,                  NULL,                      NULL,                         0},
    {ITT_SWITCH, "FADING EFFECT:",      "GKFDYJT BCXTPYJDTYBT:",    M_RD_MessagesFade,            0}, // ПЛАВНОЕ ИСЧЕЗНОВЕНИЕ
    {ITT_SWITCH, "TEXT CASTS SHADOWS:", "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText,            0}, // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_TITLE,  "MISC",                "HFPYJT",                   NULL,                         0}, // РАЗНОЕ
    {ITT_LRFUNC, "LOCAL TIME:",         "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,               0}  // СИСТЕМНОЕ ВРЕМЯ
};

static Menu_t MessagesMenu = {
    36, 36,
    32,
    "MESSAGES AND TEXTS", "CJJ,OTYBZ B NTRCNS", false, // СООБЩЕНИЯ И ТЕКСТЫ
    9, MessagesItems, false,
    DrawMessagesMenu,
    NULL,
    &DisplayMenu,
    1
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_SWITCH, "OVERLAY MODE:", "HT;BV YFKJ;TYBZ:",  M_RD_AutoMapOverlay, 0}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_SWITCH, "ROTATE MODE:",  "HT;BV DHFOTYBZ:",   M_RD_AutoMapRotate,  0}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_SWITCH, "FOLLOW MODE:",  "HT;BV CKTLJDFYBZ:", M_RD_AutoMapFollow,  0}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_SWITCH, "GRID:",         "CTNRF:",            M_RD_AutoMapGrid,    0}  // СЕТКА
};

static Menu_t AutomapMenu = {
    102, 82,
    32,
    "AUTOMAP SETTINGS", "YFCNHJQRB RFHNS", false, // НАСТРОЙКИ КАРТЫ
    4, AutomapItems, false,
    DrawAutomapMenu,
    NULL,
    &DisplayMenu,
    0
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_TITLE,  "VOLUME",                   "UHJVRJCNM",                     NULL,             0}, // ГРОМКОСТЬ
    {ITT_LRFUNC, "SFX VOLUME",               "UHJVRJCNM PDERF",               M_RD_SfxVolume,   0}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_LRFUNC, "MUSIC VOLUME",             "UHJVRJCNM VEPSRB",              M_RD_MusVolume,   0}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_TITLE,  "CHANNELS",                 "DJCGHJBPDTLTYBT",               NULL,             0}, // ВОСПРОИЗВЕДЕНИЕ
    {ITT_LRFUNC, "SFX CHANNELS",             "PDERJDST RFYFKS",               M_RD_SfxChannels, 0}, // ЗВУКОВЫЕ КАНАЛЫ
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_TITLE,  "ADVANCED",                 "LJGJKYBNTKMYJ",                 NULL,             0}, // ДОПОЛНИТЕЛЬНО
    {ITT_SETMENU,"SOUND SYSTEM SETTINGS...", "YFCNHJQRB PDERJDJQ CBCNTVS>>>", &SoundSysMenu,    0}  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ...
};

static Menu_t SoundMenu = {
    36, 36,
    32,
    "SOUND OPTIONS", "YFCNHJQRB PDERF", false, // НАСТРОЙКИ ЗВУКА
    10, SoundItems, false,
    DrawSoundMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    {ITT_TITLE,  "SOUND SYSTEM",          "PDERJDFZ CBCNTVF",           NULL,               0}, // ЗВУКВАЯ СИСТЕМА
    {ITT_SWITCH, "SOUND EFFECTS:",        "PDERJDST \'AATRNS:",         M_RD_SoundDevice,   0}, // ЗВУКОВЫЕ ЭФФЕКТЫ:
    {ITT_LRFUNC, "MUSIC:",                "VEPSRF:",                    M_RD_MusicDevice,   0}, // МУЗЫКА:
    {ITT_TITLE,  "QUALITY",               "RFXTCNDJ PDEXFYBZ",          NULL,               0}, // КАЧЕСТВО ЗВУЧАНИЯ
    {ITT_LRFUNC, "SAMPLING FREQUENCY:",   "XFCNJNF LBCRHTNBPFWBB:",     M_RD_Sampling,      0}, // ЧАСТОТА ДИСКРЕТИЗАЦИИ:
    {ITT_TITLE,  "MISCELLANEOUS",         "HFPYJT",                     NULL,               0}, // РАЗНОЕ
    {ITT_SWITCH, "SOUND EFFECTS MODE:",   "HT;BV PDERJDS[ \'AATRNJD:",  M_RD_SndMode,       0}, // РЕЖИМ ЗВУКОВЫХ ЭФФЕКТОВ
    {ITT_SWITCH, "PITCH-SHIFTED SOUNDS:", "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting, 0}, // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
    {ITT_SWITCH, "MUTE INACTIVE WINDOW:", "PDER D YTFRNBDYJV JRYT:",    M_RD_MuteInactive,  0}, // ЗВУК В НЕАКТИВНОМ ОКНЕ
};

static Menu_t SoundSysMenu = {
    36, 36,
    32,
    "SOUND SYSTEM SETTINGS", "YFCNHJQRB PDERJDJQ CBCNTVS", false, // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
    9, SoundSysItems, false,
    DrawSoundSystemMenu,
    NULL,
    &SoundMenu,
    1
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_TITLE,   "CONTROLS",              "EGHFDKTYBT",                NULL,                     0}, // УПРАВЛЕНИЕ
    {ITT_SETMENU, "CUSTOMIZE CONTROLS...", "YFCNHJQRB EGHFDKTYBZ>>>",   &Bindings1Menu,           0}, // Настройки управления...
    {ITT_EFUNC,   "GAMEPAD SETTINGS...",   "YFCNHJQRB UTQVGFLF>>>",     OpenControllerSelectMenu, 0}, // Настройки геймпада...
    {ITT_SWITCH,  "ALWAYS RUN:",           "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,           0}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_TITLE,   "MOUSE",                 "VSIM",                      NULL,                     0}, // МЫШЬ
    {ITT_LRFUNC,  "MOUSE SENSIVITY",       "CRJHJCNM VSIB",             M_RD_Sensitivity,         0}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,   NULL,                    NULL,                        NULL,                     0},
    {ITT_SWITCH,  "MOUSE LOOK:",           "J,PJH VSIM.:",              M_RD_MouseLook,           0}, // ОБЗОР МЫШЬЮ
    {ITT_SWITCH,  "INVERT Y AXIS:",        "DTHNBRFKMYFZ BYDTHCBZ:",    M_RD_InvertY,             0}, // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    {ITT_SWITCH,  "VERTICAL MOVEMENT:",    "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,              0}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36, 36,
    32,
    "CONTROL SETTINGS", "EGHFDKTYBT", false, // УПРАВЛЕНИЕ
    10, ControlsItems, false,
    DrawControlsMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t BindingsPageDescriptor = {
    7, BindingsMenuPages,
    252, 165,
    CR_WHITE
};


static MenuItem_t Bindings1Items[] = {
    {ITT_TITLE,   "MOVEMENT",      "LDB;TYBT",             NULL,               0},
    {ITT_EFUNC,   "MOVE FORWARD",  "LDB;TYBT DGTHTL",      BK_StartBindingKey, bk_forward},      // Движение вперед
    {ITT_EFUNC,   "MOVE BACKWARD", "LDB;TYBT YFPFL",       BK_StartBindingKey, bk_backward},     // Движение назад
    {ITT_EFUNC,   "TURN Left",     "GJDJHJN YFKTDJ",       BK_StartBindingKey, bk_turn_left},    // Поворот налево
    {ITT_EFUNC,   "TURN Right",    "GJDJHJN YFGHFDJ",      BK_StartBindingKey, bk_turn_right},   // Поворот направо
    {ITT_EFUNC,   "STRAFE LEFT",   ",JRJV DKTDJ",          BK_StartBindingKey, bk_strafe_left},  // Боком влево
    {ITT_EFUNC,   "STRAFE RIGHT",  ",JRJV DGHFDJ",         BK_StartBindingKey, bk_strafe_right}, // Боком вправо
    {ITT_EFUNC,   "SPEED ON",      ",TU",                  BK_StartBindingKey, bk_speed},        // Бег
    {ITT_EFUNC,   "STRAFE ON",     "LDB;TYBT ,JRJV",       BK_StartBindingKey, bk_strafe},       // Движение боком
    {ITT_EFUNC,   "FLY UP",        "KTNTNM DDTH[",         BK_StartBindingKey, bk_fly_up},       // Лететь вверх
    {ITT_EFUNC,   "FLY DOWN",      "KTNTNM DYBP",          BK_StartBindingKey, bk_fly_down},     // Лететь вних
    {ITT_EFUNC,   "STOP FLYING",   "JCNFYJDBNM GJKTN",     BK_StartBindingKey, bk_fly_stop},     // Остановить полёт
    {ITT_EMPTY,   NULL,            NULL,                   NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",   "CKTLE.OFZ CNHFYBWF `", &Bindings2Menu,     0},               // Cледующая страница >
    {ITT_SETMENU, "< LAST PAGE",   "^ GJCKTLYZZ CNHFYBWF", &Bindings7Menu,     0},               // < Последняя страница
    {ITT_EMPTY,   NULL,            NULL,                   NULL,               0}
};

static Menu_t Bindings1Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings1Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (2)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings2Items[] = {
    {ITT_TITLE,   "ACTION",          "LTQCNDBT",              NULL,               0},
    {ITT_EFUNC,   "FIRE/ATTACK",     "FNFRF/CNHTKM,F",        BK_StartBindingKey, bk_fire},        // Атака/стрельба
    {ITT_EFUNC,   "USE",             "BCGJKMPJDFNM",          BK_StartBindingKey, bk_use},         // Использовать
    {ITT_EFUNC,   "JUMP",            "GHS;JR",                BK_StartBindingKey, bk_jump},        //Прыжок
    {ITT_TITLE,   "WEAPONS",         "JHE;BT",                NULL,               0},       // Оружие
    {ITT_EFUNC,   "WEAPON 1",        "JHE;BT 1",              BK_StartBindingKey, bk_weapon_1},    // Оружие 1
    {ITT_EFUNC,   "WEAPON 2",        "JHE;BT 2",              BK_StartBindingKey, bk_weapon_2},    // Оружие 2
    {ITT_EFUNC,   "WEAPON 3",        "JHE;BT 3",              BK_StartBindingKey, bk_weapon_3},    // Оружие 3
    {ITT_EFUNC,   "WEAPON 4",        "JHE;BT 4",              BK_StartBindingKey, bk_weapon_4},    // Оружие 4
    {ITT_EFUNC,   "PREVIOUS WEAPON", "GHTLSLEOTT JHE;BT",     BK_StartBindingKey, bk_weapon_prev}, // Предыдущее оружие
    {ITT_EFUNC,   "NEXT WEAPON",     "CKTLE.OTT JHE;BT",      BK_StartBindingKey, bk_weapon_next}, // Следующее оружие
    {ITT_EMPTY,   NULL,              NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,              NULL,                    NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",     "CKTLE.OFZ CNHFYBWF `",  &Bindings3Menu,     0},              // Cледующая страница >
    {ITT_SETMENU, "< PREV PAGE",     "^ GHTLSLEOFZ CNHFYBWF", &Bindings1Menu,     0},              // < Предыдущая страница
    {ITT_EMPTY,   NULL,              NULL,                    NULL,               0}
};

static Menu_t Bindings2Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings2Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (3)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings3Items[] = {
    {ITT_TITLE,   "SHORTCUT KEYS",         ",SCNHSQ LJCNEG",        NULL,               0},
    {ITT_EFUNC,   "QUICK SAVE",            ",SCNHJT CJ[HFYTYBT",    BK_StartBindingKey, bk_qsave},            // Быстрое сохранение
    {ITT_EFUNC,   "QUICK LOAD",            ",SCNHFZ PFUHEPRF",      BK_StartBindingKey, bk_qload},            // Быстрая загрузка
    {ITT_EFUNC,   "RESTART LEVEL/DEMO",    "GTHTPFGECR EHJDYZ",     BK_StartBindingKey, bk_reloadlevel},      // Перезапуск уровня
    {ITT_EFUNC,   "SAVE A SCREENSHOT",     "CRHBYIJN",              BK_StartBindingKey, bk_screenshot},       // Скриншот
    {ITT_EFUNC,   "FINISH DEMO RECORDING", "PFRJYXBNM PFGBCM LTVJ", BK_StartBindingKey, bk_finish_demo},      // Закончить запись демо
    {ITT_TITLE,   "TOGGLEABLES",           "GTHTRK.XTYBT",          NULL,               0},
    {ITT_EFUNC,   "MOUSE LOOK",            "J,PJH VSIM.",           BK_StartBindingKey, bk_toggle_mlook},     // Обзор мышью
    {ITT_EFUNC,   "ALWAYS RUN",            "GJCNJZYYSQ ,TU",        BK_StartBindingKey, bk_toggle_autorun},   // Постоянный бег
    {ITT_EFUNC,   "CROSSHAIR",             "GHBWTK",                BK_StartBindingKey, bk_toggle_crosshair}, // Прицел
    {ITT_EFUNC,   "LEVEL FLIPPING",        "PTHRFKBHJDFYBT EHJDYZ", BK_StartBindingKey, bk_toggle_fliplvls},  // Зеркалирование уровня
    {ITT_EMPTY,   NULL,                    NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,                    NULL,                    NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",           "CKTLE.OFZ CNHFYBWF `",  &Bindings4Menu,     0},                   // Cледующая страница >
    {ITT_SETMENU, "< PREV PAGE",           "^ GHTLSLEOFZ CNHFYBWF", &Bindings2Menu,     0},                   // < Предыдущая страница
    {ITT_EMPTY,   NULL,                    NULL,                    NULL,               0}
};

static Menu_t Bindings3Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings3Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (4)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings4Items[] = {
    {ITT_TITLE,   "AUTOMAP",          "RFHNF",                 NULL,                0},
    {ITT_EFUNC,   "TOGGLE AUTOMAP",   "JNRHSNM RFHNE",         BK_StartBindingKey, bk_map_toggle},    // Открыть карту
    {ITT_EFUNC,   "ZOOM IN",          "GHB,KBPBNM",            BK_StartBindingKey, bk_map_zoom_in},   // Приблизить
    {ITT_EFUNC,   "ZOOM OUT",         "JNLFKBNM",              BK_StartBindingKey, bk_map_zoom_out},  // Отдалить
    {ITT_EFUNC,   "MAXIMUM ZOOM OUT", "GJKYSQ VFCINF,",        BK_StartBindingKey, bk_map_zoom_max},  // Полный масштаб
    {ITT_EFUNC,   "FOLLOW MODE",      "HT;BV CKTLJDFYBZ",      BK_StartBindingKey, bk_map_follow},    // Режим следования
    {ITT_EFUNC,   "OVERLAY MODE",     "HT;BV YFKJ;TYBZ",       BK_StartBindingKey, bk_map_overlay},   // Режим наложения
    {ITT_EFUNC,   "ROTATE MODE",      "HT;BV DHFOTYBZ",        BK_StartBindingKey, bk_map_rotate},    // Режим вращения
    {ITT_EFUNC,   "TOGGLE GRID",      "CTNRF",                 BK_StartBindingKey, bk_map_grid},      // Сетка
    {ITT_EMPTY,   NULL,               NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,               NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,               NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,               NULL,                    NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",      "CKTLE.OFZ CNHFYBWF `",  &Bindings5Menu,     0},                // Cледующая страница >
    {ITT_SETMENU, "< PREV PAGE",      "^ GHTLSLEOFZ CNHFYBWF", &Bindings3Menu,     0},                // < Предыдущая страница
    {ITT_EMPTY,   NULL,               NULL,                    NULL,               0}
};

static Menu_t Bindings4Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings4Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (5)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings5Items[] = {
    {ITT_TITLE,   "INVENTORY",            "BYDTYNFHM",                 NULL,               0},
    {ITT_EFUNC,   "NEXT ITEM",            "CKTLE.OBQ GHTLVTN",         BK_StartBindingKey, bk_inv_right},
    {ITT_EFUNC,   "PREVIOUS ITEM",        "GHTLSLEOBQ GHTLVTN",        BK_StartBindingKey, bk_inv_left},
    {ITT_EFUNC,   "ACTIVATE ITEM",        "BCGJKMPJDFNM GHTLVTN",      BK_StartBindingKey, bk_inv_use_artifact},
    {ITT_EFUNC,   "USE ALL ITEMS",        "BCGJKMPJDFNM DCT GHTLVTNS", BK_StartBindingKey, bk_arti_all},           // Использовать все предметы
    {ITT_EFUNC,   "QUARTZ FLASK",         "RDFHWTDSQ AKFRJY",          BK_StartBindingKey, bk_arti_quartz},
    {ITT_EFUNC,   "MYSTIC URN",           "VBCNBXTCRFZ EHYF",          BK_StartBindingKey, bk_arti_urn},
    {ITT_EFUNC,   "FLECHETTE",            "PTKMT",                     BK_StartBindingKey, bk_arti_bomb},
    {ITT_EFUNC,   "DISC OF REPULSION",    "LBCR JNNJH;TYBZ",           BK_StartBindingKey, bk_arti_blastradius},
    {ITT_EFUNC,   "ICON OF THE DEFENDER", "CBVDJK PFOBNYBRF",          BK_StartBindingKey, bk_arti_invulnerability},
    {ITT_EFUNC,   "PORKALATOR",           "CDBYJVJHATH",               BK_StartBindingKey, bk_arti_egg},
    {ITT_EFUNC,   "CHAOS DEVICE",         "'V,KTVF [FJCF",             BK_StartBindingKey, bk_arti_chaosdevice},
    {ITT_EMPTY,   NULL,                   NULL,                        NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",          "CKTLE.OFZ CNHFYBWF `",      &Bindings6Menu,     0},                     // Cледующая страница >
    {ITT_SETMENU, "< PREV PAGE",          "^ GHTLSLEOFZ CNHFYBWF",     &Bindings4Menu,     0},                     // < Предыдущая страница
    {ITT_EMPTY,   NULL,                   NULL,                        NULL,               0}
};

static Menu_t Bindings5Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings5Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (6)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings6Items[] = {
    {ITT_EFUNC,   "BANISHMENT DEVICE",   "'V,KTVF BPUYFYBZ",          BK_StartBindingKey, bk_arti_teleportother},
    {ITT_EFUNC,   "WINGS OF WRATH",      "RHSKMZ UYTDF",              BK_StartBindingKey, bk_arti_wings},
    {ITT_EFUNC,   "TORCH",               "AFRTK",                     BK_StartBindingKey, bk_arti_torch},
    {ITT_EFUNC,   "KRATER OF MIGHT",     "XFIF VJUEOTCNDF",           BK_StartBindingKey, bk_arti_boostmana},
    {ITT_EFUNC,   "DRAGONSKIN BRACERS",  "YFHEXB BP LHFRJYMTQ RJ;B",  BK_StartBindingKey, bk_arti_boostarmor},
    {ITT_EFUNC,   "DARK SERVANT",        "NTVYSQ CKEUF",              BK_StartBindingKey, bk_arti_summon},
    {ITT_EFUNC,   "BOOTS OF SPEED",      "CFGJUB-CRJHJ[JLS",          BK_StartBindingKey, bk_arti_speed},
    {ITT_EFUNC,   "MYSTIC AMBIT INCANT", "XFHS VFUBXTCRJUJ TLBYCNDF", BK_StartBindingKey, bk_arti_healingradius},
    {ITT_TITLE,   "LOOK",                "J,PJH",                     NULL,               0},              // Обзор
    {ITT_EFUNC,   "LOOK UP",             "CVJNHTNM DDTH[",            BK_StartBindingKey, bk_look_up},            // Смотреть вверх
    {ITT_EFUNC,   "LOOK DOWN",           "CVJNHTNM DYBP",             BK_StartBindingKey, bk_look_down},          // Смотреть вниз
    {ITT_EFUNC,   "CENTER LOOK",         "CVJNHTNM GHZVJ",            BK_StartBindingKey, bk_look_center},        // Смотреть прямо
    {ITT_EMPTY,   NULL,                  NULL,                        NULL,               0},
    {ITT_SETMENU, "NEXT PAGE >",         "CKTLE.OFZ CNHFYBWF `",      &Bindings7Menu,     0},                     // Cледующая страница >
    {ITT_SETMENU, "< PREV PAGE",         "^ GHTLSLEOFZ CNHFYBWF",     &Bindings5Menu,     0},                     // < Предыдущая страница
    {ITT_EMPTY,   NULL,                  NULL,                        NULL,               0}
};

static Menu_t Bindings6Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings6Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    0
};

// -----------------------------------------------------------------------------
// Key bindings (7)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings7Items[] = {
    {ITT_TITLE,   "MULTIPLAYER",         "CTNTDFZ BUHF",          NULL,               0},                     // Сетевая игра
    {ITT_EFUNC,   "MULTIPLAYER SPY",     "DBL LHEUJUJ BUHJRF",    BK_StartBindingKey, bk_spy},                // Вид другого игрока
    {ITT_EFUNC,   "SEND MESSAGE",        "JNGHFDBNM CJJ,OTYBT",   BK_StartBindingKey, bk_multi_msg},          // Отправить сообщение
    {ITT_EFUNC,   "MESSAGE TO PLAYER 1", "CJJ,OTYBT BUHJRE 1",    BK_StartBindingKey, bk_multi_msg_player_0}, // Сообщение игроку 1
    {ITT_EFUNC,   "MESSAGE TO PLAYER 2", "CJJ,OTYBT BUHJRE 2",    BK_StartBindingKey, bk_multi_msg_player_1}, // Сообщение игроку 2
    {ITT_EFUNC,   "MESSAGE TO PLAYER 3", "CJJ,OTYBT BUHJRE 3",    BK_StartBindingKey, bk_multi_msg_player_2}, // Сообщение игроку 3
    {ITT_EFUNC,   "MESSAGE TO PLAYER 4", "CJJ,OTYBT BUHJRE 4",    BK_StartBindingKey, bk_multi_msg_player_3}, // Сообщение игроку 4
    {ITT_EFUNC,   "MESSAGE TO PLAYER 5", "CJJ,OTYBT BUHJRE 5",    BK_StartBindingKey, bk_multi_msg_player_4}, // Сообщение игроку 5
    {ITT_EFUNC,   "MESSAGE TO PLAYER 6", "CJJ,OTYBT BUHJRE 6",    BK_StartBindingKey, bk_multi_msg_player_5}, // Сообщение игроку 6
    {ITT_EFUNC,   "MESSAGE TO PLAYER 7", "CJJ,OTYBT BUHJRE 7",    BK_StartBindingKey, bk_multi_msg_player_6}, // Сообщение игроку 7
    {ITT_EFUNC,   "MESSAGE TO PLAYER 8", "CJJ,OTYBT BUHJRE 8",    BK_StartBindingKey, bk_multi_msg_player_7}, // Сообщение игроку 8
    {ITT_EMPTY,   NULL,                  NULL,                    NULL,               0},
    {ITT_EMPTY,   NULL,                  NULL,                    NULL,               0},
    {ITT_SETMENU, "FIRST PAGE >",        "GTHDFZ CNHFYBWF `",     &Bindings1Menu,     0},                     // Первая страница >
    {ITT_SETMENU, "< PREV PAGE",         "^ GHTLSLEOFZ CNHFYBWF", &Bindings6Menu,     0},                     // < Предыдущая страница
    {ITT_EMPTY,   NULL,                  NULL,                    NULL,               0}
};

static Menu_t Bindings7Menu = {
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    16, Bindings7Items, false,
    M_RD_Draw_Bindings,
    &BindingsPageDescriptor,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gamepad
// -----------------------------------------------------------------------------

static MenuItem_t GamepadSelectItems[] = {
    {ITT_SWITCH, "ENABLE GAMEPAD:",     "BCGJKMPJDFNM UTQVGFL:", M_RD_UseGamepad,            0}, // ИСПОЛЬЗОВАТЬ ГЕЙМПАД
    {ITT_EMPTY,  NULL,                  NULL,                    NULL,                       0},
    {ITT_TITLE,  "ACTIVE CONTROLLERS:", "FRNBDYST UTQVGFLS:",    NULL,                       0}, // АКТИАНЫЕ ГЕЙМПАДЫ
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1},
    {ITT_EMPTY,  NULL,                  NULL,                    OpenControllerOptionsMenu, -1}
};

static Menu_t GamepadSelectMenu = {
    76, 66,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    13, GamepadSelectItems, false,
    DrawGamepadSelectMenu,
    NULL,
    &ControlsMenu,
    0
};

static const PageDescriptor_t GamepadPageDescriptor = {
    2, GamepadMenuPages,
    252, 182,
    CR_WHITE
};

static MenuItem_t Gamepad1Items[] = {
    {ITT_LRFUNC,  "LEFT X AXIS:",  "KTDFZ [ JCM:",         M_RD_BindAxis_LX,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",    M_RD_SensitivityAxis_LX, 0},
    {ITT_SWITCH,  "INVERT AXIS:",  "BYDTHNBHJDFNM JCM:",   M_RD_InvertAxis_LX,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",    "VTHNDFZ PJYF:",        M_RD_DeadZoneAxis_LX,    0},
    {ITT_EMPTY,   NULL,            NULL,                   NULL,                    0},
    {ITT_LRFUNC,  "LEFT Y AXIS:",  "KTDFZ E JCM:",         M_RD_BindAxis_LY,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",    M_RD_SensitivityAxis_LY, 0},
    {ITT_SWITCH,  "INVERT AXIS:",  "BYDTHNBHJDFNM JCM:",   M_RD_InvertAxis_LY,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",    "VTHNDFZ PJYF:",        M_RD_DeadZoneAxis_LY,    0},
    {ITT_EMPTY,   NULL,            NULL,                   NULL,                    0},
    {ITT_LRFUNC,  "LEFT TRIGGER:", "KTDSQ NHBUUTH:",       M_RD_BindAxis_LT,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",    M_RD_SensitivityAxis_LT, 0},
    {ITT_SWITCH,  "INVERT AXIS:",  "BYDTHNBHJDFNM JCM:",   M_RD_InvertAxis_LT,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",    "VTHNDFZ PJYF:",        M_RD_DeadZoneAxis_LT,    0},
    {ITT_EMPTY,   NULL,            NULL,                   NULL,                    0},
    {ITT_SETMENU, "NEXT PAGE >",   "CKTLE>OFZ CNHFYBWF `", &Gamepad2Menu,           0}
};

static Menu_t Gamepad1Menu = {
    36, 21,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    16, Gamepad1Items, false,
    DrawGamepadMenu_1,
    &GamepadPageDescriptor,
    &GamepadSelectMenu,
    0
};

static MenuItem_t Gamepad2Items[] = {
    {ITT_LRFUNC,  "RIGHT X AXIS:",  "GHFDFZ [ JCM:",         M_RD_BindAxis_RX,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",     M_RD_SensitivityAxis_RX, 0},
    {ITT_SWITCH,  "INVERT AXIS:",   "BYDTHNBHJDFNM JCM:",    M_RD_InvertAxis_RX,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",     "VTHNDFZ PJYF:",         M_RD_DeadZoneAxis_RX,    0},
    {ITT_EMPTY,   NULL,             NULL,                    NULL,                    0},
    {ITT_LRFUNC,  "RIGHT Y AXIS:",  "GHFDFZ E JCM:",         M_RD_BindAxis_RY,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",     M_RD_SensitivityAxis_RY, 0},
    {ITT_SWITCH,  "INVERT AXIS:",   "BYDTHNBHJDFNM JCM:",    M_RD_InvertAxis_RY,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",     "VTHNDFZ PJYF:",         M_RD_DeadZoneAxis_RY,    0},
    {ITT_EMPTY,   NULL,             NULL,                    NULL,                    0},
    {ITT_LRFUNC,  "RIGHT TRIGGER:", "GHFDSQ NHBUUTH:",       M_RD_BindAxis_RT,        0},
    {ITT_LRFUNC,  "SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",     M_RD_SensitivityAxis_RT, 0},
    {ITT_SWITCH,  "INVERT AXIS:",   "BYDTHNBHJDFNM JCM:",    M_RD_InvertAxis_RT,      0},
    {ITT_LRFUNC,  "DEAD ZONE:",     "VTHNDFZ PJYF:",         M_RD_DeadZoneAxis_RT,    0},
    {ITT_EMPTY,   NULL,             NULL,                    NULL,                    0},
    {ITT_SETMENU, "< PREV PAGE",    "^ GHTLSLEOFZ CNHFYBWF", &Gamepad1Menu,           0}
};

static Menu_t Gamepad2Menu = {
    36, 21,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    16, Gamepad2Items, false,
    DrawGamepadMenu_2,
    &GamepadPageDescriptor,
    &GamepadSelectMenu,
    0
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t GameplayPageDescriptor = {
    2, GameplayMenuPages,
    254, 172,
    CR_GRAY2GDARKGRAY_HEXEN
};

static MenuItem_t Gameplay1Items[] = {
    {ITT_TITLE,  "VISUAL",                       "UHFABRF",                      NULL,                0}, // ГРАФИКА
    {ITT_SWITCH, "BRIGHTMAPS:",                  ",HFQNVFGGBYU:",                M_RD_Brightmaps,     0}, // БРАЙТМАППИНГ
    {ITT_SWITCH, "FAKE CONTRAST:",               "BVBNFWBZ RJYNHFCNYJCNB:",      M_RD_FakeContrast,   0}, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_SWITCH, "SKY DRAWING MODE:",            "HT;BV JNHBCJDRB YT,F:",        M_RD_LinearSky,      0}, // РЕЖИМ ОТРИСОВКИ НЕБА
    {ITT_SWITCH, "RANDOMLY MIRRORED CORPSES:",   "PTHRFKMYJT JNHF;TYBT NHEGJD:", M_RD_FlipCorpses,    0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ ТРУПОВ
    {ITT_SWITCH, "FLIP WEAPONS:",                "PTHRFKMYJT JNHF;TYBT JHE;BZ:", M_RD_FlipWeapons,    0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ ОРУЖИЯ
    {ITT_TITLE,  "PHYSICAL",                     "ABPBRF",                       NULL,                0}, // ФИЗИКА
    {ITT_SWITCH, "COLLISION PHYSICS:",           "ABPBRF CNJKRYJDTYBQ:",         M_RD_Collision,      0}, // ФИЗИКА СТОЛКНОВЕНИЙ
    {ITT_SWITCH, "CORPSES SLIDING FROM LEDGES:", "NHEGS CGJKPF.N C DJPDSITYBQ:", M_RD_Torque,         0}, // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_LRFUNC, "FLOATING ITEMS AMPLITUDE:" ,   "KTDBNFWBZ GHTLVTNJD:",         M_RD_FloatAmplitude, 0}, // АМПЛИТУДА ЛЕВИТАЦИИ ПРЕДМЕТОВ
    {ITT_EMPTY,   NULL,                          NULL,                           NULL,                0},
    {ITT_EMPTY,   NULL,                          NULL,                           NULL,                0},
    {ITT_EMPTY,   NULL,                          NULL,                           NULL,                0},
    {ITT_EMPTY,   NULL,                          NULL,                           NULL,                0},
    {ITT_SETMENU, "NEXT PAGE >",                 "CKTLE.OFZ CNHFYBWF `",         &Gameplay2Menu,      0}  // СЛЕДУЮЩАЯ СТРАНИЦА >
};

static Menu_t Gameplay1Menu = {
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    15, Gameplay1Items, false,
    DrawGameplay1Menu,
    &GameplayPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gameplay features (2)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay2Items[] = {
    {ITT_TITLE,  "STATUS BAR",           "CNFNEC-,FH",                    NULL,                 0}, // СТАТУС-БАР
    {ITT_SWITCH, "COLORED STATUS BAR:",  "HFPYJWDTNYST \'KTVTYNS:",       M_RD_ColoredSBar,     0}, // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ
    {ITT_LRFUNC, "COLORED HEALTH GEM:",  "JRHFIBDFYBT RFVYZ PLJHJDMZ:",   M_RD_ColoredGem,      0}, // ОКРАШИВАНИЕ КАМНЯ ЗДОРОВЬЯ
    {ITT_SWITCH, "SHOW NEGATIVE HEALTH:","JNHBWFNTKMYJT PLJHJDMT:",       M_RD_NegativeHealth,  0}, // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ
    {ITT_TITLE,  "CROSSHAIR",            "GHBWTK",                        NULL,                 0}, // ПРИЦЕЛ
    {ITT_SWITCH, "DRAW CROSSHAIR:",      "JNJ,HF;FNM GHBWTK:",            M_RD_CrossHairDraw,   0}, // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_SWITCH, "INDICATION:",          "BYLBRFWBZ:",                    M_RD_CrossHairType,   0}, // ИНДИКАЦИЯ
    {ITT_SWITCH, "INCREASED SIZE:",      "EDTKBXTYYSQ HFPVTH:",           M_RD_CrossHairScale,  0}, // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_TITLE,  "GAMEPLAY",             "UTQVGKTQ",                      NULL,                 0}, // ГЕЙМПЛЕЙ
    {ITT_SWITCH, "FLIP GAME LEVELS:",    "PTHRFKMYJT JNHF;TYBT EHJDYTQ:", M_RD_FlipLevels,      0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_SWITCH, "PLAY INTERNAL DEMOS:", "GHJBUHSDFNM LTVJPFGBCB:",       M_RD_NoDemos,         0}, // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_EMPTY,   NULL,                  NULL,                            NULL,                 0},
    {ITT_EMPTY,   NULL,                  NULL,                            NULL,                 0},
    {ITT_EMPTY,   NULL,                  NULL,                            NULL,                 0},
    {ITT_SETMENU, "< PREV PAGE",           "^ GHTLSLEOFZ CNHFYBWF",       &Gameplay1Menu,       0}  // < ПРЕДЫДУЩАЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    15, Gameplay2Items, false,
    DrawGameplay2Menu,
    &GameplayPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Level select (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t LevelSelectPageDescriptor = {
    5, LevelSelectMenuPages,
    248, 176,
    CR_GRAY2GDARKGRAY_HEXEN
};

static MenuItem_t Level1Items[] = {
    {ITT_LRFUNC,  "CLASS:",             "RKFCC:",                M_RD_SelectiveClass,   0}, // КЛАСС
    {ITT_LRFUNC,  "SKILL LEVEL:",       "CKJ;YJCNM:",            M_RD_SelectiveSkill,   0}, // СЛОЖНОСТЬ
    {ITT_LRFUNC,  "HUB:",               "[F,:",                  M_RD_SelectiveHub,     0}, // ХАБ
    {ITT_LRFUNC,  "MAP:",               "EHJDTYM:",              M_RD_SelectiveMap,     0}, // УРОВЕНЬ
    {ITT_TITLE,   "PLAYER",             "BUHJR",                 NULL,                  0}, // ИГРОК
    {ITT_LRFUNC,  "HEALTH:",            "PLJHJDMT:",             M_RD_SelectiveHealth,  0}, // ЗДОРОВЬЕ
    {ITT_TITLE,   "ARMOR:",             ",HJYZ:",                NULL,                  0}, // БРОНЯ
    {ITT_LRFUNC,  "MESH ARMOR:",        "RJKMXEUF:",             M_RD_SelectiveArmor_0, 0}, // КОЛЬЧУГА
    {ITT_LRFUNC,  "FALCON SHIELD:",     "CJRJKBYSQ OBN:",        M_RD_SelectiveArmor_1, 0}, // СОКОЛИНЫЙ ЩИТ
    {ITT_LRFUNC,  "PLATINUM HELMET:",   "GKFNBYJDSQ IKTV:",      M_RD_SelectiveArmor_2, 0}, // ПЛАТИНОВЫЙ ШЛЕМ
    {ITT_LRFUNC,  "AMULET OF WARDING:", "FVEKTN CNHF;F:",        M_RD_SelectiveArmor_3, 0}, // АМУЛЕТ СТРАЖА
    {ITT_EMPTY,   NULL,                 NULL,                    NULL,                  0},
    {ITT_EMPTY,   NULL,                 NULL,                    NULL,                  0},
    {ITT_EMPTY,   NULL,                 NULL,                    NULL,                  0},
    {ITT_EMPTY,   NULL,                 NULL,                    NULL,                  0},
    {ITT_SETMENU, "NEXT PAGE >",        "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu2_F,   0}, // СЛЕДУЮЩАЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",         "YFXFNM BUHE",           G_DoSelectiveGame,     0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu1 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level1Items, false,
    DrawLevelSelect1Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (2)
// -----------------------------------------------------------------------------

static MenuItem_t Level2Items_F[] = {
    {ITT_TITLE,   "WEAPONS",                "JHE:TT",                     NULL,                 0}, // ОРУЖЕЕ
    {ITT_SWITCH,  "TIMON'S AXE:",           "NJGJH NBVJYF:",              M_RD_SelectiveWp_0,   0}, // ТОПОР ТИМОНА
    {ITT_SWITCH,  "HAMMER OF RETRIBUTION:", "VJKJN DJPVTPLBZ:",           M_RD_SelectiveWp_1,   0}, // МОЛОТ ВОЗМЕЗДИЯ
    {ITT_SWITCH,  "QUIETUS:",               "GJCKTLYBQ LJDJL:",           M_RD_SelectiveWp_2,   0}, // ПОСЛЕДНИЙ ДОВОД
    {ITT_SWITCH,  "HANDLE OF QUIETUS:",     "HERJZNM GJCKTLYTUJ LJDJLF:", M_RD_SelectiveWp_P_0, 0}, // РУКОЯТЬ ПОСЛЕДНЕГО ДОВОДА
    {ITT_SWITCH,  "GUARD OF QUIETUS:",      "UFHLF GJCKTLYTUJ LJDJLF:",   M_RD_SelectiveWp_P_1, 0}, // ГАРДА ПОСЛЕДНЕГО ДОВОДА
    {ITT_SWITCH,  "BLADE OF QUIETUS:",      "KTPDBT GJCKTLYTUJ LJDJLF:",  M_RD_SelectiveWp_P_2, 0}, // ЛЕЗВИЕ ПОСЛЕДНЕГО ДОВОДА
    {ITT_TITLE,   "MANA",                   "VFYF",                       NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                  "CBYZZ:",                     M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                 "PTK~YFZ:",                   M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",              "FHNTAFRNS",                  NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",          "RDFHWTDSQ AKFRJY:",          M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",            "VBCNBXTCRFZ EHYF:",          M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",             "PTKMT:",                     M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                     NULL,                         NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE >",            "CKTLE.OFZ CNHFYBWF `",      &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",             "YFXFNM BUHE",                G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_F = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_F, false,
    DrawLevelSelect2Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    1
};

static MenuItem_t Level2Items_C[] = {
    {ITT_TITLE,   "WEAPONS",               "JHE:TT",                  NULL,                 0}, // ОРУЖЕЕ
    {ITT_SWITCH,  "SERPENT STAFF:",        "PVTBYSQ GJCJ[:",          M_RD_SelectiveWp_0,   0}, // ЗМЕИНЫЙ ПОСОХ
    {ITT_SWITCH,  "FIRESTORM:",            "JUYTYYSQ INJHV:",         M_RD_SelectiveWp_1,   0}, // ОГНЕННЫЙ ШТОРМ
    {ITT_SWITCH,  "WRAITHVERGE:",          ";TPK LE[JD:",             M_RD_SelectiveWp_2,   0}, // ЖЕЗЛ ДУХОВ
    {ITT_SWITCH,  "POLE OF WRAITHVERGE:",  "LHTDRJ ;TPKF LE[JD:",     M_RD_SelectiveWp_P_0, 0}, // ДРЕВКО ЖЕЗЛА ДУХОВ
    {ITT_SWITCH,  "CROSS OF WRAITHVERGE:", "RHTCNJDBYF ;TPKF LE[JD:", M_RD_SelectiveWp_P_1, 0}, // КРЕСТОВИНА ЖЕЗЛА ДУХОВ
    {ITT_SWITCH,  "HEAD OF WRAITHVERGE:",  "YFDTHITYBT ;TPKF LE[JD:", M_RD_SelectiveWp_P_2, 0}, // НАВЕРШЕНИЕ ЖЕЗЛА ДУХОВ
    {ITT_TITLE,   "MANA",                  "VFYF",                    NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                 "CBYZZ:",                  M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                "PTK~YFZ:",                M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",             "FHNTAFRNS",               NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",         "RDFHWTDSQ AKFRJY:",       M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",           "VBCNBXTCRFZ EHYF:",       M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",            "PTKMT:",                  M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                    NULL,                      NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE >",           "CKTLE.OFZ CNHFYBWF `",   &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",            "YFXFNM BUHE",             G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_C = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_C, false,
    DrawLevelSelect2Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    1
};

static MenuItem_t Level2Items_M[] = {
    {ITT_TITLE,   "WEAPONS",                 "JHE:TT",                      NULL,                 0}, // ОРУЖЕЕ
    {ITT_SWITCH,  "FROST SHARDS:",           "KTLZYST JCRJKRB:",            M_RD_SelectiveWp_0,   0}, // ЛЕДЯНЫЕ ОСКОЛКИ
    {ITT_SWITCH,  "ARC OF DEATH:",           "LEUF CVTHNB:",                M_RD_SelectiveWp_1,   0}, // ДУГА СМЕРТИ
    {ITT_SWITCH,  "BLOODSCOURGE:",           "RHJDFDSQ ,BX:",               M_RD_SelectiveWp_2,   0}, // КРОВАВЫЙ БИЧ
    {ITT_SWITCH,  "BINDER OF BLOODSCOURGE:", "GJLRJDTW RHJDFDJUJ ,BXF:",    M_RD_SelectiveWp_P_0, 0}, // ПОДКОВЕЦ КРОВАВОГО БИЧА
    {ITT_SWITCH,  "SHAFT OF BLOODSCOURGE:",  "LHTDRJ RHJDFDJUJ ,BXF:",      M_RD_SelectiveWp_P_1, 0}, // ДРЕВКО КРОВАВОГО БИЧА
    {ITT_SWITCH,  "KNOB OF BLOODSCOURGE:",   "YF,FKLFIYBR RHJDFDJUJ ,BXF:", M_RD_SelectiveWp_P_2, 0}, // НАБАЛДАШНИК КРОВАВОГО БИЧА
    {ITT_TITLE,   "MANA",                    "VFYF",                        NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                   "CBYZZ:",                      M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                  "PTK~YFZ:",                    M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",               "FHNTAFRNS",                   NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",           "RDFHWTDSQ AKFRJY:",           M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",             "VBCNBXTCRFZ EHYF:",           M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",              "PTKMT:",                      M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                      NULL,                          NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE >",             "CKTLE.OFZ CNHFYBWF `",       &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",              "YFXFNM BUHE",                 G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_M = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_M, false,
    DrawLevelSelect2Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Level select (3)
// -----------------------------------------------------------------------------

static MenuItem_t Level3Items[] = {
    {ITT_LRFUNC,   "DISC OF REPULSION:",    "LBCR JNNJH;TYBZ:",           M_RD_SelectiveArti_3,  0}, // ДИСК ОТТОРЖЕНИЯ
    {ITT_LRFUNC,   "ICON OF THE DEFENDER:", "CBVDJK PFOBNYBRF:",          M_RD_SelectiveArti_4,  0}, // СИМВОЛ ЗАЩИТНИКА
    {ITT_LRFUNC,   "PORKALATOR:",           "CDBYJVJHATH:",               M_RD_SelectiveArti_5,  0}, // СВИНОМОРФЕР
    {ITT_LRFUNC,   "CHAOS DEVICE:",         "'V,KTVF [FJCF:",             M_RD_SelectiveArti_6,  0}, // ЭМБЛЕМА ХАОСА
    {ITT_LRFUNC,   "BANISHMENT DEVICE:",    "'V,KTVF BPUYFYBZ:",          M_RD_SelectiveArti_7,  0}, // ЭМБЛЕМА ИЗГНАНИЯ
    {ITT_LRFUNC,   "WINGS OF WRATH:",       "RHSKMZ UYTDF:",              M_RD_SelectiveArti_8,  0}, // КРЫЛЬЯ ГНЕВА
    {ITT_LRFUNC,   "TORCH:",                "AFRTK:",                     M_RD_SelectiveArti_9,  0}, // ФАКЕЛ
    {ITT_LRFUNC,   "KRATER OF MIGHT:",      "XFIF VJUEOTCNDF:",           M_RD_SelectiveArti_10, 0}, // ЧАША МОГУЩЕСТВА
    {ITT_LRFUNC,   "DRAGONSKIN BRACERS:",   "YFHEXB BP LHFRJYMTQ RJ;B:",  M_RD_SelectiveArti_11, 0}, // НАРУЧИ ИЗ ДРАКОНЬЕЙ КОЖИ
    {ITT_LRFUNC,   "DARK SERVANT:",         "NTVYSQ CKEUF:",              M_RD_SelectiveArti_12, 0}, // ТЕМНЫЙ СЛУГА
    {ITT_LRFUNC,   "BOOTS OF SPEED:",       "CFGJUB-CRJHJ[JLS:",          M_RD_SelectiveArti_13, 0}, // САПОГИ-СКОРОХОДЫ
    {ITT_LRFUNC,   "MYSTIC AMBIT INCANT:",  "XFHS VFUBXTCRJUJ TLBYCNDF:", M_RD_SelectiveArti_14, 0}, // ЧАРЫ МАГИЧЕСКОГО ЕДИНСТВА
    {ITT_TITLE,    "KEYS",                  "RK.XB",                      NULL,                  0}, // КЛЮЧИ
    {ITT_SWITCH,   "EMERALD KEY:",          "BPEVHELYSQ RK.X:",           M_RD_SelectiveKey_0,   0}, // ИЗУМРУДНЫЙ КЛЮЧ
    {ITT_EMPTY,    NULL,                    NULL,                         NULL,                  0},
    {ITT_SETMENU,  "NEXT PAGE >",           "CKTLE.OFZ CNHFYBWF `",      &LevelSelectMenu4,     0}, // СЛЕДУЮЩАЯ СТРАНИЦА >
    {ITT_EFUNC,    "START GAME",            "YFXFNM BUHE",                G_DoSelectiveGame,     0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu3 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level3Items, false,
    DrawLevelSelect3Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (4)
// -----------------------------------------------------------------------------

static MenuItem_t Level4Items[] = {
    {ITT_SWITCH,  "SILVER KEY:",         "CTHT,HZYSQ RK.X:",        M_RD_SelectiveKey_1,     0}, // СЕРЕБРЯНЫЙ КЛЮЧ
    {ITT_SWITCH,  "FIRE KEY:",           "JUYTYYSQ RK.X:",          M_RD_SelectiveKey_2,     0}, // ОГНЕННЫЙ КЛЮЧ
    {ITT_SWITCH,  "STEEL KEY:",          "CNFKMYJQ RK.X:",          M_RD_SelectiveKey_3,     0}, // СТАЛЬНОЙ КЛЮЧ
    {ITT_SWITCH,  "HORN KEY:",           "HJUJDJQ RK.X:",           M_RD_SelectiveKey_4,     0}, // РОГОВОЙ КЛЮЧ
    {ITT_SWITCH,  "CAVE KEY:",           "GTOTHYSQ RK.X:",          M_RD_SelectiveKey_5,     0}, // ПЕЩЕРНЫЙ КЛЮЧ
    {ITT_SWITCH,  "CASTLE KEY:",         "RK.X JN PFVRF:",          M_RD_SelectiveKey_6,     0}, // КЛЮЧ ОТ ЗАМКА
    {ITT_SWITCH,  "SWAMP KEY:",          ",JKJNYSQ RK.X:",          M_RD_SelectiveKey_7,     0}, // БОЛОТНЫЙ КЛЮЧ
    {ITT_SWITCH,  "RUSTED KEY:",         "H;FDSQ RK.X:",            M_RD_SelectiveKey_8,     0}, // РЖАВЫЙ КЛЮЧ
    {ITT_SWITCH,  "DUNGEON KEY:",        "RK.X JN GJLPTVTKMZ:",     M_RD_SelectiveKey_9,     0}, // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
    {ITT_SWITCH,  "AXE KEY:",            "RK.X-NJGJH:",             M_RD_SelectiveKey_10,    0}, // КЛЮЧ-ТОПОР
    {ITT_TITLE,   "PUZZLE ITEMS",        "GFPKJDST GHTLVTNS",       NULL,                    0}, // ПАЗЛОВЫЕ ПЕРЕДМЕТЫ
    {ITT_LRFUNC,  "FLAME MASK:",         "VFCRF GKFVTYB:",          M_RD_SelectivePuzzle_0,  0}, // МАСКА ПЛАМЕНИ
    {ITT_LRFUNC,  "HEART OF D'SPARIL:",  "CTHLWT L&CGFHBKF:",       M_RD_SelectivePuzzle_1,  0}, // СЕРДЦЕ Д'СПАРИЛА
    {ITT_LRFUNC,  "RUBY PLANET:",        "HE,BYJDFZ GKFYTNF:",      M_RD_SelectivePuzzle_2,  0}, // РУБИНОВАЯ ПЛАНЕТА
    {ITT_EMPTY,   NULL,                  NULL,                      NULL,                    0},
    {ITT_SETMENU, "LAST PAGE >",         "GJCKTLYZZ CNHFYBWF `",   &LevelSelectMenu5,       0}, // ПОСЛЕДНЯЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",          "YFXFNM BUHE",             G_DoSelectiveGame,       0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu4 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level4Items, false,
    DrawLevelSelect4Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (5)
// -----------------------------------------------------------------------------

static MenuItem_t Level5Items[] = {
    {ITT_LRFUNC,  "EMERALD PLANET 1:",   "BPEVHELYFZ GKFYTNF 1:",   M_RD_SelectivePuzzle_3,  0}, // ИЗУМРУДНАЯ ПЛАНЕТА 1
    {ITT_LRFUNC,  "EMERALD PLANET 2:",   "BPEVHELYFZ GKFYTNF 2:",   M_RD_SelectivePuzzle_4,  0}, // ИЗУМРУДНАЯ ПЛАНЕТА 2
    {ITT_LRFUNC,  "SAPPHIRE PLANET 1:",  "CFGABHJDFZ GKFYTNF 1:",   M_RD_SelectivePuzzle_5,  0}, // САПФИРОВАЯ ПЛАНЕТА 1
    {ITT_LRFUNC,  "SAPPHIRE PLANET 2:",  "CFGABHJDFZ GKFYTNF 2:",   M_RD_SelectivePuzzle_6,  0}, // САПФИРОВАЯ ПЛАНЕТА 2
    {ITT_LRFUNC,  "CLOCK GEAR (S):",     "XFCJDFZ ITCNTHYZ (C):",   M_RD_SelectivePuzzle_7,  0}, // ЧАСОВАЯ ШЕСТЕРНЯ (Б&С)
    {ITT_LRFUNC,  "CLOCK GEAR (B):",     "XFCJDFZ ITCNTHYZ (,):",   M_RD_SelectivePuzzle_8,  0}, // ЧАСОВАЯ ШЕСТЕРНЯ (Б)
    {ITT_LRFUNC,  "CLOCK GEAR (S B):",   "XFCJDFZ ITCNTHYZ (C ,):", M_RD_SelectivePuzzle_9,  0}, // ЧАСОВАЯ ШЕСТЕРНЯ (С&Б)
    {ITT_LRFUNC,  "CLOCK GEAR (B S):",   "XFCJDFZ ITCNTHYZ (, C):", M_RD_SelectivePuzzle_10, 0}, // ЧАСОВАЯ ШЕСТЕРНЯ (С)
    {ITT_LRFUNC,  "DAEMON CODEX:",       "RJLTRC LTVJYF:",          M_RD_SelectivePuzzle_11, 0}, // КОДЕКС ДЕМОНА
    {ITT_LRFUNC,  "LIBER OSCURA:",       NULL, /* eng text */       M_RD_SelectivePuzzle_12, 0}, // LIBER OSCURA
    {ITT_LRFUNC,  "YORICK'S SKULL:",     "XTHTG QJHBRF:",           M_RD_SelectivePuzzle_13, 0}, // ЧЕРЕП ЙОРИКА
    {ITT_LRFUNC,  "GLAIVE SEAL:",        "GTXFNM DJBNTKZ:",         M_RD_SelectivePuzzle_14, 0}, // ПЕЧАТЬ ВОИТЕЛЯ
    {ITT_LRFUNC,  "HOlY RELIC:",         "CDZNFZ HTKBRDBZ:",        M_RD_SelectivePuzzle_15, 0}, // СВЯТАЯ РЕЛИКВИЯ
    {ITT_LRFUNC,  "SIGIL OF THE MAGUS:", "CBVDJK VFUF:",            M_RD_SelectivePuzzle_16, 0}, // СИМВОЛ МАГА
    {ITT_EMPTY,   NULL,                  NULL,                      NULL,                    0},
    {ITT_SETMENU, "FIRST PAGE >",        "GTHDFZ CNHFYBWF `",      &LevelSelectMenu1,       0}, // ПЕРВАЯ СТРАНИЦА >
    {ITT_EFUNC,   "START GAME",          "YFXFNM BUHE",             G_DoSelectiveGame,       0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu5 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level5Items, false,
    DrawLevelSelect5Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    0
};

static MenuItem_t VanillaOptionsItems[] = {
    {ITT_EFUNC,   "END GAME",          "PFRJYXBNM BUHE",   SCEndGame,             0}, // ЗАКОНЧИТЬ ИГРУ
    {ITT_LRFUNC,  "MESSAGES : ",        "CJJ,OTYBZ : ",    M_RD_Messages,         0}, // СООБЩЕНИЯ:
    {ITT_LRFUNC,  "MOUSE SENSITIVITY", "CRJHJCNM VSIB",    M_RD_Sensitivity,      0}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,   NULL,                NULL,               NULL,                  0},
    {ITT_SETMENU, "MORE...",           "LJGJKYBNTKMYJ>>>", &VanillaOptions2Menu,  0}  // ДОПОЛНИТЕЛЬНО...
};

static Menu_t VanillaOptionsMenu = {
    88, 88,
    30,
    NULL, NULL, true,
    5, VanillaOptionsItems, true,
    DrawOptionsMenu_Vanilla,
    NULL,
    &HMainMenu,
    0,
};

static MenuItem_t VanillaOptions2Items[] = {
    {ITT_LRFUNC, "SCREEN SIZE",  "HFPVTH 'RHFYF",    M_RD_ScreenSize, 0}, // РАЗМЕР ЭКРАНА
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "SFX VOLUME",   "UHJVRJCNM PDERF",  M_RD_SfxVolume,  0}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "MUSIC VOLUME", "UHJVRJCNM VEPSRB", M_RD_MusVolume,  0}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0}
};

static Menu_t VanillaOptions2Menu = {
    90, 90,
    20,
    NULL, NULL, true,
    6, VanillaOptions2Items, true,
    DrawOptions2Menu_Vanilla,
    NULL,
    &VanillaOptionsMenu,
    0
};

static MenuItem_t FilesItems[] = {
    {ITT_SETMENU_NONET, "LOAD GAME", "PFUHEPBNM BUHE", &LoadMenu, 2}, // ЗАГРУЗИТЬ ИГРУ
    {ITT_SETMENU,       "SAVE GAME", "CJ[HFYBNM BUHE", &SaveMenu, 0}  // СОХРАНИТЬ ИГРУ
};

static Menu_t FilesMenu = {
    110, 110,
    60,
    NULL, NULL, true,
    2, FilesItems, true,
    DrawFilesMenu,
    NULL,
    &HMainMenu,
    0
};

static MenuItem_t LoadItems[] = {
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 0},
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 1},
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 2},
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 3},
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 4},
    {ITT_EFUNC, NULL, NULL, SCLoadGame, 5}
};

static Menu_t LoadMenu = {
    70, 70,
    30,
    "LOAD GAME", "PFUHEPBNM BUHE", true, // ЗАГРУЗИТЬ ИГРУ
    6, LoadItems, true,
    DrawSaveLoadMenu,
    NULL,
    &FilesMenu,
    0
};

static MenuItem_t SaveItems[] = {
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 0},
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 1},
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 2},
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 3},
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 4},
    {ITT_EFUNC, NULL, NULL, SCSaveGame, 5}
};

static Menu_t SaveMenu = {
    70, 70,
    30,
    "SAVE GAME", "CJ[HFYBNM BUHE", true, // СОХРАНИТЬ ИГРУ
    6, SaveItems, true,
    DrawSaveLoadMenu,
    NULL,
    &FilesMenu,
    0
};

static char *GammaText[] = {
	TXT_GAMMA_0_50,
	TXT_GAMMA_0_55,
	TXT_GAMMA_0_60,
	TXT_GAMMA_0_65,
	TXT_GAMMA_0_70,
	TXT_GAMMA_0_75,
	TXT_GAMMA_0_80,
	TXT_GAMMA_0_85,
	TXT_GAMMA_0_90,
	TXT_GAMMA_1_0,
	TXT_GAMMA_1_125,
	TXT_GAMMA_1_25,
	TXT_GAMMA_1_375,
	TXT_GAMMA_1_5,
	TXT_GAMMA_1_625,
	TXT_GAMMA_1_75,
	TXT_GAMMA_1_875,
	TXT_GAMMA_2_0
};

static char *GammaText_Rus[] = {
	TXT_GAMMA_RUS_0_50,
	TXT_GAMMA_RUS_0_55,
	TXT_GAMMA_RUS_0_60,
	TXT_GAMMA_RUS_0_65,
	TXT_GAMMA_RUS_0_70,
	TXT_GAMMA_RUS_0_75,
	TXT_GAMMA_RUS_0_80,
	TXT_GAMMA_RUS_0_85,
	TXT_GAMMA_RUS_0_90,
	TXT_GAMMA_RUS_1_0,
	TXT_GAMMA_RUS_1_125,
	TXT_GAMMA_RUS_1_25,
	TXT_GAMMA_RUS_1_375,
	TXT_GAMMA_RUS_1_5,
	TXT_GAMMA_RUS_1_625,
	TXT_GAMMA_RUS_1_75,
	TXT_GAMMA_RUS_1_875,
	TXT_GAMMA_RUS_2_0
};

// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    FontAYellowBaseLump = W_GetNumForName("FONTAY_S") + 1; // eng small yellow
    FontFYellowBaseLump = W_GetNumForName("FONTFY_S") + 1; // rus small yellow
    RD_M_InitFonts(// [JN] Original English fonts
                   "FONTA_S",
                   "FONTB_S",
                   // [JN] Small special font used for time/fps widget
                   "FONTC_S",
                   // [JN] Unchangable English fonts
                   "FONTD_S",
                   "FONTE_S",
                   // [JN] Unchangable Russian fonts
                   "FONTF_S",
                   "FONTG_S");

    RD_Menu_InitMenu(20, 10, OnActivateMenu, OnDeactivateMenu);

    RD_Menu_InitSliders(// [Dasperal] Big slider
                        "M_SLDLT",
                        "M_SLDMD1",
                        "M_SLDMD2",
                        "M_SLDRT",
                        "M_SLDKB",
                        // [Dasperal] Small slider
                        "M_RDSLDL",
                        "M_RDSLD1",
                        "M_RDSLDR",
                        "M_RDSLG",
                        // [Dasperal] Gem translation
                        CR_NONE,
                        CR_GREEN2GRAY_HEXEN,
                        CR_GREEN2RED_HEXEN);

    RD_Menu_InitCursor(// [Dasperal] Big cursor
                        "M_SLCTR1",
                        "M_SLCTR2",
                        // [Dasperal] Small cursor
                        "M_RDGEM1",
                        "M_RDGEM2",
                        -1, -1, -28, -14);

    if(vanillaparm)
        OptionsMenu = &VanillaOptionsMenu;
    else
        OptionsMenu = &RDOptionsMenu;

    HMainItems[1].pointer = OptionsMenu;
    MainMenu = &HMainMenu;

    CurrentMenu = MainMenu;
    CurrentItPos = CurrentMenu->lastOn;

    menuactive = false;
//      messageson = true;              // Set by defaults in .CFG
    MauloBaseLump = W_GetNumForName("FBULA0");  // ("M_SKL00");

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;
}

//==========================================================================
//
// MN_DrTextAYellow and MN_DrTextAYellowFade
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

void MN_DrTextAYellowFade(char *text, int x, int y, byte *table)
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
            V_DrawFadePatch(x, y, p, table);
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
            p = W_CacheLumpNum(FontFYellowBaseLump + c - 33, PU_CACHE);
            V_DrawShadowedPatchRaven(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

void MN_DrTextSmallYellowRUSFade(char *text, int x, int y, byte *table)
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
            p = W_CacheLumpNum(FontFYellowBaseLump + c - 33, PU_CACHE);
            V_DrawFadePatch(x, y, p, table);
            x += SHORT(p->width) - 1;
        }
    }
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
    if (menuactive == false)
    {
        if (askforquit)
        {
            if (english_language)
            {
                RD_M_DrawTextA(QuitEndMsg[typeofask - 1], 160 -
                           RD_M_TextAWidth(QuitEndMsg[typeofask - 1]) / 2
                           + wide_delta, 80);
            }
            else
            {
                RD_M_DrawTextSmallRUS(QuitEndMsg_Rus[typeofask - 1], 160 -
                           RD_M_TextSmallRUSWidth(QuitEndMsg_Rus[typeofask - 1]) / 2
                           + wide_delta, 80, CR_NONE);
            }

            if (typeofask == 3)
            {
                RD_M_DrawTextA(SlotText[quicksave - 1], 160 -
                           RD_M_TextAWidth(SlotText[quicksave - 1]) / 2
                           + wide_delta, 90);
                RD_M_DrawTextA("?", 160 +
                           RD_M_TextAWidth(SlotText[quicksave - 1]) / 2
                           + wide_delta, 90);
            }
            if (typeofask == 4)
            {
                RD_M_DrawTextA(SlotText[quickload - 1], 160 -
                           RD_M_TextAWidth(SlotText[quickload - 1]) / 2
                           + wide_delta, 90);
                RD_M_DrawTextA("?", 160 +
                           RD_M_TextAWidth(SlotText[quicksave - 1]) / 2
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
        RD_Menu_DrawMenu(CurrentMenu, MenuTime, CurrentItPos);
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
        RD_M_DrawTextB("CHOOSE CLASS:", 34 + wide_delta, 24);
    }
    else
    {
        RD_M_DrawTextBigRUS("DS,THBNT RKFCC:", 5 + wide_delta, 24);   // ВЫБЕРИТЕ КЛАСС:
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
    // [JN] Update Status bar.
    // [Dasperal] Once this line is deleted, Delete DrawSkillMenu function and
    // replace it's reference to NULL
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
// PROC DrawSaveLoadMenu
//
//---------------------------------------------------------------------------

static void DrawSaveLoadMenu(void)
{
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots();
}

static boolean ReadDescriptionForSlot(int slot, char *description)
{
    FILE *fp;
    boolean found;
    char name[RD_MAX_PATH];
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

static void DrawFileSlots()
{
    int i;
    int x;
    int y;

    x = 70; // [Dasperal] SaveMenu and LoadMenu have the same x and the same y
    y = 30; // so inline them here to eliminate the Menu_t* argument
    for (i = 0; i < 6; i++)
    {
        V_DrawShadowedPatchRaven(x + wide_delta, y, W_CacheLumpName("M_FSLOT", PU_CACHE));
        if (SlotStatus[i])
        {
            RD_M_DrawTextA(SlotText[i], x + 5 + wide_delta, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}

//---------------------------------------------------------------------------
// DrawOptionsMenu
//---------------------------------------------------------------------------

static void DrawOptionsMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);
}

// -----------------------------------------------------------------------------
// DrawRenderingMenu
// -----------------------------------------------------------------------------

static void DrawRenderingMenu(void)
{
	static char num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Display aspect ratio
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 185 + wide_delta, 42, CR_NONE);

        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            RD_M_DrawTextSmallENG("THE PROGRAM MUST BE RESTARTED", 51 + wide_delta, 135, CR_GRAY2RED_HEXEN);
        }

        // Vertical sync
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallENG("N/A", 216 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else
        {
            RD_M_DrawTextSmallENG(vsync ? "ON" : "OFF", 216 + wide_delta, 52, CR_NONE);
        }

        // FPS limit
        RD_Menu_DrawSliderSmallInline(100, 62, 11, (max_fps-40) / 20);
        // Numerical representation of slider position
        M_snprintf(num, 4, "%d", max_fps);
        RD_M_DrawTextSmallENG(num, 208 + wide_delta, 63, 
                              max_fps < 60 ? CR_GRAY2GDARKGRAY_HEXEN :
                              max_fps < 100 ? CR_NONE :
                              max_fps < 260 ? CR_GRAY2GREEN_HEXEN : 
							  max_fps < 999 ? CR_GRAY2DARKGOLD_HEXEN : CR_GRAY2RED_HEXEN);

        // FPS counter
        RD_M_DrawTextSmallENG(show_fps ? "ON" : "OFF", 129 + wide_delta, 72, CR_NONE);

        // Pixel scaling
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallENG("N/A", 131 + wide_delta, 82, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else
        {
            RD_M_DrawTextSmallENG(smoothing ? "SMOOTH" : "SHARP", 131 + wide_delta, 82, CR_NONE);
        }

        // Porch palette changing
        RD_M_DrawTextSmallENG(vga_porch_flash ? "ON" : "OFF", 205 + wide_delta, 92, CR_NONE);

        // Tip for faster sliding
        if (CurrentItPos == 3)
        {
            RD_M_DrawTextSmallENG("HOLD RUN BUTTON FOR FASTER SLIDING",
                                  39 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
        }
    }
    else
    {
        // Соотношение сторон экрана
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 230 + wide_delta, 42, CR_NONE);

        // Informative message: НЕОБХОДИМ ПЕРЕЗАПУСК ИГРЫ
        if (aspect_ratio_temp != aspect_ratio)
        {
            RD_M_DrawTextSmallRUS("YTJ,[JLBV GTHTPFGECR GHJUHFVVS", 46 + wide_delta, 135, CR_GRAY2RED_HEXEN);
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallRUS("Y/L", 236 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else
        {
            RD_M_DrawTextSmallRUS(vsync ? "DRK" : "DSRK", 236 + wide_delta, 52, CR_NONE);
        }

        // Ограничение FPS
        RD_M_DrawTextSmallENG("FPS:", 123 + wide_delta, 62, CR_NONE);
        RD_Menu_DrawSliderSmallInline(154, 62, 11, (max_fps-40) / 20);
        // Numerical representation of slider position
        M_snprintf(num, 4, "%d", max_fps);
        RD_M_DrawTextSmallENG(num, 262 + wide_delta, 63, 
                              max_fps < 60 ? CR_GRAY2GDARKGRAY_HEXEN :
                              max_fps < 100 ? CR_NONE :
                              max_fps < 260 ? CR_GRAY2GREEN_HEXEN : 
							  max_fps < 999 ? CR_GRAY2DARKGOLD_HEXEN : CR_GRAY2RED_HEXEN);

        // Счетчик кадровой частоты
        RD_M_DrawTextSmallRUS(show_fps ? "DRK" : "DSRK", 223 + wide_delta, 72, CR_NONE);

        // Пиксельное сглаживание
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallRUS("Y/L", 211 + wide_delta, 82, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else
        {
            RD_M_DrawTextSmallRUS(smoothing ? "DRK" : "DSRK", 211 + wide_delta, 82, CR_NONE);
        }

        // Изменение палитры краев экрана
        RD_M_DrawTextSmallRUS(vga_porch_flash ? "DRK" : "DSRK", 265 + wide_delta, 92, CR_NONE);

        // Для ускоренного пролистывания
        // удерживайте кнопку бега
        if (CurrentItPos == 3)
        {
            RD_M_DrawTextSmallRUS("LKZ ECRJHTYYJUJ GHJKBCNSDFYBZ",
                                  51 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
            RD_M_DrawTextSmallRUS("ELTH;BDFQNT RYJGRE ,TUF",
                                  73 + wide_delta, 172, CR_GRAY2GDARKGRAY_HEXEN);
        }
    }

    // Screenshot format / Формат скриншотов (same english values)
    RD_M_DrawTextSmallENG(png_screenshots ? "PNG" : "PCX", 175 + wide_delta, 112, CR_NONE);
}

static void M_RD_Change_Widescreen(Direction_t direction)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in MN_Init and stored into config file in M_QuitResponse.
    RD_Menu_SpinInt(&aspect_ratio_temp, 0, 4, direction);
}

static void M_RD_Change_VSync()
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
        return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

static void M_RD_MaxFPS(Direction_t direction)
{
    // [JN] Speed up slider movement while holding "run" key.
    switch (direction)
    {
        case LEFT_DIR:
            max_fps -= BK_isKeyPressed(bk_speed) ? 10 : 1;
            if (max_fps >= 35)
            {
                S_StartSound (NULL, SFX_PICKUP_KEY);
            }
        break;
        case RIGHT_DIR:
            max_fps += BK_isKeyPressed(bk_speed) ? 10 : 1;
            if (max_fps <= 999)
            {
                S_StartSound (NULL, SFX_PICKUP_KEY);
            }
        break;
    }

    // Prevent overflows / incorrect values.
    if (max_fps < 35)
    {
        max_fps = 35;
    }
    if (max_fps > 999)
    {
        max_fps = 999;
    }

    // Toggle internal variable for frame interpolation.
    if (max_fps == 35)
    {
        uncapped_fps = 0;
    }
    else
    {
        uncapped_fps = 1;
    }
}

static void M_RD_FPScounter()
{
    show_fps ^= 1;
}

static void M_RD_Smoothing()
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

static void M_RD_PorchFlashing()
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
}

static void M_RD_Screenshots()
{
    png_screenshots ^= 1;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    static char num[4];

    // Draw menu background. Hide it for a moment while changing 
    // screen size, gamma and level brightness in GS_LEVEL game state.
    if (gamestate != GS_LEVEL || (gamestate == GS_LEVEL && menubgwait < I_GetTime()))
	{
		V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);
	}

    if (english_language)
    {
        // Graphics detail
        RD_M_DrawTextSmallENG(detailLevel ? "LOW" : "HIGH", 149 + wide_delta, 82, CR_NONE);
    }
    else
    {
        // Детализация графики
        RD_M_DrawTextSmallRUS(detailLevel ? "YBPRFZ" : "DSCJRFZ", 188 + wide_delta, 82, CR_NONE);
    }

    // Screen size
    if (aspect_ratio >= 2)
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 4, screenblocks - 9);
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 85 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
    }
    else
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 10, screenblocks - 3);
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 135 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
    }

    // Level brightness
    RD_Menu_DrawSliderSmall(&DisplayMenu, 72, 5, level_brightness / 16);
}

static void M_RD_ScreenSize(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt(&screenblocks, 3, 12, direction);

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

void M_RD_Brightness(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideFloat_Step(&brightness, 0.01F, 1.0F, 0.01F, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

static void M_RD_Gamma(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideInt(&usegamma, 0, 17, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

void M_RD_Saturation(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideFloat_Step(&color_saturation, 0.01F, 1.0F, 0.01F, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

void M_RD_ShowPalette()
{
    show_palette ^= 1;
}

void M_RD_RED_Color(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideFloat_Step(&r_color_factor, 0.01F, 1.0F, 0.01F, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

void M_RD_GREEN_Color(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideFloat_Step(&g_color_factor, 0.01F, 1.0F, 0.01F, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

void M_RD_BLUE_Color(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 100;

    RD_Menu_SlideFloat_Step(&b_color_factor, 0.01F, 1.0F, 0.01F, direction);

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

// -----------------------------------------------------------------------------
// DrawColorMenu
// -----------------------------------------------------------------------------

static void DrawColorMenu(void)
{
    int  i;
    char num[8];
    char *num_and_percent;
    // [JN] Hack to allow proper placement for gamma slider.
    int gamma_slider = usegamma == 0 ? 0 :
                       usegamma == 17 ? 2 : 1;

    // Draw menu background. Hide it for a moment while changing 
    // screen size, gamma and level brightness in GS_LEVEL game state.
    if (gamestate != GS_LEVEL || (gamestate == GS_LEVEL && menubgwait < I_GetTime()))
	{
        V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);
	}

    if (english_language)
    {
        RD_M_DrawTextSmallENG("Brightness", 70 + wide_delta, 25, CR_NONE);
        RD_M_DrawTextSmallENG("Gamma", 105 + wide_delta, 35, CR_NONE);
        RD_M_DrawTextSmallENG("Saturation", 71 + wide_delta, 45, CR_NONE);
        RD_M_DrawTextSmallENG("Show palette", 57 + wide_delta, 55, CR_NONE);
        RD_M_DrawTextSmallENG(show_palette ? "ON" : "OFF", 165 + wide_delta, 55, CR_NONE);

        RD_M_DrawTextSmallENG("COLOR INTENSITY",
                              107 + wide_delta, 65, CR_GRAY2DARKGOLD_HEXEN);

        RD_M_DrawTextSmallENG("RED", 121 + wide_delta, 75, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallENG("GREEN", 105 + wide_delta, 85, CR_GRAY2GREEN_HEXEN);
        RD_M_DrawTextSmallENG("BLUE", 116 + wide_delta, 95, CR_GRAY2BLUE_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallRUS("zhrjcnm", 93 + wide_delta, 25, CR_NONE);       // Яркость
        RD_M_DrawTextSmallRUS("ufvvf", 106 + wide_delta, 35, CR_NONE);        // Гамма
        RD_M_DrawTextSmallRUS("yfcsotyyjcnm", 50 + wide_delta, 45, CR_NONE);  // Насыщенность
        RD_M_DrawTextSmallRUS("wdtnjdfz gfkbnhf", 25 + wide_delta, 55, CR_NONE);  // Цветовая палитра
        RD_M_DrawTextSmallRUS(show_palette ? "DRK" : "DSRK", 165 + wide_delta, 55, CR_NONE);

        RD_M_DrawTextSmallRUS("byntycbdyjcnm wdtnf",  // Интенсивность цвета
                              89 + wide_delta, 65, CR_GRAY2DARKGOLD_HEXEN);

        RD_M_DrawTextSmallRUS("rhfcysq", 90 + wide_delta, 75, CR_GRAY2RED_HEXEN);   // Красный
        RD_M_DrawTextSmallRUS("ptktysq", 90 + wide_delta, 85, CR_GRAY2GREEN_HEXEN); // Зелёный
        RD_M_DrawTextSmallRUS("cbybq", 109 + wide_delta, 95, CR_GRAY2BLUE_HEXEN);   // Синий
    }

    // Brightness slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 25, 10, brightness * 10);
    i = brightness * 100;                            // Do a float to int conversion for slider value.
    M_snprintf(num, 5, "%d", i);                     // Numerical representation of slider position.
    num_and_percent = M_StringJoin(num, "%", NULL);  // Consolidate numerical value and % sign.
    RD_M_DrawTextSmallENG(num_and_percent, 264 + wide_delta, 26, CR_NONE);

    // Gamma-correction slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 35, 10, usegamma / 2 + gamma_slider);
    M_snprintf(num, 6, "%s", gammalevel_names[usegamma]);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 36, CR_NONE);

    // Saturation slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 45, 10, color_saturation * 10);
    i = color_saturation * 100;                      // Do a float to int conversion for slider value.
    M_snprintf(num, 5, "%d", i);                     // Numerical representation of slider position.
    num_and_percent = M_StringJoin(num, "%", NULL);  // Consolidate numerical value and % sign.
    RD_M_DrawTextSmallENG(num_and_percent, 264 + wide_delta, 46, CR_NONE);

    // RED intensity slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 75, 10, r_color_factor * 10);
    M_snprintf(num, 5, "%3f", r_color_factor);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 76, CR_GRAY2RED_HEXEN);

    // GREEN intensity slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 85, 10, g_color_factor * 10);
    M_snprintf(num, 5, "%3f", g_color_factor);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 86, CR_GRAY2GREEN_HEXEN);

    // BLUE intensity slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 95, 10, b_color_factor * 10);
    M_snprintf(num, 5, "%3f", b_color_factor);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 96, CR_GRAY2BLUE_HEXEN);

    if (show_palette)
    {
        V_DrawPatchUnscaled(wide_delta*2, 200, W_CacheLumpName(("M_COLORS"), PU_CACHE), NULL);
    }
}

static void M_RD_LevelBrightness(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt_Step(&level_brightness, 0, 64, 16, direction);
}

static void M_RD_Detail()
{
    detailLevel ^= 1;

    R_SetViewSize (screenblocks, detailLevel);
}

// -----------------------------------------------------------------------------
// DrawMessagesMenu
// -----------------------------------------------------------------------------

static void DrawMessagesMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Messages
        RD_M_DrawTextSmallENG(show_messages ? "ON" : "OFF", 108 + wide_delta, 42, CR_NONE);

        // Messages alignment
        RD_M_DrawTextSmallENG(messages_alignment == 1 ? "LEFT EDGE OF THE SCREEN" :
                              messages_alignment == 2 ? "LEFT EDGE OF THE STATUS BAR" :
                                                        "CENTERED", 108 + wide_delta, 52, CR_NONE);

        // Messages timeout (text)
        RD_M_DrawTextSmallENG(messages_timeout == 1 ? "1 SECOND" :
                              messages_timeout == 2 ? "2 SECONDS" :
                              messages_timeout == 3 ? "3 SECONDS" :
                              messages_timeout == 4 ? "4 SECONDS" :
                              messages_timeout == 5 ? "5 SECONDS" :
                              messages_timeout == 6 ? "6 SECONDS" :
                              messages_timeout == 7 ? "7 SECONDS" :
                              messages_timeout == 8 ? "8 SECONDS" :
                              messages_timeout == 9 ? "9 SECONDS" :
                                                      "10 SECONDS", 
                              136 + wide_delta, 72, CR_GRAY2GDARKGRAY_HEXEN);

        // Fading effect 
        RD_M_DrawTextSmallENG(message_fade ? "ON" : "OFF", 140 + wide_delta, 82, CR_NONE);

        // Text casts shadows
        RD_M_DrawTextSmallENG(draw_shadowed_text ? "ON" : "OFF", 179 + wide_delta, 92, CR_NONE);

        // Local time
        RD_M_DrawTextSmallENG(local_time == 1 ? "12-HOUR (HH:MM)" :
                              local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                              local_time == 3 ? "24-HOUR (HH:MM)" :
                              local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF",
                              110 + wide_delta, 112, CR_NONE);
    }
    else
    {
        // Отображение сообщений
        RD_M_DrawTextSmallRUS(show_messages ? "DRK" : "DSRK", 208 + wide_delta, 42, CR_NONE);

        // Выравнивание сообщений
        RD_M_DrawTextSmallRUS(messages_alignment == 1 ? "GJ RHF. \'RHFYF" :
                              messages_alignment == 2 ? "GJ RHF. CNFNEC-,FHF" :
                                                        "GJ WTYNHE", 138 + wide_delta, 52, CR_NONE);

        // Таймаут отображения (текст)
        RD_M_DrawTextSmallRUS(messages_timeout == 1 ? "1 CTREYLF" :
                              messages_timeout == 2 ? "2 CTREYLS" :
                              messages_timeout == 3 ? "3 CTREYLS" :
                              messages_timeout == 4 ? "4 CTREYLS" :
                              messages_timeout == 5 ? "5 CTREYL" :
                              messages_timeout == 6 ? "6 CTREYL" :
                              messages_timeout == 7 ? "7 CTREYL" :
                              messages_timeout == 8 ? "8 CTREYL" :
                              messages_timeout == 9 ? "9 CTREYL" :
                                                      "10 CTREYL",
                              136 + wide_delta, 72, CR_GRAY2GDARKGRAY_HEXEN);

        // Плавное исчезновение
        RD_M_DrawTextSmallRUS(message_fade ? "DRK" : "DSRK", 193 + wide_delta, 82, CR_NONE);

        // Тексты отбрасывают тень
        RD_M_DrawTextSmallRUS(draw_shadowed_text ? "DRK" : "DSRK", 220 + wide_delta, 92, CR_NONE);

        // Системное время
        RD_M_DrawTextSmallRUS(local_time == 1 ? "12-XFCJDJT (XX:VV)" :
                              local_time == 2 ? "12-XFCJDJT (XX:VV:CC)" :
                              local_time == 3 ? "24-XFCJDJT (XX:VV)" :
                              local_time == 4 ? "24-XFCJDJT (XX:VV:CC)" : "DSRK",
                              157 + wide_delta, 112, CR_NONE);
    }

    // Messages timeout (slider)
    RD_Menu_DrawSliderSmall(&MessagesMenu, 72, 10, messages_timeout - 1);
}

static void M_RD_Messages(Direction_t direction)
{
    show_messages ^= 1;
    if (show_messages)
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
    if (vanillaparm)
    {
        S_StartSound(NULL, SFX_CHAT);
    }
}

static void M_RD_MessagesAlignment(Direction_t direction)
{
    RD_Menu_SpinInt(&messages_alignment, 0, 2, direction);
}

static void M_RD_MessagesTimeout(Direction_t direction)
{
    RD_Menu_SlideInt(&messages_timeout, 1, 10, direction);
}

static void M_RD_MessagesFade()
{
    message_fade ^= 1;
}

static void M_RD_ShadowedText()
{
    draw_shadowed_text ^= 1;
}

static void M_RD_LocalTime(Direction_t direction)
{
    RD_Menu_SpinInt(&local_time, 0, 4, direction);
}

// -----------------------------------------------------------------------------
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Overlay mode
        RD_M_DrawTextSmallENG(automap_overlay ? "ON" : "OFF", 200 + wide_delta, 32, CR_NONE);

        // Rotate mode
        RD_M_DrawTextSmallENG(automap_rotate ? "ON" : "OFF", 193 + wide_delta, 42, CR_NONE);

        // Follow mode
        RD_M_DrawTextSmallENG(automap_follow ? "ON" : "OFF", 189 + wide_delta, 52, CR_NONE);

        // Grid
        RD_M_DrawTextSmallENG(automap_grid ? "ON" : "OFF", 138 + wide_delta, 62, CR_NONE);
    }
    else
    {
        // Режим наложения
        RD_M_DrawTextSmallRUS(automap_overlay ? "DRK" : "DSRK", 208 + wide_delta, 32, CR_NONE);

        // Режим вращения
        RD_M_DrawTextSmallRUS(automap_rotate ? "DRK" : "DSRK", 200 + wide_delta, 42, CR_NONE);

        // Режим следования
        RD_M_DrawTextSmallRUS(automap_follow ? "DRK" : "DSRK", 215 + wide_delta, 52, CR_NONE);

        // Сетка
        RD_M_DrawTextSmallRUS(automap_grid ? "DRK" : "DSRK", 128 + wide_delta, 62, CR_NONE);
    }
}

static void M_RD_AutoMapOverlay()
{
    automap_overlay ^= 1;
}

static void M_RD_AutoMapRotate()
{
    automap_rotate ^= 1;
}

static void M_RD_AutoMapFollow()
{
    automap_follow ^= 1;
}

static void M_RD_AutoMapGrid()
{
    automap_grid ^= 1;
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    // SFX Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 52, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%3d", snd_MaxVolume_tmp);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 53, CR_GRAY2GDARKGRAY_HEXEN);

    // Music Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 72, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 73, CR_GRAY2GDARKGRAY_HEXEN);

    // SFX Channels
    RD_Menu_DrawSliderSmall(&SoundMenu, 102, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%3d", snd_Channels);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 103, CR_GRAY2GDARKGRAY_HEXEN);
}

static void M_RD_SfxVolume(Direction_t direction)
{
    RD_Menu_SlideInt(&snd_MaxVolume, 0, 15, direction);
    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable.
    soundchanged = true;        // we'll set it when we leave the menu
}

static void M_RD_MusVolume(Direction_t direction)
{
    RD_Menu_SlideInt(&snd_MusicVolume, 0, 15, direction);
    S_SetMusicVolume();
}

static void M_RD_SfxChannels(Direction_t direction)
{
    RD_Menu_SlideInt_Step(&snd_Channels, 4, 64, 4, direction);

    // Reallocate sound channels
    S_ChannelsRealloc();
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundSystemMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Sound effects
        if (snd_sfxdevice == 0)
        {
            RD_M_DrawTextSmallENG("DISABLED", 144 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else if (snd_sfxdevice == 3)
        {
            RD_M_DrawTextSmallENG("DIGITAL SFX", 144 + wide_delta, 42, CR_NONE);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            RD_M_DrawTextSmallENG("DISABLED", 80 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            RD_M_DrawTextSmallENG("OPL2 SYNTH", 80 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            RD_M_DrawTextSmallENG("OPL3 SYNTH", 80 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 5)
        {
            RD_M_DrawTextSmallENG("GUS EMULATION", 80 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC/TRACKER", 80 + wide_delta, 52, CR_NONE);
        }

        // Sampling frequency (hz)
        if (snd_samplerate == 44100)
        {
            RD_M_DrawTextSmallENG("44100 HZ", 178 + wide_delta, 72, CR_NONE);
        }
        else if (snd_samplerate == 22050)
        {
            RD_M_DrawTextSmallENG("22050 HZ", 178 + wide_delta, 72, CR_NONE);
        }
        else if (snd_samplerate == 11025)
        {
            RD_M_DrawTextSmallENG("11025 HZ", 178 + wide_delta, 72, CR_NONE);
        }

        // SFX Mode
        RD_M_DrawTextSmallENG(snd_monomode ? "MONO" : "STEREO", 181 + wide_delta, 92, CR_NONE);

        // Pitch-Shifted sounds
        RD_M_DrawTextSmallENG(snd_pitchshift ? "ON" : "OFF", 189 + wide_delta, 102, CR_NONE);

        // Mute inactive window
        RD_M_DrawTextSmallENG(mute_inactive_window ? "ON" : "OFF", 184 + wide_delta, 112, CR_NONE);

        // Informative message:
        if (CurrentItPos == 4)
        {
            RD_M_DrawTextSmallENG("CHANGING WILL REQUIRE RESTART OF THE PROGRAM", 3 + wide_delta, 132, CR_GRAY2RED_HEXEN);
        }
    }
    else
    {
        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            RD_M_DrawTextSmallRUS("JNRK.XTYS", 173 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else if (snd_sfxdevice == 3)
        {
            // ЦИФРОВЫЕ
            RD_M_DrawTextSmallRUS("WBAHJDST", 173 + wide_delta, 42, CR_NONE);
        }

        // Музыка
        if (snd_musicdevice == 0)
        {   
            // ОТКЛЮЧЕНА
            RD_M_DrawTextSmallRUS("JNRK.XTYF", 91 + wide_delta, 52, CR_GRAY2GDARKGRAY_HEXEN);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // СИНТЕЗ OPL2
            RD_M_DrawTextSmallRUS("CBYNTP", 91 + wide_delta, 52, CR_NONE);
            RD_M_DrawTextSmallENG("OPL2", 140 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // СИНТЕЗ OPL3
            RD_M_DrawTextSmallRUS("CBYNTP", 91 + wide_delta, 52, CR_NONE);
            RD_M_DrawTextSmallENG("OPL3", 140 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 5)
        {
            // ЭМУЛЯЦИЯ GUS
            RD_M_DrawTextSmallRUS("\'VEKZWBZ", 91 + wide_delta, 52, CR_NONE);
            RD_M_DrawTextSmallENG("GUS", 155 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 8)
        {
            // MIDI/MP3/OGG/FLAC
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC/TRACKER", 91 + wide_delta, 52, CR_NONE);
        }

        // Частота дискретизации (гц)
        if (snd_samplerate == 44100)
        {
            RD_M_DrawTextSmallRUS("44100 UW", 200 + wide_delta, 72, CR_NONE);
        }
        else if (snd_samplerate == 22050)
        {
            RD_M_DrawTextSmallRUS("22050 UW", 200 + wide_delta, 72, CR_NONE);
        }
        else if (snd_samplerate == 11025)
        {
            RD_M_DrawTextSmallRUS("11025 UW", 200 + wide_delta, 72, CR_NONE);
        }

        // Режим звуковых эффектов
        RD_M_DrawTextSmallRUS(snd_monomode ? "VJYJ" : "CNTHTJ", 226 + wide_delta, 92, CR_NONE);

        // Произвольный питч-шифтинг
        RD_M_DrawTextSmallRUS(snd_pitchshift ? "DRK" : "DSRK", 230 + wide_delta, 102, CR_NONE);

        // Звук в неактивном окне
        RD_M_DrawTextSmallRUS(mute_inactive_window ? "DSRK" : "DRK", 201 + wide_delta, 112, CR_NONE);

        // Informative message: ИЗМЕНЕНИЕ ПОТРЕБУЕТ ПЕРЕЗАПУСК ПРОГРАММЫ
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
        {
            RD_M_DrawTextSmallRUS("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS",
                                  11 + wide_delta, 132, CR_GRAY2RED_HEXEN);
        }
    }
}

static void M_RD_SoundDevice()
{
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 3;
        else if (snd_sfxdevice == 3)
            snd_sfxdevice = 0;

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume, recalculate sound curve
    snd_MaxVolume_tmp = snd_MaxVolume;
    soundchanged = true;
}

static void M_RD_MusicDevice(Direction_t direction)
{
    switch(direction)
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
        default:
            break;
    }
    RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
}

static void M_RD_Sampling(Direction_t direction)
{
    RD_Menu_ShiftSpinInt(&snd_samplerate, 11025, 44100, direction);
}

static void M_RD_SndMode()
{
    snd_monomode ^= 1;
}

static void M_RD_PitchShifting()
{
    snd_pitchshift ^= 1;
}

static void M_RD_MuteInactive()
{
    mute_inactive_window ^= 1;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    static char num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Always run
        RD_M_DrawTextSmallENG(alwaysRun ? "ON" : "OFF", 118 + wide_delta, 62, CR_NONE);

        // Mouse look
        RD_M_DrawTextSmallENG(mlook ? "ON" : "OFF", 118 + wide_delta, 102, CR_NONE);

        // Invert Y axis
        RD_M_DrawTextSmallENG(mouse_y_invert ? "ON" : "OFF", 133 + wide_delta, 112,
                              !mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);

        // Novert
        RD_M_DrawTextSmallENG(!novert ? "ON" : "OFF", 168 + wide_delta, 122,
                              mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);
    }
    else
    {
        // Режим постоянного бега
        RD_M_DrawTextSmallRUS(alwaysRun ? "DRK" : "DSRK", 209 + wide_delta, 62, CR_NONE);

        // Обзор мышью
        RD_M_DrawTextSmallRUS(mlook ? "DRK" : "DSRK", 132 + wide_delta, 102, CR_NONE);

        // Вертикальная инверсия
        RD_M_DrawTextSmallRUS(mouse_y_invert ? "DRK" : "DSRK", 199 + wide_delta, 112,
                              !mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);

        // Вертикальное перемещение
        RD_M_DrawTextSmallRUS(!novert ? "DRK" : "DSRK", 227 + wide_delta, 122,
                              mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);
    }

    // Mouse sensivity
    RD_Menu_DrawSliderSmall(&ControlsMenu, 92, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 93, CR_GRAY2GDARKGRAY_HEXEN);
}

static void M_RD_AlwaysRun()
{
    alwaysRun ^= 1;
}

static void M_RD_MouseLook()
{
    mlook ^= 1;
    if (!mlook)
        players[consoleplayer].centering = true;
}

static void M_RD_Sensitivity(Direction_t direction)
{
    RD_Menu_SlideInt(&mouseSensitivity, 0, 255, direction); // [crispy] extended range
}

static void M_RD_InvertY()
{
    mouse_y_invert ^= 1;
}

static void M_RD_Novert()
{
    novert ^= 1;
}

// -----------------------------------------------------------------------------
// Key bindings
// -----------------------------------------------------------------------------
void M_RD_Draw_Bindings()
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        RD_M_DrawTextSmallENG("ENTER TO CHANGE, DEL TO CLEAR", 55 + wide_delta, 176, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallENG("PGUP/PGDN TO TURN PAGES", 75 + wide_delta, 185, CR_GRAY2RED_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallENG("ENTER =", 44 + wide_delta, 176, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallRUS("= YFPYFXBNM<", 88 + wide_delta, 176, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallENG("DEL =", 176 + wide_delta, 176, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallRUS("JXBCNBNM", 213 + wide_delta, 176, CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG("PGUP/PGDN =", 55 + wide_delta, 185, CR_GRAY2RED_HEXEN);
        RD_M_DrawTextSmallRUS("KBCNFNM CNHFYBWS", 139 + wide_delta, 185, CR_GRAY2RED_HEXEN);
    }

    RD_Menu_Draw_Bindings(english_language ? 195 : 230);
}

// -----------------------------------------------------------------------------
// DrawGamepadMenu
// -----------------------------------------------------------------------------

static void OpenControllerSelectMenu()
{
    for(int i = 3; i < 13; ++i)
    {
        if(activeControllers[i - 3] != NULL)
        {
            GamepadSelectItems[i].type = ITT_EFUNC;
            GamepadSelectItems[i].option = i - 3;
        }
        else
        {
            GamepadSelectItems[i].type = ITT_EMPTY;
            GamepadSelectItems[i].option = -1;
        }
    }

    currentController = NULL;
    RD_Menu_SetMenu(&GamepadSelectMenu);
}

static void DrawGamepadSelectMenu()
{
    static char name[30];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if(english_language)
    {
        RD_M_DrawTextSmallENG(useController ? "ON" : "OFF", 190 + wide_delta, 32,
                              useController ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallRUS(useController ? "DRK" : "DSRK", 223 + wide_delta, 32,
                              useController ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }

    for(int i = 3; i < 13; ++i)
    {
        if(GamepadSelectItems[i].option != -1)
        {
            M_snprintf(name, 29, "%s...", activeControllers[GamepadSelectItems[i].option]->name);
            RD_M_DrawTextSmallENG(name, (english_language ? GamepadSelectMenu.x_eng : GamepadSelectMenu.x_rus) + wide_delta,
                                  GamepadSelectMenu.y + 10 * i, CR_NONE);
        }
    }
}

static void M_RD_UseGamepad()
{
    useController ^= 1;
    if(useController)
        I_InitControllerModule();
    else
        I_ShutdownController();
    OpenControllerSelectMenu();
}

static void OpenControllerOptionsMenu(int controller)
{
    currentController = activeControllers[controller];
    RD_Menu_SetMenu(&Gamepad1Menu);
}

static char* GetAxisNameENG(int axis)
{
    switch (axis)
    {
        case CONTROLLER_AXIS_BUTTON:
            return "BUTTON";
        case CONTROLLER_AXIS_MOVE:
            return "MOVE";
        case CONTROLLER_AXIS_STRAFE:
            return "STRAFE";
        case CONTROLLER_AXIS_TURN:
            return "TURN";
        case CONTROLLER_AXIS_VLOOK:
            return "VERTICAL LOOK";
        default:
            return "?";
    }
}

static char* GetAxisNameRUS(int axis)
{
    switch (axis)
    {
        case CONTROLLER_AXIS_BUTTON:
            return "RYJGRF"; // КНОПКА
        case CONTROLLER_AXIS_MOVE:
            return "LDB;TYBT"; // ДВИЖЕНИЕ
        case CONTROLLER_AXIS_STRAFE:
            return "LDB;TYBT ,JRJV"; // ДВИЖЕНИЕ БОКОМ
        case CONTROLLER_AXIS_TURN:
            return "GJDJHJN"; // ПОВОРОТ
        case CONTROLLER_AXIS_VLOOK:
            return "DTHNBRFKMYSQ J,PJH"; // ВЕРТИКАЛЬНЫЙ ОБЗОР
        default:
            return "?";
    }
}

static void DrawGamepadMenu_1()
{
    static char num[5];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if(english_language)
    {
        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTX]),
                              120 + wide_delta, 32, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? "ON" : "OFF",
                              120 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 62, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTY]),
                              120 + wide_delta, 82, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 92, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? "ON" : "OFF",
                              120 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 112, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT]),
                              130 + wide_delta, 132, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallENG(num, 260 + wide_delta, 142, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? "ON" : "OFF",
                              120 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTX]),
                              110 + wide_delta, 32, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? "DRK" : "DSRK",
                              155 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 62, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTY]),
                              107 + wide_delta, 82, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 92, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? "DRK" : "DSRK",
                              155 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 112, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT]),
                              127 + wide_delta, 132, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallRUS(num, 290 + wide_delta, 142, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? "DRK" : "DSRK",
                              155 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallRUS(num, 223 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
    }
}

static void M_RD_BindAxis_LY(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTY],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_LY(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY],
                     1, 16, direction);
}

static void M_RD_InvertAxis_LY()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ^= 1;
}

static void M_RD_DeadZoneAxis_LY(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY],
                     0, 100, direction);
}

static void M_RD_BindAxis_LX(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTX],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_LX(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX],
                     1, 16, direction);
}

static void M_RD_InvertAxis_LX()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ^= 1;
}

static void M_RD_DeadZoneAxis_LX(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX],
                     0, 100, direction);
}

static void M_RD_BindAxis_LT(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_LT(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT],
                     1, 16, direction);
}

static void M_RD_InvertAxis_LT()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ^= 1;
}

static void M_RD_DeadZoneAxis_LT(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT],
                     0, 100, direction);
}

static void DrawGamepadMenu_2()
{
    static char num[5];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if(english_language)
    {
        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX]),
                              125 + wide_delta, 32, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? "ON" : "OFF",
                              120 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 62, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY]),
                              125 + wide_delta, 82, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 92, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? "ON" : "OFF",
                              120 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 112, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]),
                              135 + wide_delta, 132, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(115, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallENG(num, 260 + wide_delta, 142, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? "ON" : "OFF",
                              120 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(110, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX]),
                              117 + wide_delta, 32, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 42, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? "DRK" : "DSRK",
                              155 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 62, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY]),
                              117 + wide_delta, 82, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 92, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? "DRK" : "DSRK",
                              155 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 112, CR_GRAY2GDARKGRAY_HEXEN);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]),
                              135 + wide_delta, 132, CR_GRAY2DARKGOLD_HEXEN);

        RD_Menu_DrawSliderSmallInline(145, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallRUS(num, 290 + wide_delta, 142, CR_GRAY2GDARKGRAY_HEXEN);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? "DRK" : "DSRK",
                              155 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_Menu_DrawSliderSmallInline(117, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallRUS(num, 223 + wide_delta, 162, CR_GRAY2GDARKGRAY_HEXEN);
    }
}

static void M_RD_BindAxis_RY(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_RY(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY],
                     1, 16, direction);
}

static void M_RD_InvertAxis_RY()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ^= 1;
}

static void M_RD_DeadZoneAxis_RY(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY],
                     0, 100, direction);
}

static void M_RD_BindAxis_RX(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_RX(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX],
                     1, 16, direction);
}

static void M_RD_InvertAxis_RX()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ^= 1;
}

static void M_RD_DeadZoneAxis_RX(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX],
                     0, 100, direction);
}

static void M_RD_BindAxis_RT(Direction_t direction)
{
    RD_Menu_SpinInt(&currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT],
                    CONTROLLER_AXIS_BUTTON, CONTROLLER_AXIS_VLOOK, direction);
}

static void M_RD_SensitivityAxis_RT(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT],
                     1, 16, direction);
}

static void M_RD_InvertAxis_RT()
{
    currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ^= 1;
}

static void M_RD_DeadZoneAxis_RT(Direction_t direction)
{
    RD_Menu_SlideInt(&currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT],
                     0, 100, direction);
}

// -----------------------------------------------------------------------------
// DrawGameplay1Menu
// -----------------------------------------------------------------------------

static void DrawGameplay1Menu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // GRAPHICAL
        //

        // Brightmaps
        RD_M_DrawTextSmallENG(brightmaps ? "ON" : "OFF", 119 + wide_delta, 42,
                              brightmaps ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Fake contrast
        RD_M_DrawTextSmallENG(fake_contrast ? "ON" : "OFF", 143 + wide_delta, 52,
                              fake_contrast ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Sky drawing mode
        RD_M_DrawTextSmallENG(linear_sky ? "LINEAR" : "ORIGINAL", 162 + wide_delta, 62,
                              linear_sky ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Randomly flipped corpses
        RD_M_DrawTextSmallENG(randomly_flipcorpses ? "ON" : "OFF", 232 + wide_delta, 72,
                              randomly_flipcorpses ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Flip weapons
        RD_M_DrawTextSmallENG(flip_weapons ? "ON" : "OFF", 130 + wide_delta, 82,
                              flip_weapons ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        //
        // PHYSICAL
        //
       
        // Collision physics
        RD_M_DrawTextSmallENG(improved_collision ? "IMPROVED" : "ORIGINAL", 159 + wide_delta, 102,
                              improved_collision ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Corpses sliding from the ledges
        RD_M_DrawTextSmallENG(torque ? "ON" : "OFF", 238 + wide_delta, 112,
                              torque ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Floating items amplitude
        RD_M_DrawTextSmallENG(floating_powerups == 1 ? "STANDARD" :
                              floating_powerups == 2 ? "HALFED" : "OFF",
                              209 + wide_delta, 122,
                              floating_powerups ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    else
    {
        //
        // ГРАФИКА
        //

        // Брайтмаппинг
        RD_M_DrawTextSmallRUS(brightmaps ? "DRK" : "DSRK", 133 + wide_delta, 42,
                              brightmaps ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Имитация контрастности
        RD_M_DrawTextSmallRUS(fake_contrast ? "DRK" : "DSRK", 205 + wide_delta, 52,
                              fake_contrast ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Режим отрисовки неба
        RD_M_DrawTextSmallRUS(linear_sky ? "KBYTQYSQ" : "JHBUBYFKMYSQ", 195 + wide_delta, 62,
                              linear_sky ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Зеркальное отражение трупов
        RD_M_DrawTextSmallRUS(randomly_flipcorpses ? "DRK" : "DSRK", 247 + wide_delta, 72,
                              randomly_flipcorpses ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Зеркальное отражение оружия
        RD_M_DrawTextSmallRUS(flip_weapons ? "DRK" : "DSRK", 250 + wide_delta, 82,
                              flip_weapons ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        //
        // ФИЗИКА
        //

        // Физика столкновений
        RD_M_DrawTextSmallRUS(improved_collision ? "EKEXITYYFZ" : "JHBUBYFKMYFZ", 186 + wide_delta, 102,
                              improved_collision ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Трупы сползают с возвышений
        RD_M_DrawTextSmallRUS(torque ? "DRK" : "DSRK", 248 + wide_delta, 112,
                              torque ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Амплитуда левитации предметов
        RD_M_DrawTextSmallRUS(floating_powerups == 1 ? "CNFYLFHNYFZ" :
                              floating_powerups == 2 ? "EVTHTYYFZ" : "DSRK",
                              188 + wide_delta, 122,
                              floating_powerups ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
}

static void M_RD_Brightmaps()
{
    brightmaps ^= 1;
}

static void M_RD_FakeContrast()
{
    fake_contrast ^= 1;
}

static void M_RD_LinearSky()
{
    linear_sky ^= 1;
}

static void M_RD_FlipCorpses()
{
    randomly_flipcorpses ^= 1;
}

static void M_RD_FlipWeapons()
{
    flip_weapons ^= 1;
}

static void M_RD_Collision()
{
    improved_collision ^= 1;
}

static void M_RD_Torque()
{
    torque ^= 1;
}

static void M_RD_FloatAmplitude(Direction_t direction)
{
    RD_Menu_SpinInt(&floating_powerups, 0, 2, direction);
}


// -----------------------------------------------------------------------------
// DrawGameplay2Menu
// -----------------------------------------------------------------------------

static void DrawGameplay2Menu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // STATUS BAR

        // Colored Status Bar
        RD_M_DrawTextSmallENG(sbar_colored ? "ON" : "OFF", 177 + wide_delta, 42,
                          sbar_colored ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Colored health gem
        RD_M_DrawTextSmallENG(sbar_colored_gem == 1 ? "BRIGHT" :
                              sbar_colored_gem == 2 ? "DARK" : "OFF",
                              175 + wide_delta, 52,
                              sbar_colored_gem == 1 ? CR_GRAY2GREEN_HEXEN :
                              sbar_colored_gem == 2 ? CR_GRAY2DARKGREEN_HEXEN :
                              CR_GRAY2RED_HEXEN);

        // Negative health
        RD_M_DrawTextSmallENG(negative_health ? "ON" : "OFF", 190 + wide_delta, 62,
                              negative_health ? CR_GRAY2GREEN_HEXEN : CR_GRAY2DARKGREEN_HEXEN);

        //
        // CROSSHAIR
        //

        // Draw crosshair
        RD_M_DrawTextSmallENG(crosshair_draw ? "ON" : "OFF", 150 + wide_delta, 82,
                              crosshair_draw ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Indication
        RD_M_DrawTextSmallENG(crosshair_type == 1 ? "HEALTH" : "STATIC",  111 + wide_delta, 92,
                              crosshair_type ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Increased size
        RD_M_DrawTextSmallENG(crosshair_scale ? "ON" : "OFF", 146 + wide_delta, 102,
                              crosshair_scale ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        //
        // GAMEPLAY
        //

        // Flip game levels
        RD_M_DrawTextSmallENG(flip_levels ? "ON" : "OFF", 153 + wide_delta, 122,
                              flip_levels ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Play internal demos
        RD_M_DrawTextSmallENG(no_internal_demos ? "OFF" : "ON", 179 + wide_delta, 132,
                              no_internal_demos ? CR_GRAY2RED_HEXEN : CR_GRAY2GREEN_HEXEN);
    }
    else
    {
        //
        // СТАТУС-БАР
        //

        // Разноцветные элементы
        RD_M_DrawTextSmallRUS(sbar_colored ? "DRK" : "DSRK", 206 + wide_delta, 42,
                              sbar_colored ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Окрашивание камня здоровья
        RD_M_DrawTextSmallRUS(sbar_colored_gem == 1 ? "CDTNKJT" :
                              sbar_colored_gem == 2 ? "NTVYJT" : "DSRK", 238 + wide_delta, 52,
                              sbar_colored_gem == 1 ? CR_GRAY2GREEN_HEXEN :
                              sbar_colored_gem == 2 ? CR_GRAY2DARKGREEN_HEXEN :
                              CR_GRAY2RED_HEXEN);

        // Отрицательное здоровье
        RD_M_DrawTextSmallRUS(negative_health ? "DRK" : "DSRK", 211 + wide_delta, 62,
                          negative_health ? CR_GRAY2GREEN_HEXEN : CR_GRAY2DARKGREEN_HEXEN);

        //
        // ПРИЦЕЛ
        //

        // Отображать прицел
        RD_M_DrawTextSmallRUS(crosshair_draw ? "DRK" : "DSRK", 175 + wide_delta, 82,
                              crosshair_draw ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Индикация
        RD_M_DrawTextSmallRUS(crosshair_type == 1 ? "PLJHJDMT" : // ЗДОРОВЬЕ
                                                    "CNFNBXYFZ", // СТАТИЧНАЯ
                              111 + wide_delta, 92, crosshair_type ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Увеличенный размер
        RD_M_DrawTextSmallRUS(crosshair_scale ? "DRK" : "DSRK", 181 + wide_delta, 102,
                              crosshair_scale ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        //
        // ГЕЙМПЛЕЙ
        //

        // Зеркальное отражение уровней
        RD_M_DrawTextSmallRUS(flip_levels ? "DRK" : "DSRK", 255 + wide_delta, 122,
                              flip_levels ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Проигрывать демозаписи
        RD_M_DrawTextSmallRUS(no_internal_demos ? "DRK" : "DSRK", 211 + wide_delta, 132,
                              no_internal_demos ? CR_GRAY2RED_HEXEN : CR_GRAY2GREEN_HEXEN);
    }
}

static void M_RD_ColoredSBar()
{
    sbar_colored ^= 1;
}

static void M_RD_ColoredGem(Direction_t direction)
{
    RD_Menu_SpinInt(&sbar_colored_gem, 0, 2, direction);
}

static void M_RD_NegativeHealth()
{
    negative_health ^= 1;
}

static void M_RD_CrossHairDraw()
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairType()
{
    crosshair_type ^= 1;
}

static void M_RD_CrossHairScale()
{
    crosshair_scale ^= 1;
}

static void M_RD_FlipLevels()
{
    flip_levels ^= 1;

    // [JN] Redraw game screen
    R_ExecuteSetViewSize();
}

static void M_RD_NoDemos()
{
    no_internal_demos ^= 1;
}


//---------------------------------------------------------------------------
// DrawLevelSelect1Menu
//---------------------------------------------------------------------------

static int ArmorMax[PCLASS_PIG][NUMARMOR] = {
    {4,3,5,1},
    {5,1,2,4},
    {3,2,1,5},
};

static int ArmorMaxTotal[PCLASS_PIG] = {16, 14, 12};

static void DrawLevelSelect1Menu(void)
{
    char  num[20];
    int totalArmor;

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if(english_language)
    {
        // CLASS
        RD_M_DrawTextSmallENG(selective_class == 0 ? "FIGHTER" :
                              selective_class == 1 ? "CLERIC" :
                              "MAGE", 248 + wide_delta, 26, CR_NONE);
    }
    else
    {
        // КЛАСС
        RD_M_DrawTextSmallRUS(selective_class == 0 ? "DJBY" :
                              selective_class == 1 ? "RKTHBR" :
                              "VFU", 248 + wide_delta, 26, CR_NONE);
    }

    // Skill level | Сложность
    M_snprintf(num, 4, "%d", selective_skill+1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 36, CR_NONE);

    // HUB | ХАБ
    M_snprintf(num, 4, "%d", selective_episode);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 46, CR_NONE);

    // MAP | УРОВЕНЬ
    M_snprintf(num, 18, "VISIT %d (MAP %d)", selective_map, P_TranslateMap(selective_map));
    RD_M_DrawTextSmallENG(num, 195 + wide_delta, 56, CR_NONE);

    // Health | Здоровье
    M_snprintf(num, 4, "%d", selective_health);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 76,
                          selective_health >= 67 ? CR_GRAY2GREEN_HEXEN :
                          selective_health >= 34 ? CR_GRAY2DARKGOLD_HEXEN :
                          CR_GRAY2RED_HEXEN);

    // ARMOR | БРОНЯ
    totalArmor = selective_armor_0 + selective_armor_1 + selective_armor_2 + selective_armor_3 +
            (selective_class == PCLASS_FIGHTER ? 3 : selective_class == PCLASS_CLERIC ? 2 : 1);
    M_snprintf(num, 4, "%d", totalArmor);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
         totalArmor > ArmorMaxTotal[selective_class] ? CR_GRAY2GREEN_HEXEN :
         totalArmor == ArmorMaxTotal[selective_class] ? CR_GRAY2DARKGOLD_HEXEN :
         CR_NONE);

    // MESH ARMOR | КОЛЬЧУГА
    M_snprintf(num, 4, "%d", selective_armor_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
        selective_armor_0 > ArmorMax[selective_class][ARMOR_ARMOR] ? CR_GRAY2GREEN_HEXEN :
        selective_armor_0 == ArmorMax[selective_class][ARMOR_ARMOR] ? CR_GRAY2DARKGOLD_HEXEN :
        CR_NONE);

    // FALCON SHIELD | СОКОЛИНЫЙ ЩИТ
    M_snprintf(num, 4, "%d", selective_armor_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
        selective_armor_1 > ArmorMax[selective_class][ARMOR_SHIELD] ? CR_GRAY2GREEN_HEXEN :
        selective_armor_1 == ArmorMax[selective_class][ARMOR_SHIELD] ? CR_GRAY2DARKGOLD_HEXEN :
        CR_NONE);

    // PLATINUM HELMET | ПЛАТИНОВЫЙ ШЛЕМ
    M_snprintf(num, 4, "%d", selective_armor_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
        selective_armor_2 > ArmorMax[selective_class][ARMOR_HELMET] ? CR_GRAY2GREEN_HEXEN :
        selective_armor_2 == ArmorMax[selective_class][ARMOR_HELMET] ? CR_GRAY2DARKGOLD_HEXEN :
        CR_NONE);

    // AMULET OF WARDING | АМУЛЕТ СТРАЖА
    M_snprintf(num, 4, "%d", selective_armor_3);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
        selective_armor_3 > ArmorMax[selective_class][ARMOR_AMULET] ? CR_GRAY2GREEN_HEXEN :
        selective_armor_3 == ArmorMax[selective_class][ARMOR_AMULET] ? CR_GRAY2DARKGOLD_HEXEN :
        CR_NONE);

    // [Dasperal] Update Status bar.
    SB_state = -1;
}

static void M_RD_SelectiveClass(Direction_t direction)
{
    RD_Menu_SpinInt(&selective_class, 0, 2, direction);

    if(selective_armor_0 > ArmorMax[selective_class][ARMOR_ARMOR] + 1)
        selective_armor_0 = ArmorMax[selective_class][ARMOR_ARMOR] + 1;
    if(selective_armor_1 > ArmorMax[selective_class][ARMOR_SHIELD] + 1)
        selective_armor_1 = ArmorMax[selective_class][ARMOR_SHIELD] + 1;
    if(selective_armor_2 > ArmorMax[selective_class][ARMOR_HELMET] + 1)
        selective_armor_2 = ArmorMax[selective_class][ARMOR_HELMET] + 1;
    if(selective_armor_3 > ArmorMax[selective_class][ARMOR_AMULET] + 1)
        selective_armor_3 = ArmorMax[selective_class][ARMOR_AMULET] + 1;

    switch (selective_class)
    {
        case PCLASS_FIGHTER:
            Level1Items[15].pointer = &LevelSelectMenu2_F;
            LevelSelectMenuPages[1] = &LevelSelectMenu2_F;
            break;
        case PCLASS_CLERIC:
            Level1Items[15].pointer = &LevelSelectMenu2_C;
            LevelSelectMenuPages[1] = &LevelSelectMenu2_C;
            break;
        case PCLASS_MAGE:
            Level1Items[15].pointer = &LevelSelectMenu2_M;
            LevelSelectMenuPages[1] = &LevelSelectMenu2_M;
            break;
        default:
            break;
    }
}

static void M_RD_SelectiveSkill(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_skill, 0, 5, direction);
}

static void M_RD_SelectiveHub(Direction_t direction)
{
    int map;
    int temp_warp;
    int warp = 0;

    // [Dasperal] Hexen Demo has only 1 hub
    if(isHexenDemo)
        return;

    map = selective_episode;
    RD_Menu_SlideInt(&selective_episode, 1, 99, direction);
    if(map == selective_episode)
        return;

    for (map = 1; map < MapCount; map++)
    {
        if(P_GetMapCluster(map) != selective_episode)
            continue;

        temp_warp = P_GetMapWarpTrans(map);
        if(temp_warp < warp || warp == 0)
            warp = temp_warp;
    }

    if(warp == 0)
    {
        selective_episode--;
        return;
    }

    if(!isDK && !hasUnknownPWads && warp == 20)
        warp = 21; // [Dasperal] CASTLE OF GRIEF instead of FORSAKEN OUTPOST

    selective_map = warp;
}

static void M_RD_SelectiveMap(Direction_t direction)
{
    int map;

    switch (direction)
    {
        case LEFT_DIR:
            selective_map--;
            map = P_TranslateMap(selective_map);
            if(map == -1 || P_GetMapCluster(map) != selective_episode)
                selective_map++;
            break;
        case RIGHT_DIR:
            selective_map++;
            map = P_TranslateMap(selective_map);
            if(map == -1 || P_GetMapCluster(map) != selective_episode)
                selective_map--;
            break;
        default:
            break;
    }
}

static void M_RD_SelectiveHealth(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_health, 1, 100, direction);
}

static void M_RD_SelectiveArmor_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor_0, 0, ArmorMax[selective_class][ARMOR_ARMOR] + 1, direction);
}

static void M_RD_SelectiveArmor_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor_1, 0, ArmorMax[selective_class][ARMOR_SHIELD] + 1, direction);
}

static void M_RD_SelectiveArmor_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor_2, 0, ArmorMax[selective_class][ARMOR_HELMET] + 1, direction);
}

static void M_RD_SelectiveArmor_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor_3, 0, ArmorMax[selective_class][ARMOR_AMULET] + 1, direction);
}

//---------------------------------------------------------------------------
// DrawLevelSelect2Menu
//---------------------------------------------------------------------------

static void DrawLevelSelect2Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        RD_M_DrawTextSmallENG(selective_wp_second ? "YES" : "NO", 248 + wide_delta, 36,
                              selective_wp_second ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG(selective_wp_third ? "YES" : "NO", 248 + wide_delta, 46,
                              selective_wp_third ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG(selective_wp_fourth ? "YES" : "NO", 248 + wide_delta, 56,
                              selective_wp_fourth ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG(selective_wp_piece_0 ? "YES" : "NO", 248 + wide_delta, 66,
                              selective_wp_piece_0 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG(selective_wp_piece_1 ? "YES" : "NO", 248 + wide_delta, 76,
                              selective_wp_piece_1 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallENG(selective_wp_piece_2 ? "YES" : "NO", 248 + wide_delta, 86,
                              selective_wp_piece_2 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    else
    {
        RD_M_DrawTextSmallRUS(selective_wp_second ? "LF" : "YTN", 248 + wide_delta, 36,
                              selective_wp_second ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallRUS(selective_wp_third ? "LF" : "YTN", 248 + wide_delta, 46,
                              selective_wp_third ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallRUS(selective_wp_fourth ? "LF" : "YTN", 248 + wide_delta, 56,
                              selective_wp_fourth ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallRUS(selective_wp_piece_0 ? "LF" : "YTN", 248 + wide_delta, 66,
                              selective_wp_piece_0 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallRUS(selective_wp_piece_1 ? "LF" : "YTN", 248 + wide_delta, 76,
                              selective_wp_piece_1 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        RD_M_DrawTextSmallRUS(selective_wp_piece_2 ? "LF" : "YTN", 248 + wide_delta, 86,
                              selective_wp_piece_2 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }

    // BLUE MANA
    M_snprintf(num, 4, "%d", selective_ammo_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_ammo_0 >= 100 ? CR_GRAY2GREEN_HEXEN :
                          selective_ammo_0 >= 50 ? CR_GRAY2DARKGOLD_HEXEN :
                          CR_GRAY2RED_HEXEN);

    // GREEN MANA
    M_snprintf(num, 4, "%d", selective_ammo_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_ammo_1 >= 100 ? CR_GRAY2GREEN_HEXEN :
                          selective_ammo_1 >= 50 ? CR_GRAY2DARKGOLD_HEXEN :
                          CR_GRAY2RED_HEXEN);

    // QUARTZ FLASK
    M_snprintf(num, 4, "%d", selective_arti_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_arti_0 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // MYSTIC URN
    M_snprintf(num, 4, "%d", selective_arti_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_arti_1 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // FLECHETTE
    M_snprintf(num, 4, "%d", selective_arti_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_arti_2 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // [Dasperal] Update Status bar.
    SB_state = -1;
}

static void M_RD_SelectiveWp_0()
{
    selective_wp_second ^= 1;
}

static void M_RD_SelectiveWp_1()
{
    selective_wp_third ^= 1;
}

static void M_RD_SelectiveWp_2()
{
    selective_wp_fourth ^= 1;

    if(selective_wp_fourth)
        selective_wp_piece_0 = selective_wp_piece_1 = selective_wp_piece_2 = 1;
    else
        selective_wp_piece_0 = selective_wp_piece_1 = selective_wp_piece_2 = 0;
}

static void M_RD_SelectiveWp_P_0()
{
    selective_wp_piece_0 ^= 1;

    if(selective_wp_piece_0 && selective_wp_piece_1 && selective_wp_piece_2)
        selective_wp_fourth = 1;
    else
        selective_wp_fourth = 0;
}

static void M_RD_SelectiveWp_P_1()
{
    selective_wp_piece_1 ^= 1;

    if(selective_wp_piece_0 && selective_wp_piece_1 && selective_wp_piece_2)
        selective_wp_fourth = 1;
    else
        selective_wp_fourth = 0;
}

static void M_RD_SelectiveWp_P_2()
{
    selective_wp_piece_2 ^= 1;

    if(selective_wp_piece_0 && selective_wp_piece_1 && selective_wp_piece_2)
        selective_wp_fourth = 1;
    else
        selective_wp_fourth = 0;
}

static void M_RD_SelectiveAmmo_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_0, 0, MAX_MANA, direction);
}

static void M_RD_SelectiveAmmo_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_1, 0, MAX_MANA, direction);
}

static void M_RD_SelectiveArti_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_0, 0, 25, direction);
}

static void M_RD_SelectiveArti_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_1, 0, 25, direction);
}

static void M_RD_SelectiveArti_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_2, 0, 25, direction);
}

//---------------------------------------------------------------------------
// DrawLevelSelect3Menu
//---------------------------------------------------------------------------

static void DrawLevelSelect3Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    //DISC OF REPULSION
    M_snprintf(num, 4, "%d", selective_arti_3);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 26,
                          selective_arti_3 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //ICON OF THE DEFENDER
    M_snprintf(num, 4, "%d", selective_arti_4);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 36,
                          selective_arti_4 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //PORKALATOR
    M_snprintf(num, 4, "%d", selective_arti_5);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 46,
                          selective_arti_5 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //CHAOS DEVICE
    M_snprintf(num, 4, "%d", selective_arti_6);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 56,
                          selective_arti_6 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //BANISHMENT DEVICE
    M_snprintf(num, 4, "%d", selective_arti_7);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 66,
                          selective_arti_7 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //WINGS OF WRATH
    M_snprintf(num, 4, "%d", selective_arti_8);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 76,
                          selective_arti_8 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //TORCH
    M_snprintf(num, 4, "%d", selective_arti_9);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
                          selective_arti_9 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //KRATER OF MIGHT
    M_snprintf(num, 4, "%d", selective_arti_10);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
                          selective_arti_10 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //DRAGONSKIN BRACERS
    M_snprintf(num, 4, "%d", selective_arti_11);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_arti_11 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //DARK SERVANT
    M_snprintf(num, 4, "%d", selective_arti_12);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_arti_12 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    //BOOTS OF SPEED
    M_snprintf(num, 4, "%d", selective_arti_13);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
                          selective_arti_13 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // MYSTIC AMBIT INCANT
    M_snprintf(num, 4, "%d", selective_arti_14);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_arti_14 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    if (english_language)
    {
        // EMERALD KEY
        RD_M_DrawTextSmallENG(selective_key_0 ? "YES" : "NO", 248 + wide_delta, 156,
                              selective_key_0 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    else
    {
        // ИЗУМРУДНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_0 ? "LF" : "YTN", 248 + wide_delta, 156,
                              selective_key_0 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }

    // [Dasperal] Update Status bar.
    SB_state = -1;
}

static void M_RD_SelectiveArti_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_3, 0, 25, direction);
}

static void M_RD_SelectiveArti_4(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_4, 0, 25, direction);
}

static void M_RD_SelectiveArti_5(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_5, 0, 25, direction);
}

static void M_RD_SelectiveArti_6(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_6, 0, 25, direction);
}

static void M_RD_SelectiveArti_7(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_7, 0, 25, direction);
}

static void M_RD_SelectiveArti_8(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_8, 0, 25, direction);
}

static void M_RD_SelectiveArti_9(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_9, 0, 25, direction);
}

static void M_RD_SelectiveArti_10(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_10, 0, 25, direction);
}

static void M_RD_SelectiveArti_11(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_11, 0, 25, direction);
}

static void M_RD_SelectiveArti_12(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_12, 0, 25, direction);
}

static void M_RD_SelectiveArti_13(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_13, 0, 25, direction);
}

static void M_RD_SelectiveArti_14(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_14, 0, 25, direction);
}

static void M_RD_SelectiveKey_0()
{
    selective_key_0 ^= 1;
}

//---------------------------------------------------------------------------
// DrawLevelSelect4Menu
//---------------------------------------------------------------------------

static void DrawLevelSelect4Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // SILVER KEY
        RD_M_DrawTextSmallENG(selective_key_1 ? "YES" : "NO", 248 + wide_delta, 26,
                              selective_key_1 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // FIRE KEY
        RD_M_DrawTextSmallENG(selective_key_2 ? "YES" : "NO", 248 + wide_delta, 36,
                              selective_key_2 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // STEEL KEY
        RD_M_DrawTextSmallENG(selective_key_3 ? "YES" : "NO", 248 + wide_delta, 46,
                              selective_key_3 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // HORN KEY
        RD_M_DrawTextSmallENG(selective_key_4 ? "YES" : "NO", 248 + wide_delta, 56,
                              selective_key_4 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // CAVE KEY
        RD_M_DrawTextSmallENG(selective_key_5 ? "YES" : "NO", 248 + wide_delta, 66,
                              selective_key_5 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // CASTLE KEY
        RD_M_DrawTextSmallENG(selective_key_6 ? "YES" : "NO", 248 + wide_delta, 76,
                              selective_key_6 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // SWAMP KEY
        RD_M_DrawTextSmallENG(selective_key_7 ? "YES" : "NO", 248 + wide_delta, 86,
                              selective_key_7 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // RUSTED KEY
        RD_M_DrawTextSmallENG(selective_key_8 ? "YES" : "NO", 248 + wide_delta, 96,
                              selective_key_8 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // DUNGEON KEY
        RD_M_DrawTextSmallENG(selective_key_9 ? "YES" : "NO", 248 + wide_delta, 106,
                              selective_key_9 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // AXE KEY
        RD_M_DrawTextSmallENG(selective_key_10 ? "YES" : "NO", 248 + wide_delta, 116,
                              selective_key_10 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    else
    {
        // СЕРЕБРЯНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_1 ? "LF" : "YTN", 248 + wide_delta, 26,
                              selective_key_1 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // ОГНЕННЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_2 ? "LF" : "YTN", 248 + wide_delta, 36,
                              selective_key_2 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // СТАЛЬНОЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_3 ? "LF" : "YTN", 248 + wide_delta, 46,
                              selective_key_3 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // РОГОВОЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_4 ? "LF" : "YTN", 248 + wide_delta, 56,
                              selective_key_4 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // ПЕЩЕРНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_5 ? "LF" : "YTN", 248 + wide_delta, 66,
                              selective_key_5 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // КЛЮЧ ОТ ЗАМКА
        RD_M_DrawTextSmallRUS(selective_key_6 ? "LF" : "YTN", 248 + wide_delta, 76,
                              selective_key_6 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // БОЛОТНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_7 ? "LF" : "YTN", 248 + wide_delta, 86,
                              selective_key_7 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // РЖАВЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_8 ? "LF" : "YTN", 248 + wide_delta, 96,
                              selective_key_8 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
        RD_M_DrawTextSmallRUS(selective_key_9 ? "LF" : "YTN", 248 + wide_delta, 106,
                              selective_key_9 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // КЛЮЧ-ТОПОР
        RD_M_DrawTextSmallRUS(selective_key_10 ? "LF" : "YTN", 248 + wide_delta, 116,
                              selective_key_10 ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);
    }
    // FLAME MASK
    M_snprintf(num, 4, "%d", selective_puzzle_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_puzzle_0 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // HEART OF D'SPARIL
    M_snprintf(num, 4, "%d", selective_puzzle_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_puzzle_1 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // RUBY PLANET
    M_snprintf(num, 4, "%d", selective_puzzle_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_puzzle_2 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // [Dasperal] Update Status bar.
    SB_state = -1;
}

static void M_RD_SelectiveKey_1()
{
    selective_key_1 ^= 1;
}

static void M_RD_SelectiveKey_2()
{
    selective_key_2 ^= 1;
}

static void M_RD_SelectiveKey_3()
{
    selective_key_3 ^= 1;
}

static void M_RD_SelectiveKey_4()
{
    selective_key_4 ^= 1;
}

static void M_RD_SelectiveKey_5()
{
    selective_key_5 ^= 1;
}

static void M_RD_SelectiveKey_6()
{
    selective_key_6 ^= 1;
}

static void M_RD_SelectiveKey_7()
{
    selective_key_7 ^= 1;
}

static void M_RD_SelectiveKey_8()
{
    selective_key_8 ^= 1;
}

static void M_RD_SelectiveKey_9()
{
    selective_key_9 ^= 1;
}

static void M_RD_SelectiveKey_10()
{
    selective_key_10 ^= 1;
}

static void M_RD_SelectivePuzzle_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_0, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_1, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_2, 0, 25, direction);
}

//---------------------------------------------------------------------------
// DrawLevelSelect5Menu
//---------------------------------------------------------------------------
static void DrawLevelSelect5Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    // EMERALD PLANET 1
    M_snprintf(num, 4, "%d", selective_puzzle_3);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 26,
                          selective_puzzle_3 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // EMERALD PLANET 2
    M_snprintf(num, 4, "%d", selective_puzzle_4);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 36,
                          selective_puzzle_4 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // SAPPHIRE PLANET 1
    M_snprintf(num, 4, "%d", selective_puzzle_5);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 46,
                          selective_puzzle_5 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // SAPPHIRE PLANET 2
    M_snprintf(num, 4, "%d", selective_puzzle_6);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 56,
                          selective_puzzle_6 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // CLOCK GEAR (B&S)
    M_snprintf(num, 4, "%d", selective_puzzle_7);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 66,
                          selective_puzzle_7 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // CLOCK GEAR (B)
    M_snprintf(num, 4, "%d", selective_puzzle_8);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 76,
                          selective_puzzle_8 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // CLOCK GEAR (S&B)
    M_snprintf(num, 4, "%d", selective_puzzle_9);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
                          selective_puzzle_9 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // CLOCK GEAR (S)
    M_snprintf(num, 4, "%d", selective_puzzle_10);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
                          selective_puzzle_10 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // DAEMON CODEX
    M_snprintf(num, 4, "%d", selective_puzzle_11);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_puzzle_11 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // LIBER OSCURA
    if(!english_language)
        RD_M_DrawTextSmallENG("LIBER OSCURA:", 40 + wide_delta, 116, CR_NONE);
    M_snprintf(num, 4, "%d", selective_puzzle_12);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_puzzle_12 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // YORICK'S SKULL
    M_snprintf(num, 4, "%d", selective_puzzle_13);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
                          selective_puzzle_13 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // GLAIVE SEAL
    M_snprintf(num, 4, "%d", selective_puzzle_14);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_puzzle_14 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // HOlY RELIC
    M_snprintf(num, 4, "%d", selective_puzzle_15);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_puzzle_15 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // SIGIL OF THE MAGUS
    M_snprintf(num, 4, "%d", selective_puzzle_16);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_puzzle_16 ? CR_NONE : CR_GRAY2GDARKGRAY_HEXEN);

    // [Dasperal] Update Status bar.
    SB_state = -1;
}

static void M_RD_SelectivePuzzle_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_3, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_4(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_4, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_5(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_5, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_6(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_6, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_7(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_7, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_8(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_8, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_9(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_9, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_10(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_10, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_11(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_11, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_12(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_12, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_13(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_13, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_14(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_14, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_15(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_15, 0, 25, direction);
}

static void M_RD_SelectivePuzzle_16(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_puzzle_16, 0, 25, direction);
}

//---------------------------------------------------------------------------
// DrawOptionsMenu_Vanilla
//---------------------------------------------------------------------------

static void DrawOptionsMenu_Vanilla(void)
{
    if (english_language)
    {
        RD_M_DrawTextB(show_messages ? "ON" : "OFF", 196 + wide_delta, 50);
    }
    else
    {
        RD_M_DrawTextBigRUS(show_messages ? "DRK>" : "DSRK>", 223 + wide_delta, 50);	// ВКЛ. / ВЫКЛ.
    }

    RD_Menu_DrawSlider(&VanillaOptionsMenu, 92, 10, mouseSensitivity);
}

static void DrawOptions2Menu_Vanilla(void)
{
    if (aspect_ratio_temp >= 2)
    {
        RD_Menu_DrawSlider(&VanillaOptions2Menu, 42, 4, screenblocks - 9);
    }
    else
    {
        RD_Menu_DrawSlider(&VanillaOptions2Menu, 42, 10, screenblocks - 3);
    }
    RD_Menu_DrawSlider(&VanillaOptions2Menu, 82, 16, snd_MaxVolume);
    RD_Menu_DrawSlider(&VanillaOptions2Menu, 122, 16, snd_MusicVolume);
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
    max_fps                 = 200; uncapped_fps = 1;
    smoothing               = 0;
    vga_porch_flash         = 0;

    // Display
    screenblocks    = 10;
    usegamma        = 7;
    level_brightness = 0;

    // Color options
    brightness       = 1.0f;
    usegamma         = 7;
    color_saturation = 1.0f;
    show_palette     = 1;
    r_color_factor   = 1.0f;
    g_color_factor   = 1.0f;
    b_color_factor   = 1.0f;

    // Messages and Texts
    show_messages      = 1;
    messages_alignment = 0;
    messages_timeout   = 4;
    message_fade       = 1;
    draw_shadowed_text = 1;
    local_time         = 0;

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
    mlook               = 0;
    players[consoleplayer].centering = true;
    mouseSensitivity    = 5;

    // Gameplay
    brightmaps          = 1;
    fake_contrast       = 0;
    linear_sky          = 1;
    flip_weapons        = 0;
    sbar_colored        = 0;
    sbar_colored_gem    = 0;
    negative_health     = 0;
    crosshair_draw      = 0;
    crosshair_type      = 1;
    crosshair_scale     = 0;
    no_internal_demos   = 0;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Reset palette.
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

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

//===========================================================================
//
// SCNetCheck
//
//===========================================================================

boolean SCNetCheck(int option)
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
        default:
            break;
    }
    menuactive = false;
    S_StartSound(NULL, SFX_CHAT);
    return false;
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
    RD_Menu_DeactivateMenu();
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
        x = (english_language ? SaveMenu.x_eng : SaveMenu.x_rus) + 1;
        y = SaveMenu.y + 1 + option * ITEM_HEIGHT;
        I_StartTextInput(x, y, x + 190, y + ITEM_HEIGHT - 2);

        M_StringCopy(oldSlotText, SlotText[option], sizeof(oldSlotText));
        ptr = SlotText[option];
        while (*ptr)
        {
            ptr++;
        }
        *ptr = ASCII_CURSOR;
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
        RD_Menu_DeactivateMenu();
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
            RD_Menu_SetMenu(&SkillMenu_F);
            break;
        case PCLASS_CLERIC:
            RD_Menu_SetMenu(&SkillMenu_C);
            break;
        case PCLASS_MAGE:
            RD_Menu_SetMenu(&SkillMenu_M);
        default:
            break;
    }
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
    RD_Menu_DeactivateMenu();
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
    int charTyped;
    MenuItem_t *item;
    extern void H2_StartTitle(void);
    extern void G_CheckDemoStatus(void);
    char *textBuffer;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (event->type == ev_quit || BK_isKeyDown(event, bk_menu_activate) || BK_isKeyDown(event, bk_quit))
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

    // Only care about keypresses beyond this point.
    if (event->type != ev_keydown &&
        event->type != ev_mouse_keydown &&
        event->type != ev_controller_keydown)
    {
        return false;
    }

    if(isBinding)
    {
        BK_BindKey(event);
        return true;
    }

    if (InfoType)
    {
        InfoType = (InfoType + 1) % 4;

        if (BK_isKeyDown(event, bk_menu_activate))
        {
            InfoType = 0;
        }
        if (!InfoType)
        {
            if (!netgame && !demoplayback)
            {
                paused = false;
            }
            RD_Menu_DeactivateMenu();
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
        return (true);          //make the info screen eat the keypress
    }

    if ((ravpic && event->data1 == KEY_F1) || BK_isKeyDown(event, bk_screenshot))
    {
        G_ScreenShot();
        return (true);
    }

    if (askforquit)
    {
        if (BK_isKeyDown(event, bk_confirm))
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
                    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
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
        else if (BK_isKeyDown(event, bk_abort))
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
        if (BK_isKeyDown(event, bk_screen_dec))
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
        else if (BK_isKeyDown(event, bk_screen_inc))
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
        else if (BK_isKeyDown(event, bk_menu_help))           // F1 (help screen)
        {
            SCInfo(0);      // start up info screens
            menuactive = true;
            return (true);
        }
        else if (BK_isKeyDown(event, bk_menu_save))           // F2 (save game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                menuactive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                RD_Menu_SetMenu(&SaveMenu);
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_menu_load))           // F3 (load game)
        {
            if (SCNetCheck(2))
            {
                menuactive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                RD_Menu_SetMenu(&LoadMenu);
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_menu_volume))         // F4 (volume)
        {
            menuactive = true;
            FileMenuKeySteal = false;
            MenuTime = 0;
            RD_Menu_SetMenu(vanillaparm ? &VanillaOptions2Menu : &SoundMenu);
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (BK_isKeyDown(event, bk_detail))         // F5 (suicide)
        {
            menuactive = false;
            askforquit = true;
            typeofask = 5;  // suicide
            return true;
        }
        else if (BK_isKeyDown(event, bk_qsave))          // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    menuactive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    RD_Menu_SetMenu(&SaveMenu);
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
        else if (BK_isKeyDown(event, bk_endgame))        // F7 (end game)
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
        else if (BK_isKeyDown(event, bk_messages))       // F8 (toggle messages)
        {
            M_RD_Messages(0);
            return true;
        }
        else if (BK_isKeyDown(event, bk_qload))          // F9 (quickload)
        {
            if (SCNetCheck(2))
            {
                if (!quickload || quickload == -1)
                {
                    menuactive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    RD_Menu_SetMenu(&LoadMenu);
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
        else if (BK_isKeyDown(event, bk_quit))           // F10 (quit)
        {
            if (gamestate == GS_LEVEL || gamestate == GS_FINALE)
            {
                SCQuitGame(0);
                S_StartSound(NULL, SFX_CHAT);
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_gamma))          // F11 (gamma correction)
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
        else if (BK_isKeyDown(event, bk_reloadlevel))                 // F12 (???)
        {
            // F12 - reload current map (devmaps mode)
            // [JN] Allow only in devparm mode, see this comment:
            // https://github.com/JNechaevsky/russian-doom/issues/210#issuecomment-702321075

            if (netgame || !devparm)
            {
                return false;
            }
            if (BK_isKeyPressed(bk_speed))
            {               // Monsters ON
                nomonsters = false;
            }
            if (BK_isKeyPressed(bk_strafe))
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
        if (BK_isKeyDown(event, bk_menu_activate) || gamestate == GS_DEMOSCREEN || demoplayback)
        {
            RD_Menu_ActivateMenu();
            return (true);
        }
        return (false);
    }

    if (event->type == ev_keydown && event->data1 == KEY_DEL)
    {
        //[Dasperal] Key bindings menus
        if(CurrentMenu == &Bindings1Menu ||
           CurrentMenu == &Bindings2Menu ||
           CurrentMenu == &Bindings3Menu ||
           CurrentMenu == &Bindings4Menu ||
           CurrentMenu == &Bindings5Menu ||
           CurrentMenu == &Bindings6Menu ||
           CurrentMenu == &Bindings7Menu)
        {
            BK_ClearBinds(CurrentMenu->items[CurrentItPos].option);
            RD_Menu_StartSound(MENU_SOUND_SLIDER_MOVE);
            return true;
        }
    }

    if (!FileMenuKeySteal)
    {
        return RD_Menu_Responder(event);
    }
    else if(event->type == ev_keydown)
    {
        // Editing file names
        // When typing a savegame name, we use the fully shifted and
        // translated input value from event->data3.
        charTyped = event->data3;
        textBuffer = &SlotText[currentSlot][slotptr];
        if (BK_isKeyDown(event, bk_menu_back))
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
        if (BK_isKeyDown(event, bk_menu_activate))
        {
            M_StringCopy(SlotText[currentSlot], oldSlotText,
                         sizeof(SlotText[currentSlot]));
            SlotStatus[currentSlot]--;
            RD_Menu_DeactivateMenu();
            return (true);
        }
        if (BK_isKeyDown(event, bk_menu_select))
        {
            SlotText[currentSlot][slotptr] = 0; // clear the cursor
            item = (MenuItem_t*) &CurrentMenu->items[CurrentItPos];
            CurrentMenu->lastOn = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                ((void (*) (int)) item->pointer)(item->option);
            }
            return (true);
        }
        if (slotptr < SLOTTEXTLEN)
        {
            if (isalpha(charTyped))
            {
                *textBuffer++ = toupper(charTyped);
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
            // [JN] Extended support of Russian alphabet input, including special symbols.
            // Commented out characters aren't working properly because of using FONTA.
            if (isdigit(charTyped) 
            || charTyped == ' '
            || charTyped == '!'
            || charTyped == '('
            || charTyped == ')'
            || charTyped == '-'
            || charTyped == '+'
            || charTyped == '='
            || charTyped == '/'
            // || charTyped == '\\'
            || charTyped == '*'
            || charTyped == '?'
            // || charTyped == '_'
            // || charTyped == '<'
            // || charTyped == '>'
            || charTyped == ':'
            || charTyped == '"'
            // || charTyped == ']'   // ъ
            // || charTyped == '['   // х
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
    return false;
}

//---------------------------------------------------------------------------
//
// PROC OnActivateMenu
//
//---------------------------------------------------------------------------

void OnActivateMenu(void)
{
    if (paused)
    {
        S_ResumeSound();
    }
    FileMenuKeySteal = false;
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    slottextloaded = false;     //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void OnDeactivateMenu(void)
{
    S_ResumeSound();    // [JN] Fix vanilla Hexen bug: resume music playing
    if (FileMenuKeySteal)
    {
        I_StopTextInput();
    }
    if (!netgame)
    {
        paused = false;
    }
    P_ClearMessage(&players[consoleplayer]);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
    // [JN] For checking of modified fullscreen graphics.
    const patch_t *page0_gfx = W_CacheLumpName("TITLE", PU_CACHE);
    const patch_t *page1_gfx = W_CacheLumpName("HELP1", PU_CACHE);
    const patch_t *page2_gfx = W_CacheLumpName("HELP2", PU_CACHE);
    const patch_t *page3_gfx = W_CacheLumpName("CREDIT", PU_CACHE);

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

    // [JN] Some complex mess to avoid using numerical identification of screens.
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
                {
                    V_DrawPatchFullScreen(W_CacheLumpName
                                          (isDK ? "TITLEDKR" : "TITLE", PU_CACHE), false);
                }
                else
                {
                    V_DrawRawScreen(W_CacheLumpName("TITLE", PU_CACHE));
                }
            break;

            case 1:
                if (page1_gfx->width == 560)
                {
                    V_DrawPatchFullScreen(W_CacheLumpName("HELP1", PU_CACHE), false);
                }
                else
                {
                    V_DrawRawScreen(W_CacheLumpName("HELP1", PU_CACHE));
                }
            break;

            case 2:
                if (page2_gfx->width == 560)
                {
                    V_DrawPatchFullScreen(W_CacheLumpName("HELP2", PU_CACHE), false);
                }
                else
                {
                    V_DrawRawScreen(W_CacheLumpName("HELP2", PU_CACHE));
                }
            break;

            case 3:
                if (page3_gfx->width == 560)
                {
                    V_DrawPatchFullScreen(W_CacheLumpName
                                          (isDK ? "CREDITDK" : "CREDIT", PU_CACHE), false);
                }
                else
                {
                    V_DrawRawScreen(W_CacheLumpName("CREDIT", PU_CACHE));
                }
            break;

            default:
            break;
        }
    }
    else
    {
        switch (InfoType)
        {
            case 0:
                V_DrawPatchFullScreen(W_CacheLumpNum(W_GetNumForName
                                      (isDK ? "TITLEDKR" : "TITLE"), PU_CACHE), false);
            break;

            case 1:
                V_DrawPatchFullScreen(W_CacheLumpNum(W_GetNumForName
                                      ("RD_HELP1"), PU_CACHE), false);
            break;

            case 2:
                V_DrawPatchFullScreen(W_CacheLumpNum(W_GetNumForName
                                      ("RD_HELP2"), PU_CACHE), false);
            break;

            case 3:
                V_DrawPatchFullScreen(W_CacheLumpNum(W_GetNumForName
                                      (isDK ? "RD_CREDK" : "RD_CREDT"), PU_CACHE), false);
            break;
        }
    }
}

void RD_Menu_StartSound(MenuSound_t sound)
{
    switch (sound)
    {
        case MENU_SOUND_CURSOR_MOVE:
            S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            break;
        case MENU_SOUND_BACK:
        case MENU_SOUND_SLIDER_MOVE:
            S_StartSound(NULL, SFX_PICKUP_KEY);
            break;
        case MENU_SOUND_CLICK:
        case MENU_SOUND_PAGE:
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            break;
        case MENU_SOUND_ACTIVATE:
        case MENU_SOUND_DEACTIVATE:
            S_StartSound(NULL, SFX_PLATFORM_STOP);
            break;
        default:
            break;
    }
}

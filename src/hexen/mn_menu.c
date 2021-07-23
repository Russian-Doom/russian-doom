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
#include "s_sound.h"
#include "v_trans.h"
#include "v_video.h"
#include "rd_menu.h"
#include "rd_rushexen.h"
#include "crispy.h"
#include "jn.h"

// MACROS ------------------------------------------------------------------
#define ITEM_HEIGHT 20
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void InitMapInfo(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SCQuitGame(int option);
static void SCClass(int option);
static void SCSkill(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCMessages(int option);
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

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static void M_RD_Change_Widescreen(Direction_t direction);
static void M_RD_Change_VSync(Direction_t direction);
static void M_RD_Uncapped(Direction_t direction);
static void M_RD_FPScounter(Direction_t direction);
static void M_RD_Smoothing(Direction_t direction);
static void M_RD_PorchFlashing(Direction_t direction);
static void M_RD_Renderer(Direction_t direction);
static void M_RD_Screenshots(Direction_t direction);

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(Direction_t direction);
static void M_RD_Gamma(Direction_t direction);
static void M_RD_LevelBrightness(Direction_t direction);
static void M_RD_LocalTime(Direction_t direction);
static void M_RD_Messages(Direction_t direction);
static void M_RD_ShadowedText(Direction_t direction);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapOverlay(Direction_t direction);
static void M_RD_AutoMapRotate(Direction_t direction);
static void M_RD_AutoMapFollow(Direction_t direction);
static void M_RD_AutoMapGrid(Direction_t direction);

// Sound
static void DrawSoundMenu(void);
static void M_RD_SfxVolume(Direction_t direction);
static void M_RD_MusVolume(Direction_t direction);
static void M_RD_SfxChannels(Direction_t direction);

// Sound system
static void DrawSoundSystemMenu(void);
static void M_RD_SoundDevice(Direction_t direction);
static void M_RD_MusicDevice(Direction_t direction);
static void M_RD_Sampling(Direction_t direction);
static void M_RD_SndMode(Direction_t direction);
static void M_RD_PitchShifting(Direction_t direction);
static void M_RD_MuteInactive(Direction_t direction);

// Controls
static void DrawControlsMenu(void);
static void M_RD_AlwaysRun(Direction_t direction);
static void M_RD_Sensitivity(Direction_t direction);
static void M_RD_MouseLook(Direction_t direction);
static void M_RD_InvertY(Direction_t direction);
static void M_RD_Novert(Direction_t direction);

// Gameplay
static void DrawGameplayMenu(void);
static void M_RD_Brightmaps(Direction_t direction);
static void M_RD_FakeContrast(Direction_t direction);
static void M_RD_CrossHairDraw(Direction_t direction);
static void M_RD_CrossHairType(Direction_t direction);
static void M_RD_CrossHairScale(Direction_t direction);
static void M_RD_FlipLevels(Direction_t direction);
static void M_RD_NoDemos(Direction_t direction);

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
static void M_RD_SelectiveWp_0(Direction_t direction);
static void M_RD_SelectiveWp_1(Direction_t direction);
static void M_RD_SelectiveWp_2(Direction_t direction);
static void M_RD_SelectiveWp_P_0(Direction_t direction);
static void M_RD_SelectiveWp_P_1(Direction_t direction);
static void M_RD_SelectiveWp_P_2(Direction_t direction);
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
static void M_RD_SelectiveKey_0(Direction_t direction);

// Level Select (4)
static void DrawLevelSelect4Menu(void);
static void M_RD_SelectiveKey_1(Direction_t direction);
static void M_RD_SelectiveKey_2(Direction_t direction);
static void M_RD_SelectiveKey_3(Direction_t direction);
static void M_RD_SelectiveKey_4(Direction_t direction);
static void M_RD_SelectiveKey_5(Direction_t direction);
static void M_RD_SelectiveKey_6(Direction_t direction);
static void M_RD_SelectiveKey_7(Direction_t direction);
static void M_RD_SelectiveKey_8(Direction_t direction);
static void M_RD_SelectiveKey_9(Direction_t direction);
static void M_RD_SelectiveKey_10(Direction_t direction);
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

extern boolean gamekeydown[256];        // The NUMKEYS macro is local to g_game
extern int MapCount;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean menuactive;
int InfoType;
int messageson = true;
boolean mn_SuicideConsole;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// [JN] Original English fonts
static int FontAYellowBaseLump; // small yellow
// [JN] Unchangable Russian fonts
static int FontFYellowBaseLump; // small yellow

static int MauloBaseLump;
static int MenuPClass;
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

//[Dasperal] Predeclare menu variables to allow referencing them before they initialized
static Menu_t ClassMenu;
static Menu_t OptionsMenu;
static Menu_t RenderingMenu;
static Menu_t DisplayMenu;
static Menu_t AutomapMenu;
static Menu_t SoundMenu;
static Menu_t SoundSysMenu;
static Menu_t ControlsMenu;
static Menu_t GameplayMenu;
static Menu_t LevelSelectMenu1;
static Menu_t LevelSelectMenu2_F;
static Menu_t LevelSelectMenu2_C;
static Menu_t LevelSelectMenu2_M;
static Menu_t LevelSelectMenu3;
static Menu_t LevelSelectMenu4;
static Menu_t LevelSelectMenu5;
static const Menu_t* LevelSelectMenuPages[] = {&LevelSelectMenu1, &LevelSelectMenu2_F, &LevelSelectMenu3, &LevelSelectMenu4, &LevelSelectMenu5};
static Menu_t Options2Menu_Vanilla;
static Menu_t FilesMenu;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

static MenuItem_t MainItems[] = {
    {ITT_SETMENU_NONET, "NEW GAME",   "YJDFZ BUHF", &ClassMenu,   1}, // НОВАЯ ИГРА
    {ITT_SETMENU,       "OPTIONS",    "YFCNHJQRB",  &OptionsMenu, 0}, // НАСТРОЙКИ
    {ITT_SETMENU,       "GAME FILES", "AFQKS BUHS", &FilesMenu,   0}, // ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,         "INFO",       "BYAJHVFWBZ", SCInfo,       0}, // ИНФОРМАЦИЯ
    {ITT_EFUNC,         "QUIT GAME",  "DS[JL",      SCQuitGame,   0}  // ВЫХОД
};

static Menu_t MainMenu = {
    110, 104,
    56,
    NULL, NULL, true,
    5, MainItems, true,
    DrawMainMenu,
    NULL, 0,
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
    NULL, 0,
    &MainMenu,
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
    NULL, 0,
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
    NULL, 0,
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
    NULL, 0,
    &ClassMenu,
    2
};

// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "RENDERING",         "DBLTJ",          &RenderingMenu,      0}, // ВИДЕО
    {ITT_SETMENU, "DISPLAY",           "\'RHFY",         &DisplayMenu,        0}, // ЭКРАН
    {ITT_SETMENU, "SOUND",             "FELBJ",          &SoundMenu,          0}, // АУДИО
    {ITT_SETMENU, "CONTROLS",          "EGHFDKTYBT",     &ControlsMenu,       0}, // УПРАВЛЕНИЕ
    {ITT_SETMENU, "GAMEPLAY",          "UTQVGKTQ",       &GameplayMenu,       0}, // ГЕЙМПЛЕЙ
    {ITT_SETMENU, "LEVEL SELECT",      "DS,JH EHJDYZ",   &LevelSelectMenu1,   0}, // ВЫБОР УРОВНЯ
    {ITT_EFUNC,   "RESET SETTINGS",    "C,HJC YFCNHJTR", M_RD_ResetSettings,  0}, // СБРОС НАСТРОЕК
    {ITT_EFUNC,   "LANGUAGE: ENGLISH", "ZPSR: HECCRBQ",  M_RD_ChangeLanguage, 0}  // ЯЗЫК: РУССКИЙ
};

static Menu_t OptionsMenu = {
    77, 77,
    31,
    "OPTIONS", "YFCNHJQRB", false,
    8, OptionsItems, true,
    DrawOptionsMenu,
    NULL, 0,
    &MainMenu,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_TITLE,  "RENDERING",                 "HTYLTHBYU",                       NULL,                   0}, // РЕНДЕРИНГ
    {ITT_LRFUNC, "DISPLAY ASPECT RATIO:",     "CJJNYJITYBT CNJHJY \'RHFYF:",     M_RD_Change_Widescreen, 0}, // СООТНОШЕНИЕ СТОРОН ЭКРАНА
    {ITT_LRFUNC, "VERTICAL SYNCHRONIZATION:", "DTHNBRFKMYFZ CBY[HJYBPFWBZ:",     M_RD_Change_VSync,      0}, // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    {ITT_LRFUNC, "FRAME RATE:",               "RFLHJDFZ XFCNJNF:",               M_RD_Uncapped,          0}, // КАДРОВАЯ ЧАСТОТА
    {ITT_LRFUNC, "FPS COUNTER:",              "CXTNXBR RFLHJDJQ XFCNJNS:",       M_RD_FPScounter,        0}, // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "PIXEL SCALING:",            "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing,         0}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_LRFUNC, "PORCH PALETTE CHANGING:",   "BPVTYTYBT GFKBNHS RHFTD 'RHFYF:", M_RD_PorchFlashing,     0}, // ИЗМЕНЕНИЕ ПАЛИТРЫ КРАЕВ ЭКРАНА
    {ITT_LRFUNC, "VIDEO RENDERER:",           "J,HF,JNRF DBLTJ:",                M_RD_Renderer,          0}, // ОБРАБОТКА ВИДЕО
    {ITT_TITLE,  "EXTRA",                     "LJGJKYBNTKMYJ",                   NULL,                   0}, // ДОПОЛНИТЕЛЬНО
    {ITT_LRFUNC, "SCREENSHOT FORMAT:",        "AJHVFN CRHBYIJNJD:",              M_RD_Screenshots,       0}  // ФОРМАТ СКРИНШОТОВ
};

static Menu_t RenderingMenu = {
    36, 36,
    32,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    10, RenderingItems, false,
    DrawRenderingMenu,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_TITLE,  "SCREEN",              "\'RHFY",                   NULL,                 0}, // ЭКРАН
    {ITT_LRFUNC, "SCREEN SIZE",         "HFPVTH BUHJDJUJ \'RHFYF",  M_RD_ScreenSize,      0}, // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC, "GAMMA-CORRECTION",    "EHJDTYM UFVVF-RJHHTRWBB",  M_RD_Gamma,           0}, // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC, "LEVEL BRIGHTNESS",    "EHJDTYM JCDTOTYYJCNB",     M_RD_LevelBrightness, 0}, // УРОВЕНЬ ОСВЕЩЕННОСТИ
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                 0},
    {ITT_TITLE,  "INTERFACE",           "BYNTHATQC",                NULL,                 0}, // ИНТЕРФЕЙС
    {ITT_LRFUNC, "LOCAL TIME:",         "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,       0}, // СИСТЕМНОЕ ВРЕМЯ
    {ITT_LRFUNC, "MESSAGES:",           "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,        0}, // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_LRFUNC, "TEXT CASTS SHADOWS:", "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText,    0}, // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_SETMENU,"AUTOMAP SETTINGS...", "YFCNHJQRB RFHNS>>>",       &AutomapMenu,         0}  // НАСТРОЙКИ КАРТЫ
};

static Menu_t DisplayMenu = {
    36, 36,
    32,
    "DISPLAY OPTIONS", "YFCNHJQRB \'RHFYF", false, // НАСТРОЙКИ ЭКРАНА
    12, DisplayItems, false,
    DrawDisplayMenu,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_LRFUNC, "OVERLAY MODE:", "HT;BV YFKJ;TYBZ:",  M_RD_AutoMapOverlay, 0}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_LRFUNC, "ROTATE MODE:",  "HT;BV DHFOTYBZ:",   M_RD_AutoMapRotate,  0}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_LRFUNC, "FOLLOW MODE:",  "HT;BV CKTLJDFYBZ:", M_RD_AutoMapFollow,  0}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_LRFUNC, "GRID:",         "CTNRF:",            M_RD_AutoMapGrid,    0}  // СЕТКА
};

static Menu_t AutomapMenu = {
    102, 82,
    32,
    "AUTOMAP SETTINGS", "YFCNHJQRB RFHNS", false, // НАСТРОЙКИ КАРТЫ
    4, AutomapItems, false,
    DrawAutomapMenu,
    NULL, 0,
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
    {ITT_SETMENU,"SOUND SYSTEM SETTINGS...", "YFCNHJQRB PDERJDJQ CBCNTVS>>>", &SoundSysMenu,    0}  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
};

static Menu_t SoundMenu = {
    36, 36,
    32,
    "SOUND OPTIONS", "YFCNHJQRB PDERF", false, // НАСТРОЙКИ ЗВУКА
    10, SoundItems, false,
    DrawSoundMenu,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    {ITT_TITLE,  "SOUND SYSTEM",          "PDERJDFZ CBCNTVF",           NULL,               0}, // ЗВУКВАЯ СИСТЕМА
    {ITT_LRFUNC, "SOUND EFFECTS:",        "PDERJDST \'AATRNS:",         M_RD_SoundDevice,   0}, // ЗВУКОВЫЕ ЭФФЕКТЫ:
    {ITT_LRFUNC, "MUSIC:",                "VEPSRF:",                    M_RD_MusicDevice,   0}, // МУЗЫКА:
    {ITT_TITLE,  "QUALITY",               "RFXTCNDJ PDEXFYBZ",          NULL,               0}, // КАЧЕСТВО ЗВУЧАНИЯ
    {ITT_LRFUNC, "SAMPLING FREQUENCY:",   "XFCNJNF LBCRHTNBPFWBB:",     M_RD_Sampling,      0}, // ЧАСТОТА ДИСКРЕТИЗАЦИИ:
    {ITT_TITLE,  "MISCELLANEOUS",         "HFPYJT",                     NULL,               0}, // РАЗНОЕ
    {ITT_LRFUNC, "SOUND EFFECTS MODE:",   "HT;BV PDERJDS[ \'AATRNJD:",  M_RD_SndMode,       0}, // РЕЖИМ ЗВУКОВЫХ ЭФФЕКТОВ
    {ITT_LRFUNC, "PITCH-SHIFTED SOUNDS:", "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting, 0}, // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
    {ITT_LRFUNC, "MUTE INACTIVE WINDOW:", "PDER D YTFRNBDYJV JRYT:",    M_RD_MuteInactive,  0}, // ЗВУК В НЕАКТИВНОМ ОКНЕ
};

static Menu_t SoundSysMenu = {
    36, 36,
    32,
    "SOUND SYSTEM SETTINGS", "YFCNHJQRB PDERJDJQ CBCNTVS", false, // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
    9, SoundSysItems, false,
    DrawSoundSystemMenu,
    NULL, 0,
    &SoundMenu,
    1
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_TITLE,  "MOVEMENT",           "GTHTLDB;TYBT",              NULL,             0}, // ПЕРЕДВИЖЕНИЕ
    {ITT_LRFUNC, "ALWAYS RUN:",        "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,   0}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_TITLE,  "MOUSE",              "VSIM",                      NULL,             0}, // МЫШЬ
    {ITT_LRFUNC, "MOUSE SENSIVITY",    "CRJHJCNM VSIB",             M_RD_Sensitivity, 0}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,  NULL,                 NULL,                        NULL,             0},
    {ITT_LRFUNC, "MOUSE LOOK:",        "J,PJH VSIM.:",              M_RD_MouseLook,   0}, // ОБЗОР МЫШЬЮ
    {ITT_LRFUNC, "INVERT Y AXIS:",     "DTHNBRFKMYFZ BYDTHCBZ:",    M_RD_InvertY,     0}, // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    {ITT_LRFUNC, "VERTICAL MOVEMENT:", "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,      0}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36, 36,
    32,
    "CONTROL SETTINGS", "EGHFDKTYBT", false, // УПРАВЛЕНИЕ
    8, ControlsItems, false,
    DrawControlsMenu,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gameplay features
// -----------------------------------------------------------------------------

static MenuItem_t GameplayItems[] = {
    {ITT_TITLE,  "VISUAL",               "UHFABRF",                       NULL,                 0}, // ГРАФИКА
    {ITT_LRFUNC, "BRIGHTMAPS:",          ",HFQNVFGGBYU:",                 M_RD_Brightmaps,      0}, // БРАЙТМАППИНГ
    {ITT_LRFUNC, "FAKE CONTRAST:",       "BVBNFWBZ RJYNHFCNYJCNB:",       M_RD_FakeContrast,    0}, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_TITLE,  "CROSSHAIR",            "GHBWTK",                        NULL,                 0}, // ПРИЦЕЛ
    {ITT_LRFUNC, "DRAW CROSSHAIR:",      "JNJ,HF;FNM GHBWTK:",            M_RD_CrossHairDraw,   0}, // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_LRFUNC, "INDICATION:",          "BYLBRFWBZ:",                    M_RD_CrossHairType,   0}, // ИНДИКАЦИЯ
    {ITT_LRFUNC, "INCREASED SIZE:",      "EDTKBXTYYSQ HFPVTH:",           M_RD_CrossHairScale,  0}, // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_TITLE,  "GAMEPLAY",             "UTQVGKTQ",                      NULL,                 0}, // ГЕЙМПЛЕЙ
    {ITT_LRFUNC, "FLIP GAME LEVELS:",    "PTHRFKMYJT JNHF;TYBT EHJDYTQ:", M_RD_FlipLevels,      0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_LRFUNC, "PLAY INTERNAL DEMOS:", "GHJBUHSDFNM LTVJPFGBCB:",       M_RD_NoDemos,         0}  // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
};

static Menu_t GameplayMenu = {
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    10, GameplayItems, false,
    DrawGameplayMenu,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Level select (1)
// -----------------------------------------------------------------------------

static MenuItem_t Level1Items[] = {
    {ITT_LRFUNC,  "CLASS:",             "RKFCC:",                M_RD_SelectiveClass,   0}, // КЛАСС
    {ITT_LRFUNC,  "SKILL LEVEL:",       "CKJ;YJCNM:",            M_RD_SelectiveSkill,   0}, // СЛОЖНОСТЬ
    {ITT_LRFUNC,  "HUB:",               "{F,:",                  M_RD_SelectiveHub,     0}, // ХАБ
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
    {ITT_SETMENU, "NEXT PAGE...",       "CKTLE.OFZ CNHFYBWF>>>", &LevelSelectMenu2_F,   0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",         "YFXFNM BUHE",           G_DoSelectiveGame,     0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu1 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level1Items, false,
    DrawLevelSelect1Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (2)
// -----------------------------------------------------------------------------

static MenuItem_t Level2Items_F[] = {
    {ITT_TITLE,   "WEAPONS",                "JHE:TT",                     NULL,                 0}, // ОРУЖЕЕ
    {ITT_LRFUNC,  "TIMON'S AXE:",           "NJGJH NBVJYF:",              M_RD_SelectiveWp_0,   0}, // ТОПОР ТИМОНА
    {ITT_LRFUNC,  "HAMMER OF RETRIBUTION:", "VJKJN DJPVTPLBZ:",           M_RD_SelectiveWp_1,   0}, // МОЛОТ ВОЗМЕЗДИЯ
    {ITT_LRFUNC,  "QUIETUS:",               "GJCKTLYBQ LJDJL:",           M_RD_SelectiveWp_2,   0}, // ПОСЛЕДНИЙ ДОВОД
    {ITT_LRFUNC,  "HANDLE OF QUIETUS:",     "HERJZNM GJCKTLYTUJ LJDJLF:", M_RD_SelectiveWp_P_0, 0}, // РУКОЯТЬ ПОСЛЕДНЕГО ДОВОДА
    {ITT_LRFUNC,  "GUARD OF QUIETUS:",      "UFHLF GJCKTLYTUJ LJDJLF:",   M_RD_SelectiveWp_P_1, 0}, // ГАРДА ПОСЛЕДНЕГО ДОВОДА
    {ITT_LRFUNC,  "BLADE OF QUIETUS:",      "KTPDBT GJCKTLYTUJ LJDJLF:",  M_RD_SelectiveWp_P_2, 0}, // ЛЕЗВИЕ ПОСЛЕДНЕГО ДОВОДА
    {ITT_TITLE,   "MANA",                   "VFYF",                       NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                  "CBYZZ:",                     M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                 "PTK~YFZ:",                   M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",              "FHNTAFRNS",                  NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",          "RDFHWTDSQ AKFRJY:",          M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",            "VBCNBXTCRFZ EHYF:",          M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",             "PTKMT:",                     M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                     NULL,                         NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE...",           "CKTLE.OFZ CNHFYBWF>>>",      &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",             "YFXFNM BUHE",                G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_F = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_F, false,
    DrawLevelSelect2Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Level2Items_C[] = {
    {ITT_TITLE,   "WEAPONS",               "JHE:TT",                  NULL,                 0}, // ОРУЖЕЕ
    {ITT_LRFUNC,  "SERPENT STAFF:",        "PVTBYSQ GJCJ[:",          M_RD_SelectiveWp_0,   0}, // ЗМЕИНЫЙ ПОСОХ
    {ITT_LRFUNC,  "FIRESTORM:",            "JUYTYYSQ INJHV:",         M_RD_SelectiveWp_1,   0}, // ОГНЕННЫЙ ШТОРМ
    {ITT_LRFUNC,  "WRAITHVERGE:",          ";TPK LE[JD:",             M_RD_SelectiveWp_2,   0}, // ЖЕЗЛ ДУХОВ
    {ITT_LRFUNC,  "POLE OF WRAITHVERGE:",  "LHTDRJ ;TPKF LE[JD:",     M_RD_SelectiveWp_P_0, 0}, // ДРЕВКО ЖЕЗЛА ДУХОВ
    {ITT_LRFUNC,  "CROSS OF WRAITHVERGE:", "RHTCNJDBYF ;TPKF LE[JD:", M_RD_SelectiveWp_P_1, 0}, // КРЕСТОВИНА ЖЕЗЛА ДУХОВ
    {ITT_LRFUNC,  "HEAD OF WRAITHVERGE:",  "YFDTHITYBT ;TPKF LE[JD:", M_RD_SelectiveWp_P_2, 0}, // НАВЕРШЕНИЕ ЖЕЗЛА ДУХОВ
    {ITT_TITLE,   "MANA",                  "VFYF",                    NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                 "CBYZZ:",                  M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                "PTK~YFZ:",                M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",             "FHNTAFRNS",               NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",         "RDFHWTDSQ AKFRJY:",       M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",           "VBCNBXTCRFZ EHYF:",       M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",            "PTKMT:",                  M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                    NULL,                      NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE...",          "CKTLE.OFZ CNHFYBWF>>>",   &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",            "YFXFNM BUHE",             G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_C = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_C, false,
    DrawLevelSelect2Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Level2Items_M[] = {
    {ITT_TITLE,   "WEAPONS",                 "JHE:TT",                      NULL,                 0}, // ОРУЖЕЕ
    {ITT_LRFUNC,  "FROST SHARDS:",           "KTLZYST JCRJKRB:",            M_RD_SelectiveWp_0,   0}, // ЛЕДЯНЫЕ ОСКОЛКИ
    {ITT_LRFUNC,  "ARC OF DEATH:",           "LEUF CVTHNB:",                M_RD_SelectiveWp_1,   0}, // ДУГА СМЕРТИ
    {ITT_LRFUNC,  "BLOODSCOURGE:",           "RHJDFDSQ ,BX:",               M_RD_SelectiveWp_2,   0}, // КРОВАВЫЙ БИЧ
    {ITT_LRFUNC,  "BINDER OF BLOODSCOURGE:", "GJLRJDTW RHJDFDJUJ ,BXF:",    M_RD_SelectiveWp_P_0, 0}, // ПОДКОВЕЦ КРОВАВОГО БИЧА
    {ITT_LRFUNC,  "SHAFT OF BLOODSCOURGE:",  "LHTDRJ RHJDFDJUJ ,BXF:",      M_RD_SelectiveWp_P_1, 0}, // ДРЕВКО КРОВАВОГО БИЧА
    {ITT_LRFUNC,  "KNOB OF BLOODSCOURGE:",   "YF,FKLFIYBR RHJDFDJUJ ,BXF:", M_RD_SelectiveWp_P_2, 0}, // НАБАЛДАШНИК КРОВАВОГО БИЧА
    {ITT_TITLE,   "MANA",                    "VFYF",                        NULL,                 0}, // МАНА
    {ITT_LRFUNC,  "BLUE:",                   "CBYZZ:",                      M_RD_SelectiveAmmo_0, 0}, // СИНЯЯ
    {ITT_LRFUNC,  "GREEN:",                  "PTK~YFZ:",                    M_RD_SelectiveAmmo_1, 0}, // ЗЕЛЁНАЯ
    {ITT_TITLE,   "ARTIFACTS",               "FHNTAFRNS",                   NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC,  "QUARTZ FLASK:",           "RDFHWTDSQ AKFRJY:",           M_RD_SelectiveArti_0, 0}, // КВАРЦЕВЫЙ ФЛАКОН
    {ITT_LRFUNC,  "MYSTIC URN:",             "VBCNBXTCRFZ EHYF:",           M_RD_SelectiveArti_1, 0}, // МИСТИЧЕСКАЯ УРНА
    {ITT_LRFUNC,  "FLECHETTE:",              "PTKMT:",                      M_RD_SelectiveArti_2, 0}, // ЗЕЛЬЕ
    {ITT_EMPTY,   NULL,                      NULL,                          NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE...",            "CKTLE.OFZ CNHFYBWF>>>",       &LevelSelectMenu3,    0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",              "YFXFNM BUHE",                 G_DoSelectiveGame,    0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu2_M = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items_M, false,
    DrawLevelSelect2Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
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
    {ITT_LRFUNC,   "EMERALD KEY:",          "BPEVHELYSQ RK.X:",           M_RD_SelectiveKey_0,   0}, // ИЗУМРУДНЫЙ КЛЮЧ
    {ITT_EMPTY,    NULL,                    NULL,                         NULL,                  0},
    {ITT_SETMENU,  "NEXT PAGE...",          "CKTLE.OFZ CNHFYBWF>>>",      &LevelSelectMenu4,     0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,   "START GAME",            "YFXFNM BUHE",                G_DoSelectiveGame,     0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu3 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level3Items, false,
    DrawLevelSelect3Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (4)
// -----------------------------------------------------------------------------

static MenuItem_t Level4Items[] = {
    {ITT_LRFUNC,  "SILVER KEY:",         "CTHT,HZYSQ RK.X:",        M_RD_SelectiveKey_1,     0}, // СЕРЕБРЯНЫЙ КЛЮЧ
    {ITT_LRFUNC,  "FIRE KEY:",           "JUYTYYSQ RK.X:",          M_RD_SelectiveKey_2,     0}, // ОГНЕННЫЙ КЛЮЧ
    {ITT_LRFUNC,  "STEEL KEY:",          "CNFKMYJQ RK.X:",          M_RD_SelectiveKey_3,     0}, // СТАЛЬНОЙ КЛЮЧ
    {ITT_LRFUNC,  "HORN KEY:",           "HJUJDJQ RK.X:",           M_RD_SelectiveKey_4,     0}, // РОГОВОЙ КЛЮЧ
    {ITT_LRFUNC,  "CAVE KEY:",           "GTOTHYSQ RK.X:",          M_RD_SelectiveKey_5,     0}, // ПЕЩЕРНЫЙ КЛЮЧ
    {ITT_LRFUNC,  "CASTLE KEY:",         "RK.X JN PFVRF:",          M_RD_SelectiveKey_6,     0}, // КЛЮЧ ОТ ЗАМКА
    {ITT_LRFUNC,  "SWAMP KEY:",          ",JKJNYSQ RK.X:",          M_RD_SelectiveKey_7,     0}, // БОЛОТНЫЙ КЛЮЧ
    {ITT_LRFUNC,  "RUSTED KEY:",         "H;FDSQ RK.X:",            M_RD_SelectiveKey_8,     0}, // РЖАВЫЙ КЛЮЧ
    {ITT_LRFUNC,  "DUNGEON KEY:",        "RK.X JN GJLPTVTKMZ:",     M_RD_SelectiveKey_9,     0}, // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
    {ITT_LRFUNC,  "AXE KEY:",            "RK.X-NJGJH:",             M_RD_SelectiveKey_10,    0}, // КЛЮЧ-ТОПОР
    {ITT_TITLE,   "PUZZLE ITEMS",        "GFPKJDST GHTLVTNS",       NULL,                    0}, // ПАЗЛОВЫЕ ПЕРЕДМЕТЫ
    {ITT_LRFUNC,  "FLAME MASK:",         "VFCRF GKFVTYB:",          M_RD_SelectivePuzzle_0,  0}, // МАСКА ПЛАМЕНИ
    {ITT_LRFUNC,  "HEART OF D'SPARIL:",  "CTHLWT L&CGFHBKF:",       M_RD_SelectivePuzzle_1,  0}, // СЕРДЦЕ Д'СПАРИЛА
    {ITT_LRFUNC,  "RUBY PLANET:",        "HE,BYJDFZ GKFYTNF:",      M_RD_SelectivePuzzle_2,  0}, // РУБИНОВАЯ ПЛАНЕТА
    {ITT_EMPTY,   NULL,                  NULL,                      NULL,                    0},
    {ITT_SETMENU, "LAST PAGE...",        "GJCKTLYZZ CNHFYBWF>>>",   &LevelSelectMenu5,       0}, // ПОСЛЕДНЯЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",          "YFXFNM BUHE",             G_DoSelectiveGame,       0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu4 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level4Items, false,
    DrawLevelSelect4Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
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
    {ITT_SETMENU, "FIRST PAGE...",       "GTHDFZ CNHFYBWF>>>",      &LevelSelectMenu1,       0}, // ПЕРВАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",          "YFXFNM BUHE",             G_DoSelectiveGame,       0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu5 = {
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level5Items, false,
    DrawLevelSelect5Menu,
    LevelSelectMenuPages, 5,
    &OptionsMenu,
    0
};

static MenuItem_t OptionsItems_Vanilla[] = {
    {ITT_EFUNC,   "END GAME",          "PFRJYXBNM BUHE",   SCEndGame,             0}, // ЗАКОНЧИТЬ ИГРУ
    {ITT_EFUNC,   "MESSAGES : ",        "CJJ,OTYBZ : ",    SCMessages,            0}, // СООБЩЕНИЯ:
    {ITT_LRFUNC,  "MOUSE SENSITIVITY", "CRJHJCNM VSIB",    M_RD_Sensitivity,      0}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,   NULL,                NULL,               NULL,                  0},
    {ITT_SETMENU, "MORE...",           "LJGJKYBNTKMYJ>>>", &Options2Menu_Vanilla, 0}  // ДОПОЛНИТЕЛЬНО...
};

static Menu_t OptionsMenu_Vanilla = {
    88, 88,
    30,
    NULL, NULL, true,
    5, OptionsItems_Vanilla, true,
    DrawOptionsMenu_Vanilla,
    NULL, 0,
    &MainMenu,
    0,
};

static MenuItem_t Options2Items_Vanilla[] = {
    {ITT_LRFUNC, "SCREEN SIZE",  "HFPVTH 'RHFYF",    M_RD_ScreenSize, 0}, // РАЗМЕР ЭКРАНА
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "SFX VOLUME",   "UHJVRJCNM PDERF",  M_RD_SfxVolume,  0}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "MUSIC VOLUME", "UHJVRJCNM VEPSRB", M_RD_MusVolume,  0}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0}
};

static Menu_t Options2Menu_Vanilla = {
    90, 90,
    20,
    NULL, NULL, true,
    6, Options2Items_Vanilla, true,
    DrawOptions2Menu_Vanilla,
    NULL, 0,
    &OptionsMenu_Vanilla,
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
    NULL, 0,
    &MainMenu,
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
    NULL, 0,
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
    NULL, 0,
    &FilesMenu,
    0
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

    RD_Menu_InitMenu(20, 10);

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
                        "INVGEMR1",
                        "INVGEMR2",
                        -1, -1, -28, -14);

    menuactive = false;
//      messageson = true;              // Set by defaults in .CFG
    MauloBaseLump = W_GetNumForName("FBULA0");  // ("M_SKL00");

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;
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
    // [Dasperal] Update Status bar.
    // [Dasperal] Once this line is deleted, Delete DrawOptionsMenu function and
    // replace it's references to NULL
    SB_state = -1;
}

// -----------------------------------------------------------------------------
// DrawRenderingMenu
// -----------------------------------------------------------------------------

static void DrawRenderingMenu(void)
{
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

        // Uncapped FPS
        RD_M_DrawTextSmallENG(uncapped_fps ? "UNCAPPED" : "35 FPS", 120 + wide_delta, 62, CR_NONE);

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

        // Video renderer
        RD_M_DrawTextSmallENG(force_software_renderer ? "SOFTWARE (CPU)" : "HARDWARE (GPU)",
                              149 + wide_delta, 102, CR_NONE);
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

        // Кадровая частота
        if (uncapped_fps)
            RD_M_DrawTextSmallRUS(",TP JUHFYBXTYBZ", 165 + wide_delta, 62, CR_NONE);
        else
            RD_M_DrawTextSmallENG("35 FPS", 165 + wide_delta, 62, CR_NONE);

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

        // Обработка видео
        RD_M_DrawTextSmallRUS(force_software_renderer ? "GHJUHFVVYFZ" : "FGGFHFNYFZ",
                              159 + wide_delta, 102, CR_NONE);
    }

    // Screenshot format / Формат скриншотов (same english values)
    RD_M_DrawTextSmallENG(png_screenshots ? "PNG" : "PCX", 175 + wide_delta, 122, CR_NONE);
}

static void M_RD_Change_Widescreen(Direction_t direction)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in MN_Init and stored into config file in M_QuitResponse.
    RD_Menu_SpinInt(&aspect_ratio_temp, 0, 4, direction);
}

static void M_RD_Change_VSync(Direction_t direction)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
        return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

static void M_RD_Uncapped(Direction_t direction)
{
    uncapped_fps ^= 1;
}

static void M_RD_FPScounter(Direction_t direction)
{
    show_fps ^= 1;
}

static void M_RD_Smoothing(Direction_t direction)
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

static void M_RD_PorchFlashing(Direction_t direction)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
}

static void M_RD_Renderer(Direction_t direction)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update status bar
    SB_state = -1;
}

static void M_RD_Screenshots(Direction_t direction)
{
    png_screenshots ^= 1;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    static char num[4];

    if (english_language)
    {
        // Local time
        RD_M_DrawTextSmallENG(local_time == 1 ? "12-HOUR (HH:MM)" :
                   local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                   local_time == 3 ? "24-HOUR (HH:MM)" :
                   local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF",
                   110 + wide_delta, 112, CR_NONE);

        // Messages
        RD_M_DrawTextSmallENG((messageson ? "ON" : "OFF"), 108 + wide_delta, 122, CR_NONE);

        // Text casts shadows
        RD_M_DrawTextSmallENG((draw_shadowed_text ? "ON" : "OFF"), 179 + wide_delta, 132, CR_NONE);
    }
    else
    {
        // Системное время
        RD_M_DrawTextSmallRUS(local_time == 1 ? "12-XFCJDJT (XX:VV)" :
                          local_time == 2 ? "12-XFCJDJT (XX:VV:CC)" :
                          local_time == 3 ? "24-XFCJDJT (XX:VV)" :
                          local_time == 4 ? "24-XFCJDJT (XX:VV:CC)" : "DSRK",
                          157 + wide_delta, 112, CR_NONE);

        // Отображение сообщений
        RD_M_DrawTextSmallRUS((messageson ? "DRK" : "DSRK"), 208 + wide_delta, 122, CR_NONE);

        // Тексты отбрасывают тень
        RD_M_DrawTextSmallRUS((draw_shadowed_text ? "DRK" : "DSRK"), 220 + wide_delta, 132, CR_NONE);
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

    // Gamma-correction
    RD_Menu_DrawSliderSmall(&DisplayMenu, 72, 18, usegamma);

    // Level brightness
    RD_Menu_DrawSliderSmall(&DisplayMenu, 92, 5, level_brightness / 16);
}

static void M_RD_ScreenSize(Direction_t direction)
{
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

static void M_RD_Gamma(Direction_t direction)
{
    RD_Menu_SlideInt(&usegamma, 0, 17, direction);

    I_SetPalette((byte *) W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE));

    P_SetMessage(&players[consoleplayer], english_language ?
                                          GammaText[usegamma] :
                                          GammaText_Rus[usegamma],
                                          false);
}

static void M_RD_Messages(Direction_t direction)
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

static void M_RD_LevelBrightness(Direction_t direction)
{
    RD_Menu_SlideInt_Step(&level_brightness, 0, 64, 16, direction);
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

static void M_RD_AutoMapOverlay(Direction_t direction)
{
    automap_overlay ^= 1;
}

static void M_RD_AutoMapRotate(Direction_t direction)
{
    automap_rotate ^= 1;
}

static void M_RD_AutoMapFollow(Direction_t direction)
{
    automap_follow ^= 1;
}

static void M_RD_AutoMapGrid(Direction_t direction)
{
    automap_grid ^= 1;
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char num[4];

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
        if (CurrentItPos == 0 || CurrentItPos == 1 || CurrentItPos == 3)
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

static void M_RD_SoundDevice(Direction_t direction)
{
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 3;
        else if (snd_sfxdevice == 3)
            snd_sfxdevice = 0;
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
}

static void M_RD_Sampling(Direction_t direction)
{
    RD_Menu_ShiftSpinInt(&snd_samplerate, 11025, 44100, direction);
}

static void M_RD_SndMode(Direction_t direction)
{
    snd_monomode ^= 1;
}

static void M_RD_PitchShifting(Direction_t direction)
{
    snd_pitchshift ^= 1;
}

static void M_RD_MuteInactive(Direction_t direction)
{
    mute_inactive_window ^= 1;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    static char num[4];

    if (english_language)
    {
        // Always run
        RD_M_DrawTextSmallENG(joybspeed >= 20 ? "ON" : "OFF", 118 + wide_delta, 42, CR_NONE);

        // Mouse look
        RD_M_DrawTextSmallENG(mlook ? "ON" : "OFF", 118 + wide_delta, 82, CR_NONE);

        // Invert Y axis
        RD_M_DrawTextSmallENG(mouse_y_invert ? "ON" : "OFF", 133 + wide_delta, 92,
                              !mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);

        // Novert
        RD_M_DrawTextSmallENG(!novert ? "ON" : "OFF", 168 + wide_delta, 102,
                              mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);
    }
    else
    {
        // Режим постоянного бега
        RD_M_DrawTextSmallRUS(joybspeed >= 20 ? "DRK" : "DSRK", 209 + wide_delta, 42, CR_NONE);

        // Обзор мышью
        RD_M_DrawTextSmallRUS(mlook ? "DRK" : "DSRK", 132 + wide_delta, 82, CR_NONE);

        // Вертикальная инверсия
        RD_M_DrawTextSmallRUS(mouse_y_invert ? "DRK" : "DSRK", 199 + wide_delta, 92,
                              !mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);

        // Вертикальное перемещение
        RD_M_DrawTextSmallRUS(!novert ? "DRK" : "DSRK", 227 + wide_delta, 102,
                              mlook ? CR_GRAY2GDARKGRAY_HEXEN : CR_NONE);
    }

    // Mouse sensivity
    RD_Menu_DrawSliderSmall(&ControlsMenu, 72, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 73, CR_GRAY2GDARKGRAY_HEXEN);
}

static void M_RD_AlwaysRun(Direction_t direction)
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

static void M_RD_MouseLook(Direction_t direction)
{
    mlook ^= 1;
    if (!mlook)
    players[consoleplayer].centering = true;
}

static void M_RD_Sensitivity(Direction_t direction)
{
    RD_Menu_SlideInt(&mouseSensitivity, 0, 255, direction); // [crispy] extended range
}

static void M_RD_InvertY(Direction_t direction)
{
    mouse_y_invert ^= 1;
}

static void M_RD_Novert(Direction_t direction)
{
    novert ^= 1;
}

// -----------------------------------------------------------------------------
// DrawGameplayMenu
// -----------------------------------------------------------------------------

static void DrawGameplayMenu(void)
{
    if (english_language)
    {
        // Brightmaps
        RD_M_DrawTextSmallENG(brightmaps ? "ON" : "OFF", 119 + wide_delta, 42,
                              brightmaps ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Fake contrast
        RD_M_DrawTextSmallENG(fake_contrast ? "ON" : "OFF", 143 + wide_delta, 52,
                              fake_contrast ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Draw crosshair
        RD_M_DrawTextSmallENG(crosshair_draw ? "ON" : "OFF", 150 + wide_delta, 72,
                              crosshair_draw ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Indication
        RD_M_DrawTextSmallENG(crosshair_type == 1 ? "HEALTH" : "STATIC",  111 + wide_delta, 82,
                              crosshair_type ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Increased size
        RD_M_DrawTextSmallENG(crosshair_scale ? "ON" : "OFF", 146 + wide_delta, 92,
                              crosshair_scale ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Flip game levels
        RD_M_DrawTextSmallENG(flip_levels ? "ON" : "OFF", 153 + wide_delta, 112,
                              flip_levels ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Play internal demos
        RD_M_DrawTextSmallENG(no_internal_demos ? "OFF" : "ON", 179 + wide_delta, 122,
                              no_internal_demos ? CR_GRAY2RED_HEXEN : CR_GRAY2GREEN_HEXEN);
    }
    else
    {
        // Брайтмаппинг
        RD_M_DrawTextSmallRUS(brightmaps ? "DRK" : "DSRK", 133 + wide_delta, 42,
                              brightmaps ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Имитация контрастности
        RD_M_DrawTextSmallRUS(fake_contrast ? "DRK" : "DSRK", 205 + wide_delta, 52,
                              fake_contrast ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Отображать прицел
        RD_M_DrawTextSmallRUS(crosshair_draw ? "DRK" : "DSRK", 175 + wide_delta, 72,
                              crosshair_draw ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Индикация
        RD_M_DrawTextSmallRUS(crosshair_type == 1 ? "PLJHJDMT" : // ЗДОРОВЬЕ
                                                         "CNFNBXYFZ", // СТАТИЧНАЯ
                              111 + wide_delta, 82, crosshair_type ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Увеличенный размер
        RD_M_DrawTextSmallRUS(crosshair_scale ? "DRK" : "DSRK", 181 + wide_delta, 92,
                              crosshair_scale ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Зеркальное отражение уровней
        RD_M_DrawTextSmallRUS(flip_levels ? "DRK" : "DSRK", 255 + wide_delta, 112,
                              flip_levels ? CR_GRAY2GREEN_HEXEN : CR_GRAY2RED_HEXEN);

        // Проигрывать демозаписи
        RD_M_DrawTextSmallRUS(no_internal_demos ? "DRK" : "DSRK", 211 + wide_delta, 122,
                              no_internal_demos ? CR_GRAY2RED_HEXEN : CR_GRAY2GREEN_HEXEN);
    }
}

static void M_RD_Brightmaps(Direction_t direction)
{
    brightmaps ^= 1;
}

static void M_RD_FakeContrast(Direction_t direction)
{
    fake_contrast ^= 1;
}

static void M_RD_ShadowedText(Direction_t direction)
{
    draw_shadowed_text ^= 1;
}

static void M_RD_CrossHairDraw(Direction_t direction)
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairType(Direction_t direction)
{
    crosshair_type ^= 1;
}

static void M_RD_CrossHairScale(Direction_t direction)
{
    crosshair_scale ^= 1;
}

static void M_RD_FlipLevels(Direction_t direction)
{
    flip_levels ^= 1;

    // [JN] Redraw game screen
    R_ExecuteSetViewSize();
}

static void M_RD_NoDemos(Direction_t direction)
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

static void M_RD_SelectiveWp_0(Direction_t direction)
{
    selective_wp_second ^= 1;
}

static void M_RD_SelectiveWp_1(Direction_t direction)
{
    selective_wp_third ^= 1;
}

static void M_RD_SelectiveWp_2(Direction_t direction)
{
    selective_wp_fourth ^= 1;

    if(selective_wp_fourth)
        selective_wp_piece_0 = selective_wp_piece_1 = selective_wp_piece_2 = 1;
    else
        selective_wp_piece_0 = selective_wp_piece_1 = selective_wp_piece_2 = 0;
}

static void M_RD_SelectiveWp_P_0(Direction_t direction)
{
    selective_wp_piece_0 ^= 1;

    if(selective_wp_piece_0 && selective_wp_piece_1 && selective_wp_piece_2)
        selective_wp_fourth = 1;
    else
        selective_wp_fourth = 0;
}

static void M_RD_SelectiveWp_P_1(Direction_t direction)
{
    selective_wp_piece_1 ^= 1;

    if(selective_wp_piece_0 && selective_wp_piece_1 && selective_wp_piece_2)
        selective_wp_fourth = 1;
    else
        selective_wp_fourth = 0;
}

static void M_RD_SelectiveWp_P_2(Direction_t direction)
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

static void M_RD_SelectiveKey_0(Direction_t direction)
{
    selective_key_0 ^= 1;
}

//---------------------------------------------------------------------------
// DrawLevelSelect4Menu
//---------------------------------------------------------------------------

static void DrawLevelSelect4Menu(void)
{
    char  num[4];

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

static void M_RD_SelectiveKey_1(Direction_t direction)
{
    selective_key_1 ^= 1;
}

static void M_RD_SelectiveKey_2(Direction_t direction)
{
    selective_key_2 ^= 1;
}

static void M_RD_SelectiveKey_3(Direction_t direction)
{
    selective_key_3 ^= 1;
}

static void M_RD_SelectiveKey_4(Direction_t direction)
{
    selective_key_4 ^= 1;
}

static void M_RD_SelectiveKey_5(Direction_t direction)
{
    selective_key_5 ^= 1;
}

static void M_RD_SelectiveKey_6(Direction_t direction)
{
    selective_key_6 ^= 1;
}

static void M_RD_SelectiveKey_7(Direction_t direction)
{
    selective_key_7 ^= 1;
}

static void M_RD_SelectiveKey_8(Direction_t direction)
{
    selective_key_8 ^= 1;
}

static void M_RD_SelectiveKey_9(Direction_t direction)
{
    selective_key_9 ^= 1;
}

static void M_RD_SelectiveKey_10(Direction_t direction)
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
        RD_M_DrawTextB(messageson ? "ON" : "OFF", 196 + wide_delta, 50);
    }
    else
    {
        RD_M_DrawTextBigRUS(messageson ? "DRK>" : "DSRK>", 223 + wide_delta, 50);	// ВКЛ. / ВЫКЛ.
    }

    RD_Menu_DrawSlider(&OptionsMenu_Vanilla, 92, 10, mouseSensitivity);
}

static void DrawOptions2Menu_Vanilla(void)
{
    if (aspect_ratio_temp >= 2)
    {
        RD_Menu_DrawSlider(&Options2Menu_Vanilla, 42, 4, screenblocks - 9);
    }
    else
    {
        RD_Menu_DrawSlider(&Options2Menu_Vanilla, 42, 10, screenblocks - 3);
    }
    RD_Menu_DrawSlider(&Options2Menu_Vanilla, 82, 16, snd_MaxVolume);
    RD_Menu_DrawSlider(&Options2Menu_Vanilla, 122, 16, snd_MusicVolume);
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
        x = (english_language ? SaveMenu.x_eng : SaveMenu.x_rus) + 1;
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
            SetMenu(&SkillMenu_F);
            break;
        case PCLASS_CLERIC:
            SetMenu(&SkillMenu_C);
            break;
        case PCLASS_MAGE:
            SetMenu(&SkillMenu_M);
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
    MenuItem_t *item;
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
        InfoType = (InfoType + 1) % 4;

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
                CurrentItPos = CurrentMenu->lastOn;
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
                CurrentItPos = CurrentMenu->lastOn;
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
            CurrentMenu = vanillaparm ? &Options2Menu_Vanilla : &SoundMenu;
            CurrentItPos = CurrentMenu->lastOn;
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
                    CurrentItPos = CurrentMenu->lastOn;
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
                    CurrentItPos = CurrentMenu->lastOn;
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
        return RD_Menu_Responder(key, charTyped);
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
    CurrentMenu = &MainMenu;
    CurrentItPos = CurrentMenu->lastOn;
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
        CurrentMenu->lastOn = CurrentItPos;
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

void SetMenu(const Menu_t* menu)
{
    CurrentMenu->lastOn = CurrentItPos;
    CurrentMenu = (Menu_t*) menu;
    CurrentItPos = CurrentMenu->lastOn;

    // [Dasperal] Force to use vanilla options menu in -vanilla game mode.
    if (vanillaparm)
    {
        if (CurrentMenu == &OptionsMenu)
        {
            CurrentMenu = &OptionsMenu_Vanilla;
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

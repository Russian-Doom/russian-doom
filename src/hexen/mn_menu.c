//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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

#include <ctype.h>
#include <SDL_scancode.h>

#include "rd_io.h"
#include "h2def.h"
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
#include "am_map.h"
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
static void DrawRenderingMenu1(void);
static void DrawRenderingMenu2(void);

// Page 1
static void M_RD_Change_Resolution(Direction_t direction);
static void M_RD_Change_Widescreen(Direction_t direction);
static void M_RD_Change_Renderer();
static void M_RD_Change_VSync();
static void M_RD_MaxFPS(Direction_t direction);
static void M_RD_PerfCounter(Direction_t direction);
static void M_RD_Smoothing();
static void M_RD_PorchFlashing();
static void M_RD_DiminishedLighting();

// Page2
static void M_RD_WindowBorder();
static void M_RD_WindowSize(Direction_t direction);
static void M_RD_WindowTitle();
static void M_RD_AlwaysOnTop();
static void M_RD_WindowAspectRatio();
static void M_RD_Screenshots();

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(Direction_t direction);
static void M_RD_LevelBrightness(Direction_t direction);
static void M_RD_BG_Detail();

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
static void M_RD_Change_Msg_Pickup_Color(Direction_t direction);
static void M_RD_Change_Msg_Quest_Color(Direction_t direction);
static void M_RD_Change_Msg_System_Color(Direction_t direction);
static void M_RD_Change_Msg_Chat_Color(Direction_t direction);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapRotate();
static void M_RD_AutoMapOverlay();
static void M_RD_AutoMapOverlayBG(Direction_t direction);
static void M_RD_AutoMapFollow();
static void M_RD_AutoMapGrid();
static void M_RD_AutoMapGridSize(Direction_t direction);
static void M_RD_AutomapMarkColor(Direction_t direction);

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
static void M_RD_SpeakerTest();
// Used for speaker test:
static boolean speaker_test_left = false;
static boolean speaker_test_right = false;
static int speaker_test_timeout;

// Controls
static void DrawControlsMenu(void);
static void M_RD_AlwaysRun();
static void M_RD_Artiskip();
static void M_RD_Sensitivity(Direction_t direction);
static void M_RD_Acceleration(Direction_t direction);
static void M_RD_Threshold(Direction_t direction);
static void M_RD_MouseLook();
static void M_RD_InvertY();
static void M_RD_Novert();

// Key Bindings
static void M_RD_Draw_Bindings();
static void DrawResetControlsMenu();
static void M_RD_ResetControls_Recommended();
static void M_RD_ResetControls_Original();

// Gamepad
static void InitControllerSelectMenu(struct Menu_s* menu);

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
static void M_RD_ExtraTrans();
static void M_RD_SwirlingLiquids();
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
static void M_RD_ShowArtiTimer(Direction_t direction);
static void M_RD_WeaponWidget(Direction_t direction);
static void M_RD_CrossHairDraw();
static void M_RD_CrossHairShape(Direction_t direction);
static void M_RD_CrossHairType();
static void M_RD_CrossHairScale();
static void M_RD_CrossHairOpacity(Direction_t direction);
int CrosshairShowcaseTimeout;

// Gameplay (page 3)
static void DrawGameplay3Menu(void);
static void M_RD_FixMapErrors();
static void M_RD_FlipLevels();
static void M_RD_NoDemos();
static void M_RD_Breathing();

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
static void DrawResetSettingsMenu(void);
static void M_RD_BackToDefaults_Recommended();
static void M_RD_BackToDefaults_Original();

// Change language
static void M_RD_ChangeLanguage(Direction_t direction);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------
extern int MapCount;
extern boolean alwaysRun;
extern sfxinfo_t S_sfx[];

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

static Translation_CR_t M_RD_ColorTranslation (int color)
{
    switch (color)
    {
        case 1:   return CR_WHITE;
        case 2:   return CR_GRAY;
        case 3:   return CR_DARKGRAY;
        case 4:   return CR_RED;
        case 5:   return CR_DARKRED;
        case 6:   return CR_GREEN;
        case 7:   return CR_DARKGREEN;
        case 8:   return CR_OLIVE;
        case 9:   return CR_BLUE2;
        case 10:  return CR_DARKBLUE;
        case 11:  return CR_NIAGARA;
        case 12:  return CR_BRIGHTYELLOW;
        case 13:  return CR_YELLOW;
        case 14:  return CR_TAN;
        case 15:  return CR_BROWN;
        default:  return CR_NONE;
    }
}

static char *M_RD_ColorName (int color)
{
    switch (color)
    {
        case 1:   return english_language ? "WHITE"      : ",TKSQ";            // БЕЛЫЙ
        case 2:   return english_language ? "GRAY"       : "CTHSQ";            // СЕРЫЙ
        case 3:   return english_language ? "DARK GRAY"  : "NTVYJ-CTHSQ";      // ТЁМНО-СЕРЫЙ
        case 4:   return english_language ? "RED"        : "RHFCYSQ";          // КРАСНЫЙ
        case 5:   return english_language ? "DARK RED"   : "NTVYJ-RHFCYSQ";    // ТЁМНО-КРАСНЫЙ
        case 6:   return english_language ? "GREEN"      : "PTKTYSQ";          // ЗЕЛЕНЫЙ
        case 7:   return english_language ? "DARK GREEN" : "NTVYJ-PTKTYSQ";    // ТЕМНО-ЗЕЛЕНЫЙ
        case 8:   return english_language ? "OLIVE"      : "JKBDRJDSQ";        // ОЛИВКОВЫЙ
        case 9:   return english_language ? "BLUE"       : "CBYBQ";            // СИНИЙ
        case 10:  return english_language ? "DARK BLUE"  : "NTVYJ-CBYBQ";      // ТЕМНО-СИНИЙ
        case 11:  return english_language ? "NIAGARA"    : "YBFUFHF";          // НИАГАРА
        case 12:  return english_language ? "YELLOW"     : ";TKNSQ";           // ЖЕЛТЫЙ
        case 13:  return english_language ? "GOLD"       : "PJKJNJQ";          // ЗОЛОТОЙ
        case 14:  return english_language ? "TAN"        : ",T;TDSQ";          // БЕЖЕВЫЙ
        case 15:  return english_language ? "BROWN"      : "RJHBXYTDSQ";       // КОРИЧНЕВЫЙ
        default:  return english_language ? "UNCOLORED"  : ",TP JRHFIBDFYBZ";  // БЕЗ ОКРАШИВАНИЯ
    }
}

// [JN] Used as a timer for hiding menu background
// while changing screen size, gamma and level brightness.
static int menubgwait;

// [JN] If true, print custom title "QUICK SAVING / LOADING" in files menu.
static boolean QuickSaveTitle;
static boolean QuickLoadTitle;

// [JN] Used as a flag for drawing Sound / Sound System menu background:
// - if menu was invoked by F4, don't draw background.
// - if menu was invoked from Options menu, draw background.
static boolean sfxbgdraw;

boolean askforquit;
static int typeofask;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static char SlotText[7][SLOTTEXTLEN + 2];
static char oldSlotText[SLOTTEXTLEN + 2];
static int SlotStatus[7];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;

//[Dasperal] Predeclare menu variables to allow referencing them before they initialized
static Menu_t* OptionsMenu;
static Menu_t ClassMenu;
static Menu_t RDOptionsMenu;
static Menu_t Rendering1Menu;
static Menu_t Rendering2Menu;
static const Menu_t* RenderingMenuPages[] = {&Rendering1Menu, &Rendering2Menu};
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
static Menu_t Bindings8Menu;
static const Menu_t* BindingsMenuPages[] = {&Bindings1Menu, &Bindings2Menu, &Bindings3Menu, &Bindings4Menu, &Bindings5Menu, &Bindings6Menu, &Bindings7Menu, &Bindings8Menu};
static Menu_t ResetControlsMenu;
static Menu_t GamepadSelectMenu;
static Menu_t Gamepad1Menu;
static Menu_t Gamepad2Menu;
static const Menu_t* GamepadMenuPages[] = {&Gamepad1Menu, &Gamepad2Menu};
static Menu_t Gameplay1Menu;
static Menu_t Gameplay2Menu;
static Menu_t Gameplay3Menu;
static const Menu_t* GameplayMenuPages[] = {&Gameplay1Menu, &Gameplay2Menu, &Gameplay3Menu};
static Menu_t LevelSelectMenu1;
static Menu_t LevelSelectMenu2_F;
static Menu_t LevelSelectMenu2_C;
static Menu_t LevelSelectMenu2_M;
static Menu_t LevelSelectMenu3;
static Menu_t LevelSelectMenu4;
static Menu_t LevelSelectMenu5;
static const Menu_t* LevelSelectMenuPages[] = {&LevelSelectMenu1, &LevelSelectMenu2_F, &LevelSelectMenu3, &LevelSelectMenu4, &LevelSelectMenu5};
static Menu_t ResetSettings;
static Menu_t VanillaOptions2Menu;
static Menu_t FilesMenu;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

static MenuItem_t HMainItems[] = {
    I_SETMENU_NONET("NEW GAME",   "YJDFZ BUHF", &ClassMenu, 1), // НОВАЯ ИГРА
    I_SETMENU(      "OPTIONS",    "YFCNHJQRB",  &RDOptionsMenu), // НАСТРОЙКИ
    I_SETMENU(      "GAME FILES", "AFQKS BUHS", &FilesMenu), // ФАЙЛЫ ИГРЫ
    I_EFUNC(        "INFO",       "BYAJHVFWBZ", SCInfo,     0), // ИНФОРМАЦИЯ
    I_EFUNC(        "QUIT GAME",  "DS[JL",      SCQuitGame, 0)  // ВЫХОД
};

MENU_STATIC(HMainMenu,
    110, 104,
    56,
    NULL, NULL, true,
    HMainItems, true,
    DrawMainMenu,
    NULL
);

static MenuItem_t ClassItems[] = {
    I_EFUNC("FIGHTER", "DJBY",      SCClass, 0), // ВОИН
    I_EFUNC("CLERIC",  "RKBHBR",    SCClass, 1), // КЛИРИК
    I_EFUNC("MAGE",    "VFU",       SCClass, 2), // МАГ
    I_EFUNC("RANDOM",  "CKEXFQYSQ", SCClass, 4), // СЛУЧАЙНЫЙ
};

MENU_STATIC(ClassMenu,
    65, 43,
    50,
    NULL, NULL, true,
    ClassItems, true,
    DrawClassMenu,
    &HMainMenu
);

static MenuItem_t SkillItems_F[] = {
    I_EFUNC("SQUIRE",    "JHE;TYJCTW", SCSkill, sk_baby),      // ОРУЖЕНОСЕЦ
    I_EFUNC("KNIGHT",    "HSWFHM",     SCSkill, sk_easy),      // РЫЦАРЬ
    I_EFUNC("WARRIOR",   "DJBNTKM",    SCSkill, sk_medium),    // ВОИТЕЛЬ
    I_EFUNC("BERSERKER", ",THCTHR",    SCSkill, sk_hard),      // БЕРСЕРК
    I_EFUNC("TITAN",     "NBNFY",      SCSkill, sk_nightmare), // ТИТАН
    I_EFUNC("AVATAR",    "DTHIBNTKM",  SCSkill, sk_ultranm)    // ВЕРШИТЕЛЬ
};

MENU_STATIC_SKILL(SkillMenu_F,
    115, 99,
    30,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    SkillItems_F, true,
    DrawSkillMenu,
    &ClassMenu,
    2
);

static MenuItem_t SkillItems_C[] = {
    I_EFUNC("ALTAR BOY", "FKNFHYBR",  SCSkill, sk_baby),      // АЛТАРНИК
    I_EFUNC("ACOLYTE",   "CKE;BNTKM", SCSkill, sk_easy),      // СЛУЖИТЕЛЬ
    I_EFUNC("PRIEST",    "CDZOTYYBR", SCSkill, sk_medium),    // СВЯЩЕННИК
    I_EFUNC("CARDINAL",  "RFHLBYFK",  SCSkill, sk_hard),      // КАРДИНАЛ
    I_EFUNC("POPE",      "TGBCRJG",   SCSkill, sk_nightmare), // ЕПИСКОП
    I_EFUNC("APOSTLE",   "FGJCNJK",   SCSkill, sk_ultranm)    // АПОСТОЛ
};

MENU_STATIC_SKILL(SkillMenu_C,
    118, 102,
    30,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    SkillItems_C, true,
    DrawSkillMenu,
    &ClassMenu,
    2
);

static MenuItem_t SkillItems_M[] = {
    I_EFUNC("APPRENTICE",     "EXTYBR",          SCSkill, sk_baby),      // УЧЕНИК
    I_EFUNC("ENCHANTER",      "XFHJLTQ",         SCSkill, sk_easy),      // ЧАРОДЕЙ
    I_EFUNC("SORCERER",       "RJKLEY",          SCSkill, sk_medium),    // КОЛДУН
    I_EFUNC("WARLOCK",        "XTHYJRYB;YBR",    SCSkill, sk_hard),      // ЧЕРНОКНИЖНИК
    I_EFUNC("HIGHER MAGE",    "DTH[JDYSQ VFU",   SCSkill, sk_nightmare), // ВЕРХОВНЫЙ МАГ
    I_EFUNC("GREAT ARCHMAGE", "DTKBRBQ FH[BVFU", SCSkill, sk_ultranm)    // ВЕЛИКИЙ АРХИМАГ
};

MENU_STATIC_SKILL(SkillMenu_M,
    87, 69,
    30,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    SkillItems_M, true,
    DrawSkillMenu,
    &ClassMenu,
    2
);

static MenuItem_t SkillItems_R[] = {
    I_EFUNC("THOU NEEDETH A WET-NURSE",       "YZYTXRF YFLJ,YF VYT",    SCSkill, sk_baby),      // НЯНЕЧКА НАДОБНА МНЕ
    I_EFUNC("YELLOWBELLIES-R-US",             "YT CNJKM VE;TCNDTYTY Z", SCSkill, sk_easy),      // НЕ СТОЛЬ МУЖЕСТВЕНЕН Я
    I_EFUNC("BRINGEST THEM ONETH",            "GJLFQNT VYT B[",         SCSkill, sk_medium),    // ПОДАЙТЕ МНЕ ИХ
    I_EFUNC("THOU ART A SMITE-MEISTER",       "BCREITY Z CHF;TYBZVB",   SCSkill, sk_hard),      // ИСКУШЕН Я СРАЖЕНИЯМИ
    I_EFUNC("BLACK PLAGUE POSSESSES THEE",    "XEVF JDKFLTKF VYJQ",     SCSkill, sk_nightmare), // ЧУМА ОВЛАДЕЛА МНОЙ
    I_EFUNC("QUICKETH ART THEE, FOUL WRAITH", "RJIVFHJV BCGJKYTY Z",    SCSkill, sk_ultranm)    // КОШМАРОМ ИСПОЛНЕН Я // [JN] Thanks to Jon Dowland for this :)
};

MENU_STATIC_SKILL(SkillMenu_R,
    38, 38,
    30,
    "CHOOSE SKILL LEVEL:", "EHJDTYM CKJ;YJCNB:", true, // УРОВЕНЬ СЛОЖНОСТИ:
    SkillItems_R, true,
    DrawSkillMenu,
    &ClassMenu,
    2
);

// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t RDOptionsItems[] = {
    I_SETMENU("RENDERING",         "DBLTJ",          &Rendering1Menu), // ВИДЕО
    I_SETMENU("DISPLAY",           "\'RHFY",         &DisplayMenu), // ЭКРАН
    I_SETMENU("SOUND",             "FELBJ",          &SoundMenu), // АУДИО
    I_SETMENU("CONTROLS",          "EGHFDKTYBT",     &ControlsMenu), // УПРАВЛЕНИЕ
    I_SETMENU("GAMEPLAY",          "UTQVGKTQ",       &Gameplay1Menu), // ГЕЙМПЛЕЙ
    I_SETMENU("LEVEL SELECT",      "DS,JH EHJDYZ",   &LevelSelectMenu1), // ВЫБОР УРОВНЯ
    I_SETMENU("RESET SETTINGS",    "C,HJC YFCNHJTR", &ResetSettings), // СБРОС НАСТРОЕК
    I_LRFUNC( "LANGUAGE: ENGLISH", "ZPSR: HECCRBQ",  M_RD_ChangeLanguage)  // ЯЗЫК: РУССКИЙ
};

MENU_STATIC(RDOptionsMenu,
    77, 77,
    31,
    "OPTIONS", "YFCNHJQRB", false,
    RDOptionsItems, true,
    DrawOptionsMenu,
    &HMainMenu
);

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static const PageDescriptor_t RenderingDescriptor = {
    2, RenderingMenuPages,
    252, 172,
    CR_GRAY
};

static MenuItem_t Rendering1Items[] = {
    I_TITLE( "RENDERING",                 "HTYLTHBYU"), // РЕНДЕРИНГ
    I_LRFUNC("RENDERING RESOLUTION:",     "HFPHTITYBT HTYLTHBYUF:",          M_RD_Change_Resolution),  // РАЗРЕШЕНИЕ РЕНДЕРИНГА
    I_LRFUNC("DISPLAY ASPECT RATIO:",     "CJJNYJITYBT CNJHJY \'RHFYF:",     M_RD_Change_Widescreen),  // СООТНОШЕНИЕ СТОРОН ЭКРАНА
    I_LRFUNC("SCREEN RENDERER:",          "HTYLTHTH \'RHFYF:",               M_RD_Change_Renderer),    // РЕНДЕРЕР ЭКРАНА
    I_SWITCH("VERTICAL SYNCHRONIZATION:", "DTHNBRFKMYFZ CBY[HJYBPFWBZ:",     M_RD_Change_VSync),       // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    I_LRFUNC("FPS LIMIT:",                "JUHFYBXTYBT",                     M_RD_MaxFPS),             // ОГРАНИЧЕНИЕ FPS
    I_LRFUNC("PERFORMANCE COUNTER:",      "CXTNXBR GHJBPDJLBNTKMYJCNB:",     M_RD_PerfCounter),        // СЧЕТЧИК ПРОИЗВОДИТЕЛЬНОСТИ
    I_SWITCH("PIXEL SCALING:",            "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing),          // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    I_SWITCH("PORCH PALETTE CHANGING:",   "BPVTYTYBT GFKBNHS RHFTD 'RHFYF:", M_RD_PorchFlashing),      // ИЗМЕНЕНИЕ ПАЛИТРЫ КРАЕВ ЭКРАНА
    I_SWITCH("DIMINISHED LIGHTING:",      "EUFCFYBT JCDTOTYBZ:",             M_RD_DiminishedLighting), // УГАСАНИЕ ОСВЕЩЕНИЯ
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "LFKTT `", &Rendering2Menu),  // ДАЛЕЕ >
    I_EMPTY
};

MENU_STATIC_PAGED(Rendering1Menu,
    36, 36,
    32,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    Rendering1Items, false,
    DrawRenderingMenu1,
    &RDOptionsMenu,
    &RenderingDescriptor
);

static MenuItem_t Rendering2Items[] = {
    I_TITLE( "WINDOW OPTIONS",                "YFCNHJQRB JRYF"),                               // НАСТРОЙКИ ОКНА
    I_SWITCH("BORDERED WINDOW:",              "JRYJ C HFVRJQ:",      M_RD_WindowBorder),       // ОКНО С РАМКОЙ
    I_LRFUNC("WINDOW SIZE:",                  "HFPVTH JRYF:",        M_RD_WindowSize),         // РАЗМЕР ОКНА
    I_SWITCH("WINDOW TITLE:",                 "PFUJKJDJR JRYF:",     M_RD_WindowTitle),        // ЗАГОЛОВОК ОКНА
    I_SWITCH("ALWAYS ON TOP:",                "GJDTH[ LHEUB[ JRJY:", M_RD_AlwaysOnTop),        // ПОВЕРХ ДРУГИХ ОКОН
    I_SWITCH("PRESERVE WINDOW ASPECT RATIO:", "GHJGJHWBB JRYF:",     M_RD_WindowAspectRatio),  // ПРОПОРЦИИ ОКНА
    I_TITLE( "EXTRA",                         "LJGJKYBNTKMYJ"),                                // ДОПОЛНИТЕЛЬНО
    I_SWITCH("SCREENSHOT FORMAT:",            "AJHVFN CRHBYIJNJD:",  M_RD_Screenshots),        // ФОРМАТ СКРИНШОТОВ
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("< PREV PAGE", "^ YFPFL", &Rendering1Menu),  // < НАЗАД
    I_EMPTY
};

MENU_STATIC_PAGED(Rendering2Menu,
    36, 36,
    32,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    Rendering2Items, false,
    DrawRenderingMenu2,
    &RDOptionsMenu,
    &RenderingDescriptor
);


// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    I_TITLE(  "SCREEN",                  "\'RHFY"), // ЭКРАН
    I_LRFUNC( "SCREEN SIZE",             "HFPVTH BUHJDJUJ \'RHFYF", M_RD_ScreenSize), // РАЗМЕР ИГРОВОГО ЭКРАНА
    I_EMPTY,
    I_LRFUNC( "LEVEL BRIGHTNESS",        "EHJDTYM JCDTOTYYJCNB",    M_RD_LevelBrightness), // УРОВЕНЬ ОСВЕЩЕННОСТИ
    I_EMPTY,
    I_SWITCH( "BACKGROUND DETAIL:",      "LTNFKBPFWBZ AJYF:",       M_RD_BG_Detail), // ДЕТАЛИЗАЦИЯ ФОНА
    I_SETMENU("COLOR OPTIONS...",        "YFCNHJQRB WDTNF>>>",      &ColorMenu), // НАСТРОЙКИ ЦВЕТА...
    I_TITLE(  "INTERFACE",               "BYNTHATQC"), // ИНТЕРФЕЙС
    I_SETMENU("MESSAGES AND TEXTS...",   "CJJ,OTYBZ B NTRCNS>>>",   &MessagesMenu), // СООБЩЕНИЯ И ТЕКСТЫ...
    I_SETMENU("AUTOMAP SETTINGS...",     "YFCNHJQRB RFHNS>>>",      &AutomapMenu)  // НАСТРОЙКИ КАРТЫ...
};

MENU_STATIC(DisplayMenu,
    36, 36,
    32,
    "DISPLAY OPTIONS", "YFCNHJQRB \'RHFYF", false, // НАСТРОЙКИ ЭКРАНА
    DisplayItems, false,
    DrawDisplayMenu,
    &RDOptionsMenu
);

// -----------------------------------------------------------------------------
// Color options
// -----------------------------------------------------------------------------

static MenuItem_t ColorItems[] = {
    I_LRFUNC("",  "", M_RD_Brightness), // Brightness | Яркость
    I_LRFUNC("",  "", M_RD_Gamma), // Gamma | Гамма
    I_LRFUNC("",  "", M_RD_Saturation), // Saturation | Насыщенность
    I_SWITCH("",  "", M_RD_ShowPalette), // Show palette | Отобразить палитру
    I_TITLE( "",  ""), // Color intensity | Цветовая интенсивность
    I_LRFUNC("",  "", M_RD_RED_Color),
    I_LRFUNC("",  "", M_RD_GREEN_Color),
    I_LRFUNC("",  "", M_RD_BLUE_Color)
};

MENU_STATIC(ColorMenu,
    164, 164,
    25,
    "COLOR OPTIONS", "YFCNHJQRF WDTNF", false,  // НАСТРОЙКИ ЦВЕТА
    ColorItems, false,
    DrawColorMenu,
    &DisplayMenu
);

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

static MenuItem_t MessagesItems[] = {
    I_TITLE( "GENERAL",             "JCYJDYJT"), // ОСНОВНОЕ
    I_SWITCH("MESSAGES:",           "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages), // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    I_LRFUNC("ALIGNMENT:",          "DSHFDYBDFYBT:",            M_RD_MessagesAlignment), // ВЫРАВНИВАНИЕ
    I_LRFUNC("MESSAGE TIMEOUT",     "NFQVFEN JNJ,HF;TYBZ",      M_RD_MessagesTimeout), // ТАЙМАУТ ОТОБРАЖЕНИЯ
    I_EMPTY,
    I_SWITCH("FADING EFFECT:",      "GKFDYJT BCXTPYJDTYBT:",    M_RD_MessagesFade), // ПЛАВНОЕ ИСЧЕЗНОВЕНИЕ
    I_SWITCH("TEXT CASTS SHADOWS:", "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText), // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    I_TITLE( "MISC",                "HFPYJT"), // РАЗНОЕ
    I_LRFUNC("LOCAL TIME:",         "CBCNTVYJT DHTVZ:",         M_RD_LocalTime), // СИСТЕМНОЕ ВРЕМЯ
    I_TITLE( "COLORS",              "WDTNF"), // ЦВЕТА
    I_LRFUNC("ITEM PICKUP:",        "GJKEXTYBT GHTLVTNJD:",     M_RD_Change_Msg_Pickup_Color), // ПОЛУЧЕНИЕ ПРЕДМЕТОВ
    I_LRFUNC("QUEST MESSAGE:",      "RDTCNJDST CJJ,OTYBZ:",     M_RD_Change_Msg_Quest_Color), // ОБНАРУЖЕНИЕ ТАЙНИКОВ
    I_LRFUNC("SYSTEM MESSAGE:",     "CBCNTVYST CJJ,OTYBZ:",     M_RD_Change_Msg_System_Color), // СИСТЕМНЫЕ СООБЩЕНИЯ
    I_LRFUNC("NETGAME CHAT:",       "XFN CTNTDJQ BUHS:",        M_RD_Change_Msg_Chat_Color)  // ЧАТ СЕТЕВОЙ ИГРЫ
};

MENU_STATIC(MessagesMenu,
    36, 36,
    32,
    "MESSAGES AND TEXTS", "CJJ,OTYBZ B NTRCNS", false, // СООБЩЕНИЯ И ТЕКСТЫ
    MessagesItems, false,
    DrawMessagesMenu,
    &DisplayMenu
);

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    I_SWITCH("ROTATE MODE:",               "HT;BV DHFOTYBZ:",                 M_RD_AutoMapRotate), // РЕЖИМ ВРАЩЕНИЯ
    I_SWITCH("OVERLAY MODE:",              "HT;BV YFKJ;TYBZ:",                M_RD_AutoMapOverlay), // РЕЖИМ НАЛОЖЕНИЯ
    I_LRFUNC("OVERLAY BACKGROUND OPACITY", "GHJPHFXYJCNM AJYF GHB YFKJ;TYBB", M_RD_AutoMapOverlayBG), // ПРОЗРАЧНОСТЬ ФОНА ПРИ НАЛОЖЕНИИ
    I_EMPTY,
    I_SWITCH("FOLLOW MODE:",               "HT;BV CKTLJDFYBZ:",               M_RD_AutoMapFollow), // РЕЖИМ СЛЕДОВАНИЯ
    I_SWITCH("GRID:",                      "CTNRF:",                          M_RD_AutoMapGrid), // СЕТКА
    I_LRFUNC("GRID SIZE:",                 "HFPVTH CTNRB:",                   M_RD_AutoMapGridSize), // РАЗМЕР СЕТКИ
    I_LRFUNC("MARK COLOR:",                "WDTN JNVTNJR:",                   M_RD_AutomapMarkColor), // ЦВЕТ ОТМЕТОК
};

MENU_STATIC(AutomapMenu,
    36, 36,
    32,
    "AUTOMAP SETTINGS", "YFCNHJQRB RFHNS", false, // НАСТРОЙКИ КАРТЫ
    AutomapItems, false,
    DrawAutomapMenu,
    &DisplayMenu
);

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    I_TITLE(  "VOLUME",                   "UHJVRJCNM"), // ГРОМКОСТЬ
    I_LRFUNC( "SFX VOLUME",               "UHJVRJCNM PDERF",               M_RD_SfxVolume), // ГРОМКОСТЬ ЗВУКА
    I_EMPTY,
    I_LRFUNC( "MUSIC VOLUME",             "UHJVRJCNM VEPSRB",              M_RD_MusVolume), // ГРОМКОСТЬ МУЗЫКИ
    I_EMPTY,
    I_TITLE(  "CHANNELS",                 "DJCGHJBPDTLTYBT"), // ВОСПРОИЗВЕДЕНИЕ
    I_LRFUNC( "SFX CHANNELS",             "PDERJDST RFYFKS",               M_RD_SfxChannels), // ЗВУКОВЫЕ КАНАЛЫ
    I_EMPTY,
    I_TITLE(  "ADVANCED",                 "LJGJKYBNTKMYJ"), // ДОПОЛНИТЕЛЬНО
    I_SETMENU("SOUND SYSTEM SETTINGS...", "YFCNHJQRB PDERJDJQ CBCNTVS>>>", &SoundSysMenu)  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ...
};

MENU_STATIC(SoundMenu,
    36, 36,
    32,
    "SOUND OPTIONS", "YFCNHJQRB PDERF", false, // НАСТРОЙКИ ЗВУКА
    SoundItems, false,
    DrawSoundMenu,
    &RDOptionsMenu
);

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    I_TITLE( "SOUND SYSTEM",          "PDERJDFZ CBCNTVF"), // ЗВУКВАЯ СИСТЕМА
    I_SWITCH("SOUND EFFECTS:",        "PDERJDST \'AATRNS:",         M_RD_SoundDevice), // ЗВУКОВЫЕ ЭФФЕКТЫ:
    I_LRFUNC("MUSIC:",                "VEPSRF:",                    M_RD_MusicDevice), // МУЗЫКА:
    I_TITLE( "QUALITY",               "RFXTCNDJ PDEXFYBZ"), // КАЧЕСТВО ЗВУЧАНИЯ
    I_LRFUNC("SAMPLING FREQUENCY:",   "XFCNJNF LBCRHTNBPFWBB:",     M_RD_Sampling), // ЧАСТОТА ДИСКРЕТИЗАЦИИ:
    I_TITLE( "MISCELLANEOUS",         "HFPYJT"), // РАЗНОЕ
    I_LRFUNC("SPEAKER TEST",          "NTCN PDERJDS[ RFYFKJD",      M_RD_SpeakerTest), // ТЕСТ ЗВУКОВЫХ КАНАЛОВ
    I_SWITCH("SOUND EFFECTS MODE:",   "HT;BV PDERJDS[ \'AATRNJD:",  M_RD_SndMode), // РЕЖИМ ЗВУКОВЫХ ЭФФЕКТОВ
    I_SWITCH("PITCH-SHIFTED SOUNDS:", "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting), // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
    I_SWITCH("MUTE INACTIVE WINDOW:", "PDER D YTFRNBDYJV JRYT:",    M_RD_MuteInactive), // ЗВУК В НЕАКТИВНОМ ОКНЕ
};

MENU_STATIC(SoundSysMenu,
    36, 36,
    32,
    "SOUND SYSTEM SETTINGS", "YFCNHJQRB PDERJDJQ CBCNTVS", false, // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
    SoundSysItems, false,
    DrawSoundSystemMenu,
    &SoundMenu
);

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    I_TITLE(  "CONTROLS",                              "EGHFDKTYBT"), // УПРАВЛЕНИЕ
    I_SETMENU("CUSTOMIZE CONTROLS...",                 "YFCNHJQRB EGHFDKTYBZ>>>",          &Bindings1Menu), // Настройки управления...
    I_SETMENU("GAMEPAD SETTINGS...",                   "YFCNHJQRB UTQVGFLF>>>",            &GamepadSelectMenu), // Настройки геймпада...
    I_SWITCH( "ALWAYS RUN:",                           "HT;BV GJCNJZYYJUJ ,TUF:",          M_RD_AlwaysRun), // РЕЖИМ ПОСТОЯННОГО БЕГА
    I_SWITCH( "SKIP ARTEFACT ON 'USE' WHILE RUNNING:", "CRBG FHNTAFRNF YF BCG> GHB ,TUT:", M_RD_Artiskip), // СКИП АРТЕФАКТА НА ИСП. ПРИ БЕГЕ
    I_TITLE(  "MOUSE",                                 "VSIM"), // МЫШЬ
    I_LRFUNC( "MOUSE SENSIVITY",                       "CRJHJCNM VSIB",                    M_RD_Sensitivity), // СКОРОСТЬ МЫШИ
    I_EMPTY,
    I_LRFUNC( "ACCELERATION",                          "FRCTKTHFWBZ",                      M_RD_Acceleration), // АКСЕЛЕРАЦИЯ
    I_EMPTY,
    I_LRFUNC( "ACCELERATION THRESHOLD",                "GJHJU FRCTKTHFWBB",                M_RD_Threshold), // ПОРОГ АКСЕЛЕРАЦИИ
    I_EMPTY,
    I_SWITCH( "MOUSE LOOK:",                           "J,PJH VSIM.:",                     M_RD_MouseLook), // ОБЗОР МЫШЬЮ
    I_SWITCH( "INVERT Y AXIS:",                        "DTHNBRFKMYFZ BYDTHCBZ:",           M_RD_InvertY), // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    I_SWITCH( "VERTICAL MOVEMENT:",                    "DTHNBRFKMYJT GTHTVTOTYBT:",        M_RD_Novert)  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

MENU_STATIC(ControlsMenu,
    36, 36,
    32,
    "CONTROL SETTINGS", "EGHFDKTYBT", false, // УПРАВЛЕНИЕ
    ControlsItems, false,
    DrawControlsMenu,
    &RDOptionsMenu
);

// -----------------------------------------------------------------------------
// Key bindings (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t BindingsPageDescriptor = {
    8, BindingsMenuPages,
    252, 165,
    CR_WHITE
};


static MenuItem_t Bindings1Items[] = {
    I_TITLE("MOVEMENT",      "LDB;TYBT"),
    I_EFUNC("MOVE FORWARD",  "LDB;TYBT DGTHTL",  BK_StartBindingKey, bk_forward),      // Движение вперед
    I_EFUNC("MOVE BACKWARD", "LDB;TYBT YFPFL",   BK_StartBindingKey, bk_backward),     // Движение назад
    I_EFUNC("TURN Left",     "GJDJHJN YFKTDJ",   BK_StartBindingKey, bk_turn_left),    // Поворот налево
    I_EFUNC("TURN Right",    "GJDJHJN YFGHFDJ",  BK_StartBindingKey, bk_turn_right),   // Поворот направо
    I_EFUNC("STRAFE LEFT",   ",JRJV DKTDJ",      BK_StartBindingKey, bk_strafe_left),  // Боком влево
    I_EFUNC("STRAFE RIGHT",  ",JRJV DGHFDJ",     BK_StartBindingKey, bk_strafe_right), // Боком вправо
    I_EFUNC("SPEED ON",      ",TU",              BK_StartBindingKey, bk_speed),        // Бег
    I_EFUNC("STRAFE ON",     "LDB;TYBT ,JRJV",   BK_StartBindingKey, bk_strafe),       // Движение боком
    I_EFUNC("FLY UP",        "KTNTNM DDTH[",     BK_StartBindingKey, bk_fly_up),       // Лететь вверх
    I_EFUNC("FLY DOWN",      "KTNTNM DYBP",      BK_StartBindingKey, bk_fly_down),     // Лететь вних
    I_EFUNC("STOP FLYING",   "JCNFYJDBNM GJKTN", BK_StartBindingKey, bk_fly_stop),     // Остановить полёт
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &Bindings2Menu), // Cледующая страница >
    I_SETMENU("< LAST PAGE", "^ GJCKTLYZZ CNHFYBWF", &Bindings8Menu), // < Последняя страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings1Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings1Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (2)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings2Items[] = {
    I_TITLE("ACTION",          "LTQCNDBT"),
    I_EFUNC("FIRE/ATTACK",     "FNFRF/CNHTKM,F",    BK_StartBindingKey, bk_fire),        // Атака/стрельба
    I_EFUNC("USE",             "BCGJKMPJDFNM",      BK_StartBindingKey, bk_use),         // Использовать
    I_EFUNC("JUMP",            "GHS;JR",            BK_StartBindingKey, bk_jump),        //Прыжок
    I_TITLE("WEAPONS",         "JHE;BT"), // Оружие
    I_EFUNC("WEAPON 1",        "JHE;BT 1",          BK_StartBindingKey, bk_weapon_1),    // Оружие 1
    I_EFUNC("WEAPON 2",        "JHE;BT 2",          BK_StartBindingKey, bk_weapon_2),    // Оружие 2
    I_EFUNC("WEAPON 3",        "JHE;BT 3",          BK_StartBindingKey, bk_weapon_3),    // Оружие 3
    I_EFUNC("WEAPON 4",        "JHE;BT 4",          BK_StartBindingKey, bk_weapon_4),    // Оружие 4
    I_EFUNC("PREVIOUS WEAPON", "GHTLSLEOTT JHE;BT", BK_StartBindingKey, bk_weapon_prev), // Предыдущее оружие
    I_EFUNC("NEXT WEAPON",     "CKTLE.OTT JHE;BT",  BK_StartBindingKey, bk_weapon_next), // Следующее оружие
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings3Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings1Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings2Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings2Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (3)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings3Items[] = {
    I_TITLE("SHORTCUT KEYS",      ",SCNHSQ LJCNEG"),
    I_EFUNC("Open help",          "'rhfy gjvjob",        BK_StartBindingKey, bk_menu_help),   // Экран помощи
    I_EFUNC("Open save menu",     "cj[hfytybt buhs",     BK_StartBindingKey, bk_menu_save),   // Сохранение игры
    I_EFUNC("Open load menu",     "pfuheprf buhs",       BK_StartBindingKey, bk_menu_load),   // Загрузка игры
    I_EFUNC("Open volume menu",   "yfcnhjqrb uhjvrjcnb", BK_StartBindingKey, bk_menu_volume), // Настройки громкости
    I_EFUNC("Suicide",            "Cebwbl",              BK_StartBindingKey, bk_suicide),      // СУИЦИД
    I_EFUNC("QUICK SAVE",         ",SCNHJT CJ[HFYTYBT",  BK_StartBindingKey, bk_qsave),       // Быстрое сохранение
    I_EFUNC("End game",           "pfrjyxbnm buhe",      BK_StartBindingKey, bk_end_game),    // Закончить игру
    I_EFUNC("QUICK LOAD",         ",SCNHFZ PFUHEPRF",    BK_StartBindingKey, bk_qload),       // Быстрая загрузка
    I_EFUNC("Quit game",          "ds[jl",               BK_StartBindingKey, bk_quit),        // Выход
    I_EFUNC("Change gamma level", "ehjdtym ufvvs",       BK_StartBindingKey, bk_gamma),       // Уровень гаммы
    I_EFUNC("RESTART LEVEL/DEMO", "GTHTPFGECR EHJDYZ",   BK_StartBindingKey, bk_reloadlevel), // Перезапуск уровня
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings4Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings2Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings3Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings3Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (4)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings4Items[] = {
    I_EFUNC("Increase screen size",  "edtk> hfpvth 'rhfyf",   BK_StartBindingKey, bk_screen_inc),       // Увел. размер экрана
    I_EFUNC("Decrease screen size",  "evtym> hfpvth 'rhfyf",  BK_StartBindingKey, bk_screen_dec),       // Умень. размер экрана
    I_EFUNC("SAVE A SCREENSHOT",     "CRHBYIJN",              BK_StartBindingKey, bk_screenshot),       // Скриншот
    I_EFUNC("Pause",                 "gfepf",                 BK_StartBindingKey, bk_pause),            // Пауза
    I_EFUNC("FINISH DEMO RECORDING", "PFRJYXBNM PFGBCM LTVJ", BK_StartBindingKey, bk_finish_demo),      // Закончить запись демо
    I_TITLE("TOGGLEABLES",           "GTHTRK.XTYBT"),
    I_EFUNC("MOUSE LOOK",            "J,PJH VSIM.",           BK_StartBindingKey, bk_toggle_mlook),     // Обзор мышью
    I_EFUNC("ALWAYS RUN",            "GJCNJZYYSQ ,TU",        BK_StartBindingKey, bk_toggle_autorun),   // Постоянный бег
    I_EFUNC("CROSSHAIR",             "GHBWTK",                BK_StartBindingKey, bk_toggle_crosshair), // Прицел
    I_EFUNC("Messages",              "cjj,otybz",             BK_StartBindingKey, bk_messages),         // Сообщения
    I_EFUNC("BACKGROUND DETAIL",     "LTNFKBPFWBZ AJYF",      BK_StartBindingKey, bk_detail),           // Детализация фона
    I_EFUNC("LEVEL FLIPPING",        "PTHRFKBHJDFYBT EHJDYZ", BK_StartBindingKey, bk_toggle_fliplvls),  // Зеркалирование уровня
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings5Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings3Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings4Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings4Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (5)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings5Items[] = {
    I_TITLE("AUTOMAP",          "RFHNF"),
    I_EFUNC("TOGGLE AUTOMAP",   "JNRHSNM RFHNE",     BK_StartBindingKey, bk_map_toggle),    // Открыть карту
    I_EFUNC("ZOOM IN",          "GHB,KBPBNM",        BK_StartBindingKey, bk_map_zoom_in),   // Приблизить
    I_EFUNC("ZOOM OUT",         "JNLFKBNM",          BK_StartBindingKey, bk_map_zoom_out),  // Отдалить
    I_EFUNC("MAXIMUM ZOOM OUT", "GJKYSQ VFCINF,",    BK_StartBindingKey, bk_map_zoom_max),  // Полный масштаб
    I_EFUNC("FOLLOW MODE",      "HT;BV CKTLJDFYBZ",  BK_StartBindingKey, bk_map_follow),    // Режим следования
    I_EFUNC("OVERLAY MODE",     "HT;BV YFKJ;TYBZ",   BK_StartBindingKey, bk_map_overlay),   // Режим наложения
    I_EFUNC("ROTATE MODE",      "HT;BV DHFOTYBZ",    BK_StartBindingKey, bk_map_rotate),    // Режим вращения
    I_EFUNC("TOGGLE GRID",      "CTNRF",             BK_StartBindingKey, bk_map_grid),      // Сетка
    I_EFUNC("ADD MARK",         "GJCNFDBNM JNVTNRE", BK_StartBindingKey, bk_map_mark),      // Поставить отметку
    I_EFUNC("CLEAR MARK",       "ELFKBNM JNVTNRE",   BK_StartBindingKey, bk_map_clearmark), // Удалить отметку
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings6Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings4Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings5Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings5Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (6)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings6Items[] = {
    I_TITLE("INVENTORY",            "BYDTYNFHM"),
    I_EFUNC("NEXT ITEM",            "CKTLE.OBQ GHTLVTN",         BK_StartBindingKey, bk_inv_right),
    I_EFUNC("PREVIOUS ITEM",        "GHTLSLEOBQ GHTLVTN",        BK_StartBindingKey, bk_inv_left),
    I_EFUNC("ACTIVATE ITEM",        "BCGJKMPJDFNM GHTLVTN",      BK_StartBindingKey, bk_inv_use_artifact),
    I_EFUNC("USE ALL ITEMS",        "BCGJKMPJDFNM DCT GHTLVTNS", BK_StartBindingKey, bk_arti_all),             // Использовать все предметы
    I_EFUNC("QUARTZ FLASK",         "RDFHWTDSQ AKFRJY",          BK_StartBindingKey, bk_arti_quartz),
    I_EFUNC("MYSTIC URN",           "VBCNBXTCRFZ EHYF",          BK_StartBindingKey, bk_arti_urn),
    I_EFUNC("FLECHETTE",            "PTKMT",                     BK_StartBindingKey, bk_arti_bomb),
    I_EFUNC("DISC OF REPULSION",    "LBCR JNNJH;TYBZ",           BK_StartBindingKey, bk_arti_blastradius),
    I_EFUNC("ICON OF THE DEFENDER", "CBVDJK PFOBNYBRF",          BK_StartBindingKey, bk_arti_invulnerability),
    I_EFUNC("PORKALATOR",           "CDBYJVJHATH",               BK_StartBindingKey, bk_arti_egg),
    I_EFUNC("CHAOS DEVICE",         "'V,KTVF [FJCF",             BK_StartBindingKey, bk_arti_chaosdevice),
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings7Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings5Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings6Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings6Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (7)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings7Items[] = {
    I_EFUNC("BANISHMENT DEVICE",   "'V,KTVF BPUYFYBZ",          BK_StartBindingKey, bk_arti_teleportother),
    I_EFUNC("WINGS OF WRATH",      "RHSKMZ UYTDF",              BK_StartBindingKey, bk_arti_wings),
    I_EFUNC("TORCH",               "AFRTK",                     BK_StartBindingKey, bk_arti_torch),
    I_EFUNC("KRATER OF MIGHT",     "XFIF VJUEOTCNDF",           BK_StartBindingKey, bk_arti_boostmana),
    I_EFUNC("DRAGONSKIN BRACERS",  "YFHEXB BP LHFRJYMTQ RJ;B",  BK_StartBindingKey, bk_arti_boostarmor),
    I_EFUNC("DARK SERVANT",        "NTVYSQ CKEUF",              BK_StartBindingKey, bk_arti_summon),
    I_EFUNC("BOOTS OF SPEED",      "CFGJUB-CRJHJ[JLS",          BK_StartBindingKey, bk_arti_speed),
    I_EFUNC("MYSTIC AMBIT INCANT", "XFHS VFUBXTCRJUJ TLBYCNDF", BK_StartBindingKey, bk_arti_healingradius),
    I_TITLE("LOOK",                "J,PJH"),                     // Обзор
    I_EFUNC("LOOK UP",             "CVJNHTNM DDTH[",            BK_StartBindingKey, bk_look_up),     // Смотреть вверх
    I_EFUNC("LOOK DOWN",           "CVJNHTNM DYBP",             BK_StartBindingKey, bk_look_down),   // Смотреть вниз
    I_EFUNC("CENTER LOOK",         "CVJNHTNM GHZVJ",            BK_StartBindingKey, bk_look_center), // Смотреть прямо
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `",  &Bindings8Menu), // Cледующая страница >
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Bindings6Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings7Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings7Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Key bindings (8)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings8Items[] = {
    I_TITLE("MULTIPLAYER",         "CTNTDFZ BUHF"), // Сетевая игра
    I_EFUNC("MULTIPLAYER SPY",     "DBL LHEUJUJ BUHJRF",  BK_StartBindingKey, bk_spy),                // Вид другого игрока
    I_EFUNC("SEND MESSAGE",        "JNGHFDBNM CJJ,OTYBT", BK_StartBindingKey, bk_multi_msg),          // Отправить сообщение
    I_EFUNC("MESSAGE TO PLAYER 1", "CJJ,OTYBT BUHJRE 1",  BK_StartBindingKey, bk_multi_msg_player_0), // Сообщение игроку 1
    I_EFUNC("MESSAGE TO PLAYER 2", "CJJ,OTYBT BUHJRE 2",  BK_StartBindingKey, bk_multi_msg_player_1), // Сообщение игроку 2
    I_EFUNC("MESSAGE TO PLAYER 3", "CJJ,OTYBT BUHJRE 3",  BK_StartBindingKey, bk_multi_msg_player_2), // Сообщение игроку 3
    I_EFUNC("MESSAGE TO PLAYER 4", "CJJ,OTYBT BUHJRE 4",  BK_StartBindingKey, bk_multi_msg_player_3), // Сообщение игроку 4
    I_EFUNC("MESSAGE TO PLAYER 5", "CJJ,OTYBT BUHJRE 5",  BK_StartBindingKey, bk_multi_msg_player_4), // Сообщение игроку 5
    I_EFUNC("MESSAGE TO PLAYER 6", "CJJ,OTYBT BUHJRE 6",  BK_StartBindingKey, bk_multi_msg_player_5), // Сообщение игроку 6
    I_EFUNC("MESSAGE TO PLAYER 7", "CJJ,OTYBT BUHJRE 7",  BK_StartBindingKey, bk_multi_msg_player_6), // Сообщение игроку 7
    I_EFUNC("MESSAGE TO PLAYER 8", "CJJ,OTYBT BUHJRE 8",  BK_StartBindingKey, bk_multi_msg_player_7), // Сообщение игроку 8
    I_EMPTY,
    I_SETMENU("RESET CONTROLS...", "C,HJCBNM EGHFDKTYBT>>>", &ResetControlsMenu), // СБРОСИТЬ УПРАВЛЕНИЕ
    I_SETMENU("FIRST PAGE >", "GTHDFZ CNHFYBWF `",     &Bindings1Menu), // Первая страница >
    I_SETMENU("< PREV PAGE",  "^ GHTLSLEOFZ CNHFYBWF", &Bindings7Menu), // < Предыдущая страница
    I_EMPTY
};

MENU_STATIC_PAGED(Bindings8Menu,
    35, 35,
    25,
    "CUSTOMIZE CONTROLS", "YFCNHJQRB EGHFDKTYBZ", false, // Настройки управления
    Bindings8Items, false,
    M_RD_Draw_Bindings,
    &ControlsMenu,
    &BindingsPageDescriptor
);

// -----------------------------------------------------------------------------
// Reset settings
// -----------------------------------------------------------------------------

static MenuItem_t ResetControlsItems[] = {
    I_EFUNC("RECOMMENDED", "HTRJVTYLJDFYYJT", M_RD_ResetControls_Recommended, 0), // РЕКОМЕНДОВАННОЕ
    I_EFUNC("ORIGINAL",    "JHBUBYFKMYJT",    M_RD_ResetControls_Original,    0), // ОРИГИНАЛЬНОЕ
};

MENU_STATIC(ResetControlsMenu,
    115, 100,
    95,
    "", "", false,
    ResetControlsItems, false,
    DrawResetControlsMenu,
    &Bindings8Menu
);

// -----------------------------------------------------------------------------
// Gamepad
// -----------------------------------------------------------------------------

static MenuItem_t GamepadSelectItems[] = {
    I_SWITCH("ENABLE GAMEPAD:",     "BCGJKMPJDFNM UTQVGFL:", M_RD_UseGamepad), // ИСПОЛЬЗОВАТЬ ГЕЙМПАД
    I_EMPTY,
    I_TITLE( "ACTIVE CONTROLLERS:", "FRNBDYST UTQVGFLS:"), // АКТИАНЫЕ ГЕЙМПАДЫ
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1),
    I_EFUNC(NULL, NULL, OpenControllerOptionsMenu, -1)
};

MENU_STATIC(GamepadSelectMenu,
    76, 66,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    GamepadSelectItems, false,
    DrawGamepadSelectMenu,
    &ControlsMenu
);

static const PageDescriptor_t GamepadPageDescriptor = {
    2, GamepadMenuPages,
    252, 182,
    CR_WHITE
};

static MenuItem_t Gamepad1Items[] = {
    I_LRFUNC("LEFT X AXIS:",  "KTDFZ [ JCM:",       M_RD_BindAxis_LX),
    I_LRFUNC("SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_LX),
    I_SWITCH("INVERT AXIS:",  "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_LX),
    I_LRFUNC("DEAD ZONE:",    "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_LX),
    I_EMPTY,
    I_LRFUNC("LEFT Y AXIS:",  "KTDFZ E JCM:",       M_RD_BindAxis_LY),
    I_LRFUNC("SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_LY),
    I_SWITCH("INVERT AXIS:",  "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_LY),
    I_LRFUNC("DEAD ZONE:",    "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_LY),
    I_EMPTY,
    I_LRFUNC("LEFT TRIGGER:", "KTDSQ NHBUUTH:",     M_RD_BindAxis_LT),
    I_LRFUNC("SENSITIVITY:",  "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_LT),
    I_SWITCH("INVERT AXIS:",  "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_LT),
    I_LRFUNC("DEAD ZONE:",    "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_LT),
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE>OFZ CNHFYBWF `", &Gamepad2Menu)
};

MENU_STATIC_PAGED(Gamepad1Menu,
    36, 21,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    Gamepad1Items, false,
    DrawGamepadMenu_1,
    &GamepadSelectMenu,
    &GamepadPageDescriptor
);

static MenuItem_t Gamepad2Items[] = {
    I_LRFUNC("RIGHT X AXIS:",  "GHFDFZ [ JCM:",      M_RD_BindAxis_RX),
    I_LRFUNC("SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_RX),
    I_SWITCH("INVERT AXIS:",   "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_RX),
    I_LRFUNC("DEAD ZONE:",     "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_RX),
    I_EMPTY,
    I_LRFUNC("RIGHT Y AXIS:",  "GHFDFZ E JCM:",      M_RD_BindAxis_RY),
    I_LRFUNC("SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_RY),
    I_SWITCH("INVERT AXIS:",   "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_RY),
    I_LRFUNC("DEAD ZONE:",     "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_RY),
    I_EMPTY,
    I_LRFUNC("RIGHT TRIGGER:", "GHFDSQ NHBUUTH:",    M_RD_BindAxis_RT),
    I_LRFUNC("SENSITIVITY:",   "XEDCNDBNTKMYJCNM:",  M_RD_SensitivityAxis_RT),
    I_SWITCH("INVERT AXIS:",   "BYDTHNBHJDFNM JCM:", M_RD_InvertAxis_RT),
    I_LRFUNC("DEAD ZONE:",     "VTHNDFZ PJYF:",      M_RD_DeadZoneAxis_RT),
    I_EMPTY,
    I_SETMENU("< PREV PAGE", "^ GHTLSLEOFZ CNHFYBWF", &Gamepad1Menu)
};

MENU_STATIC_PAGED(Gamepad2Menu,
    36, 21,
    32,
    "GAMEPAD SETTINGS", "YFCNHJQRB UTQVGFLF", false, // Настройки геймпада
    Gamepad2Items, false,
    DrawGamepadMenu_2,
    &GamepadSelectMenu,
    &GamepadPageDescriptor
);

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t GameplayPageDescriptor = {
    3, GameplayMenuPages,
    254, 172,
    CR_GRAY
};

static MenuItem_t Gameplay1Items[] = {
    I_TITLE( "VISUAL",                       "UHFABRF"), // ГРАФИКА
    I_SWITCH("BRIGHTMAPS:",                  ",HFQNVFGGBYU:",                M_RD_Brightmaps), // БРАЙТМАППИНГ
    I_SWITCH("FAKE CONTRAST:",               "BVBNFWBZ RJYNHFCNYJCNB:",      M_RD_FakeContrast), // ИМИТАЦИЯ КОНТРАСТНОСТИ
    I_SWITCH("EXTRA TRANSLUCENCY:",          "LJGJKYBNTKMYFZ GHJPHFXYJCNM:", M_RD_ExtraTrans), // ДОПОЛНИТЕЛЬНАЯ ПРОЗРАЧНОСТЬ
    I_SWITCH("SWIRLING LIQUIDS:",            "EKEXITYYFZ FYBVFWBZ ;BLRJCNTQ:", M_RD_SwirlingLiquids), // УЛУЧШЕННАЯ АНИМАЦИЯ ЖИДКОСТЕЙ
    I_SWITCH("SKY DRAWING MODE:",            "HT;BV JNHBCJDRB YT,F:",        M_RD_LinearSky), // РЕЖИМ ОТРИСОВКИ НЕБА
    I_SWITCH("RANDOMLY MIRRORED CORPSES:",   "PTHRFKMYJT JNHF;TYBT NHEGJD:", M_RD_FlipCorpses), // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ ТРУПОВ
    I_SWITCH("FLIP WEAPONS:",                "PTHRFKMYJT JNHF;TYBT JHE;BZ:", M_RD_FlipWeapons), // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ ОРУЖИЯ
    I_TITLE( "PHYSICAL",                     "ABPBRF"), // ФИЗИКА
    I_SWITCH("COLLISION PHYSICS:",           "ABPBRF CNJKRYJDTYBQ:",         M_RD_Collision), // ФИЗИКА СТОЛКНОВЕНИЙ
    I_SWITCH("CORPSES SLIDING FROM LEDGES:", "NHEGS CGJKPF.N C DJPDSITYBQ:", M_RD_Torque), // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    I_LRFUNC("FLOATING ITEMS AMPLITUDE:" ,   "KTDBNFWBZ GHTLVTNJD:",         M_RD_FloatAmplitude), // АМПЛИТУДА ЛЕВИТАЦИИ ПРЕДМЕТОВ
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &Gameplay2Menu), // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_SETMENU("< LAST PAGE", "^ GJCKTLYZZ CNHFYBWF", &Gameplay3Menu)  // < ПОСЛЕДНЯЯ СТРАНИЦА
};

MENU_STATIC_PAGED(Gameplay1Menu,
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    Gameplay1Items, false,
    DrawGameplay1Menu,
    &RDOptionsMenu,
    &GameplayPageDescriptor
);

// -----------------------------------------------------------------------------
// Gameplay features (2)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay2Items[] = {
    I_TITLE( "STATUS BAR",            "CNFNEC-,FH"), // СТАТУС-БАР
    I_SWITCH("COLORED STATUS BAR:",   "HFPYJWDTNYST \'KTVTYNS:",     M_RD_ColoredSBar), // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ
    I_LRFUNC("COLORED HEALTH GEM:",   "JRHFIBDFYBT RFVYZ PLJHJDMZ:", M_RD_ColoredGem), // ОКРАШИВАНИЕ КАМНЯ ЗДОРОВЬЯ
    I_SWITCH("SHOW NEGATIVE HEALTH:", "JNHBWFNTKMYJT PLJHJDMT:",     M_RD_NegativeHealth), // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ
    I_LRFUNC("ARTIFACTS TIMER:",      "NFQVTH FHNTAFRNJD:",          M_RD_ShowArtiTimer), // ТАЙМЕР АРТЕФАКТОВ
    I_LRFUNC("ASSEMBLED WEAPON WIDGET:", "DBL;TN CJ,HFYYJUJ JHE;BZ:", M_RD_WeaponWidget), // ВИДЖЕТ СОБРАННОГО ОРУЖИЯ
    I_TITLE( "CROSSHAIR",             "GHBWTK"), // ПРИЦЕЛ
    I_SWITCH("DRAW CROSSHAIR:",       "JNJ,HF;FNM GHBWTK:",          M_RD_CrossHairDraw), // ОТОБРАЖАТЬ ПРИЦЕЛ
    I_LRFUNC("SHAPE:",                "AJHVF:",                      M_RD_CrossHairShape), // ФОРМА
    I_SWITCH("INDICATION:",           "BYLBRFWBZ:",                  M_RD_CrossHairType), // ИНДИКАЦИЯ
    I_SWITCH("INCREASED SIZE:",       "EDTKBXTYYSQ HFPVTH:",         M_RD_CrossHairScale), // УВЕЛИЧЕННЫЙ РАЗМЕР
    I_LRFUNC("OPACITY:",              "YTGHJPHFXYJCNM:",             M_RD_CrossHairOpacity), // НЕПРОЗРАЧНОСТЬ
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("LAST PAGE >",  "GJCKTLYZZ CNHFYBWF `", &Gameplay3Menu), // ПОСЛЕДНЯЯ СТРАНИЦА >
    I_SETMENU("< FIRST PAGE", "^ GTHDFZ CNHFYBWF",    &Gameplay1Menu)  // < ПЕРВАЯ СТРАНИЦА
};

MENU_STATIC_PAGED(Gameplay2Menu,
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    Gameplay2Items, false,
    DrawGameplay2Menu,
    &RDOptionsMenu,
    &GameplayPageDescriptor
);

// -----------------------------------------------------------------------------
// Gameplay features (3)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay3Items[] = {
    I_TITLE( "GAMEPLAY",                    "UTQVGKTQ"), // ГЕЙМПЛЕЙ
    I_SWITCH("FIX ERRORS ON VANILLA MAPS:", "ECNHFYZNM JIB,RB JHBU> EHJDYTQ:", M_RD_FixMapErrors), // УСТРАНЯТЬ ОШИБКИ ОРИГ. УРОВНЕЙ
    I_SWITCH("FLIP GAME LEVELS:",           "PTHRFKMYJT JNHF;TYBT EHJDYTQ:",   M_RD_FlipLevels), // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    I_SWITCH("PLAY INTERNAL DEMOS:",        "GHJBUHSDFNM LTVJPFGBCB:",         M_RD_NoDemos), // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    I_SWITCH("IMITATE PLAYER'S BREATHING:", "BVBNFWBZ LS[FYBZ BUHJRF:",        M_RD_Breathing), // ИМИТАЦИЯ ДЫХАНИЯ ИГРОКА
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("FIRST PAGE >", "GTHDFZ CNHFYBWF `",     &Gameplay1Menu), // ПЕРВАЯ СТРАНИЦА >
    I_SETMENU("< PREV PAGE",  "^ GHTLSLEOFZ CNHFYBWF", &Gameplay2Menu)  // < ПРЕДЫДУЩАЯ СТРАНИЦА
};

MENU_STATIC_PAGED(Gameplay3Menu,
    36, 36,
    32,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    Gameplay3Items, false,
    DrawGameplay3Menu,
    &RDOptionsMenu,
    &GameplayPageDescriptor
);

// -----------------------------------------------------------------------------
// Level select (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t LevelSelectPageDescriptor = {
    5, LevelSelectMenuPages,
    248, 176,
    CR_GRAY
};

static MenuItem_t Level1Items[] = {
    I_LRFUNC("CLASS:",             "RKFCC:",           M_RD_SelectiveClass), // КЛАСС
    I_LRFUNC("SKILL LEVEL:",       "CKJ;YJCNM:",       M_RD_SelectiveSkill), // СЛОЖНОСТЬ
    I_LRFUNC("HUB:",               "[F,:",             M_RD_SelectiveHub), // ХАБ
    I_LRFUNC("MAP:",               "EHJDTYM:",         M_RD_SelectiveMap), // УРОВЕНЬ
    I_TITLE( "PLAYER",             "BUHJR"), // ИГРОК
    I_LRFUNC("HEALTH:",            "PLJHJDMT:",        M_RD_SelectiveHealth), // ЗДОРОВЬЕ
    I_TITLE( "ARMOR:",             ",HJYZ:"), // БРОНЯ
    I_LRFUNC("MESH ARMOR:",        "RJKMXEUF:",        M_RD_SelectiveArmor_0), // КОЛЬЧУГА
    I_LRFUNC("FALCON SHIELD:",     "CJRJKBYSQ OBN:",   M_RD_SelectiveArmor_1), // СОКОЛИНЫЙ ЩИТ
    I_LRFUNC("PLATINUM HELMET:",   "GKFNBYJDSQ IKTV:", M_RD_SelectiveArmor_2), // ПЛАТИНОВЫЙ ШЛЕМ
    I_LRFUNC("AMULET OF WARDING:", "FVEKTN CNHF;F:",   M_RD_SelectiveArmor_3), // АМУЛЕТ СТРАЖА
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu2_F), // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu1,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level1Items, false,
    DrawLevelSelect1Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

// -----------------------------------------------------------------------------
// Level select (2)
// -----------------------------------------------------------------------------

static MenuItem_t Level2Items_F[] = {
    I_TITLE( "WEAPONS",                "JHE;BT"), // ОРУЖИЕ
    I_SWITCH("TIMON'S AXE:",           "NJGJH NBVJYF:",              M_RD_SelectiveWp_0), // ТОПОР ТИМОНА
    I_SWITCH("HAMMER OF RETRIBUTION:", "VJKJN DJPVTPLBZ:",           M_RD_SelectiveWp_1), // МОЛОТ ВОЗМЕЗДИЯ
    I_SWITCH("QUIETUS:",               "GJCKTLYBQ LJDJL:",           M_RD_SelectiveWp_2), // ПОСЛЕДНИЙ ДОВОД
    I_SWITCH("HANDLE OF QUIETUS:",     "HERJZNM GJCKTLYTUJ LJDJLF:", M_RD_SelectiveWp_P_0), // РУКОЯТЬ ПОСЛЕДНЕГО ДОВОДА
    I_SWITCH("GUARD OF QUIETUS:",      "UFHLF GJCKTLYTUJ LJDJLF:",   M_RD_SelectiveWp_P_1), // ГАРДА ПОСЛЕДНЕГО ДОВОДА
    I_SWITCH("BLADE OF QUIETUS:",      "KTPDBT GJCKTLYTUJ LJDJLF:",  M_RD_SelectiveWp_P_2), // ЛЕЗВИЕ ПОСЛЕДНЕГО ДОВОДА
    I_TITLE( "MANA",                   "VFYF"), // МАНА
    I_LRFUNC("BLUE:",                  "CBYZZ:",                     M_RD_SelectiveAmmo_0), // СИНЯЯ
    I_LRFUNC("GREEN:",                 "PTK~YFZ:",                   M_RD_SelectiveAmmo_1), // ЗЕЛЁНАЯ
    I_TITLE( "ARTIFACTS",              "FHNTAFRNS"), // АРТЕФАКТЫ
    I_LRFUNC("QUARTZ FLASK:",          "RDFHWTDSQ AKFRJY:",          M_RD_SelectiveArti_0), // КВАРЦЕВЫЙ ФЛАКОН
    I_LRFUNC("MYSTIC URN:",            "VBCNBXTCRFZ EHYF:",          M_RD_SelectiveArti_1), // МИСТИЧЕСКАЯ УРНА
    I_LRFUNC("FLECHETTE:",             "PTKMT:",                     M_RD_SelectiveArti_2), // ЗЕЛЬЕ
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu3), // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0)  // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu2_F,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level2Items_F, false,
    DrawLevelSelect2Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

static MenuItem_t Level2Items_C[] = {
    I_TITLE( "WEAPONS",               "JHE;BT"), // ОРУЖИЕ
    I_SWITCH("SERPENT STAFF:",        "PVTBYSQ GJCJ[:",          M_RD_SelectiveWp_0), // ЗМЕИНЫЙ ПОСОХ
    I_SWITCH("FIRESTORM:",            "JUYTYYSQ INJHV:",         M_RD_SelectiveWp_1), // ОГНЕННЫЙ ШТОРМ
    I_SWITCH("WRAITHVERGE:",          ";TPK LE[JD:",             M_RD_SelectiveWp_2), // ЖЕЗЛ ДУХОВ
    I_SWITCH("POLE OF WRAITHVERGE:",  "LHTDRJ ;TPKF LE[JD:",     M_RD_SelectiveWp_P_0), // ДРЕВКО ЖЕЗЛА ДУХОВ
    I_SWITCH("CROSS OF WRAITHVERGE:", "RHTCNJDBYF ;TPKF LE[JD:", M_RD_SelectiveWp_P_1), // КРЕСТОВИНА ЖЕЗЛА ДУХОВ
    I_SWITCH("HEAD OF WRAITHVERGE:",  "YFDTHITYBT ;TPKF LE[JD:", M_RD_SelectiveWp_P_2), // НАВЕРШЕНИЕ ЖЕЗЛА ДУХОВ
    I_TITLE( "MANA",                  "VFYF"), // МАНА
    I_LRFUNC("BLUE:",                 "CBYZZ:",                  M_RD_SelectiveAmmo_0), // СИНЯЯ
    I_LRFUNC("GREEN:",                "PTK~YFZ:",                M_RD_SelectiveAmmo_1), // ЗЕЛЁНАЯ
    I_TITLE( "ARTIFACTS",             "FHNTAFRNS"), // АРТЕФАКТЫ
    I_LRFUNC("QUARTZ FLASK:",         "RDFHWTDSQ AKFRJY:",       M_RD_SelectiveArti_0), // КВАРЦЕВЫЙ ФЛАКОН
    I_LRFUNC("MYSTIC URN:",           "VBCNBXTCRFZ EHYF:",       M_RD_SelectiveArti_1), // МИСТИЧЕСКАЯ УРНА
    I_LRFUNC("FLECHETTE:",            "PTKMT:",                  M_RD_SelectiveArti_2), // ЗЕЛЬЕ
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu3),   // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu2_C,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level2Items_C, false,
    DrawLevelSelect2Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

static MenuItem_t Level2Items_M[] = {
    I_TITLE( "WEAPONS",                 "JHE;BT"), // ОРУЖИЕ
    I_SWITCH("FROST SHARDS:",           "KTLZYST JCRJKRB:",            M_RD_SelectiveWp_0), // ЛЕДЯНЫЕ ОСКОЛКИ
    I_SWITCH("ARC OF DEATH:",           "LEUF CVTHNB:",                M_RD_SelectiveWp_1), // ДУГА СМЕРТИ
    I_SWITCH("BLOODSCOURGE:",           "RHJDFDSQ ,BX:",               M_RD_SelectiveWp_2), // КРОВАВЫЙ БИЧ
    I_SWITCH("BINDER OF BLOODSCOURGE:", "GJLRJDTW RHJDFDJUJ ,BXF:",    M_RD_SelectiveWp_P_0), // ПОДКОВЕЦ КРОВАВОГО БИЧА
    I_SWITCH("SHAFT OF BLOODSCOURGE:",  "LHTDRJ RHJDFDJUJ ,BXF:",      M_RD_SelectiveWp_P_1), // ДРЕВКО КРОВАВОГО БИЧА
    I_SWITCH("KNOB OF BLOODSCOURGE:",   "YF,FKLFIYBR RHJDFDJUJ ,BXF:", M_RD_SelectiveWp_P_2), // НАБАЛДАШНИК КРОВАВОГО БИЧА
    I_TITLE( "MANA",                    "VFYF"), // МАНА
    I_LRFUNC("BLUE:",                   "CBYZZ:",                      M_RD_SelectiveAmmo_0), // СИНЯЯ
    I_LRFUNC("GREEN:",                  "PTK~YFZ:",                    M_RD_SelectiveAmmo_1), // ЗЕЛЁНАЯ
    I_TITLE( "ARTIFACTS",               "FHNTAFRNS"), // АРТЕФАКТЫ
    I_LRFUNC("QUARTZ FLASK:",           "RDFHWTDSQ AKFRJY:",           M_RD_SelectiveArti_0), // КВАРЦЕВЫЙ ФЛАКОН
    I_LRFUNC("MYSTIC URN:",             "VBCNBXTCRFZ EHYF:",           M_RD_SelectiveArti_1), // МИСТИЧЕСКАЯ УРНА
    I_LRFUNC("FLECHETTE:",              "PTKMT:",                      M_RD_SelectiveArti_2), // ЗЕЛЬЕ
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu3),   // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu2_M,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level2Items_M, false,
    DrawLevelSelect2Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

// -----------------------------------------------------------------------------
// Level select (3)
// -----------------------------------------------------------------------------

static MenuItem_t Level3Items[] = {
    I_LRFUNC("DISC OF REPULSION:",    "LBCR JNNJH;TYBZ:",           M_RD_SelectiveArti_3), // ДИСК ОТТОРЖЕНИЯ
    I_LRFUNC("ICON OF THE DEFENDER:", "CBVDJK PFOBNYBRF:",          M_RD_SelectiveArti_4), // СИМВОЛ ЗАЩИТНИКА
    I_LRFUNC("PORKALATOR:",           "CDBYJVJHATH:",               M_RD_SelectiveArti_5), // СВИНОМОРФЕР
    I_LRFUNC("CHAOS DEVICE:",         "'V,KTVF [FJCF:",             M_RD_SelectiveArti_6), // ЭМБЛЕМА ХАОСА
    I_LRFUNC("BANISHMENT DEVICE:",    "'V,KTVF BPUYFYBZ:",          M_RD_SelectiveArti_7), // ЭМБЛЕМА ИЗГНАНИЯ
    I_LRFUNC("WINGS OF WRATH:",       "RHSKMZ UYTDF:",              M_RD_SelectiveArti_8), // КРЫЛЬЯ ГНЕВА
    I_LRFUNC("TORCH:",                "AFRTK:",                     M_RD_SelectiveArti_9), // ФАКЕЛ
    I_LRFUNC("KRATER OF MIGHT:",      "XFIF VJUEOTCNDF:",           M_RD_SelectiveArti_10), // ЧАША МОГУЩЕСТВА
    I_LRFUNC("DRAGONSKIN BRACERS:",   "YFHEXB BP LHFRJYMTQ RJ;B:",  M_RD_SelectiveArti_11), // НАРУЧИ ИЗ ДРАКОНЬЕЙ КОЖИ
    I_LRFUNC("DARK SERVANT:",         "NTVYSQ CKEUF:",              M_RD_SelectiveArti_12), // ТЕМНЫЙ СЛУГА
    I_LRFUNC("BOOTS OF SPEED:",       "CFGJUB-CRJHJ[JLS:",          M_RD_SelectiveArti_13), // САПОГИ-СКОРОХОДЫ
    I_LRFUNC("MYSTIC AMBIT INCANT:",  "XFHS VFUBXTCRJUJ TLBYCNDF:", M_RD_SelectiveArti_14), // ЧАРЫ МАГИЧЕСКОГО ЕДИНСТВА
    I_TITLE( "KEYS",                  "RK.XB"), // КЛЮЧИ
    I_SWITCH("EMERALD KEY:",          "BPEVHELYSQ RK.X:",           M_RD_SelectiveKey_0), // ИЗУМРУДНЫЙ КЛЮЧ
    I_EMPTY,
    I_SETMENU("NEXT PAGE >", "CKTLE.OFZ CNHFYBWF `", &LevelSelectMenu4),   // СЛЕДУЮЩАЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu3,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level3Items, false,
    DrawLevelSelect3Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

// -----------------------------------------------------------------------------
// Level select (4)
// -----------------------------------------------------------------------------

static MenuItem_t Level4Items[] = {
    I_SWITCH("SILVER KEY:",        "CTHT,HZYSQ RK.X:",    M_RD_SelectiveKey_1), // СЕРЕБРЯНЫЙ КЛЮЧ
    I_SWITCH("FIRE KEY:",          "JUYTYYSQ RK.X:",      M_RD_SelectiveKey_2), // ОГНЕННЫЙ КЛЮЧ
    I_SWITCH("STEEL KEY:",         "CNFKMYJQ RK.X:",      M_RD_SelectiveKey_3), // СТАЛЬНОЙ КЛЮЧ
    I_SWITCH("HORN KEY:",          "HJUJDJQ RK.X:",       M_RD_SelectiveKey_4), // РОГОВОЙ КЛЮЧ
    I_SWITCH("CAVE KEY:",          "GTOTHYSQ RK.X:",      M_RD_SelectiveKey_5), // ПЕЩЕРНЫЙ КЛЮЧ
    I_SWITCH("CASTLE KEY:",        "RK.X JN PFVRF:",      M_RD_SelectiveKey_6), // КЛЮЧ ОТ ЗАМКА
    I_SWITCH("SWAMP KEY:",         ",JKJNYSQ RK.X:",      M_RD_SelectiveKey_7), // БОЛОТНЫЙ КЛЮЧ
    I_SWITCH("RUSTED KEY:",        "H;FDSQ RK.X:",        M_RD_SelectiveKey_8), // РЖАВЫЙ КЛЮЧ
    I_SWITCH("DUNGEON KEY:",       "RK.X JN GJLPTVTKMZ:", M_RD_SelectiveKey_9), // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
    I_SWITCH("AXE KEY:",           "RK.X-NJGJH:",         M_RD_SelectiveKey_10), // КЛЮЧ-ТОПОР
    I_TITLE( "PUZZLE ITEMS",       "GFPKJDST GHTLVTNS"), // ПАЗЛОВЫЕ ПЕРЕДМЕТЫ
    I_LRFUNC("FLAME MASK:",        "VFCRF GKFVTYB:",      M_RD_SelectivePuzzle_0), // МАСКА ПЛАМЕНИ
    I_LRFUNC("HEART OF D'SPARIL:", "CTHLWT L&CGFHBKF:",   M_RD_SelectivePuzzle_1), // СЕРДЦЕ Д'СПАРИЛА
    I_LRFUNC("RUBY PLANET:",       "HE,BYJDFZ GKFYTNF:",  M_RD_SelectivePuzzle_2), // РУБИНОВАЯ ПЛАНЕТА
    I_EMPTY,
    I_SETMENU("LAST PAGE >", "GJCKTLYZZ CNHFYBWF `", &LevelSelectMenu5),   // ПОСЛЕДНЯЯ СТРАНИЦА >
    I_EFUNC("START GAME",    "YFXFNM BUHE",          G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu4,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level4Items, false,
    DrawLevelSelect4Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

// -----------------------------------------------------------------------------
// Level select (5)
// -----------------------------------------------------------------------------

static MenuItem_t Level5Items[] = {
    I_LRFUNC("EMERALD PLANET 1:",   "BPEVHELYFZ GKFYTNF 1:",   M_RD_SelectivePuzzle_3), // ИЗУМРУДНАЯ ПЛАНЕТА 1
    I_LRFUNC("EMERALD PLANET 2:",   "BPEVHELYFZ GKFYTNF 2:",   M_RD_SelectivePuzzle_4), // ИЗУМРУДНАЯ ПЛАНЕТА 2
    I_LRFUNC("SAPPHIRE PLANET 1:",  "CFGABHJDFZ GKFYTNF 1:",   M_RD_SelectivePuzzle_5), // САПФИРОВАЯ ПЛАНЕТА 1
    I_LRFUNC("SAPPHIRE PLANET 2:",  "CFGABHJDFZ GKFYTNF 2:",   M_RD_SelectivePuzzle_6), // САПФИРОВАЯ ПЛАНЕТА 2
    I_LRFUNC("CLOCK GEAR (S):",     "XFCJDFZ ITCNTHYZ (C):",   M_RD_SelectivePuzzle_7), // ЧАСОВАЯ ШЕСТЕРНЯ (Б&С)
    I_LRFUNC("CLOCK GEAR (B):",     "XFCJDFZ ITCNTHYZ (,):",   M_RD_SelectivePuzzle_8), // ЧАСОВАЯ ШЕСТЕРНЯ (Б)
    I_LRFUNC("CLOCK GEAR (S B):",   "XFCJDFZ ITCNTHYZ (C ,):", M_RD_SelectivePuzzle_9), // ЧАСОВАЯ ШЕСТЕРНЯ (С&Б)
    I_LRFUNC("CLOCK GEAR (B S):",   "XFCJDFZ ITCNTHYZ (, C):", M_RD_SelectivePuzzle_10), // ЧАСОВАЯ ШЕСТЕРНЯ (С)
    I_LRFUNC("DAEMON CODEX:",       "RJLTRC LTVJYF:",          M_RD_SelectivePuzzle_11), // КОДЕКС ДЕМОНА
    I_LRFUNC("LIBER OSCURA:",       NULL, /* eng text */       M_RD_SelectivePuzzle_12), // LIBER OSCURA
    I_LRFUNC("YORICK'S SKULL:",     "XTHTG QJHBRF:",           M_RD_SelectivePuzzle_13), // ЧЕРЕП ЙОРИКА
    I_LRFUNC("GLAIVE SEAL:",        "GTXFNM DJBNTKZ:",         M_RD_SelectivePuzzle_14), // ПЕЧАТЬ ВОИТЕЛЯ
    I_LRFUNC("HOlY RELIC:",         "CDZNFZ HTKBRDBZ:",        M_RD_SelectivePuzzle_15), // СВЯТАЯ РЕЛИКВИЯ
    I_LRFUNC("SIGIL OF THE MAGUS:", "CBVDJK VFUF:",            M_RD_SelectivePuzzle_16), // СИМВОЛ МАГА
    I_EMPTY,
    I_SETMENU("FIRST PAGE >", "GTHDFZ CNHFYBWF `", &LevelSelectMenu1),   // ПЕРВАЯ СТРАНИЦА >
    I_EFUNC("START GAME",     "YFXFNM BUHE",       G_DoSelectiveGame, 0) // НАЧАТЬ ИГРУ
};

MENU_STATIC_PAGED(LevelSelectMenu5,
    74, 40,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    Level5Items, false,
    DrawLevelSelect5Menu,
    &RDOptionsMenu,
    &LevelSelectPageDescriptor
);

// -----------------------------------------------------------------------------
// Reset settings
// -----------------------------------------------------------------------------

static MenuItem_t ResetSettingstems[] = {
    I_EFUNC("RECOMMENDED", "HTRJVTYLJDFYYSQ", M_RD_BackToDefaults_Recommended, 0), // РЕКОМЕНДОВАННЫЙ
    I_EFUNC("ORIGINAL",    "JHBUBYFKMYSQ",    M_RD_BackToDefaults_Original,    0), // ОРИГИНАЛЬНЫЙ
};

MENU_STATIC(ResetSettings,
    115, 100,
    95,
    "", "", false,
    ResetSettingstems, false,
    DrawResetSettingsMenu,
    &RDOptionsMenu
);

// -----------------------------------------------------------------------------
// Vanilla options menu
// -----------------------------------------------------------------------------

static MenuItem_t VanillaOptionsItems[] = {
    I_EFUNC( "END GAME",          "PFRJYXBNM BUHE",   SCEndGame, 0), // ЗАКОНЧИТЬ ИГРУ
    I_LRFUNC("MESSAGES : ",       "CJJ,OTYBZ : ",     M_RD_Messages), // СООБЩЕНИЯ:
    I_LRFUNC("MOUSE SENSITIVITY", "CRJHJCNM VSIB",    M_RD_Sensitivity), // СКОРОСТЬ МЫШИ
    I_EMPTY,
    I_SETMENU("MORE...",          "LJGJKYBNTKMYJ>>>", &VanillaOptions2Menu)  // ДОПОЛНИТЕЛЬНО...
};

MENU_STATIC(VanillaOptionsMenu,
    88, 88,
    30,
    NULL, NULL, true,
    VanillaOptionsItems, true,
    DrawOptionsMenu_Vanilla,
    &HMainMenu
);

static MenuItem_t VanillaOptions2Items[] = {
    I_LRFUNC("SCREEN SIZE",  "HFPVTH 'RHFYF",    M_RD_ScreenSize), // РАЗМЕР ЭКРАНА
    I_EMPTY,
    I_LRFUNC("SFX VOLUME",   "UHJVRJCNM PDERF",  M_RD_SfxVolume), // ГРОМКОСТЬ ЗВУКА
    I_EMPTY,
    I_LRFUNC("MUSIC VOLUME", "UHJVRJCNM VEPSRB", M_RD_MusVolume), // ГРОМКОСТЬ МУЗЫКИ
    I_EMPTY
};

MENU_STATIC(VanillaOptions2Menu,
    90, 90,
    20,
    NULL, NULL, true,
    VanillaOptions2Items, true,
    DrawOptions2Menu_Vanilla,
    &VanillaOptionsMenu
);

static MenuItem_t FilesItems[] = {
    I_SETMENU_NONET("LOAD GAME", "PFUHEPBNM BUHE", &LoadMenu, 2), // ЗАГРУЗИТЬ ИГРУ
    I_SETMENU(      "SAVE GAME", "CJ[HFYBNM BUHE", &SaveMenu)  // СОХРАНИТЬ ИГРУ
};

MENU_STATIC(FilesMenu,
    110, 79,
    60,
    NULL, NULL, true,
    FilesItems, true,
    DrawFilesMenu,
    &HMainMenu
);

static MenuItem_t LoadItems[] = {
    I_EFUNC(NULL, NULL, SCLoadGame, 0),
    I_EFUNC(NULL, NULL, SCLoadGame, 1),
    I_EFUNC(NULL, NULL, SCLoadGame, 2),
    I_EFUNC(NULL, NULL, SCLoadGame, 3),
    I_EFUNC(NULL, NULL, SCLoadGame, 4),
    I_EFUNC(NULL, NULL, SCLoadGame, 5),
    I_EFUNC(NULL, NULL, SCLoadGame, 6)
};

MENU_STATIC(LoadMenu,
    64, 64,
    17,
    NULL, NULL, true, // ЗАГРУЗИТЬ ИГРУ
    LoadItems, true,
    DrawSaveLoadMenu,
    &FilesMenu
);

static MenuItem_t SaveItems[] = {
    I_EFUNC(NULL, NULL, SCSaveGame, 0),
    I_EFUNC(NULL, NULL, SCSaveGame, 1),
    I_EFUNC(NULL, NULL, SCSaveGame, 2),
    I_EFUNC(NULL, NULL, SCSaveGame, 3),
    I_EFUNC(NULL, NULL, SCSaveGame, 4),
    I_EFUNC(NULL, NULL, SCSaveGame, 5),
    I_EFUNC(NULL, NULL, SCSaveGame, 6)
};

MENU_STATIC(SaveMenu,
    64, 64,
    17,
    NULL, NULL, true,
    SaveItems, true,
    DrawSaveLoadMenu,
    &FilesMenu
);

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

    // [JN] Rendering resolution: remember choosen widescreen variable before quit.
    rendering_resolution_temp = rendering_resolution;
    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;
    // [JN] Screen renderer: set temp variable for rendering menu.
    opengles_renderer_temp = opengles_renderer;

    // [JN] Init message colors.
    M_RD_Define_Msg_Color(msg_pickup, message_color_pickup);
    M_RD_Define_Msg_Color(msg_quest, message_color_quest);
    M_RD_Define_Msg_Color(msg_system, message_color_system);
    M_RD_Define_Msg_Color(msg_chat, message_color_chat);
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

    // [JN] Decrease speaker test timer if it's active, don't go negative.
    if (speaker_test_timeout)
    {
        speaker_test_timeout--;
    }

    // [JN] Decrease crosshair color showcase timer, reset when it reaches zero.
    if (CrosshairShowcaseTimeout > 0)
    {
        CrosshairShowcaseTimeout--;
    }
    else
    {
        CrosshairShowcaseTimeout = 105; // Equals TICRATE * 3, three seconds.
    }
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
    "ARE YOU SURE YOU WANT TO DELETE SAVED GAME:",
    "" // [JN] Placeholder for language changing
};

char *QuitEndMsg_Rus[] = {
    "DS LTQCNDBNTKMYJ ;TKFTNT DSQNB?",			// ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ВЫЙТИ?
    "DS LTQCNDBNTKMYJ ;TKFTNT PFRJYXBNM BUHE?",	// ВЫ ДЕЙСТВИТЕЛЬНО ЖЕЛАЕТЕ ЗАКОНЧИТЬ ИГРУ?
    "DSGJKYBNM ,SCNHJT CJ[HFYTYBT BUHS:",		// ВЫПОЛНИТЬ БЫСТРОЕ СОХРАНЕНИЕ ИГРЫ:
    "DSGJKYBNM ,SCNHE. PFUHEPRE BUHS:",			// ВЫПОЛНИТЬ БЫСТРУЮ ЗАГРУЗКУ ИГРЫ:
    "DS LTQCNDBNTKMYJ [JNBNT CJDTHIBNM CEBWBL?",  // ВЫ ДЕЙСТВИТЕЛЬНО ХОТИТЕ СОВЕРШИТЬ СУИЦИД?
    "ELFKBNM CJ[HFYTYYE. BUHE:",				// УДАЛИТЬ СОХРАНЕННУЮ ИГРУ:
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

            // [JN] Print save game name in deletion request.
            if (typeofask == 6)
            {
                RD_M_DrawTextA(SlotText[CurrentItPos],
                               160 - RD_M_TextAWidth(SlotText[CurrentItPos]) / 2 + wide_delta, 90);
                RD_M_DrawTextA("?",
                               160 + RD_M_TextAWidth(SlotText[CurrentItPos]) / 2 + wide_delta, 90);
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
    static char *boxLumpName[5] = {
        "m_fbox",
        "m_cbox",
        "m_mbox",
        "TNT1A0",  // [JN] Unused (pig class)
        "M_RBOX"   // [JN] Random class (ENG)
    };
    static char *boxLumpName_Rus[5] = {
        "rd_fbox",
        "rd_cbox",
        "rd_mbox",
        "TNT1A0",  // [JN] Unused (pig class)
        "RD_RBOX"  // [JN] Random class (RUS)
    };
    static char *walkLumpName[3] = {
        "m_fwalk1",
        "m_cwalk1",
        "m_mwalk1"
    };

    if (english_language)
    {
        RD_M_DrawTextB("CHOOSE CLASS:", 27 + wide_delta, 24);
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

    // [JN] Don't draw walking player for random class.
    if (class < 3)
    {
        V_DrawPatch(174 + 24 + wide_delta, 8 + 12,
                    W_CacheLumpNum(W_GetNumForName(walkLumpName[class])
                                   + ((MenuTime >> 3) & 3), PU_CACHE), NULL);
    }
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
    QuickSaveTitle = false;
    QuickLoadTitle = false;
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

    if (english_language)
    {
        if (CurrentMenu == &LoadMenu)
        {
            RD_M_DrawTextBigENG(QuickLoadTitle ? "QUICK LOAD" : "LOAD GAME", 
                                160 - RD_M_TextBigENGWidth
                               (QuickLoadTitle ? "QUICK LOAD" : "LOAD GAME") / 2 + wide_delta, 1);
        }
        else
        {
            RD_M_DrawTextBigENG(QuickSaveTitle ? "QUICK SAVE" : "SAVE GAME",
                                160 - RD_M_TextBigENGWidth
                               (QuickSaveTitle ? "QUICK SAVE" : "SAVE GAME") / 2 + wide_delta, 1);
        }
    }
    else
    {
        if (CurrentMenu == &LoadMenu)
        {
            RD_M_DrawTextBigRUS(QuickLoadTitle ? ",SCNHFZ PFUHEPRF" : "PFUHEPBNM BUHE", 
                                160 - RD_M_TextBigRUSWidth
                               (QuickLoadTitle ? ",SCNHFZ PFUHEPRF" : "PFUHEPBNM BUHE") / 2 + wide_delta, 1);
        }
        else
        {
            RD_M_DrawTextBigRUS(QuickSaveTitle ? ",SCNHJT CJ[HFYTYBT" : "CJ[HFYBNM BUHE",
                                160 - RD_M_TextBigRUSWidth
                               (QuickSaveTitle ? ",SCNHJT CJ[HFYTYBT" : "CJ[HFYBNM BUHE") / 2 + wide_delta, 1);
        }
    }
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

    for (slot = 0; slot < 7; slot++)
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

    x = 64; // [Dasperal] SaveMenu and LoadMenu have the same x and the same y
    y = 18; // so inline them here to eliminate the Menu_t* argument
    for (i = 0; i < 7; i++)
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
    // Sound / Sound System menu background will be drawn.
    sfxbgdraw = true;

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);
}

// -----------------------------------------------------------------------------
// DrawRenderingMenu1
// -----------------------------------------------------------------------------

static void DrawRenderingMenu1(void)
{
	static char num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Rendering resolution
        RD_M_DrawTextSmallENG(rendering_resolution_temp == 1 ? "MIDDLE" :
                              rendering_resolution_temp == 2 ? "HIGH" : "LOW",
                              189 + wide_delta, 42, CR_NONE);

        // Display aspect ratio
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 185 + wide_delta, 52, CR_NONE);

        // Screen renderer
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallENG("N/A", 161 + wide_delta, 62, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(opengles_renderer_temp ? "OPENGL ES 2.0" :
#ifdef _WIN32
            // On Windows, default is always Direct 3D 9.
            "DIRECT 3D",
#else
            // On other OSes it is unclear, so use OS preferred.
            "PREFERRED BY OS",
#endif
            161 + wide_delta, 62, CR_NONE);
        }

        // Vertical sync
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallENG("N/A", 216 + wide_delta, 72, CR_GRAY);
        }
        else
        {
            RD_M_DrawTextSmallENG(vsync ? "ON" : "OFF", 216 + wide_delta, 72, CR_NONE);
        }

        // FPS limit
        RD_Menu_DrawSliderSmallInline(100, 82, 11, (max_fps-40) / 20);
        // Numerical representation of slider position
        M_snprintf(num, 4, "%d", max_fps);
        RD_M_DrawTextSmallENG(num, 208 + wide_delta, 83, 
                              max_fps < 60 ? CR_GRAY :
                              max_fps < 100 ? CR_NONE :
                              max_fps < 260 ? CR_GREEN : 
							  max_fps < 999 ? CR_YELLOW : CR_RED);

        // Performance counter
        RD_M_DrawTextSmallENG(show_fps == 1 ? "FPS only" :
                              show_fps == 2 ? "FULL" : "OFF", 
                              192 + wide_delta, 92, CR_NONE);

        // Pixel scaling
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallENG("N/A", 131 + wide_delta, 102, CR_GRAY);
        }
        else
        {
            RD_M_DrawTextSmallENG(smoothing ? "SMOOTH" : "SHARP", 131 + wide_delta, 102, CR_NONE);
        }

        // Porch palette changing
        RD_M_DrawTextSmallENG(vga_porch_flash ? "ON" : "OFF", 205 + wide_delta, 112, CR_NONE);

        // Diminished lighting
        RD_M_DrawTextSmallENG(smoothlight ? "SMOOTH" : "ORIGINAL", 169 + wide_delta, 122, CR_NONE);

        // Informative message
        if (rendering_resolution_temp != rendering_resolution
        ||  aspect_ratio_temp != aspect_ratio
        ||  opengles_renderer_temp != opengles_renderer)
        {
            RD_M_DrawTextSmallENG("THE PROGRAM MUST BE RESTARTED", 51 + wide_delta, 142, MenuTime & 32 ? CR_RED : CR_DARKRED);
        }

        // Tip for faster sliding
        if (CurrentItPos == 5)
        {
            RD_M_DrawTextSmallENG("HOLD RUN BUTTON FOR FASTER SLIDING",
                                  39 + wide_delta, 152, CR_DARKRED);
        }
    }
    else
    {
        // Разрешение рендеринга
        RD_M_DrawTextSmallRUS(rendering_resolution_temp == 1 ? "CHTLYTT" :
                              rendering_resolution_temp == 2 ? "DSCJRJT" : "YBPRJT",
                              204 + wide_delta, 42, CR_NONE);

        // Соотношение сторон экрана
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                   aspect_ratio_temp == 2 ? "16:9" :
                   aspect_ratio_temp == 3 ? "16:10" :
                   aspect_ratio_temp == 4 ? "21:9" :
                                            "4:3", 230 + wide_delta, 52, CR_NONE);

        // Экрнанный рендерер
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallRUS("Y/L", 158 + wide_delta, 62, CR_DARKRED); // Н/Д
        }
        else
        {
            if (opengles_renderer_temp)
            {
                RD_M_DrawTextSmallENG("OPENGL ES 2.0", 158 + wide_delta, 62, CR_NONE);
            }
            else
            {
#ifdef _WIN32
                // On Windows, default is always Direct 3D 9.
                RD_M_DrawTextSmallENG("DIRECT 3D", 158 + wide_delta, 62, CR_NONE);
#else
                // On other OSes it is unclear, so use OS preferred.
                RD_M_DrawTextSmallRUS("GJ DS,JHE JC", 158 + wide_delta, 62, CR_NONE); // ПО ВЫБОРУ ОС
#endif
            }
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallRUS("Y/L", 236 + wide_delta, 72, CR_GRAY);
        }
        else
        {
            RD_M_DrawTextSmallRUS(vsync ? "DRK" : "DSRK", 236 + wide_delta, 72, CR_NONE);
        }

        // Ограничение FPS
        RD_M_DrawTextSmallENG("FPS:", 123 + wide_delta, 82, CR_NONE);
        RD_Menu_DrawSliderSmallInline(154, 82, 11, (max_fps-40) / 20);
        // Numerical representation of slider position
        M_snprintf(num, 4, "%d", max_fps);
        RD_M_DrawTextSmallENG(num, 262 + wide_delta, 83,
                              max_fps < 60 ? CR_GRAY :
                              max_fps < 100 ? CR_NONE :
                              max_fps < 260 ? CR_GREEN : 
							  max_fps < 999 ? CR_YELLOW : CR_RED);

        // Счетчик производительности
        RD_M_DrawTextSmallRUS(show_fps == 1 ? "" : // Print as US string below
                              show_fps == 2 ? "gjkysq" : "dsrk",
                              236 + wide_delta, 92, CR_NONE);
        // Print "FPS" separately, RU sting doesn't fit in 4:3 aspect ratio :(
        if (show_fps == 1) RD_M_DrawTextSmallENG("fps", 236 + wide_delta, 92, CR_NONE);

        // Пиксельное сглаживание
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallRUS("Y/L", 211 + wide_delta, 102, CR_GRAY);
        }
        else
        {
            RD_M_DrawTextSmallRUS(smoothing ? "DRK" : "DSRK", 211 + wide_delta, 102, CR_NONE);
        }

        // Изменение палитры краев экрана
        RD_M_DrawTextSmallRUS(vga_porch_flash ? "DRK" : "DSRK", 265 + wide_delta, 112, CR_NONE);

        // Угасание освещения
        RD_M_DrawTextSmallRUS(smoothlight ? "GKFDYJT" : "JHBUBYFKMYJT", 180 + wide_delta, 122, CR_NONE);

        // Informative message: НЕОБХОДИМ ПЕРЕЗАПУСК ИГРЫ
        if (rendering_resolution_temp != rendering_resolution
        || aspect_ratio_temp != aspect_ratio
        || opengles_renderer_temp != opengles_renderer)
        {
            RD_M_DrawTextSmallRUS("YTJ,[JLBV GTHTPFGECR GHJUHFVVS",
                                  46 + wide_delta, 142, MenuTime & 32 ? CR_RED : CR_DARKRED);
        }

        // Для ускоренного пролистывания
        // удерживайте кнопку бега
        if (CurrentItPos == 5)
        {
            RD_M_DrawTextSmallRUS("LKZ ECRJHTYYJUJ GHJKBCNSDFYBZ",
                                  51 + wide_delta, 152, CR_DARKRED);
            RD_M_DrawTextSmallRUS("ELTH;BDFQNT RYJGRE ,TUF",
                                  73 + wide_delta, 162, CR_DARKRED);
        }
    }
}

static void DrawRenderingMenu2(void)
{
    char  win_width[8];
    char  win_height[8];
    char *window_size;

    // Consolidate window size string for printing.
    M_snprintf(win_width, 8, "%d", window_width);
    M_snprintf(win_height, 8, "%d", window_height);
    window_size = M_StringJoin(win_width, "x", win_height, NULL);

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Bordered window
        RD_M_DrawTextSmallENG(window_border ? "ON" : "OFF", 157 + wide_delta, 42, CR_NONE);

        // Window size
        RD_M_DrawTextSmallENG(window_size, 121 + wide_delta, 52, fullscreen ? CR_GRAY : CR_NONE);

        // Tip for faster sliding
        if (CurrentItPos == 2)
        {
            if (fullscreen)
            {
                RD_M_DrawTextSmallCenteredENG("SWITCH TO WINDOWED MODE FIRST", 132, CR_DARKGREEN);
                RD_M_DrawTextSmallCenteredENG("BY PRESSING \"ALT+ENTER\"", 142, CR_DARKGREEN);
            }
            else
            {
                RD_M_DrawTextSmallCenteredENG("USE ARROW KEYS TO CHANGE SIZE,", 132, CR_DARKGREEN);
                RD_M_DrawTextSmallCenteredENG("HOLD RUN BUTTON FOR FASTER SLIDING.", 142, CR_DARKGREEN);
            }
        }

        // Window title
        RD_M_DrawTextSmallENG(window_title_short ? "BRIEF" : "FULL", 124 + wide_delta, 62, CR_NONE);

        // Always on top
        RD_M_DrawTextSmallENG(window_ontop ? "ON" : "OFF", 138 + wide_delta, 72, CR_NONE);

        // Preserve window aspect ratio
        RD_M_DrawTextSmallENG(preserve_window_aspect_ratio ? "ON" : "OFF", 249 + wide_delta, 82, CR_NONE);
    }
    else
    {
        // Окно с рамкой
        RD_M_DrawTextSmallRUS(window_border ? "DRK" : "DSRK", 138 + wide_delta, 42, CR_NONE);

        // Размер окна
        RD_M_DrawTextSmallENG(window_size, 126 + wide_delta, 52, fullscreen ? CR_GRAY : CR_NONE);

        // Подсказка для ускоренного изменения размера
        if (CurrentItPos == 2)
        {
            if (fullscreen)
            {
                RD_M_DrawTextSmallCenteredRUS("GTHTRK.XBNTCM D JRJYYSQ HT;BV", 132, CR_DARKGREEN);
                RD_M_DrawTextSmallRUS("BCGJKMPEZ CJXTNFYBT RKFDBI", 16, 142, CR_DARKGREEN);
                RD_M_DrawTextSmallENG("\"ALT+ENTER\"", 220 + wide_delta, 142, CR_DARKGREEN);
            }
            else
            {
            RD_M_DrawTextSmallCenteredRUS("LKZ ECRJHTYYJUJ BPVTYTYBZ HFPVTHF", 132, CR_DARKGREEN);
            RD_M_DrawTextSmallCenteredRUS("ELTH;BDFQNT RYJGRE ,TUF", 142, CR_DARKGREEN);
            }
        }

        // Заголовок окна
        RD_M_DrawTextSmallRUS(window_title_short ? "RHFNRBQ" : "GJLHJ,YSQ", 148 + wide_delta, 62, CR_NONE);

        // Поверх других окон
        RD_M_DrawTextSmallRUS(window_ontop ? "DRK" : "DSRK", 177 + wide_delta, 72, CR_NONE);

        // Пропорции окна
        RD_M_DrawTextSmallRUS(preserve_window_aspect_ratio  ? "ABRCBHJDFYYST" : "CDJ,JLYST", 147 + wide_delta, 82, CR_NONE);
    }

    // Screenshot format / Формат скриншотов (same english values)
    RD_M_DrawTextSmallENG(png_screenshots ? "PNG" : "PCX", 176 + wide_delta, 102, CR_NONE);

}

static void M_RD_Change_Resolution(Direction_t direction)
{
    // [JN] Rendering resolution: changing only temp variable here.
    // Initially it is set in MN_Init and stored into config file in M_QuitResponse.
    RD_Menu_SpinInt(&rendering_resolution_temp, 0, 2, direction);
}

static void M_RD_Change_Widescreen(Direction_t direction)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in MN_Init and stored into config file in M_QuitResponse.
    RD_Menu_SpinInt(&aspect_ratio_temp, 0, 4, direction);
}

static void M_RD_Change_Renderer()
{
    // [JN] Disable toggling in software renderer
    if (force_software_renderer == 1)
    {
        return;
    }

    opengles_renderer_temp ^= 1;
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

static void M_RD_PerfCounter(Direction_t direction)
{
    RD_Menu_SpinInt(&show_fps, 0, 2, direction);
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

static void M_RD_DiminishedLighting()
{
    smoothlight ^= 1;

    // Recalculate light tables
    R_InitLightTables();
}

static void M_RD_WindowBorder()
{
    window_border ^= 1;
    
    I_ToggleWindowBorder();
}


static void M_RD_WindowSize(Direction_t direction)
{
    // Disallow to change size in full screen mode.
    if (fullscreen)
    {
        return;
    }

    switch (direction)
    {
        case LEFT_DIR:
            window_width  -= BK_isKeyPressed(bk_speed) ? 10 : 1;
            window_height -= BK_isKeyPressed(bk_speed) ? 10 : 1;
        break;
        case RIGHT_DIR:
            window_width  += BK_isKeyPressed(bk_speed) ? 10 : 1;
            window_height += BK_isKeyPressed(bk_speed) ? 10 : 1;
        break;
    }

    // Prevent overflows / incorrect values.
    if (window_width < screenwidth >> hires)
    {
        window_width = screenwidth >> hires; // SDL_SetWindowMinimumSize in i_video.c
    }
    if (window_width > 3440)
    {
        window_width = 3440;
    }
    if (window_height < actualheight >> hires)
    {
        window_height = actualheight >> hires; // SDL_SetWindowMinimumSize in i_video.c
    }
    if (window_height > 1440)
    {
        window_height = 1440;
    }

    AdjustWindowSize();
    SDL_SetWindowSize(screen, window_width, window_height);
}

static void M_RD_WindowTitle()
{
    window_title_short ^= 1;
    
    I_InitWindowTitle();
}

static void M_RD_AlwaysOnTop()
{
    window_ontop ^= 1;

    I_KeepWindowOnTop();
}

static void M_RD_WindowAspectRatio()
{
    preserve_window_aspect_ratio ^= 1;
    
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
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
        // Background detail
        RD_M_DrawTextSmallENG(hud_detaillevel ? "LOW" : "HIGH", 169 + wide_delta, 82, CR_NONE);
    }
    else
    {
        // Background detail
        RD_M_DrawTextSmallRUS(hud_detaillevel ? "YBPRFZ" : "DSCJRFZ", 167 + wide_delta, 82, CR_NONE);
    }

    // Screen size
    if (aspect_ratio >= 2)
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 4, screenblocks - 9);
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 83 + wide_delta, 53, CR_GRAY);
    }
    else
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 10, screenblocks - 3);
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 131 + wide_delta, 53, CR_GRAY);
    }

    // Level brightness
    RD_Menu_DrawSliderSmall(&DisplayMenu, 72, 9, extra_level_brightness);
    // Numerical representation of slider position
    M_snprintf(num, 4, "%d", extra_level_brightness);
    dp_translation = cr[CR_GRAY];
    RD_M_DrawTextA(num, 128 + wide_delta, 73);
    dp_translation = NULL;
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

    R_SetViewSize(screenblocks);
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
                              107 + wide_delta, 65, CR_YELLOW);

        RD_M_DrawTextSmallENG("RED", 121 + wide_delta, 75, CR_RED);
        RD_M_DrawTextSmallENG("GREEN", 105 + wide_delta, 85, CR_GREEN);
        RD_M_DrawTextSmallENG("BLUE", 116 + wide_delta, 95, CR_BLUE2);
    }
    else
    {
        RD_M_DrawTextSmallRUS("zhrjcnm", 93 + wide_delta, 25, CR_NONE);       // Яркость
        RD_M_DrawTextSmallRUS("ufvvf", 106 + wide_delta, 35, CR_NONE);        // Гамма
        RD_M_DrawTextSmallRUS("yfcsotyyjcnm", 50 + wide_delta, 45, CR_NONE);  // Насыщенность
        RD_M_DrawTextSmallRUS("wdtnjdfz gfkbnhf", 25 + wide_delta, 55, CR_NONE);  // Цветовая палитра
        RD_M_DrawTextSmallRUS(show_palette ? "DRK" : "DSRK", 165 + wide_delta, 55, CR_NONE);

        RD_M_DrawTextSmallRUS("byntycbdyjcnm wdtnf",  // Интенсивность цвета
                              89 + wide_delta, 65, CR_YELLOW);

        RD_M_DrawTextSmallRUS("rhfcysq", 90 + wide_delta, 75, CR_RED);   // Красный
        RD_M_DrawTextSmallRUS("ptktysq", 90 + wide_delta, 85, CR_GREEN); // Зелёный
        RD_M_DrawTextSmallRUS("cbybq", 109 + wide_delta, 95, CR_BLUE2);   // Синий
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
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 76, CR_RED);

    // GREEN intensity slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 85, 10, g_color_factor * 10);
    M_snprintf(num, 5, "%3f", g_color_factor);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 86, CR_GREEN);

    // BLUE intensity slider
    RD_Menu_DrawSliderSmall(&ColorMenu, 95, 10, b_color_factor * 10);
    M_snprintf(num, 5, "%3f", b_color_factor);  // Numerical representation of slider position
    RD_M_DrawTextSmallENG(num, 264 + wide_delta, 96, CR_BLUE2);

    if (show_palette)
    {
        V_DrawPatchUnscaled(wide_delta*2, 200, W_CacheLumpName(("M_COLORS"), PU_CACHE), NULL);
    }
}

static void M_RD_LevelBrightness(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt(&extra_level_brightness, 0, 8, direction);
}

static void M_RD_BG_Detail()
{
    hud_detaillevel ^= 1;

    // [JN] Update screen border.
    setsizeneeded = true;
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
                              136 + wide_delta, 72, CR_GRAY);

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

        // Item pickup
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_color_pickup), 120 + wide_delta, 132,
                              M_RD_ColorTranslation(message_color_pickup));

        // Quest message
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_color_quest), 144 + wide_delta, 142,
                              M_RD_ColorTranslation(message_color_quest));

        // System message
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_color_system), 152 + wide_delta, 152,
                              M_RD_ColorTranslation(message_color_system));

        // Netgame chat
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_color_chat), 135 + wide_delta, 162,
                              M_RD_ColorTranslation(message_color_chat));
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
                              136 + wide_delta, 72, CR_GRAY);

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

        // Получение предметов
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_color_pickup), 187 + wide_delta, 132,
                              M_RD_ColorTranslation(message_color_pickup));

        // Квестовое сообщение
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_color_quest), 192 + wide_delta, 142,
                              M_RD_ColorTranslation(message_color_quest));

        // Системные сообщения
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_color_system), 191 + wide_delta, 152,
                              M_RD_ColorTranslation(message_color_system));

        // Чат сетевой игры
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_color_chat), 162 + wide_delta, 162,
                              M_RD_ColorTranslation(message_color_chat));
    }

    // Messages timeout (slider)
    RD_Menu_DrawSliderSmall(&MessagesMenu, 72, 10, messages_timeout - 1);
}

static void M_RD_Messages(Direction_t direction)
{
    show_messages ^= 1;

    P_SetMessage(&players[consoleplayer], show_messages ?
                 txt_messages_on : txt_messages_off, msg_system, true);

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

void M_RD_Define_Msg_Color (MessageType_t messageType, int color)
{
    Translation_CR_t * colorVar;
    switch (messageType)
    {
        case msg_pickup: // Item pickup.
            colorVar = &messages_pickup_color_set;
            break;
        case msg_quest: // Revealed secret
            colorVar = &messages_quest_color_set;
            break;
        case msg_system: // System message
            colorVar = &messages_system_color_set;
            break;
        case msg_chat: // Netgame chat
            colorVar = &messages_chat_color_set;
            break;
        default:
            return;
    }

    // [JN] No coloring in vanilla.
    if (vanillaparm)
    {
        *colorVar = CR_NONE;
    }
    else
    {
        switch (color)
        {
            case 1:   *colorVar = CR_WHITE;            break;
            case 2:   *colorVar = CR_GRAY;             break;
            case 3:   *colorVar = CR_DARKGRAY;         break;
            case 4:   *colorVar = CR_RED;              break;
            case 5:   *colorVar = CR_DARKRED;          break;
            case 6:   *colorVar = CR_GREEN;            break;
            case 7:   *colorVar = CR_DARKGREEN;        break;
            case 8:   *colorVar = CR_OLIVE;            break;
            case 9:   *colorVar = CR_BLUE2;            break;            
            case 10:  *colorVar = CR_DARKBLUE;         break;
            case 11:  *colorVar = CR_NIAGARA;          break;
            case 12:  *colorVar = CR_BRIGHTYELLOW;     break;
            case 13:  *colorVar = CR_YELLOW;           break;
            case 14:  *colorVar = CR_TAN;              break;
            case 15:  *colorVar = CR_BROWN;            break;
            default:  *colorVar = CR_NONE;             break;
        }
    }
}

void M_RD_Change_Msg_Pickup_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_color_pickup, 0, 15, direction);

    // [JN] Redefine pickup message color.
    M_RD_Define_Msg_Color(msg_pickup, message_color_pickup);
}

void M_RD_Change_Msg_Quest_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_color_quest, 0, 15, direction);

    // [JN] Redefine quest message color.
    M_RD_Define_Msg_Color(msg_quest, message_color_quest);
}

void M_RD_Change_Msg_System_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_color_system, 0, 15, direction);

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_Color(msg_system, message_color_system);
}

void M_RD_Change_Msg_Chat_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_color_chat, 0, 15, direction);

    // [JN] Redefine netgame chat message color.
    M_RD_Define_Msg_Color(msg_chat, message_color_chat);
}

// -----------------------------------------------------------------------------
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    char num[4];

    M_snprintf(num, 4, "%d", automap_grid_size);

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Rotate mode
        RD_M_DrawTextSmallENG(automap_rotate ? "ON" : "OFF", 127 + wide_delta, 32, CR_NONE);

        // Overlay mode
        RD_M_DrawTextSmallENG(automap_overlay ? "ON" : "OFF", 134 + wide_delta, 42, CR_NONE);

        // Follow mode
        RD_M_DrawTextSmallENG(automap_follow ? "ON" : "OFF", 123 + wide_delta, 72, CR_NONE);

        // Grid
        RD_M_DrawTextSmallENG(automap_grid ? "ON" : "OFF", 72 + wide_delta, 82, CR_NONE);

        // Grid size
        RD_M_DrawTextSmallENG(num, 105 + wide_delta, 92, CR_NONE);

        // Mark color
        RD_M_DrawTextSmallENG(M_RD_ColorName(automap_mark_color+1), 118 + wide_delta, 102,
                              M_RD_ColorTranslation(automap_mark_color+1));
    }
    else
    {
        // Режим вращения
        RD_M_DrawTextSmallRUS(automap_rotate ? "DRK" : "DSRK", 154 + wide_delta, 32, CR_NONE);

        // Режим наложения
        RD_M_DrawTextSmallRUS(automap_overlay ? "DRK" : "DSRK", 162 + wide_delta, 42, CR_NONE);

        // Режим следования
        RD_M_DrawTextSmallRUS(automap_follow ? "DRK" : "DSRK", 169 + wide_delta, 72, CR_NONE);

        // Сетка
        RD_M_DrawTextSmallRUS(automap_grid ? "DRK" : "DSRK", 82 + wide_delta, 82, CR_NONE);

        // Размер сетки
        RD_M_DrawTextSmallRUS(num, 133 + wide_delta, 92, CR_NONE);

        // Цвет отметок
        RD_M_DrawTextSmallRUS(M_RD_ColorName(automap_mark_color+1), 133 + wide_delta, 102,
                              M_RD_ColorTranslation(automap_mark_color+1));
    }

    // Overlay background opacity
    RD_Menu_DrawSliderSmall(&AutomapMenu, 62, 9, automap_overlay_bg / 3);
    // Numerical representation of slider position
    M_snprintf(num, 5, "%d", automap_overlay_bg);
    RD_M_DrawTextSmallENG(num, 128 + wide_delta, 63, CR_GRAY);
}

static void M_RD_AutoMapOverlay()
{
    automap_overlay ^= 1;
}

static void M_RD_AutoMapOverlayBG(Direction_t direction)
{
    RD_Menu_SlideInt(&automap_overlay_bg, 0, 24, direction);
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

static void M_RD_AutoMapGridSize(Direction_t direction)
{
    RD_Menu_ShiftSlideInt(&automap_grid_size, 32, 512, direction);
}

static void M_RD_AutomapMarkColor(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_mark_color, 0, 14, direction);

    // [JN] Reinitialize automap mark color.
    AM_initMarksColor(automap_mark_color);
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char num[4];

    // Draw menu background.
    if (sfxbgdraw)
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    // SFX Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 52, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%3d", snd_MaxVolume_tmp);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 53, CR_GRAY);

    // Music Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 72, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 73, CR_GRAY);

    // SFX Channels
    RD_Menu_DrawSliderSmall(&SoundMenu, 102, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%3d", snd_Channels);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 103, CR_GRAY);
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
    if (sfxbgdraw)
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Sound effects
        if (snd_sfxdevice == 0)
        {
            RD_M_DrawTextSmallENG("DISABLED", 144 + wide_delta, 42, CR_GRAY);
        }
        else if (snd_sfxdevice == 3)
        {
            RD_M_DrawTextSmallENG("DIGITAL SFX", 144 + wide_delta, 42, CR_NONE);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            RD_M_DrawTextSmallENG("DISABLED", 80 + wide_delta, 52, CR_GRAY);
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
        RD_M_DrawTextSmallENG(snd_monomode ? "MONO" : "STEREO", 181 + wide_delta, 102, CR_NONE);

        // Pitch-Shifted sounds
        RD_M_DrawTextSmallENG(snd_pitchshift ? "ON" : "OFF", 189 + wide_delta, 112, CR_NONE);

        // Mute inactive window
        RD_M_DrawTextSmallENG(mute_inactive_window ? "ON" : "OFF", 184 + wide_delta, 122, CR_NONE);

        // Informative message:
        if (CurrentItPos == 1 || CurrentItPos == 2 || CurrentItPos == 4)
        {
            RD_M_DrawTextSmallENG("CHANGING WILL REQUIRE RESTART OF THE PROGRAM", 3 + wide_delta, 132, CR_RED);
        }
    }
    else
    {
        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            RD_M_DrawTextSmallRUS("JNRK.XTYS", 173 + wide_delta, 42, CR_GRAY);
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
            RD_M_DrawTextSmallRUS("JNRK.XTYF", 91 + wide_delta, 52, CR_GRAY);
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
        RD_M_DrawTextSmallRUS(snd_monomode ? "VJYJ" : "CNTHTJ", 226 + wide_delta, 102, CR_NONE);

        // Произвольный питч-шифтинг
        RD_M_DrawTextSmallRUS(snd_pitchshift ? "DRK" : "DSRK", 230 + wide_delta, 112, CR_NONE);

        // Звук в неактивном окне
        RD_M_DrawTextSmallRUS(mute_inactive_window ? "DSRK" : "DRK", 201 + wide_delta, 122, CR_NONE);

        // Informative message: ИЗМЕНЕНИЕ ПОТРЕБУЕТ ПЕРЕЗАПУСК ПРОГРАММЫ
        if (CurrentItPos == 1 || CurrentItPos == 2 || CurrentItPos == 4)
        {
            RD_M_DrawTextSmallRUS("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS",
                                  11 + wide_delta, 132, CR_RED);
        }
    }


    // [JN] Speaker test routine.
    if (speaker_test_timeout)
    {
        dp_translation = cr[CR_WHITE];

        if (snd_sfxdevice == 0 || snd_sfxdevice == 1)
        {
            if (speaker_test_timeout >= 15)
            {
                if (english_language)
                {
                    RD_M_DrawTextSmallENG("enable digital effects first!",
                                        61 + wide_delta, 142, CR_RED);
                }
                else
                {
                    // ВКЛЮЧИТЕ ЦИФРОВЫЕ ЭФФЕКТЫ!
                    RD_M_DrawTextSmallRUS("drk.xbnt wbahjdst \'aatrns!",
                                        57 + wide_delta, 142, CR_RED);
                }
            }
            speaker_test_left = false;
            speaker_test_right = false;
        }
        else if (snd_monomode)
        {
            if (speaker_test_timeout <= 30)
            {
                if (english_language)
                {
                    RD_M_DrawTextSmallENG("< mono mode >", 116 + wide_delta, 
                                          142, CR_GREEN);
                }
                else
                {
                    // < МОНО РЕЖИМ >
                    RD_M_DrawTextSmallRUS("^ vjyj ht;bv `", 111 + wide_delta,
                                          142, CR_GREEN);
                }
            }

            if (speaker_test_left && speaker_test_right)
            {
                I_StartSound(&S_sfx[SFX_PICKUP_KEY], snd_Channels_RD-1,
                             snd_MaxVolume * 8, 128, 127); 
                speaker_test_timeout = 30;
            }

            speaker_test_left = false;
            speaker_test_right = false;
        }
        else
        {
            if (speaker_test_timeout > 30)
            {
                if (english_language)
                {
                    RD_M_DrawTextSmallENG("< left channel", 112 + wide_delta,
                                          142, CR_GREEN);
                }
                else
                {
                    // < ЛЕВЫЙ КАНАЛ
                    RD_M_DrawTextSmallRUS("^ ktdsq rfyfk", 113 + wide_delta,
                                          142, CR_GREEN);
                }

                if (speaker_test_left)
                {
                    I_StartSound(&S_sfx[SFX_PICKUP_KEY], snd_Channels_RD-1, 
                                 snd_MaxVolume * 8, -96 * FRACUNIT, 127); 
                }

                speaker_test_left = false;
            }
            else if (speaker_test_timeout <= 30)
            {
                if (english_language)
                {
                    RD_M_DrawTextSmallENG("right channel >", 108 + wide_delta,
                                          142, CR_GREEN);
                }
                else
                {
                    // ПРАВЫЙ КАНАЛ >
                    RD_M_DrawTextSmallRUS("ghfdsq rfyfk `", 109 + wide_delta, 
                                          142, CR_GREEN);
                }

                if (speaker_test_right)
                {
                    I_StartSound(&S_sfx[SFX_PICKUP_KEY], snd_Channels_RD-1,
                                 snd_MaxVolume * 8, 96 * FRACUNIT, 127);
                }

                speaker_test_right = false;
            }
        }
        dp_translation = NULL;
    }
}

static void M_RD_SoundDevice()
{
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 3;
        else if (snd_sfxdevice == 3)
            snd_sfxdevice = 0;

    // [JN] Not safe for hot-swapping.
    // Reinitialize SFX module
    // InitSfxModule(snd_sfxdevice);
    // 
    // Call sfx device changing routine
    // S_RD_Change_SoundDevice();
    // 
    // Reinitialize sound volume, recalculate sound curve
    // snd_MaxVolume_tmp = snd_MaxVolume;
    // soundchanged = true;
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

static void M_RD_SpeakerTest()
{
    // [JN] We only sets necessary booleans and timer here.
    // Actual testing routine will be done in M_RD_Draw_Audio_System,
    // since it's a timer based events, and we need to keep game tics
    // running while testing is active.
    speaker_test_left = true;
    speaker_test_right = true;
    speaker_test_timeout = 60;
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

        // Artiskip
        RD_M_DrawTextSmallENG(artiskip ? "ON" : "OFF", 292 + wide_delta, 72, CR_NONE);

        // Mouse look
        RD_M_DrawTextSmallENG(mlook ? "ON" : "OFF", 118 + wide_delta, 152, CR_NONE);

        // Invert Y axis
        RD_M_DrawTextSmallENG(mouse_y_invert ? "ON" : "OFF", 133 + wide_delta, 162,
                              !mlook ? CR_GRAY : CR_NONE);

        // Novert
        RD_M_DrawTextSmallENG(!novert ? "ON" : "OFF", 168 + wide_delta, 172,
                              mlook ? CR_GRAY : CR_NONE);
    }
    else
    {
        // Режим постоянного бега
        RD_M_DrawTextSmallRUS(alwaysRun ? "DRK" : "DSRK", 209 + wide_delta, 62, CR_NONE);

        // Скип артефактов
        RD_M_DrawTextSmallRUS(artiskip ? "DRK" : "DSRK", 263 + wide_delta, 72, CR_NONE);

        // Обзор мышью
        RD_M_DrawTextSmallRUS(mlook ? "DRK" : "DSRK", 132 + wide_delta, 152, CR_NONE);

        // Вертикальная инверсия
        RD_M_DrawTextSmallRUS(mouse_y_invert ? "DRK" : "DSRK", 199 + wide_delta, 162,
                              !mlook ? CR_GRAY : CR_NONE);

        // Вертикальное перемещение
        RD_M_DrawTextSmallRUS(!novert ? "DRK" : "DSRK", 227 + wide_delta, 172,
                              mlook ? CR_GRAY : CR_NONE);
    }

    //
    // Sliders
    //

    // Mouse sensivity
    RD_Menu_DrawSliderSmall(&ControlsMenu, 102, 12, mouseSensitivity);
    M_snprintf(num, 4, "%d", mouseSensitivity);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 103, CR_GRAY);

    // Acceleration
    RD_Menu_DrawSliderSmall(&ControlsMenu, 122, 12, mouse_acceleration * 4 - 4);
    M_snprintf(num, 4, "%f", mouse_acceleration);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 123, CR_GRAY);

    // Threshold
    RD_Menu_DrawSliderSmall(&ControlsMenu, 142, 12, mouse_threshold / 2);
    M_snprintf(num, 4, "%d", mouse_threshold);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 143, CR_GRAY);
}

static void M_RD_AlwaysRun()
{
    alwaysRun ^= 1;
}

static void M_RD_Artiskip()
{
    artiskip ^= 1;
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

static void M_RD_Acceleration(Direction_t direction)
{
    RD_Menu_SlideFloat_Step(&mouse_acceleration, 1.1F, 5.0F, 0.1F, direction);
}

static void M_RD_Threshold(Direction_t direction)
{
    RD_Menu_SlideInt(&mouse_threshold, 0, 32, direction);
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
static void M_RD_Draw_Bindings()
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        RD_M_DrawTextSmallENG("ENTER TO CHANGE, DEL TO CLEAR", 55 + wide_delta, 176, CR_RED);
        RD_M_DrawTextSmallENG("PGUP/PGDN TO TURN PAGES", 75 + wide_delta, 185, CR_RED);
    }
    else
    {
        RD_M_DrawTextSmallENG("ENTER =", 44 + wide_delta, 176, CR_RED);
        RD_M_DrawTextSmallRUS("= YFPYFXBNM<", 88 + wide_delta, 176, CR_RED);
        RD_M_DrawTextSmallENG("DEL =", 176 + wide_delta, 176, CR_RED);
        RD_M_DrawTextSmallRUS("JXBCNBNM", 213 + wide_delta, 176, CR_RED);

        RD_M_DrawTextSmallENG("PGUP/PGDN =", 55 + wide_delta, 185, CR_RED);
        RD_M_DrawTextSmallRUS("KBCNFNM CNHFYBWS", 139 + wide_delta, 185, CR_RED);
    }

    RD_Menu_Draw_Bindings(english_language ? 195 : 230);
}

// -----------------------------------------------------------------------------
// DrawResetControlsMenu
// -----------------------------------------------------------------------------

static void DrawResetControlsMenu()
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        RD_M_DrawTextBigENG("CONTROLS RESET", 96 + wide_delta, 42);

        // Explanations
        RD_M_DrawTextSmallENG("CONTROLS WILL BE RESET TO", 75 + wide_delta, 145, CR_RED);
        if (CurrentItPos == 0)
        {
            RD_M_DrawTextSmallENG("PORT'S DEFAULTS", 105 + wide_delta, 155, CR_RED);
        }
        else
        {
            RD_M_DrawTextSmallENG("ORIGINAL HEXEN DEFAULTS", 81 + wide_delta, 155, CR_RED);
        }
    }
    else
    {
        RD_M_DrawTextBigRUS("C,HJC EGHFDKTYBZ", 82 + wide_delta, 42);  // СБРОС УПРАВЛЕНИЯ

        // Пояснения
        RD_M_DrawTextSmallRUS(",ELTN BCGJKMPJDFYJ EGHFDKTYBT", 60 + wide_delta, 145, CR_RED);  // Будет использовано управление
        if (CurrentItPos == 0)
        {
            RD_M_DrawTextSmallRUS("HTRJVTYLETVJT GJHNJV", 92 + wide_delta, 155, CR_RED);  // рекомендуемое портом
        }
        else
        {
            RD_M_DrawTextSmallRUS("JHBUBYFKMYJUJ", 98 + wide_delta, 155, CR_RED);
            RD_M_DrawTextSmallENG("HEXEN", 200 + wide_delta, 155, CR_RED);
        }
    }
}

static void M_RD_ResetControls_Recommended()
{
    for(bound_key_t key = bk_forward; key < bk__serializable; key++)
    {
        BK_ClearBinds(key);
    }
    BK_ApplyDefaultBindings();
    RD_Menu_SetMenu(&Bindings1Menu);
}

static void M_RD_ResetControls_Original()
{
    for(bound_key_t key = bk_forward; key < bk__serializable; key++)
    {
        BK_ClearBinds(key);
    }
    BK_ApplyVanilaBindings();
    RD_Menu_SetMenu(&Bindings1Menu);
}

// -----------------------------------------------------------------------------
// DrawGamepadMenu
// -----------------------------------------------------------------------------

static void InitControllerSelectMenu(struct Menu_s* const menu)
{
    for(int i = 3; i < 13; ++i)
    {
        if(activeControllers[i - 3] != NULL)
        {
            menu->items[i].status = ENABLED;
            menu->items[i].option = i - 3;
        }
        else
        {
            menu->items[i].status = HIDDEN;
            menu->items[i].option = -1;
        }
    }

    currentController = NULL;
}

static void DrawGamepadSelectMenu()
{
    static char name[30];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if(english_language)
    {
        RD_M_DrawTextSmallENG(useController ? "ON" : "OFF", 190 + wide_delta, 32,
                              useController ? CR_GREEN : CR_RED);
    }
    else
    {
        RD_M_DrawTextSmallRUS(useController ? "DRK" : "DSRK", 223 + wide_delta, 32,
                              useController ? CR_GREEN : CR_RED);
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
    InitControllerSelectMenu(&GamepadSelectMenu);
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
                              120 + wide_delta, 32, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 42, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? "ON" : "OFF",
                              120 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 62, CR_GRAY);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTY]),
                              120 + wide_delta, 82, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 92, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? "ON" : "OFF",
                              120 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 112, CR_GRAY);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT]),
                              130 + wide_delta, 132, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallENG(num, 260 + wide_delta, 142, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? "ON" : "OFF",
                              120 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 162, CR_GRAY);
    }
    else
    {
        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTX]),
                              110 + wide_delta, 32, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 42, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? "DRK" : "DSRK",
                              155 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTX] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTX]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 62, CR_GRAY);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_LEFTY]),
                              107 + wide_delta, 82, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 92, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? "DRK" : "DSRK",
                              155 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_LEFTY] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_LEFTY]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 112, CR_GRAY);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT]),
                              127 + wide_delta, 132, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallRUS(num, 290 + wide_delta, 142, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? "DRK" : "DSRK",
                              155 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERLEFT] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERLEFT]);
        RD_M_DrawTextSmallRUS(num, 223 + wide_delta, 162, CR_GRAY);
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
                              125 + wide_delta, 32, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 42, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? "ON" : "OFF",
                              120 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 62, CR_GRAY);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY]),
                              125 + wide_delta, 82, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallENG(num, 263 + wide_delta, 92, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? "ON" : "OFF",
                              120 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 112, CR_GRAY);


        RD_M_DrawTextSmallENG(GetAxisNameENG(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]),
                              135 + wide_delta, 132, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(115, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallENG(num, 260 + wide_delta, 142, CR_GRAY);

        RD_M_DrawTextSmallENG(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? "ON" : "OFF",
                              120 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(110, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallENG(num, 210 + wide_delta, 162, CR_GRAY);
    }
    else
    {
        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTX]),
                              117 + wide_delta, 32, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 42, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 42, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? "DRK" : "DSRK",
                              155 + wide_delta, 52,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTX] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 62, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTX]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 62, CR_GRAY);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_RIGHTY]),
                              117 + wide_delta, 82, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 92, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallRUS(num, 293 + wide_delta, 92, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? "DRK" : "DSRK",
                              155 + wide_delta, 102,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_RIGHTY] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 112, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_RIGHTY]);
        RD_M_DrawTextSmallRUS(num, 217 + wide_delta, 112, CR_GRAY);


        RD_M_DrawTextSmallRUS(GetAxisNameRUS(currentController->bindAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]),
                              135 + wide_delta, 132, CR_YELLOW);

        RD_Menu_DrawSliderSmallInline(145, 142, 16,
                                      currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] - 1);
        if(currentController)
            M_snprintf(num, 6, "%2d", currentController->axisSensitivity[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallRUS(num, 290 + wide_delta, 142, CR_GRAY);

        RD_M_DrawTextSmallRUS(currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? "DRK" : "DSRK",
                              155 + wide_delta, 152,
                              currentController->invertAxis[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] ? CR_GREEN : CR_RED);

        RD_Menu_DrawSliderSmallInline(117, 162, 10,
                                      currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] / 10);
        if(currentController)
            M_snprintf(num, 6, "%3d%%", currentController->axisDeadZone[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]);
        RD_M_DrawTextSmallRUS(num, 223 + wide_delta, 162, CR_GRAY);
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
                              brightmaps ? CR_GREEN : CR_RED);

        // Fake contrast
        RD_M_DrawTextSmallENG(fake_contrast ? "ON" : "OFF", 143 + wide_delta, 52,
                              fake_contrast ? CR_GREEN : CR_RED);

        // Extra translucency
        RD_M_DrawTextSmallENG(translucency ? "ON" : "OFF", 180 + wide_delta, 62,
                              translucency ? CR_GREEN : CR_RED);

        // Swirling liquids
        RD_M_DrawTextSmallENG(swirling_liquids ? "ON" : "OFF", 147 + wide_delta, 72,
                              swirling_liquids ? CR_GREEN : CR_RED);

        // Sky drawing mode
        RD_M_DrawTextSmallENG(linear_sky ? "LINEAR" : "ORIGINAL", 162 + wide_delta, 82,
                              linear_sky ? CR_GREEN : CR_RED);

        // Randomly flipped corpses
        RD_M_DrawTextSmallENG(randomly_flipcorpses ? "ON" : "OFF", 232 + wide_delta, 92,
                              randomly_flipcorpses ? CR_GREEN : CR_RED);

        // Flip weapons
        RD_M_DrawTextSmallENG(flip_weapons ? "ON" : "OFF", 130 + wide_delta, 102,
                              flip_weapons ? CR_GREEN : CR_RED);

        //
        // PHYSICAL
        //
       
        // Collision physics
        RD_M_DrawTextSmallENG(improved_collision ? "IMPROVED" : "ORIGINAL", 159 + wide_delta, 122,
                              improved_collision ? CR_GREEN : CR_RED);

        // Corpses sliding from the ledges
        RD_M_DrawTextSmallENG(torque ? "ON" : "OFF", 238 + wide_delta, 132,
                              torque ? CR_GREEN : CR_RED);

        // Floating items amplitude
        RD_M_DrawTextSmallENG(floating_powerups == 1 ? "STANDARD" :
                              floating_powerups == 2 ? "HALFED" : "OFF",
                              209 + wide_delta, 142,
                              floating_powerups ? CR_GREEN : CR_RED);
    }
    else
    {
        //
        // ГРАФИКА
        //

        // Брайтмаппинг
        RD_M_DrawTextSmallRUS(brightmaps ? "DRK" : "DSRK", 133 + wide_delta, 42,
                              brightmaps ? CR_GREEN : CR_RED);

        // Имитация контрастности
        RD_M_DrawTextSmallRUS(fake_contrast ? "DRK" : "DSRK", 205 + wide_delta, 52,
                              fake_contrast ? CR_GREEN : CR_RED);

        // Дополнительная прозрачность
        RD_M_DrawTextSmallRUS(translucency ? "DRK" : "DSRK", 245 + wide_delta, 62,
                              translucency ? CR_GREEN : CR_RED);

        // Улучшенная анимация жидкостей
        RD_M_DrawTextSmallRUS(swirling_liquids ? "DRK" : "DSRK", 261 + wide_delta, 72,
                              swirling_liquids ? CR_GREEN : CR_RED);

        // Режим отрисовки неба
        RD_M_DrawTextSmallRUS(linear_sky ? "KBYTQYSQ" : "JHBUBYFKMYSQ", 195 + wide_delta, 82,
                              linear_sky ? CR_GREEN : CR_RED);

        // Зеркальное отражение трупов
        RD_M_DrawTextSmallRUS(randomly_flipcorpses ? "DRK" : "DSRK", 247 + wide_delta, 92,
                              randomly_flipcorpses ? CR_GREEN : CR_RED);

        // Зеркальное отражение оружия
        RD_M_DrawTextSmallRUS(flip_weapons ? "DRK" : "DSRK", 250 + wide_delta, 102,
                              flip_weapons ? CR_GREEN : CR_RED);

        //
        // ФИЗИКА
        //

        // Физика столкновений
        RD_M_DrawTextSmallRUS(improved_collision ? "EKEXITYYFZ" : "JHBUBYFKMYFZ", 186 + wide_delta, 122,
                              improved_collision ? CR_GREEN : CR_RED);

        // Трупы сползают с возвышений
        RD_M_DrawTextSmallRUS(torque ? "DRK" : "DSRK", 248 + wide_delta, 132,
                              torque ? CR_GREEN : CR_RED);

        // Амплитуда левитации предметов
        RD_M_DrawTextSmallRUS(floating_powerups == 1 ? "CNFYLFHNYFZ" :
                              floating_powerups == 2 ? "EVTHTYYFZ" : "DSRK",
                              188 + wide_delta, 142,
                              floating_powerups ? CR_GREEN : CR_RED);
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

static void M_RD_ExtraTrans()
{
    translucency ^= 1;
}

static void M_RD_SwirlingLiquids()
{
    swirling_liquids ^= 1;
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
                          sbar_colored ? CR_GREEN : CR_RED);

        // Colored health gem
        RD_M_DrawTextSmallENG(sbar_colored_gem == 1 ? "BRIGHT" :
                              sbar_colored_gem == 2 ? "DARK" : "OFF",
                              175 + wide_delta, 52,
                              sbar_colored_gem == 1 ? CR_GREEN :
                              sbar_colored_gem == 2 ? CR_DARKGREEN :
                              CR_RED);

        // Negative health
        RD_M_DrawTextSmallENG(negative_health ? "ON" : "OFF", 190 + wide_delta, 62,
                              negative_health ? CR_GREEN : CR_RED);

        // Artifacts timer
        RD_M_DrawTextSmallENG(show_artifacts_timer == 1 ? "GOLD" :
                              show_artifacts_timer == 2 ? "SILVER" :
                              show_artifacts_timer == 3 ? "COLORED" : "OFF",
                              150 + wide_delta, 72,
                              show_artifacts_timer == 1 ? CR_YELLOW :
                              show_artifacts_timer == 2 ? CR_GRAY :
                              show_artifacts_timer == 3 ? CR_GREEN :
                              CR_RED);

        // Assembled weapon widget
        RD_M_DrawTextSmallENG(weapon_widget == 1 ? "HORIZONTAL" :
                              weapon_widget == 2 ? "VERTICAL" : "OFF",
                              214 + wide_delta, 82,
                              weapon_widget ? CR_GREEN : CR_RED);

        //
        // CROSSHAIR
        //

        // Draw crosshair
        RD_M_DrawTextSmallENG(crosshair_draw ? "ON" : "OFF", 150 + wide_delta, 102,
                              crosshair_draw ? CR_GREEN : CR_RED);

        // Shape
        RD_M_DrawTextSmallENG(crosshair_shape == 1 ? "CROSS/2" :
                              crosshair_shape == 2 ? "X" :
                              crosshair_shape == 3 ? "CIRCLE" :
                              crosshair_shape == 4 ? "ANGLE" :
                              crosshair_shape == 5 ? "TRIANGLE" :
                              crosshair_shape == 6 ? "DOT" : "CROSS",
                              84 + wide_delta, 112, CR_GREEN);

        // Indication
        RD_M_DrawTextSmallENG(crosshair_type == 1 ? "HEALTH" : "STATIC",  111 + wide_delta, 122,
                              crosshair_type ? CR_GREEN : CR_RED);

        // Increased size
        RD_M_DrawTextSmallENG(crosshair_scale ? "ON" : "OFF", 146 + wide_delta, 132,
                              crosshair_scale ? CR_GREEN : CR_RED);
    }
    else
    {
        //
        // СТАТУС-БАР
        //

        // Разноцветные элементы
        RD_M_DrawTextSmallRUS(sbar_colored ? "DRK" : "DSRK", 206 + wide_delta, 42,
                              sbar_colored ? CR_GREEN : CR_RED);

        // Окрашивание камня здоровья
        RD_M_DrawTextSmallRUS(sbar_colored_gem == 1 ? "CDTNKJT" :
                              sbar_colored_gem == 2 ? "NTVYJT" : "DSRK", 238 + wide_delta, 52,
                              sbar_colored_gem == 1 ? CR_GREEN :
                              sbar_colored_gem == 2 ? CR_DARKGREEN :
                              CR_RED);

        // Отрицательное здоровье
        RD_M_DrawTextSmallRUS(negative_health ? "DRK" : "DSRK", 211 + wide_delta, 62,
                              negative_health ? CR_GREEN : CR_RED);

        // Таймер артефаектов
        RD_M_DrawTextSmallRUS(show_artifacts_timer == 1 ? "PJKJNJQ" :
                              show_artifacts_timer == 2 ? "CTHT,HZYSQ" :
                              show_artifacts_timer == 3 ? "HFPYJWDTNYSQ" : "DSRK",
                              175 + wide_delta, 72,
                              show_artifacts_timer == 1 ? CR_YELLOW :
                              show_artifacts_timer == 2 ? CR_GRAY :
                              show_artifacts_timer == 3 ? CR_GREEN :
                              CR_RED);

        // Виджет собранного оружия
        RD_M_DrawTextSmallRUS(weapon_widget == 1 ? "UJHBPJYNFKMYSQ" :
                              weapon_widget == 2 ? "DTHNBRFKMYSQ" : "DSRK",
                              229 + wide_delta, 82,
                              weapon_widget ? CR_GREEN : CR_RED);

        //
        // ПРИЦЕЛ
        //

        // Отображать прицел
        RD_M_DrawTextSmallRUS(crosshair_draw ? "DRK" : "DSRK", 175 + wide_delta, 102,
                              crosshair_draw ? CR_GREEN : CR_RED);

        // Форма
        RD_M_DrawTextSmallRUS(crosshair_shape == 1 ? "RHTCN/2" :      // КРЕСТ/2
                              crosshair_shape == 2 ? "[" :            // X
                              crosshair_shape == 3 ? "RHEU" :         // КРУГ
                              crosshair_shape == 4 ? "EUJK" :         // УГОЛ
                              crosshair_shape == 5 ? "NHTEUJKMYBR" :  // ТРЕУГОЛЬНИК
                              crosshair_shape == 6 ? "NJXRF" :        // ТОЧКА
                                                     "RHTCN",         // КРЕСТ
                              87 + wide_delta, 112, CR_GREEN);

        // Индикация
        RD_M_DrawTextSmallRUS(crosshair_type == 1 ? "PLJHJDMT" : // ЗДОРОВЬЕ
                                                    "CNFNBXYFZ", // СТАТИЧНАЯ
                              111 + wide_delta, 122, crosshair_type ? CR_GREEN : CR_RED);

        // Увеличенный размер
        RD_M_DrawTextSmallRUS(crosshair_scale ? "DRK" : "DSRK", 181 + wide_delta, 132,
                              crosshair_scale ? CR_GREEN : CR_RED);
    }

    // Draw crosshair background.
    V_DrawPatch(235 + wide_delta, 113, W_CacheLumpName("XHAIRBOX", PU_CACHE), NULL);
    // Colorize crosshair depending on it's type.
    Crosshair_Colorize_inMenu();
    // Draw crosshair preview.
    if (crosshair_scale)
    {
        V_DrawPatch(250 + wide_delta, 128, CrosshairPatch, CrosshairOpacity);
    }
    else
    {
        V_DrawPatchUnscaled(500 + wide_delta*2, 256, CrosshairPatch, CrosshairOpacity);
    }
    // Clear colorization.
    dp_translation = NULL;

    // Opacity | Непрозрачность
    RD_M_DrawTextSmallENG(crosshair_opacity == 0 ? "20%" :
                          crosshair_opacity == 1 ? "30%" :
                          crosshair_opacity == 2 ? "40%" :
                          crosshair_opacity == 3 ? "50%" :
                          crosshair_opacity == 4 ? "60%" :
                          crosshair_opacity == 5 ? "70%" :
                          crosshair_opacity == 6 ? "80%" :
                          crosshair_opacity == 7 ? "90%" : "100%",
                          (english_language ? 95 : 149) + wide_delta,
                          142, CR_GRAY);
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

static void M_RD_ShowArtiTimer(Direction_t direction)
{
    RD_Menu_SpinInt(&show_artifacts_timer, 0, 3, direction);
}

static void M_RD_WeaponWidget(Direction_t direction)
{
    RD_Menu_SpinInt(&weapon_widget, 0, 2, direction);
}

static void M_RD_CrossHairDraw()
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairShape(Direction_t direction)
{
    RD_Menu_SpinInt(&crosshair_shape, 0, 6, direction);
    Crosshair_DefinePatch();
}

static void M_RD_CrossHairType()
{
    crosshair_type ^= 1;
}

static void M_RD_CrossHairScale()
{
    crosshair_scale ^= 1;
    Crosshair_DefineDrawingFunc();
}

static void M_RD_CrossHairOpacity(Direction_t direction)
{
    RD_Menu_SlideInt(&crosshair_opacity, 0, 8, direction);
    Crosshair_DefineOpacity();
}


// -----------------------------------------------------------------------------
// DrawGameplay2Menu
// -----------------------------------------------------------------------------

static void DrawGameplay3Menu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // GAMEPLAY
        //

        // Fix errors of vanilla maps
        RD_M_DrawTextSmallENG(fix_map_errors ? "ON" : "OFF", 226 + wide_delta, 42,
                              fix_map_errors ? CR_GREEN : CR_RED);

        // Flip game levels
        RD_M_DrawTextSmallENG(flip_levels ? "ON" : "OFF", 153 + wide_delta, 52,
                              flip_levels ? CR_GREEN : CR_RED);

        // Play internal demos
        RD_M_DrawTextSmallENG(no_internal_demos ? "OFF" : "ON", 179 + wide_delta, 62,
                              no_internal_demos ? CR_RED : CR_GREEN);

        // Imitate player's breathing
        RD_M_DrawTextSmallENG(breathing ? "ON" : "OFF", 224 + wide_delta, 72,
                              breathing ? CR_GREEN : CR_RED);
    }
    else
    {
        //
        // ГЕЙМПЛЕЙ
        //

        // Устранять ошибки оригинальных уровней
        RD_M_DrawTextSmallRUS(fix_map_errors ? "DRK" : "DSRK", 257 + wide_delta, 42,
                              fix_map_errors ? CR_GREEN : CR_RED);

        // Зеркальное отражение уровней
        RD_M_DrawTextSmallRUS(flip_levels ? "DRK" : "DSRK", 255 + wide_delta, 52,
                              flip_levels ? CR_GREEN : CR_RED);

        // Проигрывать демозаписи
        RD_M_DrawTextSmallRUS(no_internal_demos ? "DRK" : "DSRK", 211 + wide_delta, 62,
                              no_internal_demos ? CR_RED : CR_GREEN);

        // Имитация дыхания игрока
        RD_M_DrawTextSmallRUS(breathing ? "DRK": "DSRK", 214 + wide_delta, 72,
                              breathing ? CR_GREEN : CR_RED);
    }
}

static void M_RD_FixMapErrors()
{
    fix_map_errors ^= 1;
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

static void M_RD_Breathing()
{
    breathing ^= 1;
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
                              selective_class == 1 ? "RKBHBR" :
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
                          selective_health >= 67 ? CR_GREEN :
                          selective_health >= 34 ? CR_YELLOW :
                          CR_RED);

    // ARMOR | БРОНЯ
    totalArmor = selective_armor_0 + selective_armor_1 + selective_armor_2 + selective_armor_3 +
            (selective_class == PCLASS_FIGHTER ? 3 : selective_class == PCLASS_CLERIC ? 2 : 1);
    M_snprintf(num, 4, "%d", totalArmor);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
         totalArmor > ArmorMaxTotal[selective_class] ? CR_GREEN :
         totalArmor == ArmorMaxTotal[selective_class] ? CR_YELLOW :
         CR_NONE);

    // MESH ARMOR | КОЛЬЧУГА
    M_snprintf(num, 4, "%d", selective_armor_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
        selective_armor_0 > ArmorMax[selective_class][ARMOR_ARMOR] ? CR_GREEN :
        selective_armor_0 == ArmorMax[selective_class][ARMOR_ARMOR] ? CR_YELLOW :
        CR_NONE);

    // FALCON SHIELD | СОКОЛИНЫЙ ЩИТ
    M_snprintf(num, 4, "%d", selective_armor_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
        selective_armor_1 > ArmorMax[selective_class][ARMOR_SHIELD] ? CR_GREEN :
        selective_armor_1 == ArmorMax[selective_class][ARMOR_SHIELD] ? CR_YELLOW :
        CR_NONE);

    // PLATINUM HELMET | ПЛАТИНОВЫЙ ШЛЕМ
    M_snprintf(num, 4, "%d", selective_armor_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
        selective_armor_2 > ArmorMax[selective_class][ARMOR_HELMET] ? CR_GREEN :
        selective_armor_2 == ArmorMax[selective_class][ARMOR_HELMET] ? CR_YELLOW :
        CR_NONE);

    // AMULET OF WARDING | АМУЛЕТ СТРАЖА
    M_snprintf(num, 4, "%d", selective_armor_3);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
        selective_armor_3 > ArmorMax[selective_class][ARMOR_AMULET] ? CR_GREEN :
        selective_armor_3 == ArmorMax[selective_class][ARMOR_AMULET] ? CR_YELLOW :
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
                              selective_wp_second ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallENG(selective_wp_third ? "YES" : "NO", 248 + wide_delta, 46,
                              selective_wp_third ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallENG(selective_wp_fourth ? "YES" : "NO", 248 + wide_delta, 56,
                              selective_wp_fourth ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallENG(selective_wp_piece_0 ? "YES" : "NO", 248 + wide_delta, 66,
                              selective_wp_piece_0 ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallENG(selective_wp_piece_1 ? "YES" : "NO", 248 + wide_delta, 76,
                              selective_wp_piece_1 ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallENG(selective_wp_piece_2 ? "YES" : "NO", 248 + wide_delta, 86,
                              selective_wp_piece_2 ? CR_GREEN : CR_RED);
    }
    else
    {
        RD_M_DrawTextSmallRUS(selective_wp_second ? "LF" : "YTN", 248 + wide_delta, 36,
                              selective_wp_second ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallRUS(selective_wp_third ? "LF" : "YTN", 248 + wide_delta, 46,
                              selective_wp_third ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallRUS(selective_wp_fourth ? "LF" : "YTN", 248 + wide_delta, 56,
                              selective_wp_fourth ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallRUS(selective_wp_piece_0 ? "LF" : "YTN", 248 + wide_delta, 66,
                              selective_wp_piece_0 ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallRUS(selective_wp_piece_1 ? "LF" : "YTN", 248 + wide_delta, 76,
                              selective_wp_piece_1 ? CR_GREEN : CR_RED);

        RD_M_DrawTextSmallRUS(selective_wp_piece_2 ? "LF" : "YTN", 248 + wide_delta, 86,
                              selective_wp_piece_2 ? CR_GREEN : CR_RED);
    }

    // BLUE MANA
    M_snprintf(num, 4, "%d", selective_ammo_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_ammo_0 >= 100 ? CR_GREEN :
                          selective_ammo_0 >= 50 ? CR_YELLOW :
                          CR_RED);

    // GREEN MANA
    M_snprintf(num, 4, "%d", selective_ammo_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_ammo_1 >= 100 ? CR_GREEN :
                          selective_ammo_1 >= 50 ? CR_YELLOW :
                          CR_RED);

    // QUARTZ FLASK
    M_snprintf(num, 4, "%d", selective_arti_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_arti_0 ? CR_NONE : CR_GRAY);

    // MYSTIC URN
    M_snprintf(num, 4, "%d", selective_arti_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_arti_1 ? CR_NONE : CR_GRAY);

    // FLECHETTE
    M_snprintf(num, 4, "%d", selective_arti_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_arti_2 ? CR_NONE : CR_GRAY);

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
                          selective_arti_3 ? CR_NONE : CR_GRAY);

    //ICON OF THE DEFENDER
    M_snprintf(num, 4, "%d", selective_arti_4);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 36,
                          selective_arti_4 ? CR_NONE : CR_GRAY);

    //PORKALATOR
    M_snprintf(num, 4, "%d", selective_arti_5);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 46,
                          selective_arti_5 ? CR_NONE : CR_GRAY);

    //CHAOS DEVICE
    M_snprintf(num, 4, "%d", selective_arti_6);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 56,
                          selective_arti_6 ? CR_NONE : CR_GRAY);

    //BANISHMENT DEVICE
    M_snprintf(num, 4, "%d", selective_arti_7);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 66,
                          selective_arti_7 ? CR_NONE : CR_GRAY);

    //WINGS OF WRATH
    M_snprintf(num, 4, "%d", selective_arti_8);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 76,
                          selective_arti_8 ? CR_NONE : CR_GRAY);

    //TORCH
    M_snprintf(num, 4, "%d", selective_arti_9);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
                          selective_arti_9 ? CR_NONE : CR_GRAY);

    //KRATER OF MIGHT
    M_snprintf(num, 4, "%d", selective_arti_10);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
                          selective_arti_10 ? CR_NONE : CR_GRAY);

    //DRAGONSKIN BRACERS
    M_snprintf(num, 4, "%d", selective_arti_11);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_arti_11 ? CR_NONE : CR_GRAY);

    //DARK SERVANT
    M_snprintf(num, 4, "%d", selective_arti_12);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_arti_12 ? CR_NONE : CR_GRAY);

    //BOOTS OF SPEED
    M_snprintf(num, 4, "%d", selective_arti_13);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
                          selective_arti_13 ? CR_NONE : CR_GRAY);

    // MYSTIC AMBIT INCANT
    M_snprintf(num, 4, "%d", selective_arti_14);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_arti_14 ? CR_NONE : CR_GRAY);

    if (english_language)
    {
        // EMERALD KEY
        RD_M_DrawTextSmallENG(selective_key_0 ? "YES" : "NO", 248 + wide_delta, 156,
                              selective_key_0 ? CR_GREEN : CR_RED);
    }
    else
    {
        // ИЗУМРУДНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_0 ? "LF" : "YTN", 248 + wide_delta, 156,
                              selective_key_0 ? CR_GREEN : CR_RED);
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
                              selective_key_1 ? CR_GREEN : CR_RED);

        // FIRE KEY
        RD_M_DrawTextSmallENG(selective_key_2 ? "YES" : "NO", 248 + wide_delta, 36,
                              selective_key_2 ? CR_GREEN : CR_RED);

        // STEEL KEY
        RD_M_DrawTextSmallENG(selective_key_3 ? "YES" : "NO", 248 + wide_delta, 46,
                              selective_key_3 ? CR_GREEN : CR_RED);

        // HORN KEY
        RD_M_DrawTextSmallENG(selective_key_4 ? "YES" : "NO", 248 + wide_delta, 56,
                              selective_key_4 ? CR_GREEN : CR_RED);

        // CAVE KEY
        RD_M_DrawTextSmallENG(selective_key_5 ? "YES" : "NO", 248 + wide_delta, 66,
                              selective_key_5 ? CR_GREEN : CR_RED);

        // CASTLE KEY
        RD_M_DrawTextSmallENG(selective_key_6 ? "YES" : "NO", 248 + wide_delta, 76,
                              selective_key_6 ? CR_GREEN : CR_RED);

        // SWAMP KEY
        RD_M_DrawTextSmallENG(selective_key_7 ? "YES" : "NO", 248 + wide_delta, 86,
                              selective_key_7 ? CR_GREEN : CR_RED);

        // RUSTED KEY
        RD_M_DrawTextSmallENG(selective_key_8 ? "YES" : "NO", 248 + wide_delta, 96,
                              selective_key_8 ? CR_GREEN : CR_RED);

        // DUNGEON KEY
        RD_M_DrawTextSmallENG(selective_key_9 ? "YES" : "NO", 248 + wide_delta, 106,
                              selective_key_9 ? CR_GREEN : CR_RED);

        // AXE KEY
        RD_M_DrawTextSmallENG(selective_key_10 ? "YES" : "NO", 248 + wide_delta, 116,
                              selective_key_10 ? CR_GREEN : CR_RED);
    }
    else
    {
        // СЕРЕБРЯНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_1 ? "LF" : "YTN", 248 + wide_delta, 26,
                              selective_key_1 ? CR_GREEN : CR_RED);

        // ОГНЕННЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_2 ? "LF" : "YTN", 248 + wide_delta, 36,
                              selective_key_2 ? CR_GREEN : CR_RED);

        // СТАЛЬНОЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_3 ? "LF" : "YTN", 248 + wide_delta, 46,
                              selective_key_3 ? CR_GREEN : CR_RED);

        // РОГОВОЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_4 ? "LF" : "YTN", 248 + wide_delta, 56,
                              selective_key_4 ? CR_GREEN : CR_RED);

        // ПЕЩЕРНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_5 ? "LF" : "YTN", 248 + wide_delta, 66,
                              selective_key_5 ? CR_GREEN : CR_RED);

        // КЛЮЧ ОТ ЗАМКА
        RD_M_DrawTextSmallRUS(selective_key_6 ? "LF" : "YTN", 248 + wide_delta, 76,
                              selective_key_6 ? CR_GREEN : CR_RED);

        // БОЛОТНЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_7 ? "LF" : "YTN", 248 + wide_delta, 86,
                              selective_key_7 ? CR_GREEN : CR_RED);

        // РЖАВЫЙ КЛЮЧ
        RD_M_DrawTextSmallRUS(selective_key_8 ? "LF" : "YTN", 248 + wide_delta, 96,
                              selective_key_8 ? CR_GREEN : CR_RED);

        // КЛЮЧ ОТ ПОДЗЕМЕЛЬЯ
        RD_M_DrawTextSmallRUS(selective_key_9 ? "LF" : "YTN", 248 + wide_delta, 106,
                              selective_key_9 ? CR_GREEN : CR_RED);

        // КЛЮЧ-ТОПОР
        RD_M_DrawTextSmallRUS(selective_key_10 ? "LF" : "YTN", 248 + wide_delta, 116,
                              selective_key_10 ? CR_GREEN : CR_RED);
    }
    // FLAME MASK
    M_snprintf(num, 4, "%d", selective_puzzle_0);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_puzzle_0 ? CR_NONE : CR_GRAY);

    // HEART OF D'SPARIL
    M_snprintf(num, 4, "%d", selective_puzzle_1);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_puzzle_1 ? CR_NONE : CR_GRAY);

    // RUBY PLANET
    M_snprintf(num, 4, "%d", selective_puzzle_2);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_puzzle_2 ? CR_NONE : CR_GRAY);

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
                          selective_puzzle_3 ? CR_NONE : CR_GRAY);

    // EMERALD PLANET 2
    M_snprintf(num, 4, "%d", selective_puzzle_4);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 36,
                          selective_puzzle_4 ? CR_NONE : CR_GRAY);

    // SAPPHIRE PLANET 1
    M_snprintf(num, 4, "%d", selective_puzzle_5);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 46,
                          selective_puzzle_5 ? CR_NONE : CR_GRAY);

    // SAPPHIRE PLANET 2
    M_snprintf(num, 4, "%d", selective_puzzle_6);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 56,
                          selective_puzzle_6 ? CR_NONE : CR_GRAY);

    // CLOCK GEAR (B&S)
    M_snprintf(num, 4, "%d", selective_puzzle_7);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 66,
                          selective_puzzle_7 ? CR_NONE : CR_GRAY);

    // CLOCK GEAR (B)
    M_snprintf(num, 4, "%d", selective_puzzle_8);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 76,
                          selective_puzzle_8 ? CR_NONE : CR_GRAY);

    // CLOCK GEAR (S&B)
    M_snprintf(num, 4, "%d", selective_puzzle_9);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 86,
                          selective_puzzle_9 ? CR_NONE : CR_GRAY);

    // CLOCK GEAR (S)
    M_snprintf(num, 4, "%d", selective_puzzle_10);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 96,
                          selective_puzzle_10 ? CR_NONE : CR_GRAY);

    // DAEMON CODEX
    M_snprintf(num, 4, "%d", selective_puzzle_11);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 106,
                          selective_puzzle_11 ? CR_NONE : CR_GRAY);

    // LIBER OSCURA
    if(!english_language)
        RD_M_DrawTextSmallENG("LIBER OSCURA:", 40 + wide_delta, 116, CR_NONE);
    M_snprintf(num, 4, "%d", selective_puzzle_12);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 116,
                          selective_puzzle_12 ? CR_NONE : CR_GRAY);

    // YORICK'S SKULL
    M_snprintf(num, 4, "%d", selective_puzzle_13);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 126,
                          selective_puzzle_13 ? CR_NONE : CR_GRAY);

    // GLAIVE SEAL
    M_snprintf(num, 4, "%d", selective_puzzle_14);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 136,
                          selective_puzzle_14 ? CR_NONE : CR_GRAY);

    // HOlY RELIC
    M_snprintf(num, 4, "%d", selective_puzzle_15);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 146,
                          selective_puzzle_15 ? CR_NONE : CR_GRAY);

    // SIGIL OF THE MAGUS
    M_snprintf(num, 4, "%d", selective_puzzle_16);
    RD_M_DrawTextSmallENG(num, 248 + wide_delta, 156,
                          selective_puzzle_16 ? CR_NONE : CR_GRAY);

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

// -----------------------------------------------------------------------------
// DrawResetSettingsMenu
// -----------------------------------------------------------------------------

static void DrawResetSettingsMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        RD_M_DrawTextBigENG("SETTINGS RESET", 96 + wide_delta, 42);

        RD_M_DrawTextSmallENG("GRAPHICAL, AUDIBLE AND GAMEPLAY SETTINGS", 19 + wide_delta, 65, CR_NONE);
        RD_M_DrawTextSmallENG("WILL BE RESET TO THEIR DEFAULT VALUES.", 31 + wide_delta, 75, CR_NONE);
        RD_M_DrawTextSmallENG("WHICH LEVEL OF VALUES TO USE?", 58 + wide_delta, 85, CR_NONE);

        // Explanations
        RD_M_DrawTextSmallENG("SETTINGS WILL BE RESET TO", 75 + wide_delta, 145, CR_RED);
        if (CurrentItPos == 0)
        {
            RD_M_DrawTextSmallENG("DEFAULT PORT'S VALUES", 85 + wide_delta, 155, CR_RED);
        }
        else
        {
            RD_M_DrawTextSmallENG("ORIGINAL HEXEN VALUES", 86 + wide_delta, 155, CR_RED);
        }
    }
    else
    {
        RD_M_DrawTextBigRUS("C,HJC YFCNHJTR", 82 + wide_delta, 42);  // СБРОС НАСТРОЕК

        RD_M_DrawTextSmallRUS("YFCNHJQRB UHFABRB< PDERF B UTQVGKTZ", 34 + wide_delta, 65, CR_NONE);      // Настройки графики, звука и геймплея
        RD_M_DrawTextSmallRUS(",ELEN C,HJITYS YF CNFYLFHNYST PYFXTYBZ>", 16 + wide_delta, 75, CR_NONE);  // Будут сброшены на стандартные значения.
        RD_M_DrawTextSmallRUS("DS,THBNT EHJDTYM PYFXTYBQ:", 66 + wide_delta, 85, CR_NONE);               // Выберите уровень значений:

        // Пояснения
        RD_M_DrawTextSmallRUS(",ELEN BCGJKMPJDFYS PYFXTYBZ", 60 + wide_delta, 145, CR_RED);  // Будут использованы значения
        if (CurrentItPos == 0)
        {
            RD_M_DrawTextSmallRUS("HTRJVTYLETVST GJHNJV", 82 + wide_delta, 155, CR_RED);  // рекомендуемые портом
        }
        else
        {
            RD_M_DrawTextSmallRUS("JHBUBYFKMYJUJ", 83 + wide_delta, 155, CR_RED);
            RD_M_DrawTextSmallENG("HEXEN", 185 + wide_delta, 155, CR_RED);
        }
    }
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
// M_RD_BackToDefaults_Recommended
//---------------------------------------------------------------------------

void M_RD_BackToDefaults_Recommended (void)
{
    // Rendering
    vsync                        = 1;
    preserve_window_aspect_ratio = 1;
    max_fps                      = 200; uncapped_fps = 1;
    show_fps                     = 0;
    smoothing                    = 0;
    vga_porch_flash              = 0;
    png_screenshots              = 1;

    // Display
    screenblocks           = 10;
    extra_level_brightness = 0;
    hud_detaillevel        = 0;

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
    message_color_pickup = 0;
    message_color_quest  = 0;
    message_color_system = 0;
    message_color_chat   = 6;

    // Automap
    automap_rotate     = 0;
    automap_overlay    = 0;
    automap_overlay_bg = 0;
    automap_follow     = 1;
    automap_grid       = 0;
    automap_grid_size  = 128;
    automap_mark_color = 3;

    // Audio
    snd_MaxVolume   = 8;
    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable,
    soundchanged = true;               // and recalc sound curve.
    snd_MusicVolume = 8;
    S_SetMusicVolume();
    snd_Channels    = 32;
    S_ChannelsRealloc();
    snd_monomode    = 0;
    snd_pitchshift  = 1;
    mute_inactive_window = 0;

    // Controls
    mouseSensitivity   = 5;
    mlook              = 0; players[consoleplayer].centering = true;
    mouse_acceleration = 2.0F;
    mouse_threshold    = 10;
    novert             = 1;
    artiskip           = 0;

    // Gameplay (1)
    brightmaps           = 1;
    fake_contrast        = 0;
    translucency         = 1;
    swirling_liquids     = 1;
    linear_sky           = 1;
    randomly_flipcorpses = 1;
    flip_weapons         = 0;
    // Gameplay (2)
    improved_collision   = 1;
    torque               = 1;
    floating_powerups    = 1;
    // Gameplay (3)
    sbar_colored         = 0;
    sbar_colored_gem     = 0;
    negative_health      = 0;
    show_artifacts_timer = 0;
    weapon_widget        = 0;
    // Gameplay (4)
    crosshair_draw       = 0;
    crosshair_shape      = 0;
    crosshair_type       = 1;
    crosshair_scale      = 0;
    crosshair_opacity    = 8;
    Crosshair_DefinePatch();
    Crosshair_DefineOpacity();
    Crosshair_DefineDrawingFunc();
    // Gameplay (5)
    flip_levels          = 0;
    no_internal_demos    = 0;
    breathing            = 0;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Reset palette.
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

    R_SetViewSize(screenblocks);

    // Update status bar
    SB_state = -1;
    BorderNeedRefresh = true;

    P_SetMessage(&players[consoleplayer], txt_settings_reset, msg_system, false);
    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
    menuactive = true;
}

static void M_RD_BackToDefaults_Original(void)
{
    // Rendering
    vsync                        = 1;
    preserve_window_aspect_ratio = 1;
    max_fps                      = 35; uncapped_fps = 1;
    show_fps                     = 0;
    smoothing                    = 0;
    vga_porch_flash              = 0;
    png_screenshots              = 1;

    // Display
    screenblocks           = 10;
    extra_level_brightness = 0;
    hud_detaillevel        = 1;

    // Color options
    brightness       = 1.0f;
    usegamma         = 9;
    color_saturation = 1.0f;
    show_palette     = 1;
    r_color_factor   = 1.0f;
    g_color_factor   = 1.0f;
    b_color_factor   = 1.0f;

    // Messages and Texts
    show_messages      = 1;
    messages_alignment = 0;
    messages_timeout   = 4;
    message_fade       = 0;
    draw_shadowed_text = 0;
    local_time         = 0;
    message_color_pickup = 0;
    message_color_quest  = 0;
    message_color_system = 0;
    message_color_chat   = 0;

    // Automap
    automap_rotate     = 0;
    automap_overlay    = 0;
    automap_overlay_bg = 0;
    automap_follow     = 1;
    automap_grid       = 0;
    automap_grid_size  = 128;
    automap_mark_color = 3;

    // Audio
    snd_MaxVolume   = 8;
    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable,
    soundchanged = true;               // and recalc sound curve.
    snd_MusicVolume = 8;
    S_SetMusicVolume();
    snd_Channels    = 8;
    S_ChannelsRealloc();
    snd_monomode    = 0;
    snd_pitchshift  = 1;
    mute_inactive_window = 0;

    // Controls
    mouseSensitivity   = 5;
    mlook              = 0; players[consoleplayer].centering = true;
    mouse_acceleration = 2.0F;
    mouse_threshold    = 10;
    novert             = 1;
    artiskip           = 0;

    // Gameplay (1)
    brightmaps           = 0;
    fake_contrast        = 0;
    translucency         = 0;
    swirling_liquids     = 0;
    linear_sky           = 0;
    randomly_flipcorpses = 0;
    flip_weapons         = 0;
    // Gameplay (2)
    improved_collision   = 0;
    torque               = 0;
    floating_powerups    = 1;
    // Gameplay (3)
    sbar_colored         = 0;
    sbar_colored_gem     = 0;
    negative_health      = 0;
    show_artifacts_timer = 0;
    weapon_widget        = 0;
    // Gameplay (4)
    crosshair_draw       = 0;
    crosshair_shape      = 0;
    crosshair_type       = 1;
    crosshair_scale      = 0;
    crosshair_opacity    = 8;
    Crosshair_DefinePatch();
    Crosshair_DefineOpacity();
    Crosshair_DefineDrawingFunc();
    // Gameplay (5)
    flip_levels          = 0;
    no_internal_demos    = 0;
    breathing            = 0;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Reset palette.
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

    R_SetViewSize(screenblocks);

    // Update status bar
    SB_state = -1;
    BorderNeedRefresh = true;

    P_SetMessage(&players[consoleplayer], txt_settings_reset, msg_system, false);
    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
    menuactive = true;
}

//---------------------------------------------------------------------------
// M_RD_ChangeLanguage
//---------------------------------------------------------------------------

static void M_RD_ChangeLanguage(Direction_t direction)
{
    extern void H2_DoAdvanceDemo(void);
    extern int demosequence;

    english_language ^= 1;

    // Clear HUD messages
    players[consoleplayer].message[0] = 0;
    players[consoleplayer].yellowMessage = 0;

    // Update language strings
    RD_DefineLanguageStrings();

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
            P_SetMessage(&players[consoleplayer], txt_cant_start_in_netgame, msg_system, true);
            break;
        case 2:                // load game
            P_SetMessage(&players[consoleplayer], txt_cant_load_in_netgame, msg_system, true);
            break;
        case 3:                // end game
            P_SetMessage(&players[consoleplayer], txt_cant_end_in_netgame, msg_system, true);
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
    RD_Menu_DeactivateMenu(false);
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
        RD_Menu_DeactivateMenu(true);
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
        P_SetMessage(&players[consoleplayer], txt_cant_start_in_netgame, msg_system, true);
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
            break;
        case PCLASS_RANDOM:
            RD_Menu_SetMenu(&SkillMenu_R);
            break;
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

    if (MenuPClass < 3)
    {
        PlayerClass[consoleplayer] = MenuPClass;
    }
    else
    {
        // [JN] Let the random choose player class:
        PlayerClass[consoleplayer] = rand() % 3;
    }
    G_DeferredNewGame(option);
    SB_SetClassData();
    SB_state = -1;
    RD_Menu_DeactivateMenu(true);
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

            // [JN] Rendering resolution: remember choosen widescreen variable before quit.
            rendering_resolution = rendering_resolution_temp;
            // [JN] Widescreen: remember choosen widescreen variable before quit.
            aspect_ratio = aspect_ratio_temp;
            // [JN] Screen renderer: remember choosen renderer variable before quit.
            opengles_renderer = opengles_renderer_temp;
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
            RD_Menu_DeactivateMenu(true);
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
        return (true);          //make the info screen eat the keypress
    }

    if ((ravpic && event->data1 == SDL_SCANCODE_F1) || BK_isKeyDown(event, bk_screenshot))
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
                    // [JN] Rendering resolution: remember choosen widescreen variable before quit.
                    rendering_resolution = rendering_resolution_temp;
                    // [JN] Widescreen: remember choosen widescreen variable before quit.
                    aspect_ratio = aspect_ratio_temp;
                    // [JN] Screen renderer: remember choosen renderer variable before quit.
                    opengles_renderer = opengles_renderer_temp;
                    I_Quit();
                    return false;
                case 2:
                    P_ClearMessage(&players[consoleplayer]);
                    askforquit = false;
                    typeofask = 0;
                    paused = false;
                    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
                    // [JN] Force to initialize map music number.
                    S_ResetMapMusicNumber();
                    // [JN] Start title music.
                    S_StartSongName("hexen", false);
                    H2_StartTitle();    // go to intro/demo mode.
                    return false;
                case 5:
                    BorderNeedRefresh = true;
                    mn_SuicideConsole = true;
                    break;
                // [JN] Delete saved game:
                case 6:
                    SV_ClearSaveSlot(CurrentItPos);

                    // Truncate text of saved game slot.
                    memset(SlotText[CurrentItPos], 0, SLOTTEXTLEN + 2);
                    // Return to the Save/Load menu.
                    menuactive = true;
                    // Indicate that slot text needs to be updated.
                    slottextloaded = false;
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                    // Redraw Save/Load items.
                    DrawSaveLoadMenu();
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
                QuickSaveTitle = false;
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
                QuickLoadTitle = false;
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
            sfxbgdraw = false; // [JN] Don't draw menu background.
            RD_Menu_SetMenu(vanillaparm ? &VanillaOptions2Menu : &SoundMenu);
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (BK_isKeyDown(event, bk_suicide))         // F5 (suicide)
        {
            // [JN] Allow to invoke only in game level state,
            // and only if player is alive.
            if (gamestate == GS_LEVEL && !demoplayback
            &&  players[consoleplayer].playerstate == PST_LIVE)
            {
                menuactive = false;
                askforquit = true;
                typeofask = 5;  // suicide
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_detail))         // detail
        {
            // TODO - consider replacing with level restart.
            M_RD_BG_Detail();
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            return true;
        }
        else if (BK_isKeyDown(event, bk_qsave))          // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    QuickSaveTitle = true;
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
                }
                else
                {
                    // [JN] Once quick save slot is chosen,
                    // skip confirmation and save immediately.
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                }
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_end_game))        // F7 (end game)
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
                    QuickLoadTitle = true;
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
                }
                else
                {
                // [JN] Once quick load slot is chosen,
                // skip confirmation and load immediately.
                SCLoadGame(quickload - 1);
                BorderNeedRefresh = true;
                }
            }
            return true;
        }
        else if (BK_isKeyDown(event, bk_quit))           // F10 (quit)
        {
            // [JN] Allow to invoke quit responce in any game states.
            SCQuitGame(0);
            S_StartSound(NULL, SFX_CHAT);
            return true;
        }
        else if (BK_isKeyDown(event, bk_reloadlevel))                 // F12 (???)
        {
            // F12 - reload current map (devmaps mode)
            // [JN] Allow only in devparm mode, see this comment:
            // https://github.com/JNechaevsky/inter-doom/issues/210#issuecomment-702321075

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
            P_SetMessage(&players[consoleplayer], txt_cheatwarp, msg_system, false);
            return true;
        }
    }

    // [JN] Allow to change gamma even while menu is active.
    if (BK_isKeyDown(event, bk_gamma))          // F11 (gamma correction)
    {
        static char *gamma_level;

        usegamma++;
        if (usegamma > 17)
        {
            usegamma = 0;
        }
        SB_PaletteFlash(true);  // force change

        gamma_level = M_StringJoin(txt_gammamsg, english_language ?
                                   gammalevel_names[usegamma] :
                                   gammalevel_names_rus[usegamma], NULL);

        P_SetMessage(&players[consoleplayer], gamma_level, msg_system, false);
        return true;
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

    if (event->type == ev_keydown && event->data1 == SDL_SCANCODE_DELETE)
    {
        // [JN] Save/load menu
        if (CurrentMenu == &LoadMenu
        ||  CurrentMenu == &SaveMenu)
        {
            if (SlotStatus[CurrentItPos] && !FileMenuKeySteal)
            {
                menuactive = false;
                askforquit = true;
                typeofask = 6;
                S_StartSound(NULL, SFX_CHAT);
                return true;
            }
            else
            {
                return true;
            }
        }

        //[Dasperal] Key bindings menus
        if(CurrentMenu == &Bindings1Menu ||
           CurrentMenu == &Bindings2Menu ||
           CurrentMenu == &Bindings3Menu ||
           CurrentMenu == &Bindings4Menu ||
           CurrentMenu == &Bindings5Menu ||
           CurrentMenu == &Bindings6Menu ||
           CurrentMenu == &Bindings7Menu ||
           CurrentMenu == &Bindings8Menu)
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
            RD_Menu_DeactivateMenu(true);
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

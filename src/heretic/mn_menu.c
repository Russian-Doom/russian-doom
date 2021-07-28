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
#include "i_timer.h" // [JN] I_GetTime()
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_trans.h"
#include "v_video.h"
#include "crispy.h"
#include "jn.h"
#include "rd_menu.h"

// Macros
#define ITEM_HEIGHT 20
#define SLOTTEXTLEN     22
#define ASCII_CURSOR '_'

// Private Functions

static void SCQuitGame(int option);
static void SCEpisode(int option);
static void SCSkill(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCMessages(int option);
static void SCInfo(int option);
static void DrawMainMenu(void);
static void DrawFileSlots();
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawSaveLoadMenu(void);
static void DrawOptionsMenu(void);
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
static void M_RD_EndText(Direction_t direction);

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(Direction_t direction);
static void M_RD_Gamma(Direction_t direction);
static void M_RD_LevelBrightness(Direction_t direction);
static void M_RD_Detail(Direction_t direction);

// Messages and Texts
static void DrawMessagesMenu(void);
static void M_RD_Messages(Direction_t direction);
static void M_RD_MessagesAlignment(Direction_t direction);
static void M_RD_MessagesTimeout(Direction_t direction);
static void M_RD_MessagesFade(Direction_t direction);
static void M_RD_ShadowedText(Direction_t direction);
static void M_RD_LocalTime(Direction_t direction);
static void M_RD_Change_Msg_Pickup_Color(Direction_t direction);
static void M_RD_Change_Msg_Secret_Color(Direction_t direction);
static void M_RD_Change_Msg_System_Color(Direction_t direction);
static void M_RD_Change_Msg_Chat_Color(Direction_t direction);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapOverlay(Direction_t direction);
static void M_RD_AutoMapRotate(Direction_t direction);
static void M_RD_AutoMapFollow(Direction_t direction);
static void M_RD_AutoMapGrid(Direction_t direction);
static void M_RD_AutoMapGridSize(Direction_t direction);
static void M_RD_AutoMapStats(Direction_t direction);
static void M_RD_AutoMapLevTime(Direction_t direction);
static void M_RD_AutoMapTotTime(Direction_t direction);
static void M_RD_AutoMapCoords(Direction_t direction);

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
static void M_RD_Acceleration(Direction_t direction);
static void M_RD_Threshold(Direction_t direction);
static void M_RD_MouseLook(Direction_t direction);
static void M_RD_InvertY(Direction_t direction);
static void M_RD_Novert(Direction_t direction);

// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static void M_RD_Brightmaps(Direction_t direction);
static void M_RD_FakeContrast(Direction_t direction);
static void M_RD_ExtraTrans(Direction_t direction);
static void M_RD_ColoredBlood(Direction_t direction);
static void M_RD_SwirlingLiquids(Direction_t direction);
static void M_RD_InvulSky(Direction_t direction);
static void M_RD_LinearSky(Direction_t direction);
static void M_RD_Torque(Direction_t direction);
static void M_RD_Bobbing(Direction_t direction);
static void M_RD_FlipCorpses(Direction_t direction);
static void M_RD_FloatAmplitude(Direction_t direction);

// Gameplay (page 2)
static void DrawGameplay2Menu(void);
static void M_RD_ColoredSBar(Direction_t direction);
static void M_RD_ColoredGem(Direction_t direction);
static void M_RD_NegativeHealth(Direction_t direction);
static void M_RD_AmmoWidgetDraw(Direction_t direction);
static void M_RD_AmmoWidgetColoring(Direction_t direction);
static void M_RD_CrossHairDraw(Direction_t direction);
static void M_RD_CrossHairType(Direction_t direction);
static void M_RD_CrossHairScale(Direction_t direction);

// Gameplay (page 3)
static void DrawGameplay3Menu(void);
static void M_RD_ZAxisSFX(Direction_t direction);
static void M_RD_AlertSFX(Direction_t direction);
static void M_RD_SecretNotify(Direction_t direction);
static void M_RD_ShowAllArti(Direction_t direction);
static void M_RD_ShowArtiTimer(Direction_t direction);
static void M_RD_FixMapErrors(Direction_t direction);
static void M_RD_FlipLevels(Direction_t direction);
static void M_RD_NoDemos(Direction_t direction);
static void M_RD_WandStart(Direction_t direction);

// Level Select (page 1)
static void DrawLevelSelect1Menu(void);
static void M_RD_SelectiveSkill(Direction_t direction);
static void M_RD_SelectiveEpisode(Direction_t direction);
static void M_RD_SelectiveMap(Direction_t direction);
static void M_RD_SelectiveHealth(Direction_t direction);
static void M_RD_SelectiveArmor(Direction_t direction);
static void M_RD_SelectiveArmorType(Direction_t direction);
static void M_RD_SelectiveGauntlets(Direction_t direction);
static void M_RD_SelectiveCrossbow(Direction_t direction);
static void M_RD_SelectiveDragonClaw(Direction_t direction);
static void M_RD_SelectiveHellStaff(Direction_t direction);
static void M_RD_SelectivePhoenixRod(Direction_t direction);
static void M_RD_SelectiveFireMace(Direction_t direction);

// Level Select (page 2)
static void DrawLevelSelect2Menu(void);
static void M_RD_SelectiveBag(Direction_t direction);
static void M_RD_SelectiveAmmo_0(Direction_t direction);
static void M_RD_SelectiveAmmo_1(Direction_t direction);
static void M_RD_SelectiveAmmo_2(Direction_t direction);
static void M_RD_SelectiveAmmo_3(Direction_t direction);
static void M_RD_SelectiveAmmo_4(Direction_t direction);
static void M_RD_SelectiveAmmo_5(Direction_t direction);
static void M_RD_SelectiveKey_0(Direction_t direction);
static void M_RD_SelectiveKey_1(Direction_t direction);
static void M_RD_SelectiveKey_2(Direction_t direction);
static void M_RD_SelectiveFast(Direction_t direction);
static void M_RD_SelectiveRespawn(Direction_t direction);

// Level Select (page 3)
static void DrawLevelSelect3Menu(void);
static void M_RD_SelectiveArti_0(Direction_t direction);
static void M_RD_SelectiveArti_1(Direction_t direction);
static void M_RD_SelectiveArti_2(Direction_t direction);
static void M_RD_SelectiveArti_3(Direction_t direction);
static void M_RD_SelectiveArti_4(Direction_t direction);
static void M_RD_SelectiveArti_5(Direction_t direction);
static void M_RD_SelectiveArti_6(Direction_t direction);
static void M_RD_SelectiveArti_7(Direction_t direction);
static void M_RD_SelectiveArti_8(Direction_t direction);
static void M_RD_SelectiveArti_9(Direction_t direction);

// Vanilla Options menu
static void DrawOptionsMenu_Vanilla(void);
static void DrawOptions2Menu_Vanilla(void);

// End game
static void M_RD_EndGame(int option);

// Reset settings
static void M_RD_ResetSettings(int option);

// Change language
static void M_RD_ChangeLanguage(Direction_t direction);

// Public Data

boolean menuactive;
int InfoType;

// Private Data

static int SkullBaseLump;
static int MenuEpisode;
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

static Translation_CR_t M_RD_ColorTranslation (int color)
{
    switch (color)
    {
        case 1:   return CR_WHITE2GRAY_HERETIC;      break;
        case 2:   return CR_WHITE2DARKGRAY_HERETIC;  break;
        case 3:   return CR_WHITE2RED_HERETIC;       break;
        case 4:   return CR_WHITE2DARKRED_HERETIC;   break;
        case 5:   return CR_WHITE2GREEN_HERETIC;     break;
        case 6:   return CR_WHITE2DARKGREEN_HERETIC; break;
        case 7:   return CR_WHITE2OLIVE_HERETIC;     break;
        case 8:   return CR_WHITE2BLUE_HERETIC;      break;
        case 9:   return CR_WHITE2DARKBLUE_HERETIC;  break;
        case 10:  return CR_WHITE2PURPLE_HERETIC;    break;
        case 11:  return CR_WHITE2NIAGARA_HERETIC;   break;
        case 12:  return CR_WHITE2AZURE_HERETIC;     break;
        case 13:  return CR_WHITE2YELLOW_HERETIC;    break;
        case 14:  return CR_WHITE2GOLD_HERETIC;      break;
        case 15:  return CR_WHITE2DARKGOLD_HERETIC;  break;
        case 16:  return CR_WHITE2TAN_HERETIC;       break;
        case 17:  return CR_WHITE2BROWN_HERETIC;     break;
        default:  return CR_NONE;                    break;
    }
}

static char *M_RD_ColorName (int color)
{
    switch (color)
    {
        case 1:   return english_language ? "GRAY"       : "CTHSQ";          break;  // СЕРЫЙ
        case 2:   return english_language ? "DARK GRAY"  : "NTVYJ-CTHSQ";    break;  // ТЁМНО-СЕРЫЙ
        case 3:   return english_language ? "RED"        : "RHFCYSQ";        break;  // КРАСНЫЙ
        case 4:   return english_language ? "DARK RED"   : "NTVYJ-RHFCYSQ";  break;  // ТЁМНО-КРАСНЫЙ
        case 5:   return english_language ? "GREEN"      : "PTKTYSQ";        break;  // ЗЕЛЕНЫЙ
        case 6:   return english_language ? "DARK GREEN" : "NTVYJ-PTKTYSQ";  break;  // ТЕМНО-ЗЕЛЕНЫЙ
        case 7:   return english_language ? "OLIVE"      : "JKBDRJDSQ";      break;  // ОЛИВКОВЫЙ
        case 8:   return english_language ? "BLUE"       : "CBYBQ";          break;  // СИНИЙ
        case 9:   return english_language ? "DARK BLUE"  : "NTVYJ-CBYBQ";    break;  // ТЕМНО-СИНИЙ
        case 10:  return english_language ? "PURPLE"     : "ABJKTNJDSQ";     break;  // ФИОЛЕТОВЫЙ
        case 11:  return english_language ? "NIAGARA"    : "YBFUFHF";        break;  // НИАГАРА
        case 12:  return english_language ? "AZURE"      : "KFPEHYSQ";       break;  // ЛАЗУРНЫЙ
        case 13:  return english_language ? "YELLOW"     : ";TKNSQ";         break;  // ЖЕЛТЫЙ
        case 14:  return english_language ? "GOLD"       : "PJKJNJQ";        break;  // ЗОЛОТОЙ
        case 15:  return english_language ? "DARK GOLD"  : "NTVYJ-PJKJNJQ";  break;  // ТЕМНО-ЗОЛОТОЙ
        case 16:  return english_language ? "TAN"        : ",T;TDSQ";        break;  // БЕЖЕВЫЙ
        case 17:  return english_language ? "BROWN"      : "RJHBXYTDSQ";     break;  // КОРИЧНЕВЫЙ
        default:  return english_language ? "WHITE"      : ",TKSQ";          break;  // БЕЛЫЙ
    }
}


// [JN] Used as a timer for hiding menu background
// while changing screen size, gamma and level brightness.
static int menubgwait;

// [JN] Used as a flag for drawing Sound / Sound System menu background:
// - if menu was invoked by F4, don't draw background.
// - if menu was invoked from Options menu, draw background.
static boolean sfxbgdraw;

// [JN] Set default mouse sensitivity to 5, like in Doom
int mouseSensitivity = 5;

//[Dasperal] Predeclare menu variables to allow referencing them before they initialized
static Menu_t* EpisodeMenu;
static Menu_t* OptionsMenu;
static Menu_t RegisteredEpisodeMenu;
static Menu_t RDOptionsMenu;
static Menu_t RenderingMenu;
static Menu_t DisplayMenu;
static Menu_t MessagesMenu;
static Menu_t AutomapMenu;
static Menu_t SoundMenu;
static Menu_t SoundSysMenu;
static Menu_t ControlsMenu;
static Menu_t Gameplay1Menu;
static Menu_t Gameplay2Menu;
static Menu_t Gameplay3Menu;
static const Menu_t* GameplayMenuPages[] = {&Gameplay1Menu, &Gameplay2Menu, &Gameplay3Menu};
static Menu_t LevelSelectMenu1;
static Menu_t LevelSelectMenu2;
static Menu_t LevelSelectMenu3;
static const Menu_t* LevelSelectMenuPages[] = {&LevelSelectMenu1, &LevelSelectMenu2, &LevelSelectMenu3};
static Menu_t VanillaOptions2Menu;
static Menu_t FilesMenu;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

static MenuItem_t MainItems[] = {
    {ITT_SETMENU_NONET, "NEW GAME",   "YJDFZ BUHF", &RegisteredEpisodeMenu, 1}, // НОВАЯ ИГРА
    {ITT_SETMENU,       "OPTIONS",    "YFCNHJQRB",  &RDOptionsMenu,         0}, // НАСТРОЙКИ
    {ITT_SETMENU,       "GAME FILES", "AFQKS BUHS", &FilesMenu,             0}, // ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,         "INFO",       "BYAJHVFWBZ", SCInfo,                 0}, // ИНФОРМАЦИЯ
    {ITT_EFUNC,         "QUIT GAME",  "DS[JL",      SCQuitGame,             0}  // ВЫХОД
};

static Menu_t MainMenu = {
    110, 103,
    56,
    NULL, NULL, true,
    5, MainItems, true,
    DrawMainMenu,
    NULL,
    NULL,
    0
};

static MenuItem_t EpisodeItems[] = {
    {ITT_EFUNC, "CITY OF THE DAMNED",   "UJHJL GHJRKZNS[",    SCEpisode, 1}, // ГОРОД ПРОКЛЯТЫХ
    {ITT_EFUNC, "HELL'S MAW",           "FLCRFZ ENHJ,F",      SCEpisode, 2}, // АДСКАЯ УТРОБА
    {ITT_EFUNC, "THE DOME OF D'SPARIL", "REGJK L\"CGFHBKF",   SCEpisode, 3}, // КУПОЛ Д'СПАРИЛА
    {ITT_EFUNC, "THE OSSUARY",          "CRKTG",              SCEpisode, 4}, // СКЛЕП
    {ITT_EFUNC, "THE STAGNANT DEMESNE", "PFCNJQYST DKFLTYBZ", SCEpisode, 5}  // ЗАСТОЙНЫЕ ВЛАДЕНИЯ
};

static Menu_t RegisteredEpisodeMenu = {
    80, 55,
    50,
    NULL, NULL, true,
    3, EpisodeItems, true,
    NULL,
    NULL,
    &MainMenu,
    0
};

static Menu_t RetailEpisodeMenu = {
    80, 55,
    30,
    NULL, NULL, true,
    5, EpisodeItems, true,
    NULL,
    NULL,
    &MainMenu,
    0
};

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, "THOU NEEDETH A WET-NURSE",       "YZYTXRF YFLJ,YF VYT",    SCSkill, sk_baby},      // НЯНЕЧКА НАДОБНА МНЕ
    {ITT_EFUNC, "YELLOWBELLIES-R-US",             "YT CNJKM VE;TCNDTYTY Z", SCSkill, sk_easy},      // НЕ СТОЛЬ МУЖЕСТВЕНЕН Я
    {ITT_EFUNC, "BRINGEST THEM ONETH",            "GJLFQNT VYT B[",         SCSkill, sk_medium},    // ПОДАЙТЕ МНЕ ИХ
    {ITT_EFUNC, "THOU ART A SMITE-MEISTER",       "BCREITY Z CHF;TYBZVB",   SCSkill, sk_hard},      // ИСКУШЕН Я СРАЖЕНИЯМИ
    {ITT_EFUNC, "BLACK PLAGUE POSSESSES THEE",    "XEVF JDKFLTKF VYJQ",     SCSkill, sk_nightmare}, // ЧУМА ОВЛАДЕЛА МНОЙ
    {ITT_EFUNC, "QUICKETH ART THEE, FOUL WRAITH", "RJIVFHJV BCGJKYTY Z",    SCSkill, sk_ultranm}    // КОШМАРОМ ИСПОЛНЕН Я // [JN] Thanks to Jon Dowland for this :)
};

static Menu_t SkillMenu = {
    38, 38,
     30,
    NULL, NULL, true,
    6, SkillItems, true,
    NULL,
    NULL,
    &RegisteredEpisodeMenu,
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
    {ITT_SETMENU, "GAMEPLAY",          "UTQVGKTQ",       &Gameplay1Menu,      0}, // ГЕЙМПЛЕЙ
    {ITT_SETMENU, "LEVEL SELECT",      "DS,JH EHJDYZ",   &LevelSelectMenu1,   0}, // ВЫБОР УРОВНЯ
    {ITT_EFUNC,   "RESET SETTINGS",    "C,HJC YFCNHJTR", M_RD_ResetSettings,  0}, // СБРОС НАСТРОЕК
    {ITT_LRFUNC,  "LANGUAGE: ENGLISH", "ZPSR: HECCRBQ",  M_RD_ChangeLanguage, 0}  // ЯЗЫК: РУССКИЙ
};

static Menu_t RDOptionsMenu = {
    81, 81,
    31,
    "OPTIONS", "YFCNHJQRB", false, // НАСТРОЙКИ
    8, RDOptionsItems, true,
    DrawOptionsMenu,
    NULL,
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
    {ITT_LRFUNC, "SCREENSHOT FORMAT:",        "AJHVFN CRHBYIJNJD:",              M_RD_Screenshots,       0}, // ФОРМАТ СКРИНШОТОВ
    {ITT_LRFUNC, "SHOW ENDTEXT SCREEN:",      "GJRFPSDFNM \'RHFY",               M_RD_EndText,           0}  // ПОКАЗЫВАТЬ ЭКРАН ENDTEXT
};

static Menu_t RenderingMenu = {
    36, 36,
    32,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    11, RenderingItems, false,
    DrawRenderingMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_TITLE,   "SCREEN",                    "\'RHFY",                  NULL,                 0}, // ЭКРАН
    {ITT_LRFUNC,  "SCREEN SIZE",               "HFPVTH BUHJDJUJ \'RHFYF", M_RD_ScreenSize,      0}, // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,   NULL,                        NULL,                      NULL,                 0},
    {ITT_LRFUNC,  "GAMMA-CORRECTION",          "EHJDTYM UFVVF-RJHHTRWBB", M_RD_Gamma,           0}, // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
    {ITT_EMPTY,   NULL,                        NULL,                      NULL,                 0},
    {ITT_LRFUNC,  "LEVEL BRIGHTNESS",          "EHJDTYM JCDTOTYYJCNB",    M_RD_LevelBrightness, 0}, // УРОВЕНЬ ОСВЕЩЕННОСТИ
    {ITT_EMPTY,   NULL,                        NULL,                      NULL,                 0},
    {ITT_LRFUNC,  "GRAPHICS DETAIL:",          "LTNFKBPFWBZ UHFABRB:",    M_RD_Detail,          0}, // ДЕТАЛИЗАЦИЯ ГРАФИКИ
    {ITT_TITLE,   "INTERFACE",                 "BYNTHATQC",               NULL,                 0}, // ИНТЕРФЕЙС
    {ITT_SETMENU, "MESSAGES AND TEXTS...",     "CJJ,OTYBZ B NTRCNS>>>",   &MessagesMenu,        0}, // СООБЩЕНИЯ И ТЕКСТЫ...
    {ITT_SETMENU, "AUTOMAP AND STATISTICS...", "RFHNF B CNFNBCNBRF>>>",   &AutomapMenu,         0}  // КАРТА И СТАТИСТИКА...
};

static Menu_t DisplayMenu = {
    36, 36,
    32,
    "DISPLAY OPTIONS", "YFCNHJQRB \'RHFYF", false, // НАСТРОЙКИ ЭКРАНА
    11, DisplayItems, false,
    DrawDisplayMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

static MenuItem_t MessagesItems[] = {
    {ITT_TITLE,  "GENERAL",             "JCYJDYJT",                 NULL,                         0}, // ОСНОВНОЕ
    {ITT_LRFUNC, "MESSAGES:",           "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,                0}, // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_LRFUNC, "ALIGNMENT:",          "DSHFDYBDFYBT:",            M_RD_MessagesAlignment,       0}, // ВЫРАВНИВАНИЕ
    {ITT_LRFUNC, "MESSAGE TIMEOUT",     "NFQVFEN JNJ,HF;TYBZ",      M_RD_MessagesTimeout,         0}, // ТАЙМАУТ ОТОБРАЖЕНИЯ
    {ITT_EMPTY,   NULL,                  NULL,                      NULL,                         0},
    {ITT_LRFUNC, "FADING EFFECT:",      "GKFDYJT BCXTPYJDTYBT:",    M_RD_MessagesFade,            0}, // ПЛАВНОЕ ИСЧЕЗНОВЕНИЕ
    {ITT_LRFUNC, "TEXT CASTS SHADOWS:", "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText,            0}, // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_TITLE,  "MISC",                "HFPYJT",                   NULL,                         0}, // РАЗНОЕ
    {ITT_LRFUNC, "LOCAL TIME:",         "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,               0}, // СИСТЕМНОЕ ВРЕМЯ
    {ITT_TITLE,  "COLORS",              "WDTNF",                    NULL,                         0}, // ЦВЕТА
    {ITT_LRFUNC, "ITEM PICKUP:",        "GJKEXTYBT GHTLVTNJD:",     M_RD_Change_Msg_Pickup_Color, 0}, // ПОЛУЧЕНИЕ ПРЕДМЕТОВ
    {ITT_LRFUNC, "REVEALED SECRET:",    "J,YFHE;TYBT NFQYBRJD:",    M_RD_Change_Msg_Secret_Color, 0}, // ОБНАРУЖЕНИЕ ТАЙНИКОВ
    {ITT_LRFUNC, "SYSTEM MESSAGE:",     "CBCNTVYST CJJ,OTYBZ:",     M_RD_Change_Msg_System_Color, 0}, // СИСТЕМНЫЕ СООБЩЕНИЯ
    {ITT_LRFUNC, "NETGAME CHAT:",       "XFN CTNTDJQ BUHS:",        M_RD_Change_Msg_Chat_Color,   0}  // ЧАТ СЕТЕВОЙ ИГРЫ
};

static Menu_t MessagesMenu = {
    36, 36,
    32,
    "MESSAGES AND TEXTS", "CJJ,OTYBZ B NTRCNS", false, // СООБЩЕНИЯ И ТЕКСТЫ
    14, MessagesItems, false,
    DrawMessagesMenu,
    NULL,
    &DisplayMenu,
    1
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_TITLE,  "AUTOMAP",        "RFHNF",              NULL,                 0}, // КАРТА
    {ITT_LRFUNC, "OVERLAY MODE:",  "HT;BV YFKJ;TYBZ:",   M_RD_AutoMapOverlay,  0}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_LRFUNC, "ROTATE MODE:",   "HT;BV DHFOTYBZ:",    M_RD_AutoMapRotate,   0}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_LRFUNC, "FOLLOW MODE:",   "HT;BV CKTLJDFYBZ:",  M_RD_AutoMapFollow,   0}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_LRFUNC, "GRID:",          "CTNRF:",             M_RD_AutoMapGrid,     0}, // СЕТКА
    {ITT_LRFUNC, "GRID SIZE:",     "HFPVTH CTNRB:",      M_RD_AutoMapGridSize, 0}, // РАЗМЕР СЕТКИ
    {ITT_TITLE,  "STATISTICS",     "CNFNBCNBRF",         NULL,                 0}, // СТАТИСТИКА
    {ITT_LRFUNC, "LEVEL STATS:",   "CNFNBCNBRF EHJDYZ:", M_RD_AutoMapStats,    0}, // СТАТИСТИКА УРОВНЯ
    {ITT_LRFUNC, "LEVEL TIME:",    "DHTVZ EHJDYZ:",      M_RD_AutoMapLevTime,  0}, // ВРЕМЯ УРОВНЯ
    {ITT_LRFUNC, "TOTAL TIME:",    "J,OTT EHJDYZ:",      M_RD_AutoMapTotTime,  0}, // ОБЩЕЕ ВРЕМЯ
    {ITT_LRFUNC, "PLAYER COORDS:", "RJJHLBYFNS BUHJRF:", M_RD_AutoMapCoords,   0}  // КООРДИНАТЫ ИГРОКА
};

static Menu_t AutomapMenu = {
    78, 61,
    32,
    "AUTOMAP AND STATISTICS", "RFHNF B CNFNBCNBRF", false, // КАРТА И СТАТИСТИКА
    11, AutomapItems, false,
    DrawAutomapMenu,
    NULL,
    &DisplayMenu,
    1
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_TITLE,   "VOLUME",                   "UHJVRJCNM",                     NULL,             0}, // ГРОМКОСТЬ
    {ITT_LRFUNC,  "SFX VOLUME",               "UHJVRJCNM PDERF",               M_RD_SfxVolume,   0}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,   NULL,                       NULL,                            NULL,             0},
    {ITT_LRFUNC,  "MUSIC VOLUME",             "UHJVRJCNM VEPSRB",              M_RD_MusVolume,   0}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,   NULL,                       NULL,                            NULL,             0},
    {ITT_TITLE,   "CHANNELS",                 "DJCGHJBPDTLTYBT",               NULL,             0}, // ВОСПРОИЗВЕДЕНИЕ
    {ITT_LRFUNC,  "SFX CHANNELS",             "PDERJDST RFYFKS",               M_RD_SfxChannels, 0}, // ЗВУКОВЫЕ КАНАЛЫ
    {ITT_EMPTY,   NULL,                       NULL,                            NULL,             0},
    {ITT_TITLE,   "ADVANCED",                 "LJGJKYBNTKMYJ",                 NULL,             0}, // ДОПОЛНИТЕЛЬНО
    {ITT_SETMENU, "SOUND SYSTEM SETTINGS...", "YFCNHJQRB PDERJDJQ CBCNTVS>>>", &SoundSysMenu,    0}  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
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
    NULL,
    &SoundMenu,
    1
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_TITLE,  "MOVEMENT",               "GTHTLDB;TYBT",              NULL,              0}, // ПЕРЕДВИЖЕНИЕ
    {ITT_LRFUNC, "ALWAYS RUN:",            "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,    0}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_TITLE,  "MOUSE",                  "VSIM",                      NULL,              0}, // МЫШЬ
    {ITT_LRFUNC, "MOUSE SENSIVITY",        "CRJHJCNM VSIB",             M_RD_Sensitivity,  0}, // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,  NULL,                     NULL,                        NULL,              0},
    {ITT_LRFUNC, "ACCELERATION",           "FRCTKTHFWBZ",               M_RD_Acceleration, 0}, // АКСЕЛЕРАЦИЯ
    {ITT_EMPTY,  NULL,                     NULL,                        NULL,              0},
    {ITT_LRFUNC, "ACCELERATION THRESHOLD", "GJHJU FRCTKTHFWBB",         M_RD_Threshold,    0}, // ПОРОГ АКСЕЛЕРАЦИИ
    {ITT_EMPTY,  NULL,                     NULL,                        NULL,              0},
    {ITT_LRFUNC, "MOUSE LOOK:",            "J,PJH VSIM.:",              M_RD_MouseLook,    0}, // ОБЗОР МЫШЬЮ
    {ITT_LRFUNC, "INVERT Y AXIS:",         "DTHNBRFKMYFZ BYDTHCBZ:",    M_RD_InvertY,      0}, // ВЕРТИКАЛЬНАЯ ИНВЕРСИЯ
    {ITT_LRFUNC, "VERTICAL MOVEMENT:",     "DTHNBRFKMYJT GTHTVTOTYBT:", M_RD_Novert,       0}  // ВЕРТИКАЛЬНОЕ ПЕРЕМЕЩЕНИЕ
};

static Menu_t ControlsMenu = {
    36, 36,
    32,
    "CONTROL SETTINGS", "EGHFDKTYBT", false, // УПРАВЛЕНИЕ
    12, ControlsItems, false,
    DrawControlsMenu,
    NULL,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t GameplayPageDescriptor = {
    3, GameplayMenuPages,
    254, 166,
    CR_WHITE2GRAY_HERETIC
};

static MenuItem_t Gameplay1Items[] = {
    {ITT_TITLE,   "VISUAL",                       "UHFABRF",                        NULL,                 0}, // ГРАФИКА
    {ITT_LRFUNC,  "BRIGHTMAPS:",                  ",HFQNVFGGBYU:",                  M_RD_Brightmaps,      0}, // БРАЙТМАППИНГ
    {ITT_LRFUNC,  "FAKE CONTRAST:",               "BVBNFWBZ RJYNHFCNYJCNB:",        M_RD_FakeContrast,    0}, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_LRFUNC,  "EXTRA TRANSLUCENCY:",          "LJGJKYBNTKMYFZ GHJPHFXYJCNM:",   M_RD_ExtraTrans,      0}, // ДОПОЛНИТЕЛЬНАЯ ПРОЗРАЧНОСТЬ
    {ITT_LRFUNC,  "COLORED BLOOD:",               "HFPYJWDTNYFZ RHJDM:",            M_RD_ColoredBlood,    0}, // РАЗНОЦВЕТНАЯ КРОВЬ
    {ITT_LRFUNC,  "SWIRLING LIQUIDS:",            "EKEXITYYFZ FYBVFWBZ ;BLRJCNTQ:", M_RD_SwirlingLiquids, 0}, // УЛУЧШЕННАЯ АНИМАЦИЯ ЖИДКОСТЕЙ
    {ITT_LRFUNC,  "INVULNERABILITY AFFECTS SKY:", "YTEZPDBVJCNM JRHFIBDFTN YT,J:",  M_RD_InvulSky,        0}, // НЕУЯЗВИМОСТЬ ОКРАШИВАЕТ НЕБО
    {ITT_LRFUNC,  "SKY DRAWING MODE:",            "HT;BV JNHBCJDRB YT,F:",          M_RD_LinearSky,       0}, // РЕЖИМ ОТРИСОВКИ НЕБА
    {ITT_TITLE,   "PHYSICAL",                     "ABPBRF",                         NULL,                 0}, // ФИЗИКА
    {ITT_LRFUNC,  "CORPSES SLIDING FROM LEDGES:", "NHEGS CGJKPF.N C DJPDSITYBQ:",   M_RD_Torque,          0}, // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_LRFUNC,  "WEAPON BOBBING WHILE FIRING:", "EKEXITYYJT GJRFXBDFYBT JHE;BZ:", M_RD_Bobbing,         0}, // УЛУЧШЕННОЕ ПОКАЧИВАНИЕ ОРУЖИЯ
    {ITT_LRFUNC,  "RANDOMLY MIRRORED CORPSES:",   "PTHRFKBHJDFYBT NHEGJD:",         M_RD_FlipCorpses,     0}, // ЗЕКРАЛИРОВАНИЕ ТРУПОВ
    {ITT_LRFUNC,  "FLOATING ITEMS AMPLITUDE:" ,   "KTDBNFWBZ GHTLVTNJD:",           M_RD_FloatAmplitude,  0}, // АМПЛИТУДА ЛЕВИТАЦИИ ПРЕДМЕТОВ
    {ITT_EMPTY,   NULL,                           NULL,                             NULL,                 0},
    {ITT_SETMENU, "NEXT PAGE...",                 "CKTLE.OFZ CNHFYBWF>>>",          &Gameplay2Menu,       0}  // СЛЕДУЮЩАЯ СТРАНИЦА
};

static Menu_t Gameplay1Menu = {
    36, 36,
    26,
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
    {ITT_TITLE,   "STATUS BAR",            "CNFNEC-,FH",                  NULL,                    0}, // СТАТУС-БАР
    {ITT_LRFUNC,  "COLORED STATUS BAR:",   "HFPYJWDTNYST \'KTVTYNS:",     M_RD_ColoredSBar,        0}, // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ
    {ITT_LRFUNC,  "COLORED HEALTH GEM:",   "JRHFIBDFYBT RFVYZ PLJHJDMZ:", M_RD_ColoredGem,         0}, // ОКРАШИВАНИЕ КАМНЯ ЗДОРОВЬЯ
    {ITT_LRFUNC,  "SHOW NEGATIVE HEALTH:", "JNHBWFNTKMYJT PLJHJDMT:",     M_RD_NegativeHealth,     0}, // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ
    {ITT_TITLE,   "AMMO WIDGET",           "DBL;TN ,JTPFGFCF",            NULL,                    0}, // ВИДЖЕТ БОЕЗАПАСА
    {ITT_LRFUNC,  "DRAW WIDGET:",          "JNJ,HF;FNM DBL;TN:",          M_RD_AmmoWidgetDraw,     0}, // ОТОБРАЖАТЬ ВИДЖЕТ
    {ITT_LRFUNC,  "COLORING:",             "WDTNJDFZ BYLBRFWBZ:",         M_RD_AmmoWidgetColoring, 0}, // ЦВЕТОВАЯ ИНДИКАЦИЯ
    {ITT_TITLE,   "CROSSHAIR",             "GHBWTK",                      NULL,                    0}, // ПРИЦЕЛ
    {ITT_LRFUNC,  "DRAW CROSSHAIR:",       "JNJ,HF;FNM GHBWTK:",          M_RD_CrossHairDraw,      0}, // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_LRFUNC,  "INDICATION:",           "BYLBRFWBZ:",                  M_RD_CrossHairType,      0}, // ИНДИКАЦИЯ
    {ITT_LRFUNC,  "INCREASED SIZE:",       "EDTKBXTYYSQ HFPVTH:",         M_RD_CrossHairScale,     0}, // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,   NULL,                    NULL,                          NULL,                    0},
    {ITT_EMPTY,   NULL,                    NULL,                          NULL,                    0},
    {ITT_EMPTY,   NULL,                    NULL,                          NULL,                    0},
    {ITT_SETMENU, "LAST PAGE...",          "GJCKTLYZZ CNHFYBWF>>>",       &Gameplay3Menu,          0}  // ПОСЛЕДНЯЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    36, 36,
    26,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    15, Gameplay2Items, false,
    DrawGameplay2Menu,
    &GameplayPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gameplay features (3)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay3Items[] = {
    {ITT_TITLE,   "AUDIBLE",                         "PDER",                            NULL,                0}, // ЗВУК
    {ITT_LRFUNC,  "SOUND ATTENUATION AXISES:",       "PFNE[FYBT PDERF GJ JCZV:",        M_RD_ZAxisSFX,       0}, // ЗАТУХАНИЕ ЗВУКА ПО ОСЯМ
    {ITT_LRFUNC,  "MONSTER ALERT WAKING UP OTHERS:", "J,OFZ NHTDJUF E VJYCNHJD:",       M_RD_AlertSFX,       0}, // ОБЩАЯ ТРЕВОГА У МОНСТРОВ
    {ITT_TITLE,   "TACTICAL",                        "NFRNBRF",                         NULL,                0}, // ТАКТИКА
    {ITT_LRFUNC,  "NOTIFY OF REVEALED SECRETS:",     "CJJ,OFNM J YFQLTYYJV NFQYBRT:",   M_RD_SecretNotify,   0}, // СООБЩАТЬ О НАЙДЕННОМ ТАЙНИКЕ
    {ITT_LRFUNC,  "SHOW ACTIVE ARTIFACTS:",          "BYLBRFWBZ FHNTAFRNJD:",           M_RD_ShowAllArti,    0}, // ИНДИКАЦИЯ АРТЕФАЕКТОВ
    {ITT_LRFUNC,  "ARTIFACTS TIMER:",                "NFQVTH FHNTAFRNJD:",              M_RD_ShowArtiTimer,  0}, // ТАЙМЕР АРТЕФАКТОВ
    {ITT_TITLE,   "GAMEPLAY",                        "UTQVGKTQ",                        NULL,                0}, // ГЕЙМПЛЕЙ
    {ITT_LRFUNC,  "FIX ERRORS ON VANILLA MAPS:",     "ECNHFYZNM JIB,RB JHBU> EHJDYTQ:", M_RD_FixMapErrors,   0}, // УСТРАНЯТЬ ОШИБКИ ОРИГИНАЛЬНЫХ УРОВНЕЙ
    {ITT_LRFUNC,  "FLIP GAME LEVELS:",               "PTHRFKMYJT JNHF;TYBT EHJDYTQ:",   M_RD_FlipLevels,     0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_LRFUNC,  "PLAY INTERNAL DEMOS:",            "GHJBUHSDFNM LTVJPFGBCB:",         M_RD_NoDemos,        0}, // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_LRFUNC,  "WAND START GAME MODE:",           NULL, /* [JN] Joint EN/RU string*/ M_RD_WandStart,      0}, // РЕЖИМ ИГРЫ "WAND START"
    {ITT_EMPTY,   NULL,                              NULL,                              NULL,                0},
    {ITT_EMPTY,   NULL,                              NULL,                              NULL,                0},
    {ITT_SETMENU, "FIRST PAGE...",                   "GTHDFZ CNHFYBWF>>>",              &Gameplay1Menu,      0}  // ПЕРВАЯ СТРАНИЦА
};

static Menu_t Gameplay3Menu = {
    36, 36,
    26,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    15, Gameplay3Items, false,
    DrawGameplay3Menu,
    &GameplayPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Level select (1)
// -----------------------------------------------------------------------------

static const PageDescriptor_t LevelSelectPageDescriptor = {
    3, LevelSelectMenuPages,
    254, 176,
    CR_WHITE2GRAY_HERETIC
};

static MenuItem_t Level1Items[] = {
    {ITT_LRFUNC,  "SKILL LEVEL:",       "CKJ;YJCNM:",            M_RD_SelectiveSkill,      0}, // СЛОЖНОСТЬ
    {ITT_LRFUNC,  "EPISODE:",           "\'GBPJL:",              M_RD_SelectiveEpisode,    0}, // ЭПИЗОД
    {ITT_LRFUNC,  "MAP:",               "EHJDTYM:",              M_RD_SelectiveMap,        0}, // УРОВЕНЬ
    {ITT_TITLE,   "PLAYER",             "BUHJR",                 NULL,                     0}, // ИГРОК
    {ITT_LRFUNC,  "HEALTH:",            "PLJHJDMT:",             M_RD_SelectiveHealth,     0}, // ЗДОРОВЬЕ
    {ITT_LRFUNC,  "ARMOR:",             ",HJYZ:",                M_RD_SelectiveArmor,      0}, // БРОНЯ
    {ITT_LRFUNC,  "ARMOR TYPE:",        "NBG ,HJYB:",            M_RD_SelectiveArmorType,  0}, // ТИП БРОНИ
    {ITT_TITLE,   "WEAPONS",            "JHE;BT",                NULL,                     0}, // ОРУЖИЕ
    {ITT_LRFUNC,  "GAUNTLETS:",         "GTHXFNRB:",             M_RD_SelectiveGauntlets,  0}, // ПЕРЧАТКИ
    {ITT_LRFUNC,  "ETHEREAL CROSSBOW:", "\'ABHYSQ FH,FKTN:",     M_RD_SelectiveCrossbow,   0}, // ЭФИРНЫЙ АРБАЛЕТ
    {ITT_LRFUNC,  "DRAGON CLAW:",       "RJUJNM LHFRJYF:",       M_RD_SelectiveDragonClaw, 0}, // КОГОТЬ ДРАКОНА
    {ITT_LRFUNC,  "HELLSTAFF:",         "GJCJ] FLF:",            M_RD_SelectiveHellStaff,  0}, // ПОСОХ АДА
    {ITT_LRFUNC,  "PHOENIX ROD:",       ";TPK ATYBRCF:",         M_RD_SelectivePhoenixRod, 0}, // ЖЕЗЛ ФЕНИКСА
    {ITT_LRFUNC,  "FIREMACE:",          "JUYTYYFZ ,EKFDF:",      M_RD_SelectiveFireMace,   0}, // ОГНЕННАЯ БУЛАВА
    {ITT_EMPTY,   NULL,                 NULL,                    NULL,                     0},
    {ITT_SETMENU, "NEXT PAGE...",       "CKTLE.OFZ CNHFYBWF>>>", &LevelSelectMenu2,        0}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC,  "START GAME",         "YFXFNM BUHE",           G_DoSelectiveGame,        0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu1 = {
    74, 74,
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

static MenuItem_t Level2Items[] = {
    {ITT_LRFUNC,  "BAG OF HOLDING:",  "YJCBKMYSQ RJITKM:",     M_RD_SelectiveBag,     0},
    {ITT_LRFUNC,  "WAND CRYSTALS:",   "RHBCNFKKS LKZ ;TPKF:",  M_RD_SelectiveAmmo_0,  0},
    {ITT_LRFUNC,  "ETHEREAL ARROWS:", "\'ABHYST CNHTKS:",      M_RD_SelectiveAmmo_1,  0},
    {ITT_LRFUNC,  "CLAW ORBS:",       "RJUNTDST IFHS:",        M_RD_SelectiveAmmo_2,  0},
    {ITT_LRFUNC,  "HELLSTAFF RUNES:", "HEYS GJCJ[F:",          M_RD_SelectiveAmmo_3,  0},
    {ITT_LRFUNC,  "FLAME ORBS:",      "GKFVTYYST IFHS:",       M_RD_SelectiveAmmo_4,  0},
    {ITT_LRFUNC,  "MACE SPHERES:",    "CATHS ,EKFDS:",         M_RD_SelectiveAmmo_5,  0},
    {ITT_TITLE,   "KEYS",             "RK.XB",                 NULL,                  0}, // КЛЮЧИ
    {ITT_LRFUNC,  "YELLOW KEY:",      ";TKNSQ RK.X:",          M_RD_SelectiveKey_0,   0},
    {ITT_LRFUNC,  "GREEN KEY:",       "PTKTYSQ RK.X:",         M_RD_SelectiveKey_1,   0},
    {ITT_LRFUNC,  "BLUE KEY:",        "CBYBQ RK.X:",           M_RD_SelectiveKey_2,   0},
    {ITT_TITLE,   "MONSTERS",         "VJYCNHS",               NULL,                  0}, // МОНСТРЫ
    {ITT_LRFUNC,  "FAST:",            "ECRJHTYYST:",           M_RD_SelectiveFast,    0},
    {ITT_LRFUNC,  "RESPAWNING:",      "DJCRHTIF.OBTCZ:",       M_RD_SelectiveRespawn, 0},
    {ITT_EMPTY,   NULL,               NULL,                    NULL,                  0},
    {ITT_SETMENU, "LAST PAGE...",     "GJCKTLYZZ CNHFYBWF>>>", &LevelSelectMenu3,     0},
    {ITT_LRFUNC,  "START GAME",       "YFXFNM BUHE",           G_DoSelectiveGame,     0}
};

static Menu_t LevelSelectMenu2 = {
    74, 74,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level2Items, false,
    DrawLevelSelect2Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (3)
// -----------------------------------------------------------------------------

static MenuItem_t Level3Items[] = {
    {ITT_TITLE,  "ARTIFACTS",              "FHNTAFRNS",            NULL,                 0}, // АРТЕФАКТЫ
    {ITT_LRFUNC, "QUARTZ FLASK:",          "RDFHWTDSQ AKFRJY:",    M_RD_SelectiveArti_0, 0},
    {ITT_LRFUNC, "MYSTIC URN:",            "VBCNBXTCRFZ EHYF:",    M_RD_SelectiveArti_1, 0},
    {ITT_LRFUNC, "TIMEBOMB:",              "XFCJDFZ ,JV,F:",       M_RD_SelectiveArti_2, 0},
    {ITT_LRFUNC, "TOME OF POWER:",         "NJV VJUEOTCNDF:",      M_RD_SelectiveArti_3, 0},
    {ITT_LRFUNC, "RING OF INVINCIBILITY:", "RJKMWJ YTEZPDBVJCNB:", M_RD_SelectiveArti_4, 0},
    {ITT_LRFUNC, "MORPH OVUM:",            "ZQWJ GHTDHFOTYBQ:",    M_RD_SelectiveArti_5, 0},
    {ITT_LRFUNC, "CHAOS DEVICE:",          "\'V,KTVF [FJCF:",      M_RD_SelectiveArti_6, 0},
    {ITT_LRFUNC, "SHADOWSPHERE:",          "NTYTDFZ CATHF:",       M_RD_SelectiveArti_7, 0},
    {ITT_LRFUNC, "WINGS OF WRATH:",        "RHSKMZ UYTDF:",        M_RD_SelectiveArti_8, 0},
    {ITT_LRFUNC, "TORCH:",                 "AFRTK:",               M_RD_SelectiveArti_9, 0},
    {ITT_EMPTY,  NULL,                     NULL,                   NULL,                 0},
    {ITT_EMPTY,  NULL,                     NULL,                   NULL,                 0},
    {ITT_EMPTY,  NULL,                     NULL,                   NULL,                 0},
    {ITT_EMPTY,  NULL,                     NULL,                   NULL,                 0},
    {ITT_SETMENU,"FIRST PAGE...",          "GTHDFZ CNHFYBWF>>>",   &LevelSelectMenu1,    0},
    {ITT_LRFUNC, "START GAME",             "YFXFNM BUHE",          G_DoSelectiveGame,    0}
};

static Menu_t LevelSelectMenu3 = {
    74, 74,
    26,
    "LEVEL SELECT", "DS,JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    17, Level3Items, false,
    DrawLevelSelect3Menu,
    &LevelSelectPageDescriptor,
    &RDOptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Vanilla options menu
// -----------------------------------------------------------------------------

static MenuItem_t VanillaOptionsItems[] = {
    {ITT_EFUNC,   "END GAME",          "PFRJYXBNM BUHE",   M_RD_EndGame,         0},
    {ITT_EFUNC,   "MESSAGES : ",       "CJJ,OTYBZ : ",     SCMessages,           0},
    {ITT_LRFUNC,  "MOUSE SENSITIVITY", "CRJHJCNM VSIB",    M_RD_Sensitivity,     0},
    {ITT_EMPTY,   NULL,                NULL,               NULL,                 0},
    {ITT_SETMENU, "MORE...",           "LJGJKYBNTKMYJ>>>", &VanillaOptions2Menu, 0}
};

static Menu_t VanillaOptionsMenu = {
    88, 88,
    30,
    NULL, NULL, true,
    5, VanillaOptionsItems, true,
    DrawOptionsMenu_Vanilla,
    NULL,
    &MainMenu,
    0,
};

// -----------------------------------------------------------------------------
// Vanilla options menu (more...)
// -----------------------------------------------------------------------------

static MenuItem_t VanillaOptions2Items[] = {
    {ITT_LRFUNC, "SCREEN SIZE",  "HFPVTH 'RHFYF",    M_RD_ScreenSize, 0},
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "SFX VOLUME",   "UHJVRJCNM PDERF",  M_RD_SfxVolume,  0},
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "MUSIC VOLUME", "UHJVRJCNM VEPSRB", M_RD_MusVolume,  0},
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
    110, 90,
    60,
    NULL, NULL, true,
    2, FilesItems, true,
    DrawFilesMenu,
    NULL,
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

// -----------------------------------------------------------------------------

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
    RD_M_InitFonts(// [JN] Original English fonts
                   DEH_String("FONTA_S"),
                   DEH_String("FONTB_S"),
                   // [JN] Small special font used for time/fps widget
                   DEH_String("FONTC_S"),
                   // [JN] Unchangable English fonts
                   DEH_String("FONTD_S"),
                   DEH_String("FONTE_S"),
                   // [JN] Unchangable Russian fonts
                   DEH_String("FONTF_S"),
                   DEH_String("FONTG_S"));

    RD_Menu_InitMenu(20, 10);

    RD_Menu_InitSliders(// [Dasperal] Big slider
                        DEH_String("M_SLDLT"),
                        DEH_String("M_SLDMD1"),
                        DEH_String("M_SLDMD2"),
                        DEH_String("M_SLDRT"),
                        DEH_String("M_SLDKB"),
                        // [Dasperal] Small slider
                        DEH_String("M_RDSLDL"),
                        DEH_String("M_RDSLD1"),
                        DEH_String("M_RDSLDR"),
                        DEH_String("M_RDSLG"),
                        // [Dasperal] Gem translations
                        CR_NONE,
                        CR_GREEN2GRAY_HERETIC,
                        CR_GREEN2RED_HERETIC);

    RD_Menu_InitCursor(// [Dasperal] Big cursor
                       DEH_String("M_SLCTR1"),
                       DEH_String("M_SLCTR2"),
                       // [Dasperal] Small cursor
                       DEH_String("M_RDGEM1"),
                       DEH_String("M_RDGEM2"),
                       -1, -1, -28, -14);

    menuactive = false;
    SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;

    if (gamemode == retail)
        EpisodeMenu = &RetailEpisodeMenu;
    else
        EpisodeMenu = &RegisteredEpisodeMenu;

    if(vanillaparm)
        OptionsMenu = &VanillaOptionsMenu;
    else
        OptionsMenu = &RDOptionsMenu;

    MainItems[0].pointer = EpisodeMenu;
    MainItems[1].pointer = OptionsMenu;
    SkillMenu.prevMenu = EpisodeMenu;

    // [JN] Init message colors.
    M_RD_Define_Msg_Color(msg_pickup, message_pickup_color);
    M_RD_Define_Msg_Color(msg_secret, message_secret_color);
    M_RD_Define_Msg_Color(msg_system, message_system_color);
    M_RD_Define_Msg_Color(msg_chat, message_chat_color);
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
    char *message;

    if (menuactive == false)
    {
        if (askforquit)
        {
            message = DEH_String(english_language ?
                                 QuitEndMsg[typeofask - 1] :
                                 QuitEndMsg_Rus[typeofask - 1]);

            if (english_language)
            {
                RD_M_DrawTextA(message, 160 - RD_M_TextAWidth(message) / 2
                                        + wide_delta, 80);
            }
            else
            {
                RD_M_DrawTextSmallRUS(message, 160 - RD_M_TextSmallRUSWidth(message) / 2
                                               + wide_delta, 80, CR_NONE);
            }

            if (typeofask == 3)
            {
                RD_M_DrawTextA(SlotText[quicksave - 1], 160 -
                                                        RD_M_TextAWidth(SlotText[quicksave - 1]) / 2 + wide_delta, 90);
                RD_M_DrawTextA(DEH_String("?"), 160 +
                                                RD_M_TextAWidth(SlotText[quicksave - 1]) / 2 + wide_delta, 90);
            }
            if (typeofask == 4)
            {
                RD_M_DrawTextA(SlotText[quickload - 1], 160 -
                                                        RD_M_TextAWidth(SlotText[quickload - 1]) / 2 + wide_delta, 90);
                RD_M_DrawTextA(DEH_String("?"), 160 +
                                                RD_M_TextAWidth(SlotText[quickload - 1]) / 2 + wide_delta, 90);
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
        filename = SV_Filename(i);
        fp = fopen(filename, "rb+");
	    free(filename);

        if (!fp)
        {
            SlotText[i][0] = 0; // empty the string
            SlotStatus[i] = 0;
            continue;
        }
        fread(&SlotText[i], SLOTTEXTLEN, 1, fp);
        fclose(fp);
        SlotStatus[i] = 1;
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
        V_DrawShadowedPatchRaven(x + wide_delta, y,
                                 W_CacheLumpName(DEH_String("M_FSLOT"), PU_CACHE));
        if (SlotStatus[i])
        {
            // [JN] Use only small English chars here
            RD_M_DrawTextSmallENG(SlotText[i], x + 5 + wide_delta, y + 5, CR_NONE);
        }
        y += ITEM_HEIGHT;
    }
}

// -----------------------------------------------------------------------------
// DrawOptionsMenu
// -----------------------------------------------------------------------------

static void DrawOptionsMenu(void)
{
    // Sound / Sound System menu background will be drawn.
    sfxbgdraw = true;

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);
}

// -----------------------------------------------------------------------------
// DrawRenderingMenu
// -----------------------------------------------------------------------------

static void DrawRenderingMenu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Display aspect ratio
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                              aspect_ratio_temp == 2 ? "16:9" :
                              aspect_ratio_temp == 3 ? "16:10" :
                              aspect_ratio_temp == 4 ? "21:9" :
                              "4:3",
                              185 + wide_delta, 42, CR_NONE);
        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            RD_M_DrawTextSmallENG("THE PROGRAM MUST BE RESTARTED",
                                  51 + wide_delta, 148, CR_WHITE2GREEN_HERETIC);
        }

        // Vertical sync
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallENG("N/A", 216 + wide_delta, 52, CR_WHITE2GRAY_HERETIC);
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
            RD_M_DrawTextSmallENG("N/A", 131 + wide_delta, 82, CR_WHITE2GRAY_HERETIC);
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

        // Show ENDTEXT screen
        RD_M_DrawTextSmallENG(show_endoom ? "ON" : "OFF", 188 + wide_delta, 132, CR_NONE);
    }
    else
    {
        // Соотношение сторон экрана
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                              aspect_ratio_temp == 2 ? "16:9" :
                              aspect_ratio_temp == 3 ? "16:10" :
                              aspect_ratio_temp == 4 ? "21:9" :
                              "4:3",
                              230 + wide_delta, 42, CR_NONE);

        // Informative message: НЕОБХОДИМ ПЕРЕЗАПУСК ИГРЫ
        if (aspect_ratio_temp != aspect_ratio)
        {
            RD_M_DrawTextSmallRUS("YTJ,[JLBV GTHTPFGECR GHJUHFVVS",
                                  46 + wide_delta, 148, CR_WHITE2GREEN_HERETIC);
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            RD_M_DrawTextSmallRUS("Y/L", 236 + wide_delta, 52, CR_WHITE2GRAY_HERETIC);
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
            RD_M_DrawTextSmallRUS("Y/L", 211 + wide_delta, 82, CR_WHITE2GRAY_HERETIC);
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

        // Показывать экран ENDTEXT
        RD_M_DrawTextSmallENG("ENDTEXT:", 160 + wide_delta, 132, CR_NONE);
        RD_M_DrawTextSmallRUS(show_endoom ? "DRK" : "DSRK", 222 + wide_delta, 132, CR_NONE);
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
}

static void M_RD_Screenshots(Direction_t direction)
{
    png_screenshots ^= 1;
}

static void M_RD_EndText(Direction_t direction)
{
    show_endoom ^= 1;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    char  num[4];

    // Draw menu background. Hide it for a moment while changing 
    // screen size, gamma and level brightness in GS_LEVEL game state.
    if (gamestate != GS_LEVEL || (gamestate == GS_LEVEL && menubgwait < I_GetTime()))
        V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Graphics detail
        RD_M_DrawTextSmallENG(detailLevel ? "LOW" : "HIGH", 149 + wide_delta, 102, CR_NONE);
    }
    else
    {
        // Детализация графики
        RD_M_DrawTextSmallRUS(detailLevel ? "YBPRFZ" : "DSCJRFZ", 188 + wide_delta, 102, CR_NONE);
    }

    //
    // Sliders
    //

    // Screen size
    if (aspect_ratio_temp >= 2)
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 4, screenblocks - 9);
        M_snprintf(num, 4, "%d", screenblocks);
        RD_M_DrawTextSmallENG(num, 88 + wide_delta, 53, CR_WHITE2GRAY_HERETIC);
    }
    else
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 52, 10, screenblocks - 3);
        M_snprintf(num, 4, "%d", screenblocks);
        dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        RD_M_DrawTextA(num, 136 + wide_delta, 53);
        dp_translation = NULL;
    }

    // Gamma-correction
    RD_Menu_DrawSliderSmall(&DisplayMenu, 72, 18, usegamma);

    // Level brightness
    RD_Menu_DrawSliderSmall(&DisplayMenu, 92, 5, level_brightness / 16);
}

static void M_RD_ScreenSize(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt(&screenblocks, 3, 12, direction); // [JN] Now we have 12 screen sizes

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
}

static void M_RD_Gamma(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt(&usegamma, 0, 17, direction);

    I_SetPalette((byte *) W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE));

    P_SetMessage(&players[consoleplayer], english_language ? 
                                          GammaText[usegamma] :
                                          GammaText_Rus[usegamma],
                                          msg_system, false);
}

static void M_RD_LevelBrightness(Direction_t direction)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    RD_Menu_SlideInt_Step(&level_brightness, 0, 64, 16, direction);
}

static void M_RD_Detail(Direction_t direction)
{
    detailLevel ^= 1;

    R_SetViewSize (screenblocks, detailLevel);

    P_SetMessage(&players[consoleplayer], detailLevel ?
                 txt_detail_low : txt_detail_high, msg_system, false);
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

        // Таймаут отображения (текст)
        RD_M_DrawTextSmallENG(messages_timeout == 1 ? "1 SECOND" :
                              messages_timeout == 2 ? "2 SECONDS" :
                              messages_timeout == 3 ? "3 SECONDS" :
                              messages_timeout == 4 ? "4 SECONDS" :
                              messages_timeout == 5 ? "5 SECONDS" :
                              messages_timeout == 6 ? "6 SECONDS" :
                              messages_timeout == 7 ? "7 SECONDS" :
                              messages_timeout == 8 ? "8 SECONDS" :
                              messages_timeout == 9 ? "9 SECONDS" :
                              "10 SECONDS", 136 + wide_delta, 73, CR_WHITE2GRAY_HERETIC);

        // Fading effect 
        RD_M_DrawTextSmallENG(message_fade ? "ON" : "OFF",
                              140 + wide_delta, 82, CR_WHITE2GRAY_HERETIC);

        // Text casts shadows
        RD_M_DrawTextSmallENG(draw_shadowed_text ? "ON" : "OFF",
                              179 + wide_delta, 92, CR_WHITE2GRAY_HERETIC);

        // Local time
        RD_M_DrawTextSmallENG(local_time == 1 ? "12-HOUR (HH:MM)" :
                              local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                              local_time == 3 ? "24-HOUR (HH:MM)" :
                              local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF",
                              110 + wide_delta, 112, CR_NONE);

        // Item pickup
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_pickup_color), 120 + wide_delta, 132,
                              M_RD_ColorTranslation(message_pickup_color));

        // Revealed secret
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_secret_color), 157 + wide_delta, 142,
                              M_RD_ColorTranslation(message_secret_color));

        // System message
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_system_color), 152 + wide_delta, 152,
                              M_RD_ColorTranslation(message_system_color));

        // Netgame chat
        RD_M_DrawTextSmallENG(M_RD_ColorName(message_chat_color), 135 + wide_delta, 162,
                              M_RD_ColorTranslation(message_chat_color));
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
                              "10 CTREYL", 136 + wide_delta, 73, CR_WHITE2GRAY_HERETIC);

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
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_pickup_color), 187 + wide_delta, 132,
                              M_RD_ColorTranslation(message_pickup_color));

        // Обнаружение тайников
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_secret_color), 195 + wide_delta, 142,
                              M_RD_ColorTranslation(message_secret_color));

        // Системные сообщения
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_system_color), 191 + wide_delta, 152,
                              M_RD_ColorTranslation(message_system_color));

        // Чат сетевой игры
        RD_M_DrawTextSmallRUS(M_RD_ColorName(message_chat_color), 162 + wide_delta, 162,
                              M_RD_ColorTranslation(message_chat_color));
    }

    // Messages timeout
    RD_Menu_DrawSliderSmall(&DisplayMenu, 72, 10, messages_timeout - 1);
}

static void M_RD_LocalTime(Direction_t direction)
{
    RD_Menu_SpinInt(&local_time, 0, 4, direction);
}

static void M_RD_Messages(Direction_t direction)
{
    show_messages ^= 1;

    if (show_messages)
    {
        P_SetMessage(&players[consoleplayer], txt_messages_on, msg_system, true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], txt_messages_off, msg_system, true);
    }

    S_StartSound(NULL, sfx_chat);
}

static void M_RD_MessagesAlignment(Direction_t direction)
{
    RD_Menu_SpinInt(&messages_alignment, 0, 2, direction);
}

static void M_RD_MessagesTimeout(Direction_t direction)
{
    RD_Menu_SlideInt(&messages_timeout, 1, 10, direction);
}

static void M_RD_MessagesFade(Direction_t direction)
{
    message_fade ^= 1;
}

static void M_RD_ShadowedText(Direction_t direction)
{
    draw_shadowed_text ^= 1;
}

void M_RD_Define_Msg_Color (MessageType_t messageType, int color)
{
    Translation_CR_t * colorVar;
    switch (messageType)
    {
        case msg_pickup: // Item pickup.
            colorVar = &messages_pickup_color_set;
            break;
        case msg_secret: // Revealed secret
            colorVar = &messages_secret_color_set;
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
            case 1:   *colorVar = CR_WHITE2GRAY_HERETIC;      break;
            case 2:   *colorVar = CR_WHITE2DARKGRAY_HERETIC;  break;
            case 3:   *colorVar = CR_WHITE2RED_HERETIC;       break;
            case 4:   *colorVar = CR_WHITE2DARKRED_HERETIC;   break;
            case 5:   *colorVar = CR_WHITE2GREEN_HERETIC;     break;
            case 6:   *colorVar = CR_WHITE2DARKGREEN_HERETIC; break;
            case 7:   *colorVar = CR_WHITE2OLIVE_HERETIC;     break;
            case 8:   *colorVar = CR_WHITE2BLUE_HERETIC;      break;
            case 9:   *colorVar = CR_WHITE2DARKBLUE_HERETIC;  break;
            case 10:  *colorVar = CR_WHITE2PURPLE_HERETIC;    break;
            case 11:  *colorVar = CR_WHITE2NIAGARA_HERETIC;   break;
            case 12:  *colorVar = CR_WHITE2AZURE_HERETIC;     break;
            case 13:  *colorVar = CR_WHITE2YELLOW_HERETIC;    break;
            case 14:  *colorVar = CR_WHITE2GOLD_HERETIC;      break;
            case 15:  *colorVar = CR_WHITE2DARKGOLD_HERETIC;  break;
            case 16:  *colorVar = CR_WHITE2TAN_HERETIC;       break;
            case 17:  *colorVar = CR_WHITE2BROWN_HERETIC;     break;
            default:  *colorVar = CR_NONE;                    break;
        }
    }
}

void M_RD_Change_Msg_Pickup_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_pickup_color, 0, 17, direction);

    // [JN] Redefine pickup message color.
    M_RD_Define_Msg_Color(msg_pickup, message_pickup_color);
}

void M_RD_Change_Msg_Secret_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_secret_color, 0, 17, direction);

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_Color(msg_secret, message_secret_color);
}

void M_RD_Change_Msg_System_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_system_color, 0, 17, direction);

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_Color(msg_system, message_system_color);
}

void M_RD_Change_Msg_Chat_Color(Direction_t direction)
{
    RD_Menu_SpinInt(&message_chat_color, 0, 17, direction);

    // [JN] Redefine netgame chat message color.
    M_RD_Define_Msg_Color(msg_chat, message_chat_color);
}

// -----------------------------------------------------------------------------
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    char  num[4];

    M_snprintf(num, 4, "%d", automap_grid_size);

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Overlay mode
        RD_M_DrawTextSmallENG(automap_overlay ? "ON" : "OFF", 176 + wide_delta, 42, CR_NONE);

        // Rotate mode
        RD_M_DrawTextSmallENG(automap_rotate ? "ON" : "OFF", 169 + wide_delta, 52, CR_NONE);

        // Follow mode
        RD_M_DrawTextSmallENG(automap_follow ? "ON" : "OFF", 165 + wide_delta, 62, CR_NONE);

        // Grid
        RD_M_DrawTextSmallENG(automap_grid ? "ON" : "OFF", 114 + wide_delta, 72, CR_NONE);

        // Grid size
        RD_M_DrawTextSmallENG(num, 147 + wide_delta, 82, CR_NONE);

        // Level stats
        RD_M_DrawTextSmallENG(automap_stats == 1 ? "IN AUTOMAP" :
                              automap_stats == 2 ? "ALWAYS" : "OFF",
                              163 + wide_delta, 102, CR_NONE);

        // Level time
        RD_M_DrawTextSmallENG(automap_level_time == 1 ? "IN AUTOMAP" :
                              automap_level_time == 2 ? "ALWAYS" : "OFF",
                              152 + wide_delta, 112, CR_NONE);

        // Total time
        RD_M_DrawTextSmallENG(automap_total_time == 1 ? "IN AUTOMAP" :
                              automap_total_time == 2 ? "ALWAYS" : "OFF",
                              153 + wide_delta, 122, CR_NONE);

        // Player coords
        RD_M_DrawTextSmallENG(automap_coords == 1 ? "IN AUTOMAP" :
                              automap_coords == 2 ? "ALWAYS" : "OFF",
                              184 + wide_delta, 132, CR_NONE);
    }
    else
    {
        // Режим наложения
        RD_M_DrawTextSmallRUS(automap_overlay ? "DRK" : "DSRK", 187 + wide_delta, 42, CR_NONE);

        // Режим вращения
        RD_M_DrawTextSmallRUS(automap_rotate ? "DRK" : "DSRK", 179 + wide_delta, 52, CR_NONE);

        // Режим следования
        RD_M_DrawTextSmallRUS(automap_follow ? "DRK" : "DSRK", 194 + wide_delta, 62, CR_NONE);

        // Сетка
        RD_M_DrawTextSmallRUS(automap_grid ? "DRK" : "DSRK", 107 + wide_delta, 72, CR_NONE);

        // Размер сетки
        RD_M_DrawTextSmallRUS(num, 158 + wide_delta, 82, CR_NONE);

        // Статистика уровня
        RD_M_DrawTextSmallRUS(automap_stats == 1 ? "YF RFHNT" :
                              automap_stats == 2 ? "DCTULF" : "DSRK",
                              193 + wide_delta, 102, CR_NONE);

        // Время уровня
        RD_M_DrawTextSmallRUS(automap_level_time == 1 ? "YF RFHNT" :
                              automap_level_time == 2 ? "DCTULF" : "DSRK",
                              158 + wide_delta, 112, CR_NONE);

        // Общее время
        RD_M_DrawTextSmallRUS(automap_total_time == 1 ? "YF RFHNT" :
                              automap_total_time == 2 ? "DCTULF" : "DSRK",
                              161 + wide_delta, 122, CR_NONE);

        // Координаты игрока
        RD_M_DrawTextSmallRUS(automap_coords == 1 ? "YF RFHNT" :
                              automap_coords == 2 ? "DCTULF" : "DSRK",
                              198 + wide_delta, 132, CR_NONE);
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

static void M_RD_AutoMapGridSize(Direction_t direction)
{
    RD_Menu_ShiftSlideInt(&automap_grid_size, 32, 512, direction);
}

static void M_RD_AutoMapStats(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_stats, 0, 2, direction);
}

static void M_RD_AutoMapLevTime(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_level_time, 0, 2, direction);
}

static void M_RD_AutoMapTotTime(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_total_time, 0, 2, direction);
}

static void M_RD_AutoMapCoords(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_coords, 0, 2, direction);
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    char  num[4];

    // Draw menu background. Don't draw if menu is invoked by pressing F4.
    if (sfxbgdraw)
        V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    //
    // Sliders
    //

    // SFX Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 52, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%d", snd_MaxVolume_tmp);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 53, CR_WHITE2GRAY_HERETIC);

    // Music Volume
    RD_Menu_DrawSliderSmall(&SoundMenu, 72, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%d", snd_MusicVolume);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 73, CR_WHITE2GRAY_HERETIC);

    // SFX Channels
    RD_Menu_DrawSliderSmall(&SoundMenu, 102, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%d", snd_Channels);
    RD_M_DrawTextSmallENG(num, 184 + wide_delta, 103, CR_WHITE2GRAY_HERETIC);
}

static void M_RD_SfxVolume(Direction_t direction)
{
    RD_Menu_SlideInt(&snd_MaxVolume, 0, 15, direction);

    snd_MaxVolume_tmp = snd_MaxVolume; // [JN] Sync temp volume variable.

    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
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
    // Draw menu background. Don't draw if menu is invoked by pressing F4.
    if (sfxbgdraw)
        V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Sound effects
        if (snd_sfxdevice == 0)
        {
            RD_M_DrawTextSmallENG("DISABLED", 144 + wide_delta, 42, CR_WHITE2GRAY_HERETIC);
        }
        else if (snd_sfxdevice == 3)
        {
            RD_M_DrawTextSmallENG("DIGITAL SFX", 144 + wide_delta, 42, CR_NONE);
        }

        // Music
        if (snd_musicdevice == 0)
        {
            RD_M_DrawTextSmallENG("DISABLED", 80 + wide_delta, 52, CR_WHITE2GRAY_HERETIC);
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
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC", 80 + wide_delta, 52, CR_NONE);
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
        if (CurrentItPos == 3)
        {
            RD_M_DrawTextSmallENG("CHANGING WILL REQUIRE RESTART OF THE PROGRAM",
                                  3 + wide_delta, 132, CR_WHITE2GREEN_HERETIC);
        }
    }
    else
    {
        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            RD_M_DrawTextSmallRUS("JNRK.XTYS", 173 + wide_delta, 42, CR_WHITE2GRAY_HERETIC);
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
            RD_M_DrawTextSmallRUS("JNRK.XTYF", 91 + wide_delta, 52, CR_WHITE2GRAY_HERETIC);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // СИНТЕЗ OPL2
            RD_M_DrawTextSmallRUS("CBYNTP J", 91 + wide_delta, 52, CR_NONE);
            RD_M_DrawTextSmallENG("OPL2", 140 + wide_delta, 52, CR_NONE);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // СИНТЕЗ OPL3
            RD_M_DrawTextSmallRUS("CBYNTP J", 91 + wide_delta, 52, CR_NONE);
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
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC", 91 + wide_delta, 52, CR_NONE);
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
        if (CurrentItPos == 3)
        {
            RD_M_DrawTextSmallRUS("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS",
                                  11 + wide_delta, 132, CR_WHITE2GREEN_HERETIC);
        }
    }
}

static void M_RD_SoundDevice(Direction_t direction)
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
    S_SetMaxVolume(true);
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

    // Shut down current music
    S_StopSong();

    // Shut down music system
    I_ShutdownSound();
    
    // Start music system
    I_InitSound(true);

    // Reinitialize music volume
    S_SetMusicVolume();

    // Restart current music
    S_StartSong(mus_song, true, true);
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
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Always run
        RD_M_DrawTextSmallENG(joybspeed >= 20 ? "ON" : "OFF", 118 + wide_delta, 42, CR_NONE);

        // Mouse look
        RD_M_DrawTextSmallENG(mlook ? "ON" : "OFF", 118 + wide_delta, 122, CR_NONE);

        // Invert Y axis
        RD_M_DrawTextSmallENG(mouse_y_invert ? "ON" : "OFF",
                              133 + wide_delta, 132, !mlook ? CR_WHITE2GRAY_HERETIC : CR_NONE);

        // Novert
        RD_M_DrawTextSmallENG(!novert ? "ON" : "OFF",
                              168 + wide_delta, 142, mlook ? CR_WHITE2GRAY_HERETIC : CR_NONE);
    }
    else
    {
        // Режим постоянного бега
        RD_M_DrawTextSmallRUS(joybspeed >= 20 ? "DRK" : "DSRK", 209 + wide_delta, 42, CR_NONE);

        // Обзор мышью
        RD_M_DrawTextSmallRUS(mlook ? "DRK" : "DSRK", 132 + wide_delta, 122, CR_NONE);

        // Вертикальная инверсия
        RD_M_DrawTextSmallRUS(mouse_y_invert ? "DRK" : "DSRK",
                              199 + wide_delta, 132, !mlook ? CR_WHITE2GRAY_HERETIC : CR_NONE);

        // Вертикальное перемещение
        RD_M_DrawTextSmallRUS(!novert ? "DRK" : "DSRK",
                              227 + wide_delta, 142, mlook ? CR_WHITE2GRAY_HERETIC : CR_NONE);
    }

    //
    // Sliders
    //

    // Mouse sensivity
    RD_Menu_DrawSliderSmall(&ControlsMenu, 72, 12, mouseSensitivity);
    M_snprintf(num, 4, "%d", mouseSensitivity);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 73, CR_WHITE2GRAY_HERETIC);

    // Acceleration
    RD_Menu_DrawSliderSmall(&ControlsMenu, 92, 12, mouse_acceleration * 4 - 4);
    M_snprintf(num, 4, "%f", mouse_acceleration);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 93, CR_WHITE2GRAY_HERETIC);

    // Threshold
    RD_Menu_DrawSliderSmall(&ControlsMenu, 112, 12, mouse_threshold / 2);
    M_snprintf(num, 4, "%d", mouse_threshold);
    RD_M_DrawTextSmallENG(num, 152 + wide_delta, 113, CR_WHITE2GRAY_HERETIC);
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

static void M_RD_MouseLook(Direction_t direction)
{
    mlook ^= 1;

    if (!mlook)
        players[consoleplayer].centering = true;
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
// DrawGameplay1Menu
// -----------------------------------------------------------------------------

static void DrawGameplay1Menu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Brightmaps
        RD_M_DrawTextSmallENG(brightmaps ? "ON" : "OFF", 119 + wide_delta, 36,
                              brightmaps ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Fake contrast
        RD_M_DrawTextSmallENG(fake_contrast ? "ON" : "OFF", 143 + wide_delta, 46,
                              fake_contrast ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Extra translucency
        RD_M_DrawTextSmallENG(translucency ? "ON" : "OFF", 180 + wide_delta, 56,
                              translucency ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Colored blood
        RD_M_DrawTextSmallENG(colored_blood ? "ON" : "OFF", 139 + wide_delta, 66,
                              colored_blood ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Swirling liquids
        RD_M_DrawTextSmallENG(swirling_liquids ? "ON" : "OFF", 147 + wide_delta, 76,
                              swirling_liquids ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Invulnerability affects sky
        RD_M_DrawTextSmallENG(invul_sky ? "ON" : "OFF", 235 + wide_delta, 86,
                              invul_sky ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Sky drawing mode
        RD_M_DrawTextSmallENG(linear_sky ? "LINEAR" : "ORIGINAL", 162 + wide_delta, 96,
                              linear_sky ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Corpses sliding from the ledges
        RD_M_DrawTextSmallENG(torque ? "ON" : "OFF", 238 + wide_delta, 116,
                              torque ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Weapon bobbing while firing
        RD_M_DrawTextSmallENG(weapon_bobbing ? "ON" : "OFF", 233 + wide_delta, 126,
                              weapon_bobbing ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Randomly flipped corpses
        RD_M_DrawTextSmallENG(randomly_flipcorpses ? "ON" : "OFF", 232 + wide_delta, 136,
                              randomly_flipcorpses ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Floating items amplitude
        RD_M_DrawTextSmallENG(floating_powerups == 1 ? "STANDARD" :
                              floating_powerups == 2 ? "HALFED" : "OFF",
                              209 + wide_delta, 146, floating_powerups ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
    else
    {
        // Брайтмаппинг
        RD_M_DrawTextSmallRUS(brightmaps ? "DRK" : "DSRK", 133 + wide_delta, 36,
                              brightmaps ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Имитация контрастности
        RD_M_DrawTextSmallRUS(fake_contrast ? "DRK" : "DSRK", 205 + wide_delta, 46,
                              fake_contrast ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Дополнительная прозрачность
        RD_M_DrawTextSmallRUS(translucency ? "DRK" : "DSRK", 245 + wide_delta, 56,
                              translucency ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Разноцветная кровь
        RD_M_DrawTextSmallRUS(colored_blood ? "DRK" : "DSRK", 178 + wide_delta, 66,
                              colored_blood ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Улучшенная анимация жидкостей
        RD_M_DrawTextSmallRUS(swirling_liquids ? "DRK" : "DSRK", 261 + wide_delta, 76,
                              swirling_liquids ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Неуязвимость окрашивает небо
        RD_M_DrawTextSmallRUS(invul_sky ? "DRK" : "DSRK", 253 + wide_delta, 86,
                              invul_sky ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Режим отрисовки неба
        RD_M_DrawTextSmallRUS(linear_sky ? "KBYTQYSQ" : "JHBUBYFKMYSQ", 195 + wide_delta, 96,
                              linear_sky ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Трупы сползают с возвышений
        RD_M_DrawTextSmallRUS(torque ? "DRK" : "DSRK", 248 + wide_delta, 116,
                              torque ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Улучшенное покачивание оружия
        RD_M_DrawTextSmallRUS(weapon_bobbing ? "DRK" : "DSRK", 260 + wide_delta, 126,
                              weapon_bobbing ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Зеркалирование трупов
        RD_M_DrawTextSmallRUS(randomly_flipcorpses ? "DRK" : "DSRK", 201 + wide_delta, 136,
                              randomly_flipcorpses ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Амплитуда левитации предметов
        RD_M_DrawTextSmallRUS(floating_powerups == 1 ? "CNFYLFHNYFZ" :
                              floating_powerups == 2 ? "EVTHTYYFZ" : "DSRK",
                              188 + wide_delta, 146, floating_powerups ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
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

static void M_RD_ExtraTrans(Direction_t direction)
{
    translucency ^= 1;
}

static void M_RD_ColoredBlood(Direction_t direction)
{
    colored_blood ^= 1;
}

static void M_RD_InvulSky(Direction_t direction)
{
    invul_sky ^= 1;
}

static void M_RD_SwirlingLiquids(Direction_t direction)
{
    swirling_liquids ^= 1;
}

static void M_RD_LinearSky(Direction_t direction)
{
    linear_sky ^= 1;
}

static void M_RD_Torque(Direction_t direction)
{
    torque ^= 1;
}

static void M_RD_Bobbing(Direction_t direction)
{
    weapon_bobbing ^= 1;
}

static void M_RD_FlipCorpses(Direction_t direction)
{
    randomly_flipcorpses ^= 1;
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
        // Colored Status Bar
        RD_M_DrawTextSmallENG(sbar_colored ? "ON" : "OFF", 177 + wide_delta, 36,
                          sbar_colored ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Colored health gem
        RD_M_DrawTextSmallENG(sbar_colored_gem == 1 ? "BRIGHT" :
                              sbar_colored_gem == 2 ? "DARK" : "OFF",
                              175 + wide_delta, 46,
                              sbar_colored_gem == 1 ? CR_WHITE2GREEN_HERETIC :
                              sbar_colored_gem == 2 ? CR_WHITE2DARKGREEN_HERETIC :
                              CR_WHITE2RED_HERETIC);

        // Negative health
        RD_M_DrawTextSmallENG(negative_health ? "ON" : "OFF", 190 + wide_delta, 56,
                              negative_health ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Draw widget
        RD_M_DrawTextSmallENG(ammo_widget == 1 ? "BRIEF" : ammo_widget == 2 ? "FULL" :  "OFF",
                              124 + wide_delta, 76, ammo_widget ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Coloring
        RD_M_DrawTextSmallENG(ammo_widget_colored ? "ON" : "OFF", 101 + wide_delta, 86,
                              ammo_widget_colored ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Draw crosshair
        RD_M_DrawTextSmallENG(crosshair_draw ? "ON" : "OFF", 150 + wide_delta, 106,
                              crosshair_draw ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Indication
        RD_M_DrawTextSmallENG(crosshair_type == 1 ? "HEALTH" :
                              crosshair_type == 2 ? "TARGET HIGHLIGHTING" :
                              crosshair_type == 3 ? "TARGET HIGHLIGHTING+HEALTH" :
                              "STATIC",
                              111 + wide_delta, 116, crosshair_type ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Increased size
        RD_M_DrawTextSmallENG(crosshair_scale ? "ON" : "OFF", 146 + wide_delta, 126,
                              crosshair_scale ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
    else
    {
        // Разноцветные элементы
        RD_M_DrawTextSmallRUS(sbar_colored ? "DRK" : "DSRK", 206 + wide_delta, 36,
                              sbar_colored ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Окрашивание камня здоровья
        RD_M_DrawTextSmallRUS(sbar_colored_gem == 1 ? "CDTNKJT" :
                              sbar_colored_gem == 2 ? "NTVYJT" : "DSRK", 238 + wide_delta, 46,
                              sbar_colored_gem == 1 ? CR_WHITE2GREEN_HERETIC :
                              sbar_colored_gem == 2 ? CR_WHITE2DARKGREEN_HERETIC :
                              CR_WHITE2RED_HERETIC);

        // Отрицательное здоровье
        RD_M_DrawTextSmallRUS(negative_health ? "DRK" : "DSRK", 211 + wide_delta, 56,
                          negative_health ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Отображать виджет
        RD_M_DrawTextSmallRUS(ammo_widget == 1 ? "RHFNRBQ" :
                              ammo_widget == 2 ? "GJLHJ,YSQ" : "DSRK", 179 + wide_delta, 76,
                              ammo_widget ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Цветовая индикация
        RD_M_DrawTextSmallRUS(ammo_widget_colored ? "DRK" : "DSRK", 178 + wide_delta, 86,
                              ammo_widget_colored ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Отображать прицел
        RD_M_DrawTextSmallRUS(crosshair_draw ? "DRK" : "DSRK", 175 + wide_delta, 106,
                              crosshair_draw ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Индикация
        RD_M_DrawTextSmallRUS(crosshair_type == 1 ? "PLJHJDMT" : // ЗДОРОВЬЕ
                              crosshair_type == 2 ? "GJLCDTNRF WTKB" : // ПОДСВЕТКА ЦЕЛИ
                              crosshair_type == 3 ? "GJLCDTNRF WTKB+PLJHJDMT" :
                              "CNFNBXYFZ", // СТАТИЧНАЯ
                              111 + wide_delta, 116, crosshair_type ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Увеличенный размер
        RD_M_DrawTextSmallRUS(crosshair_scale ? "DRK" : "DSRK", 181 + wide_delta, 126,
                              crosshair_scale ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
}

static void M_RD_ColoredSBar(Direction_t direction)
{
    sbar_colored ^= 1;
}

static void M_RD_ColoredGem(Direction_t direction)
{
    RD_Menu_SpinInt(&sbar_colored_gem, 0, 2, direction);
}

static void M_RD_NegativeHealth(Direction_t direction)
{
    negative_health ^= 1;
}

static void M_RD_AmmoWidgetDraw(Direction_t direction)
{
    RD_Menu_SpinInt(&ammo_widget, 0, 2, direction);
}

static void M_RD_AmmoWidgetColoring(Direction_t direction)
{
    ammo_widget_colored ^= 1;
}

static void M_RD_CrossHairDraw(Direction_t direction)
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairType(Direction_t direction)
{
    RD_Menu_SpinInt(&crosshair_type, 0, 3, direction);
}

static void M_RD_CrossHairScale(Direction_t direction)
{
    crosshair_scale ^= 1;
}

// -----------------------------------------------------------------------------
// DrawGameplay3Menu
// -----------------------------------------------------------------------------

static void DrawGameplay3Menu(void)
{
    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Sound attenuation axises
        RD_M_DrawTextSmallENG(z_axis_sfx ? "X/Y/Z" : "X/Y", 219 + wide_delta, 36,
                              z_axis_sfx ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Monster alert waking up others
        RD_M_DrawTextSmallENG(noise_alert_sfx ? "ON" : "OFF", 262 + wide_delta, 46,
                              noise_alert_sfx ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Notify of revealed secrets
        RD_M_DrawTextSmallENG(secret_notification ? "ON" : "OFF", 235 + wide_delta, 66,
                              secret_notification ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Active artifacts
        RD_M_DrawTextSmallENG(show_all_artifacts ? "ALL" : "WINGS/TOME", 195 + wide_delta, 76,
                              show_all_artifacts ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Artifacts timer
        RD_M_DrawTextSmallENG(show_artifacts_timer == 1 ? "GOLD" :
                              show_artifacts_timer == 2 ? "SILVER" :
                              show_artifacts_timer == 3 ? "COLORED" : "OFF",
                              150 + wide_delta, 86,
                              show_artifacts_timer == 1 ? CR_WHITE2DARKGOLD_HERETIC :
                              show_artifacts_timer == 2 ? CR_WHITE2GRAY_HERETIC :
                              show_artifacts_timer == 3 ? CR_WHITE2GREEN_HERETIC :
                              CR_WHITE2RED_HERETIC);

        // Fix errors of vanilla maps
        RD_M_DrawTextSmallENG(fix_map_errors ? "ON" : "OFF", 226 + wide_delta, 106,
                              fix_map_errors ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Flip game levels
        RD_M_DrawTextSmallENG(flip_levels ? "ON" : "OFF",
                              153 + wide_delta, 116,
                              flip_levels ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Play internal demos
        RD_M_DrawTextSmallENG(no_internal_demos ? "OFF" : "ON",
                              179 + wide_delta, 126,
                              no_internal_demos ? CR_WHITE2RED_HERETIC : CR_WHITE2GREEN_HERETIC);

        // Wand start
        RD_M_DrawTextSmallENG(pistol_start ? "ON" : "OFF",
                              193 + wide_delta, 136,
                              pistol_start ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
    else
    {
        // Затухание звука по осям
        RD_M_DrawTextSmallENG(z_axis_sfx ? "X/Y/Z" : "X/Y",
                              209 + wide_delta, 36, z_axis_sfx ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Общая тревога у монстров
        RD_M_DrawTextSmallRUS(noise_alert_sfx ? "DRK" : "DSRK", 223 + wide_delta, 46,
                              noise_alert_sfx ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);


        // Сообщать о найденном тайнике
        RD_M_DrawTextSmallRUS(secret_notification ? "DRK" : "DSRK", 251 + wide_delta, 66,
                          secret_notification ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Индикация артефаектов
        RD_M_DrawTextSmallRUS(show_all_artifacts ? "DCT FHNTAFRNS" : "RHSKMZ/NJV", 196 + wide_delta, 76,
                              show_all_artifacts ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Таймер артефаектов
        RD_M_DrawTextSmallRUS(show_artifacts_timer == 1 ? "PJKJNJQ" :
                              show_artifacts_timer == 2 ? "CTHT,HZYSQ" :
                              show_artifacts_timer == 3 ? "HFPYJWDTNYSQ" : "DSRK",
                              175 + wide_delta, 86,
                              show_artifacts_timer == 1 ? CR_WHITE2DARKGOLD_HERETIC :
                              show_artifacts_timer == 2 ? CR_WHITE2GRAY_HERETIC :
                              show_artifacts_timer == 3 ? CR_WHITE2GREEN_HERETIC :
                              CR_WHITE2RED_HERETIC);

        // Устранять ошибки оригинальных уровней
        RD_M_DrawTextSmallRUS(fix_map_errors ? "DRK" : "DSRK", 257 + wide_delta, 106,
                              fix_map_errors ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Зеркальное отражение уровней
        RD_M_DrawTextSmallRUS(flip_levels ? "DRK" : "DSRK",
                              255 + wide_delta, 116,
                              flip_levels ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Проигрывать демозаписи
        RD_M_DrawTextSmallRUS(no_internal_demos ? "DSRK" : "DRK",
                              211 + wide_delta, 126,
                              no_internal_demos ? CR_WHITE2RED_HERETIC : CR_WHITE2GREEN_HERETIC);

        // Режим игры "Wand start"
        RD_M_DrawTextSmallRUS("HT;BV BUHS", 36 + wide_delta, 136, CR_NONE);
        RD_M_DrawTextSmallENG("\"WAND START\":", 120 + wide_delta, 136, CR_NONE);
        RD_M_DrawTextSmallRUS(pistol_start ? "DRK" : "DSRK",
                              217 + wide_delta, 136,
                              pistol_start ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
}

static void M_RD_ZAxisSFX(Direction_t direction)
{
    z_axis_sfx ^= 1;
}

static void M_RD_AlertSFX(Direction_t direction)
{
    noise_alert_sfx ^= 1;
}

static void M_RD_SecretNotify(Direction_t direction)
{
    secret_notification ^= 1;
}

static void M_RD_ShowAllArti(Direction_t direction)
{
    show_all_artifacts ^= 1;
}

static void M_RD_ShowArtiTimer(Direction_t direction)
{
    RD_Menu_SpinInt(&show_artifacts_timer, 0, 3, direction);
}

static void M_RD_FixMapErrors(Direction_t direction)
{
    fix_map_errors ^= 1;
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

static void M_RD_WandStart(Direction_t direction)
{
    pistol_start ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect1Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect1Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Gauntlets
        RD_M_DrawTextSmallENG(selective_wp_gauntlets ? "YES" : "NO", 228 + wide_delta, 106,
                              selective_wp_gauntlets ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Ethereal Crossbow
        RD_M_DrawTextSmallENG(selective_wp_crossbow ? "YES" : "NO", 228 + wide_delta, 116,
                              selective_wp_crossbow ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Dragon Claw
        RD_M_DrawTextSmallENG(selective_wp_dragonclaw ? "YES" : "NO", 228 + wide_delta, 126,
                              selective_wp_dragonclaw ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Hellstaff
        RD_M_DrawTextSmallENG(selective_wp_hellstaff ? "YES" : "NO", 228 + wide_delta, 136,
                              selective_wp_hellstaff ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Phoenix Rod
        RD_M_DrawTextSmallENG(selective_wp_phoenixrod ? "YES" : "NO", 228 + wide_delta, 146,
                              selective_wp_phoenixrod ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Firemace
        RD_M_DrawTextSmallENG(selective_wp_firemace ? "YES" : "NO", 228 + wide_delta, 156,
                              selective_wp_firemace ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
    else
    {
        // Перчатки
        RD_M_DrawTextSmallRUS(selective_wp_gauntlets ? "LF" : "YTN", 228 + wide_delta, 106,
                              selective_wp_gauntlets ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Эфирный арбалет
        RD_M_DrawTextSmallRUS(selective_wp_crossbow ? "LF" : "YTN", 228 + wide_delta, 116,
                              selective_wp_crossbow ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Коготь дракона
        RD_M_DrawTextSmallRUS(selective_wp_dragonclaw ? "LF" : "YTN", 228 + wide_delta, 126,
                              selective_wp_dragonclaw ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Посох Ада
        RD_M_DrawTextSmallRUS(selective_wp_hellstaff ? "LF" : "YTN", 228 + wide_delta, 136,
                              selective_wp_hellstaff ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Жезл Феникса
        RD_M_DrawTextSmallRUS(selective_wp_phoenixrod ? "LF" : "YTN", 228 + wide_delta, 146,
                              selective_wp_phoenixrod ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Огненная булава
        RD_M_DrawTextSmallRUS(selective_wp_firemace ? "LF" : "YTN", 228 + wide_delta, 156,
                              selective_wp_firemace ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }

    // Skill level | Сложность
    M_snprintf(num, 4, "%d", selective_skill+1);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 26, CR_NONE);

    // Episode | Эпизод
    if (gamemode == shareware)
    {
        RD_M_DrawTextSmallENG("1", 228 + wide_delta, 36, CR_WHITE2GRAY_HERETIC);
    }
    else
    {
        M_snprintf(num, 4, "%d", selective_episode);
        RD_M_DrawTextSmallENG(num, 228 + wide_delta, 36, CR_NONE);
    }

    // Map | Уровень
    M_snprintf(num, 4, "%d", selective_map);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 46, CR_NONE);

    // Health | Здоровье
    M_snprintf(num, 4, "%d", selective_health);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 66,
                          selective_health >= 67 ? CR_WHITE2GREEN_HERETIC :
                          selective_health >= 34 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Armor | Броня
    M_snprintf(num, 4, "%d", selective_armor);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 76,
                          selective_armor == 0 ? CR_WHITE2RED_HERETIC :
                          selective_armortype == 1 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2GREEN_HERETIC);

    // Armor type | Тип брони
    M_snprintf(num, 4, "%d", selective_armortype);
    RD_M_DrawTextSmallENG(selective_armortype == 1 ? "1" : "2", 228 + wide_delta, 86,
                          selective_armortype == 1 ? CR_WHITE2DARKGOLD_HERETIC : CR_WHITE2GREEN_HERETIC);
}

static void M_RD_SelectiveSkill(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_skill, 0, 5, direction);
}

static void M_RD_SelectiveEpisode(Direction_t direction)
{
    // [JN] Shareware have only 1 episode.
    if (gamemode == shareware)
        return;

    RD_Menu_SlideInt(&selective_episode, 1, gamemode == retail ? 5 : 3, direction);
}

static void M_RD_SelectiveMap(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_map, 1, 9, direction);
}

static void M_RD_SelectiveHealth(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_health, 1, 100, direction);
}

static void M_RD_SelectiveArmor(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor, 0, selective_armortype == 1 ? 100 : 200, direction);
}

static void M_RD_SelectiveArmorType(Direction_t direction)
{
    selective_armortype++;

    if (selective_armortype > 2)
        selective_armortype = 1;

    // [JN] Silver Shield armor can't go above 100.
    if (selective_armortype == 1 && selective_armor > 100)
        selective_armor = 100;
}

static void M_RD_SelectiveGauntlets(Direction_t direction)
{
    selective_wp_gauntlets ^= 1;
}

static void M_RD_SelectiveCrossbow(Direction_t direction)
{
    selective_wp_crossbow ^= 1;
}

static void M_RD_SelectiveDragonClaw(Direction_t direction)
{
    selective_wp_dragonclaw ^= 1;
}

static void M_RD_SelectiveHellStaff(Direction_t direction)
{
    selective_wp_hellstaff ^= 1;
}

static void M_RD_SelectivePhoenixRod(Direction_t direction)
{
    selective_wp_phoenixrod ^= 1;
}

static void M_RD_SelectiveFireMace(Direction_t direction)
{
    selective_wp_firemace ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect2Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect2Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Bag of Holding
        RD_M_DrawTextSmallENG(selective_backpack ? "YES" : "NO", 228 + wide_delta, 26,
                              selective_backpack ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Yellow Key
        RD_M_DrawTextSmallENG(selective_key_0 ? "YES" : "NO", 228 + wide_delta, 106,
                              selective_key_0 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Green Key
        RD_M_DrawTextSmallENG(selective_key_1 ? "YES" : "NO", 228 + wide_delta, 116,
                              selective_key_1 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Blue Key
        RD_M_DrawTextSmallENG(selective_key_2 ? "YES" : "NO", 228 + wide_delta, 126,
                              selective_key_2 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Fast Monsters
        RD_M_DrawTextSmallENG(selective_fast ? "YES" : "NO", 228 + wide_delta, 146,
                              selective_fast ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Respawning Monsters
        RD_M_DrawTextSmallENG(selective_respawn ? "YES" : "NO", 228 + wide_delta, 156,
                              selective_respawn ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }
    else
    {
        // Носильный кошель
        RD_M_DrawTextSmallRUS(selective_backpack ? "LF" : "YTN", 228 + wide_delta, 26,
                              selective_backpack ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Желтый ключ
        RD_M_DrawTextSmallRUS(selective_key_0 ? "LF" : "YTN", 228 + wide_delta, 106,
                              selective_key_0 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Зеленый ключ
        RD_M_DrawTextSmallRUS(selective_key_1 ? "LF" : "YTN", 228 + wide_delta, 116,
                              selective_key_1 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Синий ключ
        RD_M_DrawTextSmallRUS(selective_key_2 ? "LF" : "YTN", 228 + wide_delta, 126,
                              selective_key_2 ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Ускоренные
        RD_M_DrawTextSmallRUS(selective_fast ? "LF" : "YTN", 228 + wide_delta, 146,
                              selective_fast ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);

        // Воскрешающиеся
        RD_M_DrawTextSmallRUS(selective_respawn ? "LF" : "YTN", 228 + wide_delta, 156,
                              selective_respawn ? CR_WHITE2GREEN_HERETIC : CR_WHITE2RED_HERETIC);
    }

    // Wand Crystals | Кристаллы для жезла
    M_snprintf(num, 4, "%d", selective_ammo_0);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 36,
                          selective_ammo_0 >= 50 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_0 >= 25 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Ethereal Arrows | Эфирные стрелы
    M_snprintf(num, 4, "%d", selective_ammo_1);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 46,
                          selective_ammo_1 >= 25 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_1 >= 12 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Claw Orbs | Когтевые шары
    M_snprintf(num, 4, "%d", selective_ammo_2);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 56,
                          selective_ammo_2 >= 100 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_2 >= 50 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Hellstaff Runes | Руны посоха
    M_snprintf(num, 4, "%d", selective_ammo_3);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 66,
                          selective_ammo_3 >= 100 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_3 >= 50 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Flame Orbs | Пламенные шары
    M_snprintf(num, 4, "%d", selective_ammo_4);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 76,
                          selective_ammo_4 >= 10 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_4 >= 5 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);

    // Mace Spheres | Сферы булавы
    M_snprintf(num, 4, "%d", selective_ammo_5);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 86,
                          selective_ammo_5 >= 75 ? CR_WHITE2GREEN_HERETIC :
                          selective_ammo_5 >= 37 ? CR_WHITE2DARKGOLD_HERETIC :
                          CR_WHITE2RED_HERETIC);
}

static void M_RD_SelectiveBag(Direction_t direction)
{
    selective_backpack ^= 1;

    if (!selective_backpack)
    {
        if (selective_ammo_0 > 100) // wand crystals
            selective_ammo_0 = 100;
        if (selective_ammo_1 > 50)  // ethereal arrows
            selective_ammo_1 = 50;
        if (selective_ammo_2 > 200) // claw orbs
            selective_ammo_2 = 200;
        if (selective_ammo_3 > 200) // hellstaff runes
            selective_ammo_3 = 200;
        if (selective_ammo_4 > 20)  // flame orbs
            selective_ammo_4 = 20;
        if (selective_ammo_5 > 150) // mace spheres
            selective_ammo_5 = 150;
    }
}

static void M_RD_SelectiveAmmo_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_0, 0, selective_backpack ? 200 : 100, direction);
}

static void M_RD_SelectiveAmmo_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_1, 0, selective_backpack ? 100 : 50, direction);
}

static void M_RD_SelectiveAmmo_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_2, 0, selective_backpack ? 400 : 200, direction);
}

static void M_RD_SelectiveAmmo_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_3, 0, selective_backpack ? 400 : 200, direction);
}

static void M_RD_SelectiveAmmo_4(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_4, 0, selective_backpack ? 40 : 20, direction);
}

static void M_RD_SelectiveAmmo_5(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_5, 0, selective_backpack ? 300 : 150, direction);
}

static void M_RD_SelectiveKey_0(Direction_t direction)
{
    selective_key_0 ^= 1;
}

static void M_RD_SelectiveKey_1(Direction_t direction)
{
    selective_key_1 ^= 1;
}

static void M_RD_SelectiveKey_2(Direction_t direction)
{
    selective_key_2 ^= 1;
}

static void M_RD_SelectiveFast(Direction_t direction)
{
    selective_fast ^= 1;
}

static void M_RD_SelectiveRespawn(Direction_t direction)
{
    selective_respawn ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect3Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect3Menu(void)
{
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    // Quartz Flask
    M_snprintf(num, 4, "%d", selective_arti_0);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 36, selective_arti_0 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Mystic Urn
    M_snprintf(num, 4, "%d", selective_arti_1);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 46, selective_arti_1 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Timebomb
    M_snprintf(num, 4, "%d", selective_arti_2);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 56, selective_arti_2 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Tome of Power
    M_snprintf(num, 4, "%d", selective_arti_3);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 66, selective_arti_3 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Ring of Invincibility
    M_snprintf(num, 4, "%d", selective_arti_4);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 76, selective_arti_4 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Morph Ovum
    M_snprintf(num, 4, "%d", selective_arti_5);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 86, selective_arti_5 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Chaos Device
    M_snprintf(num, 4, "%d", selective_arti_6);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 96, selective_arti_6 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Shadowsphere
    M_snprintf(num, 4, "%d", selective_arti_7);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 106, selective_arti_7 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Wings of Wrath
    M_snprintf(num, 4, "%d", selective_arti_8);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 116, selective_arti_8 ? CR_NONE : CR_WHITE2GRAY_HERETIC);

    // Torch
    M_snprintf(num, 4, "%d", selective_arti_9);
    RD_M_DrawTextSmallENG(num, 228 + wide_delta, 126, selective_arti_9 ? CR_NONE : CR_WHITE2GRAY_HERETIC);
}

static void M_RD_SelectiveArti_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_0, 0, 16, direction);
}

static void M_RD_SelectiveArti_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_1, 0, 16, direction);
}

static void M_RD_SelectiveArti_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_2, 0, 16, direction);
}

static void M_RD_SelectiveArti_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_3, 0, 16, direction);
}

static void M_RD_SelectiveArti_4(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_4, 0, 16, direction);
}

static void M_RD_SelectiveArti_5(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_5, 0, 16, direction);
}

static void M_RD_SelectiveArti_6(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_6, 0, 16, direction);
}

static void M_RD_SelectiveArti_7(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_7, 0, 16, direction);
}

static void M_RD_SelectiveArti_8(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_8, 0, 16, direction);
}

static void M_RD_SelectiveArti_9(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_arti_9, 0, 16, direction);
}

//---------------------------------------------------------------------------
// [JN] Vanilla Options menu 1 and 2
//---------------------------------------------------------------------------

static void DrawOptionsMenu_Vanilla(void)
{
    if (english_language)
    {
        RD_M_DrawTextB(show_messages ? "ON" : "OFF", 196 + wide_delta, 50);
    }
    else
    {
        RD_M_DrawTextBigRUS(show_messages ? "DRK" : "DSRK", 223 + wide_delta, 50);
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
// M_RD_EndGame
//---------------------------------------------------------------------------

static void M_RD_EndGame(int option)
{
    if (demoplayback || netgame)
    {
        return;
    }

    menuactive = false;
    askforquit = true;
    typeofask = 2;              //endgame

    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

//---------------------------------------------------------------------------
// M_RD_ResetSettings
//---------------------------------------------------------------------------

static void M_RD_ResetSettings(int option)
{
    menuactive = false;
    askforquit = true;
    typeofask = 5;              // Reset settings to their defaults

    if (!netgame && !demoplayback)
    {
        paused = true;
    }
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
    messages_alignment  = 0;
    messages_timeout    = 4;
    message_fade        = 1;
    draw_shadowed_text  = 1;

    // Messages
    message_pickup_color = 0;
    message_secret_color = 15;
    message_system_color = 0;
    message_chat_color = 5;

    // Automap
    automap_overlay    = 0;
    automap_rotate     = 0;
    automap_follow     = 1;
    automap_grid       = 0;
    automap_grid_size  = 128;    
    automap_stats      = 1;
    automap_level_time = 1;
    automap_total_time = 0;
    automap_coords     = 0;

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
    sbar_colored_gem     = 0;
    colored_blood        = 1;
    swirling_liquids     = 1;
    invul_sky            = 1;
    linear_sky            = 1;
    torque               = 1;
    weapon_bobbing       = 1;
    randomly_flipcorpses = 1;
    ammo_widget          = 0;
    ammo_widget_colored  = 1;

    // Gameplay (2)
    z_axis_sfx           = 0;
    noise_alert_sfx      = 0;
    secret_notification  = 1;
    show_all_artifacts   = 0;
    show_artifacts_timer = 0;
    negative_health      = 0;
    crosshair_draw       = 0;
    crosshair_type       = 1;
    crosshair_scale      = 0;
    fix_map_errors       = 1;
    flip_levels          = 0;
    no_internal_demos    = 0;
    pistol_start         = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();
    R_SetViewSize(screenblocks, detailLevel);

    BorderNeedRefresh = true;

    P_SetMessage(&players[consoleplayer], txt_settings_reset, msg_system, false);
    S_StartSound(NULL, sfx_dorcls);
    menuactive = true;
}

//---------------------------------------------------------------------------
// M_RD_ChangeLanguage
//---------------------------------------------------------------------------

static void M_RD_ChangeLanguage(Direction_t direction)
{
    extern void D_DoAdvanceDemo(void);
    extern int demosequence;

    english_language ^= 1;
    
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

    // Restart finale text
    if (gamestate == GS_FINALE)
    {
        F_StartFinale();
    }
}

//---------------------------------------------------------------------------
//
// PROC SCNetCheck
//
//---------------------------------------------------------------------------

boolean SCNetCheck(int option)
{
    if (!netgame)
    {                           // okay to go into the menu
        return true;
    }
    switch (option)
    {
        case 1:
            P_SetMessage(&players[consoleplayer], txt_cant_start_in_netgame, msg_system, true);
            break;
        case 2:
            P_SetMessage(&players[consoleplayer], txt_cant_load_in_netgame, msg_system, true);
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
// PROC SCMessages
//
//---------------------------------------------------------------------------

static void SCMessages(int option)
{
    show_messages ^= 1;

    if (show_messages)
    {
        P_SetMessage(&players[consoleplayer], txt_messages_on, msg_system, true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], txt_messages_off, msg_system, true);
    }

    S_StartSound(NULL, sfx_chat);
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void SCLoadGame(int option)
{
    char *filename;

    if (!SlotStatus[option])
    {
        return; // slot's empty...don't try and load
    }

    filename = SV_Filename((int) option);
    G_LoadGame(filename);
    free(filename);

    MN_DeactivateMenu();
    BorderNeedRefresh = true;

    if (quickload == -1)
    {
        quickload = ((int) option) + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
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
        y = SaveMenu.y + 1 + ((int) option) * ITEM_HEIGHT;
        I_StartTextInput(x, y, x + 190, y + ITEM_HEIGHT - 2);

        M_StringCopy(oldSlotText, SlotText[(int) option], sizeof(oldSlotText));
        ptr = SlotText[option];
        while (*ptr)
        {
            ptr++;
        }
        *ptr = '_';
        *(ptr + 1) = 0;
        SlotStatus[option]++;
        currentSlot = (int) option;
        slotptr = ptr - SlotText[(int) option];
        return;
    }
    else
    {
        G_SaveGame((int) option, SlotText[option]);
        FileMenuKeySteal = false;
        I_StopTextInput();
        MN_DeactivateMenu();
    }
    BorderNeedRefresh = true;
    if (quicksave == -1)
    {
        quicksave = ((int) option) + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCEpisode
//
//---------------------------------------------------------------------------

static void SCEpisode(int option)
{
    if (gamemode == shareware && option > 1)
    {
        P_SetMessage(&players[consoleplayer], txt_registered_only, msg_system, true);
    }
    else
    {
        MenuEpisode = (int) option;
        RD_Menu_SetMenu(&SkillMenu);
    }
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static void SCSkill(int option)
{
    G_DeferedInitNew(option, MenuEpisode, 1);
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
    S_StartSound(NULL, sfx_dorcls);

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
    int key;
    int mousewait = 0;
    MenuItem_t *item;
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

    // [JN] Initialize events.
    charTyped = 0;
    key = -1;

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
    // [JN] Support for mouse controls.
    else
    {
        if (event->type == ev_mouse && mousewait < I_GetTime())
        {
            // [JN] Catch all incoming data1 mouse events. Makes middle mouse button
            // working for message interruption and for binding ability.
            if (event->data1)
            {
                key = event->data1;
                mousewait = I_GetTime() + 5;
            }

            // [JN] Do not read mouse events while typing and "typeofask" events.
            if (!FileMenuKeySteal)
            {
                if (event->data1&1)
                {
                    key = key_menu_forward;
                    mousewait = I_GetTime() + 15;
                }
                if (event->data1&2)
                {
                    key = key_menu_back;
                    mousewait = I_GetTime() + 15;
                }
            }
            // [crispy] scroll menus with mouse wheel
            if (mousebprevweapon >= 0 && event->data1 & (1 << mousebprevweapon))
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 1;
            }
            else
            if (mousebnextweapon >= 0 && event->data1 & (1 << mousebnextweapon))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 1;
            }
        }
        else
        {
            if (event->type == ev_keydown)
            {
                key = event->data1;
                charTyped = event->data2;
            }
        }
    }

    if (key == -1)
    {
        return false;
    }

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
                    P_SetMessage(&players[consoleplayer], txt_quicksaving, msg_system, false);
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                    break;

                case 4:
                    P_SetMessage(&players[consoleplayer], txt_quickloading, msg_system, false);
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
                CurrentItPos = CurrentMenu->lastOn;
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
                CurrentItPos = CurrentMenu->lastOn;
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
            sfxbgdraw = false; // [JN] Don't draw menu background.
            // [JN] Force to use vanilla options 2 menu in -vanilla game mode.
            if (vanillaparm)
            {
                CurrentMenu = &VanillaOptions2Menu;
            }
            else
            {
                CurrentMenu = &SoundMenu;
            }
            CurrentItPos = CurrentMenu->lastOn;
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
            // [JN] Restored variable detail mode.
            M_RD_Detail(0);
            S_StartSound(NULL, sfx_chat);
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
                    CurrentItPos = CurrentMenu->lastOn;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_dorcls);
                    slottextloaded = false; //reload the slot text, when needed
                    quicksave = -1;
                    P_SetMessage(&players[consoleplayer], txt_choose_qsave, msg_system, true);
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
                CurrentItPos = CurrentMenu->lastOn;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
                quickload = -1;
                P_SetMessage(&players[consoleplayer], txt_choose_qload, msg_system, true);
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
            // [JN] Allow to invoke quit responce in any game states.
            SCQuitGame(0);
            S_StartSound(NULL, sfx_chat);
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
                                                  msg_system, false);
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
        CurrentMenu->lastOn = CurrentItPos;
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
            default:
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

void RD_Menu_StartSound(MenuSound_t sound)
{
    switch (sound)
    {
        case MENU_SOUND_CURSOR_MOVE:
        case MENU_SOUND_BACK:
            S_StartSound(NULL, sfx_switch);
            break;
        case MENU_SOUND_SLIDER_MOVE:
            S_StartSound(NULL, sfx_keyup);
            break;
        case MENU_SOUND_CLICK:
        case MENU_SOUND_ACTIVATE:
        case MENU_SOUND_DEACTIVATE:
        case MENU_SOUND_PAGE:
            S_StartSound(NULL, sfx_dorcls);
            break;
        default:
            break;
    }
}

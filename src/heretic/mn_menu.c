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

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN     22
#define ASCII_CURSOR '_'

// [JN] Sizes of small font and small arrow for RD menu
#define ITEM_HEIGHT_SMALL 10
#define SELECTOR_XOFFSET_SMALL (-14)

// Private Functions

static void InitFonts(void);
static void SetMenu(const Menu_t* menu);
static void SCNetCheck_NG_LG(Menu_t* menu);
static boolean SCNetCheck(int option);
static void SCQuitGame(intptr_t option);
static void SCEpisode(intptr_t option);
static void SCSkill(intptr_t option);
static void SCLoadGame(intptr_t option);
static void SCSaveGame(intptr_t option);
static void SCMessages(intptr_t option);
static void SCInfo(intptr_t option);
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
static void DrawSliderSmall(Menu_t * menu, int y, int width, int slot);
void MN_LoadSlotText(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static void M_RD_Change_Widescreen(intptr_t option);
static void M_RD_Change_VSync(intptr_t option);
static void M_RD_Uncapped(intptr_t option);
static void M_RD_FPScounter(intptr_t option);
static void M_RD_Smoothing(intptr_t option);
static void M_RD_PorchFlashing(intptr_t option);
static void M_RD_Renderer(intptr_t option);
static void M_RD_Screenshots(intptr_t option);
static void M_RD_EndText(intptr_t option);

// Display
static void DrawDisplayMenu(void);
static void M_RD_ScreenSize(intptr_t option);
static void M_RD_Gamma(intptr_t option);
static void M_RD_LevelBrightness(intptr_t option);
static void M_RD_Detail(intptr_t option);

// Messages and Texts
static void DrawMessagesMenu(void);
static void M_RD_Messages(intptr_t option);
static void M_RD_MessagesAlignment(intptr_t option);
static void M_RD_MessagesTimeout(intptr_t option);
static void M_RD_MessagesFade(intptr_t option);
static void M_RD_ShadowedText(intptr_t option);
static void M_RD_LocalTime(intptr_t option);
static void M_RD_Define_Msg_Pickup_Color(void);
static void M_RD_Change_Msg_Pickup_Color(intptr_t option);
static void M_RD_Define_Msg_Secret_Color(void);
static void M_RD_Change_Msg_Secret_Color(intptr_t option);
static void M_RD_Define_Msg_System_Color(void);
static void M_RD_Change_Msg_System_Color(intptr_t option);
static void M_RD_Define_Msg_Chat_Color(void);
static void M_RD_Change_Msg_Chat_Color(intptr_t option);

// Automap
static void DrawAutomapMenu(void);
static void M_RD_AutoMapOverlay(intptr_t option);
static void M_RD_AutoMapRotate(intptr_t option);
static void M_RD_AutoMapFollow(intptr_t option);
static void M_RD_AutoMapGrid(intptr_t option);
static void M_RD_AutoMapGridSize(intptr_t option);
static void M_RD_AutoMapStats(intptr_t option);
static void M_RD_AutoMapLevTime(intptr_t option);
static void M_RD_AutoMapTotTime(intptr_t option);
static void M_RD_AutoMapCoords(intptr_t option);

// Sound
static void DrawSoundMenu(void);
static void M_RD_SfxVolume(intptr_t option);
static void M_RD_MusVolume(intptr_t option);
static void M_RD_SfxChannels(intptr_t option);

// Sound system
static void DrawSoundSystemMenu(void);
static void M_RD_SoundDevice(intptr_t option);
static void M_RD_MusicDevice(intptr_t option);
static void M_RD_Sampling(intptr_t option);
static void M_RD_SndMode(intptr_t option);
static void M_RD_PitchShifting(intptr_t option);
static void M_RD_MuteInactive(intptr_t option);

// Controls
static void DrawControlsMenu(void);
static void M_RD_AlwaysRun(intptr_t option);
static void M_RD_Sensitivity(intptr_t option);
static void M_RD_Acceleration(intptr_t option);
static void M_RD_Threshold(intptr_t option);
static void M_RD_MouseLook(intptr_t option);
static void M_RD_InvertY(intptr_t option);
static void M_RD_Novert(intptr_t option);

// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static void M_RD_Brightmaps(intptr_t option);
static void M_RD_FakeContrast(intptr_t option);
static void M_RD_ExtraTrans(intptr_t option);
static void M_RD_ColoredBlood(intptr_t option);
static void M_RD_SwirlingLiquids(intptr_t option);
static void M_RD_InvulSky(intptr_t option);
static void M_RD_LinearSky(intptr_t option);
static void M_RD_Torque(intptr_t option);
static void M_RD_Bobbing(intptr_t option);
static void M_RD_FlipCorpses(intptr_t option);
static void M_RD_FloatAmplitude(intptr_t option);

// Gameplay (page 2)
static void DrawGameplay2Menu(void);
static void M_RD_ColoredSBar(intptr_t option);
static void M_RD_ColoredGem(intptr_t option);
static void M_RD_NegativeHealth(intptr_t option);
static void M_RD_AmmoWidgetDraw(intptr_t option);
static void M_RD_AmmoWidgetColoring(intptr_t option);
static void M_RD_CrossHairDraw(intptr_t option);
static void M_RD_CrossHairType(intptr_t option);
static void M_RD_CrossHairScale(intptr_t option);

// Gameplay (page 3)
static void DrawGameplay3Menu(void);
static void M_RD_ZAxisSFX(intptr_t option);
static void M_RD_AlertSFX(intptr_t option);
static void M_RD_SecretNotify(intptr_t option);
static void M_RD_ShowAllArti(intptr_t option);
static void M_RD_ShowArtiTimer(intptr_t option);
static void M_RD_FixMapErrors(intptr_t option);
static void M_RD_FlipLevels(intptr_t option);
static void M_RD_NoDemos(intptr_t option);
static void M_RD_WandStart(intptr_t option);

// Level Select (page 1)
static void DrawLevelSelect1Menu(void);
static void M_RD_SelectiveSkill(intptr_t option);
static void M_RD_SelectiveEpisode(intptr_t option);
static void M_RD_SelectiveMap(intptr_t option);
static void M_RD_SelectiveHealth(intptr_t option);
static void M_RD_SelectiveArmor(intptr_t option);
static void M_RD_SelectiveArmorType(intptr_t option);
static void M_RD_SelectiveGauntlets(intptr_t option);
static void M_RD_SelectiveCrossbow(intptr_t option);
static void M_RD_SelectiveDragonClaw(intptr_t option);
static void M_RD_SelectiveHellStaff(intptr_t option);
static void M_RD_SelectivePhoenixRod(intptr_t option);
static void M_RD_SelectiveFireMace(intptr_t option);

// Level Select (page 2)
static void DrawLevelSelect2Menu(void);
static void M_RD_SelectiveBag(intptr_t option);
static void M_RD_SelectiveAmmo_0(intptr_t option);
static void M_RD_SelectiveAmmo_1(intptr_t option);
static void M_RD_SelectiveAmmo_2(intptr_t option);
static void M_RD_SelectiveAmmo_3(intptr_t option);
static void M_RD_SelectiveAmmo_4(intptr_t option);
static void M_RD_SelectiveAmmo_5(intptr_t option);
static void M_RD_SelectiveKey_0(intptr_t option);
static void M_RD_SelectiveKey_1(intptr_t option);
static void M_RD_SelectiveKey_2(intptr_t option);
static void M_RD_SelectiveFast(intptr_t option);
static void M_RD_SelectiveRespawn(intptr_t option);

// Level Select (page 3)
static void DrawLevelSelect3Menu(void);
static void M_RD_SelectiveArti_0(intptr_t option);
static void M_RD_SelectiveArti_1(intptr_t option);
static void M_RD_SelectiveArti_2(intptr_t option);
static void M_RD_SelectiveArti_3(intptr_t option);
static void M_RD_SelectiveArti_4(intptr_t option);
static void M_RD_SelectiveArti_5(intptr_t option);
static void M_RD_SelectiveArti_6(intptr_t option);
static void M_RD_SelectiveArti_7(intptr_t option);
static void M_RD_SelectiveArti_8(intptr_t option);
static void M_RD_SelectiveArti_9(intptr_t option);

// Vanilla Options menu
static void DrawOptionsMenu_Vanilla(void);
static void DrawOptions2Menu_Vanilla(void);

// End game
static void M_RD_EndGame(intptr_t option);

// Reset settings
static void M_RD_ResetSettings(intptr_t option);

// Change language
static void M_RD_ChangeLanguage(intptr_t option);

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

// [JN] Choosen message colors
byte *messages_pickup_color_set;
byte *messages_secret_color_set;
byte *messages_system_color_set;
byte *messages_chat_color_set;

static byte *M_RD_ColorTranslation (int color)
{
    switch (color)
    {
        case 1:   return cr[CR_WHITE2GRAY_HERETIC];      break;
        case 2:   return cr[CR_WHITE2DARKGRAY_HERETIC];  break;
        case 3:   return cr[CR_WHITE2RED_HERETIC];       break;
        case 4:   return cr[CR_WHITE2DARKRED_HERETIC];   break;
        case 5:   return cr[CR_WHITE2GREEN_HERETIC];     break;
        case 6:   return cr[CR_WHITE2DARKGREEN_HERETIC]; break;
        case 7:   return cr[CR_WHITE2OLIVE_HERETIC];     break;
        case 8:   return cr[CR_WHITE2BLUE_HERETIC];      break;
        case 9:   return cr[CR_WHITE2DARKBLUE_HERETIC];  break;
        case 10:  return cr[CR_WHITE2PURPLE_HERETIC];    break;
        case 11:  return cr[CR_WHITE2NIAGARA_HERETIC];   break;
        case 12:  return cr[CR_WHITE2AZURE_HERETIC];     break;
        case 13:  return cr[CR_WHITE2YELLOW_HERETIC];    break;
        case 14:  return cr[CR_WHITE2GOLD_HERETIC];      break;
        case 15:  return cr[CR_WHITE2DARKGOLD_HERETIC];  break;
        case 16:  return cr[CR_WHITE2TAN_HERETIC];       break;
        case 17:  return cr[CR_WHITE2BROWN_HERETIC];     break;
        default:  return NULL;                           break;
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
static Menu_t EpisodeMenu;
static Menu_t OptionsMenu;
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
static Menu_t LevelSelectMenu1;
static Menu_t LevelSelectMenu2;
static Menu_t LevelSelectMenu3;
static Menu_t Options2Menu_Vanilla;
static Menu_t FilesMenu;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

static MenuItem_t MainItems[] = {
    {ITT_EFUNC,   "NEW GAME",   "YJDFZ BUHF", (void (*)(intptr_t)) SCNetCheck_NG_LG, (const intptr_t) &EpisodeMenu}, // НОВАЯ ИГРА
    {ITT_SETMENU, "OPTIONS",    "YFCNHJQRB",  NULL,       (const intptr_t) &OptionsMenu}, // НАСТРОЙКИ
    {ITT_SETMENU, "GAME FILES", "AFQKS BUHS", NULL,       (const intptr_t) &FilesMenu}, // ФАЙЛЫ ИГРЫ
    {ITT_EFUNC,   "INFO",       "BYAJHVFWBZ", SCInfo,     0}, // ИНФОРМАЦИЯ
    {ITT_EFUNC,   "QUIT GAME",  "DS[JL",      SCQuitGame, 0} // ВЫХОД
};

static Menu_t MainMenu = {
    110, 56,
    103,
    5, MainItems,
    DrawMainMenu,
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

static Menu_t EpisodeMenu = {
    80, 50,
    55,
    3, EpisodeItems,
    DrawEpisodeMenu,
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
    38, 30,
    38,
    6, SkillItems,
    DrawSkillMenu,
    &EpisodeMenu,
    2
};

// -----------------------------------------------------------------------------
// [JN] Custom options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "RENDERING",         "DBLTJ",          NULL, (const intptr_t) &RenderingMenu}, // ВИДЕО
    {ITT_SETMENU, "DISPLAY",           "\'RHFY",         NULL, (const intptr_t) &DisplayMenu}, // ЭКРАН
    {ITT_SETMENU, "SOUND",             "FELBJ",          NULL, (const intptr_t) &SoundMenu}, // АУДИО
    {ITT_SETMENU, "CONTROLS",          "EGHFDKTYBT",     NULL, (const intptr_t) &ControlsMenu}, // УПРАВЛЕНИЕ
    {ITT_SETMENU, "GAMEPLAY",          "UTQVGKTQ",       NULL, (const intptr_t) &Gameplay1Menu}, // ГЕЙМПЛЕЙ
    {ITT_SETMENU, "LEVEL SELECT",      "DS,JH EHJDYZ",   NULL, (const intptr_t) &LevelSelectMenu1}, // ВЫБОР УРОВНЯ
    {ITT_EFUNC,   "RESET SETTINGS",    "C,HJC YFCNHJTR", M_RD_ResetSettings,  0}, // СБРОС НАСТРОЕК
    {ITT_LRFUNC,  "LANGUAGE: ENGLISH", "ZPSR: HECCRBQ",  M_RD_ChangeLanguage, 0}  // ЯЗЫК: РУССКИЙ
};

static Menu_t OptionsMenu = {
    81, 31,
    81,
    8, OptionsItems,
    DrawOptionsMenu,
    &MainMenu,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_LRFUNC, "DISPLAY ASPECT RATIO:",     "CJJNYJITYBT CNJHJY \'RHFYF:",     M_RD_Change_Widescreen, 0}, // СООТНОШЕНИЕ СТОРОН ЭКРАНА
    {ITT_LRFUNC, "VERTICAL SYNCHRONIZATION:", "DTHNBRFKMYFZ CBY[HJYBPFWBZ:",     M_RD_Change_VSync,      0}, // ВЕРТИКАЛЬНАЯ СИНХРОНИЗАЦИЯ
    {ITT_LRFUNC, "FRAME RATE:",               "RFLHJDFZ XFCNJNF:",               M_RD_Uncapped,          0}, // КАДРОВАЯ ЧАСТОТА
    {ITT_LRFUNC, "FPS COUNTER:",              "CXTNXBR RFLHJDJQ XFCNJNS:",       M_RD_FPScounter,        0}, // СЧЕТЧИК КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "PIXEL SCALING:",            "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing,         0}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_LRFUNC, "PORCH PALETTE CHANGING:",   "BPVTYTYBT GFKBNHS RHFTD 'RHFYF:", M_RD_PorchFlashing,     0}, // ИЗМЕНЕНИЕ ПАЛИТРЫ КРАЕВ ЭКРАНА
    {ITT_LRFUNC, "VIDEO RENDERER:",           "J,HF,JNRF DBLTJ:",                M_RD_Renderer,          0}, // ОБРАБОТКА ВИДЕО
    {ITT_EMPTY,  NULL,                        NULL,                              NULL,                   0},
    {ITT_LRFUNC, "SCREENSHOT FORMAT:",        "AJHVFN CRHBYIJNJD:",              M_RD_Screenshots,       0}, // ФОРМАТ СКРИНШОТОВ
    {ITT_LRFUNC, "SHOW ENDTEXT SCREEN:",      "GJRFPSDFNM \'RHFY",               M_RD_EndText,           0}  // ПОКАЗЫВАТЬ ЭКРАН ENDTEXT
};

static Menu_t RenderingMenu = {
    36, 42,
    36,
    10, RenderingItems,
    DrawRenderingMenu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_LRFUNC,  "SCREEN SIZE",         "HFPVTH BUHJDJUJ \'RHFYF",  M_RD_ScreenSize,      0}, // РАЗМЕР ИГРОВОГО ЭКРАНА
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC,  "GAMMA-CORRECTION",    "EHJDTYM UFVVF-RJHHTRWBB",  M_RD_Gamma,           0}, // УРОВЕНЬ ГАММА-КОРРЕКЦИИ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC,  "LEVEL BRIGHTNESS",    "EHJDTYM JCDTOTYYJCNB",     M_RD_LevelBrightness, 0}, // УРОВЕНЬ ОСВЕЩЕННОСТИ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                 0},
    {ITT_LRFUNC, "GRAPHICS DETAIL:",     "LTNFKBPFWBZ UHFABRB:",     M_RD_Detail,          0}, // ДЕТАЛИЗАЦИЯ ГРАФИКИ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                 0},
    {ITT_SETMENU, "MESSAGES AND TEXTS...", "CJJ,OTYBZ B NTRCNS>>>",     NULL, (const intptr_t) &MessagesMenu}, // СООБЩЕНИЯ И ТЕКСТЫ...
    {ITT_SETMENU, "AUTOMAP AND STATISTICS...", "RFHNF B CNFNBCNBRF>>>", NULL, (const intptr_t) &AutomapMenu}   // КАРТА И СТАТИСТИКА...
};

static Menu_t DisplayMenu = {
    36, 42,
    36,
    10, DisplayItems,
    DrawDisplayMenu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

static MenuItem_t MessagesItems[] = {
    {ITT_LRFUNC,  "MESSAGES:",           "JNJ,HF;TYBT CJJ,OTYBQ:",   M_RD_Messages,                0}, // ОТОБРАЖЕНИЕ СООБЩЕНИЙ
    {ITT_LRFUNC,  "ALIGNMENT:",          "DSHFDYBDFYBT:",            M_RD_MessagesAlignment,       0}, // ВЫРАВНИВАНИЕ
    {ITT_LRFUNC,  "MESSAGE TIMEOUT",     "NFQVFEN JNJ,HF;TYBZ",      M_RD_MessagesTimeout,         0}, // ТАЙМАУТ ОТОБРАЖЕНИЯ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                         0},
    {ITT_LRFUNC,  "FADING EFFECT:",      "GKFDYJT BCXTPYJDTYBT:",    M_RD_MessagesFade,            0}, // ПЛАВНОЕ ИСЧЕЗНОВЕНИЕ
    {ITT_LRFUNC,  "TEXT CASTS SHADOWS:", "NTRCNS JN,HFCSDF.N NTYM:", M_RD_ShadowedText,            0}, // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                         0},
    {ITT_LRFUNC,  "LOCAL TIME:",         "CBCNTVYJT DHTVZ:",         M_RD_LocalTime,               0}, // СИСТЕМНОЕ ВРЕМЯ
    {ITT_EMPTY,   NULL,                  NULL,                       NULL,                         0},
    {ITT_LRFUNC,  "ITEM PICKUP:",        "GJKEXTYBT GHTLVTNJD:",     M_RD_Change_Msg_Pickup_Color, 0}, // ПОЛУЧЕНИЕ ПРЕДМЕТОВ
    {ITT_LRFUNC,  "REVEALED SECRET:",    "J,YFHE;TYBT NFQYBRJD:",    M_RD_Change_Msg_Secret_Color, 0}, // ОБНАРУЖЕНИЕ ТАЙНИКОВ
    {ITT_LRFUNC,  "SYSTEM MESSAGE:",     "CBCNTVYST CJJ,OTYBZ:",     M_RD_Change_Msg_System_Color, 0}, // СИСТЕМНЫЕ СООБЩЕНИЯ
    {ITT_LRFUNC,  "NETGAME CHAT:",       "XFN CTNTDJQ BUHS:",        M_RD_Change_Msg_Chat_Color,   0}  // ЧАТ СЕТЕВОЙ ИГРЫ
};

static Menu_t MessagesMenu = {
    36, 42,
    36,
    13, MessagesItems,
    DrawMessagesMenu,
    &DisplayMenu,
    0
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_LRFUNC, "OVERLAY MODE:", "HT;BV YFKJ;TYBZ:",   M_RD_AutoMapOverlay, 0}, // РЕЖИМ НАЛОЖЕНИЯ
    {ITT_LRFUNC, "ROTATE MODE:",  "HT;BV DHFOTYBZ:",    M_RD_AutoMapRotate,  0}, // РЕЖИМ ВРАЩЕНИЯ
    {ITT_LRFUNC, "FOLLOW MODE:",  "HT;BV CKTLJDFYBZ:",  M_RD_AutoMapFollow,  0}, // РЕЖИМ СЛЕДОВАНИЯ
    {ITT_LRFUNC, "GRID:",         "CTNRF:",             M_RD_AutoMapGrid,    0}, // СЕТКА
    {ITT_LRFUNC, "GRID SIZE:",    "HFPVTH CTNRB:",      M_RD_AutoMapGridSize,0}, // РАЗМЕР СЕТКИ
    {ITT_EMPTY,   NULL,           NULL,                 NULL,                0},
    {ITT_LRFUNC, "LEVEL STATS:",  "CNFNBCNBRF EHJDYZ:", M_RD_AutoMapStats,   0}, // СТАТИСТИКА УРОВНЯ
    {ITT_LRFUNC, "LEVEL TIME:",   "DHTVZ EHJDYZ:",      M_RD_AutoMapLevTime, 0}, // ВРЕМЯ УРОВНЯ
    {ITT_LRFUNC, "TOTAL TIME:",   "J,OTT EHJDYZ:",      M_RD_AutoMapTotTime, 0}, // ОБЩЕЕ ВРЕМЯ
    {ITT_LRFUNC, "PLAYER COORDS:","RJJHLBYFNS BUHJRF:", M_RD_AutoMapCoords,  0}  // КООРДИНАТЫ ИГРОКА
};

static Menu_t AutomapMenu = {
    78, 42,
    61,
    10, AutomapItems,
    DrawAutomapMenu,
    &DisplayMenu,
    0
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_LRFUNC, "SFX VOLUME",               "UHJVRJCNM PDERF",               M_RD_SfxVolume,   0}, // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_LRFUNC, "MUSIC VOLUME",             "UHJVRJCNM VEPSRB",              M_RD_MusVolume,   0}, // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_LRFUNC, "SFX CHANNELS",             "PDERJDST RFYFKS",               M_RD_SfxChannels, 0}, // ЗВУКОВЫЕ КАНАЛЫ
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_EMPTY,  NULL,                       NULL,                            NULL,             0},
    {ITT_SETMENU,"SOUND SYSTEM SETTINGS...", "YFCNHJQRB PDERJDJQ CBCNTVS>>>", NULL, (const intptr_t) &SoundSysMenu}  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ
};

static Menu_t SoundMenu = {
    36, 42,
    36,
    9, SoundItems,
    DrawSoundMenu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    {ITT_LRFUNC, "SOUND EFFECTS:",        "PDERJDST \'AATRNS:",         M_RD_SoundDevice,   0}, // ЗВУКОВЫЕ ЭФФЕКТЫ:
    {ITT_LRFUNC, "MUSIC:",                "VEPSRF:",                    M_RD_MusicDevice,   0}, // МУЗЫКА:
    {ITT_EMPTY,  NULL,                    NULL,                         NULL,               0},
    {ITT_LRFUNC, "SAMPLING FREQUENCY:",   "XFCNJNF LBCRHTNBPFWBB:",     M_RD_Sampling,      0}, // ЧАСТОТА ДИСКРЕТИЗАЦИИ:
    {ITT_EMPTY,  NULL,                    NULL,                         NULL,               0},
    {ITT_LRFUNC, "SOUND EFFECTS MODE:",   "HT;BV PDERJDS[ \'AATRNJD:",  M_RD_SndMode,       0}, // РЕЖИМ ЗВУКОВЫХ ЭФФЕКТОВ
    {ITT_LRFUNC, "PITCH-SHIFTED SOUNDS:", "GHJBPDJKMYSQ GBNX-IBANBYU:", M_RD_PitchShifting, 0}, // ПРОИЗВОЛЬНЫЙ ПИТЧ-ШИФТИНГ
    {ITT_LRFUNC, "MUTE INACTIVE WINDOW:", "PDER D YTFRNBDYJV JRYT:",    M_RD_MuteInactive,  0}, // ЗВУК В НЕАКТИВНОМ ОКНЕ
};

static Menu_t SoundSysMenu = {
    36, 42,
    36,
    8, SoundSysItems,
    DrawSoundSystemMenu,
    &SoundMenu,
    0
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_LRFUNC, "ALWAYS RUN:",            "HT;BV GJCNJZYYJUJ ,TUF:",   M_RD_AlwaysRun,    0}, // РЕЖИМ ПОСТОЯННОГО БЕГА
    {ITT_EMPTY,  NULL,                     NULL,                        NULL,              0},
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
    36, 42,
    36,
    11, ControlsItems,
    DrawControlsMenu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay1Items[] = {
    {ITT_LRFUNC, "BRIGHTMAPS:",                  ",HFQNVFGGBYU:",                  M_RD_Brightmaps,     0}, // БРАЙТМАППИНГ
    {ITT_LRFUNC, "FAKE CONTRAST:",               "BVBNFWBZ RJYNHFCNYJCNB:",        M_RD_FakeContrast,   0}, // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_LRFUNC, "EXTRA TRANSLUCENCY:",          "LJGJKYBNTKMYFZ GHJPHFXYJCNM:",   M_RD_ExtraTrans,     0}, // ДОПОЛНИТЕЛЬНАЯ ПРОЗРАЧНОСТЬ
    {ITT_LRFUNC, "COLORED BLOOD:",               "HFPYJWDTNYFZ RHJDM:",            M_RD_ColoredBlood,   0}, // РАЗНОЦВЕТНАЯ КРОВЬ
    {ITT_LRFUNC, "SWIRLING LIQUIDS:",            "EKEXITYYFZ FYBVFWBZ ;BLRJCNTQ:", M_RD_SwirlingLiquids,0}, // УЛУЧШЕННАЯ АНИМАЦИЯ ЖИДКОСТЕЙ
    {ITT_LRFUNC, "INVULNERABILITY AFFECTS SKY:", "YTEZPDBVJCNM JRHFIBDFTN YT,J:",  M_RD_InvulSky,       0}, // НЕУЯЗВИМОСТЬ ОКРАШИВАЕТ НЕБО
    {ITT_LRFUNC, "SKY DRAWING MODE:",            "HT;BV JNHBCJDRB YT,F:",          M_RD_LinearSky,      0}, // РЕЖИМ ОТРИСОВКИ НЕБА
    {ITT_EMPTY,  NULL,                           NULL,                             NULL,                0},
    {ITT_LRFUNC, "CORPSES SLIDING FROM LEDGES:", "NHEGS CGJKPF.N C DJPDSITYBQ:",   M_RD_Torque,         0}, // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_LRFUNC, "WEAPON BOBBING WHILE FIRING:", "EKEXITYYJT GJRFXBDFYBT JHE;BZ:", M_RD_Bobbing,        0}, // УЛУЧШЕННОЕ ПОКАЧИВАНИЕ ОРУЖИЯ
    {ITT_LRFUNC, "RANDOMLY MIRRORED CORPSES:",   "PTHRFKBHJDFYBT NHEGJD:",         M_RD_FlipCorpses,    0}, // ЗЕКРАЛИРОВАНИЕ ТРУПОВ
    {ITT_LRFUNC, "FLOATING ITEMS AMPLITUDE:" ,   "KTDBNFWBZ GHTLVTNJD:",           M_RD_FloatAmplitude, 0}, // АМПЛИТУДА ЛЕВИТАЦИИ ПРЕДМЕТОВ
    {ITT_EMPTY,  NULL,                           NULL,                             NULL,                0},
    {ITT_SETMENU,"NEXT PAGE...",                 "CKTLE.OFZ CNHFYBWF>>>",          NULL, (const intptr_t) &Gameplay2Menu} // СЛЕДУЮЩАЯ СТРАНИЦА
};

static Menu_t Gameplay1Menu = {
    36, 36,
    36,
    14, Gameplay1Items,
    DrawGameplay1Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Gameplay features (2)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay2Items[] = {
    {ITT_LRFUNC, "COLORED STATUS BAR:",   "HFPYJWDTNYST \'KTVTYNS:",     M_RD_ColoredSBar,        0}, // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ
    {ITT_LRFUNC, "COLORED HEALTH GEM:",   "JRHFIBDFYBT RFVYZ PLJHJDMZ:", M_RD_ColoredGem,         0}, // ОКРАШИВАНИЕ КАМНЯ ЗДОРОВЬЯ
    {ITT_LRFUNC, "SHOW NEGATIVE HEALTH:", "JNHBWFNTKMYJT PLJHJDMT:",     M_RD_NegativeHealth,     0}, // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ
    {ITT_EMPTY,  NULL,                    NULL,                          NULL,                    0},
    {ITT_LRFUNC, "DRAW WIDGET:",          "JNJ,HF;FNM DBL;TN:",          M_RD_AmmoWidgetDraw,     0}, // ОТОБРАЖАТЬ ВИДЖЕТ
    {ITT_LRFUNC, "COLORING:",             "WDTNJDFZ BYLBRFWBZ:",         M_RD_AmmoWidgetColoring, 0}, // ЦВЕТОВАЯ ИНДИКАЦИЯ
    {ITT_EMPTY,  NULL,                    NULL,                          NULL,                    0},
    {ITT_LRFUNC, "DRAW CROSSHAIR:",       "JNJ,HF;FNM GHBWTK:",          M_RD_CrossHairDraw,      0}, // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_LRFUNC, "INDICATION:",           "BYLBRFWBZ:",                  M_RD_CrossHairType,      0}, // ИНДИКАЦИЯ
    {ITT_LRFUNC, "INCREASED SIZE:",       "EDTKBXTYYSQ HFPVTH:",         M_RD_CrossHairScale,     0}, // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,  NULL,                    NULL,                          NULL,                    0},
    {ITT_EMPTY,  NULL,                    NULL,                          NULL,                    0},
    {ITT_EMPTY,  NULL,                    NULL,                          NULL,                    0},
    {ITT_SETMENU,"LAST PAGE...",          "GJCKTLYZZ CNHFYBWF>>>",       NULL, (const intptr_t) &Gameplay3Menu} // ПОСЛЕДНЯЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    36, 36,
    36,
    14, Gameplay2Items,
    DrawGameplay2Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Gameplay features (3)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay3Items[] = {
    {ITT_LRFUNC, "SOUND ATTENUATION AXISES:",   "PFNE[FYBT PDERF GJ JCZV:",      M_RD_ZAxisSFX,       0}, // ЗАТУХАНИЕ ЗВУКА ПО ОСЯМ
    {ITT_LRFUNC, "MONSTER ALERT WAKING UP OTHERS:", "J,OFZ NHTDJUF E VJYCNHJD:", M_RD_AlertSFX,       0}, // ОБЩАЯ ТРЕВОГА У МОНСТРОВ
    {ITT_EMPTY,  NULL,                          NULL,                            NULL,                0},
    {ITT_LRFUNC, "NOTIFY OF REVEALED SECRETS:", "CJJ,OFNM J YFQLTYYJV NFQYBRT:", M_RD_SecretNotify,   0}, // СООБЩАТЬ О НАЙДЕННОМ ТАЙНИКЕ
    {ITT_LRFUNC, "SHOW ACTIVE ARTIFACTS:",      "BYLBRFWBZ FHNTAFRNJD:",         M_RD_ShowAllArti,    0}, // ИНДИКАЦИЯ АРТЕФАЕКТОВ
    {ITT_LRFUNC, "ARTIFACTS TIMER:",            "NFQVTH FHNTAFRNJD:",            M_RD_ShowArtiTimer,  0}, // ТАЙМЕР АРТЕФАКТОВ
    {ITT_EMPTY,  NULL,                          NULL,                            NULL,                0},
    {ITT_LRFUNC, "FIX ERRORS ON VANILLA MAPS:","ECNHFYZNM JIB,RB JHBU> EHJDYTQ:",M_RD_FixMapErrors,   0}, // УСТРАНЯТЬ ОШИБКИ ОРИГИНАЛЬНЫХ УРОВНЕЙ
    {ITT_LRFUNC, "FLIP GAME LEVELS:",           "PTHRFKMYJT JNHF;TYBT EHJDYTQ:", M_RD_FlipLevels,     0}, // ЗЕРКАЛЬНОЕ ОТРАЖЕНИЕ УРОВНЕЙ
    {ITT_LRFUNC, "PLAY INTERNAL DEMOS:",        "GHJBUHSDFNM LTVJPFGBCB:",       M_RD_NoDemos,        0}, // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_LRFUNC, "WAND START GAME MODE:",       " ",/* [JN] Joint EN/RU string*/ M_RD_WandStart,      0}, // РЕЖИМ ИГРЫ "WAND START"
    {ITT_EMPTY,  NULL,                          NULL,                            NULL,                0},
    {ITT_EMPTY,  NULL,                          NULL,                            NULL,                0},
    {ITT_SETMENU,"FIRST PAGE...",               "GTHDFZ CNHFYBWF>>>",            NULL, (const intptr_t) &Gameplay1Menu} // ПЕРВАЯ СТРАНИЦА
};

static Menu_t Gameplay3Menu = {
    36, 36,
    36,
    14, Gameplay3Items,
    DrawGameplay3Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (1)
// -----------------------------------------------------------------------------

static MenuItem_t Level1Items[] = {
    {ITT_LRFUNC, "SKILL LEVEL:",       "CKJ;YJCNM:",            M_RD_SelectiveSkill,      0}, // СЛОЖНОСТЬ
    {ITT_LRFUNC, "EPISODE:",           "\'GBPJL:",              M_RD_SelectiveEpisode,    0}, // ЭПИЗОД
    {ITT_LRFUNC, "MAP:",               "EHJDTYM:",              M_RD_SelectiveMap,        0}, // УРОВЕНЬ
    {ITT_EMPTY,  NULL,                 NULL,                    NULL,                     0},
    {ITT_LRFUNC, "HEALTH:",            "PLJHJDMT:",             M_RD_SelectiveHealth,     0}, // ЗДОРОВЬЕ
    {ITT_LRFUNC, "ARMOR:",             ",HJYZ:",                M_RD_SelectiveArmor,      0}, // БРОНЯ
    {ITT_LRFUNC, "ARMOR TYPE:",        "NBG ,HJYB:",            M_RD_SelectiveArmorType,  0}, // ТИП БРОНИ
    {ITT_EMPTY,  NULL,                 NULL,                    NULL,                     0},
    {ITT_LRFUNC, "GAUNTLETS:",         "GTHXFNRB:",             M_RD_SelectiveGauntlets,  0}, // ПЕРЧАТКИ
    {ITT_LRFUNC, "ETHEREAL CROSSBOW:", "\'ABHYSQ FH,FKTN:",     M_RD_SelectiveCrossbow,   0}, // ЭФИРНЫЙ АРБАЛЕТ
    {ITT_LRFUNC, "DRAGON CLAW:",       "RJUJNM LHFRJYF:",       M_RD_SelectiveDragonClaw, 0}, // КОГОТЬ ДРАКОНА
    {ITT_LRFUNC, "HELLSTAFF:",         "GJCJ] FLF:",            M_RD_SelectiveHellStaff,  0}, // ПОСОХ АДА
    {ITT_LRFUNC, "PHOENIX ROD:",       ";TPK ATYBRCF:",         M_RD_SelectivePhoenixRod, 0}, // ЖЕЗЛ ФЕНИКСА
    {ITT_LRFUNC, "FIREMACE:",          "JUYTYYFZ ,EKFDF:",      M_RD_SelectiveFireMace,   0}, // ОГНЕННАЯ БУЛАВА
    {ITT_EMPTY,  NULL,                 NULL,                    NULL,                     0},
    {ITT_SETMENU,"NEXT PAGE...",       "CKTLE.OFZ CNHFYBWF>>>", NULL, (const intptr_t) &LevelSelectMenu2}, // СЛЕДУЮЩАЯ СТРАНИЦА...
    {ITT_LRFUNC, "START GAME",         "YFXFNM BUHE",           G_DoSelectiveGame,        0}  // НАЧАТЬ ИГРУ
};

static Menu_t LevelSelectMenu1 = {
    74, 26,
    74,
    17, Level1Items,
    DrawLevelSelect1Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (2)
// -----------------------------------------------------------------------------

static MenuItem_t Level2Items[] = {
    {ITT_LRFUNC, "BAG OF HOLDING:",  "YJCBKMYSQ RJITKM:",     M_RD_SelectiveBag,     0},
    {ITT_LRFUNC, "WAND CRYSTALS:",   "RHBCNFKKS LKZ ;TPKF:",  M_RD_SelectiveAmmo_0,  0},
    {ITT_LRFUNC, "ETHEREAL ARROWS:", "\'ABHYST CNHTKS:",      M_RD_SelectiveAmmo_1,  0},
    {ITT_LRFUNC, "CLAW ORBS:",       "RJUNTDST IFHS:",        M_RD_SelectiveAmmo_2,  0},
    {ITT_LRFUNC, "HELLSTAFF RUNES:", "HEYS GJCJ[F:",          M_RD_SelectiveAmmo_3,  0},
    {ITT_LRFUNC, "FLAME ORBS:",      "GKFVTYYST IFHS:",       M_RD_SelectiveAmmo_4,  0},
    {ITT_LRFUNC, "MACE SPHERES:",    "CATHS ,EKFDS:",         M_RD_SelectiveAmmo_5,  0},
    {ITT_EMPTY,  NULL,               NULL,                    NULL,                  0},
    {ITT_LRFUNC, "YELLOW KEY:",      ";TKNSQ RK.X:",          M_RD_SelectiveKey_0,   0},
    {ITT_LRFUNC, "GREEN KEY:",       "PTKTYSQ RK.X:",         M_RD_SelectiveKey_1,   0},
    {ITT_LRFUNC, "BLUE KEY:",        "CBYBQ RK.X:",           M_RD_SelectiveKey_2,   0},
    {ITT_EMPTY,  NULL,               NULL,                    NULL,                  0},
    {ITT_LRFUNC, "FAST:",            "ECRJHTYYST:",           M_RD_SelectiveFast,    0},
    {ITT_LRFUNC, "RESPAWNING:",       "DJCRHTIF.OBTCZ:",      M_RD_SelectiveRespawn, 0},
    {ITT_EMPTY,  NULL,               NULL,                    NULL,                  0},
    {ITT_SETMENU,"LAST PAGE...",     "GJCKTLYZZ CNHFYBWF>>>", NULL, (const intptr_t) &LevelSelectMenu3},
    {ITT_LRFUNC, "START GAME",       "YFXFNM BUHE",           G_DoSelectiveGame,     0}
};

static Menu_t LevelSelectMenu2 = {
    74, 26,
    74,
    17, Level2Items,
    DrawLevelSelect2Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Level select (3)
// -----------------------------------------------------------------------------

static MenuItem_t Level3Items[] = {
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
    {ITT_SETMENU,"FIRST PAGE...",          "GTHDFZ CNHFYBWF>>>",   NULL, (const intptr_t) &LevelSelectMenu1},
    {ITT_LRFUNC, "START GAME",             "YFXFNM BUHE",          G_DoSelectiveGame,    0}
};

static Menu_t LevelSelectMenu3 = {
    74, 36,
    74,
    16, Level3Items,
    DrawLevelSelect3Menu,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Vanilla options menu
// -----------------------------------------------------------------------------

static MenuItem_t OptionsItems_Vanilla[] = {
    {ITT_EFUNC,   "END GAME",          "PFRJYXBNM BUHE",   M_RD_EndGame,     0},
    {ITT_EFUNC,   "MESSAGES : ",       "CJJ,OTYBZ : ",     SCMessages,       0},
    {ITT_LRFUNC,  "MOUSE SENSITIVITY", "CRJHJCNM VSIB",    M_RD_Sensitivity, 0},
    {ITT_EMPTY,   NULL,                NULL,               NULL,             0},
    {ITT_SETMENU, "MORE...",           "LJGJKYBNTKMYJ>>>", NULL, (const intptr_t) &Options2Menu_Vanilla}
};

static Menu_t OptionsMenu_Vanilla = {
    88, 30,
    88,
    5, OptionsItems_Vanilla,
    DrawOptionsMenu_Vanilla,
    &MainMenu,
    0,
};

// -----------------------------------------------------------------------------
// Vanilla options menu (more...)
// -----------------------------------------------------------------------------

static MenuItem_t Options2Items_Vanilla[] = {
    {ITT_LRFUNC, "SCREEN SIZE",  "HFPVTH 'RHFYF",    M_RD_ScreenSize, 0},
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "SFX VOLUME",   "UHJVRJCNM PDERF",  M_RD_SfxVolume,  0},
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0},
    {ITT_LRFUNC, "MUSIC VOLUME", "UHJVRJCNM VEPSRB", M_RD_MusVolume,  0},
    {ITT_EMPTY,  NULL,           NULL,               NULL,            0}
};

static Menu_t Options2Menu_Vanilla = {
    90, 20,
    90,
    6, Options2Items_Vanilla,
    DrawOptions2Menu_Vanilla,
    &OptionsMenu_Vanilla,
    0
};

static MenuItem_t FilesItems[] = {
    {ITT_EFUNC,   "LOAD GAME", "PFUHEPBNM BUHE", (void (*)(intptr_t)) SCNetCheck_NG_LG, (const intptr_t) &LoadMenu}, // ЗАГРУЗИТЬ ИГРУ
    {ITT_SETMENU, "SAVE GAME", "CJ[HFYBNM BUHE", NULL, (const intptr_t) &SaveMenu} // СОХРАНИТЬ ИГРУ
};

static Menu_t FilesMenu = {
    110, 60,
    90,
    2, FilesItems,
    DrawFilesMenu,
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
    70, 30,
    70,
    6, LoadItems,
    DrawLoadMenu,
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
    70, 30,
    70,
    6, SaveItems,
    DrawSaveMenu,
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
    InitFonts();
    menuactive = false;
    SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;

    if (gamemode == retail)
    {                           // Add episodes 4 and 5 to the menu
        EpisodeMenu.itemCount = 5;
        EpisodeMenu.y -= ITEM_HEIGHT; //TODO test why
    }

    // [JN] Init message colors.
    M_RD_Define_Msg_Pickup_Color();
    M_RD_Define_Msg_Secret_Color();
    M_RD_Define_Msg_System_Color();
    M_RD_Define_Msg_Chat_Color();
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

/*
================================================================================
=
= MN_DrTextAFade
=
= [JN] Draw text using font A with fading effect.
=
================================================================================
*/

void MN_DrTextAFade(char *text, int x, int y, byte *table)
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
            V_DrawFadePatch(x, y, p, table);
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

/*
================================================================================
=
= MN_DrTextAFade
=
= [JN] Draw text using Russian small font (F) with fading effect.
=
================================================================================
*/

void MN_DrTextSmallRUSFade(char *text, int x, int y, byte *table)
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
            V_DrawFadePatch(x, y, p, table);
            x += SHORT(p->width) - 1;
        }
    }
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
    const MenuItem_t *item;
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
        x = english_language ? CurrentMenu->x_eng : CurrentMenu->x_rus;
        y = CurrentMenu->y;
        item = CurrentMenu->items;
        for (i = 0; i < CurrentMenu->itemCount; i++)
        {
            if (item->type != ITT_EMPTY && (english_language ? item->text_eng : item->text_rus))
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
                        MN_DrTextBigENG(DEH_String((char*) item->text_eng), x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextSmallENG(DEH_String((char*) item->text_eng), x + wide_delta, y);
                    }
                }
                else
                {
                    if (CurrentMenu == &MainMenu
                    ||  CurrentMenu == &EpisodeMenu
                    ||  CurrentMenu == &FilesMenu
                    ||  CurrentMenu == &SkillMenu
                    ||  CurrentMenu == &OptionsMenu
                    ||  CurrentMenu == &OptionsMenu_Vanilla
                    ||  CurrentMenu == &Options2Menu_Vanilla)
                    {
                        MN_DrTextBigRUS(DEH_String((char*) item->text_rus), x + wide_delta, y);
                    }
                    else
                    {
                        MN_DrTextSmallRUS(DEH_String((char*) item->text_rus), x + wide_delta, y);
                    }
                }
            }

            if (CurrentMenu == &MainMenu
            ||  CurrentMenu == &EpisodeMenu
            ||  CurrentMenu == &FilesMenu
            ||  CurrentMenu == &SkillMenu
            ||  CurrentMenu == &OptionsMenu
            ||  CurrentMenu == &OptionsMenu_Vanilla
            ||  CurrentMenu == &Options2Menu_Vanilla)
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
        ||  CurrentMenu == &OptionsMenu_Vanilla
        ||  CurrentMenu == &Options2Menu_Vanilla)
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
    //FIXME WTF
}

//---------------------------------------------------------------------------
//
// PROC DrawSkillMenu
//
//---------------------------------------------------------------------------

static void DrawSkillMenu(void)
{
    //FIXME WTF
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

static void DrawFileSlots(Menu_t * menu)
{
    int i;
    int x;
    int y;

    x = english_language ? menu->x_eng : menu->x_rus;
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

    // Sound / Sound System menu background will be drawn.
    sfxbgdraw = true;

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
    char *title_eng = DEH_String("RENDERING OPTIONS");
    char *title_rus = DEH_String("YFCNHJQRB DBLTJ");  // НАСТРОЙКИ ВИДЕО

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
            dp_translation = cr[CR_WHITE2GREEN_HERETIC];
            MN_DrTextSmallENG(DEH_String("THE PROGRAM MUST BE RESTARTED"),
                                         51 + wide_delta, 148);
            dp_translation = NULL;
        }

        // Vertical sync
        if (force_software_renderer)
        {
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("EXTRA"), 36 + wide_delta, 112);
        dp_translation = NULL;

        // Show ENDTEXT screen
        MN_DrTextSmallENG(DEH_String(show_endoom ? "ON" : "OFF"),
                                     188 + wide_delta, 132);
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
            dp_translation = cr[CR_WHITE2GREEN_HERETIC];
            MN_DrTextSmallRUS(DEH_String("YTJ,[JLBV GTHTPFGECR GHJUHFVVS"), 
                                         46 + wide_delta, 148);
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer)
        {
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + wide_delta, 112);
        dp_translation = NULL;

        // Показывать экран ENDTEXT
        MN_DrTextSmallENG(DEH_String("ENDTEXT:"), 160 + wide_delta, 132);
        MN_DrTextSmallRUS(DEH_String(show_endoom ? "DRK" : "DSRK"),
                                     222 + wide_delta, 132);
    }

    // Screenshot format / Формат скриншотов (same english values)
    MN_DrTextSmallENG(DEH_String(png_screenshots ? "PNG" : "PCX"),
                                 175 + wide_delta, 122);

}

static void M_RD_Change_Widescreen(intptr_t option)
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
        default:
            break;
    }
}

static void M_RD_Change_VSync(intptr_t option)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
        return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

static void M_RD_Uncapped(intptr_t option)
{
    uncapped_fps ^= 1;
}

static void M_RD_FPScounter(intptr_t option)
{
    show_fps ^= 1;
}

static void M_RD_Smoothing(intptr_t option)
{
    // [JN] Disable smoothing toggling in software renderer
    if (force_software_renderer == 1)
        return;

    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

static void M_RD_PorchFlashing(intptr_t option)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
}

static void M_RD_Renderer(intptr_t option)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();
}

static void M_RD_Screenshots(intptr_t option)
{
    png_screenshots ^= 1;
}

static void M_RD_EndText(intptr_t option)
{
    show_endoom ^= 1;
}

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    char *title_eng = DEH_String("DISPLAY OPTIONS");
    char *title_rus = DEH_String("YFCNHJQRB \'RHFYF");  // НАСТРОЙКИ ЭКРАНА
    char  num[4];

    // Draw menu background. Hide it for a moment while changing 
    // screen size, gamma and level brightness in GS_LEVEL game state.
    if (gamestate != GS_LEVEL || (gamestate == GS_LEVEL && menubgwait < I_GetTime()))
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("SCREEN"), 36 + wide_delta, 32);
        MN_DrTextSmallENG(DEH_String("INTERFACE"), 36 + wide_delta, 112);
        dp_translation = NULL;

        // Graphics detail
        MN_DrTextSmallENG(DEH_String(detailLevel ? "LOW" : "HIGH"),
                                     149 + wide_delta, 102);
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("\'RHFY"), 36 + wide_delta, 32);
        MN_DrTextSmallRUS(DEH_String("BYNTHATQC"), 36 + wide_delta, 112);
        dp_translation = NULL;

        // Детализация графики
        MN_DrTextSmallRUS(DEH_String(detailLevel ? "YBPRFZ" : "DSCJRFZ"),
                                     188 + wide_delta, 102);
    }

    //
    // Sliders
    //

    // Screen size
    if (aspect_ratio_temp >= 2)
    {
        DrawSliderSmall(&DisplayMenu, 52, 4, screenblocks - 9);
        M_snprintf(num, 4, "%d", screenblocks);
        dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallENG(num, 88 + wide_delta, 53);
        dp_translation = NULL;
    }
    else
    {
        DrawSliderSmall(&DisplayMenu, 52, 10, screenblocks - 3);
        M_snprintf(num, 4, "%d", screenblocks);
        dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextA(num, 136 + wide_delta, 53);
        dp_translation = NULL;
    }

    // Gamma-correction
    DrawSliderSmall(&DisplayMenu, 72, 18, usegamma);

    // Level brightness
    DrawSliderSmall(&DisplayMenu, 92, 5, level_brightness / 16);
}

static void M_RD_ScreenSize(intptr_t option)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

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
}

static void M_RD_Gamma(intptr_t option)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    switch(option)
    {
        case 0:
            if (usegamma > 0)
                usegamma--;
            break;

        case 1:
            if (usegamma < 17)
                usegamma++;
        default:
            break;
    }

    I_SetPalette((byte *) W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE));

    P_SetMessage(&players[consoleplayer], english_language ? 
                                          GammaText[usegamma] :
                                          GammaText_Rus[usegamma],
                                          msg_system, false);
}

static void M_RD_LevelBrightness(intptr_t option)
{
    // [JN] Hide menu background for a moment.
    menubgwait = I_GetTime() + 25;

    switch(option)
    {
        case 0:
            if (level_brightness > 0)
                level_brightness -= 16;
            break;

        case 1:
            if (level_brightness < 64)
                level_brightness += 16;
        default:
            break;
    }
}

static void M_RD_Detail(intptr_t option)
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
    char *title_eng = DEH_String("MESSAGES AND TEXTS");
    char *title_rus = DEH_String("CJJ,OTYBZ B NTRCNS");  // СООБЩЕНИЯ И ТЕКСТЫ

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // GENERAL, MISC., COLORS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("GENERAL"), 36 + wide_delta, 32);
        MN_DrTextSmallENG(DEH_String("MISC."), 36 + wide_delta, 102);
        MN_DrTextSmallENG(DEH_String("COLORS"), 36 + wide_delta, 122);
        dp_translation = NULL;

        // Messages
        MN_DrTextSmallENG(DEH_String(show_messages ? "ON" : "OFF"),
                                     108 + wide_delta, 42);

        // Messages alignment
        MN_DrTextSmallENG(messages_alignment == 1 ? "LEFT EDGE OF THE SCREEN" :
                          messages_alignment == 2 ? "LEFT EDGE OF THE STATUS BAR" :
                                                    "CENTERED", 108 + wide_delta, 52);

        // Таймаут отображения (текст)
        dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallENG(messages_timeout == 1 ? "1 SECOND"  :
                          messages_timeout == 2 ? "2 SECONDS" :
                          messages_timeout == 3 ? "3 SECONDS" :
                          messages_timeout == 4 ? "4 SECONDS" :
                          messages_timeout == 5 ? "5 SECONDS" :
                          messages_timeout == 6 ? "6 SECONDS" :
                          messages_timeout == 7 ? "7 SECONDS" :
                          messages_timeout == 8 ? "8 SECONDS" :
                          messages_timeout == 9 ? "9 SECONDS" :
                                                  "10 SECONDS", 136 + wide_delta, 73);
        dp_translation = NULL;

        // Fading effect 
        MN_DrTextSmallENG(DEH_String(message_fade ? "ON" : "OFF"),
                                     140 + wide_delta, 82);

        // Text casts shadows
        MN_DrTextSmallENG(DEH_String(draw_shadowed_text ? "ON" : "OFF"),
                                     179 + wide_delta, 92);
        dp_translation = NULL;

        // Local time
        MN_DrTextSmallENG(DEH_String(
                          local_time == 1 ? "12-HOUR (HH:MM)" :
                          local_time == 2 ? "12-HOUR (HH:MM:SS)" :
                          local_time == 3 ? "24-HOUR (HH:MM)" :
                          local_time == 4 ? "24-HOUR (HH:MM:SS)" : "OFF"),
                          110 + wide_delta, 112);

        // Item pickup
        dp_translation = M_RD_ColorTranslation(message_pickup_color);
        MN_DrTextSmallENG(DEH_String(M_RD_ColorName(message_pickup_color)), 120 + wide_delta, 132);
        dp_translation = NULL;

        // Revealed secret
        dp_translation = M_RD_ColorTranslation(message_secret_color);
        MN_DrTextSmallENG(DEH_String(M_RD_ColorName(message_secret_color)), 157 + wide_delta, 142);
        dp_translation = NULL;

        // System message
        dp_translation = M_RD_ColorTranslation(message_system_color);
        MN_DrTextSmallENG(DEH_String(M_RD_ColorName(message_system_color)), 152 + wide_delta, 152);
        dp_translation = NULL;

        // Netgame chat
        dp_translation = M_RD_ColorTranslation(message_chat_color);
        MN_DrTextSmallENG(DEH_String(M_RD_ColorName(message_chat_color)), 135 + wide_delta, 162);
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
        // GENERAL, MISC., COLORS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("JCYJDYJT"), 36 + wide_delta, 32); // ОСНОВНОЕ
        MN_DrTextSmallRUS(DEH_String("HFPYJT"), 36 + wide_delta, 102);  // РАЗНОЕ
        MN_DrTextSmallRUS(DEH_String("WDTNF"), 36 + wide_delta, 122);   // ЦВЕТА
        dp_translation = NULL;

        // Отображение сообщений
        MN_DrTextSmallRUS(DEH_String(show_messages ? "DRK" : "DSRK"),
                                     208 + wide_delta, 42);

        // Выравнивание сообщений
        MN_DrTextSmallRUS(messages_alignment == 1 ? "GJ RHF. \'RHFYF" :
                          messages_alignment == 2 ? "GJ RHF. CNFNEC-,FHF" :
                                                    "GJ WTYNHE", 138 + wide_delta, 52);

        // Таймаут отображения (текст)
        dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallRUS(messages_timeout == 1 ? "1 CTREYLF" :
                          messages_timeout == 2 ? "2 CTREYLS" :
                          messages_timeout == 3 ? "3 CTREYLS" :
                          messages_timeout == 4 ? "4 CTREYLS" :
                          messages_timeout == 5 ? "5 CTREYL"  :
                          messages_timeout == 6 ? "6 CTREYL"  :
                          messages_timeout == 7 ? "7 CTREYL"  :
                          messages_timeout == 8 ? "8 CTREYL"  :
                          messages_timeout == 9 ? "9 CTREYL"  :
                                                  "10 CTREYL", 136 + wide_delta, 73);
        dp_translation = NULL;

        // Плавное исчезновение
        MN_DrTextSmallRUS(DEH_String(message_fade ? "DRK" : "DSRK"),
                                     193 + wide_delta, 82);

        // Тексты отбрасывают тень
        MN_DrTextSmallRUS(DEH_String(draw_shadowed_text ? "DRK" : "DSRK"),
                                     220 + wide_delta, 92);

        // Системное время
        MN_DrTextSmallRUS(DEH_String(
                          local_time == 1 ? "12-XFCJDJT (XX:VV)" :
                          local_time == 2 ? "12-XFCJDJT (XX:VV:CC)" :
                          local_time == 3 ? "24-XFCJDJT (XX:VV)" :
                          local_time == 4 ? "24-XFCJDJT (XX:VV:CC)" : "DSRK"),
                          157 + wide_delta, 112);

        // Получение предметов
        dp_translation = M_RD_ColorTranslation(message_pickup_color);
        MN_DrTextSmallRUS(DEH_String(M_RD_ColorName(message_pickup_color)), 187 + wide_delta, 132);
        dp_translation = NULL;

        // Обнаружение тайников
        dp_translation = M_RD_ColorTranslation(message_secret_color);
        MN_DrTextSmallRUS(DEH_String(M_RD_ColorName(message_secret_color)), 195 + wide_delta, 142);
        dp_translation = NULL;

        // Системные сообщения
        dp_translation = M_RD_ColorTranslation(message_system_color);
        MN_DrTextSmallRUS(DEH_String(M_RD_ColorName(message_system_color)), 191 + wide_delta, 152);
        dp_translation = NULL;

        // Чат сетевой игры
        dp_translation = M_RD_ColorTranslation(message_chat_color);
        MN_DrTextSmallRUS(DEH_String(M_RD_ColorName(message_chat_color)), 162 + wide_delta, 162);
        dp_translation = NULL;
    }

    // Messages timeout
    DrawSliderSmall(&DisplayMenu, 72, 10, messages_timeout - 1);
}

static void M_RD_LocalTime(intptr_t option)
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
        default:
            break;
    }
}

static void M_RD_Messages(intptr_t option)
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

static void M_RD_MessagesAlignment(intptr_t option)
{
    switch (option)
    {
        case 0: 
        messages_alignment--;
        if (messages_alignment < 0) 
            messages_alignment = 2;
        break;

        case 1:
        messages_alignment++;
        if (messages_alignment > 2)
            messages_alignment = 0;
        break;
    }
}

static void M_RD_MessagesTimeout(intptr_t option)
{
    switch(option)
    {
        case 0:
        if (messages_timeout > 1)
            messages_timeout--;
        break;

        case 1:
        if (messages_timeout < 10)
            messages_timeout++;
        break;
    }
}

static void M_RD_MessagesFade(intptr_t option)
{
    message_fade ^= 1;
}

static void M_RD_ShadowedText(intptr_t option)
{
    draw_shadowed_text ^= 1;
}

static void M_RD_Define_Msg_Pickup_Color (void)
{
    // [JN] No coloring in vanilla.
    if (vanillaparm)
    {
        messages_pickup_color_set = NULL;
    }
    else
    {
        switch (message_pickup_color)
        {
            case 1:   messages_pickup_color_set = cr[CR_WHITE2GRAY_HERETIC];      break;
            case 2:   messages_pickup_color_set = cr[CR_WHITE2DARKGRAY_HERETIC];  break;
            case 3:   messages_pickup_color_set = cr[CR_WHITE2RED_HERETIC];       break;
            case 4:   messages_pickup_color_set = cr[CR_WHITE2DARKRED_HERETIC];   break;
            case 5:   messages_pickup_color_set = cr[CR_WHITE2GREEN_HERETIC];     break;
            case 6:   messages_pickup_color_set = cr[CR_WHITE2DARKGREEN_HERETIC]; break;
            case 7:   messages_pickup_color_set = cr[CR_WHITE2OLIVE_HERETIC];     break;
            case 8:   messages_pickup_color_set = cr[CR_WHITE2BLUE_HERETIC];      break;
            case 9:   messages_pickup_color_set = cr[CR_WHITE2DARKBLUE_HERETIC];  break;
            case 10:  messages_pickup_color_set = cr[CR_WHITE2PURPLE_HERETIC];    break;
            case 11:  messages_pickup_color_set = cr[CR_WHITE2NIAGARA_HERETIC];   break;
            case 12:  messages_pickup_color_set = cr[CR_WHITE2AZURE_HERETIC];     break;
            case 13:  messages_pickup_color_set = cr[CR_WHITE2YELLOW_HERETIC];    break;
            case 14:  messages_pickup_color_set = cr[CR_WHITE2GOLD_HERETIC];      break;
            case 15:  messages_pickup_color_set = cr[CR_WHITE2DARKGOLD_HERETIC];  break;
            case 16:  messages_pickup_color_set = cr[CR_WHITE2TAN_HERETIC];       break;
            case 17:  messages_pickup_color_set = cr[CR_WHITE2BROWN_HERETIC];     break;
            default:  messages_pickup_color_set = NULL;                           break;
        }
    }
}

void M_RD_Change_Msg_Pickup_Color(intptr_t option)
{
    switch(option)
    {
        case 0:
        message_pickup_color--;
        if (message_pickup_color < 0)
            message_pickup_color = 17;
        break;

        case 1:
        message_pickup_color++;
        if (message_pickup_color > 17)
            message_pickup_color = 0;
        break;
    }

    // [JN] Redefine pickup message color.
    M_RD_Define_Msg_Pickup_Color();
}

static void M_RD_Define_Msg_Secret_Color (void)
{
    // [JN] No coloring in vanilla.
    if (vanillaparm)
    {
        messages_secret_color_set = NULL;
    }
    else
    {
        switch (message_secret_color)
        {
            case 1:   messages_secret_color_set = cr[CR_WHITE2GRAY_HERETIC];      break;
            case 2:   messages_secret_color_set = cr[CR_WHITE2DARKGRAY_HERETIC];  break;
            case 3:   messages_secret_color_set = cr[CR_WHITE2RED_HERETIC];       break;
            case 4:   messages_secret_color_set = cr[CR_WHITE2DARKRED_HERETIC];   break;
            case 5:   messages_secret_color_set = cr[CR_WHITE2GREEN_HERETIC];     break;
            case 6:   messages_secret_color_set = cr[CR_WHITE2DARKGREEN_HERETIC]; break;
            case 7:   messages_secret_color_set = cr[CR_WHITE2OLIVE_HERETIC];     break;
            case 8:   messages_secret_color_set = cr[CR_WHITE2BLUE_HERETIC];      break;
            case 9:   messages_secret_color_set = cr[CR_WHITE2DARKBLUE_HERETIC];  break;
            case 10:  messages_secret_color_set = cr[CR_WHITE2PURPLE_HERETIC];    break;
            case 11:  messages_secret_color_set = cr[CR_WHITE2NIAGARA_HERETIC];   break;
            case 12:  messages_secret_color_set = cr[CR_WHITE2AZURE_HERETIC];     break;
            case 13:  messages_secret_color_set = cr[CR_WHITE2YELLOW_HERETIC];    break;
            case 14:  messages_secret_color_set = cr[CR_WHITE2GOLD_HERETIC];      break;
            case 15:  messages_secret_color_set = cr[CR_WHITE2DARKGOLD_HERETIC];  break;
            case 16:  messages_secret_color_set = cr[CR_WHITE2TAN_HERETIC];       break;
            case 17:  messages_secret_color_set = cr[CR_WHITE2BROWN_HERETIC];     break;
            default:  messages_secret_color_set = NULL;                           break;
        }
    }
}

void M_RD_Change_Msg_Secret_Color(intptr_t option)
{
    switch(option)
    {
        case 0:
        message_secret_color--;
        if (message_secret_color < 0)
            message_secret_color = 17;
        break;

        case 1:
        message_secret_color++;
        if (message_secret_color > 17)
            message_secret_color = 0;
        break;
    }

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_Secret_Color();
}

static void M_RD_Define_Msg_System_Color (void)
{
    // [JN] No coloring in vanilla.
    if (vanillaparm)
    {
        messages_system_color_set = NULL;
    }
    else
    {
        switch (message_system_color)
        {
            case 1:   messages_system_color_set = cr[CR_WHITE2GRAY_HERETIC];      break;
            case 2:   messages_system_color_set = cr[CR_WHITE2DARKGRAY_HERETIC];  break;
            case 3:   messages_system_color_set = cr[CR_WHITE2RED_HERETIC];       break;
            case 4:   messages_system_color_set = cr[CR_WHITE2DARKRED_HERETIC];   break;
            case 5:   messages_system_color_set = cr[CR_WHITE2GREEN_HERETIC];     break;
            case 6:   messages_system_color_set = cr[CR_WHITE2DARKGREEN_HERETIC]; break;
            case 7:   messages_system_color_set = cr[CR_WHITE2OLIVE_HERETIC];     break;
            case 8:   messages_system_color_set = cr[CR_WHITE2BLUE_HERETIC];      break;
            case 9:   messages_system_color_set = cr[CR_WHITE2DARKBLUE_HERETIC];  break;
            case 10:  messages_system_color_set = cr[CR_WHITE2PURPLE_HERETIC];    break;
            case 11:  messages_system_color_set = cr[CR_WHITE2NIAGARA_HERETIC];   break;
            case 12:  messages_system_color_set = cr[CR_WHITE2AZURE_HERETIC];     break;
            case 13:  messages_system_color_set = cr[CR_WHITE2YELLOW_HERETIC];    break;
            case 14:  messages_system_color_set = cr[CR_WHITE2GOLD_HERETIC];      break;
            case 15:  messages_system_color_set = cr[CR_WHITE2DARKGOLD_HERETIC];  break;
            case 16:  messages_system_color_set = cr[CR_WHITE2TAN_HERETIC];       break;
            case 17:  messages_system_color_set = cr[CR_WHITE2BROWN_HERETIC];     break;
            default:  messages_system_color_set = NULL;                           break;
        }
    }
}

void M_RD_Change_Msg_System_Color(intptr_t option)
{
    switch(option)
    {
        case 0:
        message_system_color--;
        if (message_system_color < 0)
            message_system_color = 17;
        break;

        case 1:
        message_system_color++;
        if (message_system_color > 17)
            message_system_color = 0;
        break;
    }

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_System_Color();
}

static void M_RD_Define_Msg_Chat_Color (void)
{
    // [JN] No coloring in vanilla.
    if (vanillaparm)
    {
        messages_chat_color_set = NULL;
    }
    else
    {
        switch (message_chat_color)
        {
            case 1:   messages_chat_color_set = cr[CR_WHITE2GRAY_HERETIC];      break;
            case 2:   messages_chat_color_set = cr[CR_WHITE2DARKGRAY_HERETIC];  break;
            case 3:   messages_chat_color_set = cr[CR_WHITE2RED_HERETIC];       break;
            case 4:   messages_chat_color_set = cr[CR_WHITE2DARKRED_HERETIC];   break;
            case 5:   messages_chat_color_set = cr[CR_WHITE2GREEN_HERETIC];     break;
            case 6:   messages_chat_color_set = cr[CR_WHITE2DARKGREEN_HERETIC]; break;
            case 7:   messages_chat_color_set = cr[CR_WHITE2OLIVE_HERETIC];     break;
            case 8:   messages_chat_color_set = cr[CR_WHITE2BLUE_HERETIC];      break;
            case 9:   messages_chat_color_set = cr[CR_WHITE2DARKBLUE_HERETIC];  break;
            case 10:  messages_chat_color_set = cr[CR_WHITE2PURPLE_HERETIC];    break;
            case 11:  messages_chat_color_set = cr[CR_WHITE2NIAGARA_HERETIC];   break;
            case 12:  messages_chat_color_set = cr[CR_WHITE2AZURE_HERETIC];     break;
            case 13:  messages_chat_color_set = cr[CR_WHITE2YELLOW_HERETIC];    break;
            case 14:  messages_chat_color_set = cr[CR_WHITE2GOLD_HERETIC];      break;
            case 15:  messages_chat_color_set = cr[CR_WHITE2DARKGOLD_HERETIC];  break;
            case 16:  messages_chat_color_set = cr[CR_WHITE2TAN_HERETIC];       break;
            case 17:  messages_chat_color_set = cr[CR_WHITE2BROWN_HERETIC];     break;
            default:  messages_chat_color_set = NULL;                           break;
        }
    }
}

void M_RD_Change_Msg_Chat_Color(intptr_t option)
{
    switch(option)
    {
        case 0:
        message_chat_color--;
        if (message_chat_color < 0)
            message_chat_color = 17;
        break;

        case 1:
        message_chat_color++;
        if (message_chat_color > 17)
            message_chat_color = 0;
        break;
    }

    // [JN] Redefine netgame chat message color.
    M_RD_Define_Msg_Chat_Color();
}

// -----------------------------------------------------------------------------
// DrawAutomapMenu
// -----------------------------------------------------------------------------

static void DrawAutomapMenu(void)
{
    char *title_eng = DEH_String("AUTOMAP AND STATISTICS");
    char *title_rus = DEH_String("RFHNF B CNFNBCNBRF");  // КАРТА И СТАТИСТИКА
    char  num[4];

    M_snprintf(num, 4, "%d", automap_grid_size);

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 7);

        //
        // AUTOMAP, STATISTICS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("AUTOMAP"), 78 + wide_delta, 32);
        MN_DrTextSmallENG(DEH_String("STATISTICS"), 78 + wide_delta, 92);
        dp_translation = NULL;

        // Overlay mode
        MN_DrTextSmallENG(DEH_String(automap_overlay ? "ON" : "OFF"),
                                     176 + wide_delta, 42);

        // Rotate mode
        MN_DrTextSmallENG(DEH_String(automap_rotate ? "ON" : "OFF"),
                                     169 + wide_delta, 52);

        // Follow mode
        MN_DrTextSmallENG(DEH_String(automap_follow ? "ON" : "OFF"),
                                     165 + wide_delta, 62);

        // Grid
        MN_DrTextSmallENG(DEH_String(automap_grid ? "ON" : "OFF"),
                                     114 + wide_delta, 72);

        // Grid size
        MN_DrTextSmallENG(DEH_String(num), 147 + wide_delta, 82);

        // Level stats
        MN_DrTextSmallENG(DEH_String(automap_stats == 1 ? "IN AUTOMAP" :
                                     automap_stats == 2 ? "ALWAYS" : "OFF"),
                                     163 + wide_delta, 102);

        // Level time
        MN_DrTextSmallENG(DEH_String(automap_level_time == 1 ? "IN AUTOMAP" :
                                     automap_level_time == 2 ? "ALWAYS" : "OFF"),
                                     152 + wide_delta, 112);

        // Total time
        MN_DrTextSmallENG(DEH_String(automap_total_time == 1 ? "IN AUTOMAP" :
                                     automap_total_time == 2 ? "ALWAYS" : "OFF"),
                                     153 + wide_delta, 122);

        // Player coords
        MN_DrTextSmallENG(DEH_String(automap_coords == 1 ? "IN AUTOMAP" :
                                     automap_coords == 2 ? "ALWAYS" : "OFF"),
                                     184 + wide_delta, 132);
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 
                                       + wide_delta, 7);

        //
        // КАРТА, СТАТИСТИКА
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("RFHNF"), 61 + wide_delta, 32);
        MN_DrTextSmallRUS(DEH_String("CNFNBCNBRF"), 61 + wide_delta, 92);
        dp_translation = NULL;

        // Режим наложения
        MN_DrTextSmallRUS(DEH_String(automap_overlay ? "DRK" : "DSRK"),
                                     187 + wide_delta, 42);

        // Режим вращения
        MN_DrTextSmallRUS(DEH_String(automap_rotate ? "DRK" : "DSRK"),
                                     179 + wide_delta, 52);

        // Режим следования
        MN_DrTextSmallRUS(DEH_String(automap_follow ? "DRK" : "DSRK"),
                                     194 + wide_delta, 62);

        // Сетка
        MN_DrTextSmallRUS(DEH_String(automap_grid ? "DRK" : "DSRK"),
                                     107 + wide_delta, 72);

        // Размер сетки
        MN_DrTextSmallRUS(DEH_String(num), 158 + wide_delta, 82);

        // Статистика уровня
        MN_DrTextSmallRUS(DEH_String(automap_stats == 1 ? "YF RFHNT" :
                                     automap_stats == 2 ? "DCTULF" : "DSRK"),
                                     193 + wide_delta, 102);

        // Время уровня
        MN_DrTextSmallRUS(DEH_String(automap_level_time == 1 ? "YF RFHNT" :
                                     automap_level_time == 2 ? "DCTULF" : "DSRK"),
                                     158 + wide_delta, 112);

        // Общее время
        MN_DrTextSmallRUS(DEH_String(automap_total_time == 1 ? "YF RFHNT" :
                                     automap_total_time == 2 ? "DCTULF" : "DSRK"),
                                     161 + wide_delta, 122);

        // Координаты игрока
        MN_DrTextSmallRUS(DEH_String(automap_coords == 1 ? "YF RFHNT" :
                                     automap_coords == 2 ? "DCTULF" : "DSRK"),
                                     198 + wide_delta, 132);
    }
}

static void M_RD_AutoMapOverlay(intptr_t option)
{
    automap_overlay ^= 1;
}

static void M_RD_AutoMapRotate(intptr_t option)
{
    automap_rotate ^= 1;
}

static void M_RD_AutoMapFollow(intptr_t option)
{
    automap_follow ^= 1;
}

static void M_RD_AutoMapGrid(intptr_t option)
{
    automap_grid ^= 1;
}

static void M_RD_AutoMapGridSize(intptr_t option)
{
    switch (option)
    {
        case 0:
            if (automap_grid_size == 512)
                automap_grid_size = 256;
            else
            if (automap_grid_size == 256)
                automap_grid_size = 128;
            else
            if (automap_grid_size == 128)
                automap_grid_size = 64;
            else
            if (automap_grid_size == 64)
                automap_grid_size = 32;
        break;

        case 1:
            if (automap_grid_size == 32)
                automap_grid_size = 64;
            else
            if (automap_grid_size == 64)
                automap_grid_size = 128;
            else
            if (automap_grid_size == 128)
                automap_grid_size = 256;
            else
            if (automap_grid_size == 256)
                automap_grid_size = 512;
        break;
    }
}

static void M_RD_AutoMapStats(intptr_t option)
{
    switch(option)
    {
        case 0: 
        automap_stats--;
        if (automap_stats < 0) 
            automap_stats = 2;
        break;
    
        case 1:
        automap_stats++;
        if (automap_stats > 2)
            automap_stats = 0;
        break;
    }
}

static void M_RD_AutoMapLevTime(intptr_t option)
{
    switch(option)
    {
        case 0: 
        automap_level_time--;
        if (automap_level_time < 0) 
            automap_level_time = 2;
        break;
    
        case 1:
        automap_level_time++;
        if (automap_level_time > 2)
            automap_level_time = 0;
        break;
    }
}

static void M_RD_AutoMapTotTime(intptr_t option)
{
    switch(option)
    {
        case 0: 
        automap_total_time--;
        if (automap_total_time < 0) 
            automap_total_time = 2;
        break;
    
        case 1:
        automap_total_time++;
        if (automap_total_time > 2)
            automap_total_time = 0;
        break;
    }
}

static void M_RD_AutoMapCoords(intptr_t option)
{
    switch(option)
    {
        case 0: 
        automap_coords--;
        if (automap_coords < 0) 
            automap_coords = 2;
        break;
    
        case 1:
        automap_coords++;
        if (automap_coords > 2)
            automap_coords = 0;
        break;
    }
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    char *title_eng = DEH_String("SOUND OPTIONS");
    char *title_rus = DEH_String("YFCNHJQRB PDERF");  // НАСТРОЙКИ ЗВУКА
    char  num[4];

    // Draw menu background. Don't draw if menu is invoked by pressing F4.
    if (sfxbgdraw)
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHJVRJCNM"), 36 + wide_delta, 32);
        MN_DrTextSmallRUS(DEH_String("DJCGHJBPDTLTYBT"), 36 + wide_delta, 82);
        MN_DrTextSmallRUS(DEH_String("LJGJKYBNTKMYJ"), 36 + wide_delta, 112);
        dp_translation = NULL;
    }

    //
    // Sliders
    //

    // SFX Volume
    DrawSliderSmall(&SoundMenu, 52, 16, snd_MaxVolume_tmp);
    M_snprintf(num, 4, "%d", snd_MaxVolume_tmp);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 53);
    dp_translation = NULL;

    // Music Volume
    DrawSliderSmall(&SoundMenu, 72, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%d", snd_MusicVolume);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 73);
    dp_translation = NULL;

    // SFX Channels
    DrawSliderSmall(&SoundMenu, 102, 16, snd_Channels / 4 - 1);
    M_snprintf(num, 4, "%d", snd_Channels);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 184 + wide_delta, 103);
    dp_translation = NULL;
}

static void M_RD_SfxVolume(intptr_t option)
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
}

static void M_RD_MusVolume(intptr_t option)
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

static void M_RD_SfxChannels(intptr_t option)
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
    char *title_eng = DEH_String("SOUND SYSTEM SETTINGS");
    char *title_rus = DEH_String("YFCNHJQRB PDERJDJQ CBCNTVS");  // НАСТРОЙКИ ЗВУКОВОЙ СИСТЕМЫ

    // Draw menu background. Don't draw if menu is invoked by pressing F4.
    if (sfxbgdraw)
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("SOUND SYSTEM"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Sound effects
        if (snd_sfxdevice == 0)
        {
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
        if (CurrentItPos == 3)
        {
            dp_translation = cr[CR_WHITE2GREEN_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("PDERJDFZ CBCNTVF"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // ОТКЛЮЧЕНЫ
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
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
        if (CurrentItPos == 3)
        {
            dp_translation = cr[CR_WHITE2GREEN_HERETIC];
            MN_DrTextSmallRUS(DEH_String("BPVTYTYBT GJNHT,ETN GTHTPFGECR GHJUHFVVS"), 
                                         11 + wide_delta, 132);
            dp_translation = NULL;
        }
    }
}

static void M_RD_SoundDevice(intptr_t option)
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
        default:
            break;
    }

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume, recalculate sound curve
    S_SetMaxVolume(true);
}

static void M_RD_MusicDevice(intptr_t option)
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

static void M_RD_Sampling(intptr_t option)
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
        default:
            break;
    }
}

static void M_RD_SndMode(intptr_t option)
{
    snd_monomode ^= 1;
}

static void M_RD_PitchShifting(intptr_t option)
{
    snd_pitchshift ^= 1;
}

static void M_RD_MuteInactive(intptr_t option)
{
    mute_inactive_window ^= 1;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    char *title_eng = DEH_String("CONTROL SETTINGS");
    char *title_rus = DEH_String("EGHFDKTYBT");  // УПРАВЛЕНИЕ
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MOVEMENT"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Always run
        MN_DrTextSmallENG(DEH_String(joybspeed >= 20 ? "ON" : "OFF"),
                                     118 + wide_delta, 42);

        //
        // MOUSE
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MOUSE"), 36 + wide_delta, 52);
        dp_translation = NULL;

        // Mouse look
        MN_DrTextSmallENG(DEH_String(mlook ? "ON" : "OFF"),
                                     118 + wide_delta, 122);

        // Invert Y axis
        if (!mlook)
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallENG(DEH_String(mouse_y_invert ? "ON" : "OFF"),
                                     133 + wide_delta, 132);
        dp_translation = NULL;

        // Novert
        if (mlook)
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
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
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("GTHTLDB;TYBT"), 36 + wide_delta, 32);
        dp_translation = NULL;

        // Режим постоянного бега
        MN_DrTextSmallRUS(DEH_String(joybspeed >= 20 ? "DRK" : "DSRK"),
                                     209 + wide_delta, 42);

        //
        // МЫШЬ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("VSIM"), 36 + wide_delta, 52);
        dp_translation = NULL;

        // Обзор мышью
        MN_DrTextSmallRUS(DEH_String(mlook ? "DRK" : "DSRK"),
                                     132 + wide_delta, 122);

        // Вертикальная инверсия
        if (!mlook)
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallRUS(DEH_String(mouse_y_invert ? "DRK" : "DSRK"),
                                     199 + wide_delta, 132);
        dp_translation = NULL;

        // Вертикальное перемещение
        if (mlook)
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
        MN_DrTextSmallRUS(DEH_String(!novert ? "DRK" : "DSRK"),
                                     227 + wide_delta, 142);
        dp_translation = NULL;
    }

    //
    // Sliders
    //

    // Mouse sensivity
    DrawSliderSmall(&ControlsMenu, 72, 12, mouseSensitivity);
    M_snprintf(num, 4, "%d", mouseSensitivity);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 73);
    dp_translation = NULL;

    // Acceleration
    DrawSliderSmall(&ControlsMenu, 92, 12, mouse_acceleration * 4 - 4);
    M_snprintf(num, 4, "%f", mouse_acceleration);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 93);
    dp_translation = NULL;

    // Threshold
    DrawSliderSmall(&ControlsMenu, 112, 12, mouse_threshold / 2);
    M_snprintf(num, 4, "%d", mouse_threshold);
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(num, 152 + wide_delta, 113);
    dp_translation = NULL;
}

static void M_RD_AlwaysRun(intptr_t option)
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

static void M_RD_Sensitivity(intptr_t option)
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

static void M_RD_Acceleration(intptr_t option)
{
    switch (option)
    {
        case 0:
            if (mouse_acceleration > 1.1)
                mouse_acceleration -= 0.1F;
            break;

        case 1:
            if (mouse_acceleration < 5.0)
                mouse_acceleration += 0.1F;
        default:
            break;
    }
}

static void M_RD_Threshold(intptr_t option)
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
        default:
            break;
    }
}

static void M_RD_MouseLook(intptr_t option)
{
    mlook ^= 1;

    if (!mlook)
        players[consoleplayer].centering = true;
}

static void M_RD_InvertY(intptr_t option)
{
    mouse_y_invert ^= 1;
}

static void M_RD_Novert(intptr_t option)
{
    novert ^= 1;
}

// -----------------------------------------------------------------------------
// DrawGameplay1Menu
// -----------------------------------------------------------------------------

static void DrawGameplay1Menu(void)
{
    char *title_eng = DEH_String("GAMEPLAY FEATURES");
    char *title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 + wide_delta, 4);

        //
        // Subheaders
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("VISUAL"), 36 + wide_delta, 26);
        MN_DrTextSmallENG(DEH_String("PHYSICAL"), 36 + wide_delta, 106);
        dp_translation = NULL;

        // Brightmaps
        dp_translation = brightmaps ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(brightmaps ? "ON" : "OFF"), 119 + wide_delta, 36);
        dp_translation = NULL;

        // Fake contrast
        dp_translation = fake_contrast ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(fake_contrast ? "ON" : "OFF"), 143 + wide_delta, 46);
        dp_translation = NULL;

        // Extra translucency
        dp_translation = translucency ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(translucency ? "ON" : "OFF"), 180 + wide_delta, 56);
        dp_translation = NULL;

        // Colored blood
        dp_translation = colored_blood ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(colored_blood ? "ON" : "OFF"), 139 + wide_delta, 66);
        dp_translation = NULL;

        // Swirling liquids
        dp_translation = swirling_liquids ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(swirling_liquids ? "ON" : "OFF"), 147 + wide_delta, 76);
        dp_translation = NULL;

        // Invulnerability affects sky
        dp_translation = invul_sky ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(invul_sky ? "ON" : "OFF"), 235 + wide_delta, 86);
        dp_translation = NULL;

        // Sky drawing mode
        dp_translation = linear_sky ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(linear_sky ? "LINEAR" : "ORIGINAL"), 162 + wide_delta, 96);
        dp_translation = NULL;

        // Corpses sliding from the ledges
        dp_translation = torque ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(torque ? "ON" : "OFF"), 238 + wide_delta, 116);
        dp_translation = NULL;

        // Weapon bobbing while firing
        dp_translation = weapon_bobbing ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(weapon_bobbing ? "ON" : "OFF"), 233 + wide_delta, 126);
        dp_translation = NULL;

        // Randomly flipped corpses
        dp_translation = randomly_flipcorpses ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(randomly_flipcorpses ? "ON" : "OFF"), 232 + wide_delta, 136);
        dp_translation = NULL;

        // Floating items amplitude
        dp_translation = floating_powerups ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(floating_powerups == 1 ? "STANDARD" :
                                     floating_powerups == 2 ? "HALFED" : "OFF"),
                                     209 + wide_delta, 146);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 4);

        //
        // ГРАФИКА, ФИЗИКА
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("UHFABRF"), 36 + wide_delta, 26);
        MN_DrTextSmallRUS(DEH_String("ABPBRF"), 36 + wide_delta, 106);
        dp_translation = NULL;

        // Брайтмаппинг
        dp_translation = brightmaps ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(brightmaps ? "DRK" : "DSRK"), 133 + wide_delta, 36);
        dp_translation = NULL;

        // Имитация контрастности
        dp_translation = fake_contrast ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(fake_contrast ? "DRK" : "DSRK"), 205 + wide_delta, 46);
        dp_translation = NULL;

        // Дополнительная прозрачность
        dp_translation = translucency ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(translucency ? "DRK" : "DSRK"), 245 + wide_delta, 56);
        dp_translation = NULL;

        // Разноцветная кровь
        dp_translation = colored_blood ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(colored_blood ? "DRK" : "DSRK"), 178 + wide_delta, 66);
        dp_translation = NULL;

        // Улучшенная анимация жидкостей
        dp_translation = swirling_liquids ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(swirling_liquids ? "DRK" : "DSRK"), 261 + wide_delta, 76);
        dp_translation = NULL;

        // Неуязвимость окрашивает небо
        dp_translation = invul_sky ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(invul_sky ? "DRK" : "DSRK"), 253 + wide_delta, 86);
        dp_translation = NULL;

        // Режим отрисовки неба
        dp_translation = linear_sky ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(linear_sky ? "KBYTQYSQ" : "JHBUBYFKMYSQ"), 195 + wide_delta, 96);
        dp_translation = NULL;

        // Трупы сползают с возвышений
        dp_translation = torque ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(torque ? "DRK" : "DSRK"), 248 + wide_delta, 116);
        dp_translation = NULL;

        // Улучшенное покачивание оружия
        dp_translation = weapon_bobbing ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(weapon_bobbing ? "DRK" : "DSRK"), 260 + wide_delta, 126);
        dp_translation = NULL;

        // Зеркалирование трупов
        dp_translation = randomly_flipcorpses ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(randomly_flipcorpses ? "DRK" : "DSRK"), 201 + wide_delta, 136);
        dp_translation = NULL;

        // Амплитуда левитации предметов
        dp_translation = floating_powerups ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(floating_powerups == 1 ? "CNFYLFHNYFZ" :
                                     floating_powerups == 2 ? "EVTHTYYFZ" : "DSRK"),
                                     188 + wide_delta, 146);
        dp_translation = NULL;
    }

    // (1/3)
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(DEH_String("(1/3)"), 254 + wide_delta, 166);
    dp_translation = NULL;
}

static void M_RD_Brightmaps(intptr_t option)
{
    brightmaps ^= 1;
}

static void M_RD_FakeContrast(intptr_t option)
{
    fake_contrast ^= 1;
}

static void M_RD_ExtraTrans(intptr_t option)
{
    translucency ^= 1;
}

static void M_RD_ColoredBlood(intptr_t option)
{
    colored_blood ^= 1;
}

static void M_RD_InvulSky(intptr_t option)
{
    invul_sky ^= 1;
}

static void M_RD_SwirlingLiquids(intptr_t option)
{
    swirling_liquids ^= 1;
}

static void M_RD_LinearSky(intptr_t option)
{
    linear_sky ^= 1;
}

static void M_RD_Torque(intptr_t option)
{
    torque ^= 1;
}

static void M_RD_Bobbing(intptr_t option)
{
    weapon_bobbing ^= 1;
}

static void M_RD_FlipCorpses(intptr_t option)
{
    randomly_flipcorpses ^= 1;
}

static void M_RD_FloatAmplitude(intptr_t option)
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
        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// DrawGameplay2Menu
// -----------------------------------------------------------------------------

static void DrawGameplay2Menu(void)
{
    char *title_eng = DEH_String("GAMEPLAY FEATURES");
    char *title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 + wide_delta, 4);

        //
        // VISUAL
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("STATUS BAR"), 36 + wide_delta, 26);
        MN_DrTextSmallENG(DEH_String("AMMO WIDGET"), 36 + wide_delta, 66);
        MN_DrTextSmallENG(DEH_String("CROSSHAIR"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Colored Status Bar
        dp_translation = sbar_colored_gem == 1 ? cr[CR_WHITE2GREEN_HERETIC] :
                         sbar_colored_gem == 2 ? cr[CR_WHITE2DARKGREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(sbar_colored ? "ON" : "OFF"), 177 + wide_delta, 36);
        dp_translation = NULL;

        // Colored health gem
        dp_translation = sbar_colored_gem ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(sbar_colored_gem == 1 ? "BRIGHT" :
                                     sbar_colored_gem == 2 ? "DARK" : "OFF"), 175 + wide_delta, 46);
        dp_translation = NULL;

        // Negative health
        dp_translation = negative_health ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(negative_health ? "ON" : "OFF"), 190 + wide_delta, 56);
        dp_translation = NULL;

        // Draw widget
        dp_translation = ammo_widget ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(ammo_widget == 1 ? "BRIEF" : 
                                     ammo_widget == 2 ? "FULL" :  "OFF"), 124 + wide_delta, 76);
        dp_translation = NULL;

        // Coloring
        dp_translation = ammo_widget_colored ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(ammo_widget_colored ? "ON" : "OFF"), 101 + wide_delta, 86);
        dp_translation = NULL;

        // Draw crosshair
        dp_translation = crosshair_draw ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_draw ? "ON" : "OFF"), 150 + wide_delta, 106);
        dp_translation = NULL;

        // Indication
        dp_translation = crosshair_type ? cr[CR_WHITE2GREEN_HERETIC] :
                                          cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_type == 1 ? "HEALTH" :
                                     crosshair_type == 2 ? "TARGET HIGHLIGHTING" :
                                     crosshair_type == 3 ? "TARGET HIGHLIGHTING+HEALTH" :
                                                            "STATIC"),
                                     111 + wide_delta, 116);
        dp_translation = NULL;

        // Increased size
        dp_translation = crosshair_scale ? cr[CR_WHITE2GREEN_HERETIC] :
                                           cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(crosshair_scale ? "ON" : "OFF"), 146 + wide_delta, 126);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2 + wide_delta, 4);

        //
        // СТАТУС-БАР, ВИДЖЕТ БОЕЗАПАСА, ПРИЦЕЛ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("CNFNEC-,FH"), 36 + wide_delta, 26);
        MN_DrTextSmallRUS(DEH_String("DBL;TN ,JTPFGFCF"), 36 + wide_delta, 66);
        MN_DrTextSmallRUS(DEH_String("GHBWTK"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Разноцветные элементы
        dp_translation = sbar_colored ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(sbar_colored ? "DRK" : "DSRK"), 206 + wide_delta, 36);
        dp_translation = NULL;

        // Окрашивание камня здоровья
        dp_translation = sbar_colored_gem == 1 ? cr[CR_WHITE2GREEN_HERETIC] :
                         sbar_colored_gem == 2 ? cr[CR_WHITE2DARKGREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(sbar_colored_gem == 1 ? "CDTNKJT" :
                                     sbar_colored_gem == 2 ? "NTVYJT" : "DSRK"), 238 + wide_delta, 46);
        dp_translation = NULL;

        // Отрицательное здоровье
        dp_translation = negative_health ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(negative_health ? "DRK" : "DSRK"), 211 + wide_delta, 56);
        dp_translation = NULL;

        // Отображать виджет
        dp_translation = ammo_widget ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(ammo_widget == 1 ? "RHFNRBQ" : 
                                     ammo_widget == 2 ? "GJLHJ,YSQ" : "DSRK"), 179 + wide_delta, 76);
        dp_translation = NULL;

        // Цветовая индикация
        dp_translation = ammo_widget_colored ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(ammo_widget_colored ? "DRK" : "DSRK"), 178 + wide_delta, 86);
        dp_translation = NULL;

        // Отображать прицел
        dp_translation = crosshair_draw ? cr[CR_WHITE2GREEN_HERETIC] :
                                          cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_draw ? "DRK" : "DSRK"), 175 + wide_delta, 106);
        dp_translation = NULL;

        // Индикация
        dp_translation = crosshair_type ? cr[CR_WHITE2GREEN_HERETIC] :
                                          cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_type == 1 ? "PLJHJDMT" :       // ЗДОРОВЬЕ
                                     crosshair_type == 2 ? "GJLCDTNRF WTKB" : // ПОДСВЕТКА ЦЕЛИ
                                     crosshair_type == 3 ? "GJLCDTNRF WTKB+PLJHJDMT" :
                                                           "CNFNBXYFZ"),      // СТАТИЧНАЯ
                                     111 + wide_delta, 116);
        dp_translation = NULL;

        // Увеличенный размер
        dp_translation = crosshair_scale ? cr[CR_WHITE2GREEN_HERETIC] :
                                           cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(crosshair_scale ? "DRK" : "DSRK"), 181 + wide_delta, 126);
        dp_translation = NULL;
    }

    // (2/3)
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(DEH_String("(2/3)"), 254 + wide_delta, 166);
    dp_translation = NULL;
}

static void M_RD_ColoredSBar(intptr_t option)
{
    sbar_colored ^= 1;
}

static void M_RD_ColoredGem(intptr_t option)
{
    switch(option)
    {
        case 0:
            sbar_colored_gem--;
            if (sbar_colored_gem < 0)
                sbar_colored_gem = 2;
            break;
        case 1:
            sbar_colored_gem++;
            if (sbar_colored_gem > 2)
                sbar_colored_gem = 0;
        default:
            break;
    }
}

static void M_RD_NegativeHealth(intptr_t option)
{
    negative_health ^= 1;
}

static void M_RD_AmmoWidgetDraw(intptr_t option)
{
    switch (option)
    {
        case 0: 
            ammo_widget--;
            if (ammo_widget < 0)
                ammo_widget = 2;
            break;

        case 1:
            ammo_widget++;
            if (ammo_widget > 2)
                ammo_widget = 0;
        default:
            break;
    }

}

static void M_RD_AmmoWidgetColoring(intptr_t option)
{
    ammo_widget_colored ^= 1;
}

static void M_RD_CrossHairDraw(intptr_t option)
{
    crosshair_draw ^= 1;
}

static void M_RD_CrossHairType(intptr_t option)
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
        default:
            break;
    }
}

static void M_RD_CrossHairScale(intptr_t option)
{
    crosshair_scale ^= 1;
}

// -----------------------------------------------------------------------------
// DrawGameplay3Menu
// -----------------------------------------------------------------------------

static void DrawGameplay3Menu(void)
{
    char *title_eng = DEH_String("GAMEPLAY FEATURES");
    char *title_rus = DEH_String("YFCNHJQRB UTQVGKTZ");  // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2 
                                       + wide_delta, 4);

        //
        // AUDIBLE
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("AUDIBLE"), 36 + wide_delta, 26);
        MN_DrTextSmallENG(DEH_String("TACTICAL"), 36 + wide_delta, 56);
        MN_DrTextSmallENG(DEH_String("GAMEPLAY"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Sound attenuation axises
        dp_translation = z_axis_sfx ? cr[CR_WHITE2GREEN_HERETIC] :
                                      cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(z_axis_sfx ? "X/Y/Z" : "X/Y"), 219 + wide_delta, 36);
        dp_translation = NULL;

        // Monster alert waking up others
        dp_translation = noise_alert_sfx ? cr[CR_WHITE2GREEN_HERETIC] :
                                           cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(noise_alert_sfx ? "ON" : "OFF", 262 + wide_delta, 46);
        dp_translation = NULL;

        // Notify of revealed secrets
        dp_translation = secret_notification ? cr[CR_WHITE2GREEN_HERETIC] :
                                               cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(secret_notification ? "ON" : "OFF"), 235 + wide_delta, 66);
        dp_translation = NULL;

        // Active artifacts
        dp_translation = show_all_artifacts ? cr[CR_WHITE2GREEN_HERETIC] :
                                              cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(show_all_artifacts ? "ALL" : "WINGS/TOME"), 195 + wide_delta, 76);
        dp_translation = NULL;

        // Artifacts timer
        dp_translation = show_artifacts_timer == 1 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                         show_artifacts_timer == 2 ? cr[CR_WHITE2GRAY_HERETIC] :
                         show_artifacts_timer == 3 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(show_artifacts_timer == 1 ? "GOLD" :
                                     show_artifacts_timer == 2 ? "SILVER" :
                                     show_artifacts_timer == 3 ? "COLORED" : "OFF"),
                                     150 + wide_delta, 86);
        dp_translation = NULL;

        // Fix errors of vanilla maps
        dp_translation = fix_map_errors ? cr[CR_WHITE2GREEN_HERETIC] :
                                          cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(fix_map_errors ? "ON" : "OFF"), 226 + wide_delta, 106);
        dp_translation = NULL;

        // Flip game levels
        dp_translation = flip_levels ? cr[CR_WHITE2GREEN_HERETIC] :
                                       cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(flip_levels ? "ON" : "OFF"), 153 + wide_delta, 116);
        dp_translation = NULL;

        // Play internal demos
        dp_translation = no_internal_demos ? cr[CR_WHITE2RED_HERETIC] :
                                             cr[CR_WHITE2GREEN_HERETIC];
        MN_DrTextSmallENG(DEH_String(no_internal_demos ? "OFF" : "ON"), 179 + wide_delta, 126);
        dp_translation = NULL;

        // Wand start
        dp_translation = pistol_start ? cr[CR_WHITE2GREEN_HERETIC] :
                                        cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(pistol_start ? "ON" : "OFF"), 193 + wide_delta, 136);
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
        // ЗВУК, ТАКТИКА, ГЕЙМПЛЕЙ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("PDER"), 36 + wide_delta, 26);
        MN_DrTextSmallRUS(DEH_String("NFRNBRF"), 36 + wide_delta, 56);
        MN_DrTextSmallRUS(DEH_String("UTQVGKTQ"), 36 + wide_delta, 96);
        dp_translation = NULL;

        // Затухание звука по осям
        dp_translation = z_axis_sfx ? cr[CR_WHITE2GREEN_HERETIC] :
                                      cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(DEH_String(z_axis_sfx ? "X/Y/Z" : "X/Y"), 209 + wide_delta, 36);
        dp_translation = NULL;

        // Общая тревога у монстров
        dp_translation = noise_alert_sfx ? cr[CR_WHITE2GREEN_HERETIC] :
                                           cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(noise_alert_sfx ? "DRK" : "DSRK", 223 + wide_delta, 46);
        dp_translation = NULL;


        // Сообщать о найденном тайнике
        dp_translation = secret_notification ? cr[CR_WHITE2GREEN_HERETIC] :
                                               cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(secret_notification ? "DRK" : "DSRK"), 251 + wide_delta, 66);
        dp_translation = NULL;

        // Индикация артефаектов
        dp_translation = show_all_artifacts ? cr[CR_WHITE2GREEN_HERETIC] :
                                              cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(show_all_artifacts ? "DCT FHNTAFRNS" : "RHSKMZ/NJV"), 196 + wide_delta, 76);
        dp_translation = NULL;

        // Таймер артефаектов
        dp_translation = show_artifacts_timer == 1 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                         show_artifacts_timer == 2 ? cr[CR_WHITE2GRAY_HERETIC] :
                         show_artifacts_timer == 3 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(show_artifacts_timer == 1 ? "PJKJNJQ" :
                                     show_artifacts_timer == 2 ? "CTHT,HZYSQ" :
                                     show_artifacts_timer == 3 ? "HFPYJWDTNYSQ" : "DSRK"),
                                     175 + wide_delta, 86);
        dp_translation = NULL;

        // Устранять ошибки оригинальных уровней
        dp_translation = fix_map_errors ? cr[CR_WHITE2GREEN_HERETIC] :
                                          cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(fix_map_errors ? "DRK" : "DSRK"), 257 + wide_delta, 106);
        dp_translation = NULL;

        // Зеркальное отражение уровней
        dp_translation = flip_levels ? cr[CR_WHITE2GREEN_HERETIC] :
                                       cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(flip_levels ? "DRK" : "DSRK"), 255 + wide_delta, 116);
        dp_translation = NULL;

        // Проигрывать демозаписи
        dp_translation = no_internal_demos ? cr[CR_WHITE2RED_HERETIC] :
                                             cr[CR_WHITE2GREEN_HERETIC];
        MN_DrTextSmallRUS(DEH_String(no_internal_demos ? "DSRK" : "DRK"), 211 + wide_delta, 126);
        dp_translation = NULL;

        // Режим игры "Wand start"
        MN_DrTextSmallRUS(DEH_String("HT;BV BUHS"), 36 + wide_delta, 136);
        MN_DrTextSmallENG(DEH_String("\"WAND START\":"), 120 + wide_delta, 136);
        dp_translation = pistol_start ? cr[CR_WHITE2GREEN_HERETIC] :
                                        cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(DEH_String(pistol_start ? "DRK" : "DSRK"), 217 + wide_delta, 136);
        dp_translation = NULL;
    }

    // (3/3)
    dp_translation = cr[CR_WHITE2GRAY_HERETIC];
    MN_DrTextSmallENG(DEH_String("(3/3)"), 254 + wide_delta, 166);
    dp_translation = NULL;
}

static void M_RD_ZAxisSFX(intptr_t option)
{
    z_axis_sfx ^= 1;
}

static void M_RD_AlertSFX(intptr_t option)
{
    noise_alert_sfx ^= 1;
}

static void M_RD_SecretNotify(intptr_t option)
{
    secret_notification ^= 1;
}

static void M_RD_ShowAllArti(intptr_t option)
{
    show_all_artifacts ^= 1;
}

static void M_RD_ShowArtiTimer(intptr_t option)
{
    switch(option)
    {
        case 0:
            show_artifacts_timer--;
            if (show_artifacts_timer < 0)
                show_artifacts_timer = 3;
            break;
        case 1:
            show_artifacts_timer++;
            if (show_artifacts_timer > 3)
                show_artifacts_timer = 0;
        default:
            break;
    }
}

static void M_RD_FixMapErrors(intptr_t option)
{
    fix_map_errors ^= 1;
}

static void M_RD_FlipLevels(intptr_t option)
{
    flip_levels ^= 1;

    // [JN] Redraw game screen
    R_ExecuteSetViewSize();
}

static void M_RD_NoDemos(intptr_t option)
{
    no_internal_demos ^= 1;
}

static void M_RD_WandStart(intptr_t option)
{
    pistol_start ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect1Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect1Menu(void)
{
    char *title_eng = DEH_String("LEVEL SELECT");
    char *title_rus = DEH_String("DS,JH EHJDYZ");  // ВЫБОР УРОВНЯ
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2
                                       + wide_delta, 4);

        //
        // PLAYER
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("PLAYER"), 74 + wide_delta, 56);
        dp_translation = NULL;

        //
        // WEAPONS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("WEAPONS"), 74 + wide_delta, 96);
        dp_translation = NULL;

        // Gauntlets
        dp_translation = selective_wp_gauntlets ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_gauntlets ? "YES" : "NO", 228 + wide_delta, 106);
        dp_translation = NULL;

        // Ethereal Crossbow
        dp_translation = selective_wp_crossbow ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_crossbow ? "YES" : "NO", 228 + wide_delta, 116);
        dp_translation = NULL;

        // Dragon Claw
        dp_translation = selective_wp_dragonclaw ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_dragonclaw ? "YES" : "NO", 228 + wide_delta, 126);
        dp_translation = NULL;

        // Hellstaff
        dp_translation = selective_wp_hellstaff ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_hellstaff ? "YES" : "NO", 228 + wide_delta, 136);
        dp_translation = NULL;

        // Phoenix Rod
        dp_translation = selective_wp_phoenixrod ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_phoenixrod ? "YES" : "NO", 228 + wide_delta, 146);
        dp_translation = NULL;

        // Firemace
        dp_translation = selective_wp_firemace ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_wp_firemace ? "YES" : "NO", 228 + wide_delta, 156);
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
        // ИГРОК
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("BUHJR"), 74 + wide_delta, 56);
        dp_translation = NULL;

        //
        // ОРУЖИЕ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("JHE;BT"), 74 + wide_delta, 96);
        dp_translation = NULL;

        // Перчатки
        dp_translation = selective_wp_gauntlets ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_gauntlets ? "LF" : "YTN", 228 + wide_delta, 106);
        dp_translation = NULL;

        // Эфирный арбалет
        dp_translation = selective_wp_crossbow ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_crossbow ? "LF" : "YTN", 228 + wide_delta, 116);
        dp_translation = NULL;

        // Коготь дракона
        dp_translation = selective_wp_dragonclaw ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_dragonclaw ? "LF" : "YTN", 228 + wide_delta, 126);
        dp_translation = NULL;

        // Посох Ада
        dp_translation = selective_wp_hellstaff ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_hellstaff ? "LF" : "YTN", 228 + wide_delta, 136);
        dp_translation = NULL;

        // Жезл Феникса
        dp_translation = selective_wp_phoenixrod ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_phoenixrod ? "LF" : "YTN", 228 + wide_delta, 146);
        dp_translation = NULL;

        // Огненная булава
        dp_translation = selective_wp_firemace ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_wp_firemace ? "LF" : "YTN", 228 + wide_delta, 156);
        dp_translation = NULL;
    }

    // The rest of values/placements are same for both languages.
    {
        // Skill level | Сложность
        M_snprintf(num, 4, "%d", selective_skill+1);
        MN_DrTextSmallENG(num, 228 + wide_delta, 26);

        // Episode | Эпизод
        if (gamemode == shareware)
        {
            dp_translation = cr[CR_WHITE2GRAY_HERETIC];
            MN_DrTextSmallENG("1", 228 + wide_delta, 36);
            dp_translation = NULL;
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_episode);
            MN_DrTextSmallENG(num, 228 + wide_delta, 36);
        }

        // Map | Уровень
        M_snprintf(num, 4, "%d", selective_map);
        MN_DrTextSmallENG(num, 228 + wide_delta, 46);

        // Health | Здоровье
        dp_translation = selective_health >= 67 ? cr[CR_WHITE2GREEN_HERETIC] :
                         selective_health >= 34 ? cr[CR_WHITE2DARKGOLD_HERETIC]  :
                                                  cr[CR_WHITE2RED_HERETIC];
        M_snprintf(num, 4, "%d", selective_health);
        MN_DrTextSmallENG(num, 228 + wide_delta, 66);

        // Armor | Броня
        dp_translation = selective_armortype == 1 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                    cr[CR_WHITE2GREEN_HERETIC];
        if (selective_armor == 0)
            dp_translation = cr[CR_WHITE2RED_HERETIC];
        M_snprintf(num, 4, "%d", selective_armor);
        MN_DrTextSmallENG(num, 228 + wide_delta, 76);
        dp_translation = NULL;

        // Armor type | Тип брони
        dp_translation = selective_armortype == 1 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                    cr[CR_WHITE2GREEN_HERETIC];
        M_snprintf(num, 4, "%d", selective_armortype);
        MN_DrTextSmallENG(selective_armortype == 1 ? "1" : "2", 228 + wide_delta, 86);
        dp_translation = NULL;
    }
}

static void M_RD_SelectiveSkill(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_skill > 0)
                selective_skill--;
            break;
        case 1:
            if (selective_skill < 5)
                selective_skill++;
        default:
            break;
    }
}

static void M_RD_SelectiveEpisode(intptr_t option)
{
    // [JN] Shareware have only 1 episode.
    if (gamemode == shareware)
        return;

    switch(option)
    {
        case 0:
            if (selective_episode > 1)
                selective_episode--;
            break;
        case 1:
            if (selective_episode < (gamemode == retail ? 5 : 3))
                selective_episode++;
        default:
            break;
    }
}

static void M_RD_SelectiveMap(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_map > 1)
                selective_map--;
            break;

        case 1:
            if (selective_map < 9)
                selective_map++;
        default:
            break;
    }
}

static void M_RD_SelectiveHealth(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_health > 1)
                selective_health--;
            break;

        case 1:
            if (selective_health < 100)
                selective_health++;
        default:
            break;
    }
}

static void M_RD_SelectiveArmor(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_armor > 0)
                selective_armor--;
            break;
        case 1:
            if (selective_armor < (selective_armortype == 1 ? 100 : 200))
                selective_armor++;
        default:
            break;
    }
}

static void M_RD_SelectiveArmorType(intptr_t option)
{
    selective_armortype++;

    if (selective_armortype > 2)
        selective_armortype = 1;

    // [JN] Silver Shield armor can't go above 100.
    if (selective_armortype == 1 && selective_armor > 100)
        selective_armor = 100;
}

static void M_RD_SelectiveGauntlets(intptr_t option)
{
    selective_wp_gauntlets ^= 1;
}

static void M_RD_SelectiveCrossbow(intptr_t option)
{
    selective_wp_crossbow ^= 1;
}

static void M_RD_SelectiveDragonClaw(intptr_t option)
{
    selective_wp_dragonclaw ^= 1;
}

static void M_RD_SelectiveHellStaff(intptr_t option)
{
    selective_wp_hellstaff ^= 1;
}

static void M_RD_SelectivePhoenixRod(intptr_t option)
{
    selective_wp_phoenixrod ^= 1;
}

static void M_RD_SelectiveFireMace(intptr_t option)
{
    selective_wp_firemace ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect2Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect2Menu(void)
{
    char *title_eng = DEH_String("LEVEL SELECT");
    char *title_rus = DEH_String("DS,JH EHJDYZ");  // ВЫБОР УРОВНЯ
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        //
        // Title
        //
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2
                                       + wide_delta, 4);

        // Bag of Holding
        dp_translation = selective_backpack ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_backpack ? "YES" : "NO", 228 + wide_delta, 26);
        dp_translation = NULL;

        //
        // KEYS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("KEYS"), 74 + wide_delta, 96);
        dp_translation = NULL;

        // Yellow Key
        dp_translation = selective_key_0 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_key_0 ? "YES" : "NO", 228 + wide_delta, 106);
        dp_translation = NULL;

        // Green Key
        dp_translation = selective_key_1 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_key_1 ? "YES" : "NO", 228 + wide_delta, 116);
        dp_translation = NULL;

        // Blue Key
        dp_translation = selective_key_2 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_key_2 ? "YES" : "NO", 228 + wide_delta, 126);
        dp_translation = NULL;

        //
        // MONSTERS
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("MONSTERS"), 74 + wide_delta, 136);
        dp_translation = NULL;

        // Fast Monsters
        dp_translation = selective_fast ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_fast ? "YES" : "NO", 228 + wide_delta, 146);
        dp_translation = NULL;

        // Respawning Monsters
        dp_translation = selective_respawn ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallENG(selective_respawn ? "YES" : "NO", 228 + wide_delta, 156);
        dp_translation = NULL;
    }
    else
    {
        //
        // Title
        //
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2
                                       + wide_delta, 4);

        // Носильный кошель
        dp_translation = selective_backpack ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_backpack ? "LF" : "YTN", 228 + wide_delta, 26);
        dp_translation = NULL;

        //
        // КЛЮЧИ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("RK.XB"), 74 + wide_delta, 96);
        dp_translation = NULL;

        // Желтый ключ
        dp_translation = selective_key_0 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_key_0 ? "LF" : "YTN", 228 + wide_delta, 106);
        dp_translation = NULL;

        // Зеленый ключ
        dp_translation = selective_key_1 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_key_1 ? "LF" : "YTN", 228 + wide_delta, 116);
        dp_translation = NULL;

        // Синий ключ
        dp_translation = selective_key_2 ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_key_2 ? "LF" : "YTN", 228 + wide_delta, 126);
        dp_translation = NULL;

        //
        // МОНСТРЫ
        //
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("VJYCNHS"), 74 + wide_delta, 136);
        dp_translation = NULL;

        // Ускоренные
        dp_translation = selective_fast ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_fast ? "LF" : "YTN", 228 + wide_delta, 146);
        dp_translation = NULL;

        // Воскрешающиеся
        dp_translation = selective_respawn ? cr[CR_WHITE2GREEN_HERETIC] : cr[CR_WHITE2RED_HERETIC];
        MN_DrTextSmallRUS(selective_respawn ? "LF" : "YTN", 228 + wide_delta, 156);
        dp_translation = NULL;
    }

    // The rest of values/placements are same for both languages.
    {
        // Wand Crystals | Кристаллы для жезла
        dp_translation = selective_ammo_0 >=  50 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_0 >=  25 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                   cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_0);
        MN_DrTextSmallENG(num, 228 + wide_delta, 36);
        dp_translation = NULL;

        // Ethereal Arrows | Эфирные стрелы
        dp_translation = selective_ammo_1 >=  25 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_1 >=  12 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                   cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_1);
        MN_DrTextSmallENG(num, 228 + wide_delta, 46);
        dp_translation = NULL;

        // Claw Orbs | Когтевые шары
        dp_translation = selective_ammo_2 >= 100 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_2 >=  50 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                   cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_2);
        MN_DrTextSmallENG(num, 228 + wide_delta, 56);
        dp_translation = NULL;

        // Hellstaff Runes | Руны посоха
        dp_translation = selective_ammo_3 >= 100 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_3 >=  50 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                   cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_3);
        MN_DrTextSmallENG(num, 228 + wide_delta, 66);
        dp_translation = NULL;

        // Flame Orbs | Пламенные шары
        dp_translation = selective_ammo_4 >= 10 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_4 >=  5 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                  cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_4);
        MN_DrTextSmallENG(num, 228 + wide_delta, 76);
        dp_translation = NULL;

        // Mace Spheres | Сферы булавы
        dp_translation = selective_ammo_5 >= 75 ? cr[CR_WHITE2GREEN_HERETIC]    :
                         selective_ammo_5 >= 37 ? cr[CR_WHITE2DARKGOLD_HERETIC] :
                                                  cr[CR_WHITE2RED_HERETIC]      ;
        M_snprintf(num, 4, "%d", selective_ammo_5);
        MN_DrTextSmallENG(num, 228 + wide_delta, 86);
        dp_translation = NULL;
    }
}

static void M_RD_SelectiveBag(intptr_t option)
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

static void M_RD_SelectiveAmmo_0(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_0 > 0)
                selective_ammo_0--;
            break;
        case 1:
            if (selective_ammo_0 < (selective_backpack ? 200 : 100))
                selective_ammo_0++;
        default:
            break;
    }
}

static void M_RD_SelectiveAmmo_1(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_1 > 0)
                selective_ammo_1--;
            break;
        case 1:
            if (selective_ammo_1 < (selective_backpack ? 100 : 50))
                selective_ammo_1++;
        default:
            break;
    }
}

static void M_RD_SelectiveAmmo_2(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_2 > 0)
                selective_ammo_2--;
            break;
        case 1:
            if (selective_ammo_2 < (selective_backpack ? 400 : 200))
                selective_ammo_2++;
        default:
            break;
    }
}

static void M_RD_SelectiveAmmo_3(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_3 > 0)
                selective_ammo_3--;
            break;
        case 1:
            if (selective_ammo_3 < (selective_backpack ? 400 : 200))
                selective_ammo_3++;
        default:
            break;
    }
}

static void M_RD_SelectiveAmmo_4(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_4 > 0)
                selective_ammo_4--;
            break;
        case 1:
            if (selective_ammo_4 < (selective_backpack ? 40 : 20))
                selective_ammo_4++;
        default:
            break;
    }
}

static void M_RD_SelectiveAmmo_5(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_ammo_5 > 0)
                selective_ammo_5--;
            break;
        case 1:
            if (selective_ammo_5 < (selective_backpack ? 300 : 150))
                selective_ammo_5++;
        default:
            break;
    }
}

static void M_RD_SelectiveKey_0(intptr_t option)
{
    selective_key_0 ^= 1;
}

static void M_RD_SelectiveKey_1(intptr_t option)
{
    selective_key_1 ^= 1;
}

static void M_RD_SelectiveKey_2(intptr_t option)
{
    selective_key_2 ^= 1;
}

static void M_RD_SelectiveFast(intptr_t option)
{
    selective_fast ^= 1;
}

static void M_RD_SelectiveRespawn(intptr_t option)
{
    selective_respawn ^= 1;
}

// -----------------------------------------------------------------------------
// DrawLevelSelect3Menu
// -----------------------------------------------------------------------------

static void DrawLevelSelect3Menu(void)
{
    char *title_eng = DEH_String("LEVEL SELECT");
    char *title_rus = DEH_String("DS,JH EHJDYZ");  // ВЫБОР УРОВНЯ
    char  num[4];

    // Draw menu background.
    V_DrawPatchFullScreen(W_CacheLumpName("MENUBG", PU_CACHE), false);

    if (english_language)
    {
        // Title
        MN_DrTextBigENG(title_eng, 160 - MN_DrTextBigENGWidth(title_eng) / 2
                                       + wide_delta, 4);

        // ARTIFACTS
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallENG(DEH_String("ARTIFACTS"), 74 + wide_delta, 26);
        dp_translation = NULL;
    }
    else
    {
        // Title
        MN_DrTextBigRUS(title_rus, 160 - MN_DrTextBigRUSWidth(title_rus) / 2
                                       + wide_delta, 4);

        // АРТЕФАКТЫ
        dp_translation = cr[CR_WHITE2DARKGOLD_HERETIC];
        MN_DrTextSmallRUS(DEH_String("FHNTAFRNS"), 74 + wide_delta, 26);
        dp_translation = NULL;
    }

    // The rest of vanules/placements are same for both languages.
    {
        // Quartz Flask
        dp_translation = selective_arti_0 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_0);
        MN_DrTextSmallENG(num, 228 + wide_delta, 36);
        dp_translation = NULL;

        // Mystic Urn
        dp_translation = selective_arti_1 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_1);
        MN_DrTextSmallENG(num, 228 + wide_delta, 46);
        dp_translation = NULL;

        // Timebomb
        dp_translation = selective_arti_2 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_2);
        MN_DrTextSmallENG(num, 228 + wide_delta, 56);
        dp_translation = NULL;

        // Tome of Power
        dp_translation = selective_arti_3 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_3);
        MN_DrTextSmallENG(num, 228 + wide_delta, 66);
        dp_translation = NULL;

        // Ring of Invincibility
        dp_translation = selective_arti_4 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_4);
        MN_DrTextSmallENG(num, 228 + wide_delta, 76);
        dp_translation = NULL;

        // Morph Ovum
        dp_translation = selective_arti_5 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_5);
        MN_DrTextSmallENG(num, 228 + wide_delta, 86);
        dp_translation = NULL;

        // Chaos Device
        dp_translation = selective_arti_6 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_6);
        MN_DrTextSmallENG(num, 228 + wide_delta, 96);
        dp_translation = NULL;

        // Shadowsphere
        dp_translation = selective_arti_7 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_7);
        MN_DrTextSmallENG(num, 228 + wide_delta, 106);
        dp_translation = NULL;

        // Wings of Wrath
        dp_translation = selective_arti_8 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_8);
        MN_DrTextSmallENG(num, 228 + wide_delta, 116);
        dp_translation = NULL;

        // Torch
        dp_translation = selective_arti_9 ? NULL : cr[CR_WHITE2GRAY_HERETIC];
        M_snprintf(num, 4, "%d", selective_arti_9);
        MN_DrTextSmallENG(num, 228 + wide_delta, 126);
        dp_translation = NULL;
    }
}

static void M_RD_SelectiveArti_0(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_0 > 0)
                selective_arti_0--;
            break;
        case 1:
            if (selective_arti_0 < 16)
                selective_arti_0++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_1(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_1 > 0)
                selective_arti_1--;
            break;
        case 1:
            if (selective_arti_1 < 16)
                selective_arti_1++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_2(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_2 > 0)
                selective_arti_2--;
            break;
        case 1:
            if (selective_arti_2 < 16)
                selective_arti_2++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_3(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_3 > 0)
                selective_arti_3--;
            break;
        case 1:
            if (selective_arti_3 < 16)
                selective_arti_3++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_4(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_4 > 0)
                selective_arti_4--;
            break;
        case 1:
            if (selective_arti_4 < 16)
                selective_arti_4++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_5(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_5 > 0)
                selective_arti_5--;
            break;
        case 1:
            if (selective_arti_5 < 16)
                selective_arti_5++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_6(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_6 > 0)
                selective_arti_6--;
            break;
        case 1:
            if (selective_arti_6 < 16)
                selective_arti_6++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_7(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_7 > 0)
                selective_arti_7--;
            break;
        case 1:
            if (selective_arti_7 < 16)
                selective_arti_7++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_8(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_8 > 0)
                selective_arti_8--;
            break;
        case 1:
            if (selective_arti_8 < 16)
                selective_arti_8++;
        default:
            break;
    }
}

static void M_RD_SelectiveArti_9(intptr_t option)
{
    switch(option)
    {
        case 0:
            if (selective_arti_9 > 0)
                selective_arti_9--;
            break;
        case 1:
            if (selective_arti_9 < 16)
                selective_arti_9++;
        default:
            break;
    }
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

static void M_RD_EndGame(intptr_t option)
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

static void M_RD_ResetSettings(intptr_t option)
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

static void M_RD_ChangeLanguage(intptr_t option)
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
// PROC SCNetCheck_NG_LG //TODO do something with this
//
//---------------------------------------------------------------------------

static void SCNetCheck_NG_LG(Menu_t* menu)
{
    if (!netgame)
    {
        SetMenu(menu); // okay to go into the menu
        return;
    }
    else if(menu == &EpisodeMenu)
    {
        P_SetMessage(&players[consoleplayer], txt_cant_start_in_netgame, msg_system, true);
    }
    else // if(menu == &LoadMenu)
    {
        P_SetMessage(&players[consoleplayer], txt_cant_load_in_netgame, msg_system, true);
    }
    menuactive = false;
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

static void SCQuitGame(intptr_t option)
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

static void SCMessages(intptr_t option)
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

static void SCLoadGame(intptr_t option)
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

static void SCSaveGame(intptr_t option)
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

static void SCEpisode(intptr_t option)
{
    if (gamemode == shareware && option > 1)
    {
        P_SetMessage(&players[consoleplayer], txt_registered_only, msg_system, true);
    }
    else
    {
        MenuEpisode = (int) option;
        SetMenu(&SkillMenu);
    }
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static void SCSkill(intptr_t option)
{
    G_DeferedInitNew(option, MenuEpisode, 1);
    MN_DeactivateMenu();
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static void SCInfo(intptr_t option)
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
    int i;
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
                CurrentMenu = &Options2Menu_Vanilla;
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
        item = (MenuItem_t*) &CurrentMenu->items[CurrentItPos];

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
                SetMenu((Menu_t *) item->option);
            }
            else if (item->func != NULL)
            {
                CurrentMenu->lastOn = CurrentItPos;
                if (item->type == ITT_LRFUNC)
                {
                    item->func(RIGHT_DIR);
                }
                else if (item->type == ITT_EFUNC)
                {
                    item->func(item->option);
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
            if (CurrentMenu->prevMenu == NULL)
            {
                MN_DeactivateMenu();
            }
            else
            {
                SetMenu(CurrentMenu->prevMenu);
            }
            return (true);
        }
        // [JN] Scroll menus by PgUp/PgDn keys
        else if (key == KEY_PGUP)
        {
            // Gameplay features
            if (CurrentMenu == &Gameplay1Menu)
            {
                SetMenu(&Gameplay3Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &Gameplay2Menu)
            {
                SetMenu(&Gameplay1Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &Gameplay3Menu)
            {
                SetMenu(&Gameplay2Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }

            // Level select
            if (CurrentMenu == &LevelSelectMenu1)
            {
                SetMenu(&LevelSelectMenu3);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &LevelSelectMenu2)
            {
                SetMenu(&LevelSelectMenu1);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &LevelSelectMenu3)
            {
                SetMenu(&LevelSelectMenu2);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
        }
        else if (key == KEY_PGDN)
        {
            // Gameplay features
            if (CurrentMenu == &Gameplay1Menu)
            {
                SetMenu(&Gameplay2Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &Gameplay2Menu)
            {
                SetMenu(&Gameplay3Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &Gameplay3Menu)
            {
                SetMenu(&Gameplay1Menu);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }

            // Level select
            if (CurrentMenu == &LevelSelectMenu1)
            {
                SetMenu(&LevelSelectMenu2);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &LevelSelectMenu2)
            {
                SetMenu(&LevelSelectMenu3);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
            if (CurrentMenu == &LevelSelectMenu3)
            {
                SetMenu(&LevelSelectMenu1);
                S_StartSound(NULL,sfx_dorcls);
                return true;
            }
        }
        else if (charTyped != 0)
        {
            // Jump to menu item based on first letter: //TODO add support for russian chars and ability to select not only first found item

            for (i = 0; i < CurrentMenu->itemCount; i++)
            {
                const char *textString = english_language ? CurrentMenu->items[i].text_eng
                                                          : CurrentMenu->items[i].text_rus;
                if (textString)
                {
                    if (toupper(charTyped) == toupper(DEH_String((char*) textString)[0]))
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
            item = (MenuItem_t*) &CurrentMenu->items[CurrentItPos];
            CurrentMenu->lastOn = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                item->func(item->option);
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


//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

static void SetMenu(const Menu_t* const menu)
{
    CurrentMenu->lastOn = CurrentItPos;
    CurrentMenu = (Menu_t*) menu;
    CurrentItPos = CurrentMenu->lastOn;

    // [JN] Force to use vanilla options menu in -vanilla game mode.
    if (vanillaparm)
    {
        if (CurrentMenu == &OptionsMenu)
        {
            CurrentMenu = &OptionsMenu_Vanilla;
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

    x = (english_language ? menu->x_eng : menu->x_rus) + 24;
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
    else if (CurrentMenu == &OptionsMenu_Vanilla && slot > 9)
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

static void DrawSliderSmall(Menu_t * menu, int y, int width, int slot)
{
    int x;
    int x2;
    int count;

    x = (english_language ? menu->x_eng : menu->x_rus) + 24;

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
    else if (CurrentMenu == &ControlsMenu && slot > 11)
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

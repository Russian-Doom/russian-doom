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
void MN_LoadSlotText(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu
// -----------------------------------------------------------------------------

// Rendering
static void DrawRenderingMenu(void);
static boolean M_RD_AspectRatio(int option);
static boolean M_RD_Uncapped(int option);
static boolean M_RD_Smoothing(int option);
static boolean M_RD_Renderer(int option);

// Display
static void DrawDisplayMenu(void);
static boolean M_RD_ScreenSize(int option);
static boolean M_RD_Gamma(int option);
static boolean M_RD_Messages(int option);
static boolean M_RD_LocalTime(int option);

// Sound
static void DrawSoundMenu(void);
static boolean M_RD_SfxVolume(int option);
static boolean M_RD_MusVolume(int option);
static boolean M_RD_SndMode(int option);
static boolean M_RD_PitchShift(int option);

// Controls
static void DrawControlsMenu(void);
static boolean M_RD_AlwaysRun(int option);
static boolean M_RD_MouseLook(int option);
static boolean M_RD_Sensitivity(int option);

// Gameplay (page 1)
static void DrawGameplay1Menu(void);
static boolean M_RD_Brightmaps(int option);
static boolean M_RD_FakeContrast(int option);
static boolean M_RD_ColoredHUD(int option);
static boolean M_RD_ColoredBlood(int option);
static boolean M_RD_InvulSky(int option);
static boolean M_RD_ShadowedText(int option);
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
static boolean M_RD_NoDemos(int option);

// End game
static boolean M_RD_EndGame(int option);

// Reset settings
static boolean M_RD_ResetSettings(int option);

// External Data

extern int detailLevel;

// Public Data

boolean menuactive;
int InfoType;
boolean messageson;

// Private Data

static int FontABaseLump;
static int FontBBaseLump;
static int FontCBaseLump;
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
    {ITT_SETMENU,   "RENDERING",        NULL,           0, MENU_RENDERING   },
    {ITT_SETMENU,   "DISPLAY",          NULL,           0, MENU_DISPLAY     },
    {ITT_SETMENU,   "SOUND",            NULL,           0, MENU_SOUND       },
    {ITT_SETMENU,   "CONTROLS",         NULL,           0, MENU_CONTROLS    },
    {ITT_SETMENU,   "GAMEPLAY",         NULL,           0, MENU_GAMEPLAY1   },
    {ITT_EFUNC,     "END GAME",         M_RD_EndGame,   0, MENU_NONE        },
    {ITT_EFUNC,     "RESET SETTINGS",   M_RD_ResetSettings,0, MENU_NONE     }
};

static MenuItem_t OptionsItems_Rus[] = {
    {ITT_SETMENU,   "DBLTJ",            NULL,           0, MENU_RENDERING   },  // ВИДЕО
    {ITT_SETMENU,   "\'RHFY",           NULL,           0, MENU_DISPLAY     },  // ЭКРАН
    {ITT_SETMENU,   "FELBJ",            NULL,           0, MENU_SOUND       },  // АУДИО
    {ITT_SETMENU,   "EGHFDKTYBT",       NULL,           0, MENU_CONTROLS    },  // УПРАВЛЕНИЕ
    {ITT_SETMENU,   "UTQVGKTQ",         NULL,           0, MENU_GAMEPLAY1   },  // ГЕЙМПЛЕЙ
    {ITT_EFUNC,     "PFRJYXBNM BUHE",   M_RD_EndGame,   0, MENU_NONE        },  // ЗАКОНЧИТЬ ИГРУ
    {ITT_EFUNC,     "C,HJC YFCNHJTR",   M_RD_ResetSettings,0, MENU_NONE     }   // СБРОС НАСТРОЕК
};

static Menu_t OptionsMenu = {
    97 + ORIGWIDTH_DELTA, 16,
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
    {ITT_LRFUNC, "FIX ASPECT RATIO:",   M_RD_AspectRatio,   0,  MENU_NONE},
    {ITT_LRFUNC, "UNCAPPED FPS:",       M_RD_Uncapped,      0,  MENU_NONE},
    {ITT_LRFUNC, "PIXEL SCALING:",      M_RD_Smoothing,     0,  MENU_NONE},
    {ITT_LRFUNC, "VIDEO RENDERER:",     M_RD_Renderer,      0,  MENU_NONE}
};

static MenuItem_t RenderingItems_Rus[] = {
    {ITT_LRFUNC, "ABRCBHJDFNM CJJNYJITYBT CNJHJY:", M_RD_AspectRatio,   0,  MENU_NONE}, // ФИКСИРОВАТЬ СООТНОШЕНИЕ СТОРОН
    {ITT_LRFUNC, "JUHFYBXTYBT RFLHJDJQ XFCNJNS:",   M_RD_Uncapped,      0,  MENU_NONE}, // ОГРАНИЧЕНИЕ КАДРОВОЙ ЧАСТОТЫ
    {ITT_LRFUNC, "GBRCTKMYJT CUKF;BDFYBT:",         M_RD_Smoothing,     0,  MENU_NONE}, // ПИКСЕЛЬНОЕ СГЛАЖИВАНИЕ
    {ITT_LRFUNC, "J,HF,JNRF DBLTJ:",                M_RD_Renderer,      0,  MENU_NONE}  // ОБРАБОТКА ВИДЕО
};

static Menu_t RenderingMenu = {
    66 + ORIGWIDTH_DELTA, 16,
    DrawRenderingMenu,
    4, RenderingItems,
    0,
    MENU_OPTIONS
};

static Menu_t RenderingMenu_Rus = {
    36 + ORIGWIDTH_DELTA, 16,
    DrawRenderingMenu,
    4, RenderingItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_LRFUNC,    "SCREEN SIZE",      M_RD_ScreenSize,0,  MENU_NONE   },
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },
    {ITT_LRFUNC,    "GAMMA-CORRECTION", M_RD_Gamma,     0,  MENU_NONE   },
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },
    {ITT_EFUNC,     "MESSAGES:",        M_RD_Messages,  0,  MENU_NONE   },
    {ITT_EFUNC,     "LOCAL TIME:",      M_RD_LocalTime, 0,  MENU_NONE   }
};

static MenuItem_t DisplayItems_Rus[] = {
    {ITT_LRFUNC,    "HFPVTH \'RHFYF",   M_RD_ScreenSize,0,  MENU_NONE   },      // РАЗМЕР ЭКРАНА
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },      // 
    {ITT_LRFUNC,    "UFVVF-RJHHTRWBZ",  M_RD_Gamma,     0,  MENU_NONE   },      // ГАММА-КОРРЕКЦИЯ
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },      //
    {ITT_EFUNC,     "CJJ,OTYBZ:",       M_RD_Messages,  0,  MENU_NONE   },      // СООБЩЕНИЯ
    {ITT_EFUNC,     "DHTVZ:",           M_RD_LocalTime, 0,  MENU_NONE   }       // ВРЕМЯ
};

static Menu_t DisplayMenu = {
    60 + ORIGWIDTH_DELTA, 16,
    DrawDisplayMenu,
    6, DisplayItems,
    0,
    MENU_OPTIONS
};

static Menu_t DisplayMenu_Rus = {
    60 + ORIGWIDTH_DELTA, 16,
    DrawDisplayMenu,
    6, DisplayItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_LRFUNC,    "SFX VOLUME",       M_RD_SfxVolume, 0,  MENU_NONE   },
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },
    {ITT_LRFUNC,    "MUSIC VOLUME",     M_RD_MusVolume, 0,  MENU_NONE   },
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },
    {ITT_EFUNC,     "SFX MODE:",        M_RD_SndMode,   0,  MENU_NONE   },
    {ITT_EFUNC,     "PITCH-SHIFTING:",  M_RD_PitchShift,0,  MENU_NONE   }
};

static MenuItem_t SoundItems_Rus[] = {
    {ITT_LRFUNC,    "UHJVRJCNM PDERF",  M_RD_SfxVolume, 0,  MENU_NONE   },      // ГРОМКОСТЬ ЗВУКА
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },      //
    {ITT_LRFUNC,    "UHJVRJCNM VEPSRB", M_RD_MusVolume, 0,  MENU_NONE   },      // ГРОМКОСТЬ МУЗЫКИ
    {ITT_EMPTY,     NULL,               NULL,           0,  MENU_NONE   },      //
    {ITT_EFUNC,     "HT;BV PDERF:",     M_RD_SndMode,   0,  MENU_NONE   },      // РЕЖИМ ЗВУКА
    {ITT_EFUNC,     "GBNX-IBANBYU:",    M_RD_PitchShift,0,  MENU_NONE   }       // ПИТЧ-ШИФТИНГ
};

static Menu_t SoundMenu = {
    72 + ORIGWIDTH_DELTA, 16,
    DrawSoundMenu,
    6, SoundItems,
    0,
    MENU_OPTIONS
};

static Menu_t SoundMenu_Rus = {
    72 + ORIGWIDTH_DELTA, 16,
    DrawSoundMenu,
    6, SoundItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_EFUNC,     "ALWAYS RUN:",      M_RD_AlwaysRun,   0,  MENU_NONE   },
    {ITT_EFUNC,     "MOUSE LOOK:",      M_RD_MouseLook,   0,  MENU_NONE   },
    {ITT_LRFUNC,    "MOUSE SENSIVITY",  M_RD_Sensitivity, 0,  MENU_NONE   },
    {ITT_EMPTY,     NULL,               NULL,             0,  MENU_NONE   }
};

static MenuItem_t ControlsItems_Rus[] = {
    {ITT_EFUNC,     "GJCNJZYYSQ ,TU:",  M_RD_AlwaysRun,   0,  MENU_NONE   },    // ПОСТОЯННЫЙ БЕГ
    {ITT_EFUNC,     "J,PJH VSIM.:",     M_RD_MouseLook,   0,  MENU_NONE   },    // ОБЗОР МЫШЬЮ
    {ITT_LRFUNC,    "CRJHJCNM VSIB",    M_RD_Sensitivity, 0,  MENU_NONE   },    // СКОРОСТЬ МЫШИ
    {ITT_EMPTY,     NULL,               NULL,             0,  MENU_NONE   }     //
};

static Menu_t ControlsMenu = {
    88 + ORIGWIDTH_DELTA, 16,
    DrawControlsMenu,
    4, ControlsItems,
    0,
    MENU_OPTIONS
};

static Menu_t ControlsMenu_Rus = {
    68 + ORIGWIDTH_DELTA, 16,
    DrawControlsMenu,
    4, ControlsItems_Rus,
    0,
    MENU_OPTIONS
};

// -----------------------------------------------------------------------------
// Gameplay features (1)
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay1Items[] = {
    {ITT_EFUNC,  "BRIGHTMAPS:",                  M_RD_Brightmaps,     0,  MENU_NONE   },
    {ITT_EFUNC,  "FAKE CONTRAST:",               M_RD_FakeContrast,   0,  MENU_NONE   },
    {ITT_EFUNC,  "COLORED HUD:",                 M_RD_ColoredHUD,     0,  MENU_NONE   },
    {ITT_EFUNC,  "COLORED BLOOD:",               M_RD_ColoredBlood,   0,  MENU_NONE   },
    {ITT_EFUNC,  "INVULNERABILITY AFFECTS SKY:", M_RD_InvulSky,       0,  MENU_NONE   },
    {ITT_EFUNC,  "TEXT CASTS SHADOWS:",          M_RD_ShadowedText,   0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EFUNC,  "SHOW LEVEL STATS ON AUTOMAP:", M_RD_AutoMapStats,   0,  MENU_NONE   },
    {ITT_EFUNC,  "NOTIFY OF REVEALED SECRETS:",  M_RD_SecretNotify,   0,  MENU_NONE   },
    {ITT_EFUNC,  "SHOW NEGATIVE HEALTH:",        M_RD_NegativeHealth, 0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_SETMENU,"NEXT PAGE",                    NULL,                0,  MENU_GAMEPLAY2 }
};

static MenuItem_t Gameplay1Items_Rus[] = {
    {ITT_EFUNC,  ",HFQNVFGGBYU:",                M_RD_Brightmaps,     0,  MENU_NONE   },    // БРАЙТМАППИНГ
    {ITT_EFUNC,  "BVBNFWBZ RJYNHFCNYJCNB:",      M_RD_FakeContrast,   0,  MENU_NONE   },    // ИМИТАЦИЯ КОНТРАСТНОСТИ
    {ITT_EFUNC,  "HFPYJWDTNYST \'KTVTYNS $:",    M_RD_ColoredHUD,     0,  MENU_NONE   },    // РАЗНОЦВЕТНЫЕ ЭЛЕМЕНТЫ HUD
    {ITT_EFUNC,  "HFPYJWDTNYFZ RHJDM:",          M_RD_ColoredBlood,   0,  MENU_NONE   },    // РАЗНОЦВЕТНАЯ КРОВЬ
    {ITT_EFUNC,  "YTEZPDBVJCNM JRHFIBDFTN YT,J:",M_RD_InvulSky,       0,  MENU_NONE   },    // НЕУЯЗВИМОСТЬ ОКРАШИВАЕТ НЕБО
    {ITT_EFUNC,  "NTRCNS JN,HFCSDF.N NTYM:",     M_RD_ShadowedText,   0,  MENU_NONE   },    // ТЕКСТЫ ОТБРАСЫВАЮТ ТЕНЬ
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EFUNC,  "CNFNBCNBRF EHJDYZ YF RFHNT:",  M_RD_AutoMapStats,   0,  MENU_NONE   },    // СТАТИСТИКА УРОВНЯ НА КАРТЕ
    {ITT_EFUNC,  "CJJ,OFNM J YFQLTYYJV NFQYBRT:",M_RD_SecretNotify,   0,  MENU_NONE   },    // СООБЩАТЬ О НАЙДЕННОМ ТАЙНИКЕ
    {ITT_EFUNC,  "JNHBWFNTKMYJT PLJHJDMT D $:",  M_RD_NegativeHealth, 0,  MENU_NONE   },    // ОТРИЦАТЕЛЬНОЕ ЗДОРОВЬЕ В HUD
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_SETMENU,"CKTLE.OFZ CNHFYBWF",           NULL,                0,  MENU_GAMEPLAY2 }  // СЛЕДУЮЩАЯ СТРАНИЦА
};

static Menu_t Gameplay1Menu = {
    53 + ORIGWIDTH_DELTA, 26,
    DrawGameplay1Menu,
    13, Gameplay1Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay1Menu_Rus = {
    53 + ORIGWIDTH_DELTA, 26,
    DrawGameplay1Menu,
    13, Gameplay1Items_Rus,
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
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EFUNC,  "DRAW CROSSHAIR:",              M_RD_CrossHairDraw,  0,  MENU_NONE   },
    {ITT_EFUNC,  "HEALTH INDICATION:",           M_RD_CrossHairHealth,0,  MENU_NONE   },
    {ITT_EFUNC,  "INCREASED SIZE:",              M_RD_CrossHairScale, 0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_EFUNC,  "PLAY INTERNAL DEMOS:",         M_RD_NoDemos,        0,  MENU_NONE   },
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },
    {ITT_SETMENU,"PREVIOUS PAGE",                NULL,                0,  MENU_GAMEPLAY1 }
};

static MenuItem_t Gameplay2Items_Rus[] = {
    {ITT_EFUNC,  "NHEGS CGJKPF.N C DJPDSITYBQ:", M_RD_Torque,         0,  MENU_NONE   },    // ТРУПЫ СПОЛЗАЮТ С ВОЗВЫШЕНИЙ
    {ITT_EFUNC,  "EKEXITYYJT GJRFXBDFYBT JHE;BZ:",M_RD_Bobbing,       0,  MENU_NONE   },    // УЛУЧШЕННОЕ ПОКАЧИВАНИЕ ОРУЖИЯ
    {ITT_EFUNC,  "PTHRFKBHJDFYBT NHEGJD:",       M_RD_FlipCorpses,    0,  MENU_NONE   },    // ЗЕКРАЛИРОВАНИЕ ТРУПОВ
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EFUNC,  "JNJ,HF;FNM GHBWTK:",           M_RD_CrossHairDraw,  0,  MENU_NONE   },    // ОТОБРАЖАТЬ ПРИЦЕЛ
    {ITT_EFUNC,  "BYLBRFWBZ PLJHJDMZ:",          M_RD_CrossHairHealth,0,  MENU_NONE   },    // ИНДИКАЦИЯ ЗДОРОВЬЯ
    {ITT_EFUNC,  "EDTKBXTYYSQ HFPVTH:",          M_RD_CrossHairScale, 0,  MENU_NONE   },    // УВЕЛИЧЕННЫЙ РАЗМЕР
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_EFUNC,  "GHJBUHSDFNM LTVJPFGBCB:",      M_RD_NoDemos,        0,  MENU_NONE   },    // ПРОИГРЫВАТЬ ДЕМОЗАПИСИ
    {ITT_EMPTY,  NULL,                           NULL,                0,  MENU_NONE   },    //
    {ITT_SETMENU,"GHTLSLEOFZ CNHFYBWF",          NULL,                0,  MENU_GAMEPLAY1 }  // ПРЕДЫДУЩАЯ СТРАНИЦА
};

static Menu_t Gameplay2Menu = {
    53 + ORIGWIDTH_DELTA, 26,
    DrawGameplay2Menu,
    13, Gameplay2Items,
    0,
    MENU_OPTIONS
};

static Menu_t Gameplay2Menu_Rus = {
    53 + ORIGWIDTH_DELTA, 26,
    DrawGameplay2Menu,
    13, Gameplay2Items_Rus,
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
        if (english_language)
        {
            EpisodeMenu.itemCount = 5;
            EpisodeMenu.y -= ITEM_HEIGHT;
        }
        else
        {
            EpisodeMenu_Rus.itemCount = 5;
            EpisodeMenu_Rus.y -= ITEM_HEIGHT;
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC InitFonts
//
//---------------------------------------------------------------------------

static void InitFonts(void)
{
    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
    FontBBaseLump = W_GetNumForName(DEH_String("FONTB_S")) + 1;
    FontCBaseLump = W_GetNumForName(DEH_String("FONTC_S")) + 1;
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
        p = W_CacheLumpNum(FontCBaseLump + c - 33, PU_CACHE);
        V_DrawPatch(x, y, p);
        x += SHORT(p->width);
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

            MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2 + ORIGWIDTH_DELTA, 80);
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
                // [JN] Draw a small text instead of big in following menus:
                if (CurrentMenu == &Gameplay1Menu 
                ||  CurrentMenu == &Gameplay2Menu
                ||  CurrentMenu == &RenderingMenu_Rus
                ||  CurrentMenu == &Gameplay1Menu_Rus
                ||  CurrentMenu == &Gameplay2Menu_Rus)
                MN_DrTextA(DEH_String(item->text), x, y);
                else
                MN_DrTextB(DEH_String(item->text), x, y);
            }

            // [JN] Use a different font's vertical spacing in following menus:
            if (CurrentMenu == &Gameplay1Menu
            ||  CurrentMenu == &Gameplay2Menu
            ||  CurrentMenu == &RenderingMenu_Rus
            ||  CurrentMenu == &Gameplay1Menu_Rus
            ||  CurrentMenu == &Gameplay2Menu_Rus)
            y += ITEM_HEIGHT_SMALL;
            else
            y += ITEM_HEIGHT;

            item++;
        }
        
        // [JN] Draw small arrow instead of big in following menus:
        if (CurrentMenu == &Gameplay1Menu
        ||  CurrentMenu == &Gameplay2Menu
        ||  CurrentMenu == &RenderingMenu_Rus
        ||  CurrentMenu == &Gameplay1Menu_Rus
        ||  CurrentMenu == &Gameplay2Menu_Rus)
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT_SMALL) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 8 ? "INVGEMR1" : "INVGEMR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET_SMALL, y,
                                     W_CacheLumpName(selName, PU_CACHE));
        }
        else
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
            selName = DEH_String(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
            V_DrawShadowedPatchRaven(x + SELECTOR_XOFFSET, y,
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
    char *title;

    title = DEH_String(english_language ?
                       "LOAD GAME" :
                       "PFUHEPBNM BUHE");   // ЗАГРУЗИТЬ ИГРУ

    MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2 + ORIGWIDTH_DELTA, 10);
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
    char *title;

    title = DEH_String(english_language ?
                       "SAVE GAME" :    
                       "CJ[HFYBNM BUHE");   // СОХРАНИТЬ ИГРУ

    MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2 + ORIGWIDTH_DELTA, 10);
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
            MN_DrTextA(SlotText[i], x + 5, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}


// -----------------------------------------------------------------------------
// DrawRenderingMenu
// -----------------------------------------------------------------------------

static void DrawRenderingMenu(void)
{
    // Fix aspect ratio:
    if (aspect_ratio_correct)
    {
        if (english_language)
        MN_DrTextB(DEH_String("ON"), 219 + ORIGWIDTH_DELTA, 16);
        else
        MN_DrTextA(DEH_String("DRK"), 270 + ORIGWIDTH_DELTA, 16);
    }
    else
    {
        if (english_language)
        MN_DrTextB(DEH_String("OFF"), 219 + ORIGWIDTH_DELTA, 16);
        else
        MN_DrTextA(DEH_String("DSRK"), 270 + ORIGWIDTH_DELTA, 16);
    }

    // Uncapped FPS:
    if (uncapped_fps)
    {
        if (english_language)
        MN_DrTextB(DEH_String("ON"), 206 + ORIGWIDTH_DELTA, 36);
        else
        MN_DrTextA(DEH_String("DSRK"), 254 + ORIGWIDTH_DELTA, 26);
    }
    else
    {
        if (english_language)
        MN_DrTextB(DEH_String("OFF"), 206 + ORIGWIDTH_DELTA, 36);
        else
        MN_DrTextA(DEH_String("DRK"), 254 + ORIGWIDTH_DELTA, 26);
    }

    // Pixel scaling:
    if (smoothing)
    {
        if (english_language)
        MN_DrTextB(DEH_String("SMOOTH"), 193 + ORIGWIDTH_DELTA, 56);
        else
        MN_DrTextA(DEH_String("DRK"), 211 + ORIGWIDTH_DELTA, 36);
    }
    else
    {
        if (english_language)
        MN_DrTextB(DEH_String("SHARP"), 193 + ORIGWIDTH_DELTA, 56);
        else
        MN_DrTextA(DEH_String("DSRK"), 211 + ORIGWIDTH_DELTA, 36);
    }

    // Video rendered:
    if (force_software_renderer)
    {
        if (english_language)
        MN_DrTextB(DEH_String("CPU"), 216 + ORIGWIDTH_DELTA, 76);
        else
        MN_DrTextA(DEH_String("GHJUHFVVYFZ"), 159 + ORIGWIDTH_DELTA, 46);
    }
    else
    {
        if (english_language)
        MN_DrTextB(DEH_String("GPU"), 216 + ORIGWIDTH_DELTA, 76);
        else
        MN_DrTextA(DEH_String("FGGFHFNYFZ"), 159 + ORIGWIDTH_DELTA, 46);
    }    
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

// -----------------------------------------------------------------------------
// DrawDisplayMenu
// -----------------------------------------------------------------------------

static void DrawDisplayMenu(void)
{
    static char num[4];

    // Screen size
#ifdef WIDESCREEN
    DrawSlider((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 4, screenblocks - 9);
    M_snprintf(num, 4, "%3d", screenblocks);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, (english_language ? 148 : 148) + ORIGWIDTH_DELTA, 41);
    dp_translation = NULL;
#else
    DrawSlider((english_language ? &DisplayMenu : &DisplayMenu_Rus), 1, 10, screenblocks - 3);
    M_snprintf(num, 4, "%3d", screenblocks);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, (english_language ? 196 : 196) + ORIGWIDTH_DELTA, 41);
    dp_translation = NULL;
#endif

    // Gamma-correction
    DrawSlider((english_language ? &DisplayMenu : &DisplayMenu_Rus), 3, 18, usegamma);

    // Messages:
    if (messageson)
    {
        MN_DrTextB(DEH_String(english_language ?  "ON" : "DRK>"),
                             (english_language ? 158 : 185) + ORIGWIDTH_DELTA, 96);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "OFF" : "DSRK>"),
                             (english_language ? 158 : 185) + ORIGWIDTH_DELTA, 96);
    }

    // Local time:
    if (local_time)
    {
        MN_DrTextB(DEH_String(english_language ?  "ON" : "DRK>"),
                             (english_language ? 161 : 134) + ORIGWIDTH_DELTA, 116);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "OFF" : "DSRK>"),
                             (english_language ? 161 : 134) + ORIGWIDTH_DELTA, 116);
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

static boolean M_RD_LocalTime(int option)
{
    local_time ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawSoundMenu
// -----------------------------------------------------------------------------

static void DrawSoundMenu(void)
{
    static char num[4];

    // SFX Volume
    DrawSlider(&SoundMenu, 1, 16, snd_MaxVolume);
    M_snprintf(num, 4, "%3d", snd_MaxVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, 251 + ORIGWIDTH_DELTA, 41);
    dp_translation = NULL;

    // Music Volume
    DrawSlider(&SoundMenu, 3, 16, snd_MusicVolume);
    M_snprintf(num, 4, "%3d", snd_MusicVolume);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, 251 + ORIGWIDTH_DELTA, 81);
    dp_translation = NULL;

    // SFX Mode
    if (snd_monomode)
    {
        MN_DrTextB(DEH_String(english_language ?  "MONO" : "VJYJ"),
                             (english_language ? 169 : 215) + ORIGWIDTH_DELTA, 96);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "STEREO" : "CNTHTJ"),
                             (english_language ? 169 : 215) + ORIGWIDTH_DELTA, 96);
    }    

    // Pitch-shifting
    if (snd_pitchshift)
    {
        MN_DrTextB(DEH_String(english_language ? "ON" : "DRK>"),
                             (english_language ? 208 : 242) + ORIGWIDTH_DELTA, 116);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "OFF" : "DSRK>"),
                             (english_language ? 208 : 242) + ORIGWIDTH_DELTA, 116);
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

static boolean M_RD_SndMode(int option)
{
    snd_monomode ^= 1;
    return true;
}

static boolean M_RD_PitchShift(int option)
{
    snd_pitchshift ^= 1;
    return true;
}

// -----------------------------------------------------------------------------
// DrawControlsMenu
// -----------------------------------------------------------------------------

static void DrawControlsMenu(void)
{
    static char num[4];

    // Mouse sensivity
    DrawSlider((english_language ? &ControlsMenu : &ControlsMenu_Rus), 3, 12, mouseSensitivity);
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    dp_translation = cr[CR_GRAY2GDARKGRAY_HERETIC];
    MN_DrTextA(num, (english_language ? 235 : 215) + ORIGWIDTH_DELTA, 81);
    dp_translation = NULL;

    // Always run
    if (joybspeed >= 20)
    {
        MN_DrTextB(DEH_String(english_language ?  "ON" : "DRK>"),
                             (english_language ? 201 : 246) + ORIGWIDTH_DELTA, 16);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "OFF" : "DSRK>"),
                             (english_language ? 201 : 246) + ORIGWIDTH_DELTA, 16);
    }

    // Mouse look
    if (mlook)
    {
        MN_DrTextB(DEH_String(english_language ?  "ON" : "DRK>"),
                             (english_language ? 208 : 227) + ORIGWIDTH_DELTA, 36);
    }
    else
    {
        MN_DrTextB(DEH_String(english_language ? "OFF" : "DSRK>"),
                             (english_language ? 208 : 227) + ORIGWIDTH_DELTA, 36);
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

static boolean M_RD_MouseLook(int option)
{
    mlook ^= 1;
    if (!mlook)
    players[consoleplayer].centering = true;
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

// -----------------------------------------------------------------------------
// DrawGameplay1Menu
// -----------------------------------------------------------------------------

static void DrawGameplay1Menu(void)
{
    // Subheaders
    dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
    MN_DrTextA(DEH_String(english_language ?  "VISUAL" : "UHFABRF"),
                         (english_language ? 53 : 53) + ORIGWIDTH_DELTA, 16);

    MN_DrTextA(DEH_String(english_language ?  "TACTICAL" : "NFRNBRF"),
                         (english_language ? 53 : 53) + ORIGWIDTH_DELTA, 96);
    dp_translation = NULL;

    // Brightmaps
    if (brightmaps)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 136 : 150) + ORIGWIDTH_DELTA, 26);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 136 : 150) + ORIGWIDTH_DELTA, 26);
        dp_translation = NULL;
    }

    // Fake contrast
    if (fake_contrast)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 160 : 222) + ORIGWIDTH_DELTA, 36);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 160 : 222) + ORIGWIDTH_DELTA, 36);
        dp_translation = NULL;
    }

    // Colored HUD
    if (colored_hud)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 143 : 252) + ORIGWIDTH_DELTA, 46);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 143 : 252) + ORIGWIDTH_DELTA, 46);
        dp_translation = NULL;
    }

    // Colored blood
    if (colored_blood)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 156 : 195) + ORIGWIDTH_DELTA, 56);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 156 : 195) + ORIGWIDTH_DELTA, 56);
        dp_translation = NULL;
    }

    // Invulnerability affects sky
    if (invul_sky)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 252 : 270) + ORIGWIDTH_DELTA, 66);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 252 : 270) + ORIGWIDTH_DELTA, 66);
        dp_translation = NULL;
    }

    // Text casts shadows
    if (draw_shadowed_text)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 197 : 237) + ORIGWIDTH_DELTA, 76);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 197 : 237) + ORIGWIDTH_DELTA, 76);
        dp_translation = NULL;
    }

    // Show level stats on automap
    if (automap_stats)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 257 : 248) + ORIGWIDTH_DELTA, 106);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 257 : 248) + ORIGWIDTH_DELTA, 106);
        dp_translation = NULL;
    }

    // Notify of revealed secrets
    if (secret_notification)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 252 : 268) + ORIGWIDTH_DELTA, 116);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 252 : 268) + ORIGWIDTH_DELTA, 116);
        dp_translation = NULL;
    }

    // Negative health
    if (negative_health)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                            (english_language ? 207 : 270) + ORIGWIDTH_DELTA, 126);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 207 : 270) + ORIGWIDTH_DELTA, 126);
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

static boolean M_RD_ShadowedText(int option)
{
    draw_shadowed_text ^= 1;
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
    // Subheader
    dp_translation = cr[CR_GRAY2DARKGOLD_HERETIC];
    MN_DrTextA(DEH_String(english_language ?  "PHYSICAL" : "ABPBRF"),
                         (english_language ? 53 : 53) + ORIGWIDTH_DELTA, 16);

    MN_DrTextA(DEH_String(english_language ?  "CROSSHAIR" : "GHBWTK"),
                         (english_language ? 53 : 53) + ORIGWIDTH_DELTA, 66);

    MN_DrTextA(DEH_String(english_language ?  "GAMEPLAY" : "UTQVGKTQ"),
                         (english_language ? 53 : 53) + ORIGWIDTH_DELTA, 116);
    dp_translation = NULL;

    // Corpses sliding from the ledges
    if (torque)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 255 : 265) + ORIGWIDTH_DELTA, 26);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 255 : 265) + ORIGWIDTH_DELTA, 26);
        dp_translation = NULL;
    }

    // Weapon bobbing while firing
    if (weapon_bobbing)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 250 : 277) + ORIGWIDTH_DELTA, 36);
        dp_translation = NULL;
    }
    else 
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 250 : 277) + ORIGWIDTH_DELTA, 36);
        dp_translation = NULL;
    }

    // Randomly flipped corpses
    if (randomly_flipcorpses)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 249 : 218) + ORIGWIDTH_DELTA, 46);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 249 : 218) + ORIGWIDTH_DELTA, 46);
        dp_translation = NULL;
    }

    // Draw crosshair
    if (crosshair_draw)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                             (english_language ? 167 : 192) + ORIGWIDTH_DELTA, 76);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 167 : 192) + ORIGWIDTH_DELTA, 76);
        dp_translation = NULL;
    }

    // Health indication
    if (crosshair_health)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                            (english_language ? 178 : 196) + ORIGWIDTH_DELTA, 86);
        dp_translation = NULL;
    }
    else 
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 178 : 196) + ORIGWIDTH_DELTA, 86);
        dp_translation = NULL;
    }

    // Increased size
    if (crosshair_scale)
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                            (english_language ? 163 : 198) + ORIGWIDTH_DELTA, 96);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 163 : 198) + ORIGWIDTH_DELTA, 96);
        dp_translation = NULL;
    }

    // Don't play internal demos
    if (no_internal_demos)
    {
        dp_translation = cr[CR_GRAY2RED_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_OFF : RD_OFF_RUS),
                             (english_language ? 196 : 228) + ORIGWIDTH_DELTA, 126);
        dp_translation = NULL;
    }
    else
    {
        dp_translation = cr[CR_GRAY2GREEN_HERETIC];
        MN_DrTextA(DEH_String(english_language ? RD_ON : RD_ON_RUS),
                            (english_language ? 196 : 228) + ORIGWIDTH_DELTA, 126);
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
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    smoothing               = 0;
    force_software_renderer = 0;

    // Display
    screenblocks    = 10;
    usegamma        = 0;
    messageson      = 1;
    local_time      = 0;

    // Audio
    snd_MaxVolume   = 8;
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu
    snd_MusicVolume = 8;
    S_SetMusicVolume();
    snd_monomode    = 0;

    // Controls
    joybspeed           = 29;
    mlook               = 0;
    players[consoleplayer].centering = true;
    mouseSensitivity    = 5;

    // Gameplay (1)
    brightmaps          = 1;
    fake_contrast       = 0;
    colored_hud         = 0;
    colored_blood       = 1;
    invul_sky           = 1;
    draw_shadowed_text  = 1;
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

    // [JN] TODO: Shareware using paletted screen instead of RAW.
    V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE") + InfoType,
                                   PU_CACHE));
//      V_DrawPatch(0, 0, W_CacheLumpNum(W_GetNumForName("TITLE")+InfoType,
//              PU_CACHE));
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
        V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKD"), PU_CACHE));
    }
    // [JN] Most right position that is "out of bounds" (red gem).
    // Only the mouse sensitivity menu requires this trick.
    else if ((CurrentMenu == &ControlsMenu || CurrentMenu == &ControlsMenu_Rus) && slot > 11)
    {
        slot = 11;
        V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKR"), PU_CACHE));
    }
    // [JN] Standard function (green gem)
    else
    V_DrawPatch(x + 4 + slot * 8, y + 7, W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
}

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#include <stdlib.h>
#include <ctype.h>


#include "doomdef.h"
#include "doomkeys.h"

#include "d_main.h"
#include "deh_main.h"

#include "i_input.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_saves.h"    // [STRIFE]
#include "p_saveg.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"
#include "p_dialog.h"
#include "st_stuff.h"
#include "v_trans.h"
#include "rd_lang.h"
#include "crispy.h"
#include "jn.h"


extern void M_QuitStrife(int);

extern patch_t *hu_font[HU_FONTSIZE];
extern patch_t *hu_font_small_eng[HU_FONTSIZE];
extern patch_t *hu_font_small_rus[HU_FONTSIZE];
extern patch_t *hu_font_big_eng[HU_FONTSIZE];
extern patch_t *hu_font_big_rus[HU_FONTSIZE];


extern boolean          message_dontfuckwithme;

extern boolean          chat_on;        // in heads-up code
extern boolean          sendsave;       // [STRIFE]

// -----------------------------------------------------------------------------
// [JN] Default values
// -----------------------------------------------------------------------------

// Rendering
int screen_wiping = 1;


int			mouseSensitivity = 5;

// [STRIFE]: removed this entirely
// Show messages has default, 0 = off, 1 = on
//int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10; // [STRIFE] default 10, not 9

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;

void    (*messageRoutine)(int response);

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean                 inhelpscreens;
boolean                 menuactive;
boolean                 menupause;      // haleyjd 08/29/10: [STRIFE] New global
int                     menupausetime;  // haleyjd 09/04/10: [STRIFE] New global
boolean                 menuindialog;   // haleyjd 09/04/10: ditto

// haleyjd 08/27/10: [STRIFE] SKULLXOFF == -28, LINEHEIGHT == 19
#define CURSORXOFF     -28
#define LINEHEIGHT      19

#define SKULLXOFF      -33  // [JN] Cursor position for blinking '*' symbol
#define LINEHEIGHT_SML  10  // [JN] Line height for small font

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

// haleyjd 09/04/10: [STRIFE] Moved menuitem / menu structures into header
// because they are needed externally by the dialog engine.

// haleyjd 08/27/10: [STRIFE] skull* stuff changed to cursor* stuff
short		itemOn;			// menu item skull is on
short		cursorAnimCounter;	// skull animation counter
short		whichCursor;		// which skull to draw

// graphic name of cursors
// haleyjd 08/27/10: [STRIFE] M_SKULL* -> M_CURS*
char    *cursorName[8] = {"M_CURS1", "M_CURS2", "M_CURS3", "M_CURS4", 
                          "M_CURS5", "M_CURS6", "M_CURS7", "M_CURS8" };

// haleyjd 20110210 [STRIFE]: skill level for menus
int menuskill;

// current menudef
menu_t*	currentMenu;                          

// haleyjd 03/01/13: [STRIFE] v1.31-only:
// Keeps track of whether the save game menu is being used to name a new
// character slot, or to just save the current game. In the v1.31 disassembly
// this was the new dword_8632C variable.
boolean namingCharacter; 

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_ReadThis3(int choice); // [STRIFE]

//void M_ChangeMessages(int choice); [STRIFE]
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_VoiceVol(int choice); // [STRIFE]
void M_MusicVol(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

//void M_FinishReadThis(int choice); - [STRIFE] unused
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawReadThis3(void); // [STRIFE]
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu prototypes
// -----------------------------------------------------------------------------

// Rendering
void M_RD_Choose_Rendering(int choice);
void M_RD_Draw_Rendering(void);
void M_RD_Change_Widescreen(int choice);
void M_RD_Change_VSync(int choice);
void M_RD_Change_Uncapped(int choice);
void M_RD_Change_FPScounter(int choice);
void M_RD_Change_DiskIcon(int choice);
void M_RD_Change_Smoothing(int choice);
void M_RD_Change_Wiping(int choice);
void M_RD_Change_Screenshots(int choice);
void M_RD_Change_Renderer(int choice);


// =============================================================================
// [JN] MAIN MENU
// =============================================================================

enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

// ------------
// English menu
// ------------

menuitem_t MainMenu[]=
{
    {1, "M_NGAME",  M_NewGame,    'n'},
    {1, "M_OPTION", M_Options,    'o'},
    {1, "M_LOADG",  M_LoadGame,   'l'},
    {1, "M_SAVEG",  M_SaveGame,   's'},
    // Another hickup with Special edition.
    {1, "M_RDTHIS", M_ReadThis,   'h'}, // haleyjd 08/28/10: 'r' -> 'h'
    {1, "M_QUITG",  M_QuitStrife, 'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,45, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};

// ------------
// Russian menu
// ------------

menuitem_t MainMenu_Rus[]=
{
    {1, "RD_NGAME", M_NewGame,    'n'}, // Новая игра
    {1, "RD_OPTN",  M_Options,    'o'}, // Настройки
    {1, "RD_LOADG", M_LoadGame,   'l'}, // Загрузка
    {1, "RD_SAVEG", M_SaveGame,   's'}, // Сохранение
    {1, "RD_INFO",  M_ReadThis,   'h'}, // Помощь!
    {1, "RD_QUITG", M_QuitStrife, 'q'}  // Выход
};

menu_t  MainDef_Rus =
{
    main_end,
    NULL,
    MainMenu_Rus,
    M_DrawMainMenu,
    97,45,
    0
};



// =============================================================================
// [JN] NEW GAME
// =============================================================================

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

// ------------
// English menu
// ------------

menuitem_t NewGameMenu[]=
{
    // haleyjd 08/28/10: [STRIFE] changed all shortcut letters
    {1,"M_JKILL",   M_ChooseSkill, 't'},
    {1,"M_ROUGH",   M_ChooseSkill, 'r'},
    {1,"M_HURT",    M_ChooseSkill, 'v'},
    {1,"M_ULTRA",   M_ChooseSkill, 'e'},
    {1,"M_NMARE",   M_ChooseSkill, 'b'}
};

menu_t  NewDef =
{
    newg_end,           // # of menu items
    &MainDef,           // previous menu - haleyjd [STRIFE] changed to MainDef
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    66,63,              // x,y
    toorough            // lastOn - haleyjd [STRIFE]: default to skill 1
};

// ------------
// Russian menu
// ------------

menuitem_t NewGameMenu_Rus[]=
{
    {1,"RD_JKILL",   M_ChooseSkill, 'h'},  // Разминка
    {1,"RD_ROUGH",   M_ChooseSkill, 'y'},  // Новичок
    {1,"RD_HURT",    M_ChooseSkill, 'd'},  // Ветеран
    {1,"RD_ULTRA",   M_ChooseSkill, '\''}, // Элита
    {1,"RD_NMARE",   M_ChooseSkill, 'r'}   // Кровавая бойня
};

menu_t  NewDef_Rus =
{
    newg_end,
    &MainDef_Rus,
    NewGameMenu_Rus,
    M_DrawNewGame,
    66,63,
    toorough
};


// =============================================================================
// [JN] OPTIONS
// =============================================================================

enum
{
    rd_rendering,
    rd_display,
    rd_sound,
    rd_controls,
    rd_gameplay,
    rd_defaults,
    rd_language,
    rd_end
} options_e;

// ------------
// English menu
// ------------

menuitem_t RD_Options_Menu[]=
{
    {1, "Rendering",        M_RD_Choose_Rendering,  'r'},
    {1, "Display",          0,    'd'},
    {1, "Sound",            0,      's'},
    {1, "Controls",         0,   'c'},
    {1, "Gameplay",         0, 'g'},
    {1, "Reset settings",   0,    'r'},
    {2, "Language:english", 0,    'l'},
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def =
{
    rd_end,
    &MainDef,
    RD_Options_Menu,
    M_DrawOptions,
    54,26,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Options_Menu_Rus[]=
{
    {1, "Dbltj",          M_RD_Choose_Rendering,  'd'}, // Видео
    {1, "\"rhfy",         0,   '\''}, // Экран
    {1, "Felbj",          0,      'f'}, // Аудио
    {1, "Eghfdktybt",     0,   'e'}, // Управление
    {1, "Utqvgktq",       0, 'u'}, // Геймплей
    {1, "C,hjc yfcnhjtr", 0,    'c'}, // Сброс настроек
    {2, "Zpsr#heccrbq",   0,    'z'}, // Язык: русский
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def_Rus =
{
    rd_end, 
    &MainDef_Rus,
    RD_Options_Menu_Rus,
    M_DrawOptions,
    54,26,
    0
};


// =============================================================================
// Read This! MENU 1 & 2 & [STRIFE] 3
// =============================================================================

enum
{
    rdthsempty1,
    read1_end
} read_e;

enum
{
    rdthsempty2,
    read2_end
} read_e2;

// haleyjd 08/28/10: Added Read This! menu 3
enum
{
    rdthsempty3,
    read3_end
} read_e3;

// ----------------
// English menu (1)
// ----------------

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

// ----------------
// Russian menu (1)
// ----------------

menuitem_t ReadMenu1_Rus[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1_Rus =
{
    read1_end,
    &MainDef_Rus,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

// ----------------
// English menu (2)
// ----------------

menuitem_t ReadMenu2[]=
{
    {1,"",M_ReadThis3,0} // haleyjd 08/28/10: [STRIFE] Go to ReadThis3
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    250,185, // haleyjd 08/28/10: [STRIFE] changed coords
    0
};

// ----------------
// Russian menu (2)
// ----------------

menuitem_t ReadMenu2_Rus[]=
{
    {1,"",M_ReadThis3,0}
};

menu_t  ReadDef2_Rus =
{
    read2_end,
    &ReadDef1_Rus,
    ReadMenu2,
    M_DrawReadThis2,
    250,185,
    0
};

// ----------------
// English menu (3)
// ----------------

menuitem_t ReadMenu3[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3 =
{
    read3_end,
    &ReadDef2,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
    0
};

// ----------------
// Russian menu (3)
// ----------------

menuitem_t ReadMenu3_Rus[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3_Rus =
{
    read3_end,
    &ReadDef2_Rus,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
    0
};


// =============================================================================
// RENDERING MENU
// =============================================================================

enum
{
    rd_rendering_widescreen,
    rd_rendering_vsync,
    rd_rendering_uncapped,
    rd_rendering_fps,
    rd_rendering_smoothing,
    rd_rendering_software,
    rd_rendering_empty1,
    rd_rendering_diskicon,
    rd_rendering_wiping,
    rd_rendering_screenshots,
    rd_rendering_end
} rd_rendering_e;

// ------------
// English menu
// ------------

menuitem_t RD_Rendering_Menu[]=
{
    {2, "Display aspect ratio:",     M_RD_Change_Widescreen,  'd'},
    {2, "Vertical synchronization:", M_RD_Change_VSync,       'v'},
    {2, "Frame rate:",               M_RD_Change_Uncapped,    'f'},
    {2, "Show FPS counter:",         M_RD_Change_FPScounter,  's'},
    {2, "Pixel scaling:",            M_RD_Change_Smoothing,   'p'},
    {2, "Video renderer:",           M_RD_Change_Renderer,    'v'},
    {-1,"",0,'\0'},
    {2, "Show hourglass icon:",      M_RD_Change_DiskIcon,    's'},
    {2, "Screen wiping effect:",     M_RD_Change_Wiping,      's'},
    {2, "Screenshot format:",        M_RD_Change_Screenshots, 's'},
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def =
{
    rd_rendering_end,
    &RD_Options_Def,
    RD_Rendering_Menu,
    M_RD_Draw_Rendering,
    38,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Rendering_Menu_Rus[]=
{
    {2, "Cjjnyjitybt cnjhjy \'rhfyf:",     M_RD_Change_Widescreen,  'c'}, // Соотношение сторон экрана
    {2, "Dthnbrfkmyfz cby[hjybpfwbz:",     M_RD_Change_VSync,       'd'}, // Вертикальная синхронизация
    {2, "Rflhjdfz xfcnjnf:",               M_RD_Change_Uncapped,    'r'}, // Кадровая частота
    {2, "Cxtnxbr rflhjdjq xfcnjns:",       M_RD_Change_FPScounter,  'c'}, // Счетчик кадровой частоты
    {2, "Gbrctkmyjt cukf;bdfybt:",         M_RD_Change_Smoothing,   'g'}, // Пиксельное сглаживание
    {2, "J,hf,jnrf dbltj:",                M_RD_Change_Renderer,    'j'}, // Обработка видео
    {-1,"",0,'\0'},                                                       // Дополнительно
    {2, "Pyfxjr gtcjxys[ xfcjd:",          M_RD_Change_DiskIcon,    'p'}, // Значок песочных часов
    {2, "\'aatrn cvtys \'rhfyjd:",         M_RD_Change_Wiping,      '\''}, // Эффект смены экранов
    {2, "Ajhvfn crhbyijnjd:",              M_RD_Change_Screenshots, 'a'}, // Формат скриншотов
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def_Rus =
{
    rd_rendering_end,
    &RD_Options_Def_Rus,
    RD_Rendering_Menu_Rus,
    M_RD_Draw_Rendering,
    11,35,
    0
};



//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    voice_vol,
    sfx_empty3,
    sfx_mouse,
    sfx_empty4,
    sound_end
} sound_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'},
    {2,"M_VOIVOL",M_VoiceVol,'v'}, 
    {-1,"",0,'\0'},
    {2,"M_MSENS",M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'}
};

menu_t  SoundDef =
{
    sound_end,
    &RD_Options_Def,
    SoundMenu,
    M_DrawSound,
    80,35,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};

void M_DrawNameChar(void);

//
// NAME CHARACTER MENU
//
// [STRIFE]
// haleyjd 20110210: New "Name Your Character" Menu
//
menu_t NameCharDef =
{
    load_end,
    &NewDef,
    SaveMenu,
    M_DrawNameChar,
    80,54,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// [STRIFE]
// haleyjd 20110210: Rewritten to read "name" file in each slot directory
//
void M_ReadSaveStrings(void)
{
    FILE *handle;
    int   i;
    char *fname = NULL;

    for(i = 0; i < load_end; i++)
    {
        if(fname)
            Z_Free(fname);
        fname = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(i, "\\name"));

        handle = fopen(fname, "rb");
        if(handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING,
                         sizeof(savegamestrings[i]));
            LoadMenu[i].status = 0;
            continue;
        }
        fread(savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = 1;
    }

    if(fname)
        Z_Free(fname);
}

//
// M_DrawNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for drawing the "Name Your Character" menu.
//
void M_DrawNameChar(void)
{
    int i;

    // "Name Your Character"
    // "Выберите слот сохранения игры"
    M_WriteText(22, 28, DEH_String("ds,thbnt ckjn cj[hfytybz buhs"));

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x-3,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x-4,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x-4 + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_DoNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for items in the "Name Your Character" menu.
//
void M_DoNameChar(int choice)
{
    int map;

    // 20130301: clear naming character flag for 1.31 save logic
    if(gameversion == exe_strife_1_31)
        namingCharacter = false;
    sendsave = 1;
    ClearTmp();
    G_WriteSaveName(choice, savegamestrings[choice]);
    quickSaveSlot = choice;  
    SaveDef.lastOn = choice;
    ClearSlot();
    FromCurr();
    
    if(isdemoversion)
        map = 33;
    else
        map = 2;

    G_DeferedInitNew(menuskill, map);
    M_ClearMenus(0);
}

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;

    V_DrawShadowedPatchStrife(55, 24, 
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x-3,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x-4,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;

    V_DrawShadowedPatchStrife(x - 8, y + 7,
                      W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0;i < 22;i++)
    {
        V_DrawShadowedPatchStrife(x, y + 7,
                          W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawShadowedPatchStrife(x, y + 7, 
                      W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    // [STRIFE]: completely rewritten
    char *name = NULL;

    G_WriteSaveName(choice, savegamestrings[choice]);
    ToCurr();

    // use safe & portable filepath concatenation for Choco
    name = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(choice, ""));

    G_ReadCurrent(name);
    quickSaveSlot = choice;
    M_ClearMenus(0);

    Z_Free(name);
}

//
// Selected from DOOM menu
//
// [STRIFE] Verified unmodified
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(loadnet), NULL, false);
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;

    V_DrawShadowedPatchStrife(53, 24, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x-3,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x-4,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x-4 + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    // [STRIFE]: completely rewritten
    if(slot >= 0)
    {
        sendsave = 1;
        G_WriteSaveName(slot, savegamestrings[slot]);
        M_ClearMenus(0);
        quickSaveSlot = slot;        
        // haleyjd 20130922: slight divergence. We clear the destination slot 
        // of files here, which vanilla did not do. As a result, 1.31 had 
        // broken save behavior to the point of unusability. fraggle agrees 
        // this is detrimental enough to be fixed - unconditionally, for now.
        ClearSlot();        
        FromCurr();
    }
    else
        M_StartMessage(DEH_String(qsavespot), NULL, false);
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    int x, y;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // We need to turn on text input:
    x = LoadDef.x - 11;
    y = LoadDef.y + choice * LINEHEIGHT - 4;
    I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

    // [STRIFE]
    quickSaveSlot = choice;
    //saveSlot = choice;

    M_StringCopy(saveOldString, savegamestrings[choice], sizeof(saveOldString));
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    // [STRIFE]
    if (netgame)
    {
        // haleyjd 20110211: Hooray for Rogue's awesome multiplayer support...
        // Невозможно сохранить сетевую игру
        M_StartMessage(DEH_String("ytdjpvj;yj cj[hfybnm ctntde. buhe"), NULL, false);
        return;
    }
    if (!usergame)
    {
        M_StartMessage(DEH_String(savedead),NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    // [STRIFE]
    if(gameversion == exe_strife_1_31)
    {
        // haleyjd 20130301: in 1.31, we can choose a slot again.
        M_SetupNextMenu(&SaveDef);
        M_ReadSaveStrings();
    }
    else
    {
        // In 1.2 and lower, you save over your character slot exclusively
        M_ReadSaveStrings();
        M_DoSave(quickSaveSlot);
    }
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

void M_QuickSave(void)
{
    if (netgame)
    {
        // haleyjd 20110211 [STRIFE]: More fun...
        // Невозможно сохранить сетевую игру
        M_StartMessage(DEH_String("ytdjpvj;yj cj[hfybnm ctntde. buhe"), NULL, false);
        return;
    }

    if (!usergame)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;	// means to pick a slot now
        return;
    }
    DEH_snprintf(tempstring, 80, qsprompt, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoadResponse
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

//
// M_QuickLoad
//
// [STRIFE] Verified unmodified
//
void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(qloadnet),NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(qsavespot),NULL,false);
        return;
    }
    DEH_snprintf(tempstring, 80, qlprompt, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
// haleyjd 08/28/10: [STRIFE] Draw HELP1, unconditionally.
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatch(0, 0, W_CacheLumpName(DEH_String(english_language ? 
                                                 "HELP1" : "HELP1R"), PU_CACHE));
}



//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] Not optional, draws HELP2
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    V_DrawPatch(0, 0, W_CacheLumpName(DEH_String(english_language ? 
                                                 "HELP2" : "HELP2R"), PU_CACHE));
}


//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] New function to draw HELP3.
//
void M_DrawReadThis3(void)
{
    inhelpscreens = true;
    
    V_DrawPatch(0, 0, W_CacheLumpName(DEH_String(english_language ? 
                                                 "HELP3" : "HELP3R"), PU_CACHE));
}

//
// Change Sfx & Music volumes
//
// haleyjd 08/29/10: [STRIFE]
// * Changed title graphic coordinates
// * Added voice volume and sensitivity sliders
//
void M_DrawSound(void)
{
    V_DrawShadowedPatchStrife (61, 13, W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
                 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
                 16,musicVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(voice_vol+1),
                 16,voiceVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_mouse+1),
                 16,mouseSensitivity);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (sfxVolume)
            sfxVolume--;
        break;
    case 1:
        if (sfxVolume < 15)
            sfxVolume++;
        break;
    }

    S_SetSfxVolume(sfxVolume * 8);
}

//
// M_VoiceVol
//
// haleyjd 08/29/10: [STRIFE] New function
// Sets voice volume level.
//
void M_VoiceVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (voiceVolume)
            voiceVolume--;
        break;
    case 1:
        if (voiceVolume < 15)
            voiceVolume++;
        break;
    }

    S_SetVoiceVolume(voiceVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (musicVolume)
            musicVolume--;
        break;
    case 1:
        if (musicVolume < 15)
            musicVolume++;
        break;
    }

    S_SetMusicVolume(musicVolume);
}




//
// M_DrawMainMenu
//
// haleyjd 08/27/10: [STRIFE] Changed x coordinate; M_DOOM -> M_STRIFE
//
void M_DrawMainMenu(void)
{
    V_DrawShadowedPatchStrife(82, 2,
                      W_CacheLumpName(DEH_String("M_STRIFE"), PU_CACHE));
}




//
// M_NewGame
//
// haleyjd 08/31/10: [STRIFE] Changed M_NEWG -> M_NGAME
//
void M_DrawNewGame(void)
{
    if (english_language)
    {
        V_DrawShadowedPatchStrife(94, 14, W_CacheLumpName(DEH_String("M_NGAME"), PU_CACHE));
        V_DrawShadowedPatchStrife(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
    }
    else
    {
        // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(14, "YJDFZ BUHF");
        // Уровень сложности:
        M_WriteTextBigCentered_RUS(38, "EHJDTYM CKJ;YJCNB#");
    }
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(newgame),NULL,false);
        return;
    }
    // haleyjd 09/07/10: [STRIFE] Removed Chex Quest and DOOM gamemodes
    if(gameversion == exe_strife_1_31)
       namingCharacter = true; // for 1.31 save logic

    M_SetupNextMenu(english_language ? &NewDef : &NewDef_Rus);
}



void M_ChooseSkill(int choice)
{
    // haleyjd 09/07/10: Removed nightmare confirmation
    // [STRIFE]: start "Name Your Character" menu
    menuskill = choice;
    currentMenu = &NameCharDef;
    itemOn = NameCharDef.lastOn;
    M_ReadSaveStrings();
}

/*
// haleyjd [STRIFE] Unused
void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}
*/


//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "OPTIONS");
    }
    else
    {
        // НАСТРОЙКИ
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB");            
    }

    /*
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawShadowedPatchStrife(89, 14, 
                      W_CacheLumpName(DEH_String("M_OPTION"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);
    */
}

void M_Options(int choice)
{
    M_SetupNextMenu(english_language ? &RD_Options_Def : &RD_Options_Def_Rus);
}


// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

void M_RD_Choose_Rendering(int choice)
{
    M_SetupNextMenu(english_language ?
                    &RD_Rendering_Def :
                    &RD_Rendering_Def_Rus);
}

void M_RD_Draw_Rendering(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "RENDERING OPTIONS");

        //
        // Rendering
        //
        dp_translation = cr[CR_GOLD2GRAY_STRIFE];
        M_WriteTextSmall_ENG(37 + wide_delta, 25, "Rendering");
        dp_translation = NULL;

        // Vertical synchronization
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_GOLD2DARKGOLD_STRIFE];
            M_WriteTextSmall_ENG(244 + wide_delta, 45, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(244 + wide_delta, 45, vsync ? "on" : "off");
        }

        // Frame rate
        M_WriteTextSmall_ENG(134 + wide_delta, 55, uncapped_fps ? "uncapped" : "35 fps");

        // Pixel scaling
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_GOLD2DARKGOLD_STRIFE];
            M_WriteTextSmall_ENG(146 + wide_delta, 75, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(146 + wide_delta, 75, smoothing ? "smooth" : "sharp");
        }

        // Video renderer
        M_WriteTextSmall_ENG(164 + wide_delta, 85, force_software_renderer ? "software" : "hardware");

        //
        // Extra
        //
        dp_translation = cr[CR_GOLD2GRAY_STRIFE];
        M_WriteTextSmall_ENG(37 + wide_delta, 95, "Extra");
        dp_translation = NULL;

        // Screen wiping effect
        M_WriteTextSmall_ENG(207 + wide_delta, 115, screen_wiping ? "crossfade" : "off");

        // Screenshot format
        M_WriteTextSmall_ENG(196 + wide_delta, 125, png_screenshots ? "png" : "pcx");
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB DBLTJ"); // НАСТРОЙКИ ВИДЕО

        //
        // Рендеринг
        //
        dp_translation = cr[CR_GOLD2GRAY_STRIFE];
        M_WriteTextSmall_RUS(11 + wide_delta, 25, "htylthbyu");
        dp_translation = NULL;

        // Вертикальная синхронизация
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_GOLD2DARKGOLD_STRIFE];
            M_WriteTextSmall_RUS(254 + wide_delta, 45, "y*l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(254 + wide_delta, 45, vsync ? "drk" : "dsrk");
        }

        // Кадровая частота
        if (uncapped_fps)
        {
            M_WriteTextSmall_RUS(164 + wide_delta, 55, ",tp juhfybxtybz");
        }
        else
        {
            M_WriteTextSmall_ENG(164 + wide_delta, 55, "35 fps");
        }

        // Пиксельное сглаживание
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_GOLD2DARKGOLD_STRIFE];
            M_WriteTextSmall_RUS(220 + wide_delta, 75, "y*l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(220 + wide_delta, 75, smoothing ? "drk" : "dsrk");
        }

        // Обработка видео
        M_WriteTextSmall_RUS(159 + wide_delta, 85, force_software_renderer ? "ghjuhfvvyfz" : "fggfhfnyfz");

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD2GRAY_STRIFE];
        M_WriteTextSmall_RUS(11 + wide_delta, 95, "ljgjkybntkmyj");
        dp_translation = NULL;

        // Эффект смены экранов
        M_WriteTextSmall_RUS(202 + wide_delta, 115, screen_wiping ? "gtht[jl" : "dsrk");

        // Формат скриншотов
        M_WriteTextSmall_ENG(179 + wide_delta, 125, png_screenshots ? "png" : "pcx");
    }
}

void M_RD_Change_Widescreen(int choice)
{
    /*
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in M_Init and stored into config file in M_QuitResponse.
    switch(choice)
    {
        case 0:
        aspect_ratio_temp--;
        if (aspect_ratio_temp < 0)
            aspect_ratio_temp = 3;
        break;

        case 1:
        aspect_ratio_temp++;
        if (aspect_ratio_temp > 3)
            aspect_ratio_temp = 0;
        break;
    }
    */
}

void M_RD_Change_VSync(int choice)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
    return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_RD_Change_Uncapped(int choice)
{
    uncapped_fps ^= 1;
}

void M_RD_Change_FPScounter(int choice)
{
    /*
    show_fps ^= 1;
    */
}

void M_RD_Change_DiskIcon(int choice)
{
    /*
    show_diskicon ^= 1;
    */
}

void M_RD_Change_Smoothing(int choice)
{
    // [JN] Disable "vsync" toggling in sofrware renderer
    if (force_software_renderer == 1)
    return;

    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_RD_Change_Wiping(int choice)
{
    screen_wiping ^= 1;
}

void M_RD_Change_Screenshots(int choice)
{
    png_screenshots ^= 1;
}

void M_RD_Change_Renderer(int choice)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();
}






//
// M_AutoUseHealth
//
// [STRIFE] New function
// haleyjd 20110211: toggle autouse health state
//
void M_AutoUseHealth(void)
{
    if(!netgame && usergame)
    {
        players[consoleplayer].cheats ^= CF_AUTOHEALTH;

        if (english_language)
        {
            if (players[consoleplayer].cheats & CF_AUTOHEALTH)
            players[consoleplayer].message = DEH_String("Auto use health ON");
            else
            players[consoleplayer].message = DEH_String("Auto use health OFF");
        }
        else
        {
            if (players[consoleplayer].cheats & CF_AUTOHEALTH)
            // Автоматическое лечение: включено
            players[consoleplayer].message = DEH_String("fdnjvfnbxtcrjt ktxtybt: drk.xtyj");
            else
            // Автоматическое лечение: выключено
            players[consoleplayer].message = DEH_String("fdnjvfnbxtcrjt ktxtybt: dsrk.xtyj");
        }
    }
}

//
// M_ChangeShowText
//
// [STRIFE] New function
//
void M_ChangeShowText(void)
{
    dialogshowtext ^= true;

    if (english_language)
    {
        if(dialogshowtext)
        players[consoleplayer].message = DEH_String("Conversation Text On");
        else
        players[consoleplayer].message = DEH_String("Conversation Text Off");
    }
    else
    {
        if (dialogshowtext)
        // Текст в диалоговых окнах: включен
        players[consoleplayer].message = DEH_String("ntrcn d lbfkjujds[ jryf[: drk.xty");
        else
        // Текст в диалоговых окнах: выключен
        players[consoleplayer].message = DEH_String("ntrcn d lbfkjujds[ jryf[: dsrk.xty");
    }
}


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (0);
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }

    if (netgame)
    {
        M_StartMessage(DEH_String(NETEND),NULL,false);
        return;
    }

    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &ReadDef1 : &ReadDef1_Rus);
}

//
// M_ReadThis2
//
// haleyjd 08/28/10: [STRIFE] Eliminated DOOM stuff.
//
void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &ReadDef2 : &ReadDef2_Rus);
}

//
// M_ReadThis3
//
// haleyjd 08/28/10: [STRIFE] New function.
//
void M_ReadThis3(int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &ReadDef3 : &ReadDef3_Rus);
}


//
// M_QuitResponse
//
// haleyjd 09/11/10: [STRIFE] Modifications to start up endgame
// demosequence.
//
void M_QuitResponse(int key)
{
    char buffer[20];

    if (key != key_menu_confirm)
        return;

    if(netgame)
        I_Quit();
    else
    {
        DEH_snprintf(buffer, sizeof(buffer), "qfmrm%i", gametic % 8 + 1);
        I_StartVoice(buffer);
        D_QuitGame();
    }
}


//
// M_QuitStrife
//
// [STRIFE] Renamed from M_QuitDOOM
// haleyjd 09/11/10: No randomized text message; that's taken care of
// by the randomized voice message after confirmation.
//
void M_QuitStrife(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring), leave);
    M_StartMessage(endstring, M_QuitResponse, true);
}




void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
    case 0:
        if (mouseSensitivity)
            mouseSensitivity--;
        break;
    case 1:
        if (mouseSensitivity < 9)
            mouseSensitivity++;
        break;
    }
}

// [STRIFE] Verified unmodified
void M_SizeDisplay(int choice)
{
    switch(choice)
    {
    case 0:
        if (screenSize > 0)
        {
            screenblocks--;
            screenSize--;
        }
        break;
    case 1:
        if (screenSize < 8)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawShadowedPatchStrife(xx, yy, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawShadowedPatchStrife(xx, yy, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }
    V_DrawShadowedPatchStrife(xx, yy, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    // [STRIFE] +2 to initial y coordinate
    V_DrawShadowedPatchStrife((x + 8) + thermDot * 8, y + 2,
                      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));
}


// haleyjd: These are from DOOM v0.5 and the prebeta! They drew those ugly red &
// blue checkboxes... preserved for historical interest, as not in Strife.
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 
                      W_CacheLumpName(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1,
                      W_CacheLumpName(DEH_String("M_CELL2"), PU_CACHE));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}



void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    size_t             i;
    int             w = 0;
    int             c;

    for (i = 0;i < strlen(string);i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT (hu_font[c]->width);
    }

    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


// -----------------------------------------------------------------------------
// M_WriteText
//
// Write a string using the hu_font
// haleyjd 09/04/10: [STRIFE]
// * Rogue made a lot of changes to this for the dialog system.
// -----------------------------------------------------------------------------

int M_WriteText (int x, int y, const char *string) // haleyjd: made const for safety w/dialog engine
{
    int	        w;
    const char* ch;
    int         c;
    int         cx;
    int         cy;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;
        if (!c)
            break;

        // haleyjd 09/04/10: [STRIFE] Don't draw spaces at the start of lines.
        if(c == ' ' && cx == x)
            continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11; // haleyjd 09/04/10: [STRIFE]: Changed 12 -> 11
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font[c]->width);

        // haleyjd 09/04/10: [STRIFE] Different linebreak handling
        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawShadowedPatchStrife(cx, cy, hu_font[c]);
            cx += w;
        }
    }

    // [STRIFE] Return final y coordinate.
    return cy + 12;
}

// -----------------------------------------------------------------------------
// M_WriteTextSmall_ENG
//
// [JN] Write a string using a small STCFS font.
// -----------------------------------------------------------------------------

int M_WriteTextSmall_ENG (int x, int y, const char *string)
{
    int	        w, c, cx, cy;
    const char* ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;

        if (!c)
        break;

        if(c == ' ' && cx == x)
        continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_eng[c]->width);

        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawShadowedPatchStrife(cx, cy, hu_font_small_eng[c]);
            cx += w;
        }
    }

    return cy + 12;
}

// -----------------------------------------------------------------------------
// M_WriteTextSmall_RUS
//
// [JN] Write a string using a small STCFS font
// -----------------------------------------------------------------------------

int M_WriteTextSmall_RUS (int x, int y, const char *string)
{
    int	        w, c, cx, cy;
    const char* ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;

        if (!c)
        break;

        if(c == ' ' && cx == x)
        continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_rus[c]->width);

        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawShadowedPatchStrife(cx, cy, hu_font_small_rus[c]);
            cx += w;
        }
    }

    return cy + 12;
}

// -----------------------------------------------------------------------------
// M_WriteTextBig_ENG
//
// [JN] Write a string using a big STCFB font
// -----------------------------------------------------------------------------

int M_WriteTextBig_ENG (int x, int y, const char *string)
{
    int	        w, c, cx, cy;
    const char* ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;

        if (!c)
        break;

        if(c == ' ' && cx == x)
        continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);

        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawShadowedPatchStrife(cx, cy, hu_font_big_eng[c]);
            cx += w;
        }
    }

    return cy + 12;
}

// -----------------------------------------------------------------------------
// M_WriteTextBig_RUS
//
// [JN] Write a string using a big STCFB font
// -----------------------------------------------------------------------------
int M_WriteTextBig_RUS (int x, int y, const char *string)
{
    int	        w, c, cx, cy;
    const char* ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;

        if (!c)
        break;

        if(c == ' ' && cx == x)
        continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11;
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);

        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawShadowedPatchStrife(cx, cy, hu_font_big_rus[c]);
            cx += w;
        }
    }

    return cy + 12;
}

// -----------------------------------------------------------------------------
// M_WriteTextBigCentered_ENG
//
// [JN] Write a centered string using the BIG hu_font_big. Only Y coord is set.
// -----------------------------------------------------------------------------

void M_WriteTextBigCentered_ENG (int y, char *string)
{
    char*   ch;
    int	    c;
    int	    cx;
    int	    cy;
    int	    w;
    int	    width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 10;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);

        V_DrawShadowedPatchStrife(cx, cy, hu_font_big_eng[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextBigCentered_RUS
//
// [JN] Write a centered string using the BIG hu_font_big. Only Y coord is set.
// -----------------------------------------------------------------------------
void M_WriteTextBigCentered_RUS (int y, char *string)
{
    char*   ch;
    int	    c;
    int	    cx;
    int	    cy;
    int	    w;
    int	    width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 10;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);

        V_DrawShadowedPatchStrife(cx, cy, hu_font_big_rus[c]);

        cx+=w;
    }
}













//
// M_DialogDimMsg
//
// [STRIFE] New function
// haleyjd 09/04/10: Painstakingly transformed from the assembly code, as the
// decompiler could not touch it. Redimensions a string to fit on screen, leaving
// at least a 20 pixel margin on the right side. The string passed in must be
// writable.
//
void M_DialogDimMsg(int x, int y, char *str, boolean useyfont)
{
    int rightbound = (ORIGWIDTH - 20) - x;
    patch_t **fontarray;  // ebp
    int linewidth = 0;    // esi
    int i = 0;            // edx
    char *message = str;  // edi
    char  bl;             // bl

    if(useyfont)
       fontarray = yfont;
    else
       fontarray = hu_font;

    bl = toupper(*message);

    if(!bl)
        return;

    // outer loop - run to end of string
    do
    {
        if(bl != '\n')
        {
            int charwidth; // eax
            int tempwidth; // ecx

            if(bl < HU_FONTSTART || bl > HU_FONTEND)
                charwidth = 4;
            else
                charwidth = SHORT(fontarray[bl - HU_FONTSTART]->width);

            tempwidth = linewidth + charwidth;

            // Test if the line still fits within the boundary...
            if(tempwidth >= rightbound)
            {
                // Doesn't fit...
                char *tempptr = &message[i]; // ebx
                char  al;                    // al

                // inner loop - run backward til a space (or the start of the
                // string) is found, subtracting width off the current line.
                // BUG: shouldn't we stop at a previous '\n' too?
                while(*tempptr != ' ' && i > 0)
                {
                    tempptr--;
                    // BUG: they didn't add the first char to linewidth yet...
                    linewidth -= charwidth; 
                    i--;
                    al = toupper(*tempptr);
                    if(al < HU_FONTSTART || al > HU_FONTEND)
                        charwidth = 4;
                    else
                        charwidth = SHORT(fontarray[al - HU_FONTSTART]->width);
                }
                // Replace the space with a linebreak.
                // BUG: what if i is zero? ... infinite loop time!
                message[i] = '\n';
                linewidth = 0;
            }
            else
            {
                // The line does fit.
                // Spaces at the start of a line don't count though.
                if(!(bl == ' ' && linewidth == 0))
                    linewidth += charwidth;
            }
        }
        else
            linewidth = 0; // '\n' seen, so reset the line width
    }
    while((bl = toupper(message[++i])) != 0); // step to the next character
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;

    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;
            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
                mouse_fire_countdown = 5;   // villsa [STRIFE]
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
        }
        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
            }
        }
    }

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch(key)
        {
        case KEY_BACKSPACE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;

        case KEY_ESCAPE:
            saveStringEnter = 0;
            I_StopTextInput();
            M_StringCopy(savegamestrings[quickSaveSlot], saveOldString,
                         sizeof(savegamestrings[quickSaveSlot]));
            break;

        case KEY_ENTER:
            // [STRIFE]
            saveStringEnter = 0;
            I_StopTextInput();
            if(gameversion == exe_strife_1_31 && !namingCharacter)
            {
               // In 1.31, we can be here as a result of normal saving again,
               // whereas in 1.2 this only ever happens when naming your
               // character to begin a new game.
               M_DoSave(quickSaveSlot);
               return true;
            }
            if (savegamestrings[quickSaveSlot][0])
                M_DoNameChar(quickSaveSlot);
            break;

        default:
            // Savegame name entry. This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using ev->data1. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation:
            // instead, use ev->data3 which gives the fully-translated and
            // modified key input.

            if (ev->type != ev_keydown)
            {
                break;
            }

            if (vanilla_keyboard_mapping)
            {
                ch = ev->data1;
            }
            else
            {
                ch = ev->data3;
            }

            ch = toupper(ch);

            if (ch != ' '
                && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                M_StringWidth(savegamestrings[quickSaveSlot]) <
                (SAVESTRINGSIZE-2)*7)
            {
                savegamestrings[quickSaveSlot][saveCharIndex++] = ch;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE
                && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);

        menupause = false;                // [STRIFE] unpause
        menuactive = false;
        S_StartSound(NULL, sfx_mtalht);   // [STRIFE] sound
        return true;
    }

    // [STRIFE]:
    // * In v1.2 this is moved to F9 (quickload)
    // * In v1.31 it is moved to F12 with DM spy, and quicksave
    //   functionality is restored separate from normal saving
    /*
    if (devparm && key == key_menu_help)
    {
        G_ScreenShot ();
        return true;
    }
    */

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(0);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(1);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();
            // haleyjd 08/29/10: [STRIFE] always ReadDef1
            currentMenu = &ReadDef1; 

            itemOn = 0;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // just saving normally, in 1.31

            if(netgame || players[consoleplayer].health <= 0 ||
                players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
            {
                // 1.31: normal save loading
                namingCharacter = false;
                M_StartControlPanel();
                M_LoadGame(0);
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                // Pre 1.31: quickload only
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = sfx_vol;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            //M_ChangeDetail(0);
            M_AutoUseHealth(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // for 1.31 save changes

            if(netgame || players[consoleplayer].health <= 0 ||
               players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuickSave();
            }
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            //M_ChangeMessages(0);
            M_ChangeShowText(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            // [STRIFE]
            // * v1.2: takes a screenshot
            // * v1.31: does quickload again
            if(gameversion == exe_strife_1_31)
            {
                namingCharacter = false;
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            else
                G_ScreenShot();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
            return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
            usegamma++;
            if (usegamma > 17)
                usegamma = 0;

            I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                                             "PALFIX" : "PLAYPAL"), PU_CACHE) +
                                                             st_palette * 768);

            players[consoleplayer].message = DEH_String(english_language ?
                                                        gammamsg[usegamma] : gammamsg_rus[usegamma]);

            return true;
        }
        else if(gameversion == exe_strife_1_31 && key == key_spy)
        {
            // haleyjd 20130301: 1.31 moved screenshots to F12.
            G_ScreenShot();
            return true;
        }
        else if (key != 0 && key == key_menu_screenshot)
        {
            G_ScreenShot();
            return true;
        }
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        return false;
    }

    
    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
                itemOn = 0;
            else itemOn++;
            S_StartSound(NULL, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

        do
        {
            if (!itemOn)
                itemOn = currentMenu->numitems-1;
            else itemOn--;
            S_StartSound(NULL, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                S_StartSound(NULL, sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                //S_StartSound(NULL, sfx_swish); [STRIFE] No sound is played here.
            }
        }
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu
        if(gameversion == exe_strife_1_31) // [STRIFE]: 1.31 saving
            namingCharacter = false;

        if(menuindialog) // [STRIFE] - Get out of dialog engine semi-gracefully
            P_DialogDoChoice(-1);

        currentMenu->lastOn = itemOn;
        M_ClearMenus (0);
        S_StartSound(NULL, sfx_mtalht); // villsa [STRIFE]: sounds
        return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, sfx_swtchn);
        }
        return true;
    }

    // Keyboard shortcut?
    // Vanilla Strife has a weird behavior where it jumps to the scroll bars
    // when certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        // Keyboard shortcut?

        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;
    
    menuactive = 1;
    menupause = true;
    currentMenu = english_language ? &MainDef : &MainDef_Rus;
    itemOn = currentMenu->lastOn;   // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    char               *name;
    int			start;

    inhelpscreens = false;
    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = 100 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start,
                             sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
    }

    if (!menuactive)
        return;

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

        // -----------------------------------------------------------------
        // [JN] Write common menus by using standard graphical patches:
        // -----------------------------------------------------------------
        if (currentMenu == &MainDef                // Main Menu
        ||  currentMenu == &MainDef_Rus            // Main Menu (Russian)
        ||  currentMenu == &NewDef                 // Skill level
        ||  currentMenu == &NewDef_Rus)            // Skill level (Russian)
        {
            // [JN] Draw patch if it's name is present,
            // i.e. don't try to draw placeholders as patches.
            if (name[0])
            V_DrawShadowedPatchStrife(x + wide_delta, y, W_CacheLumpName(name, PU_CACHE));

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }

        // -----------------------------------------------------------------
        // [JN] Write English options menu with big English font
        // -----------------------------------------------------------------
        else
        if (currentMenu == &RD_Options_Def)
        {
            M_WriteTextBig_ENG(x + wide_delta, y, name);

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // -----------------------------------------------------------------
        // [JN] Write Russian options menu with big Russian font
        // -----------------------------------------------------------------
        else 
        if (currentMenu == &RD_Options_Def_Rus)
        {
            M_WriteTextBig_RUS(x + wide_delta, y, name);

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // -----------------------------------------------------------------
        // [JN] Write English submenus with small English font
        // -----------------------------------------------------------------
        else
        if (currentMenu == &RD_Rendering_Def)
        {
            M_WriteTextSmall_ENG(x + wide_delta, y, name);

            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
        // -----------------------------------------------------------------
        // [JN] Write Russian submenus with small Russian font
        // -----------------------------------------------------------------            
        else
        if (currentMenu == &RD_Rendering_Def_Rus)
        {
            M_WriteTextSmall_RUS(x + wide_delta, y, name);
        
            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }

        else
        {
            if (name[0])
            V_DrawShadowedPatchStrife (x-14, y, W_CacheLumpName(name, PU_CACHE));

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }

        
    }

    // [JN] Define where to draw blinking skull and where blinking '*' symbol.
    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
    if (currentMenu == &RD_Rendering_Def   || currentMenu == &RD_Rendering_Def_Rus)
    {
        // Draw blinking '*' symbol
        if (gametic & 8)
        dp_translation = cr[CR_GOLD2DARKGOLD_STRIFE];
        
        M_WriteTextSmall_ENG(x + SKULLXOFF + 24 + wide_delta, 
                             currentMenu->y + itemOn*LINEHEIGHT_SML, "*");
        dp_translation = NULL;
    }
    else
    {
        // Draw Sigil
        V_DrawPatch(x + CURSORXOFF - 7, currentMenu->y - 6 + itemOn*LINEHEIGHT,
                        W_CacheLumpName(DEH_String(cursorName[whichCursor]),
                                        PU_CACHE));
    }
}


//
// M_ClearMenus
//
// haleyjd 08/28/10: [STRIFE] Added an int param so this can be called by menus.
//         09/08/10: Added menupause.
//
void M_ClearMenus (int choice)
{
    choice = 0;     // haleyjd: for no warning; not from decompilation.
    menuactive = 0;
    menupause = 0;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
// haleyjd 08/27/10: [STRIFE] Rewritten for Sigil cursor
//
void M_Ticker (void)
{
    if (--cursorAnimCounter <= 0)
    {
        whichCursor = (whichCursor + 1) % 8;
        cursorAnimCounter = 5;
    }
}


//
// M_Init
//
// haleyjd 08/27/10: [STRIFE] Removed DOOM gamemode stuff
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichCursor = 0;
    cursorAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive; // STRIFE-FIXME: assigns 0 here...
    quickSaveSlot = -1;

    // [STRIFE]: Initialize savegame paths and clear temporary directory
    G_WriteSaveName(5, "ME");
    ClearTmp();

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.
}


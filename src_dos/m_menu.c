//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2018 Julian Nechaevsky
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>


#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"

#include "d_main.h"

#include "i_system.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_misc.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"



extern patch_t*		hu_font[HU_FONTSIZE];
extern boolean		message_dontfuckwithme;

extern boolean		chat_on;		// in heads-up code

extern int st_palette;

//
// defaulted values
//
int			mouseSensitivity;       // has default

// Show messages has default, 0 = off, 1 = on
int			showMessages;
	
int         sfxVolume;
int         musicVolume;

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel;		
int			screenblocks;		// has default

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

// [JN] Additional title "БЫСТРОЕ СОХРАНЕНИЕ"
boolean QuickSaveTitle;   

void    (*messageRoutine)(int response);

#define SAVESTRINGSIZE 	24

char gammamsg[17][29] =
{
    GAMMALVL0,
    GAMMALV02,
    GAMMALV05,
    GAMMALV07,
    GAMMALVL1,
    GAMMALV12,
    GAMMALV15,
    GAMMALV17,
    GAMMALVL2,
    GAMMALV22,
    GAMMALV25,
    GAMMALV27,
    GAMMALVL3,
    GAMMALV32,
    GAMMALV35,
    GAMMALV37,
    GAMMALVL4
};

char endmsg1[NUM_QUITMESSAGES][80] =
{
    // DOOM1
    QUITMSG,
	// Пожалуйста, не уходите! \n Здесь осталось ещё много демонов!
	"gj;fkeqcnf< yt e[jlbnt! \n pltcm jcnfkjcm tot vyjuj ltvjyjd!",
	// Давайте определимся, это будет \n ваша первая и последняя попытка!
	"lfdfqnt jghtltkbvcz< 'nj ,eltn \n dfif gthdfz b gjcktlyzz gjgsnrf!",
	// На Вашем месте я не уходил. \n Работать намного скучнее.
	"yf dfitv vtcnt z ,s yt e[jlbk>\nhf,jnfnm yfvyjuj crexytt>",
	// Вы хотите сказать, что операционная \n система лучше чем я, да?
	"ds [jnbnt crfpfnm< xnj jgthfwbjyyfz \n cbcntvf kexit xtv z< lf?",
	// Не уходите, в дальнем углу \n притаился еще один монстр!
	"yt e[jlbnt< d lfkmytv euke \n ghbnfbkcz tot jlby vjycnh!",
	// Знаете, когда Вы вернётесь, \n Вас будут ждать большие неприятности.
	"pyftnt< rjulf ds dthytntcm< \n dfc ,elen ;lfnm ,jkmibt ytghbznyjcnb>",
	// Давайте, уходите. \n Мне абсолютно всё равно.
	"lfdfqnt< e[jlbnt> \n vyt f,cjk.nyj dct hfdyj>"
};

char endmsg2[NUM_QUITMESSAGES][80] =
{
    // QuitDOOM II messages
    QUITMSG,
    // Всерьез задумали выйти? \n Полагаю, это не очень-то разумно!
    "dcthmtp pflevfkb dsqnb?\ngjkfuf.< 'nj yt jxtym-nj hfpevyj!", 
    // Не уходите, в операционной системе \n вас ждут ужасные демоны!
    "yt e[jlbnt< d jgthfwbjyyjq cbcntvt\ndfc ;len e;fcyst ltvjys!",
    // Отлично, вот и убирайтесь отсюда к \n своим скучным программам!
    "jnkbxyj< djn b e,bhfqntcm jnc.lf\nr cdjbv crexysv ghjuhfvvfv!", 
    // Вам определённо следует \n остаться на сверхурочное время!
    "dfv jghtltktyyj cktletn\njcnfnmcz yf cdth[ehjxyjt dhtvz!",
    // Слушайте, лучше останьтесь, иначе \n может случится что-то страшное.
    "ckeifqnt< kexit jcnfymntcm< byfxt\nvj;tn ckexbnmcz xnj-nj cnhfiyjt>",
    // Просто уходите. когда вернётесь, \n вам крупно не поздоровится.
    "ghjcnj e[jlbnt> rjulf dthytntcm<\ndfv rhegyj yt gjpljhjdbncz>", 
    // Вам очень повезло, что за это \n с вами ничего не случится!
    "dfv jxtym gjdtpkj< xnj pf 'nj \n c dfvb ybxtuj yt ckexbncz!"
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];


//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short	status;
    
    char	name[10];
    
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
} menuitem_t;



typedef struct menu_s
{
    short		numitems;	// # of menu items
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)();	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

char    skullNameUNM[2][9] = {"M_SKUNM1","M_SKUNM2"};

// current menudef
menu_t*	currentMenu;                          

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
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawReadThisRetail(void);
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
void M_WriteText(int x, int y, char *string);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartControlPanel(void);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);




//
// DOOM MENU
//
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

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'r'},
    {1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    ultra_nm,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"M_JKILL",	M_ChooseSkill, 'i'},
    {1,"M_ROUGH",	M_ChooseSkill, 'h'},
    {1,"M_HURT",	M_ChooseSkill, 'h'},
    {1,"M_ULTRA",	M_ChooseSkill, 'u'},
    {1,"M_NMARE",	M_ChooseSkill, 'n'},
    {1,"M_UNMARE",  M_ChooseSkill, 'z'}
};

menu_t  NewDef =
{
    newg_end,		// # of menu items
    &EpiDef,		// previous menu
    NewGameMenu,	// menuitem_t ->
    M_DrawNewGame,	// drawing routine ->
    48,63,              // x,y
    hurtme		// lastOn
};



//
// OPTIONS MENU
//
enum
{
    endgame,
    messages,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_ENDGAM",	M_EndGame,'e'},
    {1,"M_MESSG",	M_ChangeMessages,'m'},
    {1,"M_DETAIL",	M_ChangeDetail,'g'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0},
    {2,"M_MSENS",	M_ChangeSensitivity,'m'},
    {-1,"",0},
    {1,"M_SVOL",	M_Sound,'s'}
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

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
    280,181,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    NULL,
    ReadMenu2,
    M_DrawReadThisRetail,
    330,168,
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
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    88,64,
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
    load7,
    load8,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'},
    {1,"", M_LoadSelect,'7'},
    {1,"", M_LoadSelect,'8'},
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    67,38, // [JN] Отцентрированы и скорректированы поля ввода текста
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
    {1,"", M_SaveSelect,'6'},
    {1,"", M_SaveSelect,'7'},
    {1,"", M_SaveSelect,'8'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    67,38, // [JN] Отцентрированы и скорректированы поля ввода текста
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    int             handle;
    int             count;
    int             i;
    char    name[256];
	
    for (i = 0;i < load_end;i++)
    {
	if (M_CheckParm("-cdrom"))
	    sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",i);
	else
	    sprintf(name,SAVEGAMENAME"%d.dsg",i);

	handle = open (name, O_RDONLY | 0, 0666);
	if (handle == -1)
	{
	    strcpy(&savegamestrings[i][0],EMPTYSTRING);
	    LoadMenu[i].status = 0;
	    continue;
	}
	count = read (handle, &savegamestrings[i], SAVESTRINGSIZE);
	close (handle);
	LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;

    // [JN] Used different title-patch: "ЗАГРУЗИТЬ ИГРУ"
    V_DrawShadowDirect (60,14,0,W_CacheLumpName("M_LGTTL",PU_CACHE));
    V_DrawPatchDirect (59,13,0,W_CacheLumpName("M_LGTTL",PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
    if (!vanilla)
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    else
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y-1+LINEHEIGHT*i);

	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawShadowDirect (x-7,y+9,0,W_CacheLumpName("M_LSLEFT",PU_CACHE));
    V_DrawPatchDirect (x-8,y+8,0,W_CacheLumpName("M_LSLEFT",PU_CACHE));
	
    for (i = 0;i < 24;i++)
    {
    V_DrawShadowDirect (x+1,y+9,0,W_CacheLumpName("M_LSCNTR",PU_CACHE));
	V_DrawPatchDirect (x,y+8,0,W_CacheLumpName("M_LSCNTR",PU_CACHE));
	x += 8;
    }

    V_DrawShadowDirect (x+1,y+9,0,W_CacheLumpName("M_LSRGHT",PU_CACHE));
    V_DrawPatchDirect (x,y+8,0,W_CacheLumpName("M_LSRGHT",PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];
	
    if (M_CheckParm("-cdrom"))
	sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",choice);
    else
	sprintf(name,SAVEGAMENAME"%d.dsg",choice);
    G_LoadGame (name);
    M_ClearMenus ();
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
	M_StartMessage(LOADNET,NULL,false);
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

    if (QuickSaveTitle)
    {
        // [JN] Using title: "БЫСТРОЕ СОХРАНЕНИЕ"
        V_DrawShadowDirect(23,14,0,W_CacheLumpName("M_QSGTTL",PU_CACHE));
        V_DrawPatchDirect (22,13,0,W_CacheLumpName("M_QSGTTL",PU_CACHE));
    }
    else
    {
        // [JN] Using title: "СОХРАНИТЬ ИГРУ"
        V_DrawShadowDirect(58,14,0,W_CacheLumpName("M_SGTTL",PU_CACHE));
        V_DrawPatchDirect (57,13,0,W_CacheLumpName("M_SGTTL",PU_CACHE));
    }

    for (i = 0;i < load_end; i++)
    {
    if (!vanilla)
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    else
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y-1+LINEHEIGHT*i);

	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
	
    if (saveStringEnter)
    {
	i = M_StringWidth(savegamestrings[saveSlot]);
	M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
	quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    saveSlot = choice;
    strcpy(saveOldString,savegamestrings[choice]);
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
	savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(SAVEDEAD,NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int ch)
{
    if (ch == 'y')
    {
	M_DoSave(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}

void M_QuickSave(void)
{
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
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

    if (!vanilla)
    {
        // [JN] Don't ask for overwriting, just save it.
        M_DoSave(quickSaveSlot);
    }
    else
    {
        sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickSaveResponse,true);
    }
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y')
    {
	M_LoadSelect(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
	M_StartMessage(QLOADNET,NULL,false);
	return;
    }
	
    if (quickSaveSlot < 0)
    {
	M_StartMessage(QSAVESPOT,NULL,false);
	return;
    }

    if (!vanilla)
    {
        // [JN] Don't ask for loading, just load it.
        M_LoadSelect(quickSaveSlot);
    }
    else
    {
        sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickLoadResponse,true);        
    }
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;
    V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP2", PU_CACHE));
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThisRetail(void)
{
    inhelpscreens = true;

    if (commercial)
        V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP", PU_CACHE));
    else
        V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP1", PU_CACHE));
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    // [JN] Используется дополнительный заголовок для меню ГРОМКОСТИ.
    V_DrawShadowDirect (91,41,0,W_CacheLumpName("M_SVLTTL",PU_CACHE));
    V_DrawPatchDirect (90,40,0,W_CacheLumpName("M_SVLTTL",PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
		 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
		 16,musicVolume);
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
	
    S_SetMusicVolume(musicVolume * 8);
}




//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    if (tnt)
        V_DrawPatchDirect (94,2,0,W_CacheLumpName("M_DOOMT",PU_CACHE));
    else if (plutonia)
        V_DrawPatchDirect (94,2,0,W_CacheLumpName("M_DOOMP",PU_CACHE));
    else
        V_DrawPatchDirect (94,2,0,W_CacheLumpName("M_DOOM",PU_CACHE));
}




//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawShadowDirect (97,15,0,W_CacheLumpName("M_NEWG",PU_CACHE));
    V_DrawPatchDirect (96,14,0,W_CacheLumpName("M_NEWG",PU_CACHE));

    V_DrawShadowDirect (55,39,0,W_CacheLumpName("M_SKILL",PU_CACHE));
    V_DrawPatchDirect (54,38,0,W_CacheLumpName("M_SKILL",PU_CACHE));
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
	M_StartMessage(NEWGAME,NULL,false);
	return;
    }
	
    if ( commercial )
	M_SetupNextMenu(&NewDef);
    else
	M_SetupNextMenu(&EpiDef);
}


//
//      M_Episode
//
int     epi;

void M_DrawEpisode(void)
{
    V_DrawShadowDirect (97,15,0,W_CacheLumpName("M_NEWG",PU_CACHE));
    V_DrawPatchDirect (96,14,0,W_CacheLumpName("M_NEWG",PU_CACHE));

    V_DrawShadowDirect (55,39,0,W_CacheLumpName("M_EPISOD",PU_CACHE));
    V_DrawPatchDirect (54,38,0,W_CacheLumpName("M_EPISOD",PU_CACHE));
}

void M_VerifyNightmare(int ch)
{
    if (ch != 'y')
	return;
		
    G_DeferedInitNew(nightmare,epi+1,1);
    M_ClearMenus ();
}

void M_VerifyUltraNightmare(int ch)
{
    if (ch != 'y')
    return;

    G_DeferedInitNew(ultra_nm,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
	M_StartMessage(NIGHTMARE,M_VerifyNightmare,true);
	return;
    }

    if (choice == ultra_nm)
    {
	M_StartMessage(ULTRANM,M_VerifyUltraNightmare,true);
	return;
    }
	
    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ( shareware
	 && choice)
    {
	M_StartMessage(SWSTRING,NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}



//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    V_DrawShadowDirect (90,14,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
    V_DrawPatchDirect (89,13,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
	
    V_DrawShadowDirect (OptionsDef.x + 176,OptionsDef.y+1+LINEHEIGHT*detail,0,
		       W_CacheLumpName(detailNames[detailLevel],PU_CACHE));
    V_DrawPatchDirect (OptionsDef.x + 175,OptionsDef.y+LINEHEIGHT*detail,0,
		       W_CacheLumpName(detailNames[detailLevel],PU_CACHE));

    V_DrawShadowDirect (OptionsDef.x + 121,OptionsDef.y+1+LINEHEIGHT*messages,0,
		       W_CacheLumpName(msgNames[showMessages],PU_CACHE));
    V_DrawPatchDirect (OptionsDef.x + 120,OptionsDef.y+LINEHEIGHT*messages,0,
		       W_CacheLumpName(msgNames[showMessages],PU_CACHE));

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(mousesens+1),
		 12,mouseSensitivity);
	
    // [JN] Initially 9. Three new screen sizes for Crispy HUDs.
    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
		 12,screenSize);
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}



//
//      Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;
	
    if (!showMessages)
	players[consoleplayer].message = MSGOFF;
    else
	players[consoleplayer].message = MSGON ;

    message_dontfuckwithme = true;
}


//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
    if (ch != 'y')
	return;
		
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
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
	M_StartMessage(NETEND,NULL,false);
	return;
    }
	
    M_StartMessage(ENDGAME,M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//
int     quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int     quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};



void M_QuitResponse(int ch)
{
    if (ch != 'y')
	return;
    // [JN] No need to play exit sfx if it's volume set to 0.
    if (!netgame && sfxVolume > 0)
    {
	if (commercial)
	    S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
	else
	    S_StartSound(NULL,quitsounds[(gametic>>2)&7]);
	I_WaitVBL(105);
    }
    I_Quit ();
}




void M_QuitDOOM(int choice)
{
  // We pick index 0 which is language sensitive,
  //  or one at random, between 1 and maximum number.
    if (commercial)
    {
        if (french)
        {
            sprintf(endstring, "%s\n\n"DOSY, endmsg2[0]);
        }
        else
        {
            sprintf(endstring, "%s\n\n"DOSY,
                    endmsg2[(gametic >> 2) % NUM_QUITMESSAGES]);
        }
    }
    else
    {
        sprintf(endstring, "%s\n\n"DOSY,
                endmsg1[(gametic >> 2) % NUM_QUITMESSAGES]);
    }
  
  if (devparm) // [JN] Quit immediately
  I_Quit ();
  else
  M_StartMessage(endstring,M_QuitResponse,true);
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
	if (mouseSensitivity < 255) // [crispy] extended range
	    mouseSensitivity++;
	break;
    }
}




void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DETAILHI;
    else
	players[consoleplayer].message = DETAILLO;
}




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
    // [JN] Initially 8. Three new screen sizes for Crispy HUDs.
	if (screenSize < 11)
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
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int		xx;
    int		i;

    xx = x;

    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("M_THERML",PU_CACHE));
    V_DrawPatchDirect (xx,y,0,W_CacheLumpName("M_THERML",PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("M_THERMM",PU_CACHE));
	V_DrawPatchDirect (xx,y,0,W_CacheLumpName("M_THERMM",PU_CACHE));
	xx += 8;
    }
    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("M_THERMR",PU_CACHE));
    V_DrawPatchDirect (xx,y,0,W_CacheLumpName("M_THERMR",PU_CACHE));

    // [crispy] do not crash anymore if value exceeds thermometer range
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        V_DrawPatchDirect ((x+8) + thermDot*8,y,0,W_CacheLumpName("M_THERMW",PU_CACHE));
    }
    else if (thermDot == 0)
    {
        V_DrawPatchDirect ((x+8) + thermDot*8,y,0,W_CacheLumpName("M_THERMD",PU_CACHE));
    }
    else
    {
        V_DrawPatchDirect ((x+8) + thermDot*8,y,0,W_CacheLumpName("M_THERMO",PU_CACHE));
    }


}



// [JN] From Doom Alpha, not needed
/*
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL1",PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL2",PU_CACHE));
}
*/


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
    int             i;
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
    int             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}


//
//      Write a string using the hu_font
//
void
M_WriteText
( int		x,
  int		y,
  char*		string)
{
    int		w;
    char*	ch;
    int		c;
    int		cx;
    int		cy;
		

    ch = string;
    cx = x;
    cy = y;
	
    while(1)
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = x;
	    cy += 12;
	    continue;
	}
		
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c>= HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (cx+w > SCREENWIDTH)
	    break;
    V_DrawShadowDirect(cx+1, cy+1, 0, hu_font[c]);
	V_DrawPatchDirect(cx, cy, 0, hu_font[c]);
	cx+=w;
    }
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
    int             i;
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
    byte*	pal;
	
    ch = -1;
	
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
	if (ev->data3 == -1)
	{
	    ch = KEY_UPARROW;
	    joywait = I_GetTime() + 5;
	}
	else if (ev->data3 == 1)
	{
	    ch = KEY_DOWNARROW;
	    joywait = I_GetTime() + 5;
	}
		
	if (ev->data2 == -1)
	{
	    ch = KEY_LEFTARROW;
	    joywait = I_GetTime() + 2;
	}
	else if (ev->data2 == 1)
	{
	    ch = KEY_RIGHTARROW;
	    joywait = I_GetTime() + 2;
	}
		
	if (ev->data1&1)
	{
	    ch = KEY_ENTER;
	    joywait = I_GetTime() + 5;
	}
	if (ev->data1&2)
	{
	    ch = KEY_BACKSPACE;
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
		ch = KEY_DOWNARROW;
		mousewait = I_GetTime() + 5;
		mousey = lasty -= 30;
	    }
	    else if (mousey > lasty+30)
	    {
		ch = KEY_UPARROW;
		mousewait = I_GetTime() + 5;
		mousey = lasty += 30;
	    }
		
	    mousex += ev->data2;
	    if (mousex < lastx-30)
	    {
		ch = KEY_LEFTARROW;
		mousewait = I_GetTime() + 5;
		mousex = lastx -= 30;
	    }
	    else if (mousex > lastx+30)
	    {
		ch = KEY_RIGHTARROW;
		mousewait = I_GetTime() + 5;
		mousex = lastx += 30;
	    }
		
	    if (ev->data1&1)
	    {
		ch = KEY_ENTER;
		mousewait = I_GetTime() + 15;
	    }
			
	    if (ev->data1&2)
	    {
		ch = KEY_BACKSPACE;
		mousewait = I_GetTime() + 15;
	    }
	}
	else
	    if (ev->type == ev_keydown)
	    {
		ch = ev->data1;
	    }
    }
    
    if (ch == -1)
	return false;

    
    // Save Game string input
    if (saveStringEnter)
    {
	switch(ch)
	{
	  case KEY_BACKSPACE:
	    if (saveCharIndex > 0)
	    {
		saveCharIndex--;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
				
	  case KEY_ESCAPE:
	    saveStringEnter = 0;
	    strcpy(&savegamestrings[saveSlot][0],saveOldString);
	    break;
				
	  case KEY_ENTER:
	    saveStringEnter = 0;
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;
				
	  default:
	    ch = toupper(ch);
	    if (ch != 32)
		if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
		    break;
	    if (ch >= 32 && ch <= 127 &&
		saveCharIndex < SAVESTRINGSIZE-1 &&
		M_StringWidth(savegamestrings[saveSlot]) <
		(SAVESTRINGSIZE-2)*8)
	    {
		savegamestrings[saveSlot][saveCharIndex++] = ch;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
	}
	return true;
    }
    
    // Take care of any messages that need input
    if (messageToPrint)
    {
	if (messageNeedsInput == true &&
	    !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE))
	    return false;
		
	menuactive = messageLastMenuActive;
	messageToPrint = 0;
	if (messageRoutine)
	    messageRoutine(ch);
			
	menuactive = false;
	S_StartSound(NULL,sfx_swtchx);
	return true;
    }
	
    if (devparm && ch == KEY_F1)
    {
	G_ScreenShot ();
	return true;
    }
		
    
    // F-Keys
    if (!menuactive)
	switch(ch)
	{
	  case KEY_MINUS:         // Screen size down
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(0);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
				
	  case KEY_EQUALS:        // Screen size up
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(1);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
				
	  case KEY_F1:            // Help key
	    M_StartControlPanel ();

	    currentMenu = &ReadDef2;
	    
	    itemOn = 0;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEY_F2:            // Save
	    QuickSaveTitle = false;
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_SaveGame(0);
	    return true;
				
	  case KEY_F3:            // Load
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_LoadGame(0);
	    return true;
				
	  case KEY_F4:            // Sound Volume
	    M_StartControlPanel ();
	    currentMenu = &SoundDef;
	    itemOn = sfx_vol;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEY_F5:            // Detail toggle
	    M_ChangeDetail(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEY_F6:            // Quicksave
	    QuickSaveTitle = true;
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickSave();
	    return true;
				
	  case KEY_F7:            // End game
	    S_StartSound(NULL,sfx_swtchn);
	    M_EndGame(0);
	    return true;
				
	  case KEY_F8:            // Toggle messages
	    M_ChangeMessages(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEY_F9:            // Quickload
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickLoad();
	    return true;
				
	  case KEY_F10:           // Quit DOOM
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuitDOOM(0);
	    return true;
				
	  case KEY_F11:           // gamma toggle
	    usegamma++;
	    if (usegamma > 16)
		usegamma = 0;
	    players[consoleplayer].message = gammamsg[usegamma];
        pal = (byte *) W_CacheLumpName ("PLAYPAL", PU_CACHE)+st_palette*768;
	    I_SetPalette (pal);
	    return true;
				
	}

    
    // Pop-up menu?
    if (!menuactive)
    {
	if (ch == KEY_ESCAPE)
	{
	    M_StartControlPanel ();
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
	return false;
    }

    
    // Keys usable within menu
    switch (ch)
    {
      case KEY_DOWNARROW:
	do
	{
	    if (itemOn+1 > currentMenu->numitems-1)
		itemOn = 0;
	    else itemOn++;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;
		
      case KEY_UPARROW:
	do
	{
	    if (!itemOn)
		itemOn = currentMenu->numitems-1;
	    else itemOn--;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;

      case KEY_LEFTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
	    currentMenu->menuitems[itemOn].routine(0);
	}
	return true;
		
      case KEY_RIGHTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
	    currentMenu->menuitems[itemOn].routine(1);
	}
	return true;

      case KEY_ENTER:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
	    currentMenu->lastOn = itemOn;
	    if (currentMenu->menuitems[itemOn].status == 2)
	    {
		currentMenu->menuitems[itemOn].routine(1);      // right arrow
		S_StartSound(NULL,sfx_stnmov);
	    }
	    else
	    {
		currentMenu->menuitems[itemOn].routine(itemOn);
		S_StartSound(NULL,sfx_pistol);
	    }
	}
	return true;
		
      case KEY_ESCAPE:
	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	S_StartSound(NULL,sfx_swtchx);
	return true;
		
      case KEY_BACKSPACE:
	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
	    S_StartSound(NULL,sfx_swtchn);
	}
	return true;
	
      case 0:
	break;
	
      default:
	for (i = itemOn+1;i < currentMenu->numitems;i++)
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
	for (i = 0;i <= itemOn;i++)
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
	break;
	
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
    currentMenu = &MainDef;         // JDC
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
    short		i;
    short		max;
    char		string[40];
    int			start;

    inhelpscreens = false;

    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
	start = 0;
	y = 100 - M_StringHeight(messageString)/2;
	while(*(messageString+start))
	{
	    for (i = 0;i < strlen(messageString+start);i++)
		if (*(messageString+start+i) == '\n')
		{
		    memset(string,0,40);
		    strncpy(string,messageString+start,i);
		    start += i+1;
		    break;
		}
				
	    if (i == strlen(messageString+start))
	    {
		strcpy(string,messageString+start);
		start += i;
	    }
				
	    x = 160 - M_StringWidth(string)/2;
	    M_WriteText(x,y,string);
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
	if (currentMenu->menuitems[i].name[0])
    {
        V_DrawShadowDirect (x+1,y+1,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
	    V_DrawPatchDirect (x,y,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
    }
	y += LINEHEIGHT;
    }

    // DRAW SKULL
    V_DrawShadowDirect(x+1 + SKULLXOFF,currentMenu->y+1 - 5 + itemOn*LINEHEIGHT, 0,
		      W_CacheLumpName(skullName[whichSkull],PU_CACHE));
    V_DrawPatchDirect(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
		      W_CacheLumpName(skullName[whichSkull],PU_CACHE));
}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
    // if (!netgame && usergame && paused)
    //       sendpause = true;
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
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }
}


//
// M_Init
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // [JN] Don't show shareware info screen (HELP2) in non-shareware Doom 1
    // Taken from Chocolate Doom (src/doom/m_menu.c)    
    if (registered || retail)
    {
        MainMenu[readthis].routine = M_ReadThis2;
        ReadDef2.prevMenu = NULL;
    }

    if (commercial)
    {
        MainMenu[readthis] = MainMenu[quitdoom];
        MainDef.numitems--;
        MainDef.y += 8;
        NewDef.prevMenu = &MainDef;
        ReadDef1.routine = M_DrawReadThisRetail;
        ReadDef1.x = 330;
        ReadDef1.y = 165;
        ReadMenu1[0].routine = M_FinishReadThis;
    }

    // We need to remove the fourth episode.
    if (!retail)
        EpiDef.numitems--;
}


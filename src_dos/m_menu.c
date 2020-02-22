//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2019 Julian Nechaevsky
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
#include "v_trans.h"

#include "r_main.h"     // R_ExecuteSetViewSize
#include "s_sound.h"    // S_ChannelsRealloc
#include "st_stuff.h"   // ST_refreshBackground and ST_drawWidgets
#include "jn.h"


extern patch_t*		hu_font[HU_FONTSIZE];
extern patch_t*		hu_font_small[HU_FONTSIZE];
extern patch_t*		hu_font_big[HU_FONTSIZE2];
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

char gammamsg[18][41] =
{
    GAMMA_IMPROVED_OFF,
    GAMMA_IMPROVED_05,
    GAMMA_IMPROVED_1,
    GAMMA_IMPROVED_15,
    GAMMA_IMPROVED_2,
    GAMMA_IMPROVED_25,
    GAMMA_IMPROVED_3,
    GAMMA_IMPROVED_35,
    GAMMA_IMPROVED_4,
    GAMMA_ORIGINAL_OFF,
    GAMMA_ORIGINAL_05,
    GAMMA_ORIGINAL_1,
    GAMMA_ORIGINAL_15,
    GAMMA_ORIGINAL_2,
    GAMMA_ORIGINAL_25,
    GAMMA_ORIGINAL_3,
    GAMMA_ORIGINAL_35,
    GAMMA_ORIGINAL_4
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
#define LINEHEIGHT_SML  10  // [JN] Line height for small font

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
    
    // [JN] Extended from 10 to 128, so long text string may appear
    char	name[128];
    
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


// -----------------------------------------------------------------------------
// [JN] Custom RD menu: font writing prototypes
// -----------------------------------------------------------------------------

void M_WriteText(int x, int y, char *string);
void M_WriteTextSmall(int x, int y, char *string);
void M_WriteTextBig(int x, int y, char *string);
void M_WriteTextBigCentered(int y, char *string);

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

void M_SfxVol(int choice);
void M_MusicVol(int choice);
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


// -----------------------------------------------------------------------------
// [JN] Custom RD menu prototypes
// -----------------------------------------------------------------------------

// Main Options menu
void M_RD_Draw_Options(void);

// Rendering
void M_RD_Choose_Rendering(int choice);
void M_RD_Draw_Rendering(void);
void M_RD_Change_DiskIcon(int choice);
void M_RD_Change_NoFlats(int choice);
void M_RD_Change_Wiping(int choice);

// Display
void M_RD_Choose_Display(int choice);
void M_RD_Draw_Display(void);
void M_RD_Change_ScreenSize(int choice);
void M_RD_Change_Gamma(int choice);
void M_RD_Change_Detail(int choice);
void M_RD_Change_Messages(int choice);

// Sound
void M_RD_Choose_Audio(int choice);
void M_RD_Draw_Audio(void);
void M_RD_Change_SfxVol(int choice);
void M_RD_Change_MusicVol(int choice);
void M_RD_Change_SfxChannels(int choice);
void M_RD_Change_SndMode(int choice);
void M_RD_Change_PitchShifting(int choice);

// Controls
void M_RD_Choose_Controls(int choice);
void M_RD_Draw_Controls(void);
void M_RD_Change_AlwaysRun();
void M_RD_Change_MouseLook(int choice);
void M_RD_Change_Sensitivity(int choice);

// Gameplay
void M_RD_Choose_Gameplay_1(int choice);
void M_RD_Choose_Gameplay_2(int choice);
void M_RD_Choose_Gameplay_3(int choice);
void M_RD_Choose_Gameplay_4(int choice);
void M_RD_Draw_Gameplay_1(void);
void M_RD_Draw_Gameplay_2(void);
void M_RD_Draw_Gameplay_3(void);
void M_RD_Draw_Gameplay_4(void);

void M_RD_Change_Brightmaps(int choice);
void M_RD_Change_FakeContrast(int choice);
void M_RD_Change_Transparency(int choice);
void M_RD_Change_ColoredHUD(int choice);
void M_RD_Change_ColoredBlood(int choice);
void M_RD_Change_SwirlingLiquids(int choice);
void M_RD_Change_InvulSky(int choice);
void M_RD_Change_ShadowedText(int choice);

void M_RD_Change_ExitSfx(int choice);
void M_RD_Change_CrushingSfx(int choice);
void M_RD_Change_BlazingSfx(int choice);
void M_RD_Change_AlertSfx(int choice);
void M_RD_Change_AutoMapStats(int choice);
void M_RD_Change_SecretNotify(int choice);
void M_RD_Change_NegativeHealth(int choice);

void M_RD_Change_WalkOverUnder(int choice);
void M_RD_Change_Torque(int choice);
void M_RD_Change_Bobbing(int choice);
void M_RD_Change_SSGBlast(int choice);
void M_RD_Change_FlipCorpses(int choice);
void M_RD_Change_FloatPowerups(int choice);

void M_RD_Change_CrosshairDraw(int choice);
void M_RD_Change_CrosshairHealth(int choice);

void M_RD_Change_ExtraPlayerFaces(int choice);
void M_RD_Change_LostSoulsQty(int choice);
void M_RD_Change_FastQSaveLoad(int choice);
void M_RD_Change_NoInternalDemos(int choice);

// Back to Defaults
void M_RD_BackToDefaultsResponse(int key);
void M_RD_BackToDefaults(int choice);


// -----------------------------------------------------------------------------
// M_WriteText
//
// Write a string using the hu_font
// -----------------------------------------------------------------------------
void M_WriteText (int x, int y, char *string)
{
    int     w, c, cx, cy;
    char*   ch;

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

// -----------------------------------------------------------------------------
// M_WriteTextSmall
//
// [JN] Write a string using a small STCFS font
// -----------------------------------------------------------------------------

void M_WriteTextSmall (int x, int y, char *string)
{
    int     w, c;
    int     cx = x;
    int     cy = y;
    char   *ch = string;

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

        w = SHORT (hu_font_small[c]->width);
        if (cx+w > SCREENWIDTH)
            break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_small[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_small[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextBig
//
// [JN] Write a string using a big STCFB font
// -----------------------------------------------------------------------------

void M_WriteTextBig (int x, int y, char *string)
{
    int    w, c, cx, cy;
    char  *ch;

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

        c = c - HU_FONTSTART2;
        if (c < 0 || c>= HU_FONTSIZE2)
        {
            cx += 7;
            continue;
        }

        w = SHORT (hu_font_big[c]->width);
        if (cx+w > SCREENWIDTH)
        break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }
}


// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
//
// [JN] Write a centered string using the BIG hu_font_big. Only Y coord is set.
// -----------------------------------------------------------------------------

void M_WriteTextBigCentered (int y, char *string)
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

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big[c]->width);
        width += w;
    }

    // draw it
    cx = SCREENWIDTH/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 10;
            continue;
        }

        w = SHORT (hu_font_big[c]->width);

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big[c]);

        cx+=w;
    }
}


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
    {1,"M_EPI1", M_Episode, 'g'}, // По колено в трупах
    {1,"M_EPI2", M_Episode, 'g'}, // Прибрежье Ада
    {1,"M_EPI3", M_Episode, 'b'}, // Инферно
    {1,"M_EPI4", M_Episode, 'n'}  // Твоя плоть истощена
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


// =============================================================================
// [JN] NEW OPTIONS MENU: STRUCTURE
// =============================================================================


//
// OPTIONS MENU
//
enum
{
    rd_rendering,
    rd_display,
    rd_sound,
    rd_controls,
    rd_gameplay,
    rd_endgame,
    rd_defaults,
    rd_end
} options_e;

menuitem_t RD_Options_Menu[]=
{
    {1,"Dbltj",          M_RD_Choose_Rendering,  'd'},   // Видео
    {1,"\"rhfy",         M_RD_Choose_Display,    '\''},  // Экран
    {1,"Felbj",          M_RD_Choose_Audio,      'f'},   // Аудио
    {1,"Eghfdktybt",     M_RD_Choose_Controls,   'e'},   // Управление
    {1,"Utqvgktq",       M_RD_Choose_Gameplay_1, 'u'},   // Геймплей
    {1,"Pfrjyxbnm buhe", M_EndGame,              'p'},   // Закончить игру
    {1,"C,hjc yfcnhjtr", M_RD_BackToDefaults,    'c'},   // Сброс настроек
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def =
{
    rd_end,
    &MainDef,
    RD_Options_Menu,
    M_RD_Draw_Options,
    60,37,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

enum
{
    rd_rendering_wiping,
    rd_rendering_empty1,
    rd_rendering_diskicon,
    rd_rendering_noflats,
    rd_rendering_end
} rd_rendering_e;

menuitem_t RD_Rendering_Menu[]=
{
    {1,"Ntrcnehs gjkf b gjnjkrf:",   M_RD_Change_NoFlats,  'n'}, // Текстуры пола и потолка
    {-1,"",0,'\0'},
    {1,"Jnj,hf;fnm pyfxjr lbcrtns:", M_RD_Change_DiskIcon, 'j'}, // Отображать значок дискеты
    {1,"Gkfdyfz cvtyf \'rhfyjd:",    M_RD_Change_Wiping,   'g'}, // Плавная смена экранов
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def =
{
    rd_rendering_end,
    &RD_Options_Def,
    RD_Rendering_Menu,
    M_RD_Draw_Rendering,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

enum
{
    rd_display_screensize,
    rd_display_empty1,
    rd_display_gamma,
    rd_display_empty2,
    rd_display_detail,
    rd_display_messages,
    rd_display_end
} rd_display_e;

menuitem_t RD_Display_Menu[]=
{
    {2,"hfpvth buhjdjuj \'rhfyf", M_RD_Change_ScreenSize, 'h'}, // Размер игрового экрана
    {-1,"",0,'\0'},                                             //
    {2,"ehjdtym ufvvf-rjhhtrwbb", M_RD_Change_Gamma,      'e'}, // Уровень гамма-коррекции
    {-1,"",0,'\0'},                                             //
    {1,"ehjdtym ltnfkbpfwbb:",    M_RD_Change_Detail,     'e'}, // Уровень детализации:
    {1,"jnj,hf;tybt cjj,otybq:",  M_RD_Change_Messages,   'j'}, // Отображение сообщений:
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def =
{
    rd_display_end,
    &RD_Options_Def,
    RD_Display_Menu,
    M_RD_Draw_Display,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

enum
{
    rd_audio_sfxvolume,
    rd_audio_empty1,
    rd_audio_musvolume,
    rd_audio_empty2,
    rd_audio_empty3,
    rd_audio_sfxchannels,
    rd_audio_empty4,
    rd_audio_sndmode,
    rd_audio_sndpitch,
    rd_audio_end
} rd_audio_e;

menuitem_t RD_Audio_Menu[]=
{
    {2,"pder",                       M_RD_Change_SfxVol,        'p'}, // Звук
    {-1,"",0,'\0'},                                                   //
    {2,"vepsrf",                     M_RD_Change_MusicVol,      'v'}, // Музыка
    {-1,"",0,'\0'},                                                   //
    {-1,"",0,'\0'},                                                   //
    {2,"Pderjdst rfyfks",            M_RD_Change_SfxChannels,   'p'}, // Звуковые каналы
    {-1,"",0,'\0'},                                                   //
    {1,"Ht;bv pderf:",               M_RD_Change_SndMode,       'h'}, // Режим звука
    {1,"ghjbpdjkmysq gbnx-ibanbyu:", M_RD_Change_PitchShifting, 'g'}, // Произвольный питч-шифтинг
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def =
{
    rd_audio_end,
    &RD_Options_Def,
    RD_Audio_Menu,
    M_RD_Draw_Audio,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

enum
{
    rd_controls_alwaysrun,
    rd_controls_empty1,
    rd_controls_sensitivity,
    rd_controls_empty2,
    rd_controls_mouselook,
    rd_controls_end
} rd_controls_e;

menuitem_t RD_Controls_Menu[]=
{
    {1,"ht;bv gjcnjzyyjuj ,tuf:", M_RD_Change_AlwaysRun,   'g'}, // Режим постоянного бега
    {-1,"",0,'\0'},                                              //
    {2,"Crjhjcnm vsib",           M_RD_Change_Sensitivity, 'c'}, // Скорость мыши
    {-1,"",0,'\0'},                                              //
    {1,"J,pjh vsim.:",            M_RD_Change_MouseLook,   'j'}, // Обзор мышью
    {-1,"",0,'\0'}
};

menu_t  RD_Controls_Def =
{
    rd_controls_end,
    &RD_Options_Def,
    RD_Controls_Menu,
    M_RD_Draw_Controls,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Gameplay enhancements
// -----------------------------------------------------------------------------

enum
{
    rd_gameplay_1_brightmaps,
    rd_gameplay_1_fake_contrast,
    rd_gameplay_1_colored_hud,
    rd_gameplay_1_colored_blood,
    rd_gameplay_1_swirling_liquids,
    rd_gameplay_1_invul_sky,
    rd_gameplay_1_draw_shadowed_text,
    rd_gameplay_1_empty1,
    rd_gameplay_1_empty2,
    rd_gameplay_1_empty3,
    rd_gameplay_1_next_page,
    rd_gameplay_1_last_page,
    rd_gameplay_1_end
} rd_gameplay_1_e;

enum
{
    rd_gameplay_2_play_exit_sfx,
    rd_gameplay_2_crushed_corpses_sfx,
    rd_gameplay_2_blazing_door_fix_sfx,
    rd_gameplay_2_noise_alert_sfx,
    rd_gameplay_2_empty1,
    rd_gameplay_2_automap_stats,
    rd_gameplay_2_secret_notification,
    rd_gameplay_2_negative_health,
    rd_gameplay_2_empty2,
    rd_gameplay_2_empty3,
    rd_gameplay_2_next_page,
    rd_gameplay_2_prev_page,
    rd_gameplay_2_end
} rd_gameplay_2_e;

enum
{
    rd_gameplay_3_over_under,
    rd_gameplay_3_torque,
    rd_gameplay_3_weapon_bobbing,
    rd_gameplay_3_ssg_blast_enemies,
    rd_gameplay_3_randomly_flipcorpses,
    rd_gameplay_3_floating_powerups,
    rd_gameplay_3_empty1,
    rd_gameplay_3_crosshair_draw,
    rd_gameplay_3_crosshair_health,
    rd_gameplay_3_next_page,
    rd_gameplay_3_prev_page,
    rd_gameplay_3_end
} rd_gameplay_3_e;

enum
{
    rd_gameplay_4_extra_player_faces,
    rd_gameplay_4_unlimited_lost_souls,
    rd_gameplay_4_fast_quickload,
    rd_gameplay_4_no_internal_demos,
    rd_gameplay_4_empty1,
    rd_gameplay_4_empty2,
    rd_gameplay_4_empty3,
    rd_gameplay_4_empty4,
    rd_gameplay_4_first_page,
    rd_gameplay_4_prev_page,
    rd_gameplay_4_end
} rd_gameplay_4_e;

menuitem_t RD_Gameplay_Menu_1[]=
{
    {1,",hfqnvfggbyu:",                     M_RD_Change_Brightmaps,     ','},   // Брайтмаппинг
    {1,"Bvbnfwbz rjynhfcnyjcnb:",           M_RD_Change_FakeContrast,   'b'},   // Имитация контрастности
    {1,"Hfpyjwdtnyst 'ktvtyns $:",          M_RD_Change_ColoredHUD,     'h'},   // Разноцветные элементы HUD
    {1,"Hfpyjwdtnyfz rhjdm b nhegs:",       M_RD_Change_ColoredBlood,   'h'},   // Разноцветная кровь и трупы
    {1,"ekexityyfz fybvfwbz ;blrjcntq:",    M_RD_Change_SwirlingLiquids,'e'},   // Улучшенная анимация жидкостей
    {1,"ytezpdbvjcnm jrhfibdftn yt,j:",     M_RD_Change_InvulSky,       'y'},   // Неуязвимость окрашивает небо
    {1,"ntrcns jn,hfcsdf.n ntym:",          M_RD_Change_ShadowedText,   'n'},   // Тексты отбрасывают тень
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"",                                  M_RD_Choose_Gameplay_2,     'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_4,     'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_1 =
{
    rd_gameplay_1_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_1,
    M_RD_Draw_Gameplay_1,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_2[]=
{
    {1,"Pderb ghb ds[jlt bp buhs:",     M_RD_Change_ExitSfx,        'p'},   // Звук при выходе из игры
    {1,"Pder hfplfdkbdfybz nhegjd:",    M_RD_Change_CrushingSfx,    'p'},   // Звук раздавливания трупов
    {1,"Jlbyjxysq pder ,scnhjq ldthb:", M_RD_Change_BlazingSfx,     'j'},   // Одиночный звук быстрой двери
    {1,"J,ofz nhtdjuf e vjycnhjd:",     M_RD_Change_AlertSfx,       'j'},   // Общая тревога у монстров
    {-1,"",0,'\0'},                                                         //
    {1,"Cnfnbcnbrf ehjdyz yf rfhnt:",   M_RD_Change_AutoMapStats,   'c'},   // Статистика уровня на карте
    {1,"Cjj,ofnm j yfqltyyjv nfqybrt:", M_RD_Change_SecretNotify,   'c'},   // Сообщать о найденном тайнике
    {1,"jnhbwfntkmyjt pljhjdmt d $:",   M_RD_Change_NegativeHealth, 'j'},   // Отрицательное здоровье в HUD
    {-1,"",0,'\0'},                                                         //
    {-1,"",0,'\0'},                                                         //
    {1,"",                              M_RD_Choose_Gameplay_3,     'l'},   // Далее >
    {1,"",                              M_RD_Choose_Gameplay_1,     'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_2 =
{
    rd_gameplay_2_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_2,
    M_RD_Draw_Gameplay_2,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_3[]=
{
    {1,"Gthtvtotybt gjl/yfl vjycnhfvb:",    M_RD_Change_WalkOverUnder,      'g'},   // Перемещение над/под монстрами
    {1,"Nhegs cgjkpf.n c djpdsitybq:",      M_RD_Change_Torque,             'n'},   // Трупы сползают с возвышений
    {1,"Ekexityyjt gjrfxbdfybt jhe;bz:",    M_RD_Change_Bobbing,            'e'},   // Улучшенное покачивание оружия
    {1,"ldecndjkrf hfphsdftn dhfujd:",      M_RD_Change_SSGBlast,           'l'},   // Двустволка разрывает врагов
    {1,"pthrfkbhjdfybt nhegjd:",            M_RD_Change_FlipCorpses,        'p'},   // Зеркалирование трупов
    {1,"Ktdbnbhe.obt caths-fhntafrns:",     M_RD_Change_FloatPowerups,      'k'},   // Левитирующие сферы-артефакты
    {-1,"",0,'\0'},                                                                 //
    {1,"Jnj,hf;fnm ghbwtk:",                M_RD_Change_CrosshairDraw,      'j'},   // Отображать прицел
    {1,"Bylbrfwbz pljhjdmz:",               M_RD_Change_CrosshairHealth,    'b'},   // Индикация здоровья
    {1,"",                                  M_RD_Choose_Gameplay_4,         'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_2,         'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_3 =
{
    rd_gameplay_3_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_3,
    M_RD_Draw_Gameplay_3,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_4[]=
{
    {1,"Ljgjkybntkmyst kbwf buhjrf:",       M_RD_Change_ExtraPlayerFaces,   'a'},   // Дополнительные лица игрока
    {1,"'ktvtynfkm ,tp juhfybxtybz lei:",   M_RD_Change_LostSoulsQty,       'a'},   // Элементаль без ограничения душ
    {1,"jnrk.xbnm pfghjc ,> pfuheprb:",     M_RD_Change_FastQSaveLoad,      'a'},   // Отключить запрос б. загрузки
    {1,"Ghjbuhsdfnm ltvjpfgbcb:",           M_RD_Change_NoInternalDemos,    'a'},   // Проигрывать демозаписи
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},                                                                 //
    {1,"",                                  M_RD_Choose_Gameplay_1,         'n'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_3,         'p'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_4 =
{
    rd_gameplay_4_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_4,
    M_RD_Draw_Gameplay_4,
    35,45,
    0
};


// =============================================================================
// [JN] NEW OPTIONS MENU: DRAWING
// =============================================================================

// -----------------------------------------------------------------------------
// Main Options menu
// -----------------------------------------------------------------------------

void M_RD_Draw_Options(void)
{
    // Write capitalized title (НАСТРОЙКИ)
    M_WriteTextBigCentered(12, "YFCNHJQRB");
}

// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

void M_RD_Choose_Rendering(int choice)
{
    M_SetupNextMenu(&RD_Rendering_Def);
}

void M_RD_Draw_Rendering(void)
{
    // Write capitalized title (НАСТРОЙКИ ВИДЕО)
    M_WriteTextBigCentered(12, "YFCNHJQRB DBLTJ");

    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "htylthbyu"); // Рендеринг
    dp_translation = NULL;

    M_WriteTextSmall(217, 45, noflats == 1 ? "dsrk" : "drk");

    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 55, "ljgjkybntkmyj"); // Дополнительно
    dp_translation = NULL;

    M_WriteTextSmall(241, 65, show_diskicon == 1 ? "drk" : "dsrk");
    M_WriteTextSmall(204, 75, screen_wiping == 1 ? "drk" : "dsrk");
}

void M_RD_Change_DiskIcon(int choice)
{
    choice = 0;
    show_diskicon = 1 - show_diskicon;
}

void M_RD_Change_NoFlats(int choice)
{
    choice = 0;
    noflats = 1 - noflats;

    // Reinitialize drawing functions
    R_ExecuteSetViewSize();
}

void M_RD_Change_Wiping(int choice)
{
    choice = 0;
    screen_wiping = 1 - screen_wiping;
}

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

void M_RD_Choose_Display(int choice)
{
    M_SetupNextMenu(&RD_Display_Def);
}

void M_RD_Draw_Display(void)
{
    char    num[4];

    // Write capitalized title (НАСТРОЙКИ ЭКРАНА)
    M_WriteTextBigCentered(12, "YFCNHJQRB \"RHFYF");

    //
    // Экран
    //
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "\'rhfy");
    dp_translation = NULL;

    // Draw screen size slider
    M_DrawThermo_Small(35, 55, 12, screenSize);

    
    // Draw numerical representation of slider position
    snprintf(num, 4, "%3d", screenblocks);
    M_WriteText(145, 55, num);

    // Draw gamma-correction slider
    M_DrawThermo_Small(35, 75, 18, usegamma);

    // Write "on" / "off" strings for features
    M_WriteText(193, 85, detailLevel == 1 ? "ybprbq" : "dscjrbq");
    M_WriteText(214, 95, showMessages == 1 ? "drk" : "dsrk");
}

void M_RD_Change_ScreenSize(int choice)
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
        if (screenSize < 11)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}

void M_RD_Change_Gamma(int choice)
{
    switch(choice)
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
    I_SetPalette ((byte *)W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE) + 
                                          st_palette * 768);
    players[consoleplayer].message = gammamsg[usegamma];
}

void M_RD_Change_Detail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    players[consoleplayer].message = detailLevel ? DETAILLO : DETAILHI;
}

void M_RD_Change_Messages(int choice)
{
    choice = 0;
    showMessages = 1 - showMessages;

    players[consoleplayer].message = showMessages ? MSGON : MSGOFF;

    message_dontfuckwithme = true;
}

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------

void M_RD_Choose_Audio(int choice)
{
    M_SetupNextMenu(&RD_Audio_Def);
}

void M_RD_Draw_Audio(void)
{
    char    num[4];

    // Write capitalized title (НАСТРОЙКИ ЗВУКА)
    M_WriteTextBigCentered(12, "YFCNHJQRB PDERF");

    //
    // Громкость
    //
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "uhjvrjcnm");
    dp_translation = NULL;

    // Draw SFX volume slider
    M_DrawThermo_Small(35, 55, 16, sfxVolume);
    // Draw numerical representation of SFX volume
    snprintf(num, 4, "%3d", sfxVolume);
    M_WriteText(177, 55, num);

    // Draw music volume slider
    M_DrawThermo_Small(35, 75, 16, musicVolume);
    // Draw numerical representation of music volume
    snprintf(num, 4, "%3d", musicVolume);
    M_WriteText(177, 75, num);

    //
    // Дополнительно
    //
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 85, "ljgjkybntkmyj");
    dp_translation = NULL;

    // Draw SFX channels slider
    M_DrawThermo_Small(35, 105, 16, numChannels / 4 - 1);
    // Draw numerical representation of channels
    snprintf(num, 4, "%3d", numChannels);
    M_WriteText(177, 105, num);

    // Write "on" / "off" strings for features
    M_WriteText(132, 115, snd_monomode == 1 ? "vjyj" : "cnthtj");
    M_WriteText(242, 125, snd_pitchshift == 1 ? "drk" : "dsrk");
}

void M_RD_Change_SfxVol(int choice)
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

void M_RD_Change_MusicVol(int choice)
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

void M_RD_Change_SfxChannels(int choice)
{
    switch(choice)
    {
        case 0:
        if (numChannels > 4)
            numChannels -= 4;
        break;
    
        case 1:
        if (numChannels < 64)
            numChannels += 4;
        break;
    }

    // Reallocate sound channels
    S_ChannelsRealloc();
}

void M_RD_Change_SndMode(int choice)
{
    choice = 0;
    snd_monomode = 1 - snd_monomode;
}

void M_RD_Change_PitchShifting(int choice)
{
    choice = 0;
    snd_pitchshift = 1 - snd_pitchshift;
}


// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

void M_RD_Choose_Controls(int choice)
{
    M_SetupNextMenu(&RD_Controls_Def);
}

void M_RD_Draw_Controls(void)
{
    char    num[4];

    // Write capitalized title (УПРАВЛЕНИЕ)
    M_WriteTextBigCentered(12, "EGHFDKTYBT");

    //
    // Передвижение
    //
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "gthtldb;tybt");
    dp_translation = NULL;

    // Always run
    M_WriteText(216, 45, joybspeed >= 20 ? "drk" : "dsrk");

    //
    // Мышь
    //
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 55, "vsim");
    dp_translation = NULL;

    // Draw mouse sensivity slider
    M_DrawThermo_Small(35, 75, 13, mouseSensitivity);
    // Draw numerical representation of mouse sensivity
    snprintf(num, 4, "%3d", mouseSensitivity);
    M_WriteText(163, 65, num);


    M_WriteText(135, 85, mlook ? "drk" : "dsrk");
}

void M_RD_Change_AlwaysRun(int choice)
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

void M_RD_Change_MouseLook(int choice)
{
    choice = 0;
    mlook = 1 - mlook;

    if (!mlook)
    players[consoleplayer].centering = true;
}

void M_RD_Change_Sensitivity(int choice)
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

// -----------------------------------------------------------------------------
// Gameplay features
// -----------------------------------------------------------------------------

void M_RD_Choose_Gameplay_1(int choice)
{
    // [JN] Don't allow to enter in -vanilla mode
    if (vanilla)
    return;

    M_SetupNextMenu(&RD_Gameplay_Def_1);
}

void M_RD_Choose_Gameplay_2(int choice)
{
    M_SetupNextMenu(&RD_Gameplay_Def_2);
}

void M_RD_Choose_Gameplay_3(int choice)
{
    M_SetupNextMenu(&RD_Gameplay_Def_3);
}

void M_RD_Choose_Gameplay_4(int choice)
{
    M_SetupNextMenu(&RD_Gameplay_Def_4);
}

void M_RD_Draw_Gameplay_1(void)
{   
    // Write capitalized title (НАСТРОЙКИ ГЕЙМПЛЕЯ)
    M_WriteTextBigCentered(10, "YFCNHJQRB UTQVGKTZ");


    // Write "on" / "off" strings for features
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "uhfabrf");  // Графика
    dp_translation = NULL;

    // Брайтмаппинг
    if (brightmaps) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(140, 45, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(140 , 45, RD_OFF); dp_translation = NULL; }
    // Имитация контрастности
    if (fake_contrast) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(217, 55, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(217, 55, RD_OFF); dp_translation = NULL; }
    // Разноцветные элементы HUD
    if (colored_hud) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(239, 65, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(239, 65, RD_OFF); dp_translation = NULL; }
    // Разноцветная кровь и трупы
    if (colored_blood) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(242, 75, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(242, 75, RD_OFF); dp_translation = NULL; }
    // Улучшенная анимация жидкостей
    if (swirling_liquids) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(275, 85, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(275, 85, RD_OFF); dp_translation = NULL; }
    // Неуязвимость окрашивает небо
    if (invul_sky) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(262, 95, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(262, 95, RD_OFF); dp_translation = NULL; }
    // Тексты отбрасывают тень
     if (draw_shadowed_text) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(226, 105, RD_ON); dp_translation = NULL; }
     else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(226, 105, RD_OFF); dp_translation = NULL; }

    // Footer
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 145, RD_NEXT); 
    M_WriteTextSmall(35, 155, RD_PREV); 
    dp_translation = NULL;
}

void M_RD_Draw_Gameplay_2(void)
{   
    // Write capitalized title (НАСТРОЙКИ ГЕЙМПЛЕЯ)
    M_WriteTextBigCentered(10, "YFCNHJQRB UTQVGKTZ");


    // Write "on" / "off" strings for features
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "Pder");  // Звук
    dp_translation = NULL;

    // Play exit sounds
    if (play_exit_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(225, 45, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(225, 45, RD_OFF); dp_translation = NULL; }
    // Sound of crushing corpses
    if (crushed_corpses_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(236, 55, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(236, 55, RD_OFF); dp_translation = NULL; }
    // Single sound of closing blazing door
    if (blazing_door_fix_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(260, 65, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(260, 65, RD_OFF); dp_translation = NULL; }
    // Monster alert waking up other monsters
    if (noise_alert_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(227, 75,RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(227, 75, RD_OFF); dp_translation = NULL; }

    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 85, "Nfrnbrf"); // Тактика
    dp_translation = NULL;

    // Show level stats on automap
    if (automap_stats) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(239, 95, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(239, 95, RD_OFF); dp_translation = NULL; }
    // Notification of revealed secrets
    if (secret_notification) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(260, 105, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(260, 105, RD_OFF); dp_translation = NULL; }
    // Show negative health
    if (negative_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(255, 115, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(255, 115, RD_OFF); dp_translation = NULL; }

    // Footer
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 145, RD_NEXT);
    M_WriteTextSmall(35, 155, RD_PREV);
    dp_translation = NULL;
}

void M_RD_Draw_Gameplay_3(void)
{
    // Write capitalized title (НАСТРОЙКИ ГЕЙМПЛЕЯ)
    M_WriteTextBigCentered(10, "YFCNHJQRB UTQVGKTZ");


    // Write "on" / "off" strings for features
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "Abpbrf");     // Физика
    dp_translation = NULL;

    // Walk over and under monsters
    if (over_under) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(274, 45, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(274, 45, RD_OFF); dp_translation = NULL; }
    // Corpses sliding from the ledges
    if (torque) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(256, 55, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(256, 55, RD_OFF); dp_translation = NULL; }
    // Weapon bobbing while firing
    if (weapon_bobbing) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(271, 65, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(271, 65, RD_OFF); dp_translation = NULL; }
    // Lethal pellet of a point-blank SSG
    if (ssg_blast_enemies) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(274, 75, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(274, 75, RD_OFF); dp_translation = NULL; }
    // Randomly mirrored corpses
    if (randomly_flipcorpses) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(207, 85, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(207, 85, RD_OFF); dp_translation = NULL; }
    // Floating powerups
    if (floating_powerups) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(275, 95, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(275, 95, RD_OFF); dp_translation = NULL; }

    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 105, "Ghbwtk");   // Прицел
    dp_translation = NULL;

    // Draw crosshair
    if (crosshair_draw) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(180, 115, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(180, 115, RD_OFF); dp_translation = NULL; }
    // Health indication
    if (crosshair_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(186, 125, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(186, 125, RD_OFF); dp_translation = NULL; }

    // Footer
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 145, RD_NEXT);
    M_WriteTextSmall(35, 155, RD_PREV);
    dp_translation = NULL;
}

void M_RD_Draw_Gameplay_4(void)
{   
    // Write capitalized title (НАСТРОЙКИ ГЕЙМПЛЕЯ)
    M_WriteTextBigCentered(10, "YFCNHJQRB UTQVGKTZ");

    // Write "on" / "off" strings for features
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 35, "Utqvgktq"); // Геймплей
    dp_translation = NULL;

    // Extra player faces on the HUD
    if (extra_player_faces) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(247, 45, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(247, 45, RD_OFF); dp_translation = NULL; }

    // Pain Elemental without Souls limit
    if (unlimited_lost_souls) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(274, 55, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(274, 55, RD_OFF); dp_translation = NULL; }

    // Don't prompt for q. saving/loading
    if (fast_quickload) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(253, 65, RD_ON); dp_translation = NULL; }
    else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(253, 65, RD_OFF); dp_translation = NULL; }

    // Play internal demos
    if (no_internal_demos) { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall(219, 75, RD_OFF); dp_translation = NULL; }
    else { dp_translation = cr[CR_GREEN]; M_WriteTextSmall(219, 75, RD_ON); dp_translation = NULL; }

    // Footer
    dp_translation = cr[CR_GOLD];
    M_WriteTextSmall(35, 145, RD_NEXT);
    M_WriteTextSmall(35, 155, RD_PREV);
    dp_translation = NULL;
}

void M_RD_Change_Brightmaps(int choice)
{
    choice = 0;
    brightmaps = 1 - brightmaps;
}

void M_RD_Change_FakeContrast(int choice)
{
    choice = 0;
    fake_contrast = 1 - fake_contrast;
}

void M_RD_Change_ColoredHUD(int choice)
{
    choice = 0;
    colored_hud = 1 - colored_hud;
    
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_ColoredBlood(int choice)
{
    choice = 0;
    colored_blood = 1 - colored_blood;
}

void M_RD_Change_SwirlingLiquids(int choice)
{
    choice = 0;
    swirling_liquids = 1 - swirling_liquids;
}

void M_RD_Change_InvulSky(int choice)
{
    choice = 0;
    invul_sky = 1 - invul_sky;
}

void M_RD_Change_ShadowedText(int choice)
{
    choice = 0;
    draw_shadowed_text = 1 - draw_shadowed_text;
}

void M_RD_Change_ExitSfx(int choice)
{
    choice = 0;
    play_exit_sfx = 1 - play_exit_sfx;
}

void M_RD_Change_CrushingSfx(int choice)
{
    choice = 0;
    crushed_corpses_sfx = 1 - crushed_corpses_sfx;
}

void M_RD_Change_BlazingSfx(int choice)
{
    choice = 0;
    blazing_door_fix_sfx = 1 - blazing_door_fix_sfx;
}

void M_RD_Change_AlertSfx(int choice)
{
    choice = 0;
    noise_alert_sfx = 1 - noise_alert_sfx;
}

void M_RD_Change_AutoMapStats(int choice)
{
    choice = 0;
    automap_stats = 1 - automap_stats;
}

void M_RD_Change_SecretNotify(int choice)
{
    choice = 0;
    secret_notification = 1 - secret_notification;
}

void M_RD_Change_NegativeHealth(int choice)
{
    choice = 0;
    negative_health = 1 - negative_health;
}

void M_RD_Change_WalkOverUnder(int choice)
{
    choice = 0;
    over_under = 1 - over_under;
}

void M_RD_Change_Torque(int choice)
{
    choice = 0;
    torque = 1 - torque;
}

void M_RD_Change_Bobbing(int choice)
{
    choice = 0;
    weapon_bobbing = 1 - weapon_bobbing;
}

void M_RD_Change_SSGBlast(int choice)
{
    choice = 0;
    ssg_blast_enemies = 1 - ssg_blast_enemies;
}

void M_RD_Change_FlipCorpses(int choice)
{
    choice = 0;
    randomly_flipcorpses = 1 - randomly_flipcorpses;
}

void M_RD_Change_FloatPowerups(int choice)
{
    choice = 0;
    floating_powerups = 1 - floating_powerups;
}

void M_RD_Change_CrosshairDraw(int choice)
{
    choice = 0;
    crosshair_draw = 1 - crosshair_draw;
}

void M_RD_Change_CrosshairHealth(int choice)
{
    choice = 0;
    crosshair_health = 1 - crosshair_health;
}

void M_RD_Change_ExtraPlayerFaces(int choice)
{
    choice = 0;
    extra_player_faces = 1 - extra_player_faces;
}

void M_RD_Change_LostSoulsQty(int choice)
{
    choice = 0;
    unlimited_lost_souls = 1 - unlimited_lost_souls;
}

void M_RD_Change_FastQSaveLoad(int choice)
{
    choice = 0;
    fast_quickload = 1 - fast_quickload;
}

void M_RD_Change_NoInternalDemos(int choice)
{
    choice = 0;
    no_internal_demos = 1 - no_internal_demos;
}

// -----------------------------------------------------------------------------
// Back to Defaults
// -----------------------------------------------------------------------------

void M_RD_BackToDefaultsResponse(int ch)
{
    static char resetmsg[24];

    if (ch != 'y')
    return;

    // Rendering
    show_diskicon = 1;
    noflats       = 0;
    screen_wiping = 1;

    // Display
    screenblocks = 10;
    screenSize = screenblocks - 3;
    usegamma     = 0;
    detailLevel  = 0;
    showMessages = 1;

    // Audio
    sfxVolume       = 8;  S_SetSfxVolume(sfxVolume * 8);
    musicVolume     = 8;  S_SetMusicVolume(musicVolume * 8);
    numChannels     = 32; S_ChannelsRealloc();
    snd_monomode    = 0;
    snd_pitchshift  = 0;

    // Controls
    joybspeed        = 29;
    mlook            = 0; players[consoleplayer].centering = true;
    mouseSensitivity = 5;

    // Gameplay
    brightmaps         = 1;
    fake_contrast      = 0;
    colored_hud        = 0;
    colored_blood      = 1;
    swirling_liquids   = 1;
    invul_sky          = 1;
    draw_shadowed_text = 1;

    play_exit_sfx        = 1;
    crushed_corpses_sfx  = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx      = 0;

    automap_stats       = 1;
    secret_notification = 1;
    negative_health     = 0;

    over_under           = 0;
    torque               = 1;
    weapon_bobbing       = 1;
    ssg_blast_enemies    = 1;
    randomly_flipcorpses = 1;
    floating_powerups    = 0;

    crosshair_draw   = 0;
    crosshair_health = 1;

    extra_player_faces   = 1;
    unlimited_lost_souls = 1;
    fast_quickload       = 1;
    no_internal_demos    = 0;

    // Set view size, execute drawing functions
    R_SetViewSize (screenblocks, detailLevel); 

    // Set palette
    I_SetPalette ((byte *)W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE) + 
                                          st_palette * 768);

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }

    // Print informative message (настройки сброшены)
    snprintf(resetmsg, sizeof(resetmsg), "Yfcnhjqrb c,hjitys");
    players[consoleplayer].message = resetmsg;
}

void M_RD_BackToDefaults(int choice)
{
    choice = 0;
    M_StartMessage(RD_DEFAULTS, M_RD_BackToDefaultsResponse,true);
}


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

    // ЗАГРУЗИТЬ ИГРУ
    M_WriteTextBigCentered(13, "PFUHEPBNM BUHE");

    for (i = 0;i < load_end; i++)
    {
    if (!vanilla)
    // [JN] TODO - cleanup
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    else
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y-1+LINEHEIGHT*i);

	// [crispy] shade empty savegame slots
	if (!LoadMenu[i].status && colored_hud && !vanilla)
	dp_translation = cr[CR_DARKRED];

	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);

	dp_translation = NULL;
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
        // БЫСТРОЕ СОХРАНЕНИЕ
        M_WriteTextBigCentered(13, "<SCNHJT CJ{HFYTYBT");
    }
    else
    {
        // СОХРАНИТЬ ИГРУ
        M_WriteTextBigCentered(13, "CJ{HFYBNM BUHE");
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

    if (fast_quickload)
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

    if (fast_quickload)
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
    // НОВАЯ ИГРА
    M_WriteTextBigCentered(14, "YJDFZ BUHF");

    // Уровень сложности:
    M_WriteTextBigCentered(38, "Ehjdtym ckj;yjcnb:");
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
    // НОВАЯ ИГРА
    M_WriteTextBigCentered(14, "YJDFZ BUHF");

    // Какой эпизод?
    M_WriteTextBigCentered(38, "Rfrjq \'gbpjl?");
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
	
    /*
    V_DrawShadowDirect (RD_Options_Def.x + 176,RD_Options_Def.y+1+LINEHEIGHT*detail,0,
		       W_CacheLumpName(detailNames[detailLevel],PU_CACHE));
    V_DrawPatchDirect (RD_Options_Def.x + 175,RD_Options_Def.y+LINEHEIGHT*detail,0,
		       W_CacheLumpName(detailNames[detailLevel],PU_CACHE));

    V_DrawShadowDirect (RD_Options_Def.x + 121,RD_Options_Def.y+1+LINEHEIGHT*messages,0,
		       W_CacheLumpName(msgNames[showMessages],PU_CACHE));
    V_DrawPatchDirect (RD_Options_Def.x + 120,RD_Options_Def.y+LINEHEIGHT*messages,0,
		       W_CacheLumpName(msgNames[showMessages],PU_CACHE));

    M_DrawThermo(RD_Options_Def.x,RD_Options_Def.y+LINEHEIGHT*(mousesens+1),
		 12,mouseSensitivity);
	
    // [JN] Initially 9. Three new screen sizes for Crispy HUDs.
    M_DrawThermo(RD_Options_Def.x,RD_Options_Def.y+LINEHEIGHT*(scrnsize+1),
		 12,screenSize);
         */
}

void M_Options(int choice)
{
    M_SetupNextMenu(&RD_Options_Def);
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
    if (!netgame && play_exit_sfx && sfxVolume > 0)
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


// -----------------------------------------------------------------------------
// [JN] Draw small thermo for RD options menu
// -----------------------------------------------------------------------------
void M_DrawThermo_Small (int x, int y, int thermWidth, int thermDot)
{
    int		xx;
    int		i;

    xx = x;
    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("RD_THRML",PU_CACHE));
    V_DrawPatchDirect (xx,y,0,W_CacheLumpName("RD_THRML",PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("RD_THRMM",PU_CACHE));
        V_DrawPatchDirect (xx,y,0,W_CacheLumpName("RD_THRMM",PU_CACHE));
        xx += 8;
    }
    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("RD_THRMR",PU_CACHE));
    V_DrawPatchDirect (xx,y,0,W_CacheLumpName("RD_THRMR",PU_CACHE));

    // [crispy] do not crash anymore if value exceeds thermometer range
    // [JN] Draw red slider instead of blue.
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        V_DrawPatchDirect ((x+8) + thermDot*8,y,0,W_CacheLumpName("RD_THRMW",PU_CACHE));
    }
    else
    {
        V_DrawPatchDirect ((x+8) + thermDot*8,y,0,W_CacheLumpName("RD_THRMO",PU_CACHE));
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
	    M_RD_Change_ScreenSize(0);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
				
	  case KEY_EQUALS:        // Screen size up
	    if (automapactive || chat_on)
		return false;
	    M_RD_Change_ScreenSize(1);
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
	    currentMenu = &RD_Audio_Def;
	    itemOn = rd_audio_sfxvolume;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEY_F5:            // Detail toggle
	    M_RD_Change_Detail(0);
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
	    M_RD_Change_Messages(0);
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
	}

    if (ch == KEY_F11)    // gamma toggle
    {
        usegamma++;
        if (usegamma > 17)
        usegamma = 0;
        players[consoleplayer].message = gammamsg[usegamma];
        pal = (byte *) W_CacheLumpName (usegamma <= 8 ?  "PALFIX" : "PLAYPAL",
                                        PU_CACHE) + st_palette * 768;
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
	
    // [JN] Scroll Gameplay features menu by PgUp/PgDn keys
    case KEY_PGUP:
    {
        currentMenu->lastOn = itemOn;

        if (currentMenu == &RD_Gameplay_Def_1)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_4);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_2)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_1);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_3)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_2);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_4)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_3);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
    }
    case KEY_PGDN:
    {
        currentMenu->lastOn = itemOn;

        if (currentMenu == &RD_Gameplay_Def_1)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_2);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_2)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_3);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_3)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_4);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_4)
        {
            M_SetupNextMenu(&RD_Gameplay_Def_1);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
    }

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
    char		string[80];
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
        // -----------------------------------------------------------------
        // [JN] Write common menus by using standard graphical patches:
        // -----------------------------------------------------------------
        if (currentMenu == &MainDef                // Main Menu
        ||  currentMenu == &EpiDef                 // Episode selection
        ||  currentMenu == &NewDef)                // Skill level
        {
            V_DrawShadowDirect (x+1,y+1,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
            V_DrawPatchDirect (x,y,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));

            // DRAW SKULL
            V_DrawShadowDirect(x+1 + SKULLXOFF,currentMenu->y+1 - 5 + itemOn*LINEHEIGHT, 0,
                               W_CacheLumpName(skullName[whichSkull],PU_CACHE));
            V_DrawPatchDirect(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
                               W_CacheLumpName(skullName[whichSkull],PU_CACHE));

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // -----------------------------------------------------------------
        // [JN] Write English options menu with big English font
        // -----------------------------------------------------------------
        else
        if (currentMenu == &RD_Options_Def)
        {
            M_WriteTextBig(x, y, currentMenu->menuitems[i].name);
        
            // DRAW SKULL
            V_DrawShadowDirect(x+1 + SKULLXOFF,currentMenu->y+1 - 5 + itemOn*LINEHEIGHT, 0,
                               W_CacheLumpName(skullName[whichSkull],PU_CACHE));
            V_DrawPatchDirect(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
                               W_CacheLumpName(skullName[whichSkull],PU_CACHE));
        
            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // -----------------------------------------------------------------
        // [JN] Write Russian options menu with big Russian font
        // -----------------------------------------------------------------
    /*
        else 
        if (currentMenu == &MainDef_Rus
        ||  currentMenu == &EpiDef_Rus
        ||  currentMenu == &NewDef_Rus
        ||  currentMenu == &RD_Options_Def_Rus
        ||  currentMenu == &Vanilla_OptionsDef_Rus
        ||  currentMenu == &Vanilla_Audio_Def_Rus)
        {
            M_WriteTextBig_RUS(x, y, name);
        
            // DRAW SKULL
            V_DrawShadowedPatchDoom(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
            W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
        
            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
    */
        // -----------------------------------------------------------------
        // [JN] Write English submenus with small English font
        // -----------------------------------------------------------------
        else
        if (currentMenu == &RD_Rendering_Def
        ||  currentMenu == &RD_Display_Def
        ||  currentMenu == &RD_Audio_Def
        ||  currentMenu == &RD_Controls_Def
        ||  currentMenu == &RD_Gameplay_Def_1
        ||  currentMenu == &RD_Gameplay_Def_2
        ||  currentMenu == &RD_Gameplay_Def_3
        ||  currentMenu == &RD_Gameplay_Def_4)
        {
            M_WriteTextSmall(x, y, currentMenu->menuitems[i].name);
        
            // [JN] Draw blinking ">" symbol
            if (whichSkull == 0)
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall(x + SKULLXOFF + 24, currentMenu->y + itemOn*LINEHEIGHT_SML, ">");
            // [JN] Clear translation
            dp_translation = NULL;
        
            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
        // -----------------------------------------------------------------
        // [JN] Write Russian submenus with small Russian font
        // -----------------------------------------------------------------            
        /*
        else
        if (currentMenu == &RD_Rendering_Def_Rus
        ||  currentMenu == &RD_Display_Def_Rus
        ||  currentMenu == &RD_Automap_Def_Rus
        ||  currentMenu == &RD_Audio_Def_Rus
        ||  currentMenu == &RD_Controls_Def_Rus
        ||  currentMenu == &RD_Gameplay_Def_1_Rus
        ||  currentMenu == &RD_Gameplay_Def_2_Rus
        ||  currentMenu == &RD_Gameplay_Def_3_Rus
        ||  currentMenu == &RD_Gameplay_Def_4_Rus)
        {
            M_WriteTextSmall_RUS(x, y, name);
            
            // [JN] Draw blinking ">" symbol
            if (whichSkull == 0)
            dp_translation = cr[CR_DARKRED];
            // [JN] Jaguar: no font color translation, draw SKULL1 as an empty symbol.
            M_WriteTextSmall_ENG(x + SKULLXOFF + 24, currentMenu->y + itemOn*LINEHEIGHT_SML,
                                 gamemission == jaguar && whichSkull == 0 ? " " : ">");
            // [JN] Clear translation
            dp_translation = NULL;
        
            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
        */

        // [JN] Saving / Loading menus, help screens. Just a blinking skull.
        if (currentMenu == &LoadDef
        ||  currentMenu == &SaveDef
        ||  currentMenu == &ReadDef1
        ||  currentMenu == &ReadDef2)
        {
            // DRAW SKULL
            V_DrawShadowDirect(x+1 + SKULLXOFF,currentMenu->y+1 - 5 + itemOn*LINEHEIGHT, 0,
                    W_CacheLumpName(skullName[whichSkull],PU_CACHE));
            V_DrawPatchDirect(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
                    W_CacheLumpName(skullName[whichSkull],PU_CACHE));
        }
    }
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


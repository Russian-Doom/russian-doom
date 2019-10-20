//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//



#include <stdlib.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "rd_lang.h"

#include "d_main.h"
#include "deh_main.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "r_local.h"

#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_controls.h"
#include "p_saveg.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"

#include "p_local.h"
#include "st_stuff.h"
#include "v_trans.h"
#include "crispy.h"
#include "jn.h"


extern patch_t*     hu_font[HU_FONTSIZE];
extern patch_t*     hu_font_small_eng[HU_FONTSIZE];
extern patch_t*     hu_font_small_rus[HU_FONTSIZE];
extern patch_t*     hu_font_big_eng[HU_FONTSIZE2];
extern patch_t*     hu_font_big_rus[HU_FONTSIZE2];
extern boolean      message_dontfuckwithme;

extern boolean      chat_on;    // in heads-up code

extern int  st_palette;

//
// defaulted values
//

int mouseSensitivity = 5;
int showMessages = 1; // Show messages has default, 0 = off, 1 = on

// Blocky mode, has default, 0 = high, 1 = normal
int detailLevel = 0;
int screenblocks = 10; // [JN] Размер экрана по-умолчанию установлен на 10

// temp for screenblocks (0-9)
int screenSize;

// -1 = no quicksave slot picked!
int quickSaveSlot;

// 1 = message to be printed
int     messageToPrint;
// ...and here is the message string!
char*   messageString;

// message x & y
int messx;
int messy;
int messageLastMenuActive;

// timed message = no input from user
boolean messageNeedsInput;

// [JN] Дополнительный титр "БЫСТРОЕ СОХРАНЕНИЕ"
boolean QuickSaveTitle;


void (*messageRoutine)(int response);

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

char gammamsg_rus[18][41] =
{
    GAMMA_IMPROVED_OFF_RUS,
    GAMMA_IMPROVED_05_RUS,
    GAMMA_IMPROVED_1_RUS,
    GAMMA_IMPROVED_15_RUS,
    GAMMA_IMPROVED_2_RUS,
    GAMMA_IMPROVED_25_RUS,
    GAMMA_IMPROVED_3_RUS,
    GAMMA_IMPROVED_35_RUS,
    GAMMA_IMPROVED_4_RUS,
    GAMMA_ORIGINAL_OFF_RUS,
    GAMMA_ORIGINAL_05_RUS,
    GAMMA_ORIGINAL_1_RUS,
    GAMMA_ORIGINAL_15_RUS,
    GAMMA_ORIGINAL_2_RUS,
    GAMMA_ORIGINAL_25_RUS,
    GAMMA_ORIGINAL_3_RUS,
    GAMMA_ORIGINAL_35_RUS,
    GAMMA_ORIGINAL_4_RUS
};

#define NUM_QUITMESSAGES 8

// Quit DOOM I messages
char *doom1_endmsg[] =
{
    "are you sure you want to\nquit this great game?",
    "please don't leave, there's more\ndemons to toast!",
    "let's beat it -- this is turning\ninto a bloodbath!",
    "i wouldn't leave if i were you.\ndos is much worse.",
    "you're trying to say you like dos\nbetter than me, right?",
    "don't leave yet -- there's a\ndemon around that corner!",
    "ya know, next time you come in here\ni'm gonna toast ya.",
    "go ahead and leave. see if i care.",
};


// Quit DOOM II messages
char *doom2_endmsg[] =
{
    "are you sure you want to\nquit this great game?",
    "you want to quit?\nthen, thou hast lost an eighth!",
    "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
    "get outta here and go back\nto your boring programs.",
    "if i were your boss, i'd \n deathmatch ya in a minute!",
    "look, bud. you leave now\nand you forfeit your body count!",
    "just leave. when you come\nback, i'll be waiting with a bat.",
    "you're lucky i don't smack\nyou for thinking about leaving.",
};

// [JN] Russian quit DOOM I messages
char *doom1_endmsg_rus[] =
{
    // Вы действительно хотите выйти \n из этой замечательной игры?
    "ds ltqcndbntkmyj [jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?",

    // Пожалуйста, не уходите! \n Здесь осталось ещё много демонов!
    "gj;fkeqcnf< yt e[jlbnt!\npltcm jcnfkjcm tot vyjuj ltvjyjd!",

    // Давайте определимся, это будет \n ваша первая и последняя попытка!
    "lfdfqnt jghtltkbvcz< 'nj ,eltn\n dfif gthdfz b gjcktlyzz gjgsnrf!",

    // На вашем месте я не уходил. \n Работать намного скучнее.
    "yf dfitv vtcnt z ,s yt e[jlbk>\nhf,jnfnm yfvyjuj crexytt>",

    // Вы хотите сказать, что операционная \n система лучше чем я, да?
    "ds [jnbnt crfpfnm< xnj jgthfwbjyyfz\ncbcntvf kexit xtv z< lf?",

    // Не уходите, в дальнем углу \n притаился еще один монстр!
    "yt e[jlbnt< d lfkmytv euke\nghbnfbkcz tot jlby vjycnh!",

    // Знаете, когда вы вернётесь, \n вас будут ждать большие неприятности.
    "pyftnt< rjulf ds dthytntcm<\ndfc ,elen ;lfnm ,jkmibt ytghbznyjcnb>",

    // Давайте, уходите. \n Мне совершенно всё равно.
    "lfdfqnt< e[jlbnt>\nvyt cjdthityyj dct hfdyj>",
};

// [JN] Russian quit DOOM II messages
char *doom2_endmsg_rus[] =
{
    //Вы действительно хотите выйти \n из этой замечательной игры?
    "ds ltqcndbntkmyj [jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?", 

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

    // Вам очень повезло, что за это \n с вами совершенно ничего не случится!
    "dfv jxtym gjdtpkj< xnj pf 'nj\nc dfvb ybxtuj yt ckexbncz!", 
};

// we are going to be entering a savegame string
int saveStringEnter;              
int saveSlot;       // which slot to save in
int saveCharIndex;  // which char we're editing

// old save description before edit
char    saveOldString[SAVESTRINGSIZE];  

boolean inhelpscreens;
boolean menuactive;

#define SKULLXOFF   -32
#define LINEHEIGHT  16
#define LINEHEIGHT_SML  10  // [JN] Line height for small font

extern boolean  sendpause;
char            savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

static boolean opldev;

//
// MENU TYPEDEFS
//

typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short   status;
    char    name[128];  // [JN] Extended from 10 to 128, so long text string may appear

    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void    (*routine)(int choice);

    // hotkey in menu
    char    alphaKey;			
} menuitem_t;


typedef struct menu_s
{
    short           numitems;       // # of menu items
    struct menu_s*  prevMenu;       // previous menu
    menuitem_t*     menuitems;      // menu items
    void            (*routine)();   // draw routine
    short           x;
    short           y;              // x,y of menu
    short           lastOn;         // last item user was on in menu
} menu_t;

short   itemOn;             // menu item skull is on
short   skullAnimCounter;   // skull animation counter
short   whichSkull;         // which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          


// -----------------------------------------------------------------------------
// [JN] Custom RD menu: font writing prototypes
// -----------------------------------------------------------------------------

void M_WriteText(int x, int y, char *string);
void M_WriteTextSmall_ENG(int x, int y, char *string);
void M_WriteTextSmall_RUS(int x, int y, char *string);
void M_WriteTextBig_ENG(int x, int y, char *string);
void M_WriteTextBig_RUS(int x, int y, char *string);
void M_WriteTextBigCentered_ENG(int y, char *string);
void M_WriteTextBigCentered_RUS(int y, char *string);


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

void M_StartGame(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawThermo_Small(int x,int y,int thermWidth,int thermDot);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
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
void M_RD_Change_VSync(int choice);
void M_RD_Change_AspectRatio(int choice);
void M_RD_Change_Uncapped(int choice);
void M_RD_Change_FPScounter(int choice);
void M_RD_Change_DiskIcon(int choice);
void M_RD_Change_Smoothing(int choice);
void M_RD_Change_Wiping(int choice);
void M_RD_Change_Screenshots(int choice);
void M_RD_Change_Renderer(int choice);

// Display
void M_RD_Choose_Display(int choice);
void M_RD_Draw_Display(void);
void M_RD_Change_ScreenSize(int choice);
void M_RD_Change_Gamma(int choice);
void M_RD_Change_Detail(int choice);
void M_RD_Change_LocalTime(int choice);
void M_RD_Change_Messages(int choice);
void M_RD_Change_MessagesColor(int choice);
void M_RD_Change_ShadowedText(int choice);

// Automap
void M_RD_Choose_AutomapSettings(int choice);
void M_RD_Draw_AutomapSettings(void);
void M_RD_Change_AutomapColor(int choice);
void M_RD_Change_AutomapStats(int choice);
void M_RD_Change_AutomapOverlay(int choice);
void M_RD_Change_AutomapRotate(int choice);
void M_RD_Change_AutomapFollow(int choice);
void M_RD_Change_AutomapGrid(int choice);

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
void M_RD_Change_Novert(int choice);
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
void M_RD_Change_ExitSfx(int choice);
void M_RD_Change_CrushingSfx(int choice);
void M_RD_Change_BlazingSfx(int choice);
void M_RD_Change_AlertSfx(int choice);
void M_RD_Change_AutoMapStats(int choice);
void M_RD_Change_SecretNotify(int choice);
void M_RD_Change_NegativeHealth(int choice);
void M_RD_Change_InfraGreenVisor(int choice);
void M_RD_Change_WalkOverUnder(int choice);
void M_RD_Change_Torque(int choice);
void M_RD_Change_Bobbing(int choice);
void M_RD_Change_SSGBlast(int choice);
void M_RD_Change_FlipCorpses(int choice);
void M_RD_Change_FloatPowerups(int choice);
void M_RD_Change_CrosshairDraw(int choice);
void M_RD_Change_CrosshairHealth(int choice);
void M_RD_Change_CrosshairScale(int choice);
void M_RD_Change_FixMapErrors(int choice);
void M_RD_Change_FlipLevels(int choice);
void M_RD_Change_ExtraPlayerFaces(int choice);
void M_RD_Change_LostSoulsQty(int choice);
void M_RD_Change_LostSoulsAgr(int choice);
void M_RD_Change_FastQSaveLoad(int choice);
void M_RD_Change_NoInternalDemos(int choice);

// Back to Defaults
void M_RD_BackToDefaultsResponse(int key);
void M_RD_BackToDefaults(int choice);

// -----------------------------------------------------------------------------
// [JN] Vanilla menu prototypes
// -----------------------------------------------------------------------------

void M_Vanilla_DrawOptions(void);
void M_Vanilla_DrawSound(void);


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
        if (cx+w > ORIGWIDTH)
            break;

        V_DrawShadowedPatchDoom(cx, cy, hu_font[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextSmall_ENG
//
// [JN] Write a string using a small STCFS font
// -----------------------------------------------------------------------------

void M_WriteTextSmall_ENG (int x, int y, char *string)
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

        w = SHORT (hu_font_small_eng[c]->width);
        if (cx+w > ORIGWIDTH)
            break;

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_eng[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextSmall_ENG
//
// [JN] Write a string using a small STCFS font
// -----------------------------------------------------------------------------

void M_WriteTextSmall_RUS (int x, int y, char *string)
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

        w = SHORT (hu_font_small_rus[c]->width);
        if (cx+w > ORIGWIDTH)
            break;

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_rus[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextBig
//
// [JN] Write a string using a big STCFB font
// -----------------------------------------------------------------------------

void M_WriteTextBig_ENG (int x, int y, char *string)
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

        w = SHORT (hu_font_big_eng[c]->width);
        if (cx+w > ORIGWIDTH)
        break;

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_eng[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextBig
//
// [JN] Write a string using a big STCFB font
// -----------------------------------------------------------------------------

void M_WriteTextBig_RUS (int x, int y, char *string)
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

        w = SHORT (hu_font_big_rus[c]->width);
        if (cx+w > ORIGWIDTH)
        break;

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_rus[c]);

        // Place one char to another with one pixel
        cx += w-1;
    }
}


// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
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

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);
        width += w;
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
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

        w = SHORT (hu_font_big_eng[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_eng[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
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

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);
        width += w;
    }

    // draw it
    cx = ORIGWIDTH/2-width/2;
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

        w = SHORT (hu_font_big_rus[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_rus[c]);

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

// Special menu for Press Beta
menuitem_t MainMenuBeta[]=
{
    {1,"M_BLVL1",  M_Episode, '1'},
    {1,"M_BLVL2",  M_Episode, '2'},
    {1,"M_BLVL3",  M_Episode, '3'},
    {1,"M_OPTION", M_Options, 'o'},
    {1,"M_QUITG",  M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97+ORIGWIDTH_DELTA,70,
    0
};

// [JN] Special menu for Press Beta
menu_t  MainDefBeta =
{
    main_end,
    NULL,
    MainMenuBeta,
    M_DrawMainMenu,
    97+ORIGWIDTH_DELTA,70,
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
    ep_end,                 // # of menu items
    &MainDef,               // previous menu
    EpisodeMenu,            // menuitem_t ->
    M_DrawEpisode,          // drawing routine ->
    48+ORIGWIDTH_DELTA,63,  // x,y
    ep1                     // lastOn
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
    {1,"M_JKILL",   M_ChooseSkill, 'i'},
    {1,"M_ROUGH",   M_ChooseSkill, 'h'},
    {1,"M_HURT",    M_ChooseSkill, 'h'},
    {1,"M_ULTRA",   M_ChooseSkill, 'u'},
    {1,"M_NMARE",   M_ChooseSkill, 'n'},
    {1,"M_UNMARE",  M_ChooseSkill, 'z'}
};

menu_t  NewDef =
{
    newg_end,               // # of menu items
    &EpiDef,                // previous menu
    NewGameMenu,            // menuitem_t ->
    M_DrawNewGame,          // drawing routine ->
    48+ORIGWIDTH_DELTA,63,  // x,y
    hurtme                  // lastOn
};


// =============================================================================
// Read This! MENU 1 & 2
// =============================================================================

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
    280,185,
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
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};


// =============================================================================
// [JN] NEW OPTIONS MENU: STRUCTURE
// =============================================================================

// -----------------------------------------------------------------------------
// Main Menu
// -----------------------------------------------------------------------------

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

// ------------
// English menu
// ------------

menuitem_t RD_Options_Menu[]=
{
    {1,"Rendering", M_RD_Choose_Rendering,      'r'},
    {1,"Display",   M_RD_Choose_Display,        'd'},
    {1,"Sound",     M_RD_Choose_Audio,          's'},
    {1,"Controls",  M_RD_Choose_Controls,       'c'},
    {1,"Gameplay",  M_RD_Choose_Gameplay_1,     'g'},
    {1,"End Game",  M_EndGame,                  'e'},
    {1,"Reset settings", M_RD_BackToDefaults,   'r'},
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def =
{
    rd_end, 
    &MainDef,
    RD_Options_Menu,
    M_RD_Draw_Options,
    60+ORIGWIDTH_DELTA, 37,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Options_Menu_Rus[]=
{
    {1,"Dbltj",         M_RD_Choose_Rendering,  'd'},   // Видео
    {1,"\"rhfy",        M_RD_Choose_Display,    '\''},  // Экран
    {1,"Felbj",         M_RD_Choose_Audio,      'f'},   // Аудио
    {1,"Eghfdktybt",    M_RD_Choose_Controls,   'e'},   // Управление
    {1,"Utqvgktq",      M_RD_Choose_Gameplay_1, 'u'},   // Геймплей
    {1,"Pfrjyxbnm buhe",M_EndGame,              'p'},   // Закончить игру
    {1,"C,hjc yfcnhjtr", M_RD_BackToDefaults,   'c'},   // Сброс настроек
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def_Rus =
{
    rd_end, 
    &MainDef,
    RD_Options_Menu_Rus,
    M_RD_Draw_Options,
    60+ORIGWIDTH_DELTA, 37,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

enum
{
    rd_rendering_vsync,
    rd_rendering_aspect,
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
    {1,"Vertical synchronization:", M_RD_Change_VSync,      'v'},
    {1,"Fix aspect ratio:",         M_RD_Change_AspectRatio,'f'},
    {1,"Uncapped framerate:",       M_RD_Change_Uncapped,   'u'},
    {1,"Show FPS counter:",         M_RD_Change_FPScounter, 's'},
    {1,"Pixel scaling:",            M_RD_Change_Smoothing,  'p'},
    {1,"Video renderer:",           M_RD_Change_Renderer,   'v'},
    {-1,"",0,'\0'},
    {1,"Show disk icon:",           M_RD_Change_DiskIcon,   's'},
    {1,"Screen wiping effect:",     M_RD_Change_Wiping,     's'},
    {1,"Screenshot format:",        M_RD_Change_Screenshots,'s'},
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def =
{
    rd_rendering_end,
    &RD_Options_Def,
    RD_Rendering_Menu,
    M_RD_Draw_Rendering,
    35+ORIGWIDTH_DELTA,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Rendering_Menu_Rus[]=
{
    {1,"Dthnbrfkmyfz cby[hjybpfwbz:",     M_RD_Change_VSync,      'd'}, // Вертикальная синхронизация
    {1,"Abrcbhjdfnm cjjnyjitybt cnjhjy:", M_RD_Change_AspectRatio,'a'}, // Фиксировать соотношение сторон
    {1,"Juhfybxtybt rflhjdjq xfcnjns:",   M_RD_Change_Uncapped,   'j'}, // Ограничение кадровой частоты
    {1,"Cxtnxbr rflhjdjq xfcnjns:",       M_RD_Change_FPScounter, 'a'}, // Счетчик кадровой частоты
    {1,"Gbrctkmyjt cukf;bdfybt:",         M_RD_Change_Smoothing,  'g'}, // Пиксельное сглаживание
    {1,"J,hf,jnrf dbltj:",                M_RD_Change_Renderer,   'j'}, // Обработка видео
    {-1,"",0,'\0'},                                                     // Дополнительно
    {1,"Jnj,hf;fnm pyfxjr lbcrtns:",      M_RD_Change_DiskIcon,   'j'}, // Отображать значок дискеты
    {1,"Gkfdyfz cvtyf \'rhfyjd:",         M_RD_Change_Wiping,     'g'}, // Плавная смена экранов
    {1,"Ajhvfn crhbyijnjd:",              M_RD_Change_Screenshots,'g'}, // Формат скриншотов
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def_Rus =
{
    rd_rendering_end,
    &RD_Options_Def_Rus,
    RD_Rendering_Menu_Rus,
    M_RD_Draw_Rendering,
    35+ORIGWIDTH_DELTA,45,
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
    rd_display_localtime,
    rd_display_empty3,
    rd_display_messages_toggle,
    rd_display_messages_color1,
    rd_display_messages_color2,
    rd_display_empty4,
    rd_display_automap_settings,
    rd_display_end
} rd_display_e;

// ------------
// English menu
// ------------

menuitem_t RD_Display_Menu[]=
{
    {2,"screen size",         M_RD_Change_ScreenSize,     's'},
    {-1,"",0,'\0'},
    {2,"gamma-correction",    M_RD_Change_Gamma,          'g'},
    {-1,"",0,'\0'},
    {1,"detail level:",       M_RD_Change_Detail,         'd'},
    {1,"local time:",         M_RD_Change_LocalTime,      'l'},
    {-1,"",0,'\0'},
    {1,"messages:",           M_RD_Change_Messages,       'm'},
    {1,"messages color:",     M_RD_Change_MessagesColor,  'm'},
    {1,"text casts shadows:", M_RD_Change_ShadowedText,   't'},
    {-1,"",0,'\0'},
    {1,"automap settings",    M_RD_Choose_AutomapSettings,'a'},
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def =
{
    rd_display_end,
    &RD_Options_Def,
    RD_Display_Menu,
    M_RD_Draw_Display,
    35+ORIGWIDTH_DELTA,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Display_Menu_Rus[]=
{
    {2,"hfpvth buhjdjuj \'rhfyf",  M_RD_Change_ScreenSize,      'h'}, // Размер игрового экрана
    {-1,"",0,'\0'},                                                   //
    {2,"ehjdtym ufvvf-rjhhtrwbb",  M_RD_Change_Gamma,           'u'}, // Уровень гамма-коррекции
    {-1,"",0,'\0'},                                                   //
    {1,"ehjdtym ltnfkbpfwbb:",     M_RD_Change_Detail,          'l'}, // Уровень детализации:
    {1,"cbcntvyjt dhtvz:",         M_RD_Change_LocalTime,       'c'}, // Системное время:
    {-1,"",0,'\0'},                                                   //
    {1,"jnj,hf;tybt cjj,otybq:",   M_RD_Change_Messages,        'c'}, // Отображение сообщений:
    {1,"wdtn cjj,otybq:",          M_RD_Change_MessagesColor,   'c'}, // Цвет сообщений:
    {1,"ntrcns jn,hfcsdf.n ntym:", M_RD_Change_ShadowedText,    'c'}, // Тексты отбрасывают тень:
    {-1,"",0,'\0'},                                                   //
    {1,"vty. yfcnhjqrb rfhns",     M_RD_Choose_AutomapSettings, 'c'}, // Меню настройки карты
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def_Rus =
{
    rd_display_end,
    &RD_Options_Def_Rus,
    RD_Display_Menu_Rus,
    M_RD_Draw_Display,
    35+ORIGWIDTH_DELTA,45,
    0
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

enum
{
    rd_automap_colors,
    rd_automap_stats,
    rd_automap_overlay,
    rd_automap_rotate,
    rd_automap_follow,
    rd_automap_grid,
    rd_automap_end
} rd_automap_e;

// ------------
// English menu
// ------------

menuitem_t RD_Automap_Menu[]=
{
    {1,"color scheme:", M_RD_Change_AutomapColor,   'c'},
    {1,"level stats:",  M_RD_Change_AutomapStats,   'l'},
    {1,"overlay mode:", M_RD_Change_AutomapOverlay, 'o'},
    {1,"rotate mode:",  M_RD_Change_AutomapRotate,  'r'},
    {1,"follow mode:",  M_RD_Change_AutomapFollow,  'f'},
    {1,"grid: o",       M_RD_Change_AutomapGrid,    'g'},
    {-1,"",0,'\0'}
};

menu_t  RD_Automap_Def =
{
    rd_automap_end,
    &RD_Display_Def,
    RD_Automap_Menu,
    M_RD_Draw_AutomapSettings,
    70+ORIGWIDTH_DELTA,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Automap_Menu_Rus[]=
{
    {1,"wdtnjdfz c[tvf:",       M_RD_Change_AutomapColor, 'h'},     // Цветовая схема:
    {1,"cnfnbcnbrf ehjdyz:",    M_RD_Change_AutomapStats, 'h'},     // Статистика уровня:
    {1,"ht;bv yfkj;tybz:",      M_RD_Change_AutomapOverlay, 'h'},   // Режим наложения:
    {1,"ht;bv dhfotybz:",       M_RD_Change_AutomapRotate, 'h'},    // Режим вращения:
    {1,"ht;bv cktljdfybz:",     M_RD_Change_AutomapFollow, 'h'},    // Режим следования:
    {1,"ctnrf:",                M_RD_Change_AutomapGrid, 'h'},      // Сетка:
    {-1,"",0,'\0'}
};

menu_t  RD_Automap_Def_Rus =
{
    rd_automap_end,
    &RD_Display_Def_Rus,
    RD_Automap_Menu_Rus,
    M_RD_Draw_AutomapSettings,
    70+ORIGWIDTH_DELTA,35,
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

// ------------
// English menu
// ------------

menuitem_t RD_Audio_Menu[]=
{
    {2,"sfx volume",            M_RD_Change_SfxVol,        's'},
    {-1,"",0,'\0'},
    {2,"music volume",          M_RD_Change_MusicVol,      'm'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {2,"sound channels",        M_RD_Change_SfxChannels,   's'},
    {-1,"",0,'\0'},
    {1,"sfx mode:",             M_RD_Change_SndMode,       's'},
    {1,"pitch-shifted sounds:", M_RD_Change_PitchShifting, 'p'},
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def =
{
    rd_audio_end,
    &RD_Options_Def,
    RD_Audio_Menu,
    M_RD_Draw_Audio,
    35+ORIGWIDTH_DELTA,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Audio_Menu_Rus[]=
{
    {2,"pder",                          M_RD_Change_SfxVol,         'u'},   // Звук
    {-1,"",0,'\0'},                                                         //
    {2,"vepsrf",                        M_RD_Change_MusicVol,       'u'},   // Музыка
    {-1,"",0,'\0'},                                                         //
    {-1,"",0,'\0'},                                                         //
    {2,"Pderjdst rfyfks",               M_RD_Change_SfxChannels,    'p'},   // Звуковые каналы
    {-1,"",0,'\0'},                                                         //
    {1,"Ht;bv pderf:",                  M_RD_Change_SndMode,        'h'},   // Режим звука
    {1,"ghjbpdjkmysq gbnx-ibanbyu: d",  M_RD_Change_PitchShifting,  'g'},   // Произвольный питч-шифтинг
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def_Rus =
{
    rd_audio_end,
    &RD_Options_Def_Rus,
    RD_Audio_Menu_Rus,
    M_RD_Draw_Audio,
    35+ORIGWIDTH_DELTA,45,
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
    rd_controls_novert,
    rd_controls_end
} rd_controls_e;

// ------------
// English menu
// ------------

menuitem_t RD_Controls_Menu[]=
{
    {1,"always run:",     M_RD_Change_AlwaysRun,   'a'},
    {-1,"",0,'\0'},
    {2,"mouse sensivity", M_RD_Change_Sensitivity, 'm'},
    {-1,"",0,'\0'},
    {1,"mouse look: o",    M_RD_Change_MouseLook,  'm'},
    {1,"vertical movement:", M_RD_Change_Novert,   'v'},
    {-1,"",0,'\0'}

};

menu_t  RD_Controls_Def =
{
    rd_controls_end,
    &RD_Options_Def,
    RD_Controls_Menu,
    M_RD_Draw_Controls,
    35+ORIGWIDTH_DELTA,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Controls_Menu_Rus[]=
{
    {1,"ht;bv gjcnjzyyjuj ,tuf:", M_RD_Change_AlwaysRun,   'g'}, // Режим постоянного бега
    {-1,"",0,'\0'},                                              //
    {2,"Crjhjcnm vsib",           M_RD_Change_Sensitivity, 'c'}, // Скорость мыши
    {-1,"",0,'\0'},                                              //
    {1,"J,pjh vsim.:",            M_RD_Change_MouseLook,   'j'}, // Обзор мышью
    {1,"dthnbrfkmyjt gthtvtotybt:", M_RD_Change_Novert,    'd'}, // Вертикальное перемещение
    {-1,"",0,'\0'}
};

menu_t  RD_Controls_Def_Rus =
{
    rd_controls_end,
    &RD_Options_Def_Rus,
    RD_Controls_Menu_Rus,
    M_RD_Draw_Controls,
    35+ORIGWIDTH_DELTA,45,
    0
};

// -----------------------------------------------------------------------------
// Gameplay enhancements
// -----------------------------------------------------------------------------

enum
{
    rd_gameplay_1_brightmaps,
    rd_gameplay_1_fake_contrast,
    rd_gameplay_1_translucency,
    rd_gameplay_1_colored_hud,
    rd_gameplay_1_colored_blood,
    rd_gameplay_1_swirling_liquids,
    rd_gameplay_1_invul_sky,
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
    rd_gameplay_2_secret_notification,
    rd_gameplay_2_negative_health,
    rd_gameplay_2_infragreen_visor,
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
    rd_gameplay_3_crosshair_scale,
    rd_gameplay_3_next_page,
    rd_gameplay_3_prev_page,
    rd_gameplay_3_end
} rd_gameplay_3_e;

enum
{
    rd_gameplay_4_fix_map_errors,
    rd_gameplay_4_flip_levels,
    rd_gameplay_4_extra_player_faces,
    rd_gameplay_4_unlimited_lost_souls,
    rd_gameplay_4_agressive_lost_souls,
    rd_gameplay_4_fast_quickload,
    rd_gameplay_4_no_internal_demos,
    rd_gameplay_4_empty1,
    rd_gameplay_4_empty2,
    rd_gameplay_4_empty3,
    rd_gameplay_4_first_page,
    rd_gameplay_4_prev_page,
    rd_gameplay_4_end
} rd_gameplay_4_e;

// ------------
// English menu
// ------------

menuitem_t RD_Gameplay_Menu_1[]=
{
    {1,"Brightmaps:",                  M_RD_Change_Brightmaps,     'b'},
    {1,"Fake contrast:",               M_RD_Change_FakeContrast,   'f'},
    {1,"Transparency:",                M_RD_Change_Transparency,   't'},
    {1,"Colored HUD elements:",        M_RD_Change_ColoredHUD,     'c'},
    {1,"Colored blood and corpses:",   M_RD_Change_ColoredBlood,   'c'},
    {1,"Swirling liquids:",            M_RD_Change_SwirlingLiquids,'s'},
    {1,"Invulnerability affects sky:", M_RD_Change_InvulSky,       'i'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"", /* Next Page > */           M_RD_Choose_Gameplay_2,     'n'},
    {1,"", /* < Last Page */           M_RD_Choose_Gameplay_4,     'l'},
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_1 =
{
    rd_gameplay_1_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_1,
    M_RD_Draw_Gameplay_1,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_2[]=
{
    {1,"Play exit sounds:",               M_RD_Change_ExitSfx,         'p'},
    {1,"Sound of crushing corpses:",      M_RD_Change_CrushingSfx,     's'},
    {1,"Single sound of blazing door:",   M_RD_Change_BlazingSfx,      's'},
    {1,"Monster alert waking up others:", M_RD_Change_AlertSfx,        'm'},
    {-1,"",0,'\0'},
    {1,"Notify of revealed secrets:",     M_RD_Change_SecretNotify,    'n'},
    {1,"Show negative health:",           M_RD_Change_NegativeHealth,  's'},
    {1,"Infragreen light amp. visor:",    M_RD_Change_InfraGreenVisor, 'i'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"", /* Next page >   */            M_RD_Choose_Gameplay_3,      'n'},
    {1,"", /* < Prev page > */            M_RD_Choose_Gameplay_1,      'p'},
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_2 =
{
    rd_gameplay_2_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_2,
    M_RD_Draw_Gameplay_2,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_3[]=
{
    {1,"Walk over and under monsters:",       M_RD_Change_WalkOverUnder,   'w'},
    {1,"Corpses sliding from the ledges:",    M_RD_Change_Torque,          'c'},
    {1,"Weapon bobbing while firing:",        M_RD_Change_Bobbing,         'w'},
    {1,"Lethal pellet of a point-blank SSG:", M_RD_Change_SSGBlast,        'l'},
    {1,"Randomly mirrored corpses:",          M_RD_Change_FlipCorpses,     'r'},
    {1,"Floating powerups:",                  M_RD_Change_FloatPowerups,   'f'},
    {-1,"",0,'\0'},
    {1,"Draw crosshair:",                     M_RD_Change_CrosshairDraw,   'd'},
    {1,"Health indication:",                  M_RD_Change_CrosshairHealth, 'h'},
    {1,"Increased size:",                     M_RD_Change_CrosshairScale,  'i'},
    {1,"", /* Next page >   */                M_RD_Choose_Gameplay_4,      'n'},
    {1,"", /* < Prev page > */                M_RD_Choose_Gameplay_2,      'p'},
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_3 =
{
    rd_gameplay_3_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_3,
    M_RD_Draw_Gameplay_3,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_4[]=
{
    {1,"Fix errors of vanilla maps:",         M_RD_Change_FixMapErrors,     'f'},
    {1,"Flip game levels:",                   M_RD_Change_FlipLevels,       'f'},
    {1,"Extra player faces on the HUD:",      M_RD_Change_ExtraPlayerFaces, 'e'},
    {1,"Pain Elemental without Souls limit:", M_RD_Change_LostSoulsQty,     'p'},
    {1,"More agressive lost souls:",          M_RD_Change_LostSoulsAgr,     'm'},
    {1,"Don't prompt for q. saving/loading:", M_RD_Change_FastQSaveLoad,    'd'},
    {1,"Play internal demos:",                M_RD_Change_NoInternalDemos,  'd'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"", /* First page >   */               M_RD_Choose_Gameplay_1,       'n'},
    {1,"", /* < Prev page > */                M_RD_Choose_Gameplay_3,       'p'},
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_4 =
{
    rd_gameplay_4_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_4,
    M_RD_Draw_Gameplay_4,
    35+ORIGWIDTH_DELTA,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Gameplay_Menu_1_Rus[]=
{
    {1,",hfqnvfggbyu:",                     M_RD_Change_Brightmaps,     ','},   // Брайтмаппинг
    {1,"Bvbnfwbz rjynhfcnyjcnb:",           M_RD_Change_FakeContrast,   'b'},   // Имитация контрастности
    {1,"Ghjphfxyjcnm j,]trnjd:",            M_RD_Change_Transparency,   'g'},   // Прозрачность объектов
    {1,"Hfpyjwdtnyst 'ktvtyns $:",          M_RD_Change_ColoredHUD,     'h'},   // Разноцветные элементы HUD
    {1,"Hfpyjwdtnyfz rhjdm b nhegs:",       M_RD_Change_ColoredBlood,   'h'},   // Разноцветная кровь и трупы
    {1,"ekexityyfz fybvfwbz ;blrjcntq:",    M_RD_Change_SwirlingLiquids,'e'},   // Улучшенная анимация жидкостей
    {1,"ytezpdbvjcnm jrhfibdftn yt,j:",     M_RD_Change_InvulSky,       'y'},   // Неуязвимость окрашивает небо
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"",                                  M_RD_Choose_Gameplay_2,     'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_4,     'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_1_Rus =
{
    rd_gameplay_1_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_1_Rus,
    M_RD_Draw_Gameplay_1,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_2_Rus[]=
{
    {1,"Pderb ghb ds[jlt bp buhs:",         M_RD_Change_ExitSfx,            'p'},   // Звук при выходе из игры
    {1,"Pder hfplfdkbdfybz nhegjd:",        M_RD_Change_CrushingSfx,        'p'},   // Звук раздавливания трупов
    {1,"Jlbyjxysq pder ,scnhjq ldthb:",     M_RD_Change_BlazingSfx,         'j'},   // Одиночный звук быстрой двери
    {1,"J,ofz nhtdjuf e vjycnhjd:",         M_RD_Change_AlertSfx,           'j'},   // Общая тревога у монстров
    {-1,"",0,'\0'},                                                                 //
    {1,"Cjj,ofnm j yfqltyyjv nfqybrt:",     M_RD_Change_SecretNotify,       'c'},   // Сообщать о найденном тайнике
    {1,"jnhbwfntkmyjt pljhjdmt d $:",       M_RD_Change_NegativeHealth,     'j'},   // Отрицательное здоровье в HUD
    {1,"Byahfptktysq dbpjh jcdtotybz:",     M_RD_Change_InfraGreenVisor,    'b'},   // Инфразеленый визор освещения
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"",                                  M_RD_Choose_Gameplay_3,         'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_1,         'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_2_Rus =
{
    rd_gameplay_2_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_2_Rus,
    M_RD_Draw_Gameplay_2,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_3_Rus[]=
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
    {1,"Edtkbxtyysq hfpvth:",               M_RD_Change_CrosshairScale,     'e'},   // Увеличенный размер
    {1,"",                                  M_RD_Choose_Gameplay_4,         'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_2,         'y'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_3_Rus =
{
    rd_gameplay_3_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_3_Rus,
    M_RD_Draw_Gameplay_3,
    35+ORIGWIDTH_DELTA,45,
    0
};

menuitem_t RD_Gameplay_Menu_4_Rus[]=
{
    {1,"ecnhfyznm jib,rb jhbu> ehjdytq:",   M_RD_Change_FixMapErrors,       'b'},   // Устранять ошибки ориг. уровней
    {1,"pthrfkmyjt jnhf;tybt ehjdytq:",     M_RD_Change_FlipLevels,         'p'},   // Зеркальное отражение уровней
    {1,"Ljgjkybntkmyst kbwf buhjrf:",       M_RD_Change_ExtraPlayerFaces,   'a'},   // Дополнительные лица игрока
    {1,"'ktvtynfkm ,tp juhfybxtybz lei:",   M_RD_Change_LostSoulsQty,       'a'},   // Элементаль без ограничения душ
    {1,"gjdsityyfz fuhtccbdyjcnm lei:",     M_RD_Change_LostSoulsAgr,       'a'},   // Повышенная агрессивность душ
    {1,"jnrk.xbnm pfghjc ,> pfuheprb:",     M_RD_Change_FastQSaveLoad,      'a'},   // Отключить запрос б. загрузки
    {1,"Ghjbuhsdfnm ltvjpfgbcb:",           M_RD_Change_NoInternalDemos,    'a'},   // Проигрывать демозаписи
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},                                                                 //
    {1,"",                                  M_RD_Choose_Gameplay_1,         'n'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_3,         'p'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_4_Rus =
{
    rd_gameplay_4_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_4_Rus,
    M_RD_Draw_Gameplay_4,
    35+ORIGWIDTH_DELTA,45,
    0
};


// =============================================================================
// [JN] VANILLA OPTIONS MENU
// =============================================================================

// -----------------------------------------------------------------------------
// Main Menu
// -----------------------------------------------------------------------------

enum
{
    vanilla_endgame,
    vanilla_messages,
    vanilla_detail,
    vanilla_scrnsize,
    vanilla_option_empty1,
    vanilla_mousesens,
    vanilla_option_empty2,
    vanilla_soundvol,
    vanilla_opt_end
} vanilla_options_e;

// ------------
// English menu
// ------------

menuitem_t Vanilla_Options_Menu[]=
{
    {1,"M_ENDGAM", M_EndGame,               'e'},
    {1,"M_MESSG",  M_RD_Change_Messages,    'm'},
    {1,"M_DETAIL", M_RD_Change_Detail,      'g'},
    {2,"M_SCRNSZ", M_RD_Change_ScreenSize,  's'},
    {-1,"",0,'\0'},
    {2,"M_MSENS",  M_RD_Change_Sensitivity, 'm'},
    {-1,"",0,'\0'},
    {1,"M_SVOL",   M_RD_Choose_Audio,       's'}
};

menu_t  Vanilla_OptionsDef =
{
    vanilla_opt_end,
    &MainDef,
    Vanilla_Options_Menu,
    M_Vanilla_DrawOptions,
    60+ORIGWIDTH_DELTA,37,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t Vanilla_Options_Menu_Rus[]=
{
    {1,"Pfrjyxbnm buhe", M_EndGame,               'p'}, // Закончить игру
    {1,"Cjj,otybz# ",    M_RD_Change_Messages,    'c'}, // Сообщения
    {1,"Ltnfkbpfwbz#",   M_RD_Change_Detail,      'l'}, // Детализация:
    {2,"Hfpvth \'rhfyf", M_RD_Change_ScreenSize,  'h'}, // Размер экрана
    {-1,"",0,'\0'},
    {2,"Crjhjcnm vsib",  M_RD_Change_Sensitivity, 'c'}, // Скорость мыши
    {-1,"",0,'\0'},
    {1,"Uhjvrjcnm",      M_RD_Choose_Audio,       'u'}  // Громкость
};

menu_t  Vanilla_OptionsDef_Rus =
{
    vanilla_opt_end,
    &MainDef,
    Vanilla_Options_Menu_Rus,
    M_Vanilla_DrawOptions,
    60+ORIGWIDTH_DELTA,37,
    0
};

// -----------------------------------------------------------------------------
// Sound Menu
// -----------------------------------------------------------------------------

enum
{
    vanilla_sfx_vol,
    vanilla_sfx_empty1,
    vanilla_music_vol,
    vanilla_sfx_empty2,
    vanilla_sound_end
} vanilla_sound_e;

// ------------
// English menu
// ------------

menuitem_t Vanilla_SoundMenu[]=
{
    {2,"M_SFXVOL", M_RD_Change_SfxVol,   's'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL", M_RD_Change_MusicVol, 'm'},
    {-1,"",0,'\0'}
};

menu_t  Vanilla_Audio_Def =
{
    vanilla_sound_end,
    &Vanilla_OptionsDef,
    Vanilla_SoundMenu,
    M_Vanilla_DrawSound,
    80+ORIGWIDTH_DELTA,64,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t Vanilla_SoundMenu_Rus[]=
{
    {2,"Pder",   M_RD_Change_SfxVol,   's'},  // Звук
    {-1,"",0,'\0'},
    {2,"Vepsrf", M_RD_Change_MusicVol, 'm'}, // Музыка
    {-1,"",0,'\0'}
};

menu_t  Vanilla_Audio_Def_Rus =
{
    vanilla_sound_end,
    &Vanilla_OptionsDef_Rus,
    Vanilla_SoundMenu_Rus,
    M_Vanilla_DrawSound,
    80+ORIGWIDTH_DELTA,64,
    0
};


// =============================================================================
// [JN] VANILLA OPTIONS MENU: DRAWING
// =============================================================================

void M_Vanilla_DrawOptions(void)
{
    if (english_language)
    {
    // - "OPTIONS" title -------------------------------------------------------
    V_DrawPatch(108 + ORIGWIDTH_DELTA, 15, 
                W_CacheLumpName(DEH_String("M_OPTTTL"), PU_CACHE));

    // - Messages --------------------------------------------------------------
    V_DrawPatch(179 + ORIGWIDTH_DELTA, 53, 
                W_CacheLumpName(DEH_String(showMessages == 1 ?
                                           "M_MSGON" : "M_MSGOFF"), PU_CACHE));

    // - Graphic Detail --------------------------------------------------------
    V_DrawPatch(235 + ORIGWIDTH_DELTA, 69, 
                W_CacheLumpName(DEH_String(detailLevel == 1 ?
                                           "M_GDLOW" : "M_GDHIGH"), PU_CACHE));
    }
    else
    {
    // - "НАСТРОЙКИ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(15, "YFCNHJQRB");
    
    M_WriteTextBig_RUS(206 + ORIGWIDTH_DELTA, 53, 
                       showMessages == 1 ? "drk/" : "dsrk/");

    M_WriteTextBig_RUS(224 + ORIGWIDTH_DELTA, 69, 
                       detailLevel == 1 ? "ybp/" : "dsc/");
    }

    // - Screen size slider ----------------------------------------------------
#ifdef WIDESCREEN
    // [JN] Wide screen: only 6 sizes are available
    M_DrawThermo(60 + ORIGWIDTH_DELTA, 102, 6, screenSize);
#else
    M_DrawThermo(60 + ORIGWIDTH_DELTA, 102, 12, screenSize);
#endif

    // - Mouse sensivity slider ------------------------------------------------
    M_DrawThermo(60 + ORIGWIDTH_DELTA, 134, 10, mouseSensitivity);
}

void M_Vanilla_DrawSound(void)
{
    if (english_language)
    {
    // - "Sound volume" title --------------------------------------------------
    V_DrawPatch (60 + ORIGWIDTH_DELTA, 38, 
                 W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));
    }
    else
    {
    // - "ГРОМКОСТЬ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(38, "UHJVRJCNM");
    }

    // - Sfx volume slider -----------------------------------------------------
    M_DrawThermo(80+ORIGWIDTH_DELTA, 81, 16, sfxVolume);

    // - Music volume slider ---------------------------------------------------
    M_DrawThermo(80+ORIGWIDTH_DELTA, 113, 16, musicVolume);
}

// =============================================================================
// [JN] NEW OPTIONS MENU: DRAWING
// =============================================================================

// -----------------------------------------------------------------------------
// Main Options menu
// -----------------------------------------------------------------------------

void M_RD_Draw_Options(void)
{
    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "OPTIONS");
    else
    M_WriteTextBigCentered_RUS(12, "YFCNHJQRB");    // НАСТРОЙКИ
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
    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "RENDERING OPTIONS");
    else
    M_WriteTextBigCentered_RUS(12, "YFCNHJQRB DBLTJ");    // НАСТРОЙКИ ВИДЕО

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Rendering");
        dp_translation = NULL;

        // - Vertical synchronization ------------------------------------------
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(216 + ORIGWIDTH_DELTA, 45, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(216 + ORIGWIDTH_DELTA, 45, vsync == 1 ? "on" : "off");
        }

        // - Fix aspect ratio --------------------------------------------------
#ifdef WIDESCREEN
        M_WriteTextSmall_ENG(155 + ORIGWIDTH_DELTA, 55, aspect_ratio_correct == 1 ? "16:9" : "off");
#else
        M_WriteTextSmall_ENG(155 + ORIGWIDTH_DELTA, 55, aspect_ratio_correct == 1 ? "4:3" : "off");
#endif

        // - Uncapped framerate ------------------------------------------------
        if (uncapped_fps)
        {
            M_WriteTextSmall_ENG(184 + ORIGWIDTH_DELTA, 65, "on");
        }
        else
        {
            M_WriteTextSmall_ENG(184 + ORIGWIDTH_DELTA, 65, "35 fps");
        }

        // - Show FPS counter --------------------------------------------------
        M_WriteTextSmall_ENG(162 + ORIGWIDTH_DELTA, 75, show_fps == 1 ? "on" : "off");

        // - Pixel scaling -----------------------------------------------------
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(135 + ORIGWIDTH_DELTA, 85, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(135 + ORIGWIDTH_DELTA, 85, smoothing == 1 ? "smooth" : "sharp");
        }

        // - Video rendered ----------------------------------------------------
        if (force_software_renderer == 1)
        M_WriteTextSmall_ENG(146 + ORIGWIDTH_DELTA, 95, "software (cpu)");
        else
        M_WriteTextSmall_ENG(146 + ORIGWIDTH_DELTA, 95, "hardware (gpu)");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 105, "Extra");
        dp_translation = NULL;

        // - Show disk icon ----------------------------------------------------
        M_WriteTextSmall_ENG(138 + ORIGWIDTH_DELTA, 115, show_diskicon == 1 ? "on" : "off");

        // - Screen wiping effect ----------------------------------------------
        M_WriteTextSmall_ENG(187 + ORIGWIDTH_DELTA, 125, screen_wiping == 1 ? "on" : "off");

        // - Screenshot format -------------------------------------------------
        M_WriteTextSmall_ENG(174 + ORIGWIDTH_DELTA, 135, png_screenshots == 1 ? "png" : "pcx");
    }
    else
    {
        //
        // Рендеринг
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "htylthbyu");
        dp_translation = NULL;

        // - Вертикальная синхронизация ----------------------------------------
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(249 + ORIGWIDTH_DELTA, 45, "y/l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(249 + ORIGWIDTH_DELTA, 45, vsync == 1 ? "drk" : "dsrk");
        }

        // - Фиксировать соотношение сторон ------------------------------------
#ifdef WIDESCREEN
        M_WriteTextSmall_RUS(279 + ORIGWIDTH_DELTA, 55, aspect_ratio_correct == 1 ? "16:9" : "dsrk");
#else
        M_WriteTextSmall_RUS(279 + ORIGWIDTH_DELTA, 55, aspect_ratio_correct == 1 ? "4:3" : "dsrk");
#endif

        // - Ограничение кадровой частоты  -------------------------------------
        if (uncapped_fps)
        {
            M_WriteTextSmall_RUS(260 + ORIGWIDTH_DELTA, 65, "dsrk");
        }
        else
        {
            M_WriteTextSmall_ENG(260 + ORIGWIDTH_DELTA, 65, "35 fps");
        }

        // - Счетчик кадровой частоты ------------------------------------------
        M_WriteTextSmall_RUS(227 + ORIGWIDTH_DELTA, 75, show_fps == 1 ? "drk" : "dsrk");

        // - Пиксельное сглаживание --------------------------------------------
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(219 + ORIGWIDTH_DELTA, 85, "y/l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(219 + ORIGWIDTH_DELTA, 85, smoothing == 1 ? "drk" : "dsrk");
        }

        // - Обработка видео ---------------------------------------------------
        M_WriteTextSmall_RUS(160 + ORIGWIDTH_DELTA, 95, force_software_renderer == 1 ? "ghjuhfvvyfz" : "fggfhfnyfz");
        if (force_software_renderer == 1)
        M_WriteTextSmall_ENG(254 + ORIGWIDTH_DELTA, 95, "(cpu)");
        else
        M_WriteTextSmall_ENG(244 + ORIGWIDTH_DELTA, 95, "(gpu)");

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 105, "ljgjkybntkmyj");
        dp_translation = NULL;

        // - Отображать значок дискеты -----------------------------------------
        M_WriteTextSmall_RUS(241 + ORIGWIDTH_DELTA, 115, show_diskicon == 1 ? "drk" : "dsrk");

        // - Плавная смена экранов ---------------------------------------------
        M_WriteTextSmall_RUS(204 + ORIGWIDTH_DELTA, 125, screen_wiping == 1 ? "drk" : "dsrk");

        // - Формат скриншотов -------------------------------------------------
        M_WriteTextSmall_ENG(180 + ORIGWIDTH_DELTA, 135, png_screenshots == 1 ? "png" : "pcx");
    }
}

void M_RD_Change_VSync(int choice)
{
    // [JN] Disable "vsync" toggling in sofrware renderer
    if (force_software_renderer == 1)
    return;

    choice = 0;
    vsync = 1 - vsync;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_RD_Change_AspectRatio(int choice)
{
    choice = 0;
    aspect_ratio_correct = 1 - aspect_ratio_correct;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_Uncapped(int choice)
{
    choice = 0;
    uncapped_fps = 1 - uncapped_fps;
}

void M_RD_Change_FPScounter(int choice)
{
    choice = 0;
    show_fps = 1 - show_fps;
}

void M_RD_Change_DiskIcon(int choice)
{
    choice = 0;
    show_diskicon = 1 - show_diskicon;
}

void M_RD_Change_Smoothing(int choice)
{
    // [JN] Disable "vsync" toggling in sofrware renderer
    if (force_software_renderer == 1)
    return;

    choice = 0;
    smoothing = 1 - smoothing;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_Wiping(int choice)
{
    choice = 0;
    screen_wiping = 1 - screen_wiping;
}

void M_RD_Change_Screenshots(int choice)
{
    choice = 0;
    png_screenshots = 1 - png_screenshots;
}

void M_RD_Change_Renderer(int choice)
{
    choice = 0;
    force_software_renderer = 1 - force_software_renderer;

    // Do a full graphics reinitialization
    I_InitGraphics();
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}


// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

void M_RD_Choose_Display(int choice)
{
    M_SetupNextMenu(english_language ?
                    &RD_Display_Def :
                    &RD_Display_Def_Rus);
}

void M_RD_Draw_Display(void)
{
    char    num[4];

    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "DISPLAY OPTIONS");
    else
    M_WriteTextBigCentered_RUS(12, "YFCNHJQRB \"RHFYF");  // НАСТРОЙКИ ЭКРАНА

    // Draw screen size slider
#ifdef WIDESCREEN
    // [JN] Wide screen: only 6 sizes are available
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 54, 6, screenSize);

    // Draw numerical representation of slider position
    M_snprintf(num, 4, "%3d", screenblocks);
    M_WriteTextSmall_ENG(96+ORIGWIDTH_DELTA, 55, num);
#else
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 54, 12, screenSize);

    // Draw numerical representation of slider position
    M_snprintf(num, 4, "%3d", screenblocks);
    M_WriteTextSmall_ENG(145+ORIGWIDTH_DELTA, 55, num);
#endif

    // Draw gamma-correction slider
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 74, 18, usegamma);

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Screen");
        dp_translation = NULL;

        // - Detail level ------------------------------------------------------
        M_WriteTextSmall_ENG(130 + ORIGWIDTH_DELTA, 85, detailLevel == 1 ? "low" : "high");

        // - Local time --------------------------------------------------------
        M_WriteTextSmall_ENG(116 + ORIGWIDTH_DELTA, 95, local_time == 1 ? "on" : "off");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 105, "Messages and texts");  
        dp_translation = NULL;

        // - Messages ----------------------------------------------------------
        M_WriteTextSmall_ENG(105 + ORIGWIDTH_DELTA, 115, showMessages == 1 ? "on" : "off");

        // - Messages color ----------------------------------------------------
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "n/a");
        }
        else
        {
            if (messages_color == 1)
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "green"); dp_translation = NULL; }
            else if (messages_color == 2)
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "blue"); dp_translation = NULL; }
            else if (messages_color == 3)
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "gold"); dp_translation = NULL; }
            else if (messages_color == 4)
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "gray"); dp_translation = NULL; }
            else if (messages_color == 5)
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "tan"); dp_translation = NULL; }
            else if (messages_color == 6)
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "brown"); dp_translation = NULL; }
            else if (messages_color == 7)
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "brick"); dp_translation = NULL; }
            else
            { dp_translation = NULL; M_WriteTextSmall_ENG(149 + ORIGWIDTH_DELTA, 125, "standard"); }
        }

        // - Text casts shadows ------------------------------------------------
        M_WriteTextSmall_ENG(177 + ORIGWIDTH_DELTA, 135, draw_shadowed_text == 1 ? "on" : "off");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 145, "Automap");
        dp_translation = NULL;
    }
    else
    {
        //
        // Экран
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "\'rhfy");
        dp_translation = NULL;

        // - Уровень детализации -----------------------------------------------
        M_WriteTextSmall_RUS(193 + ORIGWIDTH_DELTA, 85, detailLevel == 1 ? "ybprbq" : "dscjrbq");

        // - Системное время ---------------------------------------------------
        M_WriteTextSmall_RUS(161 + ORIGWIDTH_DELTA, 95, local_time == 1 ? "drk" : "dsrk");

        //
        // Сообщения и тексты
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 105, "cjj,otybz b ntrcns");  
        dp_translation = NULL;

        // - Отображение сообщений ---------------------------------------------
        M_WriteTextSmall_RUS(214 + ORIGWIDTH_DELTA, 115, showMessages == 1 ? "drk" : "dsrk");

        // - Цвет сообщений ----------------------------------------------------
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "y/l"); // н/д
        }
        else
        {
            if (messages_color == 1) // Зеленый
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "ptktysq"); dp_translation = NULL; }
            else if (messages_color == 2) // Синий
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "cbybq"); dp_translation = NULL; }
            else if (messages_color == 3) // Желтый
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, ";tknsq"); dp_translation = NULL; }
            else if (messages_color == 4) // Белый
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, ",tksq"); dp_translation = NULL; }
            else if (messages_color == 5) // Бежевый
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, ",t;tdsq"); dp_translation = NULL; }
            else if (messages_color == 6) // Коричневый
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "rjhbxytdsq"); dp_translation = NULL; }
            else if (messages_color == 7) // Розовый
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "hjpjdsq"); dp_translation = NULL; }
            else                            // Стандартный (красный)
            { dp_translation = NULL; M_WriteTextSmall_RUS(155 + ORIGWIDTH_DELTA, 125, "cnfylfhnysq"); }
        }

        // - Тексты отбрасывают тень -------------------------------------------
        M_WriteTextSmall_RUS(226 + ORIGWIDTH_DELTA, 135, draw_shadowed_text == 1 ? "drk" : "dsrk");

        //
        // Карта
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 145, "rfhnf");
        dp_translation = NULL;
    }
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

#ifdef WIDESCREEN
    // Wide screen: don't allow unsupported (bordered) views
    // screenblocks - config file variable
    if (screenblocks < 9)
        screenblocks = 9;
    if (screenblocks > 14)
        screenblocks = 14;
    
    // screenSize - slider variable/lenght
    if (screenSize < 0)
        screenSize = 0;
    if (screenSize > 5)
        screenSize = 5;
#endif

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
    I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                                     "PALFIX" :
                                                     "PLAYPAL"),
                                                     PU_CACHE) + 
                                                     st_palette * 768);
    players[consoleplayer].message = DEH_String(english_language ? 
                                               gammamsg[usegamma] :
                                               gammamsg_rus[usegamma]);
}

void M_RD_Change_Detail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
    players[consoleplayer].message = DEH_String(english_language ?
                                     DETAILHI : DETAILHI_RUS);
    else
    players[consoleplayer].message = DEH_String(english_language ?
                                     DETAILLO : DETAILLO_RUS);
}

void M_RD_Change_Messages(int choice)
{
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
    players[consoleplayer].message = DEH_String(english_language ?
                                     MSGOFF : MSGOFF_RUS);
    else
    players[consoleplayer].message = DEH_String(english_language ?
                                     MSGON : MSGON_RUS);

    message_dontfuckwithme = true;
}

void M_RD_Change_LocalTime(int choice)
{
    choice = 0;
    local_time = 1 - local_time;
}

void M_RD_Change_MessagesColor(int choice)
{
    static char msgcolor[32];

    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
    return;
    
    choice = 0;
    messages_color++;
    
    if (messages_color > 7)
        messages_color = 0;

    // Print example message
    M_snprintf(msgcolor, sizeof(msgcolor), english_language ? 
                                           "Pickup message example" :
                                           "ghjdthrf wdtnf cjj,otybz");
    players[consoleplayer].message = msgcolor;
}

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

void M_RD_Choose_AutomapSettings(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Automap_Def :
                    &RD_Automap_Def_Rus);
}

void M_RD_Draw_AutomapSettings(void)
{
    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "AUTOMAP OPTIONS");
    else
    M_WriteTextBigCentered_RUS(12, "YFCNHJQRB RFHNS");    // НАСТРОЙКИ КАРТЫ

    if (english_language)
    {
        // - Automap colors (English only names, different placement) ----------
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(170 + ORIGWIDTH_DELTA, 35, "n/a");
        }
        else
        {
            if (automap_color == 0)
            M_WriteTextSmall_ENG (170 + ORIGWIDTH_DELTA, 35, "doom");
            else if (automap_color == 1)
            M_WriteTextSmall_ENG (170 + ORIGWIDTH_DELTA, 35, "boom");
            else if (automap_color == 2)
            M_WriteTextSmall_ENG (170 + ORIGWIDTH_DELTA, 35, "jaguar");
            else if (automap_color == 3)
            M_WriteTextSmall_ENG (170 + ORIGWIDTH_DELTA, 35, "raven");
            else
            M_WriteTextSmall_ENG (170 + ORIGWIDTH_DELTA, 35, "strife");
        }

        // - Level stats -------------------------------------------------------
        M_WriteTextSmall_ENG(159 + ORIGWIDTH_DELTA, 45, automap_stats == 1 ? "on" : "off");
        
        // - Overlay mode ------------------------------------------------------
        M_WriteTextSmall_ENG(170 + ORIGWIDTH_DELTA, 55, automap_overlay == 1 ? "on" : "off");

        // - Rotate mode -------------------------------------------------------
        M_WriteTextSmall_ENG(163 + ORIGWIDTH_DELTA, 65, automap_rotate == 1 ? "on" : "off");

        // - Follow mode -------------------------------------------------------
        M_WriteTextSmall_ENG(164 + ORIGWIDTH_DELTA, 75, automap_follow == 1 ? "on" : "off");

        // - Grid --------------------------------------------------------------
        M_WriteTextSmall_ENG(106 + ORIGWIDTH_DELTA, 85, automap_grid == 1 ? "on" : "off");
    }
    else
    {
        // - Automap colors (English only names, different placement) ----------
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(191 + ORIGWIDTH_DELTA, 35, "y/l"); // н/д
        }
        else
        {
            if (automap_color == 0)
            M_WriteTextSmall_ENG (191 + ORIGWIDTH_DELTA, 35, "doom");
            else if (automap_color == 1)
            M_WriteTextSmall_ENG (191 + ORIGWIDTH_DELTA, 35, "boom");
            else if (automap_color == 2)
            M_WriteTextSmall_ENG (191 + ORIGWIDTH_DELTA, 35, "jaguar");
            else if (automap_color == 3)
            M_WriteTextSmall_ENG (191 + ORIGWIDTH_DELTA, 35, "raven");
            else
            M_WriteTextSmall_ENG (191 + ORIGWIDTH_DELTA, 35, "strife");
        }
        
        // - Статистика уровня -------------------------------------------------
        M_WriteTextSmall_RUS(210 + ORIGWIDTH_DELTA, 45, automap_stats == 1 ? "drk" : "dsrk");
        
        // - Режим наложения ---------------------------------------------------
        M_WriteTextSmall_RUS(203 + ORIGWIDTH_DELTA, 55, automap_overlay == 1 ? "drk" : "dsrk");

        // - Режим вращения ---------------------------------------------------
        M_WriteTextSmall_RUS(194 + ORIGWIDTH_DELTA, 65, automap_rotate == 1 ? "drk" : "dsrk");

        // - Режим следования ---------------------------------------------------
        M_WriteTextSmall_RUS(208 + ORIGWIDTH_DELTA, 75, automap_follow == 1 ? "drk" : "dsrk");

        // - Сетка -------------------------------------------------------------
        M_WriteTextSmall_RUS(118 + ORIGWIDTH_DELTA, 85, automap_grid == 1 ? "drk" : "dsrk");
    }
}

void M_RD_Change_AutomapColor(int choice)
{
    // [JN] Disable automap colors changing in Jaguar
    if (gamemission == jaguar)
    return;

    choice = 0;
    automap_color++;

    if (automap_color > 4)
        automap_color = 0;
}

void M_RD_Change_AutomapOverlay(int choice)
{
    choice = 0;
    automap_overlay = 1 - automap_overlay;
}

void M_RD_Change_AutomapStats(int choice)
{
    choice = 0;
    automap_stats = 1 - automap_stats;
}

void M_RD_Change_AutomapRotate(int choice)
{
    choice = 0;
    automap_rotate = 1 - automap_rotate;
}

void M_RD_Change_AutomapFollow(int choice)
{
    choice = 0;
    automap_follow = 1 - automap_follow;
}

void M_RD_Change_AutomapGrid(int choice)
{
    choice = 0;
    automap_grid = 1 - automap_grid;
}

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------

void M_RD_Choose_Audio(int choice)
{
    if (vanillaparm)
    {
        M_SetupNextMenu(english_language ?
                        &Vanilla_Audio_Def :
                        &Vanilla_Audio_Def_Rus);
    }
    else
    {
        M_SetupNextMenu(english_language ?
                        &RD_Audio_Def :
                        &RD_Audio_Def_Rus);
    }
}

void M_RD_Draw_Audio(void)
{
    char    num[4];

    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "SOUND OPTIONS");
    else
    M_WriteTextBigCentered_RUS(12, "YFCNHJQRB PDERF");    // НАСТРОЙКИ ЗВУКА

    // Draw SFX volume slider
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 54, 16, sfxVolume);
    // Draw numerical representation of SFX volume
    M_snprintf(num, 4, "%3d", sfxVolume);
    M_WriteTextSmall_ENG(177+ORIGWIDTH_DELTA, 55, num);

    // Draw music volume slider
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 74, 16, musicVolume);
    // Draw numerical representation of music volume
    M_snprintf(num, 4, "%3d", musicVolume);
    M_WriteTextSmall_ENG(177+ORIGWIDTH_DELTA, 75, num);

    // Draw SFX channels slider
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 104, 16, snd_channels / 4 - 1);
    // Draw numerical representation of channels
    M_snprintf(num, 4, "%3d", snd_channels);
    M_WriteTextSmall_ENG(177+ORIGWIDTH_DELTA, 105, num);

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "volume");
        dp_translation = NULL;

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 85, "extra");
        dp_translation = NULL;

        // - Sfx mode ----------------------------------------------------------
        M_WriteTextSmall_ENG(104 + ORIGWIDTH_DELTA, 115, snd_monomode == 1 ? "mono" : "stereo");

        // - Pitch-shifted sounds ----------------------------------------------
        M_WriteTextSmall_ENG(186 + ORIGWIDTH_DELTA, 125, snd_pitchshift == 1 ? "on" : "off");
    }
    else
    {
        //
        // Громкость
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "uhjvrjcnm");
        dp_translation = NULL;

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 85, "ljgjkybntkmyj");
        dp_translation = NULL;

        // - Режим звука -------------------------------------------------------
        M_WriteTextSmall_RUS(132 + ORIGWIDTH_DELTA, 115, snd_monomode == 1 ? "vjyj" : "cnthtj");

        // - Произвольный питч-шифтинг -----------------------------------------
        M_WriteTextSmall_RUS(242 + ORIGWIDTH_DELTA, 125, snd_pitchshift == 1 ? "drk" : "dsrk");
    }
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
        if (snd_channels > 4)
            snd_channels -= 4;
        break;
    
        case 1:
        if (snd_channels < 64)
            snd_channels += 4;
        break;
    }

    // Reallocate sound channels
    S_ChannelsRealloc();
}

void M_RD_Change_SndMode(int choice)
{
    choice = 0;
    snd_monomode = 1 - snd_monomode;

    // Update stereo separation
    S_UpdateStereoSeparation();
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
    M_SetupNextMenu(english_language ?
                    &RD_Controls_Def :
                    &RD_Controls_Def_Rus);
}

void M_RD_Draw_Controls(void)
{
    char    num[4];

    // Write capitalized title
    if (english_language)
    M_WriteTextBigCentered_ENG(12, "CONTROL SETTINGS");
    else
    M_WriteTextBigCentered_RUS(12, "EGHFDKTYBT");     // УПРАВЛЕНИЕ

    // Draw mouse sensivity slider
    M_DrawThermo_Small(35+ORIGWIDTH_DELTA, 74, 17, mouseSensitivity);
    // Draw numerical representation of mouse sensivity
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    M_WriteTextSmall_ENG(189+ORIGWIDTH_DELTA, 75, num);

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "movement");
        dp_translation = NULL;

        // - Always run --------------------------------------------------------
        M_WriteTextSmall_ENG(119 + ORIGWIDTH_DELTA, 45, joybspeed >= 20 ? "on" : "off");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 55, "mouse");
        dp_translation = NULL;

        // - Mouse look --------------------------------------------------------
        M_WriteTextSmall_ENG(119 + ORIGWIDTH_DELTA, 85, mlook ? "on" : "off");

        // - Vertical movement -------------------------------------------------
        M_WriteTextSmall_ENG(171 + ORIGWIDTH_DELTA, 95, !novert ? "on" : "off");
    }
    else
    {
        //
        // Передвижение
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "gthtldb;tybt");
        dp_translation = NULL;

        // - Режим постоянного бега --------------------------------------------
        M_WriteTextSmall_RUS(216 + ORIGWIDTH_DELTA, 45, joybspeed >= 20 ? "drk" : "dsrk");

        //
        // Мышь
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 55, "vsim");
        dp_translation = NULL;

        // - Обзор мышью -------------------------------------------------------
        M_WriteTextSmall_RUS(135 + ORIGWIDTH_DELTA, 85, mlook ? "drk" : "dsrk");

        // - Вертикальное перемещение ------------------------------------------
        M_WriteTextSmall_RUS(235 + ORIGWIDTH_DELTA, 95, !novert ? "drk" : "dsrk");
    }
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

void M_RD_Change_Novert(int choice)
{
    choice = 0;
    novert = 1 - novert;
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
    M_SetupNextMenu(english_language ? 
                    &RD_Gameplay_Def_1 :
                    &RD_Gameplay_Def_1_Rus);
}

void M_RD_Choose_Gameplay_2(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Gameplay_Def_2 :
                    &RD_Gameplay_Def_2_Rus);
}

void M_RD_Choose_Gameplay_3(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Gameplay_Def_3 :
                    &RD_Gameplay_Def_3_Rus);
}

void M_RD_Choose_Gameplay_4(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Gameplay_Def_4 :
                    &RD_Gameplay_Def_4_Rus);
}

void M_RD_Jaguar_Menu_Background(void)
{
    if (gamemission != jaguar)
    return;

    inhelpscreens = true;
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
    V_DrawPatch(0 + ORIGWIDTH_DELTA, 0, W_CacheLumpName(DEH_String("INTERPIC"), 
                                                        PU_CACHE));
}

void M_RD_Draw_Gameplay_1(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    M_WriteTextBigCentered_ENG(10, "GAMEPLAY FEATURES");
    else
    M_WriteTextBigCentered_RUS(10, "YFCNHJQRB UTQVGKTZ");    // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Graphical");
        dp_translation = NULL;

        // - Brightmaps --------------------------------------------------------
        if (brightmaps) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(119 + ORIGWIDTH_DELTA, 45, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(119 + ORIGWIDTH_DELTA, 45, RD_OFF); dp_translation = NULL; }

        // - Fake contrast -----------------------------------------------------
        if (fake_contrast) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(142 + ORIGWIDTH_DELTA, 55, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(142 + ORIGWIDTH_DELTA, 55, RD_OFF); dp_translation = NULL; }

        // - Transparency ------------------------------------------------------
        if (translucency) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(138 + ORIGWIDTH_DELTA, 65, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(138 + ORIGWIDTH_DELTA, 65, RD_OFF); dp_translation = NULL; }

        // - Colored HUD elements ----------------------------------------------
        if (colored_hud) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(195 + ORIGWIDTH_DELTA, 75, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(195 + ORIGWIDTH_DELTA, 75, RD_OFF); dp_translation = NULL; }

        // - Colored blood and corpses -----------------------------------------
        if (colored_blood) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(229 + ORIGWIDTH_DELTA, 85, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(229 + ORIGWIDTH_DELTA, 85, RD_OFF); dp_translation = NULL; }

        // - Swirling liquids --------------------------------------------------
        if (swirling_liquids) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(150 + ORIGWIDTH_DELTA, 95, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(150 + ORIGWIDTH_DELTA, 95, RD_OFF); dp_translation = NULL; }

        // - Invulnerability affects sky ---------------------------------------
        if (invul_sky) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(237 + ORIGWIDTH_DELTA, 105, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(237 + ORIGWIDTH_DELTA, 105, RD_OFF); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 145, "next page >"); 
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 155, "< last page"); 
        dp_translation = NULL;
    }
    else
    {
        //
        // Графика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "uhfabrf");
        dp_translation = NULL;

        // - Брайтмаппинг ------------------------------------------------------
        if (brightmaps) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(140 + ORIGWIDTH_DELTA, 45, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(140 + ORIGWIDTH_DELTA, 45, RD_OFF_RUS); dp_translation = NULL; }

        // - Имитация контрастности --------------------------------------------
        if (fake_contrast) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(217 + ORIGWIDTH_DELTA, 55, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(217 + ORIGWIDTH_DELTA, 55, RD_OFF_RUS); dp_translation = NULL; }

        // - Прозрачность объектов ---------------------------------------------
        if (translucency) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(207 + ORIGWIDTH_DELTA, 65, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(207 + ORIGWIDTH_DELTA, 65, RD_OFF_RUS); dp_translation = NULL; }

        // - Разноцветные элементы HUD -----------------------------------------
        if (colored_hud) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(239 + ORIGWIDTH_DELTA, 75, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(239 + ORIGWIDTH_DELTA, 75, RD_OFF_RUS); dp_translation = NULL; }

        // - Разноцветная кровь и трупы ----------------------------------------
        if (colored_blood) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(242 + ORIGWIDTH_DELTA, 85, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(242 + ORIGWIDTH_DELTA, 85, RD_OFF_RUS); dp_translation = NULL; }

        // - Улучшенная анимация жидкостей -------------------------------------
        if (swirling_liquids) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(275 + ORIGWIDTH_DELTA, 95, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(275 + ORIGWIDTH_DELTA, 95, RD_OFF_RUS); dp_translation = NULL; }

        // - Неуязвимость окрашивает небо --------------------------------------
        if (invul_sky) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(262 + ORIGWIDTH_DELTA, 105, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(262 + ORIGWIDTH_DELTA, 105, RD_OFF_RUS); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 145, RD_NEXT_RUS); 
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 155, RD_PREV_RUS); 
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_2(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    M_WriteTextBigCentered_ENG(10, "GAMEPLAY FEATURES");
    else
    M_WriteTextBigCentered_RUS(10, "YFCNHJQRB UTQVGKTZ");    // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Audible");
        dp_translation = NULL;

        // - Play exit sounds --------------------------------------------------
        if (play_exit_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(158 + ORIGWIDTH_DELTA, 45, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(158 + ORIGWIDTH_DELTA, 45, RD_OFF); dp_translation = NULL; }

        // - Sound of crushing corpses -----------------------------------------
        if (crushed_corpses_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(223 + ORIGWIDTH_DELTA, 55, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(223 + ORIGWIDTH_DELTA, 55, RD_OFF); dp_translation = NULL; }

        // - Single sound of closing blazing door ------------------------------
        if (blazing_door_fix_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(240 + ORIGWIDTH_DELTA, 65, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(240 + ORIGWIDTH_DELTA, 65, RD_OFF); dp_translation = NULL; }

        // - Monster alert waking up other monsters ----------------------------
        if (noise_alert_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(263 + ORIGWIDTH_DELTA, 75,RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(263 + ORIGWIDTH_DELTA, 75, RD_OFF); dp_translation = NULL; }
        
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 85, "Tactical");
        dp_translation = NULL;

        // Notify of revealed secrets
        if (secret_notification) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(232 + ORIGWIDTH_DELTA, 95, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(232 + ORIGWIDTH_DELTA, 95, RD_OFF); dp_translation = NULL; }
        // Show negative health
        if (negative_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(190 + ORIGWIDTH_DELTA, 105, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(190 + ORIGWIDTH_DELTA, 105, RD_OFF); dp_translation = NULL; }
        // Infragreen light amp. visor
        if (infragreen_visor) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(230 + ORIGWIDTH_DELTA, 115, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(230 + ORIGWIDTH_DELTA, 115, RD_OFF); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 145, "next page >");
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 155, "< prev page");
        dp_translation = NULL;
    }
    else
    {   
        //
        // Звук
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "Pder");
        dp_translation = NULL;

        // - Звуки при выходе из игры ------------------------------------------
        if (play_exit_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(225 + ORIGWIDTH_DELTA, 45, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(225 + ORIGWIDTH_DELTA, 45, RD_OFF_RUS); dp_translation = NULL; }

        // - Звук раздавливания трупов -----------------------------------------
        if (crushed_corpses_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(236 + ORIGWIDTH_DELTA, 55, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(236 + ORIGWIDTH_DELTA, 55, RD_OFF_RUS); dp_translation = NULL; }

        // - Одиночный звук быстро двери ---------------------------------------
        if (blazing_door_fix_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(260 + ORIGWIDTH_DELTA, 65, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(260 + ORIGWIDTH_DELTA, 65, RD_OFF_RUS); dp_translation = NULL; }

        // - Общая тревога у монстров ------------------------------------------
        if (noise_alert_sfx) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(227 + ORIGWIDTH_DELTA, 75,RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(227 + ORIGWIDTH_DELTA, 75, RD_OFF_RUS); dp_translation = NULL; }

        //
        // Тактика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 85, "Nfrnbrf");
        dp_translation = NULL;

        // - Сообщать о найденном тайнике --------------------------------------
        if (secret_notification) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(260 + ORIGWIDTH_DELTA, 95, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(260 + ORIGWIDTH_DELTA, 95, RD_OFF_RUS); dp_translation = NULL; }

        // - Отрицательное здоровье в HUD --------------------------------------
        if (negative_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(255 + ORIGWIDTH_DELTA, 105, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(255 + ORIGWIDTH_DELTA, 105, RD_OFF_RUS); dp_translation = NULL; }

        // - Инфразеленый визор освещения --------------------------------------
        if (infragreen_visor) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(266 + ORIGWIDTH_DELTA, 115, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(266 + ORIGWIDTH_DELTA, 115, RD_OFF_RUS); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 155, RD_PREV_RUS);
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_3(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    M_WriteTextBigCentered_ENG(10, "GAMEPLAY FEATURES");
    else
    M_WriteTextBigCentered_RUS(10, "YFCNHJQRB UTQVGKTZ");    // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Physical");
        dp_translation = NULL;

        // - Walk over and under monsters --------------------------------------
        if (over_under) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(250 + ORIGWIDTH_DELTA, 45, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(250 + ORIGWIDTH_DELTA, 45, RD_OFF); dp_translation = NULL; }

        // - Corpses sliding from the ledges -----------------------------------
        if (torque) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(264 + ORIGWIDTH_DELTA, 55, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(264 + ORIGWIDTH_DELTA, 55, RD_OFF); dp_translation = NULL; }

        // - Weapon bobbing while firing ---------------------------------------
        if (weapon_bobbing) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(233 + ORIGWIDTH_DELTA, 65, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(233 + ORIGWIDTH_DELTA, 65, RD_OFF); dp_translation = NULL; }

        // - Lethal pellet of a point-blank SSG --------------------------------
        if (ssg_blast_enemies) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(287 + ORIGWIDTH_DELTA, 75, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(287 + ORIGWIDTH_DELTA, 75, RD_OFF); dp_translation = NULL; }

        // - Randomly mirrored corpses -----------------------------------------
        if (randomly_flipcorpses) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(231 + ORIGWIDTH_DELTA, 85, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(231 + ORIGWIDTH_DELTA, 85, RD_OFF); dp_translation = NULL; }

        // - Floating powerups -------------------------------------------------
        if (floating_powerups) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(171 + ORIGWIDTH_DELTA, 95, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(171 + ORIGWIDTH_DELTA, 95, RD_OFF); dp_translation = NULL; }

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 105, "Crosshair");
        dp_translation = NULL;

        // - Draw crosshair ----------------------------------------------------
        if (crosshair_draw) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(146 + ORIGWIDTH_DELTA, 115, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(146 + ORIGWIDTH_DELTA, 115, RD_OFF); dp_translation = NULL; }

        // - Health indication -------------------------------------------------
        if (crosshair_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(163 + ORIGWIDTH_DELTA, 125, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(163 + ORIGWIDTH_DELTA, 125, RD_OFF); dp_translation = NULL; }

        // - Increased size ----------------------------------------------------
        if (crosshair_scale) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(140 + ORIGWIDTH_DELTA, 135, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(140 + ORIGWIDTH_DELTA, 135, RD_OFF); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 145, "next page >");
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 155, "< prev page");
        dp_translation = NULL;
    }
    else
    {
        //
        // Физика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "Abpbrf");
        dp_translation = NULL;

        // - Перемещение под/над монстрами -------------------------------------
        if (over_under) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(274 + ORIGWIDTH_DELTA, 45, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(274 + ORIGWIDTH_DELTA, 45, RD_OFF_RUS); dp_translation = NULL; }

        // - Трупы сползают с возвышений ---------------------------------------
        if (torque) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(256 + ORIGWIDTH_DELTA, 55, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(256 + ORIGWIDTH_DELTA, 55, RD_OFF_RUS); dp_translation = NULL; }

        // - Улучшенное покачивание оружия -------------------------------------
        if (weapon_bobbing) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(271 + ORIGWIDTH_DELTA, 65, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(271 + ORIGWIDTH_DELTA, 65, RD_OFF_RUS); dp_translation = NULL; }

        // - Двустволка разрывает врагов ---------------------------------------
        if (ssg_blast_enemies) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(254 + ORIGWIDTH_DELTA, 75, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(254 + ORIGWIDTH_DELTA, 75, RD_OFF_RUS); dp_translation = NULL; }

        // - Зеркалирование трупов ---------------------------------------------
        if (randomly_flipcorpses) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(207 + ORIGWIDTH_DELTA, 85, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(207 + ORIGWIDTH_DELTA, 85, RD_OFF_RUS); dp_translation = NULL; }

        // - Левитирующие сферы-артефакты --------------------------------------
        if (floating_powerups) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(275 + ORIGWIDTH_DELTA, 95, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(275 + ORIGWIDTH_DELTA, 95, RD_OFF_RUS); dp_translation = NULL; }

        //
        // Прицел
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 105, "Ghbwtk");
        dp_translation = NULL;

        // - Отображать прицел -------------------------------------------------
        if (crosshair_draw) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(180 + ORIGWIDTH_DELTA, 115, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(180 + ORIGWIDTH_DELTA, 115, RD_OFF_RUS); dp_translation = NULL; }

        // - Индикация здоровья ------------------------------------------------
        if (crosshair_health) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(186 + ORIGWIDTH_DELTA, 125, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(186 + ORIGWIDTH_DELTA, 125, RD_OFF_RUS); dp_translation = NULL; }

        // - Увеличенный размер ------------------------------------------------
        if (crosshair_scale) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(185 + ORIGWIDTH_DELTA, 135, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(185 + ORIGWIDTH_DELTA, 135, RD_OFF_RUS); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 155, RD_PREV_RUS);
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_4(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    M_WriteTextBigCentered_ENG(10, "GAMEPLAY FEATURES");
    else
    M_WriteTextBigCentered_RUS(10, "YFCNHJQRB UTQVGKTZ");    // НАСТРОЙКИ ГЕЙМПЛЕЯ

    // Write "on" / "off" strings for features
    if (english_language)
    {
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 35, "Gameplay");
        dp_translation = NULL;

        // - Fix errors of vanilla maps ----------------------------------------
        if (fix_map_errors) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(226 + ORIGWIDTH_DELTA, 45, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(226 + ORIGWIDTH_DELTA, 45, RD_OFF); dp_translation = NULL; }

        // - Flip game levels --------------------------------------------------
        if (flip_levels) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(158 + ORIGWIDTH_DELTA, 55, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(158 + ORIGWIDTH_DELTA, 55, RD_OFF); dp_translation = NULL; }

        // - Extra player faces on the HUD -------------------------------------
        if (extra_player_faces) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(255 + ORIGWIDTH_DELTA, 65, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(255 + ORIGWIDTH_DELTA, 65, RD_OFF); dp_translation = NULL; }

        // - Pain Elemental without Souls limit --------------------------------
        if (unlimited_lost_souls) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(284 + ORIGWIDTH_DELTA, 75, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(284 + ORIGWIDTH_DELTA, 75, RD_OFF); dp_translation = NULL; }

        // - More agressive lost souls -----------------------------------------
        if (agressive_lost_souls) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(222 + ORIGWIDTH_DELTA, 85, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(222 + ORIGWIDTH_DELTA, 85, RD_OFF); dp_translation = NULL; }

        // - Don't prompt for q. saving/loading --------------------------------
        if (fast_quickload) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(281 + ORIGWIDTH_DELTA, 95, RD_ON); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(281 + ORIGWIDTH_DELTA, 95, RD_OFF); dp_translation = NULL; }

        // - Play internal demos -----------------------------------------------
        if (no_internal_demos) { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_ENG(183 + ORIGWIDTH_DELTA, 105, RD_OFF); dp_translation = NULL; }
        else { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(183 + ORIGWIDTH_DELTA, 105, RD_ON); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 145, "first page >");
        M_WriteTextSmall_ENG(35 + ORIGWIDTH_DELTA, 155, "< prev page");
        dp_translation = NULL;
    }
    else
    {
        //
        // Геймплей
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 35, "Utqvgktq");
        dp_translation = NULL;

        // - Устранять ошибки ориг. уровней ------------------------------------
        if (fix_map_errors) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(269 + ORIGWIDTH_DELTA, 45, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(269 + ORIGWIDTH_DELTA, 45, RD_OFF_RUS); dp_translation = NULL; }

        // - Зеркальное отражение уровней ------------------------------------
        if (flip_levels) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(263 + ORIGWIDTH_DELTA, 55, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(263 + ORIGWIDTH_DELTA, 55, RD_OFF_RUS); dp_translation = NULL; }

        // - Дополнительные лица игрока ----------------------------------------
        if (extra_player_faces) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(247 + ORIGWIDTH_DELTA, 65, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(247 + ORIGWIDTH_DELTA, 65, RD_OFF_RUS); dp_translation = NULL; }

        // - Элементаль без ограничения Душ ------------------------------------
        if (unlimited_lost_souls) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(274 + ORIGWIDTH_DELTA, 75, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(274 + ORIGWIDTH_DELTA, 75, RD_OFF_RUS); dp_translation = NULL; }

        // - Повышенная агрессивность Душ --------------------------------------
        if (agressive_lost_souls) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(266 + ORIGWIDTH_DELTA, 85, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(266 + ORIGWIDTH_DELTA, 85, RD_OFF_RUS); dp_translation = NULL; }

        // - Отключить запрос б. загрузки --------------------------------------
        if (fast_quickload) { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(253 + ORIGWIDTH_DELTA, 95, RD_ON_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(253 + ORIGWIDTH_DELTA, 95, RD_OFF_RUS); dp_translation = NULL; }

        // - Проигрывать демозаписи --------------------------------------------
        if (no_internal_demos) { dp_translation = cr[CR_DARKRED]; M_WriteTextSmall_RUS(219 + ORIGWIDTH_DELTA, 105, RD_OFF_RUS); dp_translation = NULL; }
        else { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(219 + ORIGWIDTH_DELTA, 105, RD_ON_RUS); dp_translation = NULL; }

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + ORIGWIDTH_DELTA, 155, RD_PREV_RUS);
        dp_translation = NULL;
    }
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

void M_RD_Change_Transparency(int choice)
{
    choice = 0;
    translucency = 1 - translucency;
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

void M_RD_Change_InfraGreenVisor(int choice)
{
    choice = 0;
    infragreen_visor = 1 - infragreen_visor;

    // Update current COLORMAP
    if (infragreen_visor && players[consoleplayer].powers[pw_infrared])
    players[consoleplayer].fixedcolormap = 33;
    else if (!infragreen_visor && players[consoleplayer].powers[pw_infrared])
    players[consoleplayer].fixedcolormap = 1;
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

void M_RD_Change_CrosshairScale(int choice)
{
    choice = 0;
    crosshair_scale = 1 - crosshair_scale;
}

void M_RD_Change_FixMapErrors(int choice)
{
    choice = 0;
    fix_map_errors = 1 - fix_map_errors;
}

void M_RD_Change_FlipLevels(int choice)
{
    extern void R_ExecuteSetViewSize();
    choice = 0;
    flip_levels = 1 - flip_levels;

    // Redraw game screen
    R_ExecuteSetViewSize();

    // Update stereo separation
    S_UpdateStereoSeparation();
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

void M_RD_Change_LostSoulsAgr(int choice)
{
    choice = 0;
    agressive_lost_souls = 1 - agressive_lost_souls;
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

void M_RD_BackToDefaultsResponse(int key)
{
    static char resetmsg[24];

    if (key != key_menu_confirm)
    return;

    // Rendering
    vsync                   = 0;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    show_fps                = 0;
    show_diskicon           = 1;
    smoothing               = 0;
    screen_wiping           = 1;
    force_software_renderer = 0;

    // Display
    screenSize      = 10;
    usegamma        = 0;
    detailLevel     = 0;
    showMessages    = 1;
    local_time      = 0;

    // Automap
    automap_color   = 0;
    automap_follow  = 1;
    automap_overlay = 0;
    automap_rotate  = 0;
    automap_grid    = 0;

    // Audio
    sfxVolume       = 8;
    S_SetSfxVolume(sfxVolume * 8);
    musicVolume     = 8;
    S_SetMusicVolume(musicVolume * 8);
    snd_channels    = 32;
    S_ChannelsRealloc();
    snd_monomode    = 0;
    snd_pitchshift  = 0;

    // Controls
    joybspeed           = 29;
    mlook               = 0;
    players[consoleplayer].centering = true;
    mouseSensitivity    = 5;

    // Gameplay
    brightmaps              = 1;
    fake_contrast           = 0;
    translucency            = 1;    
    colored_hud             = 0;
    messages_color          = 0;
    colored_blood           = 1;
    swirling_liquids        = 1;
    invul_sky               = 1;
    draw_shadowed_text      = 1;

    play_exit_sfx = 1;
    crushed_corpses_sfx = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx     = 0;

    automap_stats = 1;
    secret_notification = 1;
    negative_health = 0;
    infragreen_visor = 0;

    over_under = 0;
    torque = 1;
    weapon_bobbing = 1;
    ssg_blast_enemies = 1;
    randomly_flipcorpses = 1;
    floating_powerups = 0;

    crosshair_draw = 0;
    crosshair_health = 1;
    crosshair_scale = 0;

    fix_map_errors = 1;
    flip_levels = 0;
    extra_player_faces = 1;
    unlimited_lost_souls = 1;
    agressive_lost_souls = 0;
    fast_quickload = 1;
    no_internal_demos = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }

    // Print informative message
    M_snprintf(resetmsg, sizeof(resetmsg), english_language ? 
                                           "Settings reset" :
                                           "Yfcnhjqrb c,hjitys");
    players[consoleplayer].message = resetmsg;
}

void M_RD_BackToDefaults(int choice)
{
    choice = 0;

    M_StartMessage(DEH_String(english_language ?
                              RD_DEFAULTS : RD_DEFAULTS_RUS),
                              M_RD_BackToDefaultsResponse,true);
}


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
    {1,"", M_LoadSelect,'8'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    67,38,
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
    67,38,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//

void M_ReadSaveStrings(void)
{
    FILE    *handle;
    int     i;
    char    name[256];

    for (i = 0;i < load_end;i++)
    {
        int retval;
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

        handle = fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], english_language ?
                                             EMPTYSTRING : EMPTYSTRING_RUS,
                                             SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }

        retval = fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = retval = SAVESTRINGSIZE;
    }
}


//
// M_LoadGame & Cie.
//
static int LoadDef_x = 72, LoadDef_y = 28;  // [JN] from Crispy Doom
void M_DrawLoad(void)
{
    int i;

    if (english_language)
    {
        // [JN] Use standard centered title "M_LOADG"
        V_DrawShadowedPatchDoom(LoadDef_x, LoadDef_y,
                           W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));
    }
    else
    {
        // [JN] Use own Russian capitalized and centered title: "ЗАГРУЗИТЬ ИГРУ"
        V_DrawShadowedPatchDoom(LoadDef_x, LoadDef_y, 
                                W_CacheLumpName(DEH_String("M_LGTTL"), PU_CACHE));
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);

        // [crispy] shade empty savegame slots
        if (!LoadMenu[i].status && colored_hud && !vanillaparm)
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
    int i;

    // [JN] Russian characters having -1 vertical offset because of "Й" char,
    // which may go out of vertical bounds. These conditions stands for pixel perfection
    // in both English and Russian languages, with and without -vanilla game mode.

    V_DrawShadowedPatchDoom(x - 8, y + (english_language ? 8 : 9),
                            W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0 ; i < 24 ; i++)
    {
        V_DrawShadowedPatchDoom(x, y + (english_language ? 8 : 9),
                                W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawShadowedPatchDoom(x, y + (english_language ? 8 : 9),
                            W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}


//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];

    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

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
        M_StartMessage(DEH_String(english_language ?
                                  LOADNET : LOADNET_RUS),
                                  NULL,false);
        
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 72, SaveDef_y = 28;  // [JN] from Crispy Doom
void M_DrawSave(void)
{
    int i;
	
    if (QuickSaveTitle)
    {
        if (english_language)
        {
            // [JN] Use standard centered title "M_SAVEG"
            V_DrawShadowedPatchDoom(SaveDef_x, SaveDef_y, 
                                    W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
        }
        else
        {
            // [JN] Use own Russian capitalized and centered title: "БЫСТРОЕ СОХРАНЕНИЕ"
            V_DrawShadowedPatchDoom(SaveDef_x, SaveDef_y,
                                    W_CacheLumpName(DEH_String("M_QSGTTL"), PU_CACHE));            
        }
    }
    else
    {
        if (english_language)
        {
            // [JN] Use standard centered title "M_SAVEG"
            V_DrawShadowedPatchDoom(SaveDef_x, SaveDef_y,
                                    W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
        }
        else
        {
            // [JN] Use own Russian capitalized and centered title: "СОХРАНИТЬ ИГРУ"
            V_DrawShadowedPatchDoom(SaveDef_x, SaveDef_y,
                                    W_CacheLumpName(DEH_String("M_SGTTL"), PU_CACHE));
        }
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
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
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);

    if (!strcmp(savegamestrings[choice], EMPTYSTRING))
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
        M_StartMessage(DEH_String(english_language ?
                                  SAVEDEAD : SAVEDEAD_RUS),
                                  NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
    return;

    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}


//
// M_QuickSave
//
char tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
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
        M_DoSave(quickSaveSlot);
    }
    else
    {
        DEH_snprintf(tempstring, 80, english_language ?
                                     QSPROMPT : QSPROMPT_RUS,
                                     savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickSaveResponse,true);
    }
}


//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(english_language ?
                                  QLOADNET : QLOADNET_RUS),
                                  NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(english_language ?
                                  QSAVESPOT : QSAVESPOT_RUS),
                                  NULL,false);
        return;
    }

    if (fast_quickload)
    {
        M_LoadSelect(quickSaveSlot);
    }
    else
    {
        DEH_snprintf(tempstring, 80, english_language ?
                                     QLPROMPT : QLPROMPT_RUS,
                                     savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickLoadResponse,true);
    }
}


//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    char *lumpname = "CREDIT";
    int skullx = 330+ORIGWIDTH_DELTA, skully = 175; // [JN] Wide screen support

    inhelpscreens = true;

#ifdef WIDESCREEN
    // [JN] Clean up remainings of the wide screen before drawing
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
#endif

    // [JN] Различные экраны помощи и скорректированное положение M_SKULL для разных версий игры

    switch (gameversion)
    {
        case exe_doom_1_666:    // [JN] Needed for Shareware 1.6 / 1.666
        case exe_doom_1_8:      // [JN] Needed for Shareware 1.8
        case exe_doom_1_9:
        case exe_hacx:
		if (gamemode == commercial )
        {
            // Doom 2
            lumpname = "HELP";
            skullx = 330+ORIGWIDTH_DELTA; // [JN] Wide screen support
            skully = 162;
        }

        else
        {
            // Doom 1
            // HELP2 is the first screen shown in Doom 1
            if (gamevariant == old_shareware)   // [JN] Red chars for older sharewares
            lumpname = "HELP2RED";  
            else                                // [JN] Green chars
            lumpname = "HELP2";

            if (english_language)
            {
                skullx = 280+ORIGWIDTH_DELTA;
            }
            else
            {
#ifdef WIDESCREEN
                skullx = 359;
#else
                skullx = 280;
#endif
            }
            skully = 185;
        }
        break;

        case exe_ultimate:
        case exe_chex:

        // Ultimate Doom always displays "HELP1".

        // Chex Quest version also uses "HELP1", even though it is based
        // on Final Doom.

        lumpname = "HELP1";
        break;

        case exe_final:
        case exe_final2:

        // Final Doom always displays "HELP".
        // [JN] Иконка черепа сдвинута чуть выше, по аналогии Doom 2,
        // чтобы не загораживать фразу "джойстика 2".

        lumpname = "HELP";
        skullx = 330+ORIGWIDTH_DELTA;   // [JN] Wide screen support
        skully = 165;
        break;

        default:
        I_Error(english_language ?
                "Unknown game version" :
                "Версия игры не определена");
        break;
    }

    // [JN] Для обоих Стадий Freedoom используется одинаковое положение
    // черепа. Так, чтобы он не перекрывал надписb "ПКМ" и "E".
    if (gamevariant == freedoom)
    {
        skullx = 323+ORIGWIDTH_DELTA;   // [JN] Wide screen support
        skully = 183;
    }

    // [JN] Pixel-perfect position for skull in Press Beta
    if (gamemode == pressbeta)
    {
            skullx = 330+ORIGWIDTH_DELTA;   // [JN] Wide screen support
            skully = 175;
    }

    lumpname = DEH_String(lumpname);

    // [JN] Wide screen support
    V_DrawPatch (ORIGWIDTH_DELTA, 0, W_CacheLumpName(lumpname, PU_CACHE));

    ReadDef1.x = skullx;
    ReadDef1.y = skully;
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

#ifdef WIDESCREEN
    // [JN] Clean up remainings of the wide screen before drawing
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
#endif

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered
    // [JN] Wide screen support

    if (gamevariant == old_shareware)   // [JN] Red chars for older sharewares
    V_DrawPatch(ORIGWIDTH_DELTA, 0, W_CacheLumpName(DEH_String("HELP1RED"), PU_CACHE));
    else                                // [JN] Green chars
    V_DrawPatch(ORIGWIDTH_DELTA, 0, W_CacheLumpName(DEH_String("HELP1"), PU_CACHE));

    // [JN] Wide screen: proper position for second HELP screen
    // TODO - needed?
    ReadDef2.x = 330+ORIGWIDTH_DELTA;
    ReadDef2.y = 175;
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    V_DrawPatch(94+ORIGWIDTH_DELTA, 2, W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
}


//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawShadowedPatchDoom(99+ORIGWIDTH_DELTA, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
    V_DrawShadowedPatchDoom(42+ORIGWIDTH_DELTA, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NEWGAME : NEWGAME_RUS),
                                  NULL,false);
        return;
    }

    // Chex Quest disabled the episode select screen, as did Doom II.

    if (gamemode == commercial || gameversion == exe_chex)
    M_SetupNextMenu(&NewDef);
    else
    M_SetupNextMenu(&EpiDef);
}


//
// M_Episode
//
int epi;

void M_DrawEpisode(void)
{
    V_DrawShadowedPatchDoom(99+ORIGWIDTH_DELTA, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
    V_DrawShadowedPatchDoom(73+ORIGWIDTH_DELTA, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
    return;

    G_DeferedInitNew(nightmare,epi+1,1);
    M_ClearMenus ();
}

void M_VerifyUltraNightmare(int key)
{
    if (key != key_menu_confirm)
    return;

    G_DeferedInitNew(ultra_nm,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NIGHTMARE : NIGHTMARE_RUS),
                                  M_VerifyNightmare,true);
        return;
    }
    if (choice == ultra_nm)
    {
        M_StartMessage(DEH_String(english_language ?
                                  ULTRANM : ULTRANM_RUS),
                                  M_VerifyUltraNightmare,true);
        return;
    }

    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ((gamemode == shareware) && choice)
    {
        M_StartMessage(DEH_String(english_language ?
                                  SWSTRING : SWSTRING_RUS),
                                  NULL,false);
        M_SetupNextMenu(&ReadDef1);
        return;
    }

    // Yet another hack...
    if ( (gamemode == registered) && (choice > 2))
    {
        fprintf (stderr, english_language ?
                        "M_Episode: fourth episode available only in Ultimate DOOM\n" :
                        "M_Episode: четвертый эпизод доступен только в Ultimate DOOM\n");
        choice = 0;
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}




void M_Options(int choice)
{
    if (vanillaparm)
    {
        M_SetupNextMenu(english_language ? 
                        &Vanilla_OptionsDef : 
                        &Vanilla_OptionsDef_Rus);
    }
    else
    {
        M_SetupNextMenu(english_language ? 
                        &RD_Options_Def : 
                        &RD_Options_Def_Rus);
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
        M_StartMessage(DEH_String(english_language ?
                                  NETEND : NETEND_RUS),NULL,false);
        return;
    }

    M_StartMessage(DEH_String(english_language ?
                              ENDGAME : ENDGAME_RUS),
                              M_EndGameResponse,true);
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
    // Doom 1.9 had two menus when playing Doom 1
    // All others had only one
    //
    // [JN] Show second screen also 1.6, 1.666 and 1.8 Sharewares

    if ((gameversion == exe_doom_1_9 && gamemode != commercial)
    || (gameversion == exe_doom_1_666 && gamemode == shareware)
    || (gameversion == exe_doom_1_8 && gamemode == shareware))
    {
        choice = 0;
        M_SetupNextMenu(&ReadDef2);
    }
    else
    {
        // Close the menu
        M_FinishReadThis(0);
    }
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}


//
// M_QuitDOOM
//
int quitsounds[8] =
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

int quitsounds2[8] =
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


void M_QuitResponse(int key)
{
    if (key != key_menu_confirm)
    return;

    // [JN] Опциональное проигрывание звука при выходе из игры
    if ((!netgame && play_exit_sfx && sfxVolume > 0) || vanillaparm)
    {
        if (gamemode == commercial && gamemission != jaguar)
        S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
        else
        S_StartSound(NULL,quitsounds[(gametic>>2)&7]);

        I_WaitVBL(105);
    }

    I_Quit ();
}


static char *M_SelectEndMessage(void)
{
    char **endmsg;

    if (logical_gamemission == doom)
    {
        endmsg = english_language ? doom1_endmsg : doom1_endmsg_rus; // Doom 1
    }
    else
    {
        endmsg = english_language ? doom2_endmsg : doom2_endmsg_rus; // Doom 2
    }

    return endmsg[gametic % NUM_QUITMESSAGES];
}


void M_QuitDOOM(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring),
                (english_language ? ("%s\n\n" DOSY) : ("%s\n\n" DOSY_RUS)),
                 DEH_String(M_SelectEndMessage()));

    M_StartMessage(endstring,M_QuitResponse,true);
}





//
// Menu Functions
//

// -----------------------------------------------------------------------------
// [JN] Draw vanilla thermo
// -----------------------------------------------------------------------------
void M_DrawThermo (int x, int y, int thermWidth, int thermDot)
{
    int     xx;
    int     i;

    xx = x;

    V_DrawPatch(xx, y, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));

    xx += 8;

    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatch(xx, y, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }

    // [crispy] do not crash anymore if value exceeds thermometer range
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
    }

    V_DrawPatch(xx, y, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    V_DrawPatch((x + 8) + thermDot * 8, y,
                W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));
}


// -----------------------------------------------------------------------------
// [JN] Draw small thermo for RD options menu
// -----------------------------------------------------------------------------
void M_DrawThermo_Small (int x, int y, int thermWidth, int thermDot)
{
    int		xx;
    int		i;

    xx = x;
    V_DrawShadowedPatchDoom(xx, y, W_CacheLumpName(DEH_String("RD_THRML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawShadowedPatchDoom(xx, y, W_CacheLumpName(DEH_String("RD_THRMM"), PU_CACHE));
	xx += 8;
    }
    V_DrawShadowedPatchDoom(xx, y, W_CacheLumpName(DEH_String("RD_THRMR"), PU_CACHE));

    // [crispy] do not crash anymore if value exceeds thermometer range
    // [JN] Draw red slider instead of blue.
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        V_DrawPatchDirect((x + 8) + thermDot * 8, y,
                W_CacheLumpName(DEH_String("RD_THRMW"), PU_CACHE));
    }
    else
    {
        V_DrawPatchDirect((x + 8) + thermDot * 8, y,
                W_CacheLumpName(DEH_String("RD_THRMO"), PU_CACHE));
    }
}


void
M_StartMessage
( char*     string,
  void*     routine,
  boolean   input )
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
    size_t  i;
    int     w = 0;
    int     c;

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
// Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t  i;
    int     h;
    int     height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
    if (string[i] == '\n')
        h += height;

    return h;
}


// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK || key == KEY_SCRLCK || key == KEY_NUMLOCK;
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
    static int      joywait = 0;
    static int      mousewait = 0;
    static int      mousey = 0;
    static int      lasty = 0;
    static int      mousex = 0;
    static int      lastx = 0;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit || (ev->type == ev_keydown
        && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // [JN] Не запрашивать подтверждение на выход при нажатии F10 в режиме разработчика (devparm).
    if (devparm && ev->data1 == key_menu_quit)
    {
        I_Quit();
        return true;
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
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
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
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
            
            // [crispy] scroll menus with mouse wheel
            // [JN] it also affecting mouse side buttons (forward/backward)
            if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
            }
            else
            if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
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
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;

            case KEY_ESCAPE:
            saveStringEnter = 0;
            M_StringCopy(savegamestrings[saveSlot], saveOldString, SAVESTRINGSIZE);
            break;

            case KEY_ENTER:
            saveStringEnter = 0;
            if (savegamestrings[saveSlot][0])
            M_DoSave(saveSlot);
            break;

            default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }

            ch = toupper(ch);

            if (ch != ' ' && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

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
        if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    if (messageRoutine)
        messageRoutine(key);

    menuactive = false;
    S_StartSound(NULL,sfx_swtchx);
    return true;
    }

    if ((devparm && key == key_menu_help) || (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot ();
        return true;
    }

    // [JN] Toggling of local time widget
    if (key == key_toggletime)
    {
        if (!local_time)
        {
            local_time = true;
        }
        else
        {
            local_time = false;
        }
        return true;
    }

    // [JN] Toggling of crosshair
    if (key == key_togglecrosshair)
    {
        static char crosshairmsg[24];

        // [JN] Crosshair is not appearing in vanilla mode, toggling disabled.
        if (vanillaparm)
        return false;

        if (!crosshair_draw)
        {
            crosshair_draw = true;
        }
        else
        {
            crosshair_draw = false;
        }
        
        if (english_language)
        {
            M_snprintf(crosshairmsg, sizeof(crosshairmsg), STSRT_CROSSHAIR "%s",
                crosshair_draw ? STSTR_CROSSHAIR_ON : STSTR_CROSSHAIR_OFF);
        }
        else
        {
            M_snprintf(crosshairmsg, sizeof(crosshairmsg), STSRT_CROSSHAIR_RUS "%s",
                crosshair_draw ? STSTR_CROSSHAIR_ON_RUS : STSTR_CROSSHAIR_OFF_RUS);
        }

        players[consoleplayer].message = crosshairmsg;
        S_StartSound(NULL,sfx_swtchn);

        return true;
    }

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
            return false;
            M_RD_Change_ScreenSize(0);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
            return false;
            M_RD_Change_ScreenSize(1);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
        M_StartControlPanel ();

        if ( gamemode == retail )
            currentMenu = &ReadDef2;
        else
            currentMenu = &ReadDef1;

        itemOn = 0;
        S_StartSound(NULL,sfx_swtchn);
        return true;
        }
        else if (key == key_menu_save)     // Save
        {
            QuickSaveTitle = false;
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_SaveGame(0);
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_LoadGame(0);
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            if (vanillaparm)
            {
            currentMenu = english_language ?
                          &Vanilla_Audio_Def : 
                          &Vanilla_Audio_Def_Rus;
            }
            else
            {
            currentMenu = english_language ?
                          &RD_Audio_Def : 
                          &RD_Audio_Def_Rus;
            }
            itemOn = rd_audio_sfxvolume;
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            M_RD_Change_Detail(0);
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            QuickSaveTitle = true;
            S_StartSound(NULL,sfx_swtchn);
            M_QuickSave();
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL,sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            M_RD_Change_Messages(0);
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuickLoad();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
            return true;
        }
        // [JN] gamma toggling moved below for making 
        // possible to change gamma even while active menu.
    }

    if (key == key_menu_gamma)    // gamma toggle
    {
        usegamma++;
        if (usegamma > 17)
            usegamma = 0;

        I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                              "PALFIX" : "PLAYPAL"), PU_CACHE) +
                                              st_palette * 768);

        players[consoleplayer].message = DEH_String(english_language ? 
                                                    gammamsg[usegamma] : 
                                                    gammamsg_rus[usegamma]);
        return true;
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL,sfx_swtchn);
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
            S_StartSound(NULL,sfx_pstop);
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
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
    {
        S_StartSound(NULL,sfx_stnmov);
        currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status)
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
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

        currentMenu->lastOn = itemOn;
        M_ClearMenus ();
        S_StartSound(NULL,sfx_swtchx);
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
            S_StartSound(NULL,sfx_swtchn);
        }
        return true;
    }

    // [crispy] delete a savegame
    else if (key == KEY_DEL)
    {
        if (currentMenu == &LoadDef || currentMenu == &SaveDef)
        {
            if (LoadMenu[itemOn].status)
            {
                currentMenu->lastOn = itemOn;
                M_ConfirmDeleteGame();
                return true;
            }
            else
            {
                return true;
            }
        }
    }

    // [JN] Scroll Gameplay features menu by PgUp/PgDn keys
    else if (key == KEY_PGUP)
    {
        currentMenu->lastOn = itemOn;
        if (currentMenu == &RD_Gameplay_Def_1
        ||  currentMenu == &RD_Gameplay_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_4 :
                           &RD_Gameplay_Def_4_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_2
        ||  currentMenu == &RD_Gameplay_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_1 :
                           &RD_Gameplay_Def_1_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_3
        ||  currentMenu == &RD_Gameplay_Def_3_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_2 :
                           &RD_Gameplay_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_4
        ||  currentMenu == &RD_Gameplay_Def_4_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_3 :
                           &RD_Gameplay_Def_3_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
    }
    else if (key == KEY_PGDN)
    {
        currentMenu->lastOn = itemOn;
        if (currentMenu == &RD_Gameplay_Def_1
        ||  currentMenu == &RD_Gameplay_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_2 :
                           &RD_Gameplay_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_2
        ||  currentMenu == &RD_Gameplay_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_3 :
                           &RD_Gameplay_Def_3_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_3
        ||  currentMenu == &RD_Gameplay_Def_3_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_4 :
                           &RD_Gameplay_Def_4_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_4
        ||  currentMenu == &RD_Gameplay_Def_4_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_1 :
                           &RD_Gameplay_Def_1_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
    }

    // Keyboard shortcut?
    // Vanilla Doom has a weird behavior where it jumps to the scroll bars
    // when the certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_pstop);
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
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}

// Display OPL debug messages - hack for GENMIDI development.

static void M_DrawOPLDev(void)
{
    extern void I_OPL_DevMessages(char *, size_t);
    char        debug[1024];
    char        *curr, *p;
    int         line;

    I_OPL_DevMessages(debug, sizeof(debug));
    curr = debug;
    line = 0;

    for (;;)
    {
        p = strchr(curr, '\n');

        if (p != NULL)
        {
            *p = '\0';
        }

        M_WriteTextSmall_ENG(0, line * 8, curr);
        ++line;

        if (p == NULL)
        {
            break;
        }

        curr = p + 1;
    }
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short    x;
    static short    y;
    unsigned int    i;
    unsigned int    max;
    char            string[80];
    char            *name;
    int             start;

    inhelpscreens = false;

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = ORIGHEIGHT/2 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start, sizeof(string));
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
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x+ORIGWIDTH_DELTA, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
    }

    if (opldev)
    {
        M_DrawOPLDev();
    }

    if (!menuactive)
    return;

    if (currentMenu->routine)
    currentMenu->routine();     // call Draw routine

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

        // if (name[0])
        // {
            // -----------------------------------------------------------------
            // [JN] Write common menus by using standard graphical patches:
            // -----------------------------------------------------------------
            if (currentMenu == &MainDef                // Main Menu
            ||  currentMenu == &MainDefBeta            // Main Menu (Press Beta) 
            ||  currentMenu == &EpiDef                 // Episode selection
            ||  currentMenu == &NewDef                 // Skill level
            ||  currentMenu == &Vanilla_OptionsDef     // Vanilla options menu
            ||  currentMenu == &Vanilla_Audio_Def)     // Vanilla sound menu
            {
                V_DrawShadowedPatchDoom (x, y, W_CacheLumpName(name, PU_CACHE));

                // DRAW SKULL
                V_DrawShadowedPatchDoom(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
                W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));

                // [JN] Big vertical spacing
                y += LINEHEIGHT;
            }
            // -----------------------------------------------------------------
            // [JN] Write English options menu with big English font
            // -----------------------------------------------------------------
            else 
            if (currentMenu == &RD_Options_Def)
            {
                M_WriteTextBig_ENG(x, y, name);

                // DRAW SKULL
                V_DrawShadowedPatchDoom(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
                W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));

                // [JN] Big vertical spacing
                y += LINEHEIGHT;
            }
            // -----------------------------------------------------------------
            // [JN] Write Russian options menu with big Russian font
            // -----------------------------------------------------------------
            else 
            if (currentMenu == &RD_Options_Def_Rus
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
            // -----------------------------------------------------------------
            // [JN] Write English submenus with small English font
            // -----------------------------------------------------------------
            else
            if (currentMenu == &RD_Rendering_Def
            ||  currentMenu == &RD_Display_Def
            ||  currentMenu == &RD_Automap_Def
            ||  currentMenu == &RD_Audio_Def
            ||  currentMenu == &RD_Controls_Def
            ||  currentMenu == &RD_Gameplay_Def_1
            ||  currentMenu == &RD_Gameplay_Def_2
            ||  currentMenu == &RD_Gameplay_Def_3
            ||  currentMenu == &RD_Gameplay_Def_4)
            {
                M_WriteTextSmall_ENG(x, y, name);

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
            // -----------------------------------------------------------------
            // [JN] Write Russian submenus with small Russian font
            // -----------------------------------------------------------------            
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
        // }

        // [JN] Saving / Loading menus, help screens. Just a blinking skull.
        if (currentMenu == &LoadDef
        ||  currentMenu == &SaveDef
        ||  currentMenu == &ReadDef1
        ||  currentMenu == &ReadDef2)
        {
            // DRAW SKULL
            V_DrawShadowedPatchDoom(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
            W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
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
#ifdef WIDESCREEN
    screenSize = screenblocks - 9;
#else
    screenSize = screenblocks - 3;
#endif
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

#ifdef WIDESCREEN
    // [JN] Wide screen: place screen size slider correctly at starup
    if (screenSize < 0)
        screenSize = 0;
    if (screenSize > 5)
        screenSize = 5;
#endif

    // [JN] Safeguard: correct invalid pickup message colors
    if (messages_color < 0 || messages_color > 7)
        messages_color = 0;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    switch ( gamemode )
    {
        case commercial:
        // Commercial has no "read this" entry.
        MainMenu[readthis] = MainMenu[quitdoom];
        MainDef.numitems--;
        MainDef.y += 8;
        NewDef.prevMenu = &MainDef;
        break;

        case shareware:
        // Episode 2 and 3 are handled,
        //  branching to an ad screen.
        case registered:
        break;

        case retail:
        // We are fine.
        break;

        case pressbeta:
        // [JN] Use special menu for Press Beta
        MainDef = MainDefBeta;
        // [JN] Remove one lower menu item
        MainDef.numitems--;
        // [JN] Correct return to previous menu
        NewDef.prevMenu = &MainDef;
        break;

        default:
        break;
    }

    // Versions of doom.exe before the Ultimate Doom release only had
    // three episodes; if we're emulating one of those then don't try
    // to show episode four. If we are, then do show episode four
    // (should crash if missing).
    if (gameversion < exe_ultimate)
    {
        EpiDef.numitems--;
    }

    // [crispy] rearrange Load Game and Save Game menus
    {
	const patch_t *patchl, *patchs, *patchm;
	short captionheight, vstep;

	patchl = W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE);
	patchs = W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE);
	patchm = W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE);

	LoadDef_x = (ORIGWIDTH - SHORT(patchl->width)) / 2 + SHORT(patchl->leftoffset);
	SaveDef_x = (ORIGWIDTH - SHORT(patchs->width)) / 2 + SHORT(patchs->leftoffset);
	LoadDef.x = SaveDef.x = (ORIGWIDTH - 24 * 8) / 2 + SHORT(patchm->leftoffset); // [crispy] see M_DrawSaveLoadBorder()

	captionheight = MAX(SHORT(patchl->height), SHORT(patchs->height));

	vstep = ORIGHEIGHT - 32; // [crispy] ST_HEIGHT
	vstep -= captionheight;
	vstep -= (load_end - 1) * LINEHEIGHT + SHORT(patchm->height);
	vstep /= 3;

	if (vstep > 0)
	{
		LoadDef_y = vstep + captionheight - SHORT(patchl->height) + SHORT(patchl->topoffset);
		SaveDef_y = vstep + captionheight - SHORT(patchs->height) + SHORT(patchs->topoffset);
		LoadDef.y = SaveDef.y = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 7; // [crispy] see M_DrawSaveLoadBorder()
	}
    }

    opldev = M_CheckParm("-opldev") > 0;
}

// [from crispy] Возможность удаления сохраненных игр
static char *savegwarning;
static void M_ConfirmDeleteGameResponse (int key)
{
    free(savegwarning);

    if (key == key_menu_confirm)
    {
        char name[256];

        M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
        remove(name);
        M_ReadSaveStrings();
    }
}

void M_ConfirmDeleteGame ()
{
    savegwarning =
    M_StringJoin(english_language ?
        "are you sure you want to\ndelete saved game\n\n\"" :
        "ds ltqcndbntkmyj [jnbnt\nelfkbnm cj[hfytyye. buhe\n\n^",
        savegamestrings[itemOn], english_language ?
        "\"?\n\n" :
        "^?\n\n",
        english_language ?
        PRESSYN : PRESSYN_RUS,
        NULL);

    M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
    messageToPrint = 2;
    S_StartSound(NULL,sfx_swtchn);
}


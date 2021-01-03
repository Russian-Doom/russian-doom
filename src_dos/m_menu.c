//
// Copyright (C) 1993-1996 Id Software, Inc.
// Copyright (C) 2016-2017 Alexey Khokholov (Nuke.YKT)
// Copyright (C) 2017 Alexandre-Xavier Labonte-Lamoureux
// Copyright (C) 2017-2021 Julian Nechaevsky
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
#include "sounds.h"
#include "m_menu.h"
#include "v_trans.h"
#include "r_main.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "rd_lang.h"
#include "jn.h"


#define NUM_QUITMESSAGES 8  // QuitDOOM messages

#define SKULLXOFF		-32
#define LINEHEIGHT		16
#define LINEHEIGHT_SML  10  // [JN] Line height for small font
#define SAVESTRINGSIZE  24

void    (*messageRoutine)(int response);

int     mouseSensitivity;       // has default
int     showMessages;           // Show messages has default, 0 = off, 1 = on
int     sfxVolume;
int     musicVolume;
int     detailLevel;            // Blocky mode, has default, 0 = high, 1 = normal
int     screenblocks;           // has default
int     screenSize;             // temp for screenblocks (0-9)
int     quickSaveSlot;          // -1 = no quicksave slot picked!
int     messageToPrint;         // 1 = message to be printed
int     messx;                  // message x
int     messy;                  // message y
int     messageLastMenuActive;
int     saveStringEnter;        // we are going to be entering a savegame string
int     saveSlot;               // which slot to save in
int     saveCharIndex;          // which char we're editing
int     epi;

char    saveOldString[SAVESTRINGSIZE];  // old save description before edit
char    savegamestrings[10][SAVESTRINGSIZE];
char    tempstring[80];
char    endstring[160];
char    skullName[2][9] = {"M_SKULL1","M_SKULL2"};  // graphic name of skulls
char    detailNames[2][9] = {"M_GDHIGH","M_GDLOW"};
char    msgNames[2][9] = {"M_MSGOFF","M_MSGON"};

char   *messageString;          // ...and here is the message string!

short   itemOn;                 // menu item skull is on
short   skullAnimCounter;       // skull animation counter
short   whichSkull;             // which skull to draw

boolean inhelpscreens;
boolean menuactive;
boolean messageNeedsInput;      // timed message = no input from user
boolean QuickSaveTitle;         // [JN] Additional title "БЫСТРОЕ СОХРАНЕНИЕ"


extern int      st_palette;

extern boolean  chat_on;		// in heads-up code
extern boolean  sendpause;
extern boolean  message_dontfuckwithme;

extern patch_t *hu_font[HU_FONTSIZE];
extern patch_t *hu_font_small_eng[HU_FONTSIZE];
extern patch_t *hu_font_small_rus[HU_FONTSIZE];
extern patch_t *hu_font_big_eng[HU_FONTSIZE2];
extern patch_t *hu_font_big_rus[HU_FONTSIZE2];



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
    short           numitems;	// # of menu items
    struct menu_s  *prevMenu;	// previous menu
    menuitem_t     *menuitems;	// menu items
    void          (*routine)();	// draw routine
    short           x;          // x of menu
    short           y;          // y of menu
    short           lastOn;		// last item user was on in menu
} menu_t;

menu_t*	currentMenu; // current menudef


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

char endmsg1[NUM_QUITMESSAGES][80] =
{
    // DOOM1
    "are you sure you want to\nquit this great game?",
    "please don't leave, there's more\ndemons to toast!",
    "let's beat it -- this is turning\ninto a bloodbath!",
    "i wouldn't leave if i were you.\ndos is much worse.",
    "you're trying to say you like dos\nbetter than me, right?",
    "don't leave yet -- there's a\ndemon around that corner!",
    "ya know, next time you come in here\ni'm gonna toast ya.",
    "go ahead and leave. see if i care."
};

char endmsg2[NUM_QUITMESSAGES][80] =
{
    // QuitDOOM II messages
    QUITMSG,
    "you want to quit?\nthen, thou hast lost an eighth!",
    "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
    "get outta here and go back\nto your boring programs.",
    "if i were your boss, i'd \n deathmatch ya in a minute!",
    "look, bud. you leave now\nand you forfeit your body count!",
    "just leave. when you come\nback, i'll be waiting with a bat.",
    "you're lucky i don't smack\nyou for thinking about leaving."
};

char endmsg1_rus[NUM_QUITMESSAGES][80] =
{
    // DOOM1
    // Вы действительно хотите выйти \n из этой замечательной игры?
    "ds ltqcndbntkmyj [jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?",
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

char endmsg2_rus[NUM_QUITMESSAGES][80] =
{
    // QuitDOOM II messages
    // Вы действительно хотите выйти \n из этой замечательной игры?
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
    // Вам очень повезло, что за это \n с вами ничего не случится!
    "dfv jxtym gjdtpkj< xnj pf 'nj \n c dfvb ybxtuj yt ckexbncz!"
};


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
void M_DrawThermo_Small (int x, int y, int thermWidth, int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
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
void M_RD_Change_NoFlats(int choice);
void M_RD_Change_FPScounter(int choice);
void M_RD_Change_DiskIcon(int choice);
void M_RD_Change_Wiping(int choice);

// Display
void M_RD_Choose_Display(int choice);
void M_RD_Draw_Display(void);
void M_RD_Change_ScreenSize(int choice);
void M_RD_Change_Gamma(int choice);
void M_RD_Change_LevelBrightness(int choice);
void M_RD_Change_Detail(int choice);
void M_RD_Change_LocalTime(int choice);
void M_RD_Change_Messages(int choice);

// Messages
void M_RD_Choose_MessagesSettings(int choice);
void M_RD_Draw_MessagesSettings(void);
void M_RD_Change_Messages(int choice);
void M_RD_Change_ShadowedText(int choice);
void M_RD_Change_Msg_Pickup_Color(int choice);
void M_RD_Change_Msg_Secret_Color(int choice);
void M_RD_Change_Msg_System_Color(int choice);
void M_RD_Change_Msg_Chat_Color(int choice);

// Automap
void M_RD_Choose_AutomapSettings(int choice);
void M_RD_Draw_AutomapSettings(void);
void M_RD_Change_AutomapColor(int choice);
void M_RD_Change_AutomapAntialias(int choice);
void M_RD_Change_AutomapStats(int choice);
// void M_RD_Change_AutomapOverlay(int choice);
void M_RD_Change_AutomapRotate(int choice);
void M_RD_Change_AutomapFollow(int choice);
void M_RD_Change_AutomapGrid(int choice);

// Sound
void M_RD_Choose_Audio(int choice);
void M_RD_Draw_Audio(void);
void M_RD_Change_SfxVol(int choice);
void M_RD_Change_MusicVol(int choice);
void M_RD_Change_SfxChannels(int choice);

// Sound system
void M_RD_Choose_SoundSystem(int choice);
void M_RD_Draw_Audio_System(void);
void M_RD_Change_SoundDevice(int choice);
void M_RD_Change_MusicDevice(int choice);
void M_RD_Change_SBport(int choice);
void M_RD_Change_SBirq(int choice);
void M_RD_Change_SBdma(int choice);
void M_RD_Change_Mport(int choice);
void M_RD_Change_Sampling(int choice);
void M_RD_Change_SndMode(int choice);
void M_RD_Change_PitchShifting(int choice);

// Controls
void M_RD_Choose_Controls(int choice);
void M_RD_Draw_Controls(void);
void M_RD_Change_AlwaysRun();
void M_RD_Change_MouseLook(int choice);
void M_RD_Change_Sensitivity(int choice);
void M_RD_Change_InvertY(int choice);
void M_RD_Change_Novert(int choice);

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
void M_RD_Change_Translucency(int choice);
void M_RD_Change_ImprovedFuzz(int choice);
void M_RD_Change_ColoredHUD(int choice);
void M_RD_Change_ColoredBlood(int choice);
void M_RD_Change_SwirlingLiquids(int choice);
void M_RD_Change_InvulSky(int choice);
void M_RD_Change_FlipWeapons(int choice);

void M_RD_Change_ExitSfx(int choice);
void M_RD_Change_CrushingSfx(int choice);
void M_RD_Change_BlazingSfx(int choice);
void M_RD_Change_AlertSfx(int choice);
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

// Language hot-swapping
void M_RD_ChangeLanguage(int choice);

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
        if (cx+w > SCREENWIDTH)
            break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_small_eng[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_small_eng[c]);

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
        if (cx+w > SCREENWIDTH)
            break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_small_rus[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_small_rus[c]);

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
        if (cx+w > SCREENWIDTH)
        break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big_eng[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big_eng[c]);

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
        if (cx+w > SCREENWIDTH)
        break;

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big_rus[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big_rus[c]);

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
    int    c, cx, cy, w, width;
    char  *ch;

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

        w = SHORT (hu_font_big_eng[c]->width);

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big_eng[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big_eng[c]);

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
    int    c, cx, cy, w, width;
    char  *ch;

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

        w = SHORT (hu_font_big_rus[c]->width);

        V_DrawShadowDirect(cx+1, cy+1, 0, hu_font_big_rus[c]);
        V_DrawPatchDirect(cx, cy, 0, hu_font_big_rus[c]);

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

// ------------
// English menu
// ------------

menuitem_t MainMenu[]=
{
    {1, "M_NGAME",  M_NewGame,  'n'},
    {1, "M_OPTION", M_Options,  'o'},
    {1, "M_LOADG",  M_LoadGame, 'l'},
    {1, "M_SAVEG",  M_SaveGame, 's'},
    // Another hickup with Special edition.
    {1, "M_RDTHIS", M_ReadThis, 'r'},
    {1, "M_QUITG",  M_QuitDOOM, 'q'}
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

// ------------
// Russian menu
// ------------

menuitem_t MainMenu_Rus[]=
{
    {1, "RD_NGAME", M_NewGame,  'y'}, // Новая игра
    {1, "RD_OPTN",  M_Options,  'y'}, // Настройки
    {1, "RD_LOADG", M_LoadGame, 'p'}, // Загрузка
    {1, "RD_SAVEG", M_SaveGame, 'c'}, // Сохранение
    {1, "RD_INFO",  M_ReadThis, 'b'}, // Информация!
    {1, "RD_QUITG", M_QuitDOOM, 'd'}  // Выход
};

menu_t  MainDef_Rus =
{
    main_end,
    NULL,
    MainMenu_Rus,
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
    ep5, // [crispy] Sigil
    ep_end
} episodes_e;

// ------------
// English menu
// ------------

menuitem_t EpisodeMenu[]=
{
    {1, "M_EPI1", M_Episode, 'k'},
    {1, "M_EPI2", M_Episode, 't'},
    {1, "M_EPI3", M_Episode, 'i'},
    {1, "M_EPI4", M_Episode, 't'},
    {1, "M_EPI5", M_Episode, 's'} // [crispy] Sigil
};

menu_t  EpiDef =
{
    ep_end,
    &MainDef,
    EpisodeMenu,
    M_DrawEpisode,
    48,63,
    ep1
};

// ------------
// Russian menu
// ------------

menuitem_t EpisodeMenu_Rus[]=
{
    {1, "RD_EPI1", M_Episode, 'g'}, // По колено в трупах
    {1, "RD_EPI2", M_Episode, 'g'}, // Прибрежье Ада
    {1, "RD_EPI3", M_Episode, 'b'}, // Инферно
    {1, "RD_EPI4", M_Episode, 'n'}, // Твоя плоть истощена
    {1, "RD_EPI5", M_Episode, 'c'}  // Сигил
};

menu_t  EpiDef_Rus =
{
    ep_end,
    &MainDef_Rus,
    EpisodeMenu_Rus,
    M_DrawEpisode,
    48,63,
    ep1
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

// ------------
// English menu
// ------------

menuitem_t NewGameMenu[]=
{
    {1, "M_JKILL",  M_ChooseSkill, 'i'},
    {1, "M_ROUGH",  M_ChooseSkill, 'h'},
    {1, "M_HURT",   M_ChooseSkill, 'h'},
    {1, "M_ULTRA",  M_ChooseSkill, 'u'},
    {1, "M_NMARE",  M_ChooseSkill, 'n'},
    {1, "M_UNMARE", M_ChooseSkill, 'u'}
};

menu_t  NewDef =
{
    newg_end,
    &EpiDef,
    NewGameMenu,
    M_DrawNewGame,
    48,63,
    hurtme
};

// ------------
// Russian menu
// ------------

menuitem_t NewGameMenu_Rus[]=
{
    {1, "RD_JKILL", M_ChooseSkill, 'v'}, // Мне рано умирать.
    {1, "RD_ROUGH", M_ChooseSkill, 'y'}, // Эй, не так грубо.
    {1, "RD_HURT",  M_ChooseSkill, 'c'}, // Сделай мне больно.
    {1, "RD_ULTRA", M_ChooseSkill, 'e'}, // Ультранасилие
    {1, "RD_NMARE", M_ChooseSkill, 'r'}, // Кошмар.
    {1, "RD_UNMAR", M_ChooseSkill, 'e'}  // Ультра кошмар!
};

menu_t  NewDef_Rus =
{
    newg_end,
    &EpiDef_Rus,
    NewGameMenu_Rus,
    M_DrawNewGame,
    48,63,
    hurtme
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
    rd_language,
    rd_end
} options_e;

// ------------
// English menu
// ------------

menuitem_t RD_Options_Menu[]=
{
    {1, "Rendering",         M_RD_Choose_Rendering,  'r'},
    {1, "Display",           M_RD_Choose_Display,    'd'},
    {1, "Sound",             M_RD_Choose_Audio,      's'},
    {1, "Controls",          M_RD_Choose_Controls,   'c'},
    {1, "Gameplay",          M_RD_Choose_Gameplay_1, 'g'},
    {1, "End Game",          M_EndGame,              'e'},
    {1, "Reset settings",    M_RD_BackToDefaults,    'r'},
    {2,  "Language:english", M_RD_ChangeLanguage,    'l'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Options_Menu_Rus[]=
{
    {1, "Dbltj",          M_RD_Choose_Rendering,  'd'}, // Видео
    {1, "\"rhfy",         M_RD_Choose_Display,   '\''}, // Экран
    {1, "Felbj",          M_RD_Choose_Audio,      'f'}, // Аудио
    {1, "Eghfdktybt",     M_RD_Choose_Controls,   'e'}, // Управление
    {1, "Utqvgktq",       M_RD_Choose_Gameplay_1, 'u'}, // Геймплей
    {1, "Pfrjyxbnm buhe", M_EndGame,              'p'}, // Закончить игру
    {1, "C,hjc yfcnhjtr", M_RD_BackToDefaults,    'c'}, // Сброс настроек
    {2, "Zpsr#heccrbq",   M_RD_ChangeLanguage,    'z'}, // Язык: русский
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def_Rus =
{
    rd_end,
    &MainDef_Rus,
    RD_Options_Menu_Rus,
    M_RD_Draw_Options,
    60,37,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

enum
{
    rd_rendering_noflats,
    rd_rendering_fps,
    rd_rendering_empty1,
    rd_rendering_diskicon,
    rd_rendering_wiping,
    
    rd_rendering_end
} rd_rendering_e;

// ------------
// English menu
// ------------

menuitem_t RD_Rendering_Menu[]=
{
    {2, "Floor and ceiling textures:", M_RD_Change_NoFlats,    'f'},
    {2, "Show FPS counter:",           M_RD_Change_FPScounter, 's'},
    {-1,"",0,'\0'},
    {2, "Show disk icon:",             M_RD_Change_DiskIcon,   's'},
    {2, "Screen wiping effect:",       M_RD_Change_Wiping,     's'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Rendering_Menu_Rus[]=
{
    {2, "Ntrcnehs gjkf b gjnjkrf:",   M_RD_Change_NoFlats,    'n'}, // Текстуры пола и потолка
    {2, "Cxtnxbr rflhjdjq xfcnjns:",  M_RD_Change_FPScounter, 'c'}, // Счетчик кадровой частоты
    {-1,"",0,'\0'},
    {2, "Jnj,hf;fnm pyfxjr lbcrtns:", M_RD_Change_DiskIcon,   'j'}, // Отображать значок дискеты
    {2, "\'aatrn cvtys \'rhfyjd:",    M_RD_Change_Wiping,    '\''}, // Эффект смены экранов
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def_Rus =
{
    rd_rendering_end,
    &RD_Options_Def_Rus,
    RD_Rendering_Menu_Rus,
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
    rd_display_level_brightness,
    rd_display_empty3,
    rd_display_detail,
    rd_display_localtime,
    rd_display_empty4,
    rd_display_messages_settings,
    rd_display_automap_settings,
    rd_display_end
} rd_display_e;

// ------------
// English menu
// ------------

menuitem_t RD_Display_Menu[]=
{
    {2, "screen size",       M_RD_Change_ScreenSize,       's'},
    {-1,"",0,'\0'},
    {2, "gamma-correction",  M_RD_Change_Gamma,            'g'},
    {-1,"",0,'\0'},
    {2, "level brightness",  M_RD_Change_LevelBrightness,  'l'},
    {-1,"",0,'\0'},
    {2, "detail level:",     M_RD_Change_Detail,           'd'},
    {2, "local time:",       M_RD_Change_LocalTime,        'l'},
    {-1,"",0,'\0'},
    {1, "messages settings", M_RD_Choose_MessagesSettings, 'm'},
    {1, "automap settings",  M_RD_Choose_AutomapSettings,  'a'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Display_Menu_Rus[]=
{
    {2, "hfpvth buhjdjuj \'rhfyf", M_RD_Change_ScreenSize,       'h'}, // Размер игрового экрана
    {-1,"",0,'\0'},
    {2, "ehjdtym ufvvf-rjhhtrwbb", M_RD_Change_Gamma,            'e'}, // Уровень гамма-коррекции
    {-1,"",0,'\0'},
    {2, "ehjdtym jcdtotyyjcnb",    M_RD_Change_LevelBrightness,  'e'}, // Уровень освещенности
    {-1,"",0,'\0'},
    {2, "ehjdtym ltnfkbpfwbb:",    M_RD_Change_Detail,           'e'}, // Уровень детализации:
    {2, "cbcntvyjt dhtvz:",        M_RD_Change_LocalTime,        'c'}, // Системное время:
    {-1,"",0,'\0'},
    {1, "yfcnhjqrb cjj,otybq",     M_RD_Choose_MessagesSettings, 'y'}, // Настройки сообщений
    {1, "yfcnhjqrb rfhns",         M_RD_Choose_AutomapSettings,  'y'}, // Настройки карты
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def_Rus =
{
    rd_display_end,
    &RD_Options_Def_Rus,
    RD_Display_Menu_Rus,
    M_RD_Draw_Display,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

enum
{
    rd_messages_toggle,
    rd_messages_shadows,
    rd_messages_empty1,
    rd_messages_pickup_color,
    rd_messages_secret_color,
    rd_messages_system_color,
    rd_messages_chat_color,
    rd_messages_end
} rd_messages_e;

// ------------
// English menu
// ------------

menuitem_t RD_Messages_Menu[]=
{
    {2, "messages enabled:",   M_RD_Change_Messages,         'm'},
    {2, "text casts shadows:", M_RD_Change_ShadowedText,     't'},
    {-1,"",0,'\0'},
    {2, "item pickup:",        M_RD_Change_Msg_Pickup_Color, 'i'},
    {2, "revealed secret:",    M_RD_Change_Msg_Secret_Color, 'r'},
    {2, "system message:",     M_RD_Change_Msg_System_Color, 's'},
    {2, "netgame chat:",       M_RD_Change_Msg_Chat_Color,   'n'},
    {-1,"",0,'\0'}
};

menu_t  RD_Messages_Def =
{
    rd_messages_end,
    &RD_Display_Def,
    RD_Messages_Menu,
    M_RD_Draw_MessagesSettings,
    35,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Messages_Menu_Rus[]=
{
    {2, "jnj,hf;tybt cjj,otybq:",   M_RD_Change_Messages,         'j'}, // Отображение сообщений:
    {2, "ntrcns jn,hfcsdf.n ntym:", M_RD_Change_ShadowedText,     'n'}, // Тексты отбрасывают тень:
    {-1,"",0,'\0'},
    {2, "gjkextybt ghtlvtnjd:",     M_RD_Change_Msg_Pickup_Color, 'g'}, // Получение предметов:
    {2, "j,yfhe;tybt nfqybrjd:",    M_RD_Change_Msg_Secret_Color, 'j'}, // Обнаружение тайников:
    {2, "cbcntvyst cjj,otybz:",     M_RD_Change_Msg_System_Color, 'c'}, // Системные сообщения:
    {2, "xfn ctntdjq buhs:",        M_RD_Change_Msg_Chat_Color,   'x'}, // Чат сетевой игры:
    {-1,"",0,'\0'}
};

menu_t  RD_Messages_Def_Rus =
{
    rd_messages_end,
    &RD_Display_Def_Rus,
    RD_Messages_Menu_Rus,
    M_RD_Draw_MessagesSettings,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

enum
{
    rd_automap_colors,
    rd_automap_antialias,
    rd_automap_stats,
//  rd_automap_overlay,
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
    {2, "color scheme:",      M_RD_Change_AutomapColor,     'c'},
    {2, "line antialiasing:", M_RD_Change_AutomapAntialias, 'l'},
    {2, "level stats:",       M_RD_Change_AutomapStats,     'l'},
//  {2, "overlay mode:",      M_RD_Change_AutomapOverlay,   'o'},
    {2, "rotate mode:",       M_RD_Change_AutomapRotate,    'r'},
    {2, "follow mode:",       M_RD_Change_AutomapFollow,    'f'},
    {2, "grid:",              M_RD_Change_AutomapGrid,      'g'},
    {-1,"",0,'\0'}
};

menu_t  RD_Automap_Def =
{
    rd_automap_end,
    &RD_Display_Def,
    RD_Automap_Menu,
    M_RD_Draw_AutomapSettings,
    70,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Automap_Menu_Rus[]=
{
    {2, "wdtnjdfz c[tvf:",    M_RD_Change_AutomapColor,     'w'}, // Цветовая схема:
    {2, "cukf;bdfybt kbybq:", M_RD_Change_AutomapAntialias, 'c'}, // Сглаживание линий:
    {2, "cnfnbcnbrf ehjdyz:", M_RD_Change_AutomapStats,     'c'}, // Статистика уровня:
//  {2, "ht;bv yfkj;tybz:",   M_RD_Change_AutomapOverlay,   'h'}, // Режим наложения:
    {2, "ht;bv dhfotybz:",    M_RD_Change_AutomapRotate,    'h'}, // Режим вращения:
    {2, "ht;bv cktljdfybz:",  M_RD_Change_AutomapFollow,    'h'}, // Режим следования:
    {2, "ctnrf:",             M_RD_Change_AutomapGrid,      'c'}, // Сетка:
    {-1,"",0,'\0'}
};

menu_t  RD_Automap_Def_Rus =
{
    rd_automap_end,
    &RD_Display_Def_Rus,
    RD_Automap_Menu_Rus,
    M_RD_Draw_AutomapSettings,
    70,35,
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
    rd_audio_empty5,
    rd_audio_soundsystem,
    rd_audio_end
} rd_audio_e;

// ------------
// English menu
// ------------

menuitem_t RD_Audio_Menu[]=
{
    {2, "sfx volume",            M_RD_Change_SfxVol,      's'},
    {-1,"",0,'\0'},
    {2, "music volume",          M_RD_Change_MusicVol,    'm'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {2, "sound channels",        M_RD_Change_SfxChannels, 's'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1 ,"sound system settings", M_RD_Choose_SoundSystem, 's'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Audio_Menu_Rus[]=
{
    {2, "pder",                       M_RD_Change_SfxVol,      'p'}, // Звук
    {-1,"",0,'\0'},
    {2, "vepsrf",                     M_RD_Change_MusicVol,    'v'}, // Музыка
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {2, "Pderjdst rfyfks",            M_RD_Change_SfxChannels, 'p'}, // Звуковые каналы
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "yfcnhjqrb pderjdjq cbcntvs", M_RD_Choose_SoundSystem, 'y'}, // Настройки звуковой системы
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def_Rus =
{
    rd_audio_end,
    &RD_Options_Def_Rus,
    RD_Audio_Menu_Rus,
    M_RD_Draw_Audio,
    35,45,
    0
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

enum
{
    rd_audio_sys_sfx,
    rd_audio_sys_music,
    rd_audio_sys_sbport,
    rd_audio_sys_sbirq,
    rd_audio_sys_sbdma,
    rd_audio_sys_mport,    
    rd_audio_sys_empty1,
    rd_audio_sys_sampling,
    rd_audio_sys_empty2,
    rd_audio_sys_sndmode,
    rd_audio_sys_sndpitch,
    rd_audio_sys_end
} rd_audio_sys_e;

// ------------
// English menu
// ------------

menuitem_t RD_Audio_System_Menu[]=
{
    {2, "sound effects:",        M_RD_Change_SoundDevice,   's'},
    {2, "music:",                M_RD_Change_MusicDevice,   'm'},
    {2, "port:",                 M_RD_Change_SBport,        'p'},
    {2, "irq:",                  M_RD_Change_SBirq,         'i'},
    {2, "dma:",                  M_RD_Change_SBdma,         'd'},
    {2, "midi port:",            M_RD_Change_Mport,         'm'},
    {-1,"",0,'\0'},
    {2, "sampling frequency:",   M_RD_Change_Sampling,      's'},
    {-1,"",0,'\0'},
    {2, "sound effects mode:",   M_RD_Change_SndMode,       's'},
    {2, "pitch-shifted sounds:", M_RD_Change_PitchShifting, 'p'},
    {-1,"",0,'\0'}
};

menu_t RD_Audio_System_Def =
{
    rd_audio_sys_end,
    &RD_Audio_Def,
    RD_Audio_System_Menu,
    M_RD_Draw_Audio_System,
    35,45,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Audio_System_Menu_Rus[]=
{
    {2, "pderjdst \'aatrns:",         M_RD_Change_SoundDevice,   'p'}, // Звуковые эффекты
    {2, "vepsrf:",                    M_RD_Change_MusicDevice,   'v'}, // Музыка
    {2, "gjhn:",                      M_RD_Change_SBport,        'g'}, // Порт
    {2,  "",                          M_RD_Change_SBirq,         'i'}, // IRQ
    {2,  "",                          M_RD_Change_SBdma,         'd'}, // DMA
    {2,  "gjhn"  ,                    M_RD_Change_Mport,         'g'}, // Порт MIDI
    {-1,"",0,'\0'},
    {2, "xfcnjnf lbcrhtnbpfwbb:",     M_RD_Change_Sampling,      'x'}, // Частота дискретизации
    {-1,"",0,'\0'},
    {2, "Ht;bv pderjds[ \'aatrnjd:",  M_RD_Change_SndMode,       'h'}, // Режим звуковых эффектов
    {2, "ghjbpdjkmysq gbnx-ibanbyu:", M_RD_Change_PitchShifting, 'g'}, // Произвольный питч-шифтинг
    {-1,"",0,'\0'}
};

menu_t RD_Audio_System_Def_Rus =
{
    rd_audio_sys_end,
    &RD_Audio_Def_Rus,
    RD_Audio_System_Menu_Rus,
    M_RD_Draw_Audio_System,
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
    rd_controls_inverty,
    rd_controls_novert,
    rd_controls_end
} rd_controls_e;

// ------------
// English menu
// ------------

menuitem_t RD_Controls_Menu[]=
{
    {2, "always run:",        M_RD_Change_AlwaysRun,   'a'},
    {-1,"",0,'\0'},
    {2, "mouse sensivity",    M_RD_Change_Sensitivity, 'm'},
    {-1,"",0,'\0'},
    {2, "mouse look:",        M_RD_Change_MouseLook,   'm'},
    {2, "invert y axis:",     M_RD_Change_InvertY,     'i'},
    {2, "vertical movement:", M_RD_Change_Novert,      'v'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Controls_Menu_Rus[]=
{
    {2, "ht;bv gjcnjzyyjuj ,tuf:",    M_RD_Change_AlwaysRun,   'h'}, // Режим постоянного бега
    {-1,"",0,'\0'},
    {2, "Crjhjcnm vsib",              M_RD_Change_Sensitivity, 'c'}, // Скорость мыши
    {-1,"",0,'\0'},
    {2, "J,pjh vsim.:",               M_RD_Change_MouseLook,   'j'}, // Обзор мышью
    {2, "dthnbrfkmyfz bydthcbz:",    M_RD_Change_InvertY,      'd'}, // Вертикальная инверсия
    {2, "dthnbrfkmyjt gthtvtotybt:", M_RD_Change_Novert,       'd'}, // Вертикальное перемещение
    {-1,"",0,'\0'}
};

menu_t  RD_Controls_Def_Rus =
{
    rd_controls_end,
    &RD_Options_Def_Rus,
    RD_Controls_Menu_Rus,
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
    rd_gameplay_1_translucency,
    rd_gameplay_1_improved_fuzz,
    rd_gameplay_1_colored_hud,
    rd_gameplay_1_colored_blood,
    rd_gameplay_1_swirling_liquids,
    rd_gameplay_1_invul_sky,
    rd_gameplay_1_flip_weapons,
    rd_gameplay_1_empty1,
    rd_gameplay_1_empty2,
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
    rd_gameplay_2_empty2,
    rd_gameplay_2_empty3,
    rd_gameplay_2_empty4,
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
    rd_gameplay_3_empty2,
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
    rd_gameplay_4_empty5,
    rd_gameplay_4_empty6,
    rd_gameplay_4_first_page,
    rd_gameplay_4_prev_page,
    rd_gameplay_4_end
} rd_gameplay_4_e;

// ------------
// English menu
// ------------

menuitem_t RD_Gameplay_Menu_1[]=
{
    {2, "Brightmaps:",                  M_RD_Change_Brightmaps,      'b'},
    {2, "Fake contrast:",               M_RD_Change_FakeContrast,    'f'},
    {2, "Translucency:",                M_RD_Change_Translucency,    't'},
    {2, "Fuzz effect:",                 M_RD_Change_ImprovedFuzz,    'f'},
    {2, "Colored HUD elements:",        M_RD_Change_ColoredHUD,      'c'},
    {2, "Colored blood and corpses:",   M_RD_Change_ColoredBlood,    'c'},
    {2, "Swirling liquids:",            M_RD_Change_SwirlingLiquids, 's'},
    {2, "Invulnerability affects sky:", M_RD_Change_InvulSky,        'i'},
    {2, "Flip weapons:",                M_RD_Change_FlipWeapons,     'f'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "", /* Next Page > */           M_RD_Choose_Gameplay_2,      'n'},
    {1, "", /* < Last Page */           M_RD_Choose_Gameplay_4,      'l'},
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
    {2, "Play exit sounds:",               M_RD_Change_ExitSfx,        'p'},
    {2, "Sound of crushing corpses:",      M_RD_Change_CrushingSfx,    's'},
    {2, "Single sound of blazing door:",   M_RD_Change_BlazingSfx,     's'},
    {2, "Monster alert waking up others:", M_RD_Change_AlertSfx,       'm'},
    {-1,"",0,'\0'},
    {2, "Notify of revealed secrets:",     M_RD_Change_SecretNotify,   'n'},
    {2, "Show negative health:",           M_RD_Change_NegativeHealth, 's'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "", /* Next page >   */            M_RD_Choose_Gameplay_3,     'n'},
    {1, "", /* < Prev page > */            M_RD_Choose_Gameplay_1,     'p'},
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
    {2, "Walk over and under monsters:",       M_RD_Change_WalkOverUnder,   'w'},
    {2, "Corpses sliding from the ledges:",    M_RD_Change_Torque,          'c'},
    {2, "Weapon bobbing while firing:",        M_RD_Change_Bobbing,         'w'},
    {2, "Lethal pellet of a point-blank SSG:", M_RD_Change_SSGBlast,        'l'},
    {2, "Randomly mirrored corpses:",          M_RD_Change_FlipCorpses,     'r'},
    {2, "Floating powerups:",                  M_RD_Change_FloatPowerups,   'f'},
    {-1,"",0,'\0'},
    {2, "Draw crosshair:",                     M_RD_Change_CrosshairDraw,   'd'},
    {2, "Health indication:",                  M_RD_Change_CrosshairHealth, 'h'},
    {-1,"",0,'\0'},
    {1, "", /* Next page >   */                M_RD_Choose_Gameplay_4,      'n'},
    {1, "", /* < Prev page > */                M_RD_Choose_Gameplay_2,      'p'},
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
    {2, "Extra player faces on the HUD:",      M_RD_Change_ExtraPlayerFaces, 'e'},
    {2, "Pain Elemental without Souls limit:", M_RD_Change_LostSoulsQty,     'p'},
    {2, "Don't prompt for q. saving/loading:", M_RD_Change_FastQSaveLoad,    'd'},
    {2, "Play internal demos:",                M_RD_Change_NoInternalDemos,  'p'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "", /* First page >  */                M_RD_Choose_Gameplay_1,       'f'},
    {1, "", /* < Prev page > */                M_RD_Choose_Gameplay_3,       'p'},
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

// ------------
// Russian menu
// ------------

menuitem_t RD_Gameplay_Menu_1_Rus[]=
{
    {2, ",hfqnvfggbyu:",                  M_RD_Change_Brightmaps,      ','}, // Брайтмаппинг
    {2, "Bvbnfwbz rjynhfcnyjcnb:",        M_RD_Change_FakeContrast,    'b'}, // Имитация контрастности
    {2, "Ghjphfxyjcnm j,]trnjd:",         M_RD_Change_Translucency,    't'}, // Прозрачность объектов
    {2, "\'aatrn ievf:",                  M_RD_Change_ImprovedFuzz,   '\''}, // Эффект шума
    {2, "Hfpyjwdtnyst 'ktvtyns $:",       M_RD_Change_ColoredHUD,      'h'}, // Разноцветные элементы HUD
    {2, "Hfpyjwdtnyfz rhjdm b nhegs:",    M_RD_Change_ColoredBlood,    'h'}, // Разноцветная кровь и трупы
    {2, "ekexityyfz fybvfwbz ;blrjcntq:", M_RD_Change_SwirlingLiquids, 'e'}, // Улучшенная анимация жидкостей
    {2, "ytezpdbvjcnm jrhfibdftn yt,j:",  M_RD_Change_InvulSky,        'y'}, // Неуязвимость окрашивает небо
    {2, "pthrfkmyjt jnhf;tybt jhe;bz:",   M_RD_Change_FlipWeapons,     'p'}, // Зеркальное отражение оружия
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "",                               M_RD_Choose_Gameplay_2,      'l'}, // Далее >
    {1, "",                               M_RD_Choose_Gameplay_4,      'y'}, // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_1_Rus =
{
    rd_gameplay_1_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_1_Rus,
    M_RD_Draw_Gameplay_1,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_2_Rus[]=
{
    {2, "Pderb ghb ds[jlt bp buhs:",     M_RD_Change_ExitSfx,        'p'}, // Звук при выходе из игры
    {2, "Pder hfplfdkbdfybz nhegjd:",    M_RD_Change_CrushingSfx,    'p'}, // Звук раздавливания трупов
    {2, "Jlbyjxysq pder ,scnhjq ldthb:", M_RD_Change_BlazingSfx,     'j'}, // Одиночный звук быстрой двери
    {2, "J,ofz nhtdjuf e vjycnhjd:",     M_RD_Change_AlertSfx,       'j'}, // Общая тревога у монстров
    {-1,"",0,'\0'},
    {2, "Cjj,ofnm j yfqltyyjv nfqybrt:", M_RD_Change_SecretNotify,   'c'}, // Сообщать о найденном тайнике
    {2, "jnhbwfntkmyjt pljhjdmt d $:",   M_RD_Change_NegativeHealth, 'j'}, // Отрицательное здоровье в HUD
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "",                              M_RD_Choose_Gameplay_3,     'l'}, // Далее >
    {1, "",                              M_RD_Choose_Gameplay_1,     'y'}, // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_2_Rus =
{
    rd_gameplay_2_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_2_Rus,
    M_RD_Draw_Gameplay_2,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_3_Rus[]=
{
    {2, "Gthtvtotybt gjl/yfl vjycnhfvb:", M_RD_Change_WalkOverUnder,   'g'}, // Перемещение над/под монстрами
    {2, "Nhegs cgjkpf.n c djpdsitybq:",   M_RD_Change_Torque,          'n'}, // Трупы сползают с возвышений
    {2, "Ekexityyjt gjrfxbdfybt jhe;bz:", M_RD_Change_Bobbing,         'e'}, // Улучшенное покачивание оружия
    {2, "ldecndjkrf hfphsdftn dhfujd:",   M_RD_Change_SSGBlast,        'l'}, // Двустволка разрывает врагов
    {2, "pthrfkbhjdfybt nhegjd:",         M_RD_Change_FlipCorpses,     'p'}, // Зеркалирование трупов
    {2, "Ktdbnbhe.obt caths-fhntafrns:",  M_RD_Change_FloatPowerups,   'k'}, // Левитирующие сферы-артефакты
    {-1,"",0,'\0'},
    {2, "Jnj,hf;fnm ghbwtk:",             M_RD_Change_CrosshairDraw,   'j'}, // Отображать прицел
    {2, "Bylbrfwbz pljhjdmz:",            M_RD_Change_CrosshairHealth, 'b'}, // Индикация здоровья
    {-1,"",0,'\0'},
    {1, "",                               M_RD_Choose_Gameplay_4,      'l'}, // Далее >
    {1, "",                               M_RD_Choose_Gameplay_2,      'y'}, // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_3_Rus =
{
    rd_gameplay_3_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_3_Rus,
    M_RD_Draw_Gameplay_3,
    35,45,
    0
};

menuitem_t RD_Gameplay_Menu_4_Rus[]=
{
    {2, "Ljgjkybntkmyst kbwf buhjrf:",     M_RD_Change_ExtraPlayerFaces, 'l'}, // Дополнительные лица игрока
    {2, "'ktvtynfkm ,tp juhfybxtybz lei:", M_RD_Change_LostSoulsQty,    '\''}, // Элементаль без ограничения душ
    {2, "jnrk.xbnm pfghjc ,> pfuheprb:",   M_RD_Change_FastQSaveLoad,    'j'}, // Отключить запрос б. загрузки
    {2, "Ghjbuhsdfnm ltvjpfgbcb:",         M_RD_Change_NoInternalDemos,  'g'}, // Проигрывать демозаписи
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1, "",                                M_RD_Choose_Gameplay_1,       'l'}, // Далее >
    {1, "",                                M_RD_Choose_Gameplay_3,       'y'}, // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_4_Rus =
{
    rd_gameplay_4_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_4_Rus,
    M_RD_Draw_Gameplay_4,
    35,45,
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
    60,37,
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
    &MainDef_Rus,
    Vanilla_Options_Menu_Rus,
    M_Vanilla_DrawOptions,
    60,37,
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
    80,64,
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
    80,64,
    0
};


// =============================================================================
// [JN] VANILLA OPTIONS MENU: DRAWING
// =============================================================================

void M_Vanilla_DrawOptions(void)
{
    if (english_language)
    {
        // OPTIONS title
        V_DrawPatchDirect(108, 15, 0, W_CacheLumpName("M_OPTTTL", PU_CACHE));

        // Messages
        V_DrawPatchDirect(179, 53, 0, W_CacheLumpName((showMessages == 1 ?
                                            "M_MSGON" : "M_MSGOFF"), PU_CACHE));

        // Graphic Detail
        V_DrawPatchDirect(235, 69, 0, W_CacheLumpName((detailLevel == 1 ?
                                            "M_GDLOW" : "M_GDHIGH"), PU_CACHE));
    }
    else
    {
        // НАСТРОЙКИ title
        M_WriteTextBigCentered_RUS(15, "YFCNHJQRB");

        M_WriteTextBig_RUS(206, 53, showMessages == 1 ? "drk/" : "dsrk/");

        M_WriteTextBig_RUS(224, 69, detailLevel == 1 ? "ybp/" : "dsc/");
    }

    // Screen size slider
    M_DrawThermo(60 , 102, 12, screenSize);

    // Mouse sensivity slider
    M_DrawThermo(60 , 134, 10, mouseSensitivity);
}

void M_Vanilla_DrawSound(void)
{
    if (english_language)
    {
        // Sound volume title
        V_DrawPatchDirect (60, 38, 0, W_CacheLumpName("M_SVOL", PU_CACHE));
    }
    else
    {
        // ГРОМКОСТЬ title
        M_WriteTextBigCentered_RUS(38, "UHJVRJCNM");
    }

    // - Sfx volume slider -----------------------------------------------------
    M_DrawThermo(80, 81, 16, sfxVolume);

    // - Music volume slider ---------------------------------------------------
    M_DrawThermo(80, 113, 16, musicVolume);
}



// =============================================================================
// [JN] NEW OPTIONS MENU: DRAWING
// =============================================================================

// -----------------------------------------------------------------------------
// Main Options menu
// -----------------------------------------------------------------------------

void M_RD_Draw_Options(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "OPTIONS");
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB"); // НАСТРОЙКИ
    }
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
        M_WriteTextBigCentered_ENG(12, "RENDERING OPTIONS");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Rendering");
        dp_translation = NULL;

        // Floor and ceiling textures
        M_WriteTextSmall_ENG(231, 45, noflats ? "off" : "on");

        // Show FPS counter
        M_WriteTextSmall_ENG(162, 55, show_fps ? "on" : "off");

        //
        // Extra
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 65, "Extra");
        dp_translation = NULL;

        // Show disk icon
        M_WriteTextSmall_ENG(138, 75, show_diskicon ? "on" : "off");

        // Screen wiping effect
        M_WriteTextSmall_ENG(187, 85, screen_wiping == 1 ? "standard" :
                                      screen_wiping == 2 ? "loading" :
                                                           "off");
    }
    else
    {
        // НАСТРОЙКИ ВИДЕО
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB DBLTJ");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "htylthbyu"); // Рендеринг
        dp_translation = NULL;

        // Текстуры пола и потолка
        M_WriteTextSmall_RUS(217, 45, noflats ? "dsrk" : "drk");

        // Счетчик кадровой частоты
        M_WriteTextSmall_RUS(227, 55, show_fps ? "drk" : "dsrk");

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 65, "ljgjkybntkmyj");
        dp_translation = NULL;

        // Отображать значок дискеты
        M_WriteTextSmall_RUS(241, 75, show_diskicon ? "drk" : "dsrk");

        // Эффект смены экранов
        M_WriteTextSmall_RUS(202, 85, screen_wiping == 1 ? "cnfylfhnysq" :
                                      screen_wiping == 2 ? "pfuheprf" :
                                                           "dsrk");
    }
}

void M_RD_Change_DiskIcon (int choice)
{
    show_diskicon ^= 1;
}

void M_RD_Change_NoFlats (int choice)
{
    noflats ^= 1;

    // Reinitialize drawing functions
    R_ExecuteSetViewSize();
}

void M_RD_Change_FPScounter(int choice)
{
    show_fps ^= 1;
}

void M_RD_Change_Wiping (int choice)
{
    switch(choice)
    {
        case 0:
        {
            screen_wiping--;
            if (screen_wiping < 0)
                screen_wiping = 2;
            break;
        }

        case 1:
        {
            screen_wiping++;
            if (screen_wiping > 2)
                screen_wiping = 0;
            break;
        }
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

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "DISPLAY OPTIONS");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Screen");
        dp_translation = NULL;

        // Detail level
        M_WriteTextSmall_ENG(130, 105, detailLevel ? "low" : "high");

        // Local time
        M_WriteTextSmall_ENG(116, 115, 
                             local_time == 1 ? "12-hour (hh:mm)" :
                             local_time == 2 ? "12-hour (hh:mm:ss)" :
                             local_time == 3 ? "24-hour (hh:mm)" :
                             local_time == 4 ? "24-hour (hh:mm:ss)" :
                                               "off");

        //
        // Interface
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 125, "Interface");  
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB \"RHFYF"); // НАСТРОЙКИ ЭКРАНА

        //
        // Экран
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "\'rhfy");
        dp_translation = NULL;

        // Уровень детализации
        M_WriteTextSmall_RUS(193, 105, detailLevel ? "ybprbq" : "dscjrbq");

        // Системное время
        M_WriteTextSmall_RUS(161, 115, 
                             local_time == 1 ? "12-xfcjdjt (xx:vv)" :
                             local_time == 2 ? "12-xfcjdjt (xx:vv:cc)" :
                             local_time == 3 ? "24-xfcjdjt (xx:vv)" :
                             local_time == 4 ? "24-xfcjdjt (xx:vv:cc)" :
                                               "dsrk");

        //
        // Интерфейс
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 125, "bynthatqc");  
        dp_translation = NULL;
    }

    // Screen size slider
    M_DrawThermo_Small(35, 55, 12, screenSize);
    // Numerical representation of slider position
    snprintf(num, 4, "%3d", screenblocks);
    M_WriteTextSmall_ENG(145, 55, num);

    // Gamma-correction slider
    M_DrawThermo_Small(35, 75, 18, usegamma);

    // Level brightness slider
    M_DrawThermo_Small(35, 95, 5, level_brightness / 16);
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
        {
            usegamma--;
        }
        break;

        case 1:
        if (usegamma < 17)
        {
            usegamma++;
        }
        break;
    }
    I_SetPalette ((byte *)W_CacheLumpName(usegamma <= 8 ?
                                          "PALFIX" :
                                          "PLAYPAL",
                                          PU_CACHE) + 
                                          st_palette * 768);
    players[consoleplayer].message_system = english_language ?
                                            gammamsg[usegamma] :
                                            gammamsg_rus[usegamma];
}

void M_RD_Change_LevelBrightness(int choice)
{
    switch(choice)
    {
        case 0:
        if (level_brightness > 0)
            level_brightness -= 16;
        break;
    
        case 1:
        if (level_brightness < 64)
            level_brightness += 16;
        break;
    }
}

void M_RD_Change_Detail(int choice)
{
    detailLevel ^= 1;
    R_SetViewSize (screenblocks, detailLevel);
    players[consoleplayer].message_system = detailLevel ? detaillo : detailhi;
}

void M_RD_Change_LocalTime(int choice)
{
    switch(choice)
    {
        case 0: 
        local_time--;
        if (local_time < 0) 
            local_time = 4;
        // Reinitialize time widget's horizontal offset
        if (gamestate == GS_LEVEL)
        {
            HU_Start();
        }
        break;

        case 1:
        local_time++;
        if (local_time > 4)
            local_time = 0;
        // Reinitialize time widget's horizontal offset
        if (gamestate == GS_LEVEL)
        {    
            HU_Start();
        }
        break;
    }
}

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

void M_RD_Choose_MessagesSettings(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Messages_Def :
                    &RD_Messages_Def_Rus);
}

void M_RD_Draw_MessagesSettings(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "MESSAGES SETTINGS");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "General");  
        dp_translation = NULL;

        // Messages
        M_WriteTextSmall_ENG(165, 45, showMessages ? "on" : "off");

        // Text casts shadows
        M_WriteTextSmall_ENG(177, 55, draw_shadowed_text ? "on" : "off");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 65, "Colors");  
        dp_translation = NULL;

        // Item pickup
        if (messages_pickup_color == 1)
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(120, 75, "green"); dp_translation = NULL; }
        else if (messages_pickup_color == 2)
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(120, 75, "blue"); dp_translation = NULL; }
        else if (messages_pickup_color == 3)
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(120, 75, "gold"); dp_translation = NULL; }
        else if (messages_pickup_color == 4)
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(120, 75, "gray"); dp_translation = NULL; }
        else if (messages_pickup_color == 5)
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(120, 75, "tan"); dp_translation = NULL; }
        else if (messages_pickup_color == 6)
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(120, 75, "brown"); dp_translation = NULL; }
        else if (messages_pickup_color == 7)
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(120, 75, "brick"); dp_translation = NULL; }
        else
        { dp_translation = NULL; M_WriteTextSmall_ENG(120, 75, "red"); }

        // Revealed secret
        if (messages_secret_color == 1)
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(157, 85, "green"); dp_translation = NULL; }
        else if (messages_secret_color == 2)
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(157, 85, "blue"); dp_translation = NULL; }
        else if (messages_secret_color == 3)
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(157, 85, "gold"); dp_translation = NULL; }
        else if (messages_secret_color == 4)
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(157, 85, "gray"); dp_translation = NULL; }
        else if (messages_secret_color == 5)
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(157, 85, "tan"); dp_translation = NULL; }
        else if (messages_secret_color == 6)
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(157, 85, "brown"); dp_translation = NULL; }
        else if (messages_secret_color == 7)
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(157, 85, "brick"); dp_translation = NULL; }
        else
        { dp_translation = NULL; M_WriteTextSmall_ENG(157, 85, "red"); }

        // System message
        if (messages_system_color == 1)
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(149, 95, "green"); dp_translation = NULL; }
        else if (messages_system_color == 2)
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(149, 95, "blue"); dp_translation = NULL; }
        else if (messages_system_color == 3)
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(149, 95, "gold"); dp_translation = NULL; }
        else if (messages_system_color == 4)
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(149, 95, "gray"); dp_translation = NULL; }
        else if (messages_system_color == 5)
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(149, 95, "tan"); dp_translation = NULL; }
        else if (messages_system_color == 6)
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(149, 95, "brown"); dp_translation = NULL; }
        else if (messages_system_color == 7)
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(149, 95, "brick"); dp_translation = NULL; }
        else
        { dp_translation = NULL; M_WriteTextSmall_ENG(149, 95, "red"); }

        // Netgame chat
        if (messages_chat_color == 1)
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(136, 105, "green"); dp_translation = NULL; }
        else if (messages_chat_color == 2)
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(136, 105, "blue"); dp_translation = NULL; }
        else if (messages_chat_color == 3)
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(136, 105, "gold"); dp_translation = NULL; }
        else if (messages_chat_color == 4)
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(136, 105, "gray"); dp_translation = NULL; }
        else if (messages_chat_color == 5)
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(136, 105, "tan"); dp_translation = NULL; }
        else if (messages_chat_color == 6)
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(136, 105, "brown"); dp_translation = NULL; }
        else if (messages_chat_color == 7)
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(136, 105, "brick"); dp_translation = NULL; }
        else
        { dp_translation = NULL; M_WriteTextSmall_ENG(136, 105, "red"); }
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB CJJ<OTYBQ"); // НАСТРОЙКИ СООБЩЕНИЙ

        //
        // Общие
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "j,obt");  
        dp_translation = NULL;

        // Отображение сообщений
        M_WriteTextSmall_RUS(214, 45, showMessages ? "drk" : "dsrk");

        // Тексты отбрасывают тень
        M_WriteTextSmall_RUS(226, 55, draw_shadowed_text ? "drk" : "dsrk");

        //
        // Цвета
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 65, "wdtnf");  
        dp_translation = NULL;

        // Получение предметов
        if (messages_pickup_color == 1) // Зеленый
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(193, 75, "ptktysq"); dp_translation = NULL; }
        else if (messages_pickup_color == 2) // Синий
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(193, 75, "cbybq"); dp_translation = NULL; }
        else if (messages_pickup_color == 3) // Желтый
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(193, 75, ";tknsq"); dp_translation = NULL; }
        else if (messages_pickup_color == 4) // Белый
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(193, 75, ",tksq"); dp_translation = NULL; }
        else if (messages_pickup_color == 5) // Бежевый
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(193, 75, ",t;tdsq"); dp_translation = NULL; }
        else if (messages_pickup_color == 6) // Коричневый
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(193, 75, "rjhbxytdsq"); dp_translation = NULL; }
        else if (messages_pickup_color == 7) // Розовый
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(193, 75, "hjpjdsq"); dp_translation = NULL; }
        else                            // Красный
        { dp_translation = NULL; M_WriteTextSmall_RUS(193, 75, "rhfcysq"); }

        // Обнаружение тайников
        if (messages_secret_color == 1) // Зеленый
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(203, 85, "ptktysq"); dp_translation = NULL; }
        else if (messages_secret_color == 2) // Синий
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(203, 85, "cbybq"); dp_translation = NULL; }
        else if (messages_secret_color == 3) // Желтый
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(203, 85, ";tknsq"); dp_translation = NULL; }
        else if (messages_secret_color == 4) // Белый
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(203, 85, ",tksq"); dp_translation = NULL; }
        else if (messages_secret_color == 5) // Бежевый
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(203, 85, ",t;tdsq"); dp_translation = NULL; }
        else if (messages_secret_color == 6) // Коричневый
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(203, 85, "rjhbxytdsq"); dp_translation = NULL; }
        else if (messages_secret_color == 7) // Розовый
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(203, 85, "hjpjdsq"); dp_translation = NULL; }
        else                            // Красный
        { dp_translation = NULL; M_WriteTextSmall_RUS(203, 85, "rhfcysq"); }

        // Системные сообщения
        if (messages_system_color == 1) // Зеленый
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(197, 95, "ptktysq"); dp_translation = NULL; }
        else if (messages_system_color == 2) // Синий
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(197, 95, "cbybq"); dp_translation = NULL; }
        else if (messages_system_color == 3) // Желтый
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(197, 95, ";tknsq"); dp_translation = NULL; }
        else if (messages_system_color == 4) // Белый
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(197, 95, ",tksq"); dp_translation = NULL; }
        else if (messages_system_color == 5) // Бежевый
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(197, 95, ",t;tdsq"); dp_translation = NULL; }
        else if (messages_system_color == 6) // Коричневый
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(197, 95, "rjhbxytdsq"); dp_translation = NULL; }
        else if (messages_system_color == 7) // Розовый
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(197, 95, "hjpjdsq"); dp_translation = NULL; }
        else                            // Красный
        { dp_translation = NULL; M_WriteTextSmall_RUS(197, 95, "rhfcysq"); }

        // Чат сетевой игры
        if (messages_chat_color == 1) // Зеленый
        { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(164, 105, "ptktysq"); dp_translation = NULL; }
        else if (messages_chat_color == 2) // Синий
        { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(164, 105, "cbybq"); dp_translation = NULL; }
        else if (messages_chat_color == 3) // Желтый
        { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(164, 105, ";tknsq"); dp_translation = NULL; }
        else if (messages_chat_color == 4) // Белый
        { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(164, 105, ",tksq"); dp_translation = NULL; }
        else if (messages_chat_color == 5) // Бежевый
        { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(164, 105, ",t;tdsq"); dp_translation = NULL; }
        else if (messages_chat_color == 6) // Коричневый
        { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(164, 105, "rjhbxytdsq"); dp_translation = NULL; }
        else if (messages_chat_color == 7) // Розовый
        { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(164, 105, "hjpjdsq"); dp_translation = NULL; }
        else                            // Красный
        { dp_translation = NULL; M_WriteTextSmall_RUS(164, 105, "rhfcysq"); }
    }
}

void M_RD_Change_Messages (int choice)
{
    showMessages ^= 1;
    players[consoleplayer].message_system = showMessages ? msgon : msgoff;
    message_dontfuckwithme = true;
}

void M_RD_Change_Msg_Pickup_Color(int choice)
{
    switch(choice)
    {
        case 0:
        messages_pickup_color--;
        if (messages_pickup_color < 0)
            messages_pickup_color = 7;
        break;

        case 1:
        messages_pickup_color++;
        if (messages_pickup_color > 7)
            messages_pickup_color = 0;
        break;
    }
}

void M_RD_Change_Msg_Secret_Color(int choice)
{
    switch(choice)
    {
        case 0:
        messages_secret_color--;
        if (messages_secret_color < 0)
            messages_secret_color = 7;
        break;

        case 1:
        messages_secret_color++;
        if (messages_secret_color > 7)
            messages_secret_color = 0;
        break;
    }
}

void M_RD_Change_Msg_System_Color(int choice)
{
    switch(choice)
    {
        case 0:
        messages_system_color--;
        if (messages_system_color < 0)
            messages_system_color = 7;
        break;

        case 1:
        messages_system_color++;
        if (messages_system_color > 7)
            messages_system_color = 0;
        break;
    }
}

void M_RD_Change_Msg_Chat_Color(int choice)
{
    switch(choice)
    {
        case 0:
        messages_chat_color--;
        if (messages_chat_color < 0)
            messages_chat_color = 7;
        break;

        case 1:
        messages_chat_color++;
        if (messages_chat_color > 7)
            messages_chat_color = 0;
        break;
    }
}

void M_RD_Change_ShadowedText(int choice)
{
    draw_shadowed_text ^= 1;
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
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "AUTOMAP SETTINGS");

        // Automap colors (English only names, different placement)
        if (automap_color == 0)
        M_WriteTextSmall_ENG (170, 35, "doom");
        else if (automap_color == 1)
        M_WriteTextSmall_ENG (170, 35, "boom");
        else if (automap_color == 2)
        M_WriteTextSmall_ENG (170, 35, "jaguar");
        else if (automap_color == 3)
        M_WriteTextSmall_ENG (170, 35, "raven");
        else
        M_WriteTextSmall_ENG (170, 35, "strife");

        // Line antialiasing
        M_WriteTextSmall_ENG(193, 45, automap_antialias ? "on" : "off");

        // Level stats
        M_WriteTextSmall_ENG(159, 55, automap_stats ? "on" : "off");

        /*
        // Overlay mode
        M_WriteTextSmall_ENG(170, 65, automap_overlay ? "on" : "off");
        */

        // Rotate mode
        M_WriteTextSmall_ENG(163, 65, automap_rotate ? "on" : "off");


        // Follow mode
        M_WriteTextSmall_ENG(164, 75, automap_follow ? "on" : "off");

        // Grid
        M_WriteTextSmall_ENG(106, 85, automap_grid ? "on" : "off");
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB RFHNS"); // НАСТРОЙКИ КАРТЫ

        // Automap colors (English only names, different placement)
        if (automap_color == 0)
        M_WriteTextSmall_ENG (191, 35, "doom");
        else if (automap_color == 1)
        M_WriteTextSmall_ENG (191, 35, "boom");
        else if (automap_color == 2)
        M_WriteTextSmall_ENG (191, 35, "jaguar");
        else if (automap_color == 3)
        M_WriteTextSmall_ENG (191, 35, "raven");
        else
        M_WriteTextSmall_ENG (191, 35, "strife");

        // Сглаживание линий
        M_WriteTextSmall_RUS(214, 45, automap_antialias ? "drk" : "dsrk");

        // Статистика уровня
        M_WriteTextSmall_RUS(210, 55, automap_stats ? "drk" : "dsrk");

        /*
        // Режим наложения
        M_WriteTextSmall_RUS(203, 65, automap_overlay ? "drk" : "dsrk");
        */

        // Режим вращения
        M_WriteTextSmall_RUS(194, 65, automap_rotate ? "drk" : "dsrk");

        // Режим следования
        M_WriteTextSmall_RUS(208, 75, automap_follow ? "drk" : "dsrk");

        // Сетка
        M_WriteTextSmall_RUS(118, 85, automap_grid ? "drk" : "dsrk");
    }
}

void M_RD_Change_AutomapColor(int choice)
{
    switch(choice)
    {
        case 0: 
        automap_color--;
        if (automap_color < 0) 
            automap_color = 4;
        break;
    
        case 1:
        automap_color++;
        if (automap_color > 4)
            automap_color = 0;
        break;
    }
}

void M_RD_Change_AutomapAntialias(int choice)
{
    automap_antialias ^= 1;
}

/*
void M_RD_Change_AutomapOverlay(int choice)
{
    automap_overlay ^= 1;
}
*/

void M_RD_Change_AutomapStats(int choice)
{
    automap_stats ^= 1;
}

void M_RD_Change_AutomapRotate(int choice)
{
    automap_rotate ^= 1;
}

void M_RD_Change_AutomapFollow(int choice)
{
    automap_follow ^= 1;
}

void M_RD_Change_AutomapGrid(int choice)
{
    automap_grid ^= 1;
}

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------

void M_RD_Choose_Audio(int choice)
{
    if (vanilla)
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
    static char num[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "SOUND OPTIONS");

        //
        // Volume
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "volume");
        dp_translation = NULL;

        //
        // Channels
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 85, "channels");
        dp_translation = NULL;

        //
        // Advanced
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 115, "ADVANCED");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB PDERF"); // НАСТРОЙКИ ЗВУКА

        //
        // Громкость
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "uhjvrjcnm");
        dp_translation = NULL;

        //
        // Воспроизведение
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 85, "djcghjbpdtltybt");
        dp_translation = NULL;

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 115, "ljgjkybntkmyj");
        dp_translation = NULL;
    }

    // Draw SFX volume slider
    M_DrawThermo_Small(35, 55, 16, sfxVolume);
    // Draw numerical representation of SFX volume
    snprintf(num, 4, "%3d", sfxVolume);
    M_WriteTextSmall_ENG(177, 55, num);

    // Draw music volume slider
    M_DrawThermo_Small(35, 75, 16, musicVolume);
    // Draw numerical representation of music volume
    snprintf(num, 4, "%3d", musicVolume);
    M_WriteTextSmall_ENG(177, 75, num);

    // Draw SFX channels slider
    M_DrawThermo_Small(35, 105, 16, numChannels / 4 - 1);
    // Draw numerical representation of channels
    snprintf(num, 4, "%3d", numChannels);
    M_WriteTextSmall_ENG(177, 105, num);
}

void M_RD_Change_SfxVol (int choice)
{
    switch(choice)
    {
        case 0:
        if (sfxVolume)
        {
            sfxVolume--;
        }
        break;

        case 1:
        if (sfxVolume < 15)
        {
            sfxVolume++;
        }
        break;
    }

    S_SetSfxVolume(sfxVolume * 8);
}

void M_RD_Change_MusicVol (int choice)
{
    switch(choice)
    {
        case 0:
        if (musicVolume)
        {
            musicVolume--;
        }
        break;

        case 1:
        if (musicVolume < 15)
        {
            musicVolume++;
        }
        break;
    }

    S_SetMusicVolume(musicVolume * 8);
}

void M_RD_Change_SfxChannels (int choice)
{
    switch(choice)
    {
        case 0:
        if (numChannels > 4)
        {
            numChannels -= 4;
        }
        break;
    
        case 1:
        if (numChannels < 64)
        {
            numChannels += 4;
        }
        break;
    }

    // Reallocate sound channels
    S_ChannelsRealloc();
}


// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

void M_RD_Choose_SoundSystem (int choice)
{
    M_SetupNextMenu(english_language ?
                    &RD_Audio_System_Def :
                    &RD_Audio_System_Def_Rus);
}

void M_RD_Draw_Audio_System (void)
{
    static char num[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "SOUND SYSTEM");

        //
        // Sound system
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "sound system");
        dp_translation = NULL;

        // Sound effects
        if (snd_DesiredSfxDevice == 0)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(141, 45, "DISABLED");
            dp_translation = NULL;
        }
        else if (snd_DesiredSfxDevice == 1)
        {
            M_WriteTextSmall_ENG(141, 45, "PC SPEAKER");
        }
        else if (snd_DesiredSfxDevice == 3)
        {
            M_WriteTextSmall_ENG(141, 45, "SOUND BLASTER");
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(141, 45, "UNSUPPORTED DEVICE");
            dp_translation = NULL;
        }

        // Music
        if (snd_DesiredMusicDevice == 0)
        {   
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(79, 55, "disabled");
            dp_translation = NULL;
        }
        else if (snd_DesiredMusicDevice == 2)
        {
            M_WriteTextSmall_ENG(79, 55, "ADLIB");
        }
        else if (snd_DesiredMusicDevice == 3)
        {
            M_WriteTextSmall_ENG(79, 55, "SOUND BLASTER");
        }
        else if (snd_DesiredMusicDevice == 5)
        {
            M_WriteTextSmall_ENG(79, 55, "GRAVIS ULTRASOUND");
        }
        else if (snd_DesiredMusicDevice == 8)
        {
            M_WriteTextSmall_ENG(79, 55, "GENERAL MIDI");
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(79, 55, "UNSUPPORTED DEVICE");
            dp_translation = NULL;
        }

        // SB Port
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(75, 65, snd_SBport == 0x210 ? "210" :
                                         snd_SBport == 0x220 ? "220" :
                                         snd_SBport == 0x230 ? "230" :
                                         snd_SBport == 0x240 ? "240" :
                                         snd_SBport == 0x250 ? "250" :
                                         snd_SBport == 0x260 ? "260" :
                                         snd_SBport == 0x280 ? "280" :
                                                               "other");
        dp_translation = NULL;

        // SB IRQ
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            snprintf(num, 4, "%3d", snd_SBirq);
            M_WriteTextSmall_ENG(55, 75, num);
        dp_translation = NULL;

        // SB DMA channel
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            snprintf(num, 4, "%3d", snd_SBdma);
            M_WriteTextSmall_ENG(60, 85, num);
        dp_translation = NULL;

        // MIDI port
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 3   // sound blaster
        ||  snd_DesiredMusicDevice == 5)  // gus
        dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(104, 95, snd_Mport == 0x220 ? "220" :
                                          snd_Mport == 0x230 ? "230" :
                                          snd_Mport == 0x240 ? "240" :
                                          snd_Mport == 0x250 ? "250" :
                                          snd_Mport == 0x300 ? "300" :
                                          snd_Mport == 0x320 ? "320" :
                                          snd_Mport == 0x330 ? "330" :
                                          snd_Mport == 0x332 ? "332" :
                                          snd_Mport == 0x334 ? "334" :
                                          snd_Mport == 0x336 ? "336" :
                                          snd_Mport == 0x340 ? "340" :
                                          snd_Mport == 0x360 ? "360" :
                                                               "other");
        dp_translation = NULL;

        //
        // Quality
        //

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 105, "quality");
        dp_translation = NULL;

        // Sampling frequency (hz)
        if (snd_samplerate == 44100)
        {
            M_WriteTextSmall_ENG(179, 115, "44100 HZ");
        }
        else if (snd_samplerate == 22050)
        {
            M_WriteTextSmall_ENG(179, 115, "22050 HZ");
        }
        else if (snd_samplerate == 11025)
        {
            M_WriteTextSmall_ENG(179, 115, "11025 HZ");
        }

        //
        // Miscellaneous
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 125, "Miscellaneous");
        dp_translation = NULL;

        // Sfx mode
        M_WriteTextSmall_ENG(178, 135, snd_monomode ? "mono" : "stereo");

        // Pitch-shifted sounds
        M_WriteTextSmall_ENG(186, 145, snd_pitchshift ? "on" : "off");

        // Informative message
        if (itemOn >= rd_audio_sys_sfx
        &&  itemOn <= rd_audio_sys_mport)
        {
            dp_translation = cr[CR_GRAY];
            M_WriteTextSmall_ENG(1, 155, "changing will require restart of the program");
            dp_translation = NULL;
        }
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "PDERJDFZ CBCNTVF"); // ЗВУКОВАЯ СИСТЕМА

        //
        // Звуковая система
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "pderjdfz cbcntvf");
        dp_translation = NULL;

        // Звуковые эффекты
        if (snd_DesiredSfxDevice == 0)
        {
            // Отключены
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(175, 45, "jnrk.xtys");
            dp_translation = NULL;
        }
        else if (snd_DesiredSfxDevice == 1)
        {
            // Динамик ПК
            M_WriteTextSmall_RUS(175, 45, "lbyfvbr gr");
        }
        else if (snd_DesiredSfxDevice == 3)
        {
            // Цифровые
            M_WriteTextSmall_ENG(175, 45, "SOUND BLASTER");
        }
        else
        {
            // Не определено
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(175, 45, "yt jghtltktyj");
            dp_translation = NULL;
        }

        // Музыка
        if (snd_DesiredMusicDevice == 0)
        {
            // Отключена
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(94, 55, "jnrk.xtyf");
            dp_translation = NULL;
        }
        else if (snd_DesiredMusicDevice == 2)
        {
            M_WriteTextSmall_ENG(94, 55, "ADLIB");
        }
        else if (snd_DesiredMusicDevice == 3)
        {
            M_WriteTextSmall_ENG(94, 55, "SOUND BLASTER");
        }
        else if (snd_DesiredMusicDevice == 5)
        {
            M_WriteTextSmall_ENG(94, 55, "GRAVIS ULTRASOUND");
        }
        else if (snd_DesiredMusicDevice == 8)
        {
            M_WriteTextSmall_ENG(94, 55, "GENERAL MIDI");
        }
        else
        {
            // Не определено
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(94, 55, "yt jghtltktyj");
            dp_translation = NULL;
        }

        // SB Порт
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(75, 65, snd_SBport == 0x210 ? "210" :
                                         snd_SBport == 0x220 ? "220" :
                                         snd_SBport == 0x230 ? "230" :
                                         snd_SBport == 0x240 ? "240" :
                                         snd_SBport == 0x250 ? "250" :
                                         snd_SBport == 0x260 ? "260" :
                                         snd_SBport == 0x280 ? "280" :
                                                               "other");
        dp_translation = NULL;

        // SB IRQ
        M_WriteTextSmall_ENG(35, 75, "irq:");
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            snprintf(num, 4, "%3d", snd_SBirq);
            M_WriteTextSmall_ENG(55, 75, num);
        dp_translation = NULL;

        // SB канал DMA
        M_WriteTextSmall_ENG(35, 85, "dma:");
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 5   // gus
        ||  snd_DesiredMusicDevice == 8)  // general midi
        dp_translation = cr[CR_DARKRED];
            snprintf(num, 4, "%3d", snd_SBdma);
            M_WriteTextSmall_ENG(60, 85, num);
        dp_translation = NULL;

        // Порт MIDI
        M_WriteTextSmall_ENG(71, 95, "midi:");
        if (snd_DesiredMusicDevice == 0   // disabled
        ||  snd_DesiredMusicDevice == 2   // adlib
        ||  snd_DesiredMusicDevice == 3   // sound blaster
        ||  snd_DesiredMusicDevice == 5)  // gus
        dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(104, 95, snd_Mport == 0x220 ? "220" :
                                          snd_Mport == 0x230 ? "230" :
                                          snd_Mport == 0x240 ? "240" :
                                          snd_Mport == 0x250 ? "250" :
                                          snd_Mport == 0x300 ? "300" :
                                          snd_Mport == 0x320 ? "320" :
                                          snd_Mport == 0x330 ? "330" :
                                          snd_Mport == 0x332 ? "332" :
                                          snd_Mport == 0x334 ? "334" :
                                          snd_Mport == 0x336 ? "336" :
                                          snd_Mport == 0x340 ? "340" :
                                          snd_Mport == 0x360 ? "360" :
                                                               "other");
        dp_translation = NULL;

        //
        // Качество звучания
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 105, "rfxfcndj pdexfybz");
        dp_translation = NULL;

        // Частота дискретизации (гц)
        if (snd_samplerate == 44100)
        {
            M_WriteTextSmall_RUS(208, 115, "44100 uw");
        }
        else if (snd_samplerate == 22050)
        {
            M_WriteTextSmall_RUS(208, 115, "22050 uw");
        }
        else if (snd_samplerate == 11025)
        {
            M_WriteTextSmall_RUS(208, 115, "11025 uw");
        }

        //
        // Разное
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 125, "hfpyjt");
        dp_translation = NULL;

        // Режим звука
        M_WriteTextSmall_RUS(231, 135, snd_monomode ? "vjyj" : "cnthtj");

        // Произвольный питч-шифтинг
        M_WriteTextSmall_RUS(242, 145, snd_pitchshift ? "drk" : "dsrk");

        // Informative message: изменение потребует перезапуск программы
        if (itemOn >= rd_audio_sys_sfx
        &&  itemOn <= rd_audio_sys_mport)
        {
            dp_translation = cr[CR_GRAY];
            M_WriteTextSmall_RUS(3, 155, "bpvtytybt gjnht,etn gthtpfgecr ghjuhfvvs");
            dp_translation = NULL;
        }
    }
}

void M_RD_Change_SoundDevice (int choice)
{
    // [JN] Available values:
    // 0 = NO SOUND FX
    // 1 = PC Speaker
    // 3 = Sound Blaster
    switch(choice)
    {
        case 0:
        {
            if (snd_DesiredSfxDevice == 0)
                snd_DesiredSfxDevice = 3;
            else 
            if (snd_DesiredSfxDevice == 3)
                snd_DesiredSfxDevice = 1;
            else 
            if (snd_DesiredSfxDevice == 1)
                snd_DesiredSfxDevice = 0;
            break;
        }

        case 1:
        {
            if (snd_DesiredSfxDevice == 0)
                snd_DesiredSfxDevice = 1;
            else 
            if (snd_DesiredSfxDevice == 1)
                snd_DesiredSfxDevice = 3;
            else 
            if (snd_DesiredSfxDevice == 3)
                snd_DesiredSfxDevice = 0;
            break;
        }
    }

    // [JN] Un-lock unsupported device
    if (snd_DesiredSfxDevice != 0
    &&  snd_DesiredSfxDevice != 1
    &&  snd_DesiredSfxDevice != 3)
    {
        snd_DesiredSfxDevice = 0;
    }
}

void M_RD_Change_MusicDevice (int choice)
{
    // [JN] Available values:
    // 0 = NO MUSIC
    // 2 = Adlib
    // 3 = Sound Blaster
    // 5 = Gravis UltraSound
    // 8 = General MIDI
    switch(choice)
    {
        case 0:
        {
            if (snd_DesiredMusicDevice == 0)
                snd_DesiredMusicDevice = 8;
            else 
            if (snd_DesiredMusicDevice == 8)
                snd_DesiredMusicDevice = 5;
            else 
            if (snd_DesiredMusicDevice == 5)
                snd_DesiredMusicDevice = 3;
            else
            if (snd_DesiredMusicDevice == 3)
                snd_DesiredMusicDevice = 2;
            else
            if (snd_DesiredMusicDevice == 2)
                snd_DesiredMusicDevice = 0;
            break;
        }

        case 1:
        {
            if (snd_DesiredMusicDevice == 0)
                snd_DesiredMusicDevice = 2;
            else
            if (snd_DesiredMusicDevice == 2)
                snd_DesiredMusicDevice = 3;
            else
            if (snd_DesiredMusicDevice == 3)
                snd_DesiredMusicDevice = 5;
            else
            if (snd_DesiredMusicDevice == 5)
                snd_DesiredMusicDevice = 8;
            else
            if (snd_DesiredMusicDevice == 8)
                snd_DesiredMusicDevice = 0;
            break;
        }

    }

    // [JN] Un-lock unsupported device
    if (snd_DesiredMusicDevice != 0
    &&  snd_DesiredMusicDevice != 2
    &&  snd_DesiredMusicDevice != 3
    &&  snd_DesiredMusicDevice != 5
    &&  snd_DesiredMusicDevice != 8)
    {
        snd_DesiredMusicDevice = 0;
    }
}

void M_RD_Change_SBport(int choice)
{
    switch(choice)
    {
        case 0:
        {
            if (snd_SBport == 0x210)
                snd_SBport  = 0x280;
            else
            if (snd_SBport == 0x280)
                snd_SBport  = 0x260;
            else
            if (snd_SBport == 0x260)
                snd_SBport  = 0x250;
            else
            if (snd_SBport == 0x250)
                snd_SBport  = 0x240;
            else
            if (snd_SBport == 0x240)
                snd_SBport  = 0x230;
            else
            if (snd_SBport == 0x230)
                snd_SBport  = 0x220;
            else
            if (snd_SBport == 0x220)
                snd_SBport  = 0x210;
            break;
        }
        case 1:
        {
            if (snd_SBport == 0x210)
                snd_SBport  = 0x220;
            else
            if (snd_SBport == 0x220)
                snd_SBport  = 0x230;
            else
            if (snd_SBport == 0x230)
                snd_SBport  = 0x240;
            else
            if (snd_SBport == 0x240)
                snd_SBport  = 0x250;
            else
            if (snd_SBport == 0x250)
                snd_SBport  = 0x260;
            else
            if (snd_SBport == 0x260)
                snd_SBport  = 0x280;
            else
            if (snd_SBport == 0x280)
                snd_SBport  = 0x210;
            break;
        }
    }
}

void M_RD_Change_SBirq(int choice)
{
    switch(choice)
    {
        case 0:
        {
            if (snd_SBirq == 2)
                snd_SBirq  = 7;
            else
            if (snd_SBirq == 7)
                snd_SBirq  = 5;
            else
            if (snd_SBirq == 5)
                snd_SBirq  = 2;
            break;
        }
        case 1:
        {
            if (snd_SBirq == 2)
                snd_SBirq  = 5;
            else
            if (snd_SBirq == 5)
                snd_SBirq  = 7;
            else
            if (snd_SBirq == 7)
                snd_SBirq  = 2;
            break;
        }
    }
}

void M_RD_Change_SBdma(int choice)
{
    switch(choice)
    {
        case 0:
        {
            if (snd_SBdma == 0)
                snd_SBdma  = 7;
            else
            if (snd_SBdma == 7)
                snd_SBdma  = 6;
            else
            if (snd_SBdma == 6)
                snd_SBdma  = 5;
            else
            if (snd_SBdma == 5)
                snd_SBdma  = 3;
            else
            if (snd_SBdma == 3)
                snd_SBdma  = 1;
            else
            if (snd_SBdma == 1)
                snd_SBdma  = 0;
            break;
        }
        case 1:
        {
            if (snd_SBdma == 0)
                snd_SBdma  = 1;
            else
            if (snd_SBdma == 1)
                snd_SBdma  = 3;
            else
            if (snd_SBdma == 3)
                snd_SBdma  = 5;
            else
            if (snd_SBdma == 5)
                snd_SBdma  = 6;
            else
            if (snd_SBdma == 6)
                snd_SBdma  = 7;
            else
            if (snd_SBdma == 7)
                snd_SBdma  = 0;
            break;
        }
    }
}

void M_RD_Change_Mport(int choice)
{
    switch(choice)
    {
        case 0:
        {
            if (snd_Mport == 0x220)
                snd_Mport  = 0x360;
            else
            if (snd_Mport == 0x360)
                snd_Mport  = 0x340;
            else
            if (snd_Mport == 0x340)
                snd_Mport  = 0x336;
            else
            if (snd_Mport == 0x336)
                snd_Mport  = 0x334;
            else
            if (snd_Mport == 0x334)
                snd_Mport  = 0x332;
            else
            if (snd_Mport == 0x332)
                snd_Mport  = 0x330;
            else
            if (snd_Mport == 0x330)
                snd_Mport  = 0x320;
            else
            if (snd_Mport == 0x320)
                snd_Mport  = 0x300;
            else
            if (snd_Mport == 0x300)
                snd_Mport  = 0x250;
            else
            if (snd_Mport == 0x250)
                snd_Mport  = 0x240;
            else
            if (snd_Mport == 0x240)
                snd_Mport  = 0x230;
            else
            if (snd_Mport == 0x230)
                snd_Mport  = 0x220;
            else
            if (snd_Mport == 0x220)
                snd_Mport  = 0x360;
            break;
        }
        case 1:
        {
            if (snd_Mport == 0x220) 
                snd_Mport  = 0x230;
            else
            if (snd_Mport == 0x230)
                snd_Mport  = 0x240;
            else
            if (snd_Mport == 0x240)
                snd_Mport  = 0x250;
            else
            if (snd_Mport == 0x250)
                snd_Mport  = 0x300;
            else
            if (snd_Mport == 0x300)
                snd_Mport  = 0x320;
            else
            if (snd_Mport == 0x320)
                snd_Mport  = 0x330;
            else
            if (snd_Mport == 0x330)
                snd_Mport  = 0x332;
            else
            if (snd_Mport == 0x332)
                snd_Mport  = 0x334;
            else
            if (snd_Mport == 0x334)
                snd_Mport  = 0x336;
            else
            if (snd_Mport == 0x336)
                snd_Mport  = 0x340;
            else
            if (snd_Mport == 0x340)
                snd_Mport  = 0x360;
            else
            if (snd_Mport == 0x360)
                snd_Mport  = 0x220;
            break;
        }
    }
}

void M_RD_Change_Sampling(int choice)
{
    switch(choice)
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
        break;
    }

    // Reset sound channels and update WAV_PlayMode
    I_SetChannels(numChannels);
}

void M_RD_Change_SndMode (int choice)
{
    snd_monomode ^= 1;
}

void M_RD_Change_PitchShifting (int choice)
{
    snd_pitchshift ^= 1;
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
    static char num[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "CONTROL SETTINGS");

        //
        // Movement
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "movement");
        dp_translation = NULL;

        // Always run
        M_WriteTextSmall_ENG(119, 45, joybspeed >= 20 ? "on" : "off");

        //
        // Mouse
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 55, "mouse");
        dp_translation = NULL;

        // Mouse look
        M_WriteTextSmall_ENG(119, 85, mlook ? "on" : "off");

        // Invert Y axis
        if (!mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(130, 95, mouse_y_invert ? "on" : "off");
        dp_translation = NULL;

        // Vertical movement
        if (mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(171, 105, !novert ? "on" : "off");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "EGHFDKTYBT"); // УПРАВЛЕНИЕ

        //
        // Передвижение
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "gthtldb;tybt");
        dp_translation = NULL;

        // Режим постоянного бега
        M_WriteTextSmall_RUS(216, 45, joybspeed >= 20 ? "drk" : "dsrk");

        //
        // Мышь
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 55, "vsim");
        dp_translation = NULL;

        // Обзор мышью
        M_WriteTextSmall_RUS(135, 85, mlook ? "drk" : "dsrk");

        // Вертикальная инверсия
        if (!mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207, 95, mouse_y_invert ? "drk" : "dsrk");
        dp_translation = NULL;

        // Вертикальное перемещение
        if (mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_RUS(235, 105, !novert ? "drk" : "dsrk");
        dp_translation = NULL;
    }

    // Draw mouse sensivity slider
    M_DrawThermo_Small(35, 74, 17, mouseSensitivity);
    // Draw numerical representation of mouse sensivity
    snprintf(num, 4, "%3d", mouseSensitivity);
    M_WriteTextSmall_ENG(189, 75, num);
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
    mlook ^= 1;

    if (!mlook)
    {
        players[consoleplayer].centering = true;
    }
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

void M_RD_Change_InvertY(int choice)
{
    mouse_y_invert ^= 1;
}

void M_RD_Change_Novert(int choice)
{
    novert ^= 1;
}


// -----------------------------------------------------------------------------
// Gameplay features
// -----------------------------------------------------------------------------

void M_RD_Choose_Gameplay_1(int choice)
{
    // [JN] Don't allow to enter in -vanilla mode
    if (vanilla)
    {
        return;
    }

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

void M_RD_Draw_Gameplay_1(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "GAMEPLAY FEATURES");

        //
        // Graphical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Graphical");
        dp_translation = NULL;

        // Brightmaps
        dp_translation = brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(119, 45, brightmaps ? "on" : "off");
        dp_translation = NULL;

        // Fake contrast
        dp_translation = fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(142, 55, fake_contrast ? "on" : "off");
        dp_translation = NULL;

        // Translucency
        dp_translation = translucency ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(138, 65, translucency ? "on" : "off");
        dp_translation = NULL;
        
        // Fuzz effect
        dp_translation = improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(125, 75, 
                             improved_fuzz == 0 ? "Original" :
                             improved_fuzz == 1 ? "Original (b&w)" :
                             improved_fuzz == 2 ? "Improved" :
                                                  "Improved (b&w)");
        dp_translation = NULL;

        // Colored HUD elements
        dp_translation = colored_hud ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(195, 85, colored_hud ? "on" : "off");
        dp_translation = NULL;

        // Colored blood and corpses
        dp_translation = colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(229, 95, colored_blood ? "on" : "off");
        dp_translation = NULL;

        // Swirling liquids
        dp_translation = swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(150, 105, swirling_liquids ? "on" : "off");
        dp_translation = NULL;

        // Invulnerability affects sky
        dp_translation = invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(237, 115, invul_sky ? "on" : "off");
        dp_translation = NULL;

        // Flip weapons
        dp_translation = flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(131, 125, flip_weapons ? "on" : "off");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 145, "next page >"); 
        M_WriteTextSmall_ENG(35, 155, "< last page"); 
        M_WriteTextSmall_ENG(231, 155, "page 1/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Графика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "uhfabrf");
        dp_translation = NULL;

        // Брайтмаппинг
        dp_translation = brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(140, 45, brightmaps ? "drk" : "dsrk");
        dp_translation = NULL;

        // Имитация контрастности
        dp_translation = fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(217, 55, fake_contrast ? "drk" : "dsrk");
        dp_translation = NULL;

        // Прозрачность объектов
        dp_translation = translucency ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207, 65, translucency ? "drk" : "dsrk");
        dp_translation = NULL;

        // Эффект шума
        dp_translation = improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(134, 75, 
                             improved_fuzz == 0 ? "Jhbubyfkmysq" :
                             improved_fuzz == 1 ? "Jhbubyfkmysq (x*,)" :
                             improved_fuzz == 2 ? "Ekexityysq" :
                                                  "Ekexityysq (x*,)");
        dp_translation = NULL;

        // Разноцветные элементы HUD
        dp_translation = colored_hud ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(239, 85, colored_hud ? "drk" : "dsrk");
        dp_translation = NULL;

        // Разноцветная кровь и трупы
        dp_translation = colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(242, 95, colored_blood ? "drk" : "dsrk");
        dp_translation = NULL;

        // Улучшенная анимация жидкостей
        dp_translation = swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(275, 105, swirling_liquids ? "drk" : "dsrk");
        dp_translation = NULL;

        // Неуязвимость окрашивает небо
        dp_translation = invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(262, 115, invul_sky ? "drk" : "dsrk");
        dp_translation = NULL;

        // Зеркальное отражение оружия
        dp_translation = flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(259, 125, flip_weapons ? "drk" : "dsrk");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 145, "lfktt \\");      // далее >
        M_WriteTextSmall_RUS(35, 155, "/ yfpfl");       // < назад
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 1*4"); // страница 1/4
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_2(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "GAMEPLAY FEATURES");

        //
        // Audible
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Audible");
        dp_translation = NULL;

        // Play exit sounds
        dp_translation = play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(158, 45, play_exit_sfx ? "on" : "off");
        dp_translation = NULL;

        // Sound of crushing corpses
        dp_translation = crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(223 , 55, crushed_corpses_sfx ? "on" : "off");
        dp_translation = NULL;

        // Single sound of closing blazing door
        dp_translation = blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(240, 65, blazing_door_fix_sfx ? "on" : "off");
        dp_translation = NULL;

        // Monster alert waking up other monsters
        dp_translation = noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(263, 75, noise_alert_sfx ? "on" : "off");
        dp_translation = NULL;

        //
        // Tactical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 85, "Tactical");
        dp_translation = NULL;

        // Notify of revealed secrets
        dp_translation = secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(232, 95, secret_notification ? "on" : "off");
        dp_translation = NULL;

        // Show negative health
        dp_translation = negative_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(190, 105, negative_health ? "on" : "off");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 145, "next page >");
        M_WriteTextSmall_ENG(35, 155, "< prev page");
        M_WriteTextSmall_ENG(231, 155, "page 2/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Звук
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "Pder");
        dp_translation = NULL;

        // Звуки при выходе из игры
        dp_translation = play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(225, 45, play_exit_sfx ? "drk" : "dsrk");
        dp_translation = NULL;

        // Звук раздавливания трупов
        dp_translation = crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(236, 55, crushed_corpses_sfx ? "drk" : "dsrk");
        dp_translation = NULL;

        // Одиночный звук быстрой двери
        dp_translation = blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(260, 65, blazing_door_fix_sfx ? "drk" : "dsrk");
        dp_translation = NULL;

        // Общая тревога у монстров
        dp_translation = noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(227, 75, noise_alert_sfx ? "drk" : "dsrk");
        dp_translation = NULL;

        //
        // Тактика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 85, "Nfrnbrf");
        dp_translation = NULL;

        // Сообщать о найденном тайнике
        dp_translation = secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(260, 95, secret_notification ? "drk" : "dsrk");
        dp_translation = NULL;

        // Отрицательное здоровье в HUD
        dp_translation = negative_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(255, 105, negative_health ? "drk" : "dsrk");
        dp_translation = NULL;

        // Footer
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 145, "lfktt \\");      // далее >
        M_WriteTextSmall_RUS(35, 155, "/ yfpfl");       // < назад
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 2*4"); // страница 2/4
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_3(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "GAMEPLAY FEATURES");

        //
        // Physical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Physical");
        dp_translation = NULL;

        // Walk over and under monsters
        dp_translation = over_under ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(250, 45, over_under ? "on" : "off");
        dp_translation = NULL;

        // Corpses sliding from the ledges
        dp_translation = torque ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(264, 55, torque ? "on" : "off");
        dp_translation = NULL;

        // Weapon bobbing while firing
        dp_translation = weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(233, 65, weapon_bobbing ? "on" : "off");
        dp_translation = NULL;

        // Lethal pellet of a point-blank SSG
        dp_translation = ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(287, 75, ssg_blast_enemies ? "on" : "off");
        dp_translation = NULL;

        // Randomly mirrored corpses
        dp_translation = randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(231, 85, randomly_flipcorpses ? "on" : "off");
        dp_translation = NULL;

        // Floating powerups
        dp_translation = floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(171, 95, floating_powerups ? "on" : "off");
        dp_translation = NULL;

        //
        // Crosshair
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 105, "Crosshair");
        dp_translation = NULL;

        // Draw crosshair
        dp_translation = crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(146, 115, crosshair_draw ? "on" : "off");
        dp_translation = NULL;

        // Health indication
        dp_translation = crosshair_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(163, 125, crosshair_health ? "on" : "off");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 145, "next page >");
        M_WriteTextSmall_ENG(35, 155, "< prev page");
        M_WriteTextSmall_ENG(231, 155, "page 3/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Физика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "Abpbrf");
        dp_translation = NULL;

        // Перемещение под/над монстрами
        dp_translation = over_under ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(274, 45, over_under ? "drk" : "dsrk");
        dp_translation = NULL;

        // Трупы сползают с возвышений
        dp_translation = torque ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(256, 55, torque ? "drk" : "dsrk");
        dp_translation = NULL;

        // Улучшенное покачивание оружия
        dp_translation = weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(271, 65, weapon_bobbing ? "drk" : "dsrk");
        dp_translation = NULL;

        // Двустволка разрывает врагов
        dp_translation = ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(254, 75, ssg_blast_enemies ? "drk" : "dsrk");
        dp_translation = NULL;

        // Зеркалирование трупов
        dp_translation = randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207, 85, randomly_flipcorpses ? "drk" : "dsrk");
        dp_translation = NULL;

        // Левитирующие сферы-артефакты
        dp_translation = floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(275, 95, floating_powerups ? "drk" : "dsrk");
        dp_translation = NULL;

        //
        // Прицел
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 105, "Ghbwtk");
        dp_translation = NULL;

        // Отображать прицел
        dp_translation = crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(180, 115, crosshair_draw ? "drk" : "dsrk");
        dp_translation = NULL;

        // Индикация здоровья
        dp_translation = crosshair_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(186, 125, crosshair_health ? "drk" : "dsrk");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 145, "lfktt \\");      // далее >
        M_WriteTextSmall_RUS(35, 155, "/ yfpfl");       // < назад
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 3*4"); // страница 3/4
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_4(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(12, "GAMEPLAY FEATURES");

        //
        // Gameplay
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 35, "Gameplay");
        dp_translation = NULL;

        // Extra player faces on the HUD
        dp_translation = extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(255, 45, extra_player_faces ? "on" : "off");
        dp_translation = NULL;

        // Pain Elemental without Souls limit
        dp_translation = unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(284, 55, unlimited_lost_souls ? "on" : "off");
        dp_translation = NULL;

        // Don't prompt for q. saving/loading
        dp_translation = fast_quickload ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(284, 65, fast_quickload ? "on" : "off");
        dp_translation = NULL;

        // Play internal demos
        dp_translation = no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_ENG(183, 75, no_internal_demos ? "off" : "on");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35, 145, "first page >");
        M_WriteTextSmall_ENG(35, 155, "< prev page");
        M_WriteTextSmall_ENG(231, 155, "page 4/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(12, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Геймплей
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 35, "Utqvgktq");
        dp_translation = NULL;

        // Дополнительные лица игрока в HUD
        dp_translation = extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(247, 45, extra_player_faces ? "drk" : "dsrk");
        dp_translation = NULL;

        // Элементаль без ограничения Душ
        dp_translation = unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(274, 55, unlimited_lost_souls ? "drk" : "dsrk");
        dp_translation = NULL;

        // Отключить запрос б. загрузки
        dp_translation = fast_quickload ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(253, 65, fast_quickload ? "drk" : "dsrk");
        dp_translation = NULL;

        // Проигрывать демозаписи
        dp_translation = no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_RUS(219, 75, no_internal_demos ? "dsrk" : "drk");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35, 145, "lfktt \\");      // далее >
        M_WriteTextSmall_RUS(35, 155, "/ yfpfl");       // < назад
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 4*4"); // страница 4/4
        dp_translation = NULL;
    }
}

void M_RD_Change_Brightmaps (int choice)
{
    brightmaps ^= 1;
}

void M_RD_Change_FakeContrast (int choice)
{
    fake_contrast ^= 1;
}

void M_RD_Change_Translucency(int choice)
{
    translucency ^= 1;
}

void M_RD_Change_ImprovedFuzz(int choice)
{
    switch(choice)
    {
        case 0: 
        improved_fuzz--;
        if (improved_fuzz < 0) 
            improved_fuzz = 3;
        break;
    
        case 1:
        improved_fuzz++;
        if (improved_fuzz > 3)
            improved_fuzz = 0;
        break;
    }

    // Redraw game screen
    R_ExecuteSetViewSize();
}

void M_RD_Change_ColoredHUD (int choice)
{
    colored_hud ^= 1;
    
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_doRefresh();
    }
}

void M_RD_Change_ColoredBlood (int choice)
{
    colored_blood ^= 1;
}

void M_RD_Change_SwirlingLiquids (int choice)
{
    swirling_liquids ^= 1;
}

void M_RD_Change_InvulSky (int choice)
{
    invul_sky ^= 1;
}

void M_RD_Change_FlipWeapons(int choice)
{
    flip_weapons ^= 1;
}

void M_RD_Change_ExitSfx (int choice)
{
    play_exit_sfx ^= 1;
}

void M_RD_Change_CrushingSfx(int choice)
{
    crushed_corpses_sfx ^= 1;
}

void M_RD_Change_BlazingSfx(int choice)
{
    blazing_door_fix_sfx ^= 1;
}

void M_RD_Change_AlertSfx (int choice)
{
    noise_alert_sfx ^= 1;
}

void M_RD_Change_SecretNotify (int choice)
{
    secret_notification ^= 1;
}

void M_RD_Change_NegativeHealth (int choice)
{
    negative_health ^= 1;
}

void M_RD_Change_WalkOverUnder (int choice)
{
    over_under ^= 1;
}

void M_RD_Change_Torque (int choice)
{
    torque ^= 1;
}

void M_RD_Change_Bobbing (int choice)
{
    weapon_bobbing ^= 1;
}

void M_RD_Change_SSGBlast (int choice)
{
    ssg_blast_enemies ^= 1;
}

void M_RD_Change_FlipCorpses (int choice)
{
    randomly_flipcorpses ^= 1;
}

void M_RD_Change_FloatPowerups (int choice)
{
    floating_powerups ^= 1;
}

void M_RD_Change_CrosshairDraw (int choice)
{
    crosshair_draw ^= 1;
}

void M_RD_Change_CrosshairHealth (int choice)
{
    crosshair_health ^= 1;
}

void M_RD_Change_ExtraPlayerFaces (int choice)
{
    extra_player_faces ^= 1;
}

void M_RD_Change_LostSoulsQty (int choice)
{
    unlimited_lost_souls ^= 1;
}

void M_RD_Change_FastQSaveLoad (int choice)
{
    fast_quickload ^= 1;
}

void M_RD_Change_NoInternalDemos (int choice)
{
    no_internal_demos ^= 1;
}

// -----------------------------------------------------------------------------
// Back to Defaults
// -----------------------------------------------------------------------------

void M_RD_BackToDefaultsResponse (int ch)
{
    static char resetmsg[24];

    if (ch != 'y')
    return;

    // Rendering
    noflats       = 0;
    show_fps      = 0;
    show_diskicon = 1;
    screen_wiping = 1;

    // Display
    screenblocks = 10;
    screenSize = screenblocks - 3;
    usegamma     = 4;
    level_brightness = 0;
    detailLevel  = 0;
    local_time   = 0;

    // Messages
    showMessages = 1;
    draw_shadowed_text = 1;
    messages_pickup_color = 0;
    messages_secret_color = 3;
    messages_system_color = 0;
    messages_chat_color   = 1;

    //Automap
    automap_color     = 0;
    automap_antialias = 1;
    automap_stats     = 1;
    automap_rotate    = 0;
    automap_grid      = 0;
    automap_follow    = 1;    
    
    // Audio
    sfxVolume       = 8;  S_SetSfxVolume(sfxVolume * 8);
    musicVolume     = 8;  S_SetMusicVolume(musicVolume * 8);
    numChannels     = 32; S_ChannelsRealloc();
    snd_samplerate  = 22050; I_SetChannels(numChannels);
    snd_monomode    = 0;
    snd_pitchshift  = 0;

    // Controls
    joybspeed        = 29;
    mouseSensitivity = 5;
    mlook            = 0; players[consoleplayer].centering = true;
    mouse_y_invert   = 0;
    novert           = 1;

    // Gameplay (1)
    brightmaps         = 1;
    fake_contrast      = 0;
    improved_fuzz      = 3;
    colored_hud        = 0;
    colored_blood      = 1;
    swirling_liquids   = 1;
    invul_sky          = 1;
    flip_weapons       = 0;

    // Gameplay (2)
    play_exit_sfx        = 0;
    crushed_corpses_sfx  = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx      = 0;

    secret_notification = 1;
    negative_health     = 0;

    // Gameplay (3)
    over_under           = 0;
    torque               = 1;
    weapon_bobbing       = 1;
    ssg_blast_enemies    = 1;
    randomly_flipcorpses = 1;
    floating_powerups    = 0;

    crosshair_draw   = 0;
    crosshair_health = 1;

    // Gameplay (4)
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
        ST_doRefresh();
    }

    // Print informative message (настройки сброшены)
    snprintf(resetmsg, sizeof(resetmsg), english_language ?
                                         "Settings reset" :
                                         "Yfcnhjqrb c,hjitys");
    players[consoleplayer].message_system = resetmsg;
}

void M_RD_BackToDefaults (int choice)
{
    choice = 0;
    M_StartMessage(english_language ? 
                   RD_DEFAULTS : RD_DEFAULTS_RUS,
                   M_RD_BackToDefaultsResponse,true);
}


// -----------------------------------------------------------------------------
// Language hot-swapping
// -----------------------------------------------------------------------------

void M_RD_ChangeLanguage (int choice)
{
    extern void D_DoAdvanceDemo(void);
    extern void F_CastDrawer(void);
    extern void F_StartFinale(void);
    extern int  demosequence;
    extern int  finalestage;

    english_language ^= 1;

    // Reset options menu
    currentMenu = english_language ? &RD_Options_Def : &RD_Options_Def_Rus;

    // Update messages
    RD_DefineLanguageStrings();

    // Update TITLEPIC/CREDIT screens in live mode
    if (gamestate == GS_DEMOSCREEN)
    {
        if (demosequence == 0
        ||  demosequence == 2
        ||  demosequence == 4)
        {
            demosequence--;
            D_DoAdvanceDemo();
        }
    }

    if (gamestate == GS_LEVEL)
    {
        // Update HUD system
        HU_Start();

        // Update status bar
        ST_Start();
    }

    // Restart finale text
    if (gamestate == GS_FINALE)
    {
        if (finalestage == 2)
        {
            // Just redraw monster's name.
            F_CastDrawer();
        }
        else
        {
            // Restart finale text output.
            F_StartFinale();
        }
    }
}

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

// ------------
// English menu
// ------------

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

// ------------
// Russian menu
// ------------

menu_t  ReadDef1_Rus =
{
    read1_end,
    &MainDef_Rus,
    ReadMenu1,
    M_DrawReadThis1,
    280,181,
    0
};

// ------------
// English menu
// ------------

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

// ------------
// Russian menu
// ------------

menu_t  ReadDef2_Rus =
{
    read2_end,
    &ReadDef1_Rus,
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

// ------------
// English menu
// ------------

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
    67,38,
    0
};

// ------------
// Russian menu
// ------------

menu_t  LoadDef_Rus =
{
    load_end,
    &MainDef_Rus,
    LoadMenu,
    M_DrawLoad,
    67,38,
    0
};


//
// SAVE GAME MENU
//

// ------------
// English menu
// ------------

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

// ------------
// Russian menu
// ------------

menu_t  SaveDef_Rus =
{
    load_end,
    &MainDef_Rus,
    SaveMenu,
    M_DrawSave,
    67,38,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings (void)
{
    int   handle;
    int   count;
    int   i;
    char  name[256];

    for (i = 0;i < load_end;i++)
    {
        if (M_CheckParm("-cdrom"))
        {
            sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.sav",i);
        }
        else
        {
            sprintf(name,SAVEGAMENAME"%d.sav",i);
        }

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

void M_DrawLoad (void)
{
    int i;

    if (english_language)
    {
        V_DrawShadowDirect (73,14,0,W_CacheLumpName("M_LOADG",PU_CACHE));
        V_DrawPatchDirect (72,13,0,W_CacheLumpName("M_LOADG",PU_CACHE));
    }
    else
    {
        M_WriteTextBigCentered_RUS(13, "PFUHEPBNM BUHE"); // ЗАГРУЗИТЬ ИГРУ
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);

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
void M_DrawSaveLoadBorder (int x,int y)
{
    int i;

    V_DrawShadowDirect (x-7,y+9,0,W_CacheLumpName("M_LSLEFT",PU_CACHE));
    V_DrawPatchDirect (x-8,y+8,0,W_CacheLumpName("M_LSLEFT",PU_CACHE));

    for (i = 0 ; i < 24 ; i++)
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
void M_LoadSelect (int choice)
{
    char name[256];

    if (M_CheckParm("-cdrom"))
    {
        sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.sav",choice);
    }
    else
    {
        sprintf(name,SAVEGAMENAME"%d.sav",choice);
    }

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
        M_StartMessage(english_language ? LOADNET : LOADNET_RUS, NULL, false);
        return;
    }

    M_SetupNextMenu(english_language ? &LoadDef : &LoadDef_Rus);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave (void)
{
    int i;

    if (english_language)
    {
        // [JN] Use standard title "M_SAVEG"
        V_DrawShadowDirect (73, 14, 0, W_CacheLumpName("M_SAVEG",PU_CACHE));
        V_DrawPatchDirect (72, 13, 0, W_CacheLumpName("M_SAVEG",PU_CACHE));
    }
    else
    {
        if (QuickSaveTitle) // БЫСТРОЕ СОХРАНЕНИЕ
        M_WriteTextBigCentered_RUS(13, "<SCNHJT CJ{HFYTYBT");
        else                // СОХРАНИТЬ ИГРУ
        M_WriteTextBigCentered_RUS(13, "CJ{HFYBNM BUHE");
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
void M_DoSave (int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
    {
        quickSaveSlot = slot;
    }
}


//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect (int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    strcpy(saveOldString,savegamestrings[choice]);
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
    {
        savegamestrings[choice][0] = 0;
    }
    saveCharIndex = strlen(savegamestrings[choice]);
}


//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
        M_StartMessage(english_language ? SAVEDEAD : SAVEDEAD_RUS, NULL, false);
        return;
    }

    if (gamestate != GS_LEVEL)
    {
        return;
    }

    M_SetupNextMenu(english_language ? &SaveDef : &SaveDef_Rus);
    M_ReadSaveStrings();
}


//
// M_QuickSave
//

void M_QuickSaveResponse (int ch)
{
    if (ch == 'y')
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickSave (void)
{
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
    {
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(english_language ? &SaveDef : &SaveDef_Rus);
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
void M_QuickLoadResponse (int ch)
{
    if (ch == 'y')
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad (void)
{
    if (netgame)
    {
        M_StartMessage(english_language ? QLOADNET : QLOADNET_RUS, NULL, false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(english_language ? QSAVESPOT : QSAVESPOT_RUS, NULL, false);
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
void M_DrawReadThis1 (void)
{
    inhelpscreens = true;
    V_DrawPatchDirect(0, 0, 0, W_CacheLumpName
                     (english_language ? "HELP2" : "HELP2R", PU_CACHE));
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThisRetail (void)
{
    inhelpscreens = true;

    if (commercial)
    {
        V_DrawPatchDirect(0, 0, 0, W_CacheLumpName
                         (english_language ? "HELP" : "HELPR", PU_CACHE));
    }
    else
    {
        if (english_language)
        {
            V_DrawPatchDirect(0, 0, 0, W_CacheLumpName("HELP1", PU_CACHE));
        }
        else
        {
        V_DrawPatchDirect(0, 0, 0, W_CacheLumpName
                         (sigil ? "HELPSIG" : "HELP1R", PU_CACHE));
        }
    }
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu (void)
{
    if (english_language)
    {
        // [JN] Always draw original "M_DOOM" in English language
        V_DrawPatchDirect (94, 2, 0, W_CacheLumpName("M_DOOM",PU_CACHE));
    }
    else
    {
        // [JN] Draw translated titles for Plutonia and TNT
        V_DrawPatchDirect (94, 2, 0, W_CacheLumpName 
                           (tnt ? "M_DOOMT" : 
                            plutonia ? "M_DOOMP" :
                                          "M_DOOM", PU_CACHE));
    }
}


//
// M_NewGame
//
void M_DrawNewGame (void)
{
    if (english_language)
    {
        V_DrawShadowDirect(97, 15, 0, W_CacheLumpName("M_NEWG", PU_CACHE));
        V_DrawPatchDirect (96, 14, 0, W_CacheLumpName("M_NEWG", PU_CACHE));

        V_DrawShadowDirect(55, 39, 0, W_CacheLumpName("M_SKILL", PU_CACHE));
        V_DrawPatchDirect (54, 38, 0, W_CacheLumpName("M_SKILL", PU_CACHE));
    }
    else
    {
        M_WriteTextBigCentered_RUS(14, "YJDFZ BUHF");         // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(38, "Ehjdtym ckj;yjcnb:"); // Уровень сложности:
    }
}

void M_NewGame (int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(english_language ? NEWGAME : NEWGAME_RUS, NULL, false);
        return;
    }

    if (commercial)
    {
        M_SetupNextMenu(english_language ? &NewDef : &NewDef_Rus);
    }
    else
    {
        M_SetupNextMenu(english_language ? &EpiDef : &EpiDef_Rus);
    }
}


//
// M_Episode
//
void M_DrawEpisode (void)
{
    if (english_language)
    {
        V_DrawShadowDirect(97, 15, 0, W_CacheLumpName("M_NEWG", PU_CACHE));
        V_DrawPatchDirect(96, 14, 0, W_CacheLumpName("M_NEWG", PU_CACHE));

        V_DrawShadowDirect (55, 39, 0, W_CacheLumpName("M_EPISOD",PU_CACHE));
        V_DrawPatchDirect (54, 38, 0, W_CacheLumpName("M_EPISOD",PU_CACHE));
    }
    else
    {
        M_WriteTextBigCentered_RUS(14, "YJDFZ BUHF");       // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(38, "Rfrjq \'gbpjl?");   // Какой эпизод?
    }
}

void M_VerifyNightmare (int ch)
{
    if (ch != 'y')
    {
        return;
    }

    G_DeferedInitNew(nightmare, epi+1, 1);
    M_ClearMenus ();
}

void M_VerifyUltraNightmare (int ch)
{
    if (ch != 'y')
    {
        return;
    }

    G_DeferedInitNew(ultra_nm, epi+1, 1);
    M_ClearMenus();
}

void M_ChooseSkill (int choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(english_language ? NIGHTMARE : NIGHTMARE_RUS,
                       M_VerifyNightmare, true);
        return;
    }

    if (choice == ultra_nm)
    {
        M_StartMessage(english_language ? ULTRANM : ULTRANM_RUS,
                       M_VerifyUltraNightmare, true);
        return;
    }

    G_DeferedInitNew(choice, epi+1, 1);
    M_ClearMenus();
}

void M_Episode (int choice)
{
    if (shareware && choice)
    {
        M_StartMessage(english_language ? SWSTRING : SWSTRING_RUS, NULL, false);
        M_SetupNextMenu(&ReadDef1);
        return;
    }

    epi = choice;
    M_SetupNextMenu(english_language ? &NewDef : &NewDef_Rus);
}


//
// M_Options
//

void M_DrawOptions (void)
{
    V_DrawShadowDirect (90,14,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
    V_DrawPatchDirect (89,13,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
}


void M_Options (int choice)
{
    if (vanilla)
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
void M_EndGameResponse (int ch)
{
    if (ch != 'y')
    {
        return;
    }

    currentMenu->lastOn = itemOn;
    M_ClearMenus();
    D_StartTitle();
}

void M_EndGame (int choice)
{
    choice = 0;

    if (!usergame)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }

    if (netgame)
    {
        M_StartMessage(english_language ? NETEND : NETEND_RUS, NULL, false);
        return;
    }

    M_StartMessage(english_language ? ENDGAME : ENDGAME_RUS,
                   M_EndGameResponse, true);
}


//
// M_ReadThis
//
void M_ReadThis (int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &ReadDef1 : &ReadDef1_Rus);
}

void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &ReadDef2 : &ReadDef2_Rus);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(english_language ? &MainDef : &MainDef_Rus);
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


void M_QuitResponse (int ch)
{
    if (ch != 'y')
    {
        return;
    }

    // [JN] No need to play exit sfx if it's volume set to 0.
    if (!netgame && play_exit_sfx && sfxVolume > 0)
    {
        if (commercial)
        {
            S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
        }
        else
        {
            S_StartSound(NULL,quitsounds[(gametic>>2)&7]);
        }
        I_WaitVBL(105);
    }
    I_Quit ();
}


void M_QuitDOOM (int choice)
{
    // We pick index 0 which is language sensitive,
    //  or one at random, between 1 and maximum number.
    if (commercial)
    {
        if (english_language)
        {
            sprintf(endstring, "%s\n\n"DOSY, 
                    endmsg2[(gametic >> 2) % NUM_QUITMESSAGES]);
        }
        else
        {
            sprintf(endstring, "%s\n\n"DOSY_RUS, 
                    endmsg2_rus[(gametic >> 2) % NUM_QUITMESSAGES]);
        }
    }
    else
    {
        if (english_language)
        {
            sprintf(endstring, "%s\n\n"DOSY, 
                    endmsg1[(gametic >> 2) % NUM_QUITMESSAGES]);
        }
        else
        {
            sprintf(endstring, "%s\n\n"DOSY_RUS,
                    endmsg1_rus[(gametic >> 2) % NUM_QUITMESSAGES]);
        }
    }

    if (devparm)
    {
        // [JN] Quit immediately
        I_Quit ();
    }
    else
    {
        M_StartMessage(endstring, M_QuitResponse, true);
    }
}


//
// Menu Functions
//
void M_DrawThermo (int x, int y, int thermWidth, int thermDot)
{
    int xx;
    int i;

    xx = x;

    V_DrawShadowDirect (xx+1,y+1,0,W_CacheLumpName("M_THERML",PU_CACHE));
    V_DrawPatchDirect (xx,y,0,W_CacheLumpName("M_THERML",PU_CACHE));

    xx += 8;

    for (i=0 ; i<thermWidth ; i++)
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


void M_StartMessage (char *string, void *routine, boolean input)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}


void M_StopMessage (void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}


//
// Find string width from hu_font chars
//
int M_StringWidth(char *string)
{
    int i;
    int w = 0;
    int c;

    for (i = 0 ; i < strlen(string) ; i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
        {
            w += 4;
        }
        else
        {
            if (english_language || currentMenu == &SaveDef
            ||  currentMenu == &SaveDef_Rus)
            {
                w += SHORT (hu_font[c]->width);
            }
            else
            {
                w += SHORT (hu_font_small_rus[c]->width);
            }
        }
    }

    return w;
}


//
// Find string height from hu_font chars
//
int M_StringHeight(char *string)
{
    int i;
    int h;
    int height = SHORT(hu_font[0]->height);

    h = height;

    for (i = 0 ; i < strlen(string) ; i++)
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
boolean M_Responder (event_t *ev)
{
    int          ch;
    int          i;
    static int   joywait = 0;
    static int   mousewait = 0;
    // [FG] disable menu control by mouse
    /*
    static int   mousey = 0;
    static int   lasty = 0;
    static int   mousex = 0;
    static int   lastx = 0;
    */
    byte        *pal;

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
            // [FG] disable menu control by mouse
            /*
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
            */

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
        else if (ev->type == ev_keydown)
        {
            ch = ev->data1;
        }
    }

    if (ch == -1)
    {
        return false;
    }

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
            {
                M_DoSave(saveSlot);
            }
            break;

            default:
            ch = toupper(ch);
            if (ch != 32)
            if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
		    break;
            if (ch >= 32 && ch <= 127 && saveCharIndex < SAVESTRINGSIZE-1 
            &&  M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE-2)*8)
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
        if (messageNeedsInput == true 
        && !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE))
        {
            return false;
        }
        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
        {
            messageRoutine(ch);
        }
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
        {
            return false;
        }
        M_RD_Change_ScreenSize(0);
        S_StartSound(NULL,sfx_stnmov);
        return true;

        case KEY_EQUALS:        // Screen size up
        if (automapactive || chat_on)
        {
            return false;
        }
        M_RD_Change_ScreenSize(1);
        S_StartSound(NULL,sfx_stnmov);
        return true;

        case KEY_F1:            // Help key
	    M_StartControlPanel ();
	    currentMenu = english_language ? &ReadDef2 : &ReadDef2_Rus;
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
        if (vanilla)
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

    if (ch == KEY_F11)          // gamma toggle
    {
        usegamma++;
        if (usegamma > 17)
        usegamma = 0;

        players[consoleplayer].message_system = english_language ?
                                                gammamsg[usegamma] :
                                                gammamsg_rus[usegamma];
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
            {
                itemOn = 0;
            }
            else
            {
                itemOn++;
            }
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);
        return true;

        case KEY_UPARROW:
        do
        {
            if (!itemOn)
            {
                itemOn = currentMenu->numitems-1;
            }
            else
            {
                itemOn--;
            }
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);
        return true;

        case KEY_LEFTARROW:
        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;

        case KEY_RIGHTARROW:
        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;

        case KEY_ENTER:
        if (currentMenu->menuitems[itemOn].routine
        &&  currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1); // right arrow
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
        case KEY_PGDN:
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
    {
        return;
    }

    menuactive = 1;
    currentMenu = english_language ? &MainDef : &MainDef_Rus;   // JDC
    itemOn = currentMenu->lastOn;                               // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    int          start;
    char         string[80];
    short        i;
    short        max;
    static short x;
    static short y;

    inhelpscreens = false;

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = 100 - M_StringHeight(messageString)/2;

        while(*(messageString+start))
        {
            for (i = 0 ; i < strlen(messageString+start) ; i++)
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

            if (english_language)
            {
                M_WriteText(x,y,string);
                y += SHORT(hu_font[0]->height);
            }
            else
            {
                M_WriteTextSmall_RUS(x, y, string);
                y += SHORT(hu_font_small_rus[0]->height); 
            }
        }
        return;
    }

    if (!menuactive)
    {
        return;
    }

    if (currentMenu->routine)
    {
        // call Draw routine
        currentMenu->routine();
    }

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i=0 ; i<max ; i++)
    {
        // ---------------------------------------------------------------------
        // [JN] Write common menus by using standard graphical patches:
        // ---------------------------------------------------------------------
        if (currentMenu == &MainDef                // Main Menu
        ||  currentMenu == &MainDef_Rus            // Main Menu
        ||  currentMenu == &EpiDef                 // Episode selection
        ||  currentMenu == &EpiDef_Rus             // Episode selection
        ||  currentMenu == &NewDef                 // Skill level
        ||  currentMenu == &NewDef_Rus             // Skill level
        ||  currentMenu == &Vanilla_OptionsDef     // Vanilla options menu
        ||  currentMenu == &Vanilla_Audio_Def)     // Vanilla sound menu
        {
            // [JN] Draw patch if it's name is present,
            // i.e. don't try to draw placeholders as patches.
            if (currentMenu->menuitems[i].name[0])
            {
                V_DrawShadowDirect (x+1,y+1,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
                V_DrawPatchDirect (x,y,0, W_CacheLumpName(currentMenu->menuitems[i].name ,PU_CACHE));
            }

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // ---------------------------------------------------------------------
        // [JN] Write English options menu with big English font
        // ---------------------------------------------------------------------
        else if (currentMenu == &RD_Options_Def)
        {
            M_WriteTextBig_ENG(x, y, currentMenu->menuitems[i].name);

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // ---------------------------------------------------------------------
        // [JN] Write Russian options menu with big Russian font
        // ---------------------------------------------------------------------
        else
        if (currentMenu == &RD_Options_Def_Rus
        ||  currentMenu == &Vanilla_OptionsDef_Rus
        ||  currentMenu == &Vanilla_Audio_Def_Rus)
        {
            M_WriteTextBig_RUS(x, y, currentMenu->menuitems[i].name);

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // ---------------------------------------------------------------------
        // [JN] Write English submenus with small English font
        // ---------------------------------------------------------------------
        else 
        if (currentMenu == &RD_Rendering_Def
        ||  currentMenu == &RD_Display_Def
        ||  currentMenu == &RD_Messages_Def
        ||  currentMenu == &RD_Automap_Def
        ||  currentMenu == &RD_Audio_Def
        ||  currentMenu == &RD_Audio_System_Def
        ||  currentMenu == &RD_Controls_Def
        ||  currentMenu == &RD_Gameplay_Def_1
        ||  currentMenu == &RD_Gameplay_Def_2
        ||  currentMenu == &RD_Gameplay_Def_3
        ||  currentMenu == &RD_Gameplay_Def_4)
        {
            M_WriteTextSmall_ENG(x, y, currentMenu->menuitems[i].name);

            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
        // ---------------------------------------------------------------------
        // [JN] Write Russian submenus with small Russian font
        // ---------------------------------------------------------------------
        else
        if (currentMenu == &RD_Rendering_Def_Rus
        ||  currentMenu == &RD_Display_Def_Rus
        ||  currentMenu == &RD_Messages_Def_Rus
        ||  currentMenu == &RD_Automap_Def_Rus
        ||  currentMenu == &RD_Audio_Def_Rus
        ||  currentMenu == &RD_Audio_System_Def_Rus
        ||  currentMenu == &RD_Controls_Def_Rus
        ||  currentMenu == &RD_Gameplay_Def_1_Rus
        ||  currentMenu == &RD_Gameplay_Def_2_Rus
        ||  currentMenu == &RD_Gameplay_Def_3_Rus
        ||  currentMenu == &RD_Gameplay_Def_4_Rus)
        {
            M_WriteTextSmall_RUS(x, y, currentMenu->menuitems[i].name);
        
            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
    }

    // [JN] Define where to draw blinking skull and where blinking ">" symbol.
    if (currentMenu == &MainDef            || currentMenu == &MainDef_Rus
    ||  currentMenu == &EpiDef             || currentMenu == &EpiDef_Rus 
    ||  currentMenu == &NewDef             || currentMenu == &NewDef_Rus
    ||  currentMenu == &ReadDef1           || currentMenu == &ReadDef1_Rus
    ||  currentMenu == &ReadDef2           || currentMenu == &ReadDef2_Rus
    ||  currentMenu == &LoadDef            || currentMenu == &LoadDef_Rus
    ||  currentMenu == &SaveDef            || currentMenu == &SaveDef_Rus
    ||  currentMenu == &RD_Options_Def     || currentMenu == &RD_Options_Def_Rus
    ||  currentMenu == &Vanilla_OptionsDef || currentMenu == &Vanilla_OptionsDef_Rus
    ||  currentMenu == &Vanilla_Audio_Def  || currentMenu == &Vanilla_Audio_Def_Rus)
    {
        // [JN] Blinking skull
        V_DrawShadowDirect(x+1 + SKULLXOFF,currentMenu->y+1 - 5 + itemOn*LINEHEIGHT, 0,
                           W_CacheLumpName(skullName[whichSkull],PU_CACHE));
        V_DrawPatchDirect(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0,
                           W_CacheLumpName(skullName[whichSkull],PU_CACHE));
    }
    else
    {
        // [JN] Blinking ">" symbol
        if (whichSkull == 0)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + SKULLXOFF + 24, currentMenu->y 
                            + itemOn*LINEHEIGHT_SML, ">");
        dp_translation = NULL;
    }
}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
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
    currentMenu = english_language ? &MainDef : &MainDef_Rus;
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
        MainMenu_Rus[readthis].routine = M_ReadThis2;
        ReadDef2.prevMenu = NULL;
        ReadDef2_Rus.prevMenu = NULL;
    }

    if (commercial)
    {
        MainMenu[readthis] = MainMenu[quitdoom];
        MainMenu_Rus[readthis] = MainMenu_Rus[quitdoom];
        MainDef.numitems--;
        MainDef_Rus.numitems--;
        MainDef.y += 8;
        MainDef_Rus.y += 8;
        NewDef.prevMenu = &MainDef;
        NewDef_Rus.prevMenu = &MainDef_Rus;
        ReadDef1.routine = M_DrawReadThisRetail;
        ReadDef1_Rus.routine = M_DrawReadThisRetail;
        ReadDef1.x = 330;
        ReadDef1.y = 165;
        ReadMenu1[0].routine = M_FinishReadThis;
    }

    // We need to remove the fourth episode.
    if (!retail)
    {
        EpiDef.numitems--;
        EpiDef_Rus.numitems--;
    }

    // [JN] If no Sigil loaded, remove fifth episode.
    if (!sigil)
    {
        EpiDef.numitems--;
        EpiDef_Rus.numitems--;
    }
}


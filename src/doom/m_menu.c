//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


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
#include "sounds.h"
#include "m_menu.h"
#include "p_local.h"
#include "st_stuff.h"
#include "v_trans.h"

#include "rd_keybinds.h"
#include "rd_lang.h"
#include "crispy.h"
#include "jn.h"


#define SKULLXOFF      -32
#define LINEHEIGHT      16
#define LINEHEIGHT_SML  10  // [JN] Line height for small font


void (*messageRoutine)(int response);

boolean inhelpscreens;
boolean menuactive;

// [JN] Save strings and messages 
int     quickSaveSlot;      // -1 = no quicksave slot picked!
int     messageToPrint;     // 1 = message to be printed
int     messageToBind;      // [JN] Indicates if we are binding key
int    *keyToBind;          // [Dasperal] Pointer to key_var to bind
char   *messageString;      // ...and here is the message string!
char    savegamestrings[10][SAVESTRINGSIZE];
char    saveOldString[SAVESTRINGSIZE];  // old save description before edit
char	endstring[160];
boolean messageNeedsInput;  // timed message = no input from user
boolean QuickSaveTitle;     // [JN] Extra title "БЫСТРОЕ СОХРАНЕНИЕ"

// message x & y
int     messx;
int     messy;
int     messageLastMenuActive;

// we are going to be entering a savegame string
int     saveStringEnter;              
int     saveSlot;           // which slot to save in
int     saveCharIndex;      // which char we're editing


static boolean opldev;
extern boolean sendpause;


//
// MENU TYPEDEFS
//

typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short   status;
    char*    name;  // [JN] Extended from 10 to 128, so long text string may appear

    // choice = menu item #.
    // if status = 2, choice=0:leftarrow, 1:rightarrow
    void    (*routine)(int choice);

    // hotkey in menu
    char    alphaKey;			
} menuitem_t;


typedef struct menu_s
{
    short           numitems;       // # of menu items
    struct menu_s  *prevMenu;       // previous menu
    menuitem_t     *menuitems;      // menu items
    void            (*routine)();   // draw routine
    short           x;
    short           y;              // x,y of menu
    short           lastOn;         // last item user was on in menu
} menu_t;

typedef struct
{
    int y;
    char eng_text[128];
    char rus_text[128];
} subtitle_t;

typedef struct
{
    struct menu_s* prevMenu_eng;    // previous menu eng
    struct menu_s* prevMenu_rus;    // previous menu rus
    void (*prevPage_routine)(int);  // routine for choosing previous page
    void (*nextPage_routine)(int);  // routine for choosing next page
    char prevPage_HotKey_eng;       // eng hotkey for prev page menu item
    char prevPage_HotKey_rus;       // rus hotkey for prev page menu item
    char nextPage_HotKey_eng;       // eng hotkey for next page menu item
    char nextPage_HotKey_rus;       // rus hotkey for next page menu item
    char pageNumber_eng[9];         // 8 chars and one \0 char
    char pageNumber_rus[13];        // 12 chars and one \0 char
    subtitle_t* subtitles;          // pointer to array of subtitle_t
    char num_of_subtitles;          // uint_8 size of subtitles array
    bound_key_t keys[11];           // keys what can be bound on this page
} key_page_t;

short   itemOn;             // menu item skull is on
short   skullAnimCounter;   // skull animation counter
short   whichSkull;         // which skull to draw

// graphic name of skulls
char   *skullName[2] = {"M_SKULL1", "M_SKULL2"};

// current menudef
menu_t *currentMenu;                          


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
void M_WriteTextSmallCentered_ENG(int y, char *string);
void M_WriteTextSmallCentered_RUS(int y, char *string);


//
// PROTOTYPES
//

menu_t* getMenuFromKeyPage(key_page_t* keyPage, boolean isEngMenu);

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
void M_RD_StartBinding(int* key_var);
void M_RD_StartBinding_Mouse(int* key_var);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu prototypes
// -----------------------------------------------------------------------------

// Main Options menu
void M_RD_Draw_Options(void);

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
void M_RD_Change_ENDOOM(int choice);
void M_RD_Change_Renderer(int choice);
void M_RD_Change_PorchFlashing(int choice);

// Display
void M_RD_Choose_Display(int choice);
void M_RD_Draw_Display(void);
void M_RD_Change_ScreenSize(int choice);
void M_RD_Change_Gamma(int choice);
void M_RD_Change_LevelBrightness(int choice);
void M_RD_Change_MenuShading(int choice);
void M_RD_Change_Detail(int choice);
void M_RD_Change_LocalTime(int choice);

// Messages
void M_RD_Choose_MessagesAndTextSettings(int choice);
void M_RD_Draw_MessagesSettings(void);
void M_RD_Change_Messages(int choice);
void M_RD_Change_ShadowedText(int choice);
void M_RD_Change_Msg_TimeOut(int choice);
void M_RD_Change_Msg_Pickup_Color(int choice);
void M_RD_Change_Msg_Secret_Color(int choice);
void M_RD_Change_Msg_System_Color(int choice);
void M_RD_Change_Msg_Chat_Color(int choice);

// Automap
void M_RD_Choose_AutomapAndStatsSettings(int choice);
void M_RD_Draw_AutomapSettings(void);
void M_RD_Change_AutomapColor(int choice);
void M_RD_Change_AutomapAntialias(int choice);
void M_RD_Change_AutomapOverlay(int choice);
void M_RD_Change_AutomapRotate(int choice);
void M_RD_Change_AutomapFollow(int choice);
void M_RD_Change_AutomapGrid(int choice);
void M_RD_Change_AutomapGridSize(int choice);
void M_RD_Change_AutomapStats(int choice);
void M_RD_Change_AutomapLevelTime(int choice);
void M_RD_Change_AutomapTotalTime(int choice);
void M_RD_Change_AutomapCoords(int choice);

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
void M_RD_Change_Sampling(int choice);
void M_RD_Change_SndMode(int choice);
void M_RD_Change_PitchShifting(int choice);
void M_RD_Change_MuteInactive(int choice);

// Controls
void M_RD_Choose_Controls(int choice);
void M_RD_Draw_Controls(void);
void M_RD_Change_MouseLook(int choice);
void M_RD_Change_InvertY(int choice);
void M_RD_Change_Novert(int choice);
void M_RD_Change_Sensitivity(int choice);
void M_RD_Change_Acceleration(int choice);
void M_RD_Change_Threshold(int choice);

// Key bindings (1)
void M_RD_Bind_Key(int choice);
void M_RD_Draw_Bindings();
void M_RD_Choose_Bindings_1(int choice);
void M_RD_Choose_Bindings_2(int choice);
void M_RD_Choose_Bindings_3(int choice);
void M_RD_Choose_Bindings_4(int choice);

// Mouse bindings
void M_RD_Choose_Mouse_Bindings(int choice);
void M_RD_Draw_Mouse_Bindings(void);
void M_RD_Mouse_Bind_FireAttack(int choice);
void M_RD_Mouse_Bind_Use(int choice);
void M_RD_Mouse_Bind_MoveForward(int choice);
void M_RD_Mouse_Bind_MoveBackward(int choice);
void M_RD_Mouse_Bind_StrafeOn(int choice);
void M_RD_Mouse_Bind_StrafeLeft(int choice);
void M_RD_Mouse_Bind_StrafeRight(int choice);
void M_RD_Mouse_Bind_PrevWeapon(int choice);
void M_RD_Mouse_Bind_NextWeapon(int choice);

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
void M_RD_Change_ZAxisSfx(int choice);
void M_RD_Change_ExitSfx(int choice);
void M_RD_Change_CrushingSfx(int choice);
void M_RD_Change_BlazingSfx(int choice);
void M_RD_Change_AlertSfx(int choice);
void M_RD_Change_SecretNotify(int choice);
void M_RD_Change_NegativeHealth(int choice);
void M_RD_Change_InfraGreenVisor(int choice);
void M_RD_Change_WalkOverUnder(int choice);
void M_RD_Change_Torque(int choice);
void M_RD_Change_Bobbing(int choice);
void M_RD_Change_SSGBlast(int choice);
void M_RD_Change_FlipCorpses(int choice);
void M_RD_Change_FloatPowerups(int choice);
void M_RD_Change_TossDrop(int choice);
void M_RD_Change_CrosshairDraw(int choice);
void M_RD_Change_CrosshairType(int choice);
void M_RD_Change_CrosshairScale(int choice);
void M_RD_Change_FixMapErrors(int choice);
void M_RD_Change_FlipLevels(int choice);
void M_RD_Change_ExtraPlayerFaces(int choice);
void M_RD_Change_LostSoulsQty(int choice);
void M_RD_Change_LostSoulsAgr(int choice);
void M_RD_Change_PistolStart(int choice);
void M_RD_Change_DemoTimer(int choice);
void M_RD_Change_DemoTimerDir(int choice);
void M_RD_Change_DemoBar(int choice);
void M_RD_Change_NoInternalDemos(int choice);

// Level select
void M_RD_Choose_LevelSelect_1(int choice);
void M_RD_Choose_LevelSelect_2(int choice);
void M_RD_Draw_Level_1(void);
void M_RD_Draw_Level_2(void);

void M_RD_Change_Selective_Skill(int choice);
void M_RD_Change_Selective_Episode(int choice);
void M_RD_Change_Selective_Map(int choice);
void M_RD_Change_Selective_Health(int choice);
void M_RD_Change_Selective_Armor(int choice);
void M_RD_Change_Selective_ArmorType(int choice);

void M_RD_Change_Selective_WP_Chainsaw(int choice);
void M_RD_Change_Selective_WP_Shotgun(int choice);
void M_RD_Change_Selective_WP_SSgun(int choice);
void M_RD_Change_Selective_WP_Chaingun(int choice);
void M_RD_Change_Selective_WP_RLauncher(int choice);
void M_RD_Change_Selective_WP_Plasmagun(int choice);
void M_RD_Change_Selective_WP_BFG9000(int choice);

void M_RD_Change_Selective_Backpack(int choice);

void M_RD_Change_Selective_Ammo_0(int choice);
void M_RD_Change_Selective_Ammo_1(int choice);
void M_RD_Change_Selective_Ammo_2(int choice);
void M_RD_Change_Selective_Ammo_3(int choice);

void M_RD_Change_Selective_Key_0(int choice);
void M_RD_Change_Selective_Key_1(int choice);
void M_RD_Change_Selective_Key_2(int choice);
void M_RD_Change_Selective_Key_3(int choice);
void M_RD_Change_Selective_Key_4(int choice);
void M_RD_Change_Selective_Key_5(int choice);

void M_RD_Change_Selective_Fast(int choice);
void M_RD_Change_Selective_Respawn(int choice);

// Reset settings
void M_RD_Choose_Reset(int choice);
void M_RD_Draw_Reset(void);
void M_RD_BackToDefaults_Recommended(int choice);
void M_RD_BackToDefaults_Original(int choice);

// Language hot-swapping
void M_RD_ChangeLanguage(int choice);

// -----------------------------------------------------------------------------
// [JN] Vanilla menu prototypes
// -----------------------------------------------------------------------------

void M_Vanilla_DrawOptions(void);
void M_Vanilla_DrawSound(void);

void setAsEmptyMenuItem(menuitem_t* item)
{
    item->status = -1;
    item->name = "";
    item->routine = NULL;
    item->alphaKey = '\0';
}

menu_t* getMenuFromKeyPage(key_page_t* keyPage, boolean isEngMenu)
{
    menuitem_t* items;
    menu_t* r_menu;

    items = malloc(sizeof(menuitem_t) * 15);
    for (int i = 0; i < 11; ++i)
    {
        if (keyPage->keys[i] == bk_null || keyPage->keys[i] == bk_size)
            setAsEmptyMenuItem(&items[i]);
        else
        {
            bound_key_descriptor *keyDescriptor = BK_getKeyDescriptor(keyPage->keys[i]);
            items[i].status = 1;
            items[i].name = isEngMenu ? keyDescriptor->eng_name : keyDescriptor->rus_name;
            items[i].routine = M_RD_Bind_Key;
            items[i].alphaKey = isEngMenu ? keyDescriptor->eng_HotKey : keyDescriptor->rus_HotKey;
        }
    }

    setAsEmptyMenuItem(&items[11]);
    setAsEmptyMenuItem(&items[14]);

    items[12].status = items[13].status = 1;
    items[12].name = items[13].name = "";

    items[12].routine = keyPage->nextPage_routine;
    items[12].alphaKey = isEngMenu ? keyPage->nextPage_HotKey_eng : keyPage->nextPage_HotKey_rus;

    items[13].routine = keyPage->prevPage_routine;
    items[13].alphaKey = isEngMenu ? keyPage->prevPage_HotKey_eng : keyPage->prevPage_HotKey_rus;

    r_menu = malloc(sizeof(menu_t));
    r_menu->numitems = 15;
    r_menu->prevMenu = isEngMenu ? keyPage->prevMenu_eng : keyPage->prevMenu_rus;
    r_menu->menuitems = items;
    r_menu->routine = M_RD_Draw_Bindings;
    r_menu->x = 35;
    r_menu->y = 35;
    r_menu->lastOn = 0;

    return r_menu;
}

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
        if (cx+w > origwidth)
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
        if (cx+w > origwidth)
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
        if (cx+w > origwidth)
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
        if (cx+w > origwidth)
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
        if (cx+w > origwidth)
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
    cx = origwidth/2-width/2;
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
    cx = origwidth/2-width/2;
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

// -----------------------------------------------------------------------------
// M_WriteTextSmallCentered_ENG
// [JN] Write a centered string using a small hu_font_small_eng.
// -----------------------------------------------------------------------------

void M_WriteTextSmallCentered_ENG (int y, char *string)
{
    char *ch;
    int   c, cx, cy;
    int   w, width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT (hu_font_small_eng[c]->width);
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

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_eng[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_eng[c]);

        cx+=w;
    }
}


// -----------------------------------------------------------------------------
// M_WriteTextSmallCentered_RUS
// [JN] Write a centered string using a small hu_font_small_rus.
// -----------------------------------------------------------------------------

void M_WriteTextSmallCentered_RUS (int y, char *string)
{
    char *ch;
    int   c, cx, cy;
    int   w, width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT (hu_font_small_rus[c]->width);
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

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_rus[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_rus[c]);

        cx+=w;
    }
}


// -----------------------------------------------------------------------------
// M_RD_Draw_Menu_Background
//
// [JN] Draws menu background using FLOOR4_8 flat graphics.
// -----------------------------------------------------------------------------

void M_RD_Draw_Menu_Background (void)
{
    byte *src = W_CacheLumpName (DEH_String("FLOOR4_8"), PU_CACHE);
    byte *dest;
    int x, y;

    dest = I_VideoBuffer;

    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < screenwidth; x++)
        {
            *dest++ = src[(y & 63) * 64 + (x & 63)];
        }
    }

    inhelpscreens = true;
}

// -----------------------------------------------------------------------------
// M_RD_Jaguar_Menu_Background
//
// [JN] Draws INTERPIC graphics as a background. Used only in Jaguar Doom.
// -----------------------------------------------------------------------------

void M_RD_Jaguar_Menu_Background(void)
{
    inhelpscreens = true;
    V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    V_DrawPatch(0 + wide_delta, 0, W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE));
}


// =============================================================================
// DOOM MENU
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

// -----------------------------------------------------------------------------
// [JN] Special menu for Press Beta
// -----------------------------------------------------------------------------

// ------------
// English menu
// ------------

menuitem_t MainMenuBeta[]=
{
    {1, "M_BLVL1",  M_Episode, '1'},
    {1, "M_BLVL2",  M_Episode, '2'},
    {1, "M_BLVL3",  M_Episode, '3'},
    {1, "M_OPTION", M_Options, 'o'},
    {1, "M_QUITG",  M_QuitDOOM,'q'}
};

menu_t  MainDefBeta =
{
    main_end,
    NULL,
    MainMenuBeta,
    M_DrawMainMenu,
    97,70,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t MainMenuBeta_Rus[]=
{
    {1, "RD_BLVL1", M_Episode, '1'},
    {1, "RD_BLVL2", M_Episode, '2'},
    {1, "RD_BLVL3", M_Episode, '3'},
    {1, "RD_OPTN",  M_Options, 'o'},
    {1, "RD_QUITG", M_QuitDOOM,'q'}
};

menu_t  MainDefBeta_Rus =
{
    main_end,
    NULL,
    MainMenuBeta_Rus,
    M_DrawMainMenu,
    97,70,
    0
};


// =============================================================================
// EPISODE SELECT
// =============================================================================

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
    {1, "M_EPI1", M_Episode,'k'},
    {1, "M_EPI2", M_Episode,'t'},
    {1, "M_EPI3", M_Episode,'i'},
    {1, "M_EPI4", M_Episode,'t'},
    {1, "M_EPI5", M_Episode,'s'} // [crispy] Sigil
};

menu_t  EpiDef =
{
    ep_end,                 // # of menu items
    &MainDef,               // previous menu
    EpisodeMenu,            // menuitem_t ->
    M_DrawEpisode,          // drawing routine ->
    48,63,                  // x,y
    ep1                     // lastOn
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


// =============================================================================
// NEW GAME
// =============================================================================

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
    {1, "M_UNMARE", M_ChooseSkill, 'z'}
};

menu_t  NewDef =
{
    newg_end,               // # of menu items
    &EpiDef,                // previous menu
    NewGameMenu,            // menuitem_t ->
    M_DrawNewGame,          // drawing routine ->
    48,63,                  // x,y
    hurtme                  // lastOn
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
// LOAD GAME MENU
// =============================================================================

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
    {1, "", M_LoadSelect, '1'},
    {1, "", M_LoadSelect, '2'},
    {1, "", M_LoadSelect, '3'},
    {1, "", M_LoadSelect, '4'},
    {1, "", M_LoadSelect, '5'},
    {1, "", M_LoadSelect, '6'},
    {1, "", M_LoadSelect, '7'},
    {1, "", M_LoadSelect, '8'}
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

// =============================================================================
// SAVE GAME MENU
// =============================================================================

// ------------
// English menu
// ------------

menuitem_t SaveMenu[]=
{
    {1, "", M_SaveSelect, '1'},
    {1, "", M_SaveSelect, '2'},
    {1, "", M_SaveSelect, '3'},
    {1, "", M_SaveSelect, '4'},
    {1, "", M_SaveSelect, '5'},
    {1, "", M_SaveSelect, '6'},
    {1, "", M_SaveSelect, '7'},
    {1, "", M_SaveSelect, '8'}
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
    {1, "", M_ReadThis2, 0}
};

// ------------
// English menu
// ------------

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
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
    {1, "", M_FinishReadThis, 0}
};

// ------------
// English menu
// ------------

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
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
    rd_levelselect,
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
    {1, "Rendering",        M_RD_Choose_Rendering,  'r'},
    {1, "Display",          M_RD_Choose_Display,    'd'},
    {1, "Sound",            M_RD_Choose_Audio,      's'},
    {1, "Controls",         M_RD_Choose_Controls,   'c'},
    {1, "Gameplay",         M_RD_Choose_Gameplay_1, 'g'},
    {1, "Level select",     M_RD_Choose_LevelSelect_1,'l'},
    {1, "End Game",         M_EndGame,              'e'},
    {1, "Reset settings",   M_RD_Choose_Reset,      'r'},
    {2, "Language:english", M_RD_ChangeLanguage,    'l'},
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def =
{
    rd_end, 
    &MainDef,
    RD_Options_Menu,
    M_RD_Draw_Options,
    60, 22,
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
    {1, "Ds,jh ehjdyz",   M_RD_Choose_LevelSelect_1,'d'}, // Выбор уровня
    {1, "Pfrjyxbnm buhe", M_EndGame,              'p'}, // Закончить игру
    {1, "C,hjc yfcnhjtr", M_RD_Choose_Reset,      'c'}, // Сброс настроек
    {2, "Zpsr#heccrbq",   M_RD_ChangeLanguage,    'z'}, // Язык: русский
    {-1,"",0,'\0'}
};

menu_t  RD_Options_Def_Rus =
{
    rd_end, 
    &MainDef_Rus,
    RD_Options_Menu_Rus,
    M_RD_Draw_Options,
    60, 22,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

enum
{
    rd_rendering_widescreen,
    rd_rendering_vsync,
    rd_rendering_uncapped,
    rd_rendering_fps,
    rd_rendering_smoothing,
    rd_rendering_vga_porch,
    rd_rendering_software,
    rd_rendering_empty1,
    rd_rendering_diskicon,
    rd_rendering_wiping,
    rd_rendering_screenshots,
    rd_rendering_endoom,
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
    {2, "Porch palette changing:",   M_RD_Change_PorchFlashing,'v'},
    {2, "Video renderer:",           M_RD_Change_Renderer,    'v'},
    {-1,"",0,'\0'},
    {2, "Show disk icon:",           M_RD_Change_DiskIcon,    's'},
    {2, "Screen wiping effect:",     M_RD_Change_Wiping,      's'},
    {2, "Screenshot format:",        M_RD_Change_Screenshots, 's'},
    {2, "Show ENDOOM screen:",       M_RD_Change_ENDOOM,      's'},
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def =
{
    rd_rendering_end,
    &RD_Options_Def,
    RD_Rendering_Menu,
    M_RD_Draw_Rendering,
    35,35,
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
    {2, "Bpvtytybt gfkbnhs rhftd 'rhfyf:", M_RD_Change_PorchFlashing,'b'}, // Изменение палитры краёв экрана
    {2, "J,hf,jnrf dbltj:",                M_RD_Change_Renderer,    'j'}, // Обработка видео
    {-1,"",0,'\0'},                                                       // Дополнительно
    {2, "Jnj,hf;fnm pyfxjr lbcrtns:",      M_RD_Change_DiskIcon,    'j'}, // Отображать значок дискеты
    {2, "\'aatrn cvtys \'rhfyjd:",         M_RD_Change_Wiping,      '\''}, // Эффект смены экранов
    {2, "Ajhvfn crhbyijnjd:",              M_RD_Change_Screenshots, 'a'}, // Формат скриншотов
    {2, "Gjrfpsdfnm \'rhfy",               M_RD_Change_ENDOOM,      'g'}, // Показывать экран ENDOOM
    {-1,"",0,'\0'}
};

menu_t  RD_Rendering_Def_Rus =
{
    rd_rendering_end,
    &RD_Options_Def_Rus,
    RD_Rendering_Menu_Rus,
    M_RD_Draw_Rendering,
    35,35,
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
    rd_display_menu_shading,
    rd_display_empty4,
    rd_display_detail,
    rd_display_localtime,
    rd_display_empty5,
    rd_display_messages_settings,
    rd_display_automap_settings,
    rd_display_end
} rd_display_e;

// ------------
// English menu
// ------------

menuitem_t RD_Display_Menu[]=
{
    {2, "screen size",         M_RD_Change_ScreenSize,      's'},
    {-1,"",0,'\0'},
    {2, "gamma-correction",    M_RD_Change_Gamma,           'g'},
    {-1,"",0,'\0'},
    {2, "level brightness",    M_RD_Change_LevelBrightness, 'l'},
    {-1,"",0,'\0'},
    {2, "menu shading",        M_RD_Change_MenuShading,     'm'},
    {-1,"",0,'\0'},
    {2, "detail level:",       M_RD_Change_Detail,          'd'},
    {2, "local time:",         M_RD_Change_LocalTime,       'l'},
    {-1,"",0,'\0'},
    {1, "messages and texts",  M_RD_Choose_MessagesAndTextSettings, 'm'},
    {1, "automap and statistics", M_RD_Choose_AutomapAndStatsSettings, 'a'},
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def =
{
    rd_display_end,
    &RD_Options_Def,
    RD_Display_Menu,
    M_RD_Draw_Display,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Display_Menu_Rus[]=
{
    {2, "hfpvth buhjdjuj \'rhfyf",  M_RD_Change_ScreenSize,      'h'}, // Размер игрового экрана
    {-1,"",0,'\0'},                                                    //
    {2, "ehjdtym ufvvf-rjhhtrwbb",  M_RD_Change_Gamma,           'e'}, // Уровень гамма-коррекции
    {-1,"",0,'\0'},                                                    //
    {2, "ehjdtym jcdtotyyjcnb",     M_RD_Change_LevelBrightness, 'e'}, // Уровень освещенности
    {-1,"",0,'\0'},                                                    //
    {2, "pfntvytybt ajyf vty.",     M_RD_Change_MenuShading,     'p'}, // Затемнение фона меню
    {-1,"",0,'\0'},                                                    //
    {2, "ehjdtym ltnfkbpfwbb:",     M_RD_Change_Detail,          'e'}, // Уровень детализации:
    {2, "cbcntvyjt dhtvz:",         M_RD_Change_LocalTime,       'c'}, // Системное время:
    {-1,"",0,'\0'},                                                    //
    {1, "cjj,otybz b ntrcns",       M_RD_Choose_MessagesAndTextSettings,'c'}, // Сообщения и тексты
    {1, "rfhnf b cnfnbcnbrf",       M_RD_Choose_AutomapAndStatsSettings, 'r'}, // Карта и статистика
    {-1,"",0,'\0'}
};

menu_t  RD_Display_Def_Rus =
{
    rd_display_end,
    &RD_Options_Def_Rus,
    RD_Display_Menu_Rus,
    M_RD_Draw_Display,
    35,35,
    0
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

enum
{
    rd_messages_toggle,
    rd_messages_timeout,
    rd_messages_empty1,
    rd_messages_shadows,
    rd_messages_empty2,
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
    {2, "messages enabled:",   M_RD_Change_Messages,        'm'},
    {2, "message timeout",     M_RD_Change_Msg_TimeOut,     'm'},
    {-1,"",0,'\0'},
    {2, "text casts shadows:", M_RD_Change_ShadowedText,    't'},
    {-1,"",0,'\0'},
    {2, "item pickup:",        M_RD_Change_Msg_Pickup_Color,'i'},
    {2, "revealed secret:",    M_RD_Change_Msg_Secret_Color,'r'},
    {2, "system message:",     M_RD_Change_Msg_System_Color,'s'},
    {2, "netgame chat:",       M_RD_Change_Msg_Chat_Color,  'n'},
    {-1,"",0,'\0'}
};

menu_t  RD_Messages_Def =
{
    rd_messages_end,
    &RD_Display_Def,
    RD_Messages_Menu,
    M_RD_Draw_MessagesSettings,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Messages_Menu_Rus[]=
{
    {2, "jnj,hf;tybt cjj,otybq:",   M_RD_Change_Messages,        'j'}, // Отображение сообщений:
    {2, "nfqvfen jnj,hf;tybz",      M_RD_Change_Msg_TimeOut,     'n'}, // Таймаут отображения
    {-1,"",0,'\0'},
    {2, "ntrcns jn,hfcsdf.n ntym:", M_RD_Change_ShadowedText,    'n'}, // Тексты отбрасывают тень:
    {-1,"",0,'\0'},
    {2, "gjkextybt ghtlvtnjd:",     M_RD_Change_Msg_Pickup_Color,'g'}, // Получение предметов:
    {2, "j,yfhe;tybt nfqybrjd:",    M_RD_Change_Msg_Secret_Color,'j'}, // Обнаружение тайников:
    {2, "cbcntvyst cjj,otybz:",     M_RD_Change_Msg_System_Color,'c'}, // Системные сообщения:
    {2, "xfn ctntdjq buhs:",        M_RD_Change_Msg_Chat_Color,  'x'}, // Чат сетевой игры:
    {-1,"",0,'\0'}
};

menu_t  RD_Messages_Def_Rus =
{
    rd_messages_end,
    &RD_Display_Def_Rus,
    RD_Messages_Menu_Rus,
    M_RD_Draw_MessagesSettings,
    35,35,
    0
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

enum
{
    rd_automap_colors,
    rd_automap_antialias,
    rd_automap_overlay,
    rd_automap_rotate,
    rd_automap_follow,
    rd_automap_grid,
    rd_automap_grid_size,
    rd_automap_empty1,
    rd_automap_stats,
    rd_automap_l_time,
    rd_automap_t_time,
    rd_automap_coords,    
    rd_automap_end
} rd_automap_e;

// ------------
// English menu
// ------------

menuitem_t RD_Automap_Menu[]=
{
    {2, "color scheme:",      M_RD_Change_AutomapColor,     'c'},
    {2, "line antialiasing:", M_RD_Change_AutomapAntialias, 'l'},
    {2, "overlay mode:",      M_RD_Change_AutomapOverlay,   'o'},
    {2, "rotate mode:",       M_RD_Change_AutomapRotate,    'r'},
    {2, "follow mode:",       M_RD_Change_AutomapFollow,    'f'},
    {2, "grid:",              M_RD_Change_AutomapGrid,      'g'},
    {2, "grid size:",         M_RD_Change_AutomapGridSize,  'g'},
    {-1,"",0,'\0'},
    {2, "level stats:",       M_RD_Change_AutomapStats,     'l'},
    {2, "level time:",        M_RD_Change_AutomapLevelTime, 'l'},
    {2, "total time:",        M_RD_Change_AutomapTotalTime, 't'},
    {2, "player coords:",     M_RD_Change_AutomapCoords,    'p'},
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
    {2, "ht;bv yfkj;tybz:",   M_RD_Change_AutomapOverlay,   'h'}, // Режим наложения:
    {2, "ht;bv dhfotybz:",    M_RD_Change_AutomapRotate,    'h'}, // Режим вращения:
    {2, "ht;bv cktljdfybz:",  M_RD_Change_AutomapFollow,    'h'}, // Режим следования:
    {2, "ctnrf:",             M_RD_Change_AutomapGrid,      'c'}, // Сетка:
    {2, "hfpvth ctnrb:",      M_RD_Change_AutomapGridSize,  'h'}, // Размер сетки:    
    {-1,"",0,'\0'},
    {2, "cnfnbcnbrf ehjdyz:", M_RD_Change_AutomapStats,     'c'}, // Статистика уровня:
    {2, "dhtvz ehjdyz:",      M_RD_Change_AutomapLevelTime, 'd'}, // Время уровня:
    {2, "j,ott dhtvz:",       M_RD_Change_AutomapTotalTime, 'j'}, // Общее время:
    {2, "rjjhlbyfns buhjrf:", M_RD_Change_AutomapCoords,    'r'}, // Координаты игрока:
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
    {2, "sfx volume",              M_RD_Change_SfxVol,      's'},
    {-1,"",0,'\0'},
    {2, "music volume",            M_RD_Change_MusicVol,    'm'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {2, "sound channels",          M_RD_Change_SfxChannels, 's'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"sound system settings",    M_RD_Choose_SoundSystem, 's'},
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def =
{
    rd_audio_end,
    &RD_Options_Def,
    RD_Audio_Menu,
    M_RD_Draw_Audio,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Audio_Menu_Rus[]=
{
    {2, "pder",                         M_RD_Change_SfxVol,      'p'}, // Звук
    {-1,"",0,'\0'},                                                    //
    {2, "vepsrf",                       M_RD_Change_MusicVol,    'v'}, // Музыка
    {-1,"",0,'\0'},                                                    //
    {-1,"",0,'\0'},                                                    //
    {2, "Pderjdst rfyfks",              M_RD_Change_SfxChannels, 'p'}, // Звуковые каналы
    {-1,"",0,'\0'},                                                    //
    {-1,"",0,'\0'},                                                    //
    {1,"yfcnhjqrb pderjdjq cbcntvs",    M_RD_Choose_SoundSystem, 'y'}, // Настройки звуковой системы...
    {-1,"",0,'\0'}
};

menu_t RD_Audio_Def_Rus =
{
    rd_audio_end,
    &RD_Options_Def_Rus,
    RD_Audio_Menu_Rus,
    M_RD_Draw_Audio,
    35,35,
    0
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

enum
{
    rd_audio_sys_sfx,
    rd_audio_sys_music,
    rd_audio_sys_empty1,
    rd_audio_sys_sampling,
    rd_audio_sys_empty2,
    rd_audio_sys_sndmode,
    rd_audio_sys_sndpitch,
    rd_audio_sys_muteinactive,
    rd_audio_sys_end
} rd_audio_sys_e;

// ------------
// English menu
// ------------

menuitem_t RD_Audio_System_Menu[]=
{
    {2, "sound effects:",        M_RD_Change_SoundDevice,   's'},
    {2, "music:",                M_RD_Change_MusicDevice,   'm'},
    {-1,"",0,'\0'},
    {2, "sampling frequency:",   M_RD_Change_Sampling,      's'},
    {-1,"",0,'\0'},
    {2, "sound effects mode:",   M_RD_Change_SndMode,       's'},
    {2, "pitch-shifted sounds:", M_RD_Change_PitchShifting, 'p'},
    {2, "mute inactive window:", M_RD_Change_MuteInactive,  'm'},
    {-1,"",0,'\0'}
};

menu_t RD_Audio_System_Def =
{
    rd_audio_end,
    &RD_Audio_Def,
    RD_Audio_System_Menu,
    M_RD_Draw_Audio_System,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Audio_System_Menu_Rus[]=
{
    {2, "pderjdst \'aatrns:",         M_RD_Change_SoundDevice,   'p'}, // Звуковые эффекты
    {2, "vepsrf:",                    M_RD_Change_MusicDevice,   'v'}, // Музыка
    {-1,"",0,'\0'},                                                    //
    {2, "xfcnjnf lbcrhtnbpfwbb:",     M_RD_Change_Sampling,      'x'}, // Частота дискретизации
    {-1,"",0,'\0'},                                                    //
    {2, "Ht;bv pderjds[ \'aatrnjd:",  M_RD_Change_SndMode,       'h'}, // Режим звуковых эффектов
    {2, "ghjbpdjkmysq gbnx-ibanbyu:", M_RD_Change_PitchShifting, 'g'}, // Произвольный питч-шифтинг
    {2, "pder d ytfrnbdyjv jryt:",    M_RD_Change_MuteInactive,  'p'}, // Звук в неактивном окне
    {-1,"",0,'\0'}
};

menu_t RD_Audio_System_Def_Rus =
{
    rd_audio_end,
    &RD_Audio_Def_Rus,
    RD_Audio_System_Menu_Rus,
    M_RD_Draw_Audio_System,
    35,35,
    0
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

enum
{
    rd_controls_kdb_bindings,
    rd_controls_mouse_bindings,
    rd_controls_empty1,
    rd_controls_sensitivity,
    rd_controls_empty2,
    rd_controls_acceleration,
    rd_controls_empty3,
    rd_controls_threshold,
    rd_controls_empty4,
    rd_controls_mouselook,
    rd_controls_novert,
    rd_controls_inverty,
    rd_controls_empty5,
    
    rd_controls_end
} rd_controls_e;

// ------------
// English menu
// ------------

menuitem_t RD_Controls_Menu[]=
{
    {1, "keyboard bindings",  M_RD_Choose_Bindings_1,     'k'},
    {1, "mouse bindings",     M_RD_Choose_Mouse_Bindings, 'm'},
    {-1,"",0,'\0'},
    {2, "sensivity",          M_RD_Change_Sensitivity,    'm'},
    {-1,"",0,'\0'},
    {2, "acceleration",       M_RD_Change_Acceleration,   'a'},
    {-1,"",0,'\0'},
    {2, "acceleration threshold", M_RD_Change_Threshold,  't'},
    {-1,"",0,'\0'},
    {2, "mouse look:",        M_RD_Change_MouseLook,      'm'},
    {2, "invert y axis:",     M_RD_Change_InvertY,        'i'},
    {2, "vertical movement:", M_RD_Change_Novert,         'v'},
    {-1,"",0,'\0'}

};

menu_t  RD_Controls_Def =
{
    rd_controls_end,
    &RD_Options_Def,
    RD_Controls_Menu,
    M_RD_Draw_Controls,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Controls_Menu_Rus[]=
{
    {1, "yfcnhjqrb rkfdbfnehs",      M_RD_Choose_Bindings_1,  'k'}, // Настройки клавиатуры
    {1, "yfcnhjqrb vsib",            M_RD_Choose_Mouse_Bindings,'k'}, // Настройки мыши
    {-1,"",0,'\0'},                                                 //
    {2, "crjhjcnm",                  M_RD_Change_Sensitivity, 'c'}, // Скорость
    {-1,"",0,'\0'},                                                 //
    {2, "frctkthfwbz",               M_RD_Change_Acceleration,'f'}, // Акселерация
    {-1,"",0,'\0'},
    {2, "gjhju frctkthfwbb",         M_RD_Change_Threshold,   'g'}, // Порог акселерации
    {-1,"",0,'\0'},
    {2, "j,pjh vsim.:",              M_RD_Change_MouseLook,   'j'}, // Обзор мышью
    {2, "dthnbrfkmyfz bydthcbz:",    M_RD_Change_InvertY,     'd'}, // Вертикальная инверсия
    {2, "dthnbrfkmyjt gthtvtotybt:", M_RD_Change_Novert,      'd'}, // Вертикальное перемещение
    {-1,"",0,'\0'}
};

menu_t  RD_Controls_Def_Rus =
{
    rd_controls_end,
    &RD_Options_Def_Rus,
    RD_Controls_Menu_Rus,
    M_RD_Draw_Controls,
    35,35,
    0
};


// -----------------------------------------------------------------------------
// Key bindings (1)
// -----------------------------------------------------------------------------

subtitle_t RD_Bindings_1_subtitles[] = {
    {25, "Movement", "ldb;tybt"},
    {115, "Action", "ltqcndbt"}
};

key_page_t RD_Bindings_1 = {
     &RD_Controls_Def,
     &RD_Controls_Def_Rus,
     M_RD_Choose_Bindings_4,
     M_RD_Choose_Bindings_2,
     'l', 'y', 'n', 'l',
     "page 1/4", "cnhfybwf 1*4",
     RD_Bindings_1_subtitles,
     2,
     {bk_forward,
    bk_backward,
    bk_turn_left,
    bk_turn_right,
    bk_strafe_left,
    bk_strafe_right,
    bk_speed,
    bk_strafe,
    bk_null,
    bk_fire,
    bk_use}
};

menu_t* RD_Bindings_Menu_Def_1;
menu_t* RD_Bindings_Menu_Def_1_Rus;

// -----------------------------------------------------------------------------
// Key bindings (2)
// -----------------------------------------------------------------------------

subtitle_t RD_Bindings_2_subtitles[] = {
    {25, "Weapons", "jhe;bt"}
};

key_page_t RD_Bindings_2 = {
    &RD_Controls_Def,
    &RD_Controls_Def_Rus,
    M_RD_Choose_Bindings_1,
    M_RD_Choose_Bindings_3,
    'p', 'y', 'n', 'l',
    "page 2/4", "cnhfybwf 2*4",
    RD_Bindings_2_subtitles,
    1,
    {bk_weapon_1,
     bk_weapon_2,
     bk_weapon_3,
     bk_weapon_4,
     bk_weapon_5,
     bk_weapon_6,
     bk_weapon_7,
     bk_weapon_8,
     bk_weapon_prev,
     bk_weapon_next,
     bk_null}
};

menu_t* RD_Bindings_Menu_Def_2;
menu_t*  RD_Bindings_Menu_Def_2_Rus;

// -----------------------------------------------------------------------------
// Key bindings (3)
// -----------------------------------------------------------------------------

subtitle_t RD_Bindings_3_subtitles[] = {
    {25, "Shortcut keys", ",scnhsq ljcneg"},
    {95, "Toggleables", "gthtrk.xtybt"}
};

key_page_t RD_Bindings_3 = {
    &RD_Controls_Def,
    &RD_Controls_Def_Rus,
    M_RD_Choose_Bindings_2,
    M_RD_Choose_Bindings_4,
    'p', 'y', 'n', 'l',
    "page 3/4", "cnhfybwf 3*4",
    RD_Bindings_3_subtitles,
    2,
    {bk_save,
     bk_load,
     bk_nextlevel,
     bk_reloadlevel,
     bk_screenshot,
     bk_finish_demo,
     bk_null,
     bk_toggle_mlook,
     bk_toggle_autorun,
     bk_toggle_crosshair,
     bk_toggle_fliplvls}
};

menu_t*  RD_Bindings_Menu_Def_3;
menu_t*  RD_Bindings_Menu_Def_3_Rus;

// -----------------------------------------------------------------------------
// Key bindings (4)
// -----------------------------------------------------------------------------

subtitle_t RD_Bindings_4_subtitles[] = {
    {25, "Automap", "rfhnf"}
};

key_page_t RD_Bindings_4 = {
    &RD_Controls_Def,
    &RD_Controls_Def_Rus,
    M_RD_Choose_Bindings_3,
    M_RD_Choose_Bindings_1,
    'p', 'y', 'f', 'l',
    "page 4/4", "cnhfybwf 4*4",
    RD_Bindings_4_subtitles,
    1,
    {bk_map_toggle,
     bk_map_zoom_in,
     bk_map_zoom_out,
     bk_map_zoom_max,
     bk_map_follow,
     bk_map_overlay,
     bk_map_rotate,
     bk_map_grid,
     bk_map_mark,
     bk_map_clearmark,
     bk_null}
};

menu_t*  RD_Bindings_Menu_Def_4;
menu_t*  RD_Bindings_Menu_Def_4_Rus;


// -----------------------------------------------------------------------------
// Mouse bindings
// -----------------------------------------------------------------------------

enum
{
    rd_mouse_bindings_attack,
    rd_mouse_bindings_use,
    rd_mouse_bindings_forward,
    rd_mouse_bindings_backward,
    rd_mouse_bindings_strafeon,
    rd_mouse_bindings_strafeleft,
    rd_mouse_bindings_straferight,
    rd_mouse_bindings_prevweapon,
    rd_mouse_bindings_nextweapon,
    rd_mouse_bindings_end
} rd_mouse_bindings_e;

// ------------
// English menu
// ------------

menuitem_t RD_Mouse_Bindings_Menu[]=
{
    {1, "Fire/Attack",     M_RD_Mouse_Bind_FireAttack,   'f'},
    {1, "Use",             M_RD_Mouse_Bind_Use,          'u'},
    {1, "Move Forward",    M_RD_Mouse_Bind_MoveForward,  'm'},
    {1, "Move Backward",   M_RD_Mouse_Bind_MoveBackward, 'm'},
    {1, "Strafe On",       M_RD_Mouse_Bind_StrafeOn,     's'},
    {1, "Strafe Left",     M_RD_Mouse_Bind_StrafeLeft,   's'},
    {1, "Strafe Right",    M_RD_Mouse_Bind_StrafeRight,  's'},
    {1, "Previous Weapon", M_RD_Mouse_Bind_PrevWeapon,   't'},
    {1, "Next Weapon",     M_RD_Mouse_Bind_NextWeapon,   't'},
    {-1,"",0,'\0'}
};

menu_t  RD_Mouse_Bindings_Menu_Def =
{
    rd_mouse_bindings_end,
    &RD_Controls_Def,
    RD_Mouse_Bindings_Menu,
    M_RD_Draw_Mouse_Bindings,
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Mouse_Bindings_Menu_Rus[]=
{
    {1, "fnfrf*cnhtkm,f",    M_RD_Mouse_Bind_FireAttack,   'f'}, // Атака/стрельба
    {1, "bcgjkmpjdfnm",      M_RD_Mouse_Bind_Use,          'b'}, // Использовать
    {1, "ldb;tybt dgthtl",   M_RD_Mouse_Bind_MoveForward,  'l'}, // Движение вперед
    {1, "ldb;tybt yfpfl",    M_RD_Mouse_Bind_MoveBackward, 'l'}, // Движение назад
    {1, "ldb;tybt ,jrjv",    M_RD_Mouse_Bind_StrafeOn,     'l'}, // Движение боком
    {1, ",jrjv dktdj",       M_RD_Mouse_Bind_StrafeLeft,   ','}, // Боком влево
    {1, ",jrjv dghfdj",      M_RD_Mouse_Bind_StrafeRight,  ','}, // Боком вправо
    {1, "ghtlsleott jhe;bt", M_RD_Mouse_Bind_PrevWeapon,   'g'}, // Предыдущее оружие
    {1, "cktle.ott jhe;bt",  M_RD_Mouse_Bind_NextWeapon,   'c'}, // Следующее оружие
    {-1,"",0,'\0'}
};

menu_t  RD_Mouse_Bindings_Menu_Def_Rus =
{
    rd_mouse_bindings_end,
    &RD_Controls_Def_Rus,
    RD_Mouse_Bindings_Menu_Rus,
    M_RD_Draw_Mouse_Bindings,
    35,35,
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
    rd_gameplay_1_empty3,
    rd_gameplay_1_next_page,
    rd_gameplay_1_last_page,
    rd_gameplay_1_end
} rd_gameplay_1_e;

enum
{
    rd_gameplay_2_z_axis_sfx,
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
    rd_gameplay_3_toss_drop,
    rd_gameplay_3_empty1,
    rd_gameplay_3_crosshair_draw,
    rd_gameplay_3_crosshair_type,
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
    rd_gameplay_4_pistol_start,
    rd_gameplay_4_empty1,
    rd_gameplay_4_demotimer,
    rd_gameplay_4_demotimerdir,
    rd_gameplay_4_demobar,
    rd_gameplay_4_no_internal_demos,
    rd_gameplay_4_first_page,
    rd_gameplay_4_prev_page,
    rd_gameplay_4_end
} rd_gameplay_4_e;

// ------------
// English menu
// ------------

menuitem_t RD_Gameplay_Menu_1[]=
{
    {2,"Brightmaps:",                  M_RD_Change_Brightmaps,     'b'},
    {2,"Fake contrast:",               M_RD_Change_FakeContrast,   'f'},
    {2,"Translucency:",                M_RD_Change_Translucency,   't'},
    {2,"Fuzz effect:",                 M_RD_Change_ImprovedFuzz,   'f'},
    {2,"Colored HUD elements:",        M_RD_Change_ColoredHUD,     'c'},
    {2,"Colored blood and corpses:",   M_RD_Change_ColoredBlood,   'c'},
    {2,"Swirling liquids:",            M_RD_Change_SwirlingLiquids,'s'},
    {2,"Invulnerability affects sky:", M_RD_Change_InvulSky,       'i'},
    {2,"Flip weapons:",                M_RD_Change_FlipWeapons,    'f'},
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_2[]=
{
    {2,"Sound attenuation axises:",       M_RD_Change_ZAxisSfx,        's'},
    {2,"Play exit sounds:",               M_RD_Change_ExitSfx,         'p'},
    {2,"Sound of crushing corpses:",      M_RD_Change_CrushingSfx,     's'},
    {2,"Single sound of blazing door:",   M_RD_Change_BlazingSfx,      's'},
    {2,"Monster alert waking up others:", M_RD_Change_AlertSfx,        'm'},
    {-1,"",0,'\0'},
    {2,"Notify of revealed secrets:",     M_RD_Change_SecretNotify,    'n'},
    {2,"Show negative health:",           M_RD_Change_NegativeHealth,  's'},
    {2,"Infragreen light amp. visor:",    M_RD_Change_InfraGreenVisor, 'i'},
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_3[]=
{
    {2,"Walk over and under monsters:",       M_RD_Change_WalkOverUnder,   'w'},
    {2,"Corpses sliding from the ledges:",    M_RD_Change_Torque,          'c'},
    {2,"Weapon bobbing while firing:",        M_RD_Change_Bobbing,         'w'},
    {2,"Lethal pellet of a point-blank SSG:", M_RD_Change_SSGBlast,        'l'},
    {2,"Randomly mirrored corpses:",          M_RD_Change_FlipCorpses,     'r'},
    {2,"Floating powerups:",                  M_RD_Change_FloatPowerups,   'f'},
    {2,"Items are tossed when dropped:",      M_RD_Change_TossDrop,        'i'},
    {-1,"",0,'\0'},
    {2,"Draw crosshair:",                     M_RD_Change_CrosshairDraw,   'd'},
    {2,"Indication:",                         M_RD_Change_CrosshairType,   'i'},
    {2,"Increased size:",                     M_RD_Change_CrosshairScale,  'i'},
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_4[]=
{
    {2,"Fix errors of vanilla maps:",         M_RD_Change_FixMapErrors,     'f'},
    {2,"Flip game levels:",                   M_RD_Change_FlipLevels,       'f'},
    {2,"Extra player faces on the HUD:",      M_RD_Change_ExtraPlayerFaces, 'e'},
    {2,"Pain Elemental without Souls limit:", M_RD_Change_LostSoulsQty,     'p'},
    {2,"More aggressive lost souls:",         M_RD_Change_LostSoulsAgr,     'm'},
    {2,"Pistol start game mode:",             M_RD_Change_PistolStart,      's'},
    {-1,"",0,'\0'},
    {2,"Show demo timer:",                    M_RD_Change_DemoTimer,        's'},
    {2,"timer direction:",                    M_RD_Change_DemoTimerDir,     't'},
    {2,"Show progress bar:",                  M_RD_Change_DemoBar,          's'},
    {2,"Play internal demos:",                M_RD_Change_NoInternalDemos,  'p'},
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
    35,35,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Gameplay_Menu_1_Rus[]=
{
    {2,",hfqnvfggbyu:",                     M_RD_Change_Brightmaps,     ','},   // Брайтмаппинг
    {2,"Bvbnfwbz rjynhfcnyjcnb:",           M_RD_Change_FakeContrast,   'b'},   // Имитация контрастности
    {2,"Ghjphfxyjcnm j,]trnjd:",            M_RD_Change_Translucency,   'g'},   // Прозрачность объектов
    {2,"\'aatrn ievf:",                     M_RD_Change_ImprovedFuzz,   '\''},  // Эффект шума
    {2,"Hfpyjwdtnyst 'ktvtyns $:",          M_RD_Change_ColoredHUD,     'h'},   // Разноцветные элементы HUD
    {2,"Hfpyjwdtnyfz rhjdm b nhegs:",       M_RD_Change_ColoredBlood,   'h'},   // Разноцветная кровь и трупы
    {2,"ekexityyfz fybvfwbz ;blrjcntq:",    M_RD_Change_SwirlingLiquids,'e'},   // Улучшенная анимация жидкостей
    {2,"ytezpdbvjcnm jrhfibdftn yt,j:",     M_RD_Change_InvulSky,       'y'},   // Неуязвимость окрашивает небо
    {2,"pthrfkmyjt jnhf;tybt jhe;bz:",      M_RD_Change_FlipWeapons,    'p'},   // Зеркальное отражение оружия
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_2_Rus[]=
{
    {2,"pfne[fybt pderf gj jczv:",          M_RD_Change_ZAxisSfx,           'p'},   // Затухание звука по осям
    {2,"Pderb ghb ds[jlt bp buhs:",         M_RD_Change_ExitSfx,            'p'},   // Звук при выходе из игры
    {2,"Pder hfplfdkbdfybz nhegjd:",        M_RD_Change_CrushingSfx,        'p'},   // Звук раздавливания трупов
    {2,"Jlbyjxysq pder ,scnhjq ldthb:",     M_RD_Change_BlazingSfx,         'j'},   // Одиночный звук быстрой двери
    {2,"J,ofz nhtdjuf e vjycnhjd:",         M_RD_Change_AlertSfx,           'j'},   // Общая тревога у монстров
    {-1,"",0,'\0'},                                                                 //
    {2,"Cjj,ofnm j yfqltyyjv nfqybrt:",     M_RD_Change_SecretNotify,       'c'},   // Сообщать о найденном тайнике
    {2,"jnhbwfntkmyjt pljhjdmt d $:",       M_RD_Change_NegativeHealth,     'j'},   // Отрицательное здоровье в HUD
    {2,"Byahfptktysq dbpjh jcdtotybz:",     M_RD_Change_InfraGreenVisor,    'b'},   // Инфразеленый визор освещения
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_3_Rus[]=
{
    {2,"Gthtvtotybt gjl*yfl vjycnhfvb:",    M_RD_Change_WalkOverUnder,      'g'},   // Перемещение над/под монстрами
    {2,"Nhegs cgjkpf.n c djpdsitybq:",      M_RD_Change_Torque,             'n'},   // Трупы сползают с возвышений
    {2,"Ekexityyjt gjrfxbdfybt jhe;bz:",    M_RD_Change_Bobbing,            'e'},   // Улучшенное покачивание оружия
    {2,"ldecndjkrf hfphsdftn dhfujd:",      M_RD_Change_SSGBlast,           'l'},   // Двустволка разрывает врагов
    {2,"pthrfkbhjdfybt nhegjd:",            M_RD_Change_FlipCorpses,        'p'},   // Зеркалирование трупов
    {2,"Ktdbnbhe.obt caths-fhntafrns:",     M_RD_Change_FloatPowerups,      'k'},   // Левитирующие сферы-артефакты
    {2,"Gjl,hfcsdfnm dsgfdibt ghtlvtns:",   M_RD_Change_TossDrop,           'g'},   // Подбрасывать выпавшие предметы
    {-1,"",0,'\0'},                                                                 //
    {2,"Jnj,hf;fnm ghbwtk:",                M_RD_Change_CrosshairDraw,      'j'},   // Отображать прицел
    {2,"Bylbrfwbz:",                        M_RD_Change_CrosshairType,      'b'},   // Индикация
    {2,"Edtkbxtyysq hfpvth:",               M_RD_Change_CrosshairScale,     'e'},   // Увеличенный размер
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
    35,35,
    0
};

menuitem_t RD_Gameplay_Menu_4_Rus[]=
{
    {2,"ecnhfyznm jib,rb jhbu> ehjdytq:",   M_RD_Change_FixMapErrors,       'e'},   // Устранять ошибки ориг. уровней
    {2,"pthrfkmyjt jnhf;tybt ehjdytq:",     M_RD_Change_FlipLevels,         'p'},   // Зеркальное отражение уровней
    {2,"Ljgjkybntkmyst kbwf buhjrf:",       M_RD_Change_ExtraPlayerFaces,   'l'},   // Дополнительные лица игрока
    {2,"'ktvtynfkm ,tp juhfybxtybz lei:",   M_RD_Change_LostSoulsQty,       '\''},  // Элементаль без ограничения душ
    {2,"gjdsityyfz fuhtccbdyjcnm lei:",     M_RD_Change_LostSoulsAgr,       'g'},   // Повышенная агрессивность душ
    {2,"", /* [JN] Joint EN/RU string */    M_RD_Change_PistolStart,        'y'},   // Режим игры "Pistol start"
    {-1,"",0,'\0'},     
    {2,"jnj,hf;fnm nfqvth:",                M_RD_Change_DemoTimer,          's'},   // Отображать таймер
    {2,"dhtvz nfqvthf:",                    M_RD_Change_DemoTimerDir,       's'},   // Время таймера
    {2,"irfkf ghjuhtccf:",                  M_RD_Change_DemoBar,            'g'},   // Шкала прогресса
    {2,"Ghjbuhsdfnm ltvjpfgbcb:",           M_RD_Change_NoInternalDemos,    'g'},   // Проигрывать демозаписи
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
    35,35,
    0
};

// -----------------------------------------------------------------------------
// Level select
// -----------------------------------------------------------------------------

enum
{
    rd_level_1_1,
    rd_level_1_2,
    rd_level_1_3,
    rd_level_1_4,
    rd_level_1_5,
    rd_level_1_6,
    rd_level_1_7,
    rd_level_1_8,
    rd_level_1_9,
    rd_level_1_10,
    rd_level_1_11,
    rd_level_1_12,
    rd_level_1_13,
    rd_level_1_14,
    rd_level_1_15,
    rd_level_1_16,
    rd_level_1_17,
    rd_level_1_18,
    rd_level_1_end
} rd_level_1_e;

// ------------
// English menu
// ------------

menuitem_t RD_Level_Menu_1[]=
{
    {2, "skill level",      M_RD_Change_Selective_Skill,        's'},
    {2, "episode",          M_RD_Change_Selective_Episode,      'e'},
    {2, "map",              M_RD_Change_Selective_Map,          'm'},
    {-1,"",0,'\0'},         // Player
    {2, "health",           M_RD_Change_Selective_Health,       'h'},
    {2, "armor",            M_RD_Change_Selective_Armor,        'a'},
    {2, "armor type",       M_RD_Change_Selective_ArmorType,    'a'},
    {-1,"",0,'\0'},         // Weapons
    {2, "chainsaw",         M_RD_Change_Selective_WP_Chainsaw,  'c'},
    {2, "shotgun",          M_RD_Change_Selective_WP_Shotgun,   's'},
    {2, "super shotgun",    M_RD_Change_Selective_WP_SSgun,     's'},
    {2, "chaingun",         M_RD_Change_Selective_WP_Chaingun,  'c'},
    {2, "rocket launcher",  M_RD_Change_Selective_WP_RLauncher, 'r'},
    {2, "plasmagun",        M_RD_Change_Selective_WP_Plasmagun, 'p'},
    {2, "bfg9000",          M_RD_Change_Selective_WP_BFG9000,   'b'},
    {-1,"",0,'\0'},
    {1, "",                 M_RD_Choose_LevelSelect_2,          'n'},
    {1, "",                 G_DoSelectiveGame,                  's'},
    {-1,"",0,'\0'}
};

menu_t RD_Level_Def_1 =
{
    rd_level_1_end,
    &RD_Options_Def,
    RD_Level_Menu_1,
    M_RD_Draw_Level_1,
    75,20,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Level_Menu_1_Rus[]=
{
    {2, "ckj;yjcnm",        M_RD_Change_Selective_Skill,        's'}, // Сложность
    {2, "\'gbpjl",          M_RD_Change_Selective_Episode,      'e'}, // Эпизод
    {2, "ehjdtym",          M_RD_Change_Selective_Map,          'm'}, // Уровень
    {-1,"",0,'\0'},         // Игрок
    {2, "pljhjdmt",         M_RD_Change_Selective_Health,       'h'}, // Здоровье
    {2, ",hjyz",            M_RD_Change_Selective_Armor,        'a'}, // Броня
    {2, "nbg ,hjyb",        M_RD_Change_Selective_ArmorType,    'a'}, // Тип брони
    {-1,"",0,'\0'},         // Оружие
    {2, ",typjgbkf",        M_RD_Change_Selective_WP_Chainsaw,  'c'}, // Бензопила
    {2, "he;mt",            M_RD_Change_Selective_WP_Shotgun,   's'}, // Ружье
    {2, "ldecndjkmyjt he;mt",M_RD_Change_Selective_WP_SSgun,    's'}, // Двуствольное ружье
    {2, "gektvtn",          M_RD_Change_Selective_WP_Chaingun,  'c'}, // Пулемет
    {2, "hfrtnybwf",        M_RD_Change_Selective_WP_RLauncher, 'r'}, // Ракетница
    {2, "gkfpvtyyfz geirf", M_RD_Change_Selective_WP_Plasmagun, 'p'}, // Плазменная пушка
    {2, "&9000",            M_RD_Change_Selective_WP_BFG9000,   'b'}, // BFG9000
    {-1,"",0,'\0'},
    {1, "",                 M_RD_Choose_LevelSelect_2,          'n'},
    {1, "",                 G_DoSelectiveGame,                  's'},
    {-1,"",0,'\0'}
};

menu_t RD_Level_Def_1_Rus =
{
    rd_level_1_end,
    &RD_Options_Def_Rus,
    RD_Level_Menu_1_Rus,
    M_RD_Draw_Level_1,
    72,20,
    0
};

enum
{
    rd_level_2_1,
    rd_level_2_2,
    rd_level_2_3,
    rd_level_2_4,
    rd_level_2_5,
    rd_level_2_6,
    rd_level_2_7,
    rd_level_2_8,
    rd_level_2_9,
    rd_level_2_10,
    rd_level_2_11,
    rd_level_2_12,
    rd_level_2_13,
    rd_level_2_14,
    rd_level_2_15,
    rd_level_2_16,
    rd_level_2_17,
    rd_level_2_18,
    rd_level_2_end
} rd_level_2_e;

// ------------
// English menu
// ------------

menuitem_t RD_Level_Menu_2[]=
{
    {2, "backpack",         M_RD_Change_Selective_Backpack,     'b'},
    {2, "bullets",          M_RD_Change_Selective_Ammo_0,       'b'},
    {2, "shells",           M_RD_Change_Selective_Ammo_1,       's'},
    {2, "rockets",          M_RD_Change_Selective_Ammo_3,       'r'},
    {2, "cells",            M_RD_Change_Selective_Ammo_2,       'c'},
    {-1,"",0,'\0'},         // Keys
    {2, "blue keycard",     M_RD_Change_Selective_Key_0,        'b'},
    {2, "yellow keycard",   M_RD_Change_Selective_Key_1,        'y'},
    {2, "red keycard",      M_RD_Change_Selective_Key_2,        'r'},
    {2, "blue skull key",   M_RD_Change_Selective_Key_3,        'b'},
    {2, "yellow skull key", M_RD_Change_Selective_Key_4,        'y'},
    {2, "red skull key",    M_RD_Change_Selective_Key_5,        'r'},
    {-1,"",0,'\0'},         // Extra
    {2, "fast monsters",    M_RD_Change_Selective_Fast,         'f'},
    {2, "respawning monsters",M_RD_Change_Selective_Respawn,    'r'},
    {-1,"",0,'\0'},
    {1, "",                 M_RD_Choose_LevelSelect_1,          'p'},
    {1, "",                 G_DoSelectiveGame,                  's'},
    {-1,"",0,'\0'}
};

menu_t RD_Level_Def_2 =
{
    rd_level_2_end,
    &RD_Options_Def,
    RD_Level_Menu_2,
    M_RD_Draw_Level_2,
    75,20,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Level_Menu_2_Rus[]=
{
    {2, "h.rpfr",               M_RD_Change_Selective_Backpack, 'b'}, // Рюкзак
    {2, "gekb",                 M_RD_Change_Selective_Ammo_0,   'b'}, // Пули
    {2, "lhj,m",                M_RD_Change_Selective_Ammo_1,   's'}, // Дробь
    {2, "hfrtns",               M_RD_Change_Selective_Ammo_3,   'r'}, // Ракеты
    {2, "\'ythubz",             M_RD_Change_Selective_Ammo_2,   'c'}, // Энергия
    {-1,"",0,'\0'},             // Ключи
    {2, "cbyzz rk.x-rfhnf",     M_RD_Change_Selective_Key_0,    'b'}, // Синяя ключ-карта
    {2, ";tknfz rk.x-rfhnf",    M_RD_Change_Selective_Key_1,    'y'}, // Желтая ключ-карта
    {2, "rhfcyfz rk.x-rfhnf",   M_RD_Change_Selective_Key_2,    'r'}, // Красная ключ-карта
    {2, "cbybq rk.x-xthtg",     M_RD_Change_Selective_Key_3,    'b'}, // Синий ключ-череп
    {2, ";tknsq rk.x-xthtg",    M_RD_Change_Selective_Key_4,    'y'}, // Желтый ключ-череп
    {2, "rhfcysq rk.x-xthtg",   M_RD_Change_Selective_Key_5,    'r'}, // Красный ключ-череп
    {-1,"",0,'\0'},             // Монстры
    {2, "ecrjhtyyst",           M_RD_Change_Selective_Fast,     'f'}, // Ускоренные
    {2, "djcrhtif.obtcz",       M_RD_Change_Selective_Respawn,  'r'}, // Воскрешающиеся
    {-1,"",0,'\0'},
    {1, "",                     M_RD_Choose_LevelSelect_1,      'p'},
    {1, "",                     G_DoSelectiveGame,              's'},
    {-1,"",0,'\0'}
};

menu_t RD_Level_Def_2_Rus =
{
    rd_level_2_end,
    &RD_Options_Def_Rus,
    RD_Level_Menu_2_Rus,
    M_RD_Draw_Level_2,
    72,20,
    0
};


// -----------------------------------------------------------------------------
// Reset settings
// -----------------------------------------------------------------------------

enum
{
    rd_reset_recommended,
    rd_reset_vanilla,
    rd_reset_end
} rd_reset_e;

// ------------
// English menu
// ------------

menuitem_t RD_Reset_Menu[]=
{
    {1, "Recommended", M_RD_BackToDefaults_Recommended, 'r'},
    {1, "Original",    M_RD_BackToDefaults_Original,    'o'},
    {-1,"",0,'\0'}
};

menu_t  RD_Reset_Def =
{
    rd_reset_end, 
    &RD_Options_Def,
    RD_Reset_Menu,
    M_RD_Draw_Reset,
    115, 95,
    0
};

// ------------
// Russian menu
// ------------

menuitem_t RD_Reset_Menu_Rus[]=
{
    {1, "Htrjvtyljdfyysq", M_RD_BackToDefaults_Recommended, 'h'}, // Рекомендованный
    {1, "Jhbubyfkmysq",    M_RD_BackToDefaults_Original,    'j'}, // Оригинальный
    {-1,"",0,'\0'}
};

menu_t  RD_Reset_Def_Rus =
{
    rd_reset_end, 
    &RD_Options_Def_Rus,
    RD_Reset_Menu_Rus,
    M_RD_Draw_Reset,
    98, 95,
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
    // - "OPTIONS" title -------------------------------------------------------
    V_DrawPatch(108 + wide_delta, 15, 
                W_CacheLumpName(DEH_String("M_OPTTTL"), PU_CACHE));

    // - Messages --------------------------------------------------------------
    V_DrawPatch(179 + wide_delta, 53, 
                W_CacheLumpName(DEH_String(showMessages == 1 ?
                                           "M_MSGON" : "M_MSGOFF"), PU_CACHE));

    // - Graphic Detail --------------------------------------------------------
    V_DrawPatch(235 + wide_delta, 69, 
                W_CacheLumpName(DEH_String(detailLevel == 1 ?
                                           "M_GDLOW" : "M_GDHIGH"), PU_CACHE));
    }
    else
    {
    // - "НАСТРОЙКИ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(15, "YFCNHJQRB");
    
    M_WriteTextBig_RUS(206 + wide_delta, 53, 
                       showMessages == 1 ? "drk/" : "dsrk/");

    M_WriteTextBig_RUS(224 + wide_delta, 69, 
                       detailLevel == 1 ? "ybp/" : "dsc/");
    }

    // - Screen size slider ----------------------------------------------------
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        M_DrawThermo(60 + wide_delta, 102, 6, screenSize);
    }
    else
    {
        M_DrawThermo(60 + wide_delta, 102, 12, screenSize);
    }

    // - Mouse sensivity slider ------------------------------------------------
    M_DrawThermo(60 + wide_delta, 134, 10, mouseSensitivity);
}

void M_Vanilla_DrawSound(void)
{
    if (english_language)
    {
    // - "Sound volume" title --------------------------------------------------
    V_DrawPatch (60 + wide_delta, 38, 
                 W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));
    }
    else
    {
    // - "ГРОМКОСТЬ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(38, "UHJVRJCNM");
    }

    // - Sfx volume slider -----------------------------------------------------
    M_DrawThermo(80 + wide_delta, 81, 16, sfxVolume);

    // - Music volume slider ---------------------------------------------------
    M_DrawThermo(80 + wide_delta, 113, 16, musicVolume);
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
        M_WriteTextBigCentered_ENG(5, "OPTIONS");
    }
    else
    {
        // НАСТРОЙКИ
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB");            
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
        M_WriteTextBigCentered_ENG(5, "RENDERING OPTIONS");

        //
        // Rendering
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Rendering");
        dp_translation = NULL;

        // Widescreen rendering
        M_WriteTextSmall_ENG(185 + wide_delta, 35, aspect_ratio_temp == 1 ? "5:4" :
                                                   aspect_ratio_temp == 2 ? "16:9" :
                                                   aspect_ratio_temp == 3 ? "16:10" :
                                                   aspect_ratio_temp == 4 ? "21:9" :
                                                                            "4:3");
        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY];
            M_WriteTextSmallCentered_ENG(156, "Program must be restarted");
            dp_translation = NULL;
        }

        // Vertical synchronization
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(216 + wide_delta, 45, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(216 + wide_delta, 45, vsync ? "on" : "off");
        }

        // Frame rate
        M_WriteTextSmall_ENG(120 + wide_delta, 55, uncapped_fps ? "uncapped" : "35 fps");

        // Show FPS counter
        M_WriteTextSmall_ENG(162 + wide_delta, 65, show_fps ? "on" : "off");

        // Pixel scaling
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(135 + wide_delta, 75, "n/a");
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_ENG(135 + wide_delta, 75, smoothing ? "smooth" : "sharp");
        }

        // Porch palette changing
        M_WriteTextSmall_ENG(207 + wide_delta, 85, vga_porch_flash ? "on" : "off");

        // Video renderer
        M_WriteTextSmall_ENG(146 + wide_delta, 95, force_software_renderer ? "software (cpu)" : "hardware (gpu)");

        //
        // Extra
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 105, "Extra");
        dp_translation = NULL;

        // Show disk icon
        M_WriteTextSmall_ENG(138 + wide_delta, 115, show_diskicon ? "on" : "off");

        // Screen wiping effect
        M_WriteTextSmall_ENG(187 + wide_delta, 125, screen_wiping == 1 ? "standard" :
                                                    screen_wiping == 2 ? "loading" :
                                                                         "off");

        // Screenshot format
        M_WriteTextSmall_ENG(174 + wide_delta, 135, png_screenshots ? "png" : "pcx");

        // Show ENDOOM screen
        M_WriteTextSmall_ENG(179 + wide_delta, 145, show_endoom ? "on" : "off");
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB DBLTJ"); // НАСТРОЙКИ ВИДЕО

        //
        // Рендеринг
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "htylthbyu");
        dp_translation = NULL;

        // Широкоформатный режим
        M_WriteTextSmall_RUS(238 + wide_delta, 35, aspect_ratio_temp == 1 ? "5:4" :
                                                   aspect_ratio_temp == 2 ? "16:9" :
                                                   aspect_ratio_temp == 3 ? "16:10" :
                                                   aspect_ratio_temp == 4 ? "21:9" :
                                                                            "4:3");

        // Informative message: Необходим перезапуск программы
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_GRAY];
            M_WriteTextSmallCentered_RUS(156, "ytj,[jlbv gthtpfgecr ghjuhfvvs");
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(249 + wide_delta, 45, "y*l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(249 + wide_delta, 45, vsync ? "drk" : "dsrk");
        }

        // Кадровая частота
        if (uncapped_fps)
        {
            M_WriteTextSmall_RUS(167 + wide_delta, 55, ",tp juhfybxtybz");
        }
        else
        {
            M_WriteTextSmall_ENG(167 + wide_delta, 55, "35 fps");
        }

        // Счетчик кадровой частоты
        M_WriteTextSmall_RUS(227 + wide_delta, 65, show_fps ? "drk" : "dsrk");

        // Пиксельное сглаживание
        if (force_software_renderer == 1)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(219 + wide_delta, 75, "y*l"); // Н/Д
            dp_translation = NULL;
        }
        else
        {
            M_WriteTextSmall_RUS(219 + wide_delta, 75, smoothing ? "drk" : "dsrk");
        }

        // Изменение палитры краёв экрана
        M_WriteTextSmall_RUS(274 + wide_delta, 85, vga_porch_flash ? "drk" : "dsrk");

        // Обработка видео
        M_WriteTextSmall_RUS(160 + wide_delta, 95, force_software_renderer ? "ghjuhfvvyfz" : "fggfhfnyfz");
        M_WriteTextSmall_ENG((force_software_renderer ? 254 : 244) + wide_delta, 95, 
                              force_software_renderer ? "(cpu)" : "(gpu)");

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 105, "ljgjkybntkmyj");
        dp_translation = NULL;

        // Отображать значок дискеты
        M_WriteTextSmall_RUS(241 + wide_delta, 115, show_diskicon ? "drk" : "dsrk");

        // Эффект смены экранов
        M_WriteTextSmall_RUS(202 + wide_delta, 125, screen_wiping == 1 ? "cnfylfhnysq" :
                                                    screen_wiping == 2 ? "pfuheprf" :
                                                                         "dsrk");

        // Формат скриншотов
        M_WriteTextSmall_ENG(180 + wide_delta, 135, png_screenshots ? "png" : "pcx");

        // Показывать экран ENDOOM
        M_WriteTextSmall_ENG(165 + wide_delta, 145, "ENDOOM:");
        M_WriteTextSmall_RUS(222 + wide_delta, 145, show_endoom ? "drk" : "dsrk");
    }
}

void M_SaveDefaults(void);

void M_RD_Change_Widescreen(int choice)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in M_Init and stored into config file in M_QuitResponse.
    switch(choice)
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
        break;
    }
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
    show_fps ^= 1;
}

void M_RD_Change_DiskIcon(int choice)
{
    show_diskicon ^= 1;
}

void M_RD_Change_Smoothing(int choice)
{
    // [JN] Disable "vsync" toggling in sofrware renderer
    if (force_software_renderer == 1)
    return;

    smoothing ^= 1;

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
    switch(choice)
    {
        case 0:
        screen_wiping--;
        if (screen_wiping < 0)
            screen_wiping = 2;
        break;

        case 1:
        screen_wiping++;
        if (screen_wiping > 2)
            screen_wiping = 0;
        break;
    }
}

void M_RD_Change_Screenshots(int choice)
{
    png_screenshots ^= 1;
}

void M_RD_Change_ENDOOM(int choice)
{
    show_endoom ^= 1;
}

void M_RD_Change_Renderer(int choice)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_PorchFlashing(int choice)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
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
    static char num[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "DISPLAY OPTIONS");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Screen");
        dp_translation = NULL;

        // Detail level
        M_WriteTextSmall_ENG(130 + wide_delta, 115, detailLevel == 1 ? "low" : "high");

        // Local time
        M_WriteTextSmall_ENG(116 + wide_delta, 125, 
                             local_time == 1 ? "12-hour (hh:mm)" :
                             local_time == 2 ? "12-hour (hh:mm:ss)" :
                             local_time == 3 ? "24-hour (hh:mm)" :
                             local_time == 4 ? "24-hour (hh:mm:ss)" :
                                               "off");

        //
        // Interface
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 135, "Interface");  
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB \"RHFYF"); // НАСТРОЙКИ ЭКРАНА

        //
        // Экран
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "\'rhfy");
        dp_translation = NULL;

        // Уровень детализации
        M_WriteTextSmall_RUS(193 + wide_delta, 115, detailLevel == 1 ? "ybprbq" : "dscjrbq");

        // Системное время
        M_WriteTextSmall_RUS(161 + wide_delta, 125, 
                             local_time == 1 ? "12-xfcjdjt (xx:vv)" :
                             local_time == 2 ? "12-xfcjdjt (xx:vv:cc)" :
                             local_time == 3 ? "24-xfcjdjt (xx:vv)" :
                             local_time == 4 ? "24-xfcjdjt (xx:vv:cc)" :
                                               "dsrk");

        //
        // Интерфейс
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 135, "bynthatqc");  
        dp_translation = NULL;
    }

    // Screen size slider
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        M_DrawThermo_Small(35 + wide_delta, 44, 6, screenSize);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        M_WriteTextSmall_ENG(96 + wide_delta, 45, num);
    }
    else
    {
        M_DrawThermo_Small(35 + wide_delta, 44, 12, screenSize);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        M_WriteTextSmall_ENG(145 + wide_delta, 45, num);
    }

    // Gamma-correction slider
    M_DrawThermo_Small(35 + wide_delta, 64, 18, usegamma);

    // Level brightness slider
    M_DrawThermo_Small(35 + wide_delta, 84, 5, level_brightness / 16);

    // Level brightness slider
    M_DrawThermo_Small(35 + wide_delta, 104, 7, menu_shading / 4);
}

void M_RD_Change_ScreenSize(int choice)
{
    extern void EnableLoadingDisk();

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

    if (aspect_ratio >= 2)
    {
        // Wide screen: don't allow unsupported views
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
    
        // Reinitialize fps and time widget's horizontal offset
        if (gamestate == GS_LEVEL)
        {
            HU_Start();
        }
        
        EnableLoadingDisk();
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

    I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                                     "PALFIX" :
                                                     "PLAYPAL"),
                                                     PU_CACHE) + 
                                                     st_palette * 768);
    players[consoleplayer].message_system = DEH_String(english_language ? 
                                                       gammamsg[usegamma] :
                                                       gammamsg_rus[usegamma]);
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

void M_RD_Change_MenuShading(int choice)
{
    switch(choice)
    {
        case 0:
        if (menu_shading > 0)
            menu_shading -= 4;
        break;
    
        case 1:
        if (menu_shading < 24)
            menu_shading += 4;
        break;
    }
}

void M_RD_Change_Detail(int choice)
{
    detailLevel ^= 1;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            DETAILHI : DETAILHI_RUS);
    else
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            DETAILLO : DETAILLO_RUS);
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

void M_RD_Choose_MessagesAndTextSettings(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Messages_Def :
                    &RD_Messages_Def_Rus);
}

void M_RD_Draw_MessagesSettings(void)
{
    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "MESSAGES AND TEXTS");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "General");  
        dp_translation = NULL;

        // Messages
        M_WriteTextSmall_ENG(165 + wide_delta, 35, showMessages ? "on" : "off");

        // Message timeout. Print "second" or "seconds", depending of ammount.
        // [JN] Note: using M_StringJoin could be a smarter way,
        // but using it will make a notable delay in drawing routine, so here:
        M_WriteTextSmall_ENG(133 + wide_delta, 55, messages_timeout == 1 ? "1 second" :
                                                   messages_timeout == 2 ? "2 seconds" :
                                                   messages_timeout == 3 ? "3 seconds" :
                                                   messages_timeout == 4 ? "4 seconds" :
                                                   messages_timeout == 5 ? "5 seconds" :
                                                   messages_timeout == 6 ? "6 seconds" :
                                                   messages_timeout == 7 ? "7 seconds" :
                                                   messages_timeout == 8 ? "8 seconds" :
                                                   messages_timeout == 9 ? "9 seconds" :
                                                                           "10 seconds");

        // Text casts shadows
        M_WriteTextSmall_ENG(177 + wide_delta, 65, draw_shadowed_text ? "on" : "off");

        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 75, "Colors");  
        dp_translation = NULL;

        // Item pickup
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(120 + wide_delta, 85, "n/a");
        }
        else
        {
            if (messages_pickup_color == 1)
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "green"); dp_translation = NULL; }
            else if (messages_pickup_color == 2)
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "blue"); dp_translation = NULL; }
            else if (messages_pickup_color == 3)
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "gold"); dp_translation = NULL; }
            else if (messages_pickup_color == 4)
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "gray"); dp_translation = NULL; }
            else if (messages_pickup_color == 5)
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "tan"); dp_translation = NULL; }
            else if (messages_pickup_color == 6)
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "brown"); dp_translation = NULL; }
            else if (messages_pickup_color == 7)
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(120 + wide_delta, 85, "brick"); dp_translation = NULL; }
            else
            { dp_translation = NULL; M_WriteTextSmall_ENG(120 + wide_delta, 85, "red"); }
        }

        // Revealed secret
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(157 + wide_delta, 95, "n/a");
        }
        else
        {
            if (messages_secret_color == 1)
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "green"); dp_translation = NULL; }
            else if (messages_secret_color == 2)
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "blue"); dp_translation = NULL; }
            else if (messages_secret_color == 3)
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "gold"); dp_translation = NULL; }
            else if (messages_secret_color == 4)
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "gray"); dp_translation = NULL; }
            else if (messages_secret_color == 5)
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "tan"); dp_translation = NULL; }
            else if (messages_secret_color == 6)
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "brown"); dp_translation = NULL; }
            else if (messages_secret_color == 7)
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(157 + wide_delta, 95, "brick"); dp_translation = NULL; }
            else
            { dp_translation = NULL; M_WriteTextSmall_ENG(157 + wide_delta, 95, "red"); }
        }

        // System message
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(149 + wide_delta, 105, "n/a");
        }
        else
        {
            if (messages_system_color == 1)
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "green"); dp_translation = NULL; }
            else if (messages_system_color == 2)
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "blue"); dp_translation = NULL; }
            else if (messages_system_color == 3)
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "gold"); dp_translation = NULL; }
            else if (messages_system_color == 4)
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "gray"); dp_translation = NULL; }
            else if (messages_system_color == 5)
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "tan"); dp_translation = NULL; }
            else if (messages_system_color == 6)
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "brown"); dp_translation = NULL; }
            else if (messages_system_color == 7)
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(149 + wide_delta, 105, "brick"); dp_translation = NULL; }
            else
            { dp_translation = NULL; M_WriteTextSmall_ENG(149 + wide_delta, 105, "red"); }
        }

        // Netgame chat
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(136 + wide_delta, 115, "n/a");
        }
        else
        {
            if (messages_chat_color == 1)
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "green"); dp_translation = NULL; }
            else if (messages_chat_color == 2)
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "blue"); dp_translation = NULL; }
            else if (messages_chat_color == 3)
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "gold"); dp_translation = NULL; }
            else if (messages_chat_color == 4)
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "gray"); dp_translation = NULL; }
            else if (messages_chat_color == 5)
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "tan"); dp_translation = NULL; }
            else if (messages_chat_color == 6)
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "brown"); dp_translation = NULL; }
            else if (messages_chat_color == 7)
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_ENG(136 + wide_delta, 115, "brick"); dp_translation = NULL; }
            else
            { dp_translation = NULL; M_WriteTextSmall_ENG(136 + wide_delta, 115, "red"); }
        }
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "CJJ<OTYBZ B NTRCNS"); // СООБЩЕНИЯ И ТЕКСТЫ

        //
        // Общие
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "j,obt");  
        dp_translation = NULL;

        // Отображение сообщений
        M_WriteTextSmall_RUS(214 + wide_delta, 35, showMessages ? "drk" : "dsrk");

        // Таймаут отображения. Печатать секунд(а/ы) в зависимости от количества.
        M_WriteTextSmall_RUS(133 + wide_delta, 55, messages_timeout == 1 ? "1 ctreylf" :
                                                   messages_timeout == 2 ? "2 ctreyls" :
                                                   messages_timeout == 3 ? "3 ctreyls" :
                                                   messages_timeout == 4 ? "4 ctreyls" :
                                                   messages_timeout == 5 ? "5 ctreyl"  :
                                                   messages_timeout == 6 ? "6 ctreyl"  :
                                                   messages_timeout == 7 ? "7 ctreyl"  :
                                                   messages_timeout == 8 ? "8 ctreyl"  :
                                                   messages_timeout == 9 ? "9 ctreyl"  :
                                                                           "10 ctreyl");

        // Тексты отбрасывают тень
        M_WriteTextSmall_RUS(226 + wide_delta, 65, draw_shadowed_text ? "drk" : "dsrk");

        //
        // Цвета
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 75, "wdtnf");  
        dp_translation = NULL;

        // Получение предметов
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(193 + wide_delta, 85, "y*l"); // н/д
        }
        else
        {
            if (messages_pickup_color == 1) // Зеленый
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(193 + wide_delta, 85, "ptktysq"); dp_translation = NULL; }
            else if (messages_pickup_color == 2) // Синий
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(193 + wide_delta, 85, "cbybq"); dp_translation = NULL; }
            else if (messages_pickup_color == 3) // Желтый
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(193 + wide_delta, 85, ";tknsq"); dp_translation = NULL; }
            else if (messages_pickup_color == 4) // Белый
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(193 + wide_delta, 85, ",tksq"); dp_translation = NULL; }
            else if (messages_pickup_color == 5) // Бежевый
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(193 + wide_delta, 85, ",t;tdsq"); dp_translation = NULL; }
            else if (messages_pickup_color == 6) // Коричневый
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(193 + wide_delta, 85, "rjhbxytdsq"); dp_translation = NULL; }
            else if (messages_pickup_color == 7) // Розовый
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(193 + wide_delta, 85, "hjpjdsq"); dp_translation = NULL; }
            else                            // Красный
            { dp_translation = NULL; M_WriteTextSmall_RUS(193 + wide_delta, 85, "rhfcysq"); }
        }

        // Обнаружение тайников
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(203 + wide_delta, 95, "y*l"); // н/д
        }
        else
        {
            if (messages_secret_color == 1) // Зеленый
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(203 + wide_delta, 95, "ptktysq"); dp_translation = NULL; }
            else if (messages_secret_color == 2) // Синий
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(203 + wide_delta, 95, "cbybq"); dp_translation = NULL; }
            else if (messages_secret_color == 3) // Желтый
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(203 + wide_delta, 95, ";tknsq"); dp_translation = NULL; }
            else if (messages_secret_color == 4) // Белый
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(203 + wide_delta, 95, ",tksq"); dp_translation = NULL; }
            else if (messages_secret_color == 5) // Бежевый
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(203 + wide_delta, 95, ",t;tdsq"); dp_translation = NULL; }
            else if (messages_secret_color == 6) // Коричневый
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(203 + wide_delta, 95, "rjhbxytdsq"); dp_translation = NULL; }
            else if (messages_secret_color == 7) // Розовый
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(203 + wide_delta, 95, "hjpjdsq"); dp_translation = NULL; }
            else                            // Красный
            { dp_translation = NULL; M_WriteTextSmall_RUS(203 + wide_delta, 95, "rhfcysq"); }
        }

        // Системные сообщения
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(197 + wide_delta, 105, "y*l"); // н/д
        }
        else
        {
            if (messages_system_color == 1) // Зеленый
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(197 + wide_delta, 105, "ptktysq"); dp_translation = NULL; }
            else if (messages_system_color == 2) // Синий
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(197 + wide_delta, 105, "cbybq"); dp_translation = NULL; }
            else if (messages_system_color == 3) // Желтый
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(197 + wide_delta, 105, ";tknsq"); dp_translation = NULL; }
            else if (messages_system_color == 4) // Белый
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(197 + wide_delta, 105, ",tksq"); dp_translation = NULL; }
            else if (messages_system_color == 5) // Бежевый
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(197 + wide_delta, 105, ",t;tdsq"); dp_translation = NULL; }
            else if (messages_system_color == 6) // Коричневый
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(197 + wide_delta, 105, "rjhbxytdsq"); dp_translation = NULL; }
            else if (messages_system_color == 7) // Розовый
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(197 + wide_delta, 105, "hjpjdsq"); dp_translation = NULL; }
            else                            // Красный
            { dp_translation = NULL; M_WriteTextSmall_RUS(197 + wide_delta, 105, "rhfcysq"); }
        }

        // Чат сетевой игры
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(164 + wide_delta, 115, "y*l"); // н/д
        }
        else
        {
            if (messages_chat_color == 1) // Зеленый
            { dp_translation = cr[CR_GREEN]; M_WriteTextSmall_RUS(164 + wide_delta, 115, "ptktysq"); dp_translation = NULL; }
            else if (messages_chat_color == 2) // Синий
            { dp_translation = cr[CR_BLUE2]; M_WriteTextSmall_RUS(164 + wide_delta, 115, "cbybq"); dp_translation = NULL; }
            else if (messages_chat_color == 3) // Желтый
            { dp_translation = cr[CR_GOLD]; M_WriteTextSmall_RUS(164 + wide_delta, 115, ";tknsq"); dp_translation = NULL; }
            else if (messages_chat_color == 4) // Белый
            { dp_translation = cr[CR_GRAY]; M_WriteTextSmall_RUS(164 + wide_delta, 115, ",tksq"); dp_translation = NULL; }
            else if (messages_chat_color == 5) // Бежевый
            { dp_translation = cr[CR_TAN]; M_WriteTextSmall_RUS(164 + wide_delta, 115, ",t;tdsq"); dp_translation = NULL; }
            else if (messages_chat_color == 6) // Коричневый
            { dp_translation = cr[CR_BROWN]; M_WriteTextSmall_RUS(164 + wide_delta, 115, "rjhbxytdsq"); dp_translation = NULL; }
            else if (messages_chat_color == 7) // Розовый
            { dp_translation = cr[CR_BRICK]; M_WriteTextSmall_RUS(164 + wide_delta, 115, "hjpjdsq"); dp_translation = NULL; }
            else                            // Красный
            { dp_translation = NULL; M_WriteTextSmall_RUS(164 + wide_delta, 115, "rhfcysq"); }
        }
    }

    // Message timeout slider
    M_DrawThermo_Small(35 + wide_delta, 54, 10, messages_timeout - 1);
}

void M_RD_Change_Messages(int choice)
{
    showMessages ^= 1;

    if (!showMessages)
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            MSGOFF : MSGOFF_RUS);
    else
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            MSGON : MSGON_RUS);

    message_dontfuckwithme = true;
}

void M_RD_Change_Msg_TimeOut(int choice)
{
    switch(choice)
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

void M_RD_Change_Msg_Pickup_Color(int choice)
{
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
    return;

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
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
    return;

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
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
    return;

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
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
    return;

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

void M_RD_Choose_AutomapAndStatsSettings(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Automap_Def :
                    &RD_Automap_Def_Rus);
}

void M_RD_Draw_AutomapSettings(void)
{
    static char num[4];

    M_snprintf(num, 4, "%d", automap_grid_size);

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "AUTOMAP AND STATS");

        //
        // Automap
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(70 + wide_delta, 25, "Automap");
        dp_translation = NULL;

        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(170 + wide_delta, 35, "n/a");
        }
        else
        {
            M_WriteTextSmall_ENG (170 + wide_delta, 35, automap_color == 1 ? "boom"   :
                                                        automap_color == 2 ? "jaguar" :
                                                        automap_color == 3 ? "raven"  :
                                                        automap_color == 4 ? "strife" :
                                                        automap_color == 5 ? "unity"  :
                                                                             "doom");
        }

        // Line antialiasing
        dp_translation = automap_antialias ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(193 + wide_delta, 45, automap_antialias ? "on" : "off");
        dp_translation = NULL;

        // Overlay mode
        dp_translation = automap_overlay ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(170 + wide_delta, 55, automap_overlay ? "on" : "off");
        dp_translation = NULL;

        // Rotate mode
        dp_translation = automap_rotate ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(163 + wide_delta, 65, automap_rotate ? "on" : "off");
        dp_translation = NULL;

        // Follow mode
        dp_translation = automap_follow ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(164 + wide_delta, 75, automap_follow ? "on" : "off");
        dp_translation = NULL;

        // Grid
        dp_translation = automap_grid ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(106 + wide_delta, 85, automap_grid ? "on" : "off");
        dp_translation = NULL;

        // Grid size
        dp_translation = automap_grid_size == 128 ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_ENG(136 + wide_delta, 95, num);
        dp_translation = NULL;

        //
        // Statistics
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(70 + wide_delta, 105, "Statistics");
        dp_translation = NULL;

        // Level stats
        dp_translation = automap_stats ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(159 + wide_delta, 115, automap_stats == 1 ? "in automap" :
                                                    automap_stats == 2 ? "always" :
                                                                         "off");
        dp_translation = NULL;

        // Level time
        dp_translation = automap_level_time ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(150 + wide_delta, 125, automap_level_time == 1 ? "in automap" :
                                                    automap_level_time == 2 ? "always" :
                                                                              "off");
        dp_translation = NULL;

        // Total time
        dp_translation = automap_total_time ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(151 + wide_delta, 135, automap_total_time == 1 ? "in automap" :
                                                    automap_total_time == 2 ? "always" :
                                                                              "off");
        dp_translation = NULL;

        // Player coords
        dp_translation = automap_coords ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(177 + wide_delta, 145, automap_coords == 1 ? "in automap" :
                                                    automap_coords == 2 ? "always" :
                                                                          "off");
        dp_translation = NULL;
        

    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "RFHNF B CNFNBCNBRF"); // КАРТА И СТАТИСТИКА

        //
        // Карта
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(70 + wide_delta, 25, "Rfhnf");
        dp_translation = NULL;

        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(191 + wide_delta, 35, "y*l"); // н/д
        }
        else
        {
            M_WriteTextSmall_ENG (189 + wide_delta, 35, automap_color == 1 ? "boom"   :
                                                        automap_color == 2 ? "jaguar" :
                                                        automap_color == 3 ? "raven"  :
                                                        automap_color == 4 ? "strife" :
                                                        automap_color == 5 ? "unity"  :
                                                                             "doom");
        }

        // Сглаживание линий
        dp_translation = automap_antialias ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(214 + wide_delta, 45, automap_antialias ? "drk" : "dsrk");
        dp_translation = NULL;

        // Режим наложения
        dp_translation = automap_overlay ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(203 + wide_delta, 55, automap_overlay ? "drk" : "dsrk");
        dp_translation = NULL;

        // Режим вращения
        dp_translation = automap_rotate ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(194 + wide_delta, 65, automap_rotate ? "drk" : "dsrk");
        dp_translation = NULL;

        // Режим следования
        dp_translation = automap_follow ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(208 + wide_delta, 75, automap_follow ? "drk" : "dsrk");
        dp_translation = NULL;

        // Сетка
        dp_translation = automap_grid ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(118 + wide_delta, 85, automap_grid ? "drk" : "dsrk");
        dp_translation = NULL;

        // Размер сетки
        dp_translation = automap_grid_size == 128 ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_ENG(171 + wide_delta, 95, num);
        dp_translation = NULL;
        
        //
        // Статистика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(70 + wide_delta, 105, "Cnfnbcnbrf");
        dp_translation = NULL;
        

        // Статистика уровня
        dp_translation = automap_stats ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(210 + wide_delta, 115, automap_stats == 1 ? "yf rfhnt" :
                                                    automap_stats == 2 ? "dctulf" :
                                                                         "dsrk");
        dp_translation = NULL;

        // Время уровня
        dp_translation = automap_level_time ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(171 + wide_delta, 125, automap_level_time == 1 ? "yf rfhnt" :
                                                    automap_level_time == 2 ? "dctulf" :
                                                                              "dsrk");
        dp_translation = NULL;

        // Общее время
        dp_translation = automap_total_time ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(166 + wide_delta, 135, automap_total_time == 1 ? "yf rfhnt" :
                                                    automap_total_time == 2 ? "dctulf" :
                                                                              "dsrk");
        dp_translation = NULL;

        // Координаты игрока
        dp_translation = automap_coords ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(213 + wide_delta, 145, automap_coords == 1 ? "yf rfhnt" :
                                                    automap_coords == 2 ? "dctulf" :
                                                                          "dsrk");
        dp_translation = NULL;
    }
}

void M_RD_Change_AutomapColor(int choice)
{
    // [JN] Disable automap colors changing in Jaguar
    if (gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        automap_color--;
        if (automap_color < 0) 
            automap_color = 5;
        break;
    
        case 1:
        automap_color++;
        if (automap_color > 5)
            automap_color = 0;
        break;
    }
}

void M_RD_Change_AutomapAntialias(int choice)
{
    automap_antialias ^= 1;
}

void M_RD_Change_AutomapOverlay(int choice)
{
    automap_overlay ^= 1;
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

void M_RD_Change_AutomapGridSize(int choice)
{
    switch(choice)
    {
        case 0:
            if (automap_grid_size == 512)
            {
                automap_grid_size = 256;
            }
            else if (automap_grid_size == 256)
            {
                automap_grid_size = 128;
            }
            else if (automap_grid_size == 128)
            {
                automap_grid_size = 64;
            }
            else if (automap_grid_size == 64)
            {
                automap_grid_size = 32;
            }
        break;

        case 1:
            if (automap_grid_size == 32)
            {
                automap_grid_size = 64;
            }
            else if (automap_grid_size == 64)
            {
                automap_grid_size = 128;
            }
            else if (automap_grid_size == 128)
            {
                automap_grid_size = 256;
            }
            else if (automap_grid_size == 256)
            {
                automap_grid_size = 512;
            }
        break;
    }
}

void M_RD_Change_AutomapStats(int choice)
{
    switch(choice)
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

void M_RD_Change_AutomapLevelTime(int choice)
{
    switch(choice)
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

void M_RD_Change_AutomapTotalTime(int choice)
{
    switch(choice)
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

void M_RD_Change_AutomapCoords(int choice)
{
    switch(choice)
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
    static char num[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "SOUND OPTIONS");

        //
        // Volume
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "volume");
        dp_translation = NULL;

        //
        // Channels
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 75, "channels");
        dp_translation = NULL;

        //
        // System
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 105, "advanced");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB PDERF"); // НАСТРОЙКИ ЗВУКА

        //
        // Громкость
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "uhjvrjcnm");
        dp_translation = NULL;

        //
        // Воспроизведение
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 75, "djcghjbpdtltybt");
        dp_translation = NULL;

        //
        // Дополнительно
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 105, "ljgjkybntkmyj");
        dp_translation = NULL;
    }

    // SFX volume slider
    M_DrawThermo_Small(35 + wide_delta, 44, 16, sfxVolume);
    // Numerical representation of SFX volume
    M_snprintf(num, 4, "%3d", sfxVolume);
    M_WriteTextSmall_ENG(177 + wide_delta, 45, num);

    // Music volume slider
    M_DrawThermo_Small(35 + wide_delta, 64, 16, musicVolume);
    // Numerical representation of music volume
    M_snprintf(num, 4, "%3d", musicVolume);
    M_WriteTextSmall_ENG(177 + wide_delta, 65, num);

    // SFX channels slider
    M_DrawThermo_Small(35 + wide_delta, 94, 16, snd_channels_rd / 4 - 1);
    // Numerical representation of channels
    M_snprintf(num, 4, "%3d", snd_channels_rd);
    M_WriteTextSmall_ENG(177 + wide_delta, 95, num);
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
        if (snd_channels_rd > 4)
            snd_channels_rd -= 4;
        break;
    
        case 1:
        if (snd_channels_rd < 64)
            snd_channels_rd += 4;
        break;
    }

    // Sync internal value
    snd_channels = snd_channels_rd;

    // Reallocate sound channels
    S_ChannelsRealloc();
}

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

void M_RD_Choose_SoundSystem(int choice)
{
    M_SetupNextMenu(english_language ?
                    &RD_Audio_System_Def :
                    &RD_Audio_System_Def_Rus);
}

void M_RD_Draw_Audio_System(void)
{
    static char snd_frequency[4];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "SOUND SYSTEM");

        //
        // Sound system
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "sound system");
        dp_translation = NULL;

        // Sound effects
        if (snd_sfxdevice == 0)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(141 + wide_delta, 35, "disabled");
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 1)
        {
            M_WriteTextSmall_ENG(141 + wide_delta, 35, "pc speaker");
        }
        else if (snd_sfxdevice == 3)
        {
            M_WriteTextSmall_ENG(141 + wide_delta, 35, "digital sfx");
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(79 + wide_delta, 45, "disabled");
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            M_WriteTextSmall_ENG(79 + wide_delta, 45, "opl2 synth");
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            M_WriteTextSmall_ENG(79 + wide_delta, 45, "opl3 synth");
        }
        else if (snd_musicdevice == 5)
        {
            M_WriteTextSmall_ENG(79 + wide_delta, 45, "gus emulation");
        }
        else if (snd_musicdevice == 8)
        {
            M_WriteTextSmall_ENG(79 + wide_delta, 45, "midi/mp3/ogg/flac");
        }

        //
        // Quality
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 55, "quality");
        dp_translation = NULL;

        // Sampling frequency
        sprintf(snd_frequency, "%d HZ", snd_samplerate);
        M_WriteTextSmall_ENG(179 + wide_delta, 65, snd_frequency);

        //
        // Miscellaneous
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 75, "Miscellaneous");
        dp_translation = NULL;

        // Sfx mode
        M_WriteTextSmall_ENG(178 + wide_delta, 85, snd_monomode ? "mono" : "stereo");

        // Pitch-shifted sounds
        M_WriteTextSmall_ENG(186 + wide_delta, 95, snd_pitchshift ? "on" : "off");

        // Mute inactive window
        M_WriteTextSmall_ENG(185 + wide_delta, 105, mute_inactive_window ? "on" : "off");
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "PDERJDFZ CBCNTVF"); // ЗВУКОВАЯ СИСТЕМА

        //
        // Звуковая система
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "pderjdfz cbcntvf");
        dp_translation = NULL;

        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // Отключены
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(175 + wide_delta, 35, "jnrk.xtys");
            dp_translation = NULL;
        }
        else if (snd_sfxdevice == 1)
        {
            // Динамик ПК
            M_WriteTextSmall_RUS(175 + wide_delta, 35, "lbyfvbr gr");
        }
        else if (snd_sfxdevice == 3)
        {
            // Цифровые
            M_WriteTextSmall_RUS(175 + wide_delta, 35, "wbahjdst");
        }

        // Музыка
        if (snd_musicdevice == 0)
        {
            // Отключена
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(94 + wide_delta, 45, "jnrk.xtyf");
            dp_translation = NULL;
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // Синтез OPL2
            M_WriteTextSmall_RUS(94 + wide_delta, 45, "cbyntp");
            M_WriteTextSmall_ENG(146 + wide_delta, 45, "opl2");
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // Синтез OPL3
            M_WriteTextSmall_RUS(94 + wide_delta, 45, "cbyntp");
            M_WriteTextSmall_ENG(146 + wide_delta, 45, "opl3");
        }
        else if (snd_musicdevice == 5)
        {
            // Эмуляция GUS
            M_WriteTextSmall_RUS(94 + wide_delta, 45, "\'vekzwbz");
            M_WriteTextSmall_ENG(164 + wide_delta, 45, "gus");
        }
        else if (snd_musicdevice == 8)
        {
            M_WriteTextSmall_ENG(94 + wide_delta, 45, "MIDI/MP3/OGG/FLAC");
        }

        //
        // Качество звучания
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 55, "rfxfcndj pdexfybz");
        dp_translation = NULL;

        // Частота дискретизации (ГЦ)
        sprintf(snd_frequency, "%d UW", snd_samplerate);
        M_WriteTextSmall_RUS(208 + wide_delta, 65, snd_frequency);

        //
        // Разное
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 75, "hfpyjt");
        dp_translation = NULL;

        // Режим звука
        M_WriteTextSmall_RUS(231 + wide_delta, 85, snd_monomode ? "vjyj" : "cnthtj");

        // Произвольный питч-шифтинг
        M_WriteTextSmall_RUS(242 + wide_delta, 95, snd_pitchshift ? "drk" : "dsrk");

        // Звук в неактивном окне
        M_WriteTextSmall_RUS(208 + wide_delta, 105, mute_inactive_window ? "dsrk" : "drk");
    }
}

void M_RD_Change_SoundDevice(int choice)
{
    switch(choice)
    {
        case 0:
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 3;
        else 
        if (snd_sfxdevice == 3)
            snd_sfxdevice = 1;
        else 
        if (snd_sfxdevice == 1)
            snd_sfxdevice = 0;
        break;
        case 1:
        if (snd_sfxdevice == 0)
            snd_sfxdevice = 1;
        else 
        if (snd_sfxdevice == 1)
            snd_sfxdevice = 3;
        else 
        if (snd_sfxdevice == 3)
            snd_sfxdevice = 0;
        break;
    }

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume
    S_SetSfxVolume(sfxVolume * 8);
}

void M_RD_Change_MusicDevice(int choice)
{
    switch(choice)
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
        break;
    }

    // Shut down current music
    S_StopMusic();

    // Shut down music system
    S_Shutdown();
    
    // Start music system
    I_InitSound(true);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(music_num_rd, true);
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

    // Shut down sound system
    I_ShutdownSound();

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume
    S_SetSfxVolume(sfxVolume * 8);

    // Shut down current music
    S_StopMusic();

    // Shut down music system
    S_Shutdown();
    
    // Start music system
    I_InitSound(true);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(music_num_rd, true);
}

void M_RD_Change_SndMode(int choice)
{
    snd_monomode ^= 1;

    // Update stereo separation
    S_UpdateStereoSeparation();
}

void M_RD_Change_PitchShifting(int choice)
{
    snd_pitchshift ^= 1;
}

void M_RD_Change_MuteInactive(int choice)
{
    mute_inactive_window ^= 1;
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
        M_WriteTextBigCentered_ENG(5, "CONTROL SETTINGS");

        //
        // Controls
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Controls");
        dp_translation = NULL;

        //
        // Mouse
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 55, "mouse");
        dp_translation = NULL;

        // Mouse look
        M_WriteTextSmall_ENG(119 + wide_delta, 125, mlook ? "on" : "off");

        // Invert Y axis
        if (!mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(130 + wide_delta, 135, mouse_y_invert ? "on" : "off");
        dp_translation = NULL;

        // Vertical movement
        if (mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(171 + wide_delta, 145, !novert ? "on" : "off");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "EGHFDKTYBT"); // УПРАВЛЕНИЕ

        //
        // Управление
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "eghfdktybt");
        dp_translation = NULL;

        //
        // Мышь
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 55, "vsim");
        dp_translation = NULL;

        // Обзор мышью
        M_WriteTextSmall_RUS(135 + wide_delta, 125, mlook ? "drk" : "dsrk");

        // Вертикальная инверсия
        if (!mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207 + wide_delta, 135, mouse_y_invert ? "drk" : "dsrk");
        dp_translation = NULL;

        // Вертикальное перемещение
        if (mlook)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_RUS(235 + wide_delta, 145, !novert ? "drk" : "dsrk");
        dp_translation = NULL;
    }

    // Mouse sensivity slider
    M_DrawThermo_Small(35 + wide_delta, 74, 17, mouseSensitivity);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    M_WriteTextSmall_ENG(189 + wide_delta, 75, num);

    // Acceleration slider
    M_DrawThermo_Small(35 + wide_delta, 94, 17, mouse_acceleration * 4 - 4);
    // Numerical representation
    M_snprintf(num, 4, "%f", mouse_acceleration);
    M_WriteTextSmall_ENG(189 + wide_delta, 95, num);

    // Acceleration threshold slider
    M_DrawThermo_Small(35 + wide_delta, 114, 17, mouse_threshold / 2);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouse_threshold);
    if (mouse_acceleration < 1.1)
    dp_translation = cr[CR_DARKRED];
    M_WriteTextSmall_ENG(189 + wide_delta, 115, num);
    dp_translation = NULL;
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
    players[consoleplayer].centering = true;
}

void M_RD_Change_InvertY(int choice)
{
    mouse_y_invert ^= 1;
}

void M_RD_Change_Novert(int choice)
{
    novert ^= 1;
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

void M_RD_Change_Acceleration(int choice)
{
    switch(choice)
    {
        case 0:
        if (mouse_acceleration > 1.1)
            mouse_acceleration -= 0.1;
        break;

        case 1:
        if (mouse_acceleration < 5.0)
            mouse_acceleration += 0.1;
        break;
    }
}

void M_RD_Change_Threshold(int choice)
{
    switch(choice)
    {
        case 0:
        if (mouse_threshold > 0)
            mouse_threshold--;
        break;

        case 1:
        if (mouse_threshold < 32)
            mouse_threshold++;
        break;
    }
}

// -----------------------------------------------------------------------------
// Key bindings
// -----------------------------------------------------------------------------

key_page_t* getCurrentKeyPage()
{
    if(currentMenu == RD_Bindings_Menu_Def_1 ||
       currentMenu == RD_Bindings_Menu_Def_1_Rus)
    {
        return &RD_Bindings_1;
    }
    else if(currentMenu == RD_Bindings_Menu_Def_2 ||
            currentMenu == RD_Bindings_Menu_Def_2_Rus)
    {
        return &RD_Bindings_2;
    }
    else if(currentMenu == RD_Bindings_Menu_Def_3 ||
            currentMenu == RD_Bindings_Menu_Def_3_Rus)
    {
        return &RD_Bindings_3;
    }
    else if(currentMenu == RD_Bindings_Menu_Def_4 ||
            currentMenu == RD_Bindings_Menu_Def_4_Rus)
    {
        return &RD_Bindings_4;
    }
    else
    {
        return NULL;
    }
}

void M_RD_Bind_Key(int choice)
{
    key_page_t* keyPage = getCurrentKeyPage();
    if(keyPage) M_RD_StartBinding(BK_getKeyDescriptor(keyPage->keys[choice])->key_var);
}

void M_RD_Draw_Bindings()
{
    key_page_t *keyPage = getCurrentKeyPage();
    int x = (english_language ? 209 : 210);

    M_RD_Draw_Menu_Background();

    if (english_language)
        M_WriteTextBigCentered_ENG(5, "Keyboard bindings");
    else
        M_WriteTextBigCentered_RUS(5, "Yfcnhjqrb rkfdbfnehs"); // Настройки клавиатуры

    dp_translation = cr[CR_GOLD];
    for (int i = 0; i < keyPage->num_of_subtitles; ++i)
    {
        if (english_language)
            M_WriteTextSmall_ENG(35 + wide_delta, keyPage->subtitles[i].y, keyPage->subtitles[i].eng_text);
        else
            M_WriteTextSmall_RUS(35 + wide_delta, keyPage->subtitles[i].y, keyPage->subtitles[i].rus_text);
    }

    dp_translation = cr[CR_GRAY];
    if (english_language)
    {
        M_WriteTextSmall_ENG(35 + wide_delta, 155, currentMenu == RD_Bindings_Menu_Def_4 ? "first page >" : "next page >");
        M_WriteTextSmall_ENG(35 + wide_delta, 165, currentMenu == RD_Bindings_Menu_Def_1 ? "< last page" : "< prev page");
        M_WriteTextSmall_ENG(x + wide_delta, 165, keyPage->pageNumber_eng);
        dp_translation = NULL;

        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(55 + wide_delta, 180, "enter to change, del to clear");
        M_WriteTextSmall_ENG(75 + wide_delta, 189, "pgup/pgdn to turn pages");
    }
    else
    {
        M_WriteTextSmall_RUS(35 + wide_delta, 155, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + wide_delta, 165, RD_PREV_RUS);
        M_WriteTextSmall_RUS(x + wide_delta, 165, keyPage->pageNumber_rus);
        dp_translation = NULL;

        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(44 + wide_delta, 180, "enter =");
        M_WriteTextSmall_RUS(88 + wide_delta, 180, "= yfpyfxbnm<");
        M_WriteTextSmall_ENG(176 + wide_delta, 180, "del =");
        M_WriteTextSmall_RUS(213 + wide_delta, 180, "jxbcnbnm");

        M_WriteTextSmall_ENG(55 + wide_delta, 189, "pgup/pgdn =");
        M_WriteTextSmall_RUS(139 + wide_delta, 189, "kbcnfnm cnhfybws");
    }
    dp_translation = NULL;

    for (int i = 0; i < 11; ++i)
    {
        bound_key_t key = keyPage->keys[i];
        if (key != bk_null)
        {
            boolean bindingThis = messageToBind && i == itemOn;
            if (bindingThis) dp_translation = cr[CR_GRAY];
            else if (BK_KeyHasNoBinds(key)) dp_translation = cr[CR_DARKRED];

            M_WriteTextSmall_ENG(x + wide_delta, i * 10 + 35, bindingThis ? "?" : BK_getBoundKeysString(key));
            dp_translation = NULL;
        }
    }
}

void M_RD_Choose_Bindings_1(int choice)
{
    M_SetupNextMenu(english_language ?
                    RD_Bindings_Menu_Def_1 :
                    RD_Bindings_Menu_Def_1_Rus);
}

void M_RD_Choose_Bindings_2(int choice)
{
    M_SetupNextMenu(english_language ?
                    RD_Bindings_Menu_Def_2 :
                    RD_Bindings_Menu_Def_2_Rus);
}

void M_RD_Choose_Bindings_3(int choice)
{
    M_SetupNextMenu(english_language ?
                    RD_Bindings_Menu_Def_3 :
                    RD_Bindings_Menu_Def_3_Rus);
}

void M_RD_Choose_Bindings_4(int choice)
{
    M_SetupNextMenu(english_language ?
                    RD_Bindings_Menu_Def_4 :
                    RD_Bindings_Menu_Def_4_Rus);
}

// -----------------------------------------------------------------------------
// M_RD_MouseBtnDrawer
// [JN] Returns button name for mouse bindings.
// -----------------------------------------------------------------------------

static char *M_RD_MouseBtnDrawer (int i)
{
    switch (i)
    {
        case -1:    return "---";
        case  0:    return "LEFT BUTTON";
        case  1:    return "RIGHT BUTTON";
        case  2:    return "MIDDLE BUTTON";
        case  3:    return "BUTTON #3";
        case  4:    return "BUTTON #4";
        default:    return "?"; // [JN] Unknown key
    }
}

// -----------------------------------------------------------------------------
// Mouse bindings
// -----------------------------------------------------------------------------

void M_RD_Choose_Mouse_Bindings(int choice)
{
    M_SetupNextMenu(english_language ?
                    &RD_Mouse_Bindings_Menu_Def :
                    &RD_Mouse_Bindings_Menu_Def_Rus);
}

void M_RD_Draw_Mouse_Bindings(void)
{
    int x = 186;
    
    M_RD_Draw_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "Mouse bindings");

        //
        // Buttons
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Buttons");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(55 + wide_delta, 180, "enter to change, del to clear");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "Yfcnhjqrb vsib"); // Настройки мыши

        //
        // Кнопки
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "ryjgrb");
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(44 + wide_delta, 180, "enter =");
        M_WriteTextSmall_RUS(88 + wide_delta, 180, "= yfpyfxbnm<");
        M_WriteTextSmall_ENG(176 + wide_delta, 180, "del =");
        M_WriteTextSmall_RUS(213 + wide_delta, 180, "jxbcnbnm");
        dp_translation = NULL;
    }

    // Fire/Attack
    if (messageToBind && itemOn == rd_mouse_bindings_attack)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 35, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebfire == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 35, M_RD_MouseBtnDrawer(mousebfire));
        dp_translation = NULL;
    }

    // Use
    if (messageToBind && itemOn == rd_mouse_bindings_use)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 45, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebuse == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 45, M_RD_MouseBtnDrawer(mousebuse));
        dp_translation = NULL;
    }

    // Move Forward
    if (messageToBind && itemOn == rd_mouse_bindings_forward)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 55, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebforward == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 55, M_RD_MouseBtnDrawer(mousebforward));
        dp_translation = NULL;
    }

    // Move Backward
    if (messageToBind && itemOn == rd_mouse_bindings_backward)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 65, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebbackward == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 65, M_RD_MouseBtnDrawer(mousebbackward));
        dp_translation = NULL;
    }

    // Strafe On
    if (messageToBind && itemOn == rd_mouse_bindings_strafeon)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 75, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebstrafe == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 75, M_RD_MouseBtnDrawer(mousebstrafe));
        dp_translation = NULL;
    }

    // Strafe Left
    if (messageToBind && itemOn == rd_mouse_bindings_strafeleft)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 85, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebstrafeleft == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 85, M_RD_MouseBtnDrawer(mousebstrafeleft));
        dp_translation = NULL;
    }

    // Strafe Right
    if (messageToBind && itemOn == rd_mouse_bindings_straferight)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 95, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebstraferight == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 95, M_RD_MouseBtnDrawer(mousebstraferight));
        dp_translation = NULL;
    }

    // Previous Weapon
    if (messageToBind && itemOn == rd_mouse_bindings_prevweapon)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 105, "?");
        dp_translation = NULL;
    }
    else
    {

        if (mousebprevweapon == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 105, M_RD_MouseBtnDrawer(mousebprevweapon));
        dp_translation = NULL;
    }

    // Next Weapon
    if (messageToBind && itemOn == rd_mouse_bindings_nextweapon)
    {
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(x + wide_delta, 115, "?");
        dp_translation = NULL;
    }
    else
    {
        if (mousebnextweapon == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x + wide_delta, 115, M_RD_MouseBtnDrawer(mousebnextweapon));
        dp_translation = NULL;
    }
}

//
// Fire/Attack
//
void M_RD_Mouse_Bind_FireAttack (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebfire);
}

//
// Use
//
void M_RD_Mouse_Bind_Use (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebuse);
}

//
// Move Forward
//
void M_RD_Mouse_Bind_MoveForward (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebforward);
}

//
// Move Backward
//
void M_RD_Mouse_Bind_MoveBackward (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebbackward);
}

//
// Strafe On
//
void M_RD_Mouse_Bind_StrafeOn (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstrafe);
}

//
// Strafe Left
//
void M_RD_Mouse_Bind_StrafeLeft (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstrafeleft);
}

//
// Strafe Right
//
void M_RD_Mouse_Bind_StrafeRight (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstraferight);
}

//
// Previous Weapon
//
void M_RD_Mouse_Bind_PrevWeapon (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebprevweapon);
}

//
// Previous Weapon
//
void M_RD_Mouse_Bind_NextWeapon (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebnextweapon);
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

void M_RD_Draw_Gameplay_1(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Graphical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Graphical");
        dp_translation = NULL;

        // Brightmaps
        dp_translation = brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(119 + wide_delta, 35, brightmaps ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Fake contrast
        dp_translation = fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(142 + wide_delta, 45, fake_contrast ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Translucency
        dp_translation = translucency ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(138 + wide_delta, 55, translucency ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Fuzz effect
        dp_translation = improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(125 + wide_delta, 65, 
                             improved_fuzz == 0 ? "Original" :
                             improved_fuzz == 1 ? "Original (b&w)" :
                             improved_fuzz == 2 ? "Improved" :
                                                  "Improved (b&w)");
        dp_translation = NULL;

        // Colored HUD elements
        dp_translation = colored_hud ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(195 + wide_delta, 75, colored_hud == 1 ? "On (no %)"  :
                                                   colored_hud == 2 ? RD_ON :
                                                                      RD_OFF);
        dp_translation = NULL;

        // Colored blood and corpses
        dp_translation = colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(229 + wide_delta, 85, colored_blood ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Swirling liquids
        dp_translation = swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(150 + wide_delta, 95, swirling_liquids ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Invulnerability affects sky
        dp_translation = invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(237 + wide_delta, 105, invul_sky ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Flip weapons
        dp_translation = flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(131 + wide_delta, 115, flip_weapons ? RD_ON : RD_OFF);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(35 + wide_delta, 145, "next page >"); 
        M_WriteTextSmall_ENG(35 + wide_delta, 155, "< last page"); 
        M_WriteTextSmall_ENG(231 + wide_delta, 155, "page 1/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Графика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "uhfabrf");
        dp_translation = NULL;

        // Брайтмаппинг
        dp_translation = brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(140 + wide_delta, 35, brightmaps ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Имитация контрастности
        dp_translation = fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(217 + wide_delta, 45, fake_contrast ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Прозрачность объектов
        dp_translation = translucency ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207 + wide_delta, 55, translucency ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Эффект шума
        dp_translation = improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(134 + wide_delta, 65, 
                             improved_fuzz == 0 ? "Jhbubyfkmysq" :
                             improved_fuzz == 1 ? "Jhbubyfkmysq (x*,)" :
                             improved_fuzz == 2 ? "Ekexityysq" :
                                                  "Ekexityysq (x*,)");
        dp_translation = NULL;

        // Разноцветные элементы HUD
        dp_translation = colored_hud ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(239 + wide_delta, 75, colored_hud == 1 ? "Drk (,tp %)" :
                                                   colored_hud == 2 ? RD_ON_RUS :
                                                                      RD_OFF_RUS);
        dp_translation = NULL;

        // Разноцветная кровь и трупы
        dp_translation = colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(242 + wide_delta, 85, colored_blood ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Улучшенная анимация жидкостей
        dp_translation = swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(275 + wide_delta, 95, swirling_liquids ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Неуязвимость окрашивает небо
        dp_translation = invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(262 + wide_delta, 105, invul_sky ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Зеркальное отражение оружия
        dp_translation = flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(259 + wide_delta, 115, flip_weapons ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(35 + wide_delta, 145, RD_NEXT_RUS); 
        M_WriteTextSmall_RUS(35 + wide_delta, 155, RD_PREV_RUS); 
        M_WriteTextSmall_RUS(197 + wide_delta, 155, "cnhfybwf 1*4");
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_2(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Audible
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Audible");
        dp_translation = NULL;

        // Sound attenuation axises
        dp_translation = z_axis_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(217 + wide_delta, 35, z_axis_sfx ? "x/y/z" : "x/y");
        dp_translation = NULL;

        // Play exit sounds
        dp_translation = play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(158 + wide_delta, 45, play_exit_sfx ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Sound of crushing corpses
        dp_translation = crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(223 + wide_delta, 55, crushed_corpses_sfx ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Single sound of closing blazing door
        dp_translation = blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(240 + wide_delta, 65, blazing_door_fix_sfx ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Monster alert waking up other monsters
        dp_translation = noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(263 + wide_delta, 75, noise_alert_sfx ? RD_ON : RD_OFF);
        dp_translation = NULL;

        //
        // Tactical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 85, "Tactical");
        dp_translation = NULL;

        // Notify of revealed secrets
        dp_translation = secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(232 + wide_delta, 95, secret_notification ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Show negative health
        dp_translation = negative_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(190 + wide_delta, 105, negative_health ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Infragreen light amp. visor
        dp_translation = infragreen_visor ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(230 + wide_delta, 115, infragreen_visor ? RD_ON : RD_OFF);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(35 + wide_delta, 145, "next page >");
        M_WriteTextSmall_ENG(35 + wide_delta, 155, "< prev page");
        M_WriteTextSmall_ENG(231 + wide_delta, 155, "page 2/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Звук
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "Pder");
        dp_translation = NULL;

        // Затухание звука по осям
        dp_translation = z_axis_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(217 + wide_delta, 35, z_axis_sfx ? "x/y/z" : "x/y");
        dp_translation = NULL;

        // Звуки при выходе из игры
        dp_translation = play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(225 + wide_delta, 45, play_exit_sfx ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Звук раздавливания трупов
        dp_translation = crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(236 + wide_delta, 55, crushed_corpses_sfx ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Одиночный звук быстрой двери
        dp_translation = blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(260 + wide_delta, 65, blazing_door_fix_sfx ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Общая тревога у монстров
        dp_translation = noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(227 + wide_delta, 75, noise_alert_sfx ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        //
        // Тактика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 85, "Nfrnbrf");
        dp_translation = NULL;

        // Сообщать о найденном тайнике
        dp_translation = secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(260 + wide_delta, 95, secret_notification ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Отрицательное здоровье в HUD
        dp_translation = negative_health ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(255 + wide_delta, 105, negative_health ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Инфразеленый визор освещения
        dp_translation = infragreen_visor ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(266 + wide_delta, 115, infragreen_visor ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Footer
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(35 + wide_delta, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + wide_delta, 155, RD_PREV_RUS);
        M_WriteTextSmall_RUS(197 + wide_delta, 155, "cnhfybwf 2*4");
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_3(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Physical
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Physical");
        dp_translation = NULL;

        // Walk over and under monsters
        dp_translation = over_under ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(250 + wide_delta, 35, over_under ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Corpses sliding from the ledges
        dp_translation = torque ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(264 + wide_delta, 45, torque ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Weapon bobbing while firing
        dp_translation = weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(233 + wide_delta, 55, weapon_bobbing ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Lethal pellet of a point-blank SSG
        dp_translation = ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(287 + wide_delta, 65, ssg_blast_enemies ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Randomly mirrored corpses
        dp_translation = randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(231 + wide_delta, 75, randomly_flipcorpses ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Floating powerups
        dp_translation = floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(171 + wide_delta, 85, floating_powerups ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Items are tossed when dropped
        dp_translation = toss_drop ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(254 + wide_delta, 95, toss_drop ? RD_ON : RD_OFF);
        dp_translation = NULL;        

        //
        // Crosshair
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 105, "Crosshair");
        dp_translation = NULL;

        // Draw crosshair
        dp_translation = crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(146 + wide_delta, 115, crosshair_draw ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Indication
        dp_translation = crosshair_type ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(111 + wide_delta, 125, 
                             crosshair_type == 1 ? "Health" :
                             crosshair_type == 2 ? "Target highlighting" :
                             crosshair_type == 3 ? "Target highlighting+Health" :
                                                   "Static");
        dp_translation = NULL;

        // Increased size
        dp_translation = crosshair_scale ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(140 + wide_delta, 135, crosshair_scale ? RD_ON : RD_OFF);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(35 + wide_delta, 145, "next page >");
        M_WriteTextSmall_ENG(35 + wide_delta, 155, "< prev page");
        M_WriteTextSmall_ENG(231 + wide_delta, 155, "page 3/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Физика
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "Abpbrf");
        dp_translation = NULL;

        // Перемещение под/над монстрами
        dp_translation = over_under ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(274 + wide_delta, 35, over_under ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Трупы сползают с возвышений
        dp_translation = torque ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(256 + wide_delta, 45, torque ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Улучшенное покачивание оружия
        dp_translation = weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(271 + wide_delta, 55, weapon_bobbing ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Двустволка разрывает врагов
        dp_translation = ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(254 + wide_delta, 65, ssg_blast_enemies ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Зеркалирование трупов
        dp_translation = randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(207 + wide_delta, 75, randomly_flipcorpses ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Левитирующие сферы-артефакты
        dp_translation = floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(275 + wide_delta, 85, floating_powerups ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Подбрасывать выпавшие предметы
        dp_translation = toss_drop ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(286 + wide_delta, 95, toss_drop ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL; 

        //
        // Прицел
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 105, "Ghbwtk");
        dp_translation = NULL;

        // Отображать прицел
        dp_translation = crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(180 + wide_delta, 115, crosshair_draw ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Индикация
        dp_translation = crosshair_type ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(117 + wide_delta, 125, 
                             crosshair_type == 1 ? "Pljhjdmt" :       // Здоровье
                             crosshair_type == 2 ? "Gjlcdtnrf wtkb" : // Подсветка цели
                             crosshair_type == 3 ? "Gjlcdtnrf wtkb+pljhjdmt" : // Подсветка цели + здоровье
                                                   "Cnfnbxyfz");      // Статичная
        dp_translation = NULL;

        // Увеличенный размер
        dp_translation = crosshair_scale ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(185 + wide_delta, 135, crosshair_scale ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(35 + wide_delta, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + wide_delta, 155, RD_PREV_RUS);
        M_WriteTextSmall_RUS(197 + wide_delta, 155, "cnhfybwf 3*4");
        dp_translation = NULL;
    }
}

void M_RD_Draw_Gameplay_4(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    M_RD_Jaguar_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Gameplay
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 25, "Gameplay");
        dp_translation = NULL;

        // Fix errors of vanilla maps
        dp_translation = fix_map_errors ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 35, fix_map_errors ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Flip game levels
        dp_translation = flip_levels ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(158 + wide_delta, 45, flip_levels ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Extra player faces on the HUD
        dp_translation = extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(255 + wide_delta, 55, extra_player_faces ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Pain Elemental without Souls limit
        dp_translation = unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(284 + wide_delta, 65, unlimited_lost_souls ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // More agressive lost souls
        dp_translation = agressive_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(230 + wide_delta, 75, agressive_lost_souls ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Pistol start
        dp_translation = pistol_start ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(203 + wide_delta, 85, pistol_start ? RD_ON : RD_OFF);
        dp_translation = NULL;

        //
        // Demos
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(35 + wide_delta, 95, "Demos");
        dp_translation = NULL;

        // Show demo timer
        dp_translation = demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(153 + wide_delta, 105, demotimer == 1 ? "playback"  :
                                                    demotimer == 2 ? "recording" :
                                                    demotimer == 3 ? "always" :
                                                                     "off");
        dp_translation = NULL;

        // Timer direction
        dp_translation = demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(148 + wide_delta, 115, demotimerdir ? "backward" : "forward");
        dp_translation = NULL;

        // Show progress bar 
        dp_translation = demobar ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(169 + wide_delta, 125, demobar ? RD_ON : RD_OFF);
        dp_translation = NULL;

        // Play internal demos
        dp_translation = no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_ENG(183 + wide_delta, 135, no_internal_demos ? RD_OFF : RD_ON);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(35 + wide_delta, 145, "first page >");
        M_WriteTextSmall_ENG(35 + wide_delta, 155, "< prev page");
        M_WriteTextSmall_ENG(231 + wide_delta, 155, "page 4/4");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Геймплей
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 25, "Utqvgktq");
        dp_translation = NULL;

        // Устранять ошибки ориг. уровней
        dp_translation = fix_map_errors ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(269 + wide_delta, 35, fix_map_errors ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Устранять ошибки ориг. уровней
        dp_translation = flip_levels ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(263 + wide_delta, 45, flip_levels ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Дополнительные лица игрока в HUD
        dp_translation = extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(247 + wide_delta, 55, extra_player_faces ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Элементаль без ограничения Душ
        dp_translation = unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(274 + wide_delta, 65, unlimited_lost_souls ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Повышенная агрессивность Душ
        dp_translation = agressive_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(266 + wide_delta, 75, agressive_lost_souls ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Режим игры "Pistol start"
        M_WriteTextSmall_RUS(35 + wide_delta, 85, "ht;bv buhs ^");
        M_WriteTextSmall_ENG(121 + wide_delta, 85, "\"Pistol start\":");
        dp_translation = pistol_start ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(229 + wide_delta, 85, pistol_start ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        //
        // Демозаписи
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(35 + wide_delta, 95, "Ltvjpfgbcb");
        dp_translation = NULL;

        // Отображать таймер
        dp_translation = demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(180 + wide_delta, 105, demotimer == 1 ? "ghb ghjbuhsdfybb"  :
                                                   demotimer == 2 ? "ghb pfgbcb" :
                                                   demotimer == 3 ? "dctulf" :
                                                                    "dsrk");
        dp_translation = NULL;

        // Время таймера
        dp_translation = demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(145 + wide_delta, 115, demotimerdir ? "jcnfdittcz" : "ghjitlitt");
        dp_translation = NULL;

        // Шкала прогресса
        dp_translation = demobar ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(161 + wide_delta, 125, demobar ? RD_ON_RUS : RD_OFF_RUS);
        dp_translation = NULL;

        // Проигрывать демозаписи
        dp_translation = no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN];
        M_WriteTextSmall_RUS(219 + wide_delta, 135, no_internal_demos ? RD_OFF_RUS : RD_ON_RUS);
        dp_translation = NULL;

        //
        // Footer
        //
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(35 + wide_delta, 145, RD_NEXT_RUS);
        M_WriteTextSmall_RUS(35 + wide_delta, 155, RD_PREV_RUS);
        M_WriteTextSmall_RUS(197 + wide_delta, 155, "cnhfybwf 4*4");
        dp_translation = NULL;
    }
}

void M_RD_Change_Brightmaps(int choice)
{
    brightmaps ^= 1;
}

void M_RD_Change_FakeContrast(int choice)
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

void M_RD_Change_ColoredHUD(int choice)
{
    switch(choice)
    {
        case 0: 
        colored_hud--;
        if (colored_hud < 0) 
            colored_hud = 2;
        break;
    
        case 1:
        colored_hud++;
        if (colored_hud > 2)
            colored_hud = 0;
        break;
    }
    
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_ColoredBlood(int choice)
{
    colored_blood ^= 1;
}

void M_RD_Change_SwirlingLiquids(int choice)
{
    swirling_liquids ^= 1;
}

void M_RD_Change_InvulSky(int choice)
{
    invul_sky ^= 1;
}

void M_RD_Change_FlipWeapons(int choice)
{
    flip_weapons ^= 1;

    // [JN] Skip weapon bobbing interpolation for next frame.
    skippsprinterp = true;
}

void M_RD_Change_ZAxisSfx(int choice)
{
    z_axis_sfx ^= 1;
}

void M_RD_Change_ExitSfx(int choice)
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

void M_RD_Change_AlertSfx(int choice)
{
     noise_alert_sfx ^= 1;
}

void M_RD_Change_SecretNotify(int choice)
{
    secret_notification ^= 1;
}

void M_RD_Change_NegativeHealth(int choice)
{
    negative_health ^= 1;
}

void M_RD_Change_InfraGreenVisor(int choice)
{
    infragreen_visor ^= 1;

    // [JN] Update current COLORMAP
    if (players[consoleplayer].powers[pw_infrared])
    {
        if (players[consoleplayer].powers[pw_invulnerability])
        {
            // Set B&W colormap
            players[consoleplayer].fixedcolormap = 32;
        }
        else
        {
            // Set respective colormap
            players[consoleplayer].fixedcolormap = infragreen_visor ? 33 : 1;
        }
    }
}

void M_RD_Change_WalkOverUnder(int choice)
{
    over_under ^= 1;
}

void M_RD_Change_Torque(int choice)
{
    torque ^= 1;
}

void M_RD_Change_Bobbing(int choice)
{
    weapon_bobbing ^= 1;
}

void M_RD_Change_SSGBlast(int choice)
{
    ssg_blast_enemies ^= 1;
}

void M_RD_Change_FlipCorpses(int choice)
{
    randomly_flipcorpses ^= 1;
}

void M_RD_Change_FloatPowerups(int choice)
{
    floating_powerups ^= 1;
}

void M_RD_Change_TossDrop(int choice)
{
    toss_drop ^= 1;
}

void M_RD_Change_CrosshairDraw(int choice)
{
    crosshair_draw ^= 1;
}

void M_RD_Change_CrosshairType(int choice)
{
    switch(choice)
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
        break;
    }
}

void M_RD_Change_CrosshairScale(int choice)
{
    crosshair_scale ^= 1;
}

void M_RD_Change_FixMapErrors(int choice)
{
    fix_map_errors ^= 1;
}

void M_RD_Change_FlipLevels(int choice)
{
    flip_levels ^= 1;

    // Redraw game screen
    R_ExecuteSetViewSize();

    // Update stereo separation
    S_UpdateStereoSeparation();
}

void M_RD_Change_ExtraPlayerFaces(int choice)
{
    extra_player_faces ^= 1;
}

void M_RD_Change_LostSoulsQty(int choice)
{
    unlimited_lost_souls ^= 1;
}

void M_RD_Change_LostSoulsAgr(int choice)
{
    agressive_lost_souls ^= 1;
}

void M_RD_Change_PistolStart(int choice)
{
    pistol_start ^= 1;
}

void M_RD_Change_DemoTimer(int choice)
{
    switch(choice)
    {
        case 0: 
        demotimer--;
        if (demotimer < 0) 
            demotimer = 3;
        break;
    
        case 1:
        demotimer++;
        if (demotimer > 3)
            demotimer = 0;
        break;
    }
}

void M_RD_Change_DemoTimerDir(int choice)
{
    demotimerdir ^= 1;
}

void M_RD_Change_DemoBar(int choice)
{
    demobar ^= 1;

    // Refresh status bar
    if (gamestate == GS_LEVEL)
    {
        ST_doRefresh();
    }
}

void M_RD_Change_NoInternalDemos(int choice)
{
    no_internal_demos ^= 1;
}

// -----------------------------------------------------------------------------
// Level select
// -----------------------------------------------------------------------------

void M_RD_Choose_LevelSelect_1(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Level_Def_1 :
                    &RD_Level_Def_1_Rus);
}

void M_RD_Choose_LevelSelect_2(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Level_Def_2 :
                    &RD_Level_Def_2_Rus);
}

void M_RD_Draw_Level_1 (void)
{
    static char num[4];

    M_RD_Draw_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(2, "LEVEL SELECT");
        
        // Skill level
        M_snprintf(num, 4, "%d", selective_skill+1);
        M_WriteTextSmall_ENG(226 + wide_delta, 20, num);

        // Episode
        if (logical_gamemission != doom)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 30, "N/A");
            dp_translation = NULL;
        }
        else
        {
            if (gamemode == shareware)
            {
                dp_translation = cr[CR_DARKRED];
                M_WriteTextSmall_ENG(226 + wide_delta, 30, "1");
                dp_translation = NULL;
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                M_WriteTextSmall_ENG(226 + wide_delta, 30, num);
            }
        }

        // Map
        if (gamemode == pressbeta)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 40, "1");
            dp_translation = NULL;
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            M_WriteTextSmall_ENG(226 + wide_delta, 40, num);
        }

        //
        // Player
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(75 + wide_delta, 50, "PLAYER");
        dp_translation = NULL;

        // Health
        dp_translation = selective_health > 100 ? cr[CR_BLUE2] :
                         selective_health >= 67 ? cr[CR_GREEN] :
                         selective_health >= 34 ? cr[CR_GOLD]  :
                                                  cr[CR_RED];
        M_snprintf(num, 4, "%d", selective_health);
        M_WriteTextSmall_ENG(226 + wide_delta, 60, num);
        dp_translation = NULL;

        // Armor
        dp_translation = selective_armortype == 1 ? cr[CR_GREEN] :
                                                    cr[CR_BLUE2];
        if (selective_armor == 0) 
        dp_translation = cr[CR_RED];
        M_snprintf(num, 4, "%d", selective_armor);
        M_WriteTextSmall_ENG(226 + wide_delta, 70, num);
        dp_translation = NULL;

        // Armor type
        dp_translation = selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2];
        M_snprintf(num, 4, "%d", selective_armortype);
        M_WriteTextSmall_ENG(226 + wide_delta, 80, num);
        dp_translation = NULL;

        //
        // Weapons
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(75 + wide_delta, 90, "WEAPONS");
        dp_translation = NULL;

        // Chainsaw
        dp_translation = selective_wp_chainsaw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 100, selective_wp_chainsaw ? "YES" : "NO");
        dp_translation = NULL;

        // Shotgun
        dp_translation = selective_wp_shotgun ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 110, selective_wp_shotgun ? "YES" : "NO");
        dp_translation = NULL;

        // Super Shotgun
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 120, "N/A");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_supershotgun ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 120, selective_wp_supershotgun ? "YES" : "NO");
            dp_translation = NULL;
        }

        // Chaingun
        dp_translation = selective_wp_chaingun ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 130, selective_wp_chaingun ? "YES" : "NO");
        dp_translation = NULL;

        // Rocket Launcher
        dp_translation = selective_wp_missile ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 140, selective_wp_missile ? "YES" : "NO");
        dp_translation = NULL;

        // Plasma Gun
        if (gamemode == shareware)
        {
            // Not available in shareware
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 150, "N/A");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_plasma ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 150, selective_wp_plasma ? "YES" : "NO");
            dp_translation = NULL;
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 160, "N/A");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_bfg ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 160, selective_wp_bfg ? "YES" : "NO");
            dp_translation = NULL;
        }

        // Next page
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(75 + wide_delta, 180, "NEXT PAGE");

        // Start game
        M_WriteTextSmall_ENG(75 + wide_delta, 190, "START GAME");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(2, "DS<JH EHJDYZ");

        // Сложность
        M_snprintf(num, 4, "%d", selective_skill+1);
        M_WriteTextSmall_ENG(226 + wide_delta, 20, num);

        // Эпизод
        if (logical_gamemission != doom)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 30, "y*l");
            dp_translation = NULL;
        }
        else
        {
            if (gamemode == shareware)
            {
                dp_translation = cr[CR_DARKRED];
                M_WriteTextSmall_ENG(226 + wide_delta, 30, "1");
                dp_translation = NULL;
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                M_WriteTextSmall_ENG(226 + wide_delta, 30, num);
            }
        }

        // Уровень
        if (gamemode == pressbeta)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_ENG(226 + wide_delta, 40, "1");
            dp_translation = NULL;
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            M_WriteTextSmall_ENG(226 + wide_delta, 40, num);
        }

        //
        // Игрок
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(72 + wide_delta, 50, "buhjr");
        dp_translation = NULL;

        // Здоровье
        dp_translation = selective_health > 100 ? cr[CR_BLUE2] :
                         selective_health >= 67 ? cr[CR_GREEN] :
                         selective_health >= 34 ? cr[CR_GOLD]  :
                                                  cr[CR_RED];
        M_snprintf(num, 4, "%d", selective_health);
        M_WriteTextSmall_ENG(226 + wide_delta, 60, num);
        dp_translation = NULL;

        // Броня
        dp_translation = selective_armortype == 1 ? cr[CR_GREEN] :
                                                    cr[CR_BLUE2];
        if (selective_armor == 0) 
        dp_translation = cr[CR_RED];
        M_snprintf(num, 4, "%d", selective_armor);
        M_WriteTextSmall_ENG(226 + wide_delta, 70, num);
        dp_translation = NULL;

        // Тип брони
        dp_translation = selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2];
        M_snprintf(num, 4, "%d", selective_armortype);
        M_WriteTextSmall_ENG(226 + wide_delta, 80, num);
        dp_translation = NULL;

        //
        // Оружие
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(72 + wide_delta, 90, "jhe;bt");
        dp_translation = NULL;

        // Бензопила
        dp_translation = selective_wp_chainsaw ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 100, selective_wp_chainsaw ? "lf" : "ytn");
        dp_translation = NULL;

        // Ружье
        dp_translation = selective_wp_shotgun ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 110, selective_wp_shotgun ? "lf" : "ytn");
        dp_translation = NULL;

        // Двуствольное ружье
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 120, "y*l");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_supershotgun ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 120, selective_wp_supershotgun ? "lf" : "ytn");
            dp_translation = NULL;
        }

        // Пулемет
        dp_translation = selective_wp_chaingun ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 130, selective_wp_chaingun ? "lf" : "ytn");
        dp_translation = NULL;

        // Ракетница
        dp_translation = selective_wp_missile ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 140, selective_wp_missile ? "lf" : "ytn");
        dp_translation = NULL;

        // Плазменная пушка
        if (gamemode == shareware)
        {
            // Not available in shareware
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 150, "y*l");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_plasma ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 150, selective_wp_plasma ? "lf" : "ytn");
            dp_translation = NULL;
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 160, "y*l");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = selective_wp_bfg ? cr[CR_GREEN] : cr[CR_DARKRED];
            M_WriteTextSmall_RUS(226 + wide_delta, 160, selective_wp_bfg ? "lf" : "ytn");
            dp_translation = NULL;
        }

        // Следующая страница
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(72 + wide_delta, 180, "cktle.ofz cnhfybwf");

        // Начать игру
        M_WriteTextSmall_RUS(72 + wide_delta, 190, "yfxfnm buhe");
        dp_translation = NULL;
    }
}

void M_RD_Draw_Level_2 (void)
{
    static char num[4];

    M_RD_Draw_Menu_Background();

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(2, "LEVEL SELECT");

        // Backpack
        dp_translation = selective_backpack ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 20, selective_backpack ? "YES" : "NO");
        dp_translation = NULL;

        // Bullets
        dp_translation = selective_ammo_0 >   200 ? cr[CR_BLUE2] :
                         selective_ammo_0 >=  100 ? cr[CR_GREEN] :
                         selective_ammo_0 >=  50  ? cr[CR_GOLD]  :
                                                    cr[CR_RED]   ;
        M_snprintf(num, 4, "%d", selective_ammo_0);
        M_WriteTextSmall_ENG(226 + wide_delta, 30, num);
        dp_translation = NULL;

        // Shells
        dp_translation = selective_ammo_1 >   50 ? cr[CR_BLUE2] :
                         selective_ammo_1 >=  25 ? cr[CR_GREEN] :
                         selective_ammo_1 >=  12  ? cr[CR_GOLD] :
                                                    cr[CR_RED]  ;
        M_snprintf(num, 4, "%d", selective_ammo_1);
        M_WriteTextSmall_ENG(226 + wide_delta, 40, num);
        dp_translation = NULL;

        // Rockets
        dp_translation = selective_ammo_3 >   50 ? cr[CR_BLUE2] :
                         selective_ammo_3 >=  25 ? cr[CR_GREEN] :
                         selective_ammo_3 >=  12  ? cr[CR_GOLD] :
                                                    cr[CR_RED]  ;
        M_snprintf(num, 4, "%d", selective_ammo_3);
        M_WriteTextSmall_ENG(226 + wide_delta, 50, num);
        dp_translation = NULL;

        // Cells
        dp_translation = selective_ammo_2 >  300 ? cr[CR_BLUE2] :
                         selective_ammo_2 >= 150 ? cr[CR_GREEN] :
                         selective_ammo_2 >=  75 ? cr[CR_GOLD]  :
                                                   cr[CR_RED]   ;
        M_snprintf(num, 4, "%d", selective_ammo_2);
        M_WriteTextSmall_ENG(226 + wide_delta, 60, num);
        dp_translation = NULL;

        //
        // Keys
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(75 + wide_delta, 70, "KEYS");
        dp_translation = NULL;

        // Blue keycard
        dp_translation = selective_key_0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 80, selective_key_0 ? "YES" : "NO");
        dp_translation = NULL;

        // Yellow keycard
        dp_translation = selective_key_1 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 90, selective_key_1 ? "YES" : "NO");
        dp_translation = NULL;

        // Red keycard
        dp_translation = selective_key_2 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 100, selective_key_2 ? "YES" : "NO");
        dp_translation = NULL;

        // Blue skull key
        dp_translation = selective_key_3 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 110, selective_key_3 ? "YES" : "NO");
        dp_translation = NULL;

        // Yellow skull key
        dp_translation = selective_key_4 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 120, selective_key_4 ? "YES" : "NO");
        dp_translation = NULL;

        // Red skull key
        dp_translation = selective_key_5 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 130, selective_key_5 ? "YES" : "NO");
        dp_translation = NULL;

        //
        // Extra
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_ENG(75 + wide_delta, 140, "EXTRA");
        dp_translation = NULL;

        // Fast monsters
        dp_translation = selective_fast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 150, selective_fast ? "YES" : "NO");
        dp_translation = NULL;

        // Respawning monsters
        dp_translation = selective_respawn ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_ENG(226 + wide_delta, 160, selective_respawn ? "YES" : "NO");
        dp_translation = NULL;

        // Previous page
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_ENG(75 + wide_delta, 180, "PREVIOUS PAGE");

        // Start game
        M_WriteTextSmall_ENG(75 + wide_delta, 190, "START GAME");
        dp_translation = NULL;
    }
    else
    {
        M_WriteTextBigCentered_RUS(2, "DS<JH EHJDYZ");

        // Рюкзак
        dp_translation = selective_backpack ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 20, selective_backpack ? "lf" : "ytn");
        dp_translation = NULL;

        // Пули
        dp_translation = selective_ammo_0 >   200 ? cr[CR_BLUE2] :
                         selective_ammo_0 >=  100 ? cr[CR_GREEN] :
                         selective_ammo_0 >=  50  ? cr[CR_GOLD]  :
                                                    cr[CR_RED]   ;
        M_snprintf(num, 4, "%d", selective_ammo_0);
        M_WriteTextSmall_ENG(226 + wide_delta, 30, num);
        dp_translation = NULL;

        // Дробь
        dp_translation = selective_ammo_1 >   50 ? cr[CR_BLUE2] :
                         selective_ammo_1 >=  25 ? cr[CR_GREEN] :
                         selective_ammo_1 >=  12  ? cr[CR_GOLD] :
                                                    cr[CR_RED]  ;
        M_snprintf(num, 4, "%d", selective_ammo_1);
        M_WriteTextSmall_ENG(226 + wide_delta, 40, num);
        dp_translation = NULL;

        // Ракеты
        dp_translation = selective_ammo_3 >   50 ? cr[CR_BLUE2] :
                         selective_ammo_3 >=  25 ? cr[CR_GREEN] :
                         selective_ammo_3 >=  12  ? cr[CR_GOLD] :
                                                    cr[CR_RED]  ;
        M_snprintf(num, 4, "%d", selective_ammo_3);
        M_WriteTextSmall_ENG(226 + wide_delta, 50, num);
        dp_translation = NULL;

        // Энергия
        dp_translation = selective_ammo_2 >  300 ? cr[CR_BLUE2] :
                         selective_ammo_2 >= 150 ? cr[CR_GREEN] :
                         selective_ammo_2 >=  75 ? cr[CR_GOLD]  :
                                                   cr[CR_RED]   ;
        M_snprintf(num, 4, "%d", selective_ammo_2);
        M_WriteTextSmall_ENG(226 + wide_delta, 60, num);
        dp_translation = NULL;

        //
        // Ключи
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(72 + wide_delta, 70, "rk.xb");
        dp_translation = NULL;

        // Синяя ключ-карта
        dp_translation = selective_key_0 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 80, selective_key_0 ? "lf" : "ytn");
        dp_translation = NULL;

        // Желтая ключ-карта
        dp_translation = selective_key_1 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 90, selective_key_1 ? "lf" : "ytn");
        dp_translation = NULL;

        // Красная ключ-карта
        dp_translation = selective_key_2 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 100, selective_key_2 ? "lf" : "ytn");
        dp_translation = NULL;

        // Синий ключ-череп
        dp_translation = selective_key_3 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 110, selective_key_3 ? "lf" : "ytn");
        dp_translation = NULL;

        // Желтый ключ-череп
        dp_translation = selective_key_4 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 120, selective_key_4 ? "lf" : "ytn");
        dp_translation = NULL;

        // Красный ключ-череп
        dp_translation = selective_key_5 ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 130, selective_key_5 ? "lf" : "ytn");
        dp_translation = NULL;

        //
        // Монстры
        //
        dp_translation = cr[CR_GOLD];
        M_WriteTextSmall_RUS(72 + wide_delta, 140, "vjycnhs");
        dp_translation = NULL;

        // Ускоренные
        dp_translation = selective_fast ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 150, selective_fast ? "lf" : "ytn");
        dp_translation = NULL;

        // Воскрешающиеся
        dp_translation = selective_respawn ? cr[CR_GREEN] : cr[CR_DARKRED];
        M_WriteTextSmall_RUS(226 + wide_delta, 160, selective_respawn ? "lf" : "ytn");
        dp_translation = NULL;

        // Предыдущая страница
        dp_translation = cr[CR_GRAY];
        M_WriteTextSmall_RUS(72 + wide_delta, 180, "ghtlsleofz cnhfybwf");

        // Начать игру
        M_WriteTextSmall_RUS(72 + wide_delta, 190, "yfxfnm buhe");
        dp_translation = NULL;
    }
}

void M_RD_Change_Selective_Skill (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_skill > 0)
            selective_skill--;
        break;

        case 1:
        if (selective_skill < 5)
            selective_skill++;
        break;
    }
}

void M_RD_Change_Selective_Episode (int choice)
{
    // [JN] Shareware have only 1 episode, 
    // Doom 2 doest not have episodes at all.
    if (gamemode == shareware || gamemode == commercial)
    return;

    switch(choice)
    {
        case 0:
        if (selective_episode > 1)
            selective_episode--;
        break;

        case 1:
        if (selective_episode < (gamemode == registered ? 3 :
                                  gamemode == pressbeta ? 3 :
                                             sgl_loaded ? 5 : 
                                                          4))
            selective_episode++;
        break;
    }
}

void M_RD_Change_Selective_Map (int choice)
{
    // [JN] There are three episoder with one map for each in Press Beta.
    if (gamemode == pressbeta)
    return;

    switch(choice)
    {
        case 0:
        if (selective_map > 0)
            selective_map--;
        break;

        case 1:
        if (selective_map < (logical_gamemission == doom ? 9  :
                                   gamemission == jaguar ? 25 :
                                                           32))
            selective_map++;
        break;
    }
}

void M_RD_Change_Selective_Health (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_health > 1)
            selective_health--;
        break;

        case 1:
        if (selective_health < 200)
            selective_health++;
        break;
    }
}

void M_RD_Change_Selective_Armor (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_armor > 0)
            selective_armor--;
        break;

        case 1:
        if (selective_armor < 200)
            selective_armor++;
        break;
    }
}

void M_RD_Change_Selective_ArmorType (int choice)
{
    selective_armortype++;
    
    if (selective_armortype > 2)
        selective_armortype = 1;
}

void M_RD_Change_Selective_WP_Chainsaw (int choice)
{
    selective_wp_chainsaw ^= 1;
}

void M_RD_Change_Selective_WP_Shotgun (int choice)
{
    selective_wp_shotgun ^= 1;
}

void M_RD_Change_Selective_WP_SSgun (int choice)
{
    // Not available in Doom 1 and Jaguar
    if (logical_gamemission == doom || gamemission == jaguar)
    return;

    selective_wp_supershotgun ^= 1;
}

void M_RD_Change_Selective_WP_Chaingun (int choice)
{
    selective_wp_chaingun ^= 1;
}

void M_RD_Change_Selective_WP_RLauncher (int choice)
{
    selective_wp_missile ^= 1;
}

void M_RD_Change_Selective_WP_Plasmagun (int choice)
{
    // Not available in shareware
    if (gamemode == shareware)
    return;

    selective_wp_plasma ^= 1;
}

void M_RD_Change_Selective_WP_BFG9000 (int choice)
{
    // Not available in shareware
    if (gamemode == shareware)
    return;

    selective_wp_bfg ^= 1;
}

void M_RD_Change_Selective_Backpack (int choice)
{
    selective_backpack ^= 1;

    if (!selective_backpack)
    {
        if (selective_ammo_0 > 200)
            selective_ammo_0 = 200;
        if (selective_ammo_1 > 50)
            selective_ammo_1 = 50;
        if (selective_ammo_3 > 50)
            selective_ammo_3 = 50;
        if (selective_ammo_2 > 300)
            selective_ammo_2 = 300;
    }
}

void M_RD_Change_Selective_Ammo_0 (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_ammo_0 > 0)
            selective_ammo_0--;
        break;

        case 1:
        if (selective_ammo_0 < (selective_backpack ? 400 : 200))
            selective_ammo_0++;
        break;
    }
}

void M_RD_Change_Selective_Ammo_1 (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_ammo_1 > 0)
            selective_ammo_1--;
        break;

        case 1:
        if (selective_ammo_1 < (selective_backpack ? 100 : 50))
            selective_ammo_1++;
        break;
    }
}

void M_RD_Change_Selective_Ammo_2 (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_ammo_2 > 0)
            selective_ammo_2--;
        break;

        case 1:
        if (selective_ammo_2 < (selective_backpack ? 600 : 300))
            selective_ammo_2++;
        break;
    }
}

void M_RD_Change_Selective_Ammo_3 (int choice)
{
    switch(choice)
    {
        case 0:
        if (selective_ammo_3 > 0)
            selective_ammo_3--;
        break;

        case 1:
        if (selective_ammo_3 < (selective_backpack ? 100 : 50))
            selective_ammo_3++;
        break;
    }
}

void M_RD_Change_Selective_Key_0 (int choice)
{
    selective_key_0 ^= 1;
}

void M_RD_Change_Selective_Key_1 (int choice)
{
    selective_key_1 ^= 1;
}

void M_RD_Change_Selective_Key_2 (int choice)
{
    selective_key_2 ^= 1;
}

void M_RD_Change_Selective_Key_3 (int choice)
{
    selective_key_3 ^= 1;
}

void M_RD_Change_Selective_Key_4 (int choice)
{
    selective_key_4 ^= 1;
}

void M_RD_Change_Selective_Key_5 (int choice)
{
    selective_key_5 ^= 1;
}

void M_RD_Change_Selective_Fast (int choice)
{
    selective_fast ^= 1;
}

void M_RD_Change_Selective_Respawn (int choice)
{
    selective_respawn ^= 1;
}

// -----------------------------------------------------------------------------
// Back to Defaults
// -----------------------------------------------------------------------------

void M_RD_Choose_Reset(int choice)
{ // pitto
    M_SetupNextMenu(english_language ?
                    &RD_Reset_Def :
                    &RD_Reset_Def_Rus);
}

void M_RD_Draw_Reset(void)
{   
    if (english_language)
    {
        M_WriteTextSmallCentered_ENG(65, "Graphical, audible and gameplay settings");
        M_WriteTextSmallCentered_ENG(75, "will be reset to their default values.");
        M_WriteTextSmallCentered_ENG(85, "Which level of values to use?");

        // Explanations
        if (itemOn == rd_reset_recommended)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_ENG(145, "Settings will be reset to");
            M_WriteTextSmallCentered_ENG(155, "default port's values");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_ENG(145, "Settings will be reset to");
            M_WriteTextSmallCentered_ENG(155, "original Doom values");
            dp_translation = NULL;
        }
    }
    else
    {
        M_WriteTextSmallCentered_RUS(65, "Yfcnhjqrb uhfabrb< pderf b utqvgktz");     // Настройки графики, звука и геймплея
        M_WriteTextSmallCentered_RUS(75, ",elen c,hjitys yf cnfylfhnyst pyfxtybz>"); // Будут сброшены на стандартные значения.
        M_WriteTextSmallCentered_RUS(85, "Ds,thbnt ehjdtym pyfxtybq:");              // Выберите уровень значений:

        // Пояснения
        if (itemOn == rd_reset_recommended)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_RUS(145, ",elen bcgjkmpjdfys pyfxtybz");       // Будут использованы значения
            M_WriteTextSmallCentered_RUS(155, "htrjvtyletvst gjhnjv");              // рекомендуемые портом
            dp_translation = NULL;
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_RUS(145, ",elen bcgjkmpjdfys pyfxtybz");       // Будут использованы значения
            M_WriteTextSmall_RUS(85 + wide_delta, 155, "jhbubyfkmyjuj");            // оригинального Doom
            M_WriteTextSmall_ENG(193 + wide_delta, 155, "Doom");
            dp_translation = NULL;
        }
    }
}

void M_RD_BackToDefaults_Recommended(int choice)
{
    static char resetmsg[24];

    // Rendering
    vsync                   = 1;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    show_fps                = 0;
    smoothing               = 0;
    vga_porch_flash         = 0;
    force_software_renderer = 0;
    show_diskicon           = 1;
    screen_wiping           = 1;
    png_screenshots         = 1;

    // Display
    screenblocks          = 10;
    usegamma              = 4;
    level_brightness      = 0;
    menu_shading          = 0;
    detailLevel           = 0;
    local_time            = 0;
    showMessages          = 1;
    messages_timeout      = 4;
    messages_pickup_color = 0;
    messages_secret_color = 3;
    messages_system_color = 0;
    messages_chat_color   = 1;
    draw_shadowed_text    = 1;

    // Automap
    automap_color     = 0;
    automap_antialias = 1;
    automap_stats     = 1;
    automap_level_time = 1;
    automap_total_time = 0;
    automap_coords    = 0;
    automap_overlay   = 0;
    automap_rotate    = 0;
    automap_follow    = 1;
    automap_grid      = 0;
    automap_grid_size = 128;

    // Audio
    snd_samplerate = 44100;
    snd_sfxdevice        = 3;
        I_ShutdownSound();                  // Shut down sound system
        InitSfxModule(snd_sfxdevice);       // Reinitialize SFX module
        S_RD_Change_SoundDevice();          // Call sfx device changing routine
    snd_musicdevice      = 3;
    snd_dmxoption        = "-opl3";
        S_StopMusic();                      // Shut down current music
        S_Shutdown();                       // Shut down music system
        I_InitSound(true);                  // Start music system
        S_ChangeMusic(music_num_rd, true);  // Restart current music
    sfxVolume            = 8;
        S_SetSfxVolume(sfxVolume * 8);
    musicVolume          = 8;
        S_SetMusicVolume(musicVolume * 8);
    snd_channels_rd      = 32;
    snd_channels         = snd_channels_rd;
        S_ChannelsRealloc();
    snd_monomode         = 0;
    snd_pitchshift       = 0;
    mute_inactive_window = 0;

    // Controls
    joybspeed        = 29;
    mlook            = 0;  players[consoleplayer].centering = true;
    mouseSensitivity = 5;
    mouse_acceleration = 2.0;
    mouse_threshold  = 10;
    novert           = 1;

    // Gameplay: Graphical
    brightmaps       = 1;
    fake_contrast    = 0;
    translucency     = 1;
    improved_fuzz    = 2;
    colored_hud      = 0;
    colored_blood    = 1;
    swirling_liquids = 1;
    invul_sky        = 1;
    flip_weapons     = 0;

    // Gameplay: Audible
    z_axis_sfx           = 0;
    play_exit_sfx        = 0;
    crushed_corpses_sfx  = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx      = 0;
    // correct_endlevel_sfx = 0; (hidden variable)

    // Gameplay: Tactical
    secret_notification = 1;
    negative_health     = 0;
    infragreen_visor    = 0;

    // Gameplay: Physical
    over_under           = 0;
    torque               = 1;
    weapon_bobbing       = 1;
    ssg_blast_enemies    = 1;
    randomly_flipcorpses = 1;
    floating_powerups    = 0;
    toss_drop            = 1;

    // Gameplay: Crosshair
    crosshair_draw   = 0;
    crosshair_type   = 1;
    crosshair_scale  = 0;

    // Gameplay: Gameplay
    fix_map_errors       = 1;
    flip_levels          = 0;
    extra_player_faces   = 1;
    unlimited_lost_souls = 1;
    agressive_lost_souls = 0;
    pistol_start         = 0;
    fast_quickload       = 1;

    // Gameplay: Demos
    demotimer         = 0;
    demotimerdir      = 0;
    demobar           = 0;
    no_internal_demos = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update screen size and fuzz effect
    R_SetViewSize (screenblocks, detailLevel);

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
    players[consoleplayer].message_system = resetmsg;
}

void M_RD_BackToDefaults_Original(int choice)
{
    static char resetmsg[24];

    // Rendering
    vsync                   = 1;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 0;
    show_fps                = 0;
    smoothing               = 0;
    vga_porch_flash         = 0;
    force_software_renderer = 0;
    show_diskicon           = 1;
    screen_wiping           = 1;
    png_screenshots         = 1;

    // Display
    screenblocks          = 10;
    usegamma              = 9;
    level_brightness      = 0;
    menu_shading          = 0;
    detailLevel           = 1;
    local_time            = 0;
    showMessages          = 1;
    messages_timeout      = 4;
    messages_pickup_color = 0;
    messages_secret_color = 0;
    messages_system_color = 0;
    messages_chat_color   = 0;
    draw_shadowed_text    = 0;

    // Automap
    automap_color     = 0;
    automap_antialias = 0;
    automap_stats     = 0;
    automap_level_time = 0;
    automap_total_time = 0;
    automap_coords    = 0;
    automap_overlay   = 0;
    automap_rotate    = 0;
    automap_follow    = 1;
    automap_grid      = 0;
    automap_grid_size = 128;

    // Audio
    snd_samplerate = 44100;
    snd_sfxdevice        = 3;
        I_ShutdownSound();                  // Shut down sound system
        InitSfxModule(snd_sfxdevice);       // Reinitialize SFX module
        S_RD_Change_SoundDevice();          // Call sfx device changing routine
    snd_musicdevice      = 3;
    snd_dmxoption        = "-opl3";
        S_StopMusic();                      // Shut down current music
        S_Shutdown();                       // Shut down music system
        I_InitSound(true);                  // Start music system
        S_ChangeMusic(music_num_rd, true);  // Restart current music
    sfxVolume            = 8;
        S_SetSfxVolume(sfxVolume * 8);
    musicVolume          = 8;
        S_SetMusicVolume(musicVolume * 8);
    snd_channels_rd      = 8;  
    snd_channels         = snd_channels_rd;
        S_ChannelsRealloc();
    snd_monomode         = 0;
    snd_pitchshift       = 0;
    mute_inactive_window = 0;

    // Controls
    joybspeed          = 29;
    mlook              = 0;  players[consoleplayer].centering = true;
    mouseSensitivity   = 5;
    mouse_acceleration = 2.0;
    mouse_threshold    = 10;
    novert             = 1;

    // Gameplay: Graphical
    brightmaps       = 0;
    fake_contrast    = 1;
    translucency     = 0;
    improved_fuzz    = 0;
    colored_hud      = 0;
    colored_blood    = 0;
    swirling_liquids = 0;
    invul_sky        = 0;
    flip_weapons     = 0;

    // Gameplay: Audible
    z_axis_sfx           = 0;
    play_exit_sfx        = 1;
    crushed_corpses_sfx  = 0;
    blazing_door_fix_sfx = 0;
    noise_alert_sfx      = 0;
    // correct_endlevel_sfx = 0; (hidden variable)

    // Gameplay: Tactical
    secret_notification = 0;
    negative_health     = 0;
    infragreen_visor    = 0;

    // Gameplay: Physical
    over_under           = 0;
    torque               = 0;
    weapon_bobbing       = 0;
    ssg_blast_enemies    = 0;
    randomly_flipcorpses = 0;
    floating_powerups    = 0;
    toss_drop            = 0;

    // Gameplay: Crosshair
    crosshair_draw   = 0;
    crosshair_type   = 0;
    crosshair_scale  = 0;

    // Gameplay: Gameplay
    fix_map_errors       = 0;
    flip_levels          = 0;
    extra_player_faces   = 0;
    unlimited_lost_souls = 0;
    agressive_lost_souls = 0;
    pistol_start         = 0;
    fast_quickload       = 1;

    // Gameplay: Demos
    demotimer         = 0;
    demotimerdir      = 0;
    demobar           = 0;
    no_internal_demos = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update screen size and fuzz effect
    R_SetViewSize (screenblocks, detailLevel);

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
    players[consoleplayer].message_system = resetmsg;
}


// -----------------------------------------------------------------------------
// Language hot-swapping
// -----------------------------------------------------------------------------

void M_RD_ChangeLanguage(int choice)
{
    extern void ST_Stop(void);
    extern void HU_Stop(void);
    extern void F_CastDrawer(void);
    extern void F_CastDrawerJaguar(void);
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

    // Update finale sequence
    if (gamestate == GS_FINALE)
    {
        if (finalestage == 2)
        {
            // Just redraw monster's names
            F_CastDrawer();
        }
        else if (finalestage == 3)
        {
            // Just redraw monster's names (Jaguar)
            F_CastDrawerJaguar();
        }
        else
        {
            // Restart finale text output
            F_StartFinale();
        }
    }

    // Update window title
    I_SetWindowTitle(english_language ? gamedescription_eng : gamedescription_rus);
    I_InitWindowTitle();
}


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
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
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
        V_DrawShadowedPatchDoom(LoadDef_x + wide_delta, LoadDef_y,
                           W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));
    }
    else
    {
        // ЗАГРУЗИТЬ ИГРУ
        M_WriteTextBigCentered_RUS(LoadDef_y, "PFUHEPBNM BUHE");
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x + wide_delta,LoadDef.y+LINEHEIGHT*i);

        // [crispy] shade empty savegame slots
        if (!LoadMenu[i].status && colored_hud && !vanillaparm)
        dp_translation = cr[CR_DARKRED];

        M_WriteText(LoadDef.x + wide_delta,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);

        dp_translation = NULL;
    }
}


//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int i;

    V_DrawShadowedPatchDoom(x - 8, y + 8, W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0 ; i < 24 ; i++)
    {
        V_DrawShadowedPatchDoom(x, y + 8, W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawShadowedPatchDoom(x, y + 8, W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
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

    M_SetupNextMenu(english_language ? &LoadDef : &LoadDef_Rus);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 72, SaveDef_y = 28;  // [JN] from Crispy Doom
void M_DrawSave(void)
{
    int i;
	
    if (english_language)
    {
        // [JN] Use standard centered title "M_SAVEG"
        V_DrawShadowedPatchDoom(SaveDef_x + wide_delta, SaveDef_y, 
                                W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    }
    else
    {
        if (QuickSaveTitle) // БЫСТРОЕ СОХРАНЕНИЕ
        M_WriteTextBigCentered_RUS(SaveDef_y, "<SCNHJT CJ{HFYTYBT");
        else                // СОХРАНИТЬ ИГРУ
        M_WriteTextBigCentered_RUS(SaveDef_y, "CJ{HFYBNM BUHE");
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x + wide_delta,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x + wide_delta,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[saveSlot]);
        M_WriteText(LoadDef.x + i + wide_delta,LoadDef.y+LINEHEIGHT*saveSlot,"_");
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

    M_SetupNextMenu(english_language ? &SaveDef : &SaveDef_Rus);
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
        M_SetupNextMenu(english_language ? &SaveDef : &SaveDef_Rus);
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
    int skullx = 330, skully = 175;

    inhelpscreens = true;

    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: clean up wide screen remainings before drawing.
        V_DrawFilledBox(0, 0, WIDESCREENWIDTH, SCREENHEIGHT, 0);
    }

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
            if (english_language)
            lumpname = "HELP";
            else
            lumpname = "HELPR";
        
            skullx = 330;
            skully = 162;
        }

        else
        {
            // Doom 1
            // HELP2 is the first screen shown in Doom 1
            if (english_language)
            lumpname = "HELP2";
            else
            lumpname = "HELP2R";

            skullx = 280;
            skully = 185;
        }
        break;

        case exe_ultimate:
        case exe_chex:

        // Ultimate Doom always displays "HELP1".

        // Chex Quest version also uses "HELP1", even though it is based
        // on Final Doom.

        if (english_language)
        lumpname = "HELP1";
        else
        lumpname = "HELP1R";
        break;

        case exe_final:
        case exe_final2:

        // Final Doom always displays "HELP".
        // [JN] Иконка черепа сдвинута чуть выше, по аналогии Doom 2,
        // чтобы не загораживать фразу "джойстика 2".

        if (english_language)
        lumpname = "HELP";
        else
        lumpname = "HELPR";
    
        skullx = 330;
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
        skullx = 323;
        skully = 183;
    }

    // [JN] Pixel-perfect position for skull in Press Beta
    if (gamemode == pressbeta)
    {
            skullx = 330;
            skully = 175;
    }

    lumpname = DEH_String(lumpname);

    V_DrawPatch (wide_delta, 0, W_CacheLumpName(lumpname, PU_CACHE));

    ReadDef1.x = ReadDef1_Rus.x = skullx;
    ReadDef1.y = ReadDef1_Rus.y = skully;
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    // [JN] Do not show HELP1 screen again (from M_DrawReadThis1) 
    // in non-shareware versions. Fixes two "BackSpace" pressings.
    if (gamemode != shareware)
    {
        ReadDef2.prevMenu = &MainDef;
        ReadDef2_Rus.prevMenu = &MainDef_Rus;
    }

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before drawing
        V_DrawFilledBox(0, 0, WIDESCREENWIDTH, SCREENHEIGHT, 0);
    }

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered

    V_DrawPatch(wide_delta, 0, W_CacheLumpName(DEH_String
               (english_language ? "HELP1" : "HELP1R"), PU_CACHE));

    ReadDef2.x = ReadDef2_Rus.x = 330;
    ReadDef2.y = ReadDef2_Rus.y = 175;
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    if (english_language)
    {
        // [JN] Always draw original "M_DOOM" in English language
        V_DrawPatch(94+wide_delta, 2, 
                    W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
    }
    else
    {
        // [JN] Draw translated titles for Plutonia and TNT
        V_DrawPatch(94+wide_delta, 2, W_CacheLumpName
                    (logical_gamemission == pack_plut ? "RD_MPLUT" :
                      logical_gamemission == pack_tnt ? "RD_MTNT" :
                                                        "M_DOOM", PU_CACHE));
    }
}


//
// M_NewGame
//
void M_DrawNewGame(void)
{
    if (english_language)
    {
        V_DrawShadowedPatchDoom(96 + wide_delta, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54 + wide_delta, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
    }
    else
    {
        // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(13, "YJDFZ BUHF");
        // Уровень сложности:
        M_WriteTextBigCentered_RUS(38, "Ehjdtym ckj;yjcnb#");
    }
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
    M_SetupNextMenu(english_language ? &NewDef : &NewDef_Rus);
    else
    M_SetupNextMenu(english_language ? &EpiDef : &EpiDef_Rus);
}


//
// M_Episode
//
int epi;

void M_DrawEpisode(void)
{
    if (english_language)
    {
        V_DrawShadowedPatchDoom(96 + wide_delta, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54 + wide_delta, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
    }
    else
    {
        // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(13, "YJDFZ BUHF");
        // Какой эпизод?
        M_WriteTextBigCentered_RUS(38, "Rfrjq \'gbpjl?");
    }
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
    M_SetupNextMenu(english_language ? &NewDef : &NewDef_Rus);
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
    M_SetupNextMenu(english_language ? &ReadDef1 : &ReadDef1_Rus);
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
        M_SetupNextMenu(english_language ? &ReadDef2 : &ReadDef2_Rus);
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

    // [JN] Widescreen: remember choosen widescreen variable before quit.
    aspect_ratio = aspect_ratio_temp;

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
        V_DrawPatch((x + 8) + thermDot * 8, y,
                W_CacheLumpName(DEH_String("RD_THRMW"), PU_CACHE));
    }
    else
    {
        V_DrawPatch((x + 8) + thermDot * 8, y,
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
    messageToBind = 0;  // [JN] NOT binding key now
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


void M_RD_Key_Binding_Routine (int key)
{
    messageToBind = 0;

    if (keyToBind)
    {
        if (key == *keyToBind)
        {
            *keyToBind = 0;
        }
        else if (!(key == KEY_ESCAPE))
        {
            *keyToBind = key;
        }

        keyToBind = 0;
    }
}

//
// M_StartBinding
// [JN] Used for key binding routine.
//
void M_RD_StartBinding (int* key_var)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageToBind = 1;  // [JN] Binding key now
    keyToBind = key_var; // [Dasperal] Specify key to bind
    messageRoutine = M_RD_Key_Binding_Routine;
    messageNeedsInput = false;
    menuactive = true;
    return;
}

void M_RD_MouseKey_Binding_Routine (int key)
{
    messageToBind = 0;

    switch (key)
    {
        case 13:
        {
            if (*keyToBind == 0 && key == 13)
                *keyToBind = -1;
            else
                *keyToBind = 0;
            break;
        }
        case 127:
        {
            if (*keyToBind == 1 && key == 127)
                *keyToBind = -1;
            else
                *keyToBind = 1;
            break;
        }
        case 4:
        {
            if (*keyToBind == 2 && key == 4)
                *keyToBind = -1;
            else
                *keyToBind = 2;
            break;
        }
        case 8:
        case 16:
        case 173:
        {
            if ((*keyToBind == 3 && key == 8)
                ||  (*keyToBind == 3 && key == 16)
                ||  (*keyToBind == 3 && key == 173))
                *keyToBind = -1;
            else
                *keyToBind = 3;

            break;
        }
        case 175:
        {
            if (*keyToBind == 4 && key == 175)
                *keyToBind = -1;
            else
                *keyToBind = 4;
            break;
        }
        case KEY_ESCAPE:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

void M_RD_StartBinding_Mouse (int* key_var)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageToBind = 1;  // [JN] Binding key now
    keyToBind = key_var; // [Dasperal] Specify key to bind
    messageRoutine = M_RD_MouseKey_Binding_Routine;
    messageNeedsInput = false;
    menuactive = true;
    return;
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
    byte doom_next[5][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 51, 43},
    {52, 53, 54, 55, 56, 59, 58, 11, 57},
    };

    byte doom2_next[33] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
    };

    byte jaguar_next[26] = {
     2,  3, 24,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 25, 15, 16, 17, 18, 19, 20, 21,
    26, 23,  1,  4, 14, 22
    };

    byte nerve_next[9] = {
    2, 3, 4, 9, 6, 7, 8, 1, 5
  };

    int changed = false;

    if (gamemode == commercial)
    {
        /*
        if (crispy->havemap33)
        doom2_next[1] = 33;
        */

        if (W_CheckNumForName("map31") < 0)
        {
            doom2_next[14] = 16;
        }

        if (gamemission == pack_hacx)
        {
            doom2_next[30] = 16;
            doom2_next[20] = 1;
        }

        /*
        if (gamemission == pack_master)
        {
            doom2_next[1] = 3;
            doom2_next[14] = 16;
            doom2_next[20] = 1;
        }
        */
    }
    else
    {
        if (gamemode == shareware)
        {
            doom_next[0][7] = 11;
        }

        if (gamemode == registered)
        {
            doom_next[2][7] = 11;
        }

        if (!sgl_loaded)
        {
            doom_next[3][7] = 11;
        }

        if (gamemode == pressbeta)
        {
            if (gameepisode == 1)
                doom_next[0][0] = 21;
            if (gameepisode == 2)
                doom_next[1][0] = 31;
            if (gameepisode == 3)
                doom_next[2][0] = 11;
        }

        if (gameversion == exe_chex)
        {
            doom_next[0][2] = 14;
            doom_next[0][4] = 11;
        }
    }

    if (gamestate == GS_LEVEL)
    {
        int epsd, map;

        if (gamemode == commercial)
        {
            epsd = gameepisode;

            if (gamemission == pack_nerve)
            {
                map = nerve_next[gamemap-1];
            }
            else if (gamemission == jaguar)
            {
                map = jaguar_next[gamemap-1];
            }
            else
            {
                map = doom2_next[gamemap-1];
            }
        }
        else
        {
            epsd = doom_next[gameepisode-1][gamemap-1] / 10;
            map = doom_next[gameepisode-1][gamemap-1] % 10;
        }

        /*
        // [crispy] special-casing for E1M10 "Sewers" support
        if (crispy->havee1m10 && gameepisode == 1)
        {
            if (gamemap == 1)
            {
                map = 10;
            }
            else if (gamemap == 10)
            {
                epsd = 1;
                map = 2;
            }
        }
        */

        G_DeferedInitNew(gameskill, epsd, map);
        changed = true;
    }

    return changed;
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
    key_page_t*     keyPage;
    // [FG] disable menu control by mouse
    /*
    static int      mousey = 0;
    static int      lasty = 0;
    static int      mousex = 0;
    static int      lastx = 0;
    */

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

    // [JN] Remember game settings and quit immediately
    // in devparm mode by pressing "close" button on window.
    if (devparm && ev->type == ev_quit)
    {
        M_QuitDOOM(0);
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

        // [JN] Disallow to use joystick keys while binding keyboard keys
        if ((currentMenu == RD_Bindings_Menu_Def_1
        ||   currentMenu == RD_Bindings_Menu_Def_1_Rus) && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (itemOn == i)
                    return false;
        }
        if ((currentMenu == RD_Bindings_Menu_Def_2
        ||   currentMenu == RD_Bindings_Menu_Def_2_Rus) && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (itemOn == i)
                    return false;
        }
        if ((currentMenu == RD_Bindings_Menu_Def_3
        ||   currentMenu == RD_Bindings_Menu_Def_3_Rus) && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (itemOn == i)
                    return false;
        }
        if ((currentMenu == RD_Bindings_Menu_Def_4
        ||   currentMenu == RD_Bindings_Menu_Def_4_Rus) && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (itemOn == i)
                    return false;
        }

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
            // [FG] disable menu control by mouse
            /*
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
            */

            // [JN] Disallow to use mouse keys while binding keyboard keys
            if ((currentMenu == RD_Bindings_Menu_Def_1
            ||   currentMenu == RD_Bindings_Menu_Def_1_Rus) && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (itemOn == i)
                        return false;
            }
            if ((currentMenu == RD_Bindings_Menu_Def_2
            ||   currentMenu == RD_Bindings_Menu_Def_2_Rus) && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (itemOn == i)
                        return false;
            }
            if ((currentMenu == RD_Bindings_Menu_Def_3
            ||   currentMenu == RD_Bindings_Menu_Def_3_Rus) && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (itemOn == i)
                        return false;
            }
            if ((currentMenu == RD_Bindings_Menu_Def_4
            ||   currentMenu == RD_Bindings_Menu_Def_4_Rus) && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (itemOn == i)
                        return false;
            }

            // [JN] Catch all incoming data1 mouse events. Makes middle mouse button 
            // working for message interruption and for binding ability.
            if (ev->data1)
            {
                key = ev->data1;
                mousewait = I_GetTime() + 5;
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
                mousewait = I_GetTime() + 1;
            }
            else
            if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 1;
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

    // [JN] Do not close Save/Load menu after deleting a savegame.
    if (currentMenu != &SaveDef
    &&  currentMenu != &SaveDef_Rus
    &&  currentMenu != &LoadDef
    &&  currentMenu != &LoadDef_Rus    
    // [JN] Do not close bindings menu after binding key / mouse button.
    &&  currentMenu != RD_Bindings_Menu_Def_1
    &&  currentMenu != RD_Bindings_Menu_Def_1_Rus
    &&  currentMenu != RD_Bindings_Menu_Def_2
    &&  currentMenu != RD_Bindings_Menu_Def_2_Rus
    &&  currentMenu != RD_Bindings_Menu_Def_3
    &&  currentMenu != RD_Bindings_Menu_Def_3_Rus
    &&  currentMenu != RD_Bindings_Menu_Def_4
    &&  currentMenu != RD_Bindings_Menu_Def_4_Rus
    &&  currentMenu != &RD_Mouse_Bindings_Menu_Def
    &&  currentMenu != &RD_Mouse_Bindings_Menu_Def_Rus)
    {
        menuactive = false;
    }
    S_StartSound(NULL,sfx_swtchx);
    return true;
    }

    if ((devparm && key == key_menu_help) || (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot ();
        return true;
    }

    // [JN] Crosshair toggling. Disalloved in vanilla mode (no crosshair there).
    if (key == key_togglecrosshair && !vanillaparm)
    {
        static char crosshairmsg[24];

        crosshair_draw ^= 1;

        M_snprintf(crosshairmsg, sizeof(crosshairmsg),
                   crosshair_draw ? ststr_crosshair_on : ststr_crosshair_off);
        players[consoleplayer].message_system = crosshairmsg;
        S_StartSound(NULL,sfx_swtchn);

        return true;
    }

    // [JN] Toggle level flipping.
    if (key == key_togglefliplvls)
    {
        flip_levels ^= 1;
        R_ExecuteSetViewSize();         // Redraw game screen
        S_UpdateStereoSeparation();     // Update stereo separation
        S_StartSound(NULL, sfx_stnmov); // Play quiet sound

        return true;
    }

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_help)     // Help key
        {
        M_StartControlPanel ();

        if ( gamemode == retail )
            currentMenu = english_language ? &ReadDef2 : &ReadDef2_Rus;
        else
            currentMenu = english_language ? &ReadDef1 : &ReadDef1_Rus;

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

        // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
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

    // [JN] Allow gamma toggling even while active menu.
    if (key == key_menu_gamma)    // gamma toggle
    {
        usegamma++;
        if (usegamma > 17)
            usegamma = 0;

        I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                              "PALFIX" : "PLAYPAL"), PU_CACHE) +
                                              st_palette * 768);

        players[consoleplayer].message_system = DEH_String(english_language ? 
                                                           gammamsg[usegamma] : 
                                                           gammamsg_rus[usegamma]);
        return true;
    }

    // [JN] Allow screen decreasing even while active menu.
	if (key == key_menu_decscreen)      // Screen size down
	{
		if (automapactive || chat_on)
		return false;
		M_RD_Change_ScreenSize(0);
		S_StartSound(NULL,sfx_stnmov);
		return true;
	}

    // [JN] Allow screen increasing even while active menu.
	if (key == key_menu_incscreen) // Screen size up
	{
		if (automapactive || chat_on)
		return false;
		M_RD_Change_ScreenSize(1);
		S_StartSound(NULL,sfx_stnmov);
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
    // [JN] Also used for clearing keyboard bindings
    else if (key == KEY_DEL)
    {
        // [JN] Save/load menu
        if (currentMenu == &LoadDef
        ||  currentMenu == &LoadDef_Rus
        ||  currentMenu == &SaveDef
        ||  currentMenu == &SaveDef_Rus)
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

        //[Dasperal] Key bindings menus
        keyPage = getCurrentKeyPage();
        if(keyPage)
        {
            BK_ClearBinds(keyPage->keys[itemOn]);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }

        // [JN] Mouse bindings menu
        if (currentMenu == &RD_Mouse_Bindings_Menu_Def
        ||  currentMenu == &RD_Mouse_Bindings_Menu_Def_Rus)
        {
            if (itemOn == rd_mouse_bindings_attack)      { mousebfire = -1; }
            if (itemOn == rd_mouse_bindings_use)         { mousebuse = -1; }
            if (itemOn == rd_mouse_bindings_forward)     { mousebforward = -1; }
            if (itemOn == rd_mouse_bindings_backward)    { mousebbackward = -1; }
            if (itemOn == rd_mouse_bindings_strafeon)    { mousebstrafe = -1; }
            if (itemOn == rd_mouse_bindings_strafeleft)  { mousebstrafeleft = -1; }
            if (itemOn == rd_mouse_bindings_straferight) { mousebstraferight = -1; }
            if (itemOn == rd_mouse_bindings_prevweapon)  { mousebprevweapon = -1; }
            if (itemOn == rd_mouse_bindings_nextweapon)  { mousebnextweapon = -1; }

            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
    }

    // [JN] Scroll Gameplay features menu by PgUp/PgDn keys
    else if (key == KEY_PGUP)
    {
        currentMenu->lastOn = itemOn;

        // [JN] Keyboard bindings menu
        if (currentMenu == RD_Bindings_Menu_Def_1
        ||  currentMenu == RD_Bindings_Menu_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_4 :
                           RD_Bindings_Menu_Def_4_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_2
        ||  currentMenu == RD_Bindings_Menu_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_1 :
                           RD_Bindings_Menu_Def_1_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_3
        ||  currentMenu == RD_Bindings_Menu_Def_3_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_2 :
                           RD_Bindings_Menu_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_4
        ||  currentMenu == RD_Bindings_Menu_Def_4_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_3 :
                           RD_Bindings_Menu_Def_3_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }

        // [JN] Gameplay features menu
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

        if (currentMenu == &RD_Level_Def_1
        ||  currentMenu == &RD_Level_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Level_Def_2 :
                           &RD_Level_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }

        if (currentMenu == &RD_Level_Def_2
        ||  currentMenu == &RD_Level_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Level_Def_1 :
                           &RD_Level_Def_1_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
    }
    else if (key == KEY_PGDN)
    {
        currentMenu->lastOn = itemOn;

        // [JN] Keyboard bindings menu
        if (currentMenu == RD_Bindings_Menu_Def_1
        ||  currentMenu == RD_Bindings_Menu_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_2 :
                           RD_Bindings_Menu_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_2
        ||  currentMenu == RD_Bindings_Menu_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_3 :
                           RD_Bindings_Menu_Def_3_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_3
        ||  currentMenu == RD_Bindings_Menu_Def_3_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_4 :
                           RD_Bindings_Menu_Def_4_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == RD_Bindings_Menu_Def_4
        ||  currentMenu == RD_Bindings_Menu_Def_4_Rus)
        {
            M_SetupNextMenu(english_language ?
                           RD_Bindings_Menu_Def_1 :
                           RD_Bindings_Menu_Def_1_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }

        // [JN] Gameplay features menu
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

        if (currentMenu == &RD_Level_Def_1
        ||  currentMenu == &RD_Level_Def_1_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Level_Def_2 :
                           &RD_Level_Def_2_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }

        if (currentMenu == &RD_Level_Def_2
        ||  currentMenu == &RD_Level_Def_2_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Level_Def_1 :
                           &RD_Level_Def_1_Rus);
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
    currentMenu = english_language ? &MainDef : &MainDef_Rus;         // JDC
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
    // [JN] Do not invoke if we are binding key.
    if (messageToPrint && !messageToBind)
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

            if (english_language)
            {
                M_WriteText(x + wide_delta, y, string);
                y += SHORT(hu_font[0]->height);
            }
            else
            {
                M_WriteTextSmall_RUS(x + wide_delta, y, string);
                y += SHORT(hu_font_small_rus[0]->height);                
            }
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

        // -----------------------------------------------------------------
        // [JN] Write common menus by using standard graphical patches:
        // -----------------------------------------------------------------
        if (currentMenu == &MainDef                // Main Menu
        ||  currentMenu == &MainDef_Rus            // Main Menu
        ||  currentMenu == &MainDefBeta            // Main Menu (Press Beta) 
        ||  currentMenu == &MainDefBeta_Rus        // Main Menu (Press Beta)
        ||  currentMenu == &EpiDef                 // Episode selection
        ||  currentMenu == &EpiDef_Rus             // Episode selection
        ||  currentMenu == &NewDef                 // Skill level
        ||  currentMenu == &NewDef_Rus             // Skill level
        ||  currentMenu == &Vanilla_OptionsDef     // Vanilla options menu
        ||  currentMenu == &Vanilla_Audio_Def)     // Vanilla sound menu
        {
            // [JN] Draw patch if it's name is present,
            // i.e. don't try to draw placeholders as patches.
            if (name[0])
            V_DrawShadowedPatchDoom (x + wide_delta, y, W_CacheLumpName(name, PU_CACHE));

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
        if (currentMenu == &MainDef_Rus
        ||  currentMenu == &EpiDef_Rus
        ||  currentMenu == &NewDef_Rus
        ||  currentMenu == &RD_Options_Def_Rus
        ||  currentMenu == &Vanilla_OptionsDef_Rus
        ||  currentMenu == &Vanilla_Audio_Def_Rus)
        {
            M_WriteTextBig_RUS(x + wide_delta, y, name);

            // [JN] Big vertical spacing
            y += LINEHEIGHT;
        }
        // -----------------------------------------------------------------
        // [JN] Write English submenus with small English font
        // -----------------------------------------------------------------
        else
        if (currentMenu == &RD_Rendering_Def
        ||  currentMenu == &RD_Display_Def
        ||  currentMenu == &RD_Messages_Def
        ||  currentMenu == &RD_Automap_Def
        ||  currentMenu == &RD_Audio_Def
        ||  currentMenu == &RD_Audio_System_Def
        ||  currentMenu == &RD_Controls_Def
        ||  currentMenu == RD_Bindings_Menu_Def_1
        ||  currentMenu == RD_Bindings_Menu_Def_2
        ||  currentMenu == RD_Bindings_Menu_Def_3
        ||  currentMenu == RD_Bindings_Menu_Def_4
        ||  currentMenu == &RD_Mouse_Bindings_Menu_Def
        ||  currentMenu == &RD_Gameplay_Def_1
        ||  currentMenu == &RD_Gameplay_Def_2
        ||  currentMenu == &RD_Gameplay_Def_3
        ||  currentMenu == &RD_Gameplay_Def_4
        ||  currentMenu == &RD_Level_Def_1
        ||  currentMenu == &RD_Level_Def_2
        ||  currentMenu == &RD_Reset_Def)
        {
            M_WriteTextSmall_ENG(x + wide_delta, y, name);

            // [JN] Small vertical spacing
            y += LINEHEIGHT_SML;
        }
        // -----------------------------------------------------------------
        // [JN] Write Russian submenus with small Russian font
        // -----------------------------------------------------------------            
        else
        if (currentMenu == &RD_Rendering_Def_Rus
        ||  currentMenu == &RD_Display_Def_Rus
        ||  currentMenu == &RD_Messages_Def_Rus
        ||  currentMenu == &RD_Automap_Def_Rus
        ||  currentMenu == &RD_Audio_Def_Rus
        ||  currentMenu == &RD_Audio_System_Def_Rus
        ||  currentMenu == &RD_Controls_Def_Rus
        ||  currentMenu == RD_Bindings_Menu_Def_1_Rus
        ||  currentMenu == RD_Bindings_Menu_Def_2_Rus
        ||  currentMenu == RD_Bindings_Menu_Def_3_Rus
        ||  currentMenu == RD_Bindings_Menu_Def_4_Rus
        ||  currentMenu == &RD_Mouse_Bindings_Menu_Def_Rus
        ||  currentMenu == &RD_Gameplay_Def_1_Rus
        ||  currentMenu == &RD_Gameplay_Def_2_Rus
        ||  currentMenu == &RD_Gameplay_Def_3_Rus
        ||  currentMenu == &RD_Gameplay_Def_4_Rus
        ||  currentMenu == &RD_Level_Def_1_Rus
        ||  currentMenu == &RD_Level_Def_2_Rus
        ||  currentMenu == &RD_Reset_Def_Rus)
        {
            M_WriteTextSmall_RUS(x + wide_delta, y, name);
        
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
        // DRAW SKULL
        V_DrawShadowedPatchDoom(x + SKULLXOFF + wide_delta, currentMenu->y - 5 + itemOn*LINEHEIGHT,
        W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
    }
    else
    {
        // [JN] Draw blinking ">" symbol
        if (whichSkull == 0)
        dp_translation = cr[CR_DARKRED];
        // [JN] Jaguar: no font color translation, draw SKULL1 as an empty symbol.
        M_WriteTextSmall_ENG(x + SKULLXOFF + 24 + wide_delta, currentMenu->y + itemOn*LINEHEIGHT_SML,
                             gamemission == jaguar && whichSkull == 0 ? " " : ">");
        // [JN] Clear translation
        dp_translation = NULL;
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
    //[Dasperal] Init Bindings Menus
    RD_Bindings_Menu_Def_1 = getMenuFromKeyPage(&RD_Bindings_1, true);
    RD_Bindings_Menu_Def_1_Rus = getMenuFromKeyPage(&RD_Bindings_1, false);
    RD_Bindings_Menu_Def_2 = getMenuFromKeyPage(&RD_Bindings_2, true);
    RD_Bindings_Menu_Def_2_Rus = getMenuFromKeyPage(&RD_Bindings_2, false);
    RD_Bindings_Menu_Def_3 = getMenuFromKeyPage(&RD_Bindings_3, true);
    RD_Bindings_Menu_Def_3_Rus = getMenuFromKeyPage(&RD_Bindings_3, false);
    RD_Bindings_Menu_Def_4 = getMenuFromKeyPage(&RD_Bindings_4, true);
    RD_Bindings_Menu_Def_4_Rus = getMenuFromKeyPage(&RD_Bindings_4, false);

    currentMenu = english_language ? &MainDef : &MainDef_Rus;

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;

    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;

    if (aspect_ratio >= 2)
    screenSize = screenblocks - 9;
    else
    screenSize = screenblocks - 3;

    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: place screen size slider correctly at starup
        if (screenSize < 0)
            screenSize = 0;
        if (screenSize > 5)
            screenSize = 5;
    }

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    switch ( gamemode )
    {
        case commercial:
        // Commercial has no "read this" entry.
        MainMenu[readthis] = MainMenu[quitdoom];
        MainMenu_Rus[readthis] = MainMenu_Rus[quitdoom];
        MainDef.numitems--;
        MainDef_Rus.numitems--;
        MainDef.y += 8;
        MainDef_Rus.y += 8;
        NewDef.prevMenu = &MainDef;
        NewDef_Rus.prevMenu = &MainDef_Rus;
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
        MainDef_Rus = MainDefBeta_Rus;
        // [JN] Remove one lower menu item
        MainDef.numitems--;
        MainDef_Rus.numitems--;
        // [JN] Correct return to previous menu
        NewDef.prevMenu = &MainDef;
        NewDef_Rus.prevMenu = &MainDef_Rus;
        break;

        default:
        break;
    }

    // [JN] Move up Jaguar options menu to don't draw it over status bar
    if (gamemission == jaguar)
    {
        RD_Options_Def.y -= 6;
        RD_Options_Def_Rus.y -= 6;
    }

    // [crispy] & [JN] Sigil
    if (!sgl_loaded)
    {
        EpiDef.numitems = EpiDef_Rus.numitems = 4;
    }

    // Versions of doom.exe before the Ultimate Doom release only had
    // three episodes; if we're emulating one of those then don't try
    // to show episode four. If we are, then do show episode four
    // (should crash if missing).
    if (gameversion < exe_ultimate)
    {
        EpiDef.numitems--;
        EpiDef_Rus.numitems--;
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
    // [JN] Do not print savegame name in Russian language because of
    // lack of English symbols in message.
    if (english_language)
    {
        savegwarning =
            M_StringJoin("are you sure you want to\ndelete saved game\n\n\"",
            savegamestrings[itemOn], "\"?\n\n", PRESSYN, NULL);
    }
    else
    {
        savegwarning =
            M_StringJoin("ds ltqcndbntkmyj [jnbnt\nelfkbnm cj[hfytyye. buhe?\n\n",
            PRESSYN_RUS, NULL);
    }

    M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
    messageToPrint = 2;
    S_StartSound(NULL,sfx_swtchn);
}


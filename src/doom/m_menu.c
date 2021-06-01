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
#include "am_map.h"         // [JN] AM_initColors();
#include "st_stuff.h"

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

// [JN] Choosen message colors, used in HUlib_drawTextLine.
byte   *messages_pickup_color_set;
byte   *messages_secret_color_set;
byte   *messages_system_color_set;
byte   *messages_chat_color_set;

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
    // [JN] Routine 3 is same to 2, but not playing activity sound itself.
    // Activity sounds are invoking by called functions / actions themselves.
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
void M_WriteTextSmall_ENG(int x, int y, char *string, byte *cr);
void M_WriteTextSmall_RUS(int x, int y, char *string, byte *cr);
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
void M_RD_Change_HUD_Detail(int choice);
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
void M_RD_Choose_Gameplay_5(int choice);
void M_RD_Draw_Gameplay_1(void);
void M_RD_Draw_Gameplay_2(void);
void M_RD_Draw_Gameplay_3(void);
void M_RD_Draw_Gameplay_4(void);
void M_RD_Draw_Gameplay_5(void);

void M_RD_Change_Brightmaps(int choice);
void M_RD_Change_FakeContrast(int choice);
void M_RD_Change_Translucency(int choice);
void M_RD_Change_ImprovedFuzz(int choice);
void M_RD_Change_ColoredBlood(int choice);
void M_RD_Change_SwirlingLiquids(int choice);
void M_RD_Change_InvulSky(int choice);
void M_RD_Change_LinearSky(int choice);
void M_RD_Change_FlipWeapons(int choice);

void M_RD_Change_ExtraPlayerFaces(int choice);
void M_RD_Change_NegativeHealth(int choice);
void M_RD_Change_SBarColored(int choice);
void M_RD_Change_SBarHighValue(int choice);
void M_RD_Change_SBarNormalValue(int choice);
void M_RD_Change_SBarLowValue(int choice);
void M_RD_Change_SBarCriticalValue(int choice);
void M_RD_Change_SBarArmorType1(int choice);
void M_RD_Change_SBarArmorType2(int choice);
void M_RD_Change_SBarArmorType0(int choice);

void M_RD_Change_ZAxisSfx(int choice);
void M_RD_Change_ExitSfx(int choice);
void M_RD_Change_CrushingSfx(int choice);
void M_RD_Change_BlazingSfx(int choice);
void M_RD_Change_AlertSfx(int choice);
void M_RD_Change_SecretNotify(int choice);
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

void M_WriteTextSmall_ENG (int x, int y, char *string, byte *cr)
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

        dp_translation = cr;
        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_eng[c]);
        dp_translation = NULL;

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextSmall_ENG
//
// [JN] Write a string using a small STCFS font
// -----------------------------------------------------------------------------

void M_WriteTextSmall_RUS (int x, int y, char *string, byte *cr)
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

        dp_translation = cr;
        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_rus[c]);
        dp_translation = NULL;

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

        V_DrawShadowedPatchDoom(cx-wide_delta, cy, hu_font_big_eng[c]);

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

        V_DrawShadowedPatchDoom(cx-wide_delta, cy, hu_font_big_rus[c]);

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
// M_RD_ColorTranslation
// [JN] Returns a color translation for given variable.
// -----------------------------------------------------------------------------

static byte *M_RD_ColorTranslation (int color)
{
    switch (color)
    {
        case 1:   return cr[CR_DARKRED];    break;
        case 2:   return cr[CR_GREEN];      break;
        case 3:   return cr[CR_DARKGREEN];  break;
        case 4:   return cr[CR_OLIVE];      break;
        case 5:   return cr[CR_BLUE2];      break;
        case 6:   return cr[CR_DARKBLUE];   break;
        case 7:   return cr[CR_YELLOW];     break;
        case 8:   return cr[CR_ORANGE];     break;
        case 9:   return cr[CR_WHITE];      break;
        case 10:  return cr[CR_GRAY];       break;
        case 11:  return cr[CR_DARKGRAY];   break;
        case 12:  return cr[CR_TAN];        break;
        case 13:  return cr[CR_BROWN];      break;
        case 14:  return cr[CR_ALMOND];     break;
        case 15:  return cr[CR_KHAKI];      break;
        case 16:  return cr[CR_PINK];       break;
        case 17:  return cr[CR_BURGUNDY];   break;
        default:  return NULL;              break;
    }
}

// -----------------------------------------------------------------------------
// M_RD_ColorName_ENG
// [JN] Returns a color name for given variable, in English language.
// -----------------------------------------------------------------------------

static char *M_RD_ColorName_ENG (int color)
{
    switch (color)
    {
        case 1:   return "dark red";    break;
        case 2:   return "green";       break;
        case 3:   return "dark green";  break;
        case 4:   return "olive";       break;
        case 5:   return "blue";        break;
        case 6:   return "dark blue";   break;
        case 7:   return "yellow";      break;
        case 8:   return "orange";      break;
        case 9:   return "white";       break;
        case 10:  return "gray";        break;
        case 11:  return "dark gray";   break;
        case 12:  return "tan";         break;
        case 13:  return "brown";       break;
        case 14:  return "almond";      break;
        case 15:  return "khaki";       break;
        case 16:  return "pink";        break;
        case 17:  return "burgundy";    break;
        default:  return "red";         break;
    }
}

// -----------------------------------------------------------------------------
// M_RD_ColorName_RUS
// [JN] Returns a color name for given variable, in Russian language.
// -----------------------------------------------------------------------------

static char *M_RD_ColorName_RUS (int color)
{
    switch (color)
    {
        case 1:   return "ntvyj-rhfcysq";  break; // Тёмно-красный
        case 2:   return "ptktysq";        break; // Зеленый
        case 3:   return "ntvyj-ptktysq";  break; // Тёмно-зеленый
        case 4:   return "jkbdrjdsq";      break; // Оливковый
        case 5:   return "cbybq";          break; // Синий
        case 6:   return "ntvyj-cbybq";    break; // Тёмно-синий
        case 7:   return ";tknsq";         break; // Жёлтый
        case 8:   return "jhfy;tdsq";      break; // Оранжевый
        case 9:   return ",tksq";          break; // Белый
        case 10:  return "cthsq";          break; // Серый
        case 11:  return "ntvyj-cthsq";    break; // Тёмно-серый
        case 12:  return ",t;tdsq";        break; // Бежевый
        case 13:  return "rjhbxytdsq";     break; // Коричневый
        case 14:  return "vbylfkmysq";     break; // Миндальный
        case 15:  return "[frb";           break; // Хаки
        case 16:  return "hjpjdsq";        break; // Розовый
        case 17:  return ",jhljdsq";       break; // Бордовый
        default:  return "rhfcysq";        break; // Красный
    }
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
    {3, "Vertical synchronization:", M_RD_Change_VSync,       'v'},
    {2, "Frame rate:",               M_RD_Change_Uncapped,    'f'},
    {2, "Show FPS counter:",         M_RD_Change_FPScounter,  's'},
    {3, "Pixel scaling:",            M_RD_Change_Smoothing,   'p'},
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
    {3, "Dthnbrfkmyfz cby[hjybpfwbz:",     M_RD_Change_VSync,       'd'}, // Вертикальная синхронизация
    {2, "Rflhjdfz xfcnjnf:",               M_RD_Change_Uncapped,    'r'}, // Кадровая частота
    {2, "Cxtnxbr rflhjdjq xfcnjns:",       M_RD_Change_FPScounter,  'c'}, // Счетчик кадровой частоты
    {3, "Gbrctkmyjt cukf;bdfybt:",         M_RD_Change_Smoothing,   'g'}, // Пиксельное сглаживание
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
    rd_display_hud,
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
    {3, "screen size",         M_RD_Change_ScreenSize,      's'},
    {-1,"",0,'\0'},
    {3, "gamma-correction",    M_RD_Change_Gamma,           'g'},
    {-1,"",0,'\0'},
    {3, "level brightness",    M_RD_Change_LevelBrightness, 'l'},
    {-1,"",0,'\0'},
    {3, "menu shading",        M_RD_Change_MenuShading,     'm'},
    {-1,"",0,'\0'},
    {2, "graphics detail:",    M_RD_Change_Detail,          'g'},
    {2, "hud background detail:", M_RD_Change_HUD_Detail,   'h'},
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
    {3, "hfpvth buhjdjuj \'rhfyf",  M_RD_Change_ScreenSize,      'h'}, // Размер игрового экрана
    {-1,"",0,'\0'},                                                    //
    {3, "ehjdtym ufvvf-rjhhtrwbb",  M_RD_Change_Gamma,           'e'}, // Уровень гамма-коррекции
    {-1,"",0,'\0'},                                                    //
    {3, "ehjdtym jcdtotyyjcnb",     M_RD_Change_LevelBrightness, 'e'}, // Уровень освещенности
    {-1,"",0,'\0'},                                                    //
    {3, "pfntvytybt ajyf vty.",     M_RD_Change_MenuShading,     'p'}, // Затемнение фона меню
    {-1,"",0,'\0'},                                                    //
    {2, "ltnfkbpfwbz uhfabrb:",     M_RD_Change_Detail,          'l'}, // Детализация графики:
    {2, "ltnfkbpfwbz ajyf",         M_RD_Change_HUD_Detail,      'l'}, // Детализация фона HUD:
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
    rd_display_localtime,
    rd_messages_empty3,
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
    {3, "message timeout",     M_RD_Change_Msg_TimeOut,     'm'},
    {-1,"",0,'\0'},
    {2, "text casts shadows:", M_RD_Change_ShadowedText,    't'},
    {-1,"",0,'\0'},
    {2, "local time:",         M_RD_Change_LocalTime,       'l'},
    {-1,"",0,'\0'},
    {3, "item pickup:",        M_RD_Change_Msg_Pickup_Color,'i'},
    {3, "revealed secret:",    M_RD_Change_Msg_Secret_Color,'r'},
    {3, "system message:",     M_RD_Change_Msg_System_Color,'s'},
    {3, "netgame chat:",       M_RD_Change_Msg_Chat_Color,  'n'},
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
    {3, "nfqvfen jnj,hf;tybz",      M_RD_Change_Msg_TimeOut,     'n'}, // Таймаут отображения
    {-1,"",0,'\0'},
    {2, "ntrcns jn,hfcsdf.n ntym:", M_RD_Change_ShadowedText,    'n'}, // Тексты отбрасывают тень:
    {-1,"",0,'\0'},
    {2, "cbcntvyjt dhtvz:",         M_RD_Change_LocalTime,       'c'}, // Системное время:
    {-1,"",0,'\0'},
    {3, "gjkextybt ghtlvtnjd:",     M_RD_Change_Msg_Pickup_Color,'g'}, // Получение предметов:
    {3, "j,yfhe;tybt nfqybrjd:",    M_RD_Change_Msg_Secret_Color,'j'}, // Обнаружение тайников:
    {3, "cbcntvyst cjj,otybz:",     M_RD_Change_Msg_System_Color,'c'}, // Системные сообщения:
    {3, "xfn ctntdjq buhs:",        M_RD_Change_Msg_Chat_Color,  'x'}, // Чат сетевой игры:
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
    {3, "color scheme:",      M_RD_Change_AutomapColor,     'c'},
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
    {3, "wdtnjdfz c[tvf:",    M_RD_Change_AutomapColor,     'w'}, // Цветовая схема:
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
    {3, "sfx volume",              M_RD_Change_SfxVol,      's'},
    {-1,"",0,'\0'},
    {3, "music volume",            M_RD_Change_MusicVol,    'm'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {3, "sound channels",          M_RD_Change_SfxChannels, 's'},
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
    {3, "pder",                         M_RD_Change_SfxVol,      'p'}, // Звук
    {-1,"",0,'\0'},                                                    //
    {3, "vepsrf",                       M_RD_Change_MusicVol,    'v'}, // Музыка
    {-1,"",0,'\0'},                                                    //
    {-1,"",0,'\0'},                                                    //
    {3, "Pderjdst rfyfks",              M_RD_Change_SfxChannels, 'p'}, // Звуковые каналы
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
    {3, "sensivity",          M_RD_Change_Sensitivity,    'm'},
    {-1,"",0,'\0'},
    {3, "acceleration",       M_RD_Change_Acceleration,   'a'},
    {-1,"",0,'\0'},
    {3, "acceleration threshold", M_RD_Change_Threshold,  't'},
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
    {3, "crjhjcnm",                  M_RD_Change_Sensitivity, 'c'}, // Скорость
    {-1,"",0,'\0'},                                                 //
    {3, "frctkthfwbz",               M_RD_Change_Acceleration,'f'}, // Акселерация
    {-1,"",0,'\0'},
    {3, "gjhju frctkthfwbb",         M_RD_Change_Threshold,   'g'}, // Порог акселерации
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
    rd_gameplay_1_colored_blood,
    rd_gameplay_1_swirling_liquids,
    rd_gameplay_1_invul_sky,
    rd_gameplay_1_linear_sky,
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
    rd_gameplay_2_extra_player_faces,
    rd_gameplay_2_negative_health,
    rd_gameplay_2_sbar_colored,
    rd_gameplay_2_empty1,
    rd_gameplay_2_high_value,
    rd_gameplay_2_normal_value,
    rd_gameplay_2_low_value,
    rd_gameplay_2_critical_value,
    rd_gameplay_2_armor_type_1,
    rd_gameplay_2_armor_type_2,
    rd_gameplay_2_armor_type_0,
    rd_gameplay_2_next_page,
    rd_gameplay_2_last_page,
    rd_gameplay_2_end
} rd_gameplay_2_e;

enum
{
    rd_gameplay_3_z_axis_sfx,
    rd_gameplay_3_play_exit_sfx,
    rd_gameplay_3_crushed_corpses_sfx,
    rd_gameplay_3_blazing_door_fix_sfx,
    rd_gameplay_3_noise_alert_sfx,
    rd_gameplay_3_empty1,
    rd_gameplay_3_secret_notification,
    rd_gameplay_3_infragreen_visor,
    rd_gameplay_3_empty2,
    rd_gameplay_3_empty3,
    rd_gameplay_3_empty4,
    rd_gameplay_3_next_page,
    rd_gameplay_3_prev_page,
    rd_gameplay_3_end
} rd_gameplay_3_e;

enum
{
    rd_gameplay_4_over_under,
    rd_gameplay_4_torque,
    rd_gameplay_4_weapon_bobbing,
    rd_gameplay_4_ssg_blast_enemies,
    rd_gameplay_4_randomly_flipcorpses,
    rd_gameplay_4_floating_powerups,
    rd_gameplay_4_toss_drop,
    rd_gameplay_4_empty1,
    rd_gameplay_4_crosshair_draw,
    rd_gameplay_4_crosshair_type,
    rd_gameplay_4_crosshair_scale,
    rd_gameplay_4_next_page,
    rd_gameplay_4_prev_page,
    rd_gameplay_4_end
} rd_gameplay_4_e;

enum
{
    rd_gameplay_5_fix_map_errors,
    rd_gameplay_5_flip_levels,
    rd_gameplay_5_unlimited_lost_souls,
    rd_gameplay_5_agressive_lost_souls,
    rd_gameplay_5_pistol_start,
    rd_gameplay_5_empty1,
    rd_gameplay_5_demotimer,
    rd_gameplay_5_demotimerdir,
    rd_gameplay_5_demobar,
    rd_gameplay_5_no_internal_demos,
    rd_gameplay_5_empty2,
    rd_gameplay_5_first_page,
    rd_gameplay_5_prev_page,
    rd_gameplay_5_end
} rd_gameplay_5_e;

// ------------
// English menu
// ------------

menuitem_t RD_Gameplay_Menu_1[]=
{
    {2,"Brightmaps:",                  M_RD_Change_Brightmaps,     'b'},
    {2,"Fake contrast:",               M_RD_Change_FakeContrast,   'f'},
    {2,"Translucency:",                M_RD_Change_Translucency,   't'},
    {2,"Fuzz effect:",                 M_RD_Change_ImprovedFuzz,   'f'},
    {2,"Colored blood and corpses:",   M_RD_Change_ColoredBlood,   'c'},
    {2,"Swirling liquids:",            M_RD_Change_SwirlingLiquids,'s'},
    {2,"Invulnerability affects sky:", M_RD_Change_InvulSky,       'i'},
    {2,"Sky drawing mode:",            M_RD_Change_LinearSky,      's'},
    {2,"Flip weapons:",                M_RD_Change_FlipWeapons,    'f'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"", /* Next Page > */           M_RD_Choose_Gameplay_2,     'n'},
    {1,"", /* < Last Page */           M_RD_Choose_Gameplay_5,     'l'},
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
    {2,"Extra player faces:",       M_RD_Change_ExtraPlayerFaces,  'e'},
    {2,"Show negative health:",     M_RD_Change_NegativeHealth,    's'},
    {2,"Colored elements:",         M_RD_Change_SBarColored,       'c'},
    {-1,"",0,'\0'},
    {3,"",                          M_RD_Change_SBarHighValue,     'h'},
    {3,"",                          M_RD_Change_SBarNormalValue,   'n'},
    {3,"",                          M_RD_Change_SBarLowValue,      'l'},
    {3,"",                          M_RD_Change_SBarCriticalValue, 'c'},
    {3,"",                          M_RD_Change_SBarArmorType1,    'a'},
    {3,"",                          M_RD_Change_SBarArmorType2,    'a'},
    {3,"",                          M_RD_Change_SBarArmorType0,    'n'},
    {1,"", /* Next Page > */        M_RD_Choose_Gameplay_3,        'n'},
    {1,"", /* < Prev page */        M_RD_Choose_Gameplay_1,        'p'},
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
    {2,"Sound attenuation axises:",       M_RD_Change_ZAxisSfx,        's'},
    {2,"Play exit sounds:",               M_RD_Change_ExitSfx,         'p'},
    {2,"Sound of crushing corpses:",      M_RD_Change_CrushingSfx,     's'},
    {2,"Single sound of blazing door:",   M_RD_Change_BlazingSfx,      's'},
    {2,"Monster alert waking up others:", M_RD_Change_AlertSfx,        'm'},
    {-1,"",0,'\0'},
    {2,"Notify of revealed secrets:",     M_RD_Change_SecretNotify,    'n'},
    {2,"Infragreen light amp. visor:",    M_RD_Change_InfraGreenVisor, 'i'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"", /* Next page >   */            M_RD_Choose_Gameplay_4,      'n'},
    {1,"", /* < Prev page > */            M_RD_Choose_Gameplay_2,      'p'},
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
    {1,"", /* Next page >   */                M_RD_Choose_Gameplay_5,      'n'},
    {1,"", /* < Prev page > */                M_RD_Choose_Gameplay_3,      'p'},
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

menuitem_t RD_Gameplay_Menu_5[]=
{
    {2,"Fix errors of vanilla maps:",         M_RD_Change_FixMapErrors,     'f'},
    {2,"Flip game levels:",                   M_RD_Change_FlipLevels,       'f'},
    {2,"Pain Elemental without Souls limit:", M_RD_Change_LostSoulsQty,     'p'},
    {2,"More aggressive lost souls:",         M_RD_Change_LostSoulsAgr,     'm'},
    {2,"Pistol start game mode:",             M_RD_Change_PistolStart,      's'},
    {-1,"",0,'\0'},
    {2,"Show demo timer:",                    M_RD_Change_DemoTimer,        's'},
    {2,"timer direction:",                    M_RD_Change_DemoTimerDir,     't'},
    {2,"Show progress bar:",                  M_RD_Change_DemoBar,          's'},
    {2,"Play internal demos:",                M_RD_Change_NoInternalDemos,  'p'},
    {-1,"",0,'\0'},
    {1,"", /* First page >   */               M_RD_Choose_Gameplay_1,       'n'},
    {1,"", /* < Prev page > */                M_RD_Choose_Gameplay_4,       'p'},
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_5 =
{
    rd_gameplay_5_end,
    &RD_Options_Def,
    RD_Gameplay_Menu_5,
    M_RD_Draw_Gameplay_5,
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
    {2,"Hfpyjwdtnyfz rhjdm b nhegs:",       M_RD_Change_ColoredBlood,   'h'},   // Разноцветная кровь и трупы
    {2,"ekexityyfz fybvfwbz ;blrjcntq:",    M_RD_Change_SwirlingLiquids,'e'},   // Улучшенная анимация жидкостей
    {2,"ytezpdbvjcnm jrhfibdftn yt,j:",     M_RD_Change_InvulSky,       'y'},   // Неуязвимость окрашивает небо
    {2,"ht;bv jnhbcjdrb yt,f:",             M_RD_Change_LinearSky,      'h'},   // Режим отрисовки неба
    {2,"pthrfkmyjt jnhf;tybt jhe;bz:",      M_RD_Change_FlipWeapons,    'p'},   // Зеркальное отражение оружия
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"",                                  M_RD_Choose_Gameplay_2,     'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_5,     'y'},   // < Назад
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
    {2,"Ljgjkybntkmyst kbwf buhjrf:",       M_RD_Change_ExtraPlayerFaces,  'l'},    // Дополнительные лица игрока
    {2,"jnhbwfntkmyjt pljhjdmt:",           M_RD_Change_NegativeHealth,    'j'},    // Отрицательное здоровье
    {2,"Hfpyjwdtnyst 'ktvtyns:",            M_RD_Change_SBarColored,       'h'},    // Разноцветные элементы
    {-1,"",0,'\0'},
    {3,"",                 M_RD_Change_SBarHighValue,     'd'},    // Высокое значение
    {3,"",              M_RD_Change_SBarNormalValue,   'y'},    // Нормальное значение
    {3,"",                  M_RD_Change_SBarLowValue,      'y'},    // Низкое значение
    {3,"",             M_RD_Change_SBarCriticalValue, 'r'},    // Критическое значение
    {3,"",                      M_RD_Change_SBarArmorType1,    'n'},    // Тип брони 1
    {3,"",                      M_RD_Change_SBarArmorType2,    'n'},    // Тип брони 2
    {3,"",                 M_RD_Change_SBarArmorType0,    'k'},    // Отсутствие брони
    {1,"",                                  M_RD_Choose_Gameplay_3,        'l'},    // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_1,        'y'},    // < Назад
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
    {2,"pfne[fybt pderf gj jczv:",          M_RD_Change_ZAxisSfx,           'p'},   // Затухание звука по осям
    {2,"Pderb ghb ds[jlt bp buhs:",         M_RD_Change_ExitSfx,            'p'},   // Звук при выходе из игры
    {2,"Pder hfplfdkbdfybz nhegjd:",        M_RD_Change_CrushingSfx,        'p'},   // Звук раздавливания трупов
    {2,"Jlbyjxysq pder ,scnhjq ldthb:",     M_RD_Change_BlazingSfx,         'j'},   // Одиночный звук быстрой двери
    {2,"J,ofz nhtdjuf e vjycnhjd:",         M_RD_Change_AlertSfx,           'j'},   // Общая тревога у монстров
    {-1,"",0,'\0'},                                                                 //
    {2,"Cjj,ofnm j yfqltyyjv nfqybrt:",     M_RD_Change_SecretNotify,       'c'},   // Сообщать о найденном тайнике
    {2,"Byahfptktysq dbpjh jcdtotybz:",     M_RD_Change_InfraGreenVisor,    'b'},   // Инфразеленый визор освещения
    {-1,"",0,'\0'},                                                                 //
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
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
    {1,"",                                  M_RD_Choose_Gameplay_5,         'l'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_3,         'y'},   // < Назад
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

menuitem_t RD_Gameplay_Menu_5_Rus[]=
{
    {2,"ecnhfyznm jib,rb jhbu> ehjdytq:",   M_RD_Change_FixMapErrors,       'e'},   // Устранять ошибки ориг. уровней
    {2,"pthrfkmyjt jnhf;tybt ehjdytq:",     M_RD_Change_FlipLevels,         'p'},   // Зеркальное отражение уровней
    {2,"'ktvtynfkm ,tp juhfybxtybz lei:",   M_RD_Change_LostSoulsQty,       '\''},  // Элементаль без ограничения душ
    {2,"gjdsityyfz fuhtccbdyjcnm lei:",     M_RD_Change_LostSoulsAgr,       'g'},   // Повышенная агрессивность душ
    {2,"", /* [JN] Joint EN/RU string */    M_RD_Change_PistolStart,        'y'},   // Режим игры "Pistol start"
    {-1,"",0,'\0'},
    {2,"jnj,hf;fnm nfqvth:",                M_RD_Change_DemoTimer,          's'},   // Отображать таймер
    {2,"dhtvz nfqvthf:",                    M_RD_Change_DemoTimerDir,       's'},   // Время таймера
    {2,"irfkf ghjuhtccf:",                  M_RD_Change_DemoBar,            'g'},   // Шкала прогресса
    {2,"Ghjbuhsdfnm ltvjpfgbcb:",           M_RD_Change_NoInternalDemos,    'g'},   // Проигрывать демозаписи
    {-1,"",0,'\0'},
    {1,"",                                  M_RD_Choose_Gameplay_1,         'n'},   // Далее >
    {1,"",                                  M_RD_Choose_Gameplay_4,         'p'},   // < Назад
    {-1,"",0,'\0'}
};

menu_t  RD_Gameplay_Def_5_Rus =
{
    rd_gameplay_5_end,
    &RD_Options_Def_Rus,
    RD_Gameplay_Menu_5_Rus,
    M_RD_Draw_Gameplay_5,
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
    {3, "episode",          M_RD_Change_Selective_Episode,      'e'},
    {3, "map",              M_RD_Change_Selective_Map,          'm'},
    {-1,"",0,'\0'},         // Player
    {2, "health",           M_RD_Change_Selective_Health,       'h'},
    {2, "armor",            M_RD_Change_Selective_Armor,        'a'},
    {2, "armor type",       M_RD_Change_Selective_ArmorType,    'a'},
    {-1,"",0,'\0'},         // Weapons
    {2, "chainsaw",         M_RD_Change_Selective_WP_Chainsaw,  'c'},
    {2, "shotgun",          M_RD_Change_Selective_WP_Shotgun,   's'},
    {3, "super shotgun",    M_RD_Change_Selective_WP_SSgun,     's'},
    {2, "chaingun",         M_RD_Change_Selective_WP_Chaingun,  'c'},
    {2, "rocket launcher",  M_RD_Change_Selective_WP_RLauncher, 'r'},
    {3, "plasmagun",        M_RD_Change_Selective_WP_Plasmagun, 'p'},
    {3, "bfg9000",          M_RD_Change_Selective_WP_BFG9000,   'b'},
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
    {3, "\'gbpjl",          M_RD_Change_Selective_Episode,      'e'}, // Эпизод
    {3, "ehjdtym",          M_RD_Change_Selective_Map,          'm'}, // Уровень
    {-1,"",0,'\0'},         // Игрок
    {2, "pljhjdmt",         M_RD_Change_Selective_Health,       'h'}, // Здоровье
    {2, ",hjyz",            M_RD_Change_Selective_Armor,        'a'}, // Броня
    {2, "nbg ,hjyb",        M_RD_Change_Selective_ArmorType,    'a'}, // Тип брони
    {-1,"",0,'\0'},         // Оружие
    {2, ",typjgbkf",        M_RD_Change_Selective_WP_Chainsaw,  'c'}, // Бензопила
    {2, "he;mt",            M_RD_Change_Selective_WP_Shotgun,   's'}, // Ружье
    {3, "ldecndjkmyjt he;mt",M_RD_Change_Selective_WP_SSgun,    's'}, // Двуствольное ружье
    {2, "gektvtn",          M_RD_Change_Selective_WP_Chaingun,  'c'}, // Пулемет
    {2, "hfrtnybwf",        M_RD_Change_Selective_WP_RLauncher, 'r'}, // Ракетница
    {3, "gkfpvtyyfz geirf", M_RD_Change_Selective_WP_Plasmagun, 'p'}, // Плазменная пушка
    {3, "&9000",            M_RD_Change_Selective_WP_BFG9000,   'b'}, // BFG9000
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
    
    M_WriteTextBig_RUS(206, 53, showMessages == 1 ? "drk/" : "dsrk/");

    M_WriteTextBig_RUS(224, 69, detailLevel == 1 ? "ybp/" : "dsc/");
    }

    // - Screen size slider ----------------------------------------------------
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        M_DrawThermo(60, 102, 6, screenSize);
    }
    else
    {
        M_DrawThermo(60, 102, 12, screenSize);
    }

    // - Mouse sensivity slider ------------------------------------------------
    M_DrawThermo(60, 134, 10, mouseSensitivity);
}

void M_Vanilla_DrawSound(void)
{
    if (english_language)
    {
    // - "Sound volume" title --------------------------------------------------
    V_DrawPatch (60, 38, W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));
    }
    else
    {
    // - "ГРОМКОСТЬ" title -----------------------------------------------------
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
    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
    inhelpscreens = true;

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "RENDERING OPTIONS");

        //
        // Rendering
        //
        M_WriteTextSmall_ENG(35, 25, "Rendering", cr[CR_YELLOW]);

        // Widescreen rendering
        M_WriteTextSmall_ENG(185, 35, aspect_ratio_temp == 1 ? "5:4" :
                                                   aspect_ratio_temp == 2 ? "16:9" :
                                                   aspect_ratio_temp == 3 ? "16:10" :
                                                   aspect_ratio_temp == 4 ? "21:9" : "4:3", NULL);
        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_WHITE];
            M_WriteTextSmallCentered_ENG(156, "Program must be restarted");
            dp_translation = NULL;
        }

        // Vertical synchronization
        if (force_software_renderer == 1)
        {
            M_WriteTextSmall_ENG(216, 45, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(216, 45, vsync ? "on" : "off", NULL);
        }

        // Frame rate
        M_WriteTextSmall_ENG(120, 55, uncapped_fps ? "uncapped" : "35 fps", NULL);

        // Show FPS counter
        M_WriteTextSmall_ENG(162, 65, show_fps ? "on" : "off", NULL);

        // Pixel scaling
        if (force_software_renderer == 1)
        {
            M_WriteTextSmall_ENG(135, 75, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(135, 75, smoothing ? "smooth" : "sharp", NULL);
        }

        // Porch palette changing
        M_WriteTextSmall_ENG(207, 85, vga_porch_flash ? "on" : "off", NULL);

        // Video renderer
        M_WriteTextSmall_ENG(146, 95, force_software_renderer ? "software (cpu)" : "hardware (gpu)", NULL);

        //
        // Extra
        //
        M_WriteTextSmall_ENG(35, 105, "Extra", cr[CR_YELLOW]);

        // Show disk icon
        M_WriteTextSmall_ENG(138, 115, show_diskicon ? "on" : "off", NULL);

        // Screen wiping effect
        M_WriteTextSmall_ENG(187, 125, screen_wiping == 1 ? "standard" :
                                                    screen_wiping == 2 ? "loading" :
                                                                         "off", NULL);

        // Screenshot format
        M_WriteTextSmall_ENG(174, 135, png_screenshots ? "png" : "pcx", NULL);

        // Show ENDOOM screen
        M_WriteTextSmall_ENG(179, 145, show_endoom ? "on" : "off", NULL);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB DBLTJ"); // НАСТРОЙКИ ВИДЕО

        //
        // Рендеринг
        //
        M_WriteTextSmall_RUS(35, 25, "htylthbyu", cr[CR_YELLOW]);

        // Широкоформатный режим
        M_WriteTextSmall_RUS(238, 35, aspect_ratio_temp == 1 ? "5:4" :
                                      aspect_ratio_temp == 2 ? "16:9" :
                                      aspect_ratio_temp == 3 ? "16:10" :
                                      aspect_ratio_temp == 4 ? "21:9" :
                                                               "4:3", NULL);

        // Informative message: Необходим перезапуск программы
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_WHITE];
            M_WriteTextSmallCentered_RUS(156, "ytj,[jlbv gthtpfgecr ghjuhfvvs");
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer == 1)
        {
            M_WriteTextSmall_RUS(249, 45, "y*l", cr[CR_DARKRED]); // Н/Д
        }
        else
        {
            M_WriteTextSmall_RUS(249, 45, vsync ? "drk" : "dsrk", NULL);
        }

        // Кадровая частота
        if (uncapped_fps)
        {
            M_WriteTextSmall_RUS(167, 55, ",tp juhfybxtybz", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(167, 55, "35 fps", NULL);
        }

        // Счетчик кадровой частоты
        M_WriteTextSmall_RUS(227, 65, show_fps ? "drk" : "dsrk", NULL);

        // Пиксельное сглаживание
        if (force_software_renderer == 1)
        {
            M_WriteTextSmall_RUS(219, 75, "y*l", cr[CR_DARKRED]); // Н/Д
        }
        else
        {
            M_WriteTextSmall_RUS(219, 75, smoothing ? "drk" : "dsrk", NULL);
        }

        // Изменение палитры краёв экрана
        M_WriteTextSmall_RUS(274, 85, vga_porch_flash ? "drk" : "dsrk", NULL);

        // Обработка видео
        M_WriteTextSmall_RUS(160, 95, force_software_renderer ? "ghjuhfvvyfz" : "fggfhfnyfz", NULL);
        M_WriteTextSmall_ENG((force_software_renderer ? 254 : 244), 95, 
                              force_software_renderer ? "(cpu)" : "(gpu)", NULL);

        //
        // Дополнительно
        //
        M_WriteTextSmall_RUS(35, 105, "ljgjkybntkmyj", cr[CR_YELLOW]);

        // Отображать значок дискеты
        M_WriteTextSmall_RUS(241, 115, show_diskicon ? "drk" : "dsrk", NULL);

        // Эффект смены экранов
        M_WriteTextSmall_RUS(202, 125, screen_wiping == 1 ? "cnfylfhnysq" :
                                                    screen_wiping == 2 ? "pfuheprf" :
                                                                         "dsrk", NULL);

        // Формат скриншотов
        M_WriteTextSmall_ENG(180, 135, png_screenshots ? "png" : "pcx", NULL);

        // Показывать экран ENDOOM
        M_WriteTextSmall_ENG(165, 145, "ENDOOM:", NULL);
        M_WriteTextSmall_RUS(222, 145, show_endoom ? "drk" : "dsrk", NULL);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
    inhelpscreens = true;

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "DISPLAY OPTIONS");

        //
        // Screen
        //
        M_WriteTextSmall_ENG(35, 25, "Screen", dp_translation = cr[CR_YELLOW]);

        // Graphics detail
        M_WriteTextSmall_ENG(150, 115, detailLevel ? "low" : "high", NULL);

        // HUD background detail
        M_WriteTextSmall_ENG(199, 125, hud_detaillevel ? "low" : "high", NULL);

        //
        // Interface
        //
        M_WriteTextSmall_ENG(35, 135, "Interface", dp_translation = cr[CR_YELLOW]);

    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB \"RHFYF"); // НАСТРОЙКИ ЭКРАНА

        //
        // Экран
        //
        M_WriteTextSmall_RUS(35, 25, "\'rhfy", cr[CR_YELLOW]);

        // Детализация графики
        M_WriteTextSmall_RUS(195, 115, detailLevel ? "ybprfz" : "dscjrfz", NULL);

        // Детализация фона HUD
        M_WriteTextSmall_ENG(167, 125, "HUD: b", NULL);
        M_WriteTextSmall_RUS(199, 125, hud_detaillevel ? "ybprfz" : "dscjrfz", NULL);

        //
        // Интерфейс
        //
        M_WriteTextSmall_RUS(35, 135, "bynthatqc", cr[CR_YELLOW]);
    }

    // Screen size slider
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        M_DrawThermo_Small(35, 44, 6, screenSize);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        M_WriteTextSmall_ENG(96, 45, num, NULL);
    }
    else
    {
        M_DrawThermo_Small(35, 44, 12, screenSize);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        M_WriteTextSmall_ENG(145, 45, num, NULL);
    }

    // Gamma-correction slider
    M_DrawThermo_Small(35, 64, 18, usegamma);

    // Level brightness slider
    M_DrawThermo_Small(35, 84, 5, level_brightness / 16);

    // Level brightness slider
    M_DrawThermo_Small(35, 104, 7, menu_shading / 4);
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
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (screenSize < 11)
        {
            screenblocks++;
            screenSize++;
            if (screenblocks <= 14)
            {
                // [JN] Routine №3: play sound only if necessary.
                S_StartSound(NULL,sfx_stnmov);
            }
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
        {
            usegamma--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (usegamma < 17) 
        {
            usegamma++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
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
        {
            level_brightness -= 16;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    
        case 1:
        if (level_brightness < 64)
        {
            level_brightness += 16;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    }
}

void M_RD_Change_MenuShading(int choice)
{
    switch(choice)
    {
        case 0:
        if (menu_shading > 0)
        {
            menu_shading -= 4;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    
        case 1:
        if (menu_shading < 24)
        {
            menu_shading += 4;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
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

void M_RD_Change_HUD_Detail(int choice)
{
    extern boolean setsizeneeded;

    hud_detaillevel ^= 1;

    // [JN] Update screen border.
    setsizeneeded = true;

    // [JN] Refresh status bar.
    inhelpscreens = true;
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

        M_WriteTextSmall_ENG(35, 25, "General", dp_translation = cr[CR_YELLOW]);

        // Messages
        M_WriteTextSmall_ENG(165, 35, showMessages ? "on" : "off", NULL);

        // Message timeout. Print "second" or "seconds", depending of ammount.
        // [JN] Note: using M_StringJoin could be a smarter way,
        // but using it will make a notable delay in drawing routine, so here:
        M_WriteTextSmall_ENG(133, 55, messages_timeout == 1 ? "1 second" :
                                      messages_timeout == 2 ? "2 seconds" :
                                      messages_timeout == 3 ? "3 seconds" :
                                      messages_timeout == 4 ? "4 seconds" :
                                      messages_timeout == 5 ? "5 seconds" :
                                      messages_timeout == 6 ? "6 seconds" :
                                      messages_timeout == 7 ? "7 seconds" :
                                      messages_timeout == 8 ? "8 seconds" :
                                      messages_timeout == 9 ? "9 seconds" :
                                                              "10 seconds", NULL);

        // Text casts shadows
        M_WriteTextSmall_ENG(177, 65, draw_shadowed_text ? "on" : "off", NULL);

        M_WriteTextSmall_ENG(35, 75, "Misc.", dp_translation = cr[CR_YELLOW]);


        // Local time
        M_WriteTextSmall_ENG(116 , 85, local_time == 1 ? "12-hour (hh:mm)"    :
                                       local_time == 2 ? "12-hour (hh:mm:ss)" :
                                       local_time == 3 ? "24-hour (hh:mm)"    :
                                       local_time == 4 ? "24-hour (hh:mm:ss)" :
                                                         "off", NULL);

        M_WriteTextSmall_ENG(35, 95, "Colors", dp_translation = cr[CR_YELLOW]);

        // Item pickup
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(120, 105, "n/a", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(120, 105, M_RD_ColorName_ENG(message_pickup_color), 
                                                        M_RD_ColorTranslation(message_pickup_color));
        }

        // Revealed secret
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(157, 115, "n/a", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(157, 115, M_RD_ColorName_ENG(message_secret_color),
                                           M_RD_ColorTranslation(message_secret_color));
        }

        // System message
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(149, 125, "n/a", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(149, 125, M_RD_ColorName_ENG(message_system_color),
                                           M_RD_ColorTranslation(message_system_color));
        }

        // Netgame chat
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(136, 135, "n/a", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(136, 135, M_RD_ColorName_ENG(message_chat_color),
                                           M_RD_ColorTranslation(message_chat_color));
        }
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "CJJ<OTYBZ B NTRCNS"); // СООБЩЕНИЯ И ТЕКСТЫ

        //
        // Основное
        //
        M_WriteTextSmall_RUS(35, 25, "jcyjdyjt", cr[CR_YELLOW]);

        // Отображение сообщений
        M_WriteTextSmall_RUS(214, 35, showMessages ? "drk" : "dsrk", NULL);

        // Таймаут отображения. Печатать секунд(а/ы) в зависимости от количества.
        M_WriteTextSmall_RUS(133, 55, messages_timeout == 1 ? "1 ctreylf" :
                                      messages_timeout == 2 ? "2 ctreyls" :
                                      messages_timeout == 3 ? "3 ctreyls" :
                                      messages_timeout == 4 ? "4 ctreyls" :
                                      messages_timeout == 5 ? "5 ctreyl"  :
                                      messages_timeout == 6 ? "6 ctreyl"  :
                                      messages_timeout == 7 ? "7 ctreyl"  :
                                      messages_timeout == 8 ? "8 ctreyl"  :
                                      messages_timeout == 9 ? "9 ctreyl"  :
                                                              "10 ctreyl", NULL);

        // Тексты отбрасывают тень
        M_WriteTextSmall_RUS(226, 65, draw_shadowed_text ? "drk" : "dsrk", NULL);

        //
        // Разное
        //
        M_WriteTextSmall_RUS(35, 75, "hfpyjt", cr[CR_YELLOW]);

        // Системное время
        M_WriteTextSmall_RUS(161, 85, local_time == 1 ? "12-xfcjdjt (xx:vv)"    :
                                      local_time == 2 ? "12-xfcjdjt (xx:vv:cc)" :
                                      local_time == 3 ? "24-xfcjdjt (xx:vv)"    :
                                      local_time == 4 ? "24-xfcjdjt (xx:vv:cc)" :
                                                        "dsrk", NULL);

        //
        // Цвета
        //
        M_WriteTextSmall_RUS(35, 95, "wdtnf", cr[CR_YELLOW]);

        // Получение предметов
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(193, 105, "y*l", NULL); // н/д
        }
        else
        {
            M_WriteTextSmall_RUS(193, 105, M_RD_ColorName_RUS(message_pickup_color),
                                           M_RD_ColorTranslation(message_pickup_color));
        }

        // Обнаружение тайников
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(203, 115, "y*l", NULL); // н/д
        }
        else
        {
            M_WriteTextSmall_RUS(203, 115, M_RD_ColorName_RUS(message_secret_color),
                                                        M_RD_ColorTranslation(message_secret_color));
        }

        // Системные сообщения
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(197, 125, "y*l", NULL); // н/д
        }
        else
        {
            M_WriteTextSmall_RUS(197, 125, M_RD_ColorName_RUS(message_system_color),
                                                        M_RD_ColorTranslation(message_system_color));
        }

        // Чат сетевой игры
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(164, 135, "y*l", NULL); // н/д
        }
        else
        {
            M_WriteTextSmall_RUS(164, 135, M_RD_ColorName_RUS(message_chat_color),
                                                        M_RD_ColorTranslation(message_chat_color));
        }
    }

    // Message timeout slider
    M_DrawThermo_Small(35, 54, 10, messages_timeout - 1);
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
        {
            messages_timeout--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (messages_timeout < 10)
        {
            messages_timeout++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    }
}

void M_RD_Define_Msg_Pickup_Color(void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        messages_pickup_color_set = NULL;
    }
    else
    {
        switch (message_pickup_color)
        {
            case 1:   messages_pickup_color_set = cr[CR_DARKRED];    break;
            case 2:   messages_pickup_color_set = cr[CR_GREEN];      break;
            case 3:   messages_pickup_color_set = cr[CR_DARKGREEN];  break;
            case 4:   messages_pickup_color_set = cr[CR_OLIVE];      break;
            case 5:   messages_pickup_color_set = cr[CR_BLUE2];      break;
            case 6:   messages_pickup_color_set = cr[CR_DARKBLUE];   break;
            case 7:   messages_pickup_color_set = cr[CR_YELLOW];       break;
            case 8:   messages_pickup_color_set = cr[CR_ORANGE];     break;
            case 9:   messages_pickup_color_set = cr[CR_WHITE];      break;
            case 10:  messages_pickup_color_set = cr[CR_GRAY];       break;
            case 11:  messages_pickup_color_set = cr[CR_DARKGRAY];   break;
            case 12:  messages_pickup_color_set = cr[CR_TAN];        break;
            case 13:  messages_pickup_color_set = cr[CR_BROWN];    break;            
            case 14:  messages_pickup_color_set = cr[CR_ALMOND];      break;
            case 15:  messages_pickup_color_set = cr[CR_KHAKI];  break;
            case 16:  messages_pickup_color_set = cr[CR_PINK];      break;
            case 17:  messages_pickup_color_set = cr[CR_BURGUNDY];  break;
            default:  messages_pickup_color_set = NULL;              break;
        }
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_Msg_Secret_Color(void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        messages_secret_color_set = NULL;
    }
    else
    {
        switch (message_secret_color)
        {
            case 1:   messages_secret_color_set = cr[CR_DARKRED];    break;
            case 2:   messages_secret_color_set = cr[CR_GREEN];      break;
            case 3:   messages_secret_color_set = cr[CR_DARKGREEN];  break;
            case 4:   messages_secret_color_set = cr[CR_OLIVE];      break;
            case 5:   messages_secret_color_set = cr[CR_BLUE2];      break;
            case 6:   messages_secret_color_set = cr[CR_DARKBLUE];   break;
            case 7:   messages_secret_color_set = cr[CR_YELLOW];       break;
            case 8:   messages_secret_color_set = cr[CR_ORANGE];     break;
            case 9:   messages_secret_color_set = cr[CR_WHITE];      break;
            case 10:  messages_secret_color_set = cr[CR_GRAY];       break;
            case 11:  messages_secret_color_set = cr[CR_DARKGRAY];   break;
            case 12:  messages_secret_color_set = cr[CR_TAN];        break;
            case 13:  messages_secret_color_set = cr[CR_BROWN];    break;            
            case 14:  messages_secret_color_set = cr[CR_ALMOND];      break;
            case 15:  messages_secret_color_set = cr[CR_KHAKI];  break;
            case 16:  messages_secret_color_set = cr[CR_PINK];      break;
            case 17:  messages_secret_color_set = cr[CR_BURGUNDY];  break;
            default:  messages_secret_color_set = NULL;              break;
        }

        // [JN] Routine №3: play sound only if necessary.
        S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_Msg_System_Color(void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        messages_system_color_set = NULL;
    }
    else
    {
        switch (message_system_color)
        {
            case 1:   messages_system_color_set = cr[CR_DARKRED];    break;
            case 2:   messages_system_color_set = cr[CR_GREEN];      break;
            case 3:   messages_system_color_set = cr[CR_DARKGREEN];  break;
            case 4:   messages_system_color_set = cr[CR_OLIVE];      break;
            case 5:   messages_system_color_set = cr[CR_BLUE2];      break;
            case 6:   messages_system_color_set = cr[CR_DARKBLUE];   break;
            case 7:   messages_system_color_set = cr[CR_YELLOW];       break;
            case 8:   messages_system_color_set = cr[CR_ORANGE];     break;
            case 9:   messages_system_color_set = cr[CR_WHITE];      break;
            case 10:  messages_system_color_set = cr[CR_GRAY];       break;
            case 11:  messages_system_color_set = cr[CR_DARKGRAY];   break;
            case 12:  messages_system_color_set = cr[CR_TAN];        break;
            case 13:  messages_system_color_set = cr[CR_BROWN];    break;            
            case 14:  messages_system_color_set = cr[CR_ALMOND];      break;
            case 15:  messages_system_color_set = cr[CR_KHAKI];  break;
            case 16:  messages_system_color_set = cr[CR_PINK];      break;
            case 17:  messages_system_color_set = cr[CR_BURGUNDY];  break;
            default:  messages_system_color_set = NULL;              break;
        }

        // [JN] Routine №3: play sound only if necessary.
        S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Redefine system message color.
    M_RD_Define_Msg_System_Color();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_Msg_Chat_Color(void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        messages_chat_color_set = NULL;
    }
    else
    {
        switch (message_chat_color)
        {
            case 1:   messages_chat_color_set = cr[CR_DARKRED];    break;
            case 2:   messages_chat_color_set = cr[CR_GREEN];      break;
            case 3:   messages_chat_color_set = cr[CR_DARKGREEN];  break;
            case 4:   messages_chat_color_set = cr[CR_OLIVE];      break;
            case 5:   messages_chat_color_set = cr[CR_BLUE2];      break;
            case 6:   messages_chat_color_set = cr[CR_DARKBLUE];   break;
            case 7:   messages_chat_color_set = cr[CR_YELLOW];       break;
            case 8:   messages_chat_color_set = cr[CR_ORANGE];     break;
            case 9:   messages_chat_color_set = cr[CR_WHITE];      break;
            case 10:  messages_chat_color_set = cr[CR_GRAY];       break;
            case 11:  messages_chat_color_set = cr[CR_DARKGRAY];   break;
            case 12:  messages_chat_color_set = cr[CR_TAN];        break;
            case 13:  messages_chat_color_set = cr[CR_BROWN];    break;            
            case 14:  messages_chat_color_set = cr[CR_ALMOND];      break;
            case 15:  messages_chat_color_set = cr[CR_KHAKI];  break;
            case 16:  messages_chat_color_set = cr[CR_PINK];      break;
            case 17:  messages_chat_color_set = cr[CR_BURGUNDY];  break;
            default:  messages_chat_color_set = NULL;              break;
        }

        // [JN] Routine №3: play sound only if necessary.
        S_StartSound(NULL,sfx_stnmov);
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
        M_WriteTextSmall_ENG(70, 25, "Automap", dp_translation = cr[CR_YELLOW]);

        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(170, 35, "n/a", NULL);
        }
        else
        {
            M_WriteTextSmall_ENG (170, 35, automap_color == 1 ? "boom"   :
                                           automap_color == 2 ? "jaguar" :
                                           automap_color == 3 ? "raven"  :
                                           automap_color == 4 ? "strife" :
                                           automap_color == 5 ? "unity"  :
                                                                "doom", NULL);
        }

        // Line antialiasing
        M_WriteTextSmall_ENG(193, 45, automap_antialias ? "on" : "off", 
                                      automap_antialias ? cr[CR_GREEN] : cr[CR_DARKRED]);


        // Overlay mode
        M_WriteTextSmall_ENG(170, 55, automap_overlay ? "on" : "off",
                                      automap_overlay ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Rotate mode
        M_WriteTextSmall_ENG(163, 65, automap_rotate ? "on" : "off",
                                      automap_rotate ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Follow mode
        M_WriteTextSmall_ENG(164, 75, automap_follow ? "on" : "off",
                                      automap_follow ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Grid
        M_WriteTextSmall_ENG(106, 85, automap_grid ? "on" : "off",
                                      automap_grid ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Grid size
        M_WriteTextSmall_ENG(136, 95, num, automap_grid_size == 128 ? cr[CR_DARKRED] : cr[CR_GREEN]);

        //
        // Statistics
        //
        M_WriteTextSmall_ENG(70, 105, "Statistics", cr[CR_YELLOW]);

        // Level stats

        M_WriteTextSmall_ENG(159, 115, automap_stats == 1 ? "in automap" :
                                       automap_stats == 2 ? "always" : "off",
                                       automap_stats ? cr[CR_GREEN] : cr[CR_DARKRED]);


        // Level time
        M_WriteTextSmall_ENG(150, 125, automap_level_time == 1 ? "in automap" :
                                       automap_level_time == 2 ? "always" : "off",
                                       automap_level_time ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Total time
        M_WriteTextSmall_ENG(151, 135, automap_total_time == 1 ? "in automap" :
                                       automap_total_time == 2 ? "always" : "off",
                                       automap_total_time ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Player coords
        M_WriteTextSmall_ENG(177, 145, automap_coords == 1 ? "in automap" :
                                       automap_coords == 2 ? "always" : "off",
                                       automap_coords ? cr[CR_GREEN] : cr[CR_DARKRED]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "RFHNF B CNFNBCNBRF"); // КАРТА И СТАТИСТИКА

        //
        // Карта
        //
        M_WriteTextSmall_RUS(70, 25, "Rfhnf", cr[CR_YELLOW]);

        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(191, 35, "y*l", NULL); // н/д
        }
        else
        {
            M_WriteTextSmall_ENG (189, 35, automap_color == 1 ? "boom"   :
                                           automap_color == 2 ? "jaguar" :
                                           automap_color == 3 ? "raven"  :
                                           automap_color == 4 ? "strife" :
                                           automap_color == 5 ? "unity"  :
                                                                "doom", NULL);
        }

        // Сглаживание линий
        M_WriteTextSmall_RUS(214, 45, automap_antialias ? "drk" : "dsrk",
                                      automap_antialias ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Режим наложения
        M_WriteTextSmall_RUS(203, 55, automap_overlay ? "drk" : "dsrk",
                                      automap_overlay ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Режим вращения
        M_WriteTextSmall_RUS(194, 65, automap_rotate ? "drk" : "dsrk",
                                      automap_rotate ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Режим следования
        M_WriteTextSmall_RUS(208, 75, automap_follow ? "drk" : "dsrk",
                                      automap_follow ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Сетка
        M_WriteTextSmall_RUS(118, 85, automap_grid ? "drk" : "dsrk",
                                      automap_grid ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Размер сетки
        M_WriteTextSmall_ENG(171, 95, num, automap_grid_size == 128 ? cr[CR_DARKRED] : cr[CR_GREEN]);
        
        //
        // Статистика
        //
        M_WriteTextSmall_RUS(70, 105, "Cnfnbcnbrf", cr[CR_YELLOW]);

        // Статистика уровня
        M_WriteTextSmall_RUS(210, 115, automap_stats == 1 ? "yf rfhnt" :
                                       automap_stats == 2 ? "dctulf" : "dsrk",
                                       automap_stats ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Время уровня
        M_WriteTextSmall_RUS(171, 125, automap_level_time == 1 ? "yf rfhnt" :
                                       automap_level_time == 2 ? "dctulf" : "dsrk",
                                       automap_level_time ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Общее время
        M_WriteTextSmall_RUS(166, 135, automap_total_time == 1 ? "yf rfhnt" :
                                       automap_total_time == 2 ? "dctulf" : "dsrk",
                                       automap_total_time ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Координаты игрока
        M_WriteTextSmall_RUS(213, 145, automap_coords == 1 ? "yf rfhnt" :
                                       automap_coords == 2 ? "dctulf" : "dsrk",
                                       automap_coords ? cr[CR_GREEN] : cr[CR_DARKRED]);
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

    // [JN] Reinitialize automap color scheme.
    AM_initColors();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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
        M_WriteTextSmall_ENG(35, 25, "volume", cr[CR_YELLOW]);

        //
        // Channels
        //
        M_WriteTextSmall_ENG(35, 75, "channels", cr[CR_YELLOW]);

        //
        // System
        //
        M_WriteTextSmall_ENG(35, 105, "advanced", cr[CR_YELLOW]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB PDERF"); // НАСТРОЙКИ ЗВУКА

        //
        // Громкость
        //
        M_WriteTextSmall_RUS(35, 25, "uhjvrjcnm", cr[CR_YELLOW]);

        //
        // Воспроизведение
        //
        M_WriteTextSmall_RUS(35, 75, "djcghjbpdtltybt", cr[CR_YELLOW]);

        //
        // Дополнительно
        //
        M_WriteTextSmall_RUS(35, 105, "ljgjkybntkmyj", cr[CR_YELLOW]);
    }

    // SFX volume slider
    M_DrawThermo_Small(35, 44, 16, sfxVolume);
    // Numerical representation of SFX volume
    M_snprintf(num, 4, "%3d", sfxVolume);
    M_WriteTextSmall_ENG(177, 45, num, NULL);

    // Music volume slider
    M_DrawThermo_Small(35, 64, 16, musicVolume);
    // Numerical representation of music volume
    M_snprintf(num, 4, "%3d", musicVolume);
    M_WriteTextSmall_ENG(177, 65, num, NULL);

    // SFX channels slider
    M_DrawThermo_Small(35, 94, 16, snd_channels / 4 - 1);
    // Numerical representation of channels
    M_snprintf(num, 4, "%3d", snd_channels);
    M_WriteTextSmall_ENG(177, 95, num, NULL);
}

void M_RD_Change_SfxVol(int choice)
{
    switch(choice)
    {
        case 0:
        if (sfxVolume)
        {
            sfxVolume--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (sfxVolume < 15)
        {
            sfxVolume++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
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
        {
            musicVolume--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (musicVolume < 15)
        {
            musicVolume++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
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
        {
            snd_channels -= 4;
            // Reallocate sound channels
            S_ChannelsRealloc();
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    
        case 1:
        if (snd_channels < 64)
        {
            snd_channels += 4;
            // Reallocate sound channels
            S_ChannelsRealloc();
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    }
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
    static char snd_frequency[16];

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "SOUND SYSTEM");

        //
        // Sound system
        //
        M_WriteTextSmall_ENG(35, 25, "sound system", cr[CR_YELLOW]);

        // Sound effects
        if (snd_sfxdevice == 0)
        {
            
            M_WriteTextSmall_ENG(141, 35, "disabled", cr[CR_DARKRED]);
        }
        else if (snd_sfxdevice == 1)
        {
            M_WriteTextSmall_ENG(141, 35, "pc speaker", NULL);
        }
        else if (snd_sfxdevice == 3)
        {
            M_WriteTextSmall_ENG(141, 35, "digital sfx", NULL);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            M_WriteTextSmall_ENG(79, 45, "disabled", cr[CR_DARKRED]);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            M_WriteTextSmall_ENG(79, 45, "opl2 synth", NULL);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            M_WriteTextSmall_ENG(79, 45, "opl3 synth", NULL);
        }
        else if (snd_musicdevice == 5)
        {
            M_WriteTextSmall_ENG(79, 45, "gus emulation", NULL);
        }
        else if (snd_musicdevice == 8)
        {
            M_WriteTextSmall_ENG(79, 45, "MIDI/MP3/OGG/FLAC/TRACKER", NULL);
        }

        //
        // Quality
        //
        M_WriteTextSmall_ENG(35, 55, "quality", cr[CR_YELLOW]);

        // Sampling frequency
        sprintf(snd_frequency, "%d HZ", snd_samplerate);
        M_WriteTextSmall_ENG(179, 65, snd_frequency, NULL);

        //
        // Miscellaneous
        //
        M_WriteTextSmall_ENG(35, 75, "Miscellaneous", cr[CR_YELLOW]);

        // Sfx mode
        M_WriteTextSmall_ENG(178, 85, snd_monomode ? "mono" : "stereo", NULL);

        // Pitch-shifted sounds
        M_WriteTextSmall_ENG(186, 95, snd_pitchshift ? "on" : "off", NULL);

        // Mute inactive window
        M_WriteTextSmall_ENG(185, 105, mute_inactive_window ? "on" : "off", NULL);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "PDERJDFZ CBCNTVF"); // ЗВУКОВАЯ СИСТЕМА

        //
        // Звуковая система
        //
        M_WriteTextSmall_RUS(35, 25, "pderjdfz cbcntvf", cr[CR_YELLOW]);

        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // Отключены
            M_WriteTextSmall_RUS(175, 35, "jnrk.xtys", cr[CR_DARKRED]);
        }
        else if (snd_sfxdevice == 1)
        {
            // Динамик ПК
            M_WriteTextSmall_RUS(175, 35, "lbyfvbr gr", NULL);
        }
        else if (snd_sfxdevice == 3)
        {
            // Цифровые
            M_WriteTextSmall_RUS(175, 35, "wbahjdst", NULL);
        }

        // Музыка
        if (snd_musicdevice == 0)
        {
            // Отключена
            M_WriteTextSmall_RUS(94, 45, "jnrk.xtyf", cr[CR_DARKRED]);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // Синтез OPL2
            M_WriteTextSmall_RUS(94, 45, "cbyntp", NULL);
            M_WriteTextSmall_ENG(146, 45, "opl2", NULL);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // Синтез OPL3
            M_WriteTextSmall_RUS(94, 45, "cbyntp", NULL);
            M_WriteTextSmall_ENG(146, 45, "opl3", NULL);
        }
        else if (snd_musicdevice == 5)
        {
            // Эмуляция GUS
            M_WriteTextSmall_RUS(94, 45, "\'vekzwbz", NULL);
            M_WriteTextSmall_ENG(164, 45, "gus", NULL);
        }
        else if (snd_musicdevice == 8)
        {
            M_WriteTextSmall_ENG(94, 45, "MIDI/MP3/OGG/FLAC/TRACKER", NULL);
        }

        //
        // Качество звучания
        //
        M_WriteTextSmall_RUS(35, 55, "rfxtcndj pdexfybz", cr[CR_YELLOW]);

        // Частота дискретизации (ГЦ)
        sprintf(snd_frequency, "%d UW", snd_samplerate);
        M_WriteTextSmall_RUS(208, 65, snd_frequency, NULL);

        //
        // Разное
        //
        M_WriteTextSmall_RUS(35, 75, "hfpyjt", cr[CR_YELLOW]);

        // Режим звука
        M_WriteTextSmall_RUS(231, 85, snd_monomode ? "vjyj" : "cnthtj", NULL);

        // Произвольный питч-шифтинг
        M_WriteTextSmall_RUS(242, 95, snd_pitchshift ? "drk" : "dsrk", NULL);

        // Звук в неактивном окне
        M_WriteTextSmall_RUS(208, 105, mute_inactive_window ? "dsrk" : "drk", NULL);
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
        M_WriteTextSmall_ENG(35, 25, "Controls", cr[CR_YELLOW]);

        //
        // Mouse
        //
        M_WriteTextSmall_ENG(35, 55, "mouse", cr[CR_YELLOW]);

        // Mouse look
        M_WriteTextSmall_ENG(119, 125, mlook ? "on" : "off", NULL);

        // Invert Y axis
        M_WriteTextSmall_ENG(130, 135, mouse_y_invert ? "on" : "off",
                                      !mlook ? cr[CR_DARKRED] : NULL);


        // Vertical movement
        M_WriteTextSmall_ENG(171, 145, !novert ? "on" : "off",
                                        mlook ? cr[CR_DARKRED] : NULL);

    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "EGHFDKTYBT"); // УПРАВЛЕНИЕ

        //
        // Управление
        //
        M_WriteTextSmall_RUS(35, 25, "eghfdktybt", cr[CR_YELLOW]);

        //
        // Мышь
        //
        M_WriteTextSmall_RUS(35, 55, "vsim", cr[CR_YELLOW]);

        // Обзор мышью
        M_WriteTextSmall_RUS(135, 125, mlook ? "drk" : "dsrk", NULL);

        // Вертикальная инверсия
        M_WriteTextSmall_RUS(207, 135, mouse_y_invert ? "drk" : "dsrk",
                                      !mlook ? cr[CR_DARKRED] : NULL);

        // Вертикальное перемещение
        M_WriteTextSmall_RUS(235, 145, !novert ? "drk" : "dsrk",
                                        mlook ? cr[CR_DARKRED] : NULL);
    }

    // Mouse sensivity slider
    M_DrawThermo_Small(35, 74, 17, mouseSensitivity);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    M_WriteTextSmall_ENG(189, 75, num, NULL);

    // Acceleration slider
    M_DrawThermo_Small(35, 94, 17, mouse_acceleration * 4 - 4);
    // Numerical representation
    M_snprintf(num, 4, "%f", mouse_acceleration);
    M_WriteTextSmall_ENG(189, 95, num, NULL);

    // Acceleration threshold slider
    M_DrawThermo_Small(35, 114, 17, mouse_threshold / 2);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouse_threshold);
    if (mouse_acceleration < 1.1)
    dp_translation = cr[CR_DARKRED];
    M_WriteTextSmall_ENG(189, 115, num, NULL);
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
        {
            mouseSensitivity--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (mouseSensitivity < 255) // [crispy] extended range
        {
            mouseSensitivity++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    }
}

void M_RD_Change_Acceleration(int choice)
{
    switch(choice)
    {
        case 0:
        if (mouse_acceleration > 1.1)
        {
            mouse_acceleration -= 0.1;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (mouse_acceleration < 5.0)
        {
            mouse_acceleration += 0.1;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;
    }
}

void M_RD_Change_Threshold(int choice)
{
    switch(choice)
    {
        case 0:
        if (mouse_threshold > 0)
        {
            mouse_threshold--;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
        break;

        case 1:
        if (mouse_threshold < 32)
        {
            mouse_threshold++;
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
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

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
        M_WriteTextBigCentered_ENG(5, "Keyboard bindings");
    else
        M_WriteTextBigCentered_RUS(5, "Yfcnhjqrb rkfdbfnehs"); // Настройки клавиатуры

    for (int i = 0; i < keyPage->num_of_subtitles; ++i)
    {
        if (english_language)
            M_WriteTextSmall_ENG(35, keyPage->subtitles[i].y, keyPage->subtitles[i].eng_text, cr[CR_YELLOW]);
        else
            M_WriteTextSmall_RUS(35, keyPage->subtitles[i].y, keyPage->subtitles[i].rus_text, cr[CR_YELLOW]);
    }

    if (english_language)
    {
        M_WriteTextSmall_ENG(35, 155, currentMenu == RD_Bindings_Menu_Def_4 ? "first page >" : "next page >", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(35, 165, currentMenu == RD_Bindings_Menu_Def_1 ? "< last page" : "< prev page", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(x, 165, keyPage->pageNumber_eng, cr[CR_WHITE]);

        M_WriteTextSmall_ENG(55, 180, "enter to change, del to clear", dp_translation = cr[CR_DARKRED]);
        M_WriteTextSmall_ENG(75, 189, "pgup/pgdn to turn pages", dp_translation = cr[CR_DARKRED]);
    }
    else
    {
        M_WriteTextSmall_RUS(35, 155, RD_NEXT_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(35, 165, RD_PREV_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(x, 165, keyPage->pageNumber_rus, cr[CR_WHITE]);

        M_WriteTextSmall_ENG(44, 180, "enter =", cr[CR_DARKRED]);
        M_WriteTextSmall_RUS(88, 180, "= yfpyfxbnm<", cr[CR_DARKRED]);
        M_WriteTextSmall_ENG(176, 180, "del =", cr[CR_DARKRED]);
        M_WriteTextSmall_RUS(213, 180, "jxbcnbnm", cr[CR_DARKRED]);

        M_WriteTextSmall_ENG(55, 189, "pgup/pgdn =", cr[CR_DARKRED]);
        M_WriteTextSmall_RUS(139, 189, "kbcnfnm cnhfybws", cr[CR_DARKRED]);
    }

    for (int i = 0; i < 11; ++i)
    {
        bound_key_t key = keyPage->keys[i];
        if (key != bk_null)
        {
            boolean bindingThis = messageToBind && i == itemOn;

            M_WriteTextSmall_ENG(x, i * 10 + 35, bindingThis ? "?" : BK_getBoundKeysString(key),
                                 bindingThis ? cr[CR_WHITE] : BK_KeyHasNoBinds(key) ? cr[CR_DARKRED] : NULL);

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
    
    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "Mouse bindings");

        //
        // Buttons
        //
        M_WriteTextSmall_ENG(35, 25, "Buttons", cr[CR_YELLOW]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(55, 180, "enter to change, del to clear", cr[CR_DARKRED]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "Yfcnhjqrb vsib"); // Настройки мыши

        //
        // Кнопки
        //
        M_WriteTextSmall_RUS(35, 25, "ryjgrb", cr[CR_YELLOW]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(44, 180, "enter =", cr[CR_DARKRED]);
        M_WriteTextSmall_RUS(88, 180, "= yfpyfxbnm<", cr[CR_DARKRED]);
        M_WriteTextSmall_ENG(176, 180, "del =", cr[CR_DARKRED]);
        M_WriteTextSmall_RUS(213, 180, "jxbcnbnm", cr[CR_DARKRED]);
    }

    // Fire/Attack
    if (messageToBind && itemOn == rd_mouse_bindings_attack)
    {
        M_WriteTextSmall_ENG(x, 35, "?", cr[CR_WHITE]);
    }
    else
    {
        if (mousebfire == -1)
        dp_translation = cr[CR_DARKRED];
        M_WriteTextSmall_ENG(x, 35, M_RD_MouseBtnDrawer(mousebfire),
                             mousebfire == -1 ? cr[CR_DARKRED] : NULL);
        //dp_translation = NULL;
    }

    // Use
    if (messageToBind && itemOn == rd_mouse_bindings_use)
    {
        M_WriteTextSmall_ENG(x, 45, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 45, M_RD_MouseBtnDrawer(mousebuse),
                             mousebuse == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Move Forward
    if (messageToBind && itemOn == rd_mouse_bindings_forward)
    {

        M_WriteTextSmall_ENG(x, 55, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 55, M_RD_MouseBtnDrawer(mousebforward),
                             mousebforward == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Move Backward
    if (messageToBind && itemOn == rd_mouse_bindings_backward)
    {
        M_WriteTextSmall_ENG(x, 65, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 65, M_RD_MouseBtnDrawer(mousebbackward),
                             mousebbackward == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Strafe On
    if (messageToBind && itemOn == rd_mouse_bindings_strafeon)
    {
        M_WriteTextSmall_ENG(x, 75, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 75, M_RD_MouseBtnDrawer(mousebstrafe),
                             mousebstrafe == -1 ? cr[CR_DARKRED] : NULL);

    }

    // Strafe Left
    if (messageToBind && itemOn == rd_mouse_bindings_strafeleft)
    {
        M_WriteTextSmall_ENG(x, 85, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 85, M_RD_MouseBtnDrawer(mousebstrafeleft),
                             mousebstrafeleft == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Strafe Right
    if (messageToBind && itemOn == rd_mouse_bindings_straferight)
    {
        M_WriteTextSmall_ENG(x, 95, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 95, M_RD_MouseBtnDrawer(mousebstraferight),
                             mousebstraferight == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Previous Weapon
    if (messageToBind && itemOn == rd_mouse_bindings_prevweapon)
    {
        M_WriteTextSmall_ENG(x, 105, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 105, M_RD_MouseBtnDrawer(mousebprevweapon),
                             mousebprevweapon == -1 ? cr[CR_DARKRED] : NULL);
    }

    // Next Weapon
    if (messageToBind && itemOn == rd_mouse_bindings_nextweapon)
    {
        M_WriteTextSmall_ENG(x, 115, "?", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextSmall_ENG(x, 115, M_RD_MouseBtnDrawer(mousebnextweapon),
                             mousebnextweapon == -1 ? cr[CR_DARKRED] : NULL);
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

void M_RD_Choose_Gameplay_5(int choice)
{
    M_SetupNextMenu(english_language ? 
                    &RD_Gameplay_Def_5 :
                    &RD_Gameplay_Def_5_Rus);
}

void M_RD_Draw_Gameplay_1(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Graphical
        //
        M_WriteTextSmall_ENG(35, 25, "Graphical", cr[CR_YELLOW]);

        // Brightmaps
        M_WriteTextSmall_ENG(119, 35, brightmaps ? RD_ON : RD_OFF,
                                      brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED]);


        // Fake contrast
        M_WriteTextSmall_ENG(142, 45, fake_contrast ? RD_ON : RD_OFF,
                                      fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Translucency
        M_WriteTextSmall_ENG(138, 55, translucency ? RD_ON : RD_OFF,
                                      translucency ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Fuzz effect
        M_WriteTextSmall_ENG(125, 65, improved_fuzz == 0 ? "Original" :
                                      improved_fuzz == 1 ? "Original (b&w)" :
                                      improved_fuzz == 2 ? "Improved" :
                                      improved_fuzz == 3 ? "Improved (b&w)" : "Translucent",
                                      improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Colored blood and corpses
        M_WriteTextSmall_ENG(229, 75, colored_blood ? RD_ON : RD_OFF,
                                      colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Swirling liquids
        M_WriteTextSmall_ENG(150, 85, swirling_liquids ? RD_ON : RD_OFF,
                                      swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Invulnerability affects sky
        M_WriteTextSmall_ENG(237, 95, invul_sky ? RD_ON : RD_OFF,
                                      invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Horizontally linear sky drawing
        M_WriteTextSmall_ENG(160, 105, linear_sky ? "linear" : "original",
                                       linear_sky ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Flip weapons
        M_WriteTextSmall_ENG(131, 115, flip_weapons ? RD_ON : RD_OFF,
                                       flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(35, 145, "next page >", cr[CR_WHITE]); 
        M_WriteTextSmall_ENG(35, 155, "< last page", cr[CR_WHITE]); 
        M_WriteTextSmall_ENG(231, 155, "page 1/5", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Графика
        //
        M_WriteTextSmall_RUS(35, 25, "uhfabrf", cr[CR_YELLOW]);

        // Брайтмаппинг
        M_WriteTextSmall_RUS(140, 35, brightmaps ? RD_ON_RUS : RD_OFF_RUS,
                                      brightmaps ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Имитация контрастности
        M_WriteTextSmall_RUS(217, 45, fake_contrast ? RD_ON_RUS : RD_OFF_RUS,
                                      fake_contrast ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Прозрачность объектов
        M_WriteTextSmall_RUS(207, 55, translucency ? RD_ON_RUS : RD_OFF_RUS,
                                      translucency ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Эффект шума
        M_WriteTextSmall_RUS(134, 65, improved_fuzz == 0 ? "Jhbubyfkmysq" :
                                      improved_fuzz == 1 ? "Jhbubyfkmysq (x*,)" :
                                      improved_fuzz == 2 ? "Ekexityysq" :
                                      improved_fuzz == 3 ? "Ekexityysq (x*,)" : "Ghjphfxysq",
                                      improved_fuzz > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Разноцветная кровь и трупы
        M_WriteTextSmall_RUS(242, 75, colored_blood ? RD_ON_RUS : RD_OFF_RUS,
                                      colored_blood ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Улучшенная анимация жидкостей
        M_WriteTextSmall_RUS(275, 85, swirling_liquids ? RD_ON_RUS : RD_OFF_RUS,
                                      swirling_liquids ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Неуязвимость окрашивает небо
        M_WriteTextSmall_RUS(262, 95, invul_sky ? RD_ON_RUS : RD_OFF_RUS,
                                      invul_sky ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Режим отрисовки неба
        M_WriteTextSmall_RUS(200, 105, linear_sky ? "kbytqysq" : "jhbubyfkmysq",
                                       linear_sky ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Зеркальное отражение оружия
        M_WriteTextSmall_RUS(259, 115, flip_weapons ? RD_ON_RUS : RD_OFF_RUS,
                                       flip_weapons ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Footer
        //
        M_WriteTextSmall_RUS(35, 145, RD_NEXT_RUS, cr[CR_WHITE]); 
        M_WriteTextSmall_RUS(35, 155, RD_PREV_RUS, cr[CR_WHITE]); 
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 1*5", cr[CR_WHITE]);
    }
}

void M_RD_Draw_Gameplay_2(void)
{
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");
        
        //
        // Status Bar
        //
        M_WriteTextSmall_ENG(35, 25, "Status bar", cr[CR_YELLOW]);

        // Extra player faces
        M_WriteTextSmall_ENG(179, 35, extra_player_faces ? RD_ON : RD_OFF,
                                      extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Show negative health
        M_WriteTextSmall_ENG(190, 45, negative_health ? RD_ON : RD_OFF,
                                      negative_health ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Colored elements
        M_WriteTextSmall_ENG(167, 55, sbar_colored == 1 ? "ON (NO %)"  :
                                      sbar_colored == 2 ? "ON" : "OFF",
                                      sbar_colored ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Coloring
        //
        M_WriteTextSmall_ENG(35, 65, "Coloring", cr[CR_YELLOW]);

        M_WriteTextSmall_ENG(35, 75, "High value:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 85, "Normal value:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 95, "Low value:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 105, "Critical value:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 115, "Armor type 1:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 125, "Armor type 2:", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_ENG(35, 135, "No armor:", sbar_colored == 0 || gamemission == jaguar? cr[CR_DARKRED] : NULL);
        
        // High Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(114, 75, "n/a", sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        }
        else
        {
            M_WriteTextSmall_ENG(114, 75, M_RD_ColorName_ENG(sbar_color_high),
                                          M_RD_ColorTranslation(sbar_color_high));
        }

        // Normal Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(135, 85, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(135, 85, M_RD_ColorName_ENG(sbar_color_normal),
                                          M_RD_ColorTranslation(sbar_color_normal));
        }

        // Low Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(111, 95, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(111, 95, M_RD_ColorName_ENG(sbar_color_low),
                                          M_RD_ColorTranslation(sbar_color_low));
        }

        // Critical value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(142, 105, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(142, 105, M_RD_ColorName_ENG(sbar_color_critical),
                                           M_RD_ColorTranslation(sbar_color_critical));
        }

        // Armor type 1
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(129, 115, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(129, 115, M_RD_ColorName_ENG(sbar_color_armor_1),
                                           M_RD_ColorTranslation(sbar_color_armor_1));
        }

        // Armor type 2
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(132, 125, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(132, 125, M_RD_ColorName_ENG(sbar_color_armor_2),
                                           M_RD_ColorTranslation(sbar_color_armor_2));
        }

        // Armor type 0
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_ENG(104, 135, "n/a", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(104, 135, M_RD_ColorName_ENG(sbar_color_armor_0),
                                           M_RD_ColorTranslation(sbar_color_armor_0));
        }

        //
        // Footer
        //
        M_WriteTextSmall_ENG(35, 145, "next page >", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(35, 155, "< prev page", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(231, 155, "page 2/5", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Статус-бар
        //
        M_WriteTextSmall_RUS(35, 25, "cnfnec-,fh", cr[CR_YELLOW]);

        // Дополнительные лица игрока
        M_WriteTextSmall_RUS(247, 35, extra_player_faces ? "DRK" : "DSRK",
                                      extra_player_faces ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Отрицательное здоровье
        M_WriteTextSmall_RUS(217, 45, negative_health ? "DRK" : "DSRK",
                                      negative_health ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Разноцветные элементы
        M_WriteTextSmall_RUS(213, 55, sbar_colored == 1 ? "DRK (,TP %)"  :
                                      sbar_colored == 2 ? "DRK" : "DSRK",
                                      sbar_colored ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Цвета
        //
        M_WriteTextSmall_RUS(35, 65, "wdtnf", cr[CR_YELLOW]);

        // Высокое значение
        M_WriteTextSmall_RUS(35, 75, "Dscjrjt pyfxtybt:",      // Высокое значение
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 85, "Yjhvfkmyjt pyfxtybt:",   // Нормальное значение
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 95, "Ybprjt pyfxtybt:",       // Низкое значение
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 105, "Rhbnbxtcrjt pyfxtybt:", // Критическое значение
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 115, "Nbg ,hjyb 1:",          // Тип брони 1
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 125, "Nbg ,hjyb 2:",          // Тип брони 2
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        M_WriteTextSmall_RUS(35, 135, "Jncencndbt ,hjyb:",     // Отсутствие брони
                             sbar_colored == 0 || gamemission == jaguar ? cr[CR_DARKRED] : NULL);
        
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(168, 75, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(168, 75, M_RD_ColorName_RUS(sbar_color_high),
                                          M_RD_ColorTranslation(sbar_color_high));
        }

        // Нормальное значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(191, 85, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(191, 85, M_RD_ColorName_RUS(sbar_color_normal),
                                          M_RD_ColorTranslation(sbar_color_normal));
        }

        // Низкое значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(158, 95, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(158, 95, M_RD_ColorName_RUS(sbar_color_low),
                                          M_RD_ColorTranslation(sbar_color_low));
        }

        // Низкое значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(197, 105, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(197, 105, M_RD_ColorName_RUS(sbar_color_critical),
                                           M_RD_ColorTranslation(sbar_color_critical));
        }

        // Тип брони 1
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(120, 115, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(120, 115, M_RD_ColorName_RUS(sbar_color_armor_1),
                                           M_RD_ColorTranslation(sbar_color_armor_1));
        }

        // Тип брони 2
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(123, 125, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(123, 125, M_RD_ColorName_RUS(sbar_color_armor_2),
                                           M_RD_ColorTranslation(sbar_color_armor_2));
        }

        // Отсутствие брони
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            M_WriteTextSmall_RUS(167, 135, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(167, 135, M_RD_ColorName_RUS(sbar_color_armor_0),
                                           M_RD_ColorTranslation(sbar_color_armor_0));
        }

        //
        // Footer
        //
        M_WriteTextSmall_RUS(35, 145, RD_NEXT_RUS, cr[CR_WHITE]); 
        M_WriteTextSmall_RUS(35, 155, RD_PREV_RUS, cr[CR_WHITE]); 
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 2*5", cr[CR_WHITE]);
    }
}

void M_RD_Draw_Gameplay_3(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Audible
        //
        M_WriteTextSmall_ENG(35, 25, "Audible", cr[CR_YELLOW]);

        // Sound attenuation axises
        M_WriteTextSmall_ENG(217, 35, z_axis_sfx ? "x/y/z" : "x/y",
                                      z_axis_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Play exit sounds
        M_WriteTextSmall_ENG(158, 45, play_exit_sfx ? RD_ON : RD_OFF,
                                      play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Sound of crushing corpses
        M_WriteTextSmall_ENG(223, 55, crushed_corpses_sfx ? RD_ON : RD_OFF,
                                      crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Single sound of closing blazing door
        M_WriteTextSmall_ENG(240, 65, blazing_door_fix_sfx ? RD_ON : RD_OFF,
                                      blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Monster alert waking up other monsters
        M_WriteTextSmall_ENG(263, 75, noise_alert_sfx ? RD_ON : RD_OFF,
                                      noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Tactical
        //
        M_WriteTextSmall_ENG(35, 85, "Tactical", cr[CR_YELLOW]);

        // Notify of revealed secrets
        M_WriteTextSmall_ENG(232, 95, secret_notification ? RD_ON : RD_OFF,
                                      secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Infragreen light amp. visor
        M_WriteTextSmall_ENG(230, 105, infragreen_visor ? RD_ON : RD_OFF,
                                       infragreen_visor ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(35, 145, "next page >", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(35, 155, "< prev page", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(231, 155, "page 3/5", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Звук
        //
        M_WriteTextSmall_RUS(35, 25, "Pder", cr[CR_YELLOW]);

        // Затухание звука по осям
        M_WriteTextSmall_ENG(217, 35, z_axis_sfx ? "x/y/z" : "x/y",
                                      z_axis_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);
        dp_translation = NULL;

        // Звуки при выходе из игры
        M_WriteTextSmall_RUS(225, 45, play_exit_sfx ? RD_ON_RUS : RD_OFF_RUS,
                                      play_exit_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Звук раздавливания трупов
        M_WriteTextSmall_RUS(236, 55, crushed_corpses_sfx ? RD_ON_RUS : RD_OFF_RUS,
                                      crushed_corpses_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Одиночный звук быстрой двери
        M_WriteTextSmall_RUS(260, 65, blazing_door_fix_sfx ? RD_ON_RUS : RD_OFF_RUS,
                                      blazing_door_fix_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Общая тревога у монстров
        M_WriteTextSmall_RUS(227, 75, noise_alert_sfx ? RD_ON_RUS : RD_OFF_RUS,
                                      noise_alert_sfx ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Тактика
        //
        M_WriteTextSmall_RUS(35, 85, "Nfrnbrf", cr[CR_YELLOW]);

        // Сообщать о найденном тайнике
        M_WriteTextSmall_RUS(260, 95, secret_notification ? RD_ON_RUS : RD_OFF_RUS,
                                      secret_notification ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Инфразеленый визор освещения
        M_WriteTextSmall_RUS(266, 105, infragreen_visor ? RD_ON_RUS : RD_OFF_RUS,
                                       infragreen_visor ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Footer
        M_WriteTextSmall_RUS(35, 145, RD_NEXT_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(35, 155, RD_PREV_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 3*5", cr[CR_WHITE]);
    }
}

void M_RD_Draw_Gameplay_4(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Physical
        //
        M_WriteTextSmall_ENG(35, 25, "Physical", cr[CR_YELLOW]);

        // Walk over and under monsters
        M_WriteTextSmall_ENG(250, 35, over_under ? RD_ON : RD_OFF,
                                      over_under ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Corpses sliding from the ledges
        M_WriteTextSmall_ENG(264, 45, torque ? RD_ON : RD_OFF,
                                      torque ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Weapon bobbing while firing
        M_WriteTextSmall_ENG(233, 55, weapon_bobbing ? RD_ON : RD_OFF,
                                      weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Lethal pellet of a point-blank SSG
        M_WriteTextSmall_ENG(287, 65, ssg_blast_enemies ? RD_ON : RD_OFF,
                                      ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Randomly mirrored corpses
        M_WriteTextSmall_ENG(231, 75, randomly_flipcorpses ? RD_ON : RD_OFF,
                                      randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Floating powerups
        M_WriteTextSmall_ENG(171, 85, floating_powerups ? RD_ON : RD_OFF,
                                      floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Items are tossed when dropped
        M_WriteTextSmall_ENG(254, 95, toss_drop ? RD_ON : RD_OFF,
                                      toss_drop ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Crosshair
        //
        M_WriteTextSmall_ENG(35, 105, "Crosshair", cr[CR_YELLOW]);

        // Draw crosshair
        M_WriteTextSmall_ENG(146, 115, crosshair_draw ? RD_ON : RD_OFF,
                                       crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Indication
        M_WriteTextSmall_ENG(111, 125, crosshair_type == 1 ? "Health" :
                                       crosshair_type == 2 ? "Target highlighting" :
                                       crosshair_type == 3 ? "Target highlighting+Health" : "Static",
                                       crosshair_type ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Increased size
        M_WriteTextSmall_ENG(140, 135, crosshair_scale ? RD_ON : RD_OFF,
                                       crosshair_scale ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(35, 145, "next page >", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(35, 155, "< prev page", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(231, 155, "page 4/5", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Физика
        //
        M_WriteTextSmall_RUS(35, 25, "Abpbrf", cr[CR_YELLOW]);

        // Перемещение под/над монстрами
        M_WriteTextSmall_RUS(274, 35, over_under ? RD_ON_RUS : RD_OFF_RUS,
                                      over_under ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Трупы сползают с возвышений
        M_WriteTextSmall_RUS(256, 45, torque ? RD_ON_RUS : RD_OFF_RUS,
                                      torque ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Улучшенное покачивание оружия
        M_WriteTextSmall_RUS(271, 55, weapon_bobbing ? RD_ON_RUS : RD_OFF_RUS,
                                      weapon_bobbing ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Двустволка разрывает врагов
        M_WriteTextSmall_RUS(254, 65, ssg_blast_enemies ? RD_ON_RUS : RD_OFF_RUS,
                                      ssg_blast_enemies ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Зеркалирование трупов
        M_WriteTextSmall_RUS(207, 75, randomly_flipcorpses ? RD_ON_RUS : RD_OFF_RUS,
                                      randomly_flipcorpses ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Левитирующие сферы-артефакты
        M_WriteTextSmall_RUS(275, 85, floating_powerups ? RD_ON_RUS : RD_OFF_RUS,
                                      floating_powerups ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Подбрасывать выпавшие предметы
        M_WriteTextSmall_RUS(285, 95, toss_drop ? RD_ON_RUS : RD_OFF_RUS,
                                      toss_drop ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Прицел
        //
        M_WriteTextSmall_RUS(35, 105, "Ghbwtk", cr[CR_YELLOW]);

        // Отображать прицел
        M_WriteTextSmall_RUS(180, 115, crosshair_draw ? RD_ON_RUS : RD_OFF_RUS,
                                       crosshair_draw ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Индикация
        M_WriteTextSmall_RUS(117, 125, crosshair_type == 1 ? "Pljhjdmt" :       // Здоровье
                                       crosshair_type == 2 ? "Gjlcdtnrf wtkb" : // Подсветка цели
                                       crosshair_type == 3 ? "Gjlcdtnrf wtkb+pljhjdmt" : "Cnfnbxyfz", // Подсветка цели + здоровье
                                       crosshair_type ? cr[CR_GREEN] : cr[CR_DARKRED]);      // Статичная

        // Увеличенный размер
        M_WriteTextSmall_RUS(185, 135, crosshair_scale ? RD_ON_RUS : RD_OFF_RUS,
                                                    crosshair_scale ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Footer
        //
        M_WriteTextSmall_RUS(35, 145, RD_NEXT_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(35, 155, RD_PREV_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 4*5", cr[CR_WHITE]);
    }
}

void M_RD_Draw_Gameplay_5(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(5, "GAMEPLAY FEATURES");

        //
        // Gameplay
        //
        M_WriteTextSmall_ENG(35, 25, "Gameplay", cr[CR_YELLOW]);

        // Fix errors of vanilla maps
        M_WriteTextSmall_ENG(226, 35, fix_map_errors ? RD_ON : RD_OFF,
                                      fix_map_errors ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Flip game levels
        M_WriteTextSmall_ENG(158, 45, flip_levels ? RD_ON : RD_OFF,
                                      flip_levels ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Pain Elemental without Souls limit
        M_WriteTextSmall_ENG(284, 55, unlimited_lost_souls ? RD_ON : RD_OFF,
                                      unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // More agressive lost souls
        M_WriteTextSmall_ENG(230, 65, agressive_lost_souls ? RD_ON : RD_OFF,
                                      agressive_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Pistol start
        M_WriteTextSmall_ENG(203, 75, pistol_start ? RD_ON : RD_OFF,
                                      pistol_start ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Demos
        //
        M_WriteTextSmall_ENG(35, 85, "Demos", cr[CR_YELLOW]);

        // Show demo timer
        M_WriteTextSmall_ENG(153, 95, demotimer == 1 ? "playback"  :
                                      demotimer == 2 ? "recording" :
                                      demotimer == 3 ? "always" : "off",
                                      demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Timer direction
        M_WriteTextSmall_ENG(148, 105, demotimerdir ? "backward" : "forward",
                                       demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Show progress bar 
        M_WriteTextSmall_ENG(169, 115, demobar ? RD_ON : RD_OFF,
                                       demobar ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Play internal demos
        M_WriteTextSmall_ENG(183, 125, no_internal_demos ? RD_OFF : RD_ON,
                                       no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN]);

        //
        // Footer
        //
        M_WriteTextSmall_ENG(35, 145, "first page >", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(35, 155, "< prev page", cr[CR_WHITE]);
        M_WriteTextSmall_ENG(231, 155, "page 5/5", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(5, "YFCNHJQRB UTQVGKTZ"); // НАСТРОЙКИ ГЕЙМПЛЕЯ

        //
        // Геймплей
        //
        M_WriteTextSmall_RUS(35, 25, "Utqvgktq", cr[CR_YELLOW]);

        // Устранять ошибки ориг. уровней
        M_WriteTextSmall_RUS(269, 35, fix_map_errors ? RD_ON_RUS : RD_OFF_RUS,
                                      fix_map_errors ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Устранять ошибки ориг. уровней
        M_WriteTextSmall_RUS(263, 45, flip_levels ? RD_ON_RUS : RD_OFF_RUS,
                                      flip_levels ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Элементаль без ограничения Душ
        M_WriteTextSmall_RUS(274, 55, unlimited_lost_souls ? RD_ON_RUS : RD_OFF_RUS,
                                      unlimited_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Повышенная агрессивность Душ
        M_WriteTextSmall_RUS(266, 65, agressive_lost_souls ? RD_ON_RUS : RD_OFF_RUS,
                                      agressive_lost_souls ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Режим игры "Pistol start"
        M_WriteTextSmall_RUS(35, 75, "ht;bv buhs ^", NULL);
        M_WriteTextSmall_ENG(121, 75, "\"Pistol start\":", NULL);
        M_WriteTextSmall_RUS(229, 75, pistol_start ? RD_ON_RUS : RD_OFF_RUS,
                                      pistol_start ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Демозаписи
        //
        M_WriteTextSmall_RUS(35, 85, "Ltvjpfgbcb", cr[CR_YELLOW]);

        // Отображать таймер
        M_WriteTextSmall_RUS(180, 95, demotimer == 1 ? "ghb ghjbuhsdfybb"  :
                                      demotimer == 2 ? "ghb pfgbcb" :
                                      demotimer == 3 ? "dctulf" : "dsrk",
                                      demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Время таймера
        M_WriteTextSmall_RUS(145, 105, demotimerdir ? "jcnfdittcz" : "ghjitlitt",
                                                    demotimer > 0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Шкала прогресса
        M_WriteTextSmall_RUS(161, 115, demobar ? RD_ON_RUS : RD_OFF_RUS,
                                                    demobar ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Проигрывать демозаписи
        M_WriteTextSmall_RUS(219, 125, no_internal_demos ? RD_OFF_RUS : RD_ON_RUS,
                                                    no_internal_demos ? cr[CR_DARKRED] : cr[CR_GREEN]);

        //
        // Footer
        //
        M_WriteTextSmall_RUS(35, 145, RD_NEXT_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(35, 155, RD_PREV_RUS, cr[CR_WHITE]);
        M_WriteTextSmall_RUS(197, 155, "cnhfybwf 5*5", cr[CR_WHITE]);
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
            improved_fuzz = 4;
        break;
    
        case 1:
        improved_fuzz++;
        if (improved_fuzz > 4)
            improved_fuzz = 0;
        break;
    }

    // Redraw game screen
    R_ExecuteSetViewSize();
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

void M_RD_Change_LinearSky(int choice)
{
    linear_sky ^= 1;
}

void M_RD_Change_FlipWeapons(int choice)
{
    flip_weapons ^= 1;

    // [JN] Skip weapon bobbing interpolation for next frame.
    skippsprinterp = true;
}

//
// Gameplay: Status Bar
//

void M_RD_Change_ExtraPlayerFaces(int choice)
{
    extra_player_faces ^= 1;
}

void M_RD_Change_NegativeHealth(int choice)
{
    negative_health ^= 1;
}

void M_RD_Change_SBarColored(int choice)
{
    switch(choice)
    {
        case 0: 
        sbar_colored--;
        if (sbar_colored < 0) 
            sbar_colored = 2;
        break;
    
        case 1:
        sbar_colored++;
        if (sbar_colored > 2)
            sbar_colored = 0;
        break;
    }
    
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Define_SBarHighValue (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_high_set = NULL;
    }
    else
    {
        switch (sbar_color_high)
        {
            case 1:   sbar_color_high_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_high_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_high_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_high_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_high_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_high_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_high_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_high_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_high_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_high_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_high_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_high_set = cr[CR_TAN];        break;
            case 13:  sbar_color_high_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_high_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_high_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_high_set = cr[CR_PINK];      break;
            case 17:  sbar_color_high_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_high_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarHighValue (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_high--;
        if (sbar_color_high < 0) 
            sbar_color_high = 17;
        break;
    
        case 1:
        sbar_color_high++;
        if (sbar_color_high > 17)
            sbar_color_high = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarHighValue();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarNormalValue (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_normal_set = NULL;
    }
    else
    {
        switch (sbar_color_normal)
        {
            case 1:   sbar_color_normal_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_normal_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_normal_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_normal_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_normal_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_normal_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_normal_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_normal_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_normal_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_normal_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_normal_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_normal_set = cr[CR_TAN];        break;
            case 13:  sbar_color_normal_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_normal_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_normal_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_normal_set = cr[CR_PINK];      break;
            case 17:  sbar_color_normal_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_normal_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarNormalValue (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_normal--;
        if (sbar_color_normal < 0) 
            sbar_color_normal = 17;
        break;
    
        case 1:
        sbar_color_normal++;
        if (sbar_color_normal > 17)
            sbar_color_normal = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarNormalValue();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarLowValue (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_low_set = NULL;
    }
    else
    {
        switch (sbar_color_low)
        {
            case 1:   sbar_color_low_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_low_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_low_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_low_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_low_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_low_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_low_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_low_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_low_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_low_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_low_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_low_set = cr[CR_TAN];        break;
            case 13:  sbar_color_low_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_low_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_low_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_low_set = cr[CR_PINK];      break;
            case 17:  sbar_color_low_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_low_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarLowValue (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_low--;
        if (sbar_color_low < 0) 
            sbar_color_low = 17;
        break;
    
        case 1:
        sbar_color_low++;
        if (sbar_color_low > 17)
            sbar_color_low = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarLowValue();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarCriticalValue (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_critical_set = NULL;
    }
    else
    {
        switch (sbar_color_critical)
        {
            case 1:   sbar_color_critical_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_critical_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_critical_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_critical_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_critical_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_critical_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_critical_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_critical_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_critical_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_critical_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_critical_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_critical_set = cr[CR_TAN];        break;
            case 13:  sbar_color_critical_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_critical_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_critical_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_critical_set = cr[CR_PINK];      break;
            case 17:  sbar_color_critical_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_critical_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarCriticalValue (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_critical--;
        if (sbar_color_critical < 0) 
            sbar_color_critical = 17;
        break;
    
        case 1:
        sbar_color_critical++;
        if (sbar_color_critical > 17)
            sbar_color_critical = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarCriticalValue();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarArmorType1 (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_armor_1_set = NULL;
    }
    else
    {
        switch (sbar_color_armor_1)
        {
            case 1:   sbar_color_armor_1_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_armor_1_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_armor_1_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_armor_1_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_armor_1_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_armor_1_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_armor_1_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_armor_1_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_armor_1_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_armor_1_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_armor_1_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_armor_1_set = cr[CR_TAN];        break;
            case 13:  sbar_color_armor_1_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_armor_1_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_armor_1_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_armor_1_set = cr[CR_PINK];      break;
            case 17:  sbar_color_armor_1_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_armor_1_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarArmorType1 (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_armor_1--;
        if (sbar_color_armor_1 < 0) 
            sbar_color_armor_1 = 17;
        break;
    
        case 1:
        sbar_color_armor_1++;
        if (sbar_color_armor_1 > 17)
            sbar_color_armor_1 = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarArmorType1();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarArmorType2 (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_armor_2_set = NULL;
    }
    else
    {
        switch (sbar_color_armor_2)
        {
            case 1:   sbar_color_armor_2_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_armor_2_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_armor_2_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_armor_2_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_armor_2_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_armor_2_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_armor_2_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_armor_2_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_armor_2_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_armor_2_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_armor_2_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_armor_2_set = cr[CR_TAN];        break;
            case 13:  sbar_color_armor_2_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_armor_2_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_armor_2_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_armor_2_set = cr[CR_PINK];      break;
            case 17:  sbar_color_armor_2_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_armor_2_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarArmorType2 (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_armor_2--;
        if (sbar_color_armor_2 < 0) 
            sbar_color_armor_2 = 17;
        break;
    
        case 1:
        sbar_color_armor_2++;
        if (sbar_color_armor_2 > 17)
            sbar_color_armor_2 = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarArmorType2();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Define_SBarArmorType0 (void)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        sbar_color_armor_0_set = NULL;
    }
    else
    {
        switch (sbar_color_armor_0)
        {
            case 1:   sbar_color_armor_0_set = cr[CR_DARKRED];    break;
            case 2:   sbar_color_armor_0_set = cr[CR_GREEN];      break;
            case 3:   sbar_color_armor_0_set = cr[CR_DARKGREEN];  break;
            case 4:   sbar_color_armor_0_set = cr[CR_OLIVE];      break;
            case 5:   sbar_color_armor_0_set = cr[CR_BLUE2];      break;
            case 6:   sbar_color_armor_0_set = cr[CR_DARKBLUE];   break;
            case 7:   sbar_color_armor_0_set = cr[CR_YELLOW];       break;
            case 8:   sbar_color_armor_0_set = cr[CR_ORANGE];     break;
            case 9:   sbar_color_armor_0_set = cr[CR_WHITE];      break;
            case 10:  sbar_color_armor_0_set = cr[CR_GRAY];       break;
            case 11:  sbar_color_armor_0_set = cr[CR_DARKGRAY];   break;
            case 12:  sbar_color_armor_0_set = cr[CR_TAN];        break;
            case 13:  sbar_color_armor_0_set = cr[CR_BROWN];    break;
            case 14:  sbar_color_armor_0_set = cr[CR_ALMOND];      break;
            case 15:  sbar_color_armor_0_set = cr[CR_KHAKI];  break;
            case 16:  sbar_color_armor_0_set = cr[CR_PINK];      break;
            case 17:  sbar_color_armor_0_set = cr[CR_BURGUNDY];  break;
            default:  sbar_color_armor_0_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarArmorType0 (int choice)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
    return;

    switch(choice)
    {
        case 0: 
        sbar_color_armor_0--;
        if (sbar_color_armor_0 < 0) 
            sbar_color_armor_0 = 17;
        break;
    
        case 1:
        sbar_color_armor_0++;
        if (sbar_color_armor_0 > 17)
            sbar_color_armor_0 = 0;
        break;
    }

    // [JN] Redefine system message color.
    M_RD_Define_SBarArmorType0();

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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
            // Set full bright colormap
            players[consoleplayer].fixedcolormap = 1;
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
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NEWGAME : NEWGAME_RUS),
                                  NULL,false);
        return;
    }
    else
    {
        M_SetupNextMenu(english_language ? 
                        &RD_Level_Def_1 :
                        &RD_Level_Def_1_Rus);
    }
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

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(2, "LEVEL SELECT");
        
        // Skill level
        M_snprintf(num, 4, "%d", selective_skill+1);
        M_WriteTextSmall_ENG(226, 20, num, NULL);

        // Episode
        if (logical_gamemission != doom)
        {
            M_WriteTextSmall_ENG(226, 30, "N/A", cr[CR_DARKRED]);
        }
        else
        {
            if (gamemode == shareware)
            {
                M_WriteTextSmall_ENG(226, 30, "1", cr[CR_DARKRED]);
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                M_WriteTextSmall_ENG(226, 30, num, NULL);
            }
        }

        // Map
        if (gamemode == pressbeta)
        {
            M_WriteTextSmall_ENG(226, 40, "1", cr[CR_DARKRED]);
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            M_WriteTextSmall_ENG(226, 40, num, NULL);
        }

        //
        // Player
        //
        M_WriteTextSmall_ENG(75, 50, "PLAYER", cr[CR_YELLOW]);

        // Health

        M_snprintf(num, 4, "%d", selective_health);
        M_WriteTextSmall_ENG(226, 60, num, selective_health > 100 ? cr[CR_BLUE2]  :
                                           selective_health >= 67 ? cr[CR_GREEN]  :
                                           selective_health >= 34 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Armor
        M_snprintf(num, 4, "%d", selective_armor);
        M_WriteTextSmall_ENG(226, 70, num, selective_armor == 0 ? cr[CR_RED] : 
                                           selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2]);

        // Armor type
        M_snprintf(num, 4, "%d", selective_armortype);
        M_WriteTextSmall_ENG(226, 80, num, selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2]);

        //
        // Weapons
        //
        M_WriteTextSmall_ENG(75, 90, "WEAPONS", cr[CR_YELLOW]);

        // Chainsaw
        M_WriteTextSmall_ENG(226, 100, selective_wp_chainsaw ? "YES" : "NO",
                                       selective_wp_chainsaw ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Shotgun
        M_WriteTextSmall_ENG(226, 110, selective_wp_shotgun ? "YES" : "NO",
                                       selective_wp_shotgun ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Super Shotgun
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            M_WriteTextSmall_ENG(226, 120, "N/A", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(226, 120, selective_wp_supershotgun ? "YES" : "NO",
                                           selective_wp_supershotgun ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // Chaingun
        M_WriteTextSmall_ENG(226, 130, selective_wp_chaingun ? "YES" : "NO",
                                       selective_wp_chaingun ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Rocket Launcher
        M_WriteTextSmall_ENG(226, 140, selective_wp_missile ? "YES" : "NO",
                                       selective_wp_missile ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Plasma Gun
        if (gamemode == shareware)
        {
            // Not available in shareware
            M_WriteTextSmall_ENG(226, 150, "N/A", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(226, 150, selective_wp_plasma ? "YES" : "NO",
                                           selective_wp_plasma ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            M_WriteTextSmall_ENG(226, 160, "N/A", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_ENG(226, 160, selective_wp_bfg ? "YES" : "NO",
                                           selective_wp_bfg ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // Next page
        M_WriteTextSmall_ENG(75, 180, "NEXT PAGE", cr[CR_WHITE]);

        // Start game
        M_WriteTextSmall_ENG(75, 190, "START GAME", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(2, "DS<JH EHJDYZ");

        // Сложность
        M_snprintf(num, 4, "%d", selective_skill+1);
        M_WriteTextSmall_ENG(226, 20, num, NULL);

        // Эпизод
        if (logical_gamemission != doom)
        {
            M_WriteTextSmall_RUS(226, 30, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            if (gamemode == shareware)
            {
                M_WriteTextSmall_ENG(226, 30, "1", cr[CR_DARKRED]);
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                M_WriteTextSmall_ENG(226, 30, num, NULL);
            }
        }

        // Уровень
        if (gamemode == pressbeta)
        {
            M_WriteTextSmall_ENG(226, 40, "1", cr[CR_DARKRED]);
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            M_WriteTextSmall_ENG(226, 40, num, NULL);
        }

        //
        // Игрок
        //
        M_WriteTextSmall_RUS(72, 50, "buhjr", cr[CR_YELLOW]);

        // Здоровье
        M_snprintf(num, 4, "%d", selective_health);
        M_WriteTextSmall_ENG(226, 60, num, selective_health > 100 ? cr[CR_BLUE2]  :
                                           selective_health >= 67 ? cr[CR_GREEN]  :
                                           selective_health >= 34 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Броня
        M_snprintf(num, 4, "%d", selective_armor);
        M_WriteTextSmall_ENG(226, 70, num, selective_armor == 0 ? cr[CR_RED] :
                                           selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2]);

        // Тип брони
        M_snprintf(num, 4, "%d", selective_armortype);
        M_WriteTextSmall_ENG(226, 80, num, selective_armortype == 1 ? cr[CR_GREEN] : cr[CR_BLUE2]);

        //
        // Оружие
        //
        M_WriteTextSmall_RUS(72, 90, "jhe;bt", cr[CR_YELLOW]);

        // Бензопила
        M_WriteTextSmall_RUS(226, 100, selective_wp_chainsaw ? "lf" : "ytn",
                                       selective_wp_chainsaw ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Ружье
        M_WriteTextSmall_RUS(226, 110, selective_wp_shotgun ? "lf" : "ytn",
                                       selective_wp_shotgun ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Двуствольное ружье
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            M_WriteTextSmall_RUS(226, 120, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(226, 120, selective_wp_supershotgun ? "lf" : "ytn",
                                           selective_wp_supershotgun ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // Пулемет
        M_WriteTextSmall_RUS(226, 130, selective_wp_chaingun ? "lf" : "ytn",
                                       selective_wp_chaingun ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Ракетница
        M_WriteTextSmall_RUS(226, 140, selective_wp_missile ? "lf" : "ytn",
                                       selective_wp_missile ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Плазменная пушка
        if (gamemode == shareware)
        {
            // Not available in shareware
            M_WriteTextSmall_RUS(226, 150, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(226, 150, selective_wp_plasma ? "lf" : "ytn",
                                           selective_wp_plasma ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            M_WriteTextSmall_RUS(226, 160, "y*l", cr[CR_DARKRED]);
        }
        else
        {
            M_WriteTextSmall_RUS(226, 160, selective_wp_bfg ? "lf" : "ytn",
                                           selective_wp_bfg ? cr[CR_GREEN] : cr[CR_DARKRED]);
        }

        // Следующая страница
        M_WriteTextSmall_RUS(72, 180, "cktle.ofz cnhfybwf", cr[CR_WHITE]);

        // Начать игру
        M_WriteTextSmall_RUS(72, 190, "yfxfnm buhe", cr[CR_WHITE]);
    }
}

void M_RD_Draw_Level_2 (void)
{
    static char num[4];

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        M_WriteTextBigCentered_ENG(2, "LEVEL SELECT");

        // Backpack
        M_WriteTextSmall_ENG(226, 20, selective_backpack ? "YES" : "NO",
                                      selective_backpack ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Bullets
        M_snprintf(num, 4, "%d", selective_ammo_0);
        M_WriteTextSmall_ENG(226, 30, num, selective_ammo_0 >  200 ? cr[CR_BLUE2]  :
                                           selective_ammo_0 >= 100 ? cr[CR_GREEN]  :
                                           selective_ammo_0 >= 50  ? cr[CR_YELLOW] :
                                                                     cr[CR_RED]);

        // Shells
        M_snprintf(num, 4, "%d", selective_ammo_1);
        M_WriteTextSmall_ENG(226, 40, num, selective_ammo_1 >  50 ? cr[CR_BLUE2]  :
                                           selective_ammo_1 >= 25 ? cr[CR_GREEN]  :
                                           selective_ammo_1 >= 12 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Rockets
        M_snprintf(num, 4, "%d", selective_ammo_3);
        M_WriteTextSmall_ENG(226, 50, num, selective_ammo_3 >  50 ? cr[CR_BLUE2]  :
                                           selective_ammo_3 >= 25 ? cr[CR_GREEN]  :
                                           selective_ammo_3 >= 12 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Cells
        M_snprintf(num, 4, "%d", selective_ammo_2);
        M_WriteTextSmall_ENG(226, 60, num, selective_ammo_2 >  300 ? cr[CR_BLUE2]  :
                                           selective_ammo_2 >= 150 ? cr[CR_GREEN]  :
                                           selective_ammo_2 >=  75 ? cr[CR_YELLOW] :
                                                                     cr[CR_RED]);

        //
        // Keys
        //
        M_WriteTextSmall_ENG(75, 70, "KEYS", cr[CR_YELLOW]);

        // Blue keycard
        M_WriteTextSmall_ENG(226, 80, selective_key_0 ? "YES" : "NO",
                                      selective_key_0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Yellow keycard
        M_WriteTextSmall_ENG(226, 90, selective_key_1 ? "YES" : "NO",
                                      selective_key_1 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Red keycard
        M_WriteTextSmall_ENG(226, 100, selective_key_2 ? "YES" : "NO",
                                       selective_key_2 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Blue skull key
        M_WriteTextSmall_ENG(226, 110, selective_key_3 ? "YES" : "NO",
                                       selective_key_3 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Yellow skull key
        M_WriteTextSmall_ENG(226, 120, selective_key_4 ? "YES" : "NO",
                                       selective_key_4 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Red skull key
        M_WriteTextSmall_ENG(226, 130, selective_key_5 ? "YES" : "NO",
                                       selective_key_5 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Extra
        //
        M_WriteTextSmall_ENG(75, 140, "EXTRA", cr[CR_YELLOW]);

        // Fast monsters
        M_WriteTextSmall_ENG(226, 150, selective_fast ? "YES" : "NO",
                                       selective_fast ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Respawning monsters
        M_WriteTextSmall_ENG(226, 160, selective_respawn ? "YES" : "NO",
                                       selective_respawn ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Previous page
        M_WriteTextSmall_ENG(75, 180, "PREVIOUS PAGE", cr[CR_WHITE]);

        // Start game
        M_WriteTextSmall_ENG(75, 190, "START GAME", cr[CR_WHITE]);
    }
    else
    {
        M_WriteTextBigCentered_RUS(2, "DS<JH EHJDYZ");

        // Рюкзак
        M_WriteTextSmall_RUS(226, 20, selective_backpack ? "lf" : "ytn",
                                      selective_backpack ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Пули
        M_snprintf(num, 4, "%d", selective_ammo_0);
        M_WriteTextSmall_ENG(226, 30, num, selective_ammo_0 >  200 ? cr[CR_BLUE2]  :
                                           selective_ammo_0 >= 100 ? cr[CR_GREEN]  :
                                           selective_ammo_0 >= 50  ? cr[CR_YELLOW] :
                                                                     cr[CR_RED]);

        // Дробь
        M_snprintf(num, 4, "%d", selective_ammo_1);
        M_WriteTextSmall_ENG(226, 40, num, selective_ammo_1 >  50 ? cr[CR_BLUE2]  :
                                           selective_ammo_1 >= 25 ? cr[CR_GREEN]  :
                                           selective_ammo_1 >= 12 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Ракеты
        M_snprintf(num, 4, "%d", selective_ammo_3);
        M_WriteTextSmall_ENG(226, 50, num, selective_ammo_3 >  50 ? cr[CR_BLUE2]  :
                                           selective_ammo_3 >= 25 ? cr[CR_GREEN]  :
                                           selective_ammo_3 >= 12 ? cr[CR_YELLOW] :
                                                                    cr[CR_RED]);

        // Энергия
        M_snprintf(num, 4, "%d", selective_ammo_2);
        M_WriteTextSmall_ENG(226, 60, num, selective_ammo_2 >  300 ? cr[CR_BLUE2]  :
                                           selective_ammo_2 >= 150 ? cr[CR_GREEN]  :
                                           selective_ammo_2 >=  75 ? cr[CR_YELLOW] :
                                                                     cr[CR_RED]);

        //
        // Ключи
        //
        M_WriteTextSmall_RUS(72, 70, "rk.xb", cr[CR_YELLOW]);

        // Синяя ключ-карта
        M_WriteTextSmall_RUS(226, 80, selective_key_0 ? "lf" : "ytn",
                                      selective_key_0 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Желтая ключ-карта
        M_WriteTextSmall_RUS(226, 90, selective_key_1 ? "lf" : "ytn",
                                      selective_key_1 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Красная ключ-карта
        M_WriteTextSmall_RUS(226, 100, selective_key_2 ? "lf" : "ytn",
                                       selective_key_2 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Синий ключ-череп
        M_WriteTextSmall_RUS(226, 110, selective_key_3 ? "lf" : "ytn",
                                       selective_key_3 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Желтый ключ-череп
        M_WriteTextSmall_RUS(226, 120, selective_key_4 ? "lf" : "ytn",
                                       selective_key_4 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Красный ключ-череп
        M_WriteTextSmall_RUS(226, 130, selective_key_5 ? "lf" : "ytn",
                                       selective_key_5 ? cr[CR_GREEN] : cr[CR_DARKRED]);

        //
        // Монстры
        //
        M_WriteTextSmall_RUS(72, 140, "vjycnhs", cr[CR_YELLOW]);

        // Ускоренные
        M_WriteTextSmall_RUS(226, 150, selective_fast ? "lf" : "ytn",
                                       selective_fast ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Воскрешающиеся
        M_WriteTextSmall_RUS(226, 160, selective_respawn ? "lf" : "ytn",
                                       selective_respawn ? cr[CR_GREEN] : cr[CR_DARKRED]);

        // Предыдущая страница
        M_WriteTextSmall_RUS(72, 180, "ghtlsleofz cnhfybwf", cr[CR_WHITE]);

        // Начать игру
        M_WriteTextSmall_RUS(72, 190, "yfxfnm buhe", cr[CR_WHITE]);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_Selective_WP_BFG9000 (int choice)
{
    // Not available in shareware
    if (gamemode == shareware)
    return;

    selective_wp_bfg ^= 1;

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
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
{
    M_SetupNextMenu(english_language ?
                    &RD_Reset_Def :
                    &RD_Reset_Def_Rus);
}

void M_RD_Draw_Reset(void)
{   
    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
    inhelpscreens = true;

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
            M_WriteTextSmall_RUS(85, 155, "jhbubyfkmyjuj", cr[CR_DARKRED]);         // оригинального Doom
            M_WriteTextSmall_ENG(193, 155, "Doom", cr[CR_DARKRED]);
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
    hud_detaillevel       = 0;
    local_time            = 0;
    showMessages          = 1;
    messages_timeout      = 4;
    message_pickup_color  = 0;
    message_secret_color  = 7;
    message_system_color  = 0;
    message_chat_color    = 2;
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
    snd_channels         = 32;
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
    colored_blood    = 1;
    swirling_liquids = 1;
    invul_sky        = 1;
    linear_sky       = 1;
    flip_weapons     = 0;

    // Gameplay: Status Bar
    sbar_colored        = 0;
    sbar_color_high     = 5;
    sbar_color_normal   = 2;
    sbar_color_low      = 7;
    sbar_color_critical = 0;
    sbar_color_armor_1  = 2;
    sbar_color_armor_2  = 5;
    sbar_color_armor_0  = 0;

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
    floating_powerups    = 1;
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

    // Reset Automap color scheme
    AM_initColors();

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
    hud_detaillevel       = 1;
    local_time            = 0;
    showMessages          = 1;
    messages_timeout      = 4;
    message_pickup_color  = 0;
    message_secret_color  = 0;
    message_system_color  = 0;
    message_chat_color    = 0;
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
    snd_channels         = 8;
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
    colored_blood    = 0;
    swirling_liquids = 0;
    invul_sky        = 0;
    linear_sky       = 0;
    flip_weapons     = 0;

    // Gameplay: Status Bar
    sbar_colored        = 0;
    sbar_color_high     = 5;
    sbar_color_normal   = 2;
    sbar_color_low      = 7;
    sbar_color_critical = 0;
    sbar_color_armor_1  = 2;
    sbar_color_armor_2  = 5;
    sbar_color_armor_0  = 0;

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

    // Reset Automap color scheme
    AM_initColors();

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
    extern void HU_Stop(void);
    extern void ST_createWidgetsJaguar(void);
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
        ST_doRefresh();
        // Update ARMS/FRAGS widget
        if (gamemission == jaguar)
        ST_createWidgetsJaguar();
        else
        ST_createWidgets(); 
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
        V_DrawShadowedPatchDoom(LoadDef_x, LoadDef_y,
                                W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));
    }
    else
    {
        // ЗАГРУЗИТЬ ИГРУ
        M_WriteTextBigCentered_RUS(LoadDef_y, "PFUHEPBNM BUHE");
    }

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x, LoadDef.y+LINEHEIGHT*i);

        // [crispy] shade empty savegame slots
        if (!LoadMenu[i].status && !vanillaparm)
        dp_translation = cr[CR_DARKRED];

        M_WriteText(LoadDef.x, LoadDef.y+LINEHEIGHT*i, savegamestrings[i]);

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
        V_DrawShadowedPatchDoom(SaveDef_x, SaveDef_y,
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

    V_DrawPatchFullScreen (W_CacheLumpName(lumpname, PU_CACHE), false);

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

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String
               (english_language ? "HELP1" : "HELP1R"), PU_CACHE), false);

    ReadDef2.x = ReadDef2_Rus.x = 330;
    ReadDef2.y = ReadDef2_Rus.y = 175;
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    inhelpscreens = true;

    if (english_language)
    {
        // [JN] Always draw original "M_DOOM" in English language
        V_DrawPatch(94 + wide_delta, 2, W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
    }
    else
    {
        // [JN] Draw translated titles for Plutonia and TNT
        V_DrawPatch(94 + wide_delta, 2, W_CacheLumpName
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
    inhelpscreens = true;

    if (english_language)
    {
        V_DrawShadowedPatchDoom(96, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
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
    inhelpscreens = true;

    if (english_language)
    {
        V_DrawShadowedPatchDoom(96, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
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
        // [JN] Return to Episode menu.
        M_SetupNextMenu(english_language ? &EpiDef : &EpiDef_Rus);
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
    // [JN] Do not close Episode menu after closing "purchase entire trilogy" message in Shareware.
    &&  (currentMenu != &EpiDef && gamemode == shareware)
    &&  (currentMenu != &EpiDef_Rus && gamemode == shareware)
    // [JN] Do not close Options menu after pressing "N" in End Game.
    &&  currentMenu != &RD_Options_Def
    &&  currentMenu != &RD_Options_Def_Rus
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

    // [JN] Allow detail toggle even while active menu.
    if (key == key_menu_detail)
    {
        M_RD_Change_Detail(0);
        S_StartSound(NULL,sfx_swtchn);
        return true;
    }
    // [JN] Allow to toggle messages even while active menu.
    else if (key == key_menu_messages)
    {
        M_RD_Change_Messages(0);
        S_StartSound(NULL,sfx_swtchn);
        return true;
    }
    // [JN] Allow gamma toggling even while active menu.
    else if (key == key_menu_gamma)
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
		return true;
	}

    // [JN] Allow screen increasing even while active menu.
	if (key == key_menu_incscreen) // Screen size up
	{
		if (automapactive || chat_on)
		return false;
		M_RD_Change_ScreenSize(1);
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
            // [JN] Play sound only if there are few menu items.
            if (currentMenu->numitems > 1)
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
            // [JN] Play sound only if there are few menu items.
            if (currentMenu->numitems > 1)
            S_StartSound(NULL,sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

    if (currentMenu->menuitems[itemOn].routine
    && (currentMenu->menuitems[itemOn].status == 2 
    ||  currentMenu->menuitems[itemOn].status == 3))
    {
        if (currentMenu->menuitems[itemOn].status == 2)
        {
            // [JN] Routine №3: play sound only if necessary.
            S_StartSound(NULL,sfx_stnmov);
        }
            
        currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine
        && (currentMenu->menuitems[itemOn].status == 2 
        ||  currentMenu->menuitems[itemOn].status == 3))
        {
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                // [JN] Routine №3: play sound only if necessary.
                S_StartSound(NULL,sfx_stnmov);
            }

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
            if (currentMenu->menuitems[itemOn].status == 2
            ||  currentMenu->menuitems[itemOn].status == 3)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                if (currentMenu->menuitems[itemOn].status == 2)
                {
                    // [JN] Routine №3: play sound only if necessary.
                    S_StartSound(NULL,sfx_stnmov);
                }
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
                           &RD_Gameplay_Def_5 :
                           &RD_Gameplay_Def_5_Rus);
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
        if (currentMenu == &RD_Gameplay_Def_5
        ||  currentMenu == &RD_Gameplay_Def_5_Rus)
        {
            M_SetupNextMenu(english_language ?
                           &RD_Gameplay_Def_4 :
                           &RD_Gameplay_Def_4_Rus);
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
                           &RD_Gameplay_Def_5 :
                           &RD_Gameplay_Def_5_Rus);
            S_StartSound(NULL,sfx_pistol);
            return true;
        }
        if (currentMenu == &RD_Gameplay_Def_5
        ||  currentMenu == &RD_Gameplay_Def_5_Rus)
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

        M_WriteTextSmall_ENG(0, line * 8, curr, NULL);
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
                M_WriteText(x, y, string);
                y += SHORT(hu_font[0]->height);
            }
            else
            {
                M_WriteTextSmall_RUS(x, y, string, NULL);
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
            V_DrawShadowedPatchDoom (x, y, W_CacheLumpName(name, PU_CACHE));

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
            M_WriteTextBig_RUS(x, y, name);

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
        ||  currentMenu == &RD_Gameplay_Def_5
        ||  currentMenu == &RD_Level_Def_1
        ||  currentMenu == &RD_Level_Def_2
        ||  currentMenu == &RD_Reset_Def)
        {
            M_WriteTextSmall_ENG(x, y, name, NULL);

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
        ||  currentMenu == &RD_Gameplay_Def_5_Rus
        ||  currentMenu == &RD_Level_Def_1_Rus
        ||  currentMenu == &RD_Level_Def_2_Rus
        ||  currentMenu == &RD_Reset_Def_Rus)
        {
            M_WriteTextSmall_RUS(x, y, name, NULL);
        
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
        V_DrawShadowedPatchDoom(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
        W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
    }
    else
    {
        // [JN] Draw blinking ">" symbol
        // [JN] Jaguar: no font color translation, draw SKULL1 as an empty symbol.
        M_WriteTextSmall_ENG(x + SKULLXOFF + 24, currentMenu->y + itemOn*LINEHEIGHT_SML,
                             gamemission == jaguar && whichSkull == 0 ? " " : ">",
                             whichSkull == 0 ? cr[CR_DARKRED] : NULL);
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

    // [JN] Init message colors.
    M_RD_Define_Msg_Pickup_Color();
    M_RD_Define_Msg_Secret_Color();
    M_RD_Define_Msg_System_Color();
    M_RD_Define_Msg_Chat_Color();

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


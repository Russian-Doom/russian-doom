//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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
// D_main.c



#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "doomfeatures.h"

#include "txt_main.h"
#include "txt_io.h"

#include "net_client.h"
#include "am_map.h"
#include "config.h"
#include "ct_chat.h"
#include "doomdef.h"
#include "deh_main.h"
#include "d_iwad.h"
#include "d_name.h"
#include "i_endoom.h"
#include "i_controller.h"
#include "i_input.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "p_local.h"
#include "rd_keybinds.h"
#include "s_sound.h"
#include "w_main.h"
#include "v_video.h"
#include "w_merge.h"
#include "jn.h"

#define STARTUP_WINDOW_X 17
#define STARTUP_WINDOW_Y 7

// -----------------------------------------------------------------------------
// [Dasperal] d_name.h var definition
// -----------------------------------------------------------------------------
char* RD_Project_Name = PACKAGE_PREFIX " Heretic";
char* RD_Project_String = PACKAGE_PREFIX " Heretic " BUILD_HERETIC_VERSION;
GameType_t RD_GameType = gt_Heretic;

GameMode_t gamemode = indetermined;
char *gamedescription = "unknown";

boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean debugmode;              // checkparm of -debug
boolean ravpic;                 // checkparm of -ravpic
boolean cdrom;                  // true if cd-rom mode active
boolean noartiskip;             // whether shift-enter skips an artifact
boolean realframe, skippsprinterp; // [JN] Interpolation for weapon bobbing

skill_t startskill;
int startepisode;
int startmap;
int UpdateState;
static boolean main_loop_started = false;
boolean autostart;

boolean advancedemo;

FILE *debugfile;

// -----------------------------------------------------------------------------
// [JN] Support for English language.
// Windows OS only: do not set game language on first launch, 
// try to determine it automatically in D_DoomMain.
// On other OSes just use English by default.
// -----------------------------------------------------------------------------

#ifdef _WIN32
int english_language = -1;
#else
int english_language = 1;
#endif

/*
================================================================================
=
= [JN] PWAD autoloading. Initially all 4 values are empty.
=
================================================================================
*/

static char *autoloadglobalpwad[10]     = { "", "", "", "" };
static char *autoloadregisteredpwad[10] = { "", "", "", "" };
static char *autoloadretailpwad[10]     = { "", "", "", "" };

//------------------------------------------------------------------------------
//
// [JN] Default variables and bindings.
//
//------------------------------------------------------------------------------

// Rendering
int show_endoom = 0;
int flashing_hom = 0;

// Display
int screenblocks = 10;
int level_brightness = 0;
int detailLevel = 0;        // Blocky mode, 0 = high, 1 = normal
int hud_detaillevel = 0;    // Blocky mode, 0 = high, 1 = normal

// Messages and Texts
int show_messages = 1;
int messages_alignment = 0;
int messages_timeout = 4;
int message_fade = 1;
int draw_shadowed_text = 1;
int local_time = 0;
int message_pickup_color = 0;
int message_secret_color = 15;
int message_system_color = 0;
int message_chat_color = 5;

// Automap
int automap_overlay = 0;
int automap_rotate = 0;
int automap_follow = 1;
int automap_grid = 0;
int automap_grid_size = 128;
int automap_mark_color = 6;
int automap_stats = 1;
int automap_level_time = 1;
int automap_total_time = 0;
int automap_coords = 0;

// Sound
int snd_MaxVolume = 10;
int snd_MusicVolume = 10;
int snd_Channels = 8;       // Default SFX channels
int snd_Channels_RD;        // For hot-swapping
int snd_monomode = 0;

// Controls
extern int alwaysRun;

// Gameplay: Graphical
int brightmaps = 1;
int fake_contrast = 0;
int translucency = 1;
int sbar_colored = 0;
int sbar_colored_gem = 0;
int colored_blood = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int linear_sky = 1;
int flip_weapons = 0;

// Gameplay: Status bar
int ammo_widget = 0;
int ammo_widget_colored = 1;

// Gameplay: Physical
int improved_collision = 1;
int torque = 1;
int weapon_bobbing = 1;
int randomly_flipcorpses = 1;
int floating_powerups = 1;

// Gameplay: Audible
int z_axis_sfx = 0;
int noise_alert_sfx = 0;

// Gameplay: Tactical
int secret_notification = 1;
int show_all_artifacts = 0;
int show_artifacts_timer = 0;
int negative_health = 0;

// Gameplay: Crosshair
int crosshair_draw = 0;
int crosshair_type = 1;
int crosshair_scale = 0;

// Gameplay: Gameplay
int fix_map_errors = 1;
int flip_levels = 0;
int no_internal_demos = 0;
int pistol_start = 0;

// Level Select (1)
int selective_skill = 2;
int selective_episode = 1;
int selective_map = 1;
int selective_health = 100;
int selective_armor = 0;
int selective_armortype = 1;
int selective_wp_gauntlets = 0;
int selective_wp_crossbow = 0;
int selective_wp_dragonclaw = 0;
int selective_wp_hellstaff = 0;
int selective_wp_phoenixrod = 0;
int selective_wp_firemace = 0;

// Level Select (2)
int selective_backpack = 0;
int selective_ammo_0 = 50;  // wand crystals
int selective_ammo_1 = 0;   // ethereal arrows
int selective_ammo_2 = 0;   // claw orbs
int selective_ammo_3 = 0;   // hellstaff runes
int selective_ammo_4 = 0;   // flame orbs
int selective_ammo_5 = 0;   // mace spheres
int selective_key_0 = 0;    // yellow key
int selective_key_1 = 0;    // green key
int selective_key_2 = 0;    // blue key
int selective_fast = 0;
int selective_respawn = 0;

// Level Select (3)
int selective_arti_0 = 0;   // Quartz Flask
int selective_arti_1 = 0;   // Mystic Urn
int selective_arti_2 = 0;   // Timebomb
int selective_arti_3 = 0;   // Tome of Power
int selective_arti_4 = 0;   // Ring of Invincibility
int selective_arti_5 = 0;   // Morph Ovum
int selective_arti_6 = 0;   // Chaos Device
int selective_arti_7 = 0;   // Shadowsphere
int selective_arti_8 = 0;   // Wings of Wrath
int selective_arti_9 = 0;   // Torch


void D_ConnectNetGame(void);
void D_CheckNetGame(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
boolean F_Responder(event_t * ev);

boolean hasDelayEvents = false;

//---------------------------------------------------------------------------
//
// PROC D_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//---------------------------------------------------------------------------

void D_ProcessEvents(void)
{
    event_t *ev;

    while((ev = D_PopEvent()) != NULL)
    {
        if(ev->type == ev_delay)
        {
            hasDelayEvents = false;
            break;
        }
        if(ev->delayed)
        {
            ev->delayed = false;
            if(!hasDelayEvents)
            {
                event_t delayEvent;
                delayEvent.type = ev_delay;
                delayEvent.delayed = false;
                delayEvent.data1 = delayEvent.data2 = delayEvent.data3 = delayEvent.data4 = 0;
                D_PostEvent(&delayEvent);
                hasDelayEvents = true;
            }
            D_PostEvent(ev);
            continue;
        }

        if (F_Responder(ev))
        {
            continue;
        }
        if (MN_Responder(ev))
        {
            continue;
        }
        G_Responder(ev);
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMessage
//
//---------------------------------------------------------------------------

void DrawMessage(void)
{
    player_t *player = &players[consoleplayer];
    int wide_4_3 = aspect_ratio >= 2 && screenblocks == 9 ? wide_delta : 0;

    // [JN] Activate message counter in non-level or paused states.
    // Make messages go away in menu, finale and help screens.
    if (gamestate != GS_LEVEL || paused)
    {
        player->messageTics--;  // Can go negative
    }

    if (player->messageTics <= 0 || !player->message)
    {                           // No message
        return;
    }

    // [JN] Colorize depending on given color type.
    switch (player->messageType)
    {
        case msg_pickup: // Item pickup.
            dp_translation = messages_pickup_color_set == CR_NONE ? NULL : cr[messages_pickup_color_set];
            break;
        case msg_secret: // Revealed secret
            dp_translation = messages_secret_color_set == CR_NONE ? NULL : cr[messages_secret_color_set];
            break;
        case msg_system: // System message
            dp_translation = messages_system_color_set == CR_NONE ? NULL : cr[messages_system_color_set];
            break;
        case msg_chat: // Netgame chat
            dp_translation = messages_chat_color_set == CR_NONE ? NULL : cr[messages_chat_color_set];
            break;
        case msg_uncolored: // Not supposed to be colored.
        default:
            break;
    }

    if (english_language)
    {
        if (player->messageTics < 10 && message_fade && !vanillaparm)
        {
            RD_M_DrawTextAFade(player->message,
                           messages_alignment == 0 ? 160 - RD_M_TextAWidth(player->message) / 2 + wide_delta :  // centered
                           messages_alignment == 1 ? 4 + wide_4_3 :   // left edge of the screen
                                                     wide_delta, 1,   // left edge of the status bar
                           player->messageTics >= 9 ? transtable90 :
                           player->messageTics >= 8 ? transtable80 :
                           player->messageTics >= 7 ? transtable70 :
                           player->messageTics >= 6 ? transtable60 :
                           player->messageTics >= 5 ? transtable50 :
                           player->messageTics >= 4 ? transtable40 :
                           player->messageTics >= 3 ? transtable30 :
                           player->messageTics >= 2 ? transtable20 :
                                                      transtable10);
        }
        else
        {
            RD_M_DrawTextA(player->message,
                       messages_alignment == 0 ? 160 - RD_M_TextAWidth(player->message) / 2 + wide_delta :  // centered
                       messages_alignment == 1 ? 4 + wide_4_3 :       // left edge of the screen
                                                 wide_delta, 1);      // left edge of the status bar
        }
    }
    else
    {
        if (player->messageTics < 10 && message_fade && !vanillaparm)
        {
            RD_M_DrawTextSmallRUSFade(player->message,
                                      messages_alignment == 0 ? 160 - RD_M_TextSmallRUSWidth(player->message) / 2 + wide_delta :  // по центру
                                      messages_alignment == 1 ? 4 + wide_4_3 :      // по краю экрана
                                                                    wide_delta, 1,  // по краю статус-бара
                                      player->messageTics >= 9 ? transtable90 :
                                      player->messageTics >= 8 ? transtable80 :
                                      player->messageTics >= 7 ? transtable70 :
                                      player->messageTics >= 6 ? transtable60 :
                                      player->messageTics >= 5 ? transtable50 :
                                      player->messageTics >= 4 ? transtable40 :
                                      player->messageTics >= 3 ? transtable30 :
                                      player->messageTics >= 2 ? transtable20 :
                                                                 transtable10);
        }
        else
        {
            RD_M_DrawTextSmallRUSFade(player->message,
                                  messages_alignment == 0 ? 160 - RD_M_TextSmallRUSWidth(player->message) / 2 + wide_delta :  // по центру
                                  messages_alignment == 1 ? 4 + wide_4_3 :           // по краю экрана
                                                            wide_delta, 1, NULL); // по краю статус-бара
        }
    }

    // [JN] Clear color translation.
    dp_translation = NULL;
}

/*
================================================================================
=
= DrawTimeAndFPS
=
= [JN] Draws local time and FPS widgets.
=
================================================================================
*/

void DrawTimeAndFPS(void)
{
    const boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);

    if (!vanillaparm)
    {
        if (local_time)
        {
            char   s[64];
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);

            strftime(s, sizeof(s), 
                     local_time == 1 ? "%I:%M %p" :    // 12-hour (HH:MM designation)
                     local_time == 2 ? "%I:%M:%S %p" : // 12-hour (HH:MM:SS designation)
                     local_time == 3 ? "%H:%M" :       // 24-hour (HH:MM)
                     local_time == 4 ? "%H:%M:%S" :    // 24-hour (HH:MM:SS)
                                       "", tm);        // No time

            RD_M_DrawTextC(s, (local_time == 1 ? 281 :
                               local_time == 2 ? 269 :
                               local_time == 3 ? 293 :
                               local_time == 4 ? 281 : 0)
                              + (wide_4_3 ? wide_delta : wide_delta * 2), 13);
        }

        if (show_fps)
        {
            char digit[9999];
            
            sprintf (digit, "%d", real_fps);
            RD_M_DrawTextC("FPS:", 279 + (wide_4_3 ? wide_delta : wide_delta * 2), 23);
            RD_M_DrawTextC(digit, 297 + (wide_4_3 ? wide_delta : wide_delta * 2), 23);   // [JN] fps digits

            // [JN] Draw extra counters, only while playing in game level.
            if (show_fps == 2 && gamestate == GS_LEVEL)
            {
                sprintf (digit, "%9d", rendered_segs);
                RD_M_DrawTextC("SEGS", 297 + (wide_4_3 ? wide_delta : wide_delta*2), 32);
                RD_M_DrawTextC(digit, 277 + (wide_4_3 ? wide_delta : wide_delta*2), 39);

                sprintf (digit, "%9d", rendered_visplanes);
                RD_M_DrawTextC("VISPLANES", 277 + (wide_4_3 ? wide_delta : wide_delta*2), 49);
                RD_M_DrawTextC(digit, 277 + (wide_4_3 ? wide_delta : wide_delta*2), 56);

                sprintf (digit, "%9d", rendered_vissprites);
                RD_M_DrawTextC("SPRITES", 285 + (wide_4_3 ? wide_delta : wide_delta*2), 66);
                RD_M_DrawTextC(digit, 277 + (wide_4_3 ? wide_delta : wide_delta*2), 73);
            }
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC D_Display
//
// Draw current display, possibly wiping it from the previous.
//
//---------------------------------------------------------------------------

extern boolean finalestage;

void D_Display(void)
{
    static int saved_gametic = -1;
    extern boolean askforquit;

    realframe = (!uncapped_fps || gametic > saved_gametic);

    if (realframe)
    {
        saved_gametic = gametic;
    }

    // [JN] Set correct palette. Allow finale stages use own palettes.
    if (gamestate != GS_LEVEL && gamestate != GS_FINALE)
    {
        I_SetPalette(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    }

    // Change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
    }

//
// do buffered drawing
//
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
                break;
            if (automapactive)
            {
                // [crispy] update automap while playing
                R_RenderPlayerView(&players[displayplayer]);
                AM_Drawer();
            }
            else
                R_RenderPlayerView(&players[displayplayer]);
            CT_Drawer();
            UpdateState |= I_FULLVIEW;
            SB_Drawer();
            break;
        case GS_INTERMISSION:
            IN_Drawer();
            break;
        case GS_FINALE:
            F_Drawer();
            break;
        case GS_DEMOSCREEN:
            D_PageDrawer();
            break;
    }

    if (testcontrols)
    {
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    if (paused && !menuactive && !askforquit)
    {
        if (!netgame)
        {
            V_DrawShadowedPatchRaven(160 + wide_delta,
                                    (viewwindowy >> hires) + 5, W_CacheLumpName
                                    (DEH_String
                                    (english_language ? 
                                     "PAUSED" : "RD_PAUSE"), PU_CACHE));
        }
        else
        {
            V_DrawShadowedPatchRaven(160 + wide_delta,
                                     70, W_CacheLumpName
                                    (DEH_String(english_language ?
                                     "PAUSED" : "RD_PAUSE"), PU_CACHE));
        }
    }
    // Handle player messages
    DrawMessage();

    // Menu drawing
    MN_Drawer();

    // [JN] Draw local time and FPS widgets on top of everything.
    DrawTimeAndFPS();

    // [JN] Performance counters were drawn, reset them.
    R_ClearStats();

    // Send out any new accumulation
    NetUpdate();

    // Flush buffered stuff to screen
    I_FinishUpdate();
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse

    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}

//---------------------------------------------------------------------------
//
// PROC D_DoomLoop
//
//---------------------------------------------------------------------------

void D_DoomLoop(void)
{
    if (M_CheckParm("-debugfile"))
    {
        char filename[20];
        M_snprintf(filename, sizeof(filename), "debug%i.txt", consoleplayer);
        debugfile = fopen(filename, "w");
    }
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_InitGraphics();

    main_loop_started = true;

    while (1)
    {
        // Process one or more tics
        // Will run at least one tic
        TryRunTics();

        // [JN] Mute and restore sound and music volume.
        if (mute_inactive_window && volume_needs_update)
        {
            if (!window_focused)
            {
                S_MuteSound();
            }
            else
            {
                S_UnMuteSound();
            }
        }

        // Move positional sounds
        S_UpdateSounds(players[consoleplayer].mo);

        // Update display, next frame, with current state.
        if (screenvisible)
        D_Display();
    }
}

/*
===============================================================================

						DEMO LOOP

===============================================================================
*/

int demosequence;
int pagetic;
char *pagename;


/*
================
=
= D_PageTicker
=
= Handles timing for warped projection
=
================
*/

void D_PageTicker(void)
{
    if (--pagetic < 0)
        D_AdvanceDemo();
}


/*
================
=
= D_PageDrawer
=
================
*/

void D_PageDrawer(void)
{
    boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);
    const patch_t *page =  W_CacheLumpName(pagename, PU_CACHE);

    if (aspect_ratio >= 2)
    {
        // [JN] Clean up remainings of the wide screen before
        // drawing any new RAW screen.
        V_DrawFilledBox(0, 0, screenwidth, SCREENHEIGHT, 0);
    }

    if (page->width == 560)
    {
        V_DrawPatchFullScreen(W_CacheLumpName(pagename, PU_CACHE), false);
    }
    else
    {
        V_DrawRawScreen(W_CacheLumpName(pagename, PU_CACHE));
    }

    if (demosequence == 1)
    {
        V_DrawShadowedPatchRaven(4 + (wide_4_3 ? wide_delta : 0), 160,
                                 W_CacheLumpName(DEH_String(english_language ?
                                 "ADVISOR" : "ADVIS_RU"), PU_CACHE));
    }
    UpdateState |= I_FULLSCRN;
}

/*
=================
=
= D_AdvanceDemo
=
= Called after each demo or intro demosequence finishes
=================
*/

void D_AdvanceDemo(void)
{
    advancedemo = true;
}

void D_DoAdvanceDemo(void)
{
    S_ResumeSound();    // [JN] Fix vanilla Heretic bug: resume music playing
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save / end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
    {
        case 0:
            pagetic = 210;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            pagename = DEH_String(gamemode == retail ? "TITLE" : "TITLEOLD");
            else
            pagename = DEH_String(gamemode == retail ? "TITLE_RT" : "TITLEOLD");
            S_StartSong(mus_titl, false, false);
            break;
        case 1:
            pagetic = 140;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            pagename = DEH_String(gamemode == retail ? "TITLE" : "TITLEOLD");
            else
            pagename = DEH_String(gamemode == retail ? "TITLE_RT" : "TITLEOLD");
            break;
        case 2:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo(DEH_String("demo1"));
            break;
        case 3:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (english_language)
            pagename = DEH_String("CREDIT");
            else
            pagename = DEH_String(gamemode == retail ? "CRED_RT" : "CRED_RG");
            break;
        case 4:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo(DEH_String("demo2"));
            break;
        case 5:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (gamemode == shareware)
            {
                if (english_language)
                pagename = DEH_String("ORDER");
                else
                pagename = DEH_String("ORDER_R");
            }
            else
            {
                if (english_language)
                pagename = DEH_String("CREDIT");
                else
                pagename = DEH_String(gamemode == retail ? "CRED_RT" : "CRED_RG");
            }
            break;
        case 6:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            if (!no_internal_demos)
            G_DeferedPlayDemo(DEH_String("demo3"));
            break;
    }
}


/*
=================
=
= D_StartTitle
=
=================
*/

void D_StartTitle(void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo();
}


/*
==============
=
= D_CheckRecordFrom
=
= -recordfrom <savegame num> <demoname>
==============
*/

void D_CheckRecordFrom(void)
{
    int p;
    char *filename;

    //!
    // @vanilla
    // @category demo
    // @arg <savenum> <demofile>
    //
    // Record a demo, loading from the given filename. Equivalent
    // to -loadgame <savenum> -record <demofile>.

    p = M_CheckParmWithArgs("-recordfrom", 2);
    if (!p)
    {
        return;
    }

    filename = SV_Filename(myargv[p + 1][0] - '0');
    G_LoadGame(filename);
    G_DoLoadGame();             // load the gameskill etc info from savegame

    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);
    D_DoomLoop();               // never returns
    free(filename);
}

/*
===============
=
= D_AddFile
=
===============
*/

// MAPDIR should be defined as the directory that holds development maps
// for the -wart # # command

#define MAPDIR "\\data\\"

#define SHAREWAREWADNAME "heretic1.wad"

char *iwadfile;

char *basedefault = "heretic.ini";

void wadprintf(void)
{
    if (debugmode)
    {
        return;
    }
    // haleyjd FIXME: convert to textscreen code?
#ifdef __WATCOMC__
    _settextposition(23, 2);
    _setbkcolor(1);
    _settextcolor(0);
    _outtext(exrnwads);
    _settextposition(24, 2);
    _outtext(exrnwads2);
#endif
}

boolean D_AddFile(char *file)
{
    wad_file_t *handle;

    printf(english_language ?
           " adding: %s\n" :
           " добавление: %s\n",
           file);

    handle = W_AddFile(file);

    return handle != NULL;
}

// haleyjd: moved up, removed WATCOMC code
void CleanExit(void)
{
    DEH_printf(english_language ?
    "Exited from HERETIC.\n" :
    "Выполнен выход из HERETIC.\n");
    exit(1);
}

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
    int i;

    BK_AddBindingsToSystemKeys();

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindControllerVariables();
    I_BindSoundVariables();

#ifdef FEATURE_MULTIPLAYER
    NET_BindVariables();
#endif

    // [JN] Support for fallback to the English language.
    M_BindIntVariable("english_language",       &english_language);

    // [JN] PWAD autoloading. Note that we are using variables 1..4, not 0...3.
    for (i = 1 ; i < 5 ; ++i)
    {
        static char pwad[32];

        M_snprintf(pwad, sizeof(pwad), "autoload_global_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadglobalpwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_registered_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadregisteredpwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_retail_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadretailpwad[i]);
    }

    // Rendering
    M_BindIntVariable("uncapped_fps",           &uncapped_fps);
    M_BindIntVariable("show_endoom",            &show_endoom);
    M_BindIntVariable("flashing_hom",           &flashing_hom);

    // Display
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("level_brightness",       &level_brightness);
    M_BindIntVariable("detaillevel",            &detailLevel);

    // Messages and Texts
    M_BindIntVariable("show_messages",          &show_messages);
    M_BindIntVariable("messages_alignment",     &messages_alignment);
    M_BindIntVariable("messages_timeout",       &messages_timeout);
    M_BindIntVariable("message_fade",           &message_fade);
    M_BindIntVariable("draw_shadowed_text",     &draw_shadowed_text);
    M_BindIntVariable("local_time",             &local_time);
    M_BindIntVariable("message_pickup_color",   &message_pickup_color);
    M_BindIntVariable("message_secret_color",   &message_secret_color);
    M_BindIntVariable("message_system_color",   &message_system_color);
    M_BindIntVariable("message_chat_color",     &message_chat_color);

    // Automap
    M_BindIntVariable("automap_overlay",        &automap_overlay);
    M_BindIntVariable("automap_rotate",         &automap_rotate);
    M_BindIntVariable("automap_follow",         &automap_follow);
    M_BindIntVariable("automap_grid",           &automap_grid);
    M_BindIntVariable("automap_grid_size",      &automap_grid_size);
    M_BindIntVariable("automap_mark_color",     &automap_mark_color);
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("automap_level_time",     &automap_level_time);
    M_BindIntVariable("automap_total_time",     &automap_total_time);
    M_BindIntVariable("automap_coords",         &automap_coords);

    // Sound
    M_BindIntVariable("sfx_volume",             &snd_MaxVolume);
    M_BindIntVariable("music_volume",           &snd_MusicVolume);
    M_BindIntVariable("snd_monomode",           &snd_monomode);
    M_BindIntVariable("snd_channels",           &snd_Channels);

    // Controls
    M_BindIntVariable("always_run",             &alwaysRun);
    M_BindIntVariable("mlook",                  &mlook);
    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);

    // Gameplay: Graphical
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("sbar_colored",           &sbar_colored);
    M_BindIntVariable("sbar_colored_gem",       &sbar_colored_gem);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("linear_sky",             &linear_sky);
    M_BindIntVariable("flip_weapons",           &flip_weapons);

    // Gameplay: Status bar
    M_BindIntVariable("ammo_widget",            &ammo_widget);
    M_BindIntVariable("ammo_widget_colored",    &ammo_widget_colored);

    // Gameplay: Physical
    M_BindIntVariable("improved_collision",     &improved_collision);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("floating_powerups",      &floating_powerups);

    // Gameplay: Audible
    M_BindIntVariable("z_axis_sfx",             &z_axis_sfx);
    M_BindIntVariable("noise_alert_sfx",        &noise_alert_sfx);

    // Gameplay: Tactical
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("show_all_artifacts",     &show_all_artifacts);
    M_BindIntVariable("show_artifacts_timer",   &show_artifacts_timer);
    M_BindIntVariable("negative_health",        &negative_health);

    // Gameplay: Crosshair
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_type",         &crosshair_type);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);    

    // Gameplay: Gameplay
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);
    M_BindIntVariable("pistol_start",           &pistol_start);

    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }
}

// 
// Called at exit to display the ENDOOM screen (ENDTEXT in Heretic)
//

static void D_Endoom(void)
{
    byte *endoom_data;

    // Disable ENDOOM?

    if (!show_endoom || testcontrols || !main_loop_started)
    {
        return;
    }

    if (english_language)
    {
        endoom_data = W_CacheLumpName(DEH_String("ENDTEXT"), PU_STATIC);
    }
    else
    {
        endoom_data = W_CacheLumpName(DEH_String(gamemode == shareware  ? "ENDTXTSW" :
                                                 gamemode == registered ? "ENDTXTRG" :
                                                                          "ENDTXTRT"),
                                                                           PU_STATIC);
    }

    I_Endoom(endoom_data);
}

//---------------------------------------------------------------------------
//
// PROC D_DoomMain
//
//---------------------------------------------------------------------------

void D_DoomMain(void)
{
    GameMission_t gamemission;
    int p;
    char file[256];
    char demolumpname[9];
    int newpwadfile;
    char* internalWadName;

#ifdef _WIN32
    // [JN] Get system preffed language...
    DWORD rd_lang_id = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));
    // ..if game language is not set yet (-1), and OS preffered language
    // is appropriate for using Russian language in the game, use it.
    if (english_language == -1)
    {
        if (rd_lang_id != LANG_RUSSIAN
        &&  rd_lang_id != LANG_UKRAINIAN
        &&  rd_lang_id != LANG_BELARUSIAN)
        english_language = 1;
        else
        english_language = 0;
    }

    // [JN] Print colorized title
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_GREEN
                                                           | FOREGROUND_RED
                                                           | FOREGROUND_GREEN
                                                           | FOREGROUND_BLUE
                                                           | FOREGROUND_INTENSITY);
    for (p = 0 ; p < 32 ; p++) DEH_printf(" ");
    DEH_printf(RD_Project_String);
    for (p = 0 ; p < 31 ; p++) DEH_printf(" ");
    DEH_printf("\n");

    // [JN] Fallback to standard console colos
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED
                                                           | FOREGROUND_GREEN
                                                           | FOREGROUND_BLUE);
#else
    // [JN] Just print an uncolored banner
    for (p = 0 ; p < 32 ; p++) DEH_printf(" ");
    DEH_printf(RD_Project_String);
    for (p = 0 ; p < 31 ; p++) DEH_printf(" ");
    DEH_printf("\n");
#endif 

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);

    I_AtExit(D_Endoom, false);

    //!
    // @vanilla
    //
    // Disable monsters.
    //

    nomonsters = M_ParmExists("-nomonsters");

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_ParmExists("-respawn");

    //!
    // @vanilla
    //
    // Take screenshots when F1 is pressed.
    //

    ravpic = M_ParmExists("-ravpic");

    //!
    // @vanilla
    //
    // Allow artifacts to be used when the run key is held down.
    //

    noartiskip = M_ParmExists("-noartiskip");

    debugmode = M_ParmExists("-debug");
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

//
// get skill / episode / map from parms
//

    //!
    // @vanilla
    // @category net
    //
    // Start a deathmatch game.
    //

    if (M_ParmExists("-deathmatch"))
    {
        deathmatch = true;
    }

    //!
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //

    p = M_CheckParmWithArgs("-skill", 1);
    if (p)
    {
        startskill = myargv[p + 1][0] - '1';
        autostart = true;
    }

    //!
    // @arg <n>
    // @vanilla
    //
    // Start playing on episode n (1-4)
    //

    p = M_CheckParmWithArgs("-episode", 1);
    if (p)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = 1;
        autostart = true;
    }

    //!
    // @arg <x> <y>
    // @vanilla
    //
    // Start a game immediately, warping to level ExMy.
    //

    p = M_CheckParmWithArgs("-warp", 2);
    if (p && p < myargc - 2)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = myargv[p + 2][0] - '0';
        autostart = true;
    }

    // Check for -CDROM

    cdrom = false;

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\heretic.cd,
    // allowing play from CD.
    //

    if (M_CheckParm("-cdrom"))
    {
        cdrom = true;
    }
#endif

    if (cdrom)
    {
        M_SetConfigDir(DEH_String("c:\\heretic.cd"));
    }
    else
    {
        M_SetConfigDir(NULL);
    }

    // Load defaults before initing other systems
    DEH_printf(english_language ?
               "M_LoadDefaults: Load system defaults.\n" :
               "M_LoadDefaults: Загрузка системных стандартов.\n");
    D_BindVariables();
    M_SetConfigFilename(PROGRAM_PREFIX "heretic.ini");
    M_LoadConfig();

    //
    // init subsystems
    //
    DEH_printf(english_language ?
               "V_Init: allocate screens.\n" :
               "V_Init: Инициализация видео.\n");
    V_Init();

    I_AtExit(M_SaveConfig, false);

    DEH_printf(english_language ?
               "Z_Init: Init zone memory allocation daemon.\n" :
               "Z_Init: Инициализация распределения памяти.\n");
    Z_Init();

    DEH_printf(english_language ?
               "W_Init: Init WADfiles.\n" :
               "W_Init: Инициализация WAD-файлов.\n");

    iwadfile = D_FindIWAD(IWAD_MASK_HERETIC, &gamemission);

    if (iwadfile == NULL)
    {
        if (english_language)
        {
            I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                    "one with the '-iwad' command line parameter.");
        }
        else
        {
            I_Error("Невозможно определить игру из за отсутствующего IWAD-файла.\n"
                    "Попробуйте указать IWAD-файл командой '-iwad'.\n");
        }
    }

    D_AddFile(iwadfile);
    W_CheckCorrectIWAD(heretic);

#ifdef FEATURE_DEHACKED
    // Load dehacked patches specified on the command line.
    DEH_ParseCommandLine();
#endif

    // Load PWAD files.
    W_ParseCommandLine();

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (!p)
    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //

        p = M_CheckParmWithArgs("-timedemo", 1);
    }

    if (p)
    {
        char *uc_filename = strdup(myargv[p + 1]);
        M_ForceUppercase(uc_filename);

        // In Vanilla, the filename must be specified without .lmp,
        // but make that optional.
        if (M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p + 1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name,
                         sizeof(demolumpname));
        }
        else
        {
            // The file failed to load, but copy the original arg as a
            // demo name to make tricks like -playdemo demo1 possible.
            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf(english_language ?
               "Playing demo %s.\n" :
               "Проигрывание демозаписи %s.\n",
               file);
    }

    // [JN] Addition: also generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    //!
    // @category demo
    //
    // Record or playback a demo without automatically quitting
    // after either level exit or player respawn.
    //

    demoextend = M_ParmExists("-demoextend");

    internalWadName = RD_M_FindInternalResource("heretic-common.wad");
    W_MergeFile(internalWadName);
    free(internalWadName);

    if (W_CheckNumForName(DEH_String("E2M1")) == -1)
    {
        gamemode = shareware;
        gamedescription = english_language ?
                          "Heretic (shareware)" :
                          "Heretic (демоверсия)";
    }
    else if (W_CheckNumForName("EXTENDED") != -1)
    {
        // Presence of the EXTENDED lump indicates the retail version
        gamemode = retail;
        gamedescription = english_language ? 
                          "Heretic: Shadow of the Serpent Riders" :
                          "Heretic: Тень Змеиных Всадников";
    }
    else
    {
        gamemode = registered;
        gamedescription = "Heretic";
    }

    // [JN] PWAD autoloading routine. Scan through all 3 
    // available variables, and don't load an empty ones. 
    // Note: you cannot use autoload with the Shareware, buy a full version!
    if (gamemode != shareware)
    {
        int i;

        for (i = 1 ; i < 5 ; ++i)
        {
            // [JN] If autoloads have not been set, initialize with defaults.
            if (autoloadglobalpwad[i] == NULL)
                autoloadglobalpwad[i] = "";
            if (autoloadregisteredpwad[i] == NULL)
                autoloadregisteredpwad[i] = "";
            if (autoloadretailpwad[i] == NULL)
                autoloadretailpwad[i] = "";

            if (strcmp(autoloadglobalpwad[i], ""))
            {
                W_MergeFile(autoloadglobalpwad[i]);
                printf(english_language ? 
                      " autoloading: %s\n" : " автозагрузка: %s\n",
                        autoloadglobalpwad[i]);
            }

            if (gamemode == registered)
            {
                if (strcmp(autoloadregisteredpwad[i], ""))
                {
                    W_MergeFile(autoloadregisteredpwad[i]);
                    printf(english_language ?
                           " autoloading: %s\n" : " автозагрузка: %s\n",
                           autoloadregisteredpwad[i]);
                }
            }
            else if (gamemode == retail)
            {
                if (strcmp(autoloadretailpwad[i], ""))
                {
                    W_MergeFile(autoloadretailpwad[i]);
                    printf(english_language ?
                           " autoloading: %s\n" : " автозагрузка: %s\n",
                           autoloadretailpwad[i]);
                }
            }
        }
    }

    // [JN] Параметр "-file" перенесен из w_main.c
    // Необходимо для того, чтобы любые ресурсы из pwad-файлов
    // загружались после руссифицированных pwad-файлов.

    newpwadfile = M_CheckParmWithArgs ("-file", 1);
    if (newpwadfile)
    {
        while (++newpwadfile != myargc && myargv[newpwadfile][0] != '-')
        {
            char *filename;
            filename = D_TryFindWADByName(myargv[newpwadfile]);
            printf(english_language ?
                   " adding: %s\n" :
                   " добавление: %s\n",
                   filename);
            W_MergeFile(filename);
        }
    }

    I_SetWindowTitle(gamedescription);

    savegamedir = M_GetSaveGameDir();

    if (M_ParmExists("-testcontrols"))
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

    I_InitTimer();
    I_InitSound(false);

#ifdef FEATURE_MULTIPLAYER
    DEH_printf(english_language ?
            "NET_Init: Init network subsystem.\n" :
            "NET_Init: Инициализация сетевой подсистемы.\n", 1);
    NET_Init ();
#endif

    D_ConnectNetGame();

    if (autostart)
    {
        char temp[64];
        DEH_snprintf(temp, sizeof(temp), english_language ?
                     "Warp to Episode %d, Map %d, Skill %d " :
                     "Перемещение в эпизод %d, уровень %d, сложность %d ",
                     startepisode, startmap, startskill + 1);
    }
    wadprintf();                // print the added wadfiles

    DEH_printf(english_language ?
               "MN_Init: Init menu system.\n" :
               "MN_Init: Инициализация игрового меню.\n");
    MN_Init();

    CT_Init();

    DEH_printf(english_language ?
               "R_Init: Init Heretic refresh daemon." :
               "R_Init: Инициализация процесса запуска Heretic.");
    R_Init();
    DEH_printf("\n");

    DEH_printf(english_language ?
               "P_Init: Init Playloop state.\n" :
               "P_Init: Инициализация игрового окружения.\n");
    P_Init();

    DEH_printf(english_language ? 
               "I_Init: Setting up machine state.\n" :
               "I_Init: Инициализация состояния компьютера.\n");
    I_CheckIsScreensaver();
    I_InitController();

    DEH_printf(english_language ?
               "S_Init: Setting up sound.\n" :
               "S_Init: Активация звуковой системы.\n");
    S_Init();
    //IO_StartupTimer();
    S_Start();

    DEH_printf(english_language ?
               "D_CheckNetGame: Checking network game status.\n" :
               "D_CheckNetGame: Проверка статуса сетевой игры.\n");
    D_CheckNetGame();

    // haleyjd: removed WATCOMC

    DEH_printf(english_language ?
               "SB_Init: Loading patches.\n" :
               "SB_Init: Загрузка патчей.\n");
    SB_Init();

    // [JN] Predifine automap marks color.
    AM_initMarksColor(automap_mark_color);

//
// start the apropriate game based on parms
//

    D_CheckRecordFrom();

    //!
    // @arg <x>
    // @category demo
    // @vanilla
    //
    // Record a demo named x.lmp.
    //

    p = M_CheckParmWithArgs("-record", 1);
    if (p)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(demolumpname);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo(demolumpname);
        D_DoomLoop();           // Never returns
    }

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in savegame slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    if (p && p < myargc - 1)
    {
        char *filename;

	filename = SV_Filename(myargv[p + 1][0] - '0');
        G_LoadGame(filename);
	free(filename);
    }

    // Check valid episode and map
    if (autostart || netgame)
    {
        if (!D_ValidEpisodeMap(heretic, gamemode, startepisode, startmap))
        {
            startepisode = 1;
            startmap = 1;
        }
    }

    if (gameaction != ga_loadgame)
    {
        UpdateState |= I_FULLSCRN;
        BorderNeedRefresh = true;
        if (autostart || netgame)
        {
            G_InitNew(startskill, startepisode, startmap, 0);
        }
        else
        {
            D_StartTitle();
        }
    }

    // [JN] Show the game we are playing
    DEH_printf(english_language ? "Starting game: " : "Запуск игры: ");
    DEH_printf("\"");
    DEH_printf(gamedescription);
    DEH_printf("\".");
    DEH_printf("\n");

    // [JN] Define and load translated strings
    RD_DefineLanguageStrings();

    D_DoomLoop();               // Never returns
}

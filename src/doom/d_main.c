//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // [JN] srand(time(0))

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "rd_lang.h"
#include "doomfeatures.h"
#include "sounds.h"
#include "d_iwad.h"
#include "z_zone.h"
#include "w_main.h"
#include "w_merge.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_menu.h"
#include "p_local.h"
#include "p_saveg.h"
#include "i_endoom.h"
#include "i_controller.h"
#include "i_input.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "net_client.h"
#include "net_dedicated.h"
#include "net_query.h"
#include "rd_keybinds.h"
#include "rd_text.h"
#include "r_local.h"
#include "d_main.h"
#include "d_name.h"
#include "am_map.h"         // [JN] AM_initColors();
#include "jn.h"

// -----------------------------------------------------------------------------
// [Dasperal] d_name.h var definition
// -----------------------------------------------------------------------------
char* RD_Project_Name = PACKAGE_PREFIX " Doom";
char* RD_Project_String = PACKAGE_PREFIX " Doom " BUILD_DOOM_VERSION;
GameType_t RD_GameType = gt_Doom;

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

// -----------------------------------------------------------------------------
// [JN] PWAD autoloading. Initially all 4 values are empty.
// -----------------------------------------------------------------------------

char *autoloadglobalpwad[10]    = { "", "", "", "" };
char *autoloaddoom1pwad[10]     = { "", "", "", "" };
char *autoloaddoom2pwad[10]     = { "", "", "", "" };
char *autoloadplutoniapwad[10]  = { "", "", "", "" };
char *autoloadtntpwad[10]       = { "", "", "", "" };

// -----------------------------------------------------------------------------
// [JN] Default values
// -----------------------------------------------------------------------------

// Rendering
int screen_wiping = 1;
int show_endoom = 0;
int flashing_hom = 0;

// Display
int screenblocks = 10, screenSize;
int level_brightness = 0;
int menu_shading = 0;
int detailLevel = 0;        // Blocky mode, has default, 0 = high, 1 = normal
int hud_detaillevel = 0;    // [JN] Also blocky mode, 0 = high, 1 = normal

// Display: Messages
int showMessages = 1;
int messages_alignment = 0;
int messages_timeout = 4;
int message_fade = 0;
int draw_shadowed_text = 1;
int local_time = 0;
int message_pickup_color = 0;
int message_secret_color = 7;
int message_system_color = 0;
int message_chat_color = 2;

// Display: Automap
int automap_color = 0;
int automap_mark_color = 10;
int automap_antialias = 1;
int automap_stats = 1;
int automap_level_time = 1;
int automap_total_time = 0;
int automap_coords = 0;
int automap_follow  = 1;
int automap_overlay = 0;
int automap_rotate  = 0;
int automap_grid    = 0;
int hud_widget_colors = 0;

// Sound
int sfxVolume = 8;          // Maximum volume of a sound effect (internal: 0-15)
int musicVolume = 8;        // Maximum volume of music.
int snd_channels = 32;
int snd_monomode = 0;

// Controls
extern int alwaysRun;
int mouseSensitivity = 5;

// Selective game
int selective_skill = 2;
int selective_episode = 1;
int selective_map = 1;

int selective_health = 100;
int selective_armor = 0;
int selective_armortype = 1;

int selective_wp_chainsaw = 0;
int selective_wp_shotgun = 0;
int selective_wp_supershotgun = 0;
int selective_wp_chaingun = 0;
int selective_wp_missile = 0;
int selective_wp_plasma = 0;
int selective_wp_bfg = 0;

int selective_backpack = 0;

int selective_ammo_0 = 50;  // bullets
int selective_ammo_1 = 0;   // shells
int selective_ammo_2 = 0;   // cells
int selective_ammo_3 = 0;   // rockets

int selective_key_0 = 0;    // blue keycard
int selective_key_1 = 0;    // yellow keycard
int selective_key_2 = 0;    // red keycard
int selective_key_3 = 0;    // blue skull key
int selective_key_4 = 0;    // yellow skull key
int selective_key_5 = 0;    // red skull key

int selective_fast = 0;
int selective_respawn = 0;

// Gameplay: Graphical
int brightmaps = 1;
int fake_contrast = 0;
int translucency = 1;
int improved_fuzz = 2;
int colored_blood = 1;
int swirling_liquids = 1;
int invul_sky = 1;
int linear_sky = 1;
int randomly_flipcorpses = 1;
int flip_weapons = 0;

// Gameplay: Status Bar
int extra_player_faces = 1;
int negative_health = 0;
int sbar_colored = 0;
int sbar_color_high = 5;
int sbar_color_normal = 2;
int sbar_color_low = 7;
int sbar_color_critical = 0;
int sbar_color_armor_1 = 2;
int sbar_color_armor_2 = 5;
int sbar_color_armor_0 = 0;

// Gameplay: Audible
int z_axis_sfx = 0;
int play_exit_sfx = 0;
int crushed_corpses_sfx = 1;
int blazing_door_fix_sfx = 1;
int noise_alert_sfx = 0;
int correct_endlevel_sfx = 0;

// Gameplay: Tactical
int secret_notification = 1;
int infragreen_visor = 0;
int horizontal_autoaim = 3;

// Gameplay: Physical
int improved_collision = 1;
int over_under = 0;
int torque = 1;
int weapon_bobbing = 1;
int ssg_blast_enemies = 1;
int floating_powerups = 1;
int toss_drop = 1;

// Gameplay: Crosshair
int crosshair_draw = 0;
int crosshair_type = 1;
int crosshair_scale = 0;

// Gameplay: Gameplay
int fix_map_errors = 1;
int unlimited_lost_souls = 1;
int agressive_lost_souls = 0;
int pistol_start = 0;
int fast_quickload = 1;
int flip_levels = 0;

// Gameplay: Demos
int demotimer = 0;
int demotimerdir = 0;
int demobar = 0;
int no_internal_demos = 0;


void D_ConnectNetGame(void);
void D_CheckNetGame(void);


// [JN] Сделана глобальной, нужна для функции автоподргузки 
// блоков DEHACKED, а также в цикле D_DoomMain.
int numiwadlumps; 


//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);

// Location where savegames are stored
char* savegamedir;

// location of IWAD and WAD files
char* iwadfile;

// [JN] Loaded SIGIL PWAD
boolean sgl_loaded;
boolean sgl_compat_loaded;
// [JN] Loaded Master-Level PWAD
boolean mlvls_loaded;
// [JN] MAP33 "Betray" available?
boolean havemap33;

// [JN] Devparm now global and available for all three games (see i_main.c)
// boolean devparm;     // started game with -devparm
boolean nomonsters;  // checkparm of -nomonsters
boolean respawnparm; // checkparm of -respawn
boolean fastparm;    // checkparm of -fast

boolean scaled_sky = false; // [JN] Boolean for sky scaling
boolean old_godface; // [JN] Boolean for extra faces while in GOD mode
boolean pfub2_replaced = false; // [JN] Check if we have a replaced PFUB2 gfx
boolean realframe, skippsprinterp; // [JN] Interpolation for weapon bobbing


skill_t startskill;
boolean autostart;
boolean advancedemo;
int startepisode;
int startmap;
int startloadgame;


// If true, the main game loop has started.
boolean main_loop_started = false;

char wadfile[1024];  // primary wad file
char mapdir[1024];   // directory of development maps

boolean hasDelayEvents = false;

//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//

void D_ProcessEvents(void)
{
    event_t* ev;

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

        if(!M_Responder(ev))
            G_Responder(ev);
    }
}

// -----------------------------------------------------------------------------
// 
// [JN] DrawTimeAndFPS
// Draws time and FPS widgets separatelly from HUD system.
//
// -----------------------------------------------------------------------------

static void DrawTimeAndFPS (void)
{
    if (!vanillaparm)
    {
        const boolean wide_4_3 = (aspect_ratio >= 2 && screenblocks == 9);

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
    
            RD_M_DrawTextC(s, (local_time == 1 ? 282 :
                              local_time == 2 ? 270 :
                              local_time == 3 ? 294 :
                              local_time == 4 ? 282 : 0)
                              + (wide_4_3 ? wide_delta : wide_delta*2), 10);
        }

        if (show_fps)
        {
            char digit[9999];

            sprintf (digit, "%d", real_fps);
            RD_M_DrawTextC("FPS:", 278 + (wide_4_3 ? wide_delta : wide_delta*2), 20);
            RD_M_DrawTextC(digit, 298 + (wide_4_3 ? wide_delta : wide_delta*2), 20);   // [JN] fps digits

            // [JN] Draw extra counters, only while playing in game level.
            if (show_fps == 2 && gamestate == GS_LEVEL)
            {
                sprintf (digit, "%9d", rendered_segs);
                RD_M_DrawTextC("SEGS", 298 + (wide_4_3 ? wide_delta : wide_delta*2), 32);
                RD_M_DrawTextC(digit, 278 + (wide_4_3 ? wide_delta : wide_delta*2), 39);

                sprintf (digit, "%9d", rendered_visplanes);
                RD_M_DrawTextC("VISPLANES", 278 + (wide_4_3 ? wide_delta : wide_delta*2), 49);
                RD_M_DrawTextC(digit, 278 + (wide_4_3 ? wide_delta : wide_delta*2), 56);

                sprintf (digit, "%9d", rendered_vissprites);
                RD_M_DrawTextC("SPRITES", 286 + (wide_4_3 ? wide_delta : wide_delta*2), 66);
                RD_M_DrawTextC(digit, 278 + (wide_4_3 ? wide_delta : wide_delta*2), 73);
            }
        }
    }
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;

extern boolean setsizeneeded;


void D_Display (void)
{
    static boolean      viewactivestate = false;
    static boolean      menuactivestate = false;
    static boolean      inhelpscreensstate = false;
    static boolean      fullscreen = false;
    static gamestate_t  oldgamestate = -1;
    static int          borderdrawcount;
    static int          saved_gametic = -1;
    int                 nowtime;
    int                 tics;
    int                 wipestart;
    int                 y;
    boolean             done;
    boolean             wipe;
    boolean             redrawsbar;

    realframe = (!uncapped_fps || gametic > saved_gametic);

    if (realframe)
    {
        saved_gametic = gametic;
    }

    if (nodrawers)
    return; // for comparative timing / profiling

    redrawsbar = false;

    // change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize ();
        oldgamestate    = -1; // force background redraw
        borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    // [JN] Wiping: make optional
    if (gamestate != wipegamestate && (screen_wiping || vanillaparm))
    {
        wipe = true;
        wipe_StartScreen(0, 0, screenwidth, SCREENHEIGHT);
    }
    else
    {
        wipe = false;
    }

    if (gamestate == GS_LEVEL && gametic)
    HU_Erase();

    // do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
        if (!gametic)
        break;

        if (automapactive && !automap_overlay)
        {
            // [crispy] update automap while playing
            R_RenderPlayerView (&players[displayplayer]);
            AM_Drawer ();
        }

        if (wipe || (scaledviewheight != SCREENHEIGHT && fullscreen))
        redrawsbar = true;

        if (inhelpscreensstate && !inhelpscreens)
        redrawsbar = true; // just put away the help screen

        ST_Drawer (scaledviewheight == SCREENHEIGHT, redrawsbar );
        fullscreen = scaledviewheight == SCREENHEIGHT;
        break;

        case GS_INTERMISSION:
        WI_Drawer ();
        break;

        case GS_FINALE:
        F_Drawer ();
        break;

        case GS_DEMOSCREEN:
        D_PageDrawer ();
        break;
    }

    // draw the view directly
    if (gamestate == GS_LEVEL && (!automapactive || automap_overlay) && gametic)
    {
        R_RenderPlayerView (&players[displayplayer]);

        if (aspect_ratio >= 2)
        {
            if (screenblocks > 10 && screenblocks < 17)
            ST_Drawer(0, 0);
        }
        else
        {
            if (screenblocks == 11 || screenblocks == 12 || screenblocks == 13)
            ST_Drawer(0, 0);            
        }
    }

    // [crispy] in automap overlay mode,
    // the HUD is drawn on top of everything else
    if (gamestate == GS_LEVEL && gametic && !(automapactive && automap_overlay))
    HU_Drawer ();

    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
    {
        I_SetPalette (W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    }

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
        viewactivestate = false;    // view was not active
        R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && (!automapactive || automap_overlay) && scaledviewwidth != (320 << hires))
    {
        if (menuactive || menuactivestate || !viewactivestate)
        borderdrawcount = 3;

        if (borderdrawcount)
        {
            R_DrawViewBorder (); // erase old menu stuff
            borderdrawcount--;
        }
    }

    if (testcontrols)
    {
        // Box showing current mouse speed
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;

    // [crispy] in automap overlay mode,
    // draw the automap and HUD on top of everything else
    if (automapactive && automap_overlay)
    {
        AM_Drawer ();
        HU_Drawer ();

        // [crispy] force redraw of border
        viewactivestate = false;
    }

    // [JN] Menu backgound shading. 
    if (menu_shading > 0 && menuactive && !vanillaparm)
    {
        for (y = 0; y < screenwidth * SCREENHEIGHT; y++)
        {
            I_VideoBuffer[y] = colormaps[menu_shading * 256 + I_VideoBuffer[y]];
        }
    
        // [crispy] force redraw of status bar and border
        viewactivestate = false;
        inhelpscreensstate = true;
    }

    // [JN] Draw pause pic. Don't draw while actime game menu and help screens.
    if (paused && !menuactive)
    {
        // [JN] Atari Jaguar: draw PAUSE pic independently, offsets done in the sprite
        if (gamemission == jaguar)
        {
            V_DrawShadowedPatchDoom(wide_delta, gamestate == GS_INTERMISSION ?
                                    -40 : 0, // [JN] Do not obstruct titles on intermission screen
                                    W_CacheLumpName (DEH_String(english_language ?
                                                                "M_PAUSE" : "RD_PAUSE"), PU_CACHE));
        }
        else
        {
            if (automapactive && !automap_overlay)
            y = 4;
            else if (gamestate == GS_INTERMISSION)  // [JN] Do not obstruct titles on intermission screen
            y = 28;
            else
            y = (viewwindowy >> hires)+4;

            V_DrawShadowedPatchDoom((viewwindowx >> hires) + ((scaledviewwidth >> hires) - 68) / 2, 
                                    y, W_CacheLumpName (DEH_String(english_language ?
                                                                   "M_PAUSE" : "RD_PAUSE"), PU_CACHE));
        }
    }

    // menus go directly to the screen
    M_Drawer ();    // menu is drawn even on top of everything

    // [JN] Draw local time and FPS widgets on top of everything, excluding wipes.
    DrawTimeAndFPS();

    // [JN] Performance counters were drawn, reset them.
    R_ClearStats();

    NetUpdate ();   // send out any new accumulation

    // normal update
    if (!wipe)
    {
        I_FinishUpdate ();  // page flip or blit buffer
        return;
    }

    // wipe update
    wipe_EndScreen(0, 0, screenwidth, SCREENHEIGHT);
    wipestart = I_GetTime () - 1;

    do
    {
        do
        {
            nowtime = I_GetTime ();
            tics = nowtime - wipestart;
            I_Sleep(1);
        } while (tics <= 0);

    wipestart = nowtime;
    done = wipe_ScreenWipe(0, 0, screenwidth, SCREENHEIGHT, tics);
    M_Drawer ();        // menu is drawn even on top of wipes
    I_FinishUpdate ();  // page flip or blit buffer
    } while (!done);
}


// [JN] Un-static for position hot switching
void EnableLoadingDisk(void)
{
    V_EnableLoadingDisk((aspect_ratio >= 2 && screenblocks == 9 ? 
                         SCREENWIDTH+wide_delta*2 : screenwidth) -
                         LOADING_DISK_W, SCREENHEIGHT - LOADING_DISK_H);
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

    // [JN] Support English/Russian language hot swapping
    M_BindIntVariable("english_language",       &english_language);

    // [JN] PWAD autoloading. Note that we are using variables 1..4, not 0...3.
    for (i = 1 ; i < 5 ; ++i)
    {
        static char pwad[24];

        M_snprintf(pwad, sizeof(pwad), "autoload_global_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadglobalpwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_doom1_pwad%i", i);
        M_BindStringVariable(pwad, &autoloaddoom1pwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_doom2_pwad%i", i);
        M_BindStringVariable(pwad, &autoloaddoom2pwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_plutonia_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadplutoniapwad[i]);

        M_snprintf(pwad, sizeof(pwad), "autoload_tnt_pwad%i", i);
        M_BindStringVariable(pwad, &autoloadtntpwad[i]);
    }

    // Rendering
    M_BindIntVariable("uncapped_fps",           &uncapped_fps);
    M_BindIntVariable("show_diskicon",          &show_diskicon);
    M_BindIntVariable("screen_wiping",          &screen_wiping);
    M_BindIntVariable("show_endoom",            &show_endoom);
    M_BindIntVariable("flashing_hom",           &flashing_hom);

    // Display
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("level_brightness",       &level_brightness);
    M_BindIntVariable("menu_shading",           &menu_shading);
    M_BindIntVariable("detaillevel",            &detailLevel);
    M_BindIntVariable("hud_detaillevel",        &hud_detaillevel);

    // Messages
    M_BindIntVariable("show_messages",          &showMessages);
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
    M_BindIntVariable("automap_color",          &automap_color);
    M_BindIntVariable("automap_mark_color",     &automap_mark_color);
    M_BindIntVariable("automap_antialias",      &automap_antialias);
    M_BindIntVariable("automap_stats",          &automap_stats);
    M_BindIntVariable("automap_level_time",     &automap_level_time);
    M_BindIntVariable("automap_total_time",     &automap_total_time);
    M_BindIntVariable("automap_coords",         &automap_coords);
    M_BindIntVariable("automap_overlay",        &automap_overlay);
    M_BindIntVariable("automap_rotate",         &automap_rotate);
    M_BindIntVariable("automap_follow",         &automap_follow);
    M_BindIntVariable("automap_grid",           &automap_grid);
    M_BindIntVariable("hud_widget_colors",      &hud_widget_colors);

    // Sound
    M_BindIntVariable("sfx_volume",             &sfxVolume);
    M_BindIntVariable("music_volume",           &musicVolume);
    M_BindIntVariable("snd_channels",           &snd_channels);
    M_BindIntVariable("snd_monomode",           &snd_monomode);

    // Controls
    M_BindIntVariable("always_run",             &alwaysRun);
    M_BindIntVariable("mlook",                  &mlook);
    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);

    // Gameplay: Graphical
    M_BindIntVariable("brightmaps",             &brightmaps);
    M_BindIntVariable("fake_contrast",          &fake_contrast);
    M_BindIntVariable("translucency",           &translucency);
    M_BindIntVariable("improved_fuzz",          &improved_fuzz);
    M_BindIntVariable("colored_blood",          &colored_blood);
    M_BindIntVariable("swirling_liquids",       &swirling_liquids);
    M_BindIntVariable("invul_sky",              &invul_sky);
    M_BindIntVariable("linear_sky",             &linear_sky);
    M_BindIntVariable("randomly_flipcorpses",   &randomly_flipcorpses);
    M_BindIntVariable("flip_weapons",           &flip_weapons);

    // Gameplay: Status bar
    M_BindIntVariable("extra_player_faces",     &extra_player_faces);
    M_BindIntVariable("negative_health",        &negative_health);
    M_BindIntVariable("sbar_colored",           &sbar_colored);
    M_BindIntVariable("sbar_color_high",        &sbar_color_high);
    M_BindIntVariable("sbar_color_normal",      &sbar_color_normal);
    M_BindIntVariable("sbar_color_low",         &sbar_color_low);
    M_BindIntVariable("sbar_color_critical",    &sbar_color_critical);
    M_BindIntVariable("sbar_color_armor_1",     &sbar_color_armor_1);
    M_BindIntVariable("sbar_color_armor_2",     &sbar_color_armor_2);
    M_BindIntVariable("sbar_color_armor_0",     &sbar_color_armor_0);

    // Gameplay: Audible
    M_BindIntVariable("z_axis_sfx",             &z_axis_sfx);
    M_BindIntVariable("play_exit_sfx",          &play_exit_sfx);
    M_BindIntVariable("crushed_corpses_sfx",    &crushed_corpses_sfx);
    M_BindIntVariable("blazing_door_fix_sfx",   &blazing_door_fix_sfx);
    M_BindIntVariable("noise_alert_sfx",        &noise_alert_sfx);
    M_BindIntVariable("correct_endlevel_sfx",   &correct_endlevel_sfx);

    // Gameplay: Tactical
    M_BindIntVariable("secret_notification",    &secret_notification);
    M_BindIntVariable("infragreen_visor",       &infragreen_visor);
    M_BindIntVariable("horizontal_autoaim",     &horizontal_autoaim);

    // Gameplay: Physical
    M_BindIntVariable("improved_collision",     &improved_collision);
    M_BindIntVariable("over_under",             &over_under);
    M_BindIntVariable("torque",                 &torque);
    M_BindIntVariable("weapon_bobbing",         &weapon_bobbing);
    M_BindIntVariable("ssg_blast_enemies",      &ssg_blast_enemies);
    M_BindIntVariable("floating_powerups",      &floating_powerups);
    M_BindIntVariable("toss_drop",              &toss_drop);

    // Gameplay: Crosshair
    M_BindIntVariable("crosshair_draw",         &crosshair_draw);
    M_BindIntVariable("crosshair_type",         &crosshair_type);
    M_BindIntVariable("crosshair_scale",        &crosshair_scale);

    // Gameplay: Gameplay
    M_BindIntVariable("fix_map_errors",         &fix_map_errors);
    M_BindIntVariable("flip_levels",            &flip_levels);
    M_BindIntVariable("unlimited_lost_souls",   &unlimited_lost_souls);
    M_BindIntVariable("agressive_lost_souls",   &agressive_lost_souls);
    M_BindIntVariable("pistol_start",           &pistol_start);
    M_BindIntVariable("fast_quickload",         &fast_quickload);
    M_BindIntVariable("demotimer",              &demotimer);
    M_BindIntVariable("demotimerdir",           &demotimerdir);
    M_BindIntVariable("demobar",                &demobar);
    M_BindIntVariable("no_internal_demos",      &no_internal_demos);

    // Multiplayer chat macros
    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }
}


//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    if (drone)  // Drone players don't need mouse focus
    return false;

    if (menuactive || paused)  // when menu is active or game is paused, release the mouse 
    return false;

    // only grab mouse when playing levels (but not demos)
    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}


//
//  D_DoomLoop
//

void D_DoomLoop (void)
{
    if (gamevariant == bfgedition && (demorecording || (gameaction == ga_playdemo) || netgame))
    {
        if (english_language)
        {
            printf(" WARNING: You are playing using one of the Doom Classic\n"
                   " IWAD files shipped with the Doom 3: BFG Edition. These are\n"
                   " known to be incompatible with the regular IWAD files and\n"
                   " may cause demos and network games to get out of sync.\n"
                   " \n"
                   " No Rest for the Living addon can be started with vanilla\n"
                   " IWAD by following command line parameter:\n"
                   " russian-doom -file nerve.wad");
        }
        else
        {
            printf(" ВНИМАНИЕ: Вы используете IWAD от Doom 3: BFG Edition.\n"
                   " Он не совместим с классическим IWAD в сетевой игре и\n" 
                   " имеет проблемы с синхронизацией демозаписей.\n"
                   " \n"
                   " Дополнение No Rest for the Living может работать и\n"
                   " с классическим IWAD, для запуска необходимо указать:\n"
                   " russian-doom -file nerve.wad");
        }
    }

    if (demorecording)
    G_BeginRecording ();

    // [JN] Allow to use disk icon drawing functions,
    // but it's is not used in Jaguar Doom.
    disk_allowed = gamemission == jaguar ? false : true;

    main_loop_started = true;

    I_SetWindowTitle(english_language ? gamedescription_eng : gamedescription_rus);
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_InitGraphics();
    EnableLoadingDisk();

    TryRunTics();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    if (testcontrols)
    {
        wipegamestate = gamestate;
    }

    while (1)
    {
        // [JN] Don't call empty function
        // frame syncronous IO operations
        // I_StartFrame ();

        // will run at least one tic
        TryRunTics ();

        // Update display, next frame, with current state.
        if (screenvisible)
        D_Display ();

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

        // move positional sounds
        S_UpdateSounds (players[consoleplayer].mo);
    }
}


//
//  DEMO LOOP
//
int     demosequence;
int     pagetic;
char    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//

void D_PageTicker (void)
{
    if (--pagetic < 0)
    D_AdvanceDemo ();
}


//
// D_PageDrawer
//

void D_PageDrawer (void)
{
    V_DrawPatchFullScreen (W_CacheLumpName(pagename, PU_CACHE), false);
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//

void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//

void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;                               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    // The Ultimate Doom executable changed the demo sequence to add
    // a DEMO4 demo.  Final Doom was based on Ultimate, so also
    // includes this change; however, the Final Doom IWADs do not
    // include a DEMO4 lump, so the game bombs out with an error
    // when it reaches this point in the demo sequence.

    // However! There is an alternate version of Final Doom that
    // includes a fixed executable.

    // [JN] Чтобы обычные версии Final DOOM не крашились,
    // поддержка четвёртой демозаписи оставлена только для Ultimate.

    if (gameversion == exe_ultimate /*|| gameversion == exe_final*/)
    demosequence = (demosequence+1)%7;
    else
    demosequence = (demosequence+1)%6;

    switch (demosequence)
    {
        case 0:
        {
            if (gamemode == commercial)
                pagetic = TICRATE * 11;
            else
                pagetic = 170;

            if (gamemission == pack_nerve)
                pagetic = 1000;

            gamestate = GS_DEMOSCREEN;

            if (gamemode == shareware)
            {
                if (english_language)
                pagename = DEH_String("TITLEPIS");
                else
                pagename = DEH_String("TITLEPSR");
            }
            else
            {
                if (english_language)
                {
                    pagename = DEH_String("TITLEPIC");
                }
                else
                {
                    if (logical_gamemission == doom)
                    pagename = DEH_String("TITLEPIR");
                    else if (logical_gamemission == jaguar)
                    pagename = DEH_String("TITLEPIJ");
                    else if (logical_gamemission == doom2)
                    pagename = DEH_String("TITLEPI2");
                    else if (logical_gamemission == pack_plut)
                    pagename = DEH_String("TITLEPIP");
                    else if (logical_gamemission == pack_tnt)
                    pagename = DEH_String("TITLEPIT");
                    else
                    pagename = DEH_String("TITLEPIC");
                }
            }
        
            if (gamemode == commercial)
                S_StartMusic(mus_dm2ttl);
            else
                S_StartMusic (mus_intro);

            break;
        }

        case 1:
        {
            if (gamemode == pressbeta       // [JN] No demos in Press Beta
            || gamemission == pack_nerve    // ... no demos in NERVE
            || no_internal_demos)           // ... no demos if they are disabled
            break;

            G_DeferedPlayDemo(DEH_String("demo1"));

            break;
        }

        case 2:
        {
            if (gamemode == pressbeta)
            break;

            if (gamemission == pack_nerve)
                pagetic = 1000;
            else
                pagetic = 200;

            gamestate = GS_DEMOSCREEN;

            if (english_language)
            {
                if (gamemode == shareware || gamemode == registered)
                pagename = DEH_String("CREDITW");
                else
                pagename = DEH_String("CREDIT");
            }
            else
            {
                // [JN] Different screens in Russian language, packed into one PWAD.
                if (gamemode == shareware || gamemode == registered)
                pagename = DEH_String("CREDITS");
                else if (gamemode == retail)
                pagename = DEH_String("CREDITU");
                else if (logical_gamemission == jaguar)
                pagename = DEH_String("CREDITJ");
                else
                pagename = DEH_String("CREDIT2");

            }
            break;
        }

        case 3:
        {
            if (gamemode == pressbeta       // [JN] No demos in Press Beta
            || gamemission == pack_nerve    // ... no demos in NERVE
            || no_internal_demos)           // ... no demos if they are disabled
            break;

            else
            G_DeferedPlayDemo(DEH_String("demo2"));

            break;
        }

        case 4:
        {
            if (gamemode == pressbeta       // [JN] No demos in Press Beta
            || gamemission == pack_nerve)    // ... no demos in NERVE
            break;

            gamestate = GS_DEMOSCREEN;

            if (gamemode == commercial)
            {
                pagetic = TICRATE * 11;
                if (english_language)
                {
                    pagename = DEH_String("TITLEPIC");
                }
                else
                {
                    if (logical_gamemission == doom)
                    pagename = DEH_String("TITLEPIR");
                    else if (logical_gamemission == jaguar)
                    pagename = DEH_String("TITLEPIJ");
                    else if (logical_gamemission == doom2)
                    pagename = DEH_String("TITLEPI2");
                    else if (logical_gamemission == pack_plut)
                    pagename = DEH_String("TITLEPIP");
                    else if (logical_gamemission == pack_tnt)
                    pagename = DEH_String("TITLEPIT");
                    else
                    pagename = DEH_String("TITLEPIC");
                }

                S_StartMusic(mus_dm2ttl);
            }
            else
            {
                pagetic = 200;

                if (gameversion >= exe_ultimate)
                {
                    if (english_language)
                    {
                        pagename = DEH_String("CREDIT");
                    }
                    else
                    {
                        // [JN] Different screens in Russian language, packed into one PWAD.
                        if (gamemode == shareware || gamemode == registered)
                        pagename = DEH_String("CREDITS");
                        else if (gamemode == retail)
                        pagename = DEH_String("CREDITU");
                        else if (logical_gamemission == jaguar)
                        pagename = DEH_String("CREDITJ");
                        else
                        pagename = DEH_String("CREDIT2");
        
                    }
                }
                else
                {
                    if (english_language)
                    pagename = DEH_String("HELP2");
                    else
                    pagename = DEH_String("HELP2R");
                }
            }

            break;
        }

        case 5:
        {
            if (gamemode == pressbeta       // [JN] No demos in Press Beta
            || gamemission == pack_nerve    // ... no demos in NERVE
            || no_internal_demos)           // ... no demos if they are disabled
            break;

            G_DeferedPlayDemo(DEH_String("demo3"));

            break;
        }

        case 6: // THE DEFINITIVE DOOM Special Edition demo
        {
            if (gamemode == pressbeta       // [JN] No demos in Press Beta
            || gamemission == pack_nerve    // ... no demos in NERVE
            || no_internal_demos)           // ... no demos if they are disabled
            break;

            G_DeferedPlayDemo(DEH_String("demo4"));

            break;
        }

        // The Doom 3: BFG Edition version of doom2.wad does not have a
        // TITLETPIC lump. Use INTERPIC instead as a workaround.
        if (gamevariant == bfgedition && !strcasecmp(pagename, "TITLEPIC") && W_CheckNumForName("titlepic") < 0)
        {
            pagename = DEH_String("INTERPIC");
        }
    }
}


//
// D_StartTitle
//

void D_StartTitle (void)
{
    gameaction = ga_nothing;
    automapactive = false; // [JN] [crispy] clear overlaid automap remainings
    demosequence = -1;
    D_AdvanceDemo ();
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] =
{
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // doom2.wad v1.666
    "                         "
    "DOOM 2: Hell on Earth v%i.%i66"
    "                          ",
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // Doom v1.666
    "                          "
    "DOOM System Startup v%i.%i66"
    "                          "
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};


//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(char *gamename)
{
    size_t  i;
    char    *deh_sub;

    for (i=0; i<arrlen(banners); ++i)
    {
        // Has the banner been replaced?
        deh_sub = DEH_String(banners[i]);

        if (deh_sub != banners[i])
        {
            size_t gamename_size;
            int version;

            // Has been replaced.
            // We need to expand via printf to include the Doom version number
            // We also need to cut off spaces to get the basic name

            gamename_size = strlen(deh_sub) + 10;
            gamename = Z_Malloc(gamename_size, PU_STATIC, 0);
            version = G_VanillaVersionCode();
            M_snprintf(gamename, gamename_size, deh_sub, version / 100, version % 100);

            while (gamename[0] != '\0' && isspace(gamename[0]))
            {
                memmove(gamename, gamename + 1, gamename_size - 1);
            }

            while (gamename[0] != '\0' && isspace(gamename[strlen(gamename)-1]))
            {
                gamename[strlen(gamename) - 1] = '\0';
            }

            return gamename;
        }
    }

    return gamename;
}


static void SetMissionForPackName(char *pack_name)
{
    int i;
    static const struct
    {
        char *name;
        int mission;
    } packs[] = {
        { "doom2",    doom2 },
        { "tnt",      pack_tnt },
        { "plutonia", pack_plut },
    };

    for (i = 0; i < arrlen(packs); ++i)
    {
        if (!strcasecmp(pack_name, packs[i].name))
        {
            gamemission = packs[i].mission;
            return;
        }
    }

    printf(english_language ?
           "Valid mission packs are:\n" :
           "Корректные серии игр:\n");

    for (i = 0; i < arrlen(packs); ++i)
    {
        printf("\t%s\n", packs[i].name);
    }

    I_Error(english_language ?
            "Unknown mission pack name: %s" :
            "Неизвестная серия игры: %s",
            pack_name);
}


//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

    if (gamemission == none)
    {
        unsigned int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i]->name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i]->name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.
			// Unknown or invalid IWAD file.
            I_Error(english_language ?
                    "Unknown or invalid IWAD file." :
                    "Неопознанный или некорректный IWAD-файл.");
        }
    }

    // Make sure gamemode is set up correctly

    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
            gamemode = retail;      // The Ultimate Doom

        else if (W_CheckNumForName("E3M1") > 0)
            gamemode = registered;  // Doom Registered

        else
            gamemode = shareware;   // Doom Shareware
    }
    else
    {
        int p;

        // Doom 2 of some kind.
        gamemode = commercial;

        // We can manually override the gamemission that we got from the
        // IWAD detection code. This allows us to eg. play Plutonia 2
        // with Freedoom and get the right level names.

        //!
        // @category compat
        // @arg <pack>
        //
        // Explicitly specify a Doom II "mission pack" to run as, instead of
        // detecting it based on the filename. Valid values are: "doom2",
        // "tnt" and "plutonia".
        //
        p = M_CheckParmWithArgs("-pack", 1);
        if (p > 0)
        {
            SetMissionForPackName(myargv[p + 1]);
        }
    }
}

//
// [JN] Load assets for Sigil (main and compat versions)
//

void D_RD_LoadSigilAssets (boolean compat)
{
    char* internalWadName = RD_M_FindInternalResource("doom-sigil.wad");
    W_MergeFile(internalWadName);
    free(internalWadName);
    gamedescription_eng = "SIGIL";
    gamedescription_rus = "СИГИЛ";

    if (compat)
    {
        sgl_compat_loaded = true;
        DEH_AddStringReplacement("RD_EPI3", "RD_SGEP3");
    }
    else
    {
        sgl_loaded = true;
        DEH_AddStringReplacement("RD_EPI5", "RD_SGEP5");
    }
}

//
// [JN] Load assets for No Rest for the Living (NERVE.WAD)
//

void D_RD_LoadNerveAssets (void)
{
    char* internalWadName;

    gamemission = pack_nerve;
    internalWadName = RD_M_FindInternalResource("doom-nerve.wad");
    W_MergeFile(internalWadName);
    free(internalWadName);

    DEH_AddStringReplacement("TITLEPIC", "DMENUPIC");
    DEH_AddStringReplacement("TITLEPI2", "DMENUPIC");
    gamedescription_eng = "DOOM 2: No Rest For The Living";
    gamedescription_rus = "DOOM 2: Нет покоя для живых";
}

// Set the gamedescription string
// [JN] На этом этапе указываем заголовок окна игры и подгрузку 
// необходимых файлов с локализованными ресурсами.

void D_SetGameDescription(void)
{
    int newpwadfile;
    char* internalWadName;

    boolean is_freedoom = W_CheckNumForName("FREEDOOM") >= 0,
            is_freedm = W_CheckNumForName("FREEDM") >= 0;

    gamedescription_eng = "Unknown game";
    gamedescription_rus = "Неизвестная игра";

    // [JN] Always load RD system pwad
    internalWadName = RD_M_FindInternalResource("doom-common.wad");
    W_MergeFile(internalWadName);
    free(internalWadName);

    // [JN] Load extra content for vanilla Doom
    if (!is_freedoom && !is_freedm)
    {
        internalWadName = RD_M_FindInternalResource("doom-extra.wad");
        W_MergeFile(internalWadName);
        free(internalWadName);
    }

    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (is_freedoom)
        {
            internalWadName = RD_M_FindInternalResource("doom-freedoom.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);

            gamedescription_eng = GetGameName("Freedoom: Phase 1");
            gamedescription_rus = GetGameName("Freedoom: Стадия 1");
        }
        else if (gamemode == retail)
        {
            gamedescription_eng = gamedescription_rus = GetGameName("The Ultimate DOOM");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-doom1.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);

            if (gameversion == exe_doom_se)
            {
                gamedescription_eng = GetGameName("DOOM: Special Edition");
                gamedescription_rus = GetGameName("Doom: Специальное издание");
            
                // Episode 4 using a sky from Episode 2
                DEH_AddStringReplacement ("SKY4",   "SKY2");
            }
        }
        else if (gamemode == registered)
        {
            gamedescription_eng = GetGameName("DOOM Registered");
            gamedescription_rus = GetGameName("DOOM");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-doom1.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        else if (gamemode == shareware)
        {
            gamedescription_eng = GetGameName("DOOM Shareware");
            gamedescription_rus = GetGameName("DOOM (Демоверсия)");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-doom1.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);

        }
        else if (gamemode == pressbeta)
        {
            internalWadName = RD_M_FindInternalResource("doom-beta.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);

            DEH_AddStringReplacement (GOTHTHBONUS,     GOTDAGGER);
            DEH_AddStringReplacement (GOTARMBONUS,     GOTCHEST);
            DEH_AddStringReplacement (GOTSUPER,        GOTEXTRALIFE);
            DEH_AddStringReplacement (GOTHTHBONUS_RUS, GOTDAGGER_RUS);
            DEH_AddStringReplacement (GOTARMBONUS_RUS, GOTCHEST_RUS);
            DEH_AddStringReplacement (GOTSUPER_RUS,    GOTEXTRALIFE_RUS);

            gamedescription_eng = GetGameName("Doom Press Release Beta");
            gamedescription_rus = GetGameName("DOOM (Бета-версия)");
        }
    }
    else
    {
        // Doom 2 of some kind.  But which mission?

        if (is_freedoom)
        {
            internalWadName = RD_M_FindInternalResource("doom-freedoom.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);

            if (is_freedm)
            {
                gamedescription_eng = gamedescription_rus = GetGameName("FreeDM");
                internalWadName = RD_M_FindInternalResource("doom-freedm.wad");
                W_MergeFile(internalWadName);
                free(internalWadName);
            }
            else
            {
                gamedescription_eng = GetGameName("Freedoom: Phase 2");
                gamedescription_rus = GetGameName("Freedoom: Стадия 2");
            }
        }
        else if (logical_gamemission == doom2)
        {
            gamedescription_eng = GetGameName("DOOM 2: Hell on Earth");
            gamedescription_rus = GetGameName("DOOM 2: Ад на Земле");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-doom2.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        else if (logical_gamemission == pack_plut)
        {
            gamedescription_eng = GetGameName("Final DOOM: Plutonia Experiment");
            gamedescription_rus = GetGameName("Final DOOM: Эксперимент “Плутония”");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-plutonia.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        else if (logical_gamemission == pack_tnt)
        {
            gamedescription_eng = GetGameName("Final DOOM: TNT - Evilution");
            gamedescription_rus = GetGameName("Final DOOM: TNT - Дьяволюция");

            // [JN] Load widescreen backgrounds
            internalWadName = RD_M_FindInternalResource("doom-wide-tnt.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        else if (logical_gamemission == jaguar)
        {
            internalWadName = RD_M_FindInternalResource("doom-jaguar.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
            
            gamedescription_eng = GetGameName("DOOM for Atari Jaguar");
            gamedescription_rus = GetGameName("DOOM для Atari Jaguar");
        }
    }

    // [JN] PWAD autoloading routine. Scan through all 4 available variables,
    // and don't load empty ones. There are two special cases: SIGIL and NERVE.
    // P.S. You cannot use autoload with the shareware version (register!),
    // as well as in Press Beta version.
    if (gamemode != shareware && gamemode != pressbeta)
    {
        int i;

        for (i = 1 ; i < 5 ; ++i)
        {
            // [JN] If autoloads have not been set, initialize with defaults.
            if (autoloadglobalpwad[i] == NULL)
                autoloadglobalpwad[i] = "";
            if (autoloaddoom1pwad[i] == NULL)
                autoloaddoom1pwad[i] = "";
            if (autoloaddoom2pwad[i] == NULL)
                autoloaddoom2pwad[i] = "";
            if (autoloadplutoniapwad[i] == NULL)
                autoloadplutoniapwad[i] = "";
            if (autoloadtntpwad[i] == NULL)
                autoloadtntpwad[i] = "";

            if (strcmp(autoloadglobalpwad[i], ""))
            {
                W_MergeFile(autoloadglobalpwad[i]);
                printf(english_language ? 
                      " autoloading: %s\n" : " автозагрузка: %s\n",
                        autoloadglobalpwad[i]);
            }

            if (logical_gamemission == doom)
            {
                if (strcmp(autoloaddoom1pwad[i], ""))
                {
                    W_MergeFile(autoloaddoom1pwad[i]);
                    printf(english_language ?
                           " autoloading: %s\n" : " автозагрузка: %s\n",
                           autoloaddoom1pwad[i]);

                    // [JN] Check for SIGIL (main) autoloading
                    if (M_StrCaseStr(autoloaddoom1pwad[i],"sigil.wad")
                    ||  M_StrCaseStr(autoloaddoom1pwad[i],"sigil_v1_2.wad")
                    ||  M_StrCaseStr(autoloaddoom1pwad[i],"sigil_v1_21.wad"))
                    {
                        D_RD_LoadSigilAssets(false);
                    }

                    // [JN] Check for SIGIL (compat) autoloading
                    if (M_StrCaseStr(autoloaddoom1pwad[i],"sigil_compat.wad")
                    ||  M_StrCaseStr(autoloaddoom1pwad[i],"sigil_compat_v1_2.wad")
                    ||  M_StrCaseStr(autoloaddoom1pwad[i],"sigil_compat_v1_21.wad"))
                    {
                        D_RD_LoadSigilAssets(true);
                    }
                }
            }
            else if (logical_gamemission == doom2)
            {
                if (strcmp(autoloaddoom2pwad[i], ""))
                {
                    W_MergeFile(autoloaddoom2pwad[i]);
                    printf(english_language ?
                           " autoloading: %s\n" : " автозагрузка: %s\n",
                           autoloaddoom2pwad[i]);

                    // [JN] Check for No Rest for Living autoloading
                    if (M_StrCaseStr(autoloaddoom2pwad[i],"nerve.wad"))
                    {
                        D_RD_LoadNerveAssets();
                    }
                }
            }
            else if (logical_gamemission == pack_plut)
            {
                if (strcmp(autoloadplutoniapwad[i], ""))
                {
                    W_MergeFile(autoloadplutoniapwad[i]);
                    printf(english_language ?
                           " autoloading: %s\n" : " автозагрузка: %s\n",
                           autoloadplutoniapwad[i]);
                }
            }
            else if (logical_gamemission == pack_tnt)
            {
                if (strcmp(autoloadtntpwad[i], ""))
                {
                    W_MergeFile(autoloadtntpwad[i]);
                    printf(english_language ?
                           " autoadding: %s\n" : " автодобавление: %s\n",
                           autoloadtntpwad[i]);
                }
            }
        }
    }

    // [JN] Параметр "-file" перенесен из w_main.c
    // Необходимо для того, что бы любые ресурсы из pwad-файлов
    // загружались после руссифицированных pwad-файлов.
    // Функция "-merge" более не используется.

    newpwadfile = M_CheckParmWithArgs ("-file", 1);

    // [JN] Using -file for Press Beta is strictly prohibited
    // [Dasperal] Fixed crash when using -beta and -file options
    if (newpwadfile && gamemode != pressbeta)
    {
        while (++newpwadfile != myargc && myargv[newpwadfile][0] != '-')
        {
            char    *filename;

            filename = D_TryFindWADByName(myargv[newpwadfile]);
            printf(english_language ?
                   " adding %s\n" :
                   " добавление: %s\n", filename);
            W_MergeFile(filename);

            // [Dasperal] Переписано на нормальный код
            // [JN] Поддержка DOOM 2: No Rest for the Living
            if (M_StrCaseStr(myargv[newpwadfile], "nerve.wad"))
            {
                D_RD_LoadNerveAssets();
            }
            // [JN] Поддержка Master Levels for DOOM 2
            else if (M_StrCaseStr(myargv[newpwadfile], "ATTACK.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "BLACKTWR.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "BLOODSEA.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "CANYON.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "CATWALK.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "COMBINE.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "FISTULA.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "GARRISON.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "GERYON.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "MANOR.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "MEPHISTO.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "MINOS.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "NESSUS.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "PARADOX.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "SUBSPACE.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "SUBTERRA.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "TEETH.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "TTRAP.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "VESPERAS.WAD") ||
                     M_StrCaseStr(myargv[newpwadfile], "VIRGIL.WAD"))
            {
                internalWadName = RD_M_FindInternalResource("doom-mlevels.wad");
                W_MergeFile(internalWadName);
                free(internalWadName);
                mlvls_loaded = true;

                gamedescription_eng = "Master Levels for DOOM 2";
                gamedescription_rus = "Мастер-уровни для DOOM 2";

                // ATTACK.WAD - Нападение
                if (M_StrCaseStr(myargv[newpwadfile], "ATTACK.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_1);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_1_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL01");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL01R");
                }
                // BLACKTWR.WAD - Черная Башня
                else if (M_StrCaseStr(myargv[newpwadfile], "BLACKTWR.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_25, MLSTR_2);
                    DEH_AddStringReplacement(HUSTR_25_RUS, MLSTR_2_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL02");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL02R");
                }
                // BLOODSEA.WAD - Крепость в Кровавом море
                else if (M_StrCaseStr(myargv[newpwadfile], "BLOODSEA.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_7, MLSTR_3);
                    DEH_AddStringReplacement(HUSTR_7_RUS, MLSTR_3_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL03");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL03R");
                }
                // CANYON.WAD - Каньон
                else if (M_StrCaseStr(myargv[newpwadfile], "CANYON.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_4);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_4_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL04");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL04R");
                }
                // CATWALK.WAD - Помост
                else if (M_StrCaseStr(myargv[newpwadfile], "CATWALK.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_5);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_5_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL05");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL05R");
                }
                // COMBINE.WAD - Комбинат
                else if (M_StrCaseStr(myargv[newpwadfile], "COMBINE.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_6);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_6_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL06");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL06R");
                    DEH_AddStringReplacement("SKY1", "MLSKY1");
                }
                // FISTULA.WAD - Фистула
                else if (M_StrCaseStr(myargv[newpwadfile], "FISTULA.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_7);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_7_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL07");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL07R");
                }
                // GARRISON.WAD - Гарнизон
                else if (M_StrCaseStr(myargv[newpwadfile], "GARRISON.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_8);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_8_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL08");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL08R");
                }
                // GERYON.WAD - Герион
                else if (M_StrCaseStr(myargv[newpwadfile], "GERYON.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_8, MLSTR_9);
                    DEH_AddStringReplacement(HUSTR_8_RUS, MLSTR_9_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL09");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL09R");
                    DEH_AddStringReplacement("SKY1", "MLSKY3");
                }
                // MANOR.WAD - Поместье Гиганта
                else if (M_StrCaseStr(myargv[newpwadfile], "MANOR.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_10);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_10_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL10");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL10R");
                    DEH_AddStringReplacement("SKY1", "MLSKY2");
                }
                // MEPHISTO.WAD - Мавзолей Мефистофеля
                else if (M_StrCaseStr(myargv[newpwadfile], "MEPHISTO.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_7, MLSTR_11);
                    DEH_AddStringReplacement(HUSTR_7_RUS, MLSTR_11_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL11");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL11R");
                }
                // MINOS.WAD - Приговор Миноса
                else if (M_StrCaseStr(myargv[newpwadfile], "MINOS.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_5, MLSTR_12);
                    DEH_AddStringReplacement(HUSTR_5_RUS, MLSTR_12_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL12");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL12R");
                    DEH_AddStringReplacement("SKY1", "MLSKY3");
                }
                // NESSUS.WAD - Несс
                else if (M_StrCaseStr(myargv[newpwadfile], "NESSUS.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_7, MLSTR_13);
                    DEH_AddStringReplacement(HUSTR_7_RUS, MLSTR_13_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL13");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL13R");
                    DEH_AddStringReplacement("SKY1", "MLSKY3");
                }
                // PARADOX.WAD - Парадокс
                else if (M_StrCaseStr(myargv[newpwadfile], "PARADOX.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_14);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_14_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL14");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL14R");
                }
                // SUBSPACE.WAD - Подпространство
                else if (M_StrCaseStr(myargv[newpwadfile], "SUBSPACE.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_15);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_15_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL15");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL15R");
                }
                // SUBTERRA.WAD - Подземелье
                else if (M_StrCaseStr(myargv[newpwadfile], "SUBTERRA.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_16);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_16_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL16");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL16R");
                }
                // TEETH.WAD - Турболифт в Преисподнюю / Дурной сон
                else if (M_StrCaseStr(myargv[newpwadfile], "TEETH.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_31, MLSTR_17);
                    DEH_AddStringReplacement(HUSTR_32, MLSTR_18);
                    DEH_AddStringReplacement(HUSTR_31_RUS, MLSTR_17_RUS);
                    DEH_AddStringReplacement(HUSTR_32_RUS, MLSTR_18_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL17");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL17R");
                }
                // TTRAP.WAD - Застрявший на Титане
                else if (M_StrCaseStr(myargv[newpwadfile], "TTRAP.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_1, MLSTR_19);
                    DEH_AddStringReplacement(HUSTR_1_RUS, MLSTR_19_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL19");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL19R");
                    DEH_AddStringReplacement("SKY1", "MLSKY2");
                }
                // VESPERAS.WAD - Вечер
                else if (M_StrCaseStr(myargv[newpwadfile], "VESPERAS.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_9, MLSTR_20);
                    DEH_AddStringReplacement(HUSTR_9_RUS, MLSTR_20_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL20");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL20R");
                    DEH_AddStringReplacement("SKY1", "MLSKY3");
                }
                // VIRGIL.WAD - Воля Вергилия
                else if (M_StrCaseStr(myargv[newpwadfile], "VIRGIL.WAD"))
                {
                    DEH_AddStringReplacement(HUSTR_3, MLSTR_21);
                    DEH_AddStringReplacement(HUSTR_3_RUS, MLSTR_21_RUS);
                    DEH_AddStringReplacement("WIF", "MLVL21");
                    DEH_AddStringReplacement("RD_WFIN", "MLVL21R");
                    DEH_AddStringReplacement("SKY1", "MLSKY3");
                }
            }
            // [JN] Support for SIGIL (main version)
            else if (M_StrCaseStr(myargv[newpwadfile], "sigil.wad") ||
                     M_StrCaseStr(myargv[newpwadfile], "sigil_v1_2.wad") ||
                     M_StrCaseStr(myargv[newpwadfile], "sigil_v1_21.wad"))
            {
                D_RD_LoadSigilAssets(false);
            }
            // [JN] Support for SIGIL (compat version)
            else if (M_StrCaseStr(myargv[newpwadfile], "sigil_compat.wad") ||
                     M_StrCaseStr(myargv[newpwadfile], "sigil_compat_v1_2.wad") ||
                     M_StrCaseStr(myargv[newpwadfile], "sigil_compat_v1_21.wad"))
            {
                D_RD_LoadSigilAssets(true);
            }
        }
    }

    // [JN] Define and load translated strings
    RD_DefineLanguageStrings();

    // [JN] Sky scaling determination. Sky itself can be loaded as new
    // patches, composed in TEXTURE1 lump, or as patches with standard names.
    if (W_CheckMultipleLumps("TEXTURE1") > 1    // Have new textures
    || W_CheckMultipleLumps("SKY1") > 1         // Doom 1 patches
    || W_CheckMultipleLumps("SKY2") > 1
    || W_CheckMultipleLumps("SKY3") > 1
    || W_CheckMultipleLumps("SKY4") > 1
    || W_CheckMultipleLumps("RSKY1") > 1        // Doom 2 patches
    || W_CheckMultipleLumps("RSKY2") > 1
    || W_CheckMultipleLumps("RSKY3") > 1
    || is_freedoom)                             // Freedoom is also not supported
    {
        scaled_sky = true;
    }
    // [JN] ...So, we are free to use taller skies? Load them.
    // Note: "scaled_sky" variable is still needed for sky rendering.
    else
    {
        scaled_sky = false;

        // Doom 1
        if (gamemode == shareware || gamemode == registered || gamemode == retail)
        {
            internalWadName = RD_M_FindInternalResource("doom-skies-doom1.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        // Doom 2
        else if (gamemission == doom2 || gamemission == pack_nerve)
        {
            internalWadName = RD_M_FindInternalResource("doom-skies-doom2.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        // TNT - Evilution
        else if (gamemission == pack_tnt)
        {
            internalWadName = RD_M_FindInternalResource("doom-skies-tnt.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
        // Plutonia
        else if (gamemission == pack_plut)
        {
            internalWadName = RD_M_FindInternalResource("doom-skies-plutonia.wad");
            W_MergeFile(internalWadName);
            free(internalWadName);
        }
    }

    // [JN] Check if we have exactly two TITLEPICs loaded in Russian version
    // of Doom registered. If two, we are free to use wide version: TITLEPIR,
    // since there is a TITLEPIC for ultimate game mode already. More clear:
    // 1) placed inside IWAD.
    // 2) placed inside doom-doom1-wide.wad.
    // 3 or more) modified titlepic inside PWAD.
    if (gamemode == registered && W_CheckMultipleLumps("TITLEPIC") <= 2)
    {
        DEH_AddStringReplacement ("TITLEPIC",   "TITLEPIR");
    }

    // [JN] Same for Ultimate DOOM.
    if (gamemode == retail && W_CheckMultipleLumps("TITLEPIC") <= 2)
    {
        DEH_AddStringReplacement ("TITLEPIC",   "TITLEPIU");
        DEH_AddStringReplacement ("TITLEPIR",   "TITLEPIU");
    }

    // [JN] Check if we have replaced skill level graphics.
    // If found, replace "Ultra-Nightmare" patch with empty
    // placeholder for keeping menu style in one fashion.
    if (W_CheckMultipleLumps("M_ROUGH") > 1
    && gamemission != jaguar    // [JN] Don't check in Jaguar Doom
    && gamemode != pressbeta)   // [JN] Don't check in Press Beta
    {
        DEH_AddStringReplacement ("M_UNMARE",   "TNT1A0");
    }

    // [JN] Check for modified player's face. If exist, 
    // don't use extra faces. Note that Freedoom should 
    // not have extra faces, as well as vanilla game mode 
    // and if this feature is disabled.
    if (W_CheckMultipleLumps("STFST01") > 1
       || W_CheckMultipleLumps("STFGOD0") > 1
       || is_freedoom || is_freedm
       || vanillaparm)
    {
        old_godface = true;
    }
    else
    {
        old_godface = false;
    }

    // [JN] Check if we are using standard 320x200 patch for PFUB2.
    // If we do, then original scrolling code will be used in F_BunnyScroll.
    if (gamemode != commercial && gamemission != jaguar && gamemode != pressbeta)
    {
        const patch_t* pfub2 = W_CacheLumpName (DEH_String("PFUB2"), PU_STATIC);

        if (W_CheckMultipleLumps("PFUB2") > 2 // lump replaced?
        || pfub2->width == 320                // lump have original width?
        || gamevariant == freedoom)           // always 320x200 in Freedoom1
        {
            pfub2_replaced = true;
        }
    }

    // [JN] Finally, some compatibility mess
    if (gamemode == pressbeta || gamemission == jaguar)
    {
        // Press Beta and Atari Jaguar have own taller skies, 
        // so it's okay to have an unscaled skies.
        scaled_sky = false;

        // Press Beta and Atari Jaguar are supposed to have
        // extra GOD faces, as well as exploding ones.
        old_godface = false;
    }
    
    // Автоматическая загрузка блока DEHACKED
    //
    // [crispy] load DEHACKED lumps by default, but allow overriding
    // [JN] Функция активируется после параметра "-file"

    if (!M_ParmExists("-nodehlump") && !M_ParmExists("-nodeh"))
    {
        int i, loaded = 0;

        for (i = numiwadlumps; i < numlumps; ++i)
        {
            // [JN] Sigil: do not load DEHACKED for 5th episode version,
            // to keep proper episode 3 level names and gfx.
            if (!strncmp(lumpinfo[i]->name, "DEHACKED", 8) && !sgl_loaded)
            {
                DEH_LoadLump(i, true, true); // [crispy] allow long, allow error
                loaded++;
            }
        }
        printf(english_language ?
        " loaded %i DEHACKED lumps from PWAD files.\n" :
        " загружено блоков Dehacked из WAD-файлов: %i.\n", loaded);
    }
}


//      print title for every printed line
char    title[128];

static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    printf(english_language ?
           " loading: %s\n" :
           " загрузка: %s\n",
           filename);
    handle = W_AddFile(filename);

    return handle != NULL;
}

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.

static char *copyright_banners[] =
{
    "===========================================================================\n"
    "ВНИМАНИЕ:  Данная копия игры DOOM подверглась изменениям.  Для приобретения\n"
    "оригинальной игры позвоните 1-800-IDGAMES или ознакомьтесь с файлом ReadMe.\n"
    "          Мы не оказываем поддержки по модифицированным продуктам.\n"
    "                       Для продолжения нажите Enter.\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "              Коммерческий продукт - распространение запрещено!\n"
    "  Просьба сообщать о фактах программного пиратства в SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                          Демонстрационная версия!\n"
    "===========================================================================\n"
};

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners(void)
{
    size_t i;

    for (i=0; i<arrlen(copyright_banners); ++i)
    {
        char *deh_s;

        deh_s = DEH_String(copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);

            // Make sure the modified banner always ends in a newline character.
            // If it doesn't, add a newline.  This fixes av.wad.

            if (deh_s[strlen(deh_s) - 1] != '\n')
            {
                printf("\n");
            }
        }
    }
}

static struct 
{
    char *description;
    char *cmdline;
    GameVersion_t version;
} gameversions[] = {
    {"Doom 1.666",           "1.666",      exe_doom_1_666},
    {"Doom 1.7/1.7a",        "1.7",        exe_doom_1_7},
    {"Doom 1.8",             "1.8",        exe_doom_1_8},
    {"Doom 1.9",             "1.9",        exe_doom_1_9},
//  {"Hacx",                 "hacx",       exe_hacx},
    {"Ultimate Doom",        "ultimate",   exe_ultimate},
    {"Doom 1.9 SE",          "doomse",     exe_doom_se},
    {"Final Doom",           "final",      exe_final},
    {"Final Doom (alt)",     "final2",     exe_final2},
//  {"Chex Quest",           "chex",       exe_chex},
    { NULL,                  NULL,         0},
};

// Initialize the game version

static void InitGameVersion(void)
{
    byte    *demolump;
    char    demolumpname[6];
    int     demoversion;
    int     p;
    int     i;
    boolean status;

    //! 
    // @arg <version>
    // @category compat
    //
    // Emulate a specific version of Doom.  Valid values are "1.666",
    // "1.7", "1.8", "1.9", "ultimate", "final", "final2", "hacx" and
    // "chex".
    //

    p = M_CheckParmWithArgs("-gameversion", 1);

    if (p)
    {
        for (i=0; gameversions[i].description != NULL; ++i)
        {
            if (!strcmp(myargv[p+1], gameversions[i].cmdline))
            {
                gameversion = gameversions[i].version;
                break;
            }
        }

        if (gameversions[i].description == NULL) 
        {
            printf(english_language ?
                   "Supported game versions:\n" :
                   "Поддерживаемые версии игр:\n");

            for (i=0; gameversions[i].description != NULL; ++i)
            {
                printf("\t%s (%s)\n", gameversions[i].cmdline, gameversions[i].description);
            }

            I_Error(english_language ?
                    "Unknown game version '%s'" :
                    "Неизвестная версия игры \"%s\"",
                    myargv[p+1]);
        }
    }
    else
    {
        // Determine automatically

        if (gamemission == pack_chex)
        {
            // chex.exe - identified by iwad filename
            gameversion = exe_chex;
        }
        else if (gamemission == pack_hacx)
        {
            // hacx.exe: identified by iwad filename
            gameversion = exe_hacx;
        }
        else if (gamemode == shareware || gamemode == registered || (gamemode == commercial && gamemission == doom2))
        {
            // original
            gameversion = exe_doom_1_9;

            // Detect version from demo lump
            for (i = 1; i <= 3; ++i)
            {
                M_snprintf(demolumpname, 6, "demo%i", i);
                if (W_CheckNumForName(demolumpname) > 0)
                {
                    demolump = W_CacheLumpName(demolumpname, PU_STATIC);
                    demoversion = demolump[0];
                    W_ReleaseLumpName(demolumpname);
                    status = true;
                    switch (demoversion)
                    {
                        case 106:
                            gameversion = exe_doom_1_666;
                            break;
                        case 107:
                            gameversion = exe_doom_1_7;
                            break;
                        case 108:
                            gameversion = exe_doom_1_8;
                            break;
                        case 109:
                            gameversion = exe_doom_1_9;
                            break;
                        default:
                            status = false;
                            break;
                    }
                    if (status)
                    {
                        break;
                    }
                }
            }
        }
        else if (gamemode == retail)
        {
            gameversion = exe_ultimate;
        }
        else if (gamemode == commercial)
        {
            // Final Doom: tnt or plutonia
            // Defaults to emulating the first Final Doom executable,
            // which has the crash in the demo loop; however, having
            // this as the default should mean that it plays back
            // most demos correctly.

            gameversion = exe_final;
        }
    }

    // The original exe does not support retail - 4th episode not supported

    if (gameversion < exe_ultimate && gamemode == retail)
    {
        gamemode = registered;
    }

    // EXEs prior to the Final Doom exes do not support Final Doom.

    if (gameversion < exe_final && gamemode == commercial && (gamemission == pack_tnt || gamemission == pack_plut))
    {
        gamemission = doom2;
    }
}

void PrintGameVersion(void)
{
    int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
            printf(english_language ?
                   "Emulating the behavior of the '%s' executable.\n" :
                   "Режим эмуляции исполняемого файла \"%s\".\n",
                   gameversions[i].description);
            break;
        }
    }
}

// Function called at exit to display the ENDOOM screen

static void D_Endoom(void)
{
    byte *endoom;

    // Don't show ENDOOM if we have it disabled, or we're running
    // in screensaver or control test mode. Only show it once the
    // game has actually started.
    // [JN] No ENDOOM available in Jaguar Doom.

    if (!show_endoom || !main_loop_started || screensaver_mode || M_CheckParm("-testcontrols") > 0
    ||  gamemission == jaguar)
    {
        return;
    }

	// [JN] Use original, replacable ENDOOM screen for English language.
    // Use various unreplacable translated screens for Russian language.

    if (english_language)
    {
        endoom = W_CacheLumpName(DEH_String("ENDOOM"), PU_STATIC);
    }
    else
    {
        endoom = W_CacheLumpName(DEH_String(gamemode == shareware ? "ENDMRDS" :
                                           gamemode == registered ? "ENDMRDR" :
                                               gamemode == retail ? "ENDMRDU" :
                                  logical_gamemission == pack_tnt ? "ENDMRDT" :
                                 logical_gamemission == pack_plut ? "ENDMRDP" :
                                                                    "ENDMRD2"),
                                                                    PU_STATIC);
    }

    I_Endoom(endoom);
}

// Load dehacked patches needed for certain IWADs.
static void LoadIwadDeh(void)
{
    // The Freedoom IWADs have DEHACKED lumps that must be loaded.
    if (gamevariant == freedoom || gamevariant == freedm)
    {
        // Old versions of Freedoom (before 2014-09) did not have technically
        // valid DEHACKED lumps, so ignore errors and just continue if this
        // is an old IWAD.
        DEH_LoadLumpByName("DEHACKED", false, true);
    }

    // If this is the HACX IWAD, we need to load the DEHACKED lump.
    if (gameversion == exe_hacx)
    {
        if (!DEH_LoadLumpByName("DEHACKED", true, false))
        {
            // "DEHACKED lump not found.  Please check that this is the "
            // "Hacx v1.2 IWAD."
            I_Error(english_language ?
                    "DEHACKED lump not found.  Please check that this is the Hacx v1.2 IWAD." :
                    "Не найден блок DEHACKED.  Проверьте расположение данного блока в IWAD-файе Hacx v1.2 IWAD.");
        }
    }

    // [JN] Doom Press Beta and Doom for Atari Jaguar 
    // have some info in DEHACKED lump, load it.
    if (gamemode == pressbeta || gamemission == jaguar)
    {
        DEH_LoadLumpByName("DEHACKED", false, true);
    }

    // Chex Quest needs a separate Dehacked patch which must be downloaded
    // and installed next to the IWAD.
    if (gameversion == exe_chex)
    {
        char    *chex_deh = NULL;
        char    *sep;

        // Look for chex.deh in the same directory as the IWAD file.
        sep = strrchr(iwadfile, DIR_SEPARATOR);

        if (sep != NULL)
        {
            size_t chex_deh_len = strlen(iwadfile) + 9;
            chex_deh = malloc(chex_deh_len);
            M_StringCopy(chex_deh, iwadfile, chex_deh_len);
            chex_deh[sep - iwadfile + 1] = '\0';
            M_StringConcat(chex_deh, "chex.deh", chex_deh_len);
        }
        else
        {
            chex_deh = M_StringDuplicate("chex.deh");
        }

        // If the dehacked patch isn't found, try searching the WAD
        // search path instead.  We might find it...
        if (!M_FileExists(chex_deh))
        {
            free(chex_deh);
            chex_deh = D_FindWADByName("chex.deh");
        }

        // Still not found?
        if (chex_deh == NULL)
        {
            I_Error("Unable to find Chex Quest dehacked file (chex.deh).\n"
                    "The dehacked file is required in order to emulate\n"
                    "chex.exe correctly.  It can be found in your nearest\n"
                    "/idgames repository mirror at:\n\n"
                    "   utils/exe_edit/patches/chexdeh.zip");
        }

        if (!DEH_LoadFile(chex_deh))
        {
            I_Error("Failed to load chex.deh needed for emulating chex.exe.");
        }
    }
}

static void G_CheckDemoStatusAtExit (void)
{
    G_CheckDemoStatus();
}

//
// D_DoomMain
//

void D_DoomMain (void)
{
    int     p;
    char    file[256];
    char    demolumpname[9];

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
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE
                           | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
                           | FOREGROUND_INTENSITY);

    for (p = 0 ; p < 32 ; p++) DEH_printf(" ");
    DEH_printf(RD_Project_String);
    for (p = 0 ; p < 31 ; p++) DEH_printf(" ");
    DEH_printf("\n");

    // [JN] Fallback to standard console colos
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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

    if (devparm)
    DEH_printf(english_language ? D_DEVSTR : D_DEVSTR_RUS);

    DEH_printf(english_language ?
               "Z_Init: Init zone memory allocation daemon. \n" :
               "Z_Init: Инициализация распределения памяти.\n");
    Z_Init ();

    // [JN] Use current time as seed for random generator.
    srand(time(0));

#ifdef FEATURE_MULTIPLAYER
    //!
    // @category net
    //
    // Start a dedicated server, routing packets but not participating
    // in the game itself.
    //

    if (M_CheckParm("-dedicated") > 0)
    {
        printf(english_language ?
               "Dedicated server mode.\n" :
               "Режим выделенного сервера.\n");
        NET_DedicatedServer();
        // Never returns
    }

    //!
    // @category net
    //
    // Query the Internet master server for a global list of active
    // servers.
    //

    if (M_CheckParm("-search"))
    {
        NET_MasterQuery();
        exit(0);
    }

    //!
    // @arg <address>
    // @category net
    //
    // Query the status of the server running on the given IP
    // address.
    //

    p = M_CheckParmWithArgs("-query", 1);

    if (p)
    {
        NET_QueryAddress(myargv[p+1]);
        exit(0);
    }

    //!
    // @category net
    //
    // Search the local LAN for running servers.
    //

    if (M_CheckParm("-localsearch"))
    {
        NET_LANQuery();
        exit(0);
    }

#endif

    //!
    // @vanilla
    //
    // Disable monsters.
    //

    nomonsters = M_CheckParm ("-nomonsters");

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_CheckParm ("-respawn");

    //!
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = M_CheckParm ("-fast");

    I_DisplayFPSDots(devparm);

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch game.
    //

    if (M_CheckParm ("-deathmatch"))
        deathmatch = 1;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 2.0 game.  Weapons do not stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-altdeath"))
        deathmatch = 2;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 3.0 game.  Weapons stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-dm3"))
        deathmatch = 3;

    // find which dir to use for config files

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\doomdata,
    // allowing play from CD.
    //

    if (M_ParmExists("-cdrom"))
    {
        DEH_printf(english_language ? D_CDROM : D_CDROM_RUS);
        M_SetConfigDir("c:\\doomdata\\");
    }
    else
#endif
    {
        // Auto-detect the configuration dir.
        M_SetConfigDir(NULL);
    }

    //!
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //

    if ( (p=M_CheckParm ("-turbo")) )
    {
        int         scale = 200;
        extern int  forwardmove[2];
        extern int  sidemove[2];

        if (p<myargc-1)
            scale = atoi (myargv[p+1]);
        if (scale < 10)
            scale = 10;
        if (scale > 400)
            scale = 400;
        
        DEH_printf(english_language ?
                   "turbo scale: %i%%\n" :
                   "турбо ускорение: %i%%\n",
                   scale);
        forwardmove[0] = forwardmove[0]*scale/100;
        forwardmove[1] = MIN(forwardmove[1]*scale/100, 127);
        sidemove[0] = sidemove[0]*scale/100;
        sidemove[1] = sidemove[1]*scale/100;
    }

    // Load configuration files before initialising other subsystems.
    DEH_printf(english_language ?
               "M_LoadDefaults: Load system defaults.\n" :
               "M_LoadDefaults: Загрузка системных стандартов.\n");
    M_SetConfigFilename(PROGRAM_PREFIX "doom.ini");
    D_BindVariables();
    M_LoadConfig();

    // init subsystems
    DEH_printf(english_language ?
               "V_Init: allocate screens.\n" :
               "V_Init: Инициализация видео.\n");
    V_Init ();

    // Save configuration at exit.
    I_AtExit(M_SaveConfig, false);

    // Find main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &gamemission);

    // None found?

    if (iwadfile == NULL)
    {
        if (english_language)
        {
            I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                    "specifying one with the '-iwad' command line parameter.\n");
        }
        else
        {
            I_Error("Невозможно определить игру из за отсутствующего IWAD-файла.\n"
                    "Попробуйте указать IWAD-файл командой '-iwad'.\n");
        }
    }

    modifiedgame = false;

    DEH_printf(english_language ?
               "W_Init: Init WADfiles.\n" :
               "W_Init: Инициализация WAD-файлов.\n");
    D_AddFile(iwadfile);
    numiwadlumps = numlumps;

    W_CheckCorrectIWAD(doom);

    // Now that we've loaded the IWAD, we can figure out what gamemission
    // we're playing and which version of Vanilla Doom we need to emulate.
    D_IdentifyVersion();
    InitGameVersion();

    // Check which IWAD variant we are using.

    if (W_CheckNumForName("FREEDOOM") >= 0)
    {
        if (W_CheckNumForName("FREEDM") >= 0)
            gamevariant = freedm;
        else
            gamevariant = freedoom;
    }
    else if (W_CheckNumForName("DMENUPIC") >= 0)
    {
        gamevariant = bfgedition;
    }
    // [JN] Checking for older sharewares
    else if (gamemode == shareware && W_CheckNumForName("STCHAT") >= 0)
    {
        gamevariant = old_shareware;
    }
    // [JN] Checking for Press Beta
    else if (W_CheckNumForName("DOOMPRES") >= 0)
    {
        gamemode = pressbeta;
    }
    // [JN] Checking for Atari Jaguar
    else if (W_CheckNumForName("RDJAGUAR") >= 0)
    {
        gamemission = jaguar;
    }

    //!
    // @category mod
    //
    // Disable automatic loading of Dehacked patches for certain
    // IWAD files.
    //
    if (!M_ParmExists("-nodeh"))
    {
        // Some IWADs have dehacked patches that need to be loaded for
        // them to be played properly.
        LoadIwadDeh();
    }

    // Doom 3: BFG Edition includes modified versions of the classic
    // IWADs which can be identified by an additional DMENUPIC lump.
    // Furthermore, the M_GDHIGH lumps have been modified in a way that
    // makes them incompatible to Vanilla Doom and the modified version
    // of doom2.wad is missing the TITLEPIC lump.
    // We specifically check for DMENUPIC here, before PWADs have been
    // loaded which could probably include a lump of that name.

    if (gamevariant == bfgedition)
    {
        printf(english_language ?
        "BFG Edition: Using workarounds as needed.\n" :
        "BFG Edition: Применение дополнительной совместимости.\n");

        // BFG Edition changes the names of the secret levels to
        // censor the Wolfenstein references.

        DEH_AddStringReplacement(HUSTR_31, "level 31: idkfa");
        DEH_AddStringReplacement(HUSTR_32, "level 32: keen");

        // [JN] Replace Russian names as well.
        DEH_AddStringReplacement(HUSTR_31_RUS, "ehjdtym 31: blraf");         // уровень 31: идкфа
        DEH_AddStringReplacement(HUSTR_32_RUS, "ehjdtym 32: rby");           // уровень 32: кин

        // The BFG edition doesn't have the "low detail" menu option (fair
        // enough). But bizarrely, it reuses the M_GDHIGH patch as a label
        // for the options menu (says "Fullscreen:"). Why the perpetrators
        // couldn't just add a new graphic lump and had to reuse this one,
        // I don't know.
        //
        // The end result is that M_GDHIGH is too wide and causes the game
        // to crash. As a workaround to get a minimum level of support for
        // the BFG edition IWADs, use the "ON"/"OFF" graphics instead.

        DEH_AddStringReplacement("M_GDHIGH", "M_MSGON");
        DEH_AddStringReplacement("M_GDLOW", "M_MSGOFF");
    }

#ifdef FEATURE_DEHACKED
    // Load Dehacked patches specified on the command line with -deh.
    // Note that there's a very careful and deliberate ordering to how
    // Dehacked patches are loaded. The order we use is:
    //  1. IWAD dehacked patches.
    //  2. Command line dehacked patches specified with -deh.
    //  3. PWAD dehacked patches in DEHACKED lumps.
    DEH_ParseCommandLine();
#endif

    // Load PWAD files.
    modifiedgame = W_ParseCommandLine();

    // Debug:
    // W_PrintDirectory();

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs ("-playdemo", 1);

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

        // With Vanilla you have to specify the file without extension,
        // but make that optional.
        if (M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p+1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name, sizeof(demolumpname));
        }
        else
        {
            // If file failed to load, still continue trying to play
            // the demo in the same way as Vanilla Doom.  This makes
            // tricks like "-playdemo demo1" possible.

            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf(english_language ?
               "Playing demo %s.\n" :
               "Проигрывание демозаписи: %s.\n", file);
    }

    I_AtExit(G_CheckDemoStatusAtExit, true);

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // Set the gamedescription string. This is only possible now that
    // we've finished loading Dehacked patches.
    D_SetGameDescription();

    savegamedir = M_GetSaveGameDir();

    // Check for -file in shareware
    if (modifiedgame && (gamevariant != freedoom))
    {
        // These are the lumps that will be checked in IWAD,
        // if any one is not present, execution will be aborted.
        char name[23][8]=
        {
            "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
            "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
            "dphoof","bfgga0","heada1","cybra1","spida1d1"
        };

        if (gamemode == shareware || gamemode == pressbeta)
            I_Error(DEH_String(english_language ?
                               "\nYou cannot -file with the shareware version. Register!" :
                               "\nВы не можете использовать -file в демонстрационной версии. Приобретите полную версию!"));

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version. 
        if (gamemode == registered)
            for (int i = 0 ; i < 23 ; i++)
            if (W_CheckNumForName(name[i])<0)
                I_Error(DEH_String(english_language ?
                                   "\nThis is not the registered version." :
                                   "\nДанная версия не является зарегистрированной."));
    }

    PrintDehackedBanners();

    DEH_printf(english_language ?
               "I_Init: Setting up machine state.\n" :
               "I_Init: Инициализация состояния компьютера.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitController();
    I_InitSound(true);

    // [crispy] check for presence of MAP33
    havemap33 = (gamemode == commercial) &&
                (W_CheckNumForName("map33") != -1) &&
                (W_CheckNumForName("cwilv32") != -1);

    // [crispy] change level name for MAP33
    if (havemap33)
    {
        DEH_AddStringReplacement(PHUSTR_1, "level 33: betray");
        DEH_AddStringReplacement(PHUSTR_1_RUS, "ehjdtym 33: ghtlfntkmcndj"); // уровень 33: предательство
    }

#ifdef FEATURE_MULTIPLAYER
    printf (english_language ?
            "NET_Init: Init network subsystem.\n" :
            "NET_Init: Инициализация сетевой подсистемы.\n");
    NET_Init ();
#endif

    // Initial netgame startup. Connect to server etc.
    D_ConnectNetGame();

    // get skill / episode / map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

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
        startskill = myargv[p+1][0]-'1';
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
        startepisode = myargv[p+1][0]-'0';
        startmap = 1;
        autostart = true;
    }

    timelimit = 0;

    //! 
    // @arg <n>
    // @category net
    // @vanilla
    //
    // For multiplayer games: exit each level after n minutes.
    //

    p = M_CheckParmWithArgs("-timer", 1);

    if (p)
    {
        timelimit = atoi(myargv[p+1]);
    }

    //!
    // @category net
    // @vanilla
    //
    // Austin Virtual Gaming: end levels after 20 minutes.
    //

    p = M_CheckParm ("-avg");

    if (p)
    {
        timelimit = 20;
    }

    //!
    // @arg [<x> <y> | <xy>]
    // @vanilla
    //
    // Start a game immediately, warping to ExMy (Doom 1) or MAPxy
    // (Doom 2)
    //

    p = M_CheckParmWithArgs("-warp", 1);

    if (p)
    {
        if (gamemode == commercial)
            startmap = atoi (myargv[p+1]);
        else
        {
            startepisode = myargv[p+1][0]-'0';

            if (p + 2 < myargc)
            {
                startmap = myargv[p+2][0]-'0';
            }
            else
            {
                startmap = 1;
            }
        }
        gameaction = ga_newgame;
        autostart = true;
    }

    // Undocumented:
    // Invoked by setup to test the controls.

    p = M_CheckParm("-testcontrols");

    if (p > 0)
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

    // Check for load game parameter
    // We do this here and save the slot number, so that the network code
    // can override it or send the load slot to other players.

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    
    if (p)
    {
        startloadgame = atoi(myargv[p+1]);
    }
    else
    {
        // Not loading a game
        startloadgame = -1;
    }

    DEH_printf(english_language ?
               "M_Init: Init miscellaneous info.\n" :
               "M_Init: Инициализация внутренних данных.\n");
    M_Init ();

    DEH_printf(english_language ?
               "R_Init: Init DOOM refresh daemon - [" :
               "R_Init: Инициализация процесса запуска DOOM - [");
    R_Init ();

    DEH_printf(english_language ?
               "\nP_Init: Init Playloop state.\n" :
               "\nP_Init: Инициализация игрового окружения.\n");
    P_Init ();

    DEH_printf(english_language ?
               "S_Init: Setting up sound.\n" :
               "S_Init: Активация звуковой системы.\n");
    S_Init (sfxVolume * 8, musicVolume);

    DEH_printf(english_language ?
               "D_CheckNetGame: Checking network game status.\n" :
               "D_CheckNetGame: Проверка статуса сетевой игры.\n");
    D_CheckNetGame ();

    PrintGameVersion();

    DEH_printf(english_language ?
               "HU_Init: Setting up heads up display.\n" :
               "HU_Init: Инициализация HUD.\n");
    HU_Init ();

    // [JN] Predifine HUD widget colors and lengths.
    HU_Init_Widgets ();

    DEH_printf(english_language ?
               "ST_Init: Init status bar.\n" :
               "ST_Init: Инициализация строки состояния.\n");
    ST_Init ();

    // [JN] Predifine automap color scheme and mark color.
    AM_initColors();
    AM_initMarksColor(automap_mark_color);

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
        G_RecordDemo (myargv[p+1]);
        autostart = true;
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;  // quit after one demo
        G_DeferedPlayDemo (demolumpname);
        D_DoomLoop ();      // never returns
    }

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo (demolumpname);
        D_DoomLoop ();      // never returns
    }

    if (startloadgame >= 0)
    {
        M_StringCopy(file, P_SaveGameFile(startloadgame), sizeof(file));
        G_LoadGame(file);
    }

    if (gameaction != ga_loadgame )
    {
        if (autostart || netgame)
            G_InitNew (startskill, startepisode, startmap);
        else
            D_StartTitle ();    // start up intro loop
    }

    // [JN] Show the game we are playing
    DEH_printf(english_language ? "Starting game: " : "Запуск игры: ");
    DEH_printf("\"");
    DEH_printf(english_language ? gamedescription_eng : gamedescription_rus);
    DEH_printf("\".");
    DEH_printf("\n");

    D_DoomLoop ();  // never returns
}


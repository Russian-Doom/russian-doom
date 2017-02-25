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
//	Intermission screens.
//

// =--------------------------------------------------------------=
// Copyright(C) 2016-2017 Julian Nechaevsky
//
// Описание:
// * Добавлены отдельные сообщения в разных родах для уровней.
// * Добавлена дополнительная графика для сетевых межмисионных
//	 экранов.
// * Добавлено отображение общего игрового времени на межмиссионном
//   экране.
// =--------------------------------------------------------------=

#include <stdio.h>

#include "z_zone.h"

#include "m_misc.h"
#include "m_random.h"

#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "g_game.h"

#include "r_local.h"
#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

// Needs access to LFB.
#include "v_video.h"

#include "wi_stuff.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES	4
#define NUMMAPS		9


// in tics
//U #define PAUSELEN		(TICRATE*2) 
//U #define SCORESTEP		100
//U #define ANIMPERIOD		32
// pixel distance from "(YOU)" to "PLAYER N"
//U #define STARDIST		10 
//U #define WK 1


// GLOBAL LOCATIONS
#define WI_TITLEY		2
#define WI_SPACINGY    		33

// SINGPLE-PLAYER STUFF
#define SP_STATSX		50
#define SP_STATSY		50

#define SP_TIMEX		16
#define SP_TIMEY		(SCREENHEIGHT-32)


// NET GAME STUFF
#define NG_STATSY		50
#define NG_STATSX		(32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX    		64


// DEATHMATCH STUFF
#define DM_MATRIXX		42
#define DM_MATRIXY		68

#define DM_SPACINGX		40

#define DM_TOTALSX		269

#define DM_KILLERSX		10
#define DM_KILLERSY		100
#define DM_VICTIMSX    		5
#define DM_VICTIMSY		50

// [JN] Показывать общее время
extern int  show_total_time;


typedef enum
{
    ANIM_ALWAYS,
    ANIM_RANDOM,
    ANIM_LEVEL

} animenum_t;

typedef struct
{
    int		x;
    int		y;
    
} point_t;


//
// Animation.
// There is another anim_t used in p_spec.
//
typedef struct
{
    animenum_t	type;

    // period in tics between animations
    int		period;

    // number of animation frames
    int		nanims;

    // location of animation
    point_t	loc;

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int		data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int		data2; 

    // actual graphics for frames of animations
    patch_t*	p[3]; 

    // following must be initialized to zero before use!

    // next value of bcnt (used in conjunction with period)
    int		nexttic;

    // last drawn animation frame
    int		lastdrawn;

    // next frame number to animate
    int		ctr;
    
    // used by RANDOM and LEVEL when animating
    int		state;  

} anim_t;


static point_t lnodes[NUMEPISODES][NUMMAPS] =
{
    // Episode 0 World Map
    {
	{ 185, 164 },	// location of level 0 (CJ)
	{ 148, 143 },	// location of level 1 (CJ)
	{ 69, 122 },	// location of level 2 (CJ)
	{ 209, 102 },	// location of level 3 (CJ)
	{ 116, 89 },	// location of level 4 (CJ)
	{ 166, 55 },	// location of level 5 (CJ)
	{ 71, 56 },	// location of level 6 (CJ)
	{ 135, 29 },	// location of level 7 (CJ)
	{ 71, 24 }	// location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    {
	{ 254, 25 },	// location of level 0 (CJ)
	{ 97, 50 },	// location of level 1 (CJ)
	{ 188, 64 },	// location of level 2 (CJ)
	{ 128, 78 },	// location of level 3 (CJ)
	{ 214, 92 },	// location of level 4 (CJ)
	{ 133, 130 },	// location of level 5 (CJ)
	{ 208, 136 },	// location of level 6 (CJ)
	{ 148, 140 },	// location of level 7 (CJ)
	{ 235, 158 }	// location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    {
	{ 156, 168 },	// location of level 0 (CJ)
	{ 48, 154 },	// location of level 1 (CJ)
	{ 174, 95 },	// location of level 2 (CJ)
	{ 265, 75 },	// location of level 3 (CJ)
	{ 130, 48 },	// location of level 4 (CJ)
	{ 279, 23 },	// location of level 5 (CJ)
	{ 198, 48 },	// location of level 6 (CJ)
	{ 140, 25 },	// location of level 7 (CJ)
	{ 281, 136 }	// location of level 8 (CJ)
    }

};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//

#define ANIM(type, period, nanims, x, y, nexttic)            \
   { (type), (period), (nanims), { (x), (y) }, (nexttic),    \
     0, { NULL, NULL, NULL }, 0, 0, 0, 0 }


static anim_t epsd0animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 224, 104, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 184, 160, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 112, 136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 72, 112, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 88, 96, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64, 48, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 192, 40, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 136, 16, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 80, 16, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64, 24, 0),
};

static anim_t epsd1animinfo[] =
{
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 1),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 2),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 3),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 4),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 5),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 6),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 7),
    ANIM(ANIM_LEVEL, TICRATE/3, 3, 192, 144, 8),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 8),
};

static anim_t epsd2animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 168, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 40, 136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 160, 96, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 80, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 120, 32, 0),
    ANIM(ANIM_ALWAYS, TICRATE/4, 3, 40, 0, 0),
};

static int NUMANIMS[NUMEPISODES] =
{
    arrlen(epsd0animinfo),
    arrlen(epsd1animinfo),
    arrlen(epsd2animinfo),
};

static anim_t *anims[NUMEPISODES] =
{
    epsd0animinfo,
    epsd1animinfo,
    epsd2animinfo
};


//
// GENERAL DATA
//

//
// Locally used stuff.
//

// States for single-player
#define SP_KILLS		0
#define SP_ITEMS		2
#define SP_SECRET		4
#define SP_FRAGS		6 
#define SP_TIME			8 
#define SP_PAR			ST_TIME

#define SP_PAUSE		1

// in seconds
#define SHOWNEXTLOCDELAY	4
//#define SHOWLASTLOCDELAY	SHOWNEXTLOCDELAY


// used to accelerate or skip a stage
static int		acceleratestage;

// wbs->pnum
static int		me;

 // specifies current state
static stateenum_t	state;

// contains information passed into intermission
static wbstartstruct_t*	wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int 		cnt;  

// used for timing of background animation
static int 		bcnt;

// signals to refresh everything for one frame
static int 		firstrefresh; 

static int		cnt_kills[MAXPLAYERS];
static int		cnt_items[MAXPLAYERS];
static int		cnt_secret[MAXPLAYERS];
static int		cnt_time;
static int		cnt_par;
static int		cnt_pause;

// # of commercial levels
static int		NUMCMAPS; 


//
//	GRAPHICS
//

// You Are Here graphic
static patch_t*		yah[3] = { NULL, NULL, NULL }; 

// splat
static patch_t*		splat[2] = { NULL, NULL };

// %, : graphics
static patch_t*		percent;
static patch_t*		colon;

// 0-9 graphic
static patch_t*		num[10];

// minus sign
static patch_t*		wiminus;

// "Finished!" graphics
static patch_t*		finished; // Завернен (WIF)
static patch_t*		finished_f; // Завершена (WIFF)
static patch_t*		finished_o; // Завершено (WIFO)
static patch_t*		finished_m; // Завершены (WIFM)

//  "Пред."
static patch_t*		mp_items;

// "Entering" graphic
static patch_t*		entering;  // Загружается
static patch_t*		enterings; // Загружаются

// "secret"
static patch_t*		sp_secret;

 // "Kills", "Scrt", "Items", "Frags"
static patch_t*		kills;
static patch_t*		secret;
static patch_t*		items;
static patch_t*		frags;

// "Враг."
static patch_t*		mp_kills;

// Time sucks.
static patch_t*		timepatch;
static patch_t*		par;
static patch_t*		sucks;

// "killers", "victims"
static patch_t*		killers;
static patch_t*		victims; 

// "Total", your face, your dead face
static patch_t*		total;
static patch_t*		star;
static patch_t*		bstar;

// "Общее время:"
static patch_t*		overtime;

// "red P[1..MAXPLAYERS]"
static patch_t*		p[MAXPLAYERS];

// "gray P[1..MAXPLAYERS]"
static patch_t*		bp[MAXPLAYERS];

 // Name graphics of each level (centered)
static patch_t**	lnames;

// Buffer storing the backdrop
static patch_t *background;

//
// CODE
//

// slam background
void WI_slamBackground(void)
{
    V_DrawPatch(0, 0, background);
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}


// Draws "<Levelname> Finished!"
void WI_drawLF(void)
{
    int y = WI_TITLEY;

    if (gamemode != commercial || wbs->last < NUMCMAPS)
    {
        // draw <LevelName> 
        V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->last]->width))/2,
                    y, lnames[wbs->last]);

        // draw "Finished!"
        y += (5*SHORT(lnames[wbs->last]->height))/4;

		// Отдельные сообщения в разных родах для уровней.
		// finished_f; - Завершена (WIFF)
		// finished_o; - Завершено (WIFO)
		// finished_m; - Завершены (WIFM)

		if (gamemission == doom) // DOOM I
		{
			if (gameepisode == 1) // По колено в трупах
				{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); } 		// E1M1: Ангар
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E1M2: Атомная электростанция
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E1M3: Завод по переработке отходов
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E1M4: Командный пункт
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E1M5: Лаборатория на Фобосе
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E1M6: Центральный пункт обработки
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E1M7: Вычислительный центр
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E1M8: Аномалия на Фобосе
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E1M9: Военная база
				}
		
		if (gameepisode == 2) // Прибрежье Ада
				{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); } 	// E2M1: Аномалия на Деймосе
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// E2M2: Хранилище
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E2M3: Очистительный завод
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E2M4: Лаборатория на Деймосе
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E2M5: Командный центр
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// E2M6: Залы проклятых
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// E2M7: Нерестилище
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E2M8: Вавилонская башня
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E2M9: Крепость тайн
				}
		if (gameepisode == 3) // Инферно
				{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); } 	// E3M1: Крепость Ада
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E3M2: Трясина отчаяния
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M3: Пандемоний
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M4: Дом боли
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M5: Нечестивый собор
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// E3M6: Гора Эреб
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M7: Лимб
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M8: Дит
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// E3M9: Кроличий сад
				}
		if (gameepisode == 4) // Твоя плоть истощена
				{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); } 	// E4M1: Под Адом
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M2: Истинное отвращение
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M3: Расчленение нечестивых
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M4: Непокорное зло
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M5: Они покаются
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M6: Супротив сих нечестивых
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M7: И последовал Ад
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M8: До бессердечия
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }	// E4M9: Страх
				}
		}
		
		if (gamemission == doom2) //  DOOM II
			{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);  }		// Уровень 1: Парадный вход
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m);  }	// Уровень 2: Подземные залы
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);  }		// Уровень 3: Вызов брошен
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 4: Средоточие
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 5: Сточные туннели
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);  }		// Уровень 6: Пресс
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);  }		// Уровень 7: Смертельно простой
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 8: Уловки и ловушки
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f);  }	// Уровень 9: Яма
					if (gamemap == 10 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f);  }	// Уровень 10: Заправочная база
					if (gamemap == 11 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 11: "o" разрушения!
					if (gamemap == 12 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f);  }	// Уровень 12: Фабрика
					if (gamemap == 13 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 13: Деловой район
					if (gamemap == 14 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 14: Глубочайшие логовища
					if (gamemap == 15 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 15: Промышленная зона
					if (gamemap == 16 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 16: Пригород
					if (gamemap == 17 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 17: Владения
					if (gamemap == 18 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 18: Внутренний двор
					if (gamemap == 19 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 19: Цитадель
					if (gamemap == 20 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 20: Попался!
					if (gamemap == 21 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 21: Нирвана
					if (gamemap == 22 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 22: Катакомбы
					if (gamemap == 23 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 23: Целые бочки веселья
					if (gamemap == 24 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 24: Пропасть
					if (gamemap == 25 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 25: Кровопады
					if (gamemap == 26 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 26: Заброшенные шахты
					if (gamemap == 27 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 27: Кондоминиум монстров
					if (gamemap == 28 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 28: Мир Духов
					if (gamemap == 29 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 29: Конец всего живого
					if (gamemap == 30 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 30: Икона греха
					if (gamemap == 31 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 31: Вольфенштайн
					if (gamemap == 32 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 32: Гроссе
					if (gamemap == 33 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 33: Предательство
			}
			
		if (gamemission == pack_plut) // Plutonia Experiment
			{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o);  }	// Уровень 1: Конго
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 2: Колодец душ
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 3: Ацтек
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 4: Запертый в клетке
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 5: Город-призрак
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o);  }	// Уровень 6: Логово Барона
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);  }		// Уровень 7: Двор-ловушка
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 8: Царство
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 9: Аббатство
					if (gamemap == 10 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished);}		// Уровень 10: Натиск
					if (gamemap == 11 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 11: Преследуемый
					if (gamemap == 12 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 12: Скорость
					if (gamemap == 13 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 13: Склеп
					if (gamemap == 14 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 14: Зарождение
					if (gamemap == 15 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 15: Сумерки
					if (gamemap == 16 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 16: Предзнаменование
					if (gamemap == 17 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 17: Компаунд
					if (gamemap == 18 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 18: Нейросфера
					if (gamemap == 19 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 19: В.Р.А.Г.
					if (gamemap == 20 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 20: Обитель Смерти
					if (gamemap == 21 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 21: Убийца
					if (gamemap == 22 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 22: Невыполнимое задание
					if (gamemap == 23 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 23: Надгробие
					if (gamemap == 24 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 24: Последний рубеж
					if (gamemap == 25 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 25: Храм тьмы
					if (gamemap == 26 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 26: Бункер
					if (gamemap == 27 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 27: Антихрист
					if (gamemap == 28 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 28: Канализация
					if (gamemap == 29 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 29: Одиссея шумов
					if (gamemap == 30 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 30: Врата Ада
					if (gamemap == 31 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 31: Киберлогово
					if (gamemap == 32 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 32: Иди к сему
					if (gamemap == 33 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// На всякий случай
			}

			if (gamemission == pack_tnt) // TNT: Evilution
			{
					if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 1: Центр управления системой
					if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 2: Барбекю из человечины
					if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 3: Центр управления питанием
					if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 4: Дыра во времени
					if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 5: Виселица
					if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 6: Открытый сезон
					if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 7: Тюрьма
					if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 8: Металл
					if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 9: Крепость
					if (gamemap == 10 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 10: Искупление
					if (gamemap == 11 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 11: Склад
					if (gamemap == 12 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 12: Кратер
					if (gamemap == 13 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 13: Переработка ядерных отходов
					if (gamemap == 14 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 14: Металлургия
					if (gamemap == 15 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 15: Мертвая зона
					if (gamemap == 16 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 16: Глубочайшие достижения
					if (gamemap == 17 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 17: Зона обработки
					if (gamemap == 18 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 18: Завод
					if (gamemap == 19 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 19: Погрузка/отправка
					if (gamemap == 20 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 20: Центральный пункт обработки
					if (gamemap == 21 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 21: Административный центр
					if (gamemap == 22 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 22: Обиталище
					if (gamemap == 23 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 23: Лунный горный проект
					if (gamemap == 24 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 24: Карьер
					if (gamemap == 25 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }	// Уровень 25: Логово Барона
					if (gamemap == 26 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 26: Баллистикс
					if (gamemap == 27 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 27: Гора Боль
					if (gamemap == 28 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 28: Чертовщина
					if (gamemap == 29 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }	// Уровень 29: Река Стикс
					if (gamemap == 30 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 30: Последний вызов
					if (gamemap == 31 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// Уровень 31: Фараон
					if (gamemap == 32 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }	// Уровень 32: Карибы
					if (gamemap == 33 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width)) / 2, y, finished); }		// На всякий случай
			}
            
			if (gamemission == pack_nerve) // No Rest for the Living
            {
                if (gamemap == 1 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }    // Уровень 1: База на Земле
                if (gamemap == 2 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_m->width)) / 2, y, finished_m); }    // Уровень 2: Лаборатории боли
                if (gamemap == 3 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width))   / 2, y, finished);   }    // Уровень 3: Каньон мертвых
                if (gamemap == 4 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }    // Уровень 4: Адская гора
                if (gamemap == 5 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }    // Уровень 5: Вивисекция
                if (gamemap == 6 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }    // Уровень 6: Кровь преисподняя
                if (gamemap == 7 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished->width))   / 2, y, finished);   }    // Уровень 7: Банкет у Барона
                if (gamemap == 8 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); }    // Уровень 8: Гробница злобы
                if (gamemap == 9 ) { V_DrawPatch((SCREENWIDTH - SHORT(finished_o->width)) / 2, y, finished_o); }    // Уровень 9: Шествие демонов
            }

//		if (gamemission == doom && gameepisode == 1 && gamemap == 05) { V_DrawPatch((SCREENWIDTH - SHORT(finished_f->width)) / 2, y, finished_f); } // E1M5: Лаборатория на Фобосе
					
		
    }
    else if (wbs->last == NUMCMAPS)
    {
        // MAP33 - nothing is displayed!
    }
    else if (wbs->last > NUMCMAPS)
    {
        // > MAP33.  Doom bombs out here with a Bad V_DrawPatch error.
        // I'm pretty sure that doom2.exe is just reading into random
        // bits of memory at this point, but let's try to be accurate
        // anyway.  This deliberately triggers a V_DrawPatch error.

        patch_t tmp = { SCREENWIDTH, SCREENHEIGHT, 1, 1, 
                        { 0, 0, 0, 0, 0, 0, 0, 0 } };

        V_DrawPatch(0, y, &tmp);
    }
}



// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // draw "Entering"
	// [JN] Используется дополнительный титр "ЗАГРУЖАЮТСЯ". 
	
	if (gamemission == doom) // По колено в трупах
	{
		if (gameepisode == 1) // По колено в трупах
		{
		if (gamemap == 1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M1: Ангар 
		if (gamemap == 2 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M2: Атомная электростанция 
		if (gamemap == 3 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M3: Завод по переработке отходов 
		if (gamemap == 4 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M4: Командный пункт 
		if (gamemap == 5 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M5: Лаборатория на Фобосе 
		if (gamemap == 6 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M6: Центральный пункт обработки 
		if (gamemap == 7 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M7: Вычислительный центр 
		if (gamemap == 8 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M8: Аномалия на Фобосе 
		if (gamemap == 9 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);} // E1M9: Военная база
		}
		
		if (gameepisode == 2) // Прибрежье Ада
		{
		if (gamemap == 1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M1: Аномалия на Деймосе
		if (gamemap == 2 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M2: Хранилище
		if (gamemap == 3 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M3: Очистительный завод 
		if (gamemap == 4 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M4: Лаборатория на Деймосе
		if (gamemap == 5 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M5: Командный центр
		if (gamemap == 6 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M6: Залы проклятых
		if (gamemap == 7 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M7: Нерестилище
		if (gamemap == 8 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}   // E2M8: Вавилонская башня
		if (gamemap == 9 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);} // E2M9: Крепость тайн (загрузка E2M6)
		}
		
		if (gameepisode == 3) // Инферно
		{
		if (gamemap == 1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M1: Крепость Ада
		if (gamemap == 2 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M2: Трясина отчаяния
		if (gamemap == 3 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M3: Пандемоний
		if (gamemap == 4 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M4: Дом боли
		if (gamemap == 5 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M5: Нечестивый собор
		if (gamemap == 6 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M6: Гора Эреб
		if (gamemap == 7 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M7: Лимб
		if (gamemap == 8 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M8: Дит
		if (gamemap == 9 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E3M9: Кроличий сад
		}
		
		if (gameepisode == 4) // Твоя плоть истощена
		{
		if (gamemap == 1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M1: Под Адом
		if (gamemap == 2 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M2: Истинное отвращение
		if (gamemap == 3 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M3: Истязание нечестивых
		if (gamemap == 4 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M4: Непокорное зло
		if (gamemap == 5 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M5: Они покаются
		if (gamemap == 6 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M6: Супротив сих нечестивых
		if (gamemap == 7 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M7: И последовал Ад
		if (gamemap == 8 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M8: До бессердечия
		if (gamemap == 9 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}  // E4M9: Страх
		}
	}
	
	if (gamemission == doom2) //  DOOM II
	{
		if (gamemap == 1-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 1: Парадный вход
		if (gamemap == 2-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 2: Подземные залы
		if (gamemap == 3-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 3: Вызов брошен
		if (gamemap == 4-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 4: Средоточие
		if (gamemap == 5-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 5: Сточные туннели
		if (gamemap == 6-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 6: Пресс
		if (gamemap == 7-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 7: Смертельно простой
		if (gamemap == 8-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 8: Уловки и ловушки
		if (gamemap == 9-1 )  {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 9: Яма
		if (gamemap == 10-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 10: Заправочная база
		if (gamemap == 11-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 11: "o" разрушения!
		if (gamemap == 12-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 12: Фабрика
		if (gamemap == 13-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 13: Деловой район
		if (gamemap == 14-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 14: Глубочайшие логовища
		if (gamemap == 15-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 15: Промышленная зона
		if (gamemap == 16-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 16: Пригород
		if (gamemap == 17-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 17: Владения
		if (gamemap == 18-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 18: Внутренний двор
		if (gamemap == 19-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 19: Цитадель
		if (gamemap == 20-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 20: Попался!
		if (gamemap == 21-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 21: Нирвана
		if (gamemap == 22-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 22: Катакомбы
		if (gamemap == 23-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 23: Целые бочки веселья
		if (gamemap == 24-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 24: Пропасть
		if (gamemap == 25-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 25: Кровопады
		if (gamemap == 26-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 26: Заброшенные шахты
		if (gamemap == 27-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 27: Кондоминиум монстров
		if (gamemap == 28-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 28: Мир Духов
		if (gamemap == 29-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 29: Конец всего живого
		if (gamemap == 30-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 30: Икона греха
		if (gamemap == 31-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 31: Вольфенштайн
		if (gamemap == 32-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 32: Гроссе
		if (gamemap == 33-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 33: Предательство
	}
	
	if (gamemission == pack_plut) // Plutonia Experiment
	{
		if (gamemap == 1-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 1: Конго
		if (gamemap == 2-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 2: Колодец душ
		if (gamemap == 3-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 3: Ацтек
		if (gamemap == 4-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 4: Запертый в клетке
		if (gamemap == 5-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 5: Город-призрак
		if (gamemap == 6-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 6: Логово Барона
		if (gamemap == 7-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 7: Двор-ловушка
		if (gamemap == 8-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 8: Царство
		if (gamemap == 9-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 9: Аббатство
		if (gamemap == 10-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 10: Натиск
		if (gamemap == 11-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 11: Преследуемый
		if (gamemap == 12-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 12: Скорость
		if (gamemap == 13-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 13: Склеп
		if (gamemap == 14-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 14: Зарождение
		if (gamemap == 15-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 15: Сумерки
		if (gamemap == 16-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 16: Предзнаменование
		if (gamemap == 17-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 17: Компаунд
		if (gamemap == 18-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 18: Нейросфера
		if (gamemap == 19-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 19: В.Р.А.Г.
		if (gamemap == 20-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 20: Обитель Смерти
		if (gamemap == 21-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 21: Убийца
		if (gamemap == 22-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 22: Невыполнимое задание
		if (gamemap == 23-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 23: Надгробие
		if (gamemap == 24-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 24: Последний рубеж
		if (gamemap == 25-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 25: Храм тьмы
		if (gamemap == 26-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 26: Бункер
		if (gamemap == 27-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 27: Антихрист
		if (gamemap == 28-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 28: Канализация
		if (gamemap == 29-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 29: Одиссея шумов
		if (gamemap == 30-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 30: Врата Ада
		if (gamemap == 31-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 31: Киберлогово
		if (gamemap == 32-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 32: Иди к сему
		if (gamemap == 33-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // На всякий случай
	}
	
	if (gamemission == pack_tnt) // TNT: Evilution
	{
		if (gamemap == 1-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 1: Центр управления системой
		if (gamemap == 2-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 2: Барбекю из человечины
		if (gamemap == 3-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 3: Центр управления питанием
		if (gamemap == 4-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 4: Дыра во времени
		if (gamemap == 5-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 5: Виселица
		if (gamemap == 6-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 6: Открытый сезон
		if (gamemap == 7-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 7: Тюрьма
		if (gamemap == 8-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 8: Металл
		if (gamemap == 9-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);   }  // Уровень 9: Крепость
		if (gamemap == 10-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 10: Искупление
		if (gamemap == 11-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 11: Склад
		if (gamemap == 12-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 12: Кратер
		if (gamemap == 13-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 13: Переработка ядерных отходов
		if (gamemap == 14-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 14: Металлургия
		if (gamemap == 15-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 15: Мертвая зона
		if (gamemap == 16-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 16: Глубочайшие достижения
		if (gamemap == 17-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 17: Зона обработки
		if (gamemap == 18-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 18: Завод
		if (gamemap == 19-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 19: Погрузка/отправка
		if (gamemap == 20-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 20: Центральный пункт обработки
		if (gamemap == 21-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 21: Административный центр
		if (gamemap == 22-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 22: Обиталище
		if (gamemap == 23-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 23: Лунный горный проект
		if (gamemap == 24-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 24: Карьер
		if (gamemap == 25-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 25: Логово Барона
		if (gamemap == 26-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 26: Баллистикс
		if (gamemap == 27-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 27: Гора Боль
		if (gamemap == 28-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 28: Чертовщина
		if (gamemap == 29-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 29: Река Стикс
		if (gamemap == 30-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 30: Последний вызов
		if (gamemap == 31-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);  }  // Уровень 31: Фараон
		if (gamemap == 32-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 32: Карибы (переход на MAP16)
		if (gamemap == 33-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}  // Уровень 32: Карибы (по факту)
	}

	if (gamemission == pack_nerve) // Нет покоя для живых
    {
        if (gamemap == 1-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 1: База на Земле
        if (gamemap == 2-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(enterings->width))/2,y,enterings);}    // Уровень 2: Лаборатории боли
        if (gamemap == 3-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 3: Каньон мертвецов
        if (gamemap == 4-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 4: Адская гора
        if (gamemap == 5-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 5: Вивисекция
        if (gamemap == 6-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 6: Кровь преисподняя
        if (gamemap == 7-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 7: Банкет у Барона
        if (gamemap == 8-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 8: Гробница злобы
        if (gamemap == 9-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}      // Уровень 9: Шествие демонов
        if (gamemap == 10-1 ) {V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,y,entering);}     // Уровень 9: Шествие демонов (по факту)
    }
	
/*    V_DrawPatch((SCREENWIDTH - SHORT(entering->width))/2,
		y,
                entering);
*/

    // draw level
    y += (5*SHORT(lnames[wbs->next]->height))/4;

    V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->next]->width))/2,
		y, 
                lnames[wbs->next]);

}

void
WI_drawOnLnode
( int		n,
  patch_t*	c[] )
{

    int		i;
    int		left;
    int		top;
    int		right;
    int		bottom;
    boolean	fits = false;

    i = 0;
    do
    {
	left = lnodes[wbs->epsd][n].x - SHORT(c[i]->leftoffset);
	top = lnodes[wbs->epsd][n].y - SHORT(c[i]->topoffset);
	right = left + SHORT(c[i]->width);
	bottom = top + SHORT(c[i]->height);

	if (left >= 0
	    && right < SCREENWIDTH
	    && top >= 0
	    && bottom < SCREENHEIGHT)
	{
	    fits = true;
	}
	else
	{
	    i++;
	}
    } while (!fits && i!=2 && c[i] != NULL);

    if (fits && i<2)
    {
	V_DrawPatch(lnodes[wbs->epsd][n].x,
                    lnodes[wbs->epsd][n].y,
		    c[i]);
    }
    else
    {
	// DEBUG
	printf("Could not place patch on level %d", n+1); 
    }
}



void WI_initAnimatedBack(void)
{
    int		i;
    anim_t*	a;

    if (gamemode == commercial)
	return;

    if (wbs->epsd > 2)
	return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
	a = &anims[wbs->epsd][i];

	// init variables
	a->ctr = -1;

	// specify the next time to draw it
	if (a->type == ANIM_ALWAYS)
	    a->nexttic = bcnt + 1 + (M_Random()%a->period);
	else if (a->type == ANIM_RANDOM)
	    a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
	else if (a->type == ANIM_LEVEL)
	    a->nexttic = bcnt + 1;
    }

}

void WI_updateAnimatedBack(void)
{
    int		i;
    anim_t*	a;

    if (gamemode == commercial)
	return;

    if (wbs->epsd > 2)
	return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
	a = &anims[wbs->epsd][i];

	if (bcnt == a->nexttic)
	{
	    switch (a->type)
	    {
	      case ANIM_ALWAYS:
		if (++a->ctr >= a->nanims) a->ctr = 0;
		a->nexttic = bcnt + a->period;
		break;

	      case ANIM_RANDOM:
		a->ctr++;
		if (a->ctr == a->nanims)
		{
		    a->ctr = -1;
		    a->nexttic = bcnt+a->data2+(M_Random()%a->data1);
		}
		else a->nexttic = bcnt + a->period;
		break;
		
	      case ANIM_LEVEL:
		// gawd-awful hack for level anims
		if (!(state == StatCount && i == 7)
		    && wbs->next == a->data1)
		{
		    a->ctr++;
		    if (a->ctr == a->nanims) a->ctr--;
		    a->nexttic = bcnt + a->period;
		}
		break;
	    }
	}

    }

}

void WI_drawAnimatedBack(void)
{
    int			i;
    anim_t*		a;

    if (gamemode == commercial)
	return;

    if (wbs->epsd > 2)
	return;

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
    {
	a = &anims[wbs->epsd][i];

	if (a->ctr >= 0)
	    V_DrawPatch(a->loc.x, a->loc.y, a->p[a->ctr]);
    }

}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int
WI_drawNum
( int		x,
  int		y,
  int		n,
  int		digits )
{

    int		fontwidth = SHORT(num[0]->width);
    int		neg;
    int		temp;

    if (digits < 0)
    {
	if (!n)
	{
	    // make variable-length zeros 1 digit long
	    digits = 1;
	}
	else
	{
	    // figure out # of digits in #
	    digits = 0;
	    temp = n;

	    while (temp)
	    {
		temp /= 10;
		digits++;
	    }
	}
    }

    neg = n < 0;
    if (neg)
	n = -n;

    // if non-number, do not draw it
    if (n == 1994)
	return 0;

    // draw the new number
    while (digits--)
    {
	x -= fontwidth;
	V_DrawPatch(x, y, num[ n % 10 ]);
	n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
	V_DrawPatch(x-=8, y, wiminus);

    return x;

}

void
WI_drawPercent
( int		x,
  int		y,
  int		p )
{
    if (p < 0)
	return;

    V_DrawPatch(x, y, percent);
    WI_drawNum(x, y, p, -1);
}



//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
void
WI_drawTime
( int		x,
  int		y,
  int		t,
  boolean   suck  )
{

    int		div;
    int		n;

    if (t<0)
	return;

    if (t <= 61*59 || !suck)
    {
	div = 1;

	do
	{
	    n = (t / div) % 60;
	    x = WI_drawNum(x, y, n, 2) - SHORT(colon->width);
	    div *= 60;

	    // draw
	    if (div==60 || t / div)
		V_DrawPatch(x, y, colon);
	    
	} while (t / div);
    }
    else
    {
	// "sucks"
	V_DrawPatch(x - SHORT(sucks->width), y, sucks); 
    }
}


void WI_End(void)
{
    void WI_unloadData(void);
    WI_unloadData();
}

void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}

void WI_updateNoState(void) {

    WI_updateAnimatedBack();

    if (!--cnt)
    {
        // Don't call WI_End yet.  G_WorldDone doesnt immediately 
        // change gamestate, so WI_Drawer is still going to get
        // run until that happens.  If we do that after WI_End
        // (which unloads all the graphics), we're in trouble.
	//WI_End();
	G_WorldDone();
    }

}

static boolean		snl_pointeron = false;


void WI_initShowNextLoc(void)
{
    state = ShowNextLoc;
    acceleratestage = 0;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    WI_initAnimatedBack();
}

void WI_updateShowNextLoc(void)
{
    WI_updateAnimatedBack();

    if (!--cnt || acceleratestage)
	WI_initNoState();
    else
	snl_pointeron = (cnt & 31) < 20;
}

void WI_drawShowNextLoc(void)
{

    int		i;
    int		last;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    if ( gamemode != commercial)
    {
  	if (wbs->epsd > 2)
	{
	    WI_drawEL();
	    return;
	}
	
	last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

	// draw a splat on taken cities.
	for (i=0 ; i<=last ; i++)
	    WI_drawOnLnode(i, splat);

	// splat the secret level?
	if (wbs->didsecret)
	    WI_drawOnLnode(8, splat);

	// draw flashing ptr
	if (snl_pointeron)
	    WI_drawOnLnode(wbs->next, yah); 
    }
    
    if (gamemission == pack_nerve && wbs->last == 7) 
        return;

    // draws which level you are entering..
    if ( (gamemode != commercial)
	 || wbs->next != 30)
	WI_drawEL();  

}

void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}

int WI_fragSum(int playernum)
{
    int		i;
    int		frags = 0;
    
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]
	    && i!=playernum)
	{
	    frags += plrs[playernum].frags[i];
	}
    }

	
    // JDC hack - negative frags.
    frags -= plrs[playernum].frags[playernum];
    // UNUSED if (frags < 0)
    // 	frags = 0;

    return frags;
}



static int		dm_state;
static int		dm_frags[MAXPLAYERS][MAXPLAYERS];
static int		dm_totals[MAXPLAYERS];



void WI_initDeathmatchStats(void)
{

    int		i;
    int		j;

    state = StatCount;
    acceleratestage = 0;
    dm_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i])
	{
	    for (j=0 ; j<MAXPLAYERS ; j++)
		if (playeringame[j])
		    dm_frags[i][j] = 0;

	    dm_totals[i] = 0;
	}
    }
    
    WI_initAnimatedBack();
}



void WI_updateDeathmatchStats(void)
{

    int		i;
    int		j;
    
    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && dm_state != 4)
    {
	acceleratestage = 0;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (playeringame[i])
	    {
		for (j=0 ; j<MAXPLAYERS ; j++)
		    if (playeringame[j])
			dm_frags[i][j] = plrs[i].frags[j];

		dm_totals[i] = WI_fragSum(i);
	    }
	}
	

	S_StartSound(0, sfx_barexp);
	dm_state = 4;
    }

    
    if (dm_state == 2)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);
	
	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (playeringame[i])
	    {
		for (j=0 ; j<MAXPLAYERS ; j++)
		{
		    if (playeringame[j]
			&& dm_frags[i][j] != plrs[i].frags[j])
		    {
			if (plrs[i].frags[j] < 0)
			    dm_frags[i][j]--;
			else
			    dm_frags[i][j]++;

			if (dm_frags[i][j] > 99)
			    dm_frags[i][j] = 99;

			if (dm_frags[i][j] < -99)
			    dm_frags[i][j] = -99;
			
			stillticking = true;
		    }
		}
		dm_totals[i] = WI_fragSum(i);

		if (dm_totals[i] > 99)
		    dm_totals[i] = 99;
		
		if (dm_totals[i] < -99)
		    dm_totals[i] = -99;
	    }
	    
	}
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    dm_state++;
	}

    }
    else if (dm_state == 4)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_slop);

	    if ( gamemode == commercial)
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (dm_state & 1)
    {
	if (!--cnt_pause)
	{
	    dm_state++;
	    cnt_pause = TICRATE;
	}
    }
}



void WI_drawDeathmatchStats(void)
{

    int		i;
    int		j;
    int		x;
    int		y;
    int		w;

    WI_slamBackground();
    
    // draw animated background
    WI_drawAnimatedBack(); 
    WI_drawLF();

    // draw stat titles (top line)
    V_DrawPatch(DM_TOTALSX-SHORT(total->width)/2,
		DM_MATRIXY-WI_SPACINGY+10,
		total);
    
    V_DrawPatch(DM_KILLERSX, DM_KILLERSY, killers);
    V_DrawPatch(DM_VICTIMSX, DM_VICTIMSY, victims);

    // draw P?
    x = DM_MATRIXX + DM_SPACINGX;
    y = DM_MATRIXY;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i])
	{
	    V_DrawPatch(x-SHORT(p[i]->width)/2,
			DM_MATRIXY - WI_SPACINGY,
			p[i]);
	    
	    V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)/2,
			y,
			p[i]);

	    if (i == me)
	    {
		V_DrawPatch(x-SHORT(p[i]->width)/2,
			    DM_MATRIXY - WI_SPACINGY,
			    bstar);

		V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)/2,
			    y,
			    star);
	    }
	}
	else
	{
	    // V_DrawPatch(x-SHORT(bp[i]->width)/2,
	    //   DM_MATRIXY - WI_SPACINGY, bp[i]);
	    // V_DrawPatch(DM_MATRIXX-SHORT(bp[i]->width)/2,
	    //   y, bp[i]);
	}
	x += DM_SPACINGX;
	y += WI_SPACINGY;
    }

    // draw stats
    y = DM_MATRIXY+10;
    w = SHORT(num[0]->width);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	x = DM_MATRIXX + DM_SPACINGX;

	if (playeringame[i])
	{
	    for (j=0 ; j<MAXPLAYERS ; j++)
	    {
		if (playeringame[j])
		    WI_drawNum(x+w, y, dm_frags[i][j], 2);

		x += DM_SPACINGX;
	    }
	    WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
	}
	y += WI_SPACINGY;
    }
}

static int	cnt_frags[MAXPLAYERS];
static int	dofrags;
static int	ng_state;

void WI_initNetgameStats(void)
{

    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (!playeringame[i])
	    continue;

	cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

	dofrags += WI_fragSum(i);
    }

    dofrags = !!dofrags;

    WI_initAnimatedBack();
}



void WI_updateNetgameStats(void)
{

    int		i;
    int		fsum;
    
    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && ng_state != 10)
    {
	acceleratestage = 0;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
	    cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
	    cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;

	    if (dofrags)
		cnt_frags[i] = WI_fragSum(i);
	}
	S_StartSound(0, sfx_barexp);
	ng_state = 10;
    }

    if (ng_state == 2)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_kills[i] += 2;

	    if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
		cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state++;
	}
    }
    else if (ng_state == 4)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_items[i] += 2;
	    if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
		cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
	    else
		stillticking = true;
	}
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state++;
	}
    }
    else if (ng_state == 6)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_secret[i] += 2;

	    if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
		cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state += 1 + 2*!dofrags;
	}
    }
    else if (ng_state == 8)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_frags[i] += 1;

	    if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
		cnt_frags[i] = fsum;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_pldeth);
	    ng_state++;
	}
    }
    else if (ng_state == 10)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_sgcock);
	    if ( gamemode == commercial )
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (ng_state & 1)
    {
	if (!--cnt_pause)
	{
	    ng_state++;
	    cnt_pause = TICRATE;
	}
    }
}



void WI_drawNetgameStats(void)
{
    int		i;
    int		x;
    int		y;
    int		pwidth = SHORT(percent->width);

    WI_slamBackground();
    
    // draw animated background
    WI_drawAnimatedBack(); 

    WI_drawLF();

    // draw stat titles (top line)
    V_DrawPatch(NG_STATSX+NG_SPACINGX-SHORT(mp_kills->width),
		NG_STATSY, mp_kills);

    V_DrawPatch(NG_STATSX+2*NG_SPACINGX-SHORT(mp_items->width),
		NG_STATSY, mp_items);

    V_DrawPatch(NG_STATSX+3*NG_SPACINGX-SHORT(secret->width),
		NG_STATSY, secret);
    
    if (dofrags)
	V_DrawPatch(NG_STATSX+4*NG_SPACINGX-SHORT(frags->width),
		    NG_STATSY, frags);

    // draw stats
    y = NG_STATSY + SHORT(kills->height);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (!playeringame[i])
	    continue;

	x = NG_STATSX;
	V_DrawPatch(x-SHORT(p[i]->width), y, p[i]);

	if (i == me)
	    V_DrawPatch(x-SHORT(p[i]->width), y, star);

	x += NG_SPACINGX;
	WI_drawPercent(x-pwidth, y+10, cnt_kills[i]);	x += NG_SPACINGX;
	WI_drawPercent(x-pwidth, y+10, cnt_items[i]);	x += NG_SPACINGX;
	WI_drawPercent(x-pwidth, y+10, cnt_secret[i]);	x += NG_SPACINGX;

	if (dofrags)
	    WI_drawNum(x, y+10, cnt_frags[i], -1);

	y += WI_SPACINGY;
    }

}

static int	sp_state;

void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();
}

void WI_updateStats(void)
{

    WI_updateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {
	acceleratestage = 0;
	cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	cnt_time = plrs[me].stime / TICRATE;
	cnt_par = wbs->partime / TICRATE;
	S_StartSound(0, sfx_barexp);
	sp_state = 10;
    }

    if (sp_state == 2)
    {
	cnt_kills[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
	{
	    cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }
    else if (sp_state == 4)
    {
	cnt_items[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
	{
	    cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }
    else if (sp_state == 6)
    {
	cnt_secret[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
	{
	    cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }

    else if (sp_state == 8)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	cnt_time += 3;

	if (cnt_time >= plrs[me].stime / TICRATE)
	    cnt_time = plrs[me].stime / TICRATE;

	cnt_par += 3;

	if (cnt_par >= wbs->partime / TICRATE)
	{
	    cnt_par = wbs->partime / TICRATE;

	    if (cnt_time >= plrs[me].stime / TICRATE)
	    {
		S_StartSound(0, sfx_barexp);
		sp_state++;
	    }
	}
    }
    else if (sp_state == 10)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_sgcock);

	    if (gamemode == commercial)
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (sp_state & 1)
    {
	if (!--cnt_pause)
	{
	    sp_state++;
	    cnt_pause = TICRATE;
	}
    }

}

void WI_drawStats(void)
{
    // line height
    int lh;	

    lh = (3*SHORT(num[0]->height))/2;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();
    
    WI_drawLF();

    V_DrawPatch(SP_STATSX, SP_STATSY, kills);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawPatch(SP_STATSX, SP_STATSY+lh, items);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    V_DrawPatch(SP_STATSX, SP_STATSY+2*lh, sp_secret);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

    V_DrawPatch(SP_TIMEX, SP_TIMEY, timepatch);
    WI_drawTime(SCREENWIDTH/2 - SP_TIMEX, SP_TIMEY, cnt_time, true);

    if (wbs->epsd < 4)
    {
	V_DrawPatch(SCREENWIDTH/2 + SP_TIMEX, SP_TIMEY, par);
	WI_drawTime(SCREENWIDTH - SP_TIMEX, SP_TIMEY, cnt_par, true);
    }
    
    // [crispy] draw total time after level time and par time
    // [JN] Но только если опция включена в настройках совместимости
    if (show_total_time)
    {
        if (sp_state > 8)
        {
        const int ttime = wbs->totaltimes / TICRATE;
        const boolean wide = (ttime > 61*59) || (SP_TIMEX + SHORT(overtime->width) >= 320/4);
    
        V_DrawPatch(SP_TIMEX + 24, SP_TIMEY + 16, overtime);
        // [crispy] choose x-position depending on width of time string
        WI_drawTime(296 - SP_TIMEX, SP_TIMEY + 16, ttime, false);
        }
    }
}

void WI_checkForAccelerate(void)
{
    int   i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
	if (playeringame[i])
	{
	    if (player->cmd.buttons & BT_ATTACK)
	    {
		if (!player->attackdown)
		    acceleratestage = 1;
		player->attackdown = true;
	    }
	    else
		player->attackdown = false;
	    if (player->cmd.buttons & BT_USE)
	    {
		if (!player->usedown)
		    acceleratestage = 1;
		player->usedown = true;
	    }
	    else
		player->usedown = false;
	}
    }
}



// Updates stuff each tick
void WI_Ticker(void)
{
    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
	// intermission music
  	if ( gamemode == commercial )
	  S_ChangeMusic(mus_dm2int, true);
	else
	  S_ChangeMusic(mus_inter, true); 
    }

    WI_checkForAccelerate();

    switch (state)
    {
      case StatCount:
	if (deathmatch) WI_updateDeathmatchStats();
	else if (netgame) WI_updateNetgameStats();
	else WI_updateStats();
	break;
	
      case ShowNextLoc:
	WI_updateShowNextLoc();
	break;
	
      case NoState:
	WI_updateNoState();
	break;
    }

}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

// Common load/unload function.  Iterates over all the graphics
// lumps to be loaded/unloaded into memory.

static void WI_loadUnloadData(load_callback_t callback)
{
    int i, j;
    char name[9];
    anim_t *a;

    if (gamemode == commercial)
    {
	for (i=0 ; i<NUMCMAPS ; i++)
	{
	    DEH_snprintf(name, 9, "CWILV%2.2d", i);
            callback(name, &lnames[i]);
	}
    }
    else
    {
	for (i=0 ; i<NUMMAPS ; i++)
	{
	    DEH_snprintf(name, 9, "WILV%d%d", wbs->epsd, i);
            callback(name, &lnames[i]);
	}

	// you are here
        callback(DEH_String("WIURH0"), &yah[0]);

	// you are here (alt.)
        callback(DEH_String("WIURH1"), &yah[1]);

	// splat
        callback(DEH_String("WISPLAT"), &splat[0]);

	if (wbs->epsd < 3)
	{
	    for (j=0;j<NUMANIMS[wbs->epsd];j++)
	    {
		a = &anims[wbs->epsd][j];
		for (i=0;i<a->nanims;i++)
		{
		    // MONDO HACK!
		    if (wbs->epsd != 1 || j != 8)
		    {
			// animations
			DEH_snprintf(name, 9, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                        callback(name, &a->p[i]);
		    }
		    else
		    {
			// HACK ALERT!
			a->p[i] = anims[1][4].p[i];
		    }
		}
	    }
	}
    }

    // More hacks on minus sign.
    callback(DEH_String("WIMINUS"), &wiminus);

    for (i=0;i<10;i++)
    {
	 // numbers 0-9
	DEH_snprintf(name, 9, "WINUM%d", i);
        callback(name, &num[i]);
    }

    // percent sign
    callback(DEH_String("WIPCNT"), &percent);

    // "finished" / "Завершен"
	callback(DEH_String("WIF"), &finished);
	
	// "Завершена"
	callback(DEH_String("WIFF"), &finished_f);
	
	// "Завершено"
	callback(DEH_String("WIFO"), &finished_o);
	
	// "Завершены"
	callback(DEH_String("WIFM"), &finished_m);
	
	// "Загружается"
    callback(DEH_String("WIENTER"), &entering);
	
	// "Загружются"
    callback(DEH_String("WIENTERS"), &enterings);

    // "kills"
    callback(DEH_String("WIOSTK"), &kills);

    // "scrt"
    callback(DEH_String("WIOSTS"), &secret);
	
     // "secret"
    callback(DEH_String("WISCRT2"), &sp_secret);

	// "Враг." (для сетевой игры)
	callback(DEH_String("WIOSTKS"), &mp_kills);
	
	// "Пред." (для сетевой игры)
	callback(DEH_String("WIOSTIS"), &mp_items);
	
    // french wad uses WIOBJ (?)
    if (W_CheckNumForName(DEH_String("WIOBJ")) >= 0)
    {
    	// "items"
    	if (netgame && !deathmatch)
            callback(DEH_String("WIOBJ"), &items);
    	else
            callback(DEH_String("WIOSTI"), &items);
    } else {
        callback(DEH_String("WIOSTI"), &items);
    }

    // "frgs"
    callback(DEH_String("WIFRGS"), &frags);

    // ":"
    callback(DEH_String("WICOLON"), &colon);

    // "time"
    callback(DEH_String("WITIME"), &timepatch);

    // "sucks"
    callback(DEH_String("WISUCKS"), &sucks);

    // "par"
    callback(DEH_String("WIPAR"), &par);

    // "killers" (vertical)
    callback(DEH_String("WIKILRS"), &killers);

    // "victims" (horiz)
    callback(DEH_String("WIVCTMS"), &victims);

    // "total"
    callback(DEH_String("WIMSTT"), &total);
    
    // "Общее время:"
    callback(DEH_String("WIOVTIME"), &overtime);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	// "1,2,3,4"
	DEH_snprintf(name, 9, "STPB%d", i);
        callback(name, &p[i]);

	// "1,2,3,4"
	DEH_snprintf(name, 9, "WIBP%d", i+1);
        callback(name, &bp[i]);
    }

    // Background image

    if (gamemode == commercial)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else if (gamemode == retail && wbs->epsd == 3)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else
    {
	DEH_snprintf(name, sizeof(name), "WIMAP%d", wbs->epsd);
    }

    // Draw backdrop and save to a temporary buffer

    callback(name, &background);
}
static void WI_loadCallback(char *name, patch_t **variable)
{
    *variable = W_CacheLumpName(name, PU_STATIC);
}

void WI_loadData(void)
{
    if (gamemode == commercial)
    {
	NUMCMAPS = 32;
	lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS,
				       PU_STATIC, NULL);
    }
    else
    {
	lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS,
				       PU_STATIC, NULL);
    }

    WI_loadUnloadData(WI_loadCallback);

    // These two graphics are special cased because we're sharing
    // them with the status bar code

    // your face
    star = W_CacheLumpName(DEH_String("STFST01"), PU_STATIC);

    // dead face
    bstar = W_CacheLumpName(DEH_String("STFDEAD0"), PU_STATIC);
}

static void WI_unloadCallback(char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}

void WI_unloadData(void)
{
    WI_loadUnloadData(WI_unloadCallback);

    // We do not free these lumps as they are shared with the status
    // bar code.
   
    // W_ReleaseLumpName("STFST01");
    // W_ReleaseLumpName("STFDEAD0");
}

void WI_Drawer (void)
{
    switch (state)
    {
      case StatCount:
	if (deathmatch)
	    WI_drawDeathmatchStats();
	else if (netgame)
	    WI_drawNetgameStats();
	else
	    WI_drawStats();
	break;
	
      case ShowNextLoc:
	WI_drawShowNextLoc();
	break;
	
      case NoState:
	WI_drawNoState();
	break;
    }
}


void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

    wbs = wbstartstruct;

#ifdef RANGECHECKING
    if (gamemode != commercial)
    {
      if ( gamemode == retail )
	RNGCHECK(wbs->epsd, 0, 3);
      else
	RNGCHECK(wbs->epsd, 0, 2);
    }
    else
    {
	RNGCHECK(wbs->last, 0, 8);
	RNGCHECK(wbs->next, 0, 8);
    }
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
	wbs->maxkills = 1;

    if (!wbs->maxitems)
	wbs->maxitems = 1;

    if (!wbs->maxsecret)
	wbs->maxsecret = 1;

    if ( gamemode != retail )
      if (wbs->epsd > 2)
	wbs->epsd -= 3;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();

    if (deathmatch)
	WI_initDeathmatchStats();
    else if (netgame)
	WI_initNetgameStats();
    else
	WI_initStats();
}

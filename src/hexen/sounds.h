//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2020 Julian Nechaevsky
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



#ifndef __SOUNDSH__
#define __SOUNDSH__

#include "i_sound.h"

#define MAX_SND_DIST    2025
#define MAX_CHANNELS    64

// Music identifiers

typedef enum
{
    mus_e1m1,
    mus_e1m2,
    mus_e1m3,
    mus_e1m4,
    mus_e1m5,
    mus_e1m6,
    mus_e1m7,
    mus_e1m8,
    mus_e1m9,
    mus_e2m1,
    mus_e2m2,
    mus_e2m3,
    mus_e2m4,
    mus_e2m5,
    mus_e2m6,
    mus_e2m7,
    mus_e2m8,
    mus_e2m9,
    mus_e3m1,
    mus_e3m2,
    mus_e3m3,
    mus_e3m4,
    mus_e3m5,
    mus_e3m6,
    mus_e3m7,
    mus_e3m8,
    mus_e3m9,
    mus_e4m1,
    mus_titl,
    mus_intr,
    mus_cptd,
    NUMMUSIC
} musicenum_t;

// Sound identifiers

typedef enum
{
    SFX_NONE,
    SFX_PLAYER_FIGHTER_NORMAL_DEATH,    // class specific death screams
    SFX_PLAYER_FIGHTER_CRAZY_DEATH,
    SFX_PLAYER_FIGHTER_EXTREME1_DEATH,
    SFX_PLAYER_FIGHTER_EXTREME2_DEATH,
    SFX_PLAYER_FIGHTER_EXTREME3_DEATH,
    SFX_PLAYER_FIGHTER_BURN_DEATH,
    SFX_PLAYER_CLERIC_NORMAL_DEATH,
    SFX_PLAYER_CLERIC_CRAZY_DEATH,
    SFX_PLAYER_CLERIC_EXTREME1_DEATH,
    SFX_PLAYER_CLERIC_EXTREME2_DEATH,
    SFX_PLAYER_CLERIC_EXTREME3_DEATH,
    SFX_PLAYER_CLERIC_BURN_DEATH,
    SFX_PLAYER_MAGE_NORMAL_DEATH,
    SFX_PLAYER_MAGE_CRAZY_DEATH,
    SFX_PLAYER_MAGE_EXTREME1_DEATH,
    SFX_PLAYER_MAGE_EXTREME2_DEATH,
    SFX_PLAYER_MAGE_EXTREME3_DEATH,
    SFX_PLAYER_MAGE_BURN_DEATH,
    SFX_PLAYER_FIGHTER_PAIN,
    SFX_PLAYER_CLERIC_PAIN,
    SFX_PLAYER_MAGE_PAIN,
    SFX_PLAYER_FIGHTER_GRUNT,
    SFX_PLAYER_CLERIC_GRUNT,
    SFX_PLAYER_MAGE_GRUNT,
    SFX_PLAYER_LAND,
    SFX_PLAYER_POISONCOUGH,
    SFX_PLAYER_FIGHTER_FALLING_SCREAM,  // class specific falling screams
    SFX_PLAYER_CLERIC_FALLING_SCREAM,
    SFX_PLAYER_MAGE_FALLING_SCREAM,
    SFX_PLAYER_FALLING_SPLAT,
    SFX_PLAYER_FIGHTER_FAILED_USE,
    SFX_PLAYER_CLERIC_FAILED_USE,
    SFX_PLAYER_MAGE_FAILED_USE,
    SFX_PLATFORM_START,
    SFX_PLATFORM_STARTMETAL,
    SFX_PLATFORM_STOP,
    SFX_STONE_MOVE,
    SFX_METAL_MOVE,
    SFX_DOOR_OPEN,
    SFX_DOOR_LOCKED,
    SFX_DOOR_METAL_OPEN,
    SFX_DOOR_METAL_CLOSE,
    SFX_DOOR_LIGHT_CLOSE,
    SFX_DOOR_HEAVY_CLOSE,
    SFX_DOOR_CREAK,
    SFX_PICKUP_WEAPON,
    SFX_PICKUP_ARTIFACT,
    SFX_PICKUP_KEY,
    SFX_PICKUP_ITEM,
    SFX_PICKUP_PIECE,
    SFX_WEAPON_BUILD,
    SFX_ARTIFACT_USE,
    SFX_ARTIFACT_BLAST,
    SFX_TELEPORT,
    SFX_THUNDER_CRASH,
    SFX_FIGHTER_PUNCH_MISS,
    SFX_FIGHTER_PUNCH_HITTHING,
    SFX_FIGHTER_PUNCH_HITWALL,
    SFX_FIGHTER_GRUNT,
    SFX_FIGHTER_AXE_HITTHING,
    SFX_FIGHTER_HAMMER_MISS,
    SFX_FIGHTER_HAMMER_HITTHING,
    SFX_FIGHTER_HAMMER_HITWALL,
    SFX_FIGHTER_HAMMER_CONTINUOUS,
    SFX_FIGHTER_HAMMER_EXPLODE,
    SFX_FIGHTER_SWORD_FIRE,
    SFX_FIGHTER_SWORD_EXPLODE,
    SFX_CLERIC_CSTAFF_FIRE,
    SFX_CLERIC_CSTAFF_EXPLODE,
    SFX_CLERIC_CSTAFF_HITTHING,
    SFX_CLERIC_FLAME_FIRE,
    SFX_CLERIC_FLAME_EXPLODE,
    SFX_CLERIC_FLAME_CIRCLE,
    SFX_MAGE_WAND_FIRE,
    SFX_MAGE_LIGHTNING_FIRE,
    SFX_MAGE_LIGHTNING_ZAP,
    SFX_MAGE_LIGHTNING_CONTINUOUS,
    SFX_MAGE_LIGHTNING_READY,
    SFX_MAGE_SHARDS_FIRE,
    SFX_MAGE_SHARDS_EXPLODE,
    SFX_MAGE_STAFF_FIRE,
    SFX_MAGE_STAFF_EXPLODE,
    SFX_SWITCH1,
    SFX_SWITCH2,
    SFX_SERPENT_SIGHT,
    SFX_SERPENT_ACTIVE,
    SFX_SERPENT_PAIN,
    SFX_SERPENT_ATTACK,
    SFX_SERPENT_MELEEHIT,
    SFX_SERPENT_DEATH,
    SFX_SERPENT_BIRTH,
    SFX_SERPENTFX_CONTINUOUS,
    SFX_SERPENTFX_HIT,
    SFX_POTTERY_EXPLODE,
    SFX_DRIP,
    SFX_CENTAUR_SIGHT,
    SFX_CENTAUR_ACTIVE,
    SFX_CENTAUR_PAIN,
    SFX_CENTAUR_ATTACK,
    SFX_CENTAUR_DEATH,
    SFX_CENTAURLEADER_ATTACK,
    SFX_CENTAUR_MISSILE_EXPLODE,
    SFX_WIND,
    SFX_BISHOP_SIGHT,
    SFX_BISHOP_ACTIVE,
    SFX_BISHOP_PAIN,
    SFX_BISHOP_ATTACK,
    SFX_BISHOP_DEATH,
    SFX_BISHOP_MISSILE_EXPLODE,
    SFX_BISHOP_BLUR,
    SFX_DEMON_SIGHT,
    SFX_DEMON_ACTIVE,
    SFX_DEMON_PAIN,
    SFX_DEMON_ATTACK,
    SFX_DEMON_MISSILE_FIRE,
    SFX_DEMON_MISSILE_EXPLODE,
    SFX_DEMON_DEATH,
    SFX_WRAITH_SIGHT,
    SFX_WRAITH_ACTIVE,
    SFX_WRAITH_PAIN,
    SFX_WRAITH_ATTACK,
    SFX_WRAITH_MISSILE_FIRE,
    SFX_WRAITH_MISSILE_EXPLODE,
    SFX_WRAITH_DEATH,
    SFX_PIG_ACTIVE1,
    SFX_PIG_ACTIVE2,
    SFX_PIG_PAIN,
    SFX_PIG_ATTACK,
    SFX_PIG_DEATH,
    SFX_MAULATOR_SIGHT,
    SFX_MAULATOR_ACTIVE,
    SFX_MAULATOR_PAIN,
    SFX_MAULATOR_HAMMER_SWING,
    SFX_MAULATOR_HAMMER_HIT,
    SFX_MAULATOR_MISSILE_HIT,
    SFX_MAULATOR_DEATH,
    SFX_FREEZE_DEATH,
    SFX_FREEZE_SHATTER,
    SFX_ETTIN_SIGHT,
    SFX_ETTIN_ACTIVE,
    SFX_ETTIN_PAIN,
    SFX_ETTIN_ATTACK,
    SFX_ETTIN_DEATH,
    SFX_FIRED_SPAWN,
    SFX_FIRED_ACTIVE,
    SFX_FIRED_PAIN,
    SFX_FIRED_ATTACK,
    SFX_FIRED_MISSILE_HIT,
    SFX_FIRED_DEATH,
    SFX_ICEGUY_SIGHT,
    SFX_ICEGUY_ACTIVE,
    SFX_ICEGUY_ATTACK,
    SFX_ICEGUY_FX_EXPLODE,
    SFX_SORCERER_SIGHT,
    SFX_SORCERER_ACTIVE,
    SFX_SORCERER_PAIN,
    SFX_SORCERER_SPELLCAST,
    SFX_SORCERER_BALLWOOSH,
    SFX_SORCERER_DEATHSCREAM,
    SFX_SORCERER_BISHOPSPAWN,
    SFX_SORCERER_BALLPOP,
    SFX_SORCERER_BALLBOUNCE,
    SFX_SORCERER_BALLEXPLODE,
    SFX_SORCERER_BIGBALLEXPLODE,
    SFX_SORCERER_HEADSCREAM,
    SFX_DRAGON_SIGHT,
    SFX_DRAGON_ACTIVE,
    SFX_DRAGON_WINGFLAP,
    SFX_DRAGON_ATTACK,
    SFX_DRAGON_PAIN,
    SFX_DRAGON_DEATH,
    SFX_DRAGON_FIREBALL_EXPLODE,
    SFX_KORAX_SIGHT,
    SFX_KORAX_ACTIVE,
    SFX_KORAX_PAIN,
    SFX_KORAX_ATTACK,
    SFX_KORAX_COMMAND,
    SFX_KORAX_DEATH,
    SFX_KORAX_STEP,
    SFX_THRUSTSPIKE_RAISE,
    SFX_THRUSTSPIKE_LOWER,
    SFX_STAINEDGLASS_SHATTER,
    SFX_FLECHETTE_BOUNCE,
    SFX_FLECHETTE_EXPLODE,
    SFX_LAVA_MOVE,
    SFX_WATER_MOVE,
    SFX_ICE_STARTMOVE,
    SFX_EARTH_STARTMOVE,
    SFX_WATER_SPLASH,
    SFX_LAVA_SIZZLE,
    SFX_SLUDGE_GLOOP,
    SFX_CHOLY_FIRE,
    SFX_SPIRIT_ACTIVE,
    SFX_SPIRIT_ATTACK,
    SFX_SPIRIT_DIE,
    SFX_VALVE_TURN,
    SFX_ROPE_PULL,
    SFX_FLY_BUZZ,
    SFX_IGNITE,
    SFX_PUZZLE_SUCCESS,
    SFX_PUZZLE_FAIL_FIGHTER,
    SFX_PUZZLE_FAIL_CLERIC,
    SFX_PUZZLE_FAIL_MAGE,
    SFX_EARTHQUAKE,
    SFX_BELLRING,
    SFX_TREE_BREAK,
    SFX_TREE_EXPLODE,
    SFX_SUITOFARMOR_BREAK,
    SFX_POISONSHROOM_PAIN,
    SFX_POISONSHROOM_DEATH,
    SFX_AMBIENT1,
    SFX_AMBIENT2,
    SFX_AMBIENT3,
    SFX_AMBIENT4,
    SFX_AMBIENT5,
    SFX_AMBIENT6,
    SFX_AMBIENT7,
    SFX_AMBIENT8,
    SFX_AMBIENT9,
    SFX_AMBIENT10,
    SFX_AMBIENT11,
    SFX_AMBIENT12,
    SFX_AMBIENT13,
    SFX_AMBIENT14,
    SFX_AMBIENT15,
    SFX_STARTUP_TICK,
    SFX_SWITCH_OTHERLEVEL,
    SFX_RESPAWN,
    SFX_KORAX_VOICE_1,
    SFX_KORAX_VOICE_2,
    SFX_KORAX_VOICE_3,
    SFX_KORAX_VOICE_4,
    SFX_KORAX_VOICE_5,
    SFX_KORAX_VOICE_6,
    SFX_KORAX_VOICE_7,
    SFX_KORAX_VOICE_8,
    SFX_KORAX_VOICE_9,
    SFX_BAT_SCREAM,
    SFX_CHAT,
    SFX_MENU_MOVE,
    SFX_CLOCK_TICK,
    SFX_FIREBALL,
    SFX_PUPPYBEAT,
    SFX_MYSTICINCANT,
    NUMSFX
} sfxenum_t;

#endif

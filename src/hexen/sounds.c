//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2023 Leonid Murin (Dasperal)
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



#include "h2def.h"
#include "sounds.h"

// Music info

/*
musicinfo_t S_music[] =
{
	{ "MUS_E1M1", 0 }, // 1-1
	{ "MUS_E1M2", 0 },
	{ "MUS_E1M3", 0 },
	{ "MUS_E1M4", 0 },
	{ "MUS_E1M5", 0 },
	{ "MUS_E1M6", 0 },
	{ "MUS_E1M7", 0 },
	{ "MUS_E1M8", 0 },
	{ "MUS_E1M9", 0 },
	{ "MUS_E2M1", 0 }, // 2-1
	{ "MUS_E2M2", 0 },
	{ "MUS_E2M3", 0 },
	{ "MUS_E2M4", 0 },
	{ "MUS_E1M4", 0 },
	{ "MUS_E2M6", 0 },
	{ "MUS_E2M7", 0 },
	{ "MUS_E2M8", 0 },
	{ "MUS_E2M9", 0 },
	{ "MUS_E1M1", 0 }, // 3-1
	{ "MUS_E3M2", 0 },
	{ "MUS_E3M3", 0 },
	{ "MUS_E1M6", 0 },
	{ "MUS_E1M3", 0 },
	{ "MUS_E1M2", 0 },
	{ "MUS_E1M5", 0 },
	{ "MUS_E1M9", 0 },
	{ "MUS_E2M6", 0 },
	{ "MUS_E1M6", 0 }, // 4-1
	{ "MUS_TITL", 0 },
	{ "MUS_INTR", 0 },
	{ "MUS_CPTD", 0 }
};
*/

// Sound info

#define SOUND(name, priority, numchannels, pitchshift) \
    { name, "", priority, NULL, pitchshift, 0, -1, 0, numchannels, NULL }

sfxinfo_t S_sfx[] = {
    // tagname, lumpname, priority, usefulness, snd_ptr, lumpnum, numchannels,
    //              pitchshift
    SOUND("", 0, 0, 0),
    SOUND("PlayerFighterNormalDeath", 256, 2, 1),
    SOUND("PlayerFighterCrazyDeath", 256, 2, 1),
    SOUND("PlayerFighterExtreme1Death", 256, 2, 1),
    SOUND("PlayerFighterExtreme2Death", 256, 2, 1),
    SOUND("PlayerFighterExtreme3Death", 256, 2, 1),
    SOUND("PlayerFighterBurnDeath", 256, 2, 1),
    SOUND("PlayerClericNormalDeath", 256, 2, 1),
    SOUND("PlayerClericCrazyDeath", 256, 2, 1),
    SOUND("PlayerClericExtreme1Death", 256, 2, 1),
    SOUND("PlayerClericExtreme2Death", 256, 2, 1),
    SOUND("PlayerClericExtreme3Death", 256, 2, 1),
    SOUND("PlayerClericBurnDeath", 256, 2, 1),
    SOUND("PlayerMageNormalDeath", 256, 2, 0),
    SOUND("PlayerMageCrazyDeath", 256, 2, 0),
    SOUND("PlayerMageExtreme1Death", 256, 2, 0),
    SOUND("PlayerMageExtreme2Death", 256, 2, 0),
    SOUND("PlayerMageExtreme3Death", 256, 2, 0),
    SOUND("PlayerMageBurnDeath", 256, 2, 0),
    SOUND("PlayerFighterPain", 256, 2, 1),
    SOUND("PlayerClericPain", 256, 2, 1),
    SOUND("PlayerMagePain", 256, 2, 0),
    SOUND("PlayerFighterGrunt", 256, 2, 1),
    SOUND("PlayerClericGrunt", 256, 2, 1),
    SOUND("PlayerMageGrunt", 256, 2, 0),
    SOUND("PlayerLand", 32, 2, 1),
    SOUND("PlayerPoisonCough", 256, 2, 1),
    SOUND("PlayerFighterFallingScream", 256, 2, 1),
    SOUND("PlayerClericFallingScream", 256, 2, 1),
    SOUND("PlayerMageFallingScream", 256, 2, 0),
    SOUND("PlayerFallingSplat", 256, 2, 1),
    SOUND("PlayerFighterFailedUse", 256, 1, 1),
    SOUND("PlayerClericFailedUse", 256, 1, 1),
    SOUND("PlayerMageFailedUse", 256, 1, 0),
    SOUND("PlatformStart", 36, 2, 1),
    SOUND("PlatformStartMetal", 36, 2, 1),
    SOUND("PlatformStop", 40, 2, 1),
    SOUND("StoneMove", 32, 2, 1),
    SOUND("MetalMove", 32, 2, 1),
    SOUND("DoorOpen", 36, 2, 1),
    SOUND("DoorLocked", 36, 2, 1),
    SOUND("DoorOpenMetal", 36, 2, 1),
    SOUND("DoorCloseMetal", 36, 2, 1),
    SOUND("DoorCloseLight", 36, 2, 1),
    SOUND("DoorCloseHeavy", 36, 2, 1),
    SOUND("DoorCreak", 36, 2, 1),
    SOUND("PickupWeapon", 36, 2, 0),
    SOUND("PickupArtifact", 36, 2, 1),
    SOUND("PickupKey", 36, 2, 1),
    SOUND("PickupItem", 36, 2, 1),
    SOUND("PickupPiece", 36, 2, 0),
    SOUND("WeaponBuild", 36, 2, 0),
    SOUND("UseArtifact", 36, 2, 1),
    SOUND("BlastRadius", 36, 2, 1),
    SOUND("Teleport", 256, 2, 1),
    SOUND("ThunderCrash", 30, 2, 1),
    SOUND("FighterPunchMiss", 80, 2, 1),
    SOUND("FighterPunchHitThing", 80, 2, 1),
    SOUND("FighterPunchHitWall", 80, 2, 1),
    SOUND("FighterGrunt", 80, 2, 1),
    SOUND("FighterAxeHitThing", 80, 2, 1),
    SOUND("FighterHammerMiss", 80, 2, 1),
    SOUND("FighterHammerHitThing", 80, 2, 1),
    SOUND("FighterHammerHitWall", 80, 2, 1),
    SOUND("FighterHammerContinuous", 32, 2, 1),
    SOUND("FighterHammerExplode", 80, 2, 1),
    SOUND("FighterSwordFire", 80, 2, 1),
    SOUND("FighterSwordExplode", 80, 2, 1),
    SOUND("ClericCStaffFire", 80, 2, 1),
    SOUND("ClericCStaffExplode", 40, 2, 1),
    SOUND("ClericCStaffHitThing", 80, 2, 1),
    SOUND("ClericFlameFire", 80, 2, 1),
    SOUND("ClericFlameExplode", 80, 2, 1),
    SOUND("ClericFlameCircle", 80, 2, 1),
    SOUND("MageWandFire", 80, 2, 1),
    SOUND("MageLightningFire", 80, 2, 1),
    SOUND("MageLightningZap", 32, 2, 1),
    SOUND("MageLightningContinuous", 32, 2, 1),
    SOUND("MageLightningReady", 30, 2, 1),
    SOUND("MageShardsFire", 80, 2, 1),
    SOUND("MageShardsExplode", 36, 2, 1),
    SOUND("MageStaffFire", 80, 2, 1),
    SOUND("MageStaffExplode", 40, 2, 1),
    SOUND("Switch1", 32, 2, 1),
    SOUND("Switch2", 32, 2, 1),
    SOUND("SerpentSight", 32, 2, 1),
    SOUND("SerpentActive", 32, 2, 1),
    SOUND("SerpentPain", 32, 2, 1),
    SOUND("SerpentAttack", 32, 2, 1),
    SOUND("SerpentMeleeHit", 32, 2, 1),
    SOUND("SerpentDeath", 40, 2, 1),
    SOUND("SerpentBirth", 32, 2, 1),
    SOUND("SerpentFXContinuous", 32, 2, 1),
    SOUND("SerpentFXHit", 32, 2, 1),
    SOUND("PotteryExplode", 32, 2, 1),
    SOUND("Drip", 32, 2, 1),
    SOUND("CentaurSight", 32, 2, 1),
    SOUND("CentaurActive", 32, 2, 1),
    SOUND("CentaurPain", 32, 2, 1),
    SOUND("CentaurAttack", 32, 2, 1),
    SOUND("CentaurDeath", 40, 2, 1),
    SOUND("CentaurLeaderAttack", 32, 2, 1),
    SOUND("CentaurMissileExplode", 32, 2, 1),
    SOUND("Wind", 1, 2, 1),
    SOUND("BishopSight", 32, 2, 1),
    SOUND("BishopActive", 32, 2, 1),
    SOUND("BishopPain", 32, 2, 1),
    SOUND("BishopAttack", 32, 2, 1),
    SOUND("BishopDeath", 40, 2, 1),
    SOUND("BishopMissileExplode", 32, 2, 1),
    SOUND("BishopBlur", 32, 2, 1),
    SOUND("DemonSight", 32, 2, 1),
    SOUND("DemonActive", 32, 2, 1),
    SOUND("DemonPain", 32, 2, 1),
    SOUND("DemonAttack", 32, 2, 1),
    SOUND("DemonMissileFire", 32, 2, 1),
    SOUND("DemonMissileExplode", 32, 2, 1),
    SOUND("DemonDeath", 40, 2, 1),
    SOUND("WraithSight", 32, 2, 1),
    SOUND("WraithActive", 32, 2, 1),
    SOUND("WraithPain", 32, 2, 1),
    SOUND("WraithAttack", 32, 2, 1),
    SOUND("WraithMissileFire", 32, 2, 1),
    SOUND("WraithMissileExplode", 32, 2, 1),
    SOUND("WraithDeath", 40, 2, 1),
    SOUND("PigActive1", 32, 2, 1),
    SOUND("PigActive2", 32, 2, 1),
    SOUND("PigPain", 32, 2, 1),
    SOUND("PigAttack", 32, 2, 1),
    SOUND("PigDeath", 40, 2, 1),
    SOUND("MaulatorSight", 32, 2, 1),
    SOUND("MaulatorActive", 32, 2, 1),
    SOUND("MaulatorPain", 32, 2, 1),
    SOUND("MaulatorHamSwing", 32, 2, 1),
    SOUND("MaulatorHamHit", 32, 2, 1),
    SOUND("MaulatorMissileHit", 32, 2, 1),
    SOUND("MaulatorDeath", 40, 2, 1),
    SOUND("FreezeDeath", 40, 2, 1),
    SOUND("FreezeShatter", 40, 2, 1),
    SOUND("EttinSight", 32, 2, 1),
    SOUND("EttinActive", 32, 2, 1),
    SOUND("EttinPain", 32, 2, 1),
    SOUND("EttinAttack", 32, 2, 1),
    SOUND("EttinDeath", 40, 2, 1),
    SOUND("FireDemonSpawn", 32, 2, 1),
    SOUND("FireDemonActive", 32, 2, 1),
    SOUND("FireDemonPain", 32, 2, 1),
    SOUND("FireDemonAttack", 32, 2, 1),
    SOUND("FireDemonMissileHit", 32, 2, 1),
    SOUND("FireDemonDeath", 40, 2, 1),
    SOUND("IceGuySight", 32, 2, 1),
    SOUND("IceGuyActive", 32, 2, 1),
    SOUND("IceGuyAttack", 32, 2, 1),
    SOUND("IceGuyMissileExplode", 32, 2, 1),
    SOUND("SorcererSight", 256, 2, 1),
    SOUND("SorcererActive", 256, 2, 1),
    SOUND("SorcererPain", 256, 2, 1),
    SOUND("SorcererSpellCast", 256, 2, 1),
    SOUND("SorcererBallWoosh", 256, 4, 1),
    SOUND("SorcererDeathScream", 256, 2, 1),
    SOUND("SorcererBishopSpawn", 80, 2, 1),
    SOUND("SorcererBallPop", 80, 2, 1),
    SOUND("SorcererBallBounce", 80, 3, 1),
    SOUND("SorcererBallExplode", 80, 3, 1),
    SOUND("SorcererBigBallExplode", 80, 3, 1),
    SOUND("SorcererHeadScream", 256, 2, 1),
    SOUND("DragonSight", 64, 2, 1),
    SOUND("DragonActive", 64, 2, 1),
    SOUND("DragonWingflap", 64, 2, 1),
    SOUND("DragonAttack", 64, 2, 1),
    SOUND("DragonPain", 64, 2, 1),
    SOUND("DragonDeath", 64, 2, 1),
    SOUND("DragonFireballExplode", 32, 2, 1),
    SOUND("KoraxSight", 256, 2, 1),
    SOUND("KoraxActive", 256, 2, 1),
    SOUND("KoraxPain", 256, 2, 1),
    SOUND("KoraxAttack", 256, 2, 1),
    SOUND("KoraxCommand", 256, 2, 1),
    SOUND("KoraxDeath", 256, 2, 1),
    SOUND("KoraxStep", 128, 2, 1),
    SOUND("ThrustSpikeRaise", 32, 2, 1),
    SOUND("ThrustSpikeLower", 32, 2, 1),
    SOUND("GlassShatter", 32, 2, 1),
    SOUND("FlechetteBounce", 32, 2, 1),
    SOUND("FlechetteExplode", 32, 2, 1),
    SOUND("LavaMove", 36, 2, 1),
    SOUND("WaterMove", 36, 2, 1),
    SOUND("IceStartMove", 36, 2, 1),
    SOUND("EarthStartMove", 36, 2, 1),
    SOUND("WaterSplash", 32, 2, 1),
    SOUND("LavaSizzle", 32, 2, 1),
    SOUND("SludgeGloop", 32, 2, 1),
    SOUND("HolySymbolFire", 64, 2, 1),
    SOUND("SpiritActive", 32, 2, 1),
    SOUND("SpiritAttack", 32, 2, 1),
    SOUND("SpiritDie", 32, 2, 1),
    SOUND("ValveTurn", 36, 2, 1),
    SOUND("RopePull", 36, 2, 1),
    SOUND("FlyBuzz", 20, 2, 1),
    SOUND("Ignite", 32, 2, 1),
    SOUND("PuzzleSuccess", 256, 2, 1),
    SOUND("PuzzleFailFighter", 256, 2, 1),
    SOUND("PuzzleFailCleric", 256, 2, 1),
    SOUND("PuzzleFailMage", 256, 2, 1),
    SOUND("Earthquake", 32, 2, 1),
    SOUND("BellRing", 32, 2, 0),
    SOUND("TreeBreak", 32, 2, 1),
    SOUND("TreeExplode", 32, 2, 1),
    SOUND("SuitofArmorBreak", 32, 2, 1),
    SOUND("PoisonShroomPain", 20, 2, 1),
    SOUND("PoisonShroomDeath", 32, 2, 1),
    SOUND("Ambient1", 1, 1, 1),
    SOUND("Ambient2", 1, 1, 1),
    SOUND("Ambient3", 1, 1, 1),
    SOUND("Ambient4", 1, 1, 1),
    SOUND("Ambient5", 1, 1, 1),
    SOUND("Ambient6", 1, 1, 1),
    SOUND("Ambient7", 1, 1, 1),
    SOUND("Ambient8", 1, 1, 1),
    SOUND("Ambient9", 1, 1, 1),
    SOUND("Ambient10", 1, 1, 1),
    SOUND("Ambient11", 1, 1, 1),
    SOUND("Ambient12", 1, 1, 1),
    SOUND("Ambient13", 1, 1, 1),
    SOUND("Ambient14", 1, 1, 1),
    SOUND("Ambient15", 1, 1, 1),
    SOUND("StartupTick", 32, 2, 1),
    SOUND("SwitchOtherLevel", 32, 2, 1),
    SOUND("Respawn", 32, 2, 1),
    SOUND("KoraxVoiceGreetings", 512, 2, 1),
    SOUND("KoraxVoiceReady", 512, 2, 1),
    SOUND("KoraxVoiceBlood", 512, 2, 1),
    SOUND("KoraxVoiceGame", 512, 2, 1),
    SOUND("KoraxVoiceBoard", 512, 2, 1),
    SOUND("KoraxVoiceWorship", 512, 2, 1),
    SOUND("KoraxVoiceMaybe", 512, 2, 1),
    SOUND("KoraxVoiceStrong", 512, 2, 1),
    SOUND("KoraxVoiceFace", 512, 2, 1),
    SOUND("BatScream", 32, 2, 1),
    SOUND("Chat", 512, 2, 1),
    SOUND("MenuMove", 32, 2, 1),
    SOUND("ClockTick", 32, 2, 1),
    SOUND("Fireball", 32, 2, 1),
    SOUND("PuppyBeat", 30, 2, 1),
    SOUND("MysticIncant", 32, 4, 1),
    //[Dasperal] Russian versions of KoraxVoice
    SOUND("KoraxVoiceGreetingsRU", 512, 2, 1),
    SOUND("KoraxVoiceReadyRU", 512, 2, 1),
    SOUND("KoraxVoiceBloodRU", 512, 2, 1),
    SOUND("KoraxVoiceGameRU", 512, 2, 1),
    SOUND("KoraxVoiceBoardRU", 512, 2, 1),
    SOUND("KoraxVoiceWorshipRU", 512, 2, 1),
    SOUND("KoraxVoiceMaybeRU", 512, 2, 1),
    SOUND("KoraxVoiceStrongRU", 512, 2, 1),
    SOUND("KoraxVoiceFaceRU", 512, 2, 1),
};

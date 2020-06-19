//
// Copyright(C) 1993-1996 Id Software, Inc.
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

//
// Памятка по символам:
// --------------------
// < = ,
// > = .
// $ = BFG
// # = y 
// @ = n
// ^ = "


#ifndef __RD_LANG__
#define __RD_LANG__


extern void RD_DefineLanguageStrings ();


// -----------------------------------------------------------------------------
// English language
// -----------------------------------------------------------------------------

//
// AM_map.C
//

extern char* amstr_followon;
extern char* amstr_followoff;
extern char* amstr_gridon;
extern char* amstr_gridoff;
extern char* amstr_markedspot;
extern char* amstr_markscleared;
extern char* amstr_overlayon;
extern char* amstr_overlayoff;
extern char* amstr_rotateon;
extern char* amstr_rotateoff;

#define AMSTR_FOLLOWON      "Follow Mode ON"
#define AMSTR_FOLLOWOFF     "Follow Mode OFF"
#define AMSTR_GRIDON        "Grid ON"
#define AMSTR_GRIDOFF       "Grid OFF"
#define AMSTR_MARKEDSPOT    "Marked Spot"
#define AMSTR_MARKSCLEARED  "All Marks Cleared"
#define AMSTR_OVERLAYON     "Overlay Mode ON"
#define AMSTR_OVERLAYOFF    "Overlay Mode OFF"
#define AMSTR_ROTATEON      "Rotate Mode ON"
#define AMSTR_ROTATEOFF     "Rotate Mode OFF"


//
// D_Main.C
//

#define D_DEVSTR    "Development mode ON.\n"
#define D_CDROM     "CD-ROM Version: default.cfg from c:\\doomdata\n"


//
// G_game.C
//

extern char* ggsaved;
extern char* ggloaded;

#define GGSAVED         "game saved."
#define GGLOADED        "game loaded."


//
// M_Menu.C
//

extern char* msgoff;
extern char* msgon;
extern char* detailhi;
extern char* detaillo;
extern char* ststr_alwrun_on;
extern char* ststr_alwrun_off;
extern char* ststr_crosshair_on;
extern char* ststr_crosshair_off;
extern char* ststr_mlook_on;
extern char* ststr_mlook_off;
extern char* ststr_testctrls;
extern char* ststr_scrnsht;

#define STSTR_ALWRUNON  "Always run ON"
#define STSTR_ALWRUNOFF "Always run OFF"
#define STSTR_CROSSHAIR_ON  "Crosshair ON"
#define STSTR_CROSSHAIR_OFF "Crosshair OFF"
#define STSTR_MLOOK_ON  "Mouse look ON"
#define STSTR_MLOOK_OFF "Mouse look OFF"
#define STSTR_TESTCTRLS "Press escape to quit."
#define STSTR_SCRNSHT   "Screen shot"

#define PRESSKEY    "press a key."
#define PRESSYN     "press y or n."
#define DOSY        "(press y to quit.)" // [crispy] remove " to dos.)"

#define QUITMSG     "are you sure you want to\nquit this great game?"
#define LOADNET     "you can't do load while in a net game!\n\n"PRESSKEY
#define QLOADNET    "you can't quickload during a netgame!\n\n"PRESSKEY
#define QSAVESPOT   "you haven't picked a quicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD    "you can't save if you aren't playing!\n\n"PRESSKEY
#define QSPROMPT    "quicksave over your game named\n\n'%s'?\n\n"PRESSYN
#define QLPROMPT    "do you want to quickload the game named\n\n'%s'?\n\n"PRESSYN

#define RD_ON       "on"
#define RD_OFF      "off"

#define RD_DEFAULTS                                 \
"ingame settings will reset to their defaults.\n"   \
"\n"                                                \
"are you sure you want to continue?\n\n"            \
PRESSYN

#define NEWGAME               \
"you can't start a new game\n"\
"while in a network game.\n\n"PRESSKEY

#define NIGHTMARE                 \
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\n"PRESSYN

#define ULTRANM                  \
"monsters aren't resurrecting,\n"\
"but their speed and damage\n"   \
"amplitude are increased.\n\n"   \
"are you sure you can survive?\n\n"PRESSYN

#define SWSTRING                            \
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\n"PRESSKEY

#define MSGOFF  "Messages OFF"
#define MSGON   "Messages ON"
#define NETEND  "you can't end a netgame!\n\n"PRESSKEY
#define ENDGAME "are you sure you want to end the game?\n\n"PRESSYN

#define DETAILHI    "High detail"
#define DETAILLO    "Low detail"

// Improved (PALFIX) and standard (PLAYPAL) gamma correction
#define GAMMA_IMPROVED_OFF  "Improved gamma correction OFF"
#define GAMMA_IMPROVED_05   "Improved gamma correction level 0.5"
#define GAMMA_IMPROVED_1    "Improved gamma correction level 1.0"
#define GAMMA_IMPROVED_15   "Improved gamma correction level 1.5"
#define GAMMA_IMPROVED_2    "Improved gamma correction level 2.0"
#define GAMMA_IMPROVED_25   "Improved gamma correction level 2.5"
#define GAMMA_IMPROVED_3    "Improved gamma correction level 3.0"
#define GAMMA_IMPROVED_35   "Improved gamma correction level 3.5"
#define GAMMA_IMPROVED_4    "Improved gamma correction level 4.0"
#define GAMMA_ORIGINAL_OFF  "Standard gamma correction OFF"
#define GAMMA_ORIGINAL_05   "Standard gamma correction level 0.5"
#define GAMMA_ORIGINAL_1    "Standard gamma correction level 1.0"
#define GAMMA_ORIGINAL_15   "Standard gamma correction level 1.5"
#define GAMMA_ORIGINAL_2    "Standard gamma correction level 2.0"
#define GAMMA_ORIGINAL_25   "Standard gamma correction level 2.5"
#define GAMMA_ORIGINAL_3    "Standard gamma correction level 3.0"
#define GAMMA_ORIGINAL_35   "Standard gamma correction level 3.5"
#define GAMMA_ORIGINAL_4    "Standard gamma correction level 4.0"

#define EMPTYSTRING	"-"


//
// P_Doors.C
//

extern char* pd_blueo;
extern char* pd_redo;
extern char* pd_yellowo;
extern char* pd_bluek;
extern char* pd_redk;
extern char* pd_yellowk;

#define PD_BLUEO    "You need a blue key to activate this object"
#define PD_REDO     "You need a red key to activate this object"
#define PD_YELLOWO  "You need a yellow key to activate this object"
#define PD_BLUEK    "You need a blue key to open this door"
#define PD_REDK     "You need a red key to open this door"
#define PD_YELLOWK  "You need a yellow key to open this door"


//
// P_inter.C
//

// armor
extern char* gotarmor;
extern char* gotmega;

// bonus items
extern char* goththbonus;
extern char* gotarmbonus;
extern char* gotsceptre;
extern char* gotbible;
extern char* gotsuper;
extern char* gotmsphere;

// cards
extern char* gotbluecard;
extern char* gotyelwcard;
extern char* gotredcard;
extern char* gotblueskul;
extern char* gotyelwskul;
extern char* gotredskull;

// medikits, heals
extern char* gotstim;
extern char* gotmedineed;
extern char* gotmedikit;

// power ups
extern char* gotinvul;
extern char* gotberserk;
extern char* gotinvis;
extern char* gotsuit;
extern char* gotmap;
extern char* gotvisor;

// ammo
extern char* gotclip;
extern char* gotclipbox;
extern char* gotrocket;
extern char* gotrocket2;
extern char* gotrockbox;
extern char* gotcell;
extern char* gotcellbox;
extern char* gotshells;
extern char* gotshells8;
extern char* gotshellbox;
extern char* gotbackpack;

// weapons
extern char* gotbfg9000;
extern char* gotchaingun;
extern char* gotchainsaw;
extern char* gotlauncher;
extern char* gotplasma;
extern char* gotshotgun;
extern char* gotshotgun2;

// armor
#define GOTARMOR    "Picked up the armor."
#define GOTMEGA     "Picked up the MegaArmor!"

// bonus items
#define GOTHTHBONUS "Picked up a health bonus."
#define GOTARMBONUS "Picked up an armor bonus."
#define GOTSCEPTRE   "Picked up an evil sceptre."
#define GOTBIBLE     "Picked up an unholy bible."
#define GOTSUPER    "Supercharge!"
#define GOTMSPHERE  "MegaSphere!"
// press beta items
#define GOTDAGGER    "Picked up a demonic dagger."
#define GOTCHEST     "Picked up a skullchest."
#define GOTEXTRALIFE "Extra Life!"

// cards
#define GOTBLUECARD "Picked up a blue keycard."
#define GOTYELWCARD "Picked up a yellow keycard."
#define GOTREDCARD  "Picked up a red keycard."
#define GOTBLUESKUL "Picked up a blue skull key."
#define GOTYELWSKUL "Picked up a yellow skull key."
#define GOTREDSKULL "Picked up a red skull key."

// medikits, heals
#define GOTSTIM     "Picked up a stimpack."
#define GOTMEDINEED "Picked up a medikit that you REALLY need!"
#define GOTMEDIKIT  "Picked up a medikit."

// power ups
#define GOTINVUL    "Invulnerability!"
#define GOTBERSERK  "Berserk!"
#define GOTINVIS    "Partial Invisibility"
#define GOTSUIT     "Radiation Shielding Suit"
#define GOTMAP      "Computer Area Map"
#define GOTVISOR    "Light Amplification Visor"

// ammo
#define GOTCLIP     "Picked up a clip."
#define GOTCLIPBOX  "Picked up a box of bullets."
#define GOTROCKET   "Picked up a rocket."
#define GOTROCKET2  "Picked up a two rockets."
#define GOTROCKBOX  "Picked up a box of rockets."
#define GOTCELL     "Picked up an energy cell."
#define GOTCELLBOX  "Picked up an energy cell pack."
#define GOTSHELLS   "Picked up 4 shotgun shells."
#define GOTSHELLS8  "Picked up 8 shotgun shells."
#define GOTSHELLBOX "Picked up a box of shotgun shells."
#define GOTBACKPACK "Picked up a backpack full of ammo!"

// weapons
#define GOTBFG9000	"You got the BFG9000!  Oh, yes."
#define GOTCHAINGUN	"You got the chaingun!"
#define GOTCHAINSAW	"A chainsaw!  Find some meat!"
#define GOTLAUNCHER	"You got the rocket launcher!"
#define GOTPLASMA	"You got the plasma gun!"
#define GOTSHOTGUN	"You got the shotgun!"
#define GOTSHOTGUN2	"You got the super shotgun!"


//
// P_Saveg.C
//

#define SAVEGAMENAME "doomsav"


//
// P_Spec.C
//

extern char* secretfound;

#define SECRETFOUND "A secret is revealed!"


//
// ST_stuff.C
//

extern char* ststr_mus;
extern char* ststr_nomus;
extern char* ststr_dqdon;
extern char* ststr_dqdoff;

extern char* ststr_kfaadded;
extern char* ststr_faadded;
extern char* ststr_kaadded;

extern char* ststr_ncon;
extern char* ststr_ncoff;

extern char* ststr_choppers;
extern char* ststr_clev;

extern char* ststr_behold;
extern char* ststr_beholdx;
extern char* ststr_beholdz;

extern char* ststr_massacre;
extern char* ststr_version;

#define STSTR_MUS		"Music Change"
#define STSTR_NOMUS		"IMPOSSIBLE SELECTION"
#define STSTR_DQDON		"Degreelessness Mode On"
#define STSTR_DQDOFF	"Degreelessness Mode Off"

#define STSTR_KFAADDED	"Very Happy Ammo Added"
#define STSTR_FAADDED	"Ammo (no keys) Added"
#define STSTR_KAADDED   "Keys Added"

#define STSTR_NCON		"No Clipping Mode ON"
#define STSTR_NCOFF		"No Clipping Mode OFF"

#define STSTR_CHOPPERS	"... doesn't suck - GM"
#define STSTR_CLEV		"Changing Level..."

#define STSTR_BEHOLD	"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define STSTR_BEHOLDX	"Power-up Activated"
#define STSTR_BEHOLDZ   "Power-up Deactivated"

#define STSTR_MASSACRE  "Monsters killed:"


//
// HU_stuff.C
//

#define HUSTR_MSGU	"[Message unsent]"


//
// DOOM 1
//

#define HUSTR_E1M1  "E1M1: Hangar"
#define HUSTR_E1M2  "E1M2: Nuclear Plant"
#define HUSTR_E1M3  "E1M3: Toxin Refinery"
#define HUSTR_E1M4  "E1M4: Command Control"
#define HUSTR_E1M5  "E1M5: Phobos Lab"
#define HUSTR_E1M6  "E1M6: Central Processing"
#define HUSTR_E1M7  "E1M7: Computer Station"
#define HUSTR_E1M8  "E1M8: Phobos Anomaly"
#define HUSTR_E1M9  "E1M9: Military Base"
#define HUSTR_E2M1  "E2M1: Deimos Anomaly"
#define HUSTR_E2M2  "E2M2: Containment Area"
#define HUSTR_E2M3  "E2M3: Refinery"
#define HUSTR_E2M4  "E2M4: Deimos Lab"
#define HUSTR_E2M5  "E2M5: Command Center"
#define HUSTR_E2M6  "E2M6: Halls of the Damned"
#define HUSTR_E2M7  "E2M7: Spawning Vats"
#define HUSTR_E2M8  "E2M8: Tower of Babel"
#define HUSTR_E2M9  "E2M9: Fortress of Mystery"
#define HUSTR_E3M1  "E3M1: Hell Keep"
#define HUSTR_E3M2  "E3M2: Slough of Despair"
#define HUSTR_E3M3  "E3M3: Pandemonium"
#define HUSTR_E3M4  "E3M4: House of Pain"
#define HUSTR_E3M5  "E3M5: Unholy Cathedral"
#define HUSTR_E3M6  "E3M6: Mt. Erebus"
#define HUSTR_E3M7  "E3M7: Limbo"
#define HUSTR_E3M8  "E3M8: Dis"
#define HUSTR_E3M9  "E3M9: Warrens"
#define HUSTR_E4M1  "E4M1: Hell Beneath"
#define HUSTR_E4M2  "E4M2: Perfect Hatred"
#define HUSTR_E4M3  "E4M3: Sever The Wicked"
#define HUSTR_E4M4  "E4M4: Unruly Evil"
#define HUSTR_E4M5  "E4M5: They Will Repent"
#define HUSTR_E4M6  "E4M6: Against Thee Wickedly"
#define HUSTR_E4M7  "E4M7: And Hell Followed"
#define HUSTR_E4M8  "E4M8: Unto The Cruel"
#define HUSTR_E4M9  "E4M9: Fear"
#define HUSTR_E5M1	"E5M1: Baphomet's Demesne"
#define HUSTR_E5M2	"E5M2: Sheol"
#define HUSTR_E5M3	"E5M3: Cages of the Damned"
#define HUSTR_E5M4	"E5M4: Paths of Wretchedness"
#define HUSTR_E5M5	"E5M5: Abaddon's Void"
#define HUSTR_E5M6	"E5M6: Unspeakable Persecution"
#define HUSTR_E5M7	"E5M7: Nightmare Underworld"
#define HUSTR_E5M8	"E5M8: Halls of Perdition"
#define HUSTR_E5M9	"E5M9: Realm of Iblis"


//
// DOOM2: Hell on Earth
//

#define HUSTR_1     "level 1: entryway"
#define HUSTR_2     "level 2: underhalls"
#define HUSTR_3     "level 3: the gantlet"
#define HUSTR_4     "level 4: the focus"
#define HUSTR_5     "level 5: the waste tunnels"
#define HUSTR_6     "level 6: the crusher"
#define HUSTR_7     "level 7: dead simple"
#define HUSTR_8     "level 8: tricks and traps"
#define HUSTR_9     "level 9: the pit"
#define HUSTR_10    "level 10: refueling base"
#define HUSTR_11    "level 11: 'o' of destruction!"
#define HUSTR_12    "level 12: the factory"
#define HUSTR_13    "level 13: downtown"
#define HUSTR_14    "level 14: the inmost dens"
#define HUSTR_15    "level 15: industrial zone"
#define HUSTR_16    "level 16: suburbs"
#define HUSTR_17    "level 17: tenements"
#define HUSTR_18    "level 18: the courtyard"
#define HUSTR_19    "level 19: the citadel"
#define HUSTR_20    "level 20: gotcha!"
#define HUSTR_21    "level 21: nirvana"
#define HUSTR_22    "level 22: the catacombs"
#define HUSTR_23    "level 23: barrels o' fun"
#define HUSTR_24    "level 24: the chasm"
#define HUSTR_25    "level 25: bloodfalls"
#define HUSTR_26    "level 26: the abandoned mines"
#define HUSTR_27    "level 27: monster condo"
#define HUSTR_28    "level 28: the spirit world"
#define HUSTR_29    "level 29: the living end"
#define HUSTR_30    "level 30: icon of sin"
#define HUSTR_31    "level 31: wolfenstein"
#define HUSTR_32    "level 32: grosse"


//
// Final DOOM: The Plutonia Experiment
//

#define PHUSTR_1    "level 1: congo"
#define PHUSTR_2    "level 2: well of souls"
#define PHUSTR_3    "level 3: aztec"
#define PHUSTR_4    "level 4: caged"
#define PHUSTR_5    "level 5: ghost town"
#define PHUSTR_6    "level 6: baron's lair"
#define PHUSTR_7    "level 7: caughtyard"
#define PHUSTR_8    "level 8: realm"
#define PHUSTR_9    "level 9: abattoire"
#define PHUSTR_10   "level 10: onslaught"
#define PHUSTR_11   "level 11: hunted"
#define PHUSTR_12   "level 12: speed"
#define PHUSTR_13   "level 13: the crypt"
#define PHUSTR_14   "level 14: genesis"
#define PHUSTR_15   "level 15: the twilight"
#define PHUSTR_16   "level 16: the omen"
#define PHUSTR_17   "level 17: compound"
#define PHUSTR_18   "level 18: neurosphere"
#define PHUSTR_19   "level 19: nme"
#define PHUSTR_20   "level 20: the death domain"
#define PHUSTR_21   "level 21: slayer"
#define PHUSTR_22   "level 22: impossible mission"
#define PHUSTR_23   "level 23: tombstone"
#define PHUSTR_24   "level 24: the final frontier"
#define PHUSTR_25   "level 25: the temple of darkness"
#define PHUSTR_26   "level 26: bunker"
#define PHUSTR_27   "level 27: anti-christ"
#define PHUSTR_28   "level 28: the sewers"
#define PHUSTR_29   "level 29: odyssey of noises"
#define PHUSTR_30   "level 30: the gateway of hell"
#define PHUSTR_31   "level 31: cyberden"
#define PHUSTR_32   "level 32: go 2 it"


//
// Final DOOM: TNT - Evilution
//

#define THUSTR_1    "level 1: system control"
#define THUSTR_2    "level 2: human bbq"
#define THUSTR_3    "level 3: power control"
#define THUSTR_4    "level 4: wormhole"
#define THUSTR_5    "level 5: hanger"
#define THUSTR_6    "level 6: open season"
#define THUSTR_7    "level 7: prison"
#define THUSTR_8    "level 8: metal"
#define THUSTR_9    "level 9: stronghold"
#define THUSTR_10   "level 10: redemption"
#define THUSTR_11   "level 11: storage facility"
#define THUSTR_12   "level 12: crater"
#define THUSTR_13   "level 13: nukage processing"
#define THUSTR_14   "level 14: steel works"
#define THUSTR_15   "level 15: dead zone"
#define THUSTR_16   "level 16: deepest reaches"
#define THUSTR_17   "level 17: processing area"
#define THUSTR_18   "level 18: mill"
#define THUSTR_19   "level 19: shipping/respawning"
#define THUSTR_20   "level 20: central processing"
#define THUSTR_21   "level 21: administration center"
#define THUSTR_22   "level 22: habitat"
#define THUSTR_23   "level 23: lunar mining project"
#define THUSTR_24   "level 24: quarry"
#define THUSTR_25   "level 25: baron's den"
#define THUSTR_26   "level 26: ballistyx"
#define THUSTR_27   "level 27: mount pain"
#define THUSTR_28   "level 28: heck"
#define THUSTR_29   "level 29: river styx"
#define THUSTR_30   "level 30: last call"
#define THUSTR_31   "level 31: pharaoh"
#define THUSTR_32   "level 32: caribbean"


//
// Chat macros
//

#define HUSTR_CHATMACRO1    "I'm ready to kick butt!"
#define HUSTR_CHATMACRO2    "I'm OK."
#define HUSTR_CHATMACRO3    "I'm not looking too good!"
#define HUSTR_CHATMACRO4    "Help!"
#define HUSTR_CHATMACRO5    "You suck!"
#define HUSTR_CHATMACRO6    "Next time, scumbag..."
#define HUSTR_CHATMACRO7    "Come here!"
#define HUSTR_CHATMACRO8    "I'll take care of it."
#define HUSTR_CHATMACRO9    "Yes"
#define HUSTR_CHATMACRO0    "No"

#define HUSTR_TALKTOSELF1   "You mumble to yourself"
#define HUSTR_TALKTOSELF2   "Who's there?"
#define HUSTR_TALKTOSELF3   "You scare yourself"
#define HUSTR_TALKTOSELF4   "You start to rave"
#define HUSTR_TALKTOSELF5   "You've lost it..."

#define HUSTR_MESSAGESENT   "[Message Sent]"


//
// The following should NOT be changed unless it seems
// just AWFULLY necessary
//

#define HUSTR_PLRGREEN  "Green: "
#define HUSTR_PLRINDIGO "Indigo: "
#define HUSTR_PLRBROWN  "Brown: "
#define HUSTR_PLRRED    "Red: "

// [JN] Do not translate these shortcut keys!
#define HUSTR_KEYGREEN  'g'
#define HUSTR_KEYINDIGO 'i'
#define HUSTR_KEYBROWN  'b'
#define HUSTR_KEYRED    'r'


//
// F_Finale.C
//

#define E1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base.  Looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"

#define E2TEXT \
"You've done it! The hideous cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But ... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to  the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's on to the final chapter of\n"\
"DOOM! -- Inferno."

#define E3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair -- for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you ..."

#define E4TEXT \
"the spider mastermind must have sent forth\n"\
"its legions of hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from hell.  but you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"besides, someone was gonna pay for what\n"\
"happened to daisy, your pet rabbit.\n"\
"\n"\
"but now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"next stop, hell on earth!"

#define E5TEXT \
"Baphomet was only doing Satan's bidding\n"\
"by bringing you back to Hell. Somehow they\n"\
"didn't understand that you're the reason\n"\
"they failed in the first place.\n"\
"\n"\
"After mopping up the place with your\n"\
"arsenal, you're ready to face the more\n"\
"advanced demons that were sent to Earth.\n"\
"\n"\
"\n"\
"Lock and load. Rip and tear."

// after level 6, put this:

#define C1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n" \
"STARPORT. BUT SOMETHING IS WRONG. THE\n" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\n" \
"\n"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" \
"OF THE STARBASE AND FIND THE CONTROLLING\n" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\n" \
"HOSTAGE."

// After level 11, put this:

#define C2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\n"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"\
"THAT YOU HAVE SAVED YOUR SPECIES.\n"\
"\n"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\n"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"\
"UP AND RETURN TO THE FRAY."

// After level 20, put this:

#define C3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"\
"TEETH AND PLUNGE THROUGH IT.\n"\
"\n"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\n"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"\
"GOT TO GO THROUGH HELL TO GET TO IT?"

// After level 29, put this:

#define C4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\n"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"\
"UP AND DIES, ITS THRASHING LIMBS\n"\
"DEVASTATING UNTOLD MILES OF HELL'S\n"\
"SURFACE.\n"\
"\n"\
"YOU'VE DONE IT. THE INVASION IS OVER.\n"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\n"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"\
"HOME. REBUILDING EARTH OUGHT TO BE A\n"\
"LOT MORE FUN THAN RUINING IT WAS.\n"

// Before level 31, put this:

#define C5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."

// Before level 32, put this:

#define C6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"

// after map 06	

#define P1TEXT  \
"You gloat over the steaming carcass of the\n"\
"Guardian.  With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell.  You relax and glance around the\n"\
"room.  Damn!  There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."

// after map 11

#define P2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."

// after map 20

#define P3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive.  Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth.  Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"

// after map 30

#define P4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place.  As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons.  You're done. Hell\n"\
"has gone back to pounding bad dead folks \n"\
"instead of good live ones.  Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up ..."

// before map 31

#define P5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32

#define P6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."

// after map 06	

#define T1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs.   It seems that UAC has\n"\
"once again gulped it down.  With their\n"\
"high turnover, it must be hard for poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays..\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."

// after map 11

#define T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."

// after map 20

#define T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

// after map 30

#define T4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeeze, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."

// before map 31

#define T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."

// before map 32

#define T6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"Maybe someone else can kick Hell's ass\n"\
"next time around. Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a cyberdemon's iron shoe."


//
// F_FINALE.C
//

#define CC_ZOMBIE   "ZOMBIEMAN"
#define CC_SHOTGUN  "SHOTGUN GUY"
#define CC_HEAVY    "HEAVY WEAPON DUDE"
#define CC_IMP      "IMP"
#define CC_DEMON    "DEMON"
#define CC_LOST     "LOST SOUL"
#define CC_CACO     "CACODEMON"
#define CC_HELL     "HELL KNIGHT"
#define CC_BARON    "BARON OF HELL"
#define CC_ARACH    "ARACHNOTRON"
#define CC_PAIN     "PAIN ELEMENTAL"
#define CC_REVEN    "REVENANT"
#define CC_MANCU    "MANCUBUS"
#define CC_ARCH     "ARCH-VILE"
#define CC_SPIDER   "THE SPIDER MASTERMIND"
#define CC_CYBER    "THE CYBERDEMON"
#define CC_HERO     "OUR HERO"


// -----------------------------------------------------------------------------
// Русский язык
// -----------------------------------------------------------------------------


//
// AM_map.C
//

#define AMSTR_FOLLOWON_RUS      "ht;bv cktljdfybz drk.xty"  // Режим следования включен
#define AMSTR_FOLLOWOFF_RUS     "ht;bv cktljdfybz dsrk.xty" // Режим следования выключен
#define AMSTR_GRIDON_RUS        "ctnrf drk.xtyf"            // Сетка включена
#define AMSTR_GRIDOFF_RUS       "ctnrf jnrk.xtyf"           // Сетка выключена
#define AMSTR_MARKEDSPOT_RUS    "jnvtnrf"                   // Отметка
#define AMSTR_MARKSCLEARED_RUS  "jnvtnrb jxbotys"           // Отметки очищены
#define AMSTR_OVERLAYON_RUS     "ht;bv yfkj;tybz drk.xty"   // Режим наложения включен
#define AMSTR_OVERLAYOFF_RUS    "ht;bv yfkj;tybz dsrk.xty"  // Режим наложения выключен
#define AMSTR_ROTATEON_RUS      "ht;bv dhfotybz drk.xty"    // Режим вращения включен
#define AMSTR_ROTATEOFF_RUS     "ht;bv dhfotybz dsrk.xty"   // Режим вращения выключен


//
// D_Main.C
//

#define D_DEVSTR_RUS    "Активирован режим разработчика.\n"
#define D_CDROM_RUS     "Дисковая версия: файлы конфигурации сохранятся в папке c:\\doomdata\n"


//
// G_game.C
//

#define GGSAVED_RUS          "buhf cj[hfytyf>"          // Игра сохранена.
#define GGLOADED_RUS         "buhf pfuhe;tyf>"          // Игра загружена.

// RD specific
#define STSTR_ALWRUNON_RUS  "gjcnjzyysq ,tu drk.xty"    // Постоянный бег включен
#define STSTR_ALWRUNOFF_RUS "gjcnjzyysq ,tu dsrk.xty"   // Постоянный бег выключен
#define STSTR_MLOOK_ON_RUS  "j,pjh vsim. drk.xty"       // Обзор мышью включен
#define STSTR_MLOOK_OFF_RUS "j,pjh vsim. dsrk.xty"      // Обзор мышью выключен
#define STSTR_TESTCTRLS_RUS "ht;bv ghjdthrb eghfdktybz" // Режим проверки управления
#define STSTR_SCRNSHT_RUS   "crhbyijn"                  // Скриншот


//
// M_Menu.C
//

#define STSTR_CROSSHAIR_ON_RUS  "ghbwtk drk.xty"    // Прицел включен
#define STSTR_CROSSHAIR_OFF_RUS "ghbwtk dsrk.xty"   // Прицел выключен

#define PRESSKEY_RUS    "(yf;vbnt k.,e. rkfdbie)"   // (Нажмите любую клавишу)
#define PRESSYN_RUS     "(yf;vbnt ^#^ bkb ^@^)"     // (Нажмите "Y" или "N")
#define DOSY_RUS        "(yf;vbnt ^#^ xnj,s dsqnb)" // (Нажмите "Y" чтобы выйти)

#define QUITMSG_RUS     "ds ltqcndbntkmyj ]jnbnt dsqnb\nbp 'njq pfvtxfntkmyjq buhs?"                // Вы действительно хотите выйти \n из этой замечательной игры?
#define LOADNET_RUS     "ytdjpvj;yj pfuhepbnm cj[hfytyye.\nbuhe d ctntdjq buht!\n\n"PRESSKEY_RUS    // Невозможно загрузить сохраненую \n игру в сетевой игре!
#define QLOADNET_RUS    "ytdjpvj;yj pfuhepbnm ,scnhjt\ncj[hfytybt d ctntdjq buht!\n\n"PRESSKEY_RUS  // Невозможно загрузить быстрое \n сохранение в сетевой игре!
#define QSAVESPOT_RUS   "e dfc yt ds,hfy ckjn ,scnhjuj cj[hfytybz!\n\n"PRESSKEY_RUS                 // У Вас не выбран слот быстрого сохранения!
#define SAVEDEAD_RUS    "ytdjpvj;yj cj[hfybnm buhe< yt yfxfd tt!\n\n"PRESSKEY_RUS                   // Невозможно сохранить игру, не начав её!
#define QSPROMPT_RUS    "gthtpfgbcfnm ,scnhjt cj[hfytybt\n^%s^?\n\n"PRESSKEY_RUS                    // quicksave over your game named\n\n'%s'?\n\n
#define QLPROMPT_RUS    "pfuhepbnm ,scnhjt cj[hfytybt\n^%s^?\n\n"PRESSKEY_RUS                       // do you want to quickload the game named\n'%s'?\n\n

#define RD_ON_RUS       "drk"       // вкл
#define RD_OFF_RUS      "dsrk"      // выкл
#define RD_NEXT_RUS     "lfktt \\"   // далее >
#define RD_PREV_RUS     "/ yfpfl"   // < назад

// Внутриигровые настройки будут
// сброшены на стандартные значения.
//
// Уверены, что хотите продолжить?
#define RD_DEFAULTS_RUS                 \
"dyenhbbuhjdst yfcnhjqrb ,elen\n"       \
"c,hjitys yf cnfylfhnyst pyfxtybz>\n"   \
"\n"                                    \
"edthtys< xnj [jnbnt ghjljk;bnm?\n\n"   \
PRESSYN_RUS

// Невозможно начать новую игру
// при активной сетевой игре.
#define NEWGAME_RUS             \
"ytdjpvj;yj yfxfnm yjde. buhe\n"\
"ghb frnbdyjq ctntdjq buht>\n\n"PRESSKEY_RUS 

// Уверены? этот уровень сложности
// даже не близок к честному.
#define NIGHTMARE_RUS              \
"edthtys? 'njn ehjdtym ckj;yjcnb\n"\
"lf;t yt ,kbpjr r xtcnyjve>\n\n"PRESSYN_RUS 

// Монстры не воскрешаются, однако
// их скорость и степень наносимого 
// ими урона, увеличены.
//
// Уверены, что сможете выжить?
#define ULTRANM_RUS                \
"vjycnhs yt djcrhtif.ncz< jlyfrj\n"\
"b[ crjhjcnm b cntgtym yfyjcbvjuj\n"  \
"bvb ehjyf< edtkbxtys>\n\n"\
"edthtys< xnj cvj;tnt ds;bnm?\n\n"PRESSYN_RUS

// Это демонстрационная версия игры.
// Вам необходимо приобрести всю трилогию.
#define SWSTRING_RUS                   \
"'nj ltvjycnhfwbjyyfz dthcbz buhs>\n\n"\
"dfv ytj,[jlbvj ghbj,htcnb dc. nhbkjub.>\n\n"PRESSKEY_RUS 

#define MSGOFF_RUS  "cjj,otybz dsrk.xtys"                                   // Сообщения выключены
#define MSGON_RUS   "cjj,otybz drk.xtys"                                    // Сообщения включены
#define NETEND_RUS  "ytdjpvj;yj pfrjyxbnm ctntde. buhe!\n\n"PRESSKEY_RUS        // Невозможно закончить сетевую игру!
#define ENDGAME_RUS "ds ltqcndbntkmyj [jnbnt pfrjyxbnm buhe?\n\n"PRESSYN_RUS    // Вы действительно хотите закончить игру?

#define DETAILHI_RUS    "dscjrfz ltnfkbpfwbz"   // Высокая детализация
#define DETAILLO_RUS    "ybprfz ltnfkbpfwbz"    // Низкая детализация

// Улучшенная (PALFIX) и стандартная (PLAYPAL) гамма-коррекция 
#define GAMMA_IMPROVED_OFF_RUS  "ekexityyfz ufvvf-rjhhtrwbz jnrk.xtyf"     // Улучшенная гамма-коррекция отключена
#define GAMMA_IMPROVED_05_RUS   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 0>5"   // Уровень улучшенной гамма-коррекции 0.5
#define GAMMA_IMPROVED_1_RUS    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 1>0"   // Уровень улучшенной гамма-коррекции 1.0
#define GAMMA_IMPROVED_15_RUS   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 1>5"   // Уровень улучшенной гамма-коррекции 1.5
#define GAMMA_IMPROVED_2_RUS    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 2>0"   // Уровень улучшенной гамма-коррекции 2.0
#define GAMMA_IMPROVED_25_RUS   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 2>5"   // Уровень улучшенной гамма-коррекции 2.5
#define GAMMA_IMPROVED_3_RUS    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 3>0"   // Уровень улучшенной гамма-коррекции 3.0
#define GAMMA_IMPROVED_35_RUS   "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 3>5"   // Уровень улучшенной гамма-коррекции 3.5
#define GAMMA_IMPROVED_4_RUS    "ehjdtym ekexityyjq ufvvf-rjhhtrwbb 4>0"   // Уровень улучшенной гамма-коррекции 4.0
#define GAMMA_ORIGINAL_OFF_RUS  "cnfylfhnyfz ufvvf-rjhhtrwbz jnrk.xtyf"    // Стандартная гамма-коррекция отключена
#define GAMMA_ORIGINAL_05_RUS   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 0>5"  // Уровень стандартной гамма-коррекции 0.5
#define GAMMA_ORIGINAL_1_RUS    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 1>0"  // Уровень стандартной гамма-коррекции 1.0
#define GAMMA_ORIGINAL_15_RUS   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 1>5"  // Уровень стандартной гамма-коррекции 1.5
#define GAMMA_ORIGINAL_2_RUS    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 2>0"  // Уровень стандартной гамма-коррекции 2.0
#define GAMMA_ORIGINAL_25_RUS   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 2>5"  // Уровень стандартной гамма-коррекции 2.5
#define GAMMA_ORIGINAL_3_RUS    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 3>0"  // Уровень стандартной гамма-коррекции 3.0
#define GAMMA_ORIGINAL_35_RUS   "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 3>5"  // Уровень стандартной гамма-коррекции 3.5
#define GAMMA_ORIGINAL_4_RUS    "ehjdtym cnfylfhnyjq ufvvf-rjhhtrwbb 4>0"  // Уровень стандартной гамма-коррекции 4.0


//
// P_Doors.C
//

#define PD_BLUEO_RUS    "lkz frnbdfwbb ye;ty cbybq rk.x>"   // Для активации нужен синий ключ.
#define PD_REDO_RUS     "lkz frnbdfwbb ye;ty rhfcysq rk.x>" // Для активации нужен красный ключ.
#define PD_YELLOWO_RUS  "lkz frnbdfwbb ye;ty ;tknsq rk.x>"  // Для активации нужен желтый ключ.
#define PD_BLUEK_RUS    "lkz jnrhsnbz ye;ty cbybq rk.x>"    // Для открытия нужен синий ключ.
#define PD_REDK_RUS     "lkz jnrhsnbz ye;ty rhfcysq rk.x>"  // Для открытия нужен красный ключ.
#define PD_YELLOWK_RUS  "lkz jnrhsnbz ye;ty ;tknsq rk.x>"   // Для открытия нужен жёлтый ключ.


//
// P_inter.C
//

// armor
#define GOTARMOR_RUS    "gjkextyf ,hjyz>"           // Получена броня.
#define GOTMEGA_RUS     "gjkextyf vtuf,hjyz!"       // Получена мегаброня!

// bonus items
#define GOTHTHBONUS_RUS "gjkexty ,jyec pljhjdmz>"       // Получен бонус здоровья.
#define GOTARMBONUS_RUS "gjkexty ,jyec ,hjyb>"          // Получен бонус брони.
#define GOTSCEPTRE_RUS  "gjkexty lmzdjkmcrbq crbgtnh>"  // Получен дьявольский скипетр.
#define GOTBIBLE_RUS    "gjkextyf ytxtcnbdfz ,b,kbz>"   // Получена нечестивая Библия.
#define GOTSUPER_RUS    "cdth[pfhzl!"                   // Сверхзаряд!
#define GOTMSPHERE_RUS  "vtufcathf!"                    // Мегасфера!
// press beta items
#define GOTDAGGER_RUS       "gjkexty ltvjybxtcrbq rby;fk>"      // Получен демонический кинжал.
#define GOTCHEST_RUS        "gjkexty ceyler c xthtgfvb>"        // Получен сундук с черепами.
#define GOTEXTRALIFE_RUS    "gjkextyf ljgjkybntkmyfz ;bpym!"    // Получена дополнительная жизнь!

#define GOTBLUECARD_RUS "gjkextyf cbyzz rk.x-rfhnf>"    // Получена синяя ключ-карта.
#define GOTYELWCARD_RUS "gjkextyf ;tknfz rk.x-rfhnf>"   // Получена желтая ключ-карта
#define GOTREDCARD_RUS  "gjkextyf rhfcyfz rk.x-rfhnf>"  // Получена красная ключ-карта.
#define GOTBLUESKUL_RUS "gjkexty cbybq rk.x-xthtg>"     // Получен синий ключ-череп.
#define GOTYELWSKUL_RUS "gjkexty ;tknsq rk.x-xthtg>"    // Получен желтый ключ-череп.
#define GOTREDSKULL_RUS "gjkexty rhfcysq rk.x-xthtg>"   // Получен красный ключ-череп.

// medikits, heals
#define GOTSTIM_RUS     "gjkexty cnbvekznjh>"                   // Получен стимулятор.
#define GOTMEDINEED_RUS "gjkextyf rhfqyt ytj,[jlbvfz fgntxrf!"  // Получена крайне необходимая аптечка!
#define GOTMEDIKIT_RUS  "gjkextyf fgntxrf>"                     // Получена аптечка.

// power ups
#define GOTINVUL_RUS    "ytezpdbvjcnm!"                 // Неуязвимость!
#define GOTBERSERK_RUS  ",thcthr!"                      // Берсерк!
#define GOTINVIS_RUS    "xfcnbxyfz ytdblbvjcnm>"        // Частичная невидимость.
#define GOTSUIT_RUS	    "rjcn.v hflbfwbjyyjq pfobns>"   // Костюм радиационной защиты.
#define GOTMAP_RUS      "rjvgm.nthyfz rfhnf ehjdyz>"    // Компьютерная карта уровня.
#define GOTVISOR_RUS    "dbpjh ecbktybz jcdtotybz>"     // Визор усиления освещения.

// ammo
#define GOTCLIP_RUS     "gjkextyf j,jqvf>"                          // Получена обойма.
#define GOTCLIPBOX_RUS  "gjkextyf rjhj,rf gfnhjyjd>"                // Получена коробка патронов.
#define GOTROCKET_RUS   "gjkextyf hfrtnf>"                          // Получена ракета.
#define GOTROCKET2_RUS  "gjkextyj ldt hfrtns>"                      // Получено две ракеты.
#define GOTROCKBOX_RUS  "gjkexty zobr hfrtn>"                       // Получен ящик ракет.
#define GOTCELL_RUS     "gjkextyf 'ythuj,fnfhtz>"                   // Получена энергобатарея.
#define GOTCELLBOX_RUS  "gjkexty 'ythujfrrevekznjh>"                // Получен энергоаккумулятор.
#define GOTSHELLS_RUS   "gjkextyj 4 gfnhjyf lkz lhj,jdbrf>"         // Получено 4 патрона для дробовика.
#define GOTSHELLS8_RUS  "gjkextyj 8 gfnhjyjd lkz lhj,jdbrf>"        // Получено 8 патронов для дробовика.
#define GOTSHELLBOX_RUS "gjkextyf rjhj,rf gfnhjyjd lkz lhj,jdbrf>"  // Получена коробка патронов для дробовика.
#define GOTBACKPACK_RUS "gjkexty h.rpfr c ,jtghbgfcfvb!"            // Получен рюкзак с боеприпасами!

// weapons
#define GOTBFG9000_RUS  "gjkextyj &9000! j lf>"             // Получено BFG9000! О да.
#define GOTCHAINGUN_RUS "gjkexty gektvtn!"                  // Получен пулемет!
#define GOTCHAINSAW_RUS ",typjgbkf! yfqlb ytvyjuj vzcf!"    // Бензопила! Найди немного мяса!
#define GOTLAUNCHER_RUS "gjkextyf hfrtnybwf!"               // Получена ракетница!
#define GOTPLASMA_RUS   "gjkextyf gkfpvtyyfz geirf!"        // Получена плазменная пушка!
#define GOTSHOTGUN_RUS  "gjkexty lhj,jdbr!"                 // Получен дробовик!
#define GOTSHOTGUN2_RUS "gjkextyj ldecndjkmyjt he;mt!"      // Получено двуствольное ружье!


//
// P_Spec.C
//

#define SECRETFOUND_RUS "j,yfhe;ty nfqybr!"     // Обнаружен тайник!


//
// ST_stuff.C
//

#define STSTR_MUS_RUS       "cvtyf vepsrb>>>"                           // Смена музыки...
#define STSTR_NOMUS_RUS     "ytrjhhtrnysq ds,jh"                        // Некорректный выбор
#define STSTR_DQDON_RUS     "ytezpdbvjcnm frnbdbhjdfyf"                 // Неуязвимость активирована
#define STSTR_DQDOFF_RUS    "ytezpdbvjcnm ltfrnbdbhjdfyf"               // Неуязвимость деактивирована

#define STSTR_KFAADDED_RUS  ",jtpfgfc gjgjkyty"                         // Боезапас пополнен
#define STSTR_FAADDED_RUS   ",jtpfgfc gjgjkyty (,tp rk.xtq)"            // Боезапас пополнен (без ключей)
#define STSTR_KAADDED_RUS   "rk.xb gjkextys"                            // Ключи получены

#define STSTR_NCON_RUS      "ghj[j;ltybt xthtp cntys frnbdbhjdfyj"      // Прохождение через стены активировано
#define STSTR_NCOFF_RUS     "ghj[j;ltybt xthtp cntys ltfrnbdbhjdfyj"    // Прохождение через стены дереактивировано

#define STSTR_CHOPPERS_RUS  ">>> yt lehcndtyyj - u>v>"                  // ... не дурственно - г.м.
#define STSTR_CLEV_RUS      "cvtyf ehjdyz>>>"                           // Смена уровня...

#define STSTR_BEHOLD_RUS    "v=,tccv>< s=,thc>< i=ytd>< r=rjc>< a=rhn>< l=dbp>" // m=бессм., ы=берс., ш=нев., к=кос., ф=крт., д=виз.
#define STSTR_BEHOLDX_RUS   "ghtlvtn frnbdbhjdfy"                       // Предмет активирован
#define STSTR_BEHOLDZ_RUS   "ghtlvtn ltfrnbdbhjdfy"                     // Предмет деактивирован

#define STSTR_MASSACRE_RUS  "eybxnj;tyj vjycnhjd:"                      // Уничтожено монстров:


//
// HU_stuff.C
//

#define HUSTR_MSGU_RUS  "(cjj,otybt yt jnghfdktyj)" // (Сообщение не отправлено)


//
// DOOM 1
//

#define HUSTR_E1M1_RUS  "t1v1: fyufh"                               // E1M1: Ангар
#define HUSTR_E1M2_RUS  "t1v2: fnjvyfz 'ktrnhjcnfywbz"              // E1M2: Атомная электростанция
#define HUSTR_E1M3_RUS  "t1v3: pfdjl gj gththf,jnrt zlj[bvbrfnjd"   // E1M3: Завод по переработке ядохимикатов
#define HUSTR_E1M4_RUS  "t1v4: rjvfylysq geyrn"                     // E1M4: Командный пункт
#define HUSTR_E1M5_RUS  "t1v5: kf,jhfnjhbz yf aj,jct"               // E1M5: Лаборатория на Фобосе
#define HUSTR_E1M6_RUS  "t1v6: wtynhfkmysq geyrn j,hf,jnrb"         // E1M6: Центральный пункт обработки
#define HUSTR_E1M7_RUS  "t1v7: dsxbckbntkmysq wtynh"                // E1M7: Вычислительный центр
#define HUSTR_E1M8_RUS  "t1v8: fyjvfkbz yf aj,jct"                  // E1M8: Аномалия на Фобосе
#define HUSTR_E1M9_RUS  "t1v9: djtyyfz ,fpf"                        // E1M9: Военная база
#define HUSTR_E2M1_RUS  "t2v1: fyjvfkbz yf ltqvjct"                 // E2M1: Аномалия на Деймосе
#define HUSTR_E2M2_RUS  "t2v2: [hfybkbot"                           // E2M2: Хранилище
#define HUSTR_E2M3_RUS  "t2v3: jxbcnbntkmysq pfdjl"                 // E2M3: Очистительный завод
#define HUSTR_E2M4_RUS  "t2v4: kf,jhfnjhbz yf ltqvjct"              // E2M4: Лаборатория на Деймосе
#define HUSTR_E2M5_RUS  "t2v5: rjvfylysq wtynh"                     // E2M5: Командный центр
#define HUSTR_E2M6_RUS  "t2v6: pfks ghjrkzns["                      // E2M6: Залы проклятых
#define HUSTR_E2M7_RUS  "t2v7: ythtcnbkbot"                         // E2M7: Нерестилище
#define HUSTR_E2M8_RUS  "t2v8: dfdbkjycrfz ,fiyz"                   // E2M8: Вавилонская башня
#define HUSTR_E2M9_RUS  "t2v9: rhtgjcnm nfqy"                       // E2M9: Крепость тайн
#define HUSTR_E3M1_RUS  "t3v1: rhtgjcnm flf"                        // E3M1: Крепость Ада
#define HUSTR_E3M2_RUS  "t3v2: nhzcbyf jnxfzybz"                    // E3M2: Трясина отчаяния
#define HUSTR_E3M3_RUS  "t3v3: gfyltvjybq"                          // E3M3: Пандемоний
#define HUSTR_E3M4_RUS  "t3v4: ljv ,jkb"                            // E3M4: Дом боли
#define HUSTR_E3M5_RUS  "t3v5: ytxtcnbdsq cj,jh"                    // E3M5: Нечестивый собор
#define HUSTR_E3M6_RUS  "t3v6: ujhf 'ht,"                           // E3M6: Гора Эреб
#define HUSTR_E3M7_RUS  "t3v7: dhfnf d kbv,"                        // E3M7: Врата в Лимб
#define HUSTR_E3M8_RUS  "t3v8: lbn"                                 // E3M8: Дит
#define HUSTR_E3M9_RUS  "t3v9: rhjkbxbq cfl"                        // E3M9: Кроличий сад
#define HUSTR_E4M1_RUS  "t4v1: gjl fljv"                            // E4M1: Под Адом
#define HUSTR_E4M2_RUS  "t4v2: bcnbyyfz ytyfdbcnm"                  // E4M2: Истинная ненависть
#define HUSTR_E4M3_RUS  "t4v3: bcnzpfybt ytxtcnbds["                // E4M3: Истязание нечестивых
#define HUSTR_E4M4_RUS  "t4v4: ytelth;bvjt pkj"                     // E4M4: Неудержимое зло
#define HUSTR_E4M5_RUS  "t4v5: jyb gjrf.ncz"                        // E4M5: Они покаются
#define HUSTR_E4M6_RUS  "t4v6: ceghjnbd cb[ ytxtcnbds["             // E4M6: Супротив сих нечестивых
#define HUSTR_E4M7_RUS  "t4v7: b gjcktljdfk fl"                     // E4M7: И последовал Ад
#define HUSTR_E4M8_RUS  "t4v8: lj ,tccthltxbz"                      // E4M8: До бессердечия
#define HUSTR_E4M9_RUS  "t4v9: cnhf["                               // E4M9: Страх
#define HUSTR_E5M1_RUS  "t5v1: dkfltybz ,fajvtnf"                   // E5M1: Владения Бафомета
#define HUSTR_E5M2_RUS  "t5v2: itjk"                                // E5M2: Шеол
#define HUSTR_E5M3_RUS  "t5v3: rktnb ghjrkzns["                     // E5M3: Клети проклятых
#define HUSTR_E5M4_RUS  "t5v4: genb ytcxfcnmz"                      // E5M4: Пути несчастья
#define HUSTR_E5M5_RUS  "t5v5: ,tplyf f,flljyf"                     // E5M5: Бездна Абаддона
#define HUSTR_E5M6_RUS  "t5v6: ytukfcyjt ujytybt"                   // E5M6: Негласное гонение
#define HUSTR_E5M7_RUS  "t5v7: j,bntkm rjivfhf"                     // E5M7: Обитель кошмара
#define HUSTR_E5M8_RUS  "t5v8: pfks gjub,tkb"                       // E5M8: Залы погибели
#define HUSTR_E5M9_RUS  "t5v9: wfhcndj b,kbcf"                      // E5M9: Царство Иблиса


//
// DOOM2: Ад на Земле
//

#define HUSTR_1_RUS     "ehjdtym 1: gfhflysq d[jl"                  // Уровень 1: Парадный вход
#define HUSTR_2_RUS     "ehjdtym 2: gjlptvyst pfks"                 // Уровень 2: Подземные залы
#define HUSTR_3_RUS     "ehjdtym 3: dspjd ,hjity"                   // Уровень 3: Вызов брошен
#define HUSTR_4_RUS     "ehjdtym 4: chtljnjxbt"                     // Уровень 4: Средоточие
#define HUSTR_5_RUS     "ehjdtym 5: cnjxyst neyytkb"                // Уровень 5: Сточные туннели
#define HUSTR_6_RUS     "ehjdtym 6: ghtcc"                          // Уровень 6: Пресс
#define HUSTR_7_RUS     "ehjdtym 7: cvthntkmyj ghjcnjq"             // Уровень 7: Смертельно простой
#define HUSTR_8_RUS     "ehjdtym 8: ekjdrb b kjdeirb"               // Уровень 8: Уловки и ловушки
#define HUSTR_9_RUS     "ehjdtym 9: zvf"                            // Уровень 9: Яма
#define HUSTR_10_RUS    "ehjdtym 10: pfghfdjxyfz ,fpf"              // Уровень 10: Заправочная база
#define HUSTR_11_RUS    "ehjdtym 11: ^j^ hfpheitybz!"               // Уровень 11: "o" разрушения!
#define HUSTR_12_RUS    "ehjdtym 12: af,hbrf"                       // Уровень 12: Фабрика
#define HUSTR_13_RUS    "ehjdtym 13: ltkjdjq hfqjy"                 // Уровень 13: Деловой район
#define HUSTR_14_RUS    "ehjdtym 14: uke,jxfqibt kjujdbof"          // Уровень 14: Глубочайшие логовища
#define HUSTR_15_RUS    "ehjdtym 15: ghjvsiktyyfz pjyf"             // Уровень 15: Промышленная зона
#define HUSTR_16_RUS    "ehjdtym 16: ghbujhjl"                      // Уровень 16: Пригород 
#define HUSTR_17_RUS    "ehjdtym 17: dkfltybz"                      // Уровень 17: Владения 
#define HUSTR_18_RUS    "ehjdtym 18: dyenhtyybq ldjh"               // Уровень 18: Внутренний двор
#define HUSTR_19_RUS    "ehjdtym 19: wbnfltkm"                      // Уровень 19: Цитадель 
#define HUSTR_20_RUS    "ehjdtym 20: gjgfkcz!"                      // Уровень 20: Попался!
#define HUSTR_21_RUS    "ehjdtym 21: ybhdfyf"                       // Уровень 21: Нирвана 
#define HUSTR_22_RUS    "ehjdtym 22: rfnfrjv,s"                     // Уровень 22: Катакомбы 
#define HUSTR_23_RUS    "ehjdtym 23: wtkst ,jxrb dtctkmz"           // Уровень 23: Целые бочки веселья
#define HUSTR_24_RUS    "ehjdtym 24: ghjgfcnm"                      // Уровень 24: Пропасть 
#define HUSTR_25_RUS    "ehjdtym 25: rhjdjgfls"                     // Уровень 25: Кровопады
#define HUSTR_26_RUS    "ehjdtym 26: pf,hjityyst if[ns"             // Уровень 26: Заброшенные шахты
#define HUSTR_27_RUS    "ehjdtym 27: rjyljvbybev vjycnhjd"          // Уровень 27: Кондоминиум монстров
#define HUSTR_28_RUS    "ehjdtym 28: vbh le[jd"                     // Уровень 28: Мир Духов
#define HUSTR_29_RUS    "ehjdtym 29: rjytw dctuj ;bdjuj"            // Уровень 29: Конец всего живого
#define HUSTR_30_RUS    "ehjdtym 30: brjyf uht[f"                   // Уровень 30: Икона греха
#define HUSTR_31_RUS    "ehjdtym 31: djkmatyinfqy"                  // Уровень 31: Вольфенштайн
#define HUSTR_32_RUS    "ehjdtym 32: uhjcct"                        // Уровень 32: Гроссе


//
// Final DOOM: Эксперимент "Плутония"
//

#define PHUSTR_1_RUS    "ehjdtym 1: rjyuj"                  // Уровень 1: Конго
#define PHUSTR_2_RUS    "ehjdtym 2: rjkjltw lei"            // Уровень 2: Колодец душ
#define PHUSTR_3_RUS    "ehjdtym 3: fwntr"                  // Уровень 3: Ацтек
#define PHUSTR_4_RUS    "ehjdtym 4: pfgthnsq d rktnrt"      // Уровень 4: Запертый в клетке
#define PHUSTR_5_RUS    "ehjdtym 5: ujhjl-ghbphfr"          // Уровень 5: Город-призрак
#define PHUSTR_6_RUS    "ehjdtym 6: kjujdj ,fhjyf"          // Уровень 6: Логово Барона
#define PHUSTR_7_RUS    "ehjdtym 7: ldjh-kjdeirf"           // Уровень 7: Двор-ловушка
#define PHUSTR_8_RUS    "ehjdtym 8: wfhcndj"                // Уровень 8: Царство
#define PHUSTR_9_RUS    "ehjdtym 9: f,,fncndj"              // Уровень 9: Аббатство
#define PHUSTR_10_RUS   "ehjdtym 10: yfnbcr"                // Уровень 10: Натиск
#define PHUSTR_11_RUS   "ehjdtym 11: ghtcktletvsq"          // Уровень 11: Преследуемый
#define PHUSTR_12_RUS   "ehjdtym 12: crjhjcnm"              // Уровень 12: Скорость
#define PHUSTR_13_RUS   "ehjdtym 13: crktg"                 // Уровень 13: Склеп
#define PHUSTR_14_RUS   "ehjdtym 14: pfhj;ltybt"            // Уровень 14: Зарождение
#define PHUSTR_15_RUS   "ehjdtym 15: cevthrb"               // Уровень 15: Сумерки
#define PHUSTR_16_RUS   "ehjdtym 16: ghtlpyfvtyjdfybt"      // Уровень 16: Предзнаменование
#define PHUSTR_17_RUS   "ehjdtym 17: rjvgfeyl"              // Уровень 17: Компаунд
#define PHUSTR_18_RUS   "ehjdtym 18: ytqhjcathf"            // Уровень 18: Нейросфера
#define PHUSTR_19_RUS   "ehjdtym 19: d>h>f>u>"              // Уровень 19: В.Р.А.Г.
#define PHUSTR_20_RUS   "ehjdtym 20: j,bntkm cvthnb"        // Уровень 20: Обитель Смерти
#define PHUSTR_21_RUS   "ehjdtym 21: e,bqwf"                // Уровень 21: Убийца
#define PHUSTR_22_RUS   "ehjdtym 22: ytdsgjkybvjt pflfybt"  // Уровень 22: Невыполнимое задание
#define PHUSTR_23_RUS   "ehjdtym 23: yfluhj,bt"             // Уровень 23: Надгробие
#define PHUSTR_24_RUS   "ehjdtym 24: gjcktlybq he,t;"       // Уровень 24: Последний рубеж
#define PHUSTR_25_RUS   "ehjdtym 25: [hfv nmvs"             // Уровень 25: Храм тьмы
#define PHUSTR_26_RUS   "ehjdtym 26: ,eyrth"                // Уровень 26: Бункер
#define PHUSTR_27_RUS   "ehjdtym 27: fynb[hbcn"             // Уровень 27: Антихрист
#define PHUSTR_28_RUS   "ehjdtym 28: rfyfkbpfwbz"           // Уровень 28: Канализация
#define PHUSTR_29_RUS   "ehjdtym 29: jlbcctz ievjd"         // Уровень 29: Одиссея шумов
#define PHUSTR_30_RUS   "ehjdtym 30: dhfnf flf"             // Уровень 30: Врата Ада
#define PHUSTR_31_RUS   "ehjdtym 31: rb,thkjujdj"           // Уровень 31: Киберлогово
#define PHUSTR_32_RUS   "ehjdtym 32: blb r ctve"            // Уровень 32: Иди к сему


//
// Final DOOM: TNT - Дьяволюция
//

#define THUSTR_1_RUS    "ehjdtym 1: wtynh eghfdktybz cbcntvjq"      // Уровень 1: Центр управления системой
#define THUSTR_2_RUS    "ehjdtym 2: ,fh,tr. bp xtkjdtxbys"          // Уровень 2: Барбекю из человечины
#define THUSTR_3_RUS    "ehjdtym 3: wtynh eghfdktybz gbnfybtv"      // Уровень 3: Центр управления питанием
#define THUSTR_4_RUS    "ehjdtym 4: lshf dj dhtvtyb"                // Уровень 4: Дыра во времени
#define THUSTR_5_RUS    "ehjdtym 5: dbctkbwf"                       // Уровень 5: Виселица
#define THUSTR_6_RUS    "ehjdtym 6: jnrhsnsq ctpjy"                 // Уровень 6: Открытый сезон
#define THUSTR_7_RUS    "ehjdtym 7: n.hmvf"                         // Уровень 7: Тюрьма
#define THUSTR_8_RUS    "ehjdtym 8: vtnfkk"                         // Уровень 8: Металл
#define THUSTR_9_RUS    "ehjdtym 9: rhtgjcnm"                       // Уровень 9: Крепость
#define THUSTR_10_RUS   "ehjdtym 10: bcregktybt"                    // Уровень 10: Искупление
#define THUSTR_11_RUS   "ehjdtym 11: crkfl"                         // Уровень 11: Склад
#define THUSTR_12_RUS   "ehjdtym 12: rhfnth"                        // Уровень 12: Кратер
#define THUSTR_13_RUS   "ehjdtym 13: gththf,jnrf zlthys[ jn[jljd"   // Уровень 13: Переработка ядерных отходов
#define THUSTR_14_RUS   "ehjdtym 14: cnfktkbntqysq pfdjl"           // Уровень 14: Cталелитейный завод
#define THUSTR_15_RUS   "ehjdtym 15: vthndfz pjyf"                  // Уровень 15: Мертвая зона
#define THUSTR_16_RUS   "ehjdtym 16: uke,jxfqibt ljcnb;tybz"        // Уровень 16: Глубочайшие достижения
#define THUSTR_17_RUS   "ehjdtym 17: pjyf j,hf,jnrb"                // Уровень 17: Зона обработки
#define THUSTR_18_RUS   "ehjdtym 18: pfdjl"                         // Уровень 18: Завод
#define THUSTR_19_RUS   "ehjdtym 19: gjuheprf*jnghfdrf"             // Уровень 19: Погрузка/отправка
#define THUSTR_20_RUS   "ehjdtym 20: wtynhfkmysq geyrn j,hf,jnrb"   // Уровень 20: Центральный пункт обработки
#define THUSTR_21_RUS   "ehjdtym 21: flvbybcnhfnbdysq wtynh"        // Уровень 21: Административный центр
#define THUSTR_22_RUS   "ehjdtym 22: j,bnfkbot"                     // Уровень 22: Обиталище
#define THUSTR_23_RUS   "ehjdtym 23: keyysq ujhysq ghjtrn"          // Уровень 23: Лунный горный проект
#define THUSTR_24_RUS   "ehjdtym 24: rfhmth"                        // Уровень 24: Карьер
#define THUSTR_25_RUS   "ehjdtym 25: kjujdj ,fhjyf"                 // Уровень 25: Логово Барона
#define THUSTR_26_RUS   "ehjdtym 26: ,fkkbcnbrc"                    // Уровень 26: Баллистикс
#define THUSTR_27_RUS   "ehjdtym 27: ujhf ,jkm"                     // Уровень 27: Гора Боль
#define THUSTR_28_RUS   "ehjdtym 28: xthnjdobyf"                    // Уровень 28: Чертовщина
#define THUSTR_29_RUS   "ehjdtym 29: htrf cnbrc"                    // Уровень 29: Река Стикс
#define THUSTR_30_RUS   "ehjdtym 30: gjcktlybq dspjd"               // Уровень 30: Последний вызов
#define THUSTR_31_RUS   "ehjdtym 31: afhfjy"                        // Уровень 31: Фараон
#define THUSTR_32_RUS   "ehjdtym 32: rfhb,s"                        // Уровень 32: Карибы


//
// Макросы для чата
// 

#define HUSTR_TALKTOSELF1_RUS   "ythfp,jhxbdjt ,jhvjnfybt>>>"   // Неразборчивое бормотание...
#define HUSTR_TALKTOSELF2_RUS   "rnj nfv?"                      // Кто там?
#define HUSTR_TALKTOSELF3_RUS   "xnj 'nj ,skj?"                 // Что это было?
#define HUSTR_TALKTOSELF4_RUS   "ds ,htlbnt>"                   // Вы бредите.
#define HUSTR_TALKTOSELF5_RUS   "rfrfz ljcflf!"                 // Какая досада!


//
// F_Finale.C
//

#define E1TEXT_RUS \
"eybxnj;bd ,fhjyjd flf b pfxbcnbd keyye.\n"\
",fpe< ds ljk;ys ,skb gj,tlbnm< yt nfr kb?\n"\
"yt nfr kb? ult pfcke;tyyfz yfuhflf b\n"\
",bktn ljvjq? xnj 'nj pf xthnjdobyf?\n"\
"nfr yt ljk;yj ,skj dct pfrjyxbnmcz!\n"\
"\n"\
"'nj vtcnj gf[ytn rfr uybkjt vzcj< yj\n"\
"dsukzlbn rfr gjnthzyyfz ,fpf yf ltqvjct>\n"\
"gj[j;t< ds pfcnhzkb d ghb,ht;mt flf< b\n"\
"tlbycndtyysq genm ntgthm - ghjqnb tuj>\n"\
"\n"\
"xnj,s ghjljk;bnm gjuhe;tybt d buhe<\n"\
"ghjqlbnt 'gbpjl ^ghb,ht;mt flf^ b tuj\n"\
"pfvtxfntkmysq cbrdtk ^byathyj^!\n"

// Уничтожив Баронов Ада и зачистив лунную
// базу, Вы должны были победить, не так ли?
// Не так ли? Где заслуженная награда и
// билет домой? Что это за чертовщина?
// Так не должно было все закончиться!
// 
// Это место пахнет как гнилое мясо, но
// выглядит как потерянная база на Деймосе.
// Похоже, вы застряли в "Прибрежье Ада", и
// единственный путь теперь - пройти его.
//
// Чтобы продолжить погружение в игру,
// пройдите эпизод прибрежье ада и его
// замечательный сиквел "Инферно"!

#define E2TEXT_RUS \
"e dfc gjkexbkjcm! e;fcysq kjhl-\n"\
"rb,thltvjy< ghfdzobq gjnthzyyjq\n"\
"ltqvjcjdcrjq ,fpjq< ,sk gjdth;ty\n"\
"b ds njh;tcndetnt! yj>>> ult ds?\n"\
"gjlj,hfdibcm r rhf. cgenybrf< ds\n"\
"j,hfoftnt dpjh dybp< xnj,s\n"\
"edbltnm e;fcye. ghfdle>\n"\
"\n"\
"ltqvjc gksdtn yfl cfvbv fljv!\n"\
"ds ybrjulf yt cksifkb< xnj,s rnj-yb,elm\n"\
"c,tufk bp flf< yj ds pfcnfdbnt e,k.lrjd \n"\
"gj;fktnm j njv< xnj jyb ghjcksifkb j dfc>\n"\
"gjcgtiyj j,dzpfdibcm dthtdrjq< ds\n"\
"cgecrftntcm yf gjdth[yjcnm flf>\n"\
"\n" \
"Ntgthm yfcnegftn dhtvz gjcktlytq ukfds\n"\
"- ^byathyj^>"

// У вас получилось! Ужасный Лорд-
// Кибердемон, правящий потерянной 
// деймосовской базой, был повержен 
// и вы торжествуете! Но... Где вы? 
// Подобравшись к краю спутника, вы
// обращаете взор вниз, чтобы  
// увидеть ужасную правду. 
// 
// Деймос плывёт над самим Адом! 
// Вы никогда не слышали, чтобы кто-нибудь
// сбегал из Ада, но вы заставите ублюдков
// пожалеть о том, что они прослышали о вас.
// Поспешно обвязавшись верёвкой, Вы 
// спускаетесь на поверхность Ада.
//
// Теперь наступает время последней главы 
// - "Инферно". 

#define E3TEXT_RUS \
"jvthpbntkmysq gferjltvjy<\n"\
"herjdjlbdibq dnjh;tybtv yf keyyst\n"\
",fps b ghbytcibq cnjkm vyjuj \n"\
"cvthntq< ,sk jrjyxfntkmyj gjdth;ty>\n"\
"\n"\
"jnrhsdftncz ctrhtnyfz ldthm b ds\n"\
"d[jlbnt d ytt> ds ljrfpfkb< xnj\n"\
"ckbirjv rhens lkz flf< b gj'njve\n"\
"fl< yfrjytw< gjcnegftn cghfdtlkbdj -\n"\
"ds ds[jlbnt bp ldthb ghzvj yf\n"\
"ptktyst gjkz ptvkb> yfrjytw-nj ljv>\n" \
"\n"\
"ds cghfibdftnt ct,z: xnj ghjbc[jlbkj\n"\
"yf ptvkt< gjrf ds chf;fkbcm c \n"\
"dscdj,jlbdibvcz pkjv? [jhjij< xnj yb\n"\
"jlyj gjhj;ltybt flf yt cvjukj ghjqnb \n"\
"xthtp 'ne ldthm dvtcnt c dfvb>>>"

// Омерзительный паукодемон,
// руководивший вторжением на лунные
// базы и принесший столь много
// смертей, был окончательно повержен.
//
// Открывается секретная дверь и Вы
// входите в нее. Вы доказали, что
// слишком круты для ада, и поэтому
// Ад, наконец, поступает справедливо -
// Вы выходите из двери прямо на
// зеленые поля Земли. Наконец-то дом.
//
// Вы спрашиваете себя: что происходило
// на Земле, пока вы сражались с
// высвободившимся злом? Хорошо, что ни
// одно порождение Ада не смогло пройти
// через эту дверь вместе с Вами...

#define E4TEXT_RUS \
"gfer-ghtldjlbntkm< ljk;yj ,snm< gjckfk\n"\
"dgthtl cdjb ktubjys gjhj;ltybq flf\n"\
"gthtl dfibv gjcktlybv chf;tybtv c 'nbv\n"\
"e;fcysv jnhjlmtv> yj ds gjikb lj rjywf\n"\
"b chfpbkb dtxysv ghjrkznmtv b cnhflfybtv\n"\
"'ne jhle nfr< rfr gjcnegbk ,s yfcnjzobq\n"\
"uthjq gthtl kbwjv cvthntkmyjq jgfcyjcnb>\n"\
"\n"\
"rhjvt njuj< rnj-nj ljk;ty ,sk pfgkfnbnm\n"\
"pf nj< xnj ckexbkjcm c ltqpb< dfitq\n"\
"ljvfiytq rhjkmxb[jq>\n"\
"\n"\
"ntgthm ds dblbnt< crjkm vyjuj ,jkb b\n"\
"rhjdb dfv ghjhjxfn gjkxbof ltvjyjd<\n"\
"ytbcnde.ob[ d yfib[ ujhjlf[>\n"\
"\n"\
"cktle.ofz jcnfyjdrf - fl yf ptvkt!"

// Паук-предводитель, должно быть, послал 
// вперёд свои легионы порождений Ада перед
// Вашим последним сражением с этим 
// ужасным отродьем. Но Вы пошли до конца 
// и сразили вечным проклятьем и страданием 
// эту орду так, как поступил бы настоящий 
// герой перед лицом смертельной опасносности.
//
// Кроме того, кто-то должен был заплатить 
// за то, что случилось с Дэйзи, вашей
// домашней крольчихой. 
// 
// Теперь Вы видите, сколь много боли и
// крови Вам пророчат полчища демонов, 
// неиствующих в наших городах.
//
// Следующая остановка - Ад на земле!

#define E5TEXT_RUS \
"gsnfzcm pfnjxbnm dfc d fl< ,fajvtn kbim\n" \
"gjrjhyj bcgjkyzk djk. cfnfys< cdjtuj\n" \
"ujcgjlbyf> b gjkysv rhf[jv j,thyekbcm\n" \
"b[ pfvscks pf cnjkm hjrjde. jib,re>\n" \
"\n" \
"bcnht,bd gjhj;ltybz flf d xthnjuf[\n" \
"ghtbcgjlybb< ds bcgjkytys htibvjcnm.\n" \
"chfpbnmcz c ,jktt vjueotcndtyysv\n" \
"jnhjlmtv ltvjybxtcrb[ cbk< gjckfyysv\n" \
"yf ptvk.>\n\nybrjulf yt clfdfqntcm> hdbnt b vtxbnt!"

// Пытаясь заточить вас в Ад, Бафомет лишь
// покорно исполнял волю Сатаны, своего
// господина. И полным крахом обернулись
// их замыслы за столь роковую ошибку.
//
// Истребив порождения Ада в чертогах
// преисподнии, вы исполнены решимостью
// сразиться с более могущественным
// отродьем демонических сил, посланным
// на Землю.
//
// Никогда не сдавайтесь. Рвите и мечите!

// after level 6, put this:

#define C1TEXT_RUS \
"ds ghjikb duke,m gjhf;tyyjuj rjcvjgjhnf<\n" \
"yj xnj-nj pltcm ytghfdbkmyj> vjycnhs\n" \
"ghbytckb c cj,jq cj,cndtyye. htfkmyjcnm<\n" \
"b nt[yjkjubz rjcvjgjhnf \n" \
"nhfycajhvbhetncz jn b[ ghbcencndbz>\n" \
"\n"\
"dgthtlb ds dblbnt fdfygjcn flf>\n" \
"tckb dfv elfcncz ghj,hfnmcz xthtp ytuj<\n" \
"ds cvj;tnt ghjybryenm d yfctktyysq\n" \
"ltvjyfvb wtynh ,fps b yfqnb eghfdkz.obq\n" \
"dsrk.xfntkm< lth;fobq yfctktybt ptvkb\n" \
"d pfkj;ybrf[>"

// Вы прошли вглубь поражённого космопорта,
// но что-то здесь неправильно. Монстры
// принесли с собой собственную реальность,
// и технология космопорта 
// трансформируется от их присутствия.
//
// Впереди Вы видите аванпост Ада.
// Если Вам удастся пробраться через него,
// Вы сможете проникнуть в населённый 
// демонами центр базы и найти управляющий 
// выключатель, держащий население Земли 
// в заложниках.

// After level 11, put this:

#define C2TEXT_RUS \
"ds gj,tlbkb! dfif gj,tlf gjpdjkbkf\n" \
"xtkjdtxtcnde 'dfrebhjdfnmcz c ptvkb\n"\
"b cgfcnbcm jn rjivfhf> ntgthm ds -\n"\
"tlbycndtyysq xtkjdtr< jcnfdibqcz yf\n"\
"gkfytnt< b k.ljtls-venfyns< [boyst\n"\
"byjgkfytnzyt b pkst le[b - dfib\n"\
"tlbycndtyyst cjctlb> eljdktndjhtyysq\n"\
"ntv< xnj cgfckb cdjq dbl< ds cgjrjqyj\n"\
"lj;blftntcm ytvbyetvjq ub,tkb>\n"\
"\n"\
"yj dcrjht herjdjlcndj ptvkb gthtlftn\n"\
"cjj,otybt c jh,bns: ^ctycjhs j,yfhe;bkb\n"\
"bcnjxybr byjgkfytnyjuj dnjh;tybz> dfif\n"\
"pflfxf - kbrdblbhjdfnm tuj^> byjgkfytnyfz\n"\
",fpf yf[jlbncz d wtynht dfituj ujhjlf<\n"\
"ytlfktrj jn rjcvjgjhnf> vtlktyyj b\n"\
"vexbntkmyj ds djpdhfoftntcm d ,jq>\n"\

// Вы победили! Ваша победа позволила 
// человечеству эвакуироваться с Земли
// и спастись от кошмара. Теперь Вы - 
// единственный человек, оставшийся на 
// планете, и людоеды-мутанты, хищные 
// инопланетяне и злые духи - Ваши
// единственные соседи. Удовлетворённый 
// спасением своего вида, Вы спокойно
// дожидаетесь неминуемой гибели.
//
// Но вскоре руководство Земли передаёт
// сообщение с орбиты: "Сенсоры обнаружили
// источник инопланетного вторжения. Ваша
// задача - ликвидировать его". Инопланетная 
// база находится в центре Вашего города, 
// недалеко от космопорта. Медленно и 
// мучительно вы возрвращаетесь в бой.

// After level 20, put this:

#define C3TEXT_RUS \
"ds yf[jlbntcm d hfpkfuf.obvcz cthlwt\n"\
"ujhjlf< d jrhe;tybb nhegjd cdjb[ dhfujd>\n"\
"ds yt dblbnt ybrfrjuj cgjcj,f eybxnj;bnm\n"\
"gjhnfk yf 'njq cnjhjyt< b gj'njve< cnbcyed\n"\
"pe,s< ghj[jlbnt crdjpm ytuj>\n"\
"\n"\
"Ljk;ty ,snm cgjcj, pfrhsnm gjhnfk\n"\
"yf lheujq cnjhjyt> b rfrjt dfv ltkj lj\n"\
"njuj< xnj ghbltncz ghjqnb xthtp fl< xnj,s\n"\
"lj,hfnmcz lj 'njuj gjhnfkf?"

// Вы находитесь в разлагающемся сердце
// города, в окружении трупов своих врагов. 
// Вы не видите никакого способа уничтожить
// портал на этой стороне, и поэтому, стиснув 
// зубы, проходите сквозь него. 

// Должен быть способ закрыть портал 
// на другой стороне. И какое Вам дело до
// того, что придётся пройти через Ад, чтобы
// добраться до этого портала?

// After level 29, put this:

#define C4TEXT_RUS \
";enxfqibq kbr cfvjuj ,jkmijuj ltvjyf<\n"\
"rjnjhjuj ds rjulf-kb,j dbltkb< heibncz\n"\
"yf dfib[ ukfpf[ gjckt njuj< rfr ds\n"\
"yfrfxfkb hfrtnfvb tuj ytpfobotyysq\n"\
"vjpu> vjycnh eufcftn b ub,ytn<\n"\
"hfpheifz ,tcxbcktyyst vbkb gjdth[yjcnb\n"\
"flf>\n"\
"\n"\
"ds cltkfkb 'nj> dnjh;tyb. rjytw> ptvkz\n"\
"cgfctyf> fl gjdth;ty> ds cghfibdftnt\n"\
"ct,z: relf ntgthm gjckt cvthnb ,elen\n"\
"gjgflfnm gkj[bt k.lb? enthtd gjn\n"\
"cj k,f< ds yfxbyftnt ljkujt gentitcndbt\n"\
"j,hfnyj ljvjq> djccnfyjdktybt ptvkb\n"\
"ljk;yj ,snm ujhfplj ,jktt bynthtcysv\n"\
"pfyznbtv< yt;tkb tt hfpheitybt>"

// Жутчайший лик самого большого демона,
// которого Вы когда-либо видели, рушится
// на Ваших глазах после того, как Вы 
// накачали ракетами его незащищенный
// мозг. Монстр угасает и гибнет, 
// разрушая бесчисленные мили поверхности 
// Ада.
//
// Вы сделали это. Вторжению конец. Земля 
// спасена. Ад повержен. Вы спрашиваете 
// себя: куда теперь после смерти будут
// попадать плохие люди? Утерев пот 
// со лба, Вы начинаете долгое путешествие
// обратно домой. Восстановление Земли 
// должно быть гораздо более интересным
// занятием, нежели её разрушение. 

// Before level 31, put this:

#define C5TEXT_RUS \
"gjplhfdkztv< ds yfikb ctrhtnysq ehjdtym!\n"\
"gj[j;t< jy ,sk gjcnhjty k.lmvb< f yt\n"\
"ltvjyfvb> k.,jgsnyj< rnj ;t yfctkztn\n"\
"'njn eujkjr flf?\n"\

// Поздравляем, вы нашли секретный уровень! 
// Похоже, он был построен людьми, 
// а не демонами. Любопытно, кто же населяет
// этот уголок Ада?

// Before level 32, put this:

#define C6TEXT_RUS \
"gjplhfdkztv< ds yfikb cdth[ctrhtnysq\n"\
"ehjdtym! kexit ,s dfv cnhtvbntkmyj\n"\
"ghjhdfnmcz crdjpm ytuj!"

// Поздравляем, вы нашли сверхсекретный 
// уровень! Лучше бы вам стремительно
// прорваться сквозь него!

// after map 06	

#define P1TEXT_RUS  \
"ds cj pkjhflcndjv cvjnhbnt yf ujhzobq\n"\
"rfhrfc cnhf;f> c tuj cvthnm. ds dshdfkb\n"\
"ecrjhbntkm bp pkjdjyys[ rjuntq flf>\n"\
"hfcckf,bdibcm< ds jrblsdftnt dpukzljv\n"\
"gjvtotybt> ghjrkznmt! nen ljk;ty ,snm\n"\
"[jnz ,s jlby hf,jxbq ghjnjnbg< yj \n"\
"ltvjys< ljk;yj ,snm< pf,hfkb tuj c cj,jq>\n"\
"\n"\
"ds ljk;ys yfqnb ghjnjnbg< byfxt dct dfib\n"\
"ghjikst ecbkbz jrf;encz yfghfcysvb>\n"\
"ghjljk;fqnt ldbufnmcz< ghjljk;fqnt\n"\
"chf;fnmcz< ghjljk;fqnt e,bdfnm>\n"\
"b lf< ghjljk;fqnt ds;bdfnm>"

// Вы со злорадством смотрите на горящий 
// каркас Стража. С его смертью вы вырвали
// Ускоритель из зловонных когтей Ада.
// Расслабившись, Вы окидываете взглядом
// помещение. Проклятье! Тут должен быть
// хотя бы один рабочий прототип, но 
// демоны, должно быть, забрали его с собой.
//
// Вы должны найти прототип, иначе все Ваши
// прошлые усилия окажутся напрасными.
// Продолжайте двигаться, продолжайте
// сражаться, продолжайте убивать. И да,
// продолжайте выживать.

// after map 11

#define P2TEXT_RUS \
"lf;t cvthntkmysq kf,bhbyn fhxdfqkjd yt\n"\
"cvju jcnfyjdbnm dfc> ds lj,hfkbcm lj\n"\
"ghjnjnbgf ecrjhbntkz< rjnjhsq dcrjht\n"\
",sk eybxnj;ty hfp b yfdctulf>\n"\
"\n"\
"eybxnj;tybt - dfif cgtwbfkmyjcnm>"

// Даже смертельный лабиринт арчвайлов не
// смог остановить Вас. Вы добрались до
// прототипа Ускорителя, который вскоре
// был уничтожен раз и навсегда.
//
// Уничтожение - Ваша специальность.

// after map 20

#define P3TEXT_RUS \
"ds ghj,bkb b ghjrjkjnbkb cdjq genm d\n"\
"cthlwt lmzdjkmcrjuj ekmz> yfcnfkj dhtvz\n"\
"lkz vbccbb ^yfqnb b eybxnj;bnm^<\n"\
"j,]trnjv rjnjhjq cnfytn ghbdhfnybr<\n"\
"xmb ytxbcnbdst jnghscrb ybpdthuf.ncz\n"\
"yf ptvk.>\n"\
"\n"\
"lf< jy gkj[jq< yj ds pyftnt rnj tot [e;t!\n"\
"\n"\
"pkj,yj e[vskzzcm< ds ghjdthztnt cdjt\n"\
"cyfhz;tybt< b ujnjdbntcm ghtgjlfnm\n"\
"e,k.lre ytvyjuj flf dfituj\n"\
"cj,cndtyyjuj bpujnjdktybz!"

// Вы пробили и проколотили свой путь в
// сердце дьявольского улья. Настало время 
// для миссии "найти и уничтожить", 
// объектом которой станет Привратник, 
// чьи нечистивые отпрыски низвергаются 
// на Землю. 
//
// Да, он плохой. Но Вы знаете кто ещё хуже!
//
// Злобно ухмыляясь, Вы проверяете своё 
// снаряжение, и готовитесь преподать 
// ублюдку немного Ада Вашего 
// собственного изготовления!

// after map 30

#define P4TEXT_RUS \
"gjckt eybxnj;tybz kbrf ghbdhfnybrf\n"\
"yfxbyf.n ajhvbhjdfnmcz bydthnbhjdfyyst\n"\
"dhfnf< rjnjhst pfcfcsdf.n d ct,z\n"\
"gjcktlybt j,kjvrb ecrjhbntkz b\n"\
"ytcrjkmrb[ jcnfdib[cz ltvjyjd>\n"\
"\n"\
"ujnjdj> fl dthyekcz yf rheub cdjz<\n"\
"gjukfofz kbim uhtiys[ k.ltq< yt;tkb\n"\
"ghfdtlys[>\n"\
"\n"\
"yt pf,elmnt gjghjcbnm dyerjd gjkj;bnm\n"\
"hfrtnybwe d dfi uhj,> Tckb gjckt cvthnb\n"\
"ds gjgfltnt d fl< jyf gjyflj,bncz\n"\
"lkz yt,jkmijq gjcktlytq pfxbcnrb>>>"

// После уничтожения лика Привратника
// начинают формироваться инвертированные 
// Врата, которые затягивают в себя 
// последние обломки Ускорителя и 
// нескольких оставшихся демонов.
//
// Готово. Ад вернулся на круги своя,
// поглащая лишь грешных людей, нежели 
// праведных.
//
// Не забудьте попросить внуков положить
// ракетницу в Ваш гроб. Если после смерти
// Вы попадёте в Ад, она понадобится
// для небольшой последней зачистки...

// before map 31

#define P5TEXT_RUS \
"ds yfikb dnjhjq gj ckj;yjcnb ehjdtym<\n"\
"rjnjhsq e yfc tcnm> yflttvcz< ds\n"\
"cj[hfybkb buhe yf ghtlsleotv ehjdyt\n"\
"bkb tot hfymit> tckb ytn< ghbujnjdmntcm\n"\
"vyjuj evbhfnm>\n"\
"\n"\
"ehjdtym hfccxbnfy bcrk.xbntkmyj yf\n"\
"ghjatccbjyfkjd>"

// Вы нашли второй по сложности уровень,
// который у нас есть. Надеемся, Вы 
// сохранили игру на предыдущем уровне 
// или ещё раньше. Если нет, приготовьтесь 
// много умирать.
//
// Уровень раcсчитан исключительно на 
// профессионалов.

// before map 32

#define P6TEXT_RUS \
"cgjhbv< ds elbdkzkbcm< rfrjq ;t ehjdtym\n"\
"cfvsq ckj;ysq? ntgthm ds pyftnt>\n"\
"ybrnj yt ds,thtncz ;bdsv>"

// Спорим, вы удивлялись, какой же уровень
// САМЫЙ сложный? Теперь Вы знаете.
// Никто не выберется живым.

// after map 06	

#define T1TEXT_RUS \
"chf;fzcm< ds ds,hfkbcm bp pfhf;tyys[\n"\
"'rcgthtvtynfkmys[ kf,jhfnjhbq>\n"\
"gj[j;t< jfr hfcnhfy;bhbkf b b[< ytcvjnhz\n"\
"yf juhjvyst rjhgjhfnbdyst lj[jls> jyb\n"\
"lf;t yt gjpf,jnbkbcm j gjregrt cnhf[jdjr\n"\
"lkz cdjb[ cjnhelybrjd>>>\n"\
"\n"\
"dgthtlb hfcgjkj;ty djtyysq rjvgktrc<\n"\
"rbifobq dbhecfvb b njkmrj ;leobq\n"\
"rfr ,s duhspnmcz d dfie gkjnm> xnj ;<\n"\
"tckb gjdtptn< d rjvgktrct dct tot\n"\
"ljk;ys jcnfdfnmcz ,jtghbgfcs>"

// Сражаясь, Вы выбрались из заражённых
// эксперементальных лабораторий.
// Похоже, ОАК растранжирила и их, несмотря
// на огромные корпоративные доходы. Они
// даже не позаботились о покупке страховок
// для своих сотрудников...
//
// Впереди расположен военный комплекс, 
// кишащий вирусами и только ждущий 
// как бы вгрызться в Вашу плоть. Что ж,
// если повезёт, в комплексе всё ещё 
// должны оставаться боеприпасы.

// after map 11

#define T2TEXT_RUS \
"dgthtlb cksity vtnfkkbxtcrbq crht;tn\n"\
"nz;tks[ vt[fybpvjd> ds edthtys< xnj jyb\n"\
"yt infvge.n jxthtlye. gfhnb.\n"\
"lmzdjkmcrb[ jnhjlbq< yj lf;t tckb 'nj\n"\
"b nfr< ds r 'njve ujnjds>\n"\
"\n"\
"'nb pderb vjuen jpyfxfnm jxthtlyjq\n"\
"rhjdfdsq atcnbdfkm< yj ds-nj e;t pyftnt<\n"\
"rfrjuj 'nj joeofnm nsczxb ujkjdjhtpjd<\n"\
"cj,hfyys[ d jlyjv ,tpevyjv e,bqwt>\n"\
"\n"\
"ds nfr ghjcnj yt clflbntcm>"

// Впереди слышен металлический скрежет 
// тяжёлых механизмов. Вы уверены, что они
// не штампуют очередную партию 
// дьявольских отродий, но даже если
// и так, Вы к этому готовы. 
// Эти звуки могут означать очередной 
// кровавый фестиваль, но вы-то уже знаете, 
// какого это ощущать тысячи головорезов, 
// собранных в одном безумном убийце.
//
// Вы так просто не сдадитесь.

#define T3TEXT_RUS \
"jnrhsdf.ofzcz gthcgtrnbdf dsukzlbn\n"\
"xthnjdcrb pyfrjvjq b gf[ytn \n"\
"ckjdyj pf;fhtyyst 'rcrhtvtyns>\n"\
"'nj vtcnj yt yhfdbkjcm dfv hfymit<\n"\
"b ds xthnjdcrb edthtys< xnj yt\n"\
"gjyhfdbncz b ctqxfc> xtv ,jkmit ds\n"\
"hfpvsikztnt yfl 'nbv< ntv gtxfkmytt\n"\
"dct cnfyjdbncz>\n"\
"\n"\
"dpdtcbd cdjt jhe;bt< ds pkjdtot\n"\
"e[vskztntcm> yfcnfkj dhtvz dcthmtp\n"\
"yflhfnm rjt-rjve pflybwe>"

// Открывающаяся перспектива выглядит 
// чертовски знакомой и пахнет 
// словно зажаренные экскременты. 
// Это место не нравилось вам раньше, 
// и Вы чертовски уверены, что не 
// понравится и сейчас. Чем больше Вы 
// размышляете над этим, тем печальнее 
// всё становится.
// 
// Взвесив своё оружие, Вы зловеще 
// ухмыляетесь. Настало время всерьез 
// надрать кое-кому задницу.

#define T4TEXT_RUS \
"dytpfgyj dct jrenfkjcm nbibyjq lj\n"\
"cfvjuj ujhbpjynf> fujybpbhe.ott flcrjt\n"\
"'[j cnb[kj< rjivfhyjt yt,j cnfkj dyjdm\n"\
"ujke,sv< nhegs vjycnhjd yfxfkb\n"\
"hfpkfufnmcz cj pkjdjyysv cvhfljv>\n"\
",j;t< yte;tkb ds b ghfdlf gj,tlbkb?\n"\
"\n"\
"gjckt rjhjnrjuj ptvktnhzctybz< bp\n"\
"hfpuhjvktyyjuj xthtgf bphsufntkz\n"\
"ltvjyjd yfxbyftn ghj,bdfnmcz zhrjt\n"\
"cbytt cdtxtybt>"

// Внезапно всё окуталось тишиной до
// самого горизона. Агонизирующее Адское
// эхо стихло, кошмарное небо стало вновь
// голубым, трупы монстров начали 
// разлагаться со зловонным смрадом.
// Боже, неужели Вы и правда победили?
//
// После короткого землетрясения, из
// разгромленного черепа Изрыгателя 
// Демонов начинает пробиваться яркое
// синее свечение.

#define T5TEXT_RUS \
"b xnj ntgthm? dct dsukzlbn cjdthityyj\n"\
"gj-lheujve< ckjdyj gjrjb dkflsrb\n"\
"nenfy[jvjyf>\n"\
"\n"\
"xnj ;< xnj ,s yt j;blfkj dfc dyenhb<\n"\
"[e;t e;t ,snm yt vj;tn< yt nfr kb?\n"\
"bkb vj;tn kexit yt nhtdj;bnm\n"\
"cgzob[ ,jujd>>>"

// И что теперь? Всё выглядит совершенно
// по-другому, словно покои владыки
// Тутанхамона.
//
// Что ж, что бы не ожидало Вас внутри,
// хуже уже быть не может, не так ли?
// Или может лучше не тревожить 
// спящих богов...

#define T6TEXT_RUS \
"ghbikj dhtvz lkz jngecrf> tq-,jue<\n"\
"djhjif ytlhf flf< ds njkmrj j ytv\n"\
"b vtxnfkb! Gecnm rnj-yb,elm lheujq\n"\
"ntgthm ,jhtncz c gjkxbofvb ltvjyjd>\n"\
"\n"\
"dgthtlb hfcgjkj;ty nb[bq ujhjljr c\n"\
"ytcgtiyj ntreotq djljq< ghbxelkbdsvb\n"\
"ljvbrfvb< b< dthjznyj< yt yfctktyysq\n"\
"flcrbv jnhjlmtv>\n"\
"\n"\
"gjrbyed nhfycgjhnyjt chtlcndj<\n"\
"ds cksibnt njgjn ;tktpyjuj rjgsnf\n"\
"rb,thltvjyf>"

// Пришло время для отпуска. Ей-богу, 
// вороша недра Ада, Вы только о нем
// и мечтали! Пусть кто-нибудь другой 
// теперь борется с полчищами демонов.
//
// Впереди располагается тихий городок с
// неспешно текущей водой, причудливыми
// домиками, и, вероятно, не населенный
// адским отродьем.
//
// Покинув транспортное средство,
// Вы слышите топот железного копыта 
// кибердемона.


//
// F_FINALE.C
//

#define CC_ZOMBIE_RUS   "pjv,b"                 // Зомби
#define CC_SHOTGUN_RUS  "pjv,b-cth;fyn"         // Зомби-сержант
#define CC_HEAVY_RUS    "gektvtnxbr"            // Пулемётчик
#define CC_IMP_RUS      ",tc"                   // Бес
#define CC_DEMON_RUS    "ltvjy"                 // Демон
#define CC_LOST_RUS     "gjnthzyyfz leif"       // Потерянная душа
#define CC_CACO_RUS     "rfrjltvjy"             // Какодемон
#define CC_HELL_RUS     "hswfhm flf"            // Рыцарь ада
#define CC_BARON_RUS    ",fhjy flf"             // Барон ада
#define CC_ARACH_RUS    "fhf[yjnhjy"            // Арахнотрон
#define CC_PAIN_RUS     "'ktvtynfkm ,jkb"       // Элементаль боли
#define CC_REVEN_RUS    "htdtyfyn"              // Ревенант
#define CC_MANCU_RUS    "vfyre,ec"              // Манкубус
#define CC_ARCH_RUS     "fhxdfqk"               // Арчвайл
#define CC_SPIDER_RUS   "gfer-ghtldjlbntkm"     // Паук-предводитель
#define CC_CYBER_RUS    "rb,thltvjy"            // Кибердемон
#define CC_HERO_RUS     "yfi uthjq"             // Наш Герой


//
// Отображение версии проекта
//

#define STSTR_VERSION       "VERSION 1.9 (YYYY-MM-DD)"
#define STSTR_VERSION_RUS   "DTHCBZ 1>9 (UUUU-VV-LL)"


#endif

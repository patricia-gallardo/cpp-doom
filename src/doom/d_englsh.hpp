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
//	Printed strings for translation.
//	English language support (default).
//

#pragma once

//
//	Printed strings for translation
//

//
// D_Main.C
//
constexpr auto D_DEVSTR = "Development mode ON.\n";
constexpr auto D_CDROM  = "CD-ROM Version: default.cfg from c:\\doomdata\n";

//
//	M_Menu.C
//
constexpr auto QUITMSG   = "are you sure you want to\nquit this great game?";
constexpr auto LOADNET   = "you can't do load while in a net game!\n\n"
                           "press a key.";
constexpr auto QLOADNET  = "you can't quickload during a netgame!\n\n"
                           "press a key.";
constexpr auto QSAVESPOT = "you haven't picked a quicksave slot yet!\n\n"
                           "press a key.";
constexpr auto SAVEDEAD  = "you can't save if you aren't playing!\n\n"
                           "press a key.";
constexpr auto QSPROMPT  = "quicksave over your game named\n\n'%s'?\n\n"
                           "press y or n.";
constexpr auto QLPROMPT  = "do you want to quickload the game named\n\n'%s'?\n\n"
                           "press y or n.";

constexpr auto NEWGAME =
    "you can't start a new game\n"
    "while in a network game.\n\n"
    "press a key.";

constexpr auto NIGHTMARE =
    "are you sure? this skill level\n"
    "isn't even remotely fair.\n\n"
    "press y or n.";

constexpr auto SWSTRING =
    "this is the shareware version of doom.\n\n"
    "you need to order the entire trilogy.\n\n"
    "press a key.";

constexpr auto MSGOFF  = "Messages OFF";
constexpr auto MSGON   = "Messages ON";
constexpr auto NETEND  = "you can't end a netgame!\n\n"
                         "press a key.";
constexpr auto ENDGAME = "are you sure you want to end the game?\n\n"
                         "press y or n.";

constexpr auto DETAILHI  = "High detail";
constexpr auto DETAILLO  = "Low detail";
constexpr auto GAMMALVL0 = "Gamma correction OFF";
constexpr auto GAMMALVL1 = "Gamma correction level 1";
constexpr auto GAMMALVL2 = "Gamma correction level 2";
constexpr auto GAMMALVL3 = "Gamma correction level 3";
constexpr auto GAMMALVL4 = "Gamma correction level 4";
// [crispy] intermediate gamma levels
constexpr auto GAMMALVL05  = "Gamma correction level 0.5";
constexpr auto GAMMALVL15  = "Gamma correction level 1.5";
constexpr auto GAMMALVL25  = "Gamma correction level 2.5";
constexpr auto GAMMALVL35  = "Gamma correction level 3.5";
constexpr auto EMPTYSTRING = "empty slot";

//
//	P_inter.C
//
constexpr auto GOTARMOR    = "Picked up the armor.";
constexpr auto GOTMEGA     = "Picked up the MegaArmor!";
constexpr auto GOTHTHBONUS = "Picked up a health bonus.";
constexpr auto GOTARMBONUS = "Picked up an armor bonus.";
constexpr auto GOTSTIM     = "Picked up a stimpack.";
constexpr auto GOTMEDINEED = "Picked up a medikit that you REALLY need!";
constexpr auto GOTMEDIKIT  = "Picked up a medikit.";
constexpr auto GOTSUPER    = "Supercharge!";

constexpr auto GOTBLUECARD = "Picked up a blue keycard.";
constexpr auto GOTYELWCARD = "Picked up a yellow keycard.";
constexpr auto GOTREDCARD  = "Picked up a red keycard.";
constexpr auto GOTBLUESKUL = "Picked up a blue skull key.";
constexpr auto GOTYELWSKUL = "Picked up a yellow skull key.";
constexpr auto GOTREDSKULL = "Picked up a red skull key.";

constexpr auto GOTINVUL   = "Invulnerability!";
constexpr auto GOTBERSERK = "Berserk!";
constexpr auto GOTINVIS   = "Partial Invisibility";
constexpr auto GOTSUIT    = "Radiation Shielding Suit";
constexpr auto GOTMAP     = "Computer Area Map";
constexpr auto GOTVISOR   = "Light Amplification Visor";
constexpr auto GOTMSPHERE = "MegaSphere!";

constexpr auto GOTCLIP     = "Picked up a clip.";
constexpr auto GOTCLIPBOX  = "Picked up a box of bullets.";
constexpr auto GOTROCKET   = "Picked up a rocket.";
constexpr auto GOTROCKBOX  = "Picked up a box of rockets.";
constexpr auto GOTCELL     = "Picked up an energy cell.";
constexpr auto GOTCELLBOX  = "Picked up an energy cell pack.";
constexpr auto GOTSHELLS   = "Picked up 4 shotgun shells.";
constexpr auto GOTSHELLBOX = "Picked up a box of shotgun shells.";
constexpr auto GOTBACKPACK = "Picked up a backpack full of ammo!";

constexpr auto GOTBFG9000  = "You got the BFG9000!  Oh, yes.";
constexpr auto GOTCHAINGUN = "You got the chaingun!";
constexpr auto GOTCHAINSAW = "A chainsaw!  Find some meat!";
constexpr auto GOTLAUNCHER = "You got the rocket launcher!";
constexpr auto GOTPLASMA   = "You got the plasma gun!";
constexpr auto GOTSHOTGUN  = "You got the shotgun!";
constexpr auto GOTSHOTGUN2 = "You got the super shotgun!";

//
// P_Doors.C
//
constexpr auto PD_BLUEO   = "You need a blue key to activate this object";
constexpr auto PD_REDO    = "You need a red key to activate this object";
constexpr auto PD_YELLOWO = "You need a yellow key to activate this object";
constexpr auto PD_BLUEK   = "You need a blue key to open this door";
constexpr auto PD_REDK    = "You need a red key to open this door";
constexpr auto PD_YELLOWK = "You need a yellow key to open this door";

//
//	G_game.C
//
constexpr auto GGSAVED = "game saved.";

//
//	HU_stuff.C
//
constexpr auto HUSTR_MSGU = "[Message unsent]";

constexpr auto HUSTR_E1M1  = "E1M1: Hangar";
constexpr auto HUSTR_E1M2  = "E1M2: Nuclear Plant";
constexpr auto HUSTR_E1M3  = "E1M3: Toxin Refinery";
constexpr auto HUSTR_E1M4  = "E1M4: Command Control";
constexpr auto HUSTR_E1M5  = "E1M5: Phobos Lab";
constexpr auto HUSTR_E1M6  = "E1M6: Central Processing";
constexpr auto HUSTR_E1M7  = "E1M7: Computer Station";
constexpr auto HUSTR_E1M8  = "E1M8: Phobos Anomaly";
constexpr auto HUSTR_E1M9  = "E1M9: Military Base";
constexpr auto HUSTR_E1M10 = "E1M10: Sewers";
constexpr auto HUSTR_E1M4B = "E1M4B: Phobos Mission Control";
constexpr auto HUSTR_E1M8B = "E1M8B: Tech Gone Bad";

constexpr auto HUSTR_E2M1 = "E2M1: Deimos Anomaly";
constexpr auto HUSTR_E2M2 = "E2M2: Containment Area";
constexpr auto HUSTR_E2M3 = "E2M3: Refinery";
constexpr auto HUSTR_E2M4 = "E2M4: Deimos Lab";
constexpr auto HUSTR_E2M5 = "E2M5: Command Center";
constexpr auto HUSTR_E2M6 = "E2M6: Halls of the Damned";
constexpr auto HUSTR_E2M7 = "E2M7: Spawning Vats";
constexpr auto HUSTR_E2M8 = "E2M8: Tower of Babel";
constexpr auto HUSTR_E2M9 = "E2M9: Fortress of Mystery";

constexpr auto HUSTR_E3M1 = "E3M1: Hell Keep";
constexpr auto HUSTR_E3M2 = "E3M2: Slough of Despair";
constexpr auto HUSTR_E3M3 = "E3M3: Pandemonium";
constexpr auto HUSTR_E3M4 = "E3M4: House of Pain";
constexpr auto HUSTR_E3M5 = "E3M5: Unholy Cathedral";
constexpr auto HUSTR_E3M6 = "E3M6: Mt. Erebus";
constexpr auto HUSTR_E3M7 = "E3M7: Limbo";
constexpr auto HUSTR_E3M8 = "E3M8: Dis";
constexpr auto HUSTR_E3M9 = "E3M9: Warrens";

constexpr auto HUSTR_E4M1 = "E4M1: Hell Beneath";
constexpr auto HUSTR_E4M2 = "E4M2: Perfect Hatred";
constexpr auto HUSTR_E4M3 = "E4M3: Sever The Wicked";
constexpr auto HUSTR_E4M4 = "E4M4: Unruly Evil";
constexpr auto HUSTR_E4M5 = "E4M5: They Will Repent";
constexpr auto HUSTR_E4M6 = "E4M6: Against Thee Wickedly";
constexpr auto HUSTR_E4M7 = "E4M7: And Hell Followed";
constexpr auto HUSTR_E4M8 = "E4M8: Unto The Cruel";
constexpr auto HUSTR_E4M9 = "E4M9: Fear";

constexpr auto HUSTR_E5M1 = "E5M1: Baphomet's Demesne";
constexpr auto HUSTR_E5M2 = "E5M2: Sheol";
constexpr auto HUSTR_E5M3 = "E5M3: Cages of the Damned";
constexpr auto HUSTR_E5M4 = "E5M4: Paths of Wretchedness";
constexpr auto HUSTR_E5M5 = "E5M5: Abaddon's Void";
constexpr auto HUSTR_E5M6 = "E5M6: Unspeakable Persecution";
constexpr auto HUSTR_E5M7 = "E5M7: Nightmare Underworld";
constexpr auto HUSTR_E5M8 = "E5M8: Halls of Perdition";
constexpr auto HUSTR_E5M9 = "E5M9: Realm of Iblis";

constexpr auto HUSTR_1  = "level 1: entryway";
constexpr auto HUSTR_2  = "level 2: underhalls";
constexpr auto HUSTR_3  = "level 3: the gantlet";
constexpr auto HUSTR_4  = "level 4: the focus";
constexpr auto HUSTR_5  = "level 5: the waste tunnels";
constexpr auto HUSTR_6  = "level 6: the crusher";
constexpr auto HUSTR_7  = "level 7: dead simple";
constexpr auto HUSTR_8  = "level 8: tricks and traps";
constexpr auto HUSTR_9  = "level 9: the pit";
constexpr auto HUSTR_10 = "level 10: refueling base";
constexpr auto HUSTR_11 = "level 11: 'o' of destruction!";

constexpr auto HUSTR_12 = "level 12: the factory";
constexpr auto HUSTR_13 = "level 13: downtown";
constexpr auto HUSTR_14 = "level 14: the inmost dens";
constexpr auto HUSTR_15 = "level 15: industrial zone";
constexpr auto HUSTR_16 = "level 16: suburbs";
constexpr auto HUSTR_17 = "level 17: tenements";
constexpr auto HUSTR_18 = "level 18: the courtyard";
constexpr auto HUSTR_19 = "level 19: the citadel";
constexpr auto HUSTR_20 = "level 20: gotcha!";

constexpr auto HUSTR_21 = "level 21: nirvana";
constexpr auto HUSTR_22 = "level 22: the catacombs";
constexpr auto HUSTR_23 = "level 23: barrels o' fun";
constexpr auto HUSTR_24 = "level 24: the chasm";
constexpr auto HUSTR_25 = "level 25: bloodfalls";
constexpr auto HUSTR_26 = "level 26: the abandoned mines";
constexpr auto HUSTR_27 = "level 27: monster condo";
constexpr auto HUSTR_28 = "level 28: the spirit world";
constexpr auto HUSTR_29 = "level 29: the living end";
constexpr auto HUSTR_30 = "level 30: icon of sin";

constexpr auto HUSTR_31 = "level 31: wolfenstein";
constexpr auto HUSTR_32 = "level 32: grosse";

constexpr auto PHUSTR_1  = "level 1: congo";
constexpr auto PHUSTR_2  = "level 2: well of souls";
constexpr auto PHUSTR_3  = "level 3: aztec";
constexpr auto PHUSTR_4  = "level 4: caged";
constexpr auto PHUSTR_5  = "level 5: ghost town";
constexpr auto PHUSTR_6  = "level 6: baron's lair";
constexpr auto PHUSTR_7  = "level 7: caughtyard";
constexpr auto PHUSTR_8  = "level 8: realm";
constexpr auto PHUSTR_9  = "level 9: abattoire";
constexpr auto PHUSTR_10 = "level 10: onslaught";
constexpr auto PHUSTR_11 = "level 11: hunted";

constexpr auto PHUSTR_12 = "level 12: speed";
constexpr auto PHUSTR_13 = "level 13: the crypt";
constexpr auto PHUSTR_14 = "level 14: genesis";
constexpr auto PHUSTR_15 = "level 15: the twilight";
constexpr auto PHUSTR_16 = "level 16: the omen";
constexpr auto PHUSTR_17 = "level 17: compound";
constexpr auto PHUSTR_18 = "level 18: neurosphere";
constexpr auto PHUSTR_19 = "level 19: nme";
constexpr auto PHUSTR_20 = "level 20: the death domain";

constexpr auto PHUSTR_21 = "level 21: slayer";
constexpr auto PHUSTR_22 = "level 22: impossible mission";
constexpr auto PHUSTR_23 = "level 23: tombstone";
constexpr auto PHUSTR_24 = "level 24: the final frontier";
constexpr auto PHUSTR_25 = "level 25: the temple of darkness";
constexpr auto PHUSTR_26 = "level 26: bunker";
constexpr auto PHUSTR_27 = "level 27: anti-christ";
constexpr auto PHUSTR_28 = "level 28: the sewers";
constexpr auto PHUSTR_29 = "level 29: odyssey of noises";
constexpr auto PHUSTR_30 = "level 30: the gateway of hell";

constexpr auto PHUSTR_31 = "level 31: cyberden";
constexpr auto PHUSTR_32 = "level 32: go 2 it";

constexpr auto THUSTR_1  = "level 1: system control";
constexpr auto THUSTR_2  = "level 2: human bbq";
constexpr auto THUSTR_3  = "level 3: power control";
constexpr auto THUSTR_4  = "level 4: wormhole";
constexpr auto THUSTR_5  = "level 5: hanger";
constexpr auto THUSTR_6  = "level 6: open season";
constexpr auto THUSTR_7  = "level 7: prison";
constexpr auto THUSTR_8  = "level 8: metal";
constexpr auto THUSTR_9  = "level 9: stronghold";
constexpr auto THUSTR_10 = "level 10: redemption";
constexpr auto THUSTR_11 = "level 11: storage facility";

constexpr auto THUSTR_12 = "level 12: crater";
constexpr auto THUSTR_13 = "level 13: nukage processing";
constexpr auto THUSTR_14 = "level 14: steel works";
constexpr auto THUSTR_15 = "level 15: dead zone";
constexpr auto THUSTR_16 = "level 16: deepest reaches";
constexpr auto THUSTR_17 = "level 17: processing area";
constexpr auto THUSTR_18 = "level 18: mill";
constexpr auto THUSTR_19 = "level 19: shipping/respawning";
constexpr auto THUSTR_20 = "level 20: central processing";

constexpr auto THUSTR_21 = "level 21: administration center";
constexpr auto THUSTR_22 = "level 22: habitat";
constexpr auto THUSTR_23 = "level 23: lunar mining project";
constexpr auto THUSTR_24 = "level 24: quarry";
constexpr auto THUSTR_25 = "level 25: baron's den";
constexpr auto THUSTR_26 = "level 26: ballistyx";
constexpr auto THUSTR_27 = "level 27: mount pain";
constexpr auto THUSTR_28 = "level 28: heck";
constexpr auto THUSTR_29 = "level 29: river styx";
constexpr auto THUSTR_30 = "level 30: last call";

constexpr auto THUSTR_31 = "level 31: pharaoh";
constexpr auto THUSTR_32 = "level 32: caribbean";

constexpr auto NHUSTR_1 = "level 1: The Earth Base";
constexpr auto NHUSTR_2 = "level 2: The Pain Labs";
constexpr auto NHUSTR_3 = "level 3: Canyon of the Dead";
constexpr auto NHUSTR_4 = "level 4: Hell Mountain";
constexpr auto NHUSTR_5 = "level 5: Vivisection";
constexpr auto NHUSTR_6 = "level 6: Inferno of Blood";
constexpr auto NHUSTR_7 = "level 7: Baron's Banquet";
constexpr auto NHUSTR_8 = "level 8: Tomb of Malevolence";
constexpr auto NHUSTR_9 = "level 9: March of the Demons";

constexpr auto MHUSTR_1  = "level 1: Attack";
constexpr auto MHUSTR_2  = "level 2: Canyon";
constexpr auto MHUSTR_3  = "level 3: The Catwalk";
constexpr auto MHUSTR_4  = "level 4: The Combine";
constexpr auto MHUSTR_5  = "level 5: The Fistula";
constexpr auto MHUSTR_6  = "level 6: The Garrison";
constexpr auto MHUSTR_7  = "level 7: Titan Manor";
constexpr auto MHUSTR_8  = "level 8: Paradox";
constexpr auto MHUSTR_9  = "level 9: Subspace";
constexpr auto MHUSTR_10 = "level 10: Subterra";
constexpr auto MHUSTR_11 = "level 11: Trapped On Titan";
constexpr auto MHUSTR_12 = "level 12: Virgil's Lead";
constexpr auto MHUSTR_13 = "level 13: Minos' Judgement";
constexpr auto MHUSTR_14 = "level 14: Bloodsea Keep";
constexpr auto MHUSTR_15 = "level 15: Mephisto's Maosoleum";
constexpr auto MHUSTR_16 = "level 16: Nessus";
constexpr auto MHUSTR_17 = "level 17: Geryon";
constexpr auto MHUSTR_18 = "level 18: Vesperas";
constexpr auto MHUSTR_19 = "level 19: Black Tower";
constexpr auto MHUSTR_20 = "level 20: The Express Elevator To Hell";
constexpr auto MHUSTR_21 = "level 21: Bad Dream";

constexpr auto HUSTR_CHATMACRO1 = "I'm ready to kick butt!";
constexpr auto HUSTR_CHATMACRO2 = "I'm OK.";
constexpr auto HUSTR_CHATMACRO3 = "I'm not looking too good!";
constexpr auto HUSTR_CHATMACRO4 = "Help!";
constexpr auto HUSTR_CHATMACRO5 = "You suck!";
constexpr auto HUSTR_CHATMACRO6 = "Next time, scumbag...";
constexpr auto HUSTR_CHATMACRO7 = "Come here!";
constexpr auto HUSTR_CHATMACRO8 = "I'll take care of it.";
constexpr auto HUSTR_CHATMACRO9 = "Yes";
constexpr auto HUSTR_CHATMACRO0 = "No";

constexpr auto HUSTR_TALKTOSELF1 = "You mumble to yourself";
constexpr auto HUSTR_TALKTOSELF2 = "Who's there?";
constexpr auto HUSTR_TALKTOSELF3 = "You scare yourself";
constexpr auto HUSTR_TALKTOSELF4 = "You start to rave";
constexpr auto HUSTR_TALKTOSELF5 = "You've lost it...";

constexpr auto HUSTR_MESSAGESENT = "[Message Sent]";

// The following should NOT be changed unless it seems
// just AWFULLY necessary

constexpr auto HUSTR_PLRGREEN  = "Green: ";
constexpr auto HUSTR_PLRINDIGO = "Indigo: ";
constexpr auto HUSTR_PLRBROWN  = "Brown: ";
constexpr auto HUSTR_PLRRED    = "Red: ";

constexpr char HUSTR_KEYGREEN  = 'g';
constexpr char HUSTR_KEYINDIGO = 'i';
constexpr char HUSTR_KEYBROWN  = 'b';
constexpr char HUSTR_KEYRED    = 'r';

//
//	AM_map.C
//

constexpr auto AMSTR_FOLLOWON  = "Follow Mode ON";
constexpr auto AMSTR_FOLLOWOFF = "Follow Mode OFF";

constexpr auto AMSTR_GRIDON  = "Grid ON";
constexpr auto AMSTR_GRIDOFF = "Grid OFF";

constexpr auto AMSTR_MARKEDSPOT   = "Marked Spot";
constexpr auto AMSTR_MARKSCLEARED = "All Marks Cleared";

constexpr auto AMSTR_OVERLAYON  = "Overlay Mode ON";
constexpr auto AMSTR_OVERLAYOFF = "Overlay Mode OFF";

constexpr auto AMSTR_ROTATEON  = "Rotate Mode ON";
constexpr auto AMSTR_ROTATEOFF = "Rotate Mode OFF";

//
//	ST_stuff.C
//

constexpr auto STSTR_MUS    = "Music Change";
constexpr auto STSTR_NOMUS  = "IMPOSSIBLE SELECTION";
constexpr auto STSTR_DQDON  = "Degreelessness Mode On";
constexpr auto STSTR_DQDOFF = "Degreelessness Mode Off";

constexpr auto STSTR_KFAADDED = "Very Happy Ammo Added";
constexpr auto STSTR_FAADDED  = "Ammo (no keys) Added";

constexpr auto STSTR_NCON  = "No Clipping Mode ON";
constexpr auto STSTR_NCOFF = "No Clipping Mode OFF";

constexpr auto STSTR_BEHOLD  = "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp";
constexpr auto STSTR_BEHOLDX = "Power-up Toggled";

constexpr auto STSTR_CHOPPERS = "... doesn't suck - GM";
constexpr auto STSTR_CLEV     = "Changing Level...";

//
//	F_Finale.C
//
constexpr auto E1TEXT =
    "Once you beat the big badasses and\n"
    "clean out the moon base you're supposed\n"
    "to win, aren't you? Aren't you? Where's\n"
    "your fat reward and ticket home? What\n"
    "the hell is this? It's not supposed to\n"
    "end this way!\n"
    "\n"
    "It stinks like rotten meat, but looks\n"
    "like the lost Deimos base.  Looks like\n"
    "you're stuck on The Shores of Hell.\n"
    "The only way out is through.\n"
    "\n"
    "To continue the DOOM experience, play\n"
    "The Shores of Hell and its amazing\n"
    "sequel, Inferno!\n";

constexpr auto E2TEXT =
    "You've done it! The hideous cyber-\n"
    "demon lord that ruled the lost Deimos\n"
    "moon base has been slain and you\n"
    "are triumphant! But ... where are\n"
    "you? You clamber to the edge of the\n"
    "moon and look down to see the awful\n"
    "truth.\n"
    "\n"
    "Deimos floats above Hell itself!\n"
    "You've never heard of anyone escaping\n"
    "from Hell, but you'll make the bastards\n"
    "sorry they ever heard of you! Quickly,\n"
    "you rappel down to  the surface of\n"
    "Hell.\n"
    "\n"
    "Now, it's on to the final chapter of\n"
    "DOOM! -- Inferno.";

constexpr auto E3TEXT =
    "The loathsome spiderdemon that\n"
    "masterminded the invasion of the moon\n"
    "bases and caused so much death has had\n"
    "its ass kicked for all time.\n"
    "\n"
    "A hidden doorway opens and you enter.\n"
    "You've proven too tough for Hell to\n"
    "contain, and now Hell at last plays\n"
    "fair -- for you emerge from the door\n"
    "to see the green fields of Earth!\n"
    "Home at last.\n"
    "\n"
    "You wonder what's been happening on\n"
    "Earth while you were battling evil\n"
    "unleashed. It's good that no Hell-\n"
    "spawn could have come through that\n"
    "door with you ...";

constexpr auto E4TEXT =
    "the spider mastermind must have sent forth\n"
    "its legions of hellspawn before your\n"
    "final confrontation with that terrible\n"
    "beast from hell.  but you stepped forward\n"
    "and brought forth eternal damnation and\n"
    "suffering upon the horde as a true hero\n"
    "would in the face of something so evil.\n"
    "\n"
    "besides, someone was gonna pay for what\n"
    "happened to daisy, your pet rabbit.\n"
    "\n"
    "but now, you see spread before you more\n"
    "potential pain and gibbitude as a nation\n"
    "of demons run amok among our cities.\n"
    "\n"
    "next stop, hell on earth!";

constexpr auto E5TEXT =
    "Baphomet was only doing Satan's bidding\n"
    "by bringing you back to Hell. Somehow they\n"
    "didn't understand that you're the reason\n"
    "they failed in the first place.\n"
    "\n"
    "After mopping up the place with your\n"
    "arsenal, you're ready to face the more\n"
    "advanced demons that were sent to Earth.\n"
    "\n"
    "\n"
    "Lock and load. Rip and tear.";

// after level 6, put this:

constexpr auto C1TEXT =
    "YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n"
    "STARPORT. BUT SOMETHING IS WRONG. THE\n"
    "MONSTERS HAVE BROUGHT THEIR OWN REALITY\n"
    "WITH THEM, AND THE STARPORT'S TECHNOLOGY\n"
    "IS BEING SUBVERTED BY THEIR PRESENCE.\n"
    "\n"
    "AHEAD, YOU SEE AN OUTPOST OF HELL, A\n"
    "FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n"
    "YOU CAN PENETRATE INTO THE HAUNTED HEART\n"
    "OF THE STARBASE AND FIND THE CONTROLLING\n"
    "SWITCH WHICH HOLDS EARTH'S POPULATION\n"
    "HOSTAGE.";

// After level 11, put this:

constexpr auto C2TEXT =
    "YOU HAVE WON! YOUR VICTORY HAS ENABLED\n"
    "HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"
    "THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"
    "HUMAN LEFT ON THE FACE OF THE PLANET.\n"
    "CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"
    "AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"
    "YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"
    "THAT YOU HAVE SAVED YOUR SPECIES.\n"
    "\n"
    "BUT THEN, EARTH CONTROL BEAMS DOWN A\n"
    "MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"
    "THE SOURCE OF THE ALIEN INVASION. IF YOU\n"
    "GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"
    "ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"
    "YOUR OWN HOME CITY, NOT FAR FROM THE\n"
    "STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"
    "UP AND RETURN TO THE FRAY.";

// After level 20, put this:

constexpr auto C3TEXT =
    "YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
    "SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
    "YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
    "ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
    "TEETH AND PLUNGE THROUGH IT.\n"
    "\n"
    "THERE MUST BE A WAY TO CLOSE IT ON THE\n"
    "OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"
    "GOT TO GO THROUGH HELL TO GET TO IT?";

// After level 29, put this:

constexpr auto C4TEXT =
    "THE HORRENDOUS VISAGE OF THE BIGGEST\n"
    "DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"
    "YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"
    "HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"
    "UP AND DIES, ITS THRASHING LIMBS\n"
    "DEVASTATING UNTOLD MILES OF HELL'S\n"
    "SURFACE.\n"
    "\n"
    "YOU'VE DONE IT. THE INVASION IS OVER.\n"
    "EARTH IS SAVED. HELL IS A WRECK. YOU\n"
    "WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"
    "DIE, NOW. WIPING THE SWEAT FROM YOUR\n"
    "FOREHEAD YOU BEGIN THE LONG TREK BACK\n"
    "HOME. REBUILDING EARTH OUGHT TO BE A\n"
    "LOT MORE FUN THAN RUINING IT WAS.\n";

// Before level 31, put this:

constexpr auto C5TEXT =
    "CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
    "LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"
    "HUMANS, RATHER THAN DEMONS. YOU WONDER\n"
    "WHO THE INMATES OF THIS CORNER OF HELL\n"
    "WILL BE.";

// Before level 32, put this:

constexpr auto C6TEXT =
    "CONGRATULATIONS, YOU'VE FOUND THE\n"
    "SUPER SECRET LEVEL!  YOU'D BETTER\n"
    "BLAZE THROUGH THIS ONE!\n";

// after map 06

constexpr auto P1TEXT =
    "You gloat over the steaming carcass of the\n"
    "Guardian.  With its death, you've wrested\n"
    "the Accelerator from the stinking claws\n"
    "of Hell.  You relax and glance around the\n"
    "room.  Damn!  There was supposed to be at\n"
    "least one working prototype, but you can't\n"
    "see it. The demons must have taken it.\n"
    "\n"
    "You must find the prototype, or all your\n"
    "struggles will have been wasted. Keep\n"
    "moving, keep fighting, keep killing.\n"
    "Oh yes, keep living, too.";

// after map 11

constexpr auto P2TEXT =
    "Even the deadly Arch-Vile labyrinth could\n"
    "not stop you, and you've gotten to the\n"
    "prototype Accelerator which is soon\n"
    "efficiently and permanently deactivated.\n"
    "\n"
    "You're good at that kind of thing.";

// after map 20

constexpr auto P3TEXT =
    "You've bashed and battered your way into\n"
    "the heart of the devil-hive.  Time for a\n"
    "Search-and-Destroy mission, aimed at the\n"
    "Gatekeeper, whose foul offspring is\n"
    "cascading to Earth.  Yeah, he's bad. But\n"
    "you know who's worse!\n"
    "\n"
    "Grinning evilly, you check your gear, and\n"
    "get ready to give the bastard a little Hell\n"
    "of your own making!";

// after map 30

constexpr auto P4TEXT =
    "The Gatekeeper's evil face is splattered\n"
    "all over the place.  As its tattered corpse\n"
    "collapses, an inverted Gate forms and\n"
    "sucks down the shards of the last\n"
    "prototype Accelerator, not to mention the\n"
    "few remaining demons.  You're done. Hell\n"
    "has gone back to pounding bad dead folks \n"
    "instead of good live ones.  Remember to\n"
    "tell your grandkids to put a rocket\n"
    "launcher in your coffin. If you go to Hell\n"
    "when you die, you'll need it for some\n"
    "final cleaning-up ...";

// before map 31

constexpr auto P5TEXT =
    "You've found the second-hardest level we\n"
    "got. Hope you have a saved game a level or\n"
    "two previous.  If not, be prepared to die\n"
    "aplenty. For master marines only.";

// before map 32

constexpr auto P6TEXT =
    "Betcha wondered just what WAS the hardest\n"
    "level we had ready for ya?  Now you know.\n"
    "No one gets out alive.";

constexpr auto T1TEXT =
    "You've fought your way out of the infested\n"
    "experimental labs.   It seems that UAC has\n"
    "once again gulped it down.  With their\n"
    "high turnover, it must be hard for poor\n"
    "old UAC to buy corporate health insurance\n"
    "nowadays..\n"
    "\n"
    "Ahead lies the military complex, now\n"
    "swarming with diseased horrors hot to get\n"
    "their teeth into you. With luck, the\n"
    "complex still has some warlike ordnance\n"
    "laying around.";

constexpr auto T2TEXT =
    "You hear the grinding of heavy machinery\n"
    "ahead.  You sure hope they're not stamping\n"
    "out new hellspawn, but you're ready to\n"
    "ream out a whole herd if you have to.\n"
    "They might be planning a blood feast, but\n"
    "you feel about as mean as two thousand\n"
    "maniacs packed into one mad killer.\n"
    "\n"
    "You don't plan to go down easy.";

constexpr auto T3TEXT =
    "The vista opening ahead looks real damn\n"
    "familiar. Smells familiar, too -- like\n"
    "fried excrement. You didn't like this\n"
    "place before, and you sure as hell ain't\n"
    "planning to like it now. The more you\n"
    "brood on it, the madder you get.\n"
    "Hefting your gun, an evil grin trickles\n"
    "onto your face. Time to take some names.";

constexpr auto T4TEXT =
    "Suddenly, all is silent, from one horizon\n"
    "to the other. The agonizing echo of Hell\n"
    "fades away, the nightmare sky turns to\n"
    "blue, the heaps of monster corpses start \n"
    "to evaporate along with the evil stench \n"
    "that filled the air. Jeeze, maybe you've\n"
    "done it. Have you really won?\n"
    "\n"
    "Something rumbles in the distance.\n"
    "A blue light begins to glow inside the\n"
    "ruined skull of the demon-spitter.";

constexpr auto T5TEXT =
    "What now? Looks totally different. Kind\n"
    "of like King Tut's condo. Well,\n"
    "whatever's here can't be any worse\n"
    "than usual. Can it?  Or maybe it's best\n"
    "to let sleeping gods lie..";

constexpr auto T6TEXT =
    "Time for a vacation. You've burst the\n"
    "bowels of hell and by golly you're ready\n"
    "for a break. You mutter to yourself,\n"
    "Maybe someone else can kick Hell's ass\n"
    "next time around. Ahead lies a quiet town,\n"
    "with peaceful flowing water, quaint\n"
    "buildings, and presumably no Hellspawn.\n"
    "\n"
    "As you step off the transport, you hear\n"
    "the stomp of a cyberdemon's iron shoe.";

constexpr auto N1TEXT =
    "TROUBLE WAS BREWING AGAIN IN YOUR FAVORITE\n"
    "VACATION SPOT... HELL. SOME CYBERDEMON\n"
    "PUNK THOUGHT HE COULD TURN HELL INTO A\n"
    "PERSONAL AMUSEMENT PARK, AND MAKE EARTH\nTHE TICKET BOOTH.\n\n"
    "WELL THAT HALF-ROBOT FREAK SHOW DIDN'T\n"
    "KNOW WHO WAS COMING TO THE FAIR. THERE'S\n"
    "NOTHING LIKE A SHOOTING GALLERY FULL OF\n"
    "HELLSPAWN TO GET THE BLOOD PUMPING...\n\n"
    "NOW THE WALLS OF THE DEMON'S LABYRINTH\n"
    "ECHO WITH THE SOUND OF HIS METALLIC LIMBS\n"
    "HITTING THE FLOOR. HIS DEATH MOAN GURGLES\n"
    "OUT THROUGH THE MESS YOU LEFT OF HIS FACE.\n\n"
    "THIS RIDE IS CLOSED.";

constexpr auto M1TEXT =
    "CONGRATULATIONS YOU HAVE FINISHED... \n\n"
    "THE MASTER LEVELS\n";

//
// Character cast strings F_FINALE.C
//
constexpr auto CC_ZOMBIE  = "ZOMBIEMAN";
constexpr auto CC_SHOTGUN = "SHOTGUN GUY";
constexpr auto CC_HEAVY   = "HEAVY WEAPON DUDE";
constexpr auto CC_IMP     = "IMP";
constexpr auto CC_DEMON   = "DEMON";
constexpr auto CC_LOST    = "LOST SOUL";
constexpr auto CC_CACO    = "CACODEMON";
constexpr auto CC_HELL    = "HELL KNIGHT";
constexpr auto CC_BARON   = "BARON OF HELL";
constexpr auto CC_ARACH   = "ARACHNOTRON";
constexpr auto CC_PAIN    = "PAIN ELEMENTAL";
constexpr auto CC_REVEN   = "REVENANT";
constexpr auto CC_MANCU   = "MANCUBUS";
constexpr auto CC_ARCH    = "ARCH-VILE";
constexpr auto CC_SPIDER  = "THE SPIDER MASTERMIND";
constexpr auto CC_CYBER   = "THE CYBERDEMON";
constexpr auto CC_HERO    = "OUR HERO";

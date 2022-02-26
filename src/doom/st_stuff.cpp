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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//

#include <cstdio>

#include <fmt/printf.h>

#include "i_swap.hpp" // [crispy] SHORT()
#include "i_video.hpp"
#include "m_argv.hpp" // [crispy] M_ParmExists()
#include "m_misc.hpp"
#include "m_random.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "deh_main.hpp"
#include "deh_misc.hpp"
#include "doomdef.hpp"

#include "g_game.hpp"

#include "r_local.hpp"
#include "st_lib.hpp"
#include "st_stuff.hpp"

#include "p_inter.hpp"
#include "p_local.hpp"

#include "am_map.hpp"
#include "m_cheat.hpp"

#include "s_sound.hpp"

// Needs access to LFB.
#include "v_video.hpp"

// State.
#include "doomstat.hpp"

// Data.
#include "dstrings.hpp"
#include "sounds.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "v_trans.hpp" // [crispy] colored cheat messages

extern int  screenblocks;  // [crispy] for the Crispy HUD
extern bool inhelpscreens; // [crispy] prevent palette changes

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
constexpr auto STARTREDPALS   = 1;
constexpr auto STARTBONUSPALS = 9;
constexpr auto NUMREDPALS     = 8;
constexpr auto NUMBONUSPALS   = 4;
// Radiation suit, green shift.
constexpr auto RADIATIONPAL = 13;

// Location of status bar
constexpr auto ST_X = 0;

constexpr auto ST_FX = 143;

// Number of status faces.
constexpr auto ST_NUMPAINFACES      = 5;
constexpr auto ST_NUMSTRAIGHTFACES  = 3;
constexpr auto ST_NUMTURNFACES      = 2;
constexpr auto ST_NUMSPECIALFACES   = 3;
constexpr auto ST_FACESTRIDE        = (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES);
constexpr auto ST_NUMEXTRAFACES     = 2;
constexpr auto ST_NUMFACES          = (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES);
constexpr auto ST_TURNOFFSET        = (ST_NUMSTRAIGHTFACES);
constexpr auto ST_OUCHOFFSET        = (ST_TURNOFFSET + ST_NUMTURNFACES);
constexpr auto ST_EVILGRINOFFSET    = (ST_OUCHOFFSET + 1);
constexpr auto ST_RAMPAGEOFFSET     = (ST_EVILGRINOFFSET + 1);
constexpr auto ST_GODFACE           = (ST_NUMPAINFACES * ST_FACESTRIDE);
constexpr auto ST_DEADFACE          = (ST_GODFACE + 1);
constexpr auto ST_FACESX            = 143;
constexpr auto ST_FACESY            = 168;
constexpr auto ST_EVILGRINCOUNT     = (2 * TICRATE);
constexpr auto ST_STRAIGHTFACECOUNT = (TICRATE / 2);
constexpr auto ST_TURNCOUNT         = (1 * TICRATE);
constexpr auto ST_RAMPAGEDELAY      = (2 * TICRATE);
constexpr auto ST_MUCHPAIN          = 20;

static auto HORIZDELTA() {
  return (crispy->widescreen == 1 ? DELTAWIDTH : 0);
}
static auto ST_ARMSBGX() {
  return (104 - HORIZDELTA());
}
static auto ST_HEALTHX() {
  return (90 - HORIZDELTA());
}
static auto ST_AMMOX() {
  return (44 - HORIZDELTA());
}

// Dimensions given in characters.
constexpr auto ST_MSGWIDTH = 52;

// main player in game
static player_t * plyr;

// ST_Start() has just been called
static bool st_firsttime;

// lump number for PLAYPAL
static int lu_palette;

// used for timing
static unsigned int st_clock;

// used for making messages go away
static int st_msgcounter = 0;

// used when in chat
[[maybe_unused]] static st_chatstateenum_t st_chatstate;

// whether in automap or first-person
[[maybe_unused]] static st_stateenum_t st_gamestate;

// whether left-side main status bar is active
static bool st_statusbaron;

// [crispy] distinguish classic status bar with background and player face from Crispy HUD
static bool st_crispyhud;
static bool st_classicstatusbar;
static bool st_statusbarface;

// whether status bar chat is active
static bool st_chat;

// value of st_chat before message popped up
static bool st_oldchat;

// whether chat window has the cursor on
[[maybe_unused]] static bool st_cursoron;

// !deathmatch
static bool st_notdeathmatch;

// !deathmatch && st_statusbaron
static bool st_armson;

// !deathmatch
static bool st_fragson;

// main bar left
static patch_t * sbar;

// main bar right, for doom 1.0
static patch_t * sbarr;

// 0-9, tall numbers
static std::array<patch_t *, 10> tallnum;

// tall % sign
static patch_t * tallpercent;

// 0-9, short, yellow (,different!) numbers
static std::array<patch_t *, 10> shortnum;

// 3 key-cards, 3 skulls
static std::array<patch_t *, NUMCARDS + 3> keys; // [crispy] support combined card and skull keys

// face status patches
static std::array<patch_t *, ST_NUMFACES> faces;

// face background
static patch_t * faceback;

// main bar right
static patch_t * armsbg;

// weapon ownership patches
static patch_t * arms[6][2];

// ready-weapon widget
static st_number_t w_ready;

// in deathmatch only, summary of frags stats
static st_number_t w_frags;

// health widget
static st_percent_t w_health;

// arms background
static st_binicon_t w_armsbg;

// weapon ownership widgets
static std::array<st_multicon_t, 6> w_arms;
// [crispy] show SSG availability in the Shotgun slot of the arms widget
static int st_shotguns;

// face status widget
static st_multicon_t w_faces;

// keycard widgets
static st_multicon_t w_keyboxes[3];

// armor widget
static st_percent_t w_armor;

// ammo widgets
static st_number_t w_ammo[4];

// max ammo widgets
static st_number_t w_maxammo[4];

// number of frags so far in deathmatch
static int st_fragscount;

// used to use appopriately pained face
static int st_oldhealth = -1;

// used for evil grin
static bool oldweaponsowned[NUMWEAPONS];

// count until face changes
static int st_facecount = 0;

// current face index, used by w_faces
static int st_faceindex = 0;

// holds key-type for each key box on bar
static int keyboxes[3];

// a random number per tick
static int st_randomnumber;

static st_stuff_t st_stuff_s = {
  .st_backing_screen       = nullptr,
  .cheat_mus               = CHEAT("idmus", 2),
  .cheat_god               = CHEAT("iddqd", 0),
  .cheat_ammo              = CHEAT("idkfa", 0),
  .cheat_ammonokey         = CHEAT("idfa", 0),
  .cheat_noclip            = CHEAT("idspispopd", 0),
  .cheat_commercial_noclip = CHEAT("idclip", 0),
  .cheat_powerup           = {
      CHEAT("idbeholdv", 0),
      CHEAT("idbeholds", 0),
      CHEAT("idbeholdi", 0),
      CHEAT("idbeholdr", 0),
      CHEAT("idbeholda", 0),
      CHEAT("idbeholdl", 0),
      CHEAT("idbehold", 0),
      CHEAT("idbehold0", 0), // [crispy] idbehold0
  },                         // [crispy] idbehold0
  .cheat_choppers = CHEAT("idchoppers", 0),
  .cheat_clev     = CHEAT("idclev", 2),
  .cheat_mypos    = CHEAT("idmypos", 0)
};
st_stuff_t * const g_st_stuff_globals = &st_stuff_s;

// [crispy] pseudo cheats to eat up the first digit typed after a cheat expecting two parameters
cheatseq_t cheat_mus1  = CHEAT("idmus", 1);
cheatseq_t cheat_clev1 = CHEAT("idclev", 1);

// [crispy] new cheats
cheatseq_t  cheat_weapon     = CHEAT("tntweap", 1);
cheatseq_t  cheat_massacre   = CHEAT("tntem", 0);  // [crispy] PrBoom+
cheatseq_t  cheat_massacre2  = CHEAT("killem", 0); // [crispy] MBF
cheatseq_t  cheat_massacre3  = CHEAT("fhhall", 0); // [crispy] Doom95
cheatseq_t  cheat_hom        = CHEAT("tnthom", 0);
cheatseq_t  cheat_notarget   = CHEAT("notarget", 0); // [crispy] PrBoom+
cheatseq_t  cheat_notarget2  = CHEAT("fhshh", 0);    // [crispy] Doom95
cheatseq_t  cheat_spechits   = CHEAT("spechits", 0);
cheatseq_t  cheat_nomomentum = CHEAT("nomomentum", 0);
cheatseq_t  cheat_showfps    = CHEAT("showfps", 0);
cheatseq_t  cheat_showfps2   = CHEAT("idrate", 0); // [crispy] PrBoom+
cheatseq_t  cheat_goobers    = CHEAT("goobers", 0);
cheatseq_t  cheat_version    = CHEAT("version", 0); // [crispy] Russian Doom
cheatseq_t  cheat_skill      = CHEAT("skill", 0);
static char msg[ST_MSGWIDTH];

// [crispy] restrict cheat usage
static inline int cht_CheckCheatSP(cheatseq_t * cht, char key) {
  if (!cht_CheckCheat(cht, key)) {
    return false;
  } else if (!crispy->singleplayer) {
    plyr->message = "Cheater!";
    return false;
  }
  return true;
}

//
// STATUS BAR CODE
//
void ST_Stop();

void ST_refreshBackground(bool force) {

  if (st_classicstatusbar || force) {
    V_UseBuffer(g_st_stuff_globals->st_backing_screen);

    V_DrawPatch(ST_X, 0, sbar);

    // draw right side of bar if needed (Doom 1.0)
    if (sbarr)
      V_DrawPatch(ST_ARMSBGX(), 0, sbarr);

    // [crispy] back up arms widget background
    if (!g_doomstat_globals->deathmatch)
      V_DrawPatch(ST_ARMSBGX(), 0, armsbg);

    if (g_doomstat_globals->netgame)
      V_DrawPatch(ST_FX, 0, faceback);

    V_RestoreBuffer();

    if (!force)
      V_CopyRect(ST_X, 0, g_st_stuff_globals->st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
  }
}

// [crispy] adapted from boom202s/M_CHEAT.C:467-498
static int ST_cheat_massacre() {
  int         killcount = 0;
  thinker_t * th        = nullptr;
  extern int  numbraintargets;
  extern void A_PainDie(mobj_t *);

  action_hook needle = P_MobjThinker;
  for (th = g_p_local_globals->thinkercap.next; th != &g_p_local_globals->thinkercap; th = th->next) {
    if (th->function == needle) {
      auto * mo = reinterpret_cast<mobj_t *>(th);

      if (static_cast<unsigned int>(mo->flags) & MF_COUNTKILL || mo->type == MT_SKULL) {
        if (mo->health > 0) {
          P_DamageMobj(mo, nullptr, nullptr, 10000);
          killcount++;
        }
        if (mo->type == MT_PAIN) {
          A_PainDie(mo);
          P_SetMobjState(mo, S_PAIN_DIE6);
        }
      }
    }
  }

  // [crispy] disable brain spitters
  numbraintargets = -1;

  return killcount;
}

// [crispy] trigger all special lines available on the map
static int ST_cheat_spechits() {
  int    speciallines = 0;
  bool   origcards[NUMCARDS];
  line_t dummy;

  // [crispy] temporarily give all keys
  for (int i = 0; i < NUMCARDS; i++) {
    origcards[i]   = plyr->cards[i];
    plyr->cards[i] = true;
  }

  for (int i = 0; i < g_r_state_globals->numlines; i++) {
    if (g_r_state_globals->lines[i].special) {
      // [crispy] do not trigger level exit switches/lines or teleporters
      if (g_r_state_globals->lines[i].special == 11 || g_r_state_globals->lines[i].special == 51 || g_r_state_globals->lines[i].special == 52 || g_r_state_globals->lines[i].special == 124 || g_r_state_globals->lines[i].special == 39 || g_r_state_globals->lines[i].special == 97) {
        continue;
      }

      // [crispy] special without tag --> DR linedef type
      // do not change door direction if it is already moving
      if (g_r_state_globals->lines[i].tag == 0 && g_r_state_globals->lines[i].sidenum[1] != NO_INDEX && g_r_state_globals->sides[g_r_state_globals->lines[i].sidenum[1]].sector->specialdata) {
        continue;
      }

      P_CrossSpecialLine(i, 0, plyr->mo);
      P_ShootSpecialLine(plyr->mo, &g_r_state_globals->lines[i]);
      P_UseSpecialLine(plyr->mo, &g_r_state_globals->lines[i], 0);

      speciallines++;
    }
  }

  for (int i = 0; i < NUMCARDS; i++) {
    plyr->cards[i] = origcards[i];
  }

  // [crispy] trigger tag 666/667 events
  dummy.tag = 666;
  if (g_doomstat_globals->gamemode == commercial) {
    if (g_doomstat_globals->gamemap == 7 ||
        // [crispy] Master Levels in PC slot 7
        (g_doomstat_globals->gamemission == pack_master && (g_doomstat_globals->gamemap == 14 || g_doomstat_globals->gamemap == 15 || g_doomstat_globals->gamemap == 16))) {
      // Mancubi
      speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);

      // Arachnotrons
      dummy.tag = 667;
      speciallines += EV_DoFloor(&dummy, raiseToTexture);
      dummy.tag = 666;
    }
  } else {
    if (g_doomstat_globals->gameepisode == 1)
      // Barons of Hell
      speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
    else if (g_doomstat_globals->gameepisode == 4) {
      if (g_doomstat_globals->gamemap == 6)
        // Cyberdemons
        speciallines += EV_DoDoor(&dummy, vld_blazeOpen);
      else if (g_doomstat_globals->gamemap == 8)
        // Spider Masterminds
        speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
    }
  }
  // Keens (no matter which level they are on)
  // this call will be ignored if the tagged sector is already moving
  // so actions triggered in the condition above will have precedence
  speciallines += EV_DoDoor(&dummy, vld_open);

  return (speciallines);
}

// [crispy] only give available weapons
static bool WeaponAvailable(int w) {
  if (w < 0 || w >= NUMWEAPONS)
    return false;

  if (w == wp_supershotgun && !crispy->havessg)
    return false;

  if ((w == wp_bfg || w == wp_plasma) && g_doomstat_globals->gamemode == shareware)
    return false;

  return true;
}

// [crispy] give or take backpack
static void GiveBackpack(bool give) {
  if (give && !plyr->backpack) {
    for (int & ammo : plyr->maxammo) {
      ammo *= 2;
    }
    plyr->backpack = true;
  } else if (!give && plyr->backpack) {
    for (int & ammo : plyr->maxammo) {
      ammo /= 2;
    }
    plyr->backpack = false;
  }
}

// Respond to keyboard input events,
//  intercept cheats.
bool ST_Responder(event_t * ev) {
  // Filter automap on/off.
  if (ev->type == ev_keyup
      && ((static_cast<unsigned int>(ev->data1) & 0xffff0000) == AM_MSGHEADER)) {
    switch (ev->data1) {
    case AM_MSGENTERED:
      st_gamestate = AutomapState;
      st_firsttime = true;
      break;

    case AM_MSGEXITED:
      //	fprintf(stderr, "AM exited\n");
      st_gamestate = FirstPersonState;
      break;
    }
  }

  // if a user keypress...
  else if (ev->type == ev_keydown) {
    if (!g_doomstat_globals->netgame && g_doomstat_globals->gameskill != sk_nightmare) {
      // 'dqd' cheat for toggleable god mode
      if (cht_CheckCheatSP(&g_st_stuff_globals->cheat_god, static_cast<char>(ev->data2))) {
        // [crispy] dead players are first respawned at the current position
        mapthing_t mt = {};
        if (plyr->playerstate == PST_DEAD) {
          extern void P_SpawnPlayer(mapthing_t * mthing);

          mt.x     = static_cast<short>(plyr->mo->x >> FRACBITS);
          mt.y     = static_cast<short>(plyr->mo->y >> FRACBITS);
          mt.angle = static_cast<short>((plyr->mo->angle + ANG45 / 2) * static_cast<uint64_t>(45) / ANG45);
          mt.type  = static_cast<short>(g_doomstat_globals->consoleplayer + 1);
          P_SpawnPlayer(&mt);

          // [crispy] spawn a teleport fog
          signed int an = plyr->mo->angle >> ANGLETOFINESHIFT;
          P_SpawnMobj(plyr->mo->x + 20 * finecosine[an], plyr->mo->y + 20 * finesine[an], plyr->mo->z, MT_TFOG);
          S_StartSound(plyr, sfx_slop);
        }

        plyr->cheats ^= CF_GODMODE;
        if (plyr->cheats & CF_GODMODE) {
          if (plyr->mo)
            plyr->mo->health = 100;

          plyr->health  = deh_god_mode_health;
          plyr->message = DEH_String(STSTR_DQDON);
        } else
          plyr->message = DEH_String(STSTR_DQDOFF);

        // [crispy] eat key press when respawning
        if (mt.type)
          return true;
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheatSP(&g_st_stuff_globals->cheat_ammonokey, static_cast<char>(ev->data2))) {
        plyr->armorpoints = deh_idfa_armor;
        plyr->armortype   = deh_idfa_armor_class;

        // [crispy] give backpack
        GiveBackpack(true);

        for (int i = 0; i < NUMWEAPONS; i++)
          if (WeaponAvailable(i)) // [crispy] only give available weapons
            plyr->weaponowned[i] = true;

        for (int i = 0; i < NUMAMMO; i++)
          plyr->ammo[i] = plyr->maxammo[i];

        // [crispy] trigger evil grin now
        plyr->bonuscount += 2;

        plyr->message = DEH_String(STSTR_FAADDED);
      }
      // 'kfa' cheat for key full ammo
      else if (cht_CheckCheatSP(&g_st_stuff_globals->cheat_ammo, static_cast<char>(ev->data2))) {
        plyr->armorpoints = deh_idkfa_armor;
        plyr->armortype   = deh_idkfa_armor_class;

        // [crispy] give backpack
        GiveBackpack(true);

        for (int i = 0; i < NUMWEAPONS; i++)
          if (WeaponAvailable(i)) // [crispy] only give available weapons
            plyr->weaponowned[i] = true;

        for (int i = 0; i < NUMAMMO; i++)
          plyr->ammo[i] = plyr->maxammo[i];

        for (bool & card : plyr->cards)
          card = true;

        // [crispy] trigger evil grin now
        plyr->bonuscount += 2;

        plyr->message = DEH_String(STSTR_KFAADDED);
      }
      // 'mus' cheat for changing music
      else if (cht_CheckCheat(&g_st_stuff_globals->cheat_mus, static_cast<char>(ev->data2))) {

        char buf[3];
        int  musnum = 0;

        plyr->message = DEH_String(STSTR_MUS);
        cht_GetParam(&g_st_stuff_globals->cheat_mus, buf);

        // Note: The original v1.9 had a bug that tried to play back
        // the Doom II music regardless of gamemode.  This was fixed
        // in the Ultimate Doom executable so that it would work for
        // the Doom 1 music as well.

        // [crispy] restart current music if IDMUS00 is entered
        if (buf[0] == '0' && buf[1] == '0') {
          S_ChangeMusic(0, 2);
          // [crispy] eat key press, i.e. don't change weapon upon music change
          return true;
        } else
          // [JN] Fixed: using a proper IDMUS selection for shareware
          // and registered game versions.
          if (g_doomstat_globals->gamemode == commercial /* || gameversion < exe_ultimate */) {
            musnum = mus_runnin + (buf[0] - '0') * 10 + buf[1] - '0' - 1;

            /*
  if (((buf[0]-'0')*10 + buf[1]-'0') > 35
&& gameversion >= exe_doom_1_8)
  */
            // [crispy] prevent crash with IDMUS00
            if (musnum < mus_runnin || musnum >= NUMMUSIC)
              plyr->message = DEH_String(STSTR_NOMUS);
            else {
              S_ChangeMusic(musnum, 1);
              // [crispy] eat key press, i.e. don't change weapon upon music change
              return true;
            }
          } else {
            musnum = mus_e1m1 + (buf[0] - '1') * 9 + (buf[1] - '1');

            /*
  if (((buf[0]-'1')*9 + buf[1]-'1') > 31)
  */
            // [crispy] prevent crash with IDMUS0x or IDMUSx0
            if (musnum < mus_e1m1 || musnum >= mus_runnin ||
                // [crispy] support dedicated music tracks for the 4th episode
                S_music[musnum].lumpnum == -1)
              plyr->message = DEH_String(STSTR_NOMUS);
            else {
              S_ChangeMusic(musnum, 1);
              // [crispy] eat key press, i.e. don't change weapon upon music change
              return true;
            }
          }
      }
      // [crispy] eat up the first digit typed after a cheat expecting two parameters
      else if (cht_CheckCheat(&cheat_mus1, static_cast<char>(ev->data2))) {
        char buf[2];

        cht_GetParam(&cheat_mus1, buf);

        return isdigit(buf[0]);
      }
      // [crispy] allow both idspispopd and idclip cheats in all gamemissions
      else if ((/* logical_gamemission() == doom
           && */
                cht_CheckCheatSP(&g_st_stuff_globals->cheat_noclip, static_cast<char>(ev->data2)))
               || (/* logical_gamemission() != doom
           && */
                   cht_CheckCheatSP(&g_st_stuff_globals->cheat_commercial_noclip, static_cast<char>(ev->data2)))) {
        // Noclip cheat.
        // For Doom 1, use the idspipsopd cheat; for all others, use
        // idclip

        plyr->cheats ^= CF_NOCLIP;

        if (plyr->cheats & CF_NOCLIP)
          plyr->message = DEH_String(STSTR_NCON);
        else
          plyr->message = DEH_String(STSTR_NCOFF);
      }
      // 'behold?' power-up cheats
      for (int i = 0; i < 6; i++) {
        if (i < 4 ? cht_CheckCheatSP(&g_st_stuff_globals->cheat_powerup[i], static_cast<char>(ev->data2)) : cht_CheckCheat(&g_st_stuff_globals->cheat_powerup[i], static_cast<char>(ev->data2))) {
          if (!plyr->powers[i])
            P_GivePower(plyr, i);
          else if (i != pw_strength && i != pw_allmap) // [crispy] disable full Automap
            plyr->powers[i] = 1;
          else
            plyr->powers[i] = 0;

          plyr->message = DEH_String(STSTR_BEHOLDX);
        }
      }
      // [crispy] idbehold0
      if (cht_CheckCheatSP(&g_st_stuff_globals->cheat_powerup[7], static_cast<char>(ev->data2))) {
        std::memset(plyr->powers, 0, sizeof(plyr->powers));
        plyr->mo->flags &= ~MF_SHADOW; // [crispy] cancel invisibility
        plyr->message = DEH_String(STSTR_BEHOLDX);
      }

      // 'behold' power-up menu
      if (cht_CheckCheat(&g_st_stuff_globals->cheat_powerup[6], static_cast<char>(ev->data2))) {
        plyr->message = DEH_String(STSTR_BEHOLD);
      }
      // 'choppers' invulnerability & chainsaw
      else if (cht_CheckCheatSP(&g_st_stuff_globals->cheat_choppers, static_cast<char>(ev->data2))) {
        plyr->weaponowned[wp_chainsaw]   = true;
        plyr->powers[pw_invulnerability] = true;
        plyr->message                    = DEH_String(STSTR_CHOPPERS);
      }
      // 'mypos' for player position
      else if (cht_CheckCheat(&g_st_stuff_globals->cheat_mypos, static_cast<char>(ev->data2))) {
        /*
static char buf[ST_MSGWIDTH];
M_snprintf(buf, sizeof(buf), "ang=0x%x;x,y=(0x%x,0x%x)",
           players[consoleplayer].mo->angle,
           players[consoleplayer].mo->x,
           players[consoleplayer].mo->y);
plyr->message = buf;
*/
        // [crispy] extra high precision IDMYPOS variant, updates for 10 seconds
        plyr->powers[pw_mapcoords] = 10 * TICRATE;
      }

      // [crispy] now follow "critical" Crispy Doom specific cheats

      // [crispy] implement Boom's "tntem" cheat
      else if (cht_CheckCheatSP(&cheat_massacre, static_cast<char>(ev->data2)) || cht_CheckCheatSP(&cheat_massacre2, static_cast<char>(ev->data2)) || cht_CheckCheatSP(&cheat_massacre3, static_cast<char>(ev->data2))) {
        int                killcount = ST_cheat_massacre();
        const char * const monster   = (g_doomstat_globals->gameversion == exe_chex) ? "Flemoid" : "Monster";
        const char * const killed    = (g_doomstat_globals->gameversion == exe_chex) ? "returned" : "killed";

        M_snprintf(msg, sizeof(msg), "%s%d %s%s%s %s", crstr[static_cast<int>(cr_t::CR_GOLD)], killcount, crstr[static_cast<int>(cr_t::CR_NONE)], monster, (killcount == 1) ? "" : "s", killed);
        plyr->message = msg;
      }
      // [crispy] implement Crispy Doom's "spechits" cheat
      else if (cht_CheckCheatSP(&cheat_spechits, static_cast<char>(ev->data2))) {
        int triggeredlines = ST_cheat_spechits();

        M_snprintf(msg, sizeof(msg), "%s%d %sSpecial Line%s Triggered", crstr[static_cast<int>(cr_t::CR_GOLD)], triggeredlines, crstr[static_cast<int>(cr_t::CR_NONE)], (triggeredlines == 1) ? "" : "s");
        plyr->message = msg;
      }
      // [crispy] implement PrBoom+'s "notarget" cheat
      else if (cht_CheckCheatSP(&cheat_notarget, static_cast<char>(ev->data2)) || cht_CheckCheatSP(&cheat_notarget2, static_cast<char>(ev->data2))) {
        plyr->cheats ^= CF_NOTARGET;

        if (plyr->cheats & CF_NOTARGET) {
          thinker_t * th = nullptr;

          // [crispy] let mobjs forget their target and tracer
          action_hook needle = P_MobjThinker;
          for (th = g_p_local_globals->thinkercap.next; th != &g_p_local_globals->thinkercap; th = th->next) {
            if (th->function == needle) {
              auto * const mo = reinterpret_cast<mobj_t *>(th);

              if (mo->target && mo->target->player) {
                mo->target = nullptr;
              }

              if (mo->tracer && mo->tracer->player) {
                mo->tracer = nullptr;
              }
            }
          }
          // [crispy] let sectors forget their soundtarget
          for (int i = 0; i < g_r_state_globals->numsectors; i++) {
            sector_t * const sector = &g_r_state_globals->sectors[i];

            sector->soundtarget = nullptr;
          }
        }

        M_snprintf(msg, sizeof(msg), "Notarget Mode %s%s", crstr[static_cast<int>(cr_t::CR_GREEN)], (plyr->cheats & CF_NOTARGET) ? "ON" : "OFF");
        plyr->message = msg;
      }
      // [crispy] implement "nomomentum" cheat, ne debug aid -- pretty useless, though
      else if (cht_CheckCheatSP(&cheat_nomomentum, static_cast<char>(ev->data2))) {
        plyr->cheats ^= CF_NOMOMENTUM;

        M_snprintf(msg, sizeof(msg), "Nomomentum Mode %s%s", crstr[static_cast<int>(cr_t::CR_GREEN)], (plyr->cheats & CF_NOMOMENTUM) ? "ON" : "OFF");
        plyr->message = msg;
      }
      // [crispy] implement Crispy Doom's "goobers" cheat, ne easter egg
      else if (cht_CheckCheatSP(&cheat_goobers, static_cast<char>(ev->data2))) {
        extern void EV_DoGoobers();

        EV_DoGoobers();

        M_snprintf(msg, sizeof(msg), "Get Psyched!");
        plyr->message = msg;
      }
      // [crispy] implement Boom's "tntweap?" weapon cheats
      else if (cht_CheckCheatSP(&cheat_weapon, static_cast<char>(ev->data2))) {
        char buf[2];

        cht_GetParam(&cheat_weapon, buf);
        int w = *buf - '1';

        // [crispy] TNTWEAP0 takes away all weapons and ammo except for the pistol and 50 bullets
        if (w == -1) {
          GiveBackpack(false);
          plyr->powers[pw_strength] = 0;

          for (int i = 0; i < NUMWEAPONS; i++) {
            plyr->weaponowned[i] = false;
            oldweaponsowned[i]   = plyr->weaponowned[i];
          }
          plyr->weaponowned[wp_fist]   = true;
          oldweaponsowned[wp_fist]     = plyr->weaponowned[wp_fist];
          plyr->weaponowned[wp_pistol] = true;
          oldweaponsowned[wp_pistol]   = plyr->weaponowned[wp_pistol];

          for (int & ammo : plyr->ammo) {
            ammo = 0;
          }
          plyr->ammo[am_clip] = deh_initial_bullets;

          if (plyr->readyweapon > wp_pistol) {
            plyr->pendingweapon = wp_pistol;
          }

          plyr->message = "All weapons removed!";

          return true;
        }

        // [crispy] only give available weapons
        if (!WeaponAvailable(w))
          return false;

        // make '1' apply beserker strength toggle
        if (w == wp_fist) {
          if (!plyr->powers[pw_strength]) {
            P_GivePower(plyr, pw_strength);
            S_StartSound(nullptr, sfx_getpow);
            plyr->message = DEH_String(GOTBERSERK);
          } else {
            plyr->powers[pw_strength] = 0;
            plyr->message             = DEH_String(STSTR_BEHOLDX);
          }
        } else {
          if (!plyr->weaponowned[w]) {
            extern bool P_GiveWeapon(player_t * player, weapontype_t weapon, bool dropped);

            P_GiveWeapon(plyr, static_cast<weapontype_t>(w), false);
            S_StartSound(nullptr, sfx_wpnup);

            if (w > 1) {
              plyr->message = DEH_String(WeaponPickupMessages[w]);
            }

            // [crispy] trigger evil grin now
            plyr->bonuscount += 2;
          } else {
            // [crispy] no reason for evil grin
            plyr->weaponowned[w] = false;
            oldweaponsowned[w]   = plyr->weaponowned[w];

            // [crispy] removed current weapon, select another one
            if (w == plyr->readyweapon) {
              extern bool P_CheckAmmo(player_t * player);

              P_CheckAmmo(plyr);
            }
          }
        }

        if (!plyr->message) {
          M_snprintf(msg, sizeof(msg), "Weapon %s%d%s %s", crstr[static_cast<int>(cr_t::CR_GOLD)], w + 1, crstr[static_cast<int>(cr_t::CR_NONE)], plyr->weaponowned[w] ? "added" : "removed");
          plyr->message = msg;
        }
      }
    }

    // [crispy] now follow "harmless" Crispy Doom specific cheats

    // [crispy] implement Crispy Doom's "showfps" cheat, ne debug aid
    if (cht_CheckCheat(&cheat_showfps, static_cast<char>(ev->data2)) || cht_CheckCheat(&cheat_showfps2, static_cast<char>(ev->data2))) {
      plyr->powers[pw_showfps] ^= 1;
    }
    // [crispy] implement Boom's "tnthom" cheat
    else if (cht_CheckCheat(&cheat_hom, static_cast<char>(ev->data2))) {
      crispy->flashinghom = !crispy->flashinghom;

      M_snprintf(msg, sizeof(msg), "HOM Detection %s%s", crstr[static_cast<int>(cr_t::CR_GREEN)], (crispy->flashinghom) ? "ON" : "OFF");
      plyr->message = msg;
    }
    // [crispy] Show engine version, build date and SDL version
    else if (cht_CheckCheat(&cheat_version, static_cast<char>(ev->data2))) {
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif
      M_snprintf(msg, sizeof(msg), "%s (%s) x%ld SDL%s", PACKAGE_STRING, BUILD_DATE, static_cast<long>(sizeof(void *)) * CHAR_BIT, crispy->sdlversion);
#undef BUILD_DATE
      plyr->message = msg;
      fmt::fprintf(stderr, "%s\n", msg);
    }
    // [crispy] Show skill level
    else if (cht_CheckCheat(&cheat_skill, static_cast<char>(ev->data2))) {
      extern const char * skilltable[];

      M_snprintf(msg, sizeof(msg), "Skill: %s", skilltable[std::clamp(static_cast<int>(g_doomstat_globals->gameskill) + 1, 0, 5)]);
      plyr->message = msg;
    }

    // 'clev' change-level cheat
    if (!g_doomstat_globals->netgame && cht_CheckCheat(&g_st_stuff_globals->cheat_clev, static_cast<char>(ev->data2)) && !g_doomstat_globals->menuactive) // [crispy] prevent only half the screen being updated
    {
      char buf[3];
      int  epsd = 0;
      int  map  = 0;

      cht_GetParam(&g_st_stuff_globals->cheat_clev, buf);

      if (g_doomstat_globals->gamemode == commercial) {
        if (g_doomstat_globals->gamemission == pack_nerve)
          epsd = 2;
        else
          epsd = 0;
        map = (buf[0] - '0') * 10 + buf[1] - '0';
      } else {
        epsd = buf[0] - '0';
        map  = buf[1] - '0';

        // Chex.exe always warps to episode 1.

        if (g_doomstat_globals->gameversion == exe_chex) {
          if (epsd > 1) {
            epsd = 1;
          }
          if (map > 5) {
            map = 5;
          }
        }
      }

      // [crispy] only fix episode/map if it doesn't exist
      if (P_GetNumForMap(epsd, map, false) < 0) {
        // Catch invalid maps.
        if (g_doomstat_globals->gamemode != commercial) {
          // [crispy] allow IDCLEV0x to work in Doom 1
          if (epsd == 0) {
            epsd = g_doomstat_globals->gameepisode;
          }
          if (epsd < 1) {
            return false;
          }
          if (epsd > 4) {
            // [crispy] Sigil
            if (!(crispy->haved1e5 && epsd == 5))
              return false;
          }
          if (epsd == 4 && g_doomstat_globals->gameversion < exe_ultimate) {
            return false;
          }
          // [crispy] IDCLEV00 restarts current map
          if ((map == 0) && (buf[0] - '0' == 0)) {
            map = g_doomstat_globals->gamemap;
          }
          // [crispy] support E1M10 "Sewers"
          if ((map == 0 || map > 9) && crispy->havee1m10 && epsd == 1) {
            map = 10;
          }
          if (map < 1) {
            return false;
          }
          if (map > 9) {
            // [crispy] support E1M10 "Sewers"
            if (!(crispy->havee1m10 && epsd == 1 && map == 10))
              return false;
          }
        } else {
          // [crispy] IDCLEV00 restarts current map
          if ((map == 0) && (buf[0] - '0' == 0)) {
            map = g_doomstat_globals->gamemap;
          }
          if (map < 1) {
            return false;
          }
          if (map > 40) {
            return false;
          }
          if (map > 9 && g_doomstat_globals->gamemission == pack_nerve) {
            return false;
          }
          if (map > 21 && g_doomstat_globals->gamemission == pack_master) {
            return false;
          }
        }
      }

      // [crispy] prevent idclev to nonexistent levels exiting the game
      if (P_GetNumForMap(epsd, map, false) >= 0) {
        // So be it.
        plyr->message = DEH_String(STSTR_CLEV);
        // [crisp] allow IDCLEV during demo playback and warp to the requested map
        if (g_doomstat_globals->demoplayback) {
          if (map > g_doomstat_globals->gamemap) {
            crispy->demowarp              = map;
            g_doomstat_globals->nodrawers = true;
            singletics                    = true;
            return true;
          } else {
            return false;
          }
        } else
          G_DeferedInitNew(g_doomstat_globals->gameskill, epsd, map);
        // [crispy] eat key press, i.e. don't change weapon upon level change
        return true;
      }
    }
    // [crispy] eat up the first digit typed after a cheat expecting two parameters
    else if (!g_doomstat_globals->netgame && cht_CheckCheat(&cheat_clev1, static_cast<char>(ev->data2)) && !g_doomstat_globals->menuactive) {
      char buf[2];

      cht_GetParam(&cheat_clev1, buf);

      return isdigit(buf[0]);
    }
  }
  return false;
}

int ST_calcPainOffset() {
  static int lastcalc  = 0;
  static int oldhealth = -1;

  int health = plyr->health > 100 ? 100 : plyr->health;

  if (health != oldhealth) {
    lastcalc  = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
    oldhealth = health;
  }
  return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
// [crispy] fix status bar face hysteresis
static int faceindex;
void       ST_updateFaceWidget() {
  static int lastattackdown = -1;
  static int priority       = 0;

  // [crispy] no evil grin or rampage face in god mode
  const bool invul = (plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability];

  // [crispy] fix status bar face hysteresis
  int painoffset = ST_calcPainOffset();

  if (priority < 10) {
    // dead
    if (!plyr->health) {
      priority     = 9;
      painoffset   = 0;
      faceindex    = ST_DEADFACE;
      st_facecount = 1;
    }
  }

  if (priority < 9) {
    if (plyr->bonuscount) {
      // picking up bonus
      bool doevilgrin = false;

      for (int i = 0; i < NUMWEAPONS; i++) {
        if (oldweaponsowned[i] != static_cast<bool>(plyr->weaponowned[i])) {
          doevilgrin         = true;
          oldweaponsowned[i] = plyr->weaponowned[i];
        }
      }
      // [crispy] no evil grin in god mode
      if (doevilgrin && !invul) {
        // evil grin if just picked up weapon
        priority     = 8;
        st_facecount = ST_EVILGRINCOUNT;
        faceindex    = ST_EVILGRINOFFSET;
      }
    }
  }

  if (priority < 8) {
    if (plyr->damagecount
        && plyr->attacker
        && plyr->attacker != plyr->mo) {
      // being attacked
      priority = 7;

      // [crispy] show "Ouch Face" as intended
      if (st_oldhealth - plyr->health > ST_MUCHPAIN) {
        // [crispy] raise "Ouch Face" priority
        priority     = 8;
        st_facecount = ST_TURNCOUNT;
        faceindex    = ST_OUCHOFFSET;
      } else {
        angle_t badguyangle = R_PointToAngle2(plyr->mo->x,
                                              plyr->mo->y,
                                              plyr->attacker->x,
                                              plyr->attacker->y);

        angle_t diffang = 0;
        int     i       = 0;
        if (badguyangle > plyr->mo->angle) {
          // whether right or left
          diffang = badguyangle - plyr->mo->angle;
          i       = diffang > ANG180;
        } else {
          // whether left or right
          diffang = plyr->mo->angle - badguyangle;
          i       = diffang <= ANG180;
        } // confusing, aint it?

        st_facecount = ST_TURNCOUNT;

        if (diffang < ANG45) {
          // head-on
          faceindex = ST_RAMPAGEOFFSET;
        } else if (i) {
          // turn face right
          faceindex = ST_TURNOFFSET;
        } else {
          // turn face left
          faceindex = ST_TURNOFFSET + 1;
        }
      }
    }
  }

  if (priority < 7) {
    // getting hurt because of your own damn stupidity
    if (plyr->damagecount) {
      // [crispy] show "Ouch Face" as intended
      if (st_oldhealth - plyr->health > ST_MUCHPAIN) {
        priority     = 7;
        st_facecount = ST_TURNCOUNT;
        faceindex    = ST_OUCHOFFSET;
      } else {
        priority     = 6;
        st_facecount = ST_TURNCOUNT;
        faceindex    = ST_RAMPAGEOFFSET;
      }
    }
  }

  if (priority < 6) {
    // rapid firing
    if (plyr->attackdown) {
      if (lastattackdown == -1)
        lastattackdown = ST_RAMPAGEDELAY;
      // [crispy] no rampage face in god mode
      else if (!--lastattackdown && !invul) {
        priority       = 5;
        faceindex      = ST_RAMPAGEOFFSET;
        st_facecount   = 1;
        lastattackdown = 1;
      }
    } else
      lastattackdown = -1;
  }

  if (priority < 5) {
    // invulnerability
    if (invul) {
      priority = 4;

      painoffset   = 0;
      faceindex    = ST_GODFACE;
      st_facecount = 1;
    }
  }

  // look left or look right if the facecount has timed out
  if (!st_facecount) {
    faceindex    = st_randomnumber % 3;
    st_facecount = ST_STRAIGHTFACECOUNT;
    priority     = 0;
  }

  st_facecount--;

  // [crispy] fix status bar face hysteresis
  st_faceindex = painoffset + faceindex;
}

void ST_updateWidgets() {
  static int largeammo = 1994; // means "n/a"

  // must redirect the pointer if the ready weapon has changed.
  //  if (w_ready.data != plyr->readyweapon)
  //  {
  if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    w_ready.num = &largeammo;
  else
    w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
  //{
  // static int tic=0;
  // static int dir=-1;
  // if (!(tic&15))
  //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
  // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
  //   dir = 1;
  // tic++;
  // }
  w_ready.data = plyr->readyweapon;

  // if (*w_ready.on)
  //  STlib_updateNum(&w_ready, true);
  // refresh weapon change
  //  }

  // update keycard multiple widgets
  for (int i = 0; i < 3; i++) {
    keyboxes[i] = plyr->cards[i] ? i : -1;

    if (plyr->cards[i + 3])
      keyboxes[i] = (keyboxes[i] == -1) ? i + 3 : i + 6; // [crispy] support combined card and skull keys

    // [crispy] blinking key or skull in the status bar
    if (plyr->tryopen[i]) {
#if defined(CRISPY_KEYBLINK_WITH_SOUND)
      if (!(plyr->tryopen[i] & (2 * KEYBLINKMASK - 1))) {
        S_StartSound(nullptr, sfx_itemup);
      }
#endif
#if defined(CRISPY_KEYBLINK_IN_CLASSIC_HUD)
      if (st_classicstatusbar && !(plyr->tryopen[i] & (KEYBLINKMASK - 1))) {
        st_firsttime = true;
      }
#endif
      plyr->tryopen[i]--;
#if !defined(CRISPY_KEYBLINK_IN_CLASSIC_HUD)
      if (st_crispyhud)
#endif
      {
        keyboxes[i] = (plyr->tryopen[i] & KEYBLINKMASK) ? i + g_p_local_globals->st_keyorskull[i] : -1;
      }

      if (!plyr->tryopen[i]) {
        w_keyboxes[i].oldinum = -1;
      }
    }
  }

  // refresh everything if this is him coming back to life
  ST_updateFaceWidget();

  // used by the w_armsbg widget
  st_notdeathmatch = !g_doomstat_globals->deathmatch;

  // used by w_arms[] widgets
  st_armson = st_statusbaron && !g_doomstat_globals->deathmatch;

  // used by w_frags widget
  st_fragson    = g_doomstat_globals->deathmatch && st_statusbaron;
  st_fragscount = 0;

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (i != g_doomstat_globals->consoleplayer)
      st_fragscount += plyr->frags[i];
    else
      st_fragscount -= plyr->frags[i];
  }

  // get rid of chat window if up because of message
  if (!--st_msgcounter)
    st_chat = st_oldchat;
}

void ST_Ticker() {

  st_clock++;
  st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = plyr->health;
}

static int st_palette = 0;

void ST_doPaletteStuff() {
  int cnt = plyr->damagecount;

  if (plyr->powers[pw_strength]) {
    // slowly fade the berzerk out
    int bzc = 12 - (plyr->powers[pw_strength] >> 6);

    if (bzc > cnt)
      cnt = bzc;
  }

  int palette = 0;
  if (cnt) {
    palette = (cnt + 7) >> 3;

    if (palette >= NUMREDPALS)
      palette = NUMREDPALS - 1;

    // [crispy] tune down a bit so the menu remains legible
    if (g_doomstat_globals->menuactive || g_doomstat_globals->paused)
      palette >>= 1;

    palette += STARTREDPALS;
  }

  else if (plyr->bonuscount && plyr->health > 0) // [crispy] never show the yellow bonus palette for a dead player
  {
    palette = (plyr->bonuscount + 7) >> 3;

    if (palette >= NUMBONUSPALS)
      palette = NUMBONUSPALS - 1;

    palette += STARTBONUSPALS;
  }

  else if (plyr->powers[pw_ironfeet] > 4 * 32
           || plyr->powers[pw_ironfeet] & 8)
    palette = RADIATIONPAL;
  else
    palette = 0;

  // In Chex Quest, the player never sees red.  Instead, the
  // radiation suit palette is used to tint the screen green,
  // as though the player is being covered in goo by an
  // attacking flemoid.

  if (g_doomstat_globals->gameversion == exe_chex
      && palette >= STARTREDPALS && palette < STARTREDPALS + NUMREDPALS) {
    palette = RADIATIONPAL;
  }

  // [crispy] prevent palette changes when in help screen or Crispness menu
  if (inhelpscreens) {
    palette = 0;
  }

  if (palette != st_palette) {
    st_palette = palette;
#ifndef CRISPY_TRUECOLOR
    uint8_t * pal = cache_lump_num<uint8_t *>(lu_palette, PU_CACHE) + palette * 768;
    I_SetPalette(pal);
#else
    I_SetPalette(palette);
#endif
  }
}

enum class hudcolor_t
{
  hudcolor_ammo,
  hudcolor_health,
  hudcolor_frags,
  hudcolor_armor
};

// [crispy] return ammo/health/armor widget color
static uint8_t * ST_WidgetColor(hudcolor_t i) {
  if (!(crispy->coloredhud & COLOREDHUD_BAR))
    return nullptr;

  switch (i) {
  case hudcolor_t::hudcolor_ammo: {
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo) {
      return nullptr;
    } else {
      int ammo     = plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
      int fullammo = g_p_local_globals->maxammo[weaponinfo[plyr->readyweapon].ammo];

      if (ammo < fullammo / 4)
        return cr_colors[static_cast<int>(cr_t::CR_RED)];
      else if (ammo < fullammo / 2)
        return cr_colors[static_cast<int>(cr_t::CR_GOLD)];
      else if (ammo <= fullammo)
        return cr_colors[static_cast<int>(cr_t::CR_GREEN)];
      else
        return cr_colors[static_cast<int>(cr_t::CR_BLUE)];
    }
    break;
  }
  case hudcolor_t::hudcolor_health: {
    int health = plyr->health;

    // [crispy] Invulnerability powerup and God Mode cheat turn Health values gray
    if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
      return cr_colors[static_cast<int>(cr_t::CR_GRAY)];
    else if (health < 25)
      return cr_colors[static_cast<int>(cr_t::CR_RED)];
    else if (health < 50)
      return cr_colors[static_cast<int>(cr_t::CR_GOLD)];
    else if (health <= 100)
      return cr_colors[static_cast<int>(cr_t::CR_GREEN)];
    else
      return cr_colors[static_cast<int>(cr_t::CR_BLUE)];

    break;
  }
  case hudcolor_t::hudcolor_frags: {
    int frags = st_fragscount;

    if (frags < 0)
      return cr_colors[static_cast<int>(cr_t::CR_RED)];
    else if (frags == 0)
      return cr_colors[static_cast<int>(cr_t::CR_GOLD)];
    else
      return cr_colors[static_cast<int>(cr_t::CR_GREEN)];

    break;
  }
  case hudcolor_t::hudcolor_armor: {
    // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
    if (plyr->cheats & CF_GODMODE || plyr->powers[pw_invulnerability])
      return cr_colors[static_cast<int>(cr_t::CR_GRAY)];
    // [crispy] color by armor type
    else if (plyr->armortype >= 2)
      return cr_colors[static_cast<int>(cr_t::CR_BLUE)];
    else if (plyr->armortype == 1)
      return cr_colors[static_cast<int>(cr_t::CR_GREEN)];
    else if (plyr->armortype == 0)
      return cr_colors[static_cast<int>(cr_t::CR_RED)];
    /*
        // [crispy] alternatively, color by armor points
        int armor = plyr->armorpoints;

        if (armor < 25)
            return cr[static_cast<int>(cr_t::CR_RED)];
        else if (armor < 50)
            return cr[static_cast<int>(cr_t::CR_GOLD)];
        else if (armor <= 100)
            return cr[static_cast<int>(cr_t::CR_GREEN)];
        else
            return cr[static_cast<int>(cr_t::CR_BLUE)];
*/
    break;
  }
  }

  return nullptr;
}

// [crispy] draw the gibbed death state frames in the Health widget
// in sync with the actual player sprite
static inline void ST_DrawGibbedPlayerSprites() {
  state_t const * state  = plyr->mo->state;
  spritedef_t *   sprdef = &g_r_state_globals->sprites[state->sprite];

  // [crispy] the TNT1 sprite is not supposed to be rendered anyway
  if (!sprdef->numframes && plyr->mo->sprite == SPR_TNT1) {
    return;
  }

  spriteframe_t * sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
  patch_t *       patch    = cache_lump_num<patch_t *>(sprframe->lump[0] + g_r_state_globals->firstspritelump, PU_CACHE);

  unsigned int flag_check = static_cast<unsigned int>(plyr->mo->flags) & MF_TRANSLATION;
  if (flag_check) {
    dp_translation = g_r_draw_globals->translationtables - 256 + (flag_check >> (MF_TRANSSHIFT - 8));
  }

  V_DrawPatch(ST_HEALTHX() - 17, 186, patch);
  dp_translation = nullptr;
}

void ST_drawWidgets(bool refresh) {
  bool gibbed = false;

  // used by w_arms[] widgets
  st_armson = st_statusbaron && !g_doomstat_globals->deathmatch;

  // used by w_frags widget
  st_fragson = g_doomstat_globals->deathmatch && st_statusbaron;

  dp_translation = ST_WidgetColor(hudcolor_t::hudcolor_ammo);
  STlib_updateNum(&w_ready, refresh);
  dp_translation = nullptr;

  // [crispy] draw "special widgets" in the Crispy HUD
  if (st_crispyhud) {
    // [crispy] draw berserk pack instead of no ammo if appropriate
    if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength]) {
      static int lump = -1;
      if (lump == -1) {
        lump = W_CheckNumForName(DEH_String("PSTRA0"));

        if (lump == -1) {
          lump = W_CheckNumForName(DEH_String("MEDIA0"));
        }
      }

      patch_t * patch = cache_lump_num<patch_t *>(lump, PU_CACHE);

      // [crispy] (23,179) is the center of the Ammo widget
      V_DrawPatch(ST_AMMOX() - 21 - SHORT(patch->width) / 2 + SHORT(patch->leftoffset),
                  179 - SHORT(patch->height) / 2 + SHORT(patch->topoffset),
                  patch);
    }

    // [crispy] draw the gibbed death state frames in the Health widget
    // in sync with the actual player sprite
    if (plyr->health <= 0 && plyr->mo->state - states >= mobjinfo[plyr->mo->type].xdeathstate) {
      ST_DrawGibbedPlayerSprites();
      gibbed = true;
    }
  }

  for (int i = 0; i < 4; i++) {
    STlib_updateNum(&w_ammo[i], refresh);
    STlib_updateNum(&w_maxammo[i], refresh);
  }

  if (!gibbed) {
    dp_translation = ST_WidgetColor(hudcolor_t::hudcolor_health);
    // [crispy] negative player health
    w_health.n.num = crispy->neghealth ? &plyr->neghealth : &plyr->health;
    STlib_updatePercent(&w_health, refresh);
  }
  dp_translation = ST_WidgetColor(hudcolor_t::hudcolor_armor);
  STlib_updatePercent(&w_armor, refresh);
  dp_translation = nullptr;

  STlib_updateBinIcon(&w_armsbg, refresh);

  // [crispy] show SSG availability in the Shotgun slot of the arms widget
  st_shotguns = plyr->weaponowned[wp_shotgun] | plyr->weaponowned[wp_supershotgun];

  for (auto & w_arm : w_arms)
    STlib_updateMultIcon(&w_arm, refresh);

  // [crispy] draw the actual face widget background
  if (st_crispyhud && screenblocks == CRISPY_HUD) {
    V_CopyRect(ST_FX + DELTAWIDTH, 1, g_st_stuff_globals->st_backing_screen, SHORT(faceback->width), ST_HEIGHT - 1, ST_FX + DELTAWIDTH, ST_Y + 1);
  }

  STlib_updateMultIcon(&w_faces, refresh);

  for (auto & w_keyboxe : w_keyboxes)
    STlib_updateMultIcon(&w_keyboxe, refresh);

  dp_translation = ST_WidgetColor(hudcolor_t::hudcolor_frags);
  STlib_updateNum(&w_frags, refresh);

  dp_translation = nullptr;
}

void ST_doRefresh() {

  st_firsttime = false;

  // draw status bar background to off-screen buff
  ST_refreshBackground(false);

  // and refresh all widgets
  ST_drawWidgets(true);
}

void ST_diffDraw() {
  // update all widgets
  ST_drawWidgets(false);
}

void ST_Drawer(bool fullscreen_param, bool refresh) {

  st_statusbaron = (!fullscreen_param) || (g_doomstat_globals->automapactive && !crispy->automapoverlay && !crispy->widescreen);
  // [crispy] immediately redraw status bar after help screens have been shown
  st_firsttime = st_firsttime || refresh || inhelpscreens;

  // [crispy] distinguish classic status bar with background and player face from Crispy HUD
  st_crispyhud        = screenblocks >= CRISPY_HUD && (!g_doomstat_globals->automapactive || crispy->automapoverlay);
  st_classicstatusbar = st_statusbaron && !st_crispyhud && !crispy->widescreen;
  st_statusbarface    = st_classicstatusbar || (st_crispyhud && screenblocks == CRISPY_HUD);

  if (crispy->cleanscreenshot == 2)
    return;

  // Do red-/gold-shifts from damage/items
  ST_doPaletteStuff();

  // [crispy] translucent HUD
  if (st_crispyhud && screenblocks > CRISPY_HUD + 1)
    dp_translucent = true;

  // If just after ST_Start(), refresh all
  if (st_firsttime) ST_doRefresh();
  // Otherwise, update as little as possible
  else
    ST_diffDraw();

  dp_translucent = false;
}

using load_callback_t = void (*)(const char *, patch_t **);

// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.

static void ST_loadUnloadGraphics(load_callback_t callback) {
  char namebuf[9];

  // Load the numbers, tall and short
  for (int i = 0; i < 10; i++) {
    DEH_snprintf(namebuf, 9, "STTNUM%d", i);
    callback(namebuf, &tallnum[i]);

    DEH_snprintf(namebuf, 9, "STYSNUM%d", i);
    callback(namebuf, &shortnum[i]);
  }

  // Load percent key.
  // Note: why not load STMINUS here, too?

  callback(DEH_String("STTPRCNT"), &tallpercent);

  // key cards
  for (int i = 0; i < NUMCARDS; i++) {
    DEH_snprintf(namebuf, 9, "STKEYS%d", i);
    callback(namebuf, &keys[i]);
  }

  // arms background
  callback(DEH_String("STARMS"), &armsbg);

  // arms ownership widgets
  for (int i = 0; i < 6; i++) {
    DEH_snprintf(namebuf, 9, "STGNUM%d", i + 2);

    // gray #
    callback(namebuf, &arms[i][0]);

    // yellow #
    arms[i][1] = shortnum[i + 2];
  }

  // face backgrounds for different color players
  DEH_snprintf(namebuf, 9, "STFB%d", g_doomstat_globals->consoleplayer);
  callback(namebuf, &faceback);

  // status bar background bits
  if (W_CheckNumForName("STBAR") >= 0) {
    callback(DEH_String("STBAR"), &sbar);
    sbarr = nullptr;
  } else {
    callback(DEH_String("STMBARL"), &sbar);
    callback(DEH_String("STMBARR"), &sbarr);
  }

  // face states
  int facenum = 0;
  for (int i = 0; i < ST_NUMPAINFACES; i++) {
    for (int j = 0; j < ST_NUMSTRAIGHTFACES; j++) {
      DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
      callback(namebuf, &faces[facenum]);
      ++facenum;
    }
    DEH_snprintf(namebuf, 9, "STFTR%d0", i); // turn right
    callback(namebuf, &faces[facenum]);
    ++facenum;
    DEH_snprintf(namebuf, 9, "STFTL%d0", i); // turn left
    callback(namebuf, &faces[facenum]);
    ++facenum;
    DEH_snprintf(namebuf, 9, "STFOUCH%d", i); // ouch!
    callback(namebuf, &faces[facenum]);
    ++facenum;
    DEH_snprintf(namebuf, 9, "STFEVL%d", i); // evil grin ;)
    callback(namebuf, &faces[facenum]);
    ++facenum;
    DEH_snprintf(namebuf, 9, "STFKILL%d", i); // pissed off
    callback(namebuf, &faces[facenum]);
    ++facenum;
  }

  callback(DEH_String("STFGOD0"), &faces[facenum]);
  ++facenum;
  callback(DEH_String("STFDEAD0"), &faces[facenum]);
  ++facenum;
}

static void ST_loadCallback(const char * lumpname, patch_t ** variable) {
  *variable = cache_lump_name<patch_t *>(lumpname, PU_STATIC);
}

void ST_loadGraphics() {
  ST_loadUnloadGraphics(ST_loadCallback);
}

void ST_loadData() {
  lu_palette = W_GetNumForName(DEH_String("PLAYPAL"));
  ST_loadGraphics();

  // [crispy] support combined card and skull keys (if provided by PWAD)
  // i.e. only for display in the status bar
  for (int i = NUMCARDS; i < NUMCARDS + 3; i++) {
    char lumpname[9];

    DEH_snprintf(lumpname, 9, "STKEYS%d", i);
    int lumpnum = W_CheckNumForName(lumpname);

    keys[i] = static_cast<patch_t *>(
        (lumpnum != -1) ? cache_lump_num<patch_t *>(lumpnum, PU_STATIC) : keys[i - 3]);
  }
}

static void ST_unloadCallback(const char * lumpname, patch_t ** variable) {
  W_ReleaseLumpName(lumpname);
  *variable = nullptr;
}

void ST_unloadGraphics() {
  ST_loadUnloadGraphics(ST_unloadCallback);
}

[[maybe_unused]] void ST_unloadData() {
  ST_unloadGraphics();
}

void ST_initData() {
  st_firsttime = true;
  plyr         = &g_doomstat_globals->players[g_doomstat_globals->consoleplayer];

  st_clock     = 0;
  st_chatstate = StartChatState;
  st_gamestate = FirstPersonState;

  st_statusbaron = true;
  st_oldchat = st_chat = false;
  st_cursoron          = false;

  faceindex    = 0; // [crispy] fix status bar face hysteresis across level changes
  st_faceindex = 0;
  st_palette   = -1;

  st_oldhealth = -1;

  for (int i = 0; i < NUMWEAPONS; i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

  for (int & keyboxe : keyboxes)
    keyboxe = -1;

  STlib_init();
}

void ST_createWidgets() {
  // Location and size of statistics,
  //  justified according to widget type.
  // Problem is, within which space? STbar? Screen?
  // Note: this could be read in by a lump.
  //       Problem is, is the stuff rendered
  //       into a buffer,
  //       or into the frame buffer?

  // AMMO number pos.
  constexpr auto ST_AMMOWIDTH = 3;
  constexpr auto ST_AMMOY     = 171;

  // HEALTH number pos.
  constexpr auto ST_HEALTHY = 171;

  // Weapon pos.
  auto           ST_ARMSX      = (111 - HORIZDELTA());
  constexpr auto ST_ARMSY      = 172;
  constexpr auto ST_ARMSBGY    = 168;
  constexpr auto ST_ARMSXSPACE = 12;
  constexpr auto ST_ARMSYSPACE = 10;

  // Frags pos.
  auto           ST_FRAGSX     = (138 - HORIZDELTA());
  constexpr auto ST_FRAGSY     = 171;
  constexpr auto ST_FRAGSWIDTH = 2;

  // ARMOR number pos.
  auto           ST_ARMORX = (221 + HORIZDELTA());
  constexpr auto ST_ARMORY = 171;

  // Key icon positions.
  auto           ST_KEY0X = (239 + HORIZDELTA());
  constexpr auto ST_KEY0Y = 171;
  auto           ST_KEY1X = (239 + HORIZDELTA());
  constexpr auto ST_KEY1Y = 181;
  auto           ST_KEY2X = (239 + HORIZDELTA());
  constexpr auto ST_KEY2Y = 191;

  // Ammunition counter.
  constexpr auto ST_AMMO0WIDTH = 3;
  auto           ST_AMMO0X     = (288 + HORIZDELTA());
  constexpr auto ST_AMMO0Y     = 173;
  constexpr auto ST_AMMO1WIDTH = ST_AMMO0WIDTH;
  auto           ST_AMMO1X     = (288 + HORIZDELTA());
  constexpr auto ST_AMMO1Y     = 179;
  constexpr auto ST_AMMO2WIDTH = ST_AMMO0WIDTH;
  auto           ST_AMMO2X     = (288 + HORIZDELTA());
  constexpr auto ST_AMMO2Y     = 191;
  constexpr auto ST_AMMO3WIDTH = ST_AMMO0WIDTH;
  auto           ST_AMMO3X     = (288 + HORIZDELTA());
  constexpr auto ST_AMMO3Y     = 185;

  // Indicate maximum ammunition.
  // Only needed because backpack exists.
  constexpr auto ST_MAXAMMO0WIDTH = 3;
  auto           ST_MAXAMMO0X     = (314 + HORIZDELTA());
  constexpr auto ST_MAXAMMO0Y     = 173;
  constexpr auto ST_MAXAMMO1WIDTH = ST_MAXAMMO0WIDTH;
  auto           ST_MAXAMMO1X     = (314 + HORIZDELTA());
  constexpr auto ST_MAXAMMO1Y     = 179;
  constexpr auto ST_MAXAMMO2WIDTH = ST_MAXAMMO0WIDTH;
  auto           ST_MAXAMMO2X     = (314 + HORIZDELTA());
  constexpr auto ST_MAXAMMO2Y     = 191;
  constexpr auto ST_MAXAMMO3WIDTH = ST_MAXAMMO0WIDTH;
  auto           ST_MAXAMMO3X     = (314 + HORIZDELTA());
  constexpr auto ST_MAXAMMO3Y     = 185;

  // [crispy] re-calculate DELTAWIDTH
  I_GetScreenDimensions();

  // ready weapon ammo
  STlib_initNum(&w_ready,
                ST_AMMOX(),
                ST_AMMOY,
                tallnum.data(),
                &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                &st_statusbaron,
                ST_AMMOWIDTH);

  // the last weapon type
  w_ready.data = plyr->readyweapon;

  // health percentage
  STlib_initPercent(&w_health,
                    ST_HEALTHX(),
                    ST_HEALTHY,
                    tallnum.data(),
                    &plyr->health,
                    &st_statusbaron,
                    tallpercent);

  // arms background
  STlib_initBinIcon(&w_armsbg,
                    ST_ARMSBGX(),
                    ST_ARMSBGY,
                    armsbg,
                    &st_notdeathmatch,
                    &st_classicstatusbar);

  // weapons owned
  for (int i = 0; i < 6; i++) {
    STlib_initMultIcon(&w_arms[i],
                       ST_ARMSX + (i % 3) * ST_ARMSXSPACE,
                       ST_ARMSY + (i / 3) * ST_ARMSYSPACE,
                       arms[i],
                       &plyr->weaponowned[i + 1],
                       &st_armson);
  }
  // [crispy] show SSG availability in the Shotgun slot of the arms widget
  w_arms[1].inum = &st_shotguns;

  // frags sum
  STlib_initNum(&w_frags,
                ST_FRAGSX,
                ST_FRAGSY,
                tallnum.data(),
                &st_fragscount,
                &st_fragson,
                ST_FRAGSWIDTH);

  // faces
  STlib_initMultIcon(&w_faces,
                     ST_FACESX,
                     ST_FACESY,
                     faces.data(),
                     &st_faceindex,
                     &st_statusbarface);

  // armor percentage - should be colored later
  STlib_initPercent(&w_armor,
                    ST_ARMORX,
                    ST_ARMORY,
                    tallnum.data(),
                    &plyr->armorpoints,
                    &st_statusbaron,
                    tallpercent);

  // keyboxes 0-2
  STlib_initMultIcon(&w_keyboxes[0],
                     ST_KEY0X,
                     ST_KEY0Y,
                     keys.data(),
                     &keyboxes[0],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[1],
                     ST_KEY1X,
                     ST_KEY1Y,
                     keys.data(),
                     &keyboxes[1],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[2],
                     ST_KEY2X,
                     ST_KEY2Y,
                     keys.data(),
                     &keyboxes[2],
                     &st_statusbaron);

  // ammo count (all four kinds)
  STlib_initNum(&w_ammo[0],
                ST_AMMO0X,
                ST_AMMO0Y,
                shortnum.data(),
                &plyr->ammo[0],
                &st_statusbaron,
                ST_AMMO0WIDTH);

  STlib_initNum(&w_ammo[1],
                ST_AMMO1X,
                ST_AMMO1Y,
                shortnum.data(),
                &plyr->ammo[1],
                &st_statusbaron,
                ST_AMMO1WIDTH);

  STlib_initNum(&w_ammo[2],
                ST_AMMO2X,
                ST_AMMO2Y,
                shortnum.data(),
                &plyr->ammo[2],
                &st_statusbaron,
                ST_AMMO2WIDTH);

  STlib_initNum(&w_ammo[3],
                ST_AMMO3X,
                ST_AMMO3Y,
                shortnum.data(),
                &plyr->ammo[3],
                &st_statusbaron,
                ST_AMMO3WIDTH);

  // max ammo count (all four kinds)
  STlib_initNum(&w_maxammo[0],
                ST_MAXAMMO0X,
                ST_MAXAMMO0Y,
                shortnum.data(),
                &plyr->maxammo[0],
                &st_statusbaron,
                ST_MAXAMMO0WIDTH);

  STlib_initNum(&w_maxammo[1],
                ST_MAXAMMO1X,
                ST_MAXAMMO1Y,
                shortnum.data(),
                &plyr->maxammo[1],
                &st_statusbaron,
                ST_MAXAMMO1WIDTH);

  STlib_initNum(&w_maxammo[2],
                ST_MAXAMMO2X,
                ST_MAXAMMO2Y,
                shortnum.data(),
                &plyr->maxammo[2],
                &st_statusbaron,
                ST_MAXAMMO2WIDTH);

  STlib_initNum(&w_maxammo[3],
                ST_MAXAMMO3X,
                ST_MAXAMMO3Y,
                shortnum.data(),
                &plyr->maxammo[3],
                &st_statusbaron,
                ST_MAXAMMO3WIDTH);
}

static bool st_stopped = true;

void ST_Start() {

  if (!st_stopped)
    ST_Stop();

  ST_initData();
  ST_createWidgets();
  st_stopped = false;

  // [crispy] correctly color the status bar face background in multiplayer
  // demos recorded by another player than player 1
  if (g_doomstat_globals->netgame && g_doomstat_globals->consoleplayer) {
    char namebuf[8];

    DEH_snprintf(namebuf, 7, "STFB%d", g_doomstat_globals->consoleplayer);
    faceback = cache_lump_name<patch_t *>(namebuf, PU_STATIC);
  }
}

void ST_Stop() {
  if (st_stopped)
    return;

#ifndef CRISPY_TRUECOLOR
  I_SetPalette(cache_lump_num<uint8_t *>(lu_palette, PU_CACHE));
#else
  I_SetPalette(0);
#endif

  st_stopped = true;
}

void ST_Init() {
  // [crispy] colorize the confusing 'behold' power-up menu
  if (!DEH_HasStringReplacement(STSTR_BEHOLD) && !M_ParmExists("-nodeh")) {
    char str_behold[80];
    M_snprintf(str_behold, sizeof(str_behold), "in%sV%suln, %sS%str, %sI%snviso, %sR%sad, %sA%sllmap, or %sL%site-amp", crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)], crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)], crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)], crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)], crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)], crstr[static_cast<int>(cr_t::CR_GOLD)], crstr[static_cast<int>(cr_t::CR_NONE)]);
    DEH_AddStringReplacement(STSTR_BEHOLD, str_behold);
  }

  ST_loadData();
  g_st_stuff_globals->st_backing_screen = zmalloc<pixel_t *>(MAXWIDTH * (ST_HEIGHT << 1) * sizeof(*g_st_stuff_globals->st_backing_screen), PU_STATIC, 0);
}

// [crispy] Demo Timer widget
void ST_DrawDemoTimer(const int time) {
  char        buffer[16];
  const int   mins = time / (60 * TICRATE);
  const float secs = static_cast<float>(time % (60 * TICRATE)) / TICRATE;
  const int   w    = shortnum[0]->width;

  int n = M_snprintf(buffer, sizeof(buffer), "%02i %05.02f", mins, secs);

  int x = (viewwindowx >> crispy->hires) + (g_r_state_globals->scaledviewwidth >> crispy->hires) - DELTAWIDTH;

  // [crispy] draw the Demo Timer widget with gray numbers
  dp_translation = cr_colors[static_cast<int>(cr_t::CR_GRAY)];
  dp_translucent = (g_doomstat_globals->gamestate == GS_LEVEL);

  while (n-- > 0) {
    const int c = buffer[n] - '0';

    x -= w;

    if (c >= 0 && c <= 9) {
      V_DrawPatch(x, viewwindowy >> crispy->hires, shortnum[c]);
    }
  }

  dp_translation = nullptr;
  dp_translucent = false;
}

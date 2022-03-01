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
// DESCRIPTION:  none
//

#include <array>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "doomdef.hpp"
#include "doomstat.hpp"

#include "deh_bexpars.hpp" // [crispy] bex_pars[]
#include "deh_main.hpp"
#include "deh_misc.hpp"

#include "f_finale.hpp"
#include "i_input.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_argv.hpp"
#include "m_controls.hpp"
#include "m_menu.hpp"
#include "m_misc.hpp"
#include "m_random.hpp"
#include "z_zone.hpp"

#include "p_extsaveg.hpp"
#include "p_saveg.hpp"
#include "p_setup.hpp"
#include "p_tick.hpp"

#include "d_main.hpp"

#include "am_map.hpp"
#include "hu_stuff.hpp"
#include "st_stuff.hpp"
#include "statdump.hpp"
#include "wi_stuff.hpp"

// Needs access to LFB.
#include "v_video.hpp"

#include "w_wad.hpp"

#include "p_local.hpp"

#include "s_sound.hpp"

// Data.
#include "dstrings.hpp"
#include "sounds.hpp"

// SKY handling - still the wrong place.
#include "r_data.hpp"
#include "r_sky.hpp"

#include "g_game.hpp"
#include "lump.hpp"
#include "memory.hpp"
#include "v_trans.hpp" // [crispy] colored "always run" message

[[maybe_unused]] constexpr auto SAVEGAMESIZE = 0x2c000;

void G_ReadDemoTiccmd(ticcmd_t * cmd);
void G_WriteDemoTiccmd(ticcmd_t * cmd);
void G_PlayerReborn(int player);

void G_DoReborn(int playernum);

void                  G_DoLoadLevel();
void                  G_DoNewGame();
void                  G_DoPlayDemo();
void                  G_DoCompleted();
[[maybe_unused]] void G_DoVictory();
void                  G_DoWorldDone();
void                  G_DoSaveGame();

// Gamestate the last time G_Ticker was called.

gamestate_t oldgamestate;

gameaction_t gameaction;

// If non-zero, exit the level after this number of minutes.

bool sendpause; // send a pause event next tic
bool sendsave;  // send a save event next tic

bool timingdemo; // if true, exit with report on completion
int  starttime;  // for comparative timing purposes

bool turbodetected[MAXPLAYERS];

[[maybe_unused]] int demostarttic; // [crispy] fix revenant internal demo bug

char *    demoname;
char *    orig_demoname; // [crispy] the name originally chosen for the demo, i.e. without "-00000"
bool      longtics;      // cph's doom 1.91 longtics hack
bool      netdemo;
uint8_t * demobuffer;
uint8_t * demo_p;
uint8_t * demoend;

uint8_t consistancy[MAXPLAYERS][BACKUPTICS];

constexpr auto TURBOTHRESHOLD = 0x32;

fixed_t forwardmove[2] = { 0x19, 0x32 };
fixed_t sidemove[2]    = { 0x18, 0x28 };
fixed_t angleturn[3]   = { 640, 1280, 320 }; // + slow turn

static int * weapon_keys[] = {
  &g_m_controls_globals->key_weapon1,
  &g_m_controls_globals->key_weapon2,
  &g_m_controls_globals->key_weapon3,
  &g_m_controls_globals->key_weapon4,
  &g_m_controls_globals->key_weapon5,
  &g_m_controls_globals->key_weapon6,
  &g_m_controls_globals->key_weapon7,
  &g_m_controls_globals->key_weapon8
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;

// Used for prev/next weapon keys.

static const struct
{
  weapontype_t weapon;
  weapontype_t weapon_num;
} weapon_order_table[] = {
  {wp_fist,          wp_fist    },
  { wp_chainsaw,     wp_fist    },
  { wp_pistol,       wp_pistol  },
  { wp_shotgun,      wp_shotgun },
  { wp_supershotgun, wp_shotgun },
  { wp_chaingun,     wp_chaingun},
  { wp_missile,      wp_missile },
  { wp_plasma,       wp_plasma  },
  { wp_bfg,          wp_bfg     }
};

constexpr auto SLOWTURNTICS    = 6;
constexpr auto NUMKEYS         = 256;
constexpr auto MAX_JOY_BUTTONS = 20;

static bool gamekeydown[NUMKEYS];
static int  turnheld; // for accelerative turning
static int  lookheld; // [crispy] for accelerative looking

static bool   mousearray[MAX_MOUSE_BUTTONS + 1];
static bool * mousebuttons = &mousearray[1]; // allow [-1]

// mouse values are used once
int mousex;
int mousex2;
int mousey;

static int  dclicktime;
static bool dclickstate;
static int  dclicks;
static int  dclicktime2;
static bool dclickstate2;
static int  dclicks2;

// joystick values are repeated
static int    joyxmove;
static int    joyymove;
static int    joystrafemove;
static int    joylook; // [crispy]
static bool   joyarray[MAX_JOY_BUTTONS + 1];
static bool * joybuttons = &joyarray[1]; // allow [-1]

static char savename[256]; // [crispy] moved here, made static
static int  savegameslot;
static char savedescription[32];

constexpr auto BODYQUESIZE = 32;

mobj_t * bodyque[BODYQUESIZE];

int vanilla_savegame_limit = 1;
int vanilla_demo_limit     = 1;

int G_CmdChecksum(ticcmd_t * cmd) {
  int sum = 0;

  for (size_t i = 0; i < sizeof(*cmd) / 4 - 1; i++)
    sum += (reinterpret_cast<int *>(cmd))[i];

  return sum;
}

static bool WeaponSelectable(weapontype_t weapon) {
  // Can't select the super shotgun in Doom 1.

  if (weapon == wp_supershotgun && !crispy->havessg) {
    return false;
  }

  // These weapons aren't available in shareware.

  if ((weapon == wp_plasma || weapon == wp_bfg)
      && g_doomstat_globals->gamemission == doom && g_doomstat_globals->gamemode == shareware) {
    return false;
  }

  // Can't select a weapon if we don't own it.

  if (!g_doomstat_globals->players[g_doomstat_globals->consoleplayer].weaponowned[weapon]) {
    return false;
  }

  // Can't select the fist if we have the chainsaw, unless
  // we also have the berserk pack.

  if (weapon == wp_fist
      && g_doomstat_globals->players[g_doomstat_globals->consoleplayer].weaponowned[wp_chainsaw]
      && !g_doomstat_globals->players[g_doomstat_globals->consoleplayer].powers[pw_strength]) {
    return false;
  }

  return true;
}

static int G_NextWeapon(int direction) {
  weapontype_t weapon;

  // Find index in the table.

  if (g_doomstat_globals->players[g_doomstat_globals->consoleplayer].pendingweapon == wp_nochange) {
    weapon = g_doomstat_globals->players[g_doomstat_globals->consoleplayer].readyweapon;
  } else {
    weapon = g_doomstat_globals->players[g_doomstat_globals->consoleplayer].pendingweapon;
  }

  size_t index = 0;
  for (index = 0; index < std::size(weapon_order_table); ++index) {
    if (weapon_order_table[index].weapon == weapon) {
      break;
    }
  }

  // Switch weapon. Don't loop forever.
  size_t start_i = index;
  do {
    index += static_cast<unsigned long>(direction);
    index = (index + std::size(weapon_order_table)) % std::size(weapon_order_table);
  } while (index != start_i && !WeaponSelectable(weapon_order_table[index].weapon));

  return weapon_order_table[index].weapon_num;
}

// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, clean screenshots, resurrection from savegames
bool speedkeydown() {
  return (g_m_controls_globals->key_speed < NUMKEYS && gamekeydown[g_m_controls_globals->key_speed]) || (g_m_controls_globals->joybspeed < MAX_JOY_BUTTONS && joybuttons[g_m_controls_globals->joybspeed]);
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
void G_BuildTiccmd(ticcmd_t * cmd, int maketic) {
  int              lspeed;
  int              side;
  player_t * const player = &g_doomstat_globals->players[g_doomstat_globals->consoleplayer];
  static char      playermessage[48];

  std::memset(cmd, 0, sizeof(ticcmd_t));

  cmd->consistancy =
      consistancy[g_doomstat_globals->consoleplayer][maketic % BACKUPTICS];

  bool strafe = gamekeydown[g_m_controls_globals->key_strafe] || mousebuttons[g_m_controls_globals->mousebstrafe] || joybuttons[g_m_controls_globals->joybstrafe];

  // fraggle: support the old "joyb_speed = 31" hack which
  // allowed an autorun effect

  // [crispy] when "always run" is active,
  // pressing the "run" key will result in walking
  int speed = g_m_controls_globals->key_speed >= NUMKEYS || g_m_controls_globals->joybspeed >= MAX_JOY_BUTTONS;
  speed ^= static_cast<int>(speedkeydown());

  int look    = 0;
  int forward = side = 0;

  // use two stage accelerative turning
  // on the keyboard and joystick
  if (joyxmove < 0
      || joyxmove > 0
      || gamekeydown[g_m_controls_globals->key_right]
      || gamekeydown[g_m_controls_globals->key_left])
    turnheld += ticdup;
  else
    turnheld = 0;

  int tspeed;
  if (turnheld < SLOWTURNTICS)
    tspeed = 2; // slow turn
  else
    tspeed = speed;

  // [crispy] use two stage accelerative looking
  if (gamekeydown[g_m_controls_globals->key_lookdown] || gamekeydown[g_m_controls_globals->key_lookup]) {
    lookheld += ticdup;
  } else {
    lookheld = 0;
  }
  if (lookheld < SLOWTURNTICS) {
    lspeed = 1;
  } else {
    lspeed = 2;
  }

  // [crispy] add quick 180° reverse
  if (gamekeydown[g_m_controls_globals->key_reverse] || mousebuttons[g_m_controls_globals->mousebreverse]) {
    cmd->angleturn                                    = static_cast<short>(cmd->angleturn + (ANG180 >> FRACBITS));
    gamekeydown[g_m_controls_globals->key_reverse]    = false;
    mousebuttons[g_m_controls_globals->mousebreverse] = false;
  }

  // [crispy] toggle "always run"
  if (gamekeydown[g_m_controls_globals->key_toggleautorun]) {
    static int joybspeed_old = 2;

    if (g_m_controls_globals->joybspeed >= MAX_JOY_BUTTONS) {
      g_m_controls_globals->joybspeed = joybspeed_old;
    } else {
      joybspeed_old                   = g_m_controls_globals->joybspeed;
      g_m_controls_globals->joybspeed = 29;
    }

    M_snprintf(playermessage, sizeof(playermessage), "ALWAYS RUN %s%s", crstr[static_cast<int>(cr_t::CR_GREEN)], (g_m_controls_globals->joybspeed >= MAX_JOY_BUTTONS) ? "ON" : "OFF");
    player->message = playermessage;
    S_StartSound(nullptr, sfx_swtchn);

    gamekeydown[g_m_controls_globals->key_toggleautorun] = false;
  }

  // [crispy] Toggle vertical mouse movement
  if (gamekeydown[g_m_controls_globals->key_togglenovert]) {
    novert = !novert;

    M_snprintf(playermessage, sizeof(playermessage), "vertical mouse movement %s%s", crstr[static_cast<int>(cr_t::CR_GREEN)], !novert ? "ON" : "OFF");
    player->message = playermessage;
    S_StartSound(nullptr, sfx_swtchn);

    gamekeydown[g_m_controls_globals->key_togglenovert] = false;
  }

  // [crispy] extra high precision IDMYPOS variant, updates for 10 seconds
  if (player->powers[pw_mapcoords]) {
    M_snprintf(playermessage, sizeof(playermessage), "X=%.10f Y=%.10f A=%d", static_cast<double>(player->mo->x) / FRACUNIT, static_cast<double>(player->mo->y) / FRACUNIT, player->mo->angle >> 24);
    player->message = playermessage;

    player->powers[pw_mapcoords]--;

    // [crispy] discard instead of going static
    if (!player->powers[pw_mapcoords]) {
      player->message = "";
    }
  }

  // let movement keys cancel each other out
  if (strafe) {
    if (gamekeydown[g_m_controls_globals->key_right]) {
      // fmt::fprintf(stderr, "strafe right\n");
      side += sidemove[speed];
    }
    if (gamekeydown[g_m_controls_globals->key_left]) {
      //	fprintf(stderr, "strafe left\n");
      side -= sidemove[speed];
    }
    if (joyxmove > 0)
      side += sidemove[speed];
    if (joyxmove < 0)
      side -= sidemove[speed];
  } else {
    if (gamekeydown[g_m_controls_globals->key_right])
      cmd->angleturn = static_cast<short>(cmd->angleturn - angleturn[tspeed]);
    if (gamekeydown[g_m_controls_globals->key_left])
      cmd->angleturn = static_cast<short>(cmd->angleturn + angleturn[tspeed]);
    if (joyxmove > 0)
      cmd->angleturn = static_cast<short>(cmd->angleturn - angleturn[tspeed]);
    if (joyxmove < 0)
      cmd->angleturn = static_cast<short>(cmd->angleturn + angleturn[tspeed]);
  }

  if (gamekeydown[g_m_controls_globals->key_up] || gamekeydown[g_m_controls_globals->key_alt_up]) // [crispy] add key_alt_*
  {
    // fmt::fprintf(stderr, "up\n");
    forward += forwardmove[speed];
  }
  if (gamekeydown[g_m_controls_globals->key_down] || gamekeydown[g_m_controls_globals->key_alt_down]) // [crispy] add key_alt_*
  {
    // fmt::fprintf(stderr, "down\n");
    forward -= forwardmove[speed];
  }

  if (joyymove < 0)
    forward += forwardmove[speed];
  if (joyymove > 0)
    forward -= forwardmove[speed];

  if (gamekeydown[g_m_controls_globals->key_strafeleft] || gamekeydown[g_m_controls_globals->key_alt_strafeleft] // [crispy] add key_alt_*
      || joybuttons[g_m_controls_globals->joybstrafeleft]
      || mousebuttons[g_m_controls_globals->mousebstrafeleft]
      || joystrafemove < 0) {
    side -= sidemove[speed];
  }

  if (gamekeydown[g_m_controls_globals->key_straferight] || gamekeydown[g_m_controls_globals->key_alt_straferight] // [crispy] add key_alt_*
      || joybuttons[g_m_controls_globals->joybstraferight]
      || mousebuttons[g_m_controls_globals->mousebstraferight]
      || joystrafemove > 0) {
    side += sidemove[speed];
  }

  // [crispy] look up/down/center keys
  if (crispy->freelook) {
    static unsigned int kbdlookctrl = 0;

    if (gamekeydown[g_m_controls_globals->key_lookup] || joylook < 0) {
      look = lspeed;
      kbdlookctrl += static_cast<unsigned int>(ticdup);
    } else if (gamekeydown[g_m_controls_globals->key_lookdown] || joylook > 0) {
      look = -lspeed;
      kbdlookctrl += static_cast<unsigned int>(ticdup);
    } else
      // [crispy] keyboard lookspring
      if (gamekeydown[g_m_controls_globals->key_lookcenter] || (crispy->freelook == FREELOOK_SPRING && kbdlookctrl)) {
        look        = TOCENTER;
        kbdlookctrl = 0;
      }
  }

  // [crispy] jump keys
  if (critical->jump) {
    if (gamekeydown[g_m_controls_globals->key_jump] || mousebuttons[g_m_controls_globals->mousebjump]
        || joybuttons[g_m_controls_globals->joybjump]) {
      cmd->arti |= AFLAG_JUMP;
    }
  }

  // buttons
  cmd->chatchar = static_cast<uint8_t>(HU_dequeueChatChar());

  if (gamekeydown[g_m_controls_globals->key_fire] || mousebuttons[g_m_controls_globals->mousebfire]
      || joybuttons[g_m_controls_globals->joybfire])
    cmd->buttons |= BT_ATTACK;

  if (gamekeydown[g_m_controls_globals->key_use]
      || joybuttons[g_m_controls_globals->joybuse]
      || mousebuttons[g_m_controls_globals->mousebuse]) {
    cmd->buttons |= BT_USE;
    // clear double clicks if hit use button
    dclicks = 0;
  }

  // If the previous or next weapon button is pressed, the
  // next_weapon variable is set to change weapons when
  // we generate a ticcmd.  Choose a new weapon.

  if (g_doomstat_globals->gamestate == GS_LEVEL && next_weapon != 0) {
    auto i = static_cast<size_t>(G_NextWeapon(next_weapon));
    cmd->buttons |= BT_CHANGE;
    cmd->buttons |= static_cast<uint8_t>(i << BT_WEAPONSHIFT);
  } else {
    // Check weapon keys.
    for (size_t i = 0; i < std::size(weapon_keys); i++) {
      int key = *weapon_keys[i];

      if (gamekeydown[key]) {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= static_cast<uint8_t>(i << BT_WEAPONSHIFT);
        break;
      }
    }
  }

  next_weapon = 0;

  // mouse
  if (mousebuttons[g_m_controls_globals->mousebforward]) {
    forward += forwardmove[speed];
  }
  if (mousebuttons[g_m_controls_globals->mousebbackward]) {
    forward -= forwardmove[speed];
  }

  if (g_m_controls_globals->dclick_use) {
    // forward double click
    if (mousebuttons[g_m_controls_globals->mousebforward] != dclickstate && dclicktime > 1) {
      dclickstate = mousebuttons[g_m_controls_globals->mousebforward];
      if (dclickstate)
        dclicks++;
      if (dclicks == 2) {
        cmd->buttons |= BT_USE;
        dclicks = 0;
      } else
        dclicktime = 0;
    } else {
      dclicktime += ticdup;
      if (dclicktime > 20) {
        dclicks     = 0;
        dclickstate = false;
      }
    }

    // strafe double click
    bool bstrafe =
        mousebuttons[g_m_controls_globals->mousebstrafe]
        || joybuttons[g_m_controls_globals->joybstrafe];
    if (bstrafe != dclickstate2 && dclicktime2 > 1) {
      dclickstate2 = bstrafe;
      if (dclickstate2)
        dclicks2++;
      if (dclicks2 == 2) {
        cmd->buttons |= BT_USE;
        dclicks2 = 0;
      } else
        dclicktime2 = 0;
    } else {
      dclicktime2 += ticdup;
      if (dclicktime2 > 20) {
        dclicks2     = 0;
        dclickstate2 = false;
      }
    }
  }

  // [crispy] mouse look
  if ((crispy->freelook && mousebuttons[g_m_controls_globals->mousebmouselook]) || crispy->mouselook) {
    cmd->lookdir = mouse_y_invert ? -mousey : mousey;
  } else if (!novert) {
    forward += mousey;
  }

  // [crispy] single click on mouse look button centers view
  if (crispy->freelook) {
    static unsigned int mbmlookctrl = 0;

    // [crispy] single click view centering
    if (mousebuttons[g_m_controls_globals->mousebmouselook]) // [crispy] clicked
    {
      mbmlookctrl += static_cast<unsigned int>(ticdup);
    } else
      // [crispy] released
      if (mbmlookctrl) {
        if (crispy->freelook == FREELOOK_SPRING || mbmlookctrl < SLOWTURNTICS) // [crispy] short click
        {
          look = TOCENTER;
        }
        mbmlookctrl = 0;
      }
  }

  if (strafe)
    side += mousex2 * 2;
  else
    cmd->angleturn = static_cast<short>(cmd->angleturn - (mousex * 0x8));

  if (mousex == 0) {
    // No movement in the previous frame

    g_doomstat_globals->testcontrols_mousespeed = 0;
  }

  mousex = mousex2 = mousey = 0;

  auto MAXPLMOVE = []() { return (forwardmove[1]); };

  if (forward > MAXPLMOVE())
    forward = MAXPLMOVE();
  else if (forward < -MAXPLMOVE())
    forward = -MAXPLMOVE();
  if (side > MAXPLMOVE())
    side = MAXPLMOVE();
  else if (side < -MAXPLMOVE())
    side = -MAXPLMOVE();

  cmd->forwardmove = static_cast<signed char>(cmd->forwardmove + forward);
  cmd->sidemove    = static_cast<signed char>(cmd->sidemove + side);

  // [crispy] lookdir delta is stored in the lower 4 bits of the lookfly variable
  if (player->playerstate == PST_LIVE) {
    if (look < 0) {
      look += 16;
    }
    cmd->lookfly = static_cast<uint8_t>(look);
  }

  // special buttons
  if (sendpause) {
    sendpause = false;
    // [crispy] ignore un-pausing in menus during demo recording
    if (!(g_doomstat_globals->menuactive && g_doomstat_globals->demorecording && g_doomstat_globals->paused) && gameaction != ga_loadgame) {
      cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }
  }

  if (sendsave) {
    sendsave     = false;
    cmd->buttons = static_cast<uint8_t>(BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT));
  }

  if (crispy->fliplevels) {
    cmd->angleturn = static_cast<short>(-cmd->angleturn);
    cmd->sidemove  = static_cast<signed char>(-cmd->sidemove);
  }

  // low-res turning

  if (g_doomstat_globals->lowres_turn) {
    static signed short carry             = 0;
    signed short        desired_angleturn = static_cast<short>(cmd->angleturn + carry);

    // round angleturn to the nearest 256 unit boundary
    // for recording demos with single byte values for turn

    cmd->angleturn = static_cast<short>((desired_angleturn + 128) & 0xff00);

    // Carry forward the error from the reduced resolution to the
    // next tic, so that successive small movements can accumulate.

    carry = static_cast<short>(desired_angleturn - cmd->angleturn);
  }
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel() {
  // Set the sky map.
  // First thing, we have a dummy sky texture name,
  //  a flat. The data is in the WAD only because
  //  we look for an actual index, instead of simply
  //  setting one.

  g_doomstat_globals->skyflatnum = R_FlatNumForName(DEH_String(SKYFLATNAME));

  // The "Sky never changes in Doom II" bug was fixed in
  // the id Anthology version of doom2.exe for Final Doom.
  // [crispy] correct "Sky never changes in Doom II" bug
  if ((g_doomstat_globals->gamemode == commercial)
      && (g_doomstat_globals->gameversion == exe_final2 || g_doomstat_globals->gameversion == exe_chex || true)) {
    const char * skytexturename = nullptr;

    if (g_doomstat_globals->gamemap < 12) {
      skytexturename = "SKY1";
    } else if (g_doomstat_globals->gamemap < 21) {
      skytexturename = "SKY2";
    } else {
      skytexturename = "SKY3";
    }

    skytexturename = DEH_String(skytexturename);

    skytexture = R_TextureNumForName(skytexturename);
  }
  // [crispy] sky texture scales
  R_InitSkyMap();

  g_doomstat_globals->levelstarttic = gametic; // for time calculation

  if (g_doomstat_globals->wipegamestate == GS_LEVEL)
    g_doomstat_globals->wipegamestate = gamestate_t::GS_FORCE_WIPE; // force a wipe

  g_doomstat_globals->gamestate = GS_LEVEL;

  for (int i = 0; i < MAXPLAYERS; i++) {
    turbodetected[i] = false;
    if (g_doomstat_globals->playeringame[i] && g_doomstat_globals->players[i].playerstate == PST_DEAD)
      g_doomstat_globals->players[i].playerstate = PST_REBORN;
    std::memset(g_doomstat_globals->players[i].frags, 0, sizeof(g_doomstat_globals->players[i].frags));
  }

  // [crispy] update the "singleplayer" variable
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);
  P_SetupLevel(g_doomstat_globals->gameepisode, g_doomstat_globals->gamemap, 0, g_doomstat_globals->gameskill);
  g_doomstat_globals->displayplayer = g_doomstat_globals->consoleplayer; // view the guy you are playing
  gameaction                        = ga_nothing;
  Z_CheckHeap();

  // clear cmd building stuff

  std::memset(gamekeydown, 0, sizeof(gamekeydown));
  joyxmove = joyymove = joystrafemove = joylook = 0;
  mousex = mousex2 = mousey = 0;
  sendpause = sendsave = g_doomstat_globals->paused = false;
  std::memset(mousearray, 0, sizeof(mousearray));
  std::memset(joyarray, 0, sizeof(joyarray));

  if (g_doomstat_globals->testcontrols) {
    g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = "Press escape to quit.";
  }
}

static void SetJoyButtons(unsigned int buttons_mask) {
  for (int i = 0; i < MAX_JOY_BUTTONS; ++i) {
    int button_on = (buttons_mask & (1 << i)) != 0;

    // Detect button press:

    if (!joybuttons[i] && button_on) {
      // Weapon cycling:

      if (i == g_m_controls_globals->joybprevweapon) {
        next_weapon = -1;
      } else if (i == g_m_controls_globals->joybnextweapon) {
        next_weapon = 1;
      }
    }

    joybuttons[i] = button_on;
  }
}

static void SetMouseButtons(unsigned int buttons_mask) {
  for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
    unsigned int button_on = (buttons_mask & (1 << i)) != 0;

    // Detect button press:

    if (!mousebuttons[i] && button_on) {
      if (i == g_m_controls_globals->mousebprevweapon) {
        next_weapon = -1;
      } else if (i == g_m_controls_globals->mousebnextweapon) {
        next_weapon = 1;
      }
    }

    mousebuttons[i] = button_on;
  }
}

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
bool G_Responder(event_t * ev) {
  // allow spy mode changes even during the demo
  if (g_doomstat_globals->gamestate == GS_LEVEL && ev->type == ev_keydown
      && ev->data1 == g_m_controls_globals->key_spy && (g_doomstat_globals->singledemo || !g_doomstat_globals->deathmatch)) {
    // spy mode
    do {
      g_doomstat_globals->displayplayer++;
      if (g_doomstat_globals->displayplayer == MAXPLAYERS)
        g_doomstat_globals->displayplayer = 0;
    } while (!g_doomstat_globals->playeringame[g_doomstat_globals->displayplayer] && g_doomstat_globals->displayplayer != g_doomstat_globals->consoleplayer);
    return true;
  }

  // any other key pops up menu if in demos
  if (gameaction == ga_nothing && !g_doomstat_globals->singledemo && (g_doomstat_globals->demoplayback || g_doomstat_globals->gamestate == GS_DEMOSCREEN)) {
    if (ev->type == ev_keydown || (ev->type == ev_mouse && ev->data1) || (ev->type == ev_joystick && ev->data1)) {
      M_StartControlPanel();
      // [crispy] play a sound if the menu is activated with a different key than ESC
      if (crispy->soundfix)
        S_StartSound(nullptr, sfx_swtchn);
      return true;
    }
    return false;
  }

  if (g_doomstat_globals->gamestate == GS_LEVEL) {
#if 0 
	if (devparm && ev->type == ev_keydown && ev->data1 == ';') 
	{ 
	    G_DeathMatchSpawnPlayer (0); 
	    return true; 
	}
#endif
    if (HU_Responder(ev))
      return true; // chat ate the event
    if (ST_Responder(ev))
      return true; // status window ate it
    if (AM_Responder(ev))
      return true; // automap ate it
  }

  if (g_doomstat_globals->gamestate == GS_FINALE) {
    if (F_Responder(ev))
      return true; // finale ate the event
  }

  if (g_doomstat_globals->testcontrols && ev->type == ev_mouse) {
    // If we are invoked by setup to test the controls, save the
    // mouse speed so that we can display it on-screen.
    // Perform a low pass filter on this so that the thermometer
    // appears to move smoothly.

    g_doomstat_globals->testcontrols_mousespeed = std::abs(ev->data2);
  }

  // If the next/previous weapon keys are pressed, set the next_weapon
  // variable to change weapons when the next ticcmd is generated.

  if (ev->type == ev_keydown && ev->data1 == g_m_controls_globals->key_prevweapon) {
    next_weapon = -1;
  } else if (ev->type == ev_keydown && ev->data1 == g_m_controls_globals->key_nextweapon) {
    next_weapon = 1;
  }

  switch (ev->type) {
  case ev_keydown:
    if (ev->data1 == g_m_controls_globals->key_pause) {
      sendpause = true;
    } else if (ev->data1 < NUMKEYS) {
      gamekeydown[ev->data1] = true;
    }

    return true; // eat key down events

  case ev_keyup:
    if (ev->data1 < NUMKEYS)
      gamekeydown[ev->data1] = false;
    return false; // always let key up events filter down

  case ev_mouse:
    SetMouseButtons(static_cast<unsigned int>(ev->data1));
    if (g_doomstat_globals->mouseSensitivity)
      mousex = ev->data2 * (g_doomstat_globals->mouseSensitivity + 5) / 10;
    else
      mousex = 0; // [crispy] disable entirely
    if (g_doomstat_globals->mouseSensitivity_x2)
      mousex2 = ev->data2 * (g_doomstat_globals->mouseSensitivity_x2 + 5) / 10; // [crispy] separate sensitivity for strafe
    else
      mousex2 = 0; // [crispy] disable entirely
    if (g_doomstat_globals->mouseSensitivity_y)
      mousey = ev->data3 * (g_doomstat_globals->mouseSensitivity_y + 5) / 10; // [crispy] separate sensitivity for y-axis
    else
      mousey = 0; // [crispy] disable entirely
    return true;  // eat events

  case ev_joystick:
    SetJoyButtons(static_cast<unsigned int>(ev->data1));
    joyxmove      = ev->data2;
    joyymove      = ev->data3;
    joystrafemove = ev->data4;
    joylook       = ev->data5;
    return true; // eat events

  default:
    break;
  }

  return false;
}

// [crispy] re-read game parameters from command line
static void G_ReadGameParms() {
  g_doomstat_globals->respawnparm = M_CheckParm("-respawn");
  g_doomstat_globals->fastparm    = M_CheckParm("-fast");
  g_doomstat_globals->nomonsters  = M_CheckParm("-nomonsters");
}

// [crispy] take a screenshot after rendering the next frame
static void G_CrispyScreenShot() {
  // [crispy] increase screenshot filename limit
  V_ScreenShot("DOOM%04i.%s");
  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String("screen shot");
  crispy->cleanscreenshot                                                = 0;
  crispy->screenshotmsg                                                  = 2;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker() {
  ticcmd_t * cmd;

  // do player reborns if needed
  for (int i = 0; i < MAXPLAYERS; i++)
    if (g_doomstat_globals->playeringame[i] && g_doomstat_globals->players[i].playerstate == PST_REBORN)
      G_DoReborn(i);

  // do things to change the game state
  while (gameaction != ga_nothing) {
    switch (gameaction) {
    case ga_loadlevel:
      G_DoLoadLevel();
      break;
    case ga_newgame:
      // [crispy] re-read game parameters from command line
      G_ReadGameParms();
      G_DoNewGame();
      break;
    case ga_loadgame:
      // [crispy] re-read game parameters from command line
      G_ReadGameParms();
      G_DoLoadGame();
      break;
    case ga_savegame:
      G_DoSaveGame();
      break;
    case ga_playdemo:
      G_DoPlayDemo();
      break;
    case ga_completed:
      G_DoCompleted();
      break;
    case ga_victory:
      F_StartFinale();
      break;
    case ga_worlddone:
      G_DoWorldDone();
      break;
    case ga_screenshot:
      // [crispy] redraw view without weapons and HUD
      if (g_doomstat_globals->gamestate == GS_LEVEL && (crispy->cleanscreenshot || crispy->screenshotmsg == 1)) {
        crispy->screenshotmsg       = 4;
        crispy->post_rendering_hook = G_CrispyScreenShot;
      } else {
        G_CrispyScreenShot();
      }
      gameaction = ga_nothing;
      break;
    case ga_nothing:
      break;
    }
  }

  // get commands, check consistancy,
  // and build new consistancy check
  int buf = (gametic / ticdup) % BACKUPTICS;

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (g_doomstat_globals->playeringame[i]) {
      cmd = &g_doomstat_globals->players[i].cmd;

      std::memcpy(cmd, &g_doomstat_globals->netcmds[i], sizeof(ticcmd_t));

      if (g_doomstat_globals->demoplayback)
        G_ReadDemoTiccmd(cmd);
      // [crispy] do not record tics while still playing back in demo continue mode
      if (g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback)
        G_WriteDemoTiccmd(cmd);

      // check for turbo cheats

      // check ~ 4 seconds whether to display the turbo message.
      // store if the turbo threshold was exceeded in any tics
      // over the past 4 seconds.  offset the checking period
      // for each player so messages are not displayed at the
      // same time.

      if (cmd->forwardmove > TURBOTHRESHOLD) {
        turbodetected[i] = true;
      }

      if ((gametic & 31) == 0
          && ((gametic >> 5) % MAXPLAYERS) == i
          && turbodetected[i]) {
        static char   turbomessage[80];
        extern char * player_names[4];
        M_snprintf(turbomessage, sizeof(turbomessage), "%s is turbo!", player_names[i]);
        g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = turbomessage;
        turbodetected[i]                                                       = false;
      }

      if (g_doomstat_globals->netgame && !netdemo && !(gametic % ticdup)) {
        if (gametic > BACKUPTICS
            && consistancy[i][buf] != cmd->consistancy) {
          I_Error("consistency failure (%i should be %i)",
                  cmd->consistancy,
                  consistancy[i][buf]);
        }
        if (g_doomstat_globals->players[i].mo)
          consistancy[i][buf] = static_cast<uint8_t>(g_doomstat_globals->players[i].mo->x);
        else
          consistancy[i][buf] = static_cast<uint8_t>(g_doomstat_globals->rndindex);
      }
    }
  }

  // check for special buttons
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (g_doomstat_globals->playeringame[i]) {
      if (g_doomstat_globals->players[i].cmd.buttons & BT_SPECIAL) {
        switch (g_doomstat_globals->players[i].cmd.buttons & BT_SPECIALMASK) {
        case BTS_PAUSE:
          g_doomstat_globals->paused ^= 1;
          if (g_doomstat_globals->paused)
            S_PauseSound();
          else
            // [crispy] Fixed bug when music was hearable with zero volume
            if (g_doomstat_globals->musicVolume)
              S_ResumeSound();
          break;

        case BTS_SAVEGAME:
          // [crispy] never override savegames by demo playback
          if (g_doomstat_globals->demoplayback)
            break;
          if (!savedescription[0]) {
            M_StringCopy(savedescription, "NET GAME", sizeof(savedescription));
          }

          savegameslot =
              (g_doomstat_globals->players[i].cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
          gameaction = ga_savegame;
          // [crispy] un-pause immediately after saving
          // (impossible to send save and pause specials within the same tic)
          if (g_doomstat_globals->demorecording && g_doomstat_globals->paused)
            sendpause = true;
          break;
        }
      }
    }
  }

  // Have we just finished displaying an intermission screen?

  if (oldgamestate == GS_INTERMISSION && g_doomstat_globals->gamestate != GS_INTERMISSION) {
    WI_End();
  }

  oldgamestate = g_doomstat_globals->gamestate;
  oldleveltime = leveltime;

  // do main actions
  switch (g_doomstat_globals->gamestate) {
  case GS_LEVEL:
    P_Ticker();
    ST_Ticker();
    AM_Ticker();
    HU_Ticker();
    break;

  case GS_INTERMISSION:
    WI_Ticker();
    break;

  case GS_FINALE:
    F_Ticker();
    break;

  case GS_DEMOSCREEN:
    D_PageTicker();
    break;
  case GS_FORCE_WIPE:
    // nothing to do here
    break;
  }
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer(int player) {
  // clear everything else to defaults
  G_PlayerReborn(player);
}

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel(int player) {
  player_t * p = &g_doomstat_globals->players[player];

  std::memset(p->powers, 0, sizeof(p->powers));
  std::memset(p->cards, 0, sizeof(p->cards));
  std::memset(p->tryopen, 0, sizeof(p->tryopen)); // [crispy] blinking key or skull in the status bar
  p->mo->flags &= ~MF_SHADOW;                     // cancel invisibility
  p->extralight    = 0;                           // cancel gun flashes
  p->fixedcolormap = 0;                           // cancel ir gogles
  p->damagecount   = 0;                           // no palette changes
  p->bonuscount    = 0;
  // [crispy] reset additional player properties
  p->lookdir        = 0;
  p->oldlookdir     = 0;
  p->centering      = false;
  p->jumpTics       = 0;
  p->recoilpitch    = 0;
  p->oldrecoilpitch = 0;
  p->psp_dy_max     = 0;
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn(int player) {
  int frags[MAXPLAYERS];

  std::memcpy(frags, g_doomstat_globals->players[player].frags, sizeof(frags));
  int killcount   = g_doomstat_globals->players[player].killcount;
  int itemcount   = g_doomstat_globals->players[player].itemcount;
  int secretcount = g_doomstat_globals->players[player].secretcount;

  player_t * p = &g_doomstat_globals->players[player];
  std::memset(p, 0, sizeof(*p));

  std::memcpy(g_doomstat_globals->players[player].frags, frags, sizeof(g_doomstat_globals->players[player].frags));
  g_doomstat_globals->players[player].killcount   = killcount;
  g_doomstat_globals->players[player].itemcount   = itemcount;
  g_doomstat_globals->players[player].secretcount = secretcount;

  p->usedown = p->attackdown = true; // don't do anything immediately
  p->playerstate             = PST_LIVE;
  p->health                  = deh_initial_health; // Use dehacked value
  // [crispy] negative player health
  p->neghealth   = p->health;
  p->readyweapon = p->pendingweapon = wp_pistol;
  p->weaponowned[wp_fist]           = true;
  p->weaponowned[wp_pistol]         = true;
  p->ammo[am_clip]                  = deh_initial_bullets;

  for (int i = 0; i < NUMAMMO; i++)
    p->maxammo[i] = g_p_local_globals->maxammo[i];
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
void P_SpawnPlayer(mapthing_t * mthing);

bool G_CheckSpot(int          playernum,
                 mapthing_t * mthing) {
  if (!g_doomstat_globals->players[playernum].mo) {
    // first spawn of level, before corpses
    for (int i = 0; i < playernum; i++)
      if (g_doomstat_globals->players[i].mo->x == mthing->x << FRACBITS
          && g_doomstat_globals->players[i].mo->y == mthing->y << FRACBITS)
        return false;
    return true;
  }

  fixed_t x = mthing->x << FRACBITS;
  fixed_t y = mthing->y << FRACBITS;

  if (!P_CheckPosition(g_doomstat_globals->players[playernum].mo, x, y))
    return false;

  // flush an old corpse if needed
  if (g_doomstat_globals->bodyqueslot >= BODYQUESIZE)
    P_RemoveMobj(bodyque[g_doomstat_globals->bodyqueslot % BODYQUESIZE]);
  bodyque[g_doomstat_globals->bodyqueslot % BODYQUESIZE] = g_doomstat_globals->players[playernum].mo;
  g_doomstat_globals->bodyqueslot++;

  // spawn a teleport fog
  subsector_t * ss = R_PointInSubsector(x, y);

  // The code in the released source looks like this:
  //
  //    an = ( ANG45 * (((unsigned int) mthing->angle)/45) )
  //         >> ANGLETOFINESHIFT;
  //    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
  //                     , ss->sector->floorheight
  //                     , MT_TFOG);
  //
  // But 'an' can be a signed value in the DOS version. This means that
  // we get a negative index and the lookups into finecosine/finesine
  // end up dereferencing values in finetangent[].
  // A player spawning on a deathmatch start facing directly west spawns
  // "silently" with no spawn fog. Emulate this.
  //
  // This code is imported from PrBoom+.
  mobj_t * mo;
  {
    fixed_t xa;
    fixed_t ya;

    // This calculation overflows in Vanilla Doom, but here we deliberately
    // avoid integer overflow as it is undefined behavior, so the value of
    // 'an' will always be positive.
    signed int an = (ANG45 >> ANGLETOFINESHIFT) * (static_cast<signed int>(mthing->angle) / 45);

    switch (an) {
    case 4096:                // -4096:
      xa = finetangent[2048]; // finecosine[-4096]
      ya = finetangent[0];    // finesine[-4096]
      break;
    case 5120:                // -3072:
      xa = finetangent[3072]; // finecosine[-3072]
      ya = finetangent[1024]; // finesine[-3072]
      break;
    case 6144:                // -2048:
      xa = finesine[0];       // finecosine[-2048]
      ya = finetangent[2048]; // finesine[-2048]
      break;
    case 7168:                // -1024:
      xa = finesine[1024];    // finecosine[-1024]
      ya = finetangent[3072]; // finesine[-1024]
      break;
    case 0:
    case 1024:
    case 2048:
    case 3072:
      xa = finecosine[an];
      ya = finesine[an];
      break;
    default:
      I_Error("G_CheckSpot: unexpected angle %d\n", an);
      xa = ya = 0;
      break;
    }
    mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);
  }

  if (g_doomstat_globals->players[g_doomstat_globals->consoleplayer].viewz != 1)
    S_StartSound(mo, sfx_telept); // don't start sound on first frame

  return true;
}

//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
void G_DeathMatchSpawnPlayer(int playernum) {
  int selections = static_cast<int>(g_doomstat_globals->deathmatch_p - g_doomstat_globals->deathmatchstarts);
  if (selections < 4)
    I_Error("Only %i deathmatch spots, 4 required", selections);

  for (int j = 0; j < 20; j++) {
    int i = P_Random() % selections;
    if (G_CheckSpot(playernum, &g_doomstat_globals->deathmatchstarts[i])) {
      g_doomstat_globals->deathmatchstarts[i].type = static_cast<short>(playernum + 1);
      P_SpawnPlayer(&g_doomstat_globals->deathmatchstarts[i]);
      return;
    }
  }

  // no good spot, so the player will probably get stuck
  P_SpawnPlayer(&g_doomstat_globals->playerstarts[playernum]);
}

// [crispy] clear the "savename" variable,
// i.e. restart level from scratch upon resurrection
static inline void G_ClearSavename() {
  M_StringCopy(savename, "", sizeof(savename));
}

//
// G_DoReborn
//
void G_DoReborn(int playernum) {
  if (!g_doomstat_globals->netgame) {
    // [crispy] if the player dies and the game has been loaded or saved
    // in the mean time, reload that savegame instead of restarting the level
    // when "Run" is pressed upon resurrection
    if (crispy->singleplayer && *savename && speedkeydown())
      gameaction = ga_loadgame;
    else {
      // reload the level from scratch
      gameaction = ga_loadlevel;
      G_ClearSavename();
    }
  } else {
    // respawn at the start

    // first dissasociate the corpse
    g_doomstat_globals->players[playernum].mo->player = nullptr;

    // spawn at random spot if in death match
    if (g_doomstat_globals->deathmatch) {
      G_DeathMatchSpawnPlayer(playernum);
      return;
    }

    if (G_CheckSpot(playernum, &g_doomstat_globals->playerstarts[playernum])) {
      P_SpawnPlayer(&g_doomstat_globals->playerstarts[playernum]);
      return;
    }

    // try to spawn at one of the other players spots
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (G_CheckSpot(playernum, &g_doomstat_globals->playerstarts[i])) {
        g_doomstat_globals->playerstarts[i].type = static_cast<short>(playernum + 1); // fake as other player
        P_SpawnPlayer(&g_doomstat_globals->playerstarts[i]);
        g_doomstat_globals->playerstarts[i].type = static_cast<short>(i + 1); // restore
        return;
      }
      // he's going to be inside something.  Too bad.
    }
    P_SpawnPlayer(&g_doomstat_globals->playerstarts[playernum]);
  }
}

void G_ScreenShot() {
  gameaction = ga_screenshot;
}

// clang-format off
// DOOM Par Times
int pars[6][10] = {
    { 0 },
    { 0, 30, 75, 120, 90, 165, 180, 180, 30, 165 },
    { 0, 90, 90, 90, 120, 90, 360, 240, 30, 170 },
    { 0, 90, 45, 90, 150, 90, 90, 165, 30, 135 }
    // [crispy] Episode 4 par times from the BFG Edition
    ,
    { 0, 165, 255, 135, 150, 180, 390, 135, 360, 180 }
    // [crispy] Episode 5 par times from Sigil v1.21
    ,
    { 0, 90, 150, 360, 420, 780, 420, 780, 300, 660 }
};

// DOOM II Par Times
int cpars[32] = {
    30, 90, 120, 120, 90, 150, 120, 120, 270, 90,     //  1-10
    210, 150, 150, 150, 210, 150, 420, 150, 210, 150, // 11-20
    240, 150, 180, 150, 150, 300, 330, 420, 300, 180, // 21-30
    120, 30                                           // 31-32
};

// [crispy] No Rest For The Living par times from the BFG Edition
static int npars[9] = {
    75, 105, 120, 105, 210, 105, 165, 105, 135
};
// clang-format on

//
// G_DoCompleted
//
bool          secretexit;
extern char * pagename;

void G_ExitLevel() {
  secretexit = false;
  G_ClearSavename();
  gameaction = ga_completed;
}

// Here's for the german edition.
void G_SecretExitLevel() {
  // IF NO WOLF3D LEVELS, NO SECRET EXIT!
  if ((g_doomstat_globals->gamemode == commercial)
      && (W_CheckNumForName("map31") < 0))
    secretexit = false;
  else
    secretexit = true;
  G_ClearSavename();
  gameaction = ga_completed;
}

void G_DoCompleted() {
  gameaction = ga_nothing;

  for (int i = 0; i < MAXPLAYERS; i++)
    if (g_doomstat_globals->playeringame[i])
      G_PlayerFinishLevel(i); // take away cards and stuff

  if (g_doomstat_globals->automapactive)
    AM_Stop();

  if (g_doomstat_globals->gamemode != commercial) {
    // Chex Quest ends after 5 levels, rather than 8.

    if (g_doomstat_globals->gameversion == exe_chex) {
      if (g_doomstat_globals->gamemap == 5) {
        gameaction = ga_victory;
        return;
      }
    } else {
      switch (g_doomstat_globals->gamemap) {
        // [crispy] display tally screen after ExM8
        /*
      case 8:
        gameaction = ga_victory;
        return;
    */
      case 9:
        for (auto & player : g_doomstat_globals->players)
          player.didsecret = true;
        break;
      }
    }
  }

  // [crispy] disable redundant code
  /*
//#if 0  Hmmm - why?
  if ( (gamemap == 8)
       && (gamemode != commercial) )
  {
      // victory
      gameaction = ga_victory;
      return;
  }

  if ( (gamemap == 9)
       && (gamemode != commercial) )
  {
      // exit secret level
      for (i=0 ; i<MAXPLAYERS ; i++)
          players[i].didsecret = true;
  }
//#endif
*/

  g_doomstat_globals->wminfo.didsecret = g_doomstat_globals->players[g_doomstat_globals->consoleplayer].didsecret;
  g_doomstat_globals->wminfo.epsd      = g_doomstat_globals->gameepisode - 1;
  g_doomstat_globals->wminfo.last      = g_doomstat_globals->gamemap - 1;

  // wminfo.next is 0 biased, unlike gamemap
  if (g_doomstat_globals->gamemission == pack_nerve && g_doomstat_globals->gamemap <= 9 && crispy->singleplayer) {
    if (secretexit)
      switch (g_doomstat_globals->gamemap) {
      case 4: g_doomstat_globals->wminfo.next = 8; break;
      }
    else
      switch (g_doomstat_globals->gamemap) {
      case 9: g_doomstat_globals->wminfo.next = 4; break;
      default: g_doomstat_globals->wminfo.next = g_doomstat_globals->gamemap;
      }
  } else if (g_doomstat_globals->gamemission == pack_master && g_doomstat_globals->gamemap <= 21 && crispy->singleplayer) {
    g_doomstat_globals->wminfo.next = g_doomstat_globals->gamemap;
  } else if (g_doomstat_globals->gamemode == commercial) {
    if (secretexit)
      if (g_doomstat_globals->gamemap == 2 && critical->havemap33)
        g_doomstat_globals->wminfo.next = 32;
      else
        switch (g_doomstat_globals->gamemap) {
        case 15: g_doomstat_globals->wminfo.next = 30; break;
        case 31: g_doomstat_globals->wminfo.next = 31; break;
        }
    else if (g_doomstat_globals->gamemap == 33 && critical->havemap33)
      g_doomstat_globals->wminfo.next = 2;
    else
      switch (g_doomstat_globals->gamemap) {
      case 31:
      case 32: g_doomstat_globals->wminfo.next = 15; break;
      default: g_doomstat_globals->wminfo.next = g_doomstat_globals->gamemap;
      }
  } else {
    if (secretexit) {
      if (critical->havee1m10 && g_doomstat_globals->gameepisode == 1 && g_doomstat_globals->gamemap == 1)
        g_doomstat_globals->wminfo.next = 9; // [crispy] go to secret level E1M10 "Sewers"
      else
        g_doomstat_globals->wminfo.next = 8; // go to secret level
    } else if (g_doomstat_globals->gamemap == 9) {
      // returning from secret level
      switch (g_doomstat_globals->gameepisode) {
      case 1:
        g_doomstat_globals->wminfo.next = 3;
        break;
      case 2:
        g_doomstat_globals->wminfo.next = 5;
        break;
      case 3:
      case 5: // [crispy] Sigil
        g_doomstat_globals->wminfo.next = 6;
        break;
      case 4:
        g_doomstat_globals->wminfo.next = 2;
        break;
      }
    } else if (critical->havee1m10 && g_doomstat_globals->gameepisode == 1 && g_doomstat_globals->gamemap == 10)
      g_doomstat_globals->wminfo.next = 1; // [crispy] returning from secret level E1M10 "Sewers"
    else
      g_doomstat_globals->wminfo.next = g_doomstat_globals->gamemap; // go to next level
  }

  g_doomstat_globals->wminfo.maxkills  = g_doomstat_globals->totalkills;
  g_doomstat_globals->wminfo.maxitems  = g_doomstat_globals->totalitems;
  g_doomstat_globals->wminfo.maxsecret = g_doomstat_globals->totalsecret;
  g_doomstat_globals->wminfo.maxfrags  = 0;

  // Set par time. Exceptions are added for purposes of
  // statcheck regression testing.
  if (g_doomstat_globals->gamemode == commercial) {
    // map33 reads its par time from beyond the cpars[] array
    if (g_doomstat_globals->gamemap == 33) {
      int cpars32 = 0;

      std::memcpy(&cpars32, DEH_String(GAMMALVL0), sizeof(int));
      cpars32 = LONG(cpars32);

      g_doomstat_globals->wminfo.partime = TICRATE * cpars32;
    }
    // [crispy] support [PARS] sections in BEX files
    else if (bex_cpars[g_doomstat_globals->gamemap - 1]) {
      g_doomstat_globals->wminfo.partime = TICRATE * bex_cpars[g_doomstat_globals->gamemap - 1];
    }
    // [crispy] single player par times for NRFTL
    else if (g_doomstat_globals->gamemission == pack_nerve && crispy->singleplayer) {
      g_doomstat_globals->wminfo.partime = TICRATE * npars[g_doomstat_globals->gamemap - 1];
    } else {
      g_doomstat_globals->wminfo.partime = TICRATE * cpars[g_doomstat_globals->gamemap - 1];
    }
  }
  // Doom episode 4 doesn't have a par time, so this
  // overflows into the cpars array.
  else if (g_doomstat_globals->gameepisode < 4 ||
           // [crispy] single player par times for episode 4
           (g_doomstat_globals->gameepisode == 4 && crispy->singleplayer) ||
           // [crispy] par times for Sigil
           g_doomstat_globals->gameepisode == 5) {
    // [crispy] support [PARS] sections in BEX files
    if (bex_pars[g_doomstat_globals->gameepisode][g_doomstat_globals->gamemap]) {
      g_doomstat_globals->wminfo.partime = TICRATE * bex_pars[g_doomstat_globals->gameepisode][g_doomstat_globals->gamemap];
    } else
      g_doomstat_globals->wminfo.partime = TICRATE * pars[g_doomstat_globals->gameepisode][g_doomstat_globals->gamemap];
  } else {
    g_doomstat_globals->wminfo.partime = TICRATE * cpars[g_doomstat_globals->gamemap];
  }

  g_doomstat_globals->wminfo.pnum = g_doomstat_globals->consoleplayer;

  for (int i = 0; i < MAXPLAYERS; i++) {
    g_doomstat_globals->wminfo.plyr[i].in      = g_doomstat_globals->playeringame[i];
    g_doomstat_globals->wminfo.plyr[i].skills  = g_doomstat_globals->players[i].killcount;
    g_doomstat_globals->wminfo.plyr[i].sitems  = g_doomstat_globals->players[i].itemcount;
    g_doomstat_globals->wminfo.plyr[i].ssecret = g_doomstat_globals->players[i].secretcount;
    g_doomstat_globals->wminfo.plyr[i].stime   = leveltime;
    std::memcpy(g_doomstat_globals->wminfo.plyr[i].frags, g_doomstat_globals->players[i].frags, sizeof(g_doomstat_globals->wminfo.plyr[i].frags));
  }

  // [crispy] CPhipps - total time for all completed levels
  // cph - modified so that only whole seconds are added to the totalleveltimes
  // value; so our total is compatible with the "naive" total of just adding
  // the times in seconds shown for each level. Also means our total time
  // will agree with Compet-n.
  g_doomstat_globals->wminfo.totaltimes = (g_doomstat_globals->totalleveltimes += (leveltime - leveltime % TICRATE));

  g_doomstat_globals->gamestate     = GS_INTERMISSION;
  g_doomstat_globals->viewactive    = false;
  g_doomstat_globals->automapactive = false;

  // [crispy] no statdump output for ExM8
  if (g_doomstat_globals->gamemode == commercial || g_doomstat_globals->gamemap != 8) {
    StatCopy(&g_doomstat_globals->wminfo);
  }

  WI_Start(&g_doomstat_globals->wminfo);
}

//
// G_WorldDone
//
void G_WorldDone() {
  gameaction = ga_worlddone;

  if (secretexit)
    // [crispy] special-casing for E1M10 "Sewers" support
    // i.e. avoid drawing the splat for E1M9 already
    if (!crispy->havee1m10 || g_doomstat_globals->gameepisode != 1 || g_doomstat_globals->gamemap != 1)
      g_doomstat_globals->players[g_doomstat_globals->consoleplayer].didsecret = true;

  if (g_doomstat_globals->gamemission == pack_nerve && crispy->singleplayer) {
    switch (g_doomstat_globals->gamemap) {
    case 8:
      F_StartFinale();
      break;
    }
  } else if (g_doomstat_globals->gamemission == pack_master && crispy->singleplayer) {
    switch (g_doomstat_globals->gamemap) {
    case 20:
      if (secretexit)
        break;
      [[fallthrough]];
    case 21:
      F_StartFinale();
      break;
    }
  } else if (g_doomstat_globals->gamemode == commercial) {
    switch (g_doomstat_globals->gamemap) {
    case 15:
    case 31:
      if (!secretexit)
        break;
      [[fallthrough]];
    case 6:
    case 11:
    case 20:
    case 30:
      F_StartFinale();
      break;
    }
  }
  // [crispy] display tally screen after ExM8
  else if (g_doomstat_globals->gamemap == 8) {
    gameaction = ga_victory;
  }
}

void G_DoWorldDone() {
  g_doomstat_globals->gamestate = GS_LEVEL;
  g_doomstat_globals->gamemap   = g_doomstat_globals->wminfo.next + 1;
  G_DoLoadLevel();
  gameaction                     = ga_nothing;
  g_doomstat_globals->viewactive = true;
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
extern bool setsizeneeded;
void        R_ExecuteSetViewSize();

void G_LoadGame(char * name) {
  M_StringCopy(savename, name, sizeof(savename));
  gameaction = ga_loadgame;
}

int  savedleveltime = 0; // [crispy] moved here for level time logging
void G_DoLoadGame() {

  // [crispy] loaded game must always be single player.
  // Needed for ability to use a further game loading, as well as
  // cheat codes and other single player only specifics.
  if (g_doomstat_globals->startloadgame == -1) {
    netdemo                        = false;
    g_doomstat_globals->netgame    = false;
    g_doomstat_globals->deathmatch = false;
  }
  gameaction = ga_nothing;

  save_stream = fopen(savename, "rb");

  if (save_stream == nullptr) {
    I_Error("Could not load savegame %s", savename);
  }

  // [crispy] read extended savegame data
  if (crispy->extsaveg) {
    // [crispy] first pass, read "savewadfilename"
    P_ReadExtendedSaveGameData(0);
  }
  // [crispy] check if WAD file is valid to restore saved map
  if (savewadfilename) {
    // [crispy] strings are not equal
    if (!savemaplumpinfo ||
        // [crispy] case-insensitive, so "doom.wad" matches "DOOM.WAD"
        !iequals(savewadfilename, W_WadNameForLump(savemaplumpinfo))) {
      M_ForceLoadGame();
      fclose(save_stream);
      return;
    } else
      // [crispy] strings are equal, but not identical
      if (savewadfilename != W_WadNameForLump(savemaplumpinfo)) {
        free(savewadfilename);
      }
  }
  savewadfilename = nullptr;

  savegame_error = false;

  if (!P_ReadSaveGameHeader()) {
    // [crispy] indicate game version mismatch
    extern void M_LoadGameVerMismatch();
    M_LoadGameVerMismatch();
    fclose(save_stream);
    return;
  }

  savedleveltime = leveltime;

  // load a base level
  G_InitNew(g_doomstat_globals->gameskill, g_doomstat_globals->gameepisode, g_doomstat_globals->gamemap);

  leveltime      = savedleveltime;
  savedleveltime = 0;

  // dearchive all the modifications
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchiveThinkers();
  P_UnArchiveSpecials();
  P_RestoreTargets(); // [crispy] restore mobj->target and mobj->tracer pointers

  if (!P_ReadSaveGameEOF())
    I_Error("Bad savegame");

  // [crispy] read more extended savegame data
  if (crispy->extsaveg) {
    P_ReadExtendedSaveGameData(1);
  }

  fclose(save_stream);

  if (setsizeneeded)
    R_ExecuteSetViewSize();

  // draw the pattern into the back screen
  R_FillBackScreen();

  // [crispy] if the player is dead in this savegame,
  // do not consider it for reload
  if (g_doomstat_globals->players[g_doomstat_globals->consoleplayer].health <= 0)
    G_ClearSavename();

  // [crisy] once loaded from the command line,
  // the next savegame will be loaded from the menu
  g_doomstat_globals->startloadgame = -1;
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
void G_SaveGame(int    slot,
                char * description) {
  savegameslot = slot;
  M_StringCopy(savedescription, description, sizeof(savedescription));
  sendsave = true;
}

void G_DoSaveGame() {
  char * recovery_savegame_file = nullptr;
  char * temp_savegame_file     = P_TempSaveGameFile();
  char * savegame_file          = P_SaveGameFile(savegameslot);

  // Open the savegame file for writing.  We write to a temporary file
  // and then rename it at the end if it was successfully written.
  // This prevents an existing savegame from being overwritten by
  // a corrupted one, or if a savegame buffer overrun occurs.
  save_stream = fopen(temp_savegame_file, "wb");

  if (save_stream == nullptr) {
    // Failed to save the game, so we're going to have to abort. But
    // to be nice, save to somewhere else before we call I_Error().
    recovery_savegame_file = M_TempFile("recovery.dsg");
    save_stream            = fopen(recovery_savegame_file, "wb");
    if (save_stream == nullptr) {
      I_Error("Failed to open either '%s' or '%s' to write savegame.",
              temp_savegame_file,
              recovery_savegame_file);
    }
  }

  savegame_error = false;

  P_WriteSaveGameHeader(savedescription);

  // [crispy] some logging when saving
  {
    const int ltime = leveltime / TICRATE,
              ttime = (g_doomstat_globals->totalleveltimes + leveltime) / TICRATE;
    extern const char * skilltable[];

    fmt::fprintf(stderr, "G_DoSaveGame: Episode %d, Map %d, %s, Time %d:%02d:%02d, Total %d:%02d:%02d.\n", g_doomstat_globals->gameepisode, g_doomstat_globals->gamemap, skilltable[std::clamp(static_cast<int>(g_doomstat_globals->gameskill) + 1, 0, 5)], ltime / 3600, (ltime % 3600) / 60, ltime % 60, ttime / 3600, (ttime % 3600) / 60, ttime % 60);
  }

  P_ArchivePlayers();
  P_ArchiveWorld();
  P_ArchiveThinkers();
  P_ArchiveSpecials();

  P_WriteSaveGameEOF();
  // [crispy] write extended savegame data
  if (crispy->extsaveg) {
    P_WriteExtendedSaveGameData();
  }

  // [crispy] unconditionally disable savegame and demo limits
  /*
  // Enforce the same savegame size limit as in Vanilla Doom,
  // except if the vanilla_savegame_limit setting is turned off.

  if (vanilla_savegame_limit && ftell(save_stream) > SAVEGAMESIZE)
  {
      I_Error("Savegame buffer overrun");
  }
  */

  // Finish up, close the savegame file.

  fclose(save_stream);

  if (recovery_savegame_file != nullptr) {
    // We failed to save to the normal location, but we wrote a
    // recovery file to the temp directory. Now we can bomb out
    // with an error.
    I_Error("Failed to open savegame file '%s' for writing.\n"
            "But your game has been saved to '%s' for recovery.",
            temp_savegame_file,
            recovery_savegame_file);
  }

  // Now rename the temporary savegame file to the actual savegame
  // file, overwriting the old savegame if there was one there.

  remove(savegame_file);
  rename(temp_savegame_file, savegame_file);

  gameaction = ga_nothing;
  M_StringCopy(savedescription, "", sizeof(savedescription));
  M_StringCopy(savename, savegame_file, sizeof(savename));

  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(GGSAVED);

  // draw the pattern into the back screen
  R_FillBackScreen();
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//
skill_t d_skill;
int     d_episode;
int     d_map;

void G_DeferedInitNew(skill_t skill,
                      int     episode,
                      int     map) {
  d_skill   = skill;
  d_episode = episode;
  d_map     = map;
  G_ClearSavename();
  gameaction = ga_newgame;

  // [crispy] if a new game is started during demo recording, start a new demo
  if (g_doomstat_globals->demorecording) {
    G_CheckDemoStatus();
    Z_Free(demoname);

    G_RecordDemo(orig_demoname);
    G_BeginRecording();
  }
}

void G_DoNewGame() {
  g_doomstat_globals->demoplayback    = false;
  netdemo                             = false;
  g_doomstat_globals->netgame         = false;
  g_doomstat_globals->deathmatch      = false;
  g_doomstat_globals->playeringame[1] = g_doomstat_globals->playeringame[2] = g_doomstat_globals->playeringame[3] = false;
  // [crispy] do not reset -respawn, -fast and -nomonsters parameters
  /*
  respawnparm = false;
  fastparm = false;
  nomonsters = false;
  */
  g_doomstat_globals->consoleplayer = 0;
  G_InitNew(d_skill, d_episode, d_map);
  gameaction = ga_nothing;
}

void G_InitNew(skill_t skill,
               int     episode,
               int     map) {
  // [crispy] make sure "fast" parameters are really only applied once
  static bool fast_applied;

  if (g_doomstat_globals->paused) {
    g_doomstat_globals->paused = false;
    S_ResumeSound();
  }

  /*
  // Note: This commented-out block of code was added at some point
  // between the DOS version(s) and the Doom source release. It isn't
  // found in disassemblies of the DOS version and causes IDCLEV and
  // the -warp command line parameter to behave differently.
  // This is left here for posterity.

  // This was quite messy with SPECIAL and commented parts.
  // Supposedly hacks to make the latest edition work.
  // It might not work properly.
  if (episode < 1)
    episode = 1;

  if ( gamemode == retail )
  {
    if (episode > 4)
      episode = 4;
  }
  else if ( gamemode == shareware )
  {
    if (episode > 1)
         episode = 1;	// only start episode 1 on shareware
  }
  else
  {
    if (episode > 3)
      episode = 3;
  }
  */

  if (skill > sk_nightmare)
    skill = sk_nightmare;

  // [crispy] only fix episode/map if it doesn't exist
  if (P_GetNumForMap(episode, map, false) < 0) {
    if (g_doomstat_globals->gameversion >= exe_ultimate) {
      if (episode == 0) {
        episode = 4;
      }
    } else {
      if (episode < 1) {
        episode = 1;
      }
      if (episode > 3) {
        episode = 3;
      }
    }

    if (episode > 1 && g_doomstat_globals->gamemode == shareware) {
      episode = 1;
    }

    if (map < 1)
      map = 1;

    if ((map > 9)
        && (g_doomstat_globals->gamemode != commercial)) {
      // [crispy] support E1M10 "Sewers"
      if (!crispy->havee1m10 || episode != 1)
        map = 9;
      else
        map = 10;
    }
  }

  M_ClearRandom();

  if (skill == sk_nightmare || g_doomstat_globals->respawnparm)
    g_doomstat_globals->respawnmonsters = true;
  else
    g_doomstat_globals->respawnmonsters = false;

  // [crispy] make sure "fast" parameters are really only applied once
  if ((g_doomstat_globals->fastparm || skill == sk_nightmare) && !fast_applied) {
    for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
      // [crispy] Fix infinite loop caused by Demon speed bug
      if (states[i].tics > 1) {
        states[i].tics >>= 1;
      }
    mobjinfo[MT_BRUISERSHOT].speed = 20 * FRACUNIT;
    mobjinfo[MT_HEADSHOT].speed    = 20 * FRACUNIT;
    mobjinfo[MT_TROOPSHOT].speed   = 20 * FRACUNIT;
    fast_applied                   = true;
  } else if (!g_doomstat_globals->fastparm && skill != sk_nightmare && fast_applied) {
    for (int i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
      states[i].tics <<= 1;
    mobjinfo[MT_BRUISERSHOT].speed = 15 * FRACUNIT;
    mobjinfo[MT_HEADSHOT].speed    = 10 * FRACUNIT;
    mobjinfo[MT_TROOPSHOT].speed   = 10 * FRACUNIT;
    fast_applied                   = false;
  }

  // force players to be initialized upon first level load
  for (auto & player : g_doomstat_globals->players)
    player.playerstate = PST_REBORN;

  g_doomstat_globals->usergame      = true; // will be set false if a demo
  g_doomstat_globals->paused        = false;
  g_doomstat_globals->demoplayback  = false;
  g_doomstat_globals->automapactive = false;
  g_doomstat_globals->viewactive    = true;
  g_doomstat_globals->gameepisode   = episode;
  g_doomstat_globals->gamemap       = map;
  g_doomstat_globals->gameskill     = skill;

  // [crispy] CPhipps - total time for all completed levels
  g_doomstat_globals->totalleveltimes = 0;
  defdemotics                         = 0;
  demostarttic                        = gametic; // [crispy] fix revenant internal demo bug

  g_doomstat_globals->viewactive = true;

  // Set the sky to use.
  //
  // Note: This IS broken, but it is how Vanilla Doom behaves.
  // See http://doomwiki.org/wiki/Sky_never_changes_in_Doom_II.
  //
  // Because we set the sky here at the start of a game, not at the
  // start of a level, the sky texture never changes unless we
  // restore from a saved game.  This was fixed before the Doom
  // source release, but this IS the way Vanilla DOS Doom behaves.

  const char * skytexturename;
  if (g_doomstat_globals->gamemode == commercial) {
    skytexturename = DEH_String("SKY3");
    skytexture     = R_TextureNumForName(skytexturename);
    if (g_doomstat_globals->gamemap < 21) {
      skytexturename = DEH_String(g_doomstat_globals->gamemap < 12 ? "SKY1" : "SKY2");
      skytexture     = R_TextureNumForName(skytexturename);
    }
  } else {
    switch (g_doomstat_globals->gameepisode) {
    default:
    case 1:
      skytexturename = "SKY1";
      break;
    case 2:
      skytexturename = "SKY2";
      break;
    case 3:
      skytexturename = "SKY3";
      break;
    case 4: // Special Edition sky
      skytexturename = "SKY4";
      break;
    case 5: // [crispy] Sigil
      skytexturename = "SKY5_ZD";
      if (R_CheckTextureNumForName(DEH_String(skytexturename)) == -1) {
        skytexturename = "SKY3";
      }
      break;
    }
    skytexturename = DEH_String(skytexturename);
    skytexture     = R_TextureNumForName(skytexturename);
  }

  G_DoLoadLevel();
}

//
// DEMO RECORDING
//
constexpr auto DEMOMARKER = 0x80;

// [crispy] demo progress bar and timer widget
int defdemotics = 0, deftotaldemotics;
// [crispy] moved here
static const char * defdemoname;

void G_ReadDemoTiccmd(ticcmd_t * cmd) {
  if (*demo_p == DEMOMARKER) {
    // end of demo data stream
    G_CheckDemoStatus();
    return;
  }

  // [crispy] if demo playback is quit by pressing 'q',
  // stay in the game, hand over controls to the player and
  // continue recording the demo under a different name
  if (gamekeydown[g_m_controls_globals->key_demo_quit] && g_doomstat_globals->singledemo && !g_doomstat_globals->netgame) {
    uint8_t * actualbuffer = demobuffer;
    char *    actualname   = M_StringDuplicate(defdemoname);

    gamekeydown[g_m_controls_globals->key_demo_quit] = false;

    // [crispy] find a new name for the continued demo
    G_RecordDemo(actualname);
    free(actualname);

    // [crispy] discard the newly allocated demo buffer
    Z_Free(demobuffer);
    demobuffer = actualbuffer;

    // [crispy] continue recording
    G_CheckDemoStatus();
    return;
  }

  cmd->forwardmove = (static_cast<signed char>(*demo_p++));
  cmd->sidemove    = (static_cast<signed char>(*demo_p++));

  // If this is a longtics demo, read back in higher resolution

  if (longtics) {
    cmd->angleturn = *demo_p++;
    cmd->angleturn = static_cast<short>(cmd->angleturn | ((*demo_p++) << 8));
  } else {
    cmd->angleturn = static_cast<short>(static_cast<unsigned char>(*demo_p++) << 8);
  }

  cmd->buttons = static_cast<unsigned char>(*demo_p++);

  // [crispy] increase demo tics counter
  // applies to both recording and playback,
  // because G_WriteDemoTiccmd() calls G_ReadDemoTiccmd() once
  defdemotics++;
}

// Increase the size of the demo buffer to allow unlimited demos

static void IncreaseDemoBuffer() {
  // Find the current size

  int current_length = static_cast<int>(demoend - demobuffer);

  // Generate a new buffer twice the size
  int new_length = current_length * 2;

  uint8_t * new_demobuffer = zmalloc<decltype(new_demobuffer)>(static_cast<size_t>(new_length), PU_STATIC, 0);
  uint8_t * new_demop      = new_demobuffer + (demo_p - demobuffer);

  // Copy over the old data

  std::memcpy(new_demobuffer, demobuffer, static_cast<size_t>(current_length));

  // Free the old buffer and point the demo pointers at the new buffer.

  Z_Free(demobuffer);

  demobuffer = new_demobuffer;
  demo_p     = new_demop;
  demoend    = demobuffer + new_length;
}

void G_WriteDemoTiccmd(ticcmd_t * cmd) {
  if (gamekeydown[g_m_controls_globals->key_demo_quit]) // press q to end demo recording
    G_CheckDemoStatus();

  uint8_t * demo_start = demo_p;

  *demo_p++ = static_cast<uint8_t>(cmd->forwardmove);
  *demo_p++ = static_cast<uint8_t>(cmd->sidemove);

  // If this is a longtics demo, record in higher resolution

  if (longtics) {
    *demo_p++ = static_cast<uint8_t>(cmd->angleturn & 0xff);
    *demo_p++ = static_cast<uint8_t>((cmd->angleturn >> 8) & 0xff);
  } else {
    *demo_p++ = static_cast<uint8_t>(cmd->angleturn >> 8);
  }

  *demo_p++ = cmd->buttons;

  // reset demo pointer back
  demo_p = demo_start;

  if (demo_p > demoend - 16) {
    // [crispy] unconditionally disable savegame and demo limits
    /*
    if (vanilla_demo_limit)
    {
        // no more space
        G_CheckDemoStatus ();
        return;
    }
    else
    */
    {
      // Vanilla demo limit disabled: unlimited
      // demo lengths!

      IncreaseDemoBuffer();
    }
  }

  G_ReadDemoTiccmd(cmd); // make SURE it is exactly the same
}

//
// G_RecordDemo
//
void G_RecordDemo(char * name) {
  // [crispy] demo file name suffix counter
  static unsigned int j  = 0;
  FILE *              fp = nullptr;

  // [crispy] the name originally chosen for the demo, i.e. without "-00000"
  if (!orig_demoname) {
    orig_demoname = name;
  }

  g_doomstat_globals->usergame = false;
  size_t demoname_size         = strlen(name) + 5 + 6; // [crispy] + 6 for "-00000"
  demoname                     = zmalloc<decltype(demoname)>(demoname_size, PU_STATIC, nullptr);
  M_snprintf(demoname, demoname_size, "%s.lmp", name);

  // [crispy] prevent overriding demos by adding a file name suffix
  for (; j <= 99999 && (fp = fopen(demoname, "rb")) != nullptr; j++) {
    M_snprintf(demoname, demoname_size, "%s-%05d.lmp", name, j);
    fclose(fp);
  }

  int maxsize = 0x20000;

  //!
  // @arg <size>
  // @category demo
  // @vanilla
  //
  // Specify the demo buffer size (KiB)
  //

  int i = M_CheckParmWithArgs("-maxdemo", 1);
  if (i)
    maxsize = std::atoi(myargv[i + 1]) * 1024;
  demobuffer = zmalloc<decltype(demobuffer)>(static_cast<size_t>(maxsize), PU_STATIC, nullptr);
  demoend    = demobuffer + maxsize;

  g_doomstat_globals->demorecording = true;
}

// Get the demo version code appropriate for the version set in gameversion.
int G_VanillaVersionCode() {
  switch (g_doomstat_globals->gameversion) {
  case exe_doom_1_666:
    return 106;
  case exe_doom_1_7:
    return 107;
  case exe_doom_1_8:
    return 108;
  case exe_doom_1_9:
  default: // All other versions are variants on v1.9:
    return 109;
  }
}

void G_BeginRecording() {
  demo_p = demobuffer;

  //!
  // @category demo
  //
  // Record a high resolution "Doom 1.91" demo.
  //

  longtics = D_NonVanillaRecord(M_ParmExists("-longtics"),
                                "Doom 1.91 demo format");

  // If not recording a longtics demo, record in low res
  g_doomstat_globals->lowres_turn = !longtics;

  if (longtics) {
    *demo_p++ = DOOM_191_VERSION;
  } else if (g_doomstat_globals->gameversion > exe_doom_1_2) {
    *demo_p++ = static_cast<uint8_t>(G_VanillaVersionCode());
  }

  *demo_p++ = static_cast<uint8_t>(g_doomstat_globals->gameskill);
  *demo_p++ = static_cast<uint8_t>(g_doomstat_globals->gameepisode);
  *demo_p++ = static_cast<uint8_t>(g_doomstat_globals->gamemap);
  if (longtics || g_doomstat_globals->gameversion > exe_doom_1_2) {
    *demo_p++ = static_cast<uint8_t>(g_doomstat_globals->deathmatch);
    *demo_p++ = g_doomstat_globals->respawnparm;
    *demo_p++ = g_doomstat_globals->fastparm;
    *demo_p++ = g_doomstat_globals->nomonsters;
    *demo_p++ = static_cast<uint8_t>(g_doomstat_globals->consoleplayer);
  }

  for (bool in_game : g_doomstat_globals->playeringame)
    *demo_p++ = in_game;
}

//
// G_PlayDemo
//

void G_DeferedPlayDemo(cstring_view name) {
  defdemoname = name.c_str();
  gameaction  = ga_playdemo;

  // [crispy] fast-forward demo up to the desired map
  // in demo warp mode or to the end of the demo in continue mode
  if (crispy->demowarp || g_doomstat_globals->demorecording) {
    g_doomstat_globals->nodrawers = true;
    singletics                    = true;
  }
}

// Generate a string describing a demo version

static const char * DemoVersionDescription(int version) {
  static char resultbuf[16];

  switch (version) {
  case 104:
    return "v1.4";
  case 105:
    return "v1.5";
  case 106:
    return "v1.6/v1.666";
  case 107:
    return "v1.7/v1.7a";
  case 108:
    return "v1.8";
  case 109:
    return "v1.9";
  case 111:
    return "v1.91 hack demo?";
  default:
    break;
  }

  // Unknown version.  Perhaps this is a pre-v1.4 IWAD?  If the version
  // byte is in the range 0-4 then it can be a v1.0-v1.2 demo.

  if (version >= 0 && version <= 4) {
    return "v1.0/v1.1/v1.2";
  } else {
    M_snprintf(resultbuf, sizeof(resultbuf), "%i.%i (unknown)", version / 100, version % 100);
    return resultbuf;
  }
}

void G_DoPlayDemo() {
  int  lumpnum, episode, map;
  bool olddemo = false;

  // [crispy] in demo continue mode free the obsolete demo buffer
  // of size 'maxsize' previously allocated in G_RecordDemo()
  if (g_doomstat_globals->demorecording) {
    Z_Free(demobuffer);
  }

  lumpnum    = W_GetNumForName(defdemoname);
  gameaction = ga_nothing;
  demobuffer = cache_lump_num<uint8_t *>(lumpnum, PU_STATIC);
  demo_p     = demobuffer;

  // [crispy] ignore empty demo lumps
  size_t lumplength = W_LumpLength(lumpnum);
  if (lumplength < 0xd) {
    g_doomstat_globals->demoplayback = true;
    G_CheckDemoStatus();
    return;
  }

  int demoversion = *demo_p++;

  if (demoversion >= 0 && demoversion <= 4) {
    olddemo = true;
    demo_p--;
  }

  longtics = false;

  // Longtics demos use the modified format that is generated by cph's
  // hacked "v1.91" doom exe. This is a non-vanilla extension.
  if (D_NonVanillaPlayback(demoversion == DOOM_191_VERSION, lumpnum, "Doom 1.91 demo format")) {
    longtics = true;
  } else if (demoversion != G_VanillaVersionCode() && !(g_doomstat_globals->gameversion <= exe_doom_1_2 && olddemo)) {
    const char * message = "Demo is from a different game version!\n"
                           "(read %i, should be %i)\n"
                           "\n"
                           "*** You may need to upgrade your version "
                           "of Doom to v1.9. ***\n"
                           "    See: https://www.doomworld.com/classicdoom"
                           "/info/patches.php\n"
                           "    This appears to be %s.";

    if (g_doomstat_globals->singledemo)
      I_Error(message, demoversion, G_VanillaVersionCode(), DemoVersionDescription(demoversion));
    // [crispy] make non-fatal
    else {
      fmt::fprintf(stderr, message, demoversion, G_VanillaVersionCode(), DemoVersionDescription(demoversion));
      fmt::fprintf(stderr, "\n");
      g_doomstat_globals->demoplayback = true;
      G_CheckDemoStatus();
      return;
    }
  }

  skill_t skill = static_cast<skill_t>(*demo_p++);
  episode       = *demo_p++;
  map           = *demo_p++;
  if (!olddemo) {
    g_doomstat_globals->deathmatch    = *demo_p++;
    g_doomstat_globals->respawnparm   = *demo_p++;
    g_doomstat_globals->fastparm      = *demo_p++;
    g_doomstat_globals->nomonsters    = *demo_p++;
    g_doomstat_globals->consoleplayer = *demo_p++;
  } else {
    g_doomstat_globals->deathmatch    = 0;
    g_doomstat_globals->respawnparm   = false;
    g_doomstat_globals->fastparm      = false;
    g_doomstat_globals->nomonsters    = false;
    g_doomstat_globals->consoleplayer = 0;
  }

  for (bool & in_game : g_doomstat_globals->playeringame)
    in_game = *demo_p++;

  if (g_doomstat_globals->playeringame[1] || M_CheckParm("-solo-net") > 0
      || M_CheckParm("-netdemo") > 0) {
    g_doomstat_globals->netgame = true;
    netdemo                     = true;
    // [crispy] impossible to continue a multiplayer demo
    g_doomstat_globals->demorecording = false;
  }

  // don't spend a lot of time in loadlevel
  g_doomstat_globals->precache = false;
  // [crispy] support playing demos from savegames
  if (g_doomstat_globals->startloadgame >= 0) {
    M_StringCopy(savename, P_SaveGameFile(g_doomstat_globals->startloadgame), sizeof(savename));
    G_DoLoadGame();
  } else {
    G_InitNew(skill, episode, map);
  }
  g_doomstat_globals->precache = true;
  starttime                    = I_GetTime();
  demostarttic                 = gametic; // [crispy] fix revenant internal demo bug

  g_doomstat_globals->usergame     = false;
  g_doomstat_globals->demoplayback = true;
  // [crispy] update the "singleplayer" variable
  CheckCrispySingleplayer(!g_doomstat_globals->demorecording && !g_doomstat_globals->demoplayback && !g_doomstat_globals->netgame);

  // [crispy] demo progress bar
  {
    int       numplayersingame = 0;
    uint8_t * demo_ptr         = demo_p;

    for (bool in_game : g_doomstat_globals->playeringame) {
      if (in_game) {
        numplayersingame++;
      }
    }

    deftotaldemotics = defdemotics = 0;

    while (*demo_ptr != DEMOMARKER && (demo_ptr - demobuffer) < static_cast<int>(lumplength)) {
      demo_ptr += numplayersingame * (longtics ? 5 : 4);
      deftotaldemotics++;
    }
  }
}

//
// G_TimeDemo
//
void G_TimeDemo(char * name) {
  //!
  // @category video
  // @vanilla
  //
  // Disable rendering the screen entirely.
  //

  g_doomstat_globals->nodrawers = M_CheckParm("-nodraw");

  timingdemo = true;
  singletics = true;

  defdemoname = name;
  gameaction  = ga_playdemo;
}

/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

bool G_CheckDemoStatus() {
  if (timingdemo) {
    int realtics;

    int endtime = I_GetTime();
    realtics    = endtime - starttime;
    float fps   = (static_cast<float>(gametic) * TICRATE) / static_cast<float>(realtics);

    // Prevent recursive calls
    timingdemo                       = false;
    g_doomstat_globals->demoplayback = false;

    I_Error("timed %i gametics in %i realtics (%f fps)",
            gametic,
            realtics,
            fps);
  }

  if (g_doomstat_globals->demoplayback) {
    W_ReleaseLumpName(defdemoname);
    g_doomstat_globals->demoplayback    = false;
    netdemo                             = false;
    g_doomstat_globals->netgame         = false;
    g_doomstat_globals->deathmatch      = false;
    g_doomstat_globals->playeringame[1] = g_doomstat_globals->playeringame[2] = g_doomstat_globals->playeringame[3] = false;
    // [crispy] leave game parameters intact when continuing a demo
    if (!g_doomstat_globals->demorecording) {
      g_doomstat_globals->respawnparm = false;
      g_doomstat_globals->fastparm    = false;
      g_doomstat_globals->nomonsters  = false;
    }
    g_doomstat_globals->consoleplayer = 0;

    // [crispy] in demo continue mode increase the demo buffer and
    // continue recording once we are done with playback
    if (g_doomstat_globals->demorecording) {
      demoend = demo_p;
      IncreaseDemoBuffer();

      g_doomstat_globals->nodrawers = false;
      singletics                    = false;

      // [crispy] start music for the current level
      if (g_doomstat_globals->gamestate == GS_LEVEL) {
        S_Start();
      }

      return true;
    }

    if (g_doomstat_globals->singledemo)
      I_Quit();
    else
      D_AdvanceDemo();

    return true;
  }

  if (g_doomstat_globals->demorecording) {
    *demo_p++ = DEMOMARKER;
    M_WriteFile(demoname, demobuffer, static_cast<int>(demo_p - demobuffer));
    Z_Free(demobuffer);
    g_doomstat_globals->demorecording = false;
    // [crispy] if a new game is started during demo recording, start a new demo
    if (gameaction != ga_newgame) {
      I_Error("Demo %s recorded", demoname);
    } else {
      fmt::fprintf(stderr, "Demo %s recorded\n", demoname);
    }
  }

  return false;
}

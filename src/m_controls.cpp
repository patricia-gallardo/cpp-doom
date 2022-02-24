//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#include "doomkeys.hpp"

#include "m_config.hpp"
#include "m_misc.hpp"
#include "m_controls.hpp"

static m_controls_t m_controls_s = {
  .key_right   = KEY_RIGHTARROW,
  .key_left    = KEY_LEFTARROW,
  .key_reverse = 0,

  .key_up              = KEY_UPARROW,
  .key_alt_up          = 'w',
  .key_down            = KEY_DOWNARROW,
  .key_alt_down        = 's',
  .key_strafeleft      = ',',
  .key_alt_strafeleft  = 'a',
  .key_straferight     = '.',
  .key_alt_straferight = 'd',
  .key_fire            = KEY_RCTRL,
  .key_use             = ' ',
  .key_strafe          = KEY_RALT,
  .key_speed           = KEY_RSHIFT,

  .key_jump          = '/',
  .key_toggleautorun = KEY_CAPSLOCK,
  .key_togglenovert  = 0,

  .key_flyup       = KEY_PGUP,
  .key_flydown     = KEY_INS,
  .key_flycenter   = KEY_HOME,
  .key_lookup      = KEY_PGDN,
  .key_lookdown    = KEY_DEL,
  .key_lookcenter  = KEY_END,
  .key_invleft     = '[',
  .key_invright    = ']',
  .key_useartifact = KEY_ENTER,

  .key_usehealth = 'h',
  .key_invquery  = 'q',
  .key_mission   = 'w',
  .key_invpop    = 'z',
  .key_invkey    = 'k',
  .key_invhome   = KEY_HOME,
  .key_invend    = KEY_END,
  .key_invuse    = KEY_ENTER,
  .key_invdrop   = KEY_BACKSPACE,

  .key_message_refresh = KEY_ENTER,
  .key_pause           = KEY_PAUSE,

  .key_multi_msg       = 't',
  .key_multi_msgplayer = {},

  .key_weapon1 = '1',
  .key_weapon2 = '2',
  .key_weapon3 = '3',
  .key_weapon4 = '4',
  .key_weapon5 = '5',
  .key_weapon6 = '6',
  .key_weapon7 = '7',
  .key_weapon8 = '8',

  .key_arti_quartz       = 0,
  .key_arti_urn          = 0,
  .key_arti_bomb         = 0,
  .key_arti_tome         = 127,
  .key_arti_ring         = 0,
  .key_arti_chaosdevice  = 0,
  .key_arti_shadowsphere = 0,
  .key_arti_wings        = 0,
  .key_arti_torch        = 0,

  .key_arti_all             = KEY_BACKSPACE,
  .key_arti_health          = '\\',
  .key_arti_poisonbag       = '0',
  .key_arti_blastradius     = '9',
  .key_arti_teleport        = '8',
  .key_arti_teleportother   = '7',
  .key_arti_egg             = '6',
  .key_arti_invulnerability = '5',

  .key_demo_quit  = 'q',
  .key_spy        = KEY_F12,
  .key_prevweapon = 0,
  .key_nextweapon = 0,

  .key_map_north     = KEY_UPARROW,
  .key_map_south     = KEY_DOWNARROW,
  .key_map_east      = KEY_RIGHTARROW,
  .key_map_west      = KEY_LEFTARROW,
  .key_map_zoomin    = '=',
  .key_map_zoomout   = '-',
  .key_map_toggle    = KEY_TAB,
  .key_map_maxzoom   = '0',
  .key_map_follow    = 'f',
  .key_map_grid      = 'g',
  .key_map_mark      = 'm',
  .key_map_clearmark = 'c',
  .key_map_overlay   = 'o',
  .key_map_rotate    = 'r',

  .key_menu_activate = KEY_ESCAPE,
  .key_menu_up       = KEY_UPARROW,
  .key_menu_down     = KEY_DOWNARROW,
  .key_menu_left     = KEY_LEFTARROW,
  .key_menu_right    = KEY_RIGHTARROW,
  .key_menu_back     = KEY_BACKSPACE,
  .key_menu_forward  = KEY_ENTER,
  .key_menu_confirm  = 'y',
  .key_menu_abort    = 'n',

  .key_menu_help     = KEY_F1,
  .key_menu_save     = KEY_F2,
  .key_menu_load     = KEY_F3,
  .key_menu_volume   = KEY_F4,
  .key_menu_detail   = KEY_F5,
  .key_menu_qsave    = KEY_F6,
  .key_menu_endgame  = KEY_F7,
  .key_menu_messages = KEY_F8,
  .key_menu_qload    = KEY_F9,
  .key_menu_quit     = KEY_F10,
  .key_menu_gamma    = KEY_F11,

  .key_menu_incscreen       = KEY_EQUALS,
  .key_menu_decscreen       = KEY_MINUS,
  .key_menu_screenshot      = 0,
  .key_menu_cleanscreenshot = 0,
  .key_menu_del             = KEY_DEL,
  .key_menu_nextlevel       = 0,
  .key_menu_reloadlevel     = 0,

  .mousebfire    = 0,
  .mousebstrafe  = 1,
  .mousebforward = 2,

  .mousebjump = -1,

  .mousebstrafeleft  = -1,
  .mousebstraferight = -1,
  .mousebbackward    = -1,
  .mousebuse         = -1,
  .mousebmouselook   = -1,
  .mousebreverse     = -1,

  .mousebprevweapon = 4,
  .mousebnextweapon = 3,

  .joybfire   = 0,
  .joybstrafe = 1,
  .joybuse    = 3,
  .joybspeed  = 2,

  .joybjump = -1,

  .joybstrafeleft  = -1,
  .joybstraferight = -1,

  .joybprevweapon = -1,
  .joybnextweapon = -1,

  .joybmenu    = -1,
  .joybautomap = -1,

  .dclick_use = 1
};
m_controls_t * const g_m_controls_globals = &m_controls_s;

//
// Bind all of the common controls used by Doom and all other games.
//

void M_BindBaseControls() {
  M_BindIntVariable("key_right", &g_m_controls_globals->key_right);
  M_BindIntVariable("key_left", &g_m_controls_globals->key_left);
  M_BindIntVariable("key_up", &g_m_controls_globals->key_up);
  M_BindIntVariable("key_alt_up", &g_m_controls_globals->key_alt_up); // [crispy]
  M_BindIntVariable("key_down", &g_m_controls_globals->key_down);
  M_BindIntVariable("key_alt_down", &g_m_controls_globals->key_alt_down); // [crispy]
  M_BindIntVariable("key_strafeleft", &g_m_controls_globals->key_strafeleft);
  M_BindIntVariable("key_alt_strafeleft", &g_m_controls_globals->key_alt_strafeleft); // [crispy]
  M_BindIntVariable("key_straferight", &g_m_controls_globals->key_straferight);
  M_BindIntVariable("key_alt_straferight", &g_m_controls_globals->key_alt_straferight); // [crispy]
  M_BindIntVariable("key_fire", &g_m_controls_globals->key_fire);
  M_BindIntVariable("key_use", &g_m_controls_globals->key_use);
  M_BindIntVariable("key_strafe", &g_m_controls_globals->key_strafe);
  M_BindIntVariable("key_speed", &g_m_controls_globals->key_speed);

  M_BindIntVariable("mouseb_fire", &g_m_controls_globals->mousebfire);
  M_BindIntVariable("mouseb_strafe", &g_m_controls_globals->mousebstrafe);
  M_BindIntVariable("mouseb_forward", &g_m_controls_globals->mousebforward);

  M_BindIntVariable("joyb_fire", &g_m_controls_globals->joybfire);
  M_BindIntVariable("joyb_strafe", &g_m_controls_globals->joybstrafe);
  M_BindIntVariable("joyb_use", &g_m_controls_globals->joybuse);
  M_BindIntVariable("joyb_speed", &g_m_controls_globals->joybspeed);

  M_BindIntVariable("joyb_menu_activate", &g_m_controls_globals->joybmenu);
  M_BindIntVariable("joyb_toggle_automap", &g_m_controls_globals->joybautomap);

  // Extra controls that are not in the Vanilla versions:

  M_BindIntVariable("joyb_strafeleft", &g_m_controls_globals->joybstrafeleft);
  M_BindIntVariable("joyb_straferight", &g_m_controls_globals->joybstraferight);
  M_BindIntVariable("mouseb_strafeleft", &g_m_controls_globals->mousebstrafeleft);
  M_BindIntVariable("mouseb_straferight", &g_m_controls_globals->mousebstraferight);
  M_BindIntVariable("mouseb_use", &g_m_controls_globals->mousebuse);
  M_BindIntVariable("mouseb_backward", &g_m_controls_globals->mousebbackward);
  M_BindIntVariable("dclick_use", &g_m_controls_globals->dclick_use);
  M_BindIntVariable("key_pause", &g_m_controls_globals->key_pause);
  M_BindIntVariable("key_message_refresh", &g_m_controls_globals->key_message_refresh);

  M_BindIntVariable("key_lookup", &g_m_controls_globals->key_lookup);         // [crispy]
  M_BindIntVariable("key_lookdown", &g_m_controls_globals->key_lookdown);     // [crispy]
  M_BindIntVariable("key_lookcenter", &g_m_controls_globals->key_lookcenter); // [crispy]

  M_BindIntVariable("key_jump", &g_m_controls_globals->key_jump);      // [crispy]
  M_BindIntVariable("mouseb_jump", &g_m_controls_globals->mousebjump); // [crispy]
  M_BindIntVariable("joyb_jump", &g_m_controls_globals->joybjump);     // [crispy]

  M_BindIntVariable("mouseb_mouselook", &g_m_controls_globals->mousebmouselook);    // [crispy]
  M_BindIntVariable("mouseb_reverse", &g_m_controls_globals->mousebreverse);        // [crispy]
  M_BindIntVariable("key_reverse", &g_m_controls_globals->key_reverse);             // [crispy]
  M_BindIntVariable("key_toggleautorun", &g_m_controls_globals->key_toggleautorun); // [crispy]
  M_BindIntVariable("key_togglenovert", &g_m_controls_globals->key_togglenovert);   // [crispy]
}

void M_BindHereticControls() {
  M_BindIntVariable("key_flyup", &g_m_controls_globals->key_flyup);
  M_BindIntVariable("key_flydown", &g_m_controls_globals->key_flydown);
  M_BindIntVariable("key_flycenter", &g_m_controls_globals->key_flycenter);

  // [crispy] key_look* moved to M_BindBaseControls()

  M_BindIntVariable("key_invleft", &g_m_controls_globals->key_invleft);
  M_BindIntVariable("key_invright", &g_m_controls_globals->key_invright);
  M_BindIntVariable("key_useartifact", &g_m_controls_globals->key_useartifact);

  M_BindIntVariable("key_arti_quartz", &g_m_controls_globals->key_arti_quartz);
  M_BindIntVariable("key_arti_urn", &g_m_controls_globals->key_arti_urn);
  M_BindIntVariable("key_arti_bomb", &g_m_controls_globals->key_arti_bomb);
  M_BindIntVariable("key_arti_tome", &g_m_controls_globals->key_arti_tome);
  M_BindIntVariable("key_arti_ring", &g_m_controls_globals->key_arti_ring);
  M_BindIntVariable("key_arti_chaosdevice", &g_m_controls_globals->key_arti_chaosdevice);
  M_BindIntVariable("key_arti_shadowsphere", &g_m_controls_globals->key_arti_shadowsphere);
  M_BindIntVariable("key_arti_wings", &g_m_controls_globals->key_arti_wings);
  M_BindIntVariable("key_arti_torch", &g_m_controls_globals->key_arti_torch);
}

void M_BindHexenControls() {
  // [crispy] *_jump moved to M_BindBaseControls()

  M_BindIntVariable("key_arti_all", &g_m_controls_globals->key_arti_all);
  M_BindIntVariable("key_arti_health", &g_m_controls_globals->key_arti_health);
  M_BindIntVariable("key_arti_poisonbag", &g_m_controls_globals->key_arti_poisonbag);
  M_BindIntVariable("key_arti_blastradius", &g_m_controls_globals->key_arti_blastradius);
  M_BindIntVariable("key_arti_teleport", &g_m_controls_globals->key_arti_teleport);
  M_BindIntVariable("key_arti_teleportother", &g_m_controls_globals->key_arti_teleportother);
  M_BindIntVariable("key_arti_egg", &g_m_controls_globals->key_arti_egg);
  M_BindIntVariable("key_arti_invulnerability", &g_m_controls_globals->key_arti_invulnerability);
}

void M_BindStrifeControls() {
  // These are shared with all games, but have different defaults:
  g_m_controls_globals->key_message_refresh = '/';

  // These keys are shared with Heretic/Hexen but have different defaults:
  g_m_controls_globals->key_jump     = 'a';
  g_m_controls_globals->key_lookup   = KEY_PGUP;
  g_m_controls_globals->key_lookdown = KEY_PGDN;
  g_m_controls_globals->key_invleft  = KEY_INS;
  g_m_controls_globals->key_invright = KEY_DEL;

  M_BindIntVariable("key_jump", &g_m_controls_globals->key_jump);
  M_BindIntVariable("key_lookUp", &g_m_controls_globals->key_lookup);
  M_BindIntVariable("key_lookDown", &g_m_controls_globals->key_lookdown);
  M_BindIntVariable("key_invLeft", &g_m_controls_globals->key_invleft);
  M_BindIntVariable("key_invRight", &g_m_controls_globals->key_invright);

  // Custom Strife-only Keys:
  M_BindIntVariable("key_useHealth", &g_m_controls_globals->key_usehealth);
  M_BindIntVariable("key_invquery", &g_m_controls_globals->key_invquery);
  M_BindIntVariable("key_mission", &g_m_controls_globals->key_mission);
  M_BindIntVariable("key_invPop", &g_m_controls_globals->key_invpop);
  M_BindIntVariable("key_invKey", &g_m_controls_globals->key_invkey);
  M_BindIntVariable("key_invHome", &g_m_controls_globals->key_invhome);
  M_BindIntVariable("key_invEnd", &g_m_controls_globals->key_invend);
  M_BindIntVariable("key_invUse", &g_m_controls_globals->key_invuse);
  M_BindIntVariable("key_invDrop", &g_m_controls_globals->key_invdrop);

  // Strife also supports jump on mouse and joystick, and in the exact same
  // manner as Hexen!
  M_BindIntVariable("mouseb_jump", &g_m_controls_globals->mousebjump);
  M_BindIntVariable("joyb_jump", &g_m_controls_globals->joybjump);
}

void M_BindWeaponControls() {
  M_BindIntVariable("key_weapon1", &g_m_controls_globals->key_weapon1);
  M_BindIntVariable("key_weapon2", &g_m_controls_globals->key_weapon2);
  M_BindIntVariable("key_weapon3", &g_m_controls_globals->key_weapon3);
  M_BindIntVariable("key_weapon4", &g_m_controls_globals->key_weapon4);
  M_BindIntVariable("key_weapon5", &g_m_controls_globals->key_weapon5);
  M_BindIntVariable("key_weapon6", &g_m_controls_globals->key_weapon6);
  M_BindIntVariable("key_weapon7", &g_m_controls_globals->key_weapon7);
  M_BindIntVariable("key_weapon8", &g_m_controls_globals->key_weapon8);

  M_BindIntVariable("key_prevweapon", &g_m_controls_globals->key_prevweapon);
  M_BindIntVariable("key_nextweapon", &g_m_controls_globals->key_nextweapon);

  M_BindIntVariable("joyb_prevweapon", &g_m_controls_globals->joybprevweapon);
  M_BindIntVariable("joyb_nextweapon", &g_m_controls_globals->joybnextweapon);

  M_BindIntVariable("mouseb_prevweapon", &g_m_controls_globals->mousebprevweapon);
  M_BindIntVariable("mouseb_nextweapon", &g_m_controls_globals->mousebnextweapon);
}

void M_BindMapControls() {
  M_BindIntVariable("key_map_north", &g_m_controls_globals->key_map_north);
  M_BindIntVariable("key_map_south", &g_m_controls_globals->key_map_south);
  M_BindIntVariable("key_map_east", &g_m_controls_globals->key_map_east);
  M_BindIntVariable("key_map_west", &g_m_controls_globals->key_map_west);
  M_BindIntVariable("key_map_zoomin", &g_m_controls_globals->key_map_zoomin);
  M_BindIntVariable("key_map_zoomout", &g_m_controls_globals->key_map_zoomout);
  M_BindIntVariable("key_map_toggle", &g_m_controls_globals->key_map_toggle);
  M_BindIntVariable("key_map_maxzoom", &g_m_controls_globals->key_map_maxzoom);
  M_BindIntVariable("key_map_follow", &g_m_controls_globals->key_map_follow);
  M_BindIntVariable("key_map_grid", &g_m_controls_globals->key_map_grid);
  M_BindIntVariable("key_map_mark", &g_m_controls_globals->key_map_mark);
  M_BindIntVariable("key_map_clearmark", &g_m_controls_globals->key_map_clearmark);
  M_BindIntVariable("key_map_overlay", &g_m_controls_globals->key_map_overlay); // [crispy]
  M_BindIntVariable("key_map_rotate", &g_m_controls_globals->key_map_rotate);   // [crispy]
}

void M_BindMenuControls() {
  M_BindIntVariable("key_menu_activate", &g_m_controls_globals->key_menu_activate);
  M_BindIntVariable("key_menu_up", &g_m_controls_globals->key_menu_up);
  M_BindIntVariable("key_menu_down", &g_m_controls_globals->key_menu_down);
  M_BindIntVariable("key_menu_left", &g_m_controls_globals->key_menu_left);
  M_BindIntVariable("key_menu_right", &g_m_controls_globals->key_menu_right);
  M_BindIntVariable("key_menu_back", &g_m_controls_globals->key_menu_back);
  M_BindIntVariable("key_menu_forward", &g_m_controls_globals->key_menu_forward);
  M_BindIntVariable("key_menu_confirm", &g_m_controls_globals->key_menu_confirm);
  M_BindIntVariable("key_menu_abort", &g_m_controls_globals->key_menu_abort);

  M_BindIntVariable("key_menu_help", &g_m_controls_globals->key_menu_help);
  M_BindIntVariable("key_menu_save", &g_m_controls_globals->key_menu_save);
  M_BindIntVariable("key_menu_load", &g_m_controls_globals->key_menu_load);
  M_BindIntVariable("key_menu_volume", &g_m_controls_globals->key_menu_volume);
  M_BindIntVariable("key_menu_detail", &g_m_controls_globals->key_menu_detail);
  M_BindIntVariable("key_menu_qsave", &g_m_controls_globals->key_menu_qsave);
  M_BindIntVariable("key_menu_endgame", &g_m_controls_globals->key_menu_endgame);
  M_BindIntVariable("key_menu_messages", &g_m_controls_globals->key_menu_messages);
  M_BindIntVariable("key_menu_qload", &g_m_controls_globals->key_menu_qload);
  M_BindIntVariable("key_menu_quit", &g_m_controls_globals->key_menu_quit);
  M_BindIntVariable("key_menu_gamma", &g_m_controls_globals->key_menu_gamma);

  M_BindIntVariable("key_menu_incscreen", &g_m_controls_globals->key_menu_incscreen);
  M_BindIntVariable("key_menu_decscreen", &g_m_controls_globals->key_menu_decscreen);
  M_BindIntVariable("key_menu_screenshot", &g_m_controls_globals->key_menu_screenshot);
  M_BindIntVariable("key_menu_cleanscreenshot", &g_m_controls_globals->key_menu_cleanscreenshot); // [crispy]
  M_BindIntVariable("key_menu_del", &g_m_controls_globals->key_menu_del);                         // [crispy]
  M_BindIntVariable("key_demo_quit", &g_m_controls_globals->key_demo_quit);
  M_BindIntVariable("key_spy", &g_m_controls_globals->key_spy);
  M_BindIntVariable("key_menu_nextlevel", &g_m_controls_globals->key_menu_nextlevel);     // [crispy]
  M_BindIntVariable("key_menu_reloadlevel", &g_m_controls_globals->key_menu_reloadlevel); // [crispy]
}

void M_BindChatControls(unsigned int num_players) {
  char         name[32]; // haleyjd: 20 not large enough - Thank you, come again!

  M_BindIntVariable("key_multi_msg", &g_m_controls_globals->key_multi_msg);

  for (unsigned int i = 0; i < num_players; ++i) {
    M_snprintf(name, sizeof(name), "key_multi_msgplayer%i", i + 1);
    M_BindIntVariable(name, &g_m_controls_globals->key_multi_msgplayer[i]);
  }
}

//
// Apply custom patches to the default values depending on the
// platform we are running on.
//

void M_ApplyPlatformDefaults() {
  // no-op. Add your platform-specific patches here.
}

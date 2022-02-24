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

#ifndef __M_CONTROLS_H__
#define __M_CONTROLS_H__

struct m_controls_t {
  int key_right;
  int key_left;
  int key_reverse;

  int key_up;
  int key_alt_up;
  int key_down;
  int key_alt_down;
  int key_strafeleft;
  int key_alt_strafeleft;
  int key_straferight;
  int key_alt_straferight;
  int key_fire;
  int key_use;
  int key_strafe;
  int key_speed;

  int key_jump;
  int key_toggleautorun;
  int key_togglenovert;

  int key_flyup;
  int key_flydown;
  int key_flycenter;
  int key_lookup;
  int key_lookdown;
  int key_lookcenter;
  int key_invleft;
  int key_invright;
  int key_useartifact;

  // villsa [STRIFE] strife keys
  int key_usehealth;
  int key_invquery;
  int key_mission;
  int key_invpop;
  int key_invkey;
  int key_invhome;
  int key_invend;
  int key_invuse;
  int key_invdrop;

  int key_message_refresh;
  int key_pause;

  // Multiplayer chat keys:

  int key_multi_msg;
  int key_multi_msgplayer[8];

  // Weapon selection keys:

  int key_weapon1;
  int key_weapon2;
  int key_weapon3;
  int key_weapon4;
  int key_weapon5;
  int key_weapon6;
  int key_weapon7;
  int key_weapon8;

  int key_arti_quartz;
  int key_arti_urn;
  int key_arti_bomb;
  int key_arti_tome;
  int key_arti_ring;
  int key_arti_chaosdevice;
  int key_arti_shadowsphere;
  int key_arti_wings;
  int key_arti_torch;

  int key_arti_all;
  int key_arti_health;
  int key_arti_poisonbag;
  int key_arti_blastradius;
  int key_arti_teleport;
  int key_arti_teleportother;
  int key_arti_egg;
  int key_arti_invulnerability;

  int key_demo_quit;
  int key_spy;
  int key_prevweapon;
  int key_nextweapon;

  int key_map_north;
  int key_map_south;
  int key_map_east;
  int key_map_west;
  int key_map_zoomin;
  int key_map_zoomout;
  int key_map_toggle;
  int key_map_maxzoom;
  int key_map_follow;
  int key_map_grid;
  int key_map_mark;
  int key_map_clearmark;
  int key_map_overlay;
  int key_map_rotate;

  // menu keys:

  int key_menu_activate;
  int key_menu_up;
  int key_menu_down;
  int key_menu_left;
  int key_menu_right;
  int key_menu_back;
  int key_menu_forward;
  int key_menu_confirm;
  int key_menu_abort;

  int key_menu_help;
  int key_menu_save;
  int key_menu_load;
  int key_menu_volume;
  int key_menu_detail;
  int key_menu_qsave;
  int key_menu_endgame;
  int key_menu_messages;
  int key_menu_qload;
  int key_menu_quit;
  int key_menu_gamma;

  int key_menu_incscreen;
  int key_menu_decscreen;
  int key_menu_screenshot;
  int key_menu_cleanscreenshot; // [crispy]
  int key_menu_del;             // [crispy]
  int key_menu_nextlevel;       // [crispy]
  int key_menu_reloadlevel;     // [crispy]

  //
  // Mouse controls
  //

  int mousebfire;
  int mousebstrafe;
  int mousebforward;

  int mousebjump;

  int mousebstrafeleft;
  int mousebstraferight;
  int mousebbackward;
  int mousebuse;
  int mousebmouselook;
  int mousebreverse;

  int mousebprevweapon;
  int mousebnextweapon;

  //
  // Joystick controls
  //

  int joybfire;
  int joybstrafe;
  int joybuse;
  int joybspeed;

  int joybjump;

  int joybstrafeleft;
  int joybstraferight;

  int joybprevweapon;
  int joybnextweapon;

  int joybmenu;
  int joybautomap;

  // Control whether if a mouse button is double clicked, it acts like
  // "use" has been pressed
  int dclick_use;
};

extern m_controls_t * const g_m_controls_globals;

void M_BindBaseControls();
void M_BindHereticControls();
void M_BindHexenControls();
void M_BindStrifeControls();
void M_BindWeaponControls();
void M_BindMapControls();
void M_BindMenuControls();
void M_BindChatControls(unsigned int num_players);

void M_ApplyPlatformDefaults();

#endif /* #ifndef __M_CONTROLS_H__ */

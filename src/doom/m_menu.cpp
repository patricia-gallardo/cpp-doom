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
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//

#include <array>
#include <cctype>
#include <cstdlib>
#include <string>

#include "cstring_view.hpp"
#include "doomdef.hpp"
#include "doomkeys.hpp"
#include "dstrings.hpp"

#include "d_main.hpp"
#include "deh_main.hpp"

#include "i_input.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_timer.hpp"
#include "i_video.hpp"
#include "m_misc.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

#include "r_local.hpp"

#include "hu_stuff.hpp"

#include "g_game.hpp"

#include "m_argv.hpp"
#include "m_controls.hpp"
#include "p_extsaveg.hpp" // [crispy] savewadfilename
#include "p_saveg.hpp"
#include "p_setup.hpp"

#include "s_sound.hpp"

#include "doomstat.hpp"

// Data.
#include "sounds.hpp"

#include "m_crispy.hpp" // [crispy] Crispness menu
#include "m_menu.hpp"

#include "lump.hpp"
#include "v_trans.hpp" // [crispy] colored "invert mouse" message

extern patch_t * hu_font[HU_FONTSIZE];
extern bool      message_dontfuckwithme;

extern bool chat_on; // in heads-up code

// Show messages has default, 0 = off, 1 = on
int showMessages = 1;

// Blocky mode, has default, 0 = high, 1 = normal
int detailLevel  = 0;
int screenblocks = 10; // [crispy] increased

// temp for screenblocks (0-9)
int screenSize;
int screenSize_min;

// -1 = no quicksave slot picked!
int quickSaveSlot;

// 1 = message to be printed
int messageToPrint;
// ...and here is the message string!
const char * messageString;

// message x & y
[[maybe_unused]] int messx;
[[maybe_unused]] int messy;
int                  messageLastMenuActive;

// timed message = no input from user
bool messageNeedsInput;

void (*messageRoutine)(int response);

// [crispy] intermediate gamma levels
std::array<std::string, 9> gammamsg = { GAMMALVL0, GAMMALVL05, GAMMALVL1, GAMMALVL15, GAMMALVL2, GAMMALVL25, GAMMALVL3, GAMMALVL35, GAMMALVL4 };

// we are going to be entering a savegame string
int         saveStringEnter;
int         saveSlot;           // which slot to save in
int         saveCharIndex;      // which char we're editing
static bool joypadSave = false; // was the save action initiated by joypad?
// old save description before edit
char saveOldString[SAVESTRINGSIZE];

bool inhelpscreens;

constexpr auto SKULLXOFF         = -32;
constexpr auto LINEHEIGHT        = 16;
constexpr auto CRISPY_LINEHEIGHT = 10; // [crispy] Crispness menu

extern bool sendpause;
char        savegamestrings[10][SAVESTRINGSIZE];

char endstring[160];

static bool opldev;

extern bool speedkeydown();

//
// MENU TYPEDEFS
//
struct menuitem_t {
  // 0 = no cursor here, 1 = ok, 2 = arrows ok
  short status {};

  char name[10] {};

  // choice = menu item #.
  // if status = 2,
  //   choice=0:leftarrow,1:rightarrow
  void (*routine)(int choice) {};

  // hotkey in menu
  char   alphaKey {};
  const char * alttext {}; // [crispy] alternative text for the Options menu
};

struct menu_t {
  short           numitems;  // # of menu items
  struct menu_t * prevMenu;  // previous menu
  menuitem_t *    menuitems; // menu items
  void (*routine)();         // draw routine
  short x;
  short y;      // x,y of menu
  short lastOn; // last item user was on in menu
};

short itemOn;           // menu item skull is on
short skullAnimCounter; // skull animation counter
short whichSkull;       // which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
const char * skullName[2] = { "M_SKULL1", "M_SKULL2" };

// current menudef
menu_t * currentMenu;

//
// PROTOTYPES
//
static void M_NewGame(int choice);
static void M_Episode(int choice);
static void M_Expansion(int choice); // [crispy] NRFTL
static void M_ChooseSkill(int choice);
static void M_LoadGame(int choice);
static void M_SaveGame(int choice);
static void M_Options(int choice);
static void M_EndGame(int choice);
static void M_ReadThis(int choice);
static void M_ReadThis2(int choice);
static void M_QuitDOOM(int choice);

static void M_ChangeMessages(int choice);
static void M_ChangeSensitivity(int choice);
static void M_ChangeSensitivity_x2(int choice); // [crispy] mouse sensitivity menu
static void M_ChangeSensitivity_y(int choice);  // [crispy] mouse sensitivity menu
static void M_MouseInvert(int choice);          // [crispy] mouse sensitivity menu
static void M_SfxVol(int choice);
static void M_MusicVol(int choice);
static void M_ChangeDetail(int choice);
void        M_SizeDisplay(int choice); // [crispy] un-static for R_ExecuteSetViewSize()
static void M_Mouse(int choice);       // [crispy] mouse sensitivity menu
static void M_Sound(int choice);

static void M_FinishReadThis(int choice);
static void M_LoadSelect(int choice);
static void M_SaveSelect(int choice);
static void M_ReadSaveStrings();
static void M_QuickSave();
static void M_QuickLoad();

static void M_DrawMainMenu();
static void M_DrawReadThis1();
static void M_DrawReadThis2();
static void M_DrawNewGame();
static void M_DrawEpisode();
static void M_DrawOptions();
static void M_DrawMouse(); // [crispy] mouse sensitivity menu
static void M_DrawSound();
static void M_DrawLoad();
static void M_DrawSave();

static void M_DrawSaveLoadBorder(int x, int y);
static void M_SetupNextMenu(menu_t * menudef);
static void M_DrawThermo(int x, int y, int thermWidth, int thermDot);
static void M_WriteText(int x, int y, cstring_view string);
int         M_StringWidth(cstring_view string); // [crispy] un-static
static int  M_StringHeight(cstring_view string);
static void M_StartMessage(cstring_view string, void (*routine)(int), bool input);
static void M_ClearMenus();

// [crispy] Crispness menu
static void M_CrispnessCur(int choice);
static void M_CrispnessNext(int choice);
static void M_CrispnessPrev(int choice);
static void M_DrawCrispness1();
static void M_DrawCrispness2();
static void M_DrawCrispness3();
static void M_DrawCrispness4();

//
// DOOM MENU
//
enum class main_e
{
  newgame = 0,
  options,
  loadgame,
  savegame,
  readthis,
  quitdoom,
  main_end
};

menuitem_t MainMenu[] = {
  {1,  "M_NGAME",  M_NewGame,  'n'},
  { 1, "M_OPTION", M_Options,  'o'},
  { 1, "M_LOADG",  M_LoadGame, 'l'},
  { 1, "M_SAVEG",  M_SaveGame, 's'},
 // Another hickup with Special edition.
  { 1, "M_RDTHIS", M_ReadThis, 'r'},
  { 1, "M_QUITG",  M_QuitDOOM, 'q'}
};

menu_t MainDef = { static_cast<short>(main_e::main_end), nullptr, MainMenu, M_DrawMainMenu, 97, 64, 0 };

//
// EPISODE SELECT
//
enum class episodes_e
{
  ep1,
  ep2 [[maybe_unused]],
  ep3 [[maybe_unused]],
  ep4 [[maybe_unused]],
  ep5 [[maybe_unused]], // [crispy] Sigil
  ep_end
};

menuitem_t EpisodeMenu[] = {
  {1,  "M_EPI1", M_Episode, 'k'},
  { 1, "M_EPI2", M_Episode, 't'},
  { 1, "M_EPI3", M_Episode, 'i'},
  { 1, "M_EPI4", M_Episode, 't'},
  { 1, "M_EPI5", M_Episode, 's'}  // [crispy] Sigil
};

menu_t EpiDef = {
  static_cast<short>(episodes_e::ep_end), // # of menu items
  &MainDef,                               // previous menu
  EpisodeMenu,                            // menuitem_t ->
  M_DrawEpisode,                          // drawing routine ->
  48,
  63,                                 // x,y
  static_cast<short>(episodes_e::ep1) // lastOn
};

//
// EXPANSION SELECT
//
enum class expansions_e
{
  ex1,
  ex2 [[maybe_unused]],
  ex_end
};

static menuitem_t ExpansionMenu[] = {
  {1,  "M_EPI1", M_Expansion, 'h'},
  { 1, "M_EPI2", M_Expansion, 'n'},
};

static menu_t ExpDef = {
  static_cast<short>(expansions_e::ex_end), // # of menu items
  &MainDef,                                 // previous menu
  ExpansionMenu,                            // menuitem_t ->
  M_DrawEpisode,                            // drawing routine ->
  48,
  63,                                   // x,y
  static_cast<short>(expansions_e::ex1) // lastOn
};

//
// NEW GAME
//
enum class newgame_e
{
  killthings [[maybe_unused]],
  toorough [[maybe_unused]],
  hurtme,
  violence [[maybe_unused]],
  nightmare [[maybe_unused]],
  newg_end
};

menuitem_t NewGameMenu[] = {
  {1,  "M_JKILL", M_ChooseSkill, 'i'},
  { 1, "M_ROUGH", M_ChooseSkill, 'h'},
  { 1, "M_HURT",  M_ChooseSkill, 'h'},
  { 1, "M_ULTRA", M_ChooseSkill, 'u'},
  { 1, "M_NMARE", M_ChooseSkill, 'n'}
};

menu_t NewDef = {
  static_cast<short>(newgame_e::newg_end), // # of menu items
  &EpiDef,                                 // previous menu
  NewGameMenu,                             // menuitem_t ->
  M_DrawNewGame,                           // drawing routine ->
  48,
  63,                                   // x,y
  static_cast<short>(newgame_e::hurtme) // lastOn
};

//
// OPTIONS MENU
//
enum class options_e
{
  endgame,
  messages,
  detail,
  scrnsize,
  option_empty1 [[maybe_unused]],
  mousesens [[maybe_unused]],
  soundvol [[maybe_unused]],
  crispness [[maybe_unused]], // [crispy] Crispness menu
  opt_end
};

menuitem_t OptionsMenu[] = {
  {1,           "M_ENDGAM",     M_EndGame,        'e',                                     ("End Game")        },
  { 1,          "M_MESSG",      M_ChangeMessages, 'm',                                     ("Messages: ")      },
  { 1,          "M_DETAIL",     M_ChangeDetail,   'g',                                     ("Graphic Detail: ")},
  { 2,          "M_SCRNSZ",     M_SizeDisplay,    's',                                     ("Screen Size")     },
  { -1,         "",             0,                '\0'                                   },
  { 1,  "M_MSENS",        M_Mouse,              'm', ("Mouse Sensitivity")                                      }, // [crispy] mouse sensitivity menu
  { 1,   "M_SVOL",        M_Sound,              's',      ("Sound Volume")},
  { 1, "M_CRISPY", M_CrispnessCur,              'c',         ("Crispness")                                      }  // [crispy] Crispness menu
};

menu_t OptionsDef = { static_cast<short>(options_e::opt_end), &MainDef, OptionsMenu, M_DrawOptions, 60, 37, 0 };

// [crispy] mouse sensitivity menu
enum class mouse_e
{
  mouse_horiz,
  mouse_empty1,
  mouse_horiz2,
  mouse_empty2,
  mouse_vert,
  mouse_empty3,
  mouse_invert,
  mouse_end
};

static menuitem_t MouseMenu[] = {
  {2,   "", M_ChangeSensitivity,    'h' },
  { -1, "", 0,                      '\0'},
  { 2,  "", M_ChangeSensitivity_x2, 's' },
  { -1, "", 0,                      '\0'},
  { 2,  "", M_ChangeSensitivity_y,  'v' },
  { -1, "", 0,                      '\0'},
  { 1,  "", M_MouseInvert,          'i' },
};

static menu_t MouseDef = { static_cast<short>(mouse_e::mouse_end), &OptionsDef, MouseMenu, M_DrawMouse, 80, 64, 0 };

// [crispy] Crispness menu
enum class crispness1_e
{
  crispness_sep_rendering,
  crispness_hires,
  crispness_widescreen,
  crispness_uncapped,
  crispness_vsync,
  crispness_smoothscaling,
  crispness_sep_rendering_ [[maybe_unused]],

  crispness_sep_visual,
  crispness_coloredhud,
  crispness_translucency,
  crispness_smoothlight,
  crispness_brightmaps,
  crispness_coloredblood,
  crispness_flipcorpses,
  crispness_sep_visual_ [[maybe_unused]],

  crispness1_next,
  crispness1_prev,
  crispness1_end
};

static menuitem_t Crispness1Menu[] = {
  {-1,  "", 0,                            '\0'},
  { 1,  "", M_CrispyToggleHires,          'h' },
  { 1,  "", M_CrispyToggleWidescreen,     'w' },
  { 1,  "", M_CrispyToggleUncapped,       'u' },
  { 1,  "", M_CrispyToggleVsync,          'v' },
  { 1,  "", M_CrispyToggleSmoothScaling,  's' },
  { -1, "", 0,                            '\0'},
  { -1, "", 0,                            '\0'},
  { 1,  "", M_CrispyToggleColoredhud,     'c' },
  { 1,  "", M_CrispyToggleTranslucency,   'e' },
  { 1,  "", M_CrispyToggleSmoothLighting, 's' },
  { 1,  "", M_CrispyToggleBrightmaps,     'b' },
  { 1,  "", M_CrispyToggleColoredblood,   'c' },
  { 1,  "", M_CrispyToggleFlipcorpses,    'r' },
  { -1, "", 0,                            '\0'},
  { 1,  "", M_CrispnessNext,              'n' },
  { 1,  "", M_CrispnessPrev,              'p' },
};

static menu_t Crispness1Def = { static_cast<short>(crispness1_e::crispness1_end), &OptionsDef, Crispness1Menu, M_DrawCrispness1, 48, 28, 1 };

enum class crispness2_e
{
  crispness_sep_audible,
  crispness_soundfull,
  crispness_soundfix,
  crispness_sndchannels,
  crispness_soundmono,
  crispness_sep_audible_ [[maybe_unused]],

  crispness_sep_navigational,
  crispness_extautomap,
  crispness_automapstats,
  crispness_leveltime,
  crispness_playercoords,
  crispness_secretmessage,
  crispness_sep_navigational_ [[maybe_unused]],

  crispness2_next,
  crispness2_prev,
  crispness2_end
};

static menuitem_t Crispness2Menu[] = {
  {-1,  "", 0,                           '\0'},
  { 1,  "", M_CrispyToggleFullsounds,    'p' },
  { 1,  "", M_CrispyToggleSoundfixes,    'm' },
  { 1,  "", M_CrispyToggleSndChannels,   's' },
  { 1,  "", M_CrispyToggleSoundMono,     'm' },
  { -1, "", 0,                           '\0'},
  { -1, "", 0,                           '\0'},
  { 1,  "", M_CrispyToggleExtAutomap,    'e' },
  { 1,  "", M_CrispyToggleAutomapstats,  's' },
  { 1,  "", M_CrispyToggleLeveltime,     'l' },
  { 1,  "", M_CrispyTogglePlayerCoords,  'p' },
  { 1,  "", M_CrispyToggleSecretmessage, 's' },
  { -1, "", 0,                           '\0'},
  { 1,  "", M_CrispnessNext,             'n' },
  { 1,  "", M_CrispnessPrev,             'p' },
};

static menu_t Crispness2Def = { static_cast<short>(crispness2_e::crispness2_end), &OptionsDef, Crispness2Menu, M_DrawCrispness2, 48, 28, 1 };

enum class crispness3_e
{
  crispness_sep_tactical,
  crispness_freelook,
  crispness_mouselook,
  crispness_bobfactor,
  crispness_centerweapon,
  crispness_weaponsquat,
  crispness_pitch,
  crispness_neghealth,
  crispness_sep_tactical_ [[maybe_unused]],

  crispness_sep_crosshair,
  crispness_crosshair,
  crispness_crosshairtype,
  crispness_crosshairhealth,
  crispness_crosshairtarget,
  crispness_sep_crosshair_ [[maybe_unused]],

  crispness3_next,
  crispness3_prev,
  crispness3_end
};

static menuitem_t Crispness3Menu[] = {
  {-1,  "", 0,                             '\0'},
  { 1,  "", M_CrispyToggleFreelook,        'a' },
  { 1,  "", M_CrispyToggleMouseLook,       'p' },
  { 1,  "", M_CrispyToggleBobfactor,       'p' },
  { 1,  "", M_CrispyToggleCenterweapon,    'c' },
  { 1,  "", M_CrispyToggleWeaponSquat,     'w' },
  { 1,  "", M_CrispyTogglePitch,           'w' },
  { 1,  "", M_CrispyToggleNeghealth,       'n' },
  { -1, "", 0,                             '\0'},
  { -1, "", 0,                             '\0'},
  { 1,  "", M_CrispyToggleCrosshair,       'd' },
  { 1,  "", M_CrispyToggleCrosshairtype,   'c' },
  { 1,  "", M_CrispyToggleCrosshairHealth, 'c' },
  { 1,  "", M_CrispyToggleCrosshairTarget, 'h' },
  { -1, "", 0,                             '\0'},
  { 1,  "", M_CrispnessNext,               'n' },
  { 1,  "", M_CrispnessPrev,               'p' },
};

static menu_t Crispness3Def = { static_cast<short>(crispness3_e::crispness3_end), &OptionsDef, Crispness3Menu, M_DrawCrispness3, 48, 28, 1 };

enum class crispness4_e
{
  crispness_sep_physical,
  crispness_freeaim,
  crispness_jumping,
  crispness_overunder,
  crispness_recoil,
  crispness_sep_physical_ [[maybe_unused]],

  crispness_sep_demos,
  crispness_demotimer,
  crispness_demotimerdir,
  crispness_demobar,
  crispness_sep_demos_ [[maybe_unused]],

  crispness4_next,
  crispness4_prev,
  crispness4_end
};

static menuitem_t Crispness4Menu[] = {
  {-1,  "", 0,                          '\0'},
  { 1,  "", M_CrispyToggleFreeaim,      'v' },
  { 1,  "", M_CrispyToggleJumping,      'a' },
  { 1,  "", M_CrispyToggleOverunder,    'w' },
  { 1,  "", M_CrispyToggleRecoil,       'w' },
  { -1, "", 0,                          '\0'},
  { -1, "", 0,                          '\0'},
  { 1,  "", M_CrispyToggleDemoTimer,    'v' },
  { 1,  "", M_CrispyToggleDemoTimerDir, 'a' },
  { 1,  "", M_CrispyToggleDemoBar,      'w' },
  { -1, "", 0,                          '\0'},
  { 1,  "", M_CrispnessNext,            'n' },
  { 1,  "", M_CrispnessPrev,            'p' },
};

static menu_t Crispness4Def = { static_cast<short>(crispness4_e::crispness4_end), &OptionsDef, Crispness4Menu, M_DrawCrispness4, 48, 28, 1 };

static menu_t * CrispnessMenus[] = {
  &Crispness1Def,
  &Crispness2Def,
  &Crispness3Def,
  &Crispness4Def,
};

static int crispness_cur;

//
// Read This! MENU 1 & 2
//
enum class read_e
{
  rdthsempty1,
  read1_end
};

menuitem_t ReadMenu1[] = {
  {1, "", M_ReadThis2, 0}
};

menu_t ReadDef1 = { static_cast<short>(read_e::read1_end), &MainDef, ReadMenu1, M_DrawReadThis1, 280, 185, 0 };

enum class read_e2
{
  rdthsempty2 [[maybe_unused]],
  read2_end
};

menuitem_t ReadMenu2[] = {
  {1, "", M_FinishReadThis, 0}
};

menu_t ReadDef2 = { static_cast<short>(read_e2::read2_end), &ReadDef1, ReadMenu2, M_DrawReadThis2, 330, 175, 0 };

//
// SOUND VOLUME MENU
//
enum class sound_e
{
  sfx_vol,
  sfx_empty1 [[maybe_unused]],
  music_vol,
  sfx_empty2 [[maybe_unused]],
  sound_end
};

menuitem_t SoundMenu[] = {
  {2,   "M_SFXVOL", M_SfxVol,   's' },
  { -1, "",         0,          '\0'},
  { 2,  "M_MUSVOL", M_MusicVol, 'm' },
  { -1, "",         0,          '\0'}
};

menu_t SoundDef = { static_cast<short>(sound_e::sound_end), &OptionsDef, SoundMenu, M_DrawSound, 80, 64, 0 };

//
// LOAD GAME MENU
//
enum class load_e
{
  load1 [[maybe_unused]],
  load2 [[maybe_unused]],
  load3 [[maybe_unused]],
  load4 [[maybe_unused]],
  load5 [[maybe_unused]],
  load6 [[maybe_unused]],
  load7 [[maybe_unused]], // [crispy] up to 8 savegames
  load8 [[maybe_unused]], // [crispy] up to 8 savegames
  load_end
};

menuitem_t LoadMenu[] = {
  {1,  "", M_LoadSelect, '1'},
  { 1, "", M_LoadSelect, '2'},
  { 1, "", M_LoadSelect, '3'},
  { 1, "", M_LoadSelect, '4'},
  { 1, "", M_LoadSelect, '5'},
  { 1, "", M_LoadSelect, '6'},
  { 1, "", M_LoadSelect, '7'}, // [crispy] up to 8 savegames
  { 1, "", M_LoadSelect, '8'}  // [crispy] up to 8 savegames
};

menu_t LoadDef = { static_cast<short>(load_e::load_end), &MainDef, LoadMenu, M_DrawLoad, 80, 54, 0 };

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[] = {
  {1,  "", M_SaveSelect, '1'},
  { 1, "", M_SaveSelect, '2'},
  { 1, "", M_SaveSelect, '3'},
  { 1, "", M_SaveSelect, '4'},
  { 1, "", M_SaveSelect, '5'},
  { 1, "", M_SaveSelect, '6'},
  { 1, "", M_SaveSelect, '7'}, // [crispy] up to 8 savegames
  { 1, "", M_SaveSelect, '8'}  // [crispy] up to 8 savegames
};

menu_t SaveDef = { static_cast<short>(load_e::load_end), &MainDef, SaveMenu, M_DrawSave, 80, 54, 0 };

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings() {
  for (int i = 0; i < static_cast<int>(load_e::load_end); i++) {
    char name[256];
    M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

    FILE * handle = fopen(name, "rb");
    if (handle == nullptr) {
      M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
      LoadMenu[i].status = 0;
      continue;
    }
    size_t retval = fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
    fclose(handle);
    LoadMenu[i].status = retval == SAVESTRINGSIZE;
  }
}

//
// M_LoadGame & Cie.
//
static int LoadDef_x = 72, LoadDef_y = 28;
void       M_DrawLoad() {
  V_DrawPatchDirect(LoadDef_x, LoadDef_y, cache_lump_name<patch_t *>(DEH_String("M_LOADG"), PU_CACHE));

  for (int i = 0; i < static_cast<int>(load_e::load_end); i++) {
    M_DrawSaveLoadBorder(LoadDef.x, LoadDef.y + LINEHEIGHT * i);

    // [crispy] shade empty savegame slots
    if (!LoadMenu[i].status) dp_translation = cr_colors[static_cast<int>(cr_t::CR_DARK)];

    M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * i, savegamestrings[i]);

    dp_translation = nullptr;
  }
}

//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y) {
  V_DrawPatchDirect(x - 8, y + 7, cache_lump_name<patch_t *>(DEH_String("M_LSLEFT"), PU_CACHE));

  for (int i = 0; i < 24; i++) {
    V_DrawPatchDirect(x, y + 7, cache_lump_name<patch_t *>(DEH_String("M_LSCNTR"), PU_CACHE));
    x += 8;
  }

  V_DrawPatchDirect(x, y + 7, cache_lump_name<patch_t *>(DEH_String("M_LSRGHT"), PU_CACHE));
}

//
// User wants to load this game
//
void M_LoadSelect(int choice) {
  char name[256];

  M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

  // [crispy] save the last game you loaded
  SaveDef.lastOn = static_cast<short>(choice);
  G_LoadGame(name);
  M_ClearMenus();

  // [crispy] allow quickload before quicksave
  if (quickSaveSlot == -2) quickSaveSlot = choice;
}

//
// Selected from DOOM menu
//
void M_LoadGame(int) {
  // [crispy] allow loading game while multiplayer demo playback
  if (g_doomstat_globals->netgame && !g_doomstat_globals->demoplayback) {
    M_StartMessage(DEH_String(LOADNET), nullptr, false);
    return;
  }

  M_SetupNextMenu(&LoadDef);
  M_ReadSaveStrings();
}

//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 72, SaveDef_y = 28;
void       M_DrawSave() {
  V_DrawPatchDirect(SaveDef_x, SaveDef_y, cache_lump_name<patch_t *>(DEH_String("M_SAVEG"), PU_CACHE));
  for (int i = 0; i < static_cast<int>(load_e::load_end); i++) {
    M_DrawSaveLoadBorder(LoadDef.x, LoadDef.y + LINEHEIGHT * i);
    M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * i, savegamestrings[i]);
  }

  if (saveStringEnter) {
    int i = M_StringWidth(savegamestrings[saveSlot]);
    M_WriteText(LoadDef.x + i, LoadDef.y + LINEHEIGHT * saveSlot, "_");
  }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot) {
  G_SaveGame(slot, savegamestrings[slot]);
  M_ClearMenus();

  // PICK QUICKSAVE SLOT YET?
  if (quickSaveSlot == -2) quickSaveSlot = slot;
}

//
// Generate a default save slot name when the user saves to
// an empty slot via the joypad.
//
static void SetDefaultSaveName(int) {
  // map from IWAD or PWAD?
  if (W_IsIWADLump(maplumpinfo) && strcmp(g_doomstat_globals->savegamedir, "") != 0) {
    M_snprintf(savegamestrings[itemOn], SAVESTRINGSIZE, "%s", maplumpinfo->name);
  } else {
    char * wadname = M_StringDuplicate(W_WadNameForLump(maplumpinfo));
    char * ext     = strrchr(wadname, '.');

    if (ext != nullptr) { *ext = '\0'; }

    M_snprintf(savegamestrings[itemOn], SAVESTRINGSIZE, "%s (%s)", maplumpinfo->name, wadname);
    free(wadname);
  }
  M_ForceUppercase(savegamestrings[itemOn]);
  joypadSave = false;
}

// [crispy] override savegame name if it already starts with a map identifier
static bool StartsWithMapIdentifier(char * str) {
  M_ForceUppercase(str);

  if (strlen(str) >= 4 && str[0] == 'E' && isdigit(str[1]) && str[2] == 'M' && isdigit(str[3])) { return true; }

  if (strlen(str) >= 5 && str[0] == 'M' && str[1] == 'A' && str[2] == 'P' && isdigit(str[3]) && isdigit(str[4])) { return true; }

  return false;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice) {
  // we are going to be intercepting all chars
  saveStringEnter = 1;

  // [crispy] load the last game you saved
  LoadDef.lastOn = static_cast<short>(choice);

  // We need to turn on text input:
  int x = LoadDef.x - 11;
  int y = LoadDef.y + choice * LINEHEIGHT - 4;
  I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

  saveSlot = choice;
  M_StringCopy(saveOldString, savegamestrings[choice], SAVESTRINGSIZE);
  if (!strcmp(savegamestrings[choice], EMPTYSTRING) ||
      // [crispy] override savegame name if it already starts with a map identifier
      StartsWithMapIdentifier(savegamestrings[choice])) {
    savegamestrings[choice][0] = 0;

    if (joypadSave || true) // [crispy] always prefill empty savegame slot names
    {
      SetDefaultSaveName(choice);
    }
  }
  saveCharIndex = static_cast<int>(strlen(savegamestrings[choice]));
}

//
// Selected from DOOM menu
//
void M_SaveGame(int) {
  if (!g_doomstat_globals->usergame) {
    M_StartMessage(DEH_String(SAVEDEAD), nullptr, false);
    return;
  }

  if (g_doomstat_globals->gamestate != GS_LEVEL) return;

  M_SetupNextMenu(&SaveDef);
  M_ReadSaveStrings();
}

//
//      M_QuickSave
//
static char tempstring[90];

void M_QuickSaveResponse(int key) {
  if (key == g_m_controls_globals->key_menu_confirm) {
    M_DoSave(quickSaveSlot);
    S_StartSound(nullptr, sfx_swtchx);
  }
}

void M_QuickSave() {
  if (!g_doomstat_globals->usergame) {
    S_StartSound(nullptr, sfx_oof);
    return;
  }

  if (g_doomstat_globals->gamestate != GS_LEVEL) return;

  if (quickSaveSlot < 0) {
    M_StartControlPanel();
    M_ReadSaveStrings();
    M_SetupNextMenu(&SaveDef);
    quickSaveSlot = -2; // means to pick a slot now
    return;
  }
  // [crispy] print savegame name in golden letters
  char * savegamestring = M_StringJoin(
      crstr[static_cast<int>(cr_t::CR_GOLD)],
      savegamestrings[quickSaveSlot],
      crstr[static_cast<int>(cr_t::CR_NONE)]);
  DEH_snprintf(tempstring, sizeof(tempstring), QSPROMPT, savegamestring);
  free(savegamestring);
  M_StartMessage(tempstring, M_QuickSaveResponse, true);
}

//
// M_QuickLoad
//
void M_QuickLoadResponse(int key) {
  if (key == g_m_controls_globals->key_menu_confirm) {
    M_LoadSelect(quickSaveSlot);
    S_StartSound(nullptr, sfx_swtchx);
  }
}

void M_QuickLoad() {
  // [crispy] allow quickloading game while multiplayer demo playback
  if (g_doomstat_globals->netgame && !g_doomstat_globals->demoplayback) {
    M_StartMessage(DEH_String(QLOADNET), nullptr, false);
    return;
  }

  if (quickSaveSlot < 0) {
    // [crispy] allow quickload before quicksave
    M_StartControlPanel();
    M_ReadSaveStrings();
    M_SetupNextMenu(&LoadDef);
    quickSaveSlot = -2;
    return;
  }
  // [crispy] print savegame name in golden letters
  char * savegamestring = M_StringJoin(
      crstr[static_cast<int>(cr_t::CR_GOLD)],
      savegamestrings[quickSaveSlot],
      crstr[static_cast<int>(cr_t::CR_NONE)]);
  DEH_snprintf(tempstring, sizeof(tempstring), QLPROMPT, savegamestring);
  free(savegamestring);
  M_StartMessage(tempstring, M_QuickLoadResponse, true);
}

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1() {
  inhelpscreens = true;

  V_DrawPatchFullScreen(cache_lump_name<patch_t *>(DEH_String("HELP2"), PU_CACHE), false);
}

//
// Read This Menus - optional second page.
//
void M_DrawReadThis2() {
  inhelpscreens = true;

  // We only ever draw the second page if this is
  // gameversion == exe_doom_1_9 and gamemode == registered

  V_DrawPatchFullScreen(cache_lump_name<patch_t *>(DEH_String("HELP1"), PU_CACHE), false);
}

void M_DrawReadThisCommercial() {
  inhelpscreens = true;

  V_DrawPatchFullScreen(cache_lump_name<patch_t *>(DEH_String("HELP"), PU_CACHE), false);
}

//
// Change Sfx & Music volumes
//
void M_DrawSound() {
  V_DrawPatchDirect(60, 38, cache_lump_name<patch_t *>(DEH_String("M_SVOL"), PU_CACHE));

  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (static_cast<int>(sound_e::sfx_vol) + 1), 16, g_doomstat_globals->sfxVolume);

  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (static_cast<int>(sound_e::music_vol) + 1), 16, g_doomstat_globals->musicVolume);
}

void M_Sound(int) {
  M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice) {
  switch (choice) {
  case 0:
    if (g_doomstat_globals->sfxVolume) g_doomstat_globals->sfxVolume--;
    break;
  case 1:
    if (g_doomstat_globals->sfxVolume < 15) g_doomstat_globals->sfxVolume++;
    break;
  }

  S_SetSfxVolume(g_doomstat_globals->sfxVolume * 8);
}

void M_MusicVol(int choice) {
  switch (choice) {
  case 0:
    if (g_doomstat_globals->musicVolume) g_doomstat_globals->musicVolume--;
    break;
  case 1:
    if (g_doomstat_globals->musicVolume < 15) g_doomstat_globals->musicVolume++;
    break;
  }

  S_SetMusicVolume(g_doomstat_globals->musicVolume * 8);
}

//
// M_DrawMainMenu
//
void M_DrawMainMenu() {
  V_DrawPatchDirect(94, 2, cache_lump_name<patch_t *>(DEH_String("M_DOOM"), PU_CACHE));
}

//
// M_NewGame
//
void M_DrawNewGame() {
  V_DrawPatchDirect(96, 14, cache_lump_name<patch_t *>(DEH_String("M_NEWG"), PU_CACHE));
  V_DrawPatchDirect(54, 38, cache_lump_name<patch_t *>(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int) {
  // [crispy] forbid New Game while recording a demo
  if (g_doomstat_globals->demorecording) { return; }

  if (g_doomstat_globals->netgame && !g_doomstat_globals->demoplayback) {
    M_StartMessage(DEH_String(NEWGAME), nullptr, false);
    return;
  }

  // Chex Quest disabled the episode select screen, as did Doom II.

  if (g_doomstat_globals->nervewadfile)
    M_SetupNextMenu(&ExpDef);
  else if (g_doomstat_globals->gamemode == commercial || g_doomstat_globals->gameversion == exe_chex)
    M_SetupNextMenu(&NewDef);
  else
    M_SetupNextMenu(&EpiDef);
}

//
//      M_Episode
//
int epi;

void M_DrawEpisode() {
  V_DrawPatchDirect(54, 38, cache_lump_name<patch_t *>(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key) {
  if (key != g_m_controls_globals->key_menu_confirm) return;

  G_DeferedInitNew(skill_t::sk_nightmare, epi + 1, 1);
  M_ClearMenus();
}

void M_ChooseSkill(int choice) {
  if (choice == skill_t::sk_nightmare) {
    M_StartMessage(DEH_String(NIGHTMARE), M_VerifyNightmare, true);
    return;
  }

  G_DeferedInitNew(static_cast<skill_t>(choice), epi + 1, 1);
  M_ClearMenus();
}

void M_Episode(int choice) {
  if ((g_doomstat_globals->gamemode == shareware) && choice) {
    M_StartMessage(DEH_String(SWSTRING), nullptr, false);
    M_SetupNextMenu(&ReadDef1);
    return;
  }

  epi = choice;
  M_SetupNextMenu(&NewDef);
}

static void M_Expansion(int choice) {
  epi = choice;
  M_SetupNextMenu(&NewDef);
}

//
// M_Options
//
// [crispy] no patches are drawn in the Options menu anymore
/*
static const char *detailNames[2] = {"M_GDHIGH","M_GDLOW"};
static const char *msgNames[2] = {"M_MSGOFF","M_MSGON"};
*/

void M_DrawOptions() {
  V_DrawPatchDirect(108, 15, cache_lump_name<patch_t *>(DEH_String("M_OPTTTL"), PU_CACHE));

  // [crispy] no patches are drawn in the Options menu anymore
  /*
  V_DrawPatchDirect(OptionsDef.x + 175, OptionsDef.y + LINEHEIGHT * detail,
                    cache_lump_name<patch_t *>(DEH_String(detailNames[detailLevel]),
                                    PU_CACHE));
*/

  M_WriteText(OptionsDef.x + M_StringWidth("Graphic Detail: "),
              OptionsDef.y + LINEHEIGHT * static_cast<int>(options_e::detail) + 8 - (M_StringHeight("HighLow") / 2),
              detailLevel ? "Low" : "High");

  // [crispy] no patches are drawn in the Options menu anymore
  /*
  V_DrawPatchDirect(OptionsDef.x + 120, OptionsDef.y + LINEHEIGHT * messages,
                    cache_lump_name<patch_t *>(DEH_String(msgNames[showMessages]),
                                    PU_CACHE));
*/
  M_WriteText(OptionsDef.x + M_StringWidth("Messages: "),
              OptionsDef.y + LINEHEIGHT * static_cast<int>(options_e::messages) + 8 - (M_StringHeight("OnOff") / 2),
              showMessages ? "On" : "Off");

  M_DrawThermo(OptionsDef.x + screenSize_min * 8, OptionsDef.y + LINEHEIGHT * (static_cast<int>(options_e::scrnsize) + 1), 9 + 3 - screenSize_min, screenSize - screenSize_min); // [crispy] Crispy HUD
}

// [crispy] mouse sensitivity menu
static void M_DrawMouse() {
  char mouse_menu_text[48];

  V_DrawPatchDirect(60, LoadDef_y, cache_lump_name<patch_t *>(DEH_String("M_MSENS"), PU_CACHE));

  M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_horiz) + 6, "HORIZONTAL: TURN");

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_empty1), 21, g_doomstat_globals->mouseSensitivity);

  M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_horiz2) + 6, "HORIZONTAL: STRAFE");

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_empty2), 21, g_doomstat_globals->mouseSensitivity_x2);

  M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_vert) + 6, "VERTICAL");

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_empty3), 21, g_doomstat_globals->mouseSensitivity_y);

  M_snprintf(mouse_menu_text, sizeof(mouse_menu_text), "%sInvert Vertical Axis: %s%s", crstr[static_cast<int>(cr_t::CR_NONE)], mouse_y_invert ? crstr[static_cast<int>(cr_t::CR_GREEN)] : crstr[static_cast<int>(cr_t::CR_DARK)], mouse_y_invert ? "On" : "Off");
  M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * static_cast<int>(mouse_e::mouse_invert) + 6, mouse_menu_text);

  dp_translation = nullptr;
}

// [crispy] Crispness menu
#include "m_background.hpp"
#include "memory.hpp"
static void M_DrawCrispnessBackground() {
  const uint8_t * const src  = crispness_background;
  pixel_t *             dest = g_i_video_globals->I_VideoBuffer;

  for (int y = 0; y < SCREENHEIGHT; y++) {
    for (int x = 0; x < SCREENWIDTH; x++) {
#ifndef CRISPY_TRUECOLOR
      *dest++ = src[(y & 63) * 64 + (x & 63)];
#else
      *dest++ = colormaps[src[(y & 63) * 64 + (x & 63)]];
#endif
    }
  }

  inhelpscreens = true;
}

static char crispy_menu_text[48];

static void M_DrawCrispnessHeader(const char * item) {
  M_snprintf(crispy_menu_text, sizeof(crispy_menu_text), "%s%s", crstr[static_cast<int>(cr_t::CR_GOLD)], item);
  M_WriteText(ORIGWIDTH / 2 - M_StringWidth(item) / 2, 12, crispy_menu_text);
}

static void M_DrawCrispnessSeparator(int y, const char * item) {
  M_snprintf(crispy_menu_text, sizeof(crispy_menu_text), "%s%s", crstr[static_cast<int>(cr_t::CR_GOLD)], item);
  M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessItem(int y, const char * item, int feat, bool cond) {
  M_snprintf(crispy_menu_text, sizeof(crispy_menu_text), "%s%s: %s%s", cond ? crstr[static_cast<int>(cr_t::CR_NONE)] : crstr[static_cast<int>(cr_t::CR_DARK)], item, cond ? (feat ? crstr[static_cast<int>(cr_t::CR_GREEN)] : crstr[static_cast<int>(cr_t::CR_DARK)]) : crstr[static_cast<int>(cr_t::CR_DARK)], cond && feat ? "On" : "Off");
  M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessMultiItem(int y, const char * item, multiitem_t * multiitem, int feat, bool cond) {
  M_snprintf(crispy_menu_text, sizeof(crispy_menu_text), "%s%s: %s%s", cond ? crstr[static_cast<int>(cr_t::CR_NONE)] : crstr[static_cast<int>(cr_t::CR_DARK)], item, cond ? (feat ? crstr[static_cast<int>(cr_t::CR_GREEN)] : crstr[static_cast<int>(cr_t::CR_DARK)]) : crstr[static_cast<int>(cr_t::CR_DARK)], cond && feat ? multiitem[feat].name : multiitem[0].name);
  M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessGoto(int y, const char * item) {
  M_snprintf(crispy_menu_text, sizeof(crispy_menu_text), "%s%s", crstr[static_cast<int>(cr_t::CR_GOLD)], item);
  M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispness1() {
  M_DrawCrispnessBackground();

  M_DrawCrispnessHeader(("Crispness 1/4"));

  M_DrawCrispnessSeparator(static_cast<int>(crispness1_e::crispness_sep_rendering), ("Rendering"));
  M_DrawCrispnessItem(
      static_cast<int>(crispness1_e::crispness_hires),
      ("High Resolution Rendering"),
      crispy->hires,
      true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness1_e::crispness_widescreen), ("Widescreen Rendering"), multiitem_widescreen, crispy->widescreen, g_i_video_globals->aspect_ratio_correct);
  M_DrawCrispnessItem(
      static_cast<int>(crispness1_e::crispness_uncapped),
      ("Uncapped Framerate"),
      crispy->uncapped,
      true);
  M_DrawCrispnessItem(
      static_cast<int>(crispness1_e::crispness_vsync),
      ("Enable VSync"),
      crispy->vsync,
      !g_i_video_globals->force_software_renderer);
  M_DrawCrispnessItem(
      static_cast<int>(crispness1_e::crispness_smoothscaling),
      ("Smooth Pixel Scaling"),
      crispy->smoothscaling,
      true);

  M_DrawCrispnessSeparator(static_cast<int>(crispness1_e::crispness_sep_visual), ("Visual"));
  M_DrawCrispnessMultiItem(static_cast<int>(crispness1_e::crispness_coloredhud), ("Colorize HUD Elements"), multiitem_coloredhud, crispy->coloredhud, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness1_e::crispness_translucency), ("Enable Translucency"), multiitem_translucency, crispy->translucency, true);
  M_DrawCrispnessItem(static_cast<int>(crispness1_e::crispness_smoothlight), ("Smooth Diminishing Lighting"), crispy->smoothlight, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness1_e::crispness_brightmaps), ("Apply Brightmaps to"), multiitem_brightmaps, crispy->brightmaps, true);
  M_DrawCrispnessItem(static_cast<int>(crispness1_e::crispness_coloredblood), ("Colored Blood and Corpses"), crispy->coloredblood, g_doomstat_globals->gameversion != exe_chex);
  M_DrawCrispnessItem(static_cast<int>(crispness1_e::crispness_flipcorpses), ("Randomly Mirrored Corpses"), crispy->flipcorpses, g_doomstat_globals->gameversion != exe_chex);

  M_DrawCrispnessGoto(static_cast<int>(crispness1_e::crispness1_next), ("Next Page >"));
  M_DrawCrispnessGoto(static_cast<int>(crispness1_e::crispness1_prev), ("< Last Page"));

  dp_translation = nullptr;
}

static void M_DrawCrispness2() {
  M_DrawCrispnessBackground();

  M_DrawCrispnessHeader(("Crispness 2/4"));

  M_DrawCrispnessSeparator(static_cast<int>(crispness2_e::crispness_sep_audible), ("Audible"));
  M_DrawCrispnessItem(
      static_cast<int>(crispness2_e::crispness_soundfull),
      ("Play sounds in full length"),
      crispy->soundfull,
      true);
  M_DrawCrispnessItem(
      static_cast<int>(crispness2_e::crispness_soundfix),
      ("Misc. Sound Fixes"),
      crispy->soundfix,
      true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness2_e::crispness_sndchannels), ("Sound Channels"), multiitem_sndchannels, snd_channels >> 4, g_i_sound_globals->snd_sfxdevice != SNDDEVICE_PCSPEAKER);
  M_DrawCrispnessItem(static_cast<int>(crispness2_e::crispness_soundmono), ("Mono SFX"), crispy->soundmono, true);

  M_DrawCrispnessSeparator(static_cast<int>(crispness2_e::crispness_sep_navigational), ("Navigational"));
  M_DrawCrispnessItem(
      static_cast<int>(crispness2_e::crispness_extautomap),
      ("Extended Automap colors"),
      crispy->extautomap,
      true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness2_e::crispness_automapstats), ("Show Level Stats"), multiitem_widgets, crispy->automapstats, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness2_e::crispness_leveltime), ("Show Level Time"), multiitem_widgets, crispy->leveltime, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness2_e::crispness_playercoords), ("Show Player Coords"), multiitem_widgets, crispy->playercoords, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness2_e::crispness_secretmessage), ("Report Revealed Secrets"), multiitem_secretmessage, crispy->secretmessage, true);

  M_DrawCrispnessGoto(static_cast<int>(crispness2_e::crispness2_next), ("Next Page >"));
  M_DrawCrispnessGoto(static_cast<int>(crispness2_e::crispness2_prev), ("< Prev Page"));

  dp_translation = nullptr;
}

static void M_DrawCrispness3() {
  M_DrawCrispnessBackground();

  M_DrawCrispnessHeader(("Crispness 3/4"));

  M_DrawCrispnessSeparator(static_cast<int>(crispness3_e::crispness_sep_tactical), ("Tactical"));

  M_DrawCrispnessMultiItem(static_cast<int>(crispness3_e::crispness_freelook), ("Allow Free Look"), multiitem_freelook, crispy->freelook, true);
  M_DrawCrispnessItem(
      static_cast<int>(crispness3_e::crispness_mouselook),
      ("Permanent Mouse Look"),
      crispy->mouselook,
      true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness3_e::crispness_bobfactor), ("Player View/Weapon Bobbing"), multiitem_bobfactor, crispy->bobfactor, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness3_e::crispness_centerweapon), ("Weapon Attack Alignment"), multiitem_centerweapon, crispy->centerweapon, crispy->bobfactor != BOBFACTOR_OFF);
  M_DrawCrispnessItem(static_cast<int>(crispness3_e::crispness_weaponsquat), ("Squat weapon down on impact"), crispy->weaponsquat, true);
  M_DrawCrispnessItem(static_cast<int>(crispness3_e::crispness_pitch), ("Weapon Recoil Pitch"), crispy->pitch, true);
  M_DrawCrispnessItem(
      static_cast<int>(crispness3_e::crispness_neghealth),
      ("Negative Player Health"),
      crispy->neghealth,
      true);
  //  M_DrawCrispnessItem(crispness_extsaveg, "Extended Savegames", crispy->extsaveg, true);

  M_DrawCrispnessSeparator(static_cast<int>(crispness3_e::crispness_sep_crosshair), ("Crosshair"));

  M_DrawCrispnessMultiItem(static_cast<int>(crispness3_e::crispness_crosshair), ("Draw Crosshair"), multiitem_crosshair, crispy->crosshair, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness3_e::crispness_crosshairtype), ("Crosshair Shape"), multiitem_crosshairtype, crispy->crosshairtype + 1, crispy->crosshair);
  M_DrawCrispnessItem(static_cast<int>(crispness3_e::crispness_crosshairhealth), ("Color indicates Health"), crispy->crosshairhealth, crispy->crosshair);
  M_DrawCrispnessItem(static_cast<int>(crispness3_e::crispness_crosshairtarget), ("Highlight on target"), crispy->crosshairtarget, crispy->crosshair);

  M_DrawCrispnessGoto(static_cast<int>(crispness3_e::crispness3_next), ("Next Page >"));
  M_DrawCrispnessGoto(static_cast<int>(crispness3_e::crispness3_prev), ("< Prev Page"));

  dp_translation = nullptr;
}

static void M_DrawCrispness4() {
  M_DrawCrispnessBackground();

  M_DrawCrispnessHeader(("Crispness 4/4"));

  M_DrawCrispnessSeparator(static_cast<int>(crispness4_e::crispness_sep_physical), ("Physical"));

  M_DrawCrispnessMultiItem(static_cast<int>(crispness4_e::crispness_freeaim), ("Vertical Aiming"), multiitem_freeaim, crispy->freeaim, crispy->singleplayer);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness4_e::crispness_jumping), ("Allow Jumping"), multiitem_jump, crispy->jump, crispy->singleplayer);
  M_DrawCrispnessItem(static_cast<int>(crispness4_e::crispness_overunder), ("Walk over/under Monsters"), crispy->overunder, crispy->singleplayer);
  M_DrawCrispnessItem(
      static_cast<int>(crispness4_e::crispness_recoil),
      ("Weapon Recoil Thrust"),
      crispy->recoil,
      crispy->singleplayer);

  M_DrawCrispnessSeparator(static_cast<int>(crispness4_e::crispness_sep_demos), ("Demos"));

  M_DrawCrispnessMultiItem(static_cast<int>(crispness4_e::crispness_demotimer), ("Show Demo Timer"), multiitem_demotimer, crispy->demotimer, true);
  M_DrawCrispnessMultiItem(static_cast<int>(crispness4_e::crispness_demotimerdir), ("Playback Timer Direction"), multiitem_demotimerdir, crispy->demotimerdir + 1, crispy->demotimer & DEMOTIMER_PLAYBACK);
  M_DrawCrispnessItem(
      static_cast<int>(crispness4_e::crispness_demobar),
      ("Show Demo Progress Bar"),
      crispy->demobar,
      true);

  M_DrawCrispnessGoto(static_cast<int>(crispness4_e::crispness4_next), ("First Page >"));
  M_DrawCrispnessGoto(static_cast<int>(crispness4_e::crispness4_prev), ("< Prev Page"));

  dp_translation = nullptr;
}

void M_Options(int) {
  M_SetupNextMenu(&OptionsDef);
}

// [crispy] correctly handle inverted y-axis
static void M_Mouse(int) {
  if (g_doomstat_globals->mouseSensitivity_y < 0) {
    g_doomstat_globals->mouseSensitivity_y = -g_doomstat_globals->mouseSensitivity_y;
    mouse_y_invert                         = 1;
  }

  if (mouse_acceleration_y < 0) {
    mouse_acceleration_y = -mouse_acceleration_y;
    mouse_y_invert       = 1;
  }

  M_SetupNextMenu(&MouseDef);
}

static void M_CrispnessCur(int) {
  M_SetupNextMenu(CrispnessMenus[crispness_cur]);
}

static void M_CrispnessNext(int) {
  if (++crispness_cur > static_cast<int>(std::size(CrispnessMenus)) - 1) { crispness_cur = 0; }

  M_CrispnessCur(0);
}

static void M_CrispnessPrev(int) {
  if (--crispness_cur < 0) { crispness_cur = static_cast<int>(std::size(CrispnessMenus) - 1); }

  M_CrispnessCur(0);
}

//
//      Toggle messages on/off
//
void M_ChangeMessages(int) {
  showMessages = 1 - showMessages;

  if (!showMessages)
    g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(MSGOFF);
  else
    g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(MSGON);

  message_dontfuckwithme = true;
}

//
// M_EndGame
//
void M_EndGameResponse(int key) {
  if (key != g_m_controls_globals->key_menu_confirm) return;

  // [crispy] killough 5/26/98: make endgame quit if recording or playing back demo
  if (g_doomstat_globals->demorecording || g_doomstat_globals->singledemo) G_CheckDemoStatus();

  // [crispy] clear quicksave slot
  quickSaveSlot       = -1;
  currentMenu->lastOn = itemOn;
  M_ClearMenus();
  D_StartTitle();
}

void M_EndGame(int) {
  if (!g_doomstat_globals->usergame) {
    S_StartSound(nullptr, sfx_oof);
    return;
  }

  if (g_doomstat_globals->netgame) {
    M_StartMessage(DEH_String(NETEND), nullptr, false);
    return;
  }

  M_StartMessage(DEH_String(ENDGAME), M_EndGameResponse, true);
}

//
// M_ReadThis
//
void M_ReadThis(int) {
  M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int) {
  M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int) {
  M_SetupNextMenu(&MainDef);
}

//
// M_QuitDOOM
//
int quitsounds[8] = { sfx_pldeth, sfx_dmpain, sfx_popain, sfx_slop, sfx_telept, sfx_posit1, sfx_posit3, sfx_sgtatk };

int quitsounds2[8] = { sfx_vilact, sfx_getpow, sfx_boscub, sfx_slop, sfx_skeswg, sfx_kntdth, sfx_bspact, sfx_sgtatk };

void M_QuitResponse(int key) {
  extern int show_endoom;

  if (key != g_m_controls_globals->key_menu_confirm) return;

  // [crispy] play quit sound only if the ENDOOM screen is also shown
  if (!g_doomstat_globals->netgame && show_endoom) {
    if (g_doomstat_globals->gamemode == commercial)
      S_StartSound(nullptr, quitsounds2[(gametic >> 2) & 7]);
    else
      S_StartSound(nullptr, quitsounds[(gametic >> 2) & 7]);
    I_WaitVBL(105);
  }
  I_Quit();
}

static const char * M_SelectEndMessage() {
  const char ** endmsg = nullptr;

  if (logical_gamemission() == doom) {
    // Doom 1

    endmsg = doom1_endmsg;
  } else {
    // Doom 2

    endmsg = doom2_endmsg;
  }

  return endmsg[gametic % NUM_QUITMESSAGES];
}

void M_QuitDOOM(int) {
  // [crispy] fast exit if "run" key is held down
  if (speedkeydown()) I_Quit();

  DEH_snprintf(endstring,
               sizeof(endstring),
               "%s\n\n"
               "(press y to quit.)",
               DEH_String(M_SelectEndMessage()));

  M_StartMessage(endstring, M_QuitResponse, true);
}

void M_ChangeSensitivity(int choice) {
  switch (choice) {
  case 0:
    if (g_doomstat_globals->mouseSensitivity) g_doomstat_globals->mouseSensitivity--;
    break;
  case 1:
    if (g_doomstat_globals->mouseSensitivity < 255) // [crispy] extended range
      g_doomstat_globals->mouseSensitivity++;
    break;
  }
}

void M_ChangeSensitivity_x2(int choice) {
  switch (choice) {
  case 0:
    if (g_doomstat_globals->mouseSensitivity_x2) g_doomstat_globals->mouseSensitivity_x2--;
    break;
  case 1:
    if (g_doomstat_globals->mouseSensitivity_x2 < 255) // [crispy] extended range
      g_doomstat_globals->mouseSensitivity_x2++;
    break;
  }
}

static void M_ChangeSensitivity_y(int choice) {
  switch (choice) {
  case 0:
    if (g_doomstat_globals->mouseSensitivity_y) g_doomstat_globals->mouseSensitivity_y--;
    break;
  case 1:
    if (g_doomstat_globals->mouseSensitivity_y < 255) // [crispy] extended range
      g_doomstat_globals->mouseSensitivity_y++;
    break;
  }
}

static void M_MouseInvert(int) {
  mouse_y_invert = !mouse_y_invert;
}

void M_ChangeDetail(int) {
  detailLevel = 1 - detailLevel;

  R_SetViewSize(screenblocks, detailLevel);

  if (!detailLevel)
    g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(DETAILHI);
  else
    g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(DETAILLO);
}

void M_SizeDisplay(int choice) {
  // [crispy] initialize screenSize_min
  screenSize_min = crispy->widescreen ? 8 : 0;

  switch (choice) {
  case 0:
    if (screenSize > screenSize_min) {
      screenblocks--;
      screenSize--;
    }
    break;
  case 1:
    if (screenSize < 8 + 3) // [crispy] Crispy HUD
    {
      screenblocks++;
      screenSize++;
    }
    break;
  }

  // [crispy] initialize screenSize_min
  if (choice == 0 || choice == 1) { R_SetViewSize(screenblocks, detailLevel); }
}

//
//      Menu Functions
//
void M_DrawThermo(int x, int y, int thermWidth, int thermDot) {
  char num[4];

  if (!thermDot) { dp_translation = cr_colors[static_cast<int>(cr_t::CR_DARK)]; }

  int xx = x;
  V_DrawPatchDirect(xx, y, cache_lump_name<patch_t *>(DEH_String("M_THERML"), PU_CACHE));
  xx += 8;
  for (int i = 0; i < thermWidth; i++) {
    V_DrawPatchDirect(xx, y, cache_lump_name<patch_t *>(DEH_String("M_THERMM"), PU_CACHE));
    xx += 8;
  }
  V_DrawPatchDirect(xx, y, cache_lump_name<patch_t *>(DEH_String("M_THERMR"), PU_CACHE));

  M_snprintf(num, 4, "%3d", thermDot);
  M_WriteText(xx + 8, y + 3, num);

  // [crispy] do not crash anymore if value exceeds thermometer range
  if (thermDot >= thermWidth) {
    thermDot       = thermWidth - 1;
    dp_translation = cr_colors[static_cast<int>(cr_t::CR_DARK)];
  }

  V_DrawPatchDirect((x + 8) + thermDot * 8, y, cache_lump_name<patch_t *>(DEH_String("M_THERMO"), PU_CACHE));

  dp_translation = nullptr;
}

void M_StartMessage(cstring_view string, void (*routine)(int), bool input) {
  messageLastMenuActive          = g_doomstat_globals->menuactive;
  messageToPrint                 = 1;
  messageString                  = string.c_str();
  messageRoutine                 = routine;
  messageNeedsInput              = input;
  g_doomstat_globals->menuactive = true;
  // [crispy] entering menus while recording demos pauses the game
  if (g_doomstat_globals->demorecording && !g_doomstat_globals->paused) { sendpause = true; }
  return;
}

//
// Find string width from hu_font chars
//
int M_StringWidth(cstring_view s) {
  int w = 0;

  const char * string = s.c_str();
  for (size_t i = 0; i < strlen(string); i++) {
    // [crispy] correctly center colorized strings
    if (string[i] == cr_esc) {
      if (string[i + 1] >= '0' && string[i + 1] <= '0' + static_cast<int>(cr_t::CRMAX) - 1) {
        i++;
        continue;
      }
    }

    int c = toupper(string[i]) - HU_FONTSTART;
    if (c < 0 || c >= HU_FONTSIZE)
      w += 4;
    else
      w += SHORT(hu_font[c]->width);
  }

  return w;
}

//
//      Find string height from hu_font chars
//
int M_StringHeight(cstring_view string) {
  int height = SHORT(hu_font[0]->height);
  int h      = height;
  const char * str = string.c_str();
  for (size_t i = 0; i < string.size(); i++)
    if (str[i] == '\n') h += height;

  return h;
}

//
//      Write a string using the hu_font
//
void M_WriteText(int x, int y, cstring_view string) {
  const char * ch = string.c_str();
  int          cx = x;
  int          cy = y;

  while (true) {
    int c = static_cast<unsigned char>(*ch++);
    if (!c) break;
    if (c == '\n') {
      cx = x;
      cy += 12;
      continue;
    }
    // [crispy] support multi-colored text
    if (c == cr_esc) {
      if (*ch >= '0' && *ch <= '0' + static_cast<int>(cr_t::CRMAX) - 1) {
        c              = static_cast<unsigned char>(*ch++);
        dp_translation = cr_colors[static_cast<int>(c - '0')];
        continue;
      }
    }

    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c >= HU_FONTSIZE) {
      cx += 4;
      continue;
    }

    int w = SHORT(hu_font[c]->width);
    if (cx + w > ORIGWIDTH) break;
    V_DrawPatchDirect(cx, cy, hu_font[c]);
    cx += w;
  }
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static bool IsNullKey(int key) {
  return key == KEY_PAUSE || key == KEY_CAPSLOCK || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
static int G_ReloadLevel() {
  int result = false;

  if (g_doomstat_globals->gamestate == GS_LEVEL) {
    // [crispy] restart demos from the map they were started
    if (g_doomstat_globals->demorecording) { g_doomstat_globals->gamemap = g_doomstat_globals->startmap; }
    G_DeferedInitNew(g_doomstat_globals->gameskill, g_doomstat_globals->gameepisode, g_doomstat_globals->gamemap);
    result = true;
  }

  return result;
}

static int G_GotoNextLevel() {
  static uint8_t doom_next[5][9] = {
    {12,  13, 19, 15, 16, 17, 18, 21, 14},
    { 22, 23, 24, 25, 29, 27, 28, 31, 26},
    { 32, 33, 34, 35, 36, 39, 38, 41, 37},
    { 42, 49, 44, 45, 46, 47, 48, 51, 43},
    { 52, 53, 54, 55, 56, 59, 58, 11, 57},
  };
  static uint8_t doom2_next[33] = { 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 1, 32, 16, 3 };
  static uint8_t nerve_next[9]  = { 2, 3, 4, 9, 6, 7, 8, 1, 5 };

  int changed = false;

  // [crispy] process only once
  if (!doom2_next[0]) {
    doom2_next[0] = 2;

    if (g_doomstat_globals->gamemode == commercial) {
      if (crispy->havemap33) doom2_next[1] = 33;

      if (W_CheckNumForName("map31") < 0) doom2_next[14] = 16;

      if (g_doomstat_globals->gamemission == pack_hacx) {
        doom2_next[30] = 16;
        doom2_next[20] = 1;
      }
      if (g_doomstat_globals->gamemission == pack_master) {
        doom2_next[1]  = 3;
        doom2_next[14] = 16;
        doom2_next[20] = 1;
      }
    } else {
      if (g_doomstat_globals->gamemode == shareware) doom_next[0][7] = 11;

      if (g_doomstat_globals->gamemode == registered) doom_next[2][7] = 11;

      if (!crispy->haved1e5) doom_next[3][7] = 11;

      if (g_doomstat_globals->gameversion == exe_chex) {
        doom_next[0][2] = 14;
        doom_next[0][4] = 11;
      }
    }
  }

  if (g_doomstat_globals->gamestate == GS_LEVEL) {
    int epsd = 0;
    int map  = 0;

    if (g_doomstat_globals->gamemode == commercial) {
      epsd = g_doomstat_globals->gameepisode;
      if (g_doomstat_globals->gamemission == pack_nerve)
        map = nerve_next[g_doomstat_globals->gamemap - 1];
      else
        map = doom2_next[g_doomstat_globals->gamemap - 1];
    } else {
      epsd = doom_next[g_doomstat_globals->gameepisode - 1][g_doomstat_globals->gamemap - 1] / 10;
      map  = doom_next[g_doomstat_globals->gameepisode - 1][g_doomstat_globals->gamemap - 1] % 10;
    }

    // [crispy] special-casing for E1M10 "Sewers" support
    if (crispy->havee1m10 && g_doomstat_globals->gameepisode == 1) {
      if (g_doomstat_globals->gamemap == 1) {
        map = 10;
      } else if (g_doomstat_globals->gamemap == 10) {
        epsd = 1;
        map  = 2;
      }
    }

    G_DeferedInitNew(g_doomstat_globals->gameskill, epsd, map);
    changed = true;
  }

  return changed;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
bool M_Responder(event_t * ev) {
  static int mousewait = 0;
  static int mousey    = 0;
  static int lasty     = 0;
  static int mousex    = 0;
  static int lastx     = 0;

  // In testcontrols mode, none of the function keys should do anything
  // - the only key is escape to quit.

  if (g_doomstat_globals->testcontrols) {
    if (ev->type == ev_quit || (ev->type == ev_keydown && (ev->data1 == g_m_controls_globals->key_menu_activate || ev->data1 == g_m_controls_globals->key_menu_quit))) {
      I_Quit();
      return true;
    }

    return false;
  }

  // "close" button pressed on window?
  if (ev->type == ev_quit) {
    // First click on close button = bring up quit confirm message.
    // Second click on close button = confirm quit

    if (g_doomstat_globals->menuactive && messageToPrint && messageRoutine == M_QuitResponse) {
      M_QuitResponse(g_m_controls_globals->key_menu_confirm);
    } else {
      S_StartSound(nullptr, sfx_swtchn);
      M_QuitDOOM(0);
    }

    return true;
  }

  // key is the key pressed, ch is the actual character typed

  int ch  = 0;
  int key = -1;

  if (ev->type == ev_joystick) {
    // Simulate key presses from joystick events to interact with the menu.

    auto JOY_BUTTON_PRESSED = [&ev](auto x) { return (x >= 0) && (ev->data1 & (1 << x)) != 0; };

    if (g_doomstat_globals->menuactive) {
      if (ev->data3 < 0) {
        key                        = g_m_controls_globals->key_menu_up;
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);
      } else if (ev->data3 > 0) {
        key                        = g_m_controls_globals->key_menu_down;
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);
      }
      if (ev->data2 < 0) {
        key                        = g_m_controls_globals->key_menu_left;
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 2);
      } else if (ev->data2 > 0) {
        key                        = g_m_controls_globals->key_menu_right;
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 2);
      }

      if (JOY_BUTTON_PRESSED(g_m_controls_globals->joybfire)) {
        // Simulate a 'Y' keypress when Doom show a Y/N dialog with Fire button.
        if (messageToPrint && messageNeedsInput) { key = g_m_controls_globals->key_menu_confirm; }
        // Simulate pressing "Enter" when we are supplying a save slot name
        else if (saveStringEnter) {
          key = KEY_ENTER;
        } else {
          // if selecting a save slot via joypad, set a flag
          if (currentMenu == &SaveDef) { joypadSave = true; }
          key = g_m_controls_globals->key_menu_forward;
        }
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);
      }
      if (JOY_BUTTON_PRESSED(g_m_controls_globals->joybuse)) {
        // Simulate a 'N' keypress when Doom show a Y/N dialog with Use button.
        if (messageToPrint && messageNeedsInput) { key = g_m_controls_globals->key_menu_abort; }
        // If user was entering a save name, back out
        else if (saveStringEnter) {
          key = KEY_ESCAPE;
        } else {
          key = g_m_controls_globals->key_menu_back;
        }
        g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);
      }
    }
    if (JOY_BUTTON_PRESSED(g_m_controls_globals->joybmenu)) {
      key                        = g_m_controls_globals->key_menu_activate;
      g_i_video_globals->joywait = static_cast<unsigned int>(I_GetTime() + 5);
    }
  } else {
    if (ev->type == ev_mouse && mousewait < I_GetTime()) {
      // [crispy] novert disables controlling the menus with the mouse
      if (!novert) { mousey += ev->data3; }
      if (mousey < lasty - 30) {
        key       = g_m_controls_globals->key_menu_down;
        mousewait = I_GetTime() + 5;
        mousey    = lasty -= 30;
      } else if (mousey > lasty + 30) {
        key       = g_m_controls_globals->key_menu_up;
        mousewait = I_GetTime() + 5;
        mousey    = lasty += 30;
      }

      mousex += ev->data2;
      if (mousex < lastx - 30) {
        key       = g_m_controls_globals->key_menu_left;
        mousewait = I_GetTime() + 5;
        mousex    = lastx -= 30;
      } else if (mousex > lastx + 30) {
        key       = g_m_controls_globals->key_menu_right;
        mousewait = I_GetTime() + 5;
        mousex    = lastx += 30;
      }

      if (ev->data1 & 1) {
        key       = g_m_controls_globals->key_menu_forward;
        mousewait = I_GetTime() + 15;
      }

      if (ev->data1 & 2) {
        key       = g_m_controls_globals->key_menu_back;
        mousewait = I_GetTime() + 15;
      }

      // [crispy] scroll menus with mouse wheel
      if (g_m_controls_globals->mousebprevweapon >= 0 && ev->data1 & (1 << g_m_controls_globals->mousebprevweapon)) {
        key       = g_m_controls_globals->key_menu_down;
        mousewait = I_GetTime() + 5;
      } else if (g_m_controls_globals->mousebnextweapon >= 0 && ev->data1 & (1 << g_m_controls_globals->mousebnextweapon)) {
        key       = g_m_controls_globals->key_menu_up;
        mousewait = I_GetTime() + 5;
      }
    } else {
      if (ev->type == ev_keydown) {
        key = ev->data1;
        ch  = ev->data2;
      }
    }
  }

  if (key == -1) return false;

  // Save Game string input
  if (saveStringEnter) {
    switch (key) {
    case KEY_BACKSPACE:
      if (saveCharIndex > 0) {
        saveCharIndex--;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
      break;

    case KEY_ESCAPE:
      saveStringEnter = 0;
      I_StopTextInput();
      M_StringCopy(savegamestrings[saveSlot], saveOldString, SAVESTRINGSIZE);
      break;

    case KEY_ENTER:
      saveStringEnter = 0;
      I_StopTextInput();
      if (savegamestrings[saveSlot][0]) M_DoSave(saveSlot);
      break;

    default:
      // Savegame name entry. This is complicated.
      // Vanilla has a bug where the shift key is ignored when entering
      // a savegame name. If vanilla_keyboard_mapping is on, we want
      // to emulate this bug by using ev->data1. But if it's turned off,
      // it implies the user doesn't care about Vanilla emulation:
      // instead, use ev->data3 which gives the fully-translated and
      // modified key input.

      if (ev->type != ev_keydown) { break; }
      if (g_i_video_globals->vanilla_keyboard_mapping) {
        ch = ev->data1;
      } else {
        ch = ev->data3;
      }

      ch = toupper(ch);

      if (ch != ' ' && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE)) { break; }

      if (ch >= 32 && ch <= 127 && saveCharIndex < SAVESTRINGSIZE - 1
          && M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE - 2) * 8) {
        savegamestrings[saveSlot][saveCharIndex++] = static_cast<char>(ch);
        savegamestrings[saveSlot][saveCharIndex]   = 0;
      }
      break;
    }
    return true;
  }

  // Take care of any messages that need input
  if (messageToPrint) {
    if (messageNeedsInput) {
      if (key != ' ' && key != KEY_ESCAPE && key != g_m_controls_globals->key_menu_confirm && key != g_m_controls_globals->key_menu_abort) { return false; }
    }

    g_doomstat_globals->menuactive = messageLastMenuActive;
    if (messageRoutine) messageRoutine(key);

    // [crispy] stay in menu
    if (messageToPrint < 2) { g_doomstat_globals->menuactive = false; }
    messageToPrint = 0; // [crispy] moved here
    S_StartSound(nullptr, sfx_swtchx);
    return true;
  }

  // [crispy] take screen shot without weapons and HUD
  if (key != 0 && key == g_m_controls_globals->key_menu_cleanscreenshot) { crispy->cleanscreenshot = (screenblocks > 10) ? 2 : 1; }

  if ((g_doomstat_globals->devparm && key == g_m_controls_globals->key_menu_help) || (key != 0 && (key == g_m_controls_globals->key_menu_screenshot || key == g_m_controls_globals->key_menu_cleanscreenshot))) {
    G_ScreenShot();
    return true;
  }

  // F-Keys
  if (!g_doomstat_globals->menuactive) {
    if (key == g_m_controls_globals->key_menu_decscreen) // Screen size down
    {
      if (g_doomstat_globals->automapactive || chat_on) return false;
      M_SizeDisplay(0);
      S_StartSound(nullptr, sfx_stnmov);
      return true;
    } else if (key == g_m_controls_globals->key_menu_incscreen) // Screen size up
    {
      if (g_doomstat_globals->automapactive || chat_on) return false;
      M_SizeDisplay(1);
      S_StartSound(nullptr, sfx_stnmov);
      return true;
    } else if (key == g_m_controls_globals->key_menu_help) // Help key
    {
      M_StartControlPanel();

      if (g_doomstat_globals->gameversion >= exe_ultimate)
        currentMenu = &ReadDef2;
      else
        currentMenu = &ReadDef1;

      itemOn = 0;
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    } else if (key == g_m_controls_globals->key_menu_save) // Save
    {
      M_StartControlPanel();
      S_StartSound(nullptr, sfx_swtchn);
      M_SaveGame(0);
      return true;
    } else if (key == g_m_controls_globals->key_menu_load) // Load
    {
      M_StartControlPanel();
      S_StartSound(nullptr, sfx_swtchn);
      M_LoadGame(0);
      return true;
    } else if (key == g_m_controls_globals->key_menu_volume) // Sound Volume
    {
      M_StartControlPanel();
      currentMenu = &SoundDef;
      itemOn      = static_cast<short>(sound_e::sfx_vol);
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    } else if (key == g_m_controls_globals->key_menu_detail) // Detail toggle
    {
      M_ChangeDetail(0);
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    } else if (key == g_m_controls_globals->key_menu_qsave) // Quicksave
    {
      S_StartSound(nullptr, sfx_swtchn);
      M_QuickSave();
      return true;
    } else if (key == g_m_controls_globals->key_menu_endgame) // End game
    {
      S_StartSound(nullptr, sfx_swtchn);
      M_EndGame(0);
      return true;
    } else if (key == g_m_controls_globals->key_menu_messages) // Toggle messages
    {
      M_ChangeMessages(0);
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    } else if (key == g_m_controls_globals->key_menu_qload) // Quickload
    {
      S_StartSound(nullptr, sfx_swtchn);
      M_QuickLoad();
      return true;
    } else if (key == g_m_controls_globals->key_menu_quit) // Quit DOOM
    {
      S_StartSound(nullptr, sfx_swtchn);
      M_QuitDOOM(0);
      return true;
    } else if (key == g_m_controls_globals->key_menu_gamma) // gamma toggle
    {
      g_i_video_globals->usegamma++;
      if (g_i_video_globals->usegamma > 4 + 4) // [crispy] intermediate gamma levels
        g_i_video_globals->usegamma = 0;
      g_doomstat_globals->players[g_doomstat_globals->consoleplayer].message = DEH_String(gammamsg[g_i_video_globals->usegamma].c_str());
#ifndef CRISPY_TRUECOLOR
      I_SetPalette(cache_lump_name<uint8_t *>(DEH_String("PLAYPAL"), PU_CACHE));
#else
      {
        extern void R_InitColormaps();
        I_SetPalette(0);
        R_InitColormaps();
        inhelpscreens = true;
        R_FillBackScreen();
        viewactive = false;
      }
#endif
      return true;
    }
    // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
    // and should be treated as such, i.e. add "if (!netgame)"
    else if (!g_doomstat_globals->netgame && key != 0 && key == g_m_controls_globals->key_menu_reloadlevel) {
      if (G_ReloadLevel()) return true;
    } else if (!g_doomstat_globals->netgame && key != 0 && key == g_m_controls_globals->key_menu_nextlevel) {
      if (G_GotoNextLevel()) return true;
    }
  }

  // Pop-up menu?
  if (!g_doomstat_globals->menuactive) {
    if (key == g_m_controls_globals->key_menu_activate) {
      M_StartControlPanel();
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    }
    return false;
  }

  // Keys usable within menu

  if (key == g_m_controls_globals->key_menu_down) {
    // Move down to next item

    do {
      if (itemOn + 1 > currentMenu->numitems - 1)
        itemOn = 0;
      else
        itemOn++;
      S_StartSound(nullptr, sfx_pstop);
    } while (currentMenu->menuitems[itemOn].status == -1);

    return true;
  } else if (key == g_m_controls_globals->key_menu_up) {
    // Move back up to previous item

    do {
      if (!itemOn)
        itemOn = static_cast<short>(currentMenu->numitems - 1);
      else
        itemOn--;
      S_StartSound(nullptr, sfx_pstop);
    } while (currentMenu->menuitems[itemOn].status == -1);

    return true;
  } else if (key == g_m_controls_globals->key_menu_left) {
    // Slide slider left

    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2) {
      S_StartSound(nullptr, sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
  } else if (key == g_m_controls_globals->key_menu_right) {
    // Slide slider right

    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2) {
      S_StartSound(nullptr, sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(1);
    }
    return true;
  } else if (key == g_m_controls_globals->key_menu_forward) {
    // Activate menu item

    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status) {
      currentMenu->lastOn = itemOn;
      if (currentMenu->menuitems[itemOn].status == 2) {
        currentMenu->menuitems[itemOn].routine(1); // right arrow
        S_StartSound(nullptr, sfx_stnmov);
      } else {
        currentMenu->menuitems[itemOn].routine(itemOn);
        S_StartSound(nullptr, sfx_pistol);
      }
    }
    return true;
  } else if (key == g_m_controls_globals->key_menu_activate) {
    // Deactivate menu

    currentMenu->lastOn = itemOn;
    M_ClearMenus();
    S_StartSound(nullptr, sfx_swtchx);
    return true;
  } else if (key == g_m_controls_globals->key_menu_back) {
    // Go back to previous menu

    currentMenu->lastOn = itemOn;
    if (currentMenu->prevMenu) {
      currentMenu = currentMenu->prevMenu;
      itemOn      = currentMenu->lastOn;
      S_StartSound(nullptr, sfx_swtchn);
    }
    return true;
  }
  // [crispy] delete a savegame
  else if (key == g_m_controls_globals->key_menu_del) {
    if (currentMenu == &LoadDef || currentMenu == &SaveDef) {
      if (LoadMenu[itemOn].status) {
        currentMenu->lastOn = itemOn;
        M_ConfirmDeleteGame();
        return true;
      } else {
        S_StartSound(nullptr, sfx_oof);
      }
    }
  }
  // [crispy] next/prev Crispness menu
  else if (key == KEY_PGUP) {
    currentMenu->lastOn = itemOn;
    if (currentMenu == CrispnessMenus[crispness_cur]) {
      M_CrispnessPrev(0);
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    }
  } else if (key == KEY_PGDN) {
    currentMenu->lastOn = itemOn;
    if (currentMenu == CrispnessMenus[crispness_cur]) {
      M_CrispnessNext(0);
      S_StartSound(nullptr, sfx_swtchn);
      return true;
    }
  }

  // Keyboard shortcut?
  // Vanilla Doom has a weird behavior where it jumps to the scroll bars
  // when the certain keys are pressed, so emulate this.

  else if (ch != 0 || IsNullKey(key)) {
    for (int i = itemOn + 1; i < currentMenu->numitems; i++) {
      if (currentMenu->menuitems[i].alphaKey == ch) {
        itemOn = static_cast<short>(i);
        S_StartSound(nullptr, sfx_pstop);
        return true;
      }
    }

    for (int i = 0; i <= itemOn; i++) {
      if (currentMenu->menuitems[i].alphaKey == ch) {
        itemOn = static_cast<short>(i);
        S_StartSound(nullptr, sfx_pstop);
        return true;
      }
    }
  }

  return false;
}

//
// M_StartControlPanel
//
void M_StartControlPanel() {
  // intro might call this repeatedly
  if (g_doomstat_globals->menuactive) return;

  // [crispy] entering menus while recording demos pauses the game
  if (g_doomstat_globals->demorecording && !g_doomstat_globals->paused) sendpause = true;

  g_doomstat_globals->menuactive = true;
  currentMenu                    = &MainDef;            // JDC
  itemOn                         = currentMenu->lastOn; // JDC
}

// Display OPL debug messages - hack for GENMIDI development.

static void M_DrawOPLDev() {
  extern void I_OPL_DevMessages(char *, size_t);
  char        debug[1024];

  I_OPL_DevMessages(debug, sizeof(debug));
  char * curr = debug;
  int    line = 0;

  for (;;) {
    char * p = strchr(curr, '\n');

    if (p != nullptr) { *p = '\0'; }

    M_WriteText(0, line * 8, curr);
    ++line;

    if (p == nullptr) { break; }

    curr = p + 1;
  }
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer() {
  static short x = 0;
  static short y = 0;

  char string[80];
  inhelpscreens = false;

  // Horiz. & Vertically center string and print it.
  if (messageToPrint) {
    // [crispy] draw a background for important questions
    if (messageToPrint == 2) { M_DrawCrispnessBackground(); }

    size_t start = 0;
    y            = static_cast<short>(ORIGHEIGHT / 2 - M_StringHeight(messageString) / 2);
    while (messageString[start] != '\0') {
      bool foundnewline = false;

      for (size_t i = 0; messageString[start + i] != '\0'; i++) {
        if (messageString[start + i] == '\n') {
          M_StringCopy(string, messageString + start, sizeof(string));
          if (i < sizeof(string)) { string[i] = '\0'; }

          foundnewline = true;
          start += i + 1;
          break;
        }
      }

      if (!foundnewline) {
        M_StringCopy(string, messageString + start, sizeof(string));
        start += strlen(string);
      }

      x = static_cast<short>(ORIGWIDTH / 2 - M_StringWidth(string) / 2);
      M_WriteText(x > 0 ? x : 0, y, string); // [crispy] prevent negative x-coords
      y = static_cast<short>(y + SHORT(hu_font[0]->height));
    }

    return;
  }

  if (opldev) { M_DrawOPLDev(); }

  if (!g_doomstat_globals->menuactive) return;

  if (currentMenu->routine) currentMenu->routine(); // call Draw routine

  // DRAW MENU
  x        = currentMenu->x;
  y        = currentMenu->y;
  auto max = static_cast<unsigned int>(currentMenu->numitems);

  for (size_t i = 0; i < max; i++) {
    const char * name = DEH_String(currentMenu->menuitems[i].name);

    if (name[0]) // && W_CheckNumForName(name) > 0) // [crispy] moved...
    {
      // [crispy] shade unavailable menu items
      if ((currentMenu == &MainDef && static_cast<main_e>(i) == main_e::savegame && (!g_doomstat_globals->usergame || g_doomstat_globals->gamestate != GS_LEVEL))
          || (currentMenu == &OptionsDef && static_cast<options_e>(i) == options_e::endgame && (!g_doomstat_globals->usergame || g_doomstat_globals->netgame))
          || (currentMenu == &MainDef && static_cast<main_e>(i) == main_e::loadgame && (g_doomstat_globals->netgame && !g_doomstat_globals->demoplayback))
          || (currentMenu == &MainDef && static_cast<main_e>(i) == main_e::newgame && (g_doomstat_globals->demorecording || (g_doomstat_globals->netgame && !g_doomstat_globals->demoplayback))))
        dp_translation = cr_colors[static_cast<int>(cr_t::CR_DARK)];

      if (currentMenu == &OptionsDef) {
        const char * alttext = currentMenu->menuitems[i].alttext;

        if (alttext)
          M_WriteText(x, y + 8 - (M_StringHeight(alttext) / 2), alttext);
      } else if (W_CheckNumForName(name) > 0) // [crispy] ...here
        V_DrawPatchDirect(x, y, cache_lump_name<patch_t *>(name, PU_CACHE));

      dp_translation = nullptr;
    }
    y = static_cast<short>(y + LINEHEIGHT);
  }

  // DRAW SKULL
  if (currentMenu == CrispnessMenus[crispness_cur]) {
    char item[4];
    M_snprintf(item, sizeof(item), "%s>", whichSkull ? crstr[static_cast<int>(cr_t::CR_NONE)] : crstr[static_cast<int>(cr_t::CR_DARK)]);
    M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * itemOn, item);
    dp_translation = nullptr;
  } else
    V_DrawPatchDirect(x + SKULLXOFF, currentMenu->y - 5 + itemOn * LINEHEIGHT, cache_lump_name<patch_t *>(DEH_String(skullName[whichSkull]), PU_CACHE));
}

//
// M_ClearMenus
//
void M_ClearMenus() {
  g_doomstat_globals->menuactive = false;

  // [crispy] entering menus while recording demos pauses the game
  if (g_doomstat_globals->demorecording && g_doomstat_globals->paused)
    sendpause = true;

  // if (!netgame && usergame && paused)
  //       sendpause = true;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t * menudef) {
  currentMenu = menudef;
  itemOn      = currentMenu->lastOn;
}

//
// M_Ticker
//
void M_Ticker() {
  if (--skullAnimCounter <= 0) {
    whichSkull ^= 1;
    skullAnimCounter = 8;
  }
}

//
// M_Init
//
void M_Init() {
  currentMenu                    = &MainDef;
  g_doomstat_globals->menuactive = false;
  itemOn                         = currentMenu->lastOn;
  whichSkull                     = 0;
  skullAnimCounter               = 10;
  screenSize                     = screenblocks - 3;
  M_SizeDisplay(-1); // [crispy] initialize screenSize_min
  messageToPrint        = 0;
  messageString         = nullptr;
  messageLastMenuActive = g_doomstat_globals->menuactive;
  quickSaveSlot         = -1;

  // Here we could catch other version dependencies,
  //  like HELP1/2, and four episodes.

  // The same hacks were used in the original Doom EXEs.

  if (g_doomstat_globals->gameversion >= exe_ultimate) {
    MainMenu[static_cast<int>(main_e::readthis)].routine = M_ReadThis2;
    ReadDef2.prevMenu                                    = nullptr;
  }

  if (g_doomstat_globals->gameversion >= exe_final && g_doomstat_globals->gameversion <= exe_final2) {
    ReadDef2.routine = M_DrawReadThisCommercial;
    // [crispy] rearrange Skull in Final Doom HELP screen
    ReadDef2.y = static_cast<short>(ReadDef2.y - 10);
  }

  if (g_doomstat_globals->gamemode == commercial) {
    MainMenu[static_cast<int>(main_e::readthis)] = MainMenu[static_cast<int>(main_e::quitdoom)];
    MainDef.numitems--;
    MainDef.y                                                = static_cast<short>(MainDef.y + 8);
    NewDef.prevMenu                                          = g_doomstat_globals->nervewadfile ? &ExpDef : &MainDef;
    ReadDef1.routine                                         = M_DrawReadThisCommercial;
    ReadDef1.x                                               = 330;
    ReadDef1.y                                               = 165;
    ReadMenu1[static_cast<int>(read_e::rdthsempty1)].routine = M_FinishReadThis;
  }

  // [crispy] Sigil
  if (!crispy->haved1e5) {
    EpiDef.numitems = 4;
  }

  // Versions of doom.exe before the Ultimate Doom release only had
  // three episodes; if we're emulating one of those then don't try
  // to show episode four. If we are, then do show episode four
  // (should crash if missing).
  if (g_doomstat_globals->gameversion < exe_ultimate) {
    EpiDef.numitems--;
  }
  // chex.exe shows only one episode.
  else if (g_doomstat_globals->gameversion == exe_chex) {
    EpiDef.numitems = 1;
    // [crispy] never show the Episode menu
    NewDef.prevMenu = &MainDef;
  }

  // [crispy] rearrange Load Game and Save Game menus
  {
    const patch_t * patchl = cache_lump_name<patch_t *>(DEH_String("M_LOADG"), PU_CACHE);
    const patch_t * patchs = cache_lump_name<patch_t *>(DEH_String("M_SAVEG"), PU_CACHE);
    const patch_t * patchm = cache_lump_name<patch_t *>(DEH_String("M_LSLEFT"), PU_CACHE);

    LoadDef_x = (ORIGWIDTH - SHORT(patchl->width)) / 2 + SHORT(patchl->leftoffset);
    SaveDef_x = (ORIGWIDTH - SHORT(patchs->width)) / 2 + SHORT(patchs->leftoffset);
    LoadDef.x = SaveDef.x = static_cast<short>((ORIGWIDTH - 24 * 8) / 2 + SHORT(patchm->leftoffset)); // [crispy] see M_DrawSaveLoadBorder()

    short captionheight = std::max((static_cast<signed short>((patchl->height))), (static_cast<signed short>((patchs->height))));

    short vstep = ORIGHEIGHT - 32; // [crispy] ST_HEIGHT
    vstep       = static_cast<short>(vstep - captionheight);
    vstep       = static_cast<short>(vstep - (static_cast<int>(load_e::load_end) - 1) * LINEHEIGHT + SHORT(patchm->height));
    vstep       = static_cast<short>(vstep / 3);

    if (vstep > 0) {
      LoadDef_y = vstep + captionheight - SHORT(patchl->height) + SHORT(patchl->topoffset);
      SaveDef_y = vstep + captionheight - SHORT(patchs->height) + SHORT(patchs->topoffset);
      int val   = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 7;
      LoadDef.y = SaveDef.y = static_cast<short>(val); // [crispy] see M_DrawSaveLoadBorder()
      MouseDef.y            = LoadDef.y;
    }
  }

  // [crispy] remove DOS reference from the game quit confirmation dialogs
  if (!M_ParmExists("-nodeh")) {
    char * replace = nullptr;

    // [crispy] "i wouldn't leave if i were you.\ndos is much worse."
    const char * string = doom1_endmsg[3];
    if (!DEH_HasStringReplacement(string)) {
      replace = M_StringReplace(string, "dos", crispy->platform);
      DEH_AddStringReplacement(string, replace);
      free(replace);
    }

    // [crispy] "you're trying to say you like dos\nbetter than me, right?"
    string = doom1_endmsg[4];
    if (!DEH_HasStringReplacement(string)) {
      replace = M_StringReplace(string, "dos", crispy->platform);
      DEH_AddStringReplacement(string, replace);
      free(replace);
    }

    // [crispy] "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!"
    string = doom2_endmsg[2];
    if (!DEH_HasStringReplacement(string)) {
      replace = M_StringReplace(string, "dos", "command");
      DEH_AddStringReplacement(string, replace);
      free(replace);
    }
  }

  opldev = M_CheckParm("-opldev") > 0;
}

// [crispy] extended savegames
static char * savegwarning;
static void   M_ForceLoadGameResponse(int key) {
  free(savegwarning);
  free(savewadfilename);

  if (key != g_m_controls_globals->key_menu_confirm || !savemaplumpinfo) {
    // [crispy] no need to end game anymore when denied to load savegame
    // M_EndGameResponse(key_menu_confirm);
    savewadfilename = nullptr;

    // [crispy] reload Load Game menu
    M_StartControlPanel();
    M_LoadGame(0);
    return;
  }

  savewadfilename = const_cast<char *>(W_WadNameForLump(savemaplumpinfo));
  gameaction      = ga_loadgame;
}

void M_ForceLoadGame() {
  savegwarning =
      savemaplumpinfo ?
          M_StringJoin("This savegame requires the file\n",
                       crstr[static_cast<int>(cr_t::CR_GOLD)],
                       savewadfilename,
                       crstr[static_cast<int>(cr_t::CR_NONE)],
                       "\n",
                       "to restore ",
                       crstr[static_cast<int>(cr_t::CR_GOLD)],
                       savemaplumpinfo->name,
                       crstr[static_cast<int>(cr_t::CR_NONE)],
                       " .\n\n",
                       "Continue to restore from\n",
                       crstr[static_cast<int>(cr_t::CR_GOLD)],
                       W_WadNameForLump(savemaplumpinfo),
                       crstr[static_cast<int>(cr_t::CR_NONE)],
                       " ?\n\n",
                       "press y or n.") :
          M_StringJoin("This savegame requires the file\n",
                       crstr[static_cast<int>(cr_t::CR_GOLD)],
                       savewadfilename,
                       crstr[static_cast<int>(cr_t::CR_NONE)],
                       "\n",
                       "to restore a map that is\n",
                       "currently not available!\n\n",
                       "press a key.");

  M_StartMessage(savegwarning, M_ForceLoadGameResponse, savemaplumpinfo != nullptr);
  messageToPrint = 2;
  S_StartSound(nullptr, sfx_swtchn);
}

static void M_ConfirmDeleteGameResponse(int key) {
  free(savegwarning);

  if (key == g_m_controls_globals->key_menu_confirm) {
    char name[256];

    M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
    remove(name);

    M_ReadSaveStrings();
  }
}

void M_ConfirmDeleteGame() {
  savegwarning =
      M_StringJoin("delete savegame\n\n",
                   crstr[static_cast<int>(cr_t::CR_GOLD)],
                   savegamestrings[itemOn],
                   crstr[static_cast<int>(cr_t::CR_NONE)],
                   " ?\n\n",
                   "press y or n.");

  M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
  messageToPrint = 2;
  S_StartSound(nullptr, sfx_swtchn);
}

// [crispy] indicate game version mismatch
[[maybe_unused]] void M_LoadGameVerMismatch() {
  M_StartMessage("Game Version Mismatch\n\n"
                 "press a key.",
                 nullptr,
                 false);
  messageToPrint = 2;
  S_StartSound(nullptr, sfx_swtchn);
}

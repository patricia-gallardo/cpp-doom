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


#include <cstdlib>
#include <cctype>


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
#include "z_zone.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"

#include "r_local.hpp"


#include "hu_stuff.hpp"

#include "g_game.hpp"

#include "m_argv.hpp"
#include "m_controls.hpp"
#include "m_misc.hpp"
#include "m_saves.hpp"    // [STRIFE]
#include "p_saveg.hpp"

#include "s_sound.hpp"

#include "doomstat.hpp"

// Data.
#include "sounds.hpp"

#include "m_menu.hpp"
#include "p_dialog.hpp"
#include "lump.hpp"


extern void M_QuitStrife(int);

extern patch_t*         hu_font[HU_FONTSIZE];
extern bool          message_dontfuckwithme;

extern bool          chat_on;        // in heads-up code
extern bool          sendsave;       // [STRIFE]

//
// defaulted values
//
int			mouseSensitivity = 5;

// [STRIFE]: removed this entirely
// Show messages has default, 0 = off, 1 = on
//int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10; // [STRIFE] default 10, not 9

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
const char		*messageString;

// message x & y
[[maybe_unused]] int			messx;
[[maybe_unused]] int			messy;
int			messageLastMenuActive;

// timed message = no input from user
bool			messageNeedsInput;

void    (*messageRoutine)(int response);

// [crispy] intermediate gamma levels
char gammamsg[5+4][26+2] =
{
    GAMMALVL0,
    GAMMALVL05,
    GAMMALVL1,
    GAMMALVL15,
    GAMMALVL2,
    GAMMALVL25,
    GAMMALVL3,
    GAMMALVL35,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

bool                 inhelpscreens;
bool                 menuactive;
bool                 menupause;      // haleyjd 08/29/10: [STRIFE] New global
int                     menupausetime;  // haleyjd 09/04/10: [STRIFE] New global
bool                 menuindialog;   // haleyjd 09/04/10: ditto

// haleyjd 08/27/10: [STRIFE] SKULLXOFF == -28, LINEHEIGHT == 19
#define CURSORXOFF		-28
#define LINEHEIGHT		19

extern bool		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

// haleyjd 09/04/10: [STRIFE] Moved menuitem / menu structures into header
// because they are needed externally by the dialog engine.

// haleyjd 08/27/10: [STRIFE] skull* stuff changed to cursor* stuff
short		itemOn;			// menu item skull is on
short		cursorAnimCounter;	// skull animation counter
short		whichCursor;		// which skull to draw

// graphic name of cursors
// haleyjd 08/27/10: [STRIFE] M_SKULL* -> M_CURS*
const char *cursorName[8] = {"M_CURS1", "M_CURS2", "M_CURS3", "M_CURS4",
                             "M_CURS5", "M_CURS6", "M_CURS7", "M_CURS8" };

// haleyjd 20110210 [STRIFE]: skill level for menus
int menuskill;

// current menudef
menu_t*	currentMenu;                          

// haleyjd 03/01/13: [STRIFE] v1.31-only:
// Keeps track of whether the save game menu is being used to name a new
// character slot, or to just save the current game. In the v1.31 disassembly
// this was the new dword_8632C variable.
bool namingCharacter;

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_ReadThis3(int choice); // [STRIFE]

//void M_ChangeMessages(int choice); [STRIFE]
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_VoiceVol(int choice); // [STRIFE]
void M_MusicVol(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

//void M_FinishReadThis(int choice); - [STRIFE] unused
void M_SaveSelect(int choice);
void M_ReadSaveStrings();
void M_QuickSave();
void M_QuickLoad();

void M_DrawMainMenu();
void M_DrawReadThis1();
void M_DrawReadThis2();
void M_DrawReadThis3(); // [STRIFE]
void M_DrawNewGame();
void M_DrawEpisode();
void M_DrawOptions();
void M_DrawSound();
void M_DrawLoad();
void M_DrawSave();

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_StartMessage(const char *string,void *routine,bool input);
void M_StopMessage();




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

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'h'}, // haleyjd 08/28/10: 'r' -> 'h'
    {1,"M_QUITG",M_QuitStrife,'q'}
};

menu_t  MainDef =
{
    static_cast<short>(main_e::main_end),
    nullptr,
    MainMenu,
    M_DrawMainMenu,
    97,45, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};


//
// EPISODE SELECT
//
/*
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};
*/

//
// NEW GAME
//
enum class newgame_e
{
    killthings [[maybe_unused]],
    toorough,
    hurtme,
    violence [[maybe_unused]],
    nightmare,
    newg_end
};

menuitem_t NewGameMenu[]=
{
    // haleyjd 08/28/10: [STRIFE] changed all shortcut letters
    {1,"M_JKILL",   M_ChooseSkill, 't'},
    {1,"M_ROUGH",   M_ChooseSkill, 'r'},
    {1,"M_HURT",    M_ChooseSkill, 'v'},
    {1,"M_ULTRA",   M_ChooseSkill, 'e'},
    {1,"M_NMARE",   M_ChooseSkill, 'b'}
};

menu_t  NewDef =
{
    static_cast<short>(newgame_e::newg_end),           // # of menu items
    &MainDef,           // previous menu - haleyjd [STRIFE] changed to MainDef
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    static_cast<short>(newgame_e::toorough)  // lastOn - haleyjd [STRIFE]: default to skill 1
};

//
// OPTIONS MENU
//
enum class options_e
{
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    endgame,
    scrnsize,
    option_empty1 [[maybe_unused]],
    soundvol [[maybe_unused]],
    opt_end
};

menuitem_t OptionsMenu[]=
{
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    {1,"M_ENDGAM",	M_EndGame,'e'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {1,"M_SVOL",	M_Sound,'s'}
};

menu_t  OptionsDef =
{
    static_cast<short>(options_e::opt_end),
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

//
// Read This! MENU 1 & 2 & [STRIFE] 3
//
enum class read_e
{
    rdthsempty1,
    read1_end
};

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    static_cast<short>(read_e::read1_end),
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum class read_e2
{
    rdthsempty2 [[maybe_unused]],
    read2_end
};

menuitem_t ReadMenu2[]=
{
    {1,"",M_ReadThis3,0} // haleyjd 08/28/10: [STRIFE] Go to ReadThis3
};

menu_t  ReadDef2 =
{
    static_cast<short>(read_e2::read2_end),
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    250,185, // haleyjd 08/28/10: [STRIFE] changed coords
    0
};

// haleyjd 08/28/10: Added Read This! menu 3
enum class read_e3
{
    rdthsempty3,
    read3_end
};

menuitem_t ReadMenu3[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3 =
{
    static_cast<short>(read_e3::read3_end),
    &ReadDef2,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
    0
};

//
// SOUND VOLUME MENU
//
enum class sound_e
{
    sfx_vol,
    sfx_empty1 [[maybe_unused]],
    music_vol,
    sfx_empty2 [[maybe_unused]],
    voice_vol,
    sfx_empty3,
    sfx_mouse,
    sfx_empty4,
    sound_end
};

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'},
    {2,"M_VOIVOL",M_VoiceVol,'v'}, 
    {-1,"",0,'\0'},
    {2,"M_MSENS",M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'}
};

menu_t  SoundDef =
{
    static_cast<short>(sound_e::sound_end),
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,35,       // [STRIFE] changed y coord 64 -> 35
    0
};

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
    load_end
};

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    static_cast<short>(load_e::load_end),
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    static_cast<short>(load_e::load_end),
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};

void M_DrawNameChar();

//
// NAME CHARACTER MENU
//
// [STRIFE]
// haleyjd 20110210: New "Name Your Character" Menu
//
menu_t NameCharDef =
{
    static_cast<short>(load_e::load_end),
    &NewDef,
    SaveMenu,
    M_DrawNameChar,
    80,54,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// [STRIFE]
// haleyjd 20110210: Rewritten to read "name" file in each slot directory
//
void M_ReadSaveStrings()
{
    FILE *handle;
    int   i;
    char *fname = nullptr;

    for(i = 0; i < static_cast<int>(load_e::load_end); i++)
    {
        if(fname)
            Z_Free(fname);
        fname = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(i, "\\name"));

        handle = fopen(fname, "rb");
        if(handle == nullptr)
        {
            M_StringCopy(savegamestrings[i], DEH_String(EMPTYSTRING),
                         sizeof(savegamestrings[i]));
            LoadMenu[i].status = 0;
            continue;
        }
        size_t retval = fread(savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = retval == SAVESTRINGSIZE;
    }

    if(fname)
        Z_Free(fname);
}

//
// M_DrawNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for drawing the "Name Your Character" menu.
//
void M_DrawNameChar()
{
    int i;

    M_WriteText(72, 28, DEH_String("Name Your Character"));

    for (i = 0;i < static_cast<int>(load_e::load_end); i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_DoNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for items in the "Name Your Character" menu.
//
void M_DoNameChar(int choice)
{
    int map;

    // 20130301: clear naming character flag for 1.31 save logic
    if(gameversion == exe_strife_1_31)
        namingCharacter = false;
    sendsave = 1;
    ClearTmp();
    G_WriteSaveName(choice, savegamestrings[choice]);
    quickSaveSlot = choice;  
    SaveDef.lastOn = static_cast<short>(choice);
    ClearSlot();
    FromCurr();
    
    if(isdemoversion)
        map = 33;
    else
        map = 2;

    G_DeferedInitNew(static_cast<skill_t>(menuskill), map);
    M_ClearMenus(0);
}

//
// M_LoadGame & Cie.
//
void M_DrawLoad()
{
    int             i;

    V_DrawPatchDirect(72, 28, 
                      cache_lump_name<patch_t *>(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < static_cast<int>(load_e::load_end); i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;

    V_DrawPatchDirect(x - 8, y + 7,
                      cache_lump_name<patch_t *>(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0;i < 24;i++)
    {
        V_DrawPatchDirect(x, y + 7,
                          cache_lump_name<patch_t *>(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawPatchDirect(x, y + 7, 
                      cache_lump_name<patch_t *>(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    // [STRIFE]: completely rewritten
    char *name = nullptr;

    G_WriteSaveName(choice, savegamestrings[choice]);
    ToCurr();

    // use safe & portable filepath concatenation for Choco
    name = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(choice, ""));

    G_ReadCurrent(name);
    quickSaveSlot = choice;
    M_ClearMenus(0);

    Z_Free(name);
}

//
// Selected from DOOM menu
//
// [STRIFE] Verified unmodified
//
void M_LoadGame (int)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(LOADNET), nullptr, false);
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave()
{
    int             i;

    V_DrawPatchDirect(72, 28, cache_lump_name<patch_t *>(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < static_cast<int>(load_e::load_end); i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    // [STRIFE]: completely rewritten
    if(slot >= 0)
    {
        sendsave = 1;
        G_WriteSaveName(slot, savegamestrings[slot]);
        M_ClearMenus(0);
        quickSaveSlot = slot;        
        // haleyjd 20130922: slight divergence. We clear the destination slot 
        // of files here, which vanilla did not do. As a result, 1.31 had 
        // broken save behavior to the point of unusability. fraggle agrees 
        // this is detrimental enough to be fixed - unconditionally, for now.
        ClearSlot();        
        FromCurr();
    }
    else
        M_StartMessage(DEH_String(QSAVESPOT), nullptr, false);
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    int x, y;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // We need to turn on text input:
    x = LoadDef.x - 11;
    y = LoadDef.y + choice * LINEHEIGHT - 4;
    I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

    // [STRIFE]
    quickSaveSlot = choice;
    //saveSlot = choice;

    M_StringCopy(saveOldString, savegamestrings[choice], sizeof(saveOldString));
    if (!strcmp(savegamestrings[choice], DEH_String(EMPTYSTRING)))
        savegamestrings[choice][0] = 0;
    saveCharIndex = static_cast<int>(strlen(savegamestrings[choice]));
}

//
// Selected from DOOM menu
//
void M_SaveGame (int)
{
    // [STRIFE]
    if (netgame)
    {
        // haleyjd 20110211: Hooray for Rogue's awesome multiplayer support...
        M_StartMessage(DEH_String("You can't save a netgame"), nullptr, false);
        return;
    }
    if (!usergame)
    {
        M_StartMessage(DEH_String(SAVEDEAD),NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    // [STRIFE]
    if(gameversion == exe_strife_1_31)
    {
        // haleyjd 20130301: in 1.31, we can choose a slot again.
        M_SetupNextMenu(&SaveDef);
        M_ReadSaveStrings();
    }
    else
    {
        // In 1.2 and lower, you save over your character slot exclusively
        M_ReadSaveStrings();
        M_DoSave(quickSaveSlot);
    }
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(nullptr, sfx_mtalht); // [STRIFE] sound
    }
}

void M_QuickSave()
{
    if (netgame)
    {
        // haleyjd 20110211 [STRIFE]: More fun...
        M_StartMessage(DEH_String("You can't save a netgame"), nullptr, false);
        return;
    }

    if (!usergame)
    {
        S_StartSound(nullptr, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;	// means to pick a slot now
        return;
    }
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring, reinterpret_cast<void *>(M_QuickSaveResponse),true);
}



//
// M_QuickLoadResponse
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(nullptr, sfx_mtalht); // [STRIFE] sound
    }
}

//
// M_QuickLoad
//
// [STRIFE] Verified unmodified
//
void M_QuickLoad()
{
    if (netgame)
    {
        M_StartMessage(DEH_String(QLOADNET),NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(QSAVESPOT),NULL,false);
        return;
    }
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring, reinterpret_cast<void *>(M_QuickLoadResponse),true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
// haleyjd 08/28/10: [STRIFE] Draw HELP1, unconditionally.
//
void M_DrawReadThis1()
{
    inhelpscreens = true;

    V_DrawPatchDirect (0, 0, cache_lump_name<patch_t *>(DEH_String("HELP1"), PU_CACHE));
}



//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] Not optional, draws HELP2
//
void M_DrawReadThis2()
{
    inhelpscreens = true;

    V_DrawPatchDirect(0, 0, cache_lump_name<patch_t *>(DEH_String("HELP2"), PU_CACHE));
}


//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] New function to draw HELP3.
//
void M_DrawReadThis3()
{
    inhelpscreens = true;
    
    V_DrawPatchDirect(0, 0, cache_lump_name<patch_t *>(DEH_String("HELP3"), PU_CACHE));
}

//
// Change Sfx & Music volumes
//
// haleyjd 08/29/10: [STRIFE]
// * Changed title graphic coordinates
// * Added voice volume and sensitivity sliders
//
void M_DrawSound()
{
    V_DrawPatchDirect (100, 10, cache_lump_name<patch_t *>(DEH_String("M_SVOL"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(static_cast<int>(sound_e::sfx_vol)+1),
                 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(static_cast<int>(sound_e::music_vol)+1),
                 16,musicVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(static_cast<int>(sound_e::voice_vol)+1),
                 16,voiceVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(static_cast<int>(sound_e::sfx_mouse)+1),
                 16,mouseSensitivity);
}

void M_Sound(int)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (sfxVolume)
            sfxVolume--;
        break;
    case 1:
        if (sfxVolume < 15)
            sfxVolume++;
        break;
    }

    S_SetSfxVolume(sfxVolume * 8);
}

//
// M_VoiceVol
//
// haleyjd 08/29/10: [STRIFE] New function
// Sets voice volume level.
//
void M_VoiceVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (voiceVolume)
            voiceVolume--;
        break;
    case 1:
        if (voiceVolume < 15)
            voiceVolume++;
        break;
    }

    S_SetVoiceVolume(voiceVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (musicVolume)
            musicVolume--;
        break;
    case 1:
        if (musicVolume < 15)
            musicVolume++;
        break;
    }

    S_SetMusicVolume(musicVolume * 8);
}




//
// M_DrawMainMenu
//
// haleyjd 08/27/10: [STRIFE] Changed x coordinate; M_DOOM -> M_STRIFE
//
void M_DrawMainMenu()
{
    V_DrawPatchDirect(84, 2,
                      cache_lump_name<patch_t *>(DEH_String("M_STRIFE"), PU_CACHE));
}




//
// M_NewGame
//
// haleyjd 08/31/10: [STRIFE] Changed M_NEWG -> M_NGAME
//
void M_DrawNewGame()
{
    V_DrawPatchDirect(96, 14, cache_lump_name<patch_t *>(DEH_String("M_NGAME"), PU_CACHE));
    V_DrawPatchDirect(54, 38, cache_lump_name<patch_t *>(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(NEWGAME),NULL,false);
        return;
    }
    // haleyjd 09/07/10: [STRIFE] Removed Chex Quest and DOOM gamemodes
    if(gameversion == exe_strife_1_31)
       namingCharacter = true; // for 1.31 save logic
    M_SetupNextMenu(&NewDef);
}


//
//      M_Episode
//

// haleyjd: [STRIFE] Unused
/*
int     epi;

void M_DrawEpisode()
{
    V_DrawPatchDirect(54, 38, cache_lump_name<patch_t *>(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
        return;

    G_DeferedInitNew(nightmare, 1);
    M_ClearMenus (0);
}
*/

void M_ChooseSkill(int choice)
{
    // haleyjd 09/07/10: Removed nightmare confirmation
    // [STRIFE]: start "Name Your Character" menu
    menuskill = choice;
    currentMenu = &NameCharDef;
    itemOn = NameCharDef.lastOn;
    M_ReadSaveStrings();
}

/*
// haleyjd [STRIFE] Unused
void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}
*/


//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions()
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawPatchDirect(108, 15, 
                      cache_lump_name<patch_t *>(DEH_String("M_OPTION"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(static_cast<int>(options_e::scrnsize)+1),
                 9,screenSize);
}

void M_Options(int)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// M_AutoUseHealth
//
// [STRIFE] New function
// haleyjd 20110211: toggle autouse health state
//
void M_AutoUseHealth()
{
    if(!netgame && usergame)
    {
        players[consoleplayer].cheats ^= CF_AUTOHEALTH;

        if(players[consoleplayer].cheats & CF_AUTOHEALTH)
            players[consoleplayer].message = DEH_String("Auto use health ON");
        else
            players[consoleplayer].message = DEH_String("Auto use health OFF");
    }
}

//
// M_ChangeShowText
//
// [STRIFE] New function
//
void M_ChangeShowText()
{
    dialogshowtext ^= true;

    if(dialogshowtext)
        players[consoleplayer].message = DEH_String("Conversation Text On");
    else
        players[consoleplayer].message = DEH_String("Conversation Text Off");
}

//
//      Toggle messages on/off
//
// [STRIFE] Messages cannot be disabled in Strife
/*
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
        players[consoleplayer].message = DEH_String(MSGOFF);
    else
        players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}
*/


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (0);
    D_StartTitle ();
}

void M_EndGame(int)
{
    if (!usergame)
    {
        S_StartSound(nullptr,sfx_oof);
        return;
    }

    if (netgame)
    {
        M_StartMessage(DEH_String(NETEND),NULL,false);
        return;
    }

    M_StartMessage(DEH_String(ENDGAME), reinterpret_cast<void *>(M_EndGameResponse),true);
}




//
// M_ReadThis
//
void M_ReadThis(int)
{
    M_SetupNextMenu(&ReadDef1);
}

//
// M_ReadThis2
//
// haleyjd 08/28/10: [STRIFE] Eliminated DOOM stuff.
//
void M_ReadThis2(int)
{
    M_SetupNextMenu(&ReadDef2);
}

//
// M_ReadThis3
//
// haleyjd 08/28/10: [STRIFE] New function.
//
void M_ReadThis3(int)
{
    M_SetupNextMenu(&ReadDef3);
}

/*
// haleyjd 08/28/10: [STRIFE] Not used.
void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}
*/

#if 0
extern void F_StartCast();

//
// M_CheckStartCast
//
// [STRIFE] New but unused function. Was going to start a cast
//   call from within the menu system... not functional even in
//   the earliest demo version.
//
void M_CheckStartCast()
{
    if(usergame)
    {
        M_StartMessage(DEH_String("You have to end your game first."), nullptr, false);
        return;
    }

    F_StartCast();
    M_ClearMenus(0);
}
#endif

//
// M_QuitResponse
//
// haleyjd 09/11/10: [STRIFE] Modifications to start up endgame
// demosequence.
//
void M_QuitResponse(int key)
{
    char buffer[20];

    if (key != key_menu_confirm)
        return;

    if(netgame)
        I_Quit();
    else
    {
        DEH_snprintf(buffer, sizeof(buffer), "qfmrm%i", gametic % 8 + 1);
        I_StartVoice(buffer);
        D_QuitGame();
    }
}

/*
// haleyjd 09/11/10: [STRIFE] Unused
static char *M_SelectEndMessage()
{
}
*/

//
// M_QuitStrife
//
// [STRIFE] Renamed from M_QuitDOOM
// haleyjd 09/11/10: No randomized text message; that's taken care of
// by the randomized voice message after confirmation.
//
void M_QuitStrife(int)
{
    DEH_snprintf(endstring, sizeof(endstring),
                 "Do you really want to leave?\n\n" DOSY);
  
    M_StartMessage(endstring, reinterpret_cast<void *>(M_QuitResponse), true);
}




void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
    case 0:
        if (mouseSensitivity)
            mouseSensitivity--;
        break;
    case 1:
        if (mouseSensitivity < 9)
            mouseSensitivity++;
        break;
    }
}

/*
// haleyjd [STRIFE] Unused
void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DEH_String(DETAILHI);
    else
	players[consoleplayer].message = DEH_String(DETAILLO);
}
*/

// [STRIFE] Verified unmodified
void M_SizeDisplay(int choice)
{
    switch(choice)
    {
    case 0:
        if (screenSize > 0)
        {
            screenblocks--;
            screenSize--;
        }
        break;
    case 1:
        if (screenSize < 8)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawPatchDirect(xx, yy, cache_lump_name<patch_t *>(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatchDirect(xx, yy, cache_lump_name<patch_t *>(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }
    V_DrawPatchDirect(xx, yy, cache_lump_name<patch_t *>(DEH_String("M_THERMR"), PU_CACHE));

    // [STRIFE] +2 to initial y coordinate
    V_DrawPatchDirect((x + 8) + thermDot * 8, y + 2,
                      cache_lump_name<patch_t *>(DEH_String("M_THERMO"), PU_CACHE));
}


// haleyjd: These are from DOOM v0.5 and the prebeta! They drew those ugly red &
// blue checkboxes... preserved for historical interest, as not in Strife.
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 
                      cache_lump_name<patch_t *>(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1,
                      cache_lump_name<patch_t *>(DEH_String("M_CELL2"), PU_CACHE));
}


void
M_StartMessage
( const char	*string,
  void*		routine,
  bool	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = reinterpret_cast<void (*)(int)>(routine);
    messageNeedsInput = input;
    menuactive = true;
    return;
}



void M_StopMessage()
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(const char *string)
{
    size_t             i;
    int             w = 0;
    int             c;

    for (i = 0;i < strlen(string);i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT (hu_font[c]->width);
    }

    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(const char *string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// M_WriteText
//
// Write a string using the hu_font
// haleyjd 09/04/10: [STRIFE]
// * Rogue made a lot of changes to this for the dialog system.
//
int
M_WriteText
( int           x,
  int           y,
  const char*   string) // haleyjd: made const for safety w/dialog engine
{
    int	        w;
    const char* ch;
    int         c;
    int         cx;
    int         cy;

    ch = string;
    cx = x;
    cy = y;

    while (true)
    {
        c = *ch++;
        if (!c)
            break;

        // haleyjd 09/04/10: [STRIFE] Don't draw spaces at the start of lines.
        if(c == ' ' && cx == x)
            continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11; // haleyjd 09/04/10: [STRIFE]: Changed 12 -> 11
            continue;
        }

        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c>= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font[c]->width);

        // haleyjd 09/04/10: [STRIFE] Different linebreak handling
        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawPatchDirect(cx, cy, hu_font[c]);
            cx += w;
        }
    }

    // [STRIFE] Return final y coordinate.
    return cy + 12;
}

//
// M_DialogDimMsg
//
// [STRIFE] New function
// haleyjd 09/04/10: Painstakingly transformed from the assembly code, as the
// decompiler could not touch it. Redimensions a string to fit on screen, leaving
// at least a 20 pixel margin on the right side. The string passed in must be
// writable.
//
void M_DialogDimMsg(int x, int, char *str, bool useyfont)
{
    int rightbound = (ORIGWIDTH - 20) - x;
    patch_t **fontarray;  // ebp
    int linewidth = 0;    // esi
    int i = 0;            // edx
    char *message = str;  // edi
    char  bl;             // bl

    if(useyfont)
       fontarray = yfont;
    else
       fontarray = hu_font;

    bl = static_cast<char>(toupper(*message));

    if(!bl)
        return;

    // outer loop - run to end of string
    do
    {
        if(bl != '\n')
        {
            int charwidth; // eax
            int tempwidth; // ecx

            if(bl < HU_FONTSTART || bl > HU_FONTEND)
                charwidth = 4;
            else
                charwidth = SHORT(fontarray[bl - HU_FONTSTART]->width);

            tempwidth = linewidth + charwidth;

            // Test if the line still fits within the boundary...
            if(tempwidth >= rightbound)
            {
                // Doesn't fit...
                char *tempptr = &message[i]; // ebx
                char  al;                    // al

                // inner loop - run backward til a space (or the start of the
                // string) is found, subtracting width off the current line.
                // BUG: shouldn't we stop at a previous '\n' too?
                while(*tempptr != ' ' && i > 0)
                {
                    tempptr--;
                    // BUG: they didn't add the first char to linewidth yet...
                    linewidth -= charwidth; 
                    i--;
                    al = static_cast<char>(toupper(*tempptr));
                    if(al < HU_FONTSTART || al > HU_FONTEND)
                        charwidth = 4;
                    else
                        charwidth = SHORT(fontarray[al - HU_FONTSTART]->width);
                }
                // Replace the space with a linebreak.
                // BUG: what if i is zero? ... infinite loop time!
                message[i] = '\n';
                linewidth = 0;
            }
            else
            {
                // The line does fit.
                // Spaces at the start of a line don't count though.
                if(!(bl == ' ' && linewidth == 0))
                    linewidth += charwidth;
            }
        }
        else
            linewidth = 0; // '\n' seen, so reset the line width
    }
    while((bl = static_cast<char>(toupper(message[++i]))) != 0); // step to the next character
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static bool IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
bool M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static  int     joywait_local = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(nullptr, sfx_swtchn);
            M_QuitStrife(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;

    if (ev->type == ev_joystick && joywait_local < I_GetTime())
    {
        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait_local = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait_local = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait_local = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait_local = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait_local = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait_local = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait_local = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;
            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
                mouse_fire_countdown = 5;   // villsa [STRIFE]
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
        }
        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
            }
        }
    }

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch(key)
        {
        case KEY_BACKSPACE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;

        case KEY_ESCAPE:
            saveStringEnter = 0;
            I_StopTextInput();
            M_StringCopy(savegamestrings[quickSaveSlot], saveOldString,
                         sizeof(savegamestrings[quickSaveSlot]));
            break;

        case KEY_ENTER:
            // [STRIFE]
            saveStringEnter = 0;
            I_StopTextInput();
            if(gameversion == exe_strife_1_31 && !namingCharacter)
            {
               // In 1.31, we can be here as a result of normal saving again,
               // whereas in 1.2 this only ever happens when naming your
               // character to begin a new game.
               M_DoSave(quickSaveSlot);
               return true;
            }
            if (savegamestrings[quickSaveSlot][0])
                M_DoNameChar(quickSaveSlot);
            break;

        default:
            // Savegame name entry. This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using ev->data1. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation:
            // instead, use ev->data3 which gives the fully-translated and
            // modified key input.

            if (ev->type != ev_keydown)
            {
                break;
            }

            if (vanilla_keyboard_mapping)
            {
                ch = ev->data1;
            }
            else
            {
                ch = ev->data3;
            }

            ch = toupper(ch);

            if (ch != ' '
                && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                M_StringWidth(savegamestrings[quickSaveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[quickSaveSlot][saveCharIndex++] = static_cast<char>(ch);
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE
                && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);

        menupause = false;                // [STRIFE] unpause
        menuactive = false;
        S_StartSound(nullptr, sfx_mtalht);   // [STRIFE] sound
        return true;
    }

    // [STRIFE]:
    // * In v1.2 this is moved to F9 (quickload)
    // * In v1.31 it is moved to F12 with DM spy, and quicksave
    //   functionality is restored separate from normal saving
    /*
    if (devparm && key == key_menu_help)
    {
        G_ScreenShot ();
        return true;
    }
    */

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(0);
            S_StartSound(nullptr, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(1);
            S_StartSound(nullptr, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();
            // haleyjd 08/29/10: [STRIFE] always ReadDef1
            currentMenu = &ReadDef1; 

            itemOn = 0;
            S_StartSound(nullptr, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // just saving normally, in 1.31

            if(netgame || players[consoleplayer].health <= 0 ||
                players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(nullptr, sfx_oof);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(nullptr, sfx_swtchn);
                M_SaveGame(0);
            }
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
            {
                // 1.31: normal save loading
                namingCharacter = false;
                M_StartControlPanel();
                M_LoadGame(0);
                S_StartSound(nullptr, sfx_swtchn);
            }
            else
            {
                // Pre 1.31: quickload only
                S_StartSound(nullptr, sfx_swtchn);
                M_QuickLoad();
            }
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = static_cast<short>(sound_e::sfx_vol);
            S_StartSound(nullptr, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            //M_ChangeDetail(0);
            M_AutoUseHealth(); // [STRIFE]
            S_StartSound(nullptr, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // for 1.31 save changes

            if(netgame || players[consoleplayer].health <= 0 ||
               players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(nullptr, sfx_oof);
            }
            else
            {
                S_StartSound(nullptr, sfx_swtchn);
                M_QuickSave();
            }
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(nullptr, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            //M_ChangeMessages(0);
            M_ChangeShowText(); // [STRIFE]
            S_StartSound(nullptr, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            // [STRIFE]
            // * v1.2: takes a screenshot
            // * v1.31: does quickload again
            if(gameversion == exe_strife_1_31)
            {
                namingCharacter = false;
                S_StartSound(nullptr, sfx_swtchn);
                M_QuickLoad();
            }
            else
                G_ScreenShot();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(nullptr, sfx_swtchn);
            M_QuitStrife(0);
            return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
            usegamma++;
            if (usegamma > 4+4) // [crispy] intermediate gamma levels
                usegamma = 0;
            players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            I_SetPalette (cache_lump_name<uint8_t *> (DEH_String("PLAYPAL"),PU_CACHE));
            return true;
        }
        else if(gameversion == exe_strife_1_31 && key == key_spy)
        {
            // haleyjd 20130301: 1.31 moved screenshots to F12.
            G_ScreenShot();
            return true;
        }
        else if (key != 0 && key == key_menu_screenshot)
        {
            G_ScreenShot();
            return true;
        }
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(nullptr, sfx_swtchn);
            return true;
        }
        return false;
    }

    
    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
                itemOn = 0;
            else itemOn++;
            S_StartSound(nullptr, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

        do
        {
            if (!itemOn)
                itemOn = static_cast<short>(currentMenu->numitems-1);
            else itemOn--;
            S_StartSound(nullptr, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(nullptr, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(nullptr, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status)
        {
            currentMenu->lastOn = itemOn;
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1);      // right arrow
                S_StartSound(nullptr, sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                //S_StartSound(nullptr, sfx_swish); [STRIFE] No sound is played here.
            }
        }
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu
        if(gameversion == exe_strife_1_31) // [STRIFE]: 1.31 saving
            namingCharacter = false;

        if(menuindialog) // [STRIFE] - Get out of dialog engine semi-gracefully
            P_DialogDoChoice(-1);

        currentMenu->lastOn = itemOn;
        M_ClearMenus (0);
        S_StartSound(nullptr, sfx_mtalht); // villsa [STRIFE]: sounds
        return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
            S_StartSound(nullptr, sfx_swtchn);
        }
        return true;
    }

    // Keyboard shortcut?
    // Vanilla Strife has a weird behavior where it jumps to the scroll bars
    // when certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        // Keyboard shortcut?

        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = static_cast<short>(i);
                S_StartSound(nullptr, sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
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
void M_StartControlPanel ()
{
    // intro might call this repeatedly
    if (menuactive)
        return;
    
    menuactive = 1;
    menupause = true;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer ()
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    const char          *name;
    int			start;

    inhelpscreens = false;
    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = static_cast<short>(100 - M_StringHeight(messageString) / 2);
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[static_cast<unsigned int>(start) + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start,
                             sizeof(string));
                start += static_cast<int>(strlen(string));
            }

            x = static_cast<short>(160 - M_StringWidth(string) / 2);
            M_WriteText(x, y, string);
            y = static_cast<short>(y + SHORT(hu_font[0]->height));
        }

        return;
    }

    if (!menuactive)
        return;

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = static_cast<unsigned int>(currentMenu->numitems);

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

        if (name[0])
        {
            V_DrawPatchDirect (x, y, cache_lump_name<patch_t *>(name, PU_CACHE));
        }
        y = static_cast<short>(y + LINEHEIGHT);
    }

    
    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
    // DRAW SIGIL
    V_DrawPatchDirect(x + CURSORXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
                      cache_lump_name<patch_t *>(DEH_String(cursorName[whichCursor]),
                                      PU_CACHE));

}


//
// M_ClearMenus
//
// haleyjd 08/28/10: [STRIFE] Added an int param so this can be called by menus.
//         09/08/10: Added menupause.
//
void M_ClearMenus (int)
{
    menuactive = false;
    menupause = false;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
// haleyjd 08/27/10: [STRIFE] Rewritten for Sigil cursor
//
void M_Ticker ()
{
    if (--cursorAnimCounter <= 0)
    {
        whichCursor = static_cast<short>( (whichCursor + 1) % 8);
        cursorAnimCounter = 5;
    }
}


//
// M_Init
//
// haleyjd 08/27/10: [STRIFE] Removed DOOM gamemode stuff
//
void M_Init ()
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichCursor = 0;
    cursorAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = nullptr;
    messageLastMenuActive = menuactive; // STRIFE-FIXME: assigns 0 here...
    quickSaveSlot = -1;

    // [STRIFE]: Initialize savegame paths and clear temporary directory
    G_WriteSaveName(5, "ME");
    ClearTmp();

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.
}


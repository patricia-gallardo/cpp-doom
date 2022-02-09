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
//	Archiving: SaveGame I/O.
//


#include <cstdlib>

#include "dstrings.hpp"
#include "deh_main.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"
#include "m_misc.hpp"
#include "p_local.hpp"
#include "p_saveg.hpp"

// State.
#include "doomstat.hpp"
#include "r_state.hpp"
#include "memory.hpp"

#define SAVEGAME_EOF 0x1d

// haleyjd 09/28/10: [STRIFE] VERSIONSIZE == 8
#define VERSIONSIZE 8 

FILE *save_stream;
int savegamelength;
bool savegame_error;

// Get the filename of a temporary file to write the savegame to.  After
// the file has been successfully saved, it will be renamed to the 
// real file.

char *P_TempSaveGameFile()
{
    static char *filename = nullptr;

    if (filename == nullptr)
    {
        filename = M_StringJoin(savegamedir, "temp.dsg", nullptr);
    }

    return filename;
}

// Get the filename of the save game file to use for the specified slot.

char *P_SaveGameFile(int slot)
{
    static char *filename = nullptr;
    static size_t filename_size;
    char basename[32];

    if (filename == nullptr)
    {
        filename_size = strlen(savegamedir) + 32;
        filename = static_cast<char *>(malloc(filename_size));
    }

    DEH_snprintf(basename, 32, SAVEGAMENAME "%d.dsg", slot);

    M_snprintf(filename, filename_size, "%s%s", savegamedir, basename);

    return filename;
}

// Endian-safe integer read/write functions

static uint8_t saveg_read8()
{
    uint8_t result;

    if (fread(&result, 1, 1, save_stream) < 1)
    {
        if (!savegame_error)
        {
            fprintf(stderr, "saveg_read8: Unexpected end of file while "
                            "reading save game\n");

            savegame_error = true;
        }
    }

    return result;
}

static void saveg_write8(uint8_t value)
{
    if (fwrite(&value, 1, 1, save_stream) < 1)
    {
        if (!savegame_error)
        {
            fprintf(stderr, "saveg_write8: Error while writing save game\n");

            savegame_error = true;
        }
    }
}

static short saveg_read16()
{
    int result;

    result = saveg_read8();
    result |= saveg_read8() << 8;

    return result;
}

static void saveg_write16(short value)
{
    saveg_write8(value & 0xff);
    saveg_write8((value >> 8) & 0xff);
}

static int saveg_read32()
{
    int result;

    result = saveg_read8();
    result |= saveg_read8() << 8;
    result |= saveg_read8() << 16;
    result |= saveg_read8() << 24;

    return result;
}

static void saveg_write32(int value)
{
    saveg_write8(value & 0xff);
    saveg_write8((value >> 8) & 0xff);
    saveg_write8((value >> 16) & 0xff);
    saveg_write8((value >> 24) & 0xff);
}

// Pad to 4-byte boundaries

static void saveg_read_pad()
{
    unsigned long pos;
    int padding;
    int i;

    pos = ftell(save_stream);

    padding = (4 - (pos & 3)) & 3;

    for (i=0; i<padding; ++i)
    {
        saveg_read8();
    }
}

static void saveg_write_pad()
{
    unsigned long pos;
    int padding;
    int i;

    pos = ftell(save_stream);

    padding = (4 - (pos & 3)) & 3;

    for (i=0; i<padding; ++i)
    {
        saveg_write8(0);
    }
}


// Pointers

static void *saveg_readp()
{
    return (void *) (intptr_t) saveg_read32();
}

static void saveg_writep(const void *p)
{
    saveg_write32(reinterpret_cast<intptr_t>(p));
}

// Enum values are 32-bit integers.

#define saveg_read_enum saveg_read32
#define saveg_write_enum saveg_write32

//
// Structure read/write functions
//

//
// mapthing_t
//

static void saveg_read_mapthing_t(mapthing_t *str)
{
    // short x;
    str->x = saveg_read16();

    // short y;
    str->y = saveg_read16();

    // short angle;
    str->angle = saveg_read16();

    // short type;
    str->type = saveg_read16();

    // short options;
    str->options = saveg_read16();
}

static void saveg_write_mapthing_t(mapthing_t *str)
{
    // short x;
    saveg_write16(str->x);

    // short y;
    saveg_write16(str->y);

    // short angle;
    saveg_write16(str->angle);

    // short type;
    saveg_write16(str->type);

    // short options;
    saveg_write16(str->options);
}

//
// actionf_t
//

static void saveg_read_actionf_t(actionf_t &str)
{
    str.acp1 = reinterpret_cast<actionf_p1>(saveg_readp());
}

static void saveg_write_actionf_t(actionf_t str)
{
    saveg_writep(reinterpret_cast<const void *>(str.acp1));
}

//
// think_t
//
// This is just an actionf_t.
//

#define saveg_read_think_t saveg_read_actionf_t
#define saveg_write_think_t saveg_write_actionf_t

//
// thinker_t
//

static void saveg_read_thinker_t(thinker_t *str)
{
    // struct thinker_s* prev;
    str->prev = static_cast<thinker_s *>(saveg_readp());

    // struct thinker_s* next;
    str->next = static_cast<thinker_s *>(saveg_readp());

    // think_t function;
    saveg_read_think_t(str->function);
}

static void saveg_write_thinker_t(thinker_t *str)
{
    // struct thinker_s* prev;
    saveg_writep(str->prev);

    // struct thinker_s* next;
    saveg_writep(str->next);

    // think_t function;
    saveg_write_think_t(str->function);
}

//
// mobj_t
//
// haleyjd 09/28/10: [STRIFE] Changed to match Strife binary mobj_t structure.
// 

static void saveg_read_mobj_t(mobj_t *str)
{
    int pl;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // fixed_t x;
    str->x = saveg_read32();

    // fixed_t y;
    str->y = saveg_read32();

    // fixed_t z;
    str->z = saveg_read32();

    // struct mobj_s* snext;
    str->snext = static_cast<mobj_s *>(saveg_readp());

    // struct mobj_s* sprev;
    str->sprev = static_cast<mobj_s *>(saveg_readp());

    // angle_t angle;
    str->angle = saveg_read32();

    // spritenum_t sprite;
    str->sprite = static_cast<spritenum_t>(saveg_read_enum());

    // int frame;
    str->frame = saveg_read32();

    // struct mobj_s* bnext;
    str->bnext = static_cast<mobj_s *>(saveg_readp());

    // struct mobj_s* bprev;
    str->bprev = static_cast<mobj_s *>(saveg_readp());

    // struct subsector_s* subsector;
    str->subsector = static_cast<subsector_s *>(saveg_readp());

    // fixed_t floorz;
    str->floorz = saveg_read32();

    // fixed_t ceilingz;
    str->ceilingz = saveg_read32();

    // fixed_t radius;
    str->radius = saveg_read32();

    // fixed_t height;
    str->height = saveg_read32();

    // fixed_t momx;
    str->momx = saveg_read32();

    // fixed_t momy;
    str->momy = saveg_read32();

    // fixed_t momz;
    str->momz = saveg_read32();

    // int validcount;
    str->validcount = saveg_read32();

    // mobjtype_t type;
    str->type = static_cast<mobjtype_t>(saveg_read_enum());

    // mobjinfo_t* info;
    str->info = static_cast<mobjinfo_t *>(saveg_readp());

    // int tics;
    str->tics = saveg_read32();

    // state_t* state;
    str->state = &states[saveg_read32()];

    // int flags;
    str->flags = saveg_read32();

    // int health;
    str->health = saveg_read32();

    // int movedir;
    str->movedir = saveg_read32();

    // int movecount;
    str->movecount = saveg_read32();

    // struct mobj_s* target;
    str->target = static_cast<mobj_s *>(saveg_readp());

    // int reactiontime;
    str->reactiontime = saveg_read32();

    // int threshold;
    str->threshold = saveg_read32();

    // struct player_s* player;
    pl = saveg_read32();

    if (pl > 0)
    {
        str->player = &players[pl - 1];
        str->player->mo = str;
    }
    else
    {
        str->player = nullptr;
    }

    // int lastlook;
    str->lastlook = saveg_read32();

    // mapthing_t spawnpoint;
    saveg_read_mapthing_t(&str->spawnpoint);

    // struct mobj_s* tracer;
    str->tracer = static_cast<mobj_s *>(saveg_readp());

    // byte miscdata;
    str->miscdata = saveg_read8(); // [STRIFE] Only change to mobj_t.
}

static void saveg_write_mobj_t(mobj_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // fixed_t x;
    saveg_write32(str->x);

    // fixed_t y;
    saveg_write32(str->y);

    // fixed_t z;
    saveg_write32(str->z);

    // struct mobj_s* snext;
    saveg_writep(str->snext);

    // struct mobj_s* sprev;
    saveg_writep(str->sprev);

    // angle_t angle;
    saveg_write32(str->angle);

    // spritenum_t sprite;
    saveg_write_enum(str->sprite);

    // int frame;
    saveg_write32(str->frame);

    // struct mobj_s* bnext;
    saveg_writep(str->bnext);

    // struct mobj_s* bprev;
    saveg_writep(str->bprev);

    // struct subsector_s* subsector;
    saveg_writep(str->subsector);

    // fixed_t floorz;
    saveg_write32(str->floorz);

    // fixed_t ceilingz;
    saveg_write32(str->ceilingz);

    // fixed_t radius;
    saveg_write32(str->radius);

    // fixed_t height;
    saveg_write32(str->height);

    // fixed_t momx;
    saveg_write32(str->momx);

    // fixed_t momy;
    saveg_write32(str->momy);

    // fixed_t momz;
    saveg_write32(str->momz);

    // int validcount;
    saveg_write32(str->validcount);

    // mobjtype_t type;
    saveg_write_enum(str->type);

    // mobjinfo_t* info;
    saveg_writep(str->info);

    // int tics;
    saveg_write32(str->tics);

    // state_t* state;
    saveg_write32(str->state - states);

    // int flags;
    saveg_write32(str->flags);

    // int health;
    saveg_write32(str->health);

    // int movedir;
    saveg_write32(str->movedir);

    // int movecount;
    saveg_write32(str->movecount);

    // struct mobj_s* target;
    saveg_writep(str->target);

    // int reactiontime;
    saveg_write32(str->reactiontime);

    // int threshold;
    saveg_write32(str->threshold);

    // struct player_s* player;
    if (str->player)
    {
        saveg_write32(str->player - players + 1);
    }
    else
    {
        saveg_write32(0);
    }

    // int lastlook;
    saveg_write32(str->lastlook);

    // mapthing_t spawnpoint;
    saveg_write_mapthing_t(&str->spawnpoint);

    // struct mobj_s* tracer;
    saveg_writep(str->tracer);

    // byte miscdata;
    saveg_write8(str->miscdata); // [STRIFE] Only change to mobj_t.
}


//
// ticcmd_t
//
// haleyjd 09/28/10: [STRIFE] Modified for Strife binary ticcmd_t structure.
//

static void saveg_read_ticcmd_t(ticcmd_t *str)
{
    // signed char forwardmove;
    str->forwardmove = saveg_read8();

    // signed char sidemove;
    str->sidemove = saveg_read8();

    // short angleturn;
    str->angleturn = saveg_read16();

    // short consistancy;
    // STRIFE-FIXME: throwing away top byte of consistancy until
    // the true Strife ticcmd_t structure is available.
    str->consistancy = (uint8_t)saveg_read16();

    // byte chatchar;
    str->chatchar = saveg_read8();

    // byte buttons;
    str->buttons = saveg_read8();

    // byte buttons2;
    str->buttons2 = saveg_read8(); // [STRIFE]

    // int inventory;
    str->inventory = saveg_read32(); // [STRIFE]
}

static void saveg_write_ticcmd_t(ticcmd_t *str)
{
    // signed char forwardmove;
    saveg_write8(str->forwardmove);

    // signed char sidemove;
    saveg_write8(str->sidemove);

    // short angleturn;
    saveg_write16(str->angleturn);

    // short consistancy;
    saveg_write16(str->consistancy);

    // byte chatchar;
    saveg_write8(str->chatchar);

    // byte buttons;
    saveg_write8(str->buttons);

    // byte buttons2;
    saveg_write8(str->buttons2); // [STRIFE]

    // int inventory;
    saveg_write32(str->inventory); // [STRIFE]
}

//
// pspdef_t
//

static void saveg_read_pspdef_t(pspdef_t *str)
{
    int state;

    // state_t* state;
    state = saveg_read32();

    if (state > 0)
    {
        str->state = &states[state];
    }
    else
    {
        str->state = nullptr;
    }

    // int tics;
    str->tics = saveg_read32();

    // fixed_t sx;
    str->sx = saveg_read32();

    // fixed_t sy;
    str->sy = saveg_read32();
}

static void saveg_write_pspdef_t(pspdef_t *str)
{
    // state_t* state;
    if (str->state)
    {
        saveg_write32(str->state - states);
    }
    else
    {
        saveg_write32(0);
    }

    // int tics;
    saveg_write32(str->tics);

    // fixed_t sx;
    saveg_write32(str->sx);

    // fixed_t sy;
    saveg_write32(str->sy);
}

//
// inventory_t 
//
// haleyjd 09/28/10: [STRIFE] handle inventory input/output
//

static void saveg_read_inventory_t(inventory_t *str)
{
    //int sprite;
    str->sprite = saveg_read32();

    //int type;
    str->type = saveg_read32();

    //int amount;
    str->amount = saveg_read32();
}

static void saveg_write_inventory_t(inventory_t *str)
{
    saveg_write32(str->sprite);
    saveg_write32(str->type);
    saveg_write32(str->amount);
}

//
// player_t
//
// haleyjd 09/28/10: [STRIFE] Modified for Strife binary player_t structure.
//

static void saveg_read_player_t(player_t *str)
{
    int i;

    // mobj_t* mo;
    str->mo = static_cast<mobj_t *>(saveg_readp());

    // playerstate_t playerstate;
    str->playerstate = static_cast<playerstate_t>(saveg_read_enum());

    // ticcmd_t cmd;
    saveg_read_ticcmd_t(&str->cmd);

    // fixed_t viewz;
    str->viewz = saveg_read32();

    // fixed_t viewheight;
    str->viewheight = saveg_read32();

    // fixed_t deltaviewheight;
    str->deltaviewheight = saveg_read32();

    // fixed_t bob;
    str->bob = saveg_read32();

    // int health;
    str->health = saveg_read32();

    // int armorpoints;
    str->armorpoints = saveg_read16(); // [STRIFE] 32 -> 16

    // int armortype;
    str->armortype = saveg_read16(); // [STRIFE] 32 -> 16

    // int powers[NUMPOWERS];
    for (i=0; i<NUMPOWERS; ++i)
    {
        str->powers[i] = saveg_read32();
    }

    // int sigiltype;
    str->sigiltype = saveg_read32(); // [STRIFE]

    // int nukagecount;
    str->nukagecount = saveg_read32(); // [STRIFE]

    // int questflags;
    str->questflags = saveg_read32(); // [STRIFE]

    // int pitch;
    str->pitch = saveg_read32(); // [STRIFE]

    // int centerview;
    str->centerview = saveg_read32(); // [STRIFE]

    // inventory_t inventory[NUMINVENTORY];
    for(i = 0; i < NUMINVENTORY; i++)
    {
        saveg_read_inventory_t(&(str->inventory[i])); // [STRIFE]
    }

    // int st_update;
    str->st_update = saveg_read32(); // [STRIFE]

    // short numinventory;
    str->numinventory = saveg_read16(); // [STRIFE]

    // short inventorycursor;
    str->inventorycursor = saveg_read16(); // [STRIFE]

    // short accuracy;
    str->accuracy = saveg_read16(); // [STRIFE]

    // short stamina;
    str->stamina = saveg_read16(); // [STRIFE]

    // bool cards[NUMCARDS];
    for (i=0; i<NUMCARDS; ++i)
    {
        str->cards[i] = saveg_read32();
    }

    // bool backpack;
    str->backpack = saveg_read32();

    // int attackdown;
    str->attackdown = saveg_read32();

    // int usedown;
    str->usedown = saveg_read32();

    // int inventorydown;
    str->inventorydown = saveg_read32(); // [STRIFE]

    // int frags[MAXPLAYERS];
    for (i=0; i<MAXPLAYERS; ++i)
    {
        str->frags[i] = saveg_read32();
    }

    // weapontype_t readyweapon;
    str->readyweapon = static_cast<weapontype_t>(saveg_read_enum());

    // weapontype_t pendingweapon;
    str->pendingweapon = static_cast<weapontype_t>(saveg_read_enum());

    // bool weaponowned[NUMWEAPONS];
    for (i=0; i<NUMWEAPONS; ++i)
    {
        str->weaponowned[i] = saveg_read32();
    }

    // int ammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        str->ammo[i] = saveg_read32();
    }

    // int maxammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        str->maxammo[i] = saveg_read32();
    }

    // int cheats;
    str->cheats = saveg_read32();

    // int refire;
    str->refire = saveg_read32();

    // short killcount;
    str->killcount = saveg_read16(); // [STRIFE] 32 -> 16

    // haleyjd 08/30/10 [STRIFE] No itemcount.
    // int itemcount;
    //str->itemcount = saveg_read32();

    // haleyjd 08/30/10 [STRIFE] No secretcount.
    // int secretcount;
    //str->secretcount = saveg_read32();

    // char* message;
    str->message = static_cast<const char *>(saveg_readp());

    // int damagecount;
    str->damagecount = saveg_read32();

    // int bonuscount;
    str->bonuscount = saveg_read32();

    // mobj_t* attacker;
    str->attacker = static_cast<mobj_t *>(saveg_readp());

    // int extralight;
    str->extralight = saveg_read32();

    // int fixedcolormap;
    str->fixedcolormap = saveg_read32();

    // int colormap; - [STRIFE] no such field
    //str->colormap = saveg_read32();

    // short allegiance;
    str->allegiance = saveg_read16(); // [STRIFE]

    // pspdef_t psprites[NUMPSPRITES];
    for (i=0; i<NUMPSPRITES; ++i)
    {
        saveg_read_pspdef_t(&str->psprites[i]);
    }

    // int mapstate[40];
    for(i = 0; i < 40; ++i) // [STRIFE]
    {
        str->mapstate[i] = saveg_read32();
    }

    // haleyjd 08/30/10: [STRIFE] No intermission, no didsecret.
    // bool didsecret;
    //str->didsecret = saveg_read32();
}

static void saveg_write_player_t(player_t *str)
{
    int i;

    // mobj_t* mo;
    saveg_writep(str->mo);

    // playerstate_t playerstate;
    saveg_write_enum(str->playerstate);

    // ticcmd_t cmd;
    saveg_write_ticcmd_t(&str->cmd);

    // fixed_t viewz;
    saveg_write32(str->viewz);

    // fixed_t viewheight;
    saveg_write32(str->viewheight);

    // fixed_t deltaviewheight;
    saveg_write32(str->deltaviewheight);

    // fixed_t bob;
    saveg_write32(str->bob);

    // int health;
    saveg_write32(str->health);

    // int armorpoints;
    saveg_write16(str->armorpoints); // [STRIFE] 32 -> 16

    // int armortype;
    saveg_write16(str->armortype); // [STRIFE] 32 -> 16

    // int powers[NUMPOWERS];
    for (i=0; i<NUMPOWERS; ++i)
    {
        saveg_write32(str->powers[i]);
    }

    // int sigiltype;
    saveg_write32(str->sigiltype); // [STRIFE]

    // int nukagecount;
    saveg_write32(str->nukagecount); // [STRIFE]

    // int questflags;
    saveg_write32(str->questflags); // [STRIFE]

    // int pitch;
    saveg_write32(str->pitch); // [STRIFE]

    // int centerview;
    saveg_write32(str->centerview); // [STRIFE]

    // inventory_t inventory[NUMINVENTORY];
    for(i = 0; i < NUMINVENTORY; ++i) // [STRIFE]
    {
        saveg_write_inventory_t(&str->inventory[i]);
    }

    // int st_update;
    saveg_write32(str->st_update); // [STRIFE]

    // short numinventory;
    saveg_write16(str->numinventory); // [STRIFE]

    // short inventorycursor;
    saveg_write16(str->inventorycursor); // [STRIFE]

    // short accuracy;
    saveg_write16(str->accuracy); // [STRIFE]

    // short stamina;
    saveg_write16(str->stamina); // [STRIFE]

    // bool cards[NUMCARDS];
    for (i=0; i<NUMCARDS; ++i)
    {
        saveg_write32(str->cards[i]);
    }

    // bool backpack;
    saveg_write32(str->backpack);

    // int attackdown;
    saveg_write32(str->attackdown);

    // int usedown;
    saveg_write32(str->usedown);

    // int inventorydown;
    saveg_write32(str->inventorydown); // [STRIFE]

    // int frags[MAXPLAYERS];
    for (i=0; i<MAXPLAYERS; ++i)
    {
        saveg_write32(str->frags[i]);
    }

    // weapontype_t readyweapon;
    saveg_write_enum(str->readyweapon);

    // weapontype_t pendingweapon;
    saveg_write_enum(str->pendingweapon);

    // bool weaponowned[NUMWEAPONS];
    for (i=0; i<NUMWEAPONS; ++i)
    {
        saveg_write32(str->weaponowned[i]);
    }

    // int ammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        saveg_write32(str->ammo[i]);
    }

    // int maxammo[NUMAMMO];
    for (i=0; i<NUMAMMO; ++i)
    {
        saveg_write32(str->maxammo[i]);
    }


    // int cheats;
    saveg_write32(str->cheats);

    // int refire;
    saveg_write32(str->refire);

    // short killcount;
    saveg_write16(str->killcount); // [STRIFE] 32 -> 16

    // haleyjd 08/30/10 [STRIFE] No itemcount
    // int itemcount;
    //saveg_write32(str->itemcount);

    // haleyjd 08/30/10 [STRIFE] No secretcount
    // int secretcount;
    //saveg_write32(str->secretcount);

    // char* message;
    saveg_writep(str->message);

    // int damagecount;
    saveg_write32(str->damagecount);

    // int bonuscount;
    saveg_write32(str->bonuscount);

    // mobj_t* attacker;
    saveg_writep(str->attacker);

    // int extralight;
    saveg_write32(str->extralight);

    // int fixedcolormap;
    saveg_write32(str->fixedcolormap);

    // int colormap; [STRIFE] no such field
    //saveg_write32(str->colormap);

    // short allegiance;
    saveg_write16(str->allegiance); // [STRIFE]

    // pspdef_t psprites[NUMPSPRITES];
    for (i=0; i<NUMPSPRITES; ++i)
    {
        saveg_write_pspdef_t(&str->psprites[i]);
    }

    // int mapstate[40];
    for(i = 0; i < 40; ++i) // [STRIFE]
    {
        saveg_write32(str->mapstate[i]);
    }

    // haleyjd 08/30/10: [STRIFE] No intermission, no secret.
    // bool didsecret;
    //saveg_write32(str->didsecret);
}


//
// ceiling_t
//

static void saveg_read_ceiling_t(ceiling_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // ceiling_e type;
    str->type = static_cast<ceiling_e>(saveg_read_enum());

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // fixed_t bottomheight;
    str->bottomheight = saveg_read32();

    // fixed_t topheight;
    str->topheight = saveg_read32();

    // fixed_t speed;
    str->speed = saveg_read32();

    // bool crush;
    str->crush = saveg_read32();

    // int direction;
    str->direction = saveg_read32();

    // int tag;
    str->tag = saveg_read32();

    // int olddirection;
    str->olddirection = saveg_read32();
}

static void saveg_write_ceiling_t(ceiling_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // ceiling_e type;
    saveg_write_enum(str->type);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // fixed_t bottomheight;
    saveg_write32(str->bottomheight);

    // fixed_t topheight;
    saveg_write32(str->topheight);

    // fixed_t speed;
    saveg_write32(str->speed);

    // bool crush;
    saveg_write32(str->crush);

    // int direction;
    saveg_write32(str->direction);

    // int tag;
    saveg_write32(str->tag);

    // int olddirection;
    saveg_write32(str->olddirection);
}

//
// vldoor_t
//
// haleyjd 09/28/10: [STRIFE] Modified for Strife binary vldoor_t structure.
//

static void saveg_read_vldoor_t(vldoor_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // vldoor_e type;
    str->type = static_cast<vldoor_e>(saveg_read_enum());

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // fixed_t topheight;
    str->topheight = saveg_read32();

    // fixed_t speed;
    str->speed = saveg_read32();

    // int direction;
    str->direction = saveg_read32();

    // int topwait;
    str->topwait = saveg_read32();

    // int topcountdown;
    str->topcountdown = saveg_read32();

    // villsa [STRIFE] new field - sound to play when opening
    //int         opensound;
    str->opensound = saveg_read32();

    // villsa [STRIFE] new field - sound to play when closing
    //int         closesound;
    str->closesound = saveg_read32();
}

static void saveg_write_vldoor_t(vldoor_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // vldoor_e type;
    saveg_write_enum(str->type);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // fixed_t topheight;
    saveg_write32(str->topheight);

    // fixed_t speed;
    saveg_write32(str->speed);

    // int direction;
    saveg_write32(str->direction);

    // int topwait;
    saveg_write32(str->topwait);

    // int topcountdown;
    saveg_write32(str->topcountdown);

    // villsa [STRIFE] new field - sound to play when opening
    //int         opensound;
    saveg_write32(str->opensound);

    // villsa [STRIFE] new field - sound to play when closing
    //int         closesound;
    saveg_write32(str->closesound);
}

//
// slidedoor_t [STRIFE]: new thinker type
//

static void saveg_read_slidedoor_t(slidedoor_t *str)
{
    int sector;
    int line;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sdt_e type;
    str->type = static_cast<sdt_e>(saveg_read_enum());

    // line_t *line1;
    line = saveg_read32();
    str->line1 = &lines[line];

    // line_t *line2;
    line = saveg_read32();
    str->line2 = &lines[line];

    // int frame;
    str->frame = saveg_read32();

    // int whichDoorIndex;
    str->whichDoorIndex = saveg_read32();

    // int timer;
    str->timer = saveg_read32();

    // sector_t *frontsector;
    sector = saveg_read32();
    str->frontsector = &sectors[sector];

    // sd_e status;
    str->status = static_cast<sd_e>(saveg_read_enum());
}

static void saveg_write_slidedoor_t(slidedoor_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sdt_e type;
    saveg_write_enum(str->type);

    // line_t *line1;
    saveg_write32(str->line1 - lines);

    // line_t *line2;
    saveg_write32(str->line2 - lines);

    // int frame;
    saveg_write32(str->frame);

    // int whichDoorIndex;
    saveg_write32(str->whichDoorIndex);

    // int timer;
    saveg_write32(str->timer);

    // sector_t *frontsector;
    saveg_write32(str->frontsector - sectors);

    // sd_e status;
    saveg_write_enum(str->status);
}

//
// floormove_t
//

static void saveg_read_floormove_t(floormove_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // floor_e type;
    str->type = static_cast<floor_e>(saveg_read_enum());

    // bool crush;
    str->crush = saveg_read32();

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // int direction;
    str->direction = saveg_read32();

    // int newspecial;
    str->newspecial = saveg_read32();

    // short texture;
    str->texture = saveg_read16();

    // fixed_t floordestheight;
    str->floordestheight = saveg_read32();

    // fixed_t speed;
    str->speed = saveg_read32();
}

static void saveg_write_floormove_t(floormove_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // floor_e type;
    saveg_write_enum(str->type);

    // bool crush;
    saveg_write32(str->crush);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // int direction;
    saveg_write32(str->direction);

    // int newspecial;
    saveg_write32(str->newspecial);

    // short texture;
    saveg_write16(str->texture);

    // fixed_t floordestheight;
    saveg_write32(str->floordestheight);

    // fixed_t speed;
    saveg_write32(str->speed);
}

//
// plat_t
//

static void saveg_read_plat_t(plat_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // fixed_t speed;
    str->speed = saveg_read32();

    // fixed_t low;
    str->low = saveg_read32();

    // fixed_t high;
    str->high = saveg_read32();

    // int wait;
    str->wait = saveg_read32();

    // int count;
    str->count = saveg_read32();

    // plat_e status;
    str->status = static_cast<plat_e>(saveg_read_enum());

    // plat_e oldstatus;
    str->oldstatus = static_cast<plat_e>(saveg_read_enum());

    // bool crush;
    str->crush = saveg_read32();

    // int tag;
    str->tag = saveg_read32();

    // plattype_e type;
    str->type = static_cast<plattype_e>(saveg_read_enum());
}

static void saveg_write_plat_t(plat_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // fixed_t speed;
    saveg_write32(str->speed);

    // fixed_t low;
    saveg_write32(str->low);

    // fixed_t high;
    saveg_write32(str->high);

    // int wait;
    saveg_write32(str->wait);

    // int count;
    saveg_write32(str->count);

    // plat_e status;
    saveg_write_enum(str->status);

    // plat_e oldstatus;
    saveg_write_enum(str->oldstatus);

    // bool crush;
    saveg_write32(str->crush);

    // int tag;
    saveg_write32(str->tag);

    // plattype_e type;
    saveg_write_enum(str->type);
}

//
// lightflash_t
//

static void saveg_read_lightflash_t(lightflash_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // int count;
    str->count = saveg_read32();

    // int maxlight;
    str->maxlight = saveg_read32();

    // int minlight;
    str->minlight = saveg_read32();

    // int maxtime;
    str->maxtime = saveg_read32();

    // int mintime;
    str->mintime = saveg_read32();
}

static void saveg_write_lightflash_t(lightflash_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // int count;
    saveg_write32(str->count);

    // int maxlight;
    saveg_write32(str->maxlight);

    // int minlight;
    saveg_write32(str->minlight);

    // int maxtime;
    saveg_write32(str->maxtime);

    // int mintime;
    saveg_write32(str->mintime);
}

//
// strobe_t
//

static void saveg_read_strobe_t(strobe_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // int count;
    str->count = saveg_read32();

    // int minlight;
    str->minlight = saveg_read32();

    // int maxlight;
    str->maxlight = saveg_read32();

    // int darktime;
    str->darktime = saveg_read32();

    // int brighttime;
    str->brighttime = saveg_read32();
}

static void saveg_write_strobe_t(strobe_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // int count;
    saveg_write32(str->count);

    // int minlight;
    saveg_write32(str->minlight);

    // int maxlight;
    saveg_write32(str->maxlight);

    // int darktime;
    saveg_write32(str->darktime);

    // int brighttime;
    saveg_write32(str->brighttime);
}

//
// glow_t
//

static void saveg_read_glow_t(glow_t *str)
{
    int sector;

    // thinker_t thinker;
    saveg_read_thinker_t(&str->thinker);

    // sector_t* sector;
    sector = saveg_read32();
    str->sector = &sectors[sector];

    // int minlight;
    str->minlight = saveg_read32();

    // int maxlight;
    str->maxlight = saveg_read32();

    // int direction;
    str->direction = saveg_read32();
}

static void saveg_write_glow_t(glow_t *str)
{
    // thinker_t thinker;
    saveg_write_thinker_t(&str->thinker);

    // sector_t* sector;
    saveg_write32(str->sector - sectors);

    // int minlight;
    saveg_write32(str->minlight);

    // int maxlight;
    saveg_write32(str->maxlight);

    // int direction;
    saveg_write32(str->direction);
}

//
// Write the header for a savegame
//
// haleyjd 09/28/10: [STRIFE] numerous modifications.
//

void P_WriteSaveGameHeader(char *)
{
    char name[VERSIONSIZE]; 
    int i; 

    /*
    [STRIFE] This is in the "NAME" file in a Strife save directory.
    for (i=0; description[i] != '\0'; ++i)
        saveg_write8(description[i]);
    for (; i<SAVESTRINGSIZE; ++i)
        saveg_write8(0);
    */

    memset (name,0,sizeof(name)); 
    M_snprintf(name, sizeof(name), "ver %i", STRIFE_VERSION);

    for (i=0; i<VERSIONSIZE; ++i)
        saveg_write8(name[i]);

    saveg_write8(gameskill);
    
    // [STRIFE] This information is implicit in the file being loaded.
    //saveg_write8(gameepisode);
    //saveg_write8(gamemap);

    for (i=0 ; i<MAXPLAYERS ; i++)
        saveg_write8(playeringame[i]);

    saveg_write8((leveltime >> 16) & 0xff);
    saveg_write8((leveltime >> 8) & 0xff);
    saveg_write8(leveltime & 0xff);
}

// 
// Read the header for a savegame
//

bool P_ReadSaveGameHeader()
{
    int	 i;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    char vcheck[VERSIONSIZE]; 
    char read_vcheck[VERSIONSIZE];

    // skip the description field 
    /*
    for (i=0; i<SAVESTRINGSIZE; ++i)
        saveg_read8();
    */
    
    for (i=0; i<VERSIONSIZE; ++i)
        read_vcheck[i] = saveg_read8();

    memset (vcheck,0,sizeof(vcheck));
    M_snprintf(vcheck, sizeof(vcheck), "ver %i", STRIFE_VERSION);
    if (strcmp(read_vcheck, vcheck) != 0)
        return false;                       // bad version 

    gameskill = static_cast<skill_t>(saveg_read8());

    // [STRIFE] This info is implicit in the file being read.
    //gameepisode = saveg_read8();
    //gamemap = saveg_read8();

    for (i=0 ; i<MAXPLAYERS ; i++)
        playeringame[i] = saveg_read8();

    // get the times 
    a = saveg_read8();
    b = saveg_read8();
    c = saveg_read8();
    leveltime = (a<<16) + (b<<8) + c; 

    return true;
}

//
// Read the end of file marker.  Returns true if read successfully.
// 

bool P_ReadSaveGameEOF()
{
    int value;

    value = saveg_read8();

    return value == SAVEGAME_EOF;
}

//
// Write the end of file marker
//

void P_WriteSaveGameEOF()
{
    saveg_write8(SAVEGAME_EOF);
}

//
// P_ArchivePlayers
//
// [STRIFE] Verified unmodified.
//
void P_ArchivePlayers ()
{
    int         i;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

        saveg_write_pad();

        saveg_write_player_t(&players[i]);
    }
}



//
// P_UnArchivePlayers
//
// [STRIFE] Verified unmodified.
//
void P_UnArchivePlayers (bool userload)
{
    int         i;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        player_t dummy;

        if (!playeringame[i])
            continue;

        saveg_read_pad();

        // haleyjd [STRIFE]: not exactly how vanilla did it, but this is
        // necessary because of Choco's change to the savegame code which
        // reads it directly from file. When not a userload, all the player_t
        // data loaded from the save is thrown away.
        if(userload)
        {
            saveg_read_player_t(&players[i]);
            players[i].mo = nullptr;
        }
        else
            saveg_read_player_t(&dummy);

        // will be set when unarc thinker
        players[i].message = nullptr;
        players[i].attacker = nullptr;
    }
}


//
// P_ArchiveWorld
//
// haleyjd 09/28/10: [STRIFE] Minor modifications.
//
void P_ArchiveWorld ()
{
    int                 i;
    int                 j;
    sector_t*           sec;
    line_t*             li;
    side_t*             si;
    
    // do sectors
    for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
        saveg_write16(sec->floorheight >> FRACBITS);
        saveg_write16(sec->ceilingheight >> FRACBITS);
        saveg_write16(sec->floorpic);
        //saveg_write16(sec->ceilingpic); [STRIFE] not saved.
        saveg_write16(sec->lightlevel);
        saveg_write16(sec->special);            // needed?
        //saveg_write16(sec->tag);                // needed? [STRIFE] not saved.
    }

    
    // do lines
    for (i=0, li = lines ; i<numlines ; i++,li++)
    {
        saveg_write16(li->flags);
        saveg_write16(li->special);
        //saveg_write16(li->tag); [STRIFE] not saved.
        for (j=0 ; j<2 ; j++)
        {
            if (li->sidenum[j] == -1)
                continue;

            si = &sides[li->sidenum[j]];

            // [STRIFE] offsets not saved.
            //saveg_write16(si->textureoffset >> FRACBITS);
            //saveg_write16(si->rowoffset >> FRACBITS);
            saveg_write16(si->toptexture);
            saveg_write16(si->bottomtexture);
            saveg_write16(si->midtexture);
        }
    }
}



//
// P_UnArchiveWorld
//
void P_UnArchiveWorld ()
{
    int			i;
    int			j;
    sector_t*		sec;
    line_t*		li;
    side_t*		si;
    
    // do sectors
    for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
        sec->floorheight = saveg_read16() << FRACBITS;
        sec->ceilingheight = saveg_read16() << FRACBITS;
        sec->floorpic = saveg_read16();
        //sec->ceilingpic = saveg_read16(); [STRIFE] not saved
        sec->lightlevel = saveg_read16();
        sec->special = saveg_read16();          // needed?
        //sec->tag = saveg_read16();              // needed? [STRIFE] not saved
        sec->specialdata = 0;
        sec->soundtarget = 0;
    }
    
    // do lines
    for (i=0, li = lines ; i<numlines ; i++,li++)
    {
        li->flags = saveg_read16();
        li->special = saveg_read16();
        //li->tag = saveg_read16(); [STRIFE] not saved
        for (j=0 ; j<2 ; j++)
        {
            if (li->sidenum[j] == -1)
                continue;
            si = &sides[li->sidenum[j]];
            // [STRIFE] offsets not saved.
            //si->textureoffset = saveg_read16() << FRACBITS;
            //si->rowoffset = saveg_read16() << FRACBITS;
            si->toptexture = saveg_read16();
            si->bottomtexture = saveg_read16();
            si->midtexture = saveg_read16();
        }
    }
}





//
// Thinkers
//
typedef enum
{
    tc_end,
    tc_mobj

} thinkerclass_t;


//
// P_ArchiveThinkers
//
// [STRIFE] Verified unmodified.
//
void P_ArchiveThinkers ()
{
    thinker_t*          th;

    // save off the current thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 == reinterpret_cast<actionf_p1>(P_MobjThinker))
        {
            saveg_write8(tc_mobj);
            saveg_write_pad();
            saveg_write_mobj_t((mobj_t *) th);

            continue;
        }

        // haleyjd: This may seem mysterious but in the DOOM prebeta, 
        // different types of things used different thinker functions. 
        // Those would have all been handled here and this message is 
        // probably a relic of that old system, not to mention the odd
        // name of this function, and use of an enumeration with only
        // two values in it.

        // I_Error ("P_ArchiveThinkers: Unknown thinker function");
    }

    // add a terminating marker
    saveg_write8(tc_end);
}



//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers ()
{
    uint8_t             tclass;
    thinker_t*          currentthinker;
    thinker_t*          next;
    mobj_t*             mobj;
    
    // remove all the current thinkers
    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        next = currentthinker->next;

        if (currentthinker->function.acp1 == reinterpret_cast<actionf_p1>(P_MobjThinker))
            P_RemoveMobj ((mobj_t *)currentthinker);
        else
            Z_Free (currentthinker);

        currentthinker = next;
    }
    P_InitThinkers ();
    
    // read in saved thinkers
    while (1)
    {
        tclass = saveg_read8();
        switch (tclass)
        {
        case tc_end:
            return; 	// end of list

        case tc_mobj:
            saveg_read_pad();
            mobj = zmalloc<mobj_t *>(sizeof(*mobj), PU_LEVEL, nullptr);
            saveg_read_mobj_t(mobj);

            // haleyjd 09/29/10: Strife sets the targets of non-allied creatures
            // who had a non-NULL target at save time to players[0].mo so that
            // they won't fall back asleep.
            //
            // BUG: As the player may not have been spawned yet, we could be
            // setting monsters' targets to the mobj which was spawned by 
            // P_SetupLevel and then removed just above. Due to a subtle glitch
            // in the DOOM engine whereby all things removed in this function
            // are leaked until the next time P_SetupLevel is called, this is a
            // safe operation - the call to P_InitThinkers above stops any of
            // the objects removed, including the player's previous body, from
            // being passed to Z_Free. One glitch relying on another!

            if(mobj->target != nullptr && (mobj->flags & MF_ALLY) != MF_ALLY)
                mobj->target = players[0].mo;
            else
                mobj->target = nullptr;

            // WARNING! Strife does not seem to set tracer! I am leaving it be
            // for now because so far no crashes have been observed, and failing
            // to set this here will almost certainly crash Choco.
            mobj->tracer = nullptr;
            P_SetThingPosition (mobj);
            mobj->info = &mobjinfo[mobj->type];
            // [STRIFE]: doesn't set these
            //mobj->floorz = mobj->subsector->sector->floorheight;
            //mobj->ceilingz = mobj->subsector->sector->ceilingheight;
            mobj->thinker.function.acp1 = reinterpret_cast<actionf_p1>(P_MobjThinker);
            P_AddThinker (&mobj->thinker);
            break;

        default:
            I_Error ("Unknown tclass %i in savegame",tclass);
        }
    }
}


//
// P_ArchiveSpecials
//
enum class specials_e
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_slidingdoor, // [STRIFE]
    tc_endspecials

};



//
// Things to handle:
//
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_SlidingDoor, (slidedoor_t: sector_t *, line_t * x 2 swizzle) [STRIFE]
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
//
void P_ArchiveSpecials ()
{
    thinker_t*          th;
    int                 i;

    // save off the current thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acv == (actionf_v) nullptr)
        {
            for (i = 0; i < MAXCEILINGS;i++)
                if (activeceilings[i] == (ceiling_t *)th)
                    break;

            if (i<MAXCEILINGS)
            {
                saveg_write8(static_cast<uint8_t>(specials_e::tc_ceiling));
                saveg_write_pad();
                saveg_write_ceiling_t((ceiling_t *) th);
            }
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_ceiling));
            saveg_write_pad();
            saveg_write_ceiling_t((ceiling_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_VerticalDoor)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_door));
            saveg_write_pad();
            saveg_write_vldoor_t((vldoor_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_SlidingDoor)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_slidingdoor));
            saveg_write_pad();
            saveg_write_slidedoor_t((slidedoor_t *)th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_MoveFloor)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_floor));
            saveg_write_pad();
            saveg_write_floormove_t((floormove_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_PlatRaise)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_plat));
            saveg_write_pad();
            saveg_write_plat_t((plat_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_LightFlash)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_flash));
            saveg_write_pad();
            saveg_write_lightflash_t((lightflash_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_strobe));
            saveg_write_pad();
            saveg_write_strobe_t((strobe_t *) th);
            continue;
        }

        if (th->function.acp1 == (actionf_p1)T_Glow)
        {
            saveg_write8(static_cast<uint8_t>(specials_e::tc_glow));
            saveg_write_pad();
            saveg_write_glow_t((glow_t *) th);
            continue;
        }
    }

    // add a terminating marker
    saveg_write8(static_cast<uint8_t>(specials_e::tc_endspecials));
}


//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials ()
{
    specials_e          tclass;
    ceiling_t*          ceiling;
    vldoor_t*           door;
    slidedoor_t*        slidedoor; // haleyjd [STRIFE]
    floormove_t*        floor;
    plat_t*             plat;
    lightflash_t*       flash;
    strobe_t*           strobe;
    glow_t*             glow;


    // read in saved thinkers
    while (true)
    {
        tclass = static_cast<specials_e>(saveg_read8());

        switch (tclass)
        {
        case specials_e::tc_endspecials:
            return;	// end of list

        case specials_e::tc_ceiling:
            saveg_read_pad();
            ceiling = zmalloc<ceiling_t *>(sizeof(*ceiling), PU_LEVEL, nullptr);
            saveg_read_ceiling_t(ceiling);
            ceiling->sector->specialdata = ceiling;

            if (ceiling->thinker.function.acp1)
                ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

            P_AddThinker (&ceiling->thinker);
            P_AddActiveCeiling(ceiling);
            break;

        case specials_e::tc_door:
            saveg_read_pad();
            door = zmalloc<vldoor_t *>(sizeof(*door), PU_LEVEL, nullptr);
            saveg_read_vldoor_t(door);
            door->sector->specialdata = door;
            door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
            P_AddThinker (&door->thinker);
            break;

        case specials_e::tc_slidingdoor:
            // haleyjd 09/29/10: [STRIFE] New thinker type for sliding doors
            saveg_read_pad();
            slidedoor = zmalloc<slidedoor_t *>(sizeof(*slidedoor), PU_LEVEL, nullptr);
            saveg_read_slidedoor_t(slidedoor);
            slidedoor->frontsector->specialdata = slidedoor;
            slidedoor->thinker.function.acp1 = (actionf_p1)T_SlidingDoor;
            P_AddThinker(&slidedoor->thinker);
            break;

        case specials_e::tc_floor:
            saveg_read_pad();
            floor = zmalloc<floormove_t *>(sizeof(*floor), PU_LEVEL, nullptr);
            saveg_read_floormove_t(floor);
            floor->sector->specialdata = floor;
            floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
            P_AddThinker (&floor->thinker);
            break;

        case specials_e::tc_plat:
            saveg_read_pad();
            plat = zmalloc<plat_t *>(sizeof(*plat), PU_LEVEL, nullptr);
            saveg_read_plat_t(plat);
            plat->sector->specialdata = plat;

            if (plat->thinker.function.acp1)
                plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

            P_AddThinker (&plat->thinker);
            P_AddActivePlat(plat);
            break;

        case specials_e::tc_flash:
            saveg_read_pad();
            flash = zmalloc<lightflash_t *>(sizeof(*flash), PU_LEVEL, nullptr);
            saveg_read_lightflash_t(flash);
            flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
            P_AddThinker (&flash->thinker);
            break;

        case specials_e::tc_strobe:
            saveg_read_pad();
            strobe = zmalloc<strobe_t *>(sizeof(*strobe), PU_LEVEL, nullptr);
            saveg_read_strobe_t(strobe);
            strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
            P_AddThinker (&strobe->thinker);
            break;

        case specials_e::tc_glow:
            saveg_read_pad();
            glow = zmalloc<glow_t *>(sizeof(*glow), PU_LEVEL, nullptr);
            saveg_read_glow_t(glow);
            glow->thinker.function.acp1 = (actionf_p1)T_Glow;
            P_AddThinker (&glow->thinker);
            break;

        default:
            I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
                     "in savegame",static_cast<int>(tclass));
        }
    }
}


//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016 Fabian Greffrath
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
//	[crispy] Archiving: Extended SaveGame I/O.
//

#include <array>
#include <cstdio>
#include <cstdlib>

#include "memory.hpp"
#include "config.h"
#include "doomstat.hpp"
#include "doomtype.hpp"
#include "m_misc.hpp"
#include "p_extsaveg.hpp"
#include "p_local.hpp"
#include "p_saveg.hpp"
#include "p_setup.hpp"
#include "s_musinfo.hpp"
#include "s_sound.hpp"
#include "z_zone.hpp"

#define MAX_LINE_LEN   260
#define MAX_STRING_LEN 80

static char *line, *string;

static void P_WritePackageTarname(const char *key)
{
    M_snprintf(line, MAX_LINE_LEN, "%s %s\n", key, PACKAGE_VERSION);
    fputs(line, save_stream);
}

// maplumpinfo->wad_file->basename

char *savewadfilename = nullptr;

static void P_WriteWadFileName(const char *key)
{
    M_snprintf(line, MAX_LINE_LEN, "%s %s\n", key, W_WadNameForLump(maplumpinfo));
    fputs(line, save_stream);
}

static void P_ReadWadFileName(const char *key)
{
    if (!savewadfilename &&
        // [crispy] only check if loaded from the menu,
        // we have no chance to show a dialog otherwise
        startloadgame == -1)
    {
        if (sscanf(line, "%s", string) == 1 && !strncmp(string, key, MAX_STRING_LEN))
        {
            if (sscanf(line, "%*s %s", string) == 1)
            {
                savewadfilename = strdup(string);
            }
        }
    }
}

// extrakills

static void P_WriteExtraKills(const char *key)
{
    if (extrakills)
    {
        M_snprintf(line, MAX_LINE_LEN, "%s %d\n", key, extrakills);
        fputs(line, save_stream);
    }
}

static void P_ReadExtraKills(const char *key)
{
    int value;

    if (sscanf(line, "%s %d", string, &value) == 2 && !strncmp(string, key, MAX_STRING_LEN))
    {
        extrakills = value;
    }
}

// totalleveltimes

static void P_WriteTotalLevelTimes(const char *key)
{
    if (totalleveltimes)
    {
        M_snprintf(line, MAX_LINE_LEN, "%s %d\n", key, totalleveltimes);
        fputs(line, save_stream);
    }
}

static void P_ReadTotalLevelTimes(const char *key)
{
    int value;

    if (sscanf(line, "%s %d", string, &value) == 2 && !strncmp(string, key, MAX_STRING_LEN))
    {
        totalleveltimes = value;
    }
}

// T_FireFlicker()

extern void T_FireFlicker(fireflicker_t *flick);

static void P_WriteFireFlicker(const char *key)
{
    thinker_t *th;

    action_hook needle = T_FireFlicker;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function == needle)
        {
            auto *flick = reinterpret_cast<fireflicker_t *>(th);

            M_snprintf(line, MAX_LINE_LEN, "%s %d %d %d %d\n",
                key,
                static_cast<int>(flick->sector - sectors),
                static_cast<int>(flick->count),
                static_cast<int>(flick->maxlight),
                static_cast<int>(flick->minlight));
            fputs(line, save_stream);
        }
    }
}

static void P_ReadFireFlicker(const char *key)
{
    int sector, count, maxlight, minlight;

    if (sscanf(line, "%s %d %d %d %d\n",
            string,
            &sector,
            &count,
            &maxlight,
            &minlight)
            == 5
        && !strncmp(string, key, MAX_STRING_LEN))
    {
        fireflicker_t *flick;

        flick = zmalloc<decltype(flick)>(sizeof(*flick), PU_LEVEL, nullptr);

        flick->sector   = &sectors[sector];
        flick->count    = count;
        flick->maxlight = maxlight;
        flick->minlight = minlight;

        flick->thinker.function = T_FireFlicker;

        P_AddThinker(&flick->thinker);
    }
}

// sector->soundtarget

static void P_WriteSoundTarget(const char *key)
{
    int       i;
    sector_t *sector;

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
    {
        if (sector->soundtarget)
        {
            M_snprintf(line, MAX_LINE_LEN, "%s %d %d\n",
                key,
                i,
                P_ThinkerToIndex(reinterpret_cast<thinker_t *>(sector->soundtarget)));
            fputs(line, save_stream);
        }
    }
}

static void P_ReadSoundTarget(const char *key)
{
    int sector, target;

    if (sscanf(line, "%s %d %d\n",
            string,
            &sector,
            &target)
            == 3
        && !strncmp(string, key, MAX_STRING_LEN))
    {
        sectors[sector].soundtarget = reinterpret_cast<mobj_t *>(P_IndexToThinker(static_cast<uint32_t>(target)));
    }
}

// sector->oldspecial

static void P_WriteOldSpecial(const char *key)
{
    int       i;
    sector_t *sector;

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
    {
        if (sector->oldspecial)
        {
            M_snprintf(line, MAX_LINE_LEN, "%s %d %d\n",
                key,
                i,
                sector->oldspecial);
            fputs(line, save_stream);
        }
    }
}

static void P_ReadOldSpecial(const char *key)
{
    int sector = 0;
    int oldspecial = 0;

    if (sscanf(line, "%s %d %d\n",
            string,
            &sector,
            &oldspecial)
            == 3
        && !strncmp(string, key, MAX_STRING_LEN))
    {
        sectors[sector].oldspecial = static_cast<short>(oldspecial);
    }
}

// buttonlist[]

extern void P_StartButton(line_t *line, bwhere_e w, int texture, int time);

static void P_WriteButton(const char *key)
{
    int i;

    for (i = 0; i < maxbuttons; i++)
    {
        button_t *button = &buttonlist[i];

        if (button->btimer)
        {
            M_snprintf(line, MAX_LINE_LEN, "%s %d %d %d %d\n",
                key,
                static_cast<int>(button->line - lines),
                static_cast<int>(button->where),
                static_cast<int>(button->btexture),
                static_cast<int>(button->btimer));
            fputs(line, save_stream);
        }
    }
}

static void P_ReadButton(const char *key)
{
    int linedef_local, where, btexture, btimer;

    if (sscanf(line, "%s %d %d %d %d\n",
            string,
            &linedef_local,
            &where,
            &btexture,
            &btimer)
            == 5
        && !strncmp(string, key, MAX_STRING_LEN))
    {
        P_StartButton(&lines[linedef_local], static_cast<bwhere_e>(where), btexture, btimer);
    }
}

// numbraintargets, braintargeton

extern int numbraintargets, braintargeton;

static void P_WriteBrainTarget(const char *key)
{
    thinker_t *th;

    action_hook needle = P_MobjThinker;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function == needle)
        {
            auto *mo = reinterpret_cast<mobj_t *>(th);

            if (mo->state == &states[S_BRAINEYE1])
            {
                M_snprintf(line, MAX_LINE_LEN, "%s %d %d\n",
                    key,
                    numbraintargets,
                    braintargeton);
                fputs(line, save_stream);

                // [crispy] return after the first brain spitter is found
                return;
            }
        }
    }
}

static void P_ReadBrainTarget(const char *key)
{
    int numtargets, targeton;

    if (sscanf(line, "%s %d %d", string, &numtargets, &targeton) == 3 && !strncmp(string, key, MAX_STRING_LEN))
    {
        numbraintargets = 0; // [crispy] force A_BrainAwake()
        braintargeton   = targeton;
    }
}

// markpoints[]

extern void AM_GetMarkPoints(int *n, long *p);
extern void AM_SetMarkPoints(int n, long *p);

static void P_WriteMarkPoints(const char *key)
{
    int  n;
    long p[20];

    AM_GetMarkPoints(&n, p);

    if (p[0] != -1)
    {
        M_snprintf(line, MAX_LINE_LEN, "%s %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            key, n,
            p[0], p[1], p[2], p[3], p[4],
            p[5], p[6], p[7], p[8], p[9],
            p[10], p[11], p[12], p[13], p[14],
            p[15], p[16], p[17], p[18], p[19]);
        fputs(line, save_stream);
    }
}

static void P_ReadMarkPoints(const char *key)
{
    int  n;
    long p[20];

    if (sscanf(line, "%s %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            string, &n,
            &p[0], &p[1], &p[2], &p[3], &p[4],
            &p[5], &p[6], &p[7], &p[8], &p[9],
            &p[10], &p[11], &p[12], &p[13], &p[14],
            &p[15], &p[16], &p[17], &p[18], &p[19])
            == 22
        && !strncmp(string, key, MAX_STRING_LEN))
    {
        AM_SetMarkPoints(n, p);
    }
}

// players[]->lookdir

static void P_WritePlayersLookdir(const char *key)
{
    int i;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i] && players[i].lookdir)
        {
            M_snprintf(line, MAX_LINE_LEN, "%s %d %d\n", key, i, players[i].lookdir);
            fputs(line, save_stream);
        }
    }
}

static void P_ReadPlayersLookdir(const char *key)
{
    int i, value;

    if (sscanf(line, "%s %d %d", string, &i, &value) == 3 && !strncmp(string, key, MAX_STRING_LEN) && i < MAXPLAYERS && (crispy->freelook || crispy->mouselook))
    {
        players[i].lookdir = value;
    }
}

// musinfo.current_item

static void P_WriteMusInfo(const char *key)
{
    if (musinfo.current_item > 0 && musinfo.items[0] > 0)
    {
        char lump[9], orig[9];

        strncpy(lump, lumpinfo[musinfo.current_item]->name, 8);
        strncpy(orig, lumpinfo[musinfo.items[0]]->name, 8);

        M_snprintf(line, MAX_LINE_LEN, "%s %s %s\n", key, lump, orig);
        fputs(line, save_stream);
    }
}

static void P_ReadMusInfo(const char *key)
{
    int  items;
    char lump[9] = { 0 }, orig[9] = { 0 };

    items = sscanf(line, "%s %s %s", string, lump, orig);

    if (items >= 2 && !strncmp(string, key, MAX_STRING_LEN))
    {
        int i;

        if ((i = W_CheckNumForName(lump)) > 0)
        {
            memset(&musinfo, 0, sizeof(musinfo));
            musinfo.current_item  = i;
            musinfo.from_savegame = true;
            S_ChangeMusInfoMusic(i, true);
        }

        if (items == 3 && (i = W_CheckNumForName(orig)) > 0)
        {
            musinfo.items[0] = i;
        }
    }
}

typedef struct
{
    const char *key;
    void (*extsavegwritefn)(const char *key);
    void (*extsavegreadfn)(const char *key);
    const int pass;
} extsavegdata_t;

static const extsavegdata_t extsavegdata[] = {
    // [crispy] @FORKS: please change this if you are going to introduce incompatible changes!
    { "crispy-doom", P_WritePackageTarname, nullptr, 0 },
    { "wadfilename", P_WriteWadFileName, P_ReadWadFileName, 0 },
    { "extrakills", P_WriteExtraKills, P_ReadExtraKills, 1 },
    { "totalleveltimes", P_WriteTotalLevelTimes, P_ReadTotalLevelTimes, 1 },
    { "fireflicker", P_WriteFireFlicker, P_ReadFireFlicker, 1 },
    { "soundtarget", P_WriteSoundTarget, P_ReadSoundTarget, 1 },
    { "oldspecial", P_WriteOldSpecial, P_ReadOldSpecial, 1 },
    { "button", P_WriteButton, P_ReadButton, 1 },
    { "braintarget", P_WriteBrainTarget, P_ReadBrainTarget, 1 },
    { "markpoints", P_WriteMarkPoints, P_ReadMarkPoints, 1 },
    { "playerslookdir", P_WritePlayersLookdir, P_ReadPlayersLookdir, 1 },
    { "musinfo", P_WriteMusInfo, P_ReadMusInfo, 0 },
};

void P_WriteExtendedSaveGameData()
{
    line = static_cast<char *>(malloc(MAX_LINE_LEN));

    for (const auto & i : extsavegdata)
    {
        i.extsavegwritefn(i.key);
    }

    free(line);
}

static void P_ReadKeyValuePairs(int pass)
{
    while (fgets(line, MAX_LINE_LEN, save_stream))
    {
        if (sscanf(line, "%s", string) == 1)
        {
            for (size_t i = 1; i < std::size(extsavegdata); i++)
            {
                if (extsavegdata[i].extsavegreadfn && extsavegdata[i].pass == pass && !strncmp(string, extsavegdata[i].key, MAX_STRING_LEN))
                {
                    extsavegdata[i].extsavegreadfn(extsavegdata[i].key);
                }
            }
        }
    }
}

// [crispy] pointer to the info struct for the map lump about to load
lumpinfo_t *savemaplumpinfo = nullptr;

void P_ReadExtendedSaveGameData(int pass)
{
    long p, curpos, endpos;
    uint8_t episode;
    uint8_t map;
    int  lumpnum = -1;

    line   = static_cast<char *>(malloc(MAX_LINE_LEN));
    string = static_cast<char *>(malloc(MAX_STRING_LEN));

    // [crispy] two-pass reading of extended savegame data
    if (pass == 1)
    {
        P_ReadKeyValuePairs(1);

        free(line);
        free(string);

        return;
    }

    curpos = ftell(save_stream);

    // [crispy] check which map we would want to load
    fseek(save_stream, SAVESTRINGSIZE + VERSIONSIZE + 1, SEEK_SET); // [crispy] + 1 for "gameskill"
    if (fread(&episode, 1, 1, save_stream) == 1 && fread(&map, 1, 1, save_stream) == 1)
    {
        lumpnum = P_GetNumForMap(static_cast<int>(episode), static_cast<int>(map), false);
    }

    if (lumpnum >= 0)
    {
        savemaplumpinfo = lumpinfo[lumpnum];
    }
    else
    {
        // [crispy] unavailable map!
        savemaplumpinfo = nullptr;
    }

    // [crispy] read key/value pairs past the end of the regular savegame data
    fseek(save_stream, 0, SEEK_END);
    endpos = ftell(save_stream);

    for (p = endpos - 1; p > 0; p--)
    {
        uint8_t curbyte;

        fseek(save_stream, p, SEEK_SET);

        if (fread(&curbyte, 1, 1, save_stream) < 1)
        {
            break;
        }

        if (curbyte == SAVEGAME_EOF)
        {
            if (!fgets(line, MAX_LINE_LEN, save_stream))
            {
                continue;
            }

            if (sscanf(line, "%s", string) == 1 && !strncmp(string, extsavegdata[0].key, MAX_STRING_LEN))
            {
                P_ReadKeyValuePairs(0);
                break;
            }
        }
    }

    free(line);
    free(string);

    // [crispy] back to where we started
    fseek(save_stream, curpos, SEEK_SET);
}

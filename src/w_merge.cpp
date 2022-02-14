//
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
// Handles merging of PWADs, similar to deutex's -merge option
//
// Ideally this should work exactly the same as in deutex, but trying to
// read the deutex source code made my brain hurt.
//

#include <cstdlib>
#include <cstring>
#include <cctype>

#include "memory.hpp"
#include "doomtype.hpp"
#include "i_swap.hpp" // [crispy] LONG()
#include "i_system.hpp"
#include "m_misc.hpp"
#include "w_merge.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

enum section_t
{
    SECTION_NORMAL,
    SECTION_FLATS,
    SECTION_SPRITES,
};

typedef struct
{
    lumpinfo_t **lumps;
    int          numlumps;
} searchlist_t;

typedef struct
{
    char        sprname[4];
    char        frame;
    lumpinfo_t *angle_lumps[8];
} sprite_frame_t;

static searchlist_t iwad;
static searchlist_t iwad_sprites;
static searchlist_t pwad;

static searchlist_t iwad_flats;
static searchlist_t pwad_sprites;
static searchlist_t pwad_flats;

// lumps with these sprites must be replaced in the IWAD
static sprite_frame_t *sprite_frames;
static int             num_sprite_frames;
static int             sprite_frames_alloced;

// Search in a list to find a lump with a particular name
// Linear search (slow!)
//
// Returns -1 if not found

static int FindInList(searchlist_t *list, const char *name)
{
    for (int i = 0; i < list->numlumps; ++i)
    {
        if (!strncasecmp(list->lumps[i]->name, name, 8))
            return i;
    }

    return -1;
}

static bool SetupList(searchlist_t *list, searchlist_t *src_list,
    const char *startname, const char *endname,
    const char *startname2, const char *endname2)
{
    list->numlumps = 0;
    int startlump      = FindInList(src_list, startname);

    if (startname2 != nullptr && startlump < 0)
    {
        startlump = FindInList(src_list, startname2);
    }

    if (startlump >= 0)
    {
        int endlump = FindInList(src_list, endname);

        if (endname2 != nullptr && endlump < 0)
        {
            endlump = FindInList(src_list, endname2);
        }

        if (endlump > startlump)
        {
            list->lumps    = src_list->lumps + startlump + 1;
            list->numlumps = endlump - startlump - 1;
            return true;
        }
    }

    return false;
}

// Sets up the sprite/flat search lists

static void SetupLists()
{
    // IWAD

    if (!SetupList(&iwad_flats, &iwad, "F_START", "F_END", nullptr, nullptr))
    {
        I_Error("Flats section not found in IWAD");
    }

    if (!SetupList(&iwad_sprites, &iwad, "S_START", "S_END", nullptr, nullptr))

    {
        I_Error("Sprites section not found in IWAD");
    }

    // PWAD

    SetupList(&pwad_flats, &pwad, "F_START", "F_END", "FF_START", "FF_END");
    SetupList(&pwad_sprites, &pwad, "S_START", "S_END", "SS_START", "SS_END");
}

// Initialize the replace list

static void InitSpriteList()
{
    if (sprite_frames == nullptr)
    {
        sprite_frames_alloced = 128;
        sprite_frames         = zmalloc<decltype(sprite_frames)>(sizeof(*sprite_frames) * static_cast<unsigned long>(sprite_frames_alloced),
            PU_STATIC, nullptr);
    }

    num_sprite_frames = 0;
}

static bool ValidSpriteLumpName(char *name)
{
    if (name[0] == '\0' || name[1] == '\0'
        || name[2] == '\0' || name[3] == '\0')
    {
        return false;
    }

    // First frame:

    if (name[4] == '\0' || !isdigit(name[5]))
    {
        return false;
    }

    // Second frame (optional):

    if (name[6] != '\0' && !isdigit(name[7]))
    {
        return false;
    }

    return true;
}

// Find a sprite frame

static sprite_frame_t *FindSpriteFrame(char *name, int frame)
{
    // Search the list and try to find the frame

    for (int i = 0; i < num_sprite_frames; ++i)
    {
        sprite_frame_t *cur = &sprite_frames[i];

        if (!strncasecmp(cur->sprname, name, 4) && cur->frame == frame)
        {
            return cur;
        }
    }

    // Not found in list; Need to add to the list

    // Grow list?

    if (num_sprite_frames >= sprite_frames_alloced)
    {
        sprite_frame_t *newframes = zmalloc<decltype(newframes)>(
            static_cast<unsigned long>(sprite_frames_alloced * 2) * sizeof(*sprite_frames),
            PU_STATIC, nullptr);
        std::memcpy(newframes, sprite_frames, static_cast<unsigned long>(sprite_frames_alloced) * sizeof(*sprite_frames));
        Z_Free(sprite_frames);
        sprite_frames_alloced *= 2;
        sprite_frames = newframes;
    }

    // Add to end of list

    sprite_frame_t *result = &sprite_frames[num_sprite_frames];
    std::memcpy(result->sprname, name, 4);
    result->frame = static_cast<char>(frame);

    for (auto & angle_lump : result->angle_lumps)
        angle_lump = nullptr;

    ++num_sprite_frames;

    return result;
}

// Check if sprite lump is needed in the new wad

static bool SpriteLumpNeeded(lumpinfo_t *lump)
{
    if (!ValidSpriteLumpName(lump->name))
    {
        return true;
    }

    // check the first frame
    sprite_frame_t *sprite = FindSpriteFrame(lump->name, lump->name[4]);
    int angle_num = lump->name[5] - '0';

    if (angle_num == 0)
    {
        // must check all frames

        for (auto & angle_lump : sprite->angle_lumps)
        {
            if (angle_lump == lump)
                return true;
        }
    }
    else
    {
        // check if this lump is being used for this frame

        if (sprite->angle_lumps[angle_num - 1] == lump)
            return true;
    }

    // second frame if any

    // no second frame?
    if (lump->name[6] == '\0')
        return false;

    sprite    = FindSpriteFrame(lump->name, lump->name[6]);
    angle_num = lump->name[7] - '0';

    if (angle_num == 0)
    {
        // must check all frames

        for (auto & angle_lump : sprite->angle_lumps)
        {
            if (angle_lump == lump)
                return true;
        }
    }
    else
    {
        // check if this lump is being used for this frame

        if (sprite->angle_lumps[angle_num - 1] == lump)
            return true;
    }

    return false;
}

static void AddSpriteLump(lumpinfo_t *lump)
{
    if (!ValidSpriteLumpName(lump->name))
    {
        return;
    }

    // first angle

    sprite_frame_t *sprite = FindSpriteFrame(lump->name, lump->name[4]);
    int angle_num = lump->name[5] - '0';

    if (angle_num == 0)
    {
        for (auto & angle_lump : sprite->angle_lumps)
            angle_lump = lump;
    }
    else
    {
        sprite->angle_lumps[angle_num - 1] = lump;
    }

    // second angle

    // no second angle?

    if (lump->name[6] == '\0')
        return;

    sprite    = FindSpriteFrame(lump->name, lump->name[6]);
    angle_num = lump->name[7] - '0';

    if (angle_num == 0)
    {
        for (auto & angle_lump : sprite->angle_lumps)
            angle_lump = lump;
    }
    else
    {
        sprite->angle_lumps[angle_num - 1] = lump;
    }
}

// Generate the list.  Run at the start, before merging

static void GenerateSpriteList()
{
    InitSpriteList();

    // Add all sprites from the IWAD

    for (int i = 0; i < iwad_sprites.numlumps; ++i)
    {
        AddSpriteLump(iwad_sprites.lumps[i]);
    }

    // Add all sprites from the PWAD
    // (replaces IWAD sprites)

    for (int i = 0; i < pwad_sprites.numlumps; ++i)
    {
        AddSpriteLump(pwad_sprites.lumps[i]);
    }
}

// Perform the merge.
//
// The merge code creates a new lumpinfo list, adding entries from the
// IWAD first followed by the PWAD.
//
// For the IWAD:
//  * Flats are added.  If a flat with the same name is in the PWAD,
//    it is ignored(deleted).  At the end of the section, all flats in the
//    PWAD are inserted.  This is consistent with the behavior of
//    deutex/deusf.
//  * Sprites are added.  The "replace list" is generated before the merge
//    from the list of sprites in the PWAD.  Any sprites in the IWAD found
//    to match the replace list are removed.  At the end of the section,
//    the sprites from the PWAD are inserted.
//
// For the PWAD:
//  * All Sprites and Flats are ignored, with the assumption they have
//    already been merged into the IWAD's sections.

static void DoMerge()
{
    // Can't ever have more lumps than we already have
    auto **newlumps = static_cast<lumpinfo_t **>(calloc(numlumps, sizeof(lumpinfo_t *)));
    int num_newlumps = 0;

    // Add IWAD lumps
    section_t current_section = SECTION_NORMAL;

    for (int i = 0; i < iwad.numlumps; ++i)
    {
        lumpinfo_t *lump = iwad.lumps[i];

        switch (current_section)
        {
        case SECTION_NORMAL:
            if (!strncasecmp(lump->name, "F_START", 8))
            {
                current_section = SECTION_FLATS;
            }
            else if (!strncasecmp(lump->name, "S_START", 8))
            {
                current_section = SECTION_SPRITES;
            }

            newlumps[num_newlumps++] = lump;

            break;

        case SECTION_FLATS:

            // Have we reached the end of the section?

            if (!strncasecmp(lump->name, "F_END", 8))
            {
                // Add all new flats from the PWAD to the end
                // of the section

                for (int n = 0; n < pwad_flats.numlumps; ++n)
                {
                    newlumps[num_newlumps++] = pwad_flats.lumps[n];
                }

                newlumps[num_newlumps++] = lump;

                // back to normal reading
                current_section = SECTION_NORMAL;
            }
            else
            {
                // If there is a flat in the PWAD with the same name,
                // do not add it now.  All PWAD flats are added to the
                // end of the section. Otherwise, if it is only in the
                // IWAD, add it now

                int lumpindex = FindInList(&pwad_flats, lump->name);

                if (lumpindex < 0)
                {
                    newlumps[num_newlumps++] = lump;
                }
            }

            break;

        case SECTION_SPRITES:

            // Have we reached the end of the section?

            if (!strncasecmp(lump->name, "S_END", 8))
            {
                // add all the PWAD sprites

                for (int n = 0; n < pwad_sprites.numlumps; ++n)
                {
                    if (SpriteLumpNeeded(pwad_sprites.lumps[n]))
                    {
                        newlumps[num_newlumps++] = pwad_sprites.lumps[n];
                    }
                }

                // copy the ending
                newlumps[num_newlumps++] = lump;

                // back to normal reading
                current_section = SECTION_NORMAL;
            }
            else
            {
                // Is this lump holding a sprite to be replaced in the
                // PWAD? If so, wait until the end to add it.

                if (SpriteLumpNeeded(lump))
                {
                    newlumps[num_newlumps++] = lump;
                }
            }

            break;
        }
    }

    // Add PWAD lumps
    current_section = SECTION_NORMAL;

    for (int i = 0; i < pwad.numlumps; ++i)
    {
        lumpinfo_t *lump = pwad.lumps[i];

        switch (current_section)
        {
        case SECTION_NORMAL:
            if (!strncasecmp(lump->name, "F_START", 8)
                || !strncasecmp(lump->name, "FF_START", 8))
            {
                current_section = SECTION_FLATS;
            }
            else if (!strncasecmp(lump->name, "S_START", 8)
                     || !strncasecmp(lump->name, "SS_START", 8))
            {
                current_section = SECTION_SPRITES;
            }
            else
            {
                // Don't include the headers of sections

                newlumps[num_newlumps++] = lump;
            }
            break;

        case SECTION_FLATS:

            // PWAD flats are ignored (already merged)

            if (!strncasecmp(lump->name, "FF_END", 8)
                || !strncasecmp(lump->name, "F_END", 8))
            {
                // end of section
                current_section = SECTION_NORMAL;
            }
            break;

        case SECTION_SPRITES:

            // PWAD sprites are ignored (already merged)

            if (!strncasecmp(lump->name, "SS_END", 8)
                || !strncasecmp(lump->name, "S_END", 8))
            {
                // end of section
                current_section = SECTION_NORMAL;
            }
            break;
        }
    }

    // Switch to the new lumpinfo, and free the old one

    free(lumpinfo);
    lumpinfo = newlumps;
    numlumps = static_cast<unsigned int>(num_newlumps);
}

[[maybe_unused]] void W_PrintDirectory()
{
    // debug
    for (unsigned int i = 0; i < numlumps; ++i)
    {
        for (unsigned int n = 0; n < 8 && lumpinfo[i]->name[n] != '\0'; ++n)
            putchar(lumpinfo[i]->name[n]);
        putchar('\n');
    }
}

// Merge in a file by name

void W_MergeFile(const char *filename)
{
    int old_numlumps = static_cast<int>(numlumps);

    // Load PWAD

    if (W_AddFile(filename) == nullptr)
        return;

    // IWAD is at the start, PWAD was appended to the end

    iwad.lumps    = lumpinfo;
    iwad.numlumps = old_numlumps;

    pwad.lumps    = lumpinfo + old_numlumps;
    pwad.numlumps = static_cast<int>(numlumps - static_cast<unsigned int>(old_numlumps));

    // Setup sprite/flat lists

    SetupLists();

    // Generate list of sprites to be replaced by the PWAD

    GenerateSpriteList();

    // Perform the merge

    DoMerge();
}

// Replace lumps in the given list with lumps from the PWAD

static void W_NWTAddLumps(searchlist_t *list)
{
    // Go through the IWAD list given, replacing lumps with lumps of
    // the same name from the PWAD
    for (int i = 0; i < list->numlumps; ++i)
    {
        int index = FindInList(&pwad, list->lumps[i]->name);

        if (index > 0)
        {
            std::memcpy(list->lumps[i], pwad.lumps[index],
                sizeof(lumpinfo_t));
        }
    }
}

// Merge sprites and flats in the way NWT does with its -af and -as
// command-line options.

void W_NWTMergeFile(const char *filename, int flags)
{
    int old_numlumps = static_cast<int>(numlumps);

    // Load PWAD

    if (W_AddFile(filename) == nullptr)
        return;

    // IWAD is at the start, PWAD was appended to the end

    iwad.lumps    = lumpinfo;
    iwad.numlumps = old_numlumps;

    pwad.lumps    = lumpinfo + old_numlumps;
    pwad.numlumps = static_cast<int>(numlumps - static_cast<unsigned int>(old_numlumps));

    // Setup sprite/flat lists

    SetupLists();

    // Merge in flats?

    if (flags & W_NWT_MERGE_FLATS)
    {
        W_NWTAddLumps(&iwad_flats);
    }

    // Sprites?

    if (flags & W_NWT_MERGE_SPRITES)
    {
        W_NWTAddLumps(&iwad_sprites);
    }

    // Discard the PWAD

    numlumps = static_cast<unsigned int>(old_numlumps);
}

// Simulates the NWT -merge command line parameter.  What this does is load
// a PWAD, then search the IWAD sprites, removing any sprite lumps that also
// exist in the PWAD.

void W_NWTDashMerge(const char *filename)
{
    int old_numlumps = static_cast<int>(numlumps);

    // Load PWAD

    wad_file_t *wad_file = W_AddFile(filename);

    if (wad_file == nullptr)
    {
        return;
    }

    // IWAD is at the start, PWAD was appended to the end

    iwad.lumps    = lumpinfo;
    iwad.numlumps = old_numlumps;

    pwad.lumps    = lumpinfo + old_numlumps;
    pwad.numlumps = static_cast<int>(numlumps - static_cast<unsigned int>(old_numlumps));

    // Setup sprite/flat lists

    SetupLists();

    // Search through the IWAD sprites list.

    for (int i = 0; i < iwad_sprites.numlumps; ++i)
    {
        if (FindInList(&pwad, iwad_sprites.lumps[i]->name) >= 0)
        {
            // Replace this entry with an empty string.  This is what
            // nwt -merge does.

            M_StringCopy(iwad_sprites.lumps[i]->name, "", 8);
        }
    }

    // Discard PWAD
    // The PWAD must now be added in again with -file.

    numlumps = static_cast<unsigned int>(old_numlumps);

    W_CloseFile(wad_file);
}

// [crispy] dump merged WAD data into a new IWAD file
int W_MergeDump(const char *file)
{
    // [crispy] WAD directory structure
    typedef struct {
        uint32_t pos;
        uint32_t size;
        char     name[8];
    } directory_t;
    directory_t *dir = nullptr;

    // [crispy] open file for writing
    FILE *fp = fopen(file, "wb");
    if (!fp)
    {
        I_Error("W_MergeDump: Failed writing to file '%s'!", file);
    }

    // [crispy] prepare directory
    dir = static_cast<decltype(dir)>(calloc(numlumps, sizeof(*dir)));
    if (!dir)
    {
        I_Error("W_MergeDump: Error allocating memory!");
    }

    // [crispy] write lumps to file, starting at offset 12
    fseek(fp, 12, SEEK_SET);
    char *   lump_p = nullptr;
    uint32_t i = 0;
    for (i = 0; i < numlumps; i++)
    {
        dir[i].pos  = LONG(ftell(fp));
        dir[i].size = LONG(lumpinfo[i]->size);
        // [crispy] lump names are zero-byte padded
        std::memset(dir[i].name, 0, 8);
        strncpy(dir[i].name, lumpinfo[i]->name, 8);

        // [crispy] avoid flooding Doom's Zone Memory
        lump_p = static_cast<decltype(lump_p)>(I_Realloc(lump_p, static_cast<size_t>(lumpinfo[i]->size)));
        W_ReadLump(static_cast<lumpindex_t>(i), lump_p);
        fwrite(lump_p, 1, static_cast<size_t>(lumpinfo[i]->size), fp);
    }
    free(lump_p);

    // [crispy] write directory
    uint32_t dir_p = LONG(ftell(fp));
    fwrite(dir, sizeof(*dir), i, fp);
    free(dir);

    // [crispy] write WAD header
    fseek(fp, 0, SEEK_SET);
    fwrite("IWAD", 1, 4, fp);
    i = LONG(i);
    fwrite(&i, 4, 1, fp);
    fwrite(&dir_p, 4, 1, fp);

    fclose(fp);

    return static_cast<int>(i);
}

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
//	Handles WAD file header, directory, lump I/O.
//


#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

#include "doomtype.hpp"

#include "i_swap.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"
#include "v_diskicon.hpp"
#include "z_zone.hpp"

#include "memory.hpp"
#include "w_wad.hpp"

typedef PACKED_STRUCT(
    {
        // Should be "IWAD" or "PWAD".
        char identification[4];
        int  numlumps;
        int  infotableofs;
    }) wadinfo_t;


typedef PACKED_STRUCT(
    {
        int  filepos;
        int  size;
        char name[8];
    }) filelump_t;

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t **lumpinfo;
size_t numlumps = 0;

// Hash table for fast lookups
static lumpindex_t *lumphash;

// Variables for the reload hack: filename of the PWAD to reload, and the
// lumps from WADs before the reload file, so we can resent numlumps and
// load the file again.
static wad_file_t *reloadhandle = nullptr;
static lumpinfo_t *reloadlumps  = nullptr;
static char *      reloadname   = nullptr;
static int         reloadlump   = -1;

// Hash function used for lump names.
unsigned int W_LumpNameHash(const char *s)
{
    // This is the djb2 string hash function, modded to work on strings
    // that have a maximum length of 8.

    unsigned int result = 5381;

    for (unsigned int i = 0; i < 8 && s[i] != '\0'; ++i)
    {
        result = ((result << 5) ^ result) ^ static_cast<unsigned int>(toupper(s[i]));
    }

    return result;
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.

wad_file_t *W_AddFile(const char *filename)
{
    // If the filename begins with a ~, it indicates that we should use the
    // reload hack.
    if (filename[0] == '~')
    {
        if (reloadname != nullptr)
        {
            I_Error("Prefixing a WAD filename with '~' indicates that the "
                    "WAD should be reloaded\n"
                    "on each level restart, for use by level authors for "
                    "rapid development. You\n"
                    "can only reload one WAD file, and it must be the last "
                    "file in the -file list.");
        }

        reloadname = strdup(filename);
        reloadlump = static_cast<int>(numlumps);
        ++filename;
    }

    // Open the file and add to directory
    wad_file_t *wad_file = W_OpenFile(filename);

    if (wad_file == nullptr)
    {
        printf(" couldn't open %s\n", filename);
        return nullptr;
    }

    filelump_t *fileinfo = nullptr;
    int         numfilelumps = 0;

    if (strcasecmp(filename + strlen(filename) - 3, "wad") != 0)
    {
        // single lump file

        // fraggle: Swap the filepos and size here.  The WAD directory
        // parsing code expects a little-endian directory, so will swap
        // them back.  Effectively we're constructing a "fake WAD directory"
        // here, as it would appear on disk.

        fileinfo = zmalloc<decltype(fileinfo)>(sizeof(filelump_t), PU_STATIC, 0);
        fileinfo->filepos = LONG(0);
        fileinfo->size    = LONG(wad_file->length);

        // Name the lump after the base of the filename (without the
        // extension).

        M_ExtractFileBase(filename, fileinfo->name);
        numfilelumps = 1;
    }
    else
    {
        // WAD file
        wadinfo_t header;
        W_Read(wad_file, 0, &header, sizeof(header));

        if (strncmp(header.identification, "IWAD", 4) != 0)
        {
            // Homebrew levels?
            if (strncmp(header.identification, "PWAD", 4) != 0)
            {
                W_CloseFile(wad_file);
                I_Error("Wad file %s doesn't have IWAD "
                        "or PWAD id\n",
                    filename);
            }

            // ???modifiedgame = true;
        }

        header.numlumps = LONG(header.numlumps);

        // Vanilla Doom doesn't like WADs with more than 4046 lumps
        // https://www.doomworld.com/vb/post/1010985
        // [crispy] disable PWAD lump number limit
        if (!strncmp(header.identification, "PWAD", 4) && header.numlumps > 4046 && false)
        {
            W_CloseFile(wad_file);
            I_Error("Error: Vanilla limit for lumps in a WAD is 4046, "
                    "PWAD %s has %d",
                filename, header.numlumps);
        }

        header.infotableofs = LONG(header.infotableofs);
        size_t length       = static_cast<unsigned long>(header.numlumps) * sizeof(filelump_t);
        fileinfo            = zmalloc<decltype(fileinfo)>(length, PU_STATIC, 0);

        W_Read(wad_file, static_cast<unsigned int>(header.infotableofs), fileinfo, length);
        numfilelumps = header.numlumps;
    }

    // Increase size of numlumps array to accomodate the new file.
    auto *filelumps = static_cast<lumpinfo_t *>(calloc(static_cast<size_t>(numfilelumps), sizeof(lumpinfo_t)));
    if (filelumps == nullptr)
    {
        W_CloseFile(wad_file);
        I_Error("Failed to allocate array for lumps from new file.");
    }

    int startlump = static_cast<int>(numlumps);
    numlumps += static_cast<unsigned int>(numfilelumps);
    lumpinfo  = static_cast<decltype(lumpinfo)>(I_Realloc(lumpinfo, numlumps * sizeof(lumpinfo_t *)));
    filelump_t *filerover = fileinfo;

    for (lumpindex_t i = startlump; i < static_cast<int>(numlumps); ++i)
    {
        lumpinfo_t *lump_p = &filelumps[i - startlump];
        lump_p->wad_file   = wad_file;
        lump_p->position   = LONG(filerover->filepos);
        lump_p->size       = LONG(filerover->size);
        lump_p->cache      = nullptr;
        strncpy(lump_p->name, filerover->name, 8);
        lumpinfo[i] = lump_p;

        ++filerover;
    }

    Z_Free(fileinfo);

    if (lumphash != nullptr)
    {
        Z_Free(lumphash);
        lumphash = nullptr;
    }

    // If this is the reload file, we need to save some details about the
    // file so that we can close it later on when we do a reload.
    if (reloadname)
    {
        reloadhandle = wad_file;
        reloadlumps  = filelumps;
    }

    return wad_file;
}


//
// W_NumLumps
//
[[maybe_unused]] int W_NumLumps()
{
    return static_cast<int>(numlumps);
}


//
// W_CheckNumForName
// Returns -1 if name not found.
//

lumpindex_t W_CheckNumForName(const char *name)
{
    // Do we have a hash table yet?

    if (lumphash != nullptr)
    {
        // We do! Excellent.

        int hash = static_cast<int>(W_LumpNameHash(name) % numlumps);

        for (lumpindex_t i = lumphash[hash]; i != -1; i = lumpinfo[i]->next)
        {
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
            {
                return i;
            }
        }
    }
    else
    {
        // We don't have a hash table generate yet. Linear search :-(
        //
        // scan backwards so patch lump files take precedence

        for (auto i = static_cast<lumpindex_t>(numlumps - 1); i >= 0; --i)
        {
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
            {
                return i;
            }
        }
    }

    // TFB. Not found.

    return -1;
}


//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
lumpindex_t W_GetNumForName(const char *name)
{
    lumpindex_t i = W_CheckNumForName(name);

    if (i < 0)
    {
        I_Error("W_GetNumForName: %s not found!", name);
    }

    return i;
}

lumpindex_t W_CheckNumForNameFromTo(const char *name, int from, int to)
{
    for (lumpindex_t i = from; i >= to; i--)
    {
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
        {
            return i;
        }
    }

    return -1;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
size_t W_LumpLength(lumpindex_t lump)
{
    if (lump >= static_cast<int>(numlumps))
    {
        I_Error("W_LumpLength: %i >= numlumps", lump);
    }

    return lumpinfo[lump]->size;
}


//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(lumpindex_t lump, void *dest)
{
    if (lump >= static_cast<int>(numlumps))
    {
        I_Error("W_ReadLump: %i >= numlumps", lump);
    }

    lumpinfo_t *l = lumpinfo[lump];

    V_BeginRead(static_cast<size_t>(l->size));

    size_t c = W_Read(l->wad_file, static_cast<unsigned int>(l->position), dest, static_cast<size_t>(l->size));

    if (c < l->size)
    {
        I_Error("W_ReadLump: only read %i of %i on lump %i",
            c, l->size, lump);
    }
}


//
// W_CacheLumpNum
//
// Load a lump into memory and return a pointer to a buffer containing
// the lump data.
//
// 'tag' is the type of zone memory buffer to allocate for the lump
// (usually PU_STATIC or PU_CACHE).  If the lump is loaded as
// PU_STATIC, it should be released back using W_ReleaseLumpNum
// when no longer needed (do not use Z_ChangeTag).
//

[[nodiscard]] void *W_CacheLumpNum(lumpindex_t lumpnum, int tag)
{
    void *result = nullptr;

    if (static_cast<unsigned>(lumpnum) >= numlumps)
    {
        I_Error("W_CacheLumpNum: %i >= numlumps", lumpnum);
    }

    lumpinfo_t *lump = lumpinfo[lumpnum];

    // Get the pointer to return.  If the lump is in a memory-mapped
    // file, we can just return a pointer to within the memory-mapped
    // region.  If the lump is in an ordinary file, we may already
    // have it cached; otherwise, load it into memory.

    if (lump->wad_file->mapped != nullptr)
    {
        // Memory mapped file, return from the mmapped region.

        result = lump->wad_file->mapped + lump->position;
    }
    else if (lump->cache != nullptr)
    {
        // Already cached, so just switch the zone tag.

        result = lump->cache;
        Z_ChangeTag(lump->cache, tag);
    }
    else
    {
        // Not yet loaded, so load it now

        lump->cache = zmalloc<decltype(lump->cache)>(W_LumpLength(lumpnum), tag, &lump->cache);
        W_ReadLump(lumpnum, lump->cache);
        result = lump->cache;
    }

    return result;
}


//
// W_CacheLumpName
//
[[nodiscard]] void *W_CacheLumpName(const char *name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//
// Release a lump back to the cache, so that it can be reused later
// without having to read from disk again, or alternatively, discarded
// if we run out of memory.
//
// Back in Vanilla Doom, this was just done using Z_ChangeTag
// directly, but now that we have WAD mmap, things are a bit more
// complicated ...
//

void W_ReleaseLumpNum(lumpindex_t lumpnum)
{
    if (static_cast<unsigned>(lumpnum) >= numlumps)
    {
        I_Error("W_ReleaseLumpNum: %i >= numlumps", lumpnum);
    }

    lumpinfo_t *lump = lumpinfo[lumpnum];

    if (lump->wad_file->mapped != nullptr)
    {
        // Memory-mapped file, so nothing needs to be done here.
    }
    else
    {
        Z_ChangeTag(lump->cache, PU_CACHE);
    }
}

void W_ReleaseLumpName(const char *name)
{
    W_ReleaseLumpNum(W_GetNumForName(name));
}

#if 0

//
// W_Profile
//
int		info[2500][10];
int		profilecount;

void W_Profile ()
{
    int		i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    int		j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpinfo[i].cache;
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = reinterpret_cast<memblock_t *>( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
	std::memcpy (name,lumpinfo[i].name,8);

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}


#endif

// Generate a hash table for fast lookups

void W_GenerateHashTable()
{
    // Free the old hash table, if there is one:
    if (lumphash != nullptr)
    {
        Z_Free(lumphash);
    }

    // Generate hash table
    if (numlumps > 0)
    {
        lumphash = zmalloc<decltype(lumphash)>(sizeof(lumpindex_t) * numlumps, PU_STATIC, nullptr);

        for (unsigned int i = 0; i < numlumps; ++i)
        {
            lumphash[i] = -1;
        }

        for (unsigned int i = 0; i < numlumps; ++i)
        {
            unsigned int hash = static_cast<unsigned int>(W_LumpNameHash(lumpinfo[i]->name) % numlumps);

            // Hook into the hash table

            lumpinfo[i]->next = lumphash[hash];
            lumphash[hash]    = static_cast<lumpindex_t>(i);
        }
    }

    // All done!
}

// The Doom reload hack. The idea here is that if you give a WAD file to -file
// prefixed with the ~ hack, that WAD file will be reloaded each time a new
// level is loaded. This lets you use a level editor in parallel and make
// incremental changes to the level you're working on without having to restart
// the game after every change.
// But: the reload feature is a fragile hack...
void W_Reload()
{
    if (reloadname == nullptr)
    {
        return;
    }

    // We must free any lumps being cached from the PWAD we're about to reload:
    for (unsigned int i = static_cast<unsigned int>(reloadlump); i < numlumps; ++i)
    {
        if (lumpinfo[i]->cache != nullptr)
        {
            Z_Free(lumpinfo[i]->cache);
        }
    }

    // Reset numlumps to remove the reload WAD file:
    numlumps = static_cast<unsigned int>(reloadlump);

    // Now reload the WAD file.
    char *filename = reloadname;

    W_CloseFile(reloadhandle);
    free(reloadlumps);

    reloadname   = nullptr;
    reloadlump   = -1;
    reloadhandle = nullptr;
    W_AddFile(filename);
    free(filename);

    // The WAD directory has changed, so we have to regenerate the
    // fast lookup hashtable:
    W_GenerateHashTable();
}

const char *W_WadNameForLump(const lumpinfo_t *lump)
{
    return M_BaseName(lump->wad_file->path);
}

bool W_IsIWADLump(const lumpinfo_t *lump)
{
    return lump->wad_file == lumpinfo[0]->wad_file;
}

// [crispy] dump lump data into a new LMP file
int W_LumpDump(const char *lumpname)
{
    const auto i = W_CheckNumForName(lumpname);

    if (i < 0 || !lumpinfo[i]->size)
    {
        return -1;
    }

    // [crispy] open file for writing
    std::string filename = std::string(lumpname) + ".lmp";
    M_ForceLowercase(filename.data());
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        I_Error("W_LumpDump: Failed writing to file '%s'!", filename.c_str());
    }

    std::string lump_p(static_cast<unsigned long>(lumpinfo[i]->size), 0);
    W_ReadLump(i, lump_p.data());
    fwrite(lump_p.data(), 1, static_cast<size_t>(lumpinfo[i]->size), fp);

    fclose(fp);

    return i;
}

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
//	Preparation of data for rendering,
//	generation of lookups, caching, retrieval by name.
//

#include <cstdio>
#include <cstdlib> // [crispy] calloc()

#include "deh_main.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"


#include "w_wad.hpp"

#include "doomdef.hpp"
#include "m_misc.hpp"
#include "r_local.hpp"
#include "p_local.hpp"

#include "doomstat.hpp"
#include "r_sky.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "r_bmaps.hpp" // [crispy] R_BrightmapForTexName()
#include "r_data.hpp"
#include "v_trans.hpp" // [crispy] tranmap, CRMAX

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//


//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef PACKED_STRUCT(
    {
        short originx;
        short originy;
        short patch;
        short stepdir;
        short colormap;
    }) mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef PACKED_STRUCT(
    {
        char       name[8];
        int        masked;
        short      width;
        short      height;
        int        obsolete;
        short      patchcount;
        mappatch_t patches[1];
    }) maptexture_t;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short originx;
    short originy;
    int   patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

using texture_t = struct texture_s;

struct texture_s {
    // Keep name for switch changing, etc.
    char  name[8];
    short width;
    short height;

    // Index in textures list

    int index;

    // Next in hash table chain

    texture_t *next;

    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    short      patchcount;
    texpatch_t patches[1];
};


int lastflat;
int numflats;

int firstpatch;
int lastpatch;
int numpatches;

int         numtextures;
texture_t **textures;
texture_t **textures_hashtable;


int *texturewidthmask;
// needed for texture pegging
int *      texturecompositesize;
short **   texturecolumnlump;
unsigned **texturecolumnofs;  // killough 4/9/98: make 32-bit
unsigned **texturecolumnofs2; // [crispy] original column offsets for single-patched textures
uint8_t  **texturecomposite;
uint8_t  **texturebrightmap; // [crispy] brightmaps

//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//


// [crispy] replace R_DrawColumnInCache(), R_GenerateComposite() and R_GenerateLookup()
// with Lee Killough's implementations found in MBF to fix Medusa bug
// taken from mbfsrc/R_DATA.C:136-425
//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

void R_DrawColumnInCache(column_t *patch, uint8_t *cache,
    int                            originy,
    int                            cacheheight, uint8_t *marks)
{
    int   count;
    int   position;
    uint8_t *source;
    int   top = -1;

    while (patch->topdelta != 0xff)
    {
        // [crispy] support for DeePsea tall patches
        if (patch->topdelta <= top)
        {
            top += patch->topdelta;
        }
        else
        {
            top = patch->topdelta;
        }
        source   = reinterpret_cast<uint8_t *>(patch + 3);
        count    = patch->length;
        position = originy + top;

        if (position < 0)
        {
            count += position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
        {
            std::memcpy(cache + position, source, static_cast<size_t>(count));

            // killough 4/9/98: remember which cells in column have been drawn,
            // so that column can later be converted into a series of posts, to
            // fix the Medusa bug.

            std::memset(marks + position, 0xff, static_cast<size_t>(count));
        }

        uint8_t *col_ptr = reinterpret_cast<uint8_t *>(patch) + patch->length + 4;
        patch = reinterpret_cast<column_t *>(col_ptr);
    }
}


//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug

void R_GenerateComposite(int texnum)
{
    uint8_t    *block;
    texture_t * texture;
    texpatch_t *patch;
    patch_t *   realpatch;
    int         x;
    int         x1;
    int         x2;
    int         i;
    column_t *  patchcol;
    short *     collump;
    unsigned *  colofs; // killough 4/9/98: make 32-bit
    uint8_t    *marks;  // killough 4/9/98: transparency marks
    uint8_t    *source; // killough 4/9/98: temporary column

    texture = textures[texnum];

    block = zmalloc<decltype(block)>(static_cast<size_t>(texturecompositesize[texnum]),
        PU_STATIC,
        &texturecomposite[texnum]);

    collump = texturecolumnlump[texnum];
    colofs  = texturecolumnofs[texnum];

    // Composite the columns together.
    patch = texture->patches;

    // killough 4/9/98: marks to identify transparent regions in merged textures
    marks = static_cast<decltype(marks)>(calloc(static_cast<size_t>(texture->width), static_cast<size_t>(texture->height)));

    // [crispy] initialize composite background to black (index 0)
    std::memset(block, 0, static_cast<size_t>(texturecompositesize[texnum]));

    for (i = 0, patch = texture->patches;
         i < texture->patchcount;
         i++, patch++)
    {
        realpatch = cache_lump_num<patch_t *>(patch->patch, PU_CACHE);
        x1        = patch->originx;
        x2        = x1 + SHORT(realpatch->width);

        if (x1 < 0)
            x = 0;
        else
            x = x1;

        if (x2 > texture->width)
            x2 = texture->width;

        for (; x < x2; x++)
        {
            // Column does not have multiple patches?
            // [crispy] generate composites for single-patched columns as well
            /*
	    if (collump[x] >= 0)
		continue;
	    */

            uint8_t *col_ptr = reinterpret_cast<uint8_t *>(realpatch) + LONG(realpatch->columnofs[x - x1]);
            patchcol = reinterpret_cast<column_t *>(col_ptr);
            R_DrawColumnInCache(patchcol,
                block + colofs[x],
                // [crispy] single-patched columns are normally not composited
                // but directly read from the patch lump ignoring their originy
                collump[x] >= 0 ? 0 : patch->originy,
                texture->height,
                marks + x * texture->height);
        }
    }

    // killough 4/9/98: Next, convert multipatched columns into true columns,
    // to fix Medusa bug while still allowing for transparent regions.

    source = static_cast<decltype(source)>(I_Realloc(nullptr, static_cast<size_t>(texture->height))); // temporary column
    for (i = 0; i < texture->width; i++)
    {
        if (collump[i] == -1) // process only multipatched columns
        {
            column_t *  col  = reinterpret_cast<column_t *>(block + colofs[i] - 3); // cached column
            const uint8_t *mark = marks + i * texture->height;
            int         j    = 0;

            // save column in temporary so we can shuffle it around
            std::memcpy(source, reinterpret_cast<uint8_t *>(col) + 3, static_cast<size_t>(texture->height));

            for (;;) // reconstruct the column by scanning transparency marks
            {
                while (j < texture->height && !mark[j]) // skip transparent cells
                    j++;

                if (j >= texture->height) // if at end of column
                {
                    col->topdelta = static_cast<uint8_t>(-1); // end-of-column marker
                    break;
                }

                col->topdelta = static_cast<uint8_t>(j); // starting offset of post

                // killough 12/98:
                // Use 32-bit len counter, to support tall 1s multipatched textures
                unsigned len = 0;
                for (len = 0; j < texture->height && mark[j]; j++)
                    len++; // count opaque cells

                col->length = static_cast<uint8_t>(len); // killough 12/98: intentionally truncate length

                // copy opaque cells from the temporary back into the column
                std::memcpy(reinterpret_cast<uint8_t *>(col) + 3, source + col->topdelta, len);
                uint8_t *col_ptr = reinterpret_cast<uint8_t *>(col) + len + 4;
                col = reinterpret_cast<column_t *>(col_ptr); // next post
            }
        }
    }

    free(source); // free temporary column
    free(marks);  // free transparency marks

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag(block, PU_CACHE);
}


//
// R_GenerateLookup
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

void R_GenerateLookup(int texnum)
{
    texture_t * texture;
    uint8_t    *patchcount; // patchcount[texture->width]
    uint8_t    *postcount;  // killough 4/9/98: keep count of posts in addition to patches.
    texpatch_t *patch;
    patch_t *   realpatch;
    int         x1;
    int         x2;
    int         i;
    short *     collump;
    unsigned *  colofs;    // killough 4/9/98: make 32-bit
    unsigned *  colofs2;   // [crispy] original column offsets
    int         csize = 0; // killough 10/98
    int         err   = 0; // killough 10/98

    texture = textures[texnum];

    // Composited texture not created yet.
    texturecomposite[texnum] = 0;

    texturecompositesize[texnum] = 0;
    collump                      = texturecolumnlump[texnum];
    colofs                       = texturecolumnofs[texnum];
    colofs2                      = texturecolumnofs2[texnum]; // [crispy] original column offsets

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.
    patchcount = zmalloc<uint8_t *>(static_cast<size_t>(texture->width), PU_STATIC, &patchcount);
    postcount  = zmalloc<uint8_t *>(static_cast<size_t>(texture->width), PU_STATIC, &postcount);
    std::memset(patchcount, 0, static_cast<size_t>(texture->width));
    std::memset(postcount, 0, static_cast<size_t>(texture->width));
    patch = texture->patches;

    for (i = 0, patch = texture->patches;
         i < texture->patchcount;
         i++, patch++)
    {
        realpatch = cache_lump_num<patch_t *>(patch->patch, PU_CACHE);
        x1        = patch->originx;
        x2        = x1 + SHORT(realpatch->width);

        int x = 0;
        if (x1 < 0)
            x = 0;
        else
            x = x1;

        if (x2 > texture->width)
            x2 = texture->width;
        for (; x < x2; x++)
        {
            patchcount[x]++;
            collump[x] = static_cast<short>(patch->patch);
            colofs[x] = colofs2[x] = static_cast<unsigned int>(LONG(realpatch->columnofs[x - x1]) + 3); // [crispy] original column offsets
        }
    }

    // killough 4/9/98: keep a count of the number of posts in column,
    // to fix Medusa bug while allowing for transparent multipatches.
    //
    // killough 12/98:
    // Post counts are only necessary if column is multipatched,
    // so skip counting posts if column comes from a single patch.
    // This allows arbitrarily tall textures for 1s walls.
    //
    // If texture is >= 256 tall, assume it's 1s, and hence it has
    // only one post per column. This avoids crashes while allowing
    // for arbitrarily tall multipatched 1s textures.

    if (texture->patchcount > 1 && texture->height < 256)
    {
        // killough 12/98: Warn about a common column construction bug
        unsigned limit = static_cast<unsigned int>(texture->height * 3 + 3); // absolute column size limit

        for (i = texture->patchcount, patch = texture->patches; --i >= 0;)
        {
            int            pat       = patch->patch;
            const patch_t *realpatch_local = cache_lump_num<const patch_t *>(pat, PU_CACHE);
            int            originx         = patch++->originx;
            int            x2_local        = originx + SHORT(realpatch_local->width);
            const int *    cofs = realpatch_local->columnofs - originx;

            if (x2_local > texture->width) x2_local = texture->width;
            if (originx < 0) originx = 0;

            for (int x = originx; x < x2_local; x++)
            {
                if (patchcount[x] > 1) // Only multipatched columns
                {
                    const uint8_t  *col_ptr = reinterpret_cast<const uint8_t *>(realpatch_local) + LONG(cofs[x]);
                    const auto *col  = reinterpret_cast<const column_t *>(col_ptr);
                    const auto  *base = reinterpret_cast<const uint8_t *>(col);

                    // count posts
                    for (; col->topdelta != 0xff; postcount[x]++)
                    {
                        if (static_cast<unsigned>(reinterpret_cast<const uint8_t *>(col) - base) <= limit)
                        {
                            const uint8_t *col_ptr2 = reinterpret_cast<const uint8_t *>(col) + col->length + 4;
                            col = reinterpret_cast<const column_t *>(col_ptr2);
                        }
                        else
                            break;
                    }
                }
            }
        }
    }

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.

    for (int x = 0; x < texture->width; x++)
    {
        if (!patchcount[x] && !err++) // killough 10/98: non-verbose output
        {
            // [crispy] fix absurd texture name in error message
            char namet[9];
            namet[8] = 0;
            std::memcpy(namet, texture->name, 8);
            printf("R_GenerateLookup: column without a patch (%s)\n",
                namet);
            // [crispy] do not return yet
            /*
	    return;
	    */
        }
        // I_Error ("R_GenerateLookup: column without a patch");

        // [crispy] treat patch-less columns the same as multi-patched
        if (patchcount[x] > 1 || !patchcount[x])
        {
            // Use the cached block.
            // [crispy] moved up here, the rest in this loop
            // applies to single-patched textures as well
            collump[x] = -1;
        }
        // killough 1/25/98, 4/9/98:
        //
        // Fix Medusa bug, by adding room for column header
        // and trailer bytes for each post in merged column.
        // For now, just allocate conservatively 4 bytes
        // per post per patch per column, since we don't
        // yet know how many posts the merged column will
        // require, and it's bounded above by this limit.

        colofs[x] = static_cast<unsigned int>(csize + 3); // three header bytes in a column
        // killough 12/98: add room for one extra post
        csize += 4 * postcount[x] + 5; // 1 stop byte plus 4 bytes per post

        // [crispy] remove limit
        /*
	    if (texturecompositesize[texnum] > 0x10000-texture->height)
	    {
		I_Error ("R_GenerateLookup: texture %i is >64k",
			 texnum);
	    }
	    */
        csize += texture->height; // height bytes of texture data
    }

    texturecompositesize[texnum] = csize;

    Z_Free(patchcount);
    Z_Free(postcount);
}


//
// R_GetColumn
//
uint8_t *
    R_GetColumn(int tex,
        int         col,
        bool     opaque)
{
    int lump;
    int ofs;
    int ofs2;

    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs  = static_cast<int>(texturecolumnofs[tex][col]);
    ofs2 = static_cast<int>(texturecolumnofs2[tex][col]);

    // [crispy] single-patched mid-textures on two-sided walls
    if (lump > 0 && !opaque)
        return cache_lump_num<uint8_t *>(lump, PU_CACHE) + ofs2;

    if (!texturecomposite[tex])
        R_GenerateComposite(tex);

    return texturecomposite[tex] + ofs;
}


static void GenerateTextureHashTable()
{
    texture_t **rover;
    int         i;
    int         key;

    textures_hashtable = zmalloc<decltype(textures_hashtable)>(sizeof(texture_t *) * static_cast<unsigned long>(numtextures), PU_STATIC, 0);

    std::memset(textures_hashtable, 0, sizeof(texture_t *) * static_cast<unsigned long>(numtextures));

    // Add all textures to hash table

    for (i = 0; i < numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = static_cast<int>(W_LumpNameHash(textures[i]->name) % static_cast<unsigned int>(numtextures));

        rover = &textures_hashtable[key];

        while (*rover != nullptr)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = nullptr;
        *rover            = textures[i];
    }
}


//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
// [crispy] partly rewritten to merge PNAMES and TEXTURE1/2 lumps
void R_InitTextures()
{
    maptexture_t *mtexture;
    texture_t *   texture;
    mappatch_t *  mpatch;
    texpatch_t *  patch;

    int i;
    int j;
    int k;

    int *maptex = nullptr;

    char name[9];

    int *patchlookup;

    int totalwidth;
    int nummappatches;
    int offset;
    int maxoff = 0;

    int *directory = nullptr;

    int temp1;
    int temp2;
    int temp3;

    typedef struct
    {
        int   lumpnum;
        void *names;
        short nummappatches;
        short summappatches;
        char *name_p;
    } pnameslump_t;

    typedef struct
    {
        int   lumpnum;
        int * maptex;
        int   maxoff;
        short numtextures;
        short sumtextures;
        short pnamesoffset;
    } texturelump_t;

    pnameslump_t * pnameslumps  = nullptr;
    texturelump_t *texturelumps = nullptr, *texturelump;

    int maxpnameslumps  = 1; // PNAMES
    int maxtexturelumps = 2; // TEXTURE1, TEXTURE2

    int numpnameslumps  = 0;
    int numtexturelumps = 0;

    // [crispy] allocate memory for the pnameslumps and texturelumps arrays
    pnameslumps  = static_cast<decltype(pnameslumps)>(I_Realloc(pnameslumps, static_cast<unsigned long>(maxpnameslumps) * sizeof(*pnameslumps)));
    texturelumps = static_cast<decltype(texturelumps)>(I_Realloc(texturelumps, static_cast<unsigned long>(maxtexturelumps) * sizeof(*texturelumps)));

    // [crispy] make sure the first available TEXTURE1/2 lumps
    // are always processed first
    texturelumps[numtexturelumps++].lumpnum = W_GetNumForName(DEH_String("TEXTURE1"));
    if ((i = W_CheckNumForName(DEH_String("TEXTURE2"))) != -1)
        texturelumps[numtexturelumps++].lumpnum = i;
    else
        texturelumps[numtexturelumps].lumpnum = -1;

    // [crispy] fill the arrays with all available PNAMES lumps
    // and the remaining available TEXTURE1/2 lumps
    nummappatches = 0;
    for (i = static_cast<int>(numlumps - 1); i >= 0; i--)
    {
        if (!strncasecmp(lumpinfo[i]->name, DEH_String("PNAMES"), 6))
        {
            if (numpnameslumps == maxpnameslumps)
            {
                maxpnameslumps++;
                pnameslumps = static_cast<decltype(pnameslumps)>(I_Realloc(pnameslumps, static_cast<unsigned long>(maxpnameslumps) * sizeof(*pnameslumps)));
            }

            pnameslump_t &lump = pnameslumps[numpnameslumps];
            lump.lumpnum       = i;
            lump.names         = cache_lump_num<patch_t *>(lump.lumpnum, PU_STATIC);
            lump.nummappatches = static_cast<short>(LONG(*(reinterpret_cast<int *>(lump.names))));

            // [crispy] accumulated number of patches in the lookup tables
            // excluding the current one
            lump.summappatches = static_cast<short>(nummappatches);
            lump.name_p        = reinterpret_cast<char *>(lump.names) + 4;

            // [crispy] calculate total number of patches
            nummappatches += lump.nummappatches;
            numpnameslumps++;
        }
        else if (!strncasecmp(lumpinfo[i]->name, DEH_String("TEXTURE"), 7))
        {
            // [crispy] support only TEXTURE1/2 lumps, not TEXTURE3 etc.
            if (lumpinfo[i]->name[7] != '1' && lumpinfo[i]->name[7] != '2')
                continue;

            // [crispy] make sure the first available TEXTURE1/2 lumps
            // are not processed again
            if (i == texturelumps[0].lumpnum || i == texturelumps[1].lumpnum) // [crispy] may still be -1
                continue;

            if (numtexturelumps == maxtexturelumps)
            {
                maxtexturelumps++;
                texturelumps = static_cast<decltype(texturelumps)>(I_Realloc(texturelumps, static_cast<unsigned long>(maxtexturelumps) * sizeof(*texturelumps)));
            }

            // [crispy] do not proceed any further, yet
            // we first need a complete pnameslumps[] array and need
            // to process texturelumps[0] (and also texturelumps[1]) as well
            texturelumps[numtexturelumps].lumpnum = i;
            numtexturelumps++;
        }
    }

    // [crispy] fill up the patch lookup table
    name[8]     = 0;
    patchlookup = zmalloc<decltype(patchlookup)>(static_cast<unsigned long>(nummappatches) * sizeof(*patchlookup), PU_STATIC, nullptr);
    for (i = 0, k = 0; i < numpnameslumps; i++)
    {
        for (j = 0; j < pnameslumps[i].nummappatches; j++)
        {
            int p, po;

            M_StringCopy(name, pnameslumps[i].name_p + j * 8, sizeof(name));
            p = po = W_CheckNumForName(name);
            // [crispy] prevent flat lumps from being mistaken as patches
            while (p >= g_r_state_globals->firstflat && p <= lastflat)
            {
                p = W_CheckNumForNameFromTo(name, p - 1, 0);
            }
            // [crispy] if the name is unambiguous, use the lump we found
            patchlookup[k++] = (p == -1) ? po : p;
        }
    }

    // [crispy] calculate total number of textures
    numtextures = 0;
    for (i = 0; i < numtexturelumps; i++)
    {
        texturelumps[i].maptex      = cache_lump_num<int *>(texturelumps[i].lumpnum, PU_STATIC);
        texturelumps[i].maxoff      = static_cast<int>(W_LumpLength(texturelumps[i].lumpnum));
        texturelumps[i].numtextures = static_cast<short>(LONG(*texturelumps[i].maptex));

        // [crispy] accumulated number of textures in the texture files
        // including the current one
        numtextures += texturelumps[i].numtextures;
        texturelumps[i].sumtextures = static_cast<short>(numtextures);

        // [crispy] link textures to their own WAD's patch lookup table (if any)
        texturelumps[i].pnamesoffset = 0;
        for (j = 0; j < numpnameslumps; j++)
        {
            // [crispy] both are from the same WAD?
            if (lumpinfo[texturelumps[i].lumpnum]->wad_file == lumpinfo[pnameslumps[j].lumpnum]->wad_file)
            {
                texturelumps[i].pnamesoffset = pnameslumps[j].summappatches;
                break;
            }
        }
    }

    // [crispy] release memory allocated for patch lookup tables
    for (i = 0; i < numpnameslumps; i++)
    {
        W_ReleaseLumpNum(pnameslumps[i].lumpnum);
    }
    free(pnameslumps);

    // [crispy] pointer to (i.e. actually before) the first texture file
    texturelump = texturelumps - 1; // [crispy] gets immediately increased below

    textures             = zmalloc<decltype(textures)>(static_cast<unsigned long>(numtextures) * sizeof(*textures), PU_STATIC, 0);
    texturecolumnlump    = zmalloc<decltype(texturecolumnlump)>(static_cast<unsigned long>(numtextures) * sizeof(*texturecolumnlump), PU_STATIC, 0);
    texturecolumnofs     = zmalloc<decltype(texturecolumnofs)>(static_cast<unsigned long>(numtextures) * sizeof(*texturecolumnofs), PU_STATIC, 0);
    texturecolumnofs2    = zmalloc<decltype(texturecolumnofs2)>(static_cast<unsigned long>(numtextures) * sizeof(*texturecolumnofs2), PU_STATIC, 0);
    texturecomposite     = zmalloc<decltype(texturecomposite)>(static_cast<unsigned long>(numtextures) * sizeof(*texturecomposite), PU_STATIC, 0);
    texturecompositesize = zmalloc<decltype(texturecompositesize)>(static_cast<unsigned long>(numtextures) * sizeof(*texturecompositesize), PU_STATIC, 0);
    texturewidthmask     = zmalloc<decltype(texturewidthmask)>(static_cast<unsigned long>(numtextures) * sizeof(*texturewidthmask), PU_STATIC, 0);
    g_r_state_globals->textureheight        = zmalloc<decltype(g_r_state_globals->textureheight)>(static_cast<unsigned long>(numtextures) * sizeof(*g_r_state_globals->textureheight), PU_STATIC, 0);
    texturebrightmap     = zmalloc<decltype(texturebrightmap)>(static_cast<unsigned long>(numtextures) * sizeof(*texturebrightmap), PU_STATIC, 0);

    totalwidth = 0;

    //	Really complex printing shit...
    temp1 = W_GetNumForName(DEH_String("S_START")); // P_???????
    temp2 = W_GetNumForName(DEH_String("S_END")) - 1;
    temp3 = ((temp2 - temp1 + 63) / 64) + ((numtextures + 63) / 64);

    // If stdout is a real console, use the classic vanilla "filling
    // up the box" effect, which uses backspace to "step back" inside
    // the box.  If stdout is a file, don't draw the box.

    if (I_ConsoleStdout())
    {
        printf("[");
#ifndef CRISPY_TRUECOLOR
        for (i = 0; i < temp3 + 9 + 1; i++) // [crispy] one more for R_InitTranMap()
#else
        for (i = 0; i < temp3 + 9; i++)
#endif
            printf(" ");
        printf("]");
#ifndef CRISPY_TRUECOLOR
        for (i = 0; i < temp3 + 10 + 1; i++) // [crispy] one more for R_InitTranMap()
#else
        for (i = 0; i < temp3 + 10; i++)
#endif
            printf("\b");
    }

    for (i = 0; i < numtextures; i++, directory++)
    {
        if (!(i & 63))
            printf(".");

        // [crispy] initialize for the first texture file lump,
        // skip through empty texture file lumps which do not contain any texture
        while (texturelump == texturelumps - 1 || i == texturelump->sumtextures)
        {
            // [crispy] start looking in next texture file
            texturelump++;
            maptex    = texturelump->maptex;
            maxoff    = texturelump->maxoff;
            directory = maptex + 1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error("R_InitTextures: bad texture directory");

        uint8_t *byte_ptr = reinterpret_cast<uint8_t *>(maptex) + offset;
        mtexture = reinterpret_cast<maptexture_t *>(byte_ptr);

        texture = textures[i] = zmalloc<decltype(texture)>(sizeof(texture_t)
                                                               + sizeof(texpatch_t) * (SHORT(mtexture->patchcount) - 1),
            PU_STATIC, 0);

        texture->width      = SHORT(mtexture->width);
        texture->height     = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        std::memcpy(texture->name, mtexture->name, sizeof(texture->name));
        mpatch = &mtexture->patches[0];
        patch  = &texture->patches[0];

        // [crispy] initialize brightmaps
        texturebrightmap[i] = R_BrightmapForTexName(texture->name);

        for (j = 0; j < texture->patchcount; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            // [crispy] apply offset for patches not in the
            // first available patch offset table
            short p = static_cast<short>(SHORT(mpatch->patch) + texturelump->pnamesoffset);
            // [crispy] catch out-of-range patches
            if (p < nummappatches)
                patch->patch = patchlookup[p];
            if (patch->patch == -1 || p >= nummappatches)
            {
                char texturename[9];
                texturename[8] = '\0';
                std::memcpy(texturename, texture->name, 8);
                // [crispy] make non-fatal
                fprintf(stderr, "R_InitTextures: Missing patch in texture %s\n",
                    texturename);
                patch->patch = 0;
            }
        }
        texturecolumnlump[i] = zmalloc<short *>(static_cast<unsigned long>(texture->width) * sizeof(**texturecolumnlump), PU_STATIC, 0);
        texturecolumnofs[i]  = zmalloc<unsigned *>(static_cast<unsigned long>(texture->width) * sizeof(**texturecolumnofs), PU_STATIC, 0);
        texturecolumnofs2[i] = zmalloc<unsigned *>(static_cast<unsigned long>(texture->width) * sizeof(**texturecolumnofs2), PU_STATIC, 0);

        j = 1;
        while (j * 2 <= texture->width)
            j <<= 1;

        texturewidthmask[i] = j - 1;
        g_r_state_globals->textureheight[i]    = texture->height << FRACBITS;

        totalwidth += texture->width;
    }

    Z_Free(patchlookup);

    // [crispy] release memory allocated for texture files
    for (i = 0; i < numtexturelumps; i++)
    {
        W_ReleaseLumpNum(texturelumps[i].lumpnum);
    }
    free(texturelumps);

    // Precalculate whatever possible.

    for (i = 0; i < numtextures; i++)
        R_GenerateLookup(i);

    // Create translation table for global animation.
    g_r_state_globals->texturetranslation = zmalloc<decltype(g_r_state_globals->texturetranslation)>((static_cast<unsigned long>(numtextures + 1)) * sizeof(*g_r_state_globals->texturetranslation), PU_STATIC, 0);

    for (i = 0; i < numtextures; i++)
        g_r_state_globals->texturetranslation[i] = i;

    GenerateTextureHashTable();
}


//
// R_InitFlats
//
void R_InitFlats()
{
    int i;

    g_r_state_globals->firstflat = W_GetNumForName(DEH_String("F_START")) + 1;
    lastflat  = W_GetNumForName(DEH_String("F_END")) - 1;
    numflats  = lastflat - g_r_state_globals->firstflat + 1;

    // Create translation table for global animation.
    g_r_state_globals->flattranslation = zmalloc<decltype(g_r_state_globals->flattranslation)>((static_cast<unsigned long>(numflats + 1)) * sizeof(*g_r_state_globals->flattranslation), PU_STATIC, 0);

    for (i = 0; i < numflats; i++)
        g_r_state_globals->flattranslation[i] = i;
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps()
{
    patch_t *patch;

    g_r_state_globals->firstspritelump = W_GetNumForName(DEH_String("S_START")) + 1;
    g_r_state_globals->lastspritelump  = W_GetNumForName(DEH_String("S_END")) - 1;

    g_r_state_globals->numspritelumps  = g_r_state_globals->lastspritelump - g_r_state_globals->firstspritelump + 1;
    g_r_state_globals->spritewidth     = zmalloc<decltype(g_r_state_globals->spritewidth)>(static_cast<unsigned long>(g_r_state_globals->numspritelumps) * sizeof(*g_r_state_globals->spritewidth), PU_STATIC, 0);
    g_r_state_globals->spriteoffset    = zmalloc<decltype(g_r_state_globals->spriteoffset)>(static_cast<unsigned long>(g_r_state_globals->numspritelumps) * sizeof(*g_r_state_globals->spriteoffset), PU_STATIC, 0);
    g_r_state_globals->spritetopoffset = zmalloc<decltype(g_r_state_globals->spritetopoffset)>(static_cast<unsigned long>(g_r_state_globals->numspritelumps) * sizeof(*g_r_state_globals->spritetopoffset), PU_STATIC, 0);

    for (int i = 0; i < g_r_state_globals->numspritelumps; i++)
    {
        if (!(i & 63))
            printf(".");

        patch              = cache_lump_num<patch_t *>(g_r_state_globals->firstspritelump + i, PU_CACHE);
        g_r_state_globals->spritewidth[i]     = SHORT(patch->width) << FRACBITS;
        g_r_state_globals->spriteoffset[i]    = SHORT(patch->leftoffset) << FRACBITS;
        g_r_state_globals->spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;
    }
}

#ifndef CRISPY_TRUECOLOR
// [crispy] initialize translucency filter map
// based in parts on the implementation from boom202s/R_DATA.C:676-787

enum class rgb_t
{
    r,
    g,
    b
};

static const int tran_filter_pct = 66;

static void R_InitTranMap()
{
    int lump = W_CheckNumForName("TRANMAP");

    // If a tranlucency filter map lump is present, use it
    if (lump != -1)
    {
        // Set a pointer to the translucency filter maps.
        tranmap = cache_lump_num<uint8_t *>(lump, PU_STATIC);
        // [crispy] loaded from a lump
        printf(":");
    }
    else
    {
        // Compose a default transparent filter map based on PLAYPAL.
        unsigned char *playpal = cache_lump_name<unsigned char *>("PLAYPAL", PU_STATIC);
        FILE *         cachefp;
        char *         fname = nullptr;
        extern char *  configdir;

        struct {
            unsigned char pct;
            unsigned char playpal[256 * 3]; // [crispy] a palette has 768 bytes!
        } cache;

        tranmap = zmalloc<decltype(tranmap)>(256 * 256, PU_STATIC, 0);
        fname   = M_StringJoin(configdir, "tranmap.dat", nullptr);

        // [crispy] open file readable
        if ((cachefp = fopen(fname, "rb")) &&
            // [crispy] could read struct cache from file
            fread(&cache, 1, sizeof(cache), cachefp) == sizeof(cache) &&
            // [crispy] same filter percents
            cache.pct == tran_filter_pct &&
            // [crispy] same base palettes
            memcmp(cache.playpal, playpal, sizeof(cache.playpal)) == 0 &&
            // [crispy] could read entire translucency map
            fread(tranmap, 256, 256, cachefp) == 256)
        {
            // [crispy] loaded from a file
            printf(".");
        }
        // [crispy] file not readable
        else
        {
            uint8_t *fg;
            uint8_t *bg;
            uint8_t  blend[3];
            uint8_t *tp = tranmap;
            int   i, j, btmp;

            I_SetPalette(playpal);
            // [crispy] background color
            for (i = 0; i < 256; i++)
            {
                // [crispy] foreground color
                for (j = 0; j < 256; j++)
                {
                    // [crispy] shortcut: identical foreground and background
                    if (i == j)
                    {
                        *tp++ = static_cast<uint8_t>(i);
                        continue;
                    }

                    bg = playpal + 3 * i;
                    fg = playpal + 3 * j;

                    // [crispy] blended color - emphasize blues
                    // Colour matching in RGB space doesn't work very well with the blues
                    // in Doom's palette. Rather than do any colour conversions, just
                    // emphasize the blues when building the translucency table.
                    int r = static_cast<int>(rgb_t::r);
                    int g = static_cast<int>(rgb_t::g);
                    int b = static_cast<int>(rgb_t::b);
                    btmp     = fg[b] * 1.666 < (fg[r] + fg[g]) ? 0 : 50;
                    blend[r] = static_cast<uint8_t>((tran_filter_pct * fg[r] + (100 - tran_filter_pct) * bg[r]) / (100 + btmp));
                    blend[g] = static_cast<uint8_t>((tran_filter_pct * fg[g] + (100 - tran_filter_pct) * bg[g]) / (100 + btmp));
                    blend[b] = static_cast<uint8_t>((tran_filter_pct * fg[b] + (100 - tran_filter_pct) * bg[b]) / 100);

                    *tp++ = static_cast<uint8_t>(I_GetPaletteIndex(blend[r], blend[g], blend[b]));
                }
            }

            // [crispy] file not readable, open writable
            if ((cachefp = fopen(fname, "wb")))
            {
                // [crispy] set filter percents
                cache.pct = tran_filter_pct;
                // [crispy] set base palette
                std::memcpy(cache.playpal, playpal, sizeof(cache.playpal));
                // [crispy] go to start of file
                fseek(cachefp, 0, SEEK_SET);
                // [crispy] write struct cache
                fwrite(&cache, 1, sizeof(cache), cachefp);
                // [crispy] write translucency map
                fwrite(tranmap, 256, 256, cachefp);

                // [crispy] generated and saved
                printf("!");
            }
            else
            {
                // [crispy] generated, but not saved
                printf("?");
            }
        }

        if (cachefp)
            fclose(cachefp);

        free(fname);

        W_ReleaseLumpName("PLAYPAL");
    }
}
#endif

//
// R_InitColormaps
//
void R_InitColormaps()
{
#ifndef CRISPY_TRUECOLOR
    int lump;

    // Load in the light tables,
    //  256 byte align tables.
    lump      = W_GetNumForName(DEH_String("COLORMAP"));
    g_r_state_globals->colormaps = cache_lump_num<lighttable_t *>(lump, PU_STATIC);
#else
    byte *playpal;
    int c, i, j = 0;
    byte r, g, b;
    extern byte **gamma2table;

    // [crispy] intermediate gamma levels
    if (!gamma2table)
    {
        extern void I_SetGammaTable();
        I_SetGammaTable();
    }

    playpal = cache_lump_name<patch_t *>("PLAYPAL", PU_STATIC);

    if (!g_r_state_globals->colormaps)
    {
        g_r_state_globals->colormaps = zmalloc<lighttable_t *>((NUMCOLORMAPS + 1) * 256 * sizeof(lighttable_t), PU_STATIC, 0);
    }

    if (crispy->truecolor)
    {
        for (c = 0; c < NUMCOLORMAPS; c++)
        {
            const float scale = 1. * c / NUMCOLORMAPS;

            for (i = 0; i < 256; i++)
            {
                r = gamma2table[usegamma][playpal[3 * i + 0]] * (1. - scale) + gamma2table[usegamma][0] * scale;
                g = gamma2table[usegamma][playpal[3 * i + 1]] * (1. - scale) + gamma2table[usegamma][0] * scale;
                b = gamma2table[usegamma][playpal[3 * i + 2]] * (1. - scale) + gamma2table[usegamma][0] * scale;

                g_r_state_globals->colormaps[j++] = 0xff000000 | (r << 16) | (g << 8) | b;
            }
        }

        // [crispy] Invulnerability (c == COLORMAPS)
        for (i = 0; i < 256; i++)
        {
            const byte gray = 0xff - (byte)(0.299 * playpal[3 * i + 0] + 0.587 * playpal[3 * i + 1] + 0.114 * playpal[3 * i + 2]);
            r = g = b = gamma2table[usegamma][gray];

            g_r_state_globals->colormaps[j++] = 0xff000000 | (r << 16) | (g << 8) | b;
        }
    }
    else
    {
        byte *const colormap = cache_lump_name<patch_t *>("COLORMAP", PU_STATIC);

        for (c = 0; c <= NUMCOLORMAPS; c++)
        {
            for (i = 0; i < 256; i++)
            {
                r = gamma2table[usegamma][playpal[3 * colormap[c * 256 + i] + 0]] & ~3;
                g = gamma2table[usegamma][playpal[3 * colormap[c * 256 + i] + 1]] & ~3;
                b = gamma2table[usegamma][playpal[3 * colormap[c * 256 + i] + 2]] & ~3;

                g_r_state_globals->colormaps[j++] = 0xff000000 | (r << 16) | (g << 8) | b;
            }
        }

        W_ReleaseLumpName("COLORMAP");
    }
#endif

    // [crispy] initialize color translation and color strings tables
    {
        uint8_t       *playpal = cache_lump_name<uint8_t *>("PLAYPAL", PU_STATIC);
        char        c[3];
        int         i, j;
        bool     keepgray = false;
        extern uint8_t V_Colorize(uint8_t * playpal, int cr, uint8_t source, bool keepgray109);

        if (!crstr)
            crstr = static_cast<decltype(crstr)>(I_Realloc(nullptr, static_cast<int>(cr_t::CRMAX) * sizeof(*crstr)));

        // [crispy] check for status bar graphics replacements
        i        = W_CheckNumForName(DEH_String("sttnum0")); // [crispy] Status Bar '0'
        keepgray = (i >= 0 && W_IsIWADLump(lumpinfo[i]));

        // [crispy] CRMAX - 2: don't override the original GREN and BLUE2 Boom tables
        for (i = 0; i < static_cast<int>(cr_t::CRMAX) - 2; i++)
        {
            for (j = 0; j < 256; j++)
            {
                cr_colors[i][j] = V_Colorize(playpal, i, static_cast<uint8_t>(j), keepgray);
            }

            M_snprintf(c, sizeof(c), "%c%c", cr_esc, '0' + i);
            crstr[i] = M_StringDuplicate(c);
        }

        W_ReleaseLumpName("PLAYPAL");
    }
}


//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData()
{
    // [crispy] Moved R_InitFlats() to the top, because it sets firstflat/lastflat
    // which are required by R_InitTextures() to prevent flat lumps from being
    // mistaken as patches and by R_InitBrightmaps() to set brightmaps for flats.
    // R_InitBrightmaps() comes next, because it sets R_BrightmapForTexName()
    // to initialize brightmaps depending on gameversion in R_InitTextures().
    R_InitFlats();
    R_InitBrightmaps();
    R_InitTextures();
    printf(".");
    //  R_InitFlats (); [crispy] moved ...
    printf(".");
    R_InitSpriteLumps();
    printf(".");
    R_InitColormaps();
#ifndef CRISPY_TRUECOLOR
    R_InitTranMap(); // [crispy] prints a mark itself
#endif
}


//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName(const char *name)
{
    int  i;
    char namet[9];

    i = W_CheckNumForNameFromTo(name, lastflat, g_r_state_globals->firstflat);

    if (i == -1)
    {
        namet[8] = 0;
        std::memcpy(namet, name, 8);
        // [crispy] make non-fatal
        fprintf(stderr, "R_FlatNumForName: %s not found\n", namet);
        // [crispy] since there is no "No Flat" marker,
        // render missing flats as SKY
        return skyflatnum;
    }
    return i - g_r_state_globals->firstflat;
}


//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName(const char *name)
{
    texture_t *texture;
    int        key;

    // "NoTexture" marker.
    if (name[0] == '-')
        return 0;

    key = static_cast<int>(W_LumpNameHash(name) % static_cast<unsigned int>(numtextures));

    texture = textures_hashtable[key];

    while (texture != nullptr)
    {
        if (!strncasecmp(texture->name, name, 8))
            return texture->index;

        texture = texture->next;
    }

    return -1;
}


//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int R_TextureNumForName(const char *name)
{
    int i;

    i = R_CheckTextureNumForName(name);

    if (i == -1)
    {
        // [crispy] fix absurd texture name in error message
        char namet[9];
        namet[8] = '\0';
        std::memcpy(namet, name, 8);
        // [crispy] make non-fatal
        fprintf(stderr, "R_TextureNumForName: %s not found\n",
            namet);
        return 0;
    }
    return i;
}


//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int flatmemory;
int texturememory;
int spritememory;

void R_PrecacheLevel()
{
    char *flatpresent;
    char *texturepresent;
    char *spritepresent;

    int i;
    int j;
    int k;
    int lump;

    texture_t *    texture;
    thinker_t *    th;
    spriteframe_t *sf;

    if (demoplayback)
        return;

    // Precache flats.
    flatpresent = zmalloc<decltype(flatpresent)>(static_cast<size_t>(numflats), PU_STATIC, nullptr);
    std::memset(flatpresent, 0, static_cast<size_t>(numflats));

    for (i = 0; i < g_r_state_globals->numsectors; i++)
    {
        flatpresent[g_r_state_globals->sectors[i].floorpic]   = 1;
        flatpresent[g_r_state_globals->sectors[i].ceilingpic] = 1;
    }

    flatmemory = 0;

    for (i = 0; i < numflats; i++)
    {
        if (flatpresent[i])
        {
            lump = g_r_state_globals->firstflat + i;
            flatmemory += static_cast<int>(lumpinfo[lump]->size);
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(flatpresent);

    // Precache textures.
    texturepresent = zmalloc<decltype(texturepresent)>(static_cast<size_t>(numtextures), PU_STATIC, nullptr);
    std::memset(texturepresent, 0, static_cast<size_t>(numtextures));

    for (i = 0; i < g_r_state_globals->numsides; i++)
    {
        texturepresent[g_r_state_globals->sides[i].toptexture]    = 1;
        texturepresent[g_r_state_globals->sides[i].midtexture]    = 1;
        texturepresent[g_r_state_globals->sides[i].bottomtexture] = 1;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.
    texturepresent[skytexture] = 1;

    texturememory = 0;
    for (i = 0; i < numtextures; i++)
    {
        if (!texturepresent[i])
            continue;

        // [crispy] precache composite textures
        R_GenerateComposite(i);

        texture = textures[i];

        for (j = 0; j < texture->patchcount; j++)
        {
            lump = texture->patches[j].patch;
            texturememory += static_cast<int>(lumpinfo[lump]->size);
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(texturepresent);

    // Precache sprites.
    spritepresent = zmalloc<decltype(spritepresent)>(static_cast<size_t>(g_r_state_globals->numsprites), PU_STATIC, nullptr);
    std::memset(spritepresent, 0, static_cast<size_t>(g_r_state_globals->numsprites));

    action_hook needle = P_MobjThinker;
    for (th = g_p_local_globals->thinkercap.next; th != &g_p_local_globals->thinkercap; th = th->next)
    {
        if (th->function == needle)
        {
            mobj_t *pMobj                = reinterpret_cast<mobj_t *>(th);
            spritepresent[pMobj->sprite] = 1;
        }
    }

    spritememory = 0;
    for (i = 0; i < g_r_state_globals->numsprites; i++)
    {
        if (!spritepresent[i])
            continue;

        for (j = 0; j < g_r_state_globals->sprites[i].numframes; j++)
        {
            sf = &g_r_state_globals->sprites[i].spriteframes[j];
            for (k = 0; k < 8; k++)
            {
                lump = g_r_state_globals->firstspritelump + sf->lump[k];
                spritememory += static_cast<int>(lumpinfo[lump]->size);
                W_CacheLumpNum(lump, PU_CACHE);
            }
        }
    }

    Z_Free(spritepresent);
}

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


#include "h2def.hpp"
#include "i_system.hpp"
#include "i_swap.hpp"
#include "m_misc.hpp"
#include "r_local.hpp"
#include "p_local.hpp"
#include "lump.hpp"
#include "memory.hpp"

typedef struct
{
    int originx;                // block origin (allways UL), which has allready
    int originy;                // accounted  for the patch's internal origin
    int patch;
} texpatch_t;

// a maptexturedef_t describes a rectangular texture, which is composed of one
// or more mappatch_t structures that arrange graphic patches
typedef struct
{
    char name[8];               // for switch changing, etc
    short width;
    short height;
    short patchcount;
    texpatch_t patches[1];      // [patchcount] drawn back to front
    //  into the cached texture
} texture_t;



int firstflat, lastflat, numflats;
int firstpatch, lastpatch, numpatches;
int firstspritelump, lastspritelump, numspritelumps;

int numtextures;
texture_t **textures;
int *texturewidthmask;
fixed_t *textureheight;         // needed for texture pegging
int *texturecompositesize;
short **texturecolumnlump;
unsigned short **texturecolumnofs;
uint8_t        **texturecomposite;

int *flattranslation;           // for global animation
int *texturetranslation;        // for global animation

fixed_t *spritewidth;           // needed for pre rendering
fixed_t *spriteoffset;
fixed_t *spritetopoffset;

lighttable_t *colormaps;


/*
==============================================================================

						MAPTEXTURE_T CACHING

when a texture is first needed, it counts the number of composite columns
required in the texture and allocates space for a column directory and any
new columns.  The directory will simply point inside other patches if there
is only one patch in a given column, but any columns with multiple patches
will have new column_ts generated.

==============================================================================
*/

/*
===================
=
= R_DrawColumnInCache
=
= Clip and draw a column from a patch into a cached post
=
===================
*/

void R_DrawColumnInCache(column_t * patch, uint8_t *cache, int originy,
                         int cacheheight)
{
    int count, position;
    uint8_t *source;

    while (patch->topdelta != 0xff)
    {
        source = reinterpret_cast<uint8_t *>(patch) + 3;
        count = patch->length;
        position = originy + patch->topdelta;
        if (position < 0)
        {
            count += position;
            position = 0;
        }
        if (position + count > cacheheight)
            count = cacheheight - position;
        if (count > 0)
            memcpy(cache + position, source, count);

        patch = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(patch) + patch->length + 4);
    }
}


/*
===================
=
= R_GenerateComposite
=
===================
*/

void R_GenerateComposite(int texnum)
{
    uint8_t        *block;
    texture_t *texture;
    texpatch_t *patch;
    patch_t *realpatch;
    int x, x1, x2;
    int i;
    column_t *patchcol;
    short *collump;
    unsigned short *colofs;

    texture = textures[texnum];
    block = zmalloc<uint8_t *>(texturecompositesize[texnum], PU_STATIC,
          &texturecomposite[texnum]);
    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];

//
// composite the columns together
//
    patch = texture->patches;

    for (i = 0, patch = texture->patches; i < texture->patchcount;
         i++, patch++)
    {
        realpatch = cache_lump_num<patch_t *>(patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);

        if (x1 < 0)
            x = 0;
        else
            x = x1;
        if (x2 > texture->width)
            x2 = texture->width;

        for (; x < x2; x++)
        {
            if (collump[x] >= 0)
                continue;       // column does not have multiple patches
            patchcol = reinterpret_cast<column_t *>(reinterpret_cast<uint8_t *>(realpatch) +
                                     LONG(realpatch->columnofs[x - x1]));
            R_DrawColumnInCache(patchcol, block + colofs[x], patch->originy,
                                texture->height);
        }

    }

// now that the texture has been built, it is purgable
    Z_ChangeTag(block, PU_CACHE);
}


/*
===================
=
= R_GenerateLookup
=
===================
*/

void R_GenerateLookup(int texnum)
{
    texture_t *texture;
    uint8_t        *patchcount; // [texture->width]
    texpatch_t *patch;
    patch_t *realpatch;
    int x, x1, x2;
    int i;
    short *collump;
    unsigned short *colofs;

    texture = textures[texnum];

    texturecomposite[texnum] = 0;       // composited not created yet
    texturecompositesize[texnum] = 0;
    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];

//
// count the number of columns that are covered by more than one patch
// fill in the lump / offset, so columns with only a single patch are
// all done
//
    patchcount = zmalloc<uint8_t *>(texture->width, PU_STATIC, &patchcount);
    memset(patchcount, 0, texture->width);
    patch = texture->patches;

    for (i = 0, patch = texture->patches; i < texture->patchcount;
         i++, patch++)
    {
        realpatch = cache_lump_num<patch_t *>(patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);
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
            colofs[x] = static_cast<unsigned short>(LONG(realpatch->columnofs[x - x1]) + 3);
        }
    }

    for (x = 0; x < texture->width; x++)
    {
        if (!patchcount[x])
        {
            ST_Message("R_GenerateLookup: column without a patch (%s)\n",
                       texture->name);
            return;
        }
//                      I_Error ("R_GenerateLookup: column without a patch");
        if (patchcount[x] > 1)
        {
            collump[x] = -1;    // use the cached block
            colofs[x] = static_cast<unsigned short>(texturecompositesize[texnum]);
            if (texturecompositesize[texnum] > 0x10000 - texture->height)
                I_Error("R_GenerateLookup: texture %i is >64k", texnum);
            texturecompositesize[texnum] += texture->height;
        }
    }

    Z_Free(patchcount);
}


/*
================
=
= R_GetColumn
=
================
*/

uint8_t *R_GetColumn(int tex, int col)
{
    int lump, ofs;

    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs = texturecolumnofs[tex][col];
    if (lump > 0)
        return cache_lump_num<uint8_t *>(lump, PU_CACHE) + ofs;
    if (!texturecomposite[tex])
        R_GenerateComposite(tex);
    return texturecomposite[tex] + ofs;
}


/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

void R_InitTextures()
{
    maptexture_t *mtexture;
    texture_t *texture;
    mappatch_t *mpatch;
    texpatch_t *patch;
    int i, j;
    int *maptex, *maptex2, *maptex1;
    char name[9], *names, *name_p;
    int *patchlookup;
    int totalwidth;
    int nummappatches;
    int offset, maxoff, maxoff2;
    int numtextures1, numtextures2;
    int *directory;

//
// load the patch names from pnames.lmp
//
    names = cache_lump_name<char *>("PNAMES", PU_STATIC);
    nummappatches = LONG(*(reinterpret_cast<int *>(names)));
    name_p = names + 4;
    patchlookup = zmalloc<int *>(nummappatches * sizeof(*patchlookup), PU_STATIC, nullptr);
    for (i = 0; i < nummappatches; i++)
    {
        M_StringCopy(name, name_p + i * 8, sizeof(name));
        patchlookup[i] = W_CheckNumForName(name);
    }
    W_ReleaseLumpName("PNAMES");

//
// load the map texture definitions from textures.lmp
//
    maptex = maptex1 = cache_lump_name<int *>("TEXTURE1", PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = static_cast<int>(W_LumpLength(W_GetNumForName("TEXTURE1")));
    directory = maptex + 1;

    if (W_CheckNumForName("TEXTURE2") != -1)
    {
        maptex2 = cache_lump_name<int *>("TEXTURE2", PU_STATIC);
        numtextures2 = LONG(*maptex2);
        maxoff2 = static_cast<int>(W_LumpLength(W_GetNumForName("TEXTURE2")));
    }
    else
    {
        maptex2 = nullptr;
        numtextures2 = 0;
        maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    textures = zmalloc<texture_t **>(numtextures * sizeof(texture_t *), PU_STATIC, 0);
    texturecolumnlump = zmalloc<short **>(numtextures * sizeof(short *), PU_STATIC, 0);
    texturecolumnofs = zmalloc<unsigned short **>(numtextures * sizeof(short *), PU_STATIC, 0);
    texturecomposite = zmalloc<uint8_t **>(numtextures * sizeof(uint8_t *), PU_STATIC, 0);
    texturecompositesize = zmalloc<int *>(numtextures * sizeof(int), PU_STATIC, 0);
    texturewidthmask = zmalloc<int *>(numtextures * sizeof(int), PU_STATIC, 0);
    textureheight = zmalloc<fixed_t *>(numtextures * sizeof(fixed_t), PU_STATIC, 0);

    totalwidth = 0;

    for (i = 0; i < numtextures; i++, directory++)
    {
        if (i == numtextures1)
        {                       // start looking in second texture file
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex + 1;
        }

        offset = LONG(*directory);
        if (offset > maxoff)
            I_Error("R_InitTextures: bad texture directory");
        mtexture = reinterpret_cast<maptexture_t *>(reinterpret_cast<uint8_t *>(maptex) + offset);
        texture = textures[i] = zmalloc<texture_t *>(sizeof(texture_t)
                                                                      + sizeof(texpatch_t) * (SHORT(mtexture->patchcount) - 1),
            PU_STATIC, 0);
        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);
        memcpy(texture->name, mtexture->name, sizeof(texture->name));
        mpatch = &mtexture->patches[0];
        patch = &texture->patches[0];
        for (j = 0; j < texture->patchcount; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            patch->patch = patchlookup[SHORT(mpatch->patch)];
            if (patch->patch == -1)
                I_Error("R_InitTextures: Missing patch in texture %s",
                        texture->name);
        }
        texturecolumnlump[i] = zmalloc<short *>(texture->width * sizeof(short), PU_STATIC, 0);
        texturecolumnofs[i] = zmalloc<unsigned short *>(texture->width * sizeof(short), PU_STATIC, 0);
        j = 1;
        while (j * 2 <= texture->width)
            j <<= 1;
        texturewidthmask[i] = j - 1;
        textureheight[i] = texture->height << FRACBITS;

        totalwidth += texture->width;
    }

    Z_Free(patchlookup);

    W_ReleaseLumpName("TEXTURE1");
    if (maptex2)
        W_ReleaseLumpName("TEXTURE2");

//
// precalculate whatever possible
//              
    for (i = 0; i < numtextures; i++)
    {
        R_GenerateLookup(i);
        if (!(i & 31))
            ST_Progress();
    }

//
// translation table for global animation
//
    texturetranslation = zmalloc<int *>((numtextures + 1) * sizeof(int), PU_STATIC, 0);
    for (i = 0; i < numtextures; i++)
        texturetranslation[i] = i;
}


/*
================
=
= R_InitFlats
=
=================
*/

void R_InitFlats()
{
    int i;

    firstflat = W_GetNumForName("F_START") + 1;
    lastflat = W_GetNumForName("F_END") - 1;
    numflats = lastflat - firstflat + 1;

// translation table for global animation
    flattranslation = zmalloc<int *>((numflats + 1) * sizeof(int), PU_STATIC, 0);
    for (i = 0; i < numflats; i++)
        flattranslation[i] = i;
}


/*
================
=
= R_InitSpriteLumps
=
= Finds the width and hoffset of all sprites in the wad, so the sprite doesn't
= need to be cached just for the header during rendering
=================
*/

void R_InitSpriteLumps()
{
    int i;
    patch_t *patch;

    firstspritelump = W_GetNumForName("S_START") + 1;
    lastspritelump = W_GetNumForName("S_END") - 1;
    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = zmalloc<fixed_t *>(numspritelumps * sizeof(fixed_t), PU_STATIC, 0);
    spriteoffset = zmalloc<fixed_t *>(numspritelumps * sizeof(fixed_t), PU_STATIC, 0);
    spritetopoffset = zmalloc<fixed_t *>(numspritelumps * sizeof(fixed_t), PU_STATIC, 0);

    for (i = 0; i < numspritelumps; i++)
    {
        if (!(i & 127))
            ST_Progress();
        patch = cache_lump_num<patch_t *>(firstspritelump + i, PU_CACHE);
        spritewidth[i] = SHORT(patch->width) << FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset) << FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset) << FRACBITS;
    }
}


/*
================
=
= R_InitColormaps
=
=================
*/

void R_InitColormaps()
{
    int lump, length;
//
// load in the light tables
// 256 byte align tables
//
    lump = W_GetNumForName("COLORMAP");
    length = static_cast<int>(W_LumpLength(lump));
    colormaps = zmalloc<lighttable_t *>(length, PU_STATIC, 0);
    W_ReadLump(lump, colormaps);
}


/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData()
{
    R_InitTextures();
    R_InitFlats();
    R_InitSpriteLumps();
    R_InitColormaps();
}

//=============================================================================

/*
================
=
= R_FlatNumForName
=
================
*/

int R_FlatNumForName(const char *name)
{
    int i;
    char namet[9];

    i = W_CheckNumForName(name);
    if (i == -1)
    {
        namet[8] = 0;
        memcpy(namet, name, 8);
        I_Error("R_FlatNumForName: %s not found", namet);
    }
    return i - firstflat;
}


/*
================
=
= R_CheckTextureNumForName
=
================
*/

int R_CheckTextureNumForName(const char *name)
{
    int i;

    if (name[0] == '-')         // no texture marker
        return 0;

    for (i = 0; i < numtextures; i++)
        if (!strncasecmp(textures[i]->name, name, 8))
            return i;

    return -1;
}


/*
================
=
= R_TextureNumForName
=
================
*/

int R_TextureNumForName(const char *name)
{
    int i;
    //char  namet[9];

    i = R_CheckTextureNumForName(name);
    if (i == -1)
        I_Error("R_TextureNumForName: %s not found", name);

    return i;
}


/*
=================
=
= R_PrecacheLevel
=
= Preloads all relevent graphics for the level
=================
*/

int flatmemory, texturememory, spritememory;

void R_PrecacheLevel()
{
    char *flatpresent;
    char *texturepresent;
    char *spritepresent;
    int i, j, k, lump;
    texture_t *texture;
    thinker_t *th;
    spriteframe_t *sf;

    if (demoplayback)
        return;

//
// precache flats
//      
    flatpresent = zmalloc<char *>(numflats, PU_STATIC, nullptr);
    memset(flatpresent, 0, numflats);
    for (i = 0; i < numsectors; i++)
    {
        flatpresent[sectors[i].floorpic] = 1;
        flatpresent[sectors[i].ceilingpic] = 1;
    }

    flatmemory = 0;
    for (i = 0; i < numflats; i++)
        if (flatpresent[i])
        {
            lump = firstflat + i;
            flatmemory += static_cast<int>(lumpinfo[lump]->size);
            W_CacheLumpNum(lump, PU_CACHE);
        }

    Z_Free(flatpresent);

//
// precache textures
//
    texturepresent = zmalloc<char *>(numtextures, PU_STATIC, nullptr);
    memset(texturepresent, 0, numtextures);

    for (i = 0; i < numsides; i++)
    {
        texturepresent[sides[i].toptexture] = 1;
        texturepresent[sides[i].midtexture] = 1;
        texturepresent[sides[i].bottomtexture] = 1;
    }

    texturepresent[Sky1Texture] = 1;
    texturepresent[Sky2Texture] = 1;

    texturememory = 0;
    for (i = 0; i < numtextures; i++)
    {
        if (!texturepresent[i])
            continue;
        texture = textures[i];
        for (j = 0; j < texture->patchcount; j++)
        {
            lump = texture->patches[j].patch;
            texturememory += static_cast<int>(lumpinfo[lump]->size);
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(texturepresent);

//
// precache sprites
//
    spritepresent = zmalloc<char *>(numsprites, PU_STATIC, nullptr);
    memset(spritepresent, 0, numsprites);
    action_hook needle = P_MobjThinker;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function == needle)
            spritepresent[(reinterpret_cast<mobj_t *>(th))->sprite] = 1;
    }

    spritememory = 0;
    for (i = 0; i < numsprites; i++)
    {
        if (!spritepresent[i])
            continue;
        for (j = 0; j < sprites[i].numframes; j++)
        {
            sf = &sprites[i].spriteframes[j];
            for (k = 0; k < 8; k++)
            {
                lump = firstspritelump + sf->lump[k];
                spritememory += static_cast<int>(lumpinfo[lump]->size);
                W_CacheLumpNum(lump, PU_CACHE);
            }
        }
    }

    Z_Free(spritepresent);
}

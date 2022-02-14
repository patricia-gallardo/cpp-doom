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

// P_main.c

#include <cstdlib>

#include "doomdef.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_bbox.hpp"
#include "p_local.hpp"
#include "s_sound.hpp"
#include "lump.hpp"
#include "memory.hpp"

void P_SpawnMapThing(mapthing_t * mthing);

int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int numsides;
side_t *sides;

int32_t *blockmap;	            // offsets in blockmap are from here // [crispy] BLOCKMAP limit
int32_t *blockmaplump;          // [crispy] BLOCKMAP limit
int bmapwidth, bmapheight;      // in mapblocks
fixed_t bmaporgx, bmaporgy;     // origin of block map
mobj_t **blocklinks;            // for thing chains

uint8_t *rejectmatrix; // for fast sight rejection

mapthing_t deathmatchstarts[10], *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];
bool playerstartsingame[MAXPLAYERS];

/*
=================
=
= P_LoadVertexes
=
=================
*/

void P_LoadVertexes(int lump)
{
    uint8_t     *data;
    int i;
    mapvertex_t *ml;
    vertex_t *li;

    numvertexes = static_cast<int>(W_LumpLength(lump) / sizeof(mapvertex_t));
    vertexes = zmalloc<vertex_t *>(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    ml = reinterpret_cast<mapvertex_t *>(data);
    li = vertexes;
    for (i = 0; i < numvertexes; i++, li++, ml++)
    {
        li->x = SHORT(ml->x) << FRACBITS;
        li->y = SHORT(ml->y) << FRACBITS;

        // [crispy] initialize pseudovertexes with actual vertex coordinates
        li->r_x = li->x;
        li->r_y = li->y;
        li->moved = false;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs(int lump)
{
    uint8_t  *data;
    int i;
    mapseg_t *ml;
    seg_t *li;
    line_t *ldef;
    int linedef_local, side;

    numsegs = static_cast<int>(W_LumpLength(lump) / sizeof(mapseg_t));
    segs = zmalloc<seg_t *>(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    std::memset(segs, 0, numsegs * sizeof(seg_t));
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    ml = reinterpret_cast<mapseg_t *>(data);
    li = segs;
    for (i = 0; i < numsegs; i++, li++, ml++)
    {
        li->v1 = &vertexes[SHORT(ml->v1)];
        li->v2 = &vertexes[SHORT(ml->v2)];

        li->angle = (SHORT(ml->angle)) << 16;
        li->offset = (SHORT(ml->offset)) << 16;
        linedef_local = SHORT(ml->linedef);
        ldef = &lines[linedef_local];
        li->linedef = ldef;
        side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        if (ldef->flags & ML_TWOSIDED)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
            li->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSubsectors
=
=================
*/

void P_LoadSubsectors(int lump)
{
    uint8_t        *data;
    int i;
    mapsubsector_t *ms;
    subsector_t *ss;

    numsubsectors = static_cast<int>(W_LumpLength(lump) / sizeof(mapsubsector_t));
    subsectors = zmalloc<subsector_t *>(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    ms = reinterpret_cast<mapsubsector_t *>(data);
    std::memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++, ms++)
    {
        ss->numlines = SHORT(ms->numsegs);
        ss->firstline = SHORT(ms->firstseg);
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors(int lump)
{
    uint8_t     *data;
    int i;
    mapsector_t *ms;
    sector_t *ss;

    numsectors = static_cast<int>(W_LumpLength(lump) / sizeof(mapsector_t));
    sectors = zmalloc<sector_t *>(numsectors * sizeof(sector_t), PU_LEVEL, 0);
    std::memset(sectors, 0, numsectors * sizeof(sector_t));
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    ms = reinterpret_cast<mapsector_t *>(data);
    ss = sectors;
    for (i = 0; i < numsectors; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = static_cast<short>(R_FlatNumForName(ms->floorpic));
        ss->ceilingpic = static_cast<short>(R_FlatNumForName(ms->ceilingpic));
        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);
        ss->thinglist = nullptr;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes(int lump)
{
    uint8_t   *data;
    int i, j, k;
    mapnode_t *mn;
    node_t *no;

    numnodes = static_cast<int>(W_LumpLength(lump) / sizeof(mapnode_t));
    nodes = zmalloc<node_t *>(numnodes * sizeof(node_t), PU_LEVEL, 0);
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    mn = reinterpret_cast<mapnode_t *>(data);
    no = nodes;
    for (i = 0; i < numnodes; i++, no++, mn++)
    {
        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;
        for (j = 0; j < 2; j++)
        {
            no->children[j] = SHORT(mn->children[j]);
            for (k = 0; k < 4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadThings
=
=================
*/

void P_LoadThings(int lump)
{
    uint8_t    *data;
    int i;
    mapthing_t spawnthing;
    mapthing_t *mt;
    int numthings;

    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);
    numthings = static_cast<int>(W_LumpLength(lump) / sizeof(mapthing_t));

    mt = reinterpret_cast<mapthing_t *>(data);
    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);
        P_SpawnMapThing(&spawnthing);
    }

    if (!deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i] && !playerstartsingame[i])
            {
                I_Error("P_LoadThings: Player %d start missing (vanilla crashes here)", i + 1);
            }
            playerstartsingame[i] = false;
        }
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadLineDefs
=
= Also counts secret lines for intermissions
=================
*/

void P_LoadLineDefs(int lump)
{
    uint8_t      *data;
    int i;
    maplinedef_t *mld;
    line_t *ld;
    vertex_t *v1, *v2;

    numlines = static_cast<int>(W_LumpLength(lump) / sizeof(maplinedef_t));
    lines = zmalloc<line_t *>(numlines * sizeof(line_t), PU_LEVEL, 0);
    std::memset(lines, 0, numlines * sizeof(line_t));
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    mld = reinterpret_cast<maplinedef_t *>(data);
    ld = lines;
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
        ld->flags = SHORT(mld->flags);
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;
        if (!ld->dx)
            ld->slopetype = ST_VERTICAL;
        else if (!ld->dy)
            ld->slopetype = ST_HORIZONTAL;
        else
        {
            if (FixedDiv(ld->dy, ld->dx) > 0)
                ld->slopetype = ST_POSITIVE;
            else
                ld->slopetype = ST_NEGATIVE;
        }

        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }
        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }
        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);
        if (ld->sidenum[0] != -1)
            ld->frontsector = sides[ld->sidenum[0]].sector;
        else
            ld->frontsector = 0;
        if (ld->sidenum[1] != -1)
            ld->backsector = sides[ld->sidenum[1]].sector;
        else
            ld->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs(int lump)
{
    uint8_t      *data;
    int i;
    mapsidedef_t *msd;
    side_t *sd;

    numsides = static_cast<int>(W_LumpLength(lump) / sizeof(mapsidedef_t));
    sides = zmalloc<side_t *>(numsides * sizeof(side_t), PU_LEVEL, 0);
    std::memset(sides, 0, numsides * sizeof(side_t));
    data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

    msd = reinterpret_cast<mapsidedef_t *>(data);
    sd = sides;
    for (i = 0; i < numsides; i++, msd++, sd++)
    {
        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;
        sd->toptexture = static_cast<short>(R_TextureNumForName(msd->toptexture));
        sd->bottomtexture = static_cast<short>(R_TextureNumForName(msd->bottomtexture));
        sd->midtexture = static_cast<short>(R_TextureNumForName(msd->midtexture));
        sd->sector = &sectors[SHORT(msd->sector)];
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadBlockMap
=
=================
*/

void P_LoadBlockMap(int lump)
{
    int i;
    int lumplen;
    short *wadblockmaplump;

    lumplen = static_cast<int>(W_LumpLength(lump));

    int count = lumplen / 2; // [crispy] remove BLOCKMAP limit

    // [crispy] remove BLOCKMAP limit
    wadblockmaplump = zmalloc<short *>(lumplen, PU_LEVEL, nullptr);
    W_ReadLump(lump, wadblockmaplump);
    blockmaplump = zmalloc<int32_t *>(sizeof(*blockmaplump) * count, PU_LEVEL, nullptr);
    blockmap = blockmaplump + 4;

    blockmaplump[0] = SHORT(wadblockmaplump[0]);
    blockmaplump[1] = SHORT(wadblockmaplump[1]);
    blockmaplump[2] = static_cast<int32_t>(SHORT(wadblockmaplump[2])) & 0xffff;
    blockmaplump[3] = static_cast<int32_t>(SHORT(wadblockmaplump[3])) & 0xffff;

    // Swap all short integers to native byte ordering:
	
    // count = lumplen / 2; // [crispy] moved up
    for (i=4; i<count; i++)
    {
        short t = SHORT(wadblockmaplump[i]);
        blockmaplump[i] = (t == -1) ? -1l : static_cast<int32_t>(t) & 0xffff;
    }

    Z_Free(wadblockmaplump);

    bmaporgx = blockmaplump[0] << FRACBITS;
    bmaporgy = blockmaplump[1] << FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

// clear out mobj chains
    size_t block_count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = zmalloc<mobj_t **>(block_count, PU_LEVEL, 0);
    std::memset(blocklinks, 0, block_count);
}




/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
=================
*/

void P_GroupLines()
{
    line_t **linebuffer;
    int i, j, total;
    line_t *li;
    sector_t *sector;
    subsector_t *ss;
    seg_t *seg;
    fixed_t bbox[4];
    int block;

// look up sector number for each subsector
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++)
    {
        seg = &segs[ss->firstline];
        ss->sector = seg->sidedef->sector;
    }

// count number of lines in each sector
    li = lines;
    total = 0;
    for (i = 0; i < numlines; i++, li++)
    {
        total++;
        li->frontsector->linecount++;
        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

// build line tables for each sector    
    linebuffer = zmalloc<line_t **>(total * sizeof(line_t *), PU_LEVEL, 0);
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        M_ClearBox(bbox);
        sector->lines = linebuffer;
        li = lines;
        for (j = 0; j < numlines; j++, li++)
        {
            if (li->frontsector == sector || li->backsector == sector)
            {
                *linebuffer++ = li;
                M_AddToBox(bbox, li->v1->x, li->v1->y);
                M_AddToBox(bbox, li->v2->x, li->v2->y);
            }
        }
        if (linebuffer - sector->lines != sector->linecount)
            I_Error("P_GroupLines: miscounted");

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
        sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight - 1 : block;
        sector->blockbox[BOXTOP] = block;

        block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM] = block;

        block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth - 1 : block;
        sector->blockbox[BOXRIGHT] = block;

        block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT] = block;
    }

}

//=============================================================================

// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but separate coordinates that are *only* used in rendering,
// i.e. r_bsp.c:R_AddLine()

static void P_RemoveSlimeTrails()
{
    int i;

    for (i = 0; i < numsegs; i++)
    {
	const line_t *l = segs[i].linedef;
	vertex_t *v = segs[i].v1;

	// [crispy] ignore exactly vertical or horizontal linedefs
	if (l->dx && l->dy)
	{
	    do
	    {
		// [crispy] vertex wasn't already moved
		if (!v->moved)
		{
		    v->moved = true;
		    // [crispy] ignore endpoints of linedefs
		    if (v != l->v1 && v != l->v2)
		    {
			// [crispy] move the vertex towards the linedef
			// by projecting it using the law of cosines
			int64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
			int64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
			int64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
			int64_t s = dx2 + dy2;

			// [crispy] MBF actually overrides v->x and v->y here
			v->r_x = static_cast<fixed_t>((dx2 * v->x + dy2 * l->v1->x + dxy * (v->y - l->v1->y)) / s);
			v->r_y = static_cast<fixed_t>((dy2 * v->y + dx2 * l->v1->y + dxy * (v->x - l->v1->x)) / s);

			// [crispy] wait a minute... moved more than 8 map units?
			// maybe that's a linguortal then, back to the original coordinates
			if (std::abs(v->r_x - v->x) > 8*FRACUNIT || std::abs(v->r_y - v->y) > 8*FRACUNIT)
			{
			    v->r_x = v->x;
			    v->r_y = v->y;
			}
		    }
		}
	    // [crispy] if v doesn't point to the second vertex of the seg already, point it there
	    } while ((v != segs[i].v2) && (v = segs[i].v2));
	}
    }
}

/*
=================
=
= P_SetupLevel
=
=================
*/

void P_SetupLevel(int episode, int map, int, skill_t)
{
    int i;
    int parm;
    char lumpname[9];
    int lumpnum;
    mobj_t *mobj;

    totalkills = totalitems = totalsecret = 0;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].killcount = players[i].secretcount
            = players[i].itemcount = 0;
    }
    players[consoleplayer].viewz = 1;   // will be set by player think

    S_Start();                  // make sure all sounds are stopped before Z_FreeTags

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    P_InitThinkers();

//
// look for a regular (development) map first
//
    lumpname[0] = 'E';
    lumpname[1] = static_cast<char>('0' + episode);
    lumpname[2] = 'M';
    lumpname[3] = static_cast<char>('0' + map);
    lumpname[4] = 0;
    leveltime = 0;

    lumpnum = W_GetNumForName(lumpname);

// note: most of this ordering is important     
    P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);

    rejectmatrix = cache_lump_num<uint8_t *>(lumpnum + ML_REJECT, PU_LEVEL);
    P_GroupLines();

    // [crispy] remove slime trails
    P_RemoveSlimeTrails();

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_InitAmbientSound();
    P_InitMonsters();
    P_OpenWeapons();
    P_LoadThings(lumpnum + ML_THINGS);
    P_CloseWeapons();

//
// if deathmatch, randomly spawn the active players
//
    TimerGame = 0;
    if (deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {                   // must give a player spot before deathmatchspawn
                mobj = P_SpawnMobj(playerstarts[i].x << 16,
                                   playerstarts[i].y << 16, 0, MT_PLAYER);
                players[i].mo = mobj;
                G_DeathMatchSpawnPlayer(i);
                P_RemoveMobj(mobj);
            }
        }

        //!
        // @arg <n>
        // @category net
        // @vanilla
        //
        // For multiplayer games: exit each level after n minutes.
        //

        parm = M_CheckParmWithArgs("-timer", 1);
        if (parm)
        {
            TimerGame = std::atoi(myargv[parm + 1]) * 35 * 60;
        }
    }

// set up world state
    P_SpawnSpecials();

// build subsector connect matrix
//      P_ConnectSubsectors ();

// preload graphics
    if (precache)
        R_PrecacheLevel();

//printf ("free memory: 0x%x\n", Z_FreeMemory());

}


/*
=================
=
= P_Init
=
=================
*/

void P_Init()
{
    P_InitSwitchList();
    P_InitPicAnims();
    P_InitTerrainTypes();
    P_InitLava();
    R_InitSprites(sprnames);
}

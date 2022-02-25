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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//

#include <cmath>

#include <fmt/printf.h>

#include "z_zone.hpp"

#include "deh_main.hpp"
#include "i_swap.hpp"
#include "m_argv.hpp"
#include "m_bbox.hpp"
#include "m_misc.hpp" // [crispy] M_StringJoin()

#include "g_game.hpp"

#include "i_system.hpp"
#include "w_wad.hpp"

#include "doomdef.hpp"
#include "p_local.hpp"

#include "s_musinfo.hpp" // [crispy] S_ParseMusInfo()
#include "s_sound.hpp"

#include "doomstat.hpp"

#include "lump.hpp"
#include "memory.hpp"
#include "p_extnodes.hpp" // [crispy] support extended node formats

void P_SpawnMapThing(mapthing_t * mthing);

static int totallines;

// [crispy] recalculate seg offsets
// adapted from prboom-plus/src/p_setup.c:474-482
fixed_t GetOffset(vertex_t * v1, vertex_t * v2) {
  fixed_t dx = (v1->x - v2->x) >> FRACBITS;
  fixed_t dy = (v1->y - v2->y) >> FRACBITS;
  fixed_t r  = static_cast<fixed_t>(sqrt(dx * dx + dy * dy)) << FRACBITS;

  return r;
}

//
// P_LoadVertexes
//
void P_LoadVertexes(int lump) {
  // Determine number of lumps:
  //  total lump length / vertex record length.
  g_r_state_globals->numvertexes = static_cast<int>(W_LumpLength(lump) / sizeof(mapvertex_t));

  // Allocate zone memory for buffer.
  g_r_state_globals->vertexes = zmalloc<decltype(g_r_state_globals->vertexes)>(static_cast<unsigned long>(g_r_state_globals->numvertexes) * sizeof(vertex_t), PU_LEVEL, 0);

  // Load data into cache.
  uint8_t * data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  mapvertex_t * ml = reinterpret_cast<mapvertex_t *>(data);
  vertex_t *    li = g_r_state_globals->vertexes;

  // Copy and convert vertex coordinates,
  // internal representation as fixed.
  for (int i = 0; i < g_r_state_globals->numvertexes; i++, li++, ml++) {
    li->x = SHORT(ml->x) << FRACBITS;
    li->y = SHORT(ml->y) << FRACBITS;

    // [crispy] initialize vertex coordinates *only* used in rendering
    li->r_x   = li->x;
    li->r_y   = li->y;
    li->moved = false;
  }

  // Free buffer memory.
  W_ReleaseLumpNum(lump);
}

//
// GetSectorAtNullAddress
//
sector_t * GetSectorAtNullAddress() {
  static bool     null_sector_is_initialized = false;
  static sector_t null_sector;

  if (!null_sector_is_initialized) {
    std::memset(&null_sector, 0, sizeof(null_sector));
    I_GetMemoryValue(0, &null_sector.floorheight, 4);
    I_GetMemoryValue(4, &null_sector.ceilingheight, 4);
    null_sector_is_initialized = true;
  }

  return &null_sector;
}

//
// P_LoadSegs
//
void P_LoadSegs(int lump) {
  uint8_t *  data          = nullptr;
  mapseg_t * ml            = nullptr;
  seg_t *    li            = nullptr;
  line_t *   ldef          = nullptr;
  int        linedef_local = 0;
  int        side          = 0;
  int        sidenum       = 0;

  g_r_state_globals->numsegs = static_cast<int>(W_LumpLength(lump) / sizeof(mapseg_t));
  g_r_state_globals->segs    = zmalloc<decltype(g_r_state_globals->segs)>(static_cast<unsigned long>(g_r_state_globals->numsegs) * sizeof(seg_t), PU_LEVEL, 0);
  std::memset(g_r_state_globals->segs, 0, static_cast<unsigned long>(g_r_state_globals->numsegs) * sizeof(seg_t));
  data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  ml = reinterpret_cast<mapseg_t *>(data);
  li = g_r_state_globals->segs;
  for (int i = 0; i < g_r_state_globals->numsegs; i++, li++, ml++) {
    li->v1 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(ml->v1))]; // [crispy] extended nodes
    li->v2 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(ml->v2))]; // [crispy] extended nodes

    li->angle = (SHORT(ml->angle)) << FRACBITS;
    //	li->offset = (SHORT(ml->offset))<<FRACBITS; // [crispy] recalculated below
    linedef_local = static_cast<unsigned short>(SHORT(ml->linedef)); // [crispy] extended nodes
    ldef          = &g_r_state_globals->lines[linedef_local];
    li->linedef   = ldef;
    side          = SHORT(ml->side);

    // e6y: check for wrong indexes
    if (static_cast<unsigned>(ldef->sidenum[side]) >= static_cast<unsigned>(g_r_state_globals->numsides)) {
      I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d", linedef_local, i, static_cast<unsigned>(ldef->sidenum[side]));
    }

    li->sidedef     = &g_r_state_globals->sides[ldef->sidenum[side]];
    li->frontsector = g_r_state_globals->sides[ldef->sidenum[side]].sector;
    // [crispy] recalculate
    li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

    if (ldef->flags & ML_TWOSIDED) {
      sidenum = ldef->sidenum[side ^ 1];

      // If the sidenum is out of range, this may be a "glass hack"
      // impassible window.  Point at side #0 (this may not be
      // the correct Vanilla behavior; however, it seems to work for
      // OTTAWAU.WAD, which is the one place I've seen this trick
      // used).

      if (sidenum < 0 || sidenum >= g_r_state_globals->numsides) {
        // [crispy] linedef has two-sided flag set, but no valid second sidedef;
        // but since it has a midtexture, it is supposed to be rendered just
        // like a regular one-sided linedef
        if (li->sidedef->midtexture) {
          li->backsector = 0;
          fmt::fprintf(stderr, "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n", i);
        } else
          li->backsector = GetSectorAtNullAddress();
      } else {
        li->backsector = g_r_state_globals->sides[sidenum].sector;
      }
    } else {
      li->backsector = 0;
    }
  }

  W_ReleaseLumpNum(lump);
}

// [crispy] fix long wall wobble
void P_SegLengths(bool contrast_only) {
  const int rightangle = std::abs(finesine[(ANG60 / 2) >> ANGLETOFINESHIFT]);

  for (int i = 0; i < g_r_state_globals->numsegs; i++) {
    seg_t * const li = &g_r_state_globals->segs[i];

    int64_t dx = li->v2->r_x - li->v1->r_x;
    int64_t dy = li->v2->r_y - li->v1->r_y;

    if (!contrast_only) {
      li->length = static_cast<uint32_t>(sqrt(static_cast<double>(dx) * static_cast<double>(dx) + static_cast<double>(dy) * static_cast<double>(dy)) / 2);

      // [crispy] re-calculate angle used for rendering
      g_r_state_globals->viewx = li->v1->r_x;
      g_r_state_globals->viewy = li->v1->r_y;
      li->r_angle              = R_PointToAngleCrispy(li->v2->r_x, li->v2->r_y);
    }

    // [crispy] smoother fake contrast
    if (!dy)
      li->fakecontrast = -LIGHTBRIGHT;
    else if (std::abs(finesine[li->r_angle >> ANGLETOFINESHIFT]) < rightangle)
      li->fakecontrast = -(LIGHTBRIGHT >> 1);
    else if (!dx)
      li->fakecontrast = LIGHTBRIGHT;
    else if (std::abs(finecosine[li->r_angle >> ANGLETOFINESHIFT]) < rightangle)
      li->fakecontrast = LIGHTBRIGHT >> 1;
    else
      li->fakecontrast = 0;
  }
}

//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump) {
  g_r_state_globals->numsubsectors = static_cast<int>(W_LumpLength(lump) / sizeof(mapsubsector_t));
  g_r_state_globals->subsectors    = zmalloc<decltype(g_r_state_globals->subsectors)>(static_cast<unsigned long>(g_r_state_globals->numsubsectors) * sizeof(subsector_t), PU_LEVEL, 0);
  uint8_t * data                   = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  // [crispy] fail on missing subsectors
  if (!data || !g_r_state_globals->numsubsectors)
    I_Error("P_LoadSubsectors: No subsectors in map!");

  auto * ms = reinterpret_cast<mapsubsector_t *>(data);
  std::memset(g_r_state_globals->subsectors, 0, static_cast<unsigned long>(g_r_state_globals->numsubsectors) * sizeof(subsector_t));
  subsector_t * ss = g_r_state_globals->subsectors;

  for (int i = 0; i < g_r_state_globals->numsubsectors; i++, ss++, ms++) {
    ss->numlines  = static_cast<unsigned short>(SHORT(ms->numsegs));  // [crispy] extended nodes
    ss->firstline = static_cast<unsigned short>(SHORT(ms->firstseg)); // [crispy] extended nodes
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadSectors
//
void P_LoadSectors(int lump) {
  // [crispy] fail on missing sectors
  if (lump >= static_cast<int>(numlumps))
    I_Error("P_LoadSectors: No sectors in map!");

  g_r_state_globals->numsectors = static_cast<int>(W_LumpLength(lump) / sizeof(mapsector_t));
  g_r_state_globals->sectors    = zmalloc<decltype(g_r_state_globals->sectors)>(static_cast<unsigned long>(g_r_state_globals->numsectors) * sizeof(sector_t), PU_LEVEL, 0);
  std::memset(g_r_state_globals->sectors, 0, static_cast<unsigned long>(g_r_state_globals->numsectors) * sizeof(sector_t));
  uint8_t * data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  // [crispy] fail on missing sectors
  if (!data || !g_r_state_globals->numsectors)
    I_Error("P_LoadSectors: No sectors in map!");

  auto *     ms = reinterpret_cast<mapsector_t *>(data);
  sector_t * ss = g_r_state_globals->sectors;
  for (int i = 0; i < g_r_state_globals->numsectors; i++, ss++, ms++) {
    ss->floorheight   = SHORT(ms->floorheight) << FRACBITS;
    ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
    ss->floorpic      = static_cast<short>(R_FlatNumForName(ms->floorpic));
    ss->ceilingpic    = static_cast<short>(R_FlatNumForName(ms->ceilingpic));
    ss->lightlevel    = SHORT(ms->lightlevel);
    ss->special       = SHORT(ms->special);
    ss->tag           = SHORT(ms->tag);
    ss->thinglist     = nullptr;
    // [crispy] WiggleFix: [kb] for R_FixWiggle()
    ss->cachedheight = 0;
    // [AM] Sector interpolation.  Even if we're
    //      not running uncapped, the renderer still
    //      uses this data.
    ss->oldfloorheight      = ss->floorheight;
    ss->interpfloorheight   = ss->floorheight;
    ss->oldceilingheight    = ss->ceilingheight;
    ss->interpceilingheight = ss->ceilingheight;
    // [crispy] inhibit sector interpolation during the 0th gametic
    ss->oldgametic = -1;
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadNodes
//
void P_LoadNodes(int lump) {
  g_r_state_globals->numnodes = static_cast<int>(W_LumpLength(lump) / sizeof(mapnode_t));
  g_r_state_globals->nodes    = zmalloc<decltype(g_r_state_globals->nodes)>(static_cast<unsigned long>(g_r_state_globals->numnodes) * sizeof(node_t), PU_LEVEL, 0);
  uint8_t * data              = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  // [crispy] warn about missing nodes
  if (!data || !g_r_state_globals->numnodes) {
    if (g_r_state_globals->numsubsectors == 1)
      fmt::fprintf(stderr, "P_LoadNodes: No nodes in map, but only one subsector.\n");
    else
      I_Error("P_LoadNodes: No nodes in map!");
  }

  auto *   mn = reinterpret_cast<mapnode_t *>(data);
  node_t * no = g_r_state_globals->nodes;

  for (int i = 0; i < g_r_state_globals->numnodes; i++, no++, mn++) {
    no->x  = SHORT(mn->x) << FRACBITS;
    no->y  = SHORT(mn->y) << FRACBITS;
    no->dx = SHORT(mn->dx) << FRACBITS;
    no->dy = SHORT(mn->dy) << FRACBITS;
    for (int j = 0; j < 2; j++) {
      no->children[j] = static_cast<unsigned short>(SHORT(mn->children[j])); // [crispy] extended nodes

      // [crispy] add support for extended nodes
      // from prboom-plus/src/p_setup.c:937-957
      if (no->children[j] == 0xFFFF)
        no->children[j] = -1;
      else if (no->children[j] & 0x8000) {
        no->children[j] &= ~0x8000;

        if (no->children[j] >= g_r_state_globals->numsubsectors)
          no->children[j] = 0;

        no->children[j] |= NF_SUBSECTOR;
      }

      for (int k = 0; k < 4; k++)
        no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
    }
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadThings
//
void P_LoadThings(int lump) {
  uint8_t * data      = cache_lump_num<uint8_t *>(lump, PU_STATIC);
  int       numthings = static_cast<int>(W_LumpLength(lump) / sizeof(mapthing_t));

  mapthing_t * mt = reinterpret_cast<mapthing_t *>(data);
  for (int i = 0; i < numthings; i++, mt++) {
    bool spawn = true;

    // Do not spawn cool, new monsters if !commercial
    if (g_doomstat_globals->gamemode != commercial) {
      switch (SHORT(mt->type)) {
      case 68: // Arachnotron
      case 64: // Archvile
      case 88: // Boss Brain
      case 89: // Boss Shooter
      case 69: // Hell Knight
      case 67: // Mancubus
      case 71: // Pain Elemental
      case 65: // Former Human Commando
      case 66: // Revenant
      case 84: // Wolf SS
        spawn = false;
        break;
      }
    }
    if (spawn == false)
      break;

    // Do spawn all other stuff.
    mapthing_t spawnthing;
    spawnthing.x       = SHORT(mt->x);
    spawnthing.y       = SHORT(mt->y);
    spawnthing.angle   = SHORT(mt->angle);
    spawnthing.type    = SHORT(mt->type);
    spawnthing.options = SHORT(mt->options);

    P_SpawnMapThing(&spawnthing);
  }

  if (!g_doomstat_globals->deathmatch) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (g_doomstat_globals->playeringame[i] && !g_doomstat_globals->playerstartsingame[i]) {
        I_Error("P_LoadThings: Player %d start missing (vanilla crashes here)", i + 1);
      }
      g_doomstat_globals->playerstartsingame[i] = false;
    }
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs(int lump) {
  uint8_t *      data;
  maplinedef_t * mld;
  line_t *       ld;
  vertex_t *     v1;
  vertex_t *     v2;
  int            warn, warn2; // [crispy] warn about invalid linedefs

  g_r_state_globals->numlines = static_cast<int>(W_LumpLength(lump) / sizeof(maplinedef_t));
  g_r_state_globals->lines    = zmalloc<decltype(g_r_state_globals->lines)>(static_cast<unsigned long>(g_r_state_globals->numlines) * sizeof(line_t), PU_LEVEL, 0);
  std::memset(g_r_state_globals->lines, 0, static_cast<unsigned long>(g_r_state_globals->numlines) * sizeof(line_t));
  data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  mld  = reinterpret_cast<maplinedef_t *>(data);
  ld   = g_r_state_globals->lines;
  warn = warn2 = 0; // [crispy] warn about invalid linedefs
  for (int i = 0; i < g_r_state_globals->numlines; i++, mld++, ld++) {
    ld->flags   = static_cast<unsigned short>(SHORT(mld->flags)); // [crispy] extended nodes
    ld->special = SHORT(mld->special);
    // [crispy] warn about unknown linedef types
    if (static_cast<unsigned short>(ld->special) > 141) {
      fmt::fprintf(stderr, "P_LoadLineDefs: Unknown special %d at line %d.\n", ld->special, i);
      warn++;
    }
    ld->tag = SHORT(mld->tag);
    // [crispy] warn about special linedefs without tag
    if (ld->special && !ld->tag) {
      switch (ld->special) {
      case 1:   // Vertical Door
      case 26:  // Blue Door/Locked
      case 27:  // Yellow Door /Locked
      case 28:  // Red Door /Locked
      case 31:  // Manual door open
      case 32:  // Blue locked door open
      case 33:  // Red locked door open
      case 34:  // Yellow locked door open
      case 117: // Blazing door raise
      case 118: // Blazing door open
      case 271: // MBF sky transfers
      case 272:
      case 48:  // Scroll Wall Left
      case 85:  // [crispy] [JN] (Boom) Scroll Texture Right
      case 11:  // s1 Exit level
      case 51:  // s1 Secret exit
      case 52:  // w1 Exit level
      case 124: // w1 Secret exit
        break;
      default:
        fmt::fprintf(stderr, "P_LoadLineDefs: Special linedef %d without tag.\n", i);
        warn2++;
        break;
      }
    }
    v1 = ld->v1 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(mld->v1))]; // [crispy] extended nodes
    v2 = ld->v2 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(mld->v2))]; // [crispy] extended nodes
    ld->dx      = v2->x - v1->x;
    ld->dy      = v2->y - v1->y;

    if (!ld->dx)
      ld->slopetype = ST_VERTICAL;
    else if (!ld->dy)
      ld->slopetype = ST_HORIZONTAL;
    else {
      if (FixedDiv(ld->dy, ld->dx) > 0)
        ld->slopetype = ST_POSITIVE;
      else
        ld->slopetype = ST_NEGATIVE;
    }

    if (v1->x < v2->x) {
      ld->bbox[BOXLEFT]  = v1->x;
      ld->bbox[BOXRIGHT] = v2->x;
    } else {
      ld->bbox[BOXLEFT]  = v2->x;
      ld->bbox[BOXRIGHT] = v1->x;
    }

    if (v1->y < v2->y) {
      ld->bbox[BOXBOTTOM] = v1->y;
      ld->bbox[BOXTOP]    = v2->y;
    } else {
      ld->bbox[BOXBOTTOM] = v2->y;
      ld->bbox[BOXTOP]    = v1->y;
    }

    // [crispy] calculate sound origin of line to be its midpoint
    ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
    ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;

    ld->sidenum[0] = SHORT(mld->sidenum[0]);
    ld->sidenum[1] = SHORT(mld->sidenum[1]);

    // [crispy] substitute dummy sidedef for missing right side
    if (ld->sidenum[0] == NO_INDEX) {
      ld->sidenum[0] = 0;
      fmt::fprintf(stderr, "P_LoadLineDefs: linedef %d without first sidedef!\n", i);
    }

    if (ld->sidenum[0] != NO_INDEX) // [crispy] extended nodes
      ld->frontsector = g_r_state_globals->sides[ld->sidenum[0]].sector;
    else
      ld->frontsector = 0;

    if (ld->sidenum[1] != NO_INDEX) // [crispy] extended nodes
      ld->backsector = g_r_state_globals->sides[ld->sidenum[1]].sector;
    else
      ld->backsector = 0;
  }

  // [crispy] warn about unknown linedef types
  if (warn) {
    fmt::fprintf(stderr, "P_LoadLineDefs: Found %d line%s with unknown linedef type.\n", warn, (warn > 1) ? "s" : "");
  }
  // [crispy] warn about special linedefs without tag
  if (warn2) {
    fmt::fprintf(stderr, "P_LoadLineDefs: Found %d special linedef%s without tag.\n", warn2, (warn2 > 1) ? "s" : "");
  }
  if (warn || warn2) {
    fmt::fprintf(stderr, "THIS MAP MAY NOT WORK AS EXPECTED!\n");
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadSideDefs
//
void P_LoadSideDefs(int lump) {
  uint8_t *      data;
  mapsidedef_t * msd;
  side_t *       sd;

  g_r_state_globals->numsides = static_cast<int>(W_LumpLength(lump) / sizeof(mapsidedef_t));
  g_r_state_globals->sides    = zmalloc<decltype(g_r_state_globals->sides)>(static_cast<unsigned long>(g_r_state_globals->numsides) * sizeof(side_t), PU_LEVEL, 0);
  std::memset(g_r_state_globals->sides, 0, static_cast<unsigned long>(g_r_state_globals->numsides) * sizeof(side_t));
  data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  msd = reinterpret_cast<mapsidedef_t *>(data);
  sd  = g_r_state_globals->sides;
  for (int i = 0; i < g_r_state_globals->numsides; i++, msd++, sd++) {
    sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
    sd->rowoffset     = SHORT(msd->rowoffset) << FRACBITS;
    sd->toptexture    = static_cast<short>(R_TextureNumForName(msd->toptexture));
    sd->bottomtexture = static_cast<short>(R_TextureNumForName(msd->bottomtexture));
    sd->midtexture    = static_cast<short>(R_TextureNumForName(msd->midtexture));
    sd->sector        = &g_r_state_globals->sectors[SHORT(msd->sector)];
    // [crispy] smooth texture scrolling
    sd->basetextureoffset = sd->textureoffset;
  }

  W_ReleaseLumpNum(lump);
}

//
// P_LoadBlockMap
//
bool P_LoadBlockMap(int lump) {
  int     i;
  int     count;
  size_t  lumplen;
  short * wadblockmaplump;

  // [crispy] (re-)create BLOCKMAP if necessary
  if (M_CheckParm("-blockmap") || lump >= static_cast<int>(numlumps) || (lumplen = W_LumpLength(lump)) < 8 || (count = static_cast<int>(lumplen / 2)) >= 0x10000) {
    return false;
  }

  // [crispy] remove BLOCKMAP limit
  // adapted from boom202s/P_SETUP.C:1025-1076
  wadblockmaplump = zmalloc<decltype(wadblockmaplump)>(lumplen, PU_LEVEL, nullptr);
  W_ReadLump(lump, wadblockmaplump);
  g_p_local_globals->blockmaplump = zmalloc<decltype(g_p_local_globals->blockmaplump)>(sizeof(*g_p_local_globals->blockmaplump) * static_cast<unsigned long>(count), PU_LEVEL, nullptr);
  g_p_local_globals->blockmap     = g_p_local_globals->blockmaplump + 4;

  g_p_local_globals->blockmaplump[0] = SHORT(wadblockmaplump[0]);
  g_p_local_globals->blockmaplump[1] = SHORT(wadblockmaplump[1]);
  g_p_local_globals->blockmaplump[2] = static_cast<int32_t>(SHORT(wadblockmaplump[2])) & 0xffff;
  g_p_local_globals->blockmaplump[3] = static_cast<int32_t>(SHORT(wadblockmaplump[3])) & 0xffff;

  // Swap all short integers to native byte ordering.

  for (i = 4; i < count; i++) {
    short t                            = SHORT(wadblockmaplump[i]);
    g_p_local_globals->blockmaplump[i] = (t == -1) ? -1l : static_cast<int32_t>(t) & 0xffff;
  }

  Z_Free(wadblockmaplump);

  // Read the header

  g_p_local_globals->bmaporgx   = g_p_local_globals->blockmaplump[0] << FRACBITS;
  g_p_local_globals->bmaporgy   = g_p_local_globals->blockmaplump[1] << FRACBITS;
  g_p_local_globals->bmapwidth  = g_p_local_globals->blockmaplump[2];
  g_p_local_globals->bmapheight = g_p_local_globals->blockmaplump[3];

  // Clear out mobj chains

  count                         = static_cast<int>(sizeof(*g_p_local_globals->blocklinks)) * g_p_local_globals->bmapwidth * g_p_local_globals->bmapheight;
  g_p_local_globals->blocklinks = zmalloc<decltype(g_p_local_globals->blocklinks)>(count, PU_LEVEL, 0);
  std::memset(g_p_local_globals->blocklinks, 0, static_cast<size_t>(count));

  // [crispy] (re-)create BLOCKMAP if necessary
  fmt::fprintf(stderr, ")\n");
  return true;
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines() {
  line_t **     linebuffer;
  line_t *      li;
  sector_t *    sector;
  subsector_t * ss;
  seg_t *       seg;
  fixed_t       bbox[4];
  int           block;

  // look up sector number for each subsector
  ss = g_r_state_globals->subsectors;
  for (int i = 0; i < g_r_state_globals->numsubsectors; i++, ss++) {
    seg        = &g_r_state_globals->segs[ss->firstline];
    ss->sector = seg->sidedef->sector;
  }

  // count number of lines in each sector
  li         = g_r_state_globals->lines;
  totallines = 0;
  for (int i = 0; i < g_r_state_globals->numlines; i++, li++) {
    totallines++;
    li->frontsector->linecount++;

    if (li->backsector && li->backsector != li->frontsector) {
      li->backsector->linecount++;
      totallines++;
    }
  }

  // build line tables for each sector
  linebuffer = zmalloc<decltype(linebuffer)>(static_cast<unsigned long>(totallines) * sizeof(line_t *), PU_LEVEL, 0);

  for (int i = 0; i < g_r_state_globals->numsectors; ++i) {
    // Assign the line buffer for this sector

    g_r_state_globals->sectors[i].lines = linebuffer;
    linebuffer += g_r_state_globals->sectors[i].linecount;

    // Reset linecount to zero so in the next stage we can count
    // lines into the list.

    g_r_state_globals->sectors[i].linecount = 0;
  }

  // Assign lines to sectors

  for (int i = 0; i < g_r_state_globals->numlines; ++i) {
    li = &g_r_state_globals->lines[i];

    if (li->frontsector != nullptr) {
      sector = li->frontsector;

      sector->lines[sector->linecount] = li;
      ++sector->linecount;
    }

    if (li->backsector != nullptr && li->frontsector != li->backsector) {
      sector = li->backsector;

      sector->lines[sector->linecount] = li;
      ++sector->linecount;
    }
  }

  // Generate bounding boxes for sectors

  sector = g_r_state_globals->sectors;
  for (int i = 0; i < g_r_state_globals->numsectors; i++, sector++) {
    M_ClearBox(bbox);

    for (int j = 0; j < sector->linecount; j++) {
      li = sector->lines[j];

      M_AddToBox(bbox, li->v1->x, li->v1->y);
      M_AddToBox(bbox, li->v2->x, li->v2->y);
    }

    // set the degenmobj_t to the middle of the bounding box
    sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
    sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

    // adjust bounding box to map blocks
    block                    = (bbox[BOXTOP] - g_p_local_globals->bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
    block                    = block >= g_p_local_globals->bmapheight ? g_p_local_globals->bmapheight - 1 : block;
    sector->blockbox[BOXTOP] = block;

    block                       = (bbox[BOXBOTTOM] - g_p_local_globals->bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    block                       = block < 0 ? 0 : block;
    sector->blockbox[BOXBOTTOM] = block;

    block                      = (bbox[BOXRIGHT] - g_p_local_globals->bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    block                      = block >= g_p_local_globals->bmapwidth ? g_p_local_globals->bmapwidth - 1 : block;
    sector->blockbox[BOXRIGHT] = block;

    block                     = (bbox[BOXLEFT] - g_p_local_globals->bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    block                     = block < 0 ? 0 : block;
    sector->blockbox[BOXLEFT] = block;
  }
}

// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but separate coordinates that are *only* used in rendering,
// i.e. r_bsp.c:R_AddLine()

static void P_RemoveSlimeTrails() {
  for (int i = 0; i < g_r_state_globals->numsegs; i++) {
    const line_t * l = g_r_state_globals->segs[i].linedef;
    vertex_t *     v = g_r_state_globals->segs[i].v1;

    // [crispy] ignore exactly vertical or horizontal linedefs
    if (l->dx && l->dy) {
      do {
        // [crispy] vertex wasn't already moved
        if (!v->moved) {
          v->moved = true;
          // [crispy] ignore endpoints of linedefs
          if (v != l->v1 && v != l->v2) {
            // [crispy] move the vertex towards the linedef
            // by projecting it using the law of cosines
            int64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
            int64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
            int64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
            int64_t s   = dx2 + dy2;

            // [crispy] MBF actually overrides v->x and v->y here
            v->r_x = static_cast<fixed_t>((dx2 * v->x + dy2 * l->v1->x + dxy * (v->y - l->v1->y)) / s);
            v->r_y = static_cast<fixed_t>((dy2 * v->y + dx2 * l->v1->y + dxy * (v->x - l->v1->x)) / s);

            // [crispy] wait a minute... moved more than 8 map units?
            // maybe that's a linguortal then, back to the original coordinates
            if (std::abs(v->r_x - v->x) > 8 * FRACUNIT || std::abs(v->r_y - v->y) > 8 * FRACUNIT) {
              v->r_x = v->x;
              v->r_y = v->y;
            }
          }
        }
        // [crispy] if v doesn't point to the second vertex of the seg already, point it there
      } while ((v != g_r_state_globals->segs[i].v2) && (v = g_r_state_globals->segs[i].v2));
    }
  }
}

// Pad the REJECT lump with extra data when the lump is too small,
// to simulate a REJECT buffer overflow in Vanilla Doom.

static void PadRejectArray(uint8_t * array, unsigned int len) {
  unsigned int byte_num;
  uint8_t *    dest;
  unsigned int padvalue;

  // Values to pad the REJECT array with:

  unsigned int rejectpad[4] = {
    0,       // Size
    0,       // Part of z_zone block header
    50,      // PU_LEVEL
    0x1d4a11 // DOOM_CONST_ZONEID
  };

  rejectpad[0] = static_cast<unsigned int>(((totallines * 4 + 3) & ~3) + 24);

  // Copy values from rejectpad into the destination array.

  dest = array;

  for (unsigned int i = 0; i < len && i < sizeof(rejectpad); ++i) {
    byte_num = i % 4;
    *dest    = (rejectpad[i / 4] >> (byte_num * 8)) & 0xff;
    ++dest;
  }

  // We only have a limited pad size.  Print a warning if the
  // REJECT lump is too small.

  if (len > sizeof(rejectpad)) {
    fmt::fprintf(stderr, "PadRejectArray: REJECT lump too short to pad! (%u > %i)\n", len, static_cast<int>(sizeof(rejectpad)));

    // Pad remaining space with 0 (or 0xff, if specified on command line).

    if (M_CheckParm("-reject_pad_with_ff")) {
      padvalue = 0xff;
    } else {
      padvalue = 0xf00;
    }

    std::memset(array + sizeof(rejectpad), static_cast<int>(padvalue), len - sizeof(rejectpad));
  }
}

static void P_LoadReject(int lumpnum) {
  // Calculate the size that the REJECT lump *should* be.

  size_t minlength = static_cast<size_t>((g_r_state_globals->numsectors * g_r_state_globals->numsectors + 7) / 8);

  // If the lump meets the minimum length, it can be loaded directly.
  // Otherwise, we need to allocate a buffer of the correct size
  // and pad it with appropriate data.

  size_t lumplen = W_LumpLength(lumpnum);

  if (lumplen >= minlength) {
    g_p_local_globals->rejectmatrix = cache_lump_num<uint8_t *>(lumpnum, PU_LEVEL);
  } else {
    g_p_local_globals->rejectmatrix = zmalloc<decltype(g_p_local_globals->rejectmatrix)>(minlength, PU_LEVEL, &g_p_local_globals->rejectmatrix);
    W_ReadLump(lumpnum, g_p_local_globals->rejectmatrix);

    PadRejectArray(g_p_local_globals->rejectmatrix + lumplen, static_cast<unsigned int>(minlength - lumplen));
  }
}

// [crispy] log game skill in plain text
const char * skilltable[] = {
  "Nothing",
  "Baby",
  "Easy",
  "Normal",
  "Hard",
  "Nightmare"
};

// [crispy] factor out map lump name and number finding into a separate function
int P_GetNumForMap(int episode, int map, bool critical_param) {
  char lumpname[9];
  int  lumpnum;

  // find map name
  if (g_doomstat_globals->gamemode == commercial) {
    if (map < 10)
      DEH_snprintf(lumpname, 9, "map0%i", map);
    else
      DEH_snprintf(lumpname, 9, "map%i", map);
  } else {
    lumpname[0] = 'E';
    lumpname[1] = static_cast<char>('0' + episode);
    lumpname[2] = 'M';
    lumpname[3] = static_cast<char>('0' + map);
    lumpname[4] = 0;
  }

  // [crispy] special-casing for E1M10 "Sewers" support
  if (crispy->havee1m10 && episode == 1 && map == 10)
    DEH_snprintf(lumpname, 9, "E1M10");

  lumpnum = critical_param ? W_GetNumForName(lumpname) : W_CheckNumForName(lumpname);

  if (g_doomstat_globals->nervewadfile && episode != 2 && map <= 9) {
    lumpnum = W_CheckNumForNameFromTo(lumpname, lumpnum - 1, 0);
  }

  return lumpnum;
}

// pointer to the current map lump info struct
lumpinfo_t * maplumpinfo;

//
// P_SetupLevel
//
void P_SetupLevel(int episode, int map, int, skill_t skill) {
  char        lumpname[9];
  int         lumpnum;
  bool        crispy_validblockmap;
  mapformat_t crispy_mapformat;

  g_doomstat_globals->totalkills = g_doomstat_globals->totalitems = g_doomstat_globals->totalsecret = g_doomstat_globals->wminfo.maxfrags = 0;
  // [crispy] count spawned monsters
  g_doomstat_globals->extrakills     = 0;
  g_doomstat_globals->wminfo.partime = 180;
  for (int i = 0; i < MAXPLAYERS; i++) {
    g_doomstat_globals->players[i].killcount = g_doomstat_globals->players[i].secretcount = g_doomstat_globals->players[i].itemcount = 0;
  }

  // [crispy] No Rest for the Living ...
  if (g_doomstat_globals->nervewadfile) {
    if (episode == 2) {
      g_doomstat_globals->gamemission = pack_nerve;
    } else {
      g_doomstat_globals->gamemission = doom2;
    }
  } else {
    if (g_doomstat_globals->gamemission == pack_nerve) {
      episode = g_doomstat_globals->gameepisode = 2;
    }
  }

  // Initial height of PointOfView
  // will be set by player think.
  g_doomstat_globals->players[g_doomstat_globals->consoleplayer].viewz = 1;

  // [crispy] stop demo warp mode now
  if (crispy->demowarp == map) {
    crispy->demowarp              = 0;
    g_doomstat_globals->nodrawers = false;
    singletics                    = false;
  }

  // [crispy] don't load map's default music if loaded from a savegame with MUSINFO data
  if (!musinfo.from_savegame) {
    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start();
  }
  musinfo.from_savegame = false;

  Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

  // UNUSED W_Profile ();
  P_InitThinkers();

  // if working with a devlopment map, reload it
  W_Reload();

  // [crispy] factor out map lump name and number finding into a separate function
  /*
  // find map name
  if ( gamemode == commercial)
  {
      if (map<10)
          DEH_snprintf(lumpname, 9, "map0%i", map);
      else
          DEH_snprintf(lumpname, 9, "map%i", map);
  }
  else
  {
      lumpname[0] = 'E';
      lumpname[1] = '0' + episode;
      lumpname[2] = 'M';
      lumpname[3] = '0' + map;
      lumpname[4] = 0;
  }

  lumpnum = W_GetNumForName (lumpname);
*/
  lumpnum = P_GetNumForMap(episode, map, true);

  maplumpinfo = lumpinfo[lumpnum];
  strncpy(lumpname, maplumpinfo->name, 8);

  leveltime    = 0;
  oldleveltime = 0;

  // [crispy] better logging
  {
    extern int savedleveltime;
    const int  ltime = savedleveltime / TICRATE,
              ttime  = (g_doomstat_globals->totalleveltimes + savedleveltime) / TICRATE;
    char * rfn_str;

    rfn_str = M_StringJoin(
        g_doomstat_globals->respawnparm ? " -respawn" : "",
        g_doomstat_globals->fastparm ? " -fast" : "",
        g_doomstat_globals->nomonsters ? " -nomonsters" : "",
        nullptr);

    fmt::fprintf(stderr, "P_SetupLevel: %s (%s) %s%s %d:%02d:%02d/%d:%02d:%02d ", maplumpinfo->name, W_WadNameForLump(maplumpinfo), skilltable[std::clamp(static_cast<int>(skill) + 1, 0, 5)], rfn_str, ltime / 3600, (ltime % 3600) / 60, ltime % 60, ttime / 3600, (ttime % 3600) / 60, ttime % 60);

    free(rfn_str);
  }
  // [crispy] check and log map and nodes format
  crispy_mapformat = P_CheckMapFormat(lumpnum);

  // note: most of this ordering is important
  crispy_validblockmap = P_LoadBlockMap(lumpnum + ML_BLOCKMAP); // [crispy] (re-)create BLOCKMAP if necessary
  P_LoadVertexes(lumpnum + ML_VERTEXES);
  P_LoadSectors(lumpnum + ML_SECTORS);
  P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

  if (crispy_mapformat & MFMT_HEXEN)
    P_LoadLineDefs_Hexen(lumpnum + ML_LINEDEFS);
  else
    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
  // [crispy] (re-)create BLOCKMAP if necessary
  if (!crispy_validblockmap) {
    extern void P_CreateBlockMap();
    P_CreateBlockMap();
  }
  if (crispy_mapformat & (MFMT_ZDBSPX | MFMT_ZDBSPZ))
    P_LoadNodes_ZDBSP(lumpnum + ML_NODES, crispy_mapformat & MFMT_ZDBSPZ);
  else if (crispy_mapformat & MFMT_DEEPBSP) {
    P_LoadSubsectors_DeePBSP(lumpnum + ML_SSECTORS);
    P_LoadNodes_DeePBSP(lumpnum + ML_NODES);
    P_LoadSegs_DeePBSP(lumpnum + ML_SEGS);
  } else {
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);
  }

  P_GroupLines();
  P_LoadReject(lumpnum + ML_REJECT);

  // [crispy] remove slime trails
  P_RemoveSlimeTrails();
  // [crispy] fix long wall wobble
  P_SegLengths(false);
  // [crispy] blinking key or skull in the status bar
  std::memset(g_p_local_globals->st_keyorskull, 0, sizeof(g_p_local_globals->st_keyorskull));

  g_doomstat_globals->bodyqueslot  = 0;
  g_doomstat_globals->deathmatch_p = g_doomstat_globals->deathmatchstarts;
  if (crispy_mapformat & MFMT_HEXEN)
    P_LoadThings_Hexen(lumpnum + ML_THINGS);
  else
    P_LoadThings(lumpnum + ML_THINGS);

  // if deathmatch, randomly spawn the active players
  if (g_doomstat_globals->deathmatch) {
    for (int i = 0; i < MAXPLAYERS; i++)
      if (g_doomstat_globals->playeringame[i]) {
        g_doomstat_globals->players[i].mo = nullptr;
        G_DeathMatchSpawnPlayer(i);
      }
  }
  // [crispy] support MUSINFO lump (dynamic music changing)
  if (g_doomstat_globals->gamemode != shareware) {
    S_ParseMusInfo(lumpname);
  }

  // clear special respawning que
  g_p_local_globals->iquehead = g_p_local_globals->iquetail = 0;

  // set up world state
  P_SpawnSpecials();

  // build subsector connect matrix
  //	UNUSED P_ConnectSubsectors ();

  // preload graphics
  if (g_doomstat_globals->precache)
    R_PrecacheLevel();

  // printf ("free memory: 0x%x\n", Z_FreeMemory());
}

// [crispy] height of the spawnstate's first sprite in pixels
static void P_InitActualHeights() {
  for (int i = 0; i < NUMMOBJTYPES; i++) {
    state_t *       state;
    spritedef_t *   sprdef;
    spriteframe_t * sprframe;
    int             lump;
    patch_t *       patch;

    state  = &states[mobjinfo[i].spawnstate];
    sprdef = &g_r_state_globals->sprites[state->sprite];

    if (!sprdef->numframes || !(mobjinfo[i].flags & (MF_SOLID | MF_SHOOTABLE))) {
      mobjinfo[i].actualheight = mobjinfo[i].height;
      continue;
    }

    sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
    lump     = sprframe->lump[0];
    patch    = cache_lump_num<patch_t *>(lump + g_r_state_globals->firstspritelump, PU_CACHE);

    // [crispy] round to the next integer multiple of 8
    mobjinfo[i].actualheight = ((patch->height + 7) & (~7)) << FRACBITS;
  }
}

//
// P_Init
//
void P_Init() {
  P_InitSwitchList();
  P_InitPicAnims();
  R_InitSprites(sprnames);
  P_InitActualHeights();
}

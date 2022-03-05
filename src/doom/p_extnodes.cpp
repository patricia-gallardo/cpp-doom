//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
//	[crispy] support maps with NODES in compressed or uncompressed ZDBSP
// 	format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
//

#include <fmt/printf.h>

#include "i_swap.hpp"
#include "i_system.hpp"
#include "m_bbox.hpp"
#include "p_local.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

// [crispy] support maps with compressed ZDBSP nodes
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include "lump.hpp"
#include "memory.hpp"
#include "p_extnodes.hpp"

void       P_SpawnMapThing(mapthing_t * mthing);
fixed_t    GetOffset(vertex_t * v1, vertex_t * v2);
sector_t * GetSectorAtNullAddress();

// [crispy] support maps with NODES in compressed or uncompressed ZDBSP
// format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
mapformat_t P_CheckMapFormat(int lumpnum) {
  mapformat_t format      = mapformat_t::MFMT_DOOMBSP;
  uint8_t *   nodes_local = nullptr;
  int         b;

  if ((b = lumpnum + ML_BLOCKMAP + 1) < static_cast<int>(numlumps) && !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8)) {
    fmt::fprintf(stderr, "Hexen (");
    format = static_cast<mapformat_t>(format | MFMT_HEXEN);
  } else
    fmt::fprintf(stderr, "Doom (");

  if (!((b = lumpnum + ML_NODES) < static_cast<int>(numlumps) && (nodes_local = cache_lump_num<uint8_t *>(b, PU_CACHE)) && W_LumpLength(b) > 0))
    fmt::fprintf(stderr, "no nodes");
  else if (!memcmp(nodes_local, "xNd4\0\0\0\0", 8)) {
    fmt::fprintf(stderr, "DeePBSP");
    format = static_cast<mapformat_t>(format | MFMT_DEEPBSP);
  } else if (!memcmp(nodes_local, "XNOD", 4)) {
    fmt::fprintf(stderr, "ZDBSP");
    format = static_cast<mapformat_t>(format | MFMT_ZDBSPX);
  } else if (!memcmp(nodes_local, "ZNOD", 4)) {
    fmt::fprintf(stderr, "compressed ZDBSP");
    format = static_cast<mapformat_t>(format | MFMT_ZDBSPZ);
  } else
    fmt::fprintf(stderr, "BSP");

  if (nodes_local)
    W_ReleaseLumpNum(b);

  return format;
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:633-752
void P_LoadSegs_DeePBSP(int lump) {
  g_r_state_globals->numsegs = static_cast<int>(W_LumpLength(lump) / sizeof(mapseg_deepbsp_t));
  g_r_state_globals->segs    = zmalloc<decltype(g_r_state_globals->segs)>(static_cast<unsigned long>(g_r_state_globals->numsegs) * sizeof(seg_t), PU_LEVEL, 0);
  mapseg_deepbsp_t * data    = cache_lump_num<mapseg_deepbsp_t *>(lump, PU_STATIC);

  for (int i = 0; i < g_r_state_globals->numsegs; i++) {
    seg_t *            li = g_r_state_globals->segs + i;
    mapseg_deepbsp_t * ml = data + i;

    li->v1 = &g_r_state_globals->vertexes[ml->v1];
    li->v2 = &g_r_state_globals->vertexes[ml->v2];

    li->angle = static_cast<angle_t>((SHORT(ml->angle)) << FRACBITS);

    //	li->offset = (SHORT(ml->offset))<<FRACBITS; // [crispy] recalculated below
    int      linedef_local = static_cast<unsigned short>(SHORT(ml->linedef));
    line_t * ldef          = &g_r_state_globals->lines[linedef_local];
    li->linedef            = ldef;
    int side               = SHORT(ml->side);

    // e6y: check for wrong indexes
    if (static_cast<unsigned>(ldef->sidenum[side]) >= static_cast<unsigned>(g_r_state_globals->numsides)) {
      I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d", linedef_local, i, static_cast<unsigned>(ldef->sidenum[side]));
    }

    li->sidedef     = &g_r_state_globals->sides[ldef->sidenum[side]];
    li->frontsector = g_r_state_globals->sides[ldef->sidenum[side]].sector;
    // [crispy] recalculate
    li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

    if (ldef->flags & ML_TWOSIDED) {
      int sidenum = ldef->sidenum[side ^ 1];

      if (sidenum < 0 || sidenum >= g_r_state_globals->numsides) {
        if (li->sidedef->midtexture) {
          li->backsector = 0;
          fmt::fprintf(stderr, "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n", i);
        } else
          li->backsector = GetSectorAtNullAddress();
      } else
        li->backsector = g_r_state_globals->sides[sidenum].sector;
    } else
      li->backsector = 0;
  }

  W_ReleaseLumpNum(lump);
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:843-863
void P_LoadSubsectors_DeePBSP(int lump) {
  g_r_state_globals->numsubsectors = static_cast<int>(W_LumpLength(lump) / sizeof(mapsubsector_deepbsp_t));
  g_r_state_globals->subsectors    = zmalloc<decltype(g_r_state_globals->subsectors)>(static_cast<unsigned long>(g_r_state_globals->numsubsectors) * sizeof(subsector_t), PU_LEVEL, 0);
  mapsubsector_deepbsp_t * data    = cache_lump_num<mapsubsector_deepbsp_t *>(lump, PU_STATIC);

  // [crispy] fail on missing subsectors
  if (!data || !g_r_state_globals->numsubsectors)
    I_Error("P_LoadSubsectors: No subsectors in map!");

  for (int i = 0; i < g_r_state_globals->numsubsectors; i++) {
    g_r_state_globals->subsectors[i].numlines  = static_cast<int>(data[i].numsegs);
    g_r_state_globals->subsectors[i].firstline = static_cast<int>(data[i].firstseg);
  }

  W_ReleaseLumpNum(lump);
}
// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:995-1038
void P_LoadNodes_DeePBSP(int lump) {

  g_r_state_globals->numnodes = static_cast<int>((W_LumpLength(lump) - 8) / sizeof(mapnode_deepbsp_t));
  g_r_state_globals->nodes    = zmalloc<decltype(g_r_state_globals->nodes)>(static_cast<unsigned long>(g_r_state_globals->numnodes) * sizeof(node_t), PU_LEVEL, 0);
  auto * data                 = cache_lump_num<const uint8_t *>(lump, PU_STATIC);

  // [crispy] warn about missing nodes
  if (!data || !g_r_state_globals->numnodes) {
    if (g_r_state_globals->numsubsectors == 1)
      fmt::fprintf(stderr, "P_LoadNodes: No nodes in map, but only one subsector.\n");
    else
      I_Error("P_LoadNodes: No nodes in map!");
  }

  // skip header
  data += 8;

  for (int i = 0; i < g_r_state_globals->numnodes; i++) {
    node_t *                  no = g_r_state_globals->nodes + i;
    const mapnode_deepbsp_t * mn = reinterpret_cast<const mapnode_deepbsp_t *>(data) + i;

    no->x  = SHORT(mn->x) << FRACBITS;
    no->y  = SHORT(mn->y) << FRACBITS;
    no->dx = SHORT(mn->dx) << FRACBITS;
    no->dy = SHORT(mn->dy) << FRACBITS;

    for (int j = 0; j < 2; j++) {
      no->children[j] = static_cast<int>(mn->children[j]);
      for (int k = 0; k < 4; k++)
        no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
    }
  }

  W_ReleaseLumpNum(lump);
}

// [crispy] support maps with compressed or uncompressed ZDBSP nodes
// adapted from prboom-plus/src/p_setup.c:1040-1331
// heavily modified, condensed and simplyfied
// - removed most paranoid checks, brought in line with Vanilla P_LoadNodes()
// - removed const type punning pointers
// - inlined P_LoadZSegs()
// - added support for compressed ZDBSP nodes
// - added support for flipped levels
void P_LoadNodes_ZDBSP(int lump, bool compressed) {
#ifdef HAVE_LIBZ
  byte * output;
#endif

  unsigned int orgVerts, newVerts;
  unsigned int numSubs, currSeg;
  unsigned int numSegs;
  unsigned int numNodes;
  vertex_t *   newvertarray = nullptr;

  auto * data = cache_lump_num<uint8_t *>(lump, PU_LEVEL);

  // 0. Uncompress nodes lump (or simply skip header)

  if (compressed) {
#ifdef HAVE_LIBZ
    const int  len = W_LumpLength(lump);
    int        outlen, err;
    z_stream * zstream;

    // first estimate for compression rate:
    // output buffer size == 2.5 * input size
    outlen = 2.5 * len;
    output = zmalloc<decltype(output)>(outlen, PU_STATIC, 0);

    // initialize stream state for decompression
    zstream = malloc(sizeof(*zstream));
    std::memset(zstream, 0, sizeof(*zstream));
    zstream->next_in   = data + 4;
    zstream->avail_in  = len - 4;
    zstream->next_out  = output;
    zstream->avail_out = outlen;

    if (inflateInit(zstream) != Z_OK)
      I_Error("P_LoadNodes: Error during ZDBSP nodes decompression initialization!");

    // resize if output buffer runs full
    while ((err = inflate(zstream, Z_SYNC_FLUSH)) == Z_OK) {
      int outlen_old     = outlen;
      outlen             = 2 * outlen_old;
      output             = I_Realloc(output, outlen);
      zstream->next_out  = output + outlen_old;
      zstream->avail_out = outlen - outlen_old;
    }

    if (err != Z_STREAM_END)
      I_Error("P_LoadNodes: Error during ZDBSP nodes decompression!");

    fmt::fprintf(stderr, "P_LoadNodes: ZDBSP nodes compression ratio %.3f\n", (float)zstream->total_out / zstream->total_in);

    data = output;

    if (inflateEnd(zstream) != Z_OK)
      I_Error("P_LoadNodes: Error during ZDBSP nodes decompression shut-down!");

    // release the original data lump
    W_ReleaseLumpNum(lump);
    free(zstream);
#else
    I_Error("P_LoadNodes: Compressed ZDBSP nodes are not supported!");
#endif
  } else {
    // skip header
    data += 4;
  }

  // 1. Load new vertices added during node building

  orgVerts = *(reinterpret_cast<unsigned int *>(data));
  data += sizeof(orgVerts);

  newVerts = *(reinterpret_cast<unsigned int *>(data));
  data += sizeof(newVerts);

  if (orgVerts + newVerts == static_cast<unsigned int>(g_r_state_globals->numvertexes)) {
    newvertarray = g_r_state_globals->vertexes;
  } else {
    newvertarray = zmalloc<decltype(newvertarray)>((orgVerts + newVerts) * sizeof(vertex_t), PU_LEVEL, 0);
    std::memcpy(newvertarray, g_r_state_globals->vertexes, orgVerts * sizeof(vertex_t));
    std::memset(newvertarray + orgVerts, 0, newVerts * sizeof(vertex_t));
  }

  for (unsigned int i = 0; i < newVerts; i++) {
    newvertarray[i + orgVerts].r_x =
        newvertarray[i + orgVerts].x = static_cast<fixed_t>(*(reinterpret_cast<unsigned int *>(data)));
    data += sizeof(newvertarray[0].x);

    newvertarray[i + orgVerts].r_y =
        newvertarray[i + orgVerts].y = static_cast<fixed_t>(*(reinterpret_cast<unsigned int *>(data)));
    data += sizeof(newvertarray[0].y);
  }

  if (g_r_state_globals->vertexes != newvertarray) {
    for (int i = 0; i < g_r_state_globals->numlines; i++) {
      g_r_state_globals->lines[i].v1 = g_r_state_globals->lines[i].v1 - g_r_state_globals->vertexes + newvertarray;
      g_r_state_globals->lines[i].v2 = g_r_state_globals->lines[i].v2 - g_r_state_globals->vertexes + newvertarray;
    }

    Z_Free(g_r_state_globals->vertexes);
    g_r_state_globals->vertexes    = newvertarray;
    g_r_state_globals->numvertexes = static_cast<int>(orgVerts + newVerts);
  }

  // 2. Load subsectors

  numSubs = *(reinterpret_cast<unsigned int *>(data));
  data += sizeof(numSubs);

  if (numSubs < 1)
    I_Error("P_LoadNodes: No subsectors in map!");

  g_r_state_globals->numsubsectors = static_cast<int>(numSubs);
  g_r_state_globals->subsectors    = zmalloc<decltype(g_r_state_globals->subsectors)>(static_cast<unsigned long>(g_r_state_globals->numsubsectors) * sizeof(subsector_t), PU_LEVEL, 0);

  for (int i = currSeg = 0; i < g_r_state_globals->numsubsectors; i++) {
    mapsubsector_zdbsp_t * mseg = reinterpret_cast<mapsubsector_zdbsp_t *>(data) + i;

    g_r_state_globals->subsectors[i].firstline = static_cast<int>(currSeg);
    g_r_state_globals->subsectors[i].numlines  = static_cast<int>(mseg->numsegs);
    currSeg += mseg->numsegs;
  }

  data += static_cast<unsigned long>(g_r_state_globals->numsubsectors) * sizeof(mapsubsector_zdbsp_t);

  // 3. Load segs

  numSegs = *(reinterpret_cast<unsigned int *>(data));
  data += sizeof(numSegs);

  // The number of stored segs should match the number of segs used by subsectors
  if (numSegs != currSeg) {
    I_Error("P_LoadNodes: Incorrect number of segs in ZDBSP nodes!");
  }

  g_r_state_globals->numsegs = static_cast<int>(numSegs);
  g_r_state_globals->segs    = zmalloc<decltype(g_r_state_globals->segs)>(static_cast<unsigned long>(g_r_state_globals->numsegs) * sizeof(seg_t), PU_LEVEL, 0);

  for (int i = 0; i < g_r_state_globals->numsegs; i++) {
    line_t *         ldef;
    unsigned int     linedef_local;
    unsigned char    side;
    seg_t *          li = g_r_state_globals->segs + i;
    mapseg_zdbsp_t * ml = reinterpret_cast<mapseg_zdbsp_t *>(data) + i;

    li->v1 = &g_r_state_globals->vertexes[ml->v1];
    li->v2 = &g_r_state_globals->vertexes[ml->v2];

    linedef_local = static_cast<unsigned short>(SHORT(ml->linedef));
    ldef          = &g_r_state_globals->lines[linedef_local];
    li->linedef   = ldef;
    side          = ml->side;

    // e6y: check for wrong indexes
    if (static_cast<unsigned>(ldef->sidenum[side]) >= static_cast<unsigned>(g_r_state_globals->numsides)) {
      I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d", linedef_local, i, static_cast<unsigned>(ldef->sidenum[side]));
    }

    li->sidedef     = &g_r_state_globals->sides[ldef->sidenum[side]];
    li->frontsector = g_r_state_globals->sides[ldef->sidenum[side]].sector;

    // seg angle and offset are not included
    li->angle  = R_PointToAngle2(g_r_state_globals->segs[i].v1->x, g_r_state_globals->segs[i].v1->y, g_r_state_globals->segs[i].v2->x, g_r_state_globals->segs[i].v2->y);
    li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

    if (ldef->flags & ML_TWOSIDED) {
      int sidenum = ldef->sidenum[side ^ 1];

      if (sidenum < 0 || sidenum >= g_r_state_globals->numsides) {
        if (li->sidedef->midtexture) {
          li->backsector = 0;
          fmt::fprintf(stderr, "P_LoadSegs: Linedef %u has two-sided flag set, but no second sidedef\n", i);
        } else
          li->backsector = GetSectorAtNullAddress();
      } else
        li->backsector = g_r_state_globals->sides[sidenum].sector;
    } else
      li->backsector = 0;
  }

  data += static_cast<unsigned long>(g_r_state_globals->numsegs) * sizeof(mapseg_zdbsp_t);

  // 4. Load nodes

  numNodes = *(reinterpret_cast<unsigned int *>(data));
  data += sizeof(numNodes);

  g_r_state_globals->numnodes = static_cast<int>(numNodes);
  g_r_state_globals->nodes    = zmalloc<decltype(g_r_state_globals->nodes)>(static_cast<unsigned long>(g_r_state_globals->numnodes) * sizeof(node_t), PU_LEVEL, 0);

  for (int i = 0; i < g_r_state_globals->numnodes; i++) {
    node_t *          no = g_r_state_globals->nodes + i;
    mapnode_zdbsp_t * mn = reinterpret_cast<mapnode_zdbsp_t *>(data) + i;

    no->x  = SHORT(mn->x) << FRACBITS;
    no->y  = SHORT(mn->y) << FRACBITS;
    no->dx = SHORT(mn->dx) << FRACBITS;
    no->dy = SHORT(mn->dy) << FRACBITS;

    for (int j = 0; j < 2; j++) {
      no->children[j] = static_cast<int>(mn->children[j]);

      for (int k = 0; k < 4; k++)
        no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
    }
  }

#ifdef HAVE_LIBZ
  if (compressed)
    Z_Free(output);
  else
#endif
    W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:348-400
void P_LoadThings_Hexen(int lump) {
  mapthing_t         spawnthing;
  mapthing_hexen_t * mt;
  int                numthings;

  auto * data = cache_lump_num<uint8_t *>(lump, PU_STATIC);
  numthings   = static_cast<int>(W_LumpLength(lump) / sizeof(mapthing_hexen_t));

  mt = reinterpret_cast<mapthing_hexen_t *>(data);
  for (int i = 0; i < numthings; i++, mt++) {
    //	spawnthing.tid = SHORT(mt->tid);
    spawnthing.x = SHORT(mt->x);
    spawnthing.y = SHORT(mt->y);
    //	spawnthing.height = SHORT(mt->height);
    spawnthing.angle   = SHORT(mt->angle);
    spawnthing.type    = SHORT(mt->type);
    spawnthing.options = SHORT(mt->options);

    //	spawnthing.special = mt->special;
    //	spawnthing.arg1 = mt->arg1;
    //	spawnthing.arg2 = mt->arg2;
    //	spawnthing.arg3 = mt->arg3;
    //	spawnthing.arg4 = mt->arg4;
    //	spawnthing.arg5 = mt->arg5;

    P_SpawnMapThing(&spawnthing);
  }

  W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:410-490
void P_LoadLineDefs_Hexen(int lump) {
  g_r_state_globals->numlines = static_cast<int>(W_LumpLength(lump) / sizeof(maplinedef_hexen_t));
  g_r_state_globals->lines    = zmalloc<decltype(g_r_state_globals->lines)>(static_cast<unsigned long>(g_r_state_globals->numlines) * sizeof(line_t), PU_LEVEL, 0);
  std::memset(g_r_state_globals->lines, 0, static_cast<unsigned long>(g_r_state_globals->numlines) * sizeof(line_t));
  auto * data = cache_lump_num<uint8_t *>(lump, PU_STATIC);

  maplinedef_hexen_t * mld  = reinterpret_cast<maplinedef_hexen_t *>(data);
  line_t *             ld   = g_r_state_globals->lines;
  int                  warn = 0; // [crispy] warn about unknown linedef types
  for (int i = 0; i < g_r_state_globals->numlines; i++, mld++, ld++) {
    ld->flags = static_cast<unsigned short>(SHORT(mld->flags));

    ld->special = mld->special;
    //	ld->arg1 = mld->arg1;
    //	ld->arg2 = mld->arg2;
    //	ld->arg3 = mld->arg3;
    //	ld->arg4 = mld->arg4;
    //	ld->arg5 = mld->arg5;

    // [crispy] warn about unknown linedef types
    if (static_cast<unsigned short>(ld->special > 141)) {
      fmt::fprintf(stderr, "P_LoadLineDefs: Unknown special %d at line %d\n", ld->special, i);
      warn++;
    }

    vertex_t * v1 = ld->v1 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(mld->v1))];
    vertex_t * v2 = ld->v2 = &g_r_state_globals->vertexes[static_cast<unsigned short>(SHORT(mld->v2))];

    ld->dx = v2->x - v1->x;
    ld->dy = v2->y - v1->y;
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

    if (ld->sidenum[0] != NO_INDEX)
      ld->frontsector = g_r_state_globals->sides[ld->sidenum[0]].sector;
    else
      ld->frontsector = 0;
    if (ld->sidenum[1] != NO_INDEX)
      ld->backsector = g_r_state_globals->sides[ld->sidenum[1]].sector;
    else
      ld->backsector = 0;
  }

  // [crispy] warn about unknown linedef types
  if (warn) {
    fmt::fprintf(stderr, "P_LoadLineDefs: Found %d line%s with unknown linedef type.\n"
                         "THIS MAP MAY NOT WORK AS EXPECTED!\n",
                 warn,
                 (warn > 1) ? "s" : "");
  }

  W_ReleaseLumpNum(lump);
}

//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] Create Blockmap
//

#include <cstdlib>

#include <fmt/printf.h>

#include "memory.hpp"
#include "i_system.hpp"
#include "p_local.hpp"
#include "z_zone.hpp"

// [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted

void P_CreateBlockMap() {
  fixed_t minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

  // First find limits of map

  for (int i = 0; i < g_r_state_globals->numvertexes; i++) {
    if (g_r_state_globals->vertexes[i].x >> FRACBITS < minx)
      minx = g_r_state_globals->vertexes[i].x >> FRACBITS;
    else if (g_r_state_globals->vertexes[i].x >> FRACBITS > maxx)
      maxx = g_r_state_globals->vertexes[i].x >> FRACBITS;
    if (g_r_state_globals->vertexes[i].y >> FRACBITS < miny)
      miny = g_r_state_globals->vertexes[i].y >> FRACBITS;
    else if (g_r_state_globals->vertexes[i].y >> FRACBITS > maxy)
      maxy = g_r_state_globals->vertexes[i].y >> FRACBITS;
  }

  // [crispy] doombsp/DRAWING.M:175-178
  minx -= 8;
  miny -= 8;
  maxx += 8;
  maxy += 8;

  // Save blockmap parameters

  g_p_local_globals->bmaporgx   = minx << FRACBITS;
  g_p_local_globals->bmaporgy   = miny << FRACBITS;
  g_p_local_globals->bmapwidth  = ((maxx - minx) >> MAPBTOFRAC) + 1;
  g_p_local_globals->bmapheight = ((maxy - miny) >> MAPBTOFRAC) + 1;

  // Compute blockmap, which is stored as a 2d array of variable-sized lists.
  //
  // Pseudocode:
  //
  // For each linedef:
  //
  //   Map the starting and ending vertices to blocks.
  //
  //   Starting in the starting vertex's block, do:
  //
  //     Add linedef to current block's list, dynamically resizing it.
  //
  //     If current block is the same as the ending vertex's block, exit loop.
  //
  //     Move to an adjacent block by moving towards the ending block in
  //     either the x or y direction, to the block which contains the linedef.

  {
    typedef struct {
      int n, nalloc, *list;
    } bmap_t;                                                                                                // blocklist structure
    auto     tot  = static_cast<unsigned int>(g_p_local_globals->bmapwidth * g_p_local_globals->bmapheight); // size of blockmap
    bmap_t * bmap = static_cast<bmap_t *>(calloc(sizeof *bmap, tot));                                        // array of blocklists
    int      x, y, adx, ady, bend;

    for (int i = 0; i < g_r_state_globals->numlines; i++) {
      int dx, dy, diff, b;

      // starting coordinates
      x = (g_r_state_globals->lines[i].v1->x >> FRACBITS) - minx;
      y = (g_r_state_globals->lines[i].v1->y >> FRACBITS) - miny;

      // x-y deltas
      adx = g_r_state_globals->lines[i].dx >> FRACBITS, dx = adx < 0 ? -1 : 1;
      ady = g_r_state_globals->lines[i].dy >> FRACBITS, dy = ady < 0 ? -1 : 1;

      // difference in preferring to move across y (>0) instead of x (<0)
      diff = !adx ? 1 : !ady ? -1 :
                               (((x >> MAPBTOFRAC) << MAPBTOFRAC) + (dx > 0 ? MAPBLOCKUNITS - 1 : 0) - x) * (ady = std::abs(ady)) * dx - (((y >> MAPBTOFRAC) << MAPBTOFRAC) + (dy > 0 ? MAPBLOCKUNITS - 1 : 0) - y) * (adx = std::abs(adx)) * dy;

      // starting block, and pointer to its blocklist structure
      b = (y >> MAPBTOFRAC) * g_p_local_globals->bmapwidth + (x >> MAPBTOFRAC);

      // ending block
      bend = (((g_r_state_globals->lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) * g_p_local_globals->bmapwidth + (((g_r_state_globals->lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

      // delta for pointer when moving across y
      dy *= g_p_local_globals->bmapwidth;

      // deltas for diff inside the loop
      adx <<= MAPBTOFRAC;
      ady <<= MAPBTOFRAC;

      // Now we simply iterate block-by-block until we reach the end block.
      while (static_cast<unsigned>(b) < tot) // failsafe -- should ALWAYS be true
      {
        // Increase size of allocated list if necessary
        if (bmap[b].n >= bmap[b].nalloc)
          bmap[b].list = static_cast<int *>(I_Realloc(
              bmap[b].list,
              (static_cast<unsigned long>(bmap[b].nalloc = bmap[b].nalloc ? bmap[b].nalloc * 2 : 8)) * sizeof *bmap->list));

        // Add linedef to end of list
        bmap[b].list[bmap[b].n++] = i;

        // If we have reached the last block, exit
        if (b == bend)
          break;

        // Move in either the x or y direction to the next block
        if (diff < 0)
          diff += ady, b += dx;
        else
          diff -= adx, b += dy;
      }
    }

    // Compute the total size of the blockmap.
    //
    // Compression of empty blocks is performed by reserving two offset words
    // at tot and tot+1.
    //
    // 4 words, unused if this routine is called, are reserved at the start.

    {
      int count = static_cast<int>(tot + 6); // we need at least 1 word per block, plus reserved's

      for (unsigned int i = 0; i < tot; i++)
        if (bmap[i].n)
          count += bmap[i].n + 2; // 1 header word + 1 trailer word + blocklist

      // Allocate blockmap lump with computed count
      g_p_local_globals->blockmaplump = zmalloc<decltype(g_p_local_globals->blockmaplump)>(sizeof(*g_p_local_globals->blockmaplump) * static_cast<unsigned long>(count), PU_LEVEL, 0);
    }

    // Now compress the blockmap.
    {
      int      ndx = static_cast<int>(tot += 4); // Advance index to start of linedef lists
      bmap_t * bp  = bmap;                       // Start of uncompressed blockmap

      g_p_local_globals->blockmaplump[ndx++] = 0;  // Store an empty blockmap list at start
      g_p_local_globals->blockmaplump[ndx++] = -1; // (Used for compression)

      for (unsigned int i = 4; i < tot; i++, bp++)
        if (bp->n) // Non-empty blocklist
        {
          g_p_local_globals->blockmaplump[g_p_local_globals->blockmaplump[i] = ndx++] = 0; // Store index & header
          do
            g_p_local_globals->blockmaplump[ndx++] = bp->list[--bp->n]; // Copy linedef list
          while (bp->n);
          g_p_local_globals->blockmaplump[ndx++] = -1; // Store trailer
          free(bp->list);                              // Free linedef list
        } else                                         // Empty blocklist: point to reserved empty blocklist
          g_p_local_globals->blockmaplump[i] = static_cast<int32_t>(tot);

      free(bmap); // Free uncompressed blockmap
    }
  }

  // [crispy] copied over from P_LoadBlockMap()
  {
    int count                     = static_cast<int>(sizeof(*g_p_local_globals->blocklinks)) * g_p_local_globals->bmapwidth * g_p_local_globals->bmapheight;
    g_p_local_globals->blocklinks = zmalloc<decltype(g_p_local_globals->blocklinks)>(static_cast<size_t>(count), PU_LEVEL, 0);
    std::memset(g_p_local_globals->blocklinks, 0, static_cast<size_t>(count));
    g_p_local_globals->blockmap = g_p_local_globals->blockmaplump + 4;
  }

  fmt::fprintf(stderr, "+BLOCKMAP)\n");
}

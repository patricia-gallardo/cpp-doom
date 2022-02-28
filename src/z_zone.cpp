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
//	Zone Memory Allocation. Neat.
//

#include <cstring>

#include <fmt/printf.h>

#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"

#include "z_zone.hpp"

//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

constexpr auto MEM_ALIGN = sizeof(void *);
constexpr auto ZONEID    = 0x1d4a11;

struct memblock_t {
  int                 size; // including the header and possibly tiny fragments
  void **             user;
  int                 tag; // PU_FREE if this is free
  int                 id;  // should be ZONEID
  struct memblock_t * next;
  struct memblock_t * prev;
};

struct memzone_t {
  // total bytes malloced, including header
  int size;

  // start / end cap for linked list
  memblock_t blocklist;

  memblock_t * rover;
};

static memzone_t * mainzone;
static bool        zero_on_free;
static bool        scan_on_free;

//
// Z_ClearZone
//
[[maybe_unused]] void Z_ClearZone(memzone_t * zone) {
  memblock_t * block = nullptr;

  // set the entire zone to one free block
  zone->blocklist.next =
      zone->blocklist.prev =
          block = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(zone) + sizeof(memzone_t));

  zone->blocklist.user = reinterpret_cast<void **>(zone);
  zone->blocklist.tag  = PU_STATIC;
  zone->rover          = block;

  block->prev = block->next = &zone->blocklist;

  // a free block.
  block->tag = PU_FREE;

  block->size = static_cast<int>(static_cast<unsigned long>(zone->size) - sizeof(memzone_t));
}

//
// Z_Init
//
void Z_Init() {
  memblock_t * block = nullptr;
  int          size  = 0;

  mainzone       = reinterpret_cast<memzone_t *>(I_ZoneBase(&size));
  mainzone->size = size;

  // set the entire zone to one free block
  mainzone->blocklist.next =
      mainzone->blocklist.prev =
          block = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(mainzone) + sizeof(memzone_t));

  mainzone->blocklist.user = reinterpret_cast<void **>(mainzone);
  mainzone->blocklist.tag  = PU_STATIC;
  mainzone->rover          = block;

  block->prev = block->next = &mainzone->blocklist;

  // free block
  block->tag = PU_FREE;

  block->size = static_cast<int>(static_cast<unsigned long>(mainzone->size) - sizeof(memzone_t));

  // [Deliberately undocumented]
  // Zone memory debugging flag. If set, memory is zeroed after it is freed
  // to deliberately break any code that attempts to use it after free.
  //
  zero_on_free = M_ParmExists("-zonezero");

  // [Deliberately undocumented]
  // Zone memory debugging flag. If set, each time memory is freed, the zone
  // heap is scanned to look for remaining pointers to the freed block.
  //
  scan_on_free = M_ParmExists("-zonescan");
}

// Scan the zone heap for pointers within the specified range, and warn about
// any remaining pointers.
static void ScanForBlock(void * start, void * end) {
  memblock_t * block = mainzone->blocklist.next;

  while (block->next != &mainzone->blocklist) {
    int tag = block->tag;

    if (tag == PU_STATIC || tag == PU_LEVEL || tag == PU_LEVSPEC) {
      // Scan for pointers on the assumption that pointers are aligned
      // on word boundaries (word size depending on pointer size):
      void ** mem = reinterpret_cast<void **>(reinterpret_cast<uint8_t *>(block) + sizeof(memblock_t));
      int     len = static_cast<int>((static_cast<unsigned long>(block->size) - sizeof(memblock_t)) / sizeof(void *));

      for (int i = 0; i < len; ++i) {
        if (start <= mem[i] && mem[i] <= end) {
          fmt::fprintf(stderr,
                       "%p has dangling pointer into freed block "
                       "%p (%p -> %p)\n",
                       reinterpret_cast<void *>(mem),
                       start,
                       reinterpret_cast<void *>(&mem[i]),
                       reinterpret_cast<void *>(mem[i]));
        }
      }
    }

    block = block->next;
  }
}

//
// Z_Free
//
void Z_Free(void * ptr) {
  auto * block = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(ptr) - sizeof(memblock_t));

  if (block->id != ZONEID)
    I_Error("Z_Free: freed a pointer without ZONEID");

  if (block->tag != PU_FREE && block->user != nullptr) {
    // clear the user's mark
    *block->user = 0;
  }

  // mark as free
  block->tag  = PU_FREE;
  block->user = nullptr;
  block->id   = 0;

  // If the -zonezero flag is provided, we zero out the block on free
  // to break code that depends on reading freed memory.
  if (zero_on_free) {
    std::memset(ptr, 0, static_cast<unsigned long>(block->size) - sizeof(memblock_t));
  }
  if (scan_on_free) {
    ScanForBlock(ptr,
                 reinterpret_cast<uint8_t *>(ptr) + block->size - sizeof(memblock_t));
  }

  memblock_t * other = block->prev;

  if (other->tag == PU_FREE) {
    // merge with previous free block
    other->size += block->size;
    other->next       = block->next;
    other->next->prev = other;

    if (block == mainzone->rover)
      mainzone->rover = other;

    block = other;
  }

  other = block->next;
  if (other->tag == PU_FREE) {
    // merge the next free block onto the end
    block->size += other->size;
    block->next       = other->next;
    block->next->prev = block;

    if (other == mainzone->rover)
      mainzone->rover = block;
  }
}

//
// Z_Malloc
// You can pass a nullptr user if the tag is < PU_PURGELEVEL.
//
constexpr auto MINFRAGMENT = 64;

void *
    Z_Malloc(int    size,
             int    tag,
             void * user) {
  size = static_cast<int>((static_cast<unsigned long>(size) + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1));

  // scan through the block list,
  // looking for the first free block
  // of sufficient size,
  // throwing out any purgable blocks along the way.

  // account for size of block header
  size = size + static_cast<int>(sizeof(memblock_t));

  // if there is a free block behind the rover,
  //  back up over them
  memblock_t * base = mainzone->rover;

  if (base->prev->tag == PU_FREE)
    base = base->prev;

  memblock_t * rover = base;
  memblock_t * start = base->prev;

  do {
    if (rover == start) {
      // scanned all the way around the list
      //          I_Error ("Z_Malloc: failed on allocation of %i bytes", size);

      // [crispy] allocate another zone twice as big
      Z_Init();

      base  = mainzone->rover;
      rover = base;
      start = base->prev;
    }

    if (rover->tag != PU_FREE) {
      if (rover->tag < PU_PURGELEVEL) {
        // hit a block that can't be purged,
        // so move base past it
        base = rover = rover->next;
      } else {
        // free the rover block (adding the size to base)

        // the rover can be the base block
        base = base->prev;
        Z_Free(reinterpret_cast<uint8_t *>(rover) + sizeof(memblock_t));
        base  = base->next;
        rover = base->next;
      }
    } else {
      rover = rover->next;
    }

  } while (base->tag != PU_FREE || base->size < size);

  // found a block big enough
  int extra = base->size - size;

  if (extra > MINFRAGMENT) {
    // there will be a free fragment after the allocated block
    auto * newblock = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(base) + size);
    newblock->size  = extra;

    newblock->tag        = PU_FREE;
    newblock->user       = nullptr;
    newblock->prev       = base;
    newblock->next       = base->next;
    newblock->next->prev = newblock;

    base->next = newblock;
    base->size = size;
  }

  if (user == nullptr && tag >= PU_PURGELEVEL)
    I_Error("Z_Malloc: an owner is required for purgable blocks");

  base->user = reinterpret_cast<void **>(user);
  base->tag  = tag;

  void * result = (reinterpret_cast<uint8_t *>(base) + sizeof(memblock_t));

  if (base->user) {
    *base->user = result;
  }

  // next allocation will start looking here
  mainzone->rover = base->next;

  base->id = ZONEID;

  return result;
}

//
// Z_FreeTags
//
void Z_FreeTags(int lowtag,
                int hightag) {
  memblock_t * next = nullptr;

  for (memblock_t * block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = next) {
    // get link before freeing
    next = block->next;

    // free block?
    if (block->tag == PU_FREE)
      continue;

    if (block->tag >= lowtag && block->tag <= hightag)
      Z_Free(reinterpret_cast<uint8_t *>(block) + sizeof(memblock_t));
  }
}

//
// Z_DumpHeap
// Note: TFileDumpHeap( stdout ) ?
//
[[maybe_unused]] void Z_DumpHeap(int lowtag,
                                 int hightag) {
  fmt::printf("zone size: %i  location: %p\n",
              mainzone->size,
              reinterpret_cast<void *>(mainzone));

  fmt::printf("tag range: %i to %i\n",
              lowtag,
              hightag);

  for (memblock_t * block = mainzone->blocklist.next;; block = block->next) {
    if (block->tag >= lowtag && block->tag <= hightag)
      fmt::printf("block:%p    size:%7i    user:%p    tag:%3i\n",
                  reinterpret_cast<void *>(block),
                  block->size,
                  reinterpret_cast<void *>(block->user),
                  block->tag);

    if (block->next == &mainzone->blocklist) {
      // all blocks have been hit
      break;
    }

    if (reinterpret_cast<uint8_t *>(block) + block->size != reinterpret_cast<uint8_t *>(block->next))
      fmt::printf("ERROR: block size does not touch the next block\n");

    if (block->next->prev != block)
      fmt::printf("ERROR: next block doesn't have proper back link\n");

    if (block->tag == PU_FREE && block->next->tag == PU_FREE)
      fmt::printf("ERROR: two consecutive free blocks\n");
  }
}

//
// Z_FileDumpHeap
//
[[maybe_unused]] void Z_FileDumpHeap(FILE * f) {
  fmt::fprintf(f, "zone size: %i  location: %p\n", mainzone->size, reinterpret_cast<void *>(mainzone));

  for (memblock_t * block = mainzone->blocklist.next;; block = block->next) {
    fmt::fprintf(f, "block:%p    size:%7i    user:%p    tag:%3i\n", reinterpret_cast<void *>(block), block->size, reinterpret_cast<void *>(block->user), block->tag);

    if (block->next == &mainzone->blocklist) {
      // all blocks have been hit
      break;
    }

    if (reinterpret_cast<uint8_t *>(block) + block->size != reinterpret_cast<uint8_t *>(block->next))
      fmt::fprintf(f, "ERROR: block size does not touch the next block\n");

    if (block->next->prev != block)
      fmt::fprintf(f, "ERROR: next block doesn't have proper back link\n");

    if (block->tag == PU_FREE && block->next->tag == PU_FREE)
      fmt::fprintf(f, "ERROR: two consecutive free blocks\n");
  }
}

//
// Z_CheckHeap
//
void Z_CheckHeap() {
  for (memblock_t * block = mainzone->blocklist.next;; block = block->next) {
    if (block->next == &mainzone->blocklist) {
      // all blocks have been hit
      break;
    }

    if (reinterpret_cast<uint8_t *>(block) + block->size != reinterpret_cast<uint8_t *>(block->next))
      I_Error("Z_CheckHeap: block size does not touch the next block\n");

    if (block->next->prev != block)
      I_Error("Z_CheckHeap: next block doesn't have proper back link\n");

    if (block->tag == PU_FREE && block->next->tag == PU_FREE)
      I_Error("Z_CheckHeap: two consecutive free blocks\n");
  }
}

//
// Z_ChangeTag
//
void Z_ChangeTag2(void * ptr, int tag, const char * file, int line) {
  auto * block = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(ptr) - sizeof(memblock_t));

  if (block->id != ZONEID)
    I_Error("%s:%i: Z_ChangeTag: block without a ZONEID!",
            file,
            line);

  if (tag >= PU_PURGELEVEL && block->user == nullptr)
    I_Error("%s:%i: Z_ChangeTag: an owner is required "
            "for purgable blocks",
            file,
            line);

  block->tag = tag;
}

[[maybe_unused]] void Z_ChangeUser(void * ptr, void ** user) {
  auto * block = reinterpret_cast<memblock_t *>(reinterpret_cast<uint8_t *>(ptr) - sizeof(memblock_t));

  if (block->id != ZONEID) {
    I_Error("Z_ChangeUser: Tried to change user for invalid block!");
  }

  block->user = user;
  *user       = ptr;
}

//
// Z_FreeMemory
//
[[maybe_unused]] int Z_FreeMemory() {
  int free = 0;

  for (memblock_t * block = mainzone->blocklist.next;
       block != &mainzone->blocklist;
       block = block->next) {
    if (block->tag == PU_FREE || block->tag >= PU_PURGELEVEL)
      free += block->size;
  }

  return free;
}

[[maybe_unused]] unsigned int Z_ZoneSize() {
  return static_cast<unsigned int>(mainzone->size);
}

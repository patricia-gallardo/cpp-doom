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
//	This is an implementation of the zone memory API which
//	uses native calls to malloc() and free().
//

#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "doomtype.hpp"
#include "i_system.hpp"
#include "z_zone.hpp"

constexpr auto ZONEID = 0x1d4a11;

using memblock_t = struct memblock_s;

struct memblock_s {
  int          id; // = ZONEID
  int          tag;
  int          size;
  void **      user;
  memblock_t * prev;
  memblock_t * next;
};

// Linked list of allocated blocks for each tag type

static memblock_t * allocated_blocks[PU_NUM_TAGS];

#ifdef TESTING

static int test_malloced = 0;

void * test_malloc(size_t size) {
  int * result;

  if (test_malloced + size > 2 * 1024 * 1024) {
    return nullptr;
  }

  test_malloced += size;

  result = malloc(size + sizeof(int));

  *result = size;

  return result + 1;
}

void test_free(void * data) {
  int * i;

  i = ((int *)data) - 1;

  test_malloced -= *i;

  free(i);
}

#define malloc test_malloc
#define free   test_free

#endif /* #ifdef TESTING */

// Add a block into the linked list for its type.

static void Z_InsertBlock(memblock_t * block) {
  block->prev                  = nullptr;
  block->next                  = allocated_blocks[block->tag];
  allocated_blocks[block->tag] = block;

  if (block->next != nullptr) {
    block->next->prev = block;
  }
}

// Remove a block from its linked list.

static void Z_RemoveBlock(memblock_t * block) {
  // Unlink from list

  if (block->prev == nullptr) {
    // Start of list

    allocated_blocks[block->tag] = block->next;
  } else {
    block->prev->next = block->next;
  }

  if (block->next != nullptr) {
    block->next->prev = block->prev;
  }
}

//
// Z_Init
//
void Z_Init() {
  std::memset(allocated_blocks, 0, sizeof(allocated_blocks));
  fmt::printf("zone memory: Using native C allocator.\n");
}

//
// Z_Free
//
void Z_Free(void * ptr) {
  auto * byte_ptr = static_cast<uint8_t *>(ptr);
  auto * block    = reinterpret_cast<memblock_t *>(byte_ptr - sizeof(memblock_t));

  if (block->id != ZONEID) {
    I_Error("Z_Free: freed a pointer without ZONEID");
  }

  if (block->tag != PU_FREE && block->user != nullptr) {
    // clear the user's mark

    *block->user = nullptr;
  }

  Z_RemoveBlock(block);

  // Free back to system

  free(block);
}

// Empty data from the cache list to allocate enough data of the size
// required.
//
// Returns true if any blocks were freed.

static bool ClearCache(int size) {
  memblock_t * next_block = nullptr;
  memblock_t * block      = allocated_blocks[PU_CACHE];

  if (block == nullptr) {
    // Cache is already empty.

    return false;
  }

  // Search to the end of the PU_CACHE list.  The blocks at the end
  // of the list are the ones that have been free for longer and
  // are more likely to be unneeded now.

  while (block->next != nullptr) {
    block = block->next;
  }

  // printf("out of memory; cleaning out the cache: %i\n", test_malloced);

  // Search backwards through the list freeing blocks until we have
  // freed the amount of memory required.

  int remaining = size;

  while (remaining > 0) {
    if (block == nullptr) {
      // No blocks left to free; we've done our best.

      break;
    }

    next_block = block->prev;

    Z_RemoveBlock(block);

    remaining -= block->size;

    if (block->user) {
      *block->user = nullptr;
    }

    free(block);

    block = next_block;
  }

  return true;
}

//
// Z_Malloc
// You can pass a nullptr user if the tag is < PU_PURGELEVEL.
//

void * Z_Malloc(int size, int tag, void * user) {
  if (tag < 0 || tag >= PU_NUM_TAGS || tag == PU_FREE) {
    I_Error("Z_Malloc: attempted to allocate a block with an invalid "
            "tag: %i",
            tag);
  }

  if (user == nullptr && tag >= PU_PURGELEVEL) {
    I_Error("Z_Malloc: an owner is required for purgable blocks");
  }

  // Malloc a block of the required size

  memblock_t * newblock = nullptr;

  while (newblock == nullptr) {
    newblock = static_cast<memblock_t *>(malloc(sizeof(memblock_t) + static_cast<unsigned long>(size)));

    if (newblock == nullptr) {
      if (!ClearCache(static_cast<int>(sizeof(memblock_t) + static_cast<unsigned long>(size)))) {
        I_Error("Z_Malloc: failed on allocation of %i bytes", size);
      }
    }
  }

  newblock->tag = tag;

  // Hook into the linked list for this tag type

  newblock->id   = ZONEID;
  newblock->user = static_cast<void **>(user);
  newblock->size = size;

  Z_InsertBlock(newblock);

  auto * data   = reinterpret_cast<unsigned char *>(newblock);
  void * result = data + sizeof(memblock_t);

  if (user != nullptr) {
    *newblock->user = result;
  }

  return result;
}

//
// Z_FreeTags
//

void Z_FreeTags(int lowtag, int hightag) {
  for (int i = lowtag; i <= hightag; ++i) {
    memblock_t * next = nullptr;

    // Free all in this chain

    for (memblock_t * block = allocated_blocks[i]; block != nullptr;) {
      next = block->next;

      // Free this block

      if (block->user != nullptr) {
        *block->user = nullptr;
      }

      free(block);

      // Jump to the next in the chain

      block = next;
    }

    // This chain is empty now

    allocated_blocks[i] = nullptr;
  }
}

//
// Z_DumpHeap
//
[[maybe_unused]] void Z_DumpHeap(int /*lowtag*/, int /*hightag*/) {
  // broken

#if 0
    memblock_t*	block;
	
   fmt::printf("zone size: %i  location: %p\n",
	    mainzone->size,mainzone);
    
   fmt::printf("tag range: %i to %i\n",
	    lowtag, hightag);
	
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	if (block->tag >= lowtag && block->tag <= hightag)
	   fmt::printf("block:%p    size:%7i    user:%p    tag:%3i\n",
		    block, block->size, block->user, block->tag);
		
	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}
	
	if ( (byte *)block + block->size != (byte *)block->next)
	   fmt::printf("ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	   fmt::printf("ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	   fmt::printf("ERROR: two consecutive free blocks\n");
    }
#endif
}

//
// Z_FileDumpHeap
//
[[maybe_unused]] void Z_FileDumpHeap(FILE * /*f*/) {
  // broken
#if 0
    memblock_t*	block;
	
    fmt::fprintf (f,"zone size: %i  location: %p\n",mainzone->size,mainzone);
	
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	fmt::fprintf (f,"block:%p    size:%7i    user:%p    tag:%3i\n",
		 block, block->size, block->user, block->tag);
		
	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}
	
	if ( (byte *)block + block->size != (byte *)block->next)
	    fmt::fprintf (f,"ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	    fmt::fprintf (f,"ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	    fmt::fprintf (f,"ERROR: two consecutive free blocks\n");
    }
#endif
}

//
// Z_CheckHeap
//
void Z_CheckHeap() {
  // Check all chains

  for (int i = 0; i < PU_NUM_TAGS; ++i) {
    memblock_t * prev = nullptr;

    for (memblock_t * block = allocated_blocks[i]; block != nullptr; block = block->next) {
      if (block->id != ZONEID) {
        I_Error("Z_CheckHeap: Block without a ZONEID!");
      }

      if (block->prev != prev) {
        I_Error("Z_CheckHeap: Doubly-linked list corrupted!");
      }

      prev = block;
    }
  }
}

//
// Z_ChangeTag
//

void Z_ChangeTag2(void * ptr, int tag, const char * file, int line) {
  auto * byte_ptr = static_cast<uint8_t *>(ptr);
  auto * block    = reinterpret_cast<memblock_t *>(byte_ptr - sizeof(memblock_t));

  if (block->id != ZONEID)
    I_Error("%s:%i: Z_ChangeTag: block without a ZONEID!",
            file,
            line);

  if (tag >= PU_PURGELEVEL && block->user == nullptr)
    I_Error("%s:%i: Z_ChangeTag: an owner is required "
            "for purgable blocks",
            file,
            line);

  // Remove the block from its current list, and rehook it into
  // its new list.

  Z_RemoveBlock(block);
  block->tag = tag;
  Z_InsertBlock(block);
}

[[maybe_unused]] void Z_ChangeUser(void * ptr, void ** user) {
  uint8_t *    byte_ptr = static_cast<uint8_t *>(ptr);
  memblock_t * block    = reinterpret_cast<memblock_t *>(byte_ptr - sizeof(memblock_t));

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
  // Limited by the system??

  return -1;
}

[[maybe_unused]] unsigned int Z_ZoneSize() {
  return 0;
}

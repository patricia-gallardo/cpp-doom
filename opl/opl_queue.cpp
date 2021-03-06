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
//     Queue of waiting callbacks, stored in a binary min heap, so that we
//     can always get the first callback.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/printf.h>

#include "memory.hpp"
#include "opl_queue.hpp"

constexpr auto MAX_OPL_QUEUE = 64;

struct opl_queue_entry_t {
  opl_callback_t callback;
  void *         data;
  uint64_t       time;
};

struct [[maybe_unused]] opl_callback_queue_s {
  opl_queue_entry_t entries[MAX_OPL_QUEUE];
  unsigned int      num_entries;
};

opl_callback_queue_t * OPL_Queue_Create() {
  auto * queue = create_struct<opl_callback_queue_t>();

  queue->num_entries = 0;

  return queue;
}

void OPL_Queue_Destroy(opl_callback_queue_t * queue) {
  free(queue);
}

int OPL_Queue_IsEmpty(opl_callback_queue_t * queue) {
  return queue->num_entries == 0;
}

void OPL_Queue_Clear(opl_callback_queue_t * queue) {
  queue->num_entries = 0;
}

void OPL_Queue_Push(opl_callback_queue_t * queue,
                    opl_callback_t         callback,
                    void *                 data,
                    uint64_t               time) {
  if (queue->num_entries >= MAX_OPL_QUEUE) {
    fmt::fprintf(stderr, "OPL_Queue_Push: Exceeded maximum callbacks\n");
    return;
  }

  // Add to last queue entry.

  int entry_id = static_cast<int>(queue->num_entries);
  ++queue->num_entries;

  // Shift existing entries down in the heap.

  while (entry_id > 0) {
    int parent_id = (entry_id - 1) / 2;

    // Is the heap condition satisfied?

    if (time >= queue->entries[parent_id].time) {
      break;
    }

    // Move the existing entry down in the heap.

    std::memcpy(&queue->entries[entry_id],
                &queue->entries[parent_id],
                sizeof(opl_queue_entry_t));

    // Advance to the parent.

    entry_id = parent_id;
  }

  // Insert new callback data.

  queue->entries[entry_id].callback = callback;
  queue->entries[entry_id].data     = data;
  queue->entries[entry_id].time     = time;
}

int OPL_Queue_Pop(opl_callback_queue_t * queue,
                  opl_callback_t *       callback,
                  void **                data) {
  // Empty?
  if (queue->num_entries <= 0) {
    return 0;
  }

  // Store the result:

  *callback = queue->entries[0].callback;
  *data     = queue->entries[0].data;

  // Decrease the heap size, and keep pointer to the last entry in
  // the heap, which must now be percolated down from the top.

  --queue->num_entries;
  opl_queue_entry_t * entry = &queue->entries[queue->num_entries];

  // Percolate down.

  int i      = 0;
  int next_i = 0;

  for (;;) {
    auto child1 = static_cast<unsigned int>(i * 2 + 1);
    auto child2 = static_cast<unsigned int>(i * 2 + 2);

    if (child1 < queue->num_entries
        && queue->entries[child1].time < entry->time) {
      // Left child is less than entry.
      // Use the minimum of left and right children.

      if (child2 < queue->num_entries
          && queue->entries[child2].time < queue->entries[child1].time) {
        next_i = static_cast<int>(child2);
      } else {
        next_i = static_cast<int>(child1);
      }
    } else if (child2 < queue->num_entries
               && queue->entries[child2].time < entry->time) {
      // Right child is less than entry.  Go down the right side.

      next_i = static_cast<int>(child2);
    } else {
      // Finished percolating.
      break;
    }

    // Percolate the next value up and advance.

    std::memcpy(&queue->entries[i],
                &queue->entries[next_i],
                sizeof(opl_queue_entry_t));
    i = next_i;
  }

  // Store the old last-entry at its new position.

  std::memcpy(&queue->entries[i], entry, sizeof(opl_queue_entry_t));

  return 1;
}

uint64_t OPL_Queue_Peek(opl_callback_queue_t * queue) {
  if (queue->num_entries > 0) {
    return queue->entries[0].time;
  } else {
    return 0;
  }
}

void OPL_Queue_AdjustCallbacks(opl_callback_queue_t * queue,
                               uint64_t               time,
                               float                  factor) {
  for (unsigned int i = 0; i < queue->num_entries; ++i) {
    auto offset            = queue->entries[i].time - time;
    queue->entries[i].time = time + static_cast<uint64_t>(static_cast<float>(offset) / factor);
  }
}

#ifdef TEST

#include <assert.h>

static void PrintQueueNode(opl_callback_queue_t * queue, int node, int depth) {
  if (node >= queue->num_entries) {
    return;
  }

  for (int i = 0; i < depth * 3; ++i) {
    fmt::printf(" ");
  }

  fmt::printf("%i\n", queue->entries[node].time);

  PrintQueueNode(queue, node * 2 + 1, depth + 1);
  PrintQueueNode(queue, node * 2 + 2, depth + 1);
}

static void PrintQueue(opl_callback_queue_t * queue) {
  PrintQueueNode(queue, 0, 0);
}

int main() {
  opl_callback_queue_t * queue;
  int                    iteration;

  queue = OPL_Queue_Create();

  for (iteration = 0; iteration < 5000; ++iteration) {
    opl_callback_t callback;
    void *         data;
    unsigned int   time;
    unsigned int   newtime;

    for (int i = 0; i < MAX_OPL_QUEUE; ++i) {
      time = rand() % 0x10000;
      OPL_Queue_Push(queue, nullptr, nullptr, time);
    }

    time = 0;

    for (int i = 0; i < MAX_OPL_QUEUE; ++i) {
      assert(!OPL_Queue_IsEmpty(queue));
      newtime = OPL_Queue_Peek(queue);
      assert(OPL_Queue_Pop(queue, &callback, &data));

      assert(newtime >= time);
      time = newtime;
    }

    assert(OPL_Queue_IsEmpty(queue));
    assert(!OPL_Queue_Pop(queue, &callback, &data));
  }
}

#endif

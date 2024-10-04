// SPDX-FileCopyrightText: © 2024 Lillith Beatrix archaic.archea@gmail.com
// SPDX-License-Identifier: MIT

// This file contains a dispatch queue implementation for the Itanium-Emul emulator.

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <UQUEUE.h>
#include <string.h>

// Initialize queue with a new base and size, might error
uint8_t init_queue(UQueue *q, uintmax_t entries) {
    q->base = malloc(entries * sizeof(uint64_t));
    memset(q->base, 0, entries * sizeof(uint64_t));

    if (q->base == NULL) {
        // error
        return 1;
    }

    q->entries = entries;
    q->write_off = 0;
    q->read_off = 0;
    q->cur_entries = 0;

    return 0;
}

uint64_t pop_queue(UQueue *q) {
    uintmax_t offset = atomic_fetch_add_explicit(&q->read_off, 1, memory_order_acquire) % q->entries;
    uint64_t entry = -1;
    
    // Grab the current entry we're on, and reset it to 0 so no one else uses it
    entry = atomic_exchange_explicit(q->base + offset, 0, memory_order_acq_rel);
    atomic_fetch_sub_explicit(&q->cur_entries, 1, memory_order_release);
    
    return entry;
}

void push_queue(UQueue *q, uint64_t val) {
    uintmax_t fetch_off = atomic_fetch_add_explicit(&q->write_off, 1, memory_order_release) % q->entries;
    atomic_fetch_add_explicit(&q->cur_entries, 1, memory_order_release);

    // Bit 0 (0x1) is the valid bit, we must set it in the new entry
    atomic_store_explicit(q->base + fetch_off, val | 0b1, memory_order_release);
}

// Current valid entry count
uintmax_t entries(UQueue *q) {
    return atomic_load(&q->cur_entries);
}
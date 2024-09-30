// SPDX-FileCopyrightText: © 2024 Lillith Beatrix archaic.archea@gmail.com
// SPDX-License-Identifier: MIT

// This file contains a dispatch queue implementation for the Itanium-Emul emulator.

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

// This *should* be lockless, and performant
// Sadly untested though, actual performance TBD
typedef struct __UQUEUE_T {
    uint64_t *base;
    uintmax_t entries;
    // Doesn't need to be atomic due to only one producer
    uintmax_t write_off;
    // Must be atomic due to multiple potential consumers
    atomic_uintmax_t read_off;
} UQueue;

// Initialize queue with a new base and size, might error
uint8_t init_queue(UQueue *q, uintmax_t entries) {
    q->base = malloc(entries * sizeof(uint64_t));

    if (q->base == NULL) {
        // error
        return 1;
    }

    q->entries = entries;
    q->write_off = 0;
    q->read_off = 0;

    return 0;
}

uint64_t pop_queue(UQueue *q) {
    rqueue:
    uintmax_t offset = atomic_fetch_add_explicit(&q->read_off, 1, memory_order_acquire);

    if (offset >= q->entries) {
        // We failed to claim a valid entry, try to reset read_off and get a new value
        atomic_compare_exchange_strong_explicit(&q->read_off, &offset, 0, memory_order_acquire, memory_order_relaxed);
        goto rqueue;
    }

    // Grab the current entry we're on, and reset it to 0 so no one else uses it
    uint64_t entry = 0;
    entry = atomic_exchange_explicit(q->base + offset, entry, memory_order_acq_rel);
    if ((entry & 0) == 1)
        // This entry is invalid, try again to get a valid entry
        goto rqueue;
    
    return entry;
}

void push_queue(UQueue *q, uint64_t val) {
    wqueue:
    uintmax_t fetch_off = q->write_off;
    q->write_off += 1;

    // We have an invalid entry, reset to 0
    if (fetch_off >= q->entries) {
        q->write_off = 1;
        fetch_off = 0;
    }

    // Bit 0 (0x1) is the valid bit, we must set it in the new entry
    *(q->base + fetch_off) = val | 0x1;
}
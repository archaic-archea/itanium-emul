// SPDX-FileCopyrightText: © 2024 Lillith Beatrix archaic.archea@gmail.com
// SPDX-License-Identifier: MIT

// This file contains an implementation of a memory unit conforming to Itanium SDM Revision 2.3
// RAM accesses are mutex'd to simulate atomic accesses and their potential performance issues accurately.

#include <stdatomic.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>

typedef struct __RAM_SLOT_T {
    atomic_uint_fast8_t lock;
    uint8_t *base;
} RamSlot;

static RamSlot *SLOT_ENTRIES = NULL;
static uintmax_t SLOT_SIZE = 0;
static uintmax_t SLOTS = 0;
static uint64_t PADDR_SHIFT = 0;

// This assumes all slots are equally sized for speed purposes
// Slot physical address ranges will be allocated at run-time
// Slot size is measured in bytes, must be a multiple of 4096
// 1 - Bad slot size
// 2 - Allocation failure
uint8_t init_ram(uintmax_t slots, uintmax_t slot_size) {
    if (slot_size & 0xFFF)
        // Slot size isn't a multiple of 4096
        return 1;
    
    int shift_amt = 0;
    SLOTS = slots;
    SLOT_SIZE = slot_size;
    SLOT_ENTRIES = malloc(slots * sizeof(RamSlot));
    for (; slots > 1; slots = slots >> 1)
        shift_amt += 1;

    PADDR_SHIFT = 64 - shift_amt;

    if (SLOT_ENTRIES == NULL)
        return 2;

    uint8_t *base = malloc(slots * slot_size);
    if (base == NULL) {
        free(SLOT_ENTRIES);
        SLOT_ENTRIES = NULL;
        return 2;
    }

    for (uint64_t i = 0; i < SLOTS; i++) {
        SLOT_ENTRIES[i].lock = 0;
        SLOT_ENTRIES[i].base = base + (i * slot_size);
    }

    return 0;
}

// Returns 0 if locking failed
uint8_t lock_slot(RamSlot *entry) {
    uint8_t zero = 0;
    return atomic_compare_exchange_strong_explicit(&entry->lock, &zero, 1, memory_order_acquire, memory_order_relaxed);
}

void free_slot(RamSlot *entry) {
    atomic_store_explicit(&entry->lock, 0, memory_order_release);
}

// Convert a physical address to an actual memory address on the host machine
// This assumes you always have the correct slot to request the address from
// If the slot is not correct then it may return an invalid pointer
uint8_t *get_addr(RamSlot *slot, uint64_t paddr) {
    uint64_t mask = -1;
    // Clear the top bits to create a mask for the paddr
    mask = mask << PADDR_SHIFT;
    mask = mask >> PADDR_SHIFT;

    // Mask out the top bits which normally indicate the slot to use
    paddr = paddr & mask;

    // Physical address is out of this slot's range, this is a bus fault
    if (paddr >= SLOT_SIZE)
        return NULL;
    
    return (void *)(slot->base + paddr);
}
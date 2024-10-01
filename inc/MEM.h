#pragma once

#include <stdint.h>
#include <stdatomic.h>
#include <UQUEUE.h>

typedef struct __RAM_SLOT_T {
    atomic_uint_fast8_t lock;
    uint8_t *base;
} RamSlot;

uint8_t init_ram(uintmax_t slots, uintmax_t slot_size);

uint8_t lock_slot(RamSlot *entry);
void free_slot(RamSlot *entry);

void *get_addr(RamSlot *slot, uint64_t paddr);
void memory_unit(UQueue *q);
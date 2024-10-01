#pragma once

#include <stdint.h>
#include <stdatomic.h>

// This should be lockless, and performant
// Avoiding locking will prevent too much contention on queues with many units
// Sadly untested as of now though, actual performance TBD
typedef struct __UQUEUE_T {
    uint64_t *base;
    uintmax_t entries;
    // Doesn't need to be atomic due to only one producer
    uintmax_t write_off;
    // Must be atomic due to multiple potential consumers
    atomic_uintmax_t read_off;
} UQueue;

uint8_t init_queue(UQueue *q, uintmax_t entries);
uint64_t pop_queue(UQueue *q);
void push_queue(UQueue *q, uint64_t val);
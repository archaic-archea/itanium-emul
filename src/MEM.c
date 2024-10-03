// SPDX-FileCopyrightText: © 2024 Lillith Beatrix archaic.archea@gmail.com
// SPDX-License-Identifier: MIT

// This file contains an implementation of a memory unit conforming to Itanium SDM Revision 2.3
// RAM accesses are mutex'd to simulate atomic accesses and their potential performance issues accurately.

#include <stdatomic.h>
#include <stdlib.h>
#include <stdint.h>
#include <UQUEUE.h>
#include <MEM.h>
#include <threads.h>
#include <EPIC.h>

#include <stdio.h>

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
void *get_addr(RamSlot *slot, uint64_t paddr) {
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

int memory_unit(void *qp) {
    UQueue *q = (UQueue *)qp;

    while (atomic_load(&RUNNING)) {
        // pop_queue should never return an invalid entry
        // Shift it down by 64 minus the size of each individual instruction in itanium
        uint64_t instr = 0;

        if (entries(q) != 0) {
            instr = pop_queue(q) >> 23;
        
            uint16_t ext = (instr >> 27) & 0x1ff;
            uint8_t m = (instr >> 36) & 0b1;
            uint8_t major = instr >> 37;
            
            // Check major opcode
            switch (major) {
                case 0:
                    switch (ext >> 6) {
                        case 0:
                            uint32_t imm = (instr >> 6 & 0xfffff) | (instr >> 15 & 0x100000);
                            uint8_t ext2 = ext >> 4 & 0b11;
                            switch (ext & 0b1111) {
                                case 0b0000:
                                    if (ext2 == 0b00) {
                                        // Break point
                                        printf("break.m %u\n", imm);
                                    } else if (ext2 == 0b01) {
                                        // Invalidate ALAT completely
                                        printf("invala\n");
                                    } else if (ext2 == 0b10) {
                                        // Flush write buffers
                                        printf("fwb\n");
                                    } else if (ext2 == 0b11) {
                                        // Serialize Data (Memory)
                                        printf("srlz.d\n");
                                    }
                                break;
                                case 0b0001:
                                    if (ext2 == 0b00) {
                                        // (Nop/Hint)
                                        if (instr >> 26 & 0b1) 
                                            printf("hint.m %u\n", imm);
                                        else
                                            printf("nop.m %u\n", imm);
                                    } else if ((ext >> 4 & 0b11) == 0b11) {
                                        // Serialize Instruction (Registers)
                                        printf("srlz.i\n");
                                    } else {
                                        printf("Major Opcode 0, x3 0, x4 1 matched to invalid x2\n");
                                        goto bad_instruction;
                                    }
                                break;
                                case 0b0010:
                                    if (ext2 == 0b01) {
                                        // Integer ALAT entry invalidation
                                        printf("invala.e Integer\n");
                                    } else if (ext2 == 0b10) {
                                        // Memory fence ordering
                                        printf("mf\n");
                                    } else {
                                        printf("Major Opcode 0, x3 0, x4 2 matched to invalid x2\n");
                                        goto bad_instruction;
                                    }
                                break;
                                case 0b0011:
                                    if (ext2 == 0b01) {
                                        // Floating Point ALAT entry invalidation
                                        printf("invala.e Floating Point\n");
                                    } else if (ext2 == 0b10) {
                                        // Memory fence acceptance
                                        printf("mf.a\n");
                                    } else if (ext2 == 0b11) {
                                        // Memory sync
                                        printf("sync.i\n");
                                    } else {
                                        printf("Major Opcode 0, x3 0, x4 3 matched to invalid x2\n");
                                        goto bad_instruction;
                                    }
                                break;
                                case 0b0100:
                                    // Set User Mask
                                break;
                                case 0b0101:
                                    // Reset User Mask
                                break;
                                case 0b0110:
                                    // Set System Mask
                                break;
                                case 0b0111:
                                    // Reset System Mask
                                break;
                                case 0b1000:
                                    // Move to AR - Immediate
                                    printf("mov.m ar=imm\n");
                                break;
                                case 0b1010:
                                    // Flush Register Stack
                                    printf("flushrs\n");
                                break;
                                case 0b1100:
                                    // Load Register Stack
                                    printf("loadrs\n");
                                break;
                                default:
                                    printf("Major Opcode 0, x3 0 matched to invalid x4\n");
                                break;
                            }
                        break;
                        case 4:
                            // chk.a.nc Integer
                            printf("chk.a.nc int\n");
                        break;
                        case 5:
                            // chk.a.clr Integer
                            printf("chk.a.clr int\n");
                        break;
                        case 6:
                            // chk.a.nc Floating Point
                            printf("chk.a.nc fp\n");
                        break;
                        case 7:
                            // chk.a.clr Floating Point
                            printf("chk.a.clr fp\n");
                        break;
                        default:
                            printf("Major Opcode 0 matched to invalid extension\n");
                            goto bad_instruction;
                        break;
                    }
                break;
                case 1:
                    // Int Spec Check
                    // FP Spec Check
                    // Flush Cache
                    // Move to AR
                    // Move from AR
                    // Move to CR
                    // Move from CR
                    // RSE Alloc
                    // Move to PSR
                    // Move from PSR
                    // Probe
                    // Probe Imm
                    // Probe Fault Imm
                    // Translation Cache Insert
                    // (Move to Indirect/Translation Register Insert)
                    // Move from Indirect
                    // Translation Purge
                    // Translation Access
                    // TC Entry Purge
                    printf("Case 1\n");
                break;
                case 4:
                    if (m) {
                        if (ext & 0b1) {
                            // Integer (Load/Store)
                            printf("Load/Store\n");
                        } else {
                            // (Semaphore/get FR)
                            printf("Semaphore/get FR\n");
                        }
                    } else if (ext & 0b1) {
                        // Load +Reg
                        printf("Load + Reg\n");
                    } else {
                        printf("Major Opcode 4 matched to invalid extension\n");
                        goto bad_instruction;
                    }
                break;
                case 5:
                    // Integer (Load/Store) +Imm
                    printf("Case 5\n");
                break;
                case 6:
                    // FP (Load/Store)
                    // (FP Load Pair/Set FR)
                    // Line Prefetch
                    // Line Prefetch - Increment by register
                    printf("Case 6\n");
                break;
                case 7:
                    // Line Prefetch - Increment by immediate
                    printf("Case 7\n");
                break;
                default:
                    bad_instruction:
                    printf("Bad instruction %lx\n", instr);
                break;
            }
        }
    }
    
    thrd_exit(1);
}
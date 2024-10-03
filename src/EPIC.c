#include <stdint.h>
#include <stdio.h>
#include <UQUEUE.h>
#include <MEM.h>
#include <stdlib.h>
#include <threads.h>
#include <EPIC.h>
#include <stdatomic.h>
#include <string.h>

volatile uint8_t RUNNING = 1;

static const template template_field_table[0x20] = {{.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1}, {.index = I_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1}, {.index = I_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = L_UNIT, .stop_bit = 0}, {.index = X_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = L_UNIT, .stop_bit = 0}, {.index = X_UNIT, .stop_bit = 1} } },
                                                    {.valid = 0},
                                                    {.valid = 0},
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 1}, {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 1}, {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1} } },    
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = I_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 1} } },
                                                    {.valid = 0},
                                                    {.valid = 0},
                                                    {.valid = 1, .slots = { {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 1} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = M_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 1} } },
                                                    {.valid = 0},
                                                    {.valid = 0},
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 0} } },
                                                    {.valid = 1, .slots = { {.index = M_UNIT, .stop_bit = 0}, {.index = F_UNIT, .stop_bit = 0}, {.index = B_UNIT, .stop_bit = 1} } },
                                                    {.valid = 0},
                                                    {.valid = 0}};

template ia64_get_template_field(bundle b) {
  //printf("template bits: %li\n", (uint64_t  )(b & 0b11111));
  return template_field_table[b & 0b11111];
}

const char* debug_get_slot_name(units unit) {
  switch (unit) {
    case M_UNIT:
      return "M-Unit";
    case I_UNIT:
      return "I-Unit";
    case L_UNIT:
      return "L-Unit";
    case X_UNIT:
      return "X-Unit";
    case F_UNIT:
      return "F-Unit";
    case B_UNIT:
      return "B-Unit";
    default:
      return "(Unknown Unit)";
  }
}

void debug_print_template(template t) {
  printf("valid: %i\n", t.valid);
  if (t.valid) {
    printf("slots:\t%s, %i\n\t%s, %i\n\t%s, %i\n", debug_get_slot_name(t.slots[0].index), t.slots[0].stop_bit,
                                                                          debug_get_slot_name(t.slots[1].index), t.slots[1].stop_bit,
                                                                          debug_get_slot_name(t.slots[2].index), t.slots[2].stop_bit);
  }
} 

int main(int argc, char *argv[]) {
  uintmax_t bin_len = 0;
  uint64_t *bin = NULL;
  
  if (argc < 2) {
    printf("No file path provided\n");
    return 4;
  }

  FILE *binf = fopen(argv[1], "r");
  if (binf == NULL) {
    printf("Failed to open file '%s'\n", argv[1]);
    return 7;
  }
  fseek(binf, 0, SEEK_END);
  bin_len = ftell(binf) >> 3;
  if (ftell(binf) & 0b1111) {
    printf("File is not multiple of 16\n");
    return 9;
  }
  // Reset to 0 for when we read the file
  fseek(binf, 0, SEEK_SET);

  bin = (uint64_t *)malloc(bin_len * sizeof(uint64_t));
  memset(bin, 0, bin_len * sizeof(uint64_t));
  if (bin == NULL) {
    printf("Failed to allocate buffer for input file '%s'\n", argv[1]);
    return 3;
  }

  fread(bin, sizeof(uint64_t), bin_len, binf);
  if (ferror(binf)) {
    printf("Failed to read '%s'\n", argv[1]);
    return 8;
  }
  fclose(binf);

  if (init_ram(8, 4096) != 0)
    return 1;

  UQueue mq;
  if (init_queue(&mq, 8))
    return 2;

  thrd_t munit_thread;
  if (thrd_create(&munit_thread, memory_unit, &mq) != thrd_success) {
    printf("Failed to create memory thread\n");
    return 5;
  }

  for (int i = 0; i < (bin_len >> 1); i += 1) {
    int idx = i << 1;
    __uint128_t val = (__uint128_t)*(bin + idx) + ((__uint128_t)*(bin + idx + 1) << 64);
    bundle example = val;
    template tmplt = ia64_get_template_field(example);

    for (int x = 0; x < 3; x++) 
      if (tmplt.slots[x].index == M_UNIT) {
        push_queue(&mq, ((val >> 5) >> (41 * x)) << 23);
      }
  }

  uintmax_t entry_cnt;
  do {
    entry_cnt = entries(&mq);
  } while (entry_cnt > 0);
  atomic_store(&RUNNING, 0);

  if (thrd_join(munit_thread, NULL) == thrd_error)
    return 6;

  return 0;
}
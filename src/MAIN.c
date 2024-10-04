#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <MEM.h>
#include <EPIC.h>
#include <UQUEUE.h>
#include <threads.h>
#include <string.h>

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
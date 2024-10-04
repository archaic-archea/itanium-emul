#pragma once

#include <stdint.h>
#include <UQUEUE.h>

extern volatile uint8_t RUNNING;

typedef struct {
  uint64_t value;
  uint8_t NaT:1;
} ia64_gpr;

typedef struct {
  ia64_gpr registers[128];
} registers;

typedef __uint128_t bundle;

typedef enum {
  M_UNIT,
  I_UNIT,
  L_UNIT,
  X_UNIT,
  F_UNIT,
  B_UNIT
} units;

typedef struct {
  units index;
  uint8_t stop_bit:1;
} order;

typedef struct {
  order slots[3];
  uint8_t valid:1;
} template;

typedef struct __Cpu_State {
    UQueue munit_dispatch;
    UQueue iunit_dispatch;
    UQueue funit_dispatch;
    UQueue bunit_dispatch;
    registers int_rf;
} CpuState;

template ia64_get_template_field(bundle b);
const char* debug_get_slot_name(units unit);
void debug_print_template(template t);
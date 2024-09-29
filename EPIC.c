#include <stdint.h>
#include <stdio.h>

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

typedef struct {
  uint64_t value;
  uint8_t NaT:1;
} ia64_gpr;


typedef struct {
  ia64_gpr registers[128];
} registers;

typedef __uint128_t bundle;

template ia64_get_template_field(bundle b) {
  printf("template bits: %li\n", (uint64_t  )(b & 0b11111));
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

int main() {
  uint64_t low = 0x0080000100000011;
  uint64_t high = 0x0084000880420000;   
  (void)template_field_table;
  bundle example = ((__uint128_t)(high) << 64) | low; 
  debug_print_template(ia64_get_template_field(example));   
}

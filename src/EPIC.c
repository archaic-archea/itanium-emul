#include <stdint.h>
#include <stdio.h>
#include <EPIC.h>

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

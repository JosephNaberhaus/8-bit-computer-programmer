#include "byte_output.h"

#include "output.h"

void init_byte_group(byte_group_t group) {
    for (int i = 0; i < NUM_PINS_IN_GROUP; i++) {
        set_up_output(group.pins[i]);
    }
}

void set_byte_group_output(byte_group_t group, uint8_t byte) {
    for (int i = 0; i < NUM_PINS_IN_GROUP; i++) {
        set_output(group.pins[i], byte & (1 << i));
    }
}

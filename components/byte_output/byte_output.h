#ifndef BYTE_OUTPUT_H
#define BYTE_OUTPUT_H

#include <stdint.h>
#include "driver/gpio.h"

#define NUM_PINS_IN_GROUP 8

typedef struct {
    // Pins listed in order from least to move significant bits
    gpio_num_t pins[NUM_PINS_IN_GROUP];
} byte_group_t;

// Initializes all of the GPIO pins in this byte group as outputs
void init_byte_group(byte_group_t group);

// Writes the byte to the GPIO pins.
void set_byte_group_output(byte_group_t group, uint8_t byte);

#endif
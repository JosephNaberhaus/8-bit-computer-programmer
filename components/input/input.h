#ifndef INPUT_H
#define INPUT_H

#include "driver/gpio.h"

typedef void (*input_callback)();

void set_up_input(gpio_num_t pin, input_callback callback);

#endif
#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdbool.h>
#include "driver/gpio.h"

void set_up_output(gpio_num_t pin);
void set_output(gpio_num_t pin, bool value);

#endif

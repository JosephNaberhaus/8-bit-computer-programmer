#include "output.h"

void set_up_output(gpio_num_t pin) {
    #ifdef DEBUG
    printf("Setting up GPIO pin %d as an output\n", pin);
    #endif

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ull << pin;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void set_output(gpio_num_t pin, bool value) {
    gpio_set_level(pin, value);
}

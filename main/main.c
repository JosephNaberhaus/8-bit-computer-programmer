#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"

#include "byte_output.h"
#include "input.h"
#include "output.h"
#include "memory_write.h"
#include "program.h"

void write_jump_to_zero() {
    #ifdef DEBUG
    printf("Writing jump to zero...\n");
    #endif

    clear_memory();
    for (int i = 0; i < 256; i++) {
        printf("%d\n", i);
        uint8_t data[3] = {0b00000001, 0b01000000, 0};
        write_memory(i, data);
    }

    #ifdef DEBUG
    printf("Done writing jump to zero.\n");
    #endif
}

void write_program() {
    #ifdef DEBUG
    printf("writing program...\n");
    #endif

    clear_memory();
    for (int i = 0; i < 256; i++) {
        uint8_t data[3] = {program[i*3], program[i*3+1], program[i*3+2]};
        write_memory(i, data);
    }

    #ifdef DEBUG
    printf("Done writing program...\n");
    #endif
}

void handle_write_switch() {
    static bool flipFlop = false;

    if (flipFlop) {
        write_program();
    } else {
        write_jump_to_zero();
    }

    flipFlop = !flipFlop;
}

void init() {
    #ifdef DEBUG
    printf("Initializing\n");
    #endif

    gpio_install_isr_service(0);

    init_memory_write();

    set_up_input(GPIO_NUM_34, handle_write_switch);

    #ifdef DEBUG
    printf("Done initializing\n");
    #endif
}

void app_main(void) {
    init();
}

#include "memory_write.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "byte_output.h"
#include "output.h"

#define wait_for_write_step vTaskDelay(1 / portTICK_PERIOD_MS);
#define wait_for_clear vTaskDelay(200 / portTICK_PERIOD_MS);

gpio_num_t hard_code_enable_pin;

gpio_num_t memory_bank_enable_pins[num_memory_banks];

byte_group_t data_group;
byte_group_t address_group;

void init_memory_write() {
    #ifdef DEBUG
    printf("Initializing memory outputs...\n");
    #endif

    hard_code_enable_pin = GPIO_NUM_3;
    set_up_output(hard_code_enable_pin);

    memory_bank_enable_pins[0] = GPIO_NUM_23;
    memory_bank_enable_pins[1] = GPIO_NUM_22;
    memory_bank_enable_pins[2] = GPIO_NUM_21;

    for (int i = 0; i < num_memory_banks; i++) {
        set_up_output(memory_bank_enable_pins[i]);
        set_output(memory_bank_enable_pins[i], true);
    }

    data_group.pins[0] = GPIO_NUM_13;
    data_group.pins[1] = GPIO_NUM_12;
    data_group.pins[2] = GPIO_NUM_14;
    data_group.pins[3] = GPIO_NUM_27;
    data_group.pins[4] = GPIO_NUM_26;
    data_group.pins[5] = GPIO_NUM_25;
    data_group.pins[6] = GPIO_NUM_33;
    data_group.pins[7] = GPIO_NUM_32;
    init_byte_group(data_group);

    address_group.pins[0] = GPIO_NUM_15;
    address_group.pins[1] = GPIO_NUM_2;
    address_group.pins[2] = GPIO_NUM_4;
    address_group.pins[3] = GPIO_NUM_16;
    address_group.pins[4] = GPIO_NUM_17;
    address_group.pins[5] = GPIO_NUM_5;
    address_group.pins[6] = GPIO_NUM_18;
    address_group.pins[7] = GPIO_NUM_19;
    init_byte_group(address_group);

    #ifdef DEBUG
    printf("Done initializing memory outputs...\n");
    #endif
}

void clear_memory() {
    for (int i = 0; i < num_memory_banks; i++) {
        #ifdef DEBUG
        printf("Clearing memory bank %d at pin %d\n", i, memory_bank_enable_pins[i]);
        #endif

        // Chip Erase
        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0xAA);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step

        set_output(hard_code_enable_pin, false);
        set_byte_group_output(address_group, 0b10101010);
        set_byte_group_output(data_group, 0x55);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step

        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0x80);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step

        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0xAA);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step

        set_output(hard_code_enable_pin, false);
        set_byte_group_output(address_group, 0b10101010);
        set_byte_group_output(data_group, 0x55);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step

        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0x10);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_clear
    }
}

void write_memory(uint8_t address, uint8_t data[num_memory_banks]) {
    for (int i = 0; i < num_memory_banks; i++) {
        #ifdef DEBUG
        printf("Writing %2x to memory bank %d with enable pin at %d\n", data[i], i, memory_bank_enable_pins[i]);
        #endif

        // Step 1
        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0xAA);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step
        // Step 2
        set_output(hard_code_enable_pin, false);
        set_byte_group_output(address_group, 0b10101010);
        set_byte_group_output(data_group, 0x55);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step
        // Step 3
        set_output(hard_code_enable_pin, true);
        set_byte_group_output(address_group, 0b01010101);
        set_byte_group_output(data_group, 0xA0);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step
        // Step 4 (write data)
        set_byte_group_output(address_group, address);
        set_byte_group_output(data_group, data[i]);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], false);
        wait_for_write_step
        set_output(memory_bank_enable_pins[i], true);
        wait_for_write_step
    }
}

void set_data_bits(uint8_t data) {
    set_byte_group_output(data_group, data);
}

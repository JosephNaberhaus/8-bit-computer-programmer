#ifndef MEMORY_WRITE_H
#define MEMORY_WRITE_H

#include <stdint.h>

#define num_memory_banks 3

void init_memory_write();
void clear_memory();
void write_memory(uint8_t address, uint8_t data[num_memory_banks]);

#endif

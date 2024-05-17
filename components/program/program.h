#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdint.h>

uint8_t program[3*256] = {
#include "assembled.txt"
};

#endif
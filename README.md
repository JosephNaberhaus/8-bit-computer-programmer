# 8 Bit Computer Programmer

This repo contains the code I use to program my breadboard 8-bit computer. It's composed of two parts. First, the [assembler](./assembler/) is a Golang application for translating assembly into machine code. The generated machine code is embedded within [program.h](./components/program/program.h). This header file is part of the second compononent, a C program that can be flashed onto an ESP32 microcontroller. I can then use this ESP32 to program my breadboard computer.



## Instruction Set

The instruction set is my own invention. The following is a brief overview of its features.

### Register Selection

Each instruction takes ones or more register selection arguments. Each one will correspond to one of the four registers availible. The notation is the `$` symbol followed by the number 1-4 for the register number (e.g. `$1`, `$2`, `$3`, or `$4`). Inputs to an instruction can also choose to use an immediate value instead of a register. This is notated by the `$i` symbol. If an instruction uses an immediate value the **final** argument must be a number between 0 and 255. For signed numbers, you will need to manually convert the bit pattern of a 2s complement number into an unsigned number.

### Comments

Programs can contain single-line comments. Just start any line with `//` and the rest of the line will be ignored.

### Labels

As with most assemblers, you can insert labels in your program to simplify control-flow. These are in the form of alphabetic characters followed by a `:` (e.g. `loopStart:`). A preprocesser will replace all references to these labels (the same as the label but without the `:`) with the instruction address it references.

For example, this program:

```
// Start with zero
MV $i $1 0
// Add 1 to the register over and over
loop:
ADD $1 $i $1 1
JMP $i loop
```

will be translated into:

```
MV $i $1 0
ADD $1 $i $1 1
JMP $i 1
```

### Instructions

| **Instruction**                 | **Description**                                                                                                                       |
|---------------------------------|---------------------------------------------------------------------------------------------------------------------------------------|
| `ADD <left> <right> <result>`   | Performs `left + right` and saves to the `result` register                                                                            |
| `SUB <left> <right> <result>`   | Performs `left - right` and saves to the `result` register. Both inputs are assumed to be in 2s complement form                       |
| `AND <first> <second> <result>` | Performs `first & second` (binary AND) and saves to the `result` register                                                             |
| `OR <first> <second> <result>`  | Performs `first \| second` (binary OR) and saves to the `result` register                                                             |
| `LT <left> <right> <result>`    | Performs `left < right` and saves `1` to `result` if true or `0` if not. Both numbers are assumed to be unsigned                      |
| `SLT <left> <right> <result>`   | Performs `left < right` and saves `1` to `result` if true or `0` if not. Both numbers are assumed to be in 2s complement form         |
| `LTEQ <left> <right> <result>`  | Performs `left <= right` and saves `1` to `result` if true or `0` if not. Both numbers are assumed to be unsigned                     |
| `SLTEQ <left> <right> <result>` | Performs `left <= right` and saves `1` to `result` if true or `0` if not. Both numbers are assumed to be in 2s complement form        |
| `EQ <first> <second> <result>`  | Performs `first == second` and saves `1` to `result` if true or `0` if not                                                            |
| `NEQ <first> <second> <result>` | Performs `first != second` and saves `1` to `result` if true or `0` if not                                                            |
| `MV <source> <destination>`     | Moves the value from `source` to `destination`. Note that you can use an immediate value for `source` if you want to load a constant  |
| `NOT <value> <result>`          | Performs a binary NOT on `value` and saves to the `result` register                                                                   |
| `SHIFTL <value> <result>`       | Shifts the bits of `value` one place to the left. The least significant bit will be filled with `0`                                   |
| `SHIFTR <value> <result>`       | Shifts the bits of `value` one place to the left. The most significant bit will be filled with a copy of the old most significant bit |
| `JMP <address>`                 | Jumps to the given address. Note that you can use an immediate value if you want to jump to a specific location                       |
| `JMPNZ <value> <address>`       | Jumps to the given address if the `value` is anything besides 0                                                                       |
| `LOAD <address> <result>`       | Loads the value at the given `address` in memory to the `result` register                                                             |
| `STORE <address> <value>`       | Stores the given `value` to the `address` in memory                                                                                   |
#pragma once

#include <inttypes.h>

typedef uint32_t Instruction;
typedef uint8_t Opcode;

enum InsOpcode {
    OP_HALT = 0x0,
    OP_PUSH = 0x1,
    OP_POP  = 0x2,
    OP_ADD  = 0x3,
    OP_SUB  = 0x4,
    OP_DIV  = 0x5,
    OP_MUL  = 0x6
};

Instruction create_instruction (Opcode op, uint32_t imm);

/*
 * `prog` is a zero terminated list of instructions. Instruction zero (0) is
 * simply halt. Run the given program.
 */
int run (Instruction *program);

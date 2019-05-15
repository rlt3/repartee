#pragma once

#include <vector>
#include <inttypes.h>

typedef uint32_t Instruction;
typedef uint8_t Opcode;

enum InsOpcode {
    OP_HALT = 0x0,
    OP_PUSH = 0x1, /* push immediate onto stack */
    OP_POP  = 0x2, /* pop top of stack */
    OP_ADD  = 0x3, /* pop n values off stack, add them, and push result */
    OP_SUB  = 0x4, /* pop n values off stack, subtract them, and push result */
    OP_DIV  = 0x5, /* pop n values off stack, divide them, and push result */
    OP_MUL  = 0x6, /* pop n values off stack, multiply them, and push result */
    OP_LOAD = 0x7, /* get value at index and push it onto the stack */
    OP_STORE = 0x8 /* pop value off stack and load it into index */
};

Instruction create_instruction (Opcode op, int32_t imm);

int run (std::vector<Instruction> program);

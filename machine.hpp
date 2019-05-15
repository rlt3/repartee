#pragma once

#include <vector>
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

Instruction create_instruction (Opcode op, int32_t imm);

int run (std::vector<Instruction> program);

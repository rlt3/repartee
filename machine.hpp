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
    OP_STORE = 0x8, /* pop value off stack and load it into index */
    OP_CMP  = 0x9,  /* pop 2 values off stack, compare them and push truth */
    OP_J    = 0xa, /* set pc = to addr */
    OP_JEZ = 0xb, /* compare popped to 0, jump to code addr if true */
    OP_JNZ = 0xc, /* compare popped to 0, jump to code addr if false */
    OP_JGZ = 0xd, /* compare if popped >0, jump to code addr if true */
    OP_JLZ = 0xe, /* compare if popped <0, jump to code addr if true */
    OP_DUP = 0xf, /* duplicate the value on top of the stack, pushing it */
};

Instruction create_instruction (Opcode op, int32_t imm);

int run (std::vector<Instruction> program);

void print_bytecode (std::vector<Instruction> program);

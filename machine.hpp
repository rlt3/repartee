#pragma once

#include <vector>
#include <inttypes.h>

typedef uint32_t Instruction;
typedef uint8_t Opcode;

enum InsOpcode {
    OP_HALT = 0x00,
    OP_PUSH = 0x01, /* push immediate onto stack */
    OP_POP  = 0x02, /* pop top of stack */
    OP_ADD  = 0x03, /* pop n values off stack, add them, and push result */
    OP_SUB  = 0x04, /* pop n values off stack, subtract them, and push result */
    OP_DIV  = 0x05, /* pop n values off stack, divide them, and push result */
    OP_MUL  = 0x06, /* pop n values off stack, multiply them, and push result */
    OP_LOAD = 0x07, /* get value at index and push it onto the stack */
    OP_STORE = 0x08, /* pop value off stack and load it into index */
    OP_CMP  = 0x09,  /* pop 2 values off stack, compare them and push truth */
    OP_J    = 0x0a, /* set pc = to addr */
    OP_JEZ = 0x0b, /* compare popped to 0, jump to code addr if true */
    OP_JNZ = 0x0c, /* compare popped to 0, jump to code addr if false */
    OP_JGZ = 0x0d, /* compare if popped >0, jump to code addr if true */
    OP_JLZ = 0x0e, /* compare if popped <0, jump to code addr if true */
    OP_DUP = 0x0f, /* duplicate the value on top of the stack, pushing it */
    OP_PUSH_LOCAL = 0x10, /* push a local value to the stack */
};

Instruction create_instruction (Opcode op, int32_t imm);

int run (std::vector<Instruction> program);

void print_bytecode (std::vector<Instruction> program);

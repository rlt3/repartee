#include <cstdlib>
#include <cstdio>
#include "machine.hpp"

#define STACK_DEPTH 250

static uint32_t stack[STACK_DEPTH] = {0};

Instruction
create_instruction (Opcode op, uint32_t imm)
{
    /* clear out top byte of immediate value so opcode can be placed there */
    return (imm & 0xFFFFFF) | (op << 24);
}

Opcode
get_opcode (Instruction ins)
{
    return (ins >> 24) & 0xFF;
}

int
run (Instruction *program)
{
    while (*program) {
        switch (get_opcode(*program)) {
            case OP_HALT:
                printf("halt\n");
                return 0;

            case OP_PUSH:
                printf("push\n");
                break;

            case OP_POP:
                printf("pop\n");
                break;

            case OP_ADD:
                printf("add\n");
                break;

            case OP_SUB:
                printf("sub\n");
                break;

            case OP_DIV:
                printf("div\n");
                break;

            case OP_MUL:
                printf("mul\n");
                break;

            default:
                printf("bad instruction. halting\n");
                return 0;
        }
        program++;
    }
    return 0;
}

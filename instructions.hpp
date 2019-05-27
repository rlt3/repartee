#pragma once

#include <inttypes.h>

typedef uint32_t Instruction;
typedef uint8_t Opcode;

enum InsOpcode {
    OP_HALT   = 0x00,
    OP_PUSHC  = 0x01, /* push constant at constant index onto stack */
    OP_POP    = 0x02, /* pop top of stack */
    OP_ADD    = 0x03, /* pop n values off stack, add them, and push result */
    OP_SUB    = 0x04, /* pop n values off stack, subtract them, and push result */
    OP_DIV    = 0x05, /* pop n values off stack, divide them, and push result */
    OP_MUL    = 0x06, /* pop n values off stack, multiply them, and push result */
    OP_LLOAD  = 0x07, /* push local's value onto stack */
    OP_LSTORE = 0x08, /* store top of stack into local */
    OP_CMPEQ  = 0x09, /* compare top 2 push 1 if eq 0 if not */
    OP_CMPNE  = 0x0a, /* compare top 2 push 1 if !eq 0 if not */
    OP_CMPLT  = 0x0b, /* compare top 2 push 1 if < 0 if not */
    OP_CMPGT  = 0x0c, /* compare top 2 push 1 if > 0 if not */
    OP_IFEQ   = 0x0d, /* jump to addr if top of stack is true */
    OP_IFNE   = 0x0e, /* jump to addr if top of stack is false */
    OP_JMP    = 0x0f  /* jump to addr */
};

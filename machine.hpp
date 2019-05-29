#pragma once

#include <iostream>
#include "instructions.hpp"
#include "expression.hpp"

struct MachineContext {
    MachineContext (uint32_t m, int8_t i, int32_t *s, int32_t f,
                    int32_t r, int32_t p, int32_t a, int32_t b)
        : max_stack(m), stack_index(i), stack(s)
        , fp(f), ra(r), pc(p), reg_a(a), reg_b(b)
    { }

    const uint32_t max_stack;
    const uint8_t stack_index;
    /* TODO should probably have copy of the entire stack */
    const int32_t *stack;
    const uint32_t fp;    /* frame pointer */
    const uint32_t ra;    /* return address */
    const uint32_t pc;    /* program counter */
    const int32_t reg_a;  /* gen purpose register */
    const int32_t reg_b;  /* gen purpose register */
};

Instruction create_instruction (Opcode op, int32_t imm);
Instruction create_instruction (Opcode op);
Opcode get_opcode (Instruction ins);
int32_t get_imm (Instruction ins);

/*
 * Evaluate the given expression in the environment and print to output stream.
 */
void evaluate (Expression &expr, std::ostream &output);

/*
 * Get the Machine's context for debugging purposes.
 */
MachineContext get_context ();

/*
 * Resume execution from the given context.
 */
void resume (MachineContext ctx, Expression &expr, std::ostream &output);

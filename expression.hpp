#pragma once

#include "instructions.hpp"
#include <vector>

/*
 * An Expression is anything executed or evaluated by the machine. It is
 * essentially an instruction builder. An Expression is built by walking the
 * AST. After building the AST an expression is locked so it cannot be further
 * mutated.
 *
 * An expression may be as simple as querying the environment, e.g. "> foo;"
 * or as complex as a function definition. An expression abstracts the details
 * of getting the machine to 'produce' values.
 */
class Expression {
public:
    Expression ();

    /* push value of constant or local onto the stack */
    void push_constant (unsigned const_index);
    void load_local (unsigned local_index);

    void binary_op (int type);
    void unary_op (int type);

    /* less-than, greater-than, equal, not equal */
    void cmp_lt ();
    void cmp_gt ();
    void cmp_eq ();
    void cmp_ne ();

    /* conditional jumps */
    void ifeq (unsigned addr);
    void ifneq (unsigned addr);

    /* unconditional jump */
    void jmp (unsigned addr);

    void lock ();

protected:
    bool locked;
    std::vector<Instruction> bytecode;
};

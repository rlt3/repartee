#pragma once

#include "instructions.hpp"
#include <vector>
#include <utility>
#include <unordered_map>

enum BinOps {
    BIN_ADD,
    BIN_MUL,
    BIN_SUB,
    BIN_DIV,
    BIN_CMP_LT,
    BIN_CMP_GT,
    BIN_CMP_EQ,
    BIN_CMP_NE
};

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

    /* setup a local on the stack and return its index */
    unsigned add_local ();

    /* push value of constant value onto the stack */
    void push_constant (int value);
    void push_constant (float value);

    /* push the value of the local at the stack index onto stack */
    void load_local (unsigned stack_index);

    /* do the given binary or unary operations */
    void binary_op (enum BinOps type);
    void unary_op (int type);

    /* conditional jumps */
    void ifeq (int addr);
    void ifneq (int addr);

    /* unconditional jump */
    void jmp (int addr);

    /* finalize the expression for evaluation */
    void finish ();

    std::vector<Instruction> code () const;
    unsigned entry () const;

protected:
    /* add a constant and produce a backpatch for current instruction */
    void add_constant (int32_t value);

    /* write all constants and patch with their relative addresses */
    void patch_constants (std::vector<Instruction> &code);

    bool is_finished;
    unsigned entry_index;
    std::vector<Instruction> bytecode;
    std::vector<Instruction> locals;
    std::unordered_map<int32_t, unsigned> constants;
    std::vector<std::pair<unsigned, unsigned>> constant_bp;
};

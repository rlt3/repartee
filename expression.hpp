#pragma once

#include "instructions.hpp"
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

enum BinOps {
    BIN_ADD,
    BIN_MUL,
    BIN_SUB,
    BIN_DIV,
    BIN_CMPLT,
    BIN_CMPGT,
    BIN_CMPEQ,
    BIN_CMPNE
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

    /* push value of constant value onto the stack */
    void push_constant (int value);
    void push_constant (float value);

    /* push the value of the local at the stack index onto stack */
    void load_local (std::string name);

    /* store the value on the stack into the local */
    void store_local (std::string name);

    /* integer arithmetic */
    void addi ();
    void subi ();
    void muli ();
    void divi ();

    /* comparisons */
    void cmplt ();
    void cmpgt ();
    void cmpeq ();
    void cmpne ();

    /* conditional jumps */
    void ifeq (int addr);
    void ifneq (int addr);

    /* unconditional jump */
    void jmp (int addr);

    /* finalize the expression for evaluation */
    void finish ();

    /* get the stack index for the local */
    unsigned get_local (std::string name) const;

    /* return final generated code */
    std::vector<Instruction> code () const;

    /* get the entry point of the generated code */
    unsigned entry () const;

protected:
    /* add a local if it doesn't exist otherwise get it */
    unsigned add_or_get_local (std::string name);

    /* add a constant and produce a backpatch for current instruction */
    void create_constant_backpatch (int32_t value);

    /* write all constants and patch with their relative addresses */
    void patch_constants (std::vector<Instruction> &code);

    /* write setup instructions for locals */
    void write_locals (std::vector<Instruction> &code);

    bool is_finished;

    /* entry point or first instruction of the expression */
    unsigned entry_index;

    unsigned num_locals;
    std::unordered_map<std::string, unsigned> locals;

    /* constants and their backpatches */
    std::unordered_map<int32_t, unsigned> constants;
    std::vector<std::pair<unsigned, unsigned>> constant_bp;

    std::vector<Instruction> bytecode;
};

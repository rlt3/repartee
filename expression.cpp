#include "error.hpp"
#include "machine.hpp"
#include "expression.hpp"

Expression::Expression ()
    : is_finished(false), entry_index(0)
{ }

/* setup a local on the stack and return its index */
unsigned
Expression::add_local ()
{
    assert(!is_finished);
    locals.push_back(create_instruction(OP_SETL));
    return locals.size();
}

/* push value of constant from addr onto the stack */
void
Expression::push_constant (int value)
{
    assert(!is_finished);
    add_constant(value);
}

void
Expression::push_constant (float value)
{
    assert(!is_finished);
    /* 
     * Need to keep the binary representation of the floating point the same
     * while in the 32bit container. This sidesteps implicit conversion to an
     * integer which alters the binary representation.
     */
    add_constant(*reinterpret_cast<int32_t*>(&value));
}

/* push the value of the local at the stack index onto stack */
void
Expression::load_local (unsigned stack_index)
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_LOADL, stack_index));
}

/* do the given binary operation */
void
Expression::binary_op (enum BinOps type)
{
    assert(!is_finished);
    Opcode op;
    switch (type) {
        case BIN_ADD:    op = OP_ADD;   break;
        case BIN_MUL:    op = OP_MUL;   break;
        case BIN_SUB:    op = OP_SUB;   break;
        case BIN_DIV:    op = OP_DIV;   break;
        case BIN_CMP_EQ: op = OP_CMPEQ; break;
        case BIN_CMP_NE: op = OP_CMPNE; break;
        case BIN_CMP_LT: op = OP_CMPGT; break;
        case BIN_CMP_GT: op = OP_CMPLT; break;
    }
    bytecode.push_back(create_instruction(op));
}

/* do the given unary operation */
void
Expression::unary_op (int type)
{
    assert(!is_finished);
    /* todo */
}

/* conditional jumps */
void
Expression::ifeq (int addr)
{
    assert(!is_finished);
    /* todo */
}

void
Expression::ifneq (int addr)
{
    assert(!is_finished);
    /* todo */
}

/* unconditional jump */
void
Expression::jmp (int addr)
{
    assert(!is_finished);
    /* todo */
}

/* add a constant and produce a backpatch for current instruction */
void
Expression::add_constant (int32_t value)
{
    /* add constant if it doesn't already exist */
    if (constants.find(value) == constants.end())
        constants[value] = 0;

    /* make a back patching record */
    constant_bp.push_back(std::make_pair(bytecode.size(), value));
    bytecode.push_back(OP_HALT); /* placeholder */
}

/* write all constants and patch with their relative addresses */
void
Expression::patch_constants (std::vector<Instruction> &code)
{
    /* Push all constants and set the value of each as its location */
    for (auto &p : constants) {
        p.second = code.size(); /* always returns length + 1 */
        code.push_back(p.first);
    }

    for (auto p : constant_bp) {
        /* 
         * Now that all constants have been pushed we can determine the relative
         * addresses of each push constant call.
         */
        int push_addr = p.first;
        int const_addr = constants[p.second];
        int reladdr = -(push_addr + (signed)constants.size() - const_addr);
        //printf("-(%d + %d - %d) = %d\n", push_addr, (signed)constants.size(), const_addr, reladdr);
        bytecode[push_addr] = create_instruction(OP_PUSHC, reladdr);
    }
}

/* finalize the expression for evaluation */
void
Expression::finish ()
{
    std::vector<Instruction> code;

    bytecode.push_back(OP_HALT);

    /* Push all constants first so they can be more easily referenced */
    patch_constants(code);

    /* At this point instructions are given so the entry point is here */
    entry_index = code.size();
    code.insert(code.end(), locals.begin(), locals.end());
    code.insert(code.end(), bytecode.begin(), bytecode.end());

    bytecode = code;
    is_finished = true;
}

std::vector<Instruction>
Expression::code () const
{
    assert(is_finished);
    return bytecode;
}

unsigned
Expression::entry () const
{
    assert(is_finished);
    return entry_index;
}

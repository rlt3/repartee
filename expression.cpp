#include "error.hpp"
#include "machine.hpp"
#include "expression.hpp"

Expression::Expression ()
    : is_finished(false), entry_index(0), num_locals(0)
{ }

/* push value of constant from addr onto the stack */
void
Expression::push_constant (int value)
{
    assert(!is_finished);
    create_constant_backpatch(value);
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
    create_constant_backpatch(*reinterpret_cast<int32_t*>(&value));
}

/* push the value of the local at the stack index onto stack */
void
Expression::load_local (std::string name)
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_LOADL, add_or_get_local(name)));
}

/* store the value on the stack into the local */
void
Expression::store_local (std::string name)
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_STOREL, add_or_get_local(name)));
}

void
Expression::addi ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_ADD));
}

void
Expression::subi ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_SUB));
}

void
Expression::muli ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_MUL));
}

void
Expression::divi ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_DIV));
}

void
Expression::cmplt ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_CMPLT));
}

void
Expression::cmpgt ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_CMPGT));
}

void
Expression::cmpeq ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_CMPEQ));
}

void
Expression::cmpne ()
{
    assert(!is_finished);
    bytecode.push_back(create_instruction(OP_CMPNE));
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
    /* Setup the storage needed for locals */
    write_locals(code);
    /* Finally, append the code */
    code.insert(code.end(), bytecode.begin(), bytecode.end());

    bytecode = code;
    is_finished = true;
}

/* setup a local on the stack and return its index */
unsigned
Expression::get_local (std::string name) const
{
    assert(locals.count(name));
    return locals.at(name);
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

/* setup a local on the stack and return its index */
unsigned
Expression::add_or_get_local (std::string name)
{
    if (locals.find(name) == locals.end()) {
        locals[name] = num_locals++;
        return num_locals - 1;
    }
    return locals.at(name);
}

void
Expression::write_locals (std::vector<Instruction> &code)
{
    /* Right now all types have the same storage creating instruction */
    for (unsigned i = 0; i < locals.size(); i++)
        code.push_back(create_instruction(OP_SETL));
}

/* add a constant and produce a backpatch for current instruction */
void
Expression::create_constant_backpatch (int32_t value)
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

    /* total amout of constants and local setup before executable code */
    unsigned header_size = constants.size() + locals.size();

    for (auto p : constant_bp) {
        /* 
         * Now that all constants have been pushed we can determine the relative
         * addresses of each push constant call.
         */
        int push_addr = p.first;
        int const_addr = constants[p.second];
        int reladdr = -(push_addr + header_size - const_addr);
        bytecode[push_addr] = create_instruction(OP_PUSHC, reladdr);
    }
}

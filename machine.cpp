#include "error.hpp"
#include "machine.hpp"

/*
 * Right now instructions are:
 * 31-24: Opcode
 * 23-0: Immediate Value
 * The 23rd bit declares whether the immediate is negative and sign extends
 * into the 32nd bit on extractions.
 */

Instruction
create_instruction (Opcode op)
{
    return (op << 24);
}

Instruction
create_instruction (Opcode op, int32_t imm)
{
    return (imm & 0xFFFFFF) | (op << 24);
}

Opcode
get_opcode (Instruction ins)
{
    return (ins >> 24) & 0xFF;
}

int32_t
get_imm (Instruction ins)
{
    /* get immediate value and sign extend it */
    return (((int32_t) ins & 0xFFFFFF) << 8) >> 8;
}

#define DEBUG true
#define STACK_DEPTH 250
#define MEM_DEPTH 250

static uint8_t stack_index = 0;
static int32_t stack[STACK_DEPTH] = {0};

void
stack_push (int32_t val)
{
    if (stack_index >= STACK_DEPTH)
        panic("stack overflow\n");
    stack[stack_index] = val;
    stack_index++;
    if (DEBUG)
        printf("  pushed: %d\n", val);
}

int32_t
stack_pop ()
{
    /* 
     * TODO: use numbers that are out of bounds of regular integers as error
     * codes, i.e. numbers that have bits set in the opcode areas.
     */
    if (stack_index == 0)
        panic("stack underflow\n");
    int32_t val = stack[stack_index - 1];
    stack_index--;
    if (DEBUG)
        printf("  popped: %d\n", val);
    return val;
}

void
handle_cmp (Opcode type)
{
    int32_t a, b;

    switch (type) {
        case OP_CMPEQ:
            if (DEBUG) printf("cmpeq\n");
            b = stack_pop();
            a = stack_pop();
            stack_push((a == b));
            break;

        case OP_CMPNE:
            if (DEBUG) printf("cmpne\n");
            b = stack_pop();
            a = stack_pop();
            stack_push((a != b));
            break;

        case OP_CMPGT:
            if (DEBUG) printf("cmpgt\n");
            b = stack_pop();
            a = stack_pop();
            stack_push((a > b));
            break;

        case OP_CMPLT:
            if (DEBUG) printf("cmplt\n");
            b = stack_pop();
            a = stack_pop();
            stack_push((a < b));
            break;

        default:
            break;
    }
}

void
handle_arithmetic (Opcode type)
{
    int32_t a, b;

    switch (type) {
        case OP_ADD:
            if (DEBUG) printf("add\n");
            b = stack_pop();
            a = stack_pop();
            a = a + b;
            break;

        case OP_SUB:
            if (DEBUG) printf("sub\n");
            b = stack_pop();
            a = stack_pop();
            a = a - b;
            break;

        case OP_DIV:
            if (DEBUG) printf("div\n");
            b = stack_pop();
            a = stack_pop();
            a = a / b;
            break;

        case OP_MUL:
            if (DEBUG) printf("mul\n");
            b = stack_pop();
            a = stack_pop();
            a = a * b;
            break;

        default:
            break;
    }

    stack_push(a);
}

/*
 * Evaluate the given expression in the environment and print to output stream.
 */
void
evaluate (Expression &expr, std::ostream &output)
{
    std::vector<Instruction> prog = expr.code();
    Instruction instruction;
    unsigned long pc = expr.entry();
    Opcode op;
    int imm;

    while (true) {
        instruction = prog[pc++];
        op = get_opcode(instruction);
        imm = get_imm(instruction);

        switch (op) {
            case OP_HALT:
                if (DEBUG) printf("halt\n");
                goto exit;

            case OP_PUSHC:
                if (DEBUG) printf("pushc %d\n", imm);
                stack_push(prog[pc - 1 + imm]);
                break;

            case OP_POP:
                if (DEBUG) printf("pop %d\n", imm);
                stack_pop();
                break;

            case OP_ADD:
            case OP_SUB:
            case OP_DIV:
            case OP_MUL:
                handle_arithmetic(op);
                break;

            /*
             * TODO:
             * When I make the Machine symbolic, use this instruction to setup
             * the Environment (prematurely removed) to hold values between
             * execution of expressions. Since the values in the Machine will
             * all be Symbols then they will be easily updated or changed
             * arbitrarily. Thie effectively allows:
             *
             * > int a = 5;
             * OK
             * > a;
             * 5
             *
             * Whereas now, the Machine running on binary, this is impossible.
             */
            case OP_SETL:
                if (DEBUG) printf("setup local\n");
                stack_push(0);
                break;

            case OP_LOADL:
                if (DEBUG) printf("loadl %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    panic("segmentation fault\n");
                stack_push(stack[imm]);
                break;

            case OP_STOREL:
                if (DEBUG) printf("storel %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    panic("segmentation fault\n");
                stack[imm] = stack_pop();
                break;

            case OP_CMPEQ:
            case OP_CMPNE:
            case OP_CMPLT:
            case OP_CMPGT:
                handle_cmp(op);
                break;

            case OP_JMP:
                if (DEBUG) printf("j %d\n", imm);
                pc = pc - 1 + imm;
                break;

            //case OP_DUP:
            //    if (DEBUG) printf("dup\n");
            //    stack_push(stack[stack_index - 1]);
            //    break;

            default:
                panic("illegal instruction %d\n", op);
                break;
        }
    }

exit:
    if (stack_index > 0) {
        output << stack_pop() << std::endl;
    } else {
        output << "OK\n";
    }
}

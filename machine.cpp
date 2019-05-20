#include <cstdlib>
#include <cstdio>
#include "machine.hpp"

#define DEBUG false

#define STACK_DEPTH 250
#define MEM_DEPTH 250

static uint8_t stack_index = 0;
static int32_t stack[STACK_DEPTH] = {0};
static int32_t mem[MEM_DEPTH] = {0};

void
machine_error (const char *err)
{
    fputs(err, stderr);
    exit(1);
}

void
stack_push (uint32_t val)
{
    if (stack_index >= STACK_DEPTH)
        machine_error("stack overflow\n");
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
        machine_error("stack underflow\n");
    int32_t val = stack[stack_index - 1];
    stack_index--;
    if (DEBUG)
        printf("  popped: %d\n", val);
    return val;
}

Instruction
create_instruction (Opcode op, int32_t imm)
{
    /* clear out top byte of immediate value so opcode can be placed there */
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
    return ins & 0xFFFFFF;
}

void
handle_arithmetic (Opcode type, int num)
{
    int a, b;

    /* 
     * two is the minimum number of operands for arithmetic operations but can
     * handle more operands.
     */
    b = stack_pop();
    num--;
    do {
        a = stack_pop();
        num--;
        switch (type) {
            case OP_ADD: b = a + b; break;
            case OP_SUB: b = a - b; break;
            case OP_MUL: b = a * b; break;
            case OP_DIV: b = a / b; break;
            default: break;
        }
    } while (num > 0);

    stack_push(b);
}

void
handle_cmp ()
{
    int a = stack_pop();
    int b = stack_pop();
    stack_push((a == b));
}

int
run (std::vector<Instruction> program)
{
    int imm;

    for (auto instruction : program) {
        imm = get_imm(instruction);

        switch (get_opcode(instruction)) {
            case OP_HALT:
                if (DEBUG) printf("halt %d\n", imm);
                return 0;

            case OP_PUSH:
                if (DEBUG) printf("push %d\n", imm);
                stack_push(imm);
                break;

            case OP_POP:
                if (DEBUG) printf("pop %d\n", imm);
                stack_pop();
                break;

            case OP_ADD:
                if (DEBUG) printf("add %d\n", imm);
                handle_arithmetic(OP_ADD, imm);
                break;

            case OP_SUB:
                if (DEBUG) printf("sub %d\n", imm);
                handle_arithmetic(OP_SUB, imm);
                break;

            case OP_DIV:
                if (DEBUG) printf("div %d\n", imm);
                handle_arithmetic(OP_DIV, imm);
                break;

            case OP_MUL:
                if (DEBUG) printf("mul %d\n", imm);
                handle_arithmetic(OP_MUL, imm);
                break;

            case OP_LOAD:
                if (DEBUG) printf("load %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    machine_error("segmentation fault\n");
                stack_push(mem[imm]);
                break;

            case OP_STORE:
                if (DEBUG) printf("store %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    machine_error("segmentation fault\n");
                mem[imm] = stack_pop();
                break;

            case OP_CMP:
                if (DEBUG) printf("cmp\n");
                handle_cmp();
                break;

            case OP_J:
                if (DEBUG) printf("j %d\n", imm);
                break;

            case OP_JEZ:
                if (DEBUG) printf("jez %d\n", imm);
                break;

            case OP_JNZ:
                if (DEBUG) printf("jnz %d\n", imm);
                break;

            case OP_JGZ:
                if (DEBUG) printf("jgz %d\n", imm);
                break;

            case OP_JLZ:
                if (DEBUG) printf("jlz %d\n", imm);
                break;

            default:
                machine_error("bad instruction. halting\n");
                return 0;
        }
    }

    return stack_pop();
}

void
print_bytecode (std::vector<Instruction> program)
{
    unsigned long i = 0;
    int imm;

    for (auto instruction : program) {
        printf("%lu: ", i++);
        imm = get_imm(instruction);

        switch (get_opcode(instruction)) {
            case OP_HALT:
                printf("halt %d\n", imm);
                return;

            case OP_PUSH:
                printf("push %d\n", imm);
                break;

            case OP_POP:
                printf("pop %d\n", imm);
                break;

            case OP_ADD:
                printf("add %d\n", imm);
                break;

            case OP_SUB:
                printf("sub %d\n", imm);
                break;

            case OP_DIV:
                printf("div %d\n", imm);
                break;

            case OP_MUL:
                printf("mul %d\n", imm);
                break;

            case OP_LOAD:
                printf("load %d\n", imm);
                break;

            case OP_STORE:
                printf("store %d\n", imm);
                break;

            case OP_CMP:
                printf("cmp\n");
                break;

            case OP_J:
                printf("j %d\n", imm);
                break;

            case OP_JEZ:
                printf("jez %d\n", imm);
                break;

            case OP_JNZ:
                printf("jnz %d\n", imm);
                break;

            case OP_JGZ:
                printf("jgz %d\n", imm);
                break;

            case OP_JLZ:
                printf("jlz %d\n", imm);
                break;

            default:
                machine_error("bad instruction. halting\n");
                return;
        }
    }
}

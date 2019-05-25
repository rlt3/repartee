#include <cstdlib>
#include <cstdio>
#include "machine.hpp"

#define DEBUG false

#define STACK_DEPTH 250
#define MEM_DEPTH 250

static uint8_t stack_index = 0;
static int32_t stack[STACK_DEPTH] = {0};
//static int32_t mem[MEM_DEPTH] = {0};

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
            case OP_ADD:
                if (DEBUG) printf("add %d\n", num);
                b = a + b;
                break;

            case OP_SUB:
                if (DEBUG) printf("sub %d\n", num);
                b = a - b;
                break;

            case OP_DIV:
                if (DEBUG) printf("div %d\n", num);
                b = a / b;
                break;

            case OP_MUL:
                if (DEBUG) printf("mul %d\n", num);
                b = a * b;
                break;

            default:
                break;
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

unsigned long
handle_jmp (unsigned long pc, Opcode op, int imm)
{
    int a;
    switch (op) {
            case OP_J:
                if (DEBUG) printf("j %d\n", imm);
                return imm;

            case OP_JEZ:
                if (DEBUG) printf("jez %d\n", imm);
                a = stack_pop();
                if (a == 0)
                    return imm;
                break;

            case OP_JNZ:
                if (DEBUG) printf("jnz %d\n", imm);
                a = stack_pop();
                if (a != 0)
                    return imm;
                break;

            case OP_JGZ:
                if (DEBUG) printf("jgz %d\n", imm);
                a = stack_pop();
                if (a > 0)
                    return imm;
                break;

            case OP_JLZ:
                if (DEBUG) printf("jlz %d\n", imm);
                a = stack_pop();
                if (a < 0)
                    return imm;
                break;

            default:
                machine_error("bad jump");
    }
    return pc;
}

int
run (std::vector<Instruction> program)
{
    Instruction instruction;
    unsigned long pc = 0;
    Opcode op;
    int imm;

    while (true) {
        instruction = program[pc++];
        imm = get_imm(instruction);
        op = get_opcode(instruction);

        switch (op) {
            case OP_HALT:
                if (DEBUG) printf("halt %d\n", imm);
                goto exit;

            case OP_PUSH_LOCAL:
                if (DEBUG) printf("local");
            case OP_PUSH:
                if (DEBUG) printf("push %d\n", imm);
                stack_push(imm);
                break;

            case OP_POP:
                if (DEBUG) printf("pop %d\n", imm);
                stack_pop();
                break;

            case OP_ADD:
            case OP_SUB:
            case OP_DIV:
            case OP_MUL:
                handle_arithmetic(op, imm);
                break;

            case OP_LOAD:
                if (DEBUG) printf("load %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    machine_error("segmentation fault\n");
                stack_push(stack[imm]);
                break;

            case OP_STORE:
                if (DEBUG) printf("store %d\n", imm);
                if (imm < 0 || imm >= MEM_DEPTH)
                    machine_error("segmentation fault\n");
                stack[imm] = stack_pop();
                break;

            case OP_CMP:
                if (DEBUG) printf("cmp\n");
                handle_cmp();
                break;

            case OP_J:
            case OP_JEZ:
            case OP_JNZ:
            case OP_JGZ:
            case OP_JLZ:
                pc = handle_jmp(pc, op, imm);
                break;

            case OP_DUP:
                if (DEBUG) printf("dup\n");
                stack_push(stack[stack_index - 1]);
                break;

            default:
                machine_error("bad instruction. halting\n");
                return 0;
        }
    }

exit:
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

            case OP_PUSH_LOCAL:
                printf("local push %d\n", imm);
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

            case OP_DUP:
                printf("dup\n");
                break;

            default:
                machine_error("bad instruction. halting\n");
                return;
        }
    }
}

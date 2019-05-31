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

/*
 * The Context of the machine at a certain point in time.
 */
struct Context {
    Context (uint32_t max, int8_t idx, int32_t *s,
             int32_t fp, int32_t ra, int32_t pc, int32_t a, int32_t b)
        : max_stack(max), stack_index(idx), stack(s)
        , fp(fp), ra(ra), pc(pc), reg_a(a), reg_b(b)
    { }

    const uint32_t max_stack;
    const uint8_t stack_index;
    const int32_t *stack;
    const uint32_t fp;
    const uint32_t ra;
    const uint32_t pc;
    const int32_t reg_a;
    const int32_t reg_b;
};

#define DEBUG true
#define STACK_MAX 250

/* general purpose registers */
static int32_t A, B; 
/* Program Counter, Return Address, and Frame Pointer */
static uint32_t PC = 0;
static uint32_t RA = 0;
static uint32_t FP = 0;
static uint8_t STACK_INDEX = 0;
static int32_t STACK[STACK_MAX] = {0};

void
stack_push (int32_t val)
{
    if (STACK_INDEX >= STACK_MAX)
        panic("stack overflow\n");
    STACK[STACK_INDEX] = val;
    STACK_INDEX++;
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
    if (STACK_INDEX == 0)
        panic("stack underflow\n");
    int32_t val = STACK[STACK_INDEX - 1];
    STACK_INDEX--;
    if (DEBUG)
        printf("  popped: %d\n", val);
    return val;
}

MachineContext
get_context ()
{
    return MachineContext(STACK_MAX, STACK_INDEX, STACK, FP, RA, PC, A, B);
}

void
resume (MachineContext ctx, Expression &expr, std::ostream &output)
{
    /*
     * TODO: Two entry points to 'run' which is the actually machine's runtime.
     * First is evaluate which sets up the context with respect to the given
     * expression and environment. The second is resume which simply copies
     * the given context and runs from that point.
     */
}

/*
 * Evaluate the given expression in the environment and print to output stream.
 */
void
evaluate (Expression &expr, std::ostream &output)
{
    std::vector<Instruction> prog = expr.code();
    Instruction instruction;
    Opcode op;
    int32_t imm;

    PC = expr.entry();
    FP = STACK_INDEX;
    RA = STACK_INDEX;

    while (true) {
        instruction = prog[PC++];
        op = get_opcode(instruction);
        imm = get_imm(instruction);

        switch (op) {
            case OP_HALT:
                if (DEBUG) printf("halt\n");
                goto exit;

            case OP_PUSHC:
                if (DEBUG) printf("pushc %d\n", imm);
                stack_push(prog[PC - 1 + imm]);
                break;

            case OP_POP:
                if (DEBUG) printf("pop %d\n", imm);
                stack_pop();
                break;

            case OP_CMPEQ:
                if (DEBUG) printf("cmpeq\n");
                B = stack_pop();
                A = stack_pop();
                stack_push((A == B));
                break;

            case OP_CMPNE:
                if (DEBUG) printf("cmpne\n");
                B = stack_pop();
                A = stack_pop();
                stack_push((A != B));
                break;

            case OP_CMPGT:
                if (DEBUG) printf("cmpgt\n");
                B = stack_pop();
                A = stack_pop();
                stack_push((A > B));
                break;

            case OP_CMPLT:
                if (DEBUG) printf("cmplt\n");
                B = stack_pop();
                A = stack_pop();
                stack_push((A < B));
                break;

            case OP_ADDI:
                if (DEBUG) printf("add\n");
                B = stack_pop();
                A = stack_pop();
                stack_push(A + B);
                break;

            case OP_SUBI:
                if (DEBUG) printf("sub\n");
                B = stack_pop();
                A = stack_pop();
                stack_push(A - B);
                break;

            case OP_DIVI:
                if (DEBUG) printf("div\n");
                B = stack_pop();
                A = stack_pop();
                stack_push(A / B);
                break;

            case OP_MULI:
                if (DEBUG) printf("mul\n");
                B = stack_pop();
                A = stack_pop();
                stack_push(A * B);
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
                if (imm < 0 || imm >= STACK_MAX)
                    panic("segmentation fault\n");
                stack_push(STACK[imm]);
                break;

            case OP_STOREL:
                if (DEBUG) printf("storel %d\n", imm);
                if (imm < 0 || imm >= STACK_MAX)
                    panic("segmentation fault\n");
                STACK[imm] = stack_pop();
                break;

            case OP_JMP:
                if (DEBUG) printf("j %d\n", imm);
                PC = PC - 1 + imm;
                break;

            //case OP_DUP:
            //    if (DEBUG) printf("dup\n");
            //    stack_push(STACK[STACK_INDEX - 1]);
            //    break;

            default:
                panic("illegal instruction %d\n", op);
                break;
        }
    }

exit:
    if (STACK_INDEX > 0) {
        output << stack_pop() << std::endl;
    } else {
        output << "OK\n";
    }
}

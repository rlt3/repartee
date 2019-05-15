#include <sstream>
#include "parser.hpp"

void 
error_func (int error_type, Token t, int data)
{
    std::stringstream s;

    switch (error_type) {
        case TKNZR_MATCH:
            s << "Expected " << tokentype_to_str(data);
            s << " instead got `" << t.str << "'";
            s << " on line " << t.line << " column " << t.column;
            break;
        default:
            break;
    }

    fprintf(stderr, "%s\n", s.str().c_str());
    exit(1);
}

void expr (TokenizedInput &T, std::vector<Instruction> &prog);

int
atom (TokenizedInput &T)
{
    return T.expect(TKN_NUMBER).to_int();
}

void
item (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        expr(T, prog);
        T.expect(TKN_RIGHT_PAREN);
    }
    else {
        prog.push_back(create_instruction(OP_PUSH, atom(T)));
    }
}

void
term (TokenizedInput &T, std::vector<Instruction> &prog)
{
    /* <term> = <item> <term> | <item> */
    item(T, prog);

    /* <termTail> = * <atom> <termTail> | / <atom> <termTail> */
    while (!T.empty()) {
        switch (T.peek().type) {
            case TKN_MUL:
                T.next();
                item(T, prog);
                prog.push_back(create_instruction(OP_MUL, 2));
                break;

            case TKN_DIV:
                T.next();
                item(T, prog);
                prog.push_back(create_instruction(OP_DIV, 2));
                break;

            default:
                return;
        }
    }
}

void
expr (TokenizedInput &T, std::vector<Instruction> &prog)
{
    /* <expr> = <term> | <term> <exprTail> */
    term(T, prog);

    /* <exprTail> = + <term> <exprTail> | - <term> <exprTail> */
    while (!T.empty()) {
        switch (T.peek().type) {
            case TKN_ADD:
                T.next();
                term(T, prog);
                prog.push_back(create_instruction(OP_ADD, 2));
                break;

            case TKN_SUB:
                T.next();
                term(T, prog);
                prog.push_back(create_instruction(OP_SUB, 2));
                break;

            default:
                return;
        }
    }
}

std::vector<Instruction>
parse (TokenizedInput &T)
{
    std::vector<Instruction> prog;
    T.set_runtime_error_func(error_func);
    expr(T, prog);
    return prog;
}

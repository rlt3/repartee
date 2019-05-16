#include <sstream>
#include <map>
#include "parser.hpp"

/* 
 * Very hacky implementation of an environment for holding where variables
 * are stored.
 */
static int num_vars = 0;
static std::map<std::string, int> var_to_id;

/* create id for the var if it's not already in our map, otherwise return it */
int
add_or_get_var_id (std::string var)
{
    if (var_to_id.find(var) != var_to_id.end()) {
        return var_to_id[var];
    }
    num_vars++;
    var_to_id[var] = num_vars;
    return num_vars;
}

void 
error_func (int error_type, Token t, int data)
{
    std::stringstream s;

    switch (error_type) {
        case TKNZR_MATCH:
            s << "Expected `" << tokentype_to_str(data) << "'";
            s << " instead got `" << t.str << "'";
            s << " on line " << t.line << " column " << t.column;
            break;
        default:
            break;
    }

    fprintf(stderr, "%s\n", s.str().c_str());
    exit(1);
}

void arithmetic (TokenizedInput &T, std::vector<Instruction> &prog);

/* <atom> = number | name */
void
atom (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_NUMBER) {
        int num = T.expect(TKN_NUMBER).to_int();
        prog.push_back(create_instruction(OP_PUSH, num));
    }
    else {
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        prog.push_back(create_instruction(OP_LOAD, var_id));
    }
}

void
item (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        arithmetic(T, prog);
        T.expect(TKN_RIGHT_PAREN);
    }
    else {
        atom(T, prog);
    }
}

void
term (TokenizedInput &T, std::vector<Instruction> &prog)
{
    /* <term> := <item> <term> | <item> */
    item(T, prog);

    /* <termTail> := * <atom> <termTail> | / <atom> <termTail> */
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
arithmetic (TokenizedInput &T, std::vector<Instruction> &prog)
{
    /* <arithmetic> := <term> | <term> <arithTail> */
    term(T, prog);

    /* <arithTail> := + <term> <arithTail> | - <term> <arithTail> */
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

/* <expr> := <name> = <arithmetic> | <arithmetic> */
void
expr (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_NAME && T.peek(1).type == TKN_EQUAL) {
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        T.expect(TKN_EQUAL);
        arithmetic(T, prog);
        prog.push_back(create_instruction(OP_STORE, var_id));
    }
    else {
        arithmetic(T, prog);
    }
}

std::vector<Instruction>
parse (TokenizedInput &T)
{
    std::vector<Instruction> prog;
    T.set_runtime_error_func(error_func);
    while (!T.empty()) {
        expr(T, prog);
        T.expect(TKN_SEMICOLON);
    }
    return prog;
}

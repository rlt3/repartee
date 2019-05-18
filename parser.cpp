#include <sstream>
#include <map>
#include "environment.hpp"
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

/*
 * TODO: Can be a general error function with a void pointer data type that can
 * be casted to a particular type since all references will exist on a stack at
 * error time. Error types then can be defined in a single place as well.
 */
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

void comp (TokenizedInput &T, std::vector<Instruction> &prog);

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

/* <item> = (<comp>) | <atom> */
void
item (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        comp(T, prog);
        T.expect(TKN_RIGHT_PAREN);
    }
    else {
        atom(T, prog);
    }
}

/* 
 * <factor> := <item> <factorTail> | <item> 
 * <factorTail> := * <atom> <factorTail> | / <atom> <factorTail>
 */
void
factor (TokenizedInput &T, std::vector<Instruction> &prog)
{
    item(T, prog);

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

/*
 * <sum> := <factor> | <factor> <sumTail>
 * <sumTail> := + <factor> <sumTail> | - <factor> <sumTail>
 */
void
sum (TokenizedInput &T, std::vector<Instruction> &prog)
{
    factor(T, prog);

    while (!T.empty()) {
        switch (T.peek().type) {
            case TKN_ADD:
                T.next();
                factor(T, prog);
                prog.push_back(create_instruction(OP_ADD, 2));
                break;

            case TKN_SUB:
                T.next();
                factor(T, prog);
                prog.push_back(create_instruction(OP_SUB, 2));
                break;

            default:
                return;
        }
    }
}

/* <comp> := <sum> == <sum> | <sum> */
void
comp (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek(1).type == TKN_EQUAL && T.peek(2).type == TKN_EQUAL) {
        sum(T, prog);
        T.expect(TKN_EQUAL);
        T.expect(TKN_EQUAL);
        sum(T, prog);
        prog.push_back(create_instruction(OP_CMP, 0));
    } else {
        sum(T, prog);
    }
}

/* <expr> := <name> = <comp> | <comp> */
void
expr (TokenizedInput &T, std::vector<Instruction> &prog)
{
    if (T.peek().type == TKN_NAME &&
        T.peek(1).type == TKN_EQUAL &&
        T.peek(2).type != TKN_EQUAL)
    {
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        T.expect(TKN_EQUAL);
        comp(T, prog);
        prog.push_back(create_instruction(OP_STORE, var_id));
    }
    else {
        comp(T, prog);
    }
}

std::vector<Instruction>
parse (TokenizedInput &T)
{
    std::vector<Instruction> prog;
    Environment E(NULL);

    T.set_runtime_error_func(error_func);

    while (!T.empty()) {
        expr(T, prog);
        T.expect(TKN_SEMICOLON);
    }
    return prog;
}

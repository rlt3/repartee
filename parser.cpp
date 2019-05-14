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

int
atom (TokenizedInput &T)
{
    return T.expect(TKN_NUMBER).to_int();
}

int
term (TokenizedInput &T)
{
    /* <term> = <atom> <term> | <atom> */
    int num = atom(T);

    /* <termTail> = * <atom> <termTail> | / <atom> <termTail> */
    while (!T.empty()) {
        switch (T.peek().type) {
            case TKN_MUL:
                T.next();
                num *= atom(T);
                break;

            case TKN_DIV:
                T.next();
                num /= atom(T);
                break;

            default:
                return num;
        }
    }

    return num;
}

int
expr (TokenizedInput &T)
{
    /* <expr> = <term> | <term> <exprTail> */
    int num = term(T);

    /* <exprTail> = + <term> <exprTail> | - <term> <exprTail> */
    while (!T.empty()) {
        switch (T.peek().type) {
            case TKN_ADD:
                T.next();
                num += term(T);
                break;

            case TKN_SUB:
                T.next();
                num -= term(T);
                break;

            default:
                return num;
        }
    }

    return num;
}

void
parse (TokenizedInput &T)
{
    T.set_runtime_error_func(error_func);
    printf("%d\n", expr(T));
}

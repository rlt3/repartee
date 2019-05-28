#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "environment.hpp"
#include "symbol.hpp"
#include "error.hpp"
#include "machine.hpp"

void
eval (std::istream &input, std::ostream &output)
{
    /*
     * + Tokenize input stream
     *   - bad characters
     * + Parse tokens into AST
     *   - malformed input, invalid sequences
     * + Produce expression (bytecode) from AST
     *   - undefined symbols, invalid types
     * + Evaluate the expression
     */
}

int
main (void)
{
    Expression expr;

    /* 5 + 20 * 4 / 5 */
    expr.push_constant(5);
    expr.push_constant(20);
    expr.binary_op(BIN_ADD);
    expr.push_constant(4);
    expr.binary_op(BIN_MUL);
    expr.push_constant(5);
    expr.binary_op(BIN_DIV);
    expr.finish();

    evaluate(expr, std::cout);

    return 0;
}

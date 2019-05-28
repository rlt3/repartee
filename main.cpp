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
    //expr.push_constant(5);
    //expr.push_constant(20);
    //expr.addi();
    //expr.push_constant(4);
    //expr.muli();
    //expr.push_constant(5);
    //expr.divi();
    //expr.finish();

    /* int a = 5; int b = 2 * a; b; */
    expr.push_constant(5);
    expr.store_local("a");
    expr.push_constant(2);
    expr.load_local("a");
    expr.muli();
    expr.store_local("b");
    expr.load_local("b");
    expr.finish();

    evaluate(expr, std::cout);

    return 0;
}

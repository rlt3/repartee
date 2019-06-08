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

    Symbol a(16);
    Symbol b(3.7);
    Symbol c(&a);

    printf("a: %d\n", a.integer());
    printf("b: %lf\n", b.floating());
    printf("c: %d\n", c.ref()->integer());

    a.set(0, 32);
    b.set(0, 7.4);
    c.set(0, &b);

    printf("a: %d\n", a.integer());
    printf("b: %lf\n", b.floating());
    printf("c: %lf\n", c.ref()->floating());

    Symbol s(24, 0);
    /* { int, double, int, double } */

    s.set(0, 2);
    s.set(4, 2.5);
    s.set(12, 3);
    s.set(16, 3.5);

    printf("s.i1: %d\n", s.integer_at(0));
    printf("s.d1: %lf\n", s.floating_at(4));
    printf("s.i2: %d\n", s.integer_at(12));
    printf("s.d2: %lf\n", s.floating_at(16));

    return 0;
}

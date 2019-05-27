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
    Symbol foo(5);
    int v = foo.integer();
    int c = 0;

    error("e: %d\n", c++);
    warning("foo: %d\n", v);
    panic("bad: %d\n", c++);

    return 0;
}

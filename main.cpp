#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.hpp"

void
usage (char *prog)
{
    fprintf(stderr, "%s <input file|->\n", prog);
    exit(1);
}

int
main (int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    TokenizedInput tokens;

    if (argv[1][0] == '-') {
        tokens = tokenize(std::cin);
    } else {
        std::ifstream input(argv[1], std::ios::in);
        tokens = tokenize(input);
    }

    if (tokens.has_error()) {
        fprintf(stderr, "%s\n", tokens.get_error().c_str());
        exit(1);
    }

    while (!tokens.end()) {
        Token t = tokens.next();
        printf("%d:%d %s\n", t.line, t.column, t.str.c_str());
    }

    putchar('\n');

    for (auto l : tokens.get_lines()) {
        printf("%s\n", l.c_str());
    }

    return 0;
}

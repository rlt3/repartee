#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "machine.hpp"
#include "environment.hpp"

void
usage (char *prog)
{
    fprintf(stderr, "%s <input file|->\n", prog);
    exit(1);
}

TokenizedInput
tokenize_input (char *filename)
{
    std::string f(filename);
    TokenizedInput tokens;

    if (f == "-") {
        tokens = tokenize(std::cin);
    } else {
        std::ifstream input(f, std::ios::in);
        if (input.fail()) {
            fprintf(stderr, "%s: `%s'\n", strerror(errno), filename);
            exit(1);
        }
        tokens = tokenize(input);
    }

    if (tokens.has_error()) {
        fprintf(stderr, "%s\n", tokens.get_error().c_str());
        exit(1);
    }

    return tokens;
}

int
main (int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    TokenizedInput tokens = tokenize_input(argv[1]);
    Environment E = parse(tokens);
    
    //E.nodes[0]->print(0);
    std::vector<Instruction> prog;
    E.nodes[0]->gen_code(prog);
    prog.push_back(OP_HALT);
    //print_bytecode(prog);
    printf("%d\n", run(prog));
    
    return 0;
}

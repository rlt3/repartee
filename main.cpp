#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "machine.hpp"
#include "environment.hpp"
#include "arg.h"

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

static bool is_debug = false;
static bool is_execute = true;

void
handle_args (int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    /* for ARGBEGIN macro see arg.h */
    ARGBEGIN {
        /* show debug */
        case 'd': is_debug = true; break;
        /* no execute */
        case 'n': is_execute = false; break;
        case 'h': usage(argv[0]); break;
        default: break;
    } ARGEND
}

int
main (int argc, char **argv)
{
    handle_args(argc, argv);

    TokenizedInput tokens = tokenize_input(argv[argc - 1]);
    Environment E = parse(tokens);
    
    std::vector<Instruction> bytecode;
    E.generate_bytecode(bytecode);

    if (is_debug) {
        E.root()->print_tree(0);
        print_bytecode(bytecode);
    }

    if (is_execute) {
        printf("%d\n", run(bytecode));
    }
    
    return 0;
}

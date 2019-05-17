#pragma once

#include "tokenizer.hpp"
#include "machine.hpp"

class Node;

/*
 * Holds all the symbol definitions for a particular environment. The root 
 * environment (whose parent is NULL) holds the definitions for global symbols.
 * Creating child environments allows for closures.
 */
class Environment {
public:
    Environment (Environment *parent);

protected:
    Environment *parent;
};

class Node {
public:
    /*  
     * Create a particular Node (Abstract Syntax Tree Node) within a particular
     * environment.
     */
    Node (Environment *env, Token t);

    void
    add_child ();

    /* actually generate the byte code */
    virtual void
    gen_code (std::vector<Instruction> &prog);

protected:
};

#pragma once

#include <vector>
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
     * environment. The environments of a particular node and its children can
     * all be the same or some different.
     */
    Node (Environment *env, Token t);

    virtual ~Node();

    void
    add_child (Node *child);

    /* actually generate the byte code */
    virtual void
    gen_code (std::vector<Instruction> &prog);

protected:
    Environment *env;
    std::vector<Node*> children;
};

class AssignmentNode : public Node {
public:
    AssignmentNode (Environment *env, Token t);
};

#pragma once

#include <vector>
#include <unordered_map>
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
    ~Environment ();

    /* register a symbol in the environment */
    void
    reg_sym (std::string sym, Node *n);

    /* add a new tree path of code execution */
    void
    add_node (Node *n);

    std::vector<Node*> nodes;

protected:
    Environment *parent;
    std::unordered_map<std::string, Node*> symbols;
};

class Node {
public:
    /*  
     * Create a particular Node (Abstract Syntax Tree Node) within a particular
     * environment. The environments of a particular node and its children can
     * all be the same or some different.
     */
    Node (Environment *env);
    virtual ~Node();

    /* add a node as a child */
    void add_child (Node *child);

    /* take children from `other`, add them to `this`, and clear the vector */
    void merge (Node *other);

    /* call from a root node to generate the bytecode for an entire tree */
    void gen_code (std::vector<Instruction> &prog);

    /* generate the bytecode for a particular node */
    virtual void code (std::vector<Instruction> &prog);

    /* debugging functions which help see how the tree has been parsed */
    void print (int lvl);
    virtual std::string name ();

    Environment *env;
    std::vector<Node*> children;
};

class AssignmentNode : public Node {
public:
    AssignmentNode (Environment *env, int var_id);
    void code (std::vector<Instruction> &prog);
    std::string name ();
    int var_id;
};

class VarNode : public Node {
public:
    VarNode (Environment *env, int id);
    void code (std::vector<Instruction> &prog);
    std::string name ();
    int var_id;
};

class AtomNode : public Node {
public:
    AtomNode (Environment *env, int value);
    void code (std::vector<Instruction> &prog);
    std::string name ();
    int value;
};

class OperatorNode : public Node {
public:
    OperatorNode (Environment *env, int type);
    void code (std::vector<Instruction> &prog);
    std::string name ();
    int type;
};

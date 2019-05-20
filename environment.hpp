#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include "tokenizer.hpp"
#include "machine.hpp"

class Environment;

/*  
 *  An Abstract Syntax Tree Node. This is the base class which all real nodes
 *  derive (and can be found below).
 */
class Node {
public:
    Node ()
        : env(NULL)
    { }

    virtual ~Node()
    { }

    void
    set_env (Environment *e)
    {
        this->env = e;
    }

    /* add a node as a child */
    void
    add_child (Node *child)
    {
        children.push_back(child);
    }

    /* take children from `other`, add them to `this`, and clear the vector */
    void
    merge (Node *other)
    {
        for (auto N : other->children)
            children.push_back(N);
        other->children.clear();
    }

    /* call from a root node to generate the bytecode for an entire tree */
    void
    gen_code (std::vector<Instruction> &prog)
    {
        if (children.size() == 0)
            return;

        for (auto C : children) {
            C->gen_code(prog);
            C->code(prog);
        }
    }

    /* generate the bytecode for a particular node */
    virtual void
    code (std::vector<Instruction> &prog)
    {
        fputs("Bad!!!\n", stderr);
        exit(1);
    }

    /* debugging functions which help see how the tree has been parsed */
    void
    print (int lvl)
    {
        for (int i = 0; i < lvl * 2; i++)
            putchar(' ');
        puts(name().c_str());
        lvl++;
        for (auto C : children)
            C->print(lvl);
    }

    virtual std::string
    name ()
    {
        return "root";
    }

    Environment *env;
    std::vector<Node*> children;
};

/*
 * Holds all the symbol definitions for a particular environment. The root
 * environment (whose parent is NULL) holds the definitions for global symbols.
 * Creating child environments allows for closures.  Also owns and creates all
 * nodes.
 */
class Environment {
public:
    Environment (Environment *parent)
        : parent(parent)
    { }

    ~Environment ()
    {
        for (auto N : nodes)
            delete N;
    }

    /* register a symbol in the environment */
    void
    reg_sym (std::string sym, Node *n)
    {
        symbols[sym] = n;
    }

    /* create a new root node for an expression */
    Node*
    root ()
    {
        return this->node(Node());
    }

    /* 
     * The "allocator" for Nodes. Creates and owns a new node in the
     * environment and allows the tree to be built without too much
     * boilerplate.
     */
    template <typename T>
    Node*
    node (T node)
    {
        Node *n = new T(node);
        n->set_env(this);
        nodes.push_back(n);
        return n;
    }

    std::vector<Node*> nodes;

protected:
    Environment *parent;
    std::unordered_map<std::string, Node*> symbols;
};

/******************************************
            AST Node Classes
 ******************************************/

class BranchNode : public Node {
public:
    BranchNode (Node *cond, Node *trueb, Node *falseb)
        : Node(), cond(cond), trueb(trueb), falseb(falseb)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        /* branch code */
    }

    std::string
    name ()
    {
        return "branch";
    }

    Node *cond;
    Node *trueb;
    Node *falseb;
};

class AssignmentNode : public Node {
public:
    AssignmentNode (int var_id)
        : Node(), var_id(var_id)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_STORE, var_id));
    }

    std::string
    name ()
    {
        return "=";
    }

    int var_id;
};

class VarNode : public Node {
public:
    VarNode (int var_id)
        : Node(), var_id(var_id)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_LOAD, var_id));
    }

    std::string
    name ()
    {
        return "var#" + std::to_string(var_id);
    }

    int var_id;
};

class AtomNode : public Node {
public:
    AtomNode (int value)
        : Node(), value(value)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_PUSH, value));
    }

    std::string
    name ()
    {
        return std::to_string(value);
    }

    int value;
};

class OperatorNode : public Node {
public:
    OperatorNode (int type)
        : Node(), type(type)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        switch (type) {
            case TKN_ADD:
                prog.push_back(create_instruction(OP_ADD, 2));
                break;
            case TKN_SUB:
                prog.push_back(create_instruction(OP_SUB, 2));
                break;
            case TKN_MUL:
                prog.push_back(create_instruction(OP_MUL, 2));
                break;
            case TKN_DIV:
                prog.push_back(create_instruction(OP_DIV, 2));
                break;
            case TKN_DBL_EQUAL:
                prog.push_back(create_instruction(OP_CMP, 0));
                break;
        }
    }

    std::string
    name ()
    {
        switch (type) {
            case TKN_ADD: return "+";
            case TKN_SUB: return "-";
            case TKN_MUL: return "*";
            case TKN_DIV: return "/";
            case TKN_DBL_EQUAL: return "==";
        }
        return "";
    }

    int type;
};

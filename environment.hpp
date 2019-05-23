#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <deque>
#include <tuple>
#include <map>
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
        : env(NULL), name("root")
    { }

    Node (std::string name)
        : env(NULL), name(name)
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
    virtual void
    print (int lvl)
    {
        for (int i = 0; i < lvl * 2; i++)
            putchar(' ');
        puts(name.c_str());
        lvl++;
        for (auto C : children)
            C->print(lvl);
    }

    Environment *env;
    std::string name;
    std::vector<Node*> children;
    /* should be true when a node produces code but doesn't have children */
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

        for (auto e : children)
            delete e;
    }

    void
    generate_backpatch (std::vector<Instruction> &prog,
            std::string lbl, Opcode op)
    {
        if (labels.find(lbl) != labels.end()) {
            fprintf(stderr, "Label %s already exists in this env\n", lbl.c_str());
            exit(1);
        }
        prog.push_back(OP_HALT);
        labels[lbl] = -1;
        auto idx = prog.size() - 1;
        backpatches.push_back(std::make_tuple(idx, op, lbl));
    }

    void
    patch_backpatch (std::vector<Instruction> &prog, std::string lbl)
    {
        labels[lbl] = prog.size();
    }

    void
    write_backpatches (std::vector<Instruction> &prog)
    {
        for (auto b : backpatches) {
            int idx = std::get<0>(b);
            Opcode op = std::get<1>(b);
            int label = labels[std::get<2>(b)];
            if (label < 0) {
                fprintf(stderr, "Label %s was never resolved\n", std::get<2>(b).c_str());
                exit(1);
            }
            prog[idx] = create_instruction(op, label);
        }
        backpatches.clear();
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

    /* The 'allocator' for sub-envs */
    Environment *
    child ()
    {
        Environment *e = new Environment(this);
        children.push_back(e);
        return e;
    }

    std::vector<Node*> nodes;
    std::vector<Environment*> children;

protected:
    Environment *parent;
    std::unordered_map<std::string, Node*> symbols;
    std::unordered_map<std::string, int> labels;
    std::vector<std::tuple<unsigned long, Opcode, std::string>> backpatches;
};

/******************************************
            AST Node Classes
 ******************************************/

/* Write out the instructions for all resolved labels/addresses */
class CondBranchNode : public Node {
public:
    CondBranchNode ()
        : Node("conditional branch")
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        env->write_backpatches(prog);
    }
};

/* Resolve the address of a particular label */
class LabelNode : public Node {
public:
    LabelNode (std::string label)
        : Node("label " + label), label(label)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        env->patch_backpatch(prog, label);
    }
    
    std::string label;
};

/* Generate a short circuit branch to given true and false labels */
class ShortCircuitNode : public Node {
public:
    ShortCircuitNode (int type, std::string tlbl, std::string flbl)
        : Node("short circuit"), type(type), true_label(tlbl), false_label(flbl)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        switch (type) {
            case TKN_LOGICAL_OR:
                env->generate_backpatch(prog, true_label, OP_JNZ);
                break;
            case TKN_LOGICAL_AND:
                env->generate_backpatch(prog, false_label, OP_JEZ);
                break;
            default:
                fprintf(stderr, "Type error (%d) %s\n", type, tokentype_to_str(type).c_str());
                exit(1);
        }
    }
    
    int type;
    std::string true_label;
    std::string false_label;
};

/* An unconditional jump to some label to-be-resolved */
class JmpNode : public Node {
public:
    JmpNode (std::string label)
        : Node("j " + label), label(label)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        env->generate_backpatch(prog, label, OP_J);
    }

    std::string label;
};

class AssignmentNode : public Node {
public:
    AssignmentNode (int var_id)
        : Node("="), var_id(var_id)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_STORE, var_id));
    }

    int var_id;
};

class VarNode : public Node {
public:
    VarNode (int var_id)
        : Node("var#" + std::to_string(var_id)), var_id(var_id)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_LOAD, var_id));
    }

    int var_id;
};

class AtomNode : public Node {
public:
    AtomNode (int value)
        : Node(std::to_string(value)), value(value)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_PUSH, value));
    }

    int value;
};

class OperatorNode : public Node {
public:
    OperatorNode (int type)
        : Node(), type(type)
    {
        switch (type) {
            case TKN_ADD: name = "+"; break;
            case TKN_SUB: name = "-"; break;
            case TKN_MUL: name = "*"; break;
            case TKN_DIV: name = "/"; break;
            case TKN_DBL_EQUAL: name = "=="; break;
        }
    }

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

    int type;
};

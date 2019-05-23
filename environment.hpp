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
        : parent(parent), lbl_true(-1), lbl_false(-1)
    { }

    ~Environment ()
    {
        for (auto N : nodes)
            delete N;
    }

    void
    generate_backpatch (std::vector<Instruction> &prog, Opcode op, bool branch)
    {
        prog.push_back(OP_HALT);
        backpatches.push_back(std::make_tuple(prog.size() - 1, op, branch));
    }

    void
    patch_backpatch (std::vector<Instruction> &prog, bool branch)
    {
        if (branch)
            lbl_true = prog.size();
        else
            lbl_false = prog.size();
    }

    void
    write_backpatches (std::vector<Instruction> &prog)
    {
        for (auto b : backpatches) {
            int idx = std::get<0>(b);
            Opcode op = std::get<1>(b);
            int label = std::get<2>(b) ? lbl_true : lbl_false;
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

    std::vector<Node*> nodes;

protected:
    Environment *parent;
    int lbl_true;
    int lbl_false;
    std::unordered_map<std::string, Node*> symbols;
    std::vector<std::tuple<int, Opcode, bool>> backpatches;
};

/******************************************
            AST Node Classes
 ******************************************/

class CondBranchNode : public Node {
public:
    CondBranchNode (Node *cond, Node *trueb, Node *falseb)
        : Node("conditional branch"), cond(cond), trueb(trueb), falseb(falseb)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        env->write_backpatches(prog);

        //int cond_patch, true_patch;

        ///* generate the IR for the conditional */
        //cond->gen_code(prog);

        ///* branch on the negation here to let 'true' statements fall through */
        //prog.push_back(0);
        //cond_patch = prog.size() - 1;

        //trueb->gen_code(prog);

        ///* if there's a false branch, we need jump to avoid it in true branch */
        //if (falseb) {
        //    prog.push_back(0);
        //    true_patch = prog.size() - 1;
        //}

        ///* patch the original negation jump instruction */
        //prog[cond_patch] = create_instruction(OP_JEZ, prog.size());

        //if (falseb) {
        //    falseb->gen_code(prog);
        //    /* finally, for true branch, patch where the false branch ends */
        //    prog[true_patch] = create_instruction(OP_J, prog.size());
        //}
    }

    void
    print (int lvl)
    {
        for (int i = 0; i < lvl * 2; i++)
            putchar(' ');
        puts(name.c_str());
        lvl++;
        cond->print(lvl);
        trueb->print(lvl);
        if (falseb) {
            falseb->print(lvl);
        }
    }

    Node *cond;
    Node *trueb;
    Node *falseb;
};

class ShortCircuitPatchNode : public Node {
public:
    ShortCircuitPatchNode (bool branch)
        : Node("short circuit"), branch(branch)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        env->patch_backpatch(prog, branch);
    }
    
    bool branch;
};

/* Fills the role of a backpatch generator */
class ShortCircuitGenNode : public Node {
public:
    ShortCircuitGenNode (int type)
        : Node("short circuit")
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        switch (type) {
            case TKN_LOGICAL_OR:
                this->env->generate_backpatch(prog, OP_JNZ, true);
                break;
            case TKN_LOGICAL_AND:
                this->env->generate_backpatch(prog, OP_JEZ, false);
                break;
            default:
                fprintf(stderr, "Type error\n");
                exit(1);
        }
    }
    
    int type;
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

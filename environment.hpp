#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <stack>
#include <assert.h>
#include "node.hpp"

/* 
 * Hold all of the backpatches for a particular scope in the environment.
 * BackpatchScopes are created through the environment by pushing a new scope
 * and then popping the scope when you need to write the resolved backpatches.
 * See: push_backpatch_scope, backpatch_scope, and pop_backpatch_scope
 */
class BackpatchScope {
public:
    BackpatchScope ()
    { }

    void
    create (std::vector<Instruction> &prog, std::string lbl, Opcode op)
    {
        if (labels.find(lbl) == labels.end())
            labels[lbl] = -1;
        prog.push_back(OP_HALT);
        auto idx = prog.size() - 1;
        backpatches.push_back(std::make_tuple(idx, op, lbl));
    }

    void
    patch (std::vector<Instruction> &prog, std::string lbl)
    {
        labels[lbl] = prog.size();
    }

    void
    write (std::vector<Instruction> &prog)
    {
        for (auto b : backpatches) {
            int idx = std::get<0>(b);
            Opcode op = std::get<1>(b);
            int label = labels[std::get<2>(b)];
            if (label < 0) {
                fprintf(stderr, "Label %s (%d) was never resolved\n", std::get<2>(b).c_str(), label);
                exit(1);
            }
            prog[idx] = create_instruction(op, label);
        }
    }

protected:
    std::unordered_map<std::string, int> labels;
    std::vector<std::tuple<unsigned long, Opcode, std::string>> backpatches;
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
        : parent(parent), root_node(NULL)
    { }

    ~Environment ()
    {
        for (auto N : nodes)
            delete N;

        for (auto e : children)
            delete e;
    }

    /* register a symbol in the environment */
    void
    define_local (std::string sym, Local v)
    {
        syms_local[sym] = v;
    }

    Local&
    get_local (std::string sym)
    {
        if (syms_local.find(sym) == syms_local.end()) {
            if (parent) {
                return parent->get_local(sym);
            } else {
                fprintf(stderr, "Symbol `%s' undefined!\n", sym.c_str());
                exit(1);
            }
        }
        return syms_local.at(sym);
    }

    void
    generate_bytecode (std::vector<Instruction> &prog)
    {
        for (auto &pair : syms_local) {
            pair.second.loc = prog.size();
            pair.second.node->code(prog);
        }

        root()->gen_code(prog);
        for (auto E : children)
            E->generate_bytecode(prog);
    }

    /* Create a function in this environment, returning a child environment */
    Environment*
    create_func (std::string func_name)
    {
        Environment *e = child();
        syms_func[func_name] = e;
        return e;
    }

    /* start a new backpatch scope for a particular path of code generation */
    void
    push_backpatch_scope ()
    {
        backpatches.push(BackpatchScope());
    }

    BackpatchScope&
    backpatch_scope ()
    {
        return backpatches.top();
    }

    void
    pop_backpatch_scope ()
    {
        assert(backpatches.size() >= 1);
        backpatches.pop();
    }

    /* 
     * Return the root Node for the entire environment tree. A tree of Nodes
     * can have multiple environments.
     */
    Node*
    root ()
    {
        /* at root environment */
        if (!parent) {
            if (root_node)
                return root_node;
            root_node = this->node(Node());
            return root_node;
        } else {
            return parent->root();
        }
    }

    /* 
     * The "allocator" for Nodes. Creates and owns a new node in the
     * environment and allows the tree to be built without too much
     * boilerplate.
     */
    template <typename T>
    T*
    node (T node)
    {
        T *n = new T(node);
        n->set_env(this);
        nodes.push_back(reinterpret_cast<Node*>(n));
        return n;
    }

protected:

    /* The 'allocator' for sub-envs */
    Environment *
    child ()
    {
        Environment *e = new Environment(this);
        children.push_back(e);
        return e;
    }

    Environment* parent;
    Node* root_node;
    
    std::unordered_map<std::string, Local> syms_local;
    std::unordered_map<std::string, Environment*> syms_func;
    std::stack<BackpatchScope> backpatches;
    std::vector<Environment*> children;
    std::vector<Node*> nodes;
};

#pragma once

#include <vector>
#include <string>
#include "tokenizer.hpp"
#include "machine.hpp"

class Environment;
class Node;

typedef enum _DataType {
    FREE,
    AUTO,
    VOID,
    INTEGER,
    FLOAT,
    STRING,
    FUNCTION,
} DataType;

std::string datatype_to_str (DataType type);

struct Local {
    Local ()
        : node(NULL), type(VOID), loc(-1)
    { }

    Local (Node *n, DataType type)
        : node(n), type(type), loc(-1)
    { }

    Node *node;
    DataType type;
    int loc;
};

/*  
 *  An Abstract Syntax Tree Node. This is the base class which all real nodes
 *  derive (and can be found below).
 */
class Node {
public:
    Node ()
        : env(NULL), name("root"), has_setup(false)
    { }

    Node (std::string name)
        : env(NULL), name(name), has_setup(false)
    { }

    Node (std::string name, bool setup)
        : env(NULL), name(name), has_setup(setup)
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
        if (children.size() == 0) {
            code(prog);
            return;
        }

        if (this->has_setup)
            this->setup();

        for (auto C : children)
            C->gen_code(prog);

        code(prog);
    }

    virtual void
    setup ()
    {
        fputs("Bad Setup\n", stderr);
        exit(1);
    }

    /* generate the bytecode for a particular node */
    virtual void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(OP_HALT);
    }

    /* debugging functions which help see how the tree has been parsed */
    virtual void
    print_tree (int lvl)
    {
        for (int i = 0; i < lvl * 2; i++)
            putchar(' ');
        this->print(lvl);
        lvl++;
        for (auto C : children)
            C->print_tree(lvl);
    }

    virtual void
    print (int lvl)
    {
        puts(name.c_str());
    }

    Environment *env;
    std::string name;
    std::vector<Node*> children;
    bool has_setup;
};

//class FuncNode : public Node {
//    FuncNode (std::string name, std::vector<Local> params)
//        : Node("function", true)
//    { }
//
//    void
//    setup ()
//    {
//        /* write params ? */
//    }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        /* write function body and return */
//    }
//};
//
///* Write out the instructions for all resolved labels/addresses */
//class IfElseNode : public Node {
//public:
//    IfElseNode ()
//        : Node("conditional branch", true)
//    { }
//
//    void
//    setup ()
//    {
//        env->push_backpatch_scope();
//    }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        env->backpatch_scope().write(prog);
//        env->pop_backpatch_scope();
//    }
//
//};
//
///*
// * Resolves the backpatch records the ShortCircuitNodes generate within the
// * same environment. Produces the code that produces a 1 or 0 for logical
// * expressions.
// */
//class LogicalNode : public Node {
//public:
//    LogicalNode (int t, std::string s, std::string e)
//        : Node(t == TKN_LOGICAL_OR ? "or" : "and", true)
//        , type(t)
//        , short_label(s)
//        , exit_label(e)
//    { }
//
//    void
//    setup ()
//    {
//        env->push_backpatch_scope();
//    }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        BackpatchScope &b = env->backpatch_scope();
//
//        if (type == TKN_LOGICAL_OR)
//            prog.push_back(create_instruction(OP_PUSH, 0));
//        else
//            prog.push_back(create_instruction(OP_PUSH, 1));
//
//        b.create(prog, exit_label, OP_J);
//        b.patch(prog, short_label);
//
//        if (type == TKN_LOGICAL_OR)
//            prog.push_back(create_instruction(OP_PUSH, 1));
//        else
//            prog.push_back(create_instruction(OP_PUSH, 0));
//
//        b.patch(prog, exit_label);
//        b.write(prog);
//
//        env->pop_backpatch_scope();
//    }
//
//    int type;
//    std::string short_label;
//    std::string exit_label;
//};
//
//class ShortCircuitNode : public Node {
//public:
//    ShortCircuitNode (int type, std::string s, std::string e)
//        : Node("short circuit"), type(type), short_label(s), exit_label(e)
//    { }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        switch (type) {
//            case TKN_LOGICAL_OR:
//                env->backpatch_scope().create(prog, short_label, OP_JNZ);
//                break;
//            case TKN_LOGICAL_AND:
//                env->backpatch_scope().create(prog, short_label, OP_JEZ);
//                break;
//            default:
//                fprintf(stderr, "Type error (%d) %s\n", type, tokentype_to_str(type).c_str());
//                exit(1);
//        }
//    }
//    
//    int type;
//    std::string short_label;
//    std::string exit_label;
//};
//
///* Resolve the address of a particular label */
//class LabelNode : public Node {
//public:
//    LabelNode (std::string label)
//        : Node("label " + label), label(label)
//    { }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        env->backpatch_scope().patch(prog, label);
//    }
//    
//    std::string label;
//};
//
//class JmpZeroNode : public Node {
//public:
//    JmpZeroNode (std::string label)
//        : Node("jez " + label), label(label)
//    { }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        env->backpatch_scope().create(prog, label, OP_JEZ);
//    }
//
//    std::string label;
//};
//
//class JmpNonNull : public Node {
//public:
//    JmpNonNull (std::string label)
//        : Node("jnz " + label), label(label)
//    { }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        env->backpatch_scope().create(prog, label, OP_JNZ);
//    }
//
//    std::string label;
//};
//
///* An unconditional jump to some label to-be-resolved */
//class JmpNode : public Node {
//public:
//    JmpNode (std::string label)
//        : Node("j " + label), label(label)
//    { }
//
//    void
//    code (std::vector<Instruction> &prog)
//    {
//        env->backpatch_scope().create(prog, label, OP_J);
//    }
//
//    std::string label;
//};


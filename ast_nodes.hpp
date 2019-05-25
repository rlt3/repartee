#pragma once

#include "environment.hpp"

class ExprNode : public Node {
public:
    ExprNode (DataType type)
        : Node(), type(type)
    { }

    ExprNode (std::string name, DataType type)
        : Node(name), type(type)
    { }

    DataType type;
};

/* 
 * Keeps the instructions for local variables abstracted from the environment.
 * Should never be a child or parent or anywhere except in the Environment's
 * container for local variables.
 */
class LocalNode : public ExprNode {
public:
    LocalNode (DataType type)
        : ExprNode("local", type)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_PUSH_LOCAL, 0));
    }
};

class VarNode : public ExprNode {
public:
    VarNode (std::string name, Local &local)
        : ExprNode(name, local.type), local(local)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_LOAD, local.loc));
    }

    Local &local;
};

class AssignmentNode : public Node {
public:
    AssignmentNode (VarNode *var, ExprNode *expr)
        : Node("="), var(var), expr(expr)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        expr->gen_code(prog);
        prog.push_back(create_instruction(OP_STORE, var->local.loc));
    }

    virtual void
    print (int lvl)
    {
        lvl++;
        puts(name.c_str());
        var->print_tree(lvl);
        expr->print_tree(lvl);
    }

    VarNode *var;
    ExprNode *expr;
};

class NumNode : public ExprNode {
public:
    NumNode (int value)
        : ExprNode(std::to_string(value), INTEGER), value(value)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        prog.push_back(create_instruction(OP_PUSH, value));
    }

    int value;
};

class StrNode : public ExprNode {
public:
    StrNode (std::string str)
        : ExprNode(str, STRING)
    { }

    void
    code (std::vector<Instruction> &prog)
    {
        //prog.push_back(create_instruction(OP_PUSH, value));
    }
};

class BinaryOpNode : public ExprNode {
public:
    BinaryOpNode (Node *left, int op, Node *right)
        : ExprNode(INTEGER), left(left), op(op), right(right)
    {
        switch (op) {
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
        Opcode ins_op;
        switch (op) {
            case TKN_ADD:
                ins_op = OP_ADD;
                break;
            case TKN_SUB:
                ins_op = OP_SUB;
                break;
            case TKN_MUL:
                ins_op = OP_MUL;
                break;
            case TKN_DIV:
                ins_op = OP_DIV;
                break;
            case TKN_DBL_EQUAL:
                ins_op = OP_CMP;
                break;
        }
        left->gen_code(prog);
        right->gen_code(prog);
        prog.push_back(create_instruction(ins_op, 2));
    }

    virtual void
    print (int lvl)
    {
        lvl++;
        puts(name.c_str());
        left->print_tree(lvl);
        right->print_tree(lvl);
    }

    Node *left;
    int op;
    Node *right;
};

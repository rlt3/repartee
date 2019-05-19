#include <cstdlib>
#include <cstdio>
#include "environment.hpp"

Environment::Environment (Environment *parent)
    : parent(parent)
{ }

Environment::~Environment ()
{
    for (auto N : nodes)
        delete N;
}

void
Environment::reg_sym (std::string sym, Node *n)
{
    symbols[sym] = n;
}

void
Environment::add_node (Node *n)
{
    nodes.push_back(n);
}

Node::Node (Environment *env)
    : env(env)
{ }

Node::~Node()
{
    for (auto N : children)
        delete N;
}

void
Node::add_child (Node *child)
{
    children.push_back(child);
}

void
Node::merge (Node *other)
{
    for (auto N : other->children)
        children.push_back(N);
    other->children.clear();
}

void
Node::gen_code (std::vector<Instruction> &prog)
{
    if (children.size() == 0)
        return;

    for (auto C : children) {
        C->gen_code(prog);
        C->code(prog);
    }
}

/* actually generate the byte code */
void
Node::code (std::vector<Instruction> &prog)
{
    fputs("Bad!!!\n", stderr);
    exit(1);
}

AssignmentNode::AssignmentNode (Environment *env, int var_id)
    : Node(env), var_id(var_id)
{ }

void
AssignmentNode::code (std::vector<Instruction> &prog)
{
    /* assignment node */
    prog.push_back(create_instruction(OP_STORE, var_id));
}

VarNode::VarNode (Environment *env, int var_id)
    : Node(env), var_id(var_id)
{ }

void
VarNode::code (std::vector<Instruction> &prog)
{
    prog.push_back(create_instruction(OP_LOAD, var_id));
}

AtomNode::AtomNode (Environment *env, int value)
    : Node(env), value(value)
{ }

void
AtomNode::code (std::vector<Instruction> &prog)
{
    prog.push_back(create_instruction(OP_PUSH, value));
}

OperatorNode::OperatorNode (Environment *env, int type)
    : Node(env), type(type)
{ }

void
OperatorNode::code (std::vector<Instruction> &prog)
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

void
Node::print (int lvl)
{
    for (int i = 0; i < lvl * 2; i++)
        putchar(' ');
    puts(name().c_str());
    lvl++;
    for (auto C : children)
        C->print(lvl);
}

std::string
Node::name ()
{
    return "root";
}

std::string
AssignmentNode::name ()
{
    return "=";
}

std::string
VarNode::name ()
{
    return "var#" + std::to_string(var_id);
}

std::string
AtomNode::name ()
{
    return std::to_string(value);
}

std::string
OperatorNode::name ()
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

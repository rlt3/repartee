#include <cstdlib>
#include <cstdio>
#include "environment.hpp"

Environment::Environment (Environment *parent)
    : parent(parent)
{ }

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

Node::Node (Environment *env, Token t)
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

/* actually generate the byte code */
void
Node::gen_code (std::vector<Instruction> &prog)
{
    fputs("Bad!!!\n", stderr);
    exit(1);
}

AssignmentNode::AssignmentNode (Environment *env, Token t)
    : Node(env, t)
{ }

VarNode::VarNode (Environment *env, Token t)
    : Node(env, t)
{ }

OperatorNode::OperatorNode (Environment *env, Token t)
    : Node(env, t)
{
    foobar.push_back(1);
    foobar.push_back(2);
    foobar.push_back(3);
    foobar.push_back(4);
}

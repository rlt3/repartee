#include <cstdlib>
#include <cstdio>
#include "environment.hpp"

Environment::Environment (Environment *parent)
    : parent(parent)
{ }

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

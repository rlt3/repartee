#include <sstream>
#include "parser.hpp"

/*
 * TODO: Can be a general error function with a void pointer data type that can
 * be casted to a particular type since all references will exist on a stack at
 * error time. Error types then can be defined in a single place as well.
 */
void 
error_func (int error_type, Token t, int data)
{
    std::stringstream s;

    switch (error_type) {
        case TKNZR_MATCH:
            s << "Expected `" << tokentype_to_str(data) << "'";
            s << " instead got `" << t.str << "'";
            s << " on line " << t.line << " column " << t.column;
            break;
        default:
            break;
    }

    fprintf(stderr, "%s\n", s.str().c_str());
    exit(1);
}

ExprNode* expr (TokenizedInput &T, Environment &E);

ExprNode*
var_free (TokenizedInput &T, Environment &E)
{
    return E.node(VarNode(T.expect(TKN_NAME).str));
}

ExprNode*
number (TokenizedInput &T, Environment &E)
{
    return E.node(NumNode(T.expect(TKN_NUMBER).to_int()));
}

ExprNode*
atom (TokenizedInput &T, Environment &E)
{
    if (T.peek().type == TKN_NUMBER) {
        return number(T, E);
    }
    else {
        return var_free(T, E);
    }
}

/* <item> = (<expr>) | <atom> */
ExprNode*
item (TokenizedInput &T, Environment &E)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        ExprNode *e = expr(T, E);
        T.expect(TKN_RIGHT_PAREN);
        return e;
    }
    else {
        return atom(T, E);
    }
}

ExprNode*
factor_tail (TokenizedInput &T, Environment &E, ExprNode *prev)
{
    /* see: sum_tail */
    if (!(T.peek().type == TKN_MUL || T.peek().type == TKN_DIV))
        return prev;

    int op = T.next().type;
    Node *right = item(T, E);

    return factor_tail(T, E, E.node(BinaryOpNode(prev, op, right)));
}

ExprNode*
factor (TokenizedInput &T, Environment &E)
{
    return factor_tail(T, E, item(T, E));
}

ExprNode*
sum_tail (TokenizedInput &T, Environment &E, ExprNode *prev)
{
    /* 
     * Create the BinaryOpsNode for each parsed binary op but pass it along to
     * the next so it can be put as a child of the next. This builds the tree
     * in 'reverse' keeping the left-associativity of arithmetic.
     */
    if (!(T.peek().type == TKN_ADD || T.peek().type == TKN_SUB))
        return prev;

    int op = T.next().type;
    Node *right = factor(T, E);

    return sum_tail(T, E, E.node(BinaryOpNode(prev, op, right)));
}

ExprNode*
sum (TokenizedInput &T, Environment &E)
{
    return sum_tail(T, E, factor(T, E));
}

bool
is_type_name (Token t)
{
    int type = t.type;
    return (type == TKN_INT || type == TKN_FLOAT || type == TKN_STRING);
}

DataType
type_name (TokenizedInput &T, Environment &E)
{
    Token t = T.next();
    DataType type;

    switch (t.type) {
        case TKN_INT:    type = INTEGER; break;
        case TKN_FLOAT:  type = FLOAT;   break;
        case TKN_STRING: type = STRING;  break;
        default: break;
    }

    return type;
}

ExprNode*
expr (TokenizedInput &T, Environment &E)
{
    return sum(T, E);
}

ExprNode*
var_declare (TokenizedInput &T, Environment &E)
{
    DataType type = type_name(T, E);
    std::string name = T.expect(TKN_NAME).str;
    E.reg_local(name, Local(E.node(LocalNode(type)), type));
    return E.node(VarNode(name, type));
}

Node*
assign (TokenizedInput &T, Environment &E)
{
    ExprNode* var;

    /* 
     * If the assignment has a type definition then we are declaring a new
     * variable of that type.
     */
    if (is_type_name(T.peek()))
        var = var_declare(T, E);
    else
        var = var_free(T, E);

    T.expect(TKN_EQUAL);
    ExprNode *val = expr(T, E);
    T.expect(TKN_SEMICOLON);

    return E.node(AssignmentNode(var, val));
}

Node *
define (TokenizedInput &T, Environment &E)
{
    if ((T.peek().type == TKN_NAME && T.peek(1).type == TKN_EQUAL)
            || is_type_name(T.peek())) {
        return assign(T, E);
    }
    else {
        Node *e = expr(T, E);
        T.expect(TKN_SEMICOLON);
        return e;
    }
}

Environment
parse (TokenizedInput &T)
{
    T.set_runtime_error_func(error_func);
    Environment E(NULL);
    Node *N = E.root();

    while (!T.empty())
        N->add_child(define(T, E));

    return E;
}

///* <comp> := <sum> == <sum> | <sum> */
//void
//comp (TokenizedInput &T, Environment &E, Node *N)
//{
//    Node tmp;
//    sum(T, E, &tmp);
//
//    if (T.peek().type == TKN_DBL_EQUAL) {
//        T.expect(TKN_DBL_EQUAL);
//        //Node *cmp = E.node(OperatorNode(TKN_DBL_EQUAL));
//        //N->add_child(cmp);
//        //cmp->merge(&tmp);
//        //sum(T, E, cmp);
//    } else {
//        N->merge(&tmp);
//    }
//}
//
///* <and> := <comp> && <and> | <comp> */
//void
//cond_and (TokenizedInput &T, Environment &E, Node *N)
//{
//    Node tmp;
//    comp(T, E, &tmp);
//
//    /* See <cond_or> */
//    if (T.peek().type == TKN_LOGICAL_AND) {
//        Node *L = E.node(LogicalNode(TKN_LOGICAL_AND, "false", "exit"));
//        L->merge(&tmp);
//        L->add_child(E.node(ShortCircuitNode(TKN_LOGICAL_AND, "false", "exit")));
//
//        while (T.peek().type == TKN_LOGICAL_AND) {
//            T.expect(TKN_LOGICAL_AND);
//            comp(T, E, L);
//            L->add_child(E.node(ShortCircuitNode(TKN_LOGICAL_AND, "false", "exit")));
//        }
//
//        N->add_child(L);
//    } else {
//        N->merge(&tmp);
//    }
//}
//
///* <or> := <and> || <or> | <and> */
//void
//cond_or (TokenizedInput &T, Environment &E, Node *N)
//{
//    Node tmp;
//    cond_and(T, E, &tmp);
//
//    if (T.peek().type == TKN_LOGICAL_OR) {
//        /*
//         * Each LogicalNode has its own environment where child ShortCircuit
//         * nodes generate backpatch records for short circuiting the true and
//         * false branches of logical expressions. All logical expressions
//         * evaluate to true or false (1 or 0) and the LogicalNode resolves the
//         * short circuited branches that provide the true or false values.
//         * Because each LogicalNode is in its own environment then child
//         * LogicalNodes will not interfere with label names.
//         */
//        Node *L = E.node(LogicalNode(TKN_LOGICAL_OR, "true", "exit"));
//        L->merge(&tmp);
//        L->add_child(E.node(ShortCircuitNode(TKN_LOGICAL_OR, "true", "exit")));
//
//        while (T.peek().type == TKN_LOGICAL_OR) {
//            T.expect(TKN_LOGICAL_OR);
//            cond_and(T, E, L);
//            L->add_child(E.node(ShortCircuitNode(TKN_LOGICAL_OR, "true", "exit")));
//        }
//
//        N->add_child(L);
//    } else {
//        N->merge(&tmp);
//    }
//}
//
//void
//body (TokenizedInput &T, Environment &E, Node *N)
//{
//    T.expect(TKN_LEFT_BRACE);
//    while (!(T.empty() || T.peek().type == TKN_RIGHT_BRACE)) {
//        expr(T, E, N);
//    }
//    T.expect(TKN_RIGHT_BRACE);
//}
//
///* 
// * <expr> := type <name>
// *         | type <name> = <cond_or>
// *         | if (<cond_or>) { <expr>+ }
// *         | if (<cond_or>) { <expr>+ } else { <expr>+ }
// *         | <cond_or>
// */
//void
//expr (TokenizedInput &T, Environment &E, Node *N)
//{
//    if (T.peek().type == TKN_INT || T.peek().type == TKN_STRING) {
//        Variable var;
//        Token t = T.next();
//        std::string name = T.expect(TKN_NAME).str;
//
//        switch (t.type) {
//            case TKN_INT:    var = Variable(name, INTEGER, false); break;
//            case TKN_STRING: var = Variable(name, STRING, false); break;
//            default: break;
//        }
//
//        E.reg_var(name, var);
//
//        if (T.peek().type == TKN_EQUAL) {
//            T.expect(TKN_EQUAL);
//            Node *val = E.node(Node());
//            cond_or(T, E, val);
//            N->add_child(E.node(AssignmentNode(var, val)));
//        }
//
//        T.expect(TKN_SEMICOLON);
//    }
//    else if (T.peek().type == TKN_IF) {
        /*
         * if (a or b) { 5; } else { 10; }
         *
         *                  IfElseNode
         *            /  /  |  |  |  \   \  \
         *          or jmp lbl 5 jmp lbl 10 lbl
         *        / | \    _|         |___   |___________
         *       a jmp b  |               |              |
         *                v               v              v
         * if (a or b) true: { 5; } else false: { 10; } exit:
         *
         * 'or' has a 'jmp' node that needs the location of the 'true' label
         * but it hasn't been resolved yet. So it makes a backpatch record for
         * that label. The first 'jmp' node of CondBranchNode is the final
         * check that the conditionals were true. It puts a conditional jump to
         * 'false'. The second 'jmp' skips the false portion of the if-else and
         * creates a record for 'exit'.  The 'lbl' nodes resolve or patch these
         * records by patching the label's address. Finally, CondBranchNode
         * writes these resolves patches as bytecode.
         */
//
//        Node *C = E.node(IfElseNode());
//
//        T.expect(TKN_IF);
//        T.expect(TKN_LEFT_PAREN);
//        cond_or(T, E, C);
//        T.expect(TKN_RIGHT_PAREN);
//
//        C->add_child(E.node(JmpZeroNode("false")));
//        body(T, E, C);
//
//        if (T.peek().type == TKN_ELSE) {
//            C->add_child(E.node(JmpNode("exit")));
//            C->add_child(E.node(LabelNode("false")));
//
//            T.expect(TKN_ELSE);
//            body(T, E, C);
//
//            C->add_child(E.node(LabelNode("exit")));
//        } else {
//            C->add_child(E.node(LabelNode("false")));
//        }
//
//        N->add_child(C);
//    }
//    else {
//        cond_or(T, E, N);
//        T.expect(TKN_SEMICOLON);
//    }
//
//}
//
//std::vector<Variable>
//params (TokenizedInput &T, Environment &E, Node *N)
//{
//    std::vector<Variable> params;
//
//    T.expect(TKN_LEFT_PAREN);
//    while (!(T.empty() || T.peek().type == TKN_RIGHT_PAREN)) {
//        T.expect(TKN_INT);
//        std::string name = T.expect(TKN_NAME).str;
//        params.push_back(Variable(name, INTEGER, true));
//        if (T.peek().type == TKN_COMMA)
//            T.next();
//    }
//    T.expect(TKN_RIGHT_PAREN);
//
//    return params;
//}
//
//void
//func (TokenizedInput &T, Environment &E, Node *N)
//{
//    if (T.peek().type != TKN_FUNC) {
//        expr(T, E, N);
//        return;
//    }
//
//    T.expect(TKN_FUNC);
//    std::string name = T.expect(TKN_NAME).str;
//    std::vector<Variable> p = params(T, E, N);
//    body(T, E, N);
//}

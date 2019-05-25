#include <sstream>
#include <map>
#include "parser.hpp"
#include "machine.hpp"

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

void cond_or (TokenizedInput &T, Environment &E, Node *N);
void expr (TokenizedInput &T, Environment &E, Node *N);

///* <atom> = number | name */
//void
//atom (TokenizedInput &T, Environment &E, Node *N)
//{
//    if (T.peek().type == TKN_NUMBER) {
//        int num = T.expect(TKN_NUMBER).to_int();
//        N->add_child(E.node(NumNode(num)));
//    }
//    else {
//        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
//        N->add_child(E.node(VarNode(var_id)));
//    }
//}
//
///* <item> = (<cond_or>) | <atom> */
//void
//item (TokenizedInput &T, Environment &E, Node *N)
//{
//    if (T.peek().type == TKN_LEFT_PAREN) {
//        T.expect(TKN_LEFT_PAREN);
//        cond_or(T, E, N);
//        T.expect(TKN_RIGHT_PAREN);
//    }
//    else {
//        atom(T, E, N);
//    }
//}
//
///* 
// * <factor> := <item> <factorTail> | <item> 
// * <factorTail> := * <atom> <factorTail> | / <atom> <factorTail>
// */
//void
//factor (TokenizedInput &T, Environment &E, Node *N)
//{
//    Node tmp;
//    item(T, E, &tmp);
//
//    /* For explanation see: sum */
//    //if (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
//    //    Node *next, *curr;
//    //    next = curr = NULL;
//
//    //    while (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
//    //        next = E.node(OperatorNode(T.next().type));
//    //        if (curr)
//    //            next->add_child(curr);
//    //        else
//    //            next->merge(&tmp);
//    //        item(T, E, next);
//    //        curr = next;
//    //    }
//
//    //    N->add_child(curr);
//    //} else {
//    //    N->merge(&tmp);
//    //}
//}
//
///*
// * <sum> := <factor> | <factor> <sumTail>
// * <sumTail> := + <factor> <sumTail> | - <factor> <sumTail>
// */
//void
//sum (TokenizedInput &T, Environment &E, Node *N)
//{
//    Node tmp;
//    factor(T, E, &tmp);
//
//    /* 
//     * Recurse (iteratively) down the sum and construct the tree from the
//     * bottom-up. If we constructed the tree as we parsed it then arithmetic
//     * expressions would be right-associative when they are naturally left
//     * associative.
//     */
//    if (T.peek().type == TKN_ADD || T.peek().type == TKN_SUB) {
//        while (T.peek().type == TKN_ADD || T.peek().type == TKN_SUB) {
//            T.next();
//            //next = E.node(OperatorNode(T.next().type));
//            ///* 
//            // * add the last parsed expression as a child of the current one
//            // * since we are building the tree in 'reverse'
//            // */
//            //if (curr)
//            //    next->add_child(curr);
//            ///* 
//            // * if !curr then we are at the top of the tree and we can add the
//            // * left part of the originally parsed expression (top of this func).
//            // */
//            //else
//            //    next->merge(&tmp);
//            factor(T, E, &tmp);
//            /* curr will be the child of whatever is parsed or child of N */
//            //curr = next;
//        }
//
//        //N->add_child(curr);
//    } else {
//        N->merge(&tmp);
//    }
//}
//
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

Node*
var (TokenizedInput &T, Environment &E)
{
    return E.node(VarNode(T.expect(TKN_NAME).str));
}

Node*
number (TokenizedInput &T, Environment &E)
{
    return E.node(NumNode(T.expect(TKN_NUMBER).to_int()));
}

Node*
sum_tail (TokenizedInput &T, Environment &E, Node *prev)
{
    /*
     * As whole BinaryOps get parsed, create the BinaryOpNode and send that to
     * the next function call.
     *
     * 2 - 5 + 3
     *
     * Parse 2 - 5 and construct the binary ops. Then notice that + is still
     * around. So, a recursive can start with the sum_tail from there. At each
     * stage the previous `prev` node is the left and the newly created node
     * is the right and it passes on to the next.
     */
    if (!(T.peek().type == TKN_ADD || T.peek().type == TKN_SUB))
        return prev;

    int op = T.next().type;
    Node *right = number(T, E);

    return sum_tail(T, E, E.node(BinaryOpNode(prev, op, right)));
}

Node*
sum (TokenizedInput &T, Environment &E)
{
    return sum_tail(T, E, number(T, E));
}

Variable
var_declare (TokenizedInput &T, Environment &E)
{
    Variable var;
    Token t = T.next();
    std::string name = T.expect(TKN_NAME).str;

    switch (t.type) {
        case TKN_INT:    var = Variable(name, INTEGER, false); break;
        case TKN_STRING: var = Variable(name, STRING, false); break;
        default: break;
    }

    return var;
}

Node*
expr (TokenizedInput &T, Environment &E)
{
    Node *s = sum(T, E);
    T.expect(TKN_SEMICOLON);
    return s;
}

Node *
define (TokenizedInput &T, Environment &E)
{
    if (T.peek().type == TKN_INT || T.peek().type == TKN_STRING) {
        Variable var = var_declare(T, E);
        E.reg_var(var.name, var);

        if (T.peek().type == TKN_EQUAL) {
            T.expect(TKN_EQUAL);
            Node *val = expr(T, E);
            T.expect(TKN_SEMICOLON);
            return E.node(AssignmentNode(var, val));
        } else {
            T.expect(TKN_SEMICOLON);
            return E.node(VarNode(var.name));
        }
    } else {
        return expr(T, E);
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

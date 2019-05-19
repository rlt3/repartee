#include <sstream>
#include <map>
#include "parser.hpp"
#include "machine.hpp"

/* 
 * Very hacky implementation of an environment for holding where variables
 * are stored.
 */
static int num_vars = 0;
static std::map<std::string, int> var_to_id;

/* create id for the var if it's not already in our map, otherwise return it */
int
add_or_get_var_id (std::string var)
{
    if (var_to_id.find(var) != var_to_id.end()) {
        return var_to_id[var];
    }
    num_vars++;
    var_to_id[var] = num_vars;
    return num_vars;
}

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

void comp (TokenizedInput &T, Node *N);

/* <atom> = number | name */
void
atom (TokenizedInput &T, Node *N)
{
    if (T.peek().type == TKN_NUMBER) {
        int num = T.expect(TKN_NUMBER).to_int();
        N->add_child(new AtomNode(N->env, num));
    }
    else {
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        N->add_child(new VarNode(N->env, var_id));
    }
}

///* <item> = (<comp>) | <atom> */
//void
//item (TokenizedInput &T, Node *N)
//{
//    if (T.peek().type == TKN_LEFT_PAREN) {
//        T.expect(TKN_LEFT_PAREN);
//        comp(T, N);
//        T.expect(TKN_RIGHT_PAREN);
//    }
//    else {
//        atom(T, N);
//    }
//}
//
///* 
// * <factor> := <item> <factorTail> | <item> 
// * <factorTail> := * <atom> <factorTail> | / <atom> <factorTail>
// */
//void
//factor (TokenizedInput &T, Node *N)
//{
//    Node *op;
//    item(T, N);
//
//    //while (!T.empty()) {
//    //    switch (T.peek().type) {
//    //        case TKN_MUL:
//    //            T.next();
//    //            op = new OperatorNode(N->env, TKN_MUL);
//    //            N->add_child(op);
//    //            item(T, op);
//    //            break;
//
//    //        case TKN_DIV:
//    //            T.next();
//    //            op = new OperatorNode(N->env, TKN_DIV);
//    //            N->add_child(op);
//    //            item(T, op);
//    //            break;
//
//    //        default:
//    //            return;
//    //    }
//    //}
//}
//
///*
// * <sum> := <factor> | <factor> <sumTail>
// * <sumTail> := + <factor> <sumTail> | - <factor> <sumTail>
// */
//void
//sum (TokenizedInput &T, Node *N)
//{
//    Node *op;
//    Node tmp(N->env);
//    factor(T, &tmp);
//
//    if (!(T.peek().type == TKN_ADD || T.peek().type == TKN_SUB))
//        goto exit;
//
//    while (!T.empty()) {
//        switch (T.peek().type) {
//            case TKN_ADD:
//                T.next();
//                op = new OperatorNode(N->env, TKN_ADD);
//                N->add_child(op);
//                op->merge(&tmp);
//                factor(T, op);
//                break;
//
//            case TKN_SUB:
//                T.next();
//                op = new OperatorNode(N->env, TKN_SUB);
//                N->add_child(op);
//                op->merge(&tmp);
//                factor(T, op);
//                break;
//
//            default:
//                goto exit;
//        }
//    }
//
//exit:
//    N->merge(&tmp);
//}
//
///* <comp> := <sum> == <sum> | <sum> */
//void
//comp (TokenizedInput &T, Node *N)
//{
//    sum(T, N);
//    //if (T.peek().type == TKN_DBL_EQUAL) {
//    //    Node *cmp = new OperatorNode(N->env, TKN_DBL_EQUAL);
//    //    sum(T, cmp);
//    //    T.expect(TKN_DBL_EQUAL);
//    //    sum(T, cmp);
//    //} else {
//    //    sum(T, N);
//    //}
//}

/*
 * <sum> := <factor> | <factor> <sumTail>
 * <sumTail> := + <factor> <sumTail> | - <factor> <sumTail>
 */
void
sum (TokenizedInput &T, Node *N)
{
    Node tmp(N->env);
    atom(T, &tmp);

    if (T.peek().type == TKN_ADD || T.peek().type == TKN_SUB) {
        Node *op = new OperatorNode(N->env, T.next().type);
        N->add_child(op);
        op->merge(&tmp);
        sum(T, op);
    } else {
        N->merge(&tmp);
    }
}

/* <expr> := <name> = <comp> | <comp> */
void
expr (TokenizedInput &T, Node *N)
{
    //if (T.peek().type == TKN_NAME && T.peek(1).type == TKN_EQUAL) {
    //    /* TODO: make this apart of the environment */
    //    int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
    //    T.expect(TKN_EQUAL);

    //    Node *assign = new AssignmentNode(N->env, var_id);
    //    N->add_child(assign);
    //    comp(T, assign);
    //}
    //else {
        sum(T, N);
    //}
}

Environment
parse (TokenizedInput &T)
{
    Environment E(NULL);

    T.set_runtime_error_func(error_func);

    while (!T.empty()) {
        Node *N = new Node(&E);
        E.add_node(N);
        expr(T, N);
        T.expect(TKN_SEMICOLON);
    }

    return E;
}

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

/* <item> = (<comp>) | <atom> */
void
item (TokenizedInput &T, Node *N)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        comp(T, N);
        T.expect(TKN_RIGHT_PAREN);
    }
    else {
        atom(T, N);
    }
}

/* 
 * <factor> := <item> <factorTail> | <item> 
 * <factorTail> := * <atom> <factorTail> | / <atom> <factorTail>
 */
void
factor (TokenizedInput &T, Node *N)
{
    Node tmp(N->env);
    item(T, &tmp);

    /* For explanation see: sum */
    if (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
        Node *next, *curr;
        next = curr = NULL;

        while (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
            next = new OperatorNode(N->env, T.next().type);
            if (curr)
                next->add_child(curr);
            else
                next->merge(&tmp);
            item(T, next);
            curr = next;
        }

        N->add_child(curr);
    } else {
        N->merge(&tmp);
    }
}

/*
 * <sum> := <factor> | <factor> <sumTail>
 * <sumTail> := + <factor> <sumTail> | - <factor> <sumTail>
 */
void
sum (TokenizedInput &T, Node *N)
{
    Node tmp(N->env);
    factor(T, &tmp);

    /* 
     * Recurse (iteratively) down the sum and construct the tree from the
     * bottom-up. If we constructed the tree as we parsed it then arithmetic
     * expressions would be right-associative when they are naturally left
     * associative.
     */
    if (T.peek().type == TKN_ADD || T.peek().type == TKN_SUB) {
        Node *next, *curr;
        next = curr = NULL;

        while (T.peek().type == TKN_ADD || T.peek().type == TKN_SUB) {
            next = new OperatorNode(N->env, T.next().type);
            /* 
             * add the last parsed expression as a child of the current one
             * since we are building the tree in 'reverse'
             */
            if (curr)
                next->add_child(curr);
            /* 
             * if !curr then we are at the top of the tree and we can add the
             * left part of the originally parsed expression (top of this func).
             */
            else
                next->merge(&tmp);
            factor(T, next);
            /* curr will be the child of whatever is parsed or child of N */
            curr = next;
        }

        N->add_child(curr);
    } else {
        N->merge(&tmp);
    }
}

/* <comp> := <sum> == <sum> | <sum> */
void
comp (TokenizedInput &T, Node *N)
{
    Node tmp(N->env);
    sum(T, &tmp);

    if (T.peek().type == TKN_DBL_EQUAL) {
        T.expect(TKN_DBL_EQUAL);
        Node *cmp = new OperatorNode(N->env, TKN_DBL_EQUAL);
        N->add_child(cmp);
        cmp->merge(&tmp);
        sum(T, cmp);
    } else {
        N->merge(&tmp);
    }
}

/* <expr> := <name> = <comp> | <comp> */
void
expr (TokenizedInput &T, Node *N)
{
    if (T.peek().type == TKN_NAME && T.peek(1).type == TKN_EQUAL) {
        /* TODO: make this apart of the environment */
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        T.expect(TKN_EQUAL);

        Node *assign = new AssignmentNode(N->env, var_id);
        N->add_child(assign);
        comp(T, assign);
    }
    else {
        comp(T, N);
    }
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

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

void comp (TokenizedInput &T, Environment &E, Node *N);
void expr (TokenizedInput &T, Environment &E, Node *N);

/* <atom> = number | name */
void
atom (TokenizedInput &T, Environment &E, Node *N)
{
    if (T.peek().type == TKN_NUMBER) {
        int num = T.expect(TKN_NUMBER).to_int();
        N->add_child(E.node(AtomNode(num)));
    }
    else {
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        N->add_child(E.node(VarNode(var_id)));
    }
}

/* <item> = (<comp>) | <atom> */
void
item (TokenizedInput &T, Environment &E, Node *N)
{
    if (T.peek().type == TKN_LEFT_PAREN) {
        T.expect(TKN_LEFT_PAREN);
        comp(T, E, N);
        T.expect(TKN_RIGHT_PAREN);
    }
    else {
        atom(T, E, N);
    }
}

/* 
 * <factor> := <item> <factorTail> | <item> 
 * <factorTail> := * <atom> <factorTail> | / <atom> <factorTail>
 */
void
factor (TokenizedInput &T, Environment &E, Node *N)
{
    Node tmp;
    item(T, E, &tmp);

    /* For explanation see: sum */
    if (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
        Node *next, *curr;
        next = curr = NULL;

        while (T.peek().type == TKN_MUL || T.peek().type == TKN_DIV) {
            next = E.node(OperatorNode(T.next().type));
            if (curr)
                next->add_child(curr);
            else
                next->merge(&tmp);
            item(T, E, next);
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
sum (TokenizedInput &T, Environment &E, Node *N)
{
    Node tmp;
    factor(T, E, &tmp);

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
            next = E.node(OperatorNode(T.next().type));
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
            factor(T, E, next);
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
comp (TokenizedInput &T, Environment &E, Node *N)
{
    Node tmp;
    sum(T, E, &tmp);

    if (T.peek().type == TKN_DBL_EQUAL) {
        T.expect(TKN_DBL_EQUAL);
        Node *cmp = E.node(OperatorNode(TKN_DBL_EQUAL));
        N->add_child(cmp);
        cmp->merge(&tmp);
        sum(T, E, cmp);
    } else {
        N->merge(&tmp);
    }
}

/* <cond_and> := <comp> && <cond_and> | <comp> */
void
cond_and (TokenizedInput &T, Environment &E, Node *N)
{
    comp(T, E, N);

    //if (T.peek().type == TKN_LOGICAL_AND) {
    //}
}

/* <cond_or> := <cond_and> || <cond_or> | <cond_and> */
void
cond_or (TokenizedInput &T, Environment &E, Node *N)
{
    cond_and(T, E, N);

    //if (T.peek().type == TKN_LOGICAL_OR) {
    //}
}

/* 
 * <expr> := <name> = <comp>
 *         | if (<cond_or>) { <expr>+ }
 *         | if (<cond_or>) { <expr>+ } else { <expr>+ }
 *         | <comp>
 */
void
expr (TokenizedInput &T, Environment &E, Node *N)
{
    if (T.peek().type == TKN_NAME && T.peek(1).type == TKN_EQUAL) {
        /* TODO: make this apart of the environment */
        int var_id = add_or_get_var_id(T.expect(TKN_NAME).str);
        T.expect(TKN_EQUAL);

        Node *assign = E.node(AssignmentNode(var_id));
        N->add_child(assign);
        comp(T, E, assign);
        T.expect(TKN_SEMICOLON);
    }
    else if (T.peek().type == TKN_IF) {
        Node *cond = E.node(Node("cond"));
        Node *trueb = E.node(Node("true"));
        Node *falseb = NULL;

        T.expect(TKN_IF);
        T.expect(TKN_LEFT_PAREN);
        cond_or(T, E, cond);
        T.expect(TKN_RIGHT_PAREN);

        T.expect(TKN_LEFT_BRACE);
        while (!(T.empty() || T.peek().type == TKN_RIGHT_BRACE)) {
            expr(T, E, trueb);
        }
        T.expect(TKN_RIGHT_BRACE);

        if (T.peek().type == TKN_ELSE) {
            falseb = E.node(Node("false"));
            T.expect(TKN_ELSE);
            T.expect(TKN_LEFT_BRACE);
            while (!(T.empty() || T.peek().type == TKN_RIGHT_BRACE)) {
                expr(T, E, falseb);
            }
            T.expect(TKN_RIGHT_BRACE);
        }

        Node *branch = E.node(CondBranchNode(cond, trueb, falseb));
        N->add_child(branch);
    }
    else {
        comp(T, E, N);
        T.expect(TKN_SEMICOLON);
    }

}

Environment
parse (TokenizedInput &T)
{
    T.set_runtime_error_func(error_func);
    Environment E(NULL);
    Node *N = E.root();

    while (!T.empty())
        expr(T, E, N);

    return E;
}

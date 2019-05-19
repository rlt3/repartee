#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <deque>

enum TokenType {
    TKN_WHITE,
    TKN_EOL,

    TKN_ADD,
    TKN_SUB,
    TKN_DIV,
    TKN_MUL,

    TKN_EQUAL,
    TKN_DBL_EQUAL,
    TKN_PERIOD,
    TKN_SEMICOLON,

    TKN_LEFT_PAREN,
    TKN_RIGHT_PAREN,
    TKN_LEFT_BRACKET,
    TKN_RIGHT_BRACKET,

    TKN_NUMBER,
    TKN_NAME
};

class Token {
public:
    Token ();
    Token (int line, int column, enum TokenType type, std::string s);

    int line;
    int column;
    enum TokenType type;
    std::string str;

    /* convert the string representation to an integer */
    int
    to_int ();
};

enum TokenizerError {
    TKNZR_MATCH
};

typedef void (*TokenizerErrorFunc) (int error_type, Token t, int data);

/*
 * Holds the list of tokens gathered from an input file. Acts as an input
 * stream of tokens implementing the common recursive descent parser functions:
 * next, peek, and match. Also contains the source lines for displaying
 * erroneous lines.
 */
class TokenizedInput {
public:
    TokenizedInput ();
    /* Only used when an error occured */
    TokenizedInput (std::string error);
    TokenizedInput (std::vector<std::string> lines, std::deque<Token> tokens);

    /* Pop the current token and return it */
    Token
    next ();

    /* Return the current token without popping it */
    Token
    peek ();

    /* Return the nth token without popping it */
    Token
    peek (unsigned nth);

    /* Match the current token. If it matches pop and return it */
    Token
    expect (enum TokenType type);

    /* set error function to call when an error occurs (after tokenization) */
    void
    set_runtime_error_func (TokenizerErrorFunc f);

    bool
    empty ();

    /* return line in the source code for a particular token */
    std::string
    source_line (Token &t);

    std::vector<std::string>&
    all_lines ();

    /* if the tokenizer ran into an error while tokenizing */
    bool
    has_error ();

    /* get the error string if there was an error */
    std::string
    get_error ();

protected:
    /* Return the EOL token */
    Token
    eol ();

    TokenizerErrorFunc error_func;
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::string error;
};

/* Tokenize an input file */
TokenizedInput tokenize (std::istream &input);

/* Get the string representation of a token type */
std::string tokentype_to_str (int type);

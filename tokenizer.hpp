#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <deque>

enum TokenType {
    TKN_WHITE,

    TKN_ADD,
    TKN_SUB,
    TKN_DIV,
    TKN_MUL,

    TKN_EQUAL,
    TKN_PERIOD,
    TKN_LEFT_PAREN,
    TKN_RIGHT_PAREN,
    TKN_LEFT_BRACKET,
    TKN_RIGHT_BRACKET,

    TKN_NUMBER,
    TKN_NAME
};

class Token {
public:
    Token ()
        : type(TKN_WHITE), str(std::string())
    { }

    Token (int l, int c, enum TokenType t, std::string s)
    /* add +1 to line and column counts for human numbers */
        : line(l + 1), column(c + 1), type(t), str(s)
    { }

    int line;
    int column;
    enum TokenType type;
    std::string str;

    int
    to_int ()
    {
        return std::stoi(str);
    }
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
    TokenizedInput ()
        : error_func(NULL), error("Unitialized")
    { }

    /* Only used when an error occured */
    TokenizedInput (std::string error)
        : error_func(NULL), error(error)
    { }

    TokenizedInput (std::vector<std::string> lines, std::deque<Token> tokens)
        : error_func(NULL), lines(lines), tokens(tokens), error(std::string())
    { }

    /* Pop the current token and return it */
    Token
    next ()
    {
        if (tokens.empty())
            return Token();
        Token t = tokens.front();
        tokens.pop_front();
        return t;
    }

    /* Return the current token without popping it */
    Token
    peek ()
    {
        if (tokens.empty())
            return Token();
        return tokens.front();
    }

    /* Match the current token. If it matches pop and return it */
    Token
    expect (enum TokenType type)
    {
        if (tokens.empty() || tokens.front().type != type) {
            error_func(TKNZR_MATCH, peek(), type);
        }
        return next();
    }

    void
    set_runtime_error_func (TokenizerErrorFunc f)
    {
        error_func = f;
    }

    bool
    terminal ()
    {
        /* TODO: if semicolon or empty */
        return true;
    }

    bool
    empty ()
    {
        return tokens.empty();
    }

    std::string
    source_line (Token &t)
    {
        if ((unsigned) t.line - 1 >= lines.size())
            return std::string();
        return lines[t.line - 1];
    }

    std::vector<std::string>&
    all_lines ()
    {
        return lines;
    }

    bool
    has_error ()
    {
        return !error.empty();
    }

    std::string
    get_error ()
    {
        return error;
    }


protected:
    TokenizerErrorFunc error_func;
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::string error;
};

/* Tokenize an input file */
TokenizedInput tokenize (std::istream &input);

/* Get the string representation of a token type */
std::string tokentype_to_str (int type);

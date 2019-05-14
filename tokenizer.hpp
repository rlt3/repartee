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
    { }

    Token (int l, int c, enum TokenType t, std::string s)
    /* add +1 to line and column counts for human numbers */
        : line(l + 1), column(c + 1), type(t), str(s)
    { }

    int line;
    int column;
    enum TokenType type;
    std::string str;
};

class TokenizedInput {
public:
    TokenizedInput ()
        : error("Unitialized")
    { }

    /* Only used when an error occured */
    TokenizedInput (std::string error)
        : error(error)
    { }

    TokenizedInput (std::vector<std::string> lines, std::deque<Token> tokens)
        : lines(lines), tokens(tokens), error(std::string())
    { }

    Token
    next ()
    {
        if (tokens.empty())
            return Token();
        Token t = tokens.front();
        tokens.pop_front();
        return t;
    }

    Token
    curr ()
    {
        if (tokens.empty())
            return Token();
        return tokens.front();
    }

    bool
    end ()
    {
        return tokens.empty();
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

    std::vector<std::string>&
    get_lines ()
    {
        return lines;
    }

protected:
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::string error;
};

/* Tokenize an input file */
TokenizedInput tokenize (std::istream &input);

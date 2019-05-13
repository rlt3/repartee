#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>

enum TokenType {
    TKN_ADD,
    TKN_SUB,
    TKN_DIV,
    TKN_MUL,
    TKN_DIGIT
};

class Token {
public:
    Token (int l, int c, enum TokenType t, int v)
    /* add +1 to line and column counts for human numbers */
        : line(l + 1), column(c + 1), type(t), value(v)
    { }

    int line;
    int column;
    enum TokenType type;
    int value;
};

class TokenizedInput {
public:
    TokenizedInput ()
        : error("Unitialized")
    { }

    TokenizedInput (std::string error)
        : error(error)
    { }

    TokenizedInput (std::vector<std::string> lines, std::deque<Token> tokens)
        : lines(lines), tokens(tokens), error(std::string())
    { }

    Token
    next ()
    {
        Token t = tokens.front();
        tokens.pop_front();
        return t;
    }

    Token
    curr ()
    {
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

TokenizedInput
tokenize (std::istream &input)
{
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::stringstream error;

    int line = 0;
    int col = 0;
    char c;
    int newline = false;

    /* setup first line */
    lines.push_back(std::string());

    while (input.get(c)) {
        switch (c) {
            case '+': tokens.push_back(Token(line, col, TKN_ADD, c)); break;
            case '-': tokens.push_back(Token(line, col, TKN_SUB, c)); break;
            case '/': tokens.push_back(Token(line, col, TKN_DIV, c)); break;
            case '*': tokens.push_back(Token(line, col, TKN_MUL, c)); break;

            case '0': case '1': case '2': case '3': case '4': 
            case '5': case '6': case '7': case '8': case '9':
                tokens.push_back(Token(line, col, TKN_DIGIT, c));
                break;

            /* reset the line */
            case '\n':
                newline = true;
                break;

            /* handle whitespace */
            case ' ': case '\t': case '\r':
                break;

            default:
                error << "Bad token at line " << line << " column " << col << ": `" << c << "'";
                return TokenizedInput(error.str());
        }

        if (newline) {
            col = 0;
            line++;
            lines.push_back(std::string());
            newline = false;
        } else {
            lines[line] += c;
            col++;
        }
    }

    return TokenizedInput(lines, tokens);
}

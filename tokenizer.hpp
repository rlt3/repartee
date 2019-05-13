#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <map>

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

static const std::string SymDigits = "0123456789";
static const std::string SymAlpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const std::string SymWhitespace = " \t\r";
static const std::map<char, enum TokenType> SymTypes = {
    { '+', TKN_ADD },
    { '-', TKN_SUB },
    { '/', TKN_DIV },
    { '*', TKN_MUL },
    { '.', TKN_PERIOD },
    { '(', TKN_LEFT_PAREN },
    { ')', TKN_RIGHT_PAREN },
    { '[', TKN_LEFT_BRACKET },
    { ']', TKN_RIGHT_BRACKET }
};

/* Join all individual characters of a particular type into a single token */
Token coalesce (char c, std::istream &input, int &line, int &col,
                enum TokenType type, const std::string &type_string);

bool
character_in (char c, const std::map<char, enum TokenType> &container)
{
    return (container.find(c) != container.end());
}

bool
character_in (char c, const std::string &container)
{
    return (container.find(c) != std::string::npos);
}

TokenizedInput
tokenize (std::istream &input)
{
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::stringstream error;

    int line = 0;
    int col = 0;
    Token t;
    char c;

    /* setup first line */
    lines.push_back(std::string());

    while (input.get(c)) {

        /* handle subsequent newlines and skip the character */
        if (c == '\n') {
            col = 0;
            line++;
            lines.push_back(std::string());
            continue;
        }

        if (character_in(c, SymTypes)) {
            t = Token(line, col, SymTypes.at(c), std::string(1, c));
        }
        else if (character_in(c, SymDigits)) {
            t = coalesce(c, input, line, col, TKN_NUMBER, SymDigits);
        }
        else if (character_in(c, SymWhitespace)) {
            t = coalesce(c, input, line, col, TKN_WHITE, SymWhitespace);
        }
        else {
            error << "Bad token at line " << line << " column " << col << ": `" << c << "'";
            return TokenizedInput(error.str());
        }

        if (t.type != TKN_WHITE)
            tokens.push_back(t);

        lines[line] += t.str;
        col++;
    }

    return TokenizedInput(lines, tokens);
}

Token
coalesce (char c, std::istream &input, int &line, int &col,
          enum TokenType type, const std::string &type_string)
{
    std::string str = "";
    str += c;
    while (input.get(c)) {
        if (character_in(c, type_string)) {
            str += c;
            col++;
        }
        else {
            input.putback(c);
            break;
        }
    }
    return Token(line, col, type, str);
}

#include <ctype.h>
#include <sstream>
#include <map>
#include "tokenizer.hpp"

static const std::map<char, enum TokenType> SymTypes = {
    { '+', TKN_ADD },
    { '-', TKN_SUB },
    { '/', TKN_DIV },
    { '*', TKN_MUL },
    { '=', TKN_EQUAL },
    { '.', TKN_PERIOD },
    { ';', TKN_SEMICOLON },
    { '(', TKN_LEFT_PAREN },
    { ')', TKN_RIGHT_PAREN },
    { '[', TKN_LEFT_BRACKET },
    { ']', TKN_RIGHT_BRACKET }
};

/* 
 * Join all individual characters of a particular type into a single token 
 * using the type function given.
 */
Token
coalesce (char c, std::istream &input, int &line, int &col,
          enum TokenType type, int (*istype)(int c))
{
    std::string str = "";
    str += c;
    while (input.get(c)) {
        if (istype(c)) {
            str += c;
            col++;
        }
        else {
            /* so next loop of tokenize doesn't skip the non `type' character */
            input.putback(c);
            break;
        }
    }
    return Token(line, col, type, str);
}

/* Tokenize an input file */
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

    /* lines are indexed by the line number and recreated from source tokens */
    lines.push_back(std::string());

    while (input.get(c)) {

        /* handle subsequent newlines and skip the character */
        if (c == '\n') {
            col = 0;
            line++;
            lines.push_back(std::string());
            continue;
        }

        if (SymTypes.find(c) != SymTypes.end()) {
            t = Token(line, col, SymTypes.at(c), std::string(1, c));
        }
        else if (isdigit(c)) {
            t = coalesce(c, input, line, col, TKN_NUMBER, isdigit);
        }
        else if (isalpha(c)) {
            t = coalesce(c, input, line, col, TKN_NAME, isalpha);
        }
        else if (isspace(c)) {
            t = coalesce(c, input, line, col, TKN_WHITE, isspace);
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

    /* remove extraneous empty line at the end */
    lines.pop_back();
    return TokenizedInput(lines, tokens);
}

std::string
tokentype_to_str (int type)
{
    switch (type) {
        case TKN_WHITE: return "whitespace";
        case TKN_ADD:   return "+";
        case TKN_SUB:   return "-";
        case TKN_DIV:   return "/";
        case TKN_MUL:   return "*";
        case TKN_EQUAL: return "=";
        case TKN_PERIOD: return ".";
        case TKN_SEMICOLON: return ";";
        case TKN_LEFT_PAREN: return "(";
        case TKN_RIGHT_PAREN: return ")";
        case TKN_LEFT_BRACKET: return "[";
        case TKN_RIGHT_BRACKET: return "]";
        case TKN_NUMBER: return "number";
        case TKN_NAME: return "name";
        case TKN_EOL: return "EOL";
        default: return "bad token";
    }
}

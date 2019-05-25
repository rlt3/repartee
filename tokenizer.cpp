#include <ctype.h>
#include <sstream>
#include <map>
#include "tokenizer.hpp"

static const std::map<std::string, enum TokenType> SymReserved = {
    { "if", TKN_IF },
    { "else", TKN_ELSE },
    { "and", TKN_LOGICAL_AND },
    { "or", TKN_LOGICAL_OR },
    { "func", TKN_FUNC },
    { "int", TKN_INT },
    { "float", TKN_FLOAT },
    { "string", TKN_STRING }
};

static const std::map<char, enum TokenType> SymTypes = {
    { '+', TKN_ADD },
    { '-', TKN_SUB },
    { '/', TKN_DIV },
    { '*', TKN_MUL },
    { '=', TKN_EQUAL },
    { '&', TKN_BITWISE_AND },
    { '|', TKN_BITWISE_OR },
    { '.', TKN_PERIOD },
    { ',', TKN_COMMA },
    { ';', TKN_SEMICOLON },
    { '(', TKN_LEFT_PAREN },
    { ')', TKN_RIGHT_PAREN },
    { '[', TKN_LEFT_BRACKET },
    { ']', TKN_RIGHT_BRACKET },
    { '{', TKN_LEFT_BRACE },
    { '}', TKN_RIGHT_BRACE }
};

Token::Token ()
    : line(0), column(0), type(TKN_WHITE), str(std::string())
{ }

Token::Token (int l, int c, enum TokenType t, std::string s)
/* add +1 to line and column counts for human numbers */
    : line(l + 1), column(c + 1), type(t), str(s)
{ }

int
Token::to_int ()
{
    return std::stoi(str);
}

TokenizedInput::TokenizedInput ()
    : error_func(NULL), error("Unitialized")
{ }

/* Only used when an error occured */
TokenizedInput::TokenizedInput (std::string error)
    : error_func(NULL), error(error)
{ }

TokenizedInput::TokenizedInput (std::vector<std::string> lines, std::deque<Token> tokens)
    : error_func(NULL), lines(lines), tokens(tokens), error(std::string())
{ }

/* Pop the current token and return it */
Token
TokenizedInput::next ()
{
    if (tokens.empty())
        return eol();
    Token t = tokens.front();
    tokens.pop_front();
    return t;
}

/* Return the current token without popping it */
Token
TokenizedInput::peek ()
{
    if (tokens.empty())
        return eol();
    return tokens.front();
}

/* Return the nth token without popping it */
Token
TokenizedInput::peek (unsigned nth)
{
    if (tokens.empty() || nth >= tokens.size())
        return eol();
    return tokens.at(nth);
}

/* Match the current token. If it matches pop and return it */
Token
TokenizedInput::expect (enum TokenType type)
{
    if (tokens.empty() || tokens.front().type != type) {
        error_func(TKNZR_MATCH, peek(), type);
    }
    return next();
}

void
TokenizedInput::set_runtime_error_func (TokenizerErrorFunc f)
{
    error_func = f;
}

bool
TokenizedInput::empty ()
{
    return tokens.empty();
}

std::string
TokenizedInput::source_line (Token &t)
{
    if ((unsigned) t.line - 1 >= lines.size())
        return std::string();
    return lines[t.line - 1];
}

std::vector<std::string>&
TokenizedInput::all_lines ()
{
    return lines;
}

bool
TokenizedInput::has_error ()
{
    return !error.empty();
}

std::string
TokenizedInput::get_error ()
{
    return error;
}

/* Return the EOL token */
Token
TokenizedInput::eol ()
{
    return Token(lines.size() - 1, lines.back().length(), TKN_EOL, "EOL");
}

/* If a particular character when repeated twice is a token */
bool
is_double (int c)
{
    switch (c) {
        case '=':
        case '&':
        case '|':
            return true;
    }
    return false;
}

Token
handle_double (char c, std::istream &input, int line, int &col, 
               enum TokenType other)
{
    char n;
    if (input.get(n) && c == n) {
        col++;
        switch (c) {
            case '=': return Token(line, col, TKN_DBL_EQUAL, "==");
            case '&': return Token(line, col, TKN_LOGICAL_AND, "&&");
            case '|': return Token(line, col, TKN_LOGICAL_OR, "||");
        }
    }
    input.putback(n);
    return Token(line, col, other, std::string(1, c));
}

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

/* 
 * Skip initial quote, consume all tokens until close quote then consume it 
 * and return.
 */
Token
coalesce_string (char c, std::istream &input, int &line, int &col)
{
    std::string str = "";
    while (input.get(c)) {
        col++;
        if (c != '"')
            str += c;
        else
            break;
    }
    return Token(line, col, TKN_STR, str);
}

bool
handle_comment (char c, std::istream &input, int &col, bool start)
{
    char s1 = start ? '/' : '*';
    char s2 = start ? '*' : '/';

    if (c == s1) {
        input.get(c);
        if (c == s2) {
            col++;
            return true;
        }
        else {
            input.putback(c);
        }
    }
    return false;
}

bool
is_endstring (char c)
{
    return (c == '"');
}

/* Tokenize an input file */
TokenizedInput
tokenize (std::istream &input)
{
    std::vector<std::string> lines;
    std::deque<Token> tokens;
    std::stringstream error;

    bool in_comment = false;
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

        /* treat all characters within a comment as whitespace */
        if (!in_comment && handle_comment(c, input, col, true))
            in_comment = true;

        if (in_comment) {
            t = Token(line, col, TKN_WHITE, " ");
            if (handle_comment(c, input, col, false))
                in_comment = false;
        } 
        else if (SymTypes.find(c) != SymTypes.end()) {
            if (is_double(c))
                t = handle_double(c, input, line, col, SymTypes.at(c));
            else
                t = Token(line, col, SymTypes.at(c), std::string(1, c));
        }
        else if (c == '"') {
            t = coalesce_string(c, input, line, col);
        }
        else if (isdigit(c)) {
            t = coalesce(c, input, line, col, TKN_NUMBER, isdigit);
        }
        else if (isalpha(c)) {
            t = coalesce(c, input, line, col, TKN_NAME, isalpha);
            if (SymReserved.find(t.str) != SymReserved.end())
                t.type = SymReserved.at(t.str);
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
        case TKN_IF: return "if";
        case TKN_ELSE: return "else";
        case TKN_FUNC: return "func";
        case TKN_INT: return "int";
        case TKN_FLOAT: return "float";
        case TKN_STRING: return "string";
        case TKN_EQUAL: return "=";
        case TKN_DBL_EQUAL: return "==";
        case TKN_BITWISE_AND: return "&";
        case TKN_BITWISE_OR: return "|";
        case TKN_LOGICAL_AND: return "&&";
        case TKN_LOGICAL_OR: return "||";
        case TKN_PERIOD: return ".";
        case TKN_COMMA: return ",";
        case TKN_SEMICOLON: return ";";
        case TKN_LEFT_PAREN: return "(";
        case TKN_RIGHT_PAREN: return ")";
        case TKN_LEFT_BRACKET: return "[";
        case TKN_RIGHT_BRACKET: return "]";
        case TKN_LEFT_BRACE: return "{";
        case TKN_RIGHT_BRACE: return "}";
        case TKN_STR: return "string type";
        case TKN_NUMBER: return "number";
        case TKN_NAME: return "name";
        case TKN_EOL: return "EOL";
        default: return "bad token";
    }
}

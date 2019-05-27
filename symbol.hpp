#pragma once

#include <string>

typedef enum _SymbolType {
    INTEGER,
    FLOAT,
    STRING,
    COMPOUND
} SymbolType;

class Symbol {
public:
    Symbol (int val)         : symtype(INTEGER) { data.i = val; }
    Symbol (float val)       : symtype(FLOAT)   { data.f = val; }
    Symbol (const char *val) : symtype(STRING)  { data.str = val; }

    SymbolType type () const { return symtype; }

    int         integer ()   { return data.i; }
    float       floating ()  { return data.f; }
    const char* string ()    { return data.str; }

protected:
    SymbolType symtype;
    union Payload {
        int i;
        float f;
        const char* str;
    } data;
};

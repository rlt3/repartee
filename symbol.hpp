#pragma once

typedef enum _SymbolType {
    INTEGER,
    FLOAT,
    STRING,
    VARIABLE,
    FUNCTION
} SymbolType;

class Symbol {
public:
    /* constant types */
    Symbol (int val)         : symtype(INTEGER) { data.i = val; }
    Symbol (float val)       : symtype(FLOAT)   { data.f = val; }
    Symbol (const char *val) : symtype(STRING)  { data.str = val; }

    /* constant values */
    int         integer ()   { return data.i; }
    float       floating ()  { return data.f; }
    const char* string ()    { return data.str; }

    /* 
     * Function or Variable which needs to lookup its value in the environment.
     * Symbols of these types are essentially references or pointers.
     */
    Symbol (SymbolType t, unsigned index)
        : symtype(t)
    {
        data.i = index;
    }

    SymbolType type () const { return symtype; }

protected:
    SymbolType symtype;
    union Payload {
        int i;
        float f;
        const char* str;
    } data;
};

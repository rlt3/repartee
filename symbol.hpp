#pragma once

#include "expression.hpp"
#include <assert.h>

/*
 * If all 'types' are defined by their storage sizes then there should be a
 * common interface for storage. Thus common types (int, floats, bytes) can get
 * pre-baked constructors whereas more complex types need a constructor with
 * size information. This size information is kept within the symbol itself so
 * it can remain dynamic within the VM but explicit in the final bytecode.
 */

typedef uint8_t Byte;

class Storage {
public:
    Storage (int value)
        : stride(1)
    {
        resize(sizeof(int));
        set(0, value);
    }

    Storage (double value)
        : stride(1)
    {
        resize(sizeof(double));
        set(0, value);
    }

    Storage (void *value)
        : stride(1)
    {
        resize(sizeof(void*));
        set(0, value);
    }

    Storage (const unsigned size, const unsigned stride)
    {
        assert(0);
        /* for arrays */

        /* 
         * TODO: figure out how to create structure types 
         * e.g { int, void*, int[10] }
         */
    }

    void
    resize (const unsigned num_bytes)
    {
        /* 0 initialize all memory */
        size = num_bytes;
        container.resize(num_bytes);
    }

    int
    integer_at (const unsigned index) const
    {
        assert((sizeof(int) * index) < container.size());
        return *(reinterpret_cast<const int*>(&container[0]) + index);
    }

    double
    floating_at (const unsigned index) const
    {
        assert((sizeof(double) * index) < container.size());
        return *(reinterpret_cast<const double*>(&container[0]) + index);
    }

    void*
    ptr_at (const unsigned index) const
    {
        assert((sizeof(void*) * index) < container.size());
        return *(reinterpret_cast<void* const*>(&container[0]) + index);
    }

    int
    integer () const
    {
        return integer_at(0);
    }

    double
    floating () const
    {
        return floating_at(0);
    }

    void*
    ptr () const
    {
        return ptr_at(0);
    }

    void
    set (unsigned index, int val)
    {
        assert((sizeof(int) * index) < container.size());
        *(reinterpret_cast<int*>(&container[0]) + index) = val;
    }

    void
    set (unsigned index, double val)
    {
        assert((sizeof(double) * index) < container.size());
        *(reinterpret_cast<double*>(&container[0]) + index) = val;
    }

    void
    set (unsigned index, void *val)
    {
        assert((sizeof(void*) * index) < container.size());
        *(reinterpret_cast<void**>(&container[0]) + index) = val;
    }

protected:
    /*
     * Guaranteed by the standard to have contiguous memory blocks. Therefore
     * it can be byte addressable and also dynamic because it is a container.
     */
    std::vector<Byte> container;
    unsigned size;
    unsigned stride;
};

typedef enum _SymbolType {
    INTEGER,
    DOUBLE,
    STRING,
    REFERENCE,
    FUNCTION
} SymbolType;

class Symbol {
public:
    Symbol (int val)
        : symtype(INTEGER)
        , storage(Storage(val))
    { }

    Symbol (double val)
        : symtype(DOUBLE)
        , storage(Storage(val))
    { }

    Symbol (void *val)
        : symtype(REFERENCE)
        , storage(Storage(val))
    { }

    SymbolType symtype;
    Storage storage;
protected:
};

//class Symbol {
//public:
//    /* constant types */
//    Symbol (int val)         : symtype(INTEGER)   { data.i = val; }
//    Symbol (float val)       : symtype(FLOAT)     { data.f = val; }
//    Symbol (const char *val) : symtype(STRING)    { data.str = val; }
//    Symbol (Expression *e)   : symtype(FUNCTION)  { data.expr = e; }
//    Symbol (Symbol *s)       : symtype(REFERENCE) { data.ref = s; }
//
//    /* constant values */
//    int         integer ()  { assert(symtype == INTEGER); return data.i; }
//    float       floating () { assert(symtype == FLOAT); return data.f; }
//    const char* string ()   { assert(symtype == STRING); return data.str; }
//    Expression* expr ()     { assert(symtype == FUNCTION); return data.expr; }
//    Symbol*     ref ()      { assert(symtype == REFERENCE); return data.ref; }
//
//    /* 
//     * Function or Variable which needs to lookup its value in the environment.
//     * Symbols of these types are essentially references or pointers.
//     */
//    Symbol (SymbolType t, unsigned index)
//        : symtype(t)
//    {
//        data.i = index;
//    }
//
//    SymbolType type () const { return symtype; }
//
//protected:
//    SymbolType symtype;
//    union Payload {
//        int i;
//        float f;
//        const char* str;
//        Expression *expr;
//        Symbol *ref;
//    } data;
//};

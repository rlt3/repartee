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
        resize(size);

        /* 
         * TODO: figure out how to create structure types 
         * e.g { int, void*, int[10] }
         */
    }

    void
    resize (const unsigned num_bytes)
    {
        /* 0 initializes all memory */
        size = num_bytes;
        container.resize(num_bytes);
    }

    /*
     * Containers are meant to be byte addressable so the index is which byte
     * to access. From the address of the first byte in the container we add
     * the index. Then we cast that index into whichever type. Before we do all
     * this we make sure that the index exists and also there's enough bytes to
     * handle a read at that index. This holds for both the 'get' methods and
     * set method.
     */

    int
    integer_at (const unsigned index) const
    {
        assert(index + sizeof(int) <= container.size());
        return *(reinterpret_cast<const int*>(&container[0] + index));
    }

    double
    floating_at (const unsigned index) const
    {
        assert(index + sizeof(double) <= container.size());
        return *(reinterpret_cast<const double*>(&container[0] + index));
    }

    void*
    ptr_at (const unsigned index) const
    {
        assert(index + sizeof(void*) <= container.size());
        return *(reinterpret_cast<void* const*>(&container[0] + index));
    }

    void
    set (unsigned index, int val)
    {
        assert(index + sizeof(int) <= container.size());
        *(reinterpret_cast<int*>(&container[0] + index)) = val;
    }

    void
    set (unsigned index, double val)
    {
        assert(index + sizeof(double) <= container.size());
        *(reinterpret_cast<double*>(&container[0] + index)) = val;
    }

    void
    set (unsigned index, void *val)
    {
        assert(index + sizeof(void*) <= container.size());
        *(reinterpret_cast<void**>(&container[0] + index)) = val;
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
    REFERENCE,
    FUNCTION,
    CHUNK        /* contiguous block of memory */
} SymbolType;

/*
 * A symbol essentially defines something in the program. Whether it be a
 * function, integer, or some complex structure, there should be some symbol
 * that acts as a handle to it.
 */
class Symbol {
public:
    Symbol (int val)
        : symtype(INTEGER)
        , storage(Storage(val))
        , was_allocated(false)
    { }

    Symbol (double val)
        : symtype(DOUBLE)
        , storage(Storage(val))
        , was_allocated(false)
    { }

    Symbol (Symbol *val)
        : symtype(REFERENCE)
        , storage(Storage(val))
        , was_allocated(false)
    { }

    Symbol (Expression *val)
        : symtype(FUNCTION)
        , storage(Storage(val))
        , was_allocated(false)
    { }

    Symbol (unsigned size, unsigned stride)
        : symtype(CHUNK)
        , storage(Storage(size, stride))
        , was_allocated(false)
    { }

    Symbol (const Symbol &other, bool was_allocated)
        : symtype(other.symtype)
        , storage(other.storage)
        , was_allocated(was_allocated)
    { }

    /* 
     * Create a new Symbol that's a clone of this one. This is how to allocate
     * Symbols and how symbols are typed allocated.
     */
    Symbol*
    allocate () const
    {
        return new Symbol(*this, true);
    }

    bool
    is_allocated () const
    {
        return was_allocated;
    }

    SymbolType
    type () const
    {
        return symtype;
    }

    /*
     * Return the value of the symbol iff it is the right type.
     */

    int
    integer_at (unsigned index) const
    { 
        return storage.integer_at(index);
    }

    double
    floating_at (unsigned index) const
    { 
        return storage.floating_at(index);
    }

    Expression*
    expr_at (unsigned index) const
    { 
        return (Expression*) storage.ptr_at(index);
    }

    Symbol*
    ref_at (unsigned index) const
    { 
        return (Symbol*) storage.ptr_at(index);
    }

    /*
     * Quality-of-Life methods for non-chunk access of values.
     */

    int
    integer () const
    { 
        assert(symtype == INTEGER);
        return integer_at(0);
    }

    float
    floating () const
    {
        assert(symtype == DOUBLE);
        return floating_at(0);
    }

    Expression*
    expr () const
    {
        assert(symtype == FUNCTION);
        return expr_at(0);
    }

    Symbol*
    ref () const
    {
        assert(symtype == REFERENCE);
        return ref_at(0);
    }

    /* 
     * Only accept indexes > 0 if the type is a chunk.
     */

    void
    set (unsigned index, int val)
    {
        assert(index == 0 || (index >= 0 && symtype == CHUNK));
        storage.set(index, val);
    }

    void
    set (unsigned index, double val)
    {
        assert(index == 0 || (index >= 0 && symtype == CHUNK));
        storage.set(index, val);
    }

    void
    set (unsigned index, void *val)
    {
        assert(index == 0 || (index >= 0 && symtype == CHUNK));
        storage.set(index, val);
    }

protected:
    SymbolType symtype;
    Storage storage;
    bool was_allocated;
};

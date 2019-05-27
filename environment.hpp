#pragma once

#include <vector>
#include <unordered_map>
#include "symbol.hpp"

class Environment {
public:
    /* The global or root environment */
    Environment ();
    /* For creating a child environment */
    Environment (Environment *parent);

    ~Environment ();

    /* 
     * Register a Constant or Local in the Environment. These methods are how
     * symbols get registered in the Environment. The methods return an index
     * into the constant and local pools respectively for lookup via
     * instructions later ('pushc' push constant, 'lload' local load).  Both
     * constants and locals can be looked up later by their string name.  A
     * constant's name is simply its value converted to a string.
     */
    int register_constant (int val);
    int register_constant (float val);
    int register_constant (std::string val);
    int register_local (std::string name, int val);
    int register_local (std::string name, float val);
    int register_local (std::string name, std::string val);

    /* Find a symbol by name or in the constant/local pool by index */
    Symbol* lookup (std::string name) const;
    Symbol* constant (int index) const;
    Symbol* local (int index) const;

    /* Create a new child environment */
    Environment* add_child ();

    /* Take ownership of an allocated symbol */
    void take_symbol (Symbol *sym);

protected:
    std::unordered_map<std::string, Symbol*> symbol_table;
    std::vector<Symbol*> constant_pool;
    std::vector<Symbol*> local_pool;

    Environment *parent;
    std::vector<Environment*> children;

private:
    /* Container for all allocated symbols for all environments */
    static std::vector<Symbol*> symbols;
};

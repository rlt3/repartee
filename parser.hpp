#pragma once
#include "tokenizer.hpp"
#include "machine.hpp"

/*
 * Parse an expression. Consumes tokens from TokenizedInput but not necessarily
 * all of them as the input may have many expressions. This function may be ran
 * many times for a particular TokenizedInput object.
 */
std::vector<Instruction> parse (TokenizedInput &T);

#pragma once

#include <iostream>
#include "instructions.hpp"
#include "expression.hpp"

Instruction create_instruction (Opcode op, int32_t imm);
Instruction create_instruction (Opcode op);
Opcode get_opcode (Instruction ins);
int32_t get_imm (Instruction ins);

/*
 * Evaluate the given expression in the environment and print to output stream.
 */
void evaluate (Expression &expr, std::ostream &output);

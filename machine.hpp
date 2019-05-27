#pragma once

#include <iostream>
#include "instructions.hpp"
#include "expression.hpp"
#include "environment.hpp"

/*
 * Evaluate the given expression in the environment and print to output stream.
 */
void evaluate (Expression &expr, Environment &env, std::ifstream &output);

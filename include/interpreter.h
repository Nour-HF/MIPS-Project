#pragma once

#include "parser.h"
#include "machine_state.h"
#include "instruction.h"
#include <string>
#include <iostream>
#include <cstdint>

class Interpreter {
public:
    Interpreter();
    machine_state run_stream(std::istream& input, uint64_t max_steps = 10000000ULL);
    machine_state run_file(const std::string& filename, uint64_t max_steps = 10000000ULL);

private:
    Parser parser;
};

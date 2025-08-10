#pragma once

#include "machine_state.h"
#include "instruction.h"
#include <string>
#include <cstdint>
#include <istream>

class Executor {
public:
    Executor();
    machine_state run_stream(std::istream& in, uint64_t max_steps = 100000ULL, bool verbose = false, uint32_t start_address = UINT32_MAX);
    machine_state run_file(const std::string& filename, uint64_t max_steps = 100000ULL, bool verbose = false, uint32_t start_address = UINT32_MAX);
};

#pragma once

#include "parser.h"
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdint>


class Assembler {
public:
    Assembler();
    std::vector<uint8_t> assemble_stream(std::istream& input);
    std::vector<uint8_t> assemble_file(const std::string& filename);
    void write_binary_to_stream(const std::vector<uint8_t>& bytes, std::ostream& out);
};
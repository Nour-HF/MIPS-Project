#include "../../include/assembler.h"
#include "../../include/parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

Assembler::Assembler() {
}

std::vector<uint8_t> Assembler::assemble_stream(std::istream& input) {
    Parser parser;
    ParseResult result = parser.parse_assembly(input);
    return parser.generate_binary(result);
}

std::vector<uint8_t> Assembler::assemble_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open input file: " + filename);
    }
    return assemble_stream(file);
}

void Assembler::write_binary_to_stream(const std::vector<uint8_t>& bytes, std::ostream& out) {
    if (!out.good()) {
        throw std::runtime_error("Output stream is not writable");
    }
    if (!bytes.empty()) {
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!out.good()) {
            throw std::runtime_error("Failed to write binary output to stream");
        }
    }
}

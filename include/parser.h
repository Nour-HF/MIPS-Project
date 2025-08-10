#pragma once

#include "instruction.h"
#include "machine_state.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <variant>
#include <cstdint>

// Assembly directive types
enum class DirectiveType {
    BYTE,      // .byte value
    HALF,      // .half value
    WORD,      // .word value
    ASCII,     // .ascii "string"
    ASCIIZ,    // .asciiz "string" (null-terminated)
    SPACE,     // .space size
    ALIGN,     // .align boundary (2^n bytes)
    TEXT,      // .text (code section)
    DATA,      // .data (data section)
    FLOAT,     // .float value
    DOUBLE     // .double value
};

// Assembly directive structure
struct AssemblyDirective {
    DirectiveType type;
    std::vector<uint32_t> values;  // For .byte, .half, .word, .space
    std::vector<float> float_values;    // For .float
    std::vector<double> double_values;  // For .double
    std::string text;              // For .ascii, .asciiz
    uint32_t alignment;            // For .align (N meaning 2^N bytes)
    // keep raw tokens for potential debugging/extension (not serialized)
    std::vector<std::string> raw_operands;

    AssemblyDirective(DirectiveType t) : type(t), alignment(0) {}
};

// Parsed line can be either an instruction or a directive
using ParsedLine = std::variant<Instruction, AssemblyDirective>;

// Label information
struct LabelInfo {
    std::string name;
    uint32_t address;

    LabelInfo(const std::string& n, uint32_t addr) : name(n), address(addr) {}
};

// Parse result containing instructions/directives and labels
struct ParseResult {
    std::vector<ParsedLine> lines;
    std::unordered_map<std::string, uint32_t> labels;
    uint32_t main_address;  // Address of main label
    bool has_main;

    ParseResult() : main_address(0), has_main(false) {}
};

class Parser {
public:
    Parser();

    // Parse assembly from input stream
    ParseResult parse_assembly(std::istream& input);

    // Parse assembly from string
    ParseResult parse_assembly(const std::string& assembly_text);

    // Parse assembly from file
    ParseResult parse_assembly_file(const std::string& filename);

    // Generate binary data from parse result
    std::vector<uint8_t> generate_binary(const ParseResult& result);

    // Get memory size needed for the program
    uint32_t calculate_memory_size(const ParseResult& result);

private:
    // Register name to Register enum mapping
    std::unordered_map<std::string, Register> register_map;

    // Instruction name to opcode/function mapping
    std::unordered_map<std::string, std::pair<Opcode, FunctionCode>> instruction_map;

    // Initialize mappings
    void init_register_map();
    void init_instruction_map();

    // Parsing helper functions
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string remove_comments(const std::string& line);
    std::string to_lower(const std::string& s);

    // Label parsing
    bool contains_label(const std::string& line);
    std::string extract_label(const std::string& line);

    // Directive parsing
    bool is_directive(const std::string& line);
    AssemblyDirective parse_directive(const std::string& line,
                                      const std::unordered_map<std::string, uint32_t>& labels);
    uint32_t get_directive_size_for_first_pass(const std::string& line, uint32_t current_pc);
    uint32_t get_directive_size(const AssemblyDirective& directive, uint32_t current_pc);

    // Instruction parsing
    bool is_instruction(const std::string& line);
    Instruction parse_instruction(const std::string& line,
                                  const std::unordered_map<std::string, uint32_t>& labels,
                                  uint32_t current_pc);

    // Register parsing
    Register parse_register(const std::string& reg_str);

    // Immediate value parsing
    uint32_t parse_immediate(const std::string& imm_str, const std::unordered_map<std::string, uint32_t>& labels);
    int32_t parse_signed_immediate(const std::string& imm_str, const std::unordered_map<std::string, uint32_t>& labels);

    // Memory operand parsing (e.g., "4($t0)")
    struct MemoryOperand {
        int32_t offset;
        Register base_register;
    };
    MemoryOperand parse_memory_operand(const std::string& mem_str);

    // String parsing for .ascii/.asciiz
    std::string parse_string_literal(const std::string& str_literal);

    // Two-pass parsing implementation
    ParseResult two_pass_parse(const std::vector<std::string>& lines);

    // First pass: collect labels and calculate section offsets
    void first_pass(const std::vector<std::string>& lines,
                    std::vector<std::tuple<std::string,bool,uint32_t,bool>>& items,
                    std::vector<std::tuple<std::string,bool,uint32_t>>& labels_raw,
                    uint32_t& text_size,
                    uint32_t& data_size);

    // Second pass: parse instructions & directives with resolved labels
    void second_pass(const std::vector<std::tuple<std::string,bool,uint32_t>>& items,
                     const std::unordered_map<std::string, uint32_t>& labels,
                     std::vector<ParsedLine>& parsed_lines);

    // Instruction type detection and parsing helpers
    InstructionFormat detect_instruction_format(const std::string& mnemonic);
    RInstruction parse_r_instruction(const std::string& mnemonic, const std::vector<std::string>& operands);
    IInstruction parse_i_instruction(const std::string& mnemonic, const std::vector<std::string>& operands,
                                     const std::unordered_map<std::string, uint32_t>& labels, uint32_t current_pc);
    JInstruction parse_j_instruction(const std::string& mnemonic, const std::vector<std::string>& operands,
                                     const std::unordered_map<std::string, uint32_t>& labels);

    // Error handling
    void throw_parse_error(const std::string& message, const std::string& line = "");
};
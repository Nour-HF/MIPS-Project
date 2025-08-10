#include "../../include/parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstring>
#include <limits>

Parser::Parser() {
    init_register_map();
    init_instruction_map();
}

ParseResult Parser::parse_assembly(std::istream& input) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(remove_comments(line));
    }
    return two_pass_parse(lines);
}

ParseResult Parser::parse_assembly(const std::string& assembly_text) {
    std::istringstream ss(assembly_text);
    return parse_assembly(ss);
}

ParseResult Parser::parse_assembly_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open assembly file: " + filename);
    }
    return parse_assembly(file);
}

std::vector<uint8_t> Parser::generate_binary(const ParseResult& result) {
    std::vector<uint8_t> binary;

    for (const auto& line : result.lines) {
        if (std::holds_alternative<Instruction>(line)) {
            uint32_t encoded = InstructionUtils::encode(std::get<Instruction>(line));
            // Store in little-endian format (LSB first) to match machine_state.
            binary.push_back(encoded & 0xFF);
            binary.push_back((encoded >> 8) & 0xFF);
            binary.push_back((encoded >> 16) & 0xFF);
            binary.push_back((encoded >> 24) & 0xFF);
        } else {
            const AssemblyDirective& dir = std::get<AssemblyDirective>(line);
            switch (dir.type) {
                case DirectiveType::BYTE:
                    for (auto v : dir.values) binary.push_back(static_cast<uint8_t>(v & 0xFF));
                    break;

                case DirectiveType::HALF:
                    for (auto v : dir.values) {
                        uint16_t val = static_cast<uint16_t>(v & 0xFFFF);
                        // little-endian: low byte first
                        binary.push_back(val & 0xFF);
                        binary.push_back((val >> 8) & 0xFF);
                    }
                    break;

                case DirectiveType::WORD:
                    for (auto v : dir.values) {
                        uint32_t val = v;
                        // little-endian: LSB first
                        binary.push_back(val & 0xFF);
                        binary.push_back((val >> 8) & 0xFF);
                        binary.push_back((val >> 16) & 0xFF);
                        binary.push_back((val >> 24) & 0xFF);
                    }
                    break;

                case DirectiveType::ASCII:
                case DirectiveType::ASCIIZ:
                    for (char c : dir.text) binary.push_back(static_cast<uint8_t>(c));
                    if (dir.type == DirectiveType::ASCIIZ) binary.push_back(0);
                    break;

                case DirectiveType::SPACE:
                    if (!dir.values.empty()) binary.insert(binary.end(), dir.values[0], 0);
                    break;

                case DirectiveType::ALIGN:
                    if (dir.alignment > 0) {
                        while (binary.size() % dir.alignment != 0) binary.push_back(0);
                    }
                    break;

                case DirectiveType::TEXT:
                case DirectiveType::DATA:
                    // section markers don't emit bytes
                    break;

                case DirectiveType::FLOAT:
                    for (float f : dir.float_values) {
                        uint32_t bits;
                        std::memcpy(&bits, &f, sizeof(float));
                        // little-endian
                        binary.push_back(bits & 0xFF);
                        binary.push_back((bits >> 8) & 0xFF);
                        binary.push_back((bits >> 16) & 0xFF);
                        binary.push_back((bits >> 24) & 0xFF);
                    }
                    break;

                case DirectiveType::DOUBLE:
                    for (double d : dir.double_values) {
                        uint64_t bits;
                        std::memcpy(&bits, &d, sizeof(double));
                        // little-endian: byte 0 = LSB
                        for (int i = 0; i < 8; ++i) {
                            binary.push_back((bits >> (i * 8)) & 0xFF);
                        }
                    }
                    break;
            }
        }
    }

    return binary;
}

uint32_t Parser::calculate_memory_size(const ParseResult& result) {
    return static_cast<uint32_t>(generate_binary(result).size());
}

void Parser::init_register_map() {
    // same mapping as before
    register_map["$zero"] = Register::ZERO;
    register_map["$0"] = Register::ZERO;

    register_map["$at"] = Register::AT;
    register_map["$1"] = Register::AT;

    register_map["$v0"] = Register::V0;
    register_map["$2"] = Register::V0;
    register_map["$v1"] = Register::V1;
    register_map["$3"] = Register::V1;

    register_map["$a0"] = Register::A0;
    register_map["$4"] = Register::A0;
    register_map["$a1"] = Register::A1;
    register_map["$5"] = Register::A1;
    register_map["$a2"] = Register::A2;
    register_map["$6"] = Register::A2;
    register_map["$a3"] = Register::A3;
    register_map["$7"] = Register::A3;

    register_map["$t0"] = Register::T0;
    register_map["$8"] = Register::T0;
    register_map["$t1"] = Register::T1;
    register_map["$9"] = Register::T1;
    register_map["$t2"] = Register::T2;
    register_map["$10"] = Register::T2;
    register_map["$t3"] = Register::T3;
    register_map["$11"] = Register::T3;
    register_map["$t4"] = Register::T4;
    register_map["$12"] = Register::T4;
    register_map["$t5"] = Register::T5;
    register_map["$13"] = Register::T5;
    register_map["$t6"] = Register::T6;
    register_map["$14"] = Register::T6;
    register_map["$t7"] = Register::T7;
    register_map["$15"] = Register::T7;

    register_map["$s0"] = Register::S0;
    register_map["$16"] = Register::S0;
    register_map["$s1"] = Register::S1;
    register_map["$17"] = Register::S1;
    register_map["$s2"] = Register::S2;
    register_map["$18"] = Register::S2;
    register_map["$s3"] = Register::S3;
    register_map["$19"] = Register::S3;
    register_map["$s4"] = Register::S4;
    register_map["$20"] = Register::S4;
    register_map["$s5"] = Register::S5;
    register_map["$21"] = Register::S5;
    register_map["$s6"] = Register::S6;
    register_map["$22"] = Register::S6;
    register_map["$s7"] = Register::S7;
    register_map["$23"] = Register::S7;

    register_map["$t8"] = Register::T8;
    register_map["$24"] = Register::T8;
    register_map["$t9"] = Register::T9;
    register_map["$25"] = Register::T9;

    register_map["$k0"] = Register::K0;
    register_map["$26"] = Register::K0;
    register_map["$k1"] = Register::K1;
    register_map["$27"] = Register::K1;

    register_map["$gp"] = Register::GP;
    register_map["$28"] = Register::GP;

    register_map["$sp"] = Register::SP;
    register_map["$29"] = Register::SP;

    register_map["$s8"] = Register::S8;
    register_map["$30"] = Register::S8;

    register_map["$ra"] = Register::RA;
    register_map["$31"] = Register::RA;
}

void Parser::init_instruction_map() {
    // R-type
    instruction_map["sll"] = {Opcode::RTYPE, FunctionCode::SLL};
    instruction_map["srl"] = {Opcode::RTYPE, FunctionCode::SRL};
    instruction_map["sra"] = {Opcode::RTYPE, FunctionCode::SRA};
    instruction_map["sllv"] = {Opcode::RTYPE, FunctionCode::SLLV};
    instruction_map["srlv"] = {Opcode::RTYPE, FunctionCode::SRLV};
    instruction_map["srav"] = {Opcode::RTYPE, FunctionCode::SRAV};
    instruction_map["jr"] = {Opcode::RTYPE, FunctionCode::JR};
    instruction_map["jalr"] = {Opcode::RTYPE, FunctionCode::JALR};
    instruction_map["mfhi"] = {Opcode::RTYPE, FunctionCode::MFHI};
    instruction_map["mthi"] = {Opcode::RTYPE, FunctionCode::MTHI};
    instruction_map["mflo"] = {Opcode::RTYPE, FunctionCode::MFLO};
    instruction_map["mtlo"] = {Opcode::RTYPE, FunctionCode::MTLO};
    instruction_map["mult"] = {Opcode::RTYPE, FunctionCode::MULT};
    instruction_map["multu"] = {Opcode::RTYPE, FunctionCode::MULTU};
    instruction_map["div"] = {Opcode::RTYPE, FunctionCode::DIV};
    instruction_map["divu"] = {Opcode::RTYPE, FunctionCode::DIVU};
    instruction_map["add"] = {Opcode::RTYPE, FunctionCode::ADD};
    instruction_map["addu"] = {Opcode::RTYPE, FunctionCode::ADDU};
    instruction_map["sub"] = {Opcode::RTYPE, FunctionCode::SUB};
    instruction_map["subu"] = {Opcode::RTYPE, FunctionCode::SUBU};
    instruction_map["and"] = {Opcode::RTYPE, FunctionCode::AND};
    instruction_map["or"] = {Opcode::RTYPE, FunctionCode::OR};
    instruction_map["xor"] = {Opcode::RTYPE, FunctionCode::XOR};
    instruction_map["nor"] = {Opcode::RTYPE, FunctionCode::NOR};
    instruction_map["slt"] = {Opcode::RTYPE, FunctionCode::SLT};
    instruction_map["sltu"] = {Opcode::RTYPE, FunctionCode::SLTU};

    // I-type
    instruction_map["beq"] = {Opcode::BEQ, FunctionCode::ADD};
    instruction_map["bne"] = {Opcode::BNE, FunctionCode::ADD};
    instruction_map["blez"] = {Opcode::BLEZ, FunctionCode::ADD};
    instruction_map["bgtz"] = {Opcode::BGTZ, FunctionCode::ADD};
    instruction_map["addi"] = {Opcode::ADDI, FunctionCode::ADD};
    instruction_map["addiu"] = {Opcode::ADDIU, FunctionCode::ADD};
    instruction_map["slti"] = {Opcode::SLTI, FunctionCode::ADD};
    instruction_map["sltiu"] = {Opcode::SLTIU, FunctionCode::ADD};
    instruction_map["andi"] = {Opcode::ANDI, FunctionCode::ADD};
    instruction_map["ori"] = {Opcode::ORI, FunctionCode::ADD};
    instruction_map["xori"] = {Opcode::XORI, FunctionCode::ADD};
    instruction_map["llo"] = {Opcode::LLO, FunctionCode::ADD};
    instruction_map["lhi"] = {Opcode::LHI, FunctionCode::ADD};
    instruction_map["trap"] = {Opcode::TRAP, FunctionCode::ADD};
    instruction_map["lb"] = {Opcode::LB, FunctionCode::ADD};
    instruction_map["lh"] = {Opcode::LH, FunctionCode::ADD};
    instruction_map["lw"] = {Opcode::LW, FunctionCode::ADD};
    instruction_map["lbu"] = {Opcode::LBU, FunctionCode::ADD};
    instruction_map["lhu"] = {Opcode::LHU, FunctionCode::ADD};
    instruction_map["sb"] = {Opcode::SB, FunctionCode::ADD};
    instruction_map["sh"] = {Opcode::SH, FunctionCode::ADD};
    instruction_map["sw"] = {Opcode::SW, FunctionCode::ADD};

    // J-type
    instruction_map["j"] = {Opcode::J, FunctionCode::ADD};
    instruction_map["jal"] = {Opcode::JAL, FunctionCode::ADD};
}

std::string Parser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

std::vector<std::string> Parser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ss(str);
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

std::string Parser::remove_comments(const std::string& line) {
    size_t pos = line.find('#');
    if (pos == std::string::npos) return trim(line);
    return trim(line.substr(0, pos));
}

std::string Parser::to_lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return std::tolower(c); });
    return r;
}

bool Parser::contains_label(const std::string& line) {
    return line.find(':') != std::string::npos;
}

std::string Parser::extract_label(const std::string& line) {
    size_t pos = line.find(':');
    if (pos == std::string::npos) return "";
    return trim(line.substr(0, pos));
}

bool Parser::is_directive(const std::string& line) {
    return !line.empty() && line[0] == '.';
}

bool Parser::is_instruction(const std::string& line) {
    return !line.empty() && !is_directive(line);
}

// ========================
// Directive Parsing (used in second pass; uses labels to resolve values if needed)
// ========================
AssemblyDirective Parser::parse_directive(const std::string& line, const std::unordered_map<std::string, uint32_t>& labels) {
    // split first token (directive) and the rest as operands string (to preserve commas & quotes)
    std::istringstream ss(line);
    std::string dir;
    ss >> dir;
    std::string rest;
    std::getline(ss, rest);
    rest = trim(rest);

    std::string ldir = to_lower(dir);
    if (ldir == ".byte") {
        AssemblyDirective d(DirectiveType::BYTE);
        if (!rest.empty()) {
            auto vals = split(rest, ',');
            for (auto &v : vals) {
                d.values.push_back(static_cast<uint32_t>(parse_signed_immediate(v, labels) & 0xFF));
                d.raw_operands.push_back(v);
            }
        }
        return d;
    } else if (ldir == ".half") {
        AssemblyDirective d(DirectiveType::HALF);
        if (!rest.empty()) {
            auto vals = split(rest, ',');
            for (auto &v : vals) {
                d.values.push_back(static_cast<uint32_t>(parse_signed_immediate(v, labels) & 0xFFFF));
                d.raw_operands.push_back(v);
            }
        }
        return d;
    } else if (ldir == ".word") {
        AssemblyDirective d(DirectiveType::WORD);
        if (!rest.empty()) {
            auto vals = split(rest, ',');
            for (auto &v : vals) {
                d.values.push_back(parse_immediate(v, labels));
                d.raw_operands.push_back(v);
            }
        }
        return d;
    } else if (ldir == ".ascii" || ldir == ".asciiz") {
        AssemblyDirective d(ldir == ".ascii" ? DirectiveType::ASCII : DirectiveType::ASCIIZ);
        if (!rest.empty()) {
            d.text = parse_string_literal(rest);
        } else {
            d.text = "";
        }
        return d;
    } else if (ldir == ".space") {
        AssemblyDirective d(DirectiveType::SPACE);
        if (!rest.empty()) {
            uint32_t n = static_cast<uint32_t>(parse_immediate(rest, labels));
            d.values.push_back(n);
            d.raw_operands.push_back(rest);
        }
        return d;
    } else if (ldir == ".align") {
        AssemblyDirective d(DirectiveType::ALIGN);
        if (!rest.empty()) {
            // interpret as integer N meaning align to 2^N bytes
            uint32_t n = static_cast<uint32_t>(parse_immediate(rest, labels));
            d.alignment = n;
        } else {
            d.alignment = 0;
        }
        return d;
    } else if (ldir == ".text") {
        AssemblyDirective d(DirectiveType::TEXT);
        return d;
    } else if (ldir == ".data") {
        AssemblyDirective d(DirectiveType::DATA);
        return d;
    } else if (ldir == ".float") {
        AssemblyDirective d(DirectiveType::FLOAT);
        if (!rest.empty()) {
            auto vals = split(rest, ',');
            for (auto &v : vals) {
                d.float_values.push_back(std::stof(v));
                d.raw_operands.push_back(v);
            }
        }
        return d;
    } else if (ldir == ".double") {
        AssemblyDirective d(DirectiveType::DOUBLE);
        if (!rest.empty()) {
            auto vals = split(rest, ',');
            for (auto &v : vals) {
                d.double_values.push_back(std::stod(v));
                d.raw_operands.push_back(v);
            }
        }
        return d;
    }

    throw_parse_error("Unknown directive: " + dir, line);
    return AssemblyDirective(DirectiveType::BYTE);
}

// For first pass we only need a size estimation; compute size based on textual operands and current pc for .align
uint32_t Parser::get_directive_size_for_first_pass(const std::string& line, uint32_t current_pc) {
    // identical parsing but don't require label resolution for sizes (count operands)
    std::istringstream ss(line);
    std::string dir;
    ss >> dir;
    std::string rest;
    std::getline(ss, rest);
    rest = trim(rest);
    std::string ldir = to_lower(dir);

    if (ldir == ".byte") {
        if (rest.empty()) return 0;
        auto vals = split(rest, ',');
        return static_cast<uint32_t>(vals.size());
    } else if (ldir == ".half") {
        if (rest.empty()) return 0;
        auto vals = split(rest, ',');
        return static_cast<uint32_t>(vals.size() * 2);
    } else if (ldir == ".word") {
        if (rest.empty()) return 0;
        auto vals = split(rest, ',');
        return static_cast<uint32_t>(vals.size() * 4);
    } else if (ldir == ".ascii") {
        if (rest.empty()) return 0;
        std::string text = parse_string_literal(rest);
        return static_cast<uint32_t>(text.size());
    } else if (ldir == ".asciiz") {
        if (rest.empty()) return 1; // at least null
        std::string text = parse_string_literal(rest);
        return static_cast<uint32_t>(text.size() + 1);
    } else if (ldir == ".space") {
        if (rest.empty()) return 0;
        uint32_t n = static_cast<uint32_t>(std::stoul(rest, nullptr, 0));
        return n;
    } else if (ldir == ".align") {
        if (rest.empty()) return 0;
        uint32_t n = static_cast<uint32_t>(std::stoul(rest, nullptr, 0));
        uint32_t align_bytes = (n >= 31) ? 0u : (1u << n);
        if (align_bytes == 0) return 0;
        uint32_t pad = (align_bytes - (current_pc % align_bytes)) % align_bytes;
        return pad;
    } else if (ldir == ".text" || ldir == ".data") {
        return 0;
    } else if (ldir == ".float") {
        if (rest.empty()) return 0;
        auto vals = split(rest, ',');
        return static_cast<uint32_t>(vals.size() * 4);
    } else if (ldir == ".double") {
        if (rest.empty()) return 0;
        auto vals = split(rest, ',');
        return static_cast<uint32_t>(vals.size() * 8);
    }

    throw_parse_error("Unknown directive (size calc): " + dir, line);
    return 0;
}

uint32_t Parser::get_directive_size(const AssemblyDirective& d, uint32_t current_pc) {
    switch (d.type) {
    case DirectiveType::BYTE:
        return static_cast<uint32_t>(d.values.size());
    case DirectiveType::HALF:
        return static_cast<uint32_t>(d.values.size() * 2);
    case DirectiveType::WORD:
        return static_cast<uint32_t>(d.values.size() * 4);
    case DirectiveType::ASCII:
        return static_cast<uint32_t>(d.text.size());
    case DirectiveType::ASCIIZ:
        return static_cast<uint32_t>(d.text.size() + 1);
    case DirectiveType::SPACE:
        return d.values.empty() ? 0 : d.values[0];
    case DirectiveType::ALIGN: {
        if (d.alignment == 0) return 0;
        uint32_t align_bytes = (d.alignment >= 31) ? 0u : (1u << d.alignment);
        if (align_bytes == 0) return 0;
        uint32_t pad = (align_bytes - (current_pc % align_bytes)) % align_bytes;
        return pad;
    }
    case DirectiveType::TEXT:
    case DirectiveType::DATA:
        return 0;
    case DirectiveType::FLOAT:
        return static_cast<uint32_t>(d.float_values.size() * 4);
    case DirectiveType::DOUBLE:
        return static_cast<uint32_t>(d.double_values.size() * 8);
    }
    return 0;
}

// ========================
// Instruction parsing
// ========================
Instruction Parser::parse_instruction(const std::string& line, const std::unordered_map<std::string, uint32_t>& labels, uint32_t current_pc) {
    std::string s = trim(line);
    if (s.empty()) throw_parse_error("Empty instruction line", line);

    // Extract mnemonic and operand string
    size_t pos = s.find_first_of(" \t");
    std::string mnemonic = (pos == std::string::npos) ? s : s.substr(0, pos);
    std::string rest = (pos == std::string::npos) ? "" : trim(s.substr(pos + 1));
    mnemonic = to_lower(mnemonic);

    auto it = instruction_map.find(mnemonic);
    if (it == instruction_map.end()) {
        throw_parse_error("Unknown instruction: " + mnemonic, line);
    }

    std::vector<std::string> operands;
    if (!rest.empty()) {
        operands = split(rest, ',');
    }

    InstructionFormat fmt = detect_instruction_format(mnemonic);
    if (fmt == InstructionFormat::R_TYPE) {
        return parse_r_instruction(mnemonic, operands);
    } else if (fmt == InstructionFormat::I_TYPE) {
        return parse_i_instruction(mnemonic, operands, labels, current_pc);
    } else {
        return parse_j_instruction(mnemonic, operands, labels);
    }
}

Register Parser::parse_register(const std::string& reg_str) {
    std::string r = trim(reg_str);
    if (r.empty()) throw_parse_error("Empty register", reg_str);

    // Accept registers with or without leading '$' (prefer with $). Our register_map uses names with $
    if (r.front() != '$') r = "$" + r;

    auto it = register_map.find(r);
    if (it == register_map.end()) {
        throw_parse_error("Unknown register: " + reg_str, reg_str);
    }
    return it->second;
}

std::string Parser::parse_string_literal(const std::string& str_literal) {
    std::string s = trim(str_literal);
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        std::string inner = s.substr(1, s.size() - 2);
        // unescape common sequences
        std::string out;
        for (size_t i = 0; i < inner.size(); ++i) {
            if (inner[i] == '\\' && i + 1 < inner.size()) {
                ++i;
                char c = inner[i];
                switch (c) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case '\\': out.push_back('\\'); break;
                case '"': out.push_back('"'); break;
                case '0': out.push_back('\0'); break;
                default: out.push_back(c); break;
                }
            } else {
                out.push_back(inner[i]);
            }
        }
        return out;
    }
    throw_parse_error("Invalid string literal: " + str_literal, str_literal);
    return {};
}

ParseResult Parser::two_pass_parse(const std::vector<std::string>& lines) {
    ParseResult result;

    // items: tuple<content, in_text(bool), offset(within section), is_directive>
    std::vector<std::tuple<std::string,bool,uint32_t,bool>> items;
    // labels raw: tuple<name, in_text, offset>
    std::vector<std::tuple<std::string,bool,uint32_t>> labels_raw;
    uint32_t text_size = 0;
    uint32_t data_size = 0;

    first_pass(lines, items, labels_raw, text_size, data_size);

    // Now compute absolute addresses: text starts at 0, data starts at text_size
    uint32_t text_base = 0;
    uint32_t data_base = text_size;

    std::unordered_map<std::string, uint32_t> labels;
    for (auto &l : labels_raw) {
        std::string name;
        bool in_text;
        uint32_t offset;
        std::tie(name, in_text, offset) = l;
        uint32_t abs = in_text ? (text_base + offset) : (data_base + offset);
        labels[name] = abs;
    }

    // second pass: produce parsed lines with code items first then data items
    // build a vector of items in order: text items then data items
    std::vector<std::tuple<std::string,bool,uint32_t>> ordered_items; // content, in_text, abs_addr (abs for second_pass)
    for (auto &it : items) {
        std::string content;
        bool in_text;
        uint32_t offset;
        bool is_dir;
        std::tie(content, in_text, offset, is_dir) = it;
        uint32_t abs = in_text ? (text_base + offset) : (data_base + offset);
        // we keep is_directive encoded by checking is_directive(content) in second_pass
        ordered_items.emplace_back(content, in_text, abs);
    }

    // Reorder: all in_text==true first (original relative order preserved), then in_text==false (original order)
    std::vector<std::tuple<std::string,bool,uint32_t>> final_order;
    for (auto &t : ordered_items) {
        if (std::get<1>(t)) final_order.push_back(t);
    }
    for (auto &t : ordered_items) {
        if (!std::get<1>(t)) final_order.push_back(t);
    }

    // Now convert final_order into the format expected by second_pass (content, abs_offset)
    std::vector<std::tuple<std::string,bool,uint32_t>> second_items;
    for (auto &t : final_order) {
        second_items.push_back(t);
    }

    // call second_pass
    second_pass(second_items, labels, result.lines);
    result.labels = labels;

    if (labels.find("main") != labels.end()) {
        result.has_main = true;
        result.main_address = labels["main"];
    }

    return result;
}

// In first_pass we collect items (instructions/directives) and raw labels, track text/data sections and their pcs
void Parser::first_pass(const std::vector<std::string>& lines,
                        std::vector<std::tuple<std::string,bool,uint32_t,bool>>& items,
                        std::vector<std::tuple<std::string,bool,uint32_t>>& labels_raw,
                        uint32_t& text_size,
                        uint32_t& data_size) {
    // items: content, in_text, offset(within that section), is_directive
    uint32_t text_pc = 0;
    uint32_t data_pc = 0;
    bool current_in_text = true; // default to text section unless .data appears

    for (auto raw_line : lines) {
        auto line = trim(raw_line);
        if (line.empty()) continue;

        // extract multiple labels at start of line (e.g., "L1: L2: instruction")
        bool again = true;
        while (again) {
            again = false;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string label = trim(line.substr(0, colon));
                if (!label.empty()) {
                    labels_raw.emplace_back(label, current_in_text, current_in_text ? text_pc : data_pc);
                }
                // remainder
                line = trim(line.substr(colon + 1));
                again = true; // in case of multiple labels
            }
        }

        if (line.empty()) continue;

        // check for section directives immediately which change current_in_text
        std::istringstream ss(line);
        std::string firstTok;
        ss >> firstTok;
        std::string lfirst = to_lower(firstTok);
        if (lfirst == ".text") {
            current_in_text = true;
            // record the directive as an item (so .text appears in result.lines but has no size)
            items.emplace_back(line, current_in_text, current_in_text ? text_pc : data_pc, true);
            continue;
        } else if (lfirst == ".data") {
            current_in_text = false;
            items.emplace_back(line, current_in_text, current_in_text ? text_pc : data_pc, true);
            continue;
        }

        if (is_directive(line)) {
            // compute size for this section using helper that only needs textual info
            uint32_t size = get_directive_size_for_first_pass(line, current_in_text ? text_pc : data_pc);
            // add an item for this directive
            items.emplace_back(line, current_in_text, current_in_text ? text_pc : data_pc, true);
            if (current_in_text) {
                text_pc += size;
            } else {
                data_pc += size;
            }
        } else {
            // instruction expected in text section. If encountered in data, we still accept but treat as text (you can change to error)
            if (!current_in_text) {
                // treat it as text anyway (some assemblers allow code in data, but typically this is an error)
                // For safety we will still accept but place it in text (and not advance data pc)
                // Mark a warning? we throw because it's usually an error
                //throw_parse_error("Instruction found in .data section", line);
            }
            // add instruction item
            items.emplace_back(line, true, text_pc, false);
            text_pc += 4;
        }
    }

    text_size = text_pc;
    data_size = data_pc;
}

// second_pass: items contains tuples (content, in_text, absolute_addr)
void Parser::second_pass(const std::vector<std::tuple<std::string,bool,uint32_t>>& items,
                         const std::unordered_map<std::string, uint32_t>& labels,
                         std::vector<ParsedLine>& parsed_lines) {
    for (auto &it : items) {
        std::string content;
        bool in_text;
        uint32_t abs;
        std::tie(content, in_text, abs) = it;

        if (is_directive(content)) {
            parsed_lines.push_back(parse_directive(content, labels));
        } else {
            parsed_lines.push_back(parse_instruction(content, labels, abs));
        }
    }
}

InstructionFormat Parser::detect_instruction_format(const std::string& mnemonic) {
    auto it = instruction_map.find(mnemonic);
    if (it == instruction_map.end()) {
        throw_parse_error("Unknown instruction format: " + mnemonic);
    }
    Opcode op = it->second.first;
    if (op == Opcode::RTYPE) return InstructionFormat::R_TYPE;
    if (op == Opcode::J || op == Opcode::JAL) return InstructionFormat::J_TYPE;
    return InstructionFormat::I_TYPE;
}

// R-type: handle common variants
RInstruction Parser::parse_r_instruction(const std::string& mnemonic, const std::vector<std::string>& operands) {
    auto found = instruction_map.find(mnemonic);
    if (found == instruction_map.end()) throw_parse_error("Unknown R instruction: " + mnemonic);
    FunctionCode funct = found->second.second;

    // helper lambda to get operand or throw
    auto get = [&](size_t idx)->std::string {
        if (idx >= operands.size()) throw_parse_error("Missing operand for " + mnemonic);
        return operands[idx];
    };

    // group by typical formats
    if (mnemonic == "sll" || mnemonic == "srl" || mnemonic == "sra") {
        // sll rd, rt, shamt
        std::string rd = get(0), rt = get(1), sh = get(2);
        uint8_t shamt = static_cast<uint8_t>(std::stoul(sh, nullptr, 0) & 0x1F);
        return RInstruction(static_cast<uint8_t>(0),
                            static_cast<uint8_t>(parse_register(rt)),
                            static_cast<uint8_t>(parse_register(rd)),
                            shamt,
                            funct);
    } else if (mnemonic == "sllv" || mnemonic == "srlv" || mnemonic == "srav") {
        // sllv rd, rt, rs  -> shamt in rs
        std::string rd = get(0), rt = get(1), rs = get(2);
        return RInstruction(static_cast<uint8_t>(parse_register(rs)),
                            static_cast<uint8_t>(parse_register(rt)),
                            static_cast<uint8_t>(parse_register(rd)),
                            0,
                            funct);
    } else if (mnemonic == "jr") {
        // jr rs
        std::string rs = get(0);
        return RInstruction(static_cast<uint8_t>(parse_register(rs)), 0, 0, 0, funct);
    } else if (mnemonic == "jalr") {
        // jalr rd, rs   or jalr rs
        if (operands.size() == 1) {
            std::string rs = get(0);
            return RInstruction(static_cast<uint8_t>(parse_register(rs)), 0, static_cast<uint8_t>(Register::RA), 0, funct);
        } else {
            std::string rd = get(0), rs = get(1);
            return RInstruction(static_cast<uint8_t>(parse_register(rs)),
                                0,
                                static_cast<uint8_t>(parse_register(rd)),
                                0,
                                funct);
        }
    } else if (mnemonic == "mfhi" || mnemonic == "mflo") {
        // mfhi rd
        std::string rd = get(0);
        return RInstruction(0, 0, static_cast<uint8_t>(parse_register(rd)), 0, funct);
    } else if (mnemonic == "mthi" || mnemonic == "mtlo") {
        // mthi rs
        std::string rs = get(0);
        return RInstruction(static_cast<uint8_t>(parse_register(rs)), 0, 0, 0, funct);
    } else if (mnemonic == "mult" || mnemonic == "multu" || mnemonic == "div" || mnemonic == "divu") {
        // mult rs, rt
        std::string rs = get(0), rt = get(1);
        return RInstruction(static_cast<uint8_t>(parse_register(rs)),
                            static_cast<uint8_t>(parse_register(rt)),
                            0,
                            0,
                            funct);
    } else {
        // default arithmetic/logical: add rd, rs, rt
        std::string rd = get(0), rs = get(1), rt = get(2);
        return RInstruction(static_cast<uint8_t>(parse_register(rs)),
                            static_cast<uint8_t>(parse_register(rt)),
                            static_cast<uint8_t>(parse_register(rd)),
                            0,
                            funct);
    }
}

// I-type parsing
IInstruction Parser::parse_i_instruction(const std::string& mnemonic, const std::vector<std::string>& operands,
                                         const std::unordered_map<std::string, uint32_t>& labels, uint32_t current_pc) {
    auto found = instruction_map.find(mnemonic);
    if (found == instruction_map.end()) throw_parse_error("Unknown I instruction: " + mnemonic);
    Opcode opcode = found->second.first;

    auto get = [&](size_t idx)->std::string {
        if (idx >= operands.size()) throw_parse_error("Missing operand for " + mnemonic);
        return operands[idx];
    };

    // Special-case: memory ops: rt, offset(base)
    if (mnemonic == "lw" || mnemonic == "sw" ||
        mnemonic == "lb" || mnemonic == "lbu" || mnemonic == "lh" || mnemonic == "lhu" ||
        mnemonic == "sb" || mnemonic == "sh") {
        std::string rt = get(0);
        std::string mem = get(1);
        auto mo = parse_memory_operand(mem);
        return IInstruction(opcode, static_cast<uint8_t>(mo.base_register), static_cast<uint8_t>(parse_register(rt)), static_cast<uint16_t>(mo.offset & 0xFFFF));
    }

    // Branches using labels: beq, bne
    if (mnemonic == "beq" || mnemonic == "bne") {
        std::string rs = get(0), rt = get(1), label = get(2);
        if (labels.find(label) == labels.end()) throw_parse_error("Unknown label in branch: " + label);
        uint32_t target = labels.at(label);
        int32_t diff = static_cast<int32_t>(target) - (static_cast<int32_t>(current_pc) + 4);
        int32_t offset = diff / 4;
        return IInstruction(opcode, static_cast<uint8_t>(parse_register(rs)), static_cast<uint8_t>(parse_register(rt)), static_cast<uint16_t>(offset & 0xFFFF));
    } else if (mnemonic == "blez" || mnemonic == "bgtz") {
        std::string rs = get(0), label = get(1);
        if (labels.find(label) == labels.end()) throw_parse_error("Unknown label in branch: " + label);
        uint32_t target = labels.at(label);
        int32_t diff = static_cast<int32_t>(target) - (static_cast<int32_t>(current_pc) + 4);
        int32_t offset = diff / 4;
        return IInstruction(opcode, static_cast<uint8_t>(parse_register(rs)), 0, static_cast<uint16_t>(offset & 0xFFFF));
    }

    // Special-case: trap (can be "trap" or "trap imm")
    if (mnemonic == "trap") {
        uint32_t imm = 0;
        if (operands.size() >= 1) {
            imm = parse_immediate(operands[0], labels) & 0xFFFFu;
        }
        // convention: put immediate in the immediate field, rs and rt unused (0)
        return IInstruction(opcode, 0 /*rs*/, 0 /*rt*/, static_cast<uint16_t>(imm));
    }

    // Immediate arithmetic/logical: addi rt, rs, imm  (and others)
    if (mnemonic == "addi" || mnemonic == "addiu" || mnemonic == "slti" || mnemonic == "sltiu" ||
        mnemonic == "andi" || mnemonic == "ori" || mnemonic == "xori" || mnemonic == "llo" || mnemonic == "lhi") {
        std::string rt = get(0), rs = get(1), imm_str = get(2);
        uint32_t imm = 0;
        if (mnemonic == "andi" || mnemonic == "ori" || mnemonic == "xori") {
            // zero-extended immediate
            imm = parse_immediate(imm_str, labels) & 0xFFFFu;
        } else {
            imm = static_cast<uint32_t>(parse_signed_immediate(imm_str, labels) & 0xFFFF);
        }
        return IInstruction(opcode, static_cast<uint8_t>(parse_register(rs)), static_cast<uint8_t>(parse_register(rt)), static_cast<uint16_t>(imm & 0xFFFF));
    }

    // default fallback
    throw_parse_error("Unhandled I-type instruction parsing: " + mnemonic);
    return IInstruction();
}


// J-type: j label
JInstruction Parser::parse_j_instruction(const std::string& mnemonic, const std::vector<std::string>& operands,
                                         const std::unordered_map<std::string, uint32_t>& labels) {
    auto found = instruction_map.find(mnemonic);
    if (found == instruction_map.end()) throw_parse_error("Unknown J instruction: " + mnemonic);
    Opcode opcode = found->second.first;
    if (operands.empty()) throw_parse_error("Missing target in jump: " + mnemonic);
    std::string target = operands[0];
    uint32_t addr = 0;
    if (labels.find(target) == labels.end()) {
        // may be an immediate address
        addr = parse_immediate(target, labels);
    } else {
        addr = labels.at(target);
    }
    uint32_t encoded_addr = (addr >> 2) & 0x03FFFFFFu;
    return JInstruction(opcode, encoded_addr);
}

// parse memory operand like "4($sp)" or "($t0)" or "-8($t1)"
Parser::MemoryOperand Parser::parse_memory_operand(const std::string& mem_str) {
    std::string s = trim(mem_str);
    size_t lparen = s.find('(');
    size_t rparen = s.find(')');
    if (lparen == std::string::npos || rparen == std::string::npos || rparen <= lparen) {
        throw_parse_error("Invalid memory operand: " + mem_str, mem_str);
    }
    std::string offset_str = trim(s.substr(0, lparen));
    int32_t offset = 0;
    if (!offset_str.empty()) {
        offset = std::stoi(offset_str);
    }
    std::string base_reg = trim(s.substr(lparen + 1, rparen - lparen - 1));
    return {offset, parse_register(base_reg)};
}

// immediate parsing: supports decimal, hex (0x...), negative, and label lookups
uint32_t Parser::parse_immediate(const std::string& imm_str, const std::unordered_map<std::string, uint32_t>& labels) {
    std::string s = trim(imm_str);
    if (s.empty()) throw_parse_error("Empty immediate", imm_str);
    // if it's a label
    if (labels.find(s) != labels.end()) return labels.at(s);
    // try numeric
    try {
        // allow 0x prefix, +/- via stoul/stoll
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            return static_cast<uint32_t>(std::stoul(s, nullptr, 16));
        }
        // standard base detection
        return static_cast<uint32_t>(std::stoul(s, nullptr, 0));
    } catch (...) {
        // maybe it's an expression like label+4 or label-8
        size_t plus = s.find('+');
        size_t minus = s.find('-', 1); // skip leading minus
        if (plus != std::string::npos) {
            std::string a = trim(s.substr(0, plus));
            std::string b = trim(s.substr(plus + 1));
            uint32_t aval = (labels.find(a) != labels.end()) ? labels.at(a) : static_cast<uint32_t>(std::stoul(a, nullptr, 0));
            uint32_t bval = static_cast<uint32_t>(std::stoul(b, nullptr, 0));
            return aval + bval;
        } else if (minus != std::string::npos) {
            std::string a = trim(s.substr(0, minus));
            std::string b = trim(s.substr(minus + 1));
            uint32_t aval = (labels.find(a) != labels.end()) ? labels.at(a) : static_cast<uint32_t>(std::stoul(a, nullptr, 0));
            uint32_t bval = static_cast<uint32_t>(std::stoul(b, nullptr, 0));
            return aval - bval;
        }
        throw_parse_error("Unable to parse immediate: " + imm_str, imm_str);
    }
    return 0;
}

int32_t Parser::parse_signed_immediate(const std::string& imm_str, const std::unordered_map<std::string, uint32_t>& labels) {
    std::string s = trim(imm_str);
    if (s.empty()) throw_parse_error("Empty signed immediate", imm_str);
    // label
    if (labels.find(s) != labels.end()) return static_cast<int32_t>(labels.at(s));
    try {
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            return static_cast<int32_t>(std::stol(s, nullptr, 16));
        }
        return static_cast<int32_t>(std::stol(s, nullptr, 0));
    } catch (...) {
        // expression support as in parse_immediate
        size_t plus = s.find('+');
        size_t minus = s.find('-', 1);
        if (plus != std::string::npos) {
            std::string a = trim(s.substr(0, plus));
            std::string b = trim(s.substr(plus + 1));
            int32_t aval = (labels.find(a) != labels.end()) ? static_cast<int32_t>(labels.at(a)) : static_cast<int32_t>(std::stol(a, nullptr, 0));
            int32_t bval = static_cast<int32_t>(std::stol(b, nullptr, 0));
            return aval + bval;
        } else if (minus != std::string::npos) {
            std::string a = trim(s.substr(0, minus));
            std::string b = trim(s.substr(minus + 1));
            int32_t aval = (labels.find(a) != labels.end()) ? static_cast<int32_t>(labels.at(a)) : static_cast<int32_t>(std::stol(a, nullptr, 0));
            int32_t bval = static_cast<int32_t>(std::stol(b, nullptr, 0));
            return aval - bval;
        }
        throw_parse_error("Unable to parse signed immediate: " + imm_str, imm_str);
    }
    return 0;
}

void Parser::throw_parse_error(const std::string& message, const std::string& line) {
    if (!line.empty()) {
        throw std::runtime_error("Parse error: " + message + " in line: " + line);
    }
    throw std::runtime_error("Parse error: " + message);
}

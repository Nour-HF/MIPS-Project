#pragma once

#include "machine_state.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <variant>
#include <unordered_map>
#include <functional>

enum class InstructionFormat {
    R_TYPE,    // Register format
    I_TYPE,    // Immediate format  
    J_TYPE     // Jump format
};

// Opcodes for different instruction types
enum class Opcode : uint8_t {
    RTYPE = 0x00,    // R-type instructions (differentiated by function)
    J = 0x02,        // Jump
    JAL = 0x03,      // Jump and link
    BEQ = 0x04,      // Branch if equal
    BNE = 0x05,      // Branch if not equal
    BLEZ = 0x06,     // Branch if <= 0
    BGTZ = 0x07,     // Branch if > 0
    ADDI = 0x08,     // Add immediate
    ADDIU = 0x09,    // Add immediate unsigned
    SLTI = 0x0A,     // Set less than immediate
    SLTIU = 0x0B,    // Set less than immediate unsigned
    ANDI = 0x0C,     // AND immediate
    ORI = 0x0D,      // OR immediate
    XORI = 0x0E,     // XOR immediate
    LLO = 0x18,      // Load low immediate
    LHI = 0x19,      // Load high immediate
    TRAP = 0x1A,     // Trap/syscall
    LB = 0x20,       // Load byte
    LH = 0x21,       // Load halfword
    LW = 0x23,       // Load word
    LBU = 0x24,      // Load byte unsigned
    LHU = 0x25,      // Load halfword unsigned
    SB = 0x28,       // Store byte
    SH = 0x29,       // Store halfword
    SW = 0x2B        // Store word
};

// Function codes for R-type instructions
enum class FunctionCode : uint8_t {
    SLL = 0x00,      // Shift left logical
    SRL = 0x02,      // Shift right logical
    SRA = 0x03,      // Shift right arithmetic
    SLLV = 0x04,     // Shift left logical variable
    SRLV = 0x06,     // Shift right logical variable
    SRAV = 0x07,     // Shift right arithmetic variable
    JR = 0x08,       // Jump register
    JALR = 0x09,     // Jump and link register
    MFHI = 0x10,     // Move from HI
    MTHI = 0x11,     // Move to HI
    MFLO = 0x12,     // Move from LO
    MTLO = 0x13,     // Move to LO
    MULT = 0x18,     // Multiply
    MULTU = 0x19,    // Multiply unsigned
    DIV = 0x1A,      // Divide
    DIVU = 0x1B,     // Divide unsigned
    ADD = 0x20,      // Add
    ADDU = 0x21,     // Add unsigned
    SUB = 0x22,      // Subtract
    SUBU = 0x23,     // Subtract unsigned
    AND = 0x24,      // Bitwise AND
    OR = 0x25,       // Bitwise OR
    XOR = 0x26,      // Bitwise XOR
    NOR = 0x27,      // Bitwise NOR
    SLT = 0x2A,      // Set less than
    SLTU = 0x2B      // Set less than unsigned
};

// Syscall enumeration
enum class Syscall : uint8_t {
    PRINT_INT = 0,
    PRINT_CHARACTER = 1,
    PRINT_STRING = 2,
    READ_INT = 3,
    READ_CHARACTER = 4,
    EXIT = 5
};

// R-type instruction format
struct RInstruction {
    Opcode opcode;           // Always 0x00 for R-type
    uint8_t rs;              // Source register 1
    uint8_t rt;              // Source register 2
    uint8_t rd;              // Destination register
    uint8_t shamt;           // Shift amount
    FunctionCode funct;      // Function code
    
    RInstruction(uint8_t rs = 0, uint8_t rt = 0, uint8_t rd = 0, 
                 uint8_t shamt = 0, FunctionCode funct = FunctionCode::ADD)
        : opcode(Opcode::RTYPE), rs(rs), rt(rt), rd(rd), shamt(shamt), funct(funct) {}
};

// I-type instruction format
struct IInstruction {
    Opcode opcode;           // Instruction opcode
    uint8_t rs;              // Source register
    uint8_t rt;              // Target register
    uint16_t immediate;      // 16-bit immediate value
    
    IInstruction(Opcode opcode = Opcode::ADDI, uint8_t rs = 0, 
                 uint8_t rt = 0, uint16_t immediate = 0)
        : opcode(opcode), rs(rs), rt(rt), immediate(immediate) {}
};

// J-type instruction format
struct JInstruction {
    Opcode opcode;           // Jump opcode
    uint32_t address;        // 26-bit jump address
    
    JInstruction(Opcode opcode = Opcode::J, uint32_t address = 0)
        : opcode(opcode), address(address & 0x3FFFFFF) {}  // Mask to 26 bits
};

// Variant to hold any instruction type
using Instruction = std::variant<RInstruction, IInstruction, JInstruction>;

class InstructionUtils {
public:
    // Encoding: Convert instruction to 32-bit binary
    static uint32_t encode(const Instruction& instr);
    
    // Decoding: Convert 32-bit binary to instruction
    static Instruction decode(uint32_t binary);
    
    // Get instruction format
    static InstructionFormat get_format(const Instruction& instr);
    
    // Get instruction name for debugging
    static std::string get_name(const Instruction& instr);
    
    // Helper functions for sign/zero extension
    static uint32_t sign_extend_16(uint16_t value);
    static uint32_t zero_extend_16(uint16_t value);
    static uint32_t sign_extend_8(uint8_t value);
    static uint32_t zero_extend_8(uint8_t value);
    
private:
    // Helper functions for encoding/decoding
    static uint32_t encode_r_type(const RInstruction& instr);
    static uint32_t encode_i_type(const IInstruction& instr);
    static uint32_t encode_j_type(const JInstruction& instr);
    
    static RInstruction decode_r_type(uint32_t binary);
    static IInstruction decode_i_type(uint32_t binary);
    static JInstruction decode_j_type(uint32_t binary);
};

// Forward declaration
class machine_state;

// Instruction execution function type
using InstructionHandler = std::function<void(machine_state&, const Instruction&)>;

// Execution engine class
class InstructionExecutor {
public:
    InstructionExecutor(std::istream& input = std::cin, std::ostream& output = std::cout);
    
    // Execute a single instruction
    void execute(machine_state& state, const Instruction& instr);
    
    // Set custom I/O streams for testing
    void set_io_streams(std::istream& input, std::ostream& output);
    
private:
    // I/O stream references for syscalls
    std::istream& input_stream;
    std::ostream& output_stream;
    
    // Handler map for different instructions
    std::unordered_map<std::string, InstructionHandler> handlers;
    
    // Initialize all instruction handlers
    void init_handlers();
    
    // Helper function to get instruction key for handler lookup
    std::string get_instruction_key(const Instruction& instr);
    
    // Individual instruction implementations
    // R-type instruction handlers
    void execute_sll(machine_state& state, const RInstruction& instr);
    void execute_srl(machine_state& state, const RInstruction& instr);
    void execute_sra(machine_state& state, const RInstruction& instr);
    void execute_sllv(machine_state& state, const RInstruction& instr);
    void execute_srlv(machine_state& state, const RInstruction& instr);
    void execute_srav(machine_state& state, const RInstruction& instr);
    void execute_jr(machine_state& state, const RInstruction& instr);
    void execute_jalr(machine_state& state, const RInstruction& instr);
    void execute_mfhi(machine_state& state, const RInstruction& instr);
    void execute_mthi(machine_state& state, const RInstruction& instr);
    void execute_mflo(machine_state& state, const RInstruction& instr);
    void execute_mtlo(machine_state& state, const RInstruction& instr);
    void execute_mult(machine_state& state, const RInstruction& instr);
    void execute_multu(machine_state& state, const RInstruction& instr);
    void execute_div(machine_state& state, const RInstruction& instr);
    void execute_divu(machine_state& state, const RInstruction& instr);
    void execute_add(machine_state& state, const RInstruction& instr);
    void execute_addu(machine_state& state, const RInstruction& instr);
    void execute_sub(machine_state& state, const RInstruction& instr);
    void execute_subu(machine_state& state, const RInstruction& instr);
    void execute_and(machine_state& state, const RInstruction& instr);
    void execute_or(machine_state& state, const RInstruction& instr);
    void execute_xor(machine_state& state, const RInstruction& instr);
    void execute_nor(machine_state& state, const RInstruction& instr);
    void execute_slt(machine_state& state, const RInstruction& instr);
    void execute_sltu(machine_state& state, const RInstruction& instr);

    // I-type instruction handlers
    void execute_beq(machine_state& state, const IInstruction& instr);
    void execute_bne(machine_state& state, const IInstruction& instr);
    void execute_blez(machine_state& state, const IInstruction& instr);
    void execute_bgtz(machine_state& state, const IInstruction& instr);
    void execute_addi(machine_state& state, const IInstruction& instr);
    void execute_addiu(machine_state& state, const IInstruction& instr);
    void execute_slti(machine_state& state, const IInstruction& instr);
    void execute_sltiu(machine_state& state, const IInstruction& instr);
    void execute_andi(machine_state& state, const IInstruction& instr);
    void execute_ori(machine_state& state, const IInstruction& instr);
    void execute_xori(machine_state& state, const IInstruction& instr);
    void execute_llo(machine_state& state, const IInstruction& instr);
    void execute_lhi(machine_state& state, const IInstruction& instr);
    void execute_lb(machine_state& state, const IInstruction& instr);
    void execute_lh(machine_state& state, const IInstruction& instr);
    void execute_lw(machine_state& state, const IInstruction& instr);
    void execute_lbu(machine_state& state, const IInstruction& instr);
    void execute_lhu(machine_state& state, const IInstruction& instr);
    void execute_sb(machine_state& state, const IInstruction& instr);
    void execute_sh(machine_state& state, const IInstruction& instr);
    void execute_sw(machine_state& state, const IInstruction& instr);

    // J-type instruction handlers
    void execute_j(machine_state& state, const JInstruction& instr);
    void execute_jal(machine_state& state, const JInstruction& instr);

    // Syscall handling
    void execute_trap(machine_state& state, const IInstruction& instr);
    void handle_syscall(machine_state& state, Syscall syscall_num);
};
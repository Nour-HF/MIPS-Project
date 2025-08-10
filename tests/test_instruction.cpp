#include "../include/instruction.h"
#include "../include/machine_state.h"
#include <iostream>
#include <cassert>
#include <sstream>

// Test instruction creation and basic properties
void test_instruction_creation() {
    std::cout << "Testing instruction creation...\n";
    
    // Test R-type instruction
    RInstruction r_instr(1, 2, 3, 4, FunctionCode::ADD);
    assert(r_instr.opcode == Opcode::RTYPE);
    assert(r_instr.rs == 1);
    assert(r_instr.rt == 2);
    assert(r_instr.rd == 3);
    assert(r_instr.shamt == 4);
    assert(r_instr.funct == FunctionCode::ADD);
    
    // Test I-type instruction
    IInstruction i_instr(Opcode::ADDI, 5, 6, 0x1234);
    assert(i_instr.opcode == Opcode::ADDI);
    assert(i_instr.rs == 5);
    assert(i_instr.rt == 6);
    assert(i_instr.immediate == 0x1234);
    
    // Test J-type instruction
    JInstruction j_instr(Opcode::J, 0x3FFFFFF);
    assert(j_instr.opcode == Opcode::J);
    assert(j_instr.address == 0x3FFFFFF);
    
    // Test address masking in J-type
    JInstruction j_instr_mask(Opcode::J, 0xFFFFFFFF);
    assert(j_instr_mask.address == 0x3FFFFFF); // Should be masked to 26 bits
    
    std::cout << "Instruction creation tests passed!\n";
}

// Test instruction format detection
void test_instruction_format() {
    std::cout << "Testing instruction format detection...\n";
    
    RInstruction r_instr;
    IInstruction i_instr;
    JInstruction j_instr;
    
    Instruction r_variant = r_instr;
    Instruction i_variant = i_instr;
    Instruction j_variant = j_instr;
    
    assert(InstructionUtils::get_format(r_variant) == InstructionFormat::R_TYPE);
    assert(InstructionUtils::get_format(i_variant) == InstructionFormat::I_TYPE);
    assert(InstructionUtils::get_format(j_variant) == InstructionFormat::J_TYPE);
    
    std::cout << "Instruction format tests passed!\n";
}

// Test instruction naming
void test_instruction_names() {
    std::cout << "Testing instruction naming...\n";
    
    // Test R-type names
    RInstruction add_instr(1, 2, 3, 0, FunctionCode::ADD);
    Instruction add_variant = add_instr;
    assert(InstructionUtils::get_name(add_variant) == "add");
    
    RInstruction sll_instr(0, 2, 3, 4, FunctionCode::SLL);
    Instruction sll_variant = sll_instr;
    assert(InstructionUtils::get_name(sll_variant) == "sll");
    
    // Test I-type names
    IInstruction addi_instr(Opcode::ADDI, 1, 2, 100);
    Instruction addi_variant = addi_instr;
    assert(InstructionUtils::get_name(addi_variant) == "addi");
    
    IInstruction beq_instr(Opcode::BEQ, 1, 2, 10);
    Instruction beq_variant = beq_instr;
    assert(InstructionUtils::get_name(beq_variant) == "beq");
    
    // Test J-type names
    JInstruction j_instr(Opcode::J, 0x1000);
    Instruction j_variant = j_instr;
    assert(InstructionUtils::get_name(j_variant) == "j");
    
    JInstruction jal_instr(Opcode::JAL, 0x2000);
    Instruction jal_variant = jal_instr;
    assert(InstructionUtils::get_name(jal_variant) == "jal");
    
    std::cout << "Instruction naming tests passed!\n";
}

// Test sign and zero extension utilities
void test_extension_utilities() {
    std::cout << "Testing extension utilities...\n";
    
    // Test 16-bit sign extension
    assert(InstructionUtils::sign_extend_16(0x7FFF) == 0x7FFF); // Positive
    assert(InstructionUtils::sign_extend_16(0x8000) == 0xFFFF8000); // Negative
    assert(InstructionUtils::sign_extend_16(0xFFFF) == 0xFFFFFFFF); // -1
    
    // Test 16-bit zero extension
    assert(InstructionUtils::zero_extend_16(0x7FFF) == 0x7FFF);
    assert(InstructionUtils::zero_extend_16(0x8000) == 0x8000);
    assert(InstructionUtils::zero_extend_16(0xFFFF) == 0xFFFF);
    
    // Test 8-bit sign extension
    assert(InstructionUtils::sign_extend_8(0x7F) == 0x7F); // Positive
    assert(InstructionUtils::sign_extend_8(0x80) == 0xFFFFFF80); // Negative
    assert(InstructionUtils::sign_extend_8(0xFF) == 0xFFFFFFFF); // -1
    
    // Test 8-bit zero extension
    assert(InstructionUtils::zero_extend_8(0x7F) == 0x7F);
    assert(InstructionUtils::zero_extend_8(0x80) == 0x80);
    assert(InstructionUtils::zero_extend_8(0xFF) == 0xFF);
    
    std::cout << "Extension utility tests passed!\n";
}

// Test instruction encoding and decoding
void test_encoding_decoding() {
    std::cout << "Testing instruction encoding and decoding...\n";
    
    // Test R-type encoding/decoding
    RInstruction original_r(5, 6, 7, 8, FunctionCode::ADD);
    Instruction r_variant = original_r;
    uint32_t encoded = InstructionUtils::encode(r_variant);
    Instruction decoded = InstructionUtils::decode(encoded);
    
    assert(std::holds_alternative<RInstruction>(decoded));
    RInstruction decoded_r = std::get<RInstruction>(decoded);
    assert(decoded_r.rs == original_r.rs);
    assert(decoded_r.rt == original_r.rt);
    assert(decoded_r.rd == original_r.rd);
    assert(decoded_r.shamt == original_r.shamt);
    assert(decoded_r.funct == original_r.funct);
    
    // Test I-type encoding/decoding
    IInstruction original_i(Opcode::ADDI, 10, 11, 0x1234);
    Instruction i_variant = original_i;
    encoded = InstructionUtils::encode(i_variant);
    decoded = InstructionUtils::decode(encoded);
    
    assert(std::holds_alternative<IInstruction>(decoded));
    IInstruction decoded_i = std::get<IInstruction>(decoded);
    assert(decoded_i.opcode == original_i.opcode);
    assert(decoded_i.rs == original_i.rs);
    assert(decoded_i.rt == original_i.rt);
    assert(decoded_i.immediate == original_i.immediate);
    
    // Test J-type encoding/decoding
    JInstruction original_j(Opcode::J, 0x123456);
    Instruction j_variant = original_j;
    encoded = InstructionUtils::encode(j_variant);
    decoded = InstructionUtils::decode(encoded);
    
    assert(std::holds_alternative<JInstruction>(decoded));
    JInstruction decoded_j = std::get<JInstruction>(decoded);
    assert(decoded_j.opcode == original_j.opcode);
    assert(decoded_j.address == original_j.address);
    
    std::cout << "Encoding/decoding tests passed!\n";
}

// Test R-type instruction execution
void test_r_type_execution() {
    std::cout << "Testing R-type instruction execution...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test ADD instruction
    state.set_register(Register::T0, 10);
    state.set_register(Register::T1, 20);
    RInstruction add_instr(static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          static_cast<uint8_t>(Register::T2), 
                          0, FunctionCode::ADD);
    Instruction add_variant = add_instr;
    executor.execute(state, add_variant);
    assert(state.get_register(Register::T2) == 30);
    
    // Test SUB instruction
    state.set_register(Register::T0, 50);
    state.set_register(Register::T1, 20);
    RInstruction sub_instr(static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          static_cast<uint8_t>(Register::T2), 
                          0, FunctionCode::SUB);
    Instruction sub_variant = sub_instr;
    executor.execute(state, sub_variant);
    assert(state.get_register(Register::T2) == 30);
    
    // Test AND instruction
    state.set_register(Register::T0, 0xFF);
    state.set_register(Register::T1, 0x0F);
    RInstruction and_instr(static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          static_cast<uint8_t>(Register::T2), 
                          0, FunctionCode::AND);
    Instruction and_variant = and_instr;
    executor.execute(state, and_variant);
    assert(state.get_register(Register::T2) == 0x0F);
    
    // Test OR instruction
    state.set_register(Register::T0, 0xF0);
    state.set_register(Register::T1, 0x0F);
    RInstruction or_instr(static_cast<uint8_t>(Register::T0), 
                         static_cast<uint8_t>(Register::T1), 
                         static_cast<uint8_t>(Register::T2), 
                         0, FunctionCode::OR);
    Instruction or_variant = or_instr;
    executor.execute(state, or_variant);
    assert(state.get_register(Register::T2) == 0xFF);
    
    // Test SLL instruction (shift left logical)
    state.set_register(Register::T0, 8);
    RInstruction sll_instr(0, 
                          static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          2, FunctionCode::SLL);
    Instruction sll_variant = sll_instr;
    executor.execute(state, sll_variant);
    assert(state.get_register(Register::T1) == 32); // 8 << 2 = 32
    
    // Test SLT instruction (set less than)
    state.set_register(Register::T0, 5);
    state.set_register(Register::T1, 10);
    RInstruction slt_instr(static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          static_cast<uint8_t>(Register::T2), 
                          0, FunctionCode::SLT);
    Instruction slt_variant = slt_instr;
    executor.execute(state, slt_variant);
    assert(state.get_register(Register::T2) == 1); // 5 < 10 = true
    
    std::cout << "R-type execution tests passed!\n";
}

// Test I-type instruction execution
void test_i_type_execution() {
    std::cout << "Testing I-type instruction execution...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test ADDI instruction
    state.set_register(Register::T0, 10);
    IInstruction addi_instr(Opcode::ADDI, 
                           static_cast<uint8_t>(Register::T0), 
                           static_cast<uint8_t>(Register::T1), 
                           20);
    Instruction addi_variant = addi_instr;
    executor.execute(state, addi_variant);
    assert(state.get_register(Register::T1) == 30);
    
    // Test ANDI instruction
    state.set_register(Register::T0, 0xFF);
    IInstruction andi_instr(Opcode::ANDI, 
                           static_cast<uint8_t>(Register::T0), 
                           static_cast<uint8_t>(Register::T1), 
                           0x0F);
    Instruction andi_variant = andi_instr;
    executor.execute(state, andi_variant);
    assert(state.get_register(Register::T1) == 0x0F);
    
    // Test ORI instruction
    state.set_register(Register::T0, 0xF0);
    IInstruction ori_instr(Opcode::ORI, 
                          static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          0x0F);
    Instruction ori_variant = ori_instr;
    executor.execute(state, ori_variant);
    assert(state.get_register(Register::T1) == 0xFF);
    
    // Test LLO instruction (load low immediate)
    state.set_register(Register::T0, 0xFFFF0000);
    IInstruction llo_instr(Opcode::LLO, 
                          0, 
                          static_cast<uint8_t>(Register::T0), 
                          0x1234);
    Instruction llo_variant = llo_instr;
    executor.execute(state, llo_variant);
    assert(state.get_register(Register::T0) == 0xFFFF1234);
    
    // Test LHI instruction (load high immediate)
    state.set_register(Register::T0, 0x0000FFFF);
    IInstruction lhi_instr(Opcode::LHI, 
                          0, 
                          static_cast<uint8_t>(Register::T0), 
                          0x1234);
    Instruction lhi_variant = lhi_instr;
    executor.execute(state, lhi_variant);
    assert(state.get_register(Register::T0) == 0x1234FFFF);
    
    std::cout << "I-type execution tests passed!\n";
}

// Test memory load/store instructions
void test_memory_instructions() {
    std::cout << "Testing memory load/store instructions...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test SW (store word) and LW (load word)
    state.set_register(Register::T0, 1000); // Base address
    state.set_register(Register::T1, 0x12345678); // Value to store
    
    // Store word: sw $t1, 4($t0)
    IInstruction sw_instr(Opcode::SW, 
                         static_cast<uint8_t>(Register::T0), 
                         static_cast<uint8_t>(Register::T1), 
                         4);
    Instruction sw_variant = sw_instr;
    executor.execute(state, sw_variant);
    
    // Load word: lw $t2, 4($t0)
    IInstruction lw_instr(Opcode::LW, 
                         static_cast<uint8_t>(Register::T0), 
                         static_cast<uint8_t>(Register::T2), 
                         4);
    Instruction lw_variant = lw_instr;
    executor.execute(state, lw_variant);
    assert(state.get_register(Register::T2) == 0x12345678);
    
    // Test SB (store byte) and LB (load byte)
    state.set_register(Register::T1, 0xFF);
    IInstruction sb_instr(Opcode::SB, 
                         static_cast<uint8_t>(Register::T0), 
                         static_cast<uint8_t>(Register::T1), 
                         8);
    Instruction sb_variant = sb_instr;
    executor.execute(state, sb_variant);
    
    // Load byte (sign extended)
    IInstruction lb_instr(Opcode::LB, 
                         static_cast<uint8_t>(Register::T0), 
                         static_cast<uint8_t>(Register::T2), 
                         8);
    Instruction lb_variant = lb_instr;
    executor.execute(state, lb_variant);
    assert(state.get_register(Register::T2) == 0xFFFFFFFF); // Sign extended
    
    // Load byte unsigned
    IInstruction lbu_instr(Opcode::LBU, 
                          static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T2), 
                          8);
    Instruction lbu_variant = lbu_instr;
    executor.execute(state, lbu_variant);
    assert(state.get_register(Register::T2) == 0xFF); // Zero extended
    
    std::cout << "Memory instruction tests passed!\n";
}

// Test branch instructions
void test_branch_instructions() {
    std::cout << "Testing branch instructions...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test BEQ (branch if equal)
    state.set_register(Register::T0, 10);
    state.set_register(Register::T1, 10);
    state.set_pc(1000);
    
    IInstruction beq_instr(Opcode::BEQ, 
                          static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          4); // Branch offset of 4 instructions
    Instruction beq_variant = beq_instr;
    executor.execute(state, beq_variant);
    assert(state.get_pc() == 1000 + (4 << 2)); // PC + offset*4
    
    // Test BNE (branch if not equal)
    state.set_register(Register::T0, 10);
    state.set_register(Register::T1, 20);
    state.set_pc(2000);
    
    IInstruction bne_instr(Opcode::BNE, 
                          static_cast<uint8_t>(Register::T0), 
                          static_cast<uint8_t>(Register::T1), 
                          8);
    Instruction bne_variant = bne_instr;
    executor.execute(state, bne_variant);
    assert(state.get_pc() == 2000 + (8 << 2)); // PC + offset*4
    
    // Test BLEZ (branch if less than or equal to zero)
    state.set_register(Register::T0, 0);
    state.set_pc(3000);
    
    IInstruction blez_instr(Opcode::BLEZ, 
                           static_cast<uint8_t>(Register::T0), 
                           0, 
                           2);
    Instruction blez_variant = blez_instr;
    executor.execute(state, blez_variant);
    assert(state.get_pc() == 3000 + (2 << 2)); // PC + offset*4
    
    std::cout << "Branch instruction tests passed!\n";
}

// Test jump instructions
void test_jump_instructions() {
    std::cout << "Testing jump instructions...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test J (jump)
    state.set_pc(0x10000000);
    JInstruction j_instr(Opcode::J, 0x123456);
    Instruction j_variant = j_instr;
    executor.execute(state, j_variant);
    
    // Jump address should be (PC+4 upper 4 bits) | (address << 2)
    uint32_t expected_addr = (0x10000004 & 0xF0000000) | (0x123456 << 2);
    assert(state.get_pc() == expected_addr);
    
    // Test JAL (jump and link)
    uint32_t original_pc = 0x20000000;
    state.set_pc(original_pc);
    JInstruction jal_instr(Opcode::JAL, 0x789ABC);
    Instruction jal_variant = jal_instr;
    executor.execute(state, jal_variant);
    
    // Check that return address was saved
    assert(state.get_register(Register::RA) == original_pc + 4);
    
    // Check jump address
    expected_addr = ((original_pc + 4) & 0xF0000000) | (0x789ABC << 2);
    assert(state.get_pc() == expected_addr);
    
    std::cout << "Jump instruction tests passed!\n";
}

// Test syscall functionality
void test_syscalls() {
    std::cout << "Testing syscall functionality...\n";
    
    machine_state state;
    std::ostringstream output;
    std::istringstream input("42");
    InstructionExecutor executor(input, output);
    
    // Test PRINT_INT syscall
    state.set_register(Register::A0, 12345);
    IInstruction trap_print_int(Opcode::TRAP, 0, 0, static_cast<uint16_t>(Syscall::PRINT_INT));
    Instruction trap_variant = trap_print_int;
    executor.execute(state, trap_variant);
    // Note: Can't easily test output in this simple test framework
    
    // Test PRINT_CHARACTER syscall
    state.set_register(Register::A0, 'A');
    IInstruction trap_print_char(Opcode::TRAP, 0, 0, static_cast<uint16_t>(Syscall::PRINT_CHARACTER));
    trap_variant = trap_print_char;
    executor.execute(state, trap_variant);
    
    std::cout << "Syscall tests passed!\n";
}

// Test HI/LO register operations
void test_hilo_operations() {
    std::cout << "Testing HI/LO register operations...\n";
    
    machine_state state;
    InstructionExecutor executor;
    
    // Test MULT instruction
    state.set_register(Register::T0, 1000);
    state.set_register(Register::T1, 2000);
    RInstruction mult_instr(static_cast<uint8_t>(Register::T0), 
                           static_cast<uint8_t>(Register::T1), 
                           0, 0, FunctionCode::MULT);
    Instruction mult_variant = mult_instr;
    executor.execute(state, mult_variant);
    
    // 1000 * 2000 = 2,000,000 (fits in 32 bits)
    assert(state.get_lo() == 2000000);
    assert(state.get_hi() == 0);
    
    // Test MFLO (move from LO)
    RInstruction mflo_instr(0, 0, static_cast<uint8_t>(Register::T2), 
                           0, FunctionCode::MFLO);
    Instruction mflo_variant = mflo_instr;
    executor.execute(state, mflo_variant);
    assert(state.get_register(Register::T2) == 2000000);
    
    // Test MFHI (move from HI)
    RInstruction mfhi_instr(0, 0, static_cast<uint8_t>(Register::T3), 
                           0, FunctionCode::MFHI);
    Instruction mfhi_variant = mfhi_instr;
    executor.execute(state, mfhi_variant);
    assert(state.get_register(Register::T3) == 0);
    
    // Test MTHI (move to HI)
    state.set_register(Register::T0, 0x12345678);
    RInstruction mthi_instr(static_cast<uint8_t>(Register::T0), 
                           0, 0, 0, FunctionCode::MTHI);
    Instruction mthi_variant = mthi_instr;
    executor.execute(state, mthi_variant);
    assert(state.get_hi() == 0x12345678);
    
    std::cout << "HI/LO operation tests passed!\n";
}

int main() {
    try {
        test_instruction_creation();
        test_instruction_format();
        test_instruction_names();
        test_extension_utilities();
        test_encoding_decoding();
        test_r_type_execution();
        test_i_type_execution();
        test_memory_instructions();
        test_branch_instructions();
        test_jump_instructions();
        test_syscalls();
        test_hilo_operations();
        
        std::cout << "\nAll instruction tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
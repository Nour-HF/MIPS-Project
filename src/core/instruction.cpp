#include "../../include/instruction.h"
#include "../../include/machine_state.h"
#include <variant>
#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>

// InstructionUtils implementation
uint32_t InstructionUtils::encode(const Instruction& instr) {
    return std::visit([](const auto& i) {
        using T = std::decay_t<decltype(i)>;
        if constexpr (std::is_same_v<T, RInstruction>) {
            return encode_r_type(i);
        } else if constexpr (std::is_same_v<T, IInstruction>) {
            return encode_i_type(i);
        } else if constexpr (std::is_same_v<T, JInstruction>) {
            return encode_j_type(i);
        }
        return uint32_t{0};
    }, instr);
}

Instruction InstructionUtils::decode(uint32_t binary) {
    uint8_t opcode = (binary >> 26) & 0x3F;
    
    if (opcode == 0x00) {
        // R-type instruction
        return decode_r_type(binary);
    } else if (opcode == 0x02 || opcode == 0x03) {
        // J-type instruction
        return decode_j_type(binary);
    } else {
        // I-type instruction
        return decode_i_type(binary);
    }
}

InstructionFormat InstructionUtils::get_format(const Instruction& instr) {
    return std::visit([](const auto& i) {
        using T = std::decay_t<decltype(i)>;
        if constexpr (std::is_same_v<T, RInstruction>) {
            return InstructionFormat::R_TYPE;
        } else if constexpr (std::is_same_v<T, IInstruction>) {
            return InstructionFormat::I_TYPE;
        } else if constexpr (std::is_same_v<T, JInstruction>) {
            return InstructionFormat::J_TYPE;
        }
        return InstructionFormat::R_TYPE;
    }, instr);
}

std::string InstructionUtils::get_name(const Instruction& instr) {
    return std::visit([](const auto& i) -> std::string {
        using T = std::decay_t<decltype(i)>;
        if constexpr (std::is_same_v<T, RInstruction>) {
            switch (i.funct) {
                case FunctionCode::SLL: return "sll";
                case FunctionCode::SRL: return "srl";
                case FunctionCode::SRA: return "sra";
                case FunctionCode::SLLV: return "sllv";
                case FunctionCode::SRLV: return "srlv";
                case FunctionCode::SRAV: return "srav";
                case FunctionCode::JR: return "jr";
                case FunctionCode::JALR: return "jalr";
                case FunctionCode::MFHI: return "mfhi";
                case FunctionCode::MTHI: return "mthi";
                case FunctionCode::MFLO: return "mflo";
                case FunctionCode::MTLO: return "mtlo";
                case FunctionCode::MULT: return "mult";
                case FunctionCode::MULTU: return "multu";
                case FunctionCode::DIV: return "div";
                case FunctionCode::DIVU: return "divu";
                case FunctionCode::ADD: return "add";
                case FunctionCode::ADDU: return "addu";
                case FunctionCode::SUB: return "sub";
                case FunctionCode::SUBU: return "subu";
                case FunctionCode::AND: return "and";
                case FunctionCode::OR: return "or";
                case FunctionCode::XOR: return "xor";
                case FunctionCode::NOR: return "nor";
                case FunctionCode::SLT: return "slt";
                case FunctionCode::SLTU: return "sltu";
                default: return "unknown_r";
            }
        } else if constexpr (std::is_same_v<T, IInstruction>) {
            switch (i.opcode) {
                case Opcode::BEQ: return "beq";
                case Opcode::BNE: return "bne";
                case Opcode::BLEZ: return "blez";
                case Opcode::BGTZ: return "bgtz";
                case Opcode::ADDI: return "addi";
                case Opcode::ADDIU: return "addiu";
                case Opcode::SLTI: return "slti";
                case Opcode::SLTIU: return "sltiu";
                case Opcode::ANDI: return "andi";
                case Opcode::ORI: return "ori";
                case Opcode::XORI: return "xori";
                case Opcode::LLO: return "llo";
                case Opcode::LHI: return "lhi";
                case Opcode::LB: return "lb";
                case Opcode::LH: return "lh";
                case Opcode::LW: return "lw";
                case Opcode::LBU: return "lbu";
                case Opcode::LHU: return "lhu";
                case Opcode::SB: return "sb";
                case Opcode::SH: return "sh";
                case Opcode::SW: return "sw";
                case Opcode::TRAP: return "trap";
                default: return "unknown_i";
            }
        } else if constexpr (std::is_same_v<T, JInstruction>) {
            switch (i.opcode) {
                case Opcode::J: return "j";
                case Opcode::JAL: return "jal";
                default: return "unknown_j";
            }
        }
        return "unknown";
    }, instr);
}

uint32_t InstructionUtils::sign_extend_16(uint16_t value) {
    if (value & 0x8000) {
        return value | 0xFFFF0000;
    }
    return value;
}

uint32_t InstructionUtils::zero_extend_16(uint16_t value) {
    return value;
}

uint32_t InstructionUtils::sign_extend_8(uint8_t value) {
    if (value & 0x80) {
        return value | 0xFFFFFF00;
    }
    return value;
}

uint32_t InstructionUtils::zero_extend_8(uint8_t value) {
    return value;
}

uint32_t InstructionUtils::encode_r_type(const RInstruction& instr) {
    return (static_cast<uint32_t>(instr.opcode) << 26) |
           (static_cast<uint32_t>(instr.rs) << 21) |
           (static_cast<uint32_t>(instr.rt) << 16) |
           (static_cast<uint32_t>(instr.rd) << 11) |
           (static_cast<uint32_t>(instr.shamt) << 6) |
           static_cast<uint32_t>(instr.funct);
}

uint32_t InstructionUtils::encode_i_type(const IInstruction& instr) {
    return (static_cast<uint32_t>(instr.opcode) << 26) |
           (static_cast<uint32_t>(instr.rs) << 21) |
           (static_cast<uint32_t>(instr.rt) << 16) |
           static_cast<uint32_t>(instr.immediate);
}

uint32_t InstructionUtils::encode_j_type(const JInstruction& instr) {
    return (static_cast<uint32_t>(instr.opcode) << 26) |
           (instr.address & 0x3FFFFFF);
}

RInstruction InstructionUtils::decode_r_type(uint32_t binary) {
    uint8_t rs = (binary >> 21) & 0x1F;
    uint8_t rt = (binary >> 16) & 0x1F;
    uint8_t rd = (binary >> 11) & 0x1F;
    uint8_t shamt = (binary >> 6) & 0x1F;
    FunctionCode funct = static_cast<FunctionCode>(binary & 0x3F);
    
    return RInstruction(rs, rt, rd, shamt, funct);
}

IInstruction InstructionUtils::decode_i_type(uint32_t binary) {
    Opcode opcode = static_cast<Opcode>((binary >> 26) & 0x3F);
    uint8_t rs = (binary >> 21) & 0x1F;
    uint8_t rt = (binary >> 16) & 0x1F;
    uint16_t immediate = binary & 0xFFFF;
    
    return IInstruction(opcode, rs, rt, immediate);
}

JInstruction InstructionUtils::decode_j_type(uint32_t binary) {
    Opcode opcode = static_cast<Opcode>((binary >> 26) & 0x3F);
    uint32_t address = binary & 0x3FFFFFF;
    
    return JInstruction(opcode, address);
}

InstructionExecutor::InstructionExecutor(std::istream& input, std::ostream& output)
    : input_stream(input), output_stream(output) {
    init_handlers();
}

void InstructionExecutor::set_io_streams(std::istream& /* input */, std::ostream& /* output */) {
}

void InstructionExecutor::execute(machine_state& state, const Instruction& instr) {
    std::string key = get_instruction_key(instr);
    
    auto it = handlers.find(key);
    if (it != handlers.end()) {
        it->second(state, instr);
    } else {
        throw std::runtime_error("Unsupported instruction: " + key);
    }
}

std::string InstructionExecutor::get_instruction_key(const Instruction& instr) {
    return InstructionUtils::get_name(instr);
}

void InstructionExecutor::init_handlers() {
    // R-type handlers
    handlers["sll"] = [this](machine_state& s, const Instruction& i) {
        execute_sll(s, std::get<RInstruction>(i));
    };
    handlers["srl"] = [this](machine_state& s, const Instruction& i) {
        execute_srl(s, std::get<RInstruction>(i));
    };
    handlers["sra"] = [this](machine_state& s, const Instruction& i) {
        execute_sra(s, std::get<RInstruction>(i));
    };
    handlers["sllv"] = [this](machine_state& s, const Instruction& i) {
        execute_sllv(s, std::get<RInstruction>(i));
    };
    handlers["srlv"] = [this](machine_state& s, const Instruction& i) {
        execute_srlv(s, std::get<RInstruction>(i));
    };
    handlers["srav"] = [this](machine_state& s, const Instruction& i) {
        execute_srav(s, std::get<RInstruction>(i));
    };
    handlers["jr"] = [this](machine_state& s, const Instruction& i) {
        execute_jr(s, std::get<RInstruction>(i));
    };
    handlers["jalr"] = [this](machine_state& s, const Instruction& i) {
        execute_jalr(s, std::get<RInstruction>(i));
    };
    handlers["mfhi"] = [this](machine_state& s, const Instruction& i) {
        execute_mfhi(s, std::get<RInstruction>(i));
    };
    handlers["mthi"] = [this](machine_state& s, const Instruction& i) {
        execute_mthi(s, std::get<RInstruction>(i));
    };
    handlers["mflo"] = [this](machine_state& s, const Instruction& i) {
        execute_mflo(s, std::get<RInstruction>(i));
    };
    handlers["mtlo"] = [this](machine_state& s, const Instruction& i) {
        execute_mtlo(s, std::get<RInstruction>(i));
    };
    handlers["mult"] = [this](machine_state& s, const Instruction& i) {
        execute_mult(s, std::get<RInstruction>(i));
    };
    handlers["multu"] = [this](machine_state& s, const Instruction& i) {
        execute_multu(s, std::get<RInstruction>(i));
    };
    handlers["div"] = [this](machine_state& s, const Instruction& i) {
        execute_div(s, std::get<RInstruction>(i));
    };
    handlers["divu"] = [this](machine_state& s, const Instruction& i) {
        execute_divu(s, std::get<RInstruction>(i));
    };
    handlers["add"] = [this](machine_state& s, const Instruction& i) {
        execute_add(s, std::get<RInstruction>(i));
    };
    handlers["addu"] = [this](machine_state& s, const Instruction& i) {
        execute_addu(s, std::get<RInstruction>(i));
    };
    handlers["sub"] = [this](machine_state& s, const Instruction& i) {
        execute_sub(s, std::get<RInstruction>(i));
    };
    handlers["subu"] = [this](machine_state& s, const Instruction& i) {
        execute_subu(s, std::get<RInstruction>(i));
    };
    handlers["and"] = [this](machine_state& s, const Instruction& i) {
        execute_and(s, std::get<RInstruction>(i));
    };
    handlers["or"] = [this](machine_state& s, const Instruction& i) {
        execute_or(s, std::get<RInstruction>(i));
    };
    handlers["xor"] = [this](machine_state& s, const Instruction& i) {
        execute_xor(s, std::get<RInstruction>(i));
    };
    handlers["nor"] = [this](machine_state& s, const Instruction& i) {
        execute_nor(s, std::get<RInstruction>(i));
    };
    handlers["slt"] = [this](machine_state& s, const Instruction& i) {
        execute_slt(s, std::get<RInstruction>(i));
    };
    handlers["sltu"] = [this](machine_state& s, const Instruction& i) {
        execute_sltu(s, std::get<RInstruction>(i));
    };

    // I-type handlers
    handlers["beq"] = [this](machine_state& s, const Instruction& i) {
        execute_beq(s, std::get<IInstruction>(i));
    };
    handlers["bne"] = [this](machine_state& s, const Instruction& i) {
        execute_bne(s, std::get<IInstruction>(i));
    };
    handlers["blez"] = [this](machine_state& s, const Instruction& i) {
        execute_blez(s, std::get<IInstruction>(i));
    };
    handlers["bgtz"] = [this](machine_state& s, const Instruction& i) {
        execute_bgtz(s, std::get<IInstruction>(i));
    };
    handlers["addi"] = [this](machine_state& s, const Instruction& i) {
        execute_addi(s, std::get<IInstruction>(i));
    };
    handlers["addiu"] = [this](machine_state& s, const Instruction& i) {
        execute_addiu(s, std::get<IInstruction>(i));
    };
    handlers["slti"] = [this](machine_state& s, const Instruction& i) {
        execute_slti(s, std::get<IInstruction>(i));
    };
    handlers["sltiu"] = [this](machine_state& s, const Instruction& i) {
        execute_sltiu(s, std::get<IInstruction>(i));
    };
    handlers["andi"] = [this](machine_state& s, const Instruction& i) {
        execute_andi(s, std::get<IInstruction>(i));
    };
    handlers["ori"] = [this](machine_state& s, const Instruction& i) {
        execute_ori(s, std::get<IInstruction>(i));
    };
    handlers["xori"] = [this](machine_state& s, const Instruction& i) {
        execute_xori(s, std::get<IInstruction>(i));
    };
    handlers["llo"] = [this](machine_state& s, const Instruction& i) {
        execute_llo(s, std::get<IInstruction>(i));
    };
    handlers["lhi"] = [this](machine_state& s, const Instruction& i) {
        execute_lhi(s, std::get<IInstruction>(i));
    };
    handlers["lb"] = [this](machine_state& s, const Instruction& i) {
        execute_lb(s, std::get<IInstruction>(i));
    };
    handlers["lh"] = [this](machine_state& s, const Instruction& i) {
        execute_lh(s, std::get<IInstruction>(i));
    };
    handlers["lw"] = [this](machine_state& s, const Instruction& i) {
        execute_lw(s, std::get<IInstruction>(i));
    };
    handlers["lbu"] = [this](machine_state& s, const Instruction& i) {
        execute_lbu(s, std::get<IInstruction>(i));
    };
    handlers["lhu"] = [this](machine_state& s, const Instruction& i) {
        execute_lhu(s, std::get<IInstruction>(i));
    };
    handlers["sb"] = [this](machine_state& s, const Instruction& i) {
        execute_sb(s, std::get<IInstruction>(i));
    };
    handlers["sh"] = [this](machine_state& s, const Instruction& i) {
        execute_sh(s, std::get<IInstruction>(i));
    };
    handlers["sw"] = [this](machine_state& s, const Instruction& i) {
        execute_sw(s, std::get<IInstruction>(i));
    };
    handlers["trap"] = [this](machine_state& s, const Instruction& i) {
        execute_trap(s, std::get<IInstruction>(i));
    };

    // J-type handlers
    handlers["j"] = [this](machine_state& s, const Instruction& i) {
        execute_j(s, std::get<JInstruction>(i));
    };
    handlers["jal"] = [this](machine_state& s, const Instruction& i) {
        execute_jal(s, std::get<JInstruction>(i));
    };
}

// R-type instruction implementations
void InstructionExecutor::execute_sll(machine_state& state, const RInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rt_val << instr.shamt;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_srl(machine_state& state, const RInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rt_val >> instr.shamt;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_sra(machine_state& state, const RInstruction& instr) {
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    int32_t result = rt_val >> instr.shamt;
    state.set_register(static_cast<Register>(instr.rd), static_cast<uint32_t>(result));
}

void InstructionExecutor::execute_sllv(machine_state& state, const RInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t shift_amount = rs_val & 0x1F; // Only use lower 5 bits
    uint32_t result = rt_val << shift_amount;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_srlv(machine_state& state, const RInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t shift_amount = rs_val & 0x1F; // Only use lower 5 bits
    uint32_t result = rt_val >> shift_amount;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_srav(machine_state& state, const RInstruction& instr) {
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t shift_amount = rs_val & 0x1F; // Only use lower 5 bits
    int32_t result = rt_val >> shift_amount;
    state.set_register(static_cast<Register>(instr.rd), static_cast<uint32_t>(result));
}

void InstructionExecutor::execute_jr(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    state.set_pc(rs_val);
}

void InstructionExecutor::execute_jalr(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    state.set_register(Register::RA, state.get_pc() + 4);
    state.set_pc(rs_val);
}

void InstructionExecutor::execute_mfhi(machine_state& state, const RInstruction& instr) {
    uint32_t hi_val = state.get_hi();
    state.set_register(static_cast<Register>(instr.rd), hi_val);
}

void InstructionExecutor::execute_mthi(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    state.set_hi(rs_val);
}

void InstructionExecutor::execute_mflo(machine_state& state, const RInstruction& instr) {
    uint32_t lo_val = state.get_lo();
    state.set_register(static_cast<Register>(instr.rd), lo_val);
}

void InstructionExecutor::execute_mtlo(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    state.set_lo(rs_val);
}

void InstructionExecutor::execute_mult(machine_state& state, const RInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    int64_t result = static_cast<int64_t>(rs_val) * static_cast<int64_t>(rt_val);
    
    state.set_lo(static_cast<uint32_t>(result & 0xFFFFFFFF));
    state.set_hi(static_cast<uint32_t>((result >> 32) & 0xFFFFFFFF));
}

void InstructionExecutor::execute_multu(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint64_t result = static_cast<uint64_t>(rs_val) * static_cast<uint64_t>(rt_val);
    
    state.set_lo(static_cast<uint32_t>(result & 0xFFFFFFFF));
    state.set_hi(static_cast<uint32_t>((result >> 32) & 0xFFFFFFFF));
}

void InstructionExecutor::execute_div(machine_state& state, const RInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    
    if (rt_val != 0) {
        state.set_lo(static_cast<uint32_t>(rs_val / rt_val));
        state.set_hi(static_cast<uint32_t>(rs_val % rt_val));
    }
}

void InstructionExecutor::execute_divu(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    
    if (rt_val != 0) {
        state.set_lo(rs_val / rt_val);
        state.set_hi(rs_val % rt_val);
    }
}

void InstructionExecutor::execute_add(machine_state& state, const RInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    int32_t result = rs_val + rt_val;
    state.set_register(static_cast<Register>(instr.rd), static_cast<uint32_t>(result));
}

void InstructionExecutor::execute_addu(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rs_val + rt_val;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_sub(machine_state& state, const RInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    int32_t result = rs_val - rt_val;
    // TODO: Check for overflow exception in real MIPS
    state.set_register(static_cast<Register>(instr.rd), static_cast<uint32_t>(result));
}

void InstructionExecutor::execute_subu(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rs_val - rt_val;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_and(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rs_val & rt_val;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_or(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rs_val | rt_val;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_xor(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = rs_val ^ rt_val;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_nor(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = ~(rs_val | rt_val);
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_slt(machine_state& state, const RInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t rt_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rt)));
    uint32_t result = (rs_val < rt_val) ? 1 : 0;
    state.set_register(static_cast<Register>(instr.rd), result);
}

void InstructionExecutor::execute_sltu(machine_state& state, const RInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = (rs_val < rt_val) ? 1 : 0;
    state.set_register(static_cast<Register>(instr.rd), result);
}

// I-type instruction implementations
void InstructionExecutor::execute_beq(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    
    if (rs_val == rt_val) {
        int32_t offset = InstructionUtils::sign_extend_16(instr.immediate) << 2;
        state.set_pc(state.get_pc() + offset);
    }
}

void InstructionExecutor::execute_bne(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    
    if (rs_val != rt_val) {
        int32_t offset = InstructionUtils::sign_extend_16(instr.immediate) << 2;
        state.set_pc(state.get_pc() + offset);
    }
}

void InstructionExecutor::execute_blez(machine_state& state, const IInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    
    if (rs_val <= 0) {
        int32_t offset = InstructionUtils::sign_extend_16(instr.immediate) << 2;
        state.set_pc(state.get_pc() + offset);
    }
}

void InstructionExecutor::execute_bgtz(machine_state& state, const IInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    
    if (rs_val > 0) {
        int32_t offset = InstructionUtils::sign_extend_16(instr.immediate) << 2;
        state.set_pc(state.get_pc() + offset);
    }
}

void InstructionExecutor::execute_addi(machine_state& state, const IInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t imm_val = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    int32_t result = rs_val + imm_val;
    // TODO: Check for overflow exception in real MIPS
    state.set_register(static_cast<Register>(instr.rt), static_cast<uint32_t>(result));
}

void InstructionExecutor::execute_addiu(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t imm_val = InstructionUtils::sign_extend_16(instr.immediate);
    uint32_t result = rs_val + imm_val;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_slti(machine_state& state, const IInstruction& instr) {
    int32_t rs_val = static_cast<int32_t>(state.get_register(static_cast<Register>(instr.rs)));
    int32_t imm_val = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t result = (rs_val < imm_val) ? 1 : 0;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_sltiu(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t imm_val = InstructionUtils::sign_extend_16(instr.immediate);
    uint32_t result = (rs_val < imm_val) ? 1 : 0;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_andi(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t imm_val = InstructionUtils::zero_extend_16(instr.immediate);
    uint32_t result = rs_val & imm_val;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_ori(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t imm_val = InstructionUtils::zero_extend_16(instr.immediate);
    uint32_t result = rs_val | imm_val;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_xori(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t imm_val = InstructionUtils::zero_extend_16(instr.immediate);
    uint32_t result = rs_val ^ imm_val;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_llo(machine_state& state, const IInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = (rt_val & 0xFFFF0000) | instr.immediate;
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_lhi(machine_state& state, const IInstruction& instr) {
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    uint32_t result = (rt_val & 0x0000FFFF) | (static_cast<uint32_t>(instr.immediate) << 16);
    state.set_register(static_cast<Register>(instr.rt), result);
}

void InstructionExecutor::execute_lb(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        uint8_t byte_val = state.read_memory8(addr);
        uint32_t result = InstructionUtils::sign_extend_8(byte_val);
        state.set_register(static_cast<Register>(instr.rt), result);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in lb instruction");
    }
}

void InstructionExecutor::execute_lh(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        uint16_t half_val = state.read_memory16(addr);
        uint32_t result = InstructionUtils::sign_extend_16(half_val);
        state.set_register(static_cast<Register>(instr.rt), result);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in lh instruction");
    }
}

void InstructionExecutor::execute_lw(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        uint32_t word_val = state.read_memory32(addr);
        state.set_register(static_cast<Register>(instr.rt), word_val);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in lw instruction");
    }
}

void InstructionExecutor::execute_lbu(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        uint8_t byte_val = state.read_memory8(addr);
        uint32_t result = InstructionUtils::zero_extend_8(byte_val);
        state.set_register(static_cast<Register>(instr.rt), result);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in lbu instruction");
    }
}

void InstructionExecutor::execute_lhu(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        uint16_t half_val = state.read_memory16(addr);
        uint32_t result = InstructionUtils::zero_extend_16(half_val);
        state.set_register(static_cast<Register>(instr.rt), result);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in lhu instruction");
    }
}

void InstructionExecutor::execute_sb(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        state.write_memory8(addr, static_cast<uint8_t>(rt_val & 0xFF));
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in sb instruction");
    }
}

void InstructionExecutor::execute_sh(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        state.write_memory16(addr, static_cast<uint16_t>(rt_val & 0xFFFF));
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in sh instruction");
    }
}

void InstructionExecutor::execute_sw(machine_state& state, const IInstruction& instr) {
    uint32_t rs_val = state.get_register(static_cast<Register>(instr.rs));
    uint32_t rt_val = state.get_register(static_cast<Register>(instr.rt));
    int32_t offset = static_cast<int32_t>(InstructionUtils::sign_extend_16(instr.immediate));
    uint32_t addr = rs_val + offset;
    
    try {
        state.write_memory32(addr, rt_val);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Memory access violation in sw instruction");
    }
}

// J-type instruction implementations
void InstructionExecutor::execute_j(machine_state& state, const JInstruction& instr) {
    // Jump address is shifted left by 2 and combined with upper 4 bits of PC+4
    uint32_t pc_plus_4 = state.get_pc() + 4;
    uint32_t jump_addr = (pc_plus_4 & 0xF0000000) | (instr.address << 2);
    state.set_pc(jump_addr);
}

void InstructionExecutor::execute_jal(machine_state& state, const JInstruction& instr) {
    // Save return address (PC + 4) to $ra
    state.set_register(Register::RA, state.get_pc() + 4);
    
    // Jump address is shifted left by 2 and combined with upper 4 bits of PC+4
    uint32_t pc_plus_4 = state.get_pc() + 4;
    uint32_t jump_addr = (pc_plus_4 & 0xF0000000) | (instr.address << 2);
    state.set_pc(jump_addr);
}

// Syscall handling
void InstructionExecutor::execute_trap(machine_state& state, const IInstruction& instr) {
    Syscall syscall_num = static_cast<Syscall>(instr.immediate);
    handle_syscall(state, syscall_num);
}

void InstructionExecutor::handle_syscall(machine_state& state, Syscall syscall_num) {
    switch (syscall_num) {
        case Syscall::PRINT_INT: {
            int32_t value = static_cast<int32_t>(state.get_register(Register::A0));
            output_stream << value << std::flush;  // Add flush
            break;
        }
        case Syscall::PRINT_CHARACTER: {
            char ch = static_cast<char>(state.get_register(Register::A0) & 0xFF);
            output_stream << ch;
            output_stream.flush();  // Add flush
            break;
        }
        case Syscall::PRINT_STRING: {
            uint32_t addr = state.get_register(Register::A0);
            try {
                while (true) {
                    uint8_t ch = state.read_memory8(addr);
                    if (ch == 0) break; // Null terminator
                    output_stream << static_cast<char>(ch);
                    addr++;
                }
                output_stream.flush();  // Add flush
            } catch (const std::out_of_range&) {
                throw std::runtime_error("Memory access violation in print_string syscall");
            }
            break;
        }
        case Syscall::READ_INT: {
            int32_t val;
            input_stream >> val;
            state.set_register(Register::V0, static_cast<uint32_t>(val));
            break;
        }
        case Syscall::READ_CHARACTER: {
            char ch;
            input_stream.get(ch);
            state.set_register(Register::V0, static_cast<uint32_t>(ch & 0xFF));
            break;
        }
        case Syscall::EXIT: {
            break;
        }
        default:
            throw std::runtime_error("Unknown syscall: " + std::to_string(static_cast<int>(syscall_num)));
    }

}
#include "../../include/executor.h"
#include "../../include/instruction.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>

Executor::Executor() {}

static std::string instr_summary(const Instruction &instr) {
    InstructionFormat fmt = InstructionUtils::get_format(instr);
    std::ostringstream os;
    if (fmt == InstructionFormat::R_TYPE) {
        const RInstruction &r = std::get<RInstruction>(instr);
        os << "R(f=" << static_cast<int>(r.funct)
           << " rs=" << static_cast<int>(r.rs)
           << " rt=" << static_cast<int>(r.rt)
           << " rd=" << static_cast<int>(r.rd)
           << " sh=" << static_cast<int>(r.shamt) << ")";
    } else if (fmt == InstructionFormat::I_TYPE) {
        const IInstruction &i = std::get<IInstruction>(instr);
        os << "I(op=" << static_cast<int>(i.opcode)
           << " rs=" << static_cast<int>(i.rs)
           << " rt=" << static_cast<int>(i.rt)
           << " imm=0x" << std::hex << (static_cast<uint32_t>(i.immediate) & 0xFFFFu) << std::dec << ")";
    } else {
        const JInstruction &j = std::get<JInstruction>(instr);
        os << "J(op=" << static_cast<int>(j.opcode)
           << " addr=0x" << std::hex << (j.address << 2) << std::dec << ")";
    }
    return os.str();
}


static std::vector<uint8_t> read_all(std::istream& in) {
    std::vector<uint8_t> buf;
    in.seekg(0, std::ios::end);
    std::streamoff size = in.tellg();
    if (size <= 0) {
        in.clear();
        in.seekg(0);
        uint8_t b;
        while (in.read(reinterpret_cast<char*>(&b), 1)) {
            buf.push_back(b);
        }
    } else {
        in.seekg(0);
        buf.resize(static_cast<size_t>(size));
        if (!in.read(reinterpret_cast<char*>(buf.data()), size)) {
            throw std::runtime_error("Failed to read binary file content.");
        }
    }
    return buf;
}

machine_state Executor::run_stream(std::istream& in, uint64_t max_steps, bool verbose, uint32_t start_address) {
    std::vector<uint8_t> bytes = read_all(in);
    if (bytes.empty()) {
        throw std::runtime_error("Binary is empty.");
    }

    uint32_t start_pc = 0;
    bool header_found = false;
    if (bytes.size() >= 8) {
        if (bytes[0] == 'M' && bytes[1] == 'I' && bytes[2] == 'P' && bytes[3] == 'S') {
            uint32_t main_addr = static_cast<uint32_t>(bytes[4]) |
                                 (static_cast<uint32_t>(bytes[5]) << 8) |
                                 (static_cast<uint32_t>(bytes[6]) << 16) |
                                 (static_cast<uint32_t>(bytes[7]) << 24);
            start_pc = main_addr;
            header_found = true;
            std::vector<uint8_t> body(bytes.begin() + 8, bytes.end());
            bytes.swap(body);
        }
    }

    if (start_address != UINT32_MAX) {
        start_pc = start_address;
    }

    machine_state state;
    state.load_memory(0u, bytes);

    if (!state.is_valid_address(start_pc, 0)) {
        throw std::runtime_error("Start PC is outside loaded binary memory: " + std::to_string(start_pc));
    }

    state.set_pc(start_pc);
    InstructionExecutor executor;

    uint64_t steps = 0;
    while (true) {
        if (steps++ >= max_steps) {
            throw std::runtime_error("Executor error: reached maximum instruction count limit.");
        }

        uint32_t pc = state.get_pc();
        if (!state.is_valid_address(pc, 4)) {
            throw std::runtime_error("Executor error: PC out of bounds at " + std::to_string(pc));
        }

        uint32_t word = state.read_memory32(pc);
        Instruction instr = InstructionUtils::decode(word);

        if (verbose) {
            std::cout << "step " << steps << " PC=0x" << std::hex << pc << std::dec
                      << " word=0x" << std::hex << word << std::dec
                      << " -> " << instr_summary(instr) << "\n";
        }

        // check TRAP
        bool is_trap = false;
        if (InstructionUtils::get_format(instr) == InstructionFormat::I_TYPE) {
            IInstruction ii = std::get<IInstruction>(instr);
            if (ii.opcode == Opcode::TRAP) is_trap = true;
        }

        uint32_t old_pc = pc;
        executor.execute(state, instr);

        if (state.get_pc() == old_pc) {
            state.increment_pc();
        }

        if (is_trap) break;
    }

    if (header_found && verbose) {
        std::cout << "Header detected: 'MIPS' header used to set main PC.\n";
    }

    return state;
}

machine_state Executor::run_file(const std::string& filename, uint64_t max_steps, bool verbose, uint32_t start_address) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open binary file: " + filename);
    return run_stream(ifs, max_steps, verbose, start_address);
}

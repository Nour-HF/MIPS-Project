#include "../../include/interpreter.h"
#include "../../include/instruction.h"
#include <fstream>
#include <stdexcept>

Interpreter::Interpreter() : parser() {
}

machine_state Interpreter::run_stream(std::istream& input, uint64_t max_steps) {
    ParseResult result = parser.parse_assembly(input);

    if (!result.has_main) {
        throw std::runtime_error("Interpreter error: 'main' label not found in assembly.");
    }

    std::vector<uint8_t> bin = parser.generate_binary(result);

    machine_state state;
    if (!bin.empty()) {
        state.load_memory(0u, bin);
    }

    state.set_pc(result.main_address);

    InstructionExecutor executor;

    // Execution loop
    uint64_t steps = 0;
    while (true) {
        if (steps++ >= max_steps) {
            throw std::runtime_error("Interpreter error: reached maximum instruction count limit.");
        }

        uint32_t pc = state.get_pc();

        if (!state.is_valid_address(pc, 4)) {
            throw std::runtime_error("Interpreter error: PC points outside valid memory at address " + std::to_string(pc));
        }

        uint32_t instr_word = state.read_memory32(pc);
        Instruction instr = InstructionUtils::decode(instr_word);

        uint32_t old_pc = pc;

        executor.execute(state, instr);

        if (state.get_pc() == old_pc) {
            state.increment_pc();
        }

        InstructionFormat fmt = InstructionUtils::get_format(instr);
        if (fmt == InstructionFormat::I_TYPE) {
            IInstruction iinstr = std::get<IInstruction>(instr);
            if (iinstr.opcode == Opcode::TRAP) {
                break;
            }
        }
    }

    return state;
}

machine_state Interpreter::run_file(const std::string& filename, uint64_t max_steps) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Cannot open assembly file: " + filename);
    }
    return run_stream(ifs, max_steps);
}

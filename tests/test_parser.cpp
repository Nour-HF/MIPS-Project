// tests/test_parser.cpp
#include "../include/parser.h"
#include "../include/instruction.h"
#include <iostream>
#include <cassert>
#include <variant>

int main() {
    try {
        Parser parser;

        // Assembly snippet to test:
        // - initial .text with several instructions (main at address 0)
        // - a label 'loop' in the middle of text
        // - a branch referencing the 'loop' label
        // - a .data section with a .word array and an .asciiz
        // - a second .text section with another label (end) that should be placed after earlier text instructions
        const std::string asm_text = R"(
            .text
            main:
                add $t0, $t1, $t2
                loop: lw $a0, 4($sp)
                beq $t0, $t1, loop
                j end
            .data
                arr: .word 1, 2, 3
                msg: .asciiz "hi"
            .text
            end: sll $t3, $t4, 2
        )";

        ParseResult res = parser.parse_assembly(asm_text);

        // must recognise main
        assert(res.has_main == true);
        // main should be the first instruction => address 0
        assert(res.main_address == 0);
        // labels should contain main, loop, end, arr, msg
        auto it_main = res.labels.find("main");
        auto it_loop = res.labels.find("loop");
        auto it_end  = res.labels.find("end");
        auto it_arr  = res.labels.find("arr");
        auto it_msg  = res.labels.find("msg");
        assert(it_main  != res.labels.end());
        assert(it_loop  != res.labels.end());
        assert(it_end   != res.labels.end());
        assert(it_arr   != res.labels.end());
        assert(it_msg   != res.labels.end());

        // check text label addresses:
        // first instruction (add) at 0, lw at 4 => loop should be 4
        assert(res.labels["main"] == 0u);
        assert(res.labels["loop"] == 4u);

        // 'end' should be after the first text block (there were 4 instructions before .data:
        // add (0), lw (4), beq (8), j (12) => next text slot = 16, so end should be 16
        assert(res.labels["end"] == 16u);

        std::cout << "Label addresses:\n";
        for (auto &p : res.labels) {
            std::cout << p.first << " => " << p.second << "\n";
        }

        // Now inspect parsed lines to find the first R-type instruction (add) and verify registers
        bool found_add = false;
        for (size_t i = 0; i < res.lines.size(); ++i) {
            if (std::holds_alternative<Instruction>(res.lines[i])) {
                Instruction instr = std::get<Instruction>(res.lines[i]);
                InstructionFormat fmt = InstructionUtils::get_format(instr);
                if (fmt == InstructionFormat::R_TYPE) {
                    RInstruction r = std::get<RInstruction>(instr);
                    // For "add $t0, $t1, $t2" parser sets r.rs = $t1, r.rt = $t2, r.rd = $t0
                    // Convert enum Register to underlying uint8_t values to compare
                    uint8_t expected_rs = static_cast<uint8_t>(Register::T1);
                    uint8_t expected_rt = static_cast<uint8_t>(Register::T2);
                    uint8_t expected_rd = static_cast<uint8_t>(Register::T0);
                    if (r.rs == expected_rs && r.rt == expected_rt && r.rd == expected_rd) {
                        found_add = true;
                        std::cout << "Found add instruction parsed correctly (rd, rs, rt): "
                                  << unsigned(r.rd) << ", " << unsigned(r.rs) << ", " << unsigned(r.rt) << "\n";
                        break;
                    }
                }
            }
        }
        assert(found_add && "add instruction not parsed into expected RInstruction fields");

        // Find .word directive for 'arr' and ensure values are [1,2,3]
        bool found_arr = false, found_msg = false;
        for (auto &pl : res.lines) {
            if (std::holds_alternative<AssemblyDirective>(pl)) {
                const AssemblyDirective &ad = std::get<AssemblyDirective>(pl);
                if (ad.type == DirectiveType::WORD && ad.values.size() >= 3) {
                    // match the .word array; values should be 1,2,3
                    if (ad.values[0] == 1u && ad.values[1] == 2u && ad.values[2] == 3u) {
                        found_arr = true;
                    }
                } else if ((ad.type == DirectiveType::ASCIIZ || ad.type == DirectiveType::ASCII) && ad.text == "hi") {
                    found_msg = true;
                }
            }
        }
        assert(found_arr && "arr .word directive not parsed as expected");
        assert(found_msg && "msg .asciiz directive not parsed as expected");

        std::cout << "Parser tests passed.\n";
        return 0;
    } catch (const std::exception &e) {
        std::cout << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}

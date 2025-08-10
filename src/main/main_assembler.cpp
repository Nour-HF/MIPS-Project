#include "../../include/assembler.h"
#include <iostream>
#include <fstream>

static void print_usage(const char* prog) {
    std::cerr << "Usage:\n";
    std::cerr << "  " << prog << "                 # read assembly from stdin, write binary to stdout\n";
    std::cerr << "  " << prog << " input.asm      # read input.asm, write binary to stdout\n";
    std::cerr << "  " << prog << " input.asm out.bin  # read input.asm, write binary to out.bin\n";
}

int main(int argc, char** argv) {
    try {
        Assembler assembler;
        std::vector<uint8_t> bytes;

        if (argc == 1) {
            // read from stdin
            bytes = assembler.assemble_stream(std::cin);
            // write to stdout
            assembler.write_binary_to_stream(bytes, std::cout);
            return 0;
        }
        else if (argc == 2) {
            // read from input file, write to stdout
            std::string in_file = argv[1];
            bytes = assembler.assemble_file(in_file);
            assembler.write_binary_to_stream(bytes, std::cout);
            return 0;
        }
        else if (argc == 3) {
            std::string in_file = argv[1];
            std::string out_file = argv[2];
            bytes = assembler.assemble_file(in_file);
            std::ofstream ofs(out_file, std::ios::binary);
            if (!ofs) {
                std::cerr << "Cannot open output file: " << out_file << std::endl;
                return 2;
            }
            assembler.write_binary_to_stream(bytes, ofs);
            ofs.close();
            return 0;
        }
        else {
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Assembler error: " << e.what() << std::endl;
        return 1;
    }
}

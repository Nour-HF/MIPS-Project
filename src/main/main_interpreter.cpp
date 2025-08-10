#include "../../include/interpreter.h"
#include <iostream>
#include <iomanip>

static void usage(const char* prog) {
    std::cerr << "Usage:\n";
    std::cerr << "  " << prog << " input.asm\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];

    try {
        Interpreter interp;
        machine_state final_state = interp.run_file(filename);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Interpreter error: " << e.what() << std::endl;
        return 2;
    }
}

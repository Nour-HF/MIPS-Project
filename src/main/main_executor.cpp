#include "../../include/executor.h"
#include <iostream>
#include <cstring>

static void usage(const char* prog) {
    std::cerr << "Usage:\n";
    std::cerr << "  " << prog << " input.bin            # execute binary, start PC = header/main or 0\n";
    std::cerr << "  " << prog << " input.bin -v         # verbose trace\n";
    std::cerr << "  " << prog << " input.bin -m <N>     # set max instruction steps (default 100000)\n";
    std::cerr << "  " << prog << " input.bin -s <addr>  # explicitly set start PC (overrides header)\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];
    bool verbose = false;
    uint64_t max_steps = 100000ULL;
    uint32_t start_addr = UINT32_MAX;

    for (int i = 2; i < argc; ++i) {
        if (std::strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (std::strcmp(argv[i], "-m") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "-m requires an argument\n";
                return 1;
            }
            max_steps = std::stoull(argv[++i]);
        } else if (std::strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "-s requires an address argument\n";
                return 1;
            }
            start_addr = static_cast<uint32_t>(std::stoul(argv[++i], nullptr, 0));
        } else {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            usage(argv[0]);
            return 1;
        }
    }

    try {
        Executor exe;
        machine_state final_state = exe.run_file(filename, max_steps, verbose, start_addr);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Executor error: " << e.what() << std::endl;
        return 2;
    }
}

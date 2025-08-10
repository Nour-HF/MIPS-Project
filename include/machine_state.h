#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>

enum class Register : uint8_t {
    ZERO = 0, AT = 1, V0 = 2, V1 = 3,
    A0 = 4, A1 = 5, A2 = 6, A3 = 7,
    T0 = 8, T1 = 9, T2 = 10, T3 = 11, T4 = 12, T5 = 13, T6 = 14, T7 = 15,
    S0 = 16, S1 = 17, S2 = 18, S3 = 19, S4 = 20, S5 = 21, S6 = 22, S7 = 23,
    T8 = 24, T9 = 25, K0 = 26, K1 = 27,
    GP = 28, SP = 29, S8 = 30, RA = 31,
};

class machine_state {
private:

    std::array<uint32_t, 32> registers{}; // 32 general registers
    std::vector<uint8_t> memory; // Memory as a vector of bytes (dynamic size)
    uint32_t pc; // Program counter
    uint32_t hi; // High word register
    uint32_t lo; // Low word register


public:

    // Initial size of memory
    machine_state(size_t memory_size = 1024 * 1024);

    // Register access
    uint32_t get_register(Register reg) const;
    void set_register(Register reg, uint32_t value);

    // Special registers access
    uint32_t get_pc() const { return pc; }
    void set_pc(uint32_t value) { pc = value; }
    void increment_pc() { pc += 4; }
    uint32_t get_hi() const { return hi; }
    uint32_t get_lo() const { return lo; }
    void set_hi(uint32_t value) { hi = value; }
    void set_lo(uint32_t value) { lo = value; }

    // Memory access
    bool is_valid_address(uint32_t addr, size_t access_size) const;
    uint8_t read_memory8(uint32_t addr) const;
    uint16_t read_memory16(uint32_t addr) const;
    uint32_t read_memory32(uint32_t addr) const;
    void write_memory8(uint32_t addr, uint8_t value);
    void write_memory16(uint32_t addr, uint16_t value);
    void write_memory32(uint32_t addr, uint32_t value);

    // Memory management
    size_t get_memory_size() const { return memory.size(); }
    void resize_memory(size_t new_size);
    void load_memory(uint32_t addr, const std::vector<uint8_t>& data);
};

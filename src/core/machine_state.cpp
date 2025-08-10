#include "../../include/machine_state.h"
#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>

machine_state::machine_state(size_t memory_size) 
    : memory(memory_size, 0),
      pc(0),
      hi(0),
      lo(0)
    {}

// Register access
uint32_t machine_state::get_register(Register reg) const {
    uint8_t index = static_cast<uint8_t>(reg);
    // $zero always returns 0
    if (index == 0) {
        return 0;
    }
    return registers[index];
}

void machine_state::set_register(Register reg, uint32_t value) {
    uint8_t index = static_cast<uint8_t>(reg);
    // $zero register ignored
    if (index == 0) {
        return;
    }
    
    registers[index] = value;
}

// Memory access

// Bounds checking helper
bool machine_state::is_valid_address(uint32_t addr, size_t access_size) const {
    return addr + access_size <= memory.size();
}

uint8_t machine_state::read_memory8(uint32_t addr) const {
    if (!is_valid_address(addr, 1)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    return memory[addr];
}

void machine_state::write_memory8(uint32_t addr, uint8_t value) {
    if (!is_valid_address(addr, 1)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    memory[addr] = value;
}

uint16_t machine_state::read_memory16(uint32_t addr) const {
    if (!is_valid_address(addr, 2)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    
    // Little-endian: least significant byte first
    return memory[addr] | (memory[addr + 1] << 8);
}

void machine_state::write_memory16(uint32_t addr, uint16_t value) {
    if (!is_valid_address(addr, 2)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    
    // Little-endian: store least significant byte first
    memory[addr] = value & 0xFF;
    memory[addr + 1] = (value >> 8) & 0xFF;
}

uint32_t machine_state::read_memory32(uint32_t addr) const {
    if (!is_valid_address(addr, 4)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    
    // Little-endian: combine 4 bytes
    return memory[addr] | 
           (memory[addr + 1] << 8) |
           (memory[addr + 2] << 16) |
           (memory[addr + 3] << 24);
}

void machine_state::write_memory32(uint32_t addr, uint32_t value) {
    if (!is_valid_address(addr, 4)) {
        throw std::out_of_range("Memory address out of bounds");
    }
    
    // Little-endian: store bytes in order
    memory[addr] = value & 0xFF;
    memory[addr + 1] = (value >> 8) & 0xFF;
    memory[addr + 2] = (value >> 16) & 0xFF;
    memory[addr + 3] = (value >> 24) & 0xFF;
}

// Memory management
void machine_state::resize_memory(size_t new_size) {
    memory.resize(new_size, 0);
}

void machine_state::load_memory(uint32_t addr, const std::vector<uint8_t>& data) {
    if (!is_valid_address(addr, data.size())) {
        throw std::out_of_range("Memory load would exceed bounds");
    }
    
    std::copy(data.begin(), data.end(), memory.begin() + addr);
}
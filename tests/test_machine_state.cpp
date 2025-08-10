#include "../include/machine_state.h"
#include <iostream>
#include <cassert>

void test_registers() {
    machine_state ms;
    
    // Test normal register access
    std::cout << ms.get_register(Register::T0) <<  "\n";
    ms.set_register(Register::T0, 42);
    std::cout << ms.get_register(Register::T0) <<  "\n";
    assert(ms.get_register(Register::T0) == 42);
    
    // Test $zero register behavior
    ms.set_register(Register::ZERO, 999);
    assert(ms.get_register(Register::ZERO) == 0);
    
    // Test all registers start at 0
    assert(ms.get_register(Register::T1) == 0);
    assert(ms.get_register(Register::S0) == 0);
    
    std::cout << "Register tests passed!\n";
}

void test_memory() {
    machine_state ms;
    
    // Test 8-bit access
    std::cout << std::hex << (int)ms.read_memory8(100) <<  "\n";
    ms.write_memory8(100, 0xAB);
    std::cout << (int)ms.read_memory8(100) <<  "\n";
    assert(ms.read_memory8(100) == 0xAB);
    
    // Test 16-bit access  
    std::cout << (int)ms.read_memory16(200) <<  "\n";
    ms.write_memory16(200, 0x1234);
    std::cout << (int)ms.read_memory16(200) <<  "\n";
    assert(ms.read_memory16(200) == 0x1234);
    
    // Test 32-bit access
    std::cout << (int)ms.read_memory32(300) <<  "\n";
    ms.write_memory32(300, 0x12345678);
    std::cout << (int)ms.read_memory32(300) <<  "\n";
    std::cout << (int)ms.read_memory16(300) <<  "\n";
    assert(ms.read_memory32(300) == 0x12345678);
    
    std::cout << "Memory tests passed!\n";
}

void test_endianness() {
    machine_state ms;
    
    // Write a 32-bit value and check individual bytes
    ms.write_memory32(0, 0x12345678);
    
    // In little-endian:
    assert(ms.read_memory8(0) == 0x78);  // LSB first
    assert(ms.read_memory8(1) == 0x56);
    assert(ms.read_memory8(2) == 0x34);
    assert(ms.read_memory8(3) == 0x12);  // MSB last
    
    std::cout << "Endianness test passed!\n";
}

void test_bounds_and_resize__checking() {
    machine_state ms(1000);  // 1000 byte memory
    
    try {
        ms.read_memory8(1000);  // Should throw
        assert(false);  // Should not reach here
    } catch (const std::out_of_range&) {
        // Expected
    }
    
    try {
        ms.read_memory32(999);  // Would read bytes 999-1002, but only 0-999 exist
        assert(false);  // Should not reach here  
    } catch (const std::out_of_range&) {
        // Expected
    }
    
    std::cout << std::dec << ms.get_memory_size() <<  "\n";
    assert(ms.get_memory_size() == 1000);
    ms.resize_memory(2000);
    std::cout << ms.get_memory_size() <<  "\n";
    assert(ms.get_memory_size() == 2000);

    std::cout << "Bounds and resize checking tests passed!\n";
}

void test_pc() {
    machine_state ms;
    
    assert(ms.get_pc() == 0);  // Should start at 0
    
    ms.set_pc(0x1000);
    assert(ms.get_pc() == 0x1000);
    
    ms.increment_pc();
    assert(ms.get_pc() == 0x1004);  // Should increment by 4
    
    std::cout << "PC tests passed!\n";
}

int main() {
    try {
        test_registers();
        test_memory();
        test_endianness();
        test_bounds_and_resize__checking();
        test_pc();
        
        std::cout << "All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
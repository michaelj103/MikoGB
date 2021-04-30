//
//  ArithmeticInstructions8.cpp
//  MikoGB
//
//  Created on 5/9/20.
//

#include "ArithmeticInstructions8.hpp"

using namespace MikoGB;

#pragma mark - Add

// a and b must be uint8_t cast to int
static uint8_t _Add8BitOperands(int a, int b, bool addCarry, CPUCore &core) {
    const int carryIn = (addCarry && core.getFlag(FlagBit::Carry)) ? 1 : 0;
    const int sum = a + b + carryIn;
    //carry in can be ignored when computing carry bits because we don't care about the low bit
    //each bit of carriedBits reflects whether there was carry out of the previous bit
    const int carriedBits = a ^ b ^ sum;
    const bool halfCarry = (carriedBits & 0x10) == 0x10; //bit 4 set? Means carry out of bit 3
    const bool carry = (carriedBits & 0x100) == 0x100; //bit 8 set? Means carry out of bit 7
    
    const uint8_t result = (0xFF & sum);
    core.setFlag(FlagBit::Zero, result == 0);
    core.setFlag(FlagBit::H, halfCarry);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Carry, carry);
    
    return result;
}

int CPUInstructions::addAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, false, core);
    
    return 1;
}

int CPUInstructions::addAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, false, core);
    
    return 2;
}

int CPUInstructions::addAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, false, core);
    
    return 2;
}

int CPUInstructions::addAccWithRegisterAndCarry(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, true, core);
    
    return 1;
}

int CPUInstructions::addAccWithPtrHLAndCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, true, core);
    
    return 2;
}

int CPUInstructions::addAccWithImmediate8AndCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Add8BitOperands(a, b, true, core);
    
    return 2;
}

#pragma mark - SUB
// TODO: Could probably combine SUB and ADD since they're so similar. Same with the carry vs non-carry versions

// a and b must be uint8_t cast to int
static uint8_t _Sub8BitOperands(int a, int b, bool subCarry, CPUCore &core) {
    const int carryVal = (subCarry && core.getFlag(FlagBit::Carry)) ? 1 : 0;
    const int difference = a - b - carryVal;
    //carry in can be ignored when computing borrow bits because we don't care about the low bit
    //each bit of borrowed reflects whether there was a borrow from that bit
    const int borrowedBits = a ^ b ^ difference;
    const bool halfCarry = (borrowedBits & 0x10) == 0x10; //bit 4 set? Means borrow from bit 4
    const bool carry = (borrowedBits & 0x100) == 0x100; //bit 8 set? Means borrow from bit 8
    
    const uint8_t result = (0xFF & difference);
    core.setFlag(FlagBit::Zero, result == 0);
    core.setFlag(FlagBit::H, halfCarry);
    core.setFlag(FlagBit::N, true);
    core.setFlag(FlagBit::Carry, carry);
    
    return result;
}

int CPUInstructions::subAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, false, core);
    
    return 1;
}

int CPUInstructions::subAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, false, core);
    
    return 2;
}

int CPUInstructions::subAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, false, core);
    
    return 2;
}

int CPUInstructions::subAccWithRegisterAndCarry(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, true, core);
    
    return 1;
}

int CPUInstructions::subAccWithImmediate8AndCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, true, core);
    
    return 2;
}

int CPUInstructions::subAccWithPtrHLAndCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Sub8BitOperands(a, b, true, core);
    
    return 2;
}

#pragma mark - AND

static uint8_t _And8BitOperands(uint8_t a, uint8_t b, CPUCore &core) {
    const uint8_t result = a & b;
    core.setFlag(FlagBit::Zero, result == 0);
    core.setFlag(FlagBit::H, true);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Carry, false);
    return result;
}

int CPUInstructions::andAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _And8BitOperands(a, b, core);
    
    return 1;
}

int CPUInstructions::andAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _And8BitOperands(a, b, core);
    
    return 2;
}

int CPUInstructions::andAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _And8BitOperands(a, b, core);
    
    return 2;
}

#pragma mark - XOR
//TODO: Set the flags correctly for XOR!

int CPUInstructions::xorAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    // Register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t result = core.registers[REGISTER_A] ^ core.registers[reg];
    core.registers[REGISTER_A] = result;
    return 1;
}

int CPUInstructions::xorAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    // Low 3 bits must be 110, indicating "memory address"
    uint16_t ptr = core.getHLptr();
    const uint8_t result = core.registers[REGISTER_A] ^ core.mainMemory[ptr];
    core.registers[REGISTER_A] = result;
    return 2;
}

int CPUInstructions::xorAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t immediate = opcode[1];
    const uint8_t result = core.registers[REGISTER_A] ^ immediate;
    core.registers[REGISTER_A] = result;
    return 2;
}

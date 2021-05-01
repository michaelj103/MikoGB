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

#pragma mark - OR

static uint8_t _Or8BitOperands(uint8_t a, uint8_t b, CPUCore &core) {
    const uint8_t result = a | b;
    core.setFlag(FlagBit::Zero, result == 0);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Carry, false);
    return result;
}

int CPUInstructions::orAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Or8BitOperands(a, b, core);
    
    return 1;
}

int CPUInstructions::orAccWithImmediate8(const uint8_t *opcode, MikoGB::CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Or8BitOperands(a, b, core);
    
    return 2;
}

int CPUInstructions::orAccWithPtrHL(const uint8_t *opcode, MikoGB::CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Or8BitOperands(a, b, core);
    
    return 2;
}

#pragma mark - XOR

static uint8_t _Xor8BitOperands(uint8_t a, uint8_t b, CPUCore &core) {
    const uint8_t result = a ^ b;
    core.setFlag(FlagBit::Zero, result == 0);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Carry, false);
    return result;
}

int CPUInstructions::xorAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    core.registers[REGISTER_A] = _Xor8BitOperands(a, b, core);
    
    return 1;
}

int CPUInstructions::xorAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    core.registers[REGISTER_A] = _Xor8BitOperands(a, b, core);
    
    return 2;
}

int CPUInstructions::xorAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    core.registers[REGISTER_A] = _Xor8BitOperands(a, b, core);
    
    return 2;
}

#pragma mark - CP

static void _Cp8BitOperands(uint8_t a, uint8_t b, CPUCore &core) {
    core.setFlag(FlagBit::Zero, a == b);
    core.setFlag(FlagBit::H, a > b);
    core.setFlag(FlagBit::N, true);
    core.setFlag(FlagBit::Carry, a < b);
}

int CPUInstructions::cpAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.registers[reg];
    _Cp8BitOperands(a, b, core);
    
    return 1;
}

int CPUInstructions::cpAccWithImmediate8(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = opcode[1];
    _Cp8BitOperands(a, b, core);
    
    return 2;
}

int CPUInstructions::cpAccWithPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint8_t a = core.registers[REGISTER_A];
    const uint8_t b = core.mainMemory[core.getHLptr()];
    _Cp8BitOperands(a, b, core);
    
    return 2;
}

#pragma mark - INC

static uint8_t _Inc8BitValue(uint8_t a, CPUCore &core) {
    const uint8_t sum = a + 1;
    // We don't actually care about the low bit, so xor with the sum to get all bits with carry outs
    // from previous bits. Only one we care about is 3, so check 4 (0x10 mask)
    const uint8_t carriedBits = a ^ sum;
    core.setFlag(FlagBit::Zero, sum == 0);
    core.setFlag(FlagBit::H, (carriedBits & 0x10) == 0x10);
    core.setFlag(FlagBit::N, false);
    //Carry flag is not touched
    return sum;
}

int CPUInstructions::incRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is mid 3 bits
    const uint8_t reg = (opcode[0] & 0x38) >> 3;
    const uint8_t val = core.registers[reg];
    core.registers[reg] = _Inc8BitValue(val, core);
    
    return 1;
}

int CPUInstructions::incPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint16_t ptrAddress = core.getHLptr();
    const uint8_t val = core.mainMemory[ptrAddress];
    core.mainMemory[ptrAddress] = _Inc8BitValue(val, core);
    
    return 3;
}

#pragma mark - DEC

static uint8_t _Dec8BitValue(uint8_t a, CPUCore &core) {
    const uint8_t diff = a - 1;
    // We don't actually care about the low bit, so xor with the diff to get all bits borrowed from
    // Only one we care about is 4, so check mask 0x10
    const uint8_t carriedBits = a ^ diff;
    core.setFlag(FlagBit::Zero, diff == 0);
    core.setFlag(FlagBit::H, (carriedBits & 0x10) == 0x10);
    core.setFlag(FlagBit::N, true);
    //Carry flag is not touched
    return diff;
}

int CPUInstructions::decRegister(const uint8_t *opcode, CPUCore &core) {
    //register code is mid 3 bits
    const uint8_t reg = (opcode[0] & 0x38) >> 3;
    const uint8_t val = core.registers[reg];
    core.registers[reg] = _Dec8BitValue(val, core);
    
    return 1;
}

int CPUInstructions::decPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint16_t ptrAddress = core.getHLptr();
    const uint8_t val = core.mainMemory[ptrAddress];
    core.mainMemory[ptrAddress] = _Dec8BitValue(val, core);
    
    return 3;
}

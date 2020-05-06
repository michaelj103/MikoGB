//
//  LoadInstructions.cpp
//  MikoGB
//
//  Created on 5/5/20.
//

#include "LoadInstructions.hpp"

using namespace MikoGB;

int CPUInstructions::loadRegisterFromRegister(const uint8_t *opcode, CPUCore &core) {
    uint8_t sourceRegister = opcode[0] & 0x7; //r' is lower 3 bits
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the next 3 bits
    
    core.registers[destRegister] = core.registers[sourceRegister];
    return 1;
}

int CPUInstructions::loadRegisterFromMemory(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t memVal = core.mainMemory[core.getHLptr()];
    core.registers[destRegister] = memVal;
    return 2;
}

int CPUInstructions::loadRegisterImmediate8(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 00
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t immediateVal = opcode[1];
    core.registers[destRegister] = immediateVal;
    return 2;
}

int CPUInstructions::loadMemoryFromRegister(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Next two bits must be 110
    uint8_t sourceRegister = opcode[0] & 0x7; //r is lower 3 bits
    core.mainMemory[core.getHLptr()] = core.registers[sourceRegister];
    return 2;
}

int CPUInstructions::loadMemoryImmediate8(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00110110
    uint8_t immediateVal = opcode[1];
    core.mainMemory[core.getHLptr()] = immediateVal;
    return 3;
}

int CPUInstructions::loadAccumulatorFromBC(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00001010
    uint8_t memVal = core.mainMemory[core.getBCptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadAccumulatorFromDE(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00011010
    uint8_t memVal = core.mainMemory[core.getDEptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadAccumulatorFromC(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11110010
    uint8_t memVal = core.mainMemory[core.getCptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadCFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11100010
    core.mainMemory[core.getCptr()] = core.registers[REGISTER_A];
    return 2;
}

int CPUInstructions::loadAccumulatorFromPtrImmediate8(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11110000
    uint16_t ptr = 0xFF00 + opcode[1]; //Load some byte from 0xFF00 - 0xFFFF into A
    core.registers[REGISTER_A] = core.mainMemory[ptr];
    return 3;
}

int CPUInstructions::loadPtrImmediate8FromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11100000
    uint16_t ptr = 0xFF00 + opcode[1]; //Load into some address from 0xFF00 - 0xFFFF
    core.mainMemory[ptr] = core.registers[REGISTER_A];
    return 3;
}


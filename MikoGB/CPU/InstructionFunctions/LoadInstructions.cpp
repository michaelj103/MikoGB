//
//  LoadInstructions.cpp
//  MikoGB
//
//  Created on 5/5/20.
//

#include "LoadInstructions.hpp"

using namespace MikoGB;

int CPUInstructions::loadRegisterFromRegister(uint8_t *opcode, CPUCore &core) {
    uint8_t sourceRegister = opcode[0] & 0x7; //r' is lower 3 bits
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the next 3 bits
    
    core.registers[destRegister] = core.registers[sourceRegister];
    return 1;
}

int CPUInstructions::loadRegisterFromMemory(uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t memVal = core.mainMemory[core.getHLptr()];
    core.registers[destRegister] = memVal;
    return 2;
}

int CPUInstructions::loadRegisterImmediate8(uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 00
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t immediateVal = opcode[1];
    core.registers[destRegister] = immediateVal;
    return 2;
}

int CPUInstructions::loadMemoryFromRegister(uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Next two bits must be 110
    uint8_t sourceRegister = opcode[0] & 0x7; //r is lower 3 bits
    core.mainMemory[core.getHLptr()] = core.registers[sourceRegister];
    return 2;
}

int CPUInstructions::loadMemoryImmediate8(uint8_t *opcode, CPUCore &core) {
    //bits must be 00110110
    uint8_t immediateVal = opcode[1];
    core.mainMemory[core.getHLptr()] = immediateVal;
    return 3;
}

int CPUInstructions::loadAccumulatorFromBC(uint8_t *opcode, CPUCore &core) {
    //bits must be 00001010
    uint8_t memVal = core.mainMemory[core.getBCptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadAccumulatorFromDE(uint8_t *opcode, CPUCore &core) {
    //bits must be 00011010
    uint8_t memVal = core.mainMemory[core.getDEptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadAccumulatorFromC(uint8_t *opcode, CPUCore &core) {
    //bits must be 11110010
    uint8_t memVal = core.mainMemory[core.getCptr()];
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadCFromAccumulator(uint8_t *opcode, CPUCore &core) {
    //bits must be 11100010
    core.mainMemory[core.getCptr()] = core.registers[REGISTER_A];
    return 2;
}


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
    uint8_t memVal = core.mainMemory[core.getHL()];
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
    core.mainMemory[core.getHL()] = core.registers[sourceRegister];
    return 2;
}

int CPUInstructions::loadMemoryImmediate8(uint8_t *opcode, CPUCore &core) {
    //bits must be 00110110
    uint8_t immediateVal = opcode[1];
    core.mainMemory[core.getHL()] = immediateVal;
    return 3;
}

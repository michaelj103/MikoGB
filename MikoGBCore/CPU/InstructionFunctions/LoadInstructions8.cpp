//
//  LoadInstructions8.cpp
//  MikoGB
//
//  Created on 5/5/20.
//

#include "LoadInstructions8.hpp"

using namespace MikoGB;

int CPUInstructions::loadRegisterFromRegister(const uint8_t *opcode, CPUCore &core) {
    uint8_t sourceRegister = opcode[0] & 0x7; //r' is lower 3 bits
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the next 3 bits
    
    core.registers[destRegister] = core.registers[sourceRegister];
    return 1;
}

int CPUInstructions::loadRegisterFromPtrHL(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t memVal = core.getMemory(core.getHLptr());
    core.registers[destRegister] = memVal;
    return 2;
}

int CPUInstructions::loadRegisterFromImmediate8(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 00
    //Low 3 bits must be 110
    uint8_t destRegister = (opcode[0] & 0x38) >> 3; //r is the second 3 bits
    uint8_t immediateVal = opcode[1];
    core.registers[destRegister] = immediateVal;
    return 2;
}

int CPUInstructions::loadPtrHLFromRegister(const uint8_t *opcode, CPUCore &core) {
    //Hi 2 bits must be 01
    //Next two bits must be 110
    const uint8_t sourceRegister = opcode[0] & 0x7; //r is lower 3 bits
    core.setMemory(core.getHLptr(), core.registers[sourceRegister]);
    return 2;
}

int CPUInstructions::loadPtrHLFromImmediate8(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00110110
    const uint8_t immediateVal = opcode[1];
    core.setMemory(core.getHLptr(), immediateVal);
    return 3;
}

int CPUInstructions::loadAccumulatorFromPtrBC(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00001010
    const uint8_t memVal = core.getMemory(core.getBCptr());
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadPtrBCFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00000010
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(core.getBCptr(), val);
    return 2;
}

int CPUInstructions::loadAccumulatorFromPtrDE(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00011010
    const uint8_t memVal = core.getMemory(core.getDEptr());
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadPtrDEFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00010010
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(core.getDEptr(), val);
    return 2;
}

int CPUInstructions::loadAccumulatorFromPtrC(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11110010
    const uint8_t memVal = core.getMemory(core.getCptr());
    core.registers[REGISTER_A] = memVal;
    return 2;
}

int CPUInstructions::loadPtrCFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11100010
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(core.getCptr(), val);
    return 2;
}

int CPUInstructions::loadPtrImmediate8FromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11100000
    uint16_t ptr = 0xFF00 + opcode[1]; //Load into some address from 0xFF00 - 0xFFFF
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(ptr, val);
    return 3;
}

int CPUInstructions::loadPtrImmediate16FromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11101010
    //TODO: Confirm that this is the correct byte order. Assuming it is based on other instructions
    const uint8_t lo = opcode[1];
    const uint8_t hi = opcode[2];
    const uint16_t ptr = word16(lo, hi);
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(ptr, val);
    return 4;
}

int CPUInstructions::loadAccumulatorFromPtrImmediate8(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11110000
    const uint16_t ptr = 0xFF00 + opcode[1]; //Load some byte from 0xFF00 - 0xFFFF into A
    core.registers[REGISTER_A] = core.getMemory(ptr);
    return 3;
}

int CPUInstructions::loadAccumulatorFromPtrImmediate16(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11111010
    //TODO: Confirm that this is the correct byte order. Assuming it is based on other instructions
    const uint8_t lo = opcode[1];
    const uint8_t hi = opcode[2];
    const uint16_t ptr = word16(lo, hi);
    core.registers[REGISTER_A] = core.getMemory(ptr);
    return 4;
}

int CPUInstructions::loadAccumulatorFromPtrHLIncrement(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00101010
    const uint8_t memVal = core.getMemory(core.getHLptr());
    core.registers[REGISTER_A] = memVal;
    core.incrementHLptr();
    return 2;
}

int CPUInstructions::loadAccumulatorFromPtrHLDecrement(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00101010
    const uint8_t memVal = core.getMemory(core.getHLptr());
    core.registers[REGISTER_A] = memVal;
    core.decrementHLptr();
    return 2;
}

int CPUInstructions::loadPtrHLIncrementFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00100010
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(core.getHLptr(), val);
    core.incrementHLptr();
    return 2;
}

int CPUInstructions::loadPtrHLDecrementFromAccumulator(const uint8_t *opcode, CPUCore &core) {
    //bits must be 00110010
    const uint8_t val = core.registers[REGISTER_A];
    core.setMemory(core.getHLptr(), val);
    core.decrementHLptr();
    return 2;
}

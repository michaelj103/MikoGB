//
//  RotateShiftInstructions.cpp
//  MikoGB
//
//  Created on 5/3/21.
//

#include "RotateShiftInstructions.hpp"

using namespace MikoGB;

#pragma mark - Rotate

int CPUInstructions::rotateLeftAccumulatorCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint8_t aVal = core.registers[REGISTER_A];
    core.setFlag(FlagBit::Carry, (aVal & 0x80) == 0x80);
    core.registers[REGISTER_A] = (aVal << 1) | ((aVal & 0x80) >> 7);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, false);
    
    return 1;
}

int CPUInstructions::rotateLeftAccumulatorThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t aVal = core.registers[REGISTER_A];
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 1 : 0;
    core.setFlag(FlagBit::Carry, (aVal & 0x80) == 0x80);
    core.registers[REGISTER_A] = (aVal << 1) | carryVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, false);
    
    return 1;
}

int CPUInstructions::rotateRightAccumulatorCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint8_t aVal = core.registers[REGISTER_A];
    core.setFlag(FlagBit::Carry, (aVal & 0x01) == 0x01);
    core.registers[REGISTER_A] = (aVal >> 1) | ((aVal & 0x01) << 7);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, false);
    
    return 1;
}

int CPUInstructions::rotateRightAccumulatorThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t aVal = core.registers[REGISTER_A];
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 0x80 : 0;
    core.setFlag(FlagBit::Carry, (aVal & 0x01) == 0x01);
    core.registers[REGISTER_A] = (aVal >> 1) | carryVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, false);
    
    return 1;
}

#pragma mark - Rotate extended opcodes

int CPUInstructions::rotateLeftRegisterCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = (rVal << 1) | ((rVal & 0x80) >> 7);
    core.setFlag(FlagBit::Carry, (rVal & 0x80) == 0x80);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::rotateLeftPtrHLCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = (mVal << 1) | ((mVal & 0x80) >> 7);
    core.setFlag(FlagBit::Carry, (mVal & 0x80) == 0x80);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

int CPUInstructions::rotateLeftRegisterThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 1 : 0;
    const uint8_t finalVal = (rVal << 1) | carryVal;
    core.setFlag(FlagBit::Carry, (rVal & 0x80) == 0x80);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::rotateLeftPtrHLThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 1 : 0;
    const uint8_t finalVal = (mVal << 1) | carryVal;
    core.setFlag(FlagBit::Carry, (mVal & 0x80) == 0x80);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

int CPUInstructions::rotateRightRegisterCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = (rVal >> 1) | ((rVal & 0x01) << 7);
    core.setFlag(FlagBit::Carry, (rVal & 0x01) == 0x01);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::rotateRightPtrHLCarryOut(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = (mVal >> 1) | ((mVal & 0x01) << 7);
    core.setFlag(FlagBit::Carry, (mVal & 0x01) == 0x01);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

int CPUInstructions::rotateRightRegisterThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 0x80 : 0;
    const uint8_t finalVal = (rVal >> 1) | carryVal;
    core.setFlag(FlagBit::Carry, (rVal & 0x01) == 0x01);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::rotateRightPtrHLThroughCarry(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t carryVal = core.getFlag(FlagBit::Carry) ? 0x80 : 0;
    const uint8_t finalVal = (mVal >> 1) | carryVal;
    core.setFlag(FlagBit::Carry, (mVal & 0x01) == 0x01);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

#pragma mark - Shift extended opcodes

int CPUInstructions::shiftLeftRegisterFill0(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = rVal << 1;
    core.setFlag(FlagBit::Carry, (rVal & 0x80) == 0x80);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::shiftLeftPtrHLFill0(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = mVal << 1;
    core.setFlag(FlagBit::Carry, (mVal & 0x80) == 0x80);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

int CPUInstructions::shiftRightRegisterFill0(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = rVal >> 1;
    core.setFlag(FlagBit::Carry, (rVal & 0x01) == 0x01);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::shiftRightPtrHLFill0(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = mVal >> 1;
    core.setFlag(FlagBit::Carry, (mVal & 0x01) == 0x01);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

int CPUInstructions::shiftRightRegisterFillHigh(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = (rVal >> 1) | (rVal & 0x80); // shift right but preserve high bit
    core.setFlag(FlagBit::Carry, (rVal & 0x01) == 0x01);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::shiftRightPtrHLFillHigh(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = (mVal >> 1) | (mVal & 0x80); // shift right but preserve high bit
    core.setFlag(FlagBit::Carry, (mVal & 0x01) == 0x01);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

#pragma mark - SWAP m

int CPUInstructions::swapRegister(const uint8_t *opcode, CPUCore &core) {
    const uint8_t reg = (opcode[1] & 0x07); // Register code is low 3 bits
    const uint8_t rVal = core.registers[reg];
    const uint8_t finalVal = ((rVal & 0xF) << 4) | ((rVal & 0xF0) >> 4);
    core.registers[reg] = finalVal;
    core.setFlag(FlagBit::Carry, false);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 2;
}

int CPUInstructions::swapPtrHL(const uint8_t *opcode, CPUCore &core) {
    const uint16_t hlPtr = core.getHLptr();
    const uint8_t mVal = core.getMemory(hlPtr);
    const uint8_t finalVal = ((mVal & 0xF) << 4) | ((mVal & 0xF0) >> 4);
    core.setMemory(hlPtr, finalVal);
    core.setFlag(FlagBit::Carry, false);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Zero, finalVal == 0);
    
    return 4;
}

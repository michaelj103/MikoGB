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

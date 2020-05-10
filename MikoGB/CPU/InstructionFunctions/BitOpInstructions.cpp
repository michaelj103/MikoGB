//
//  BitOpInstructions.cpp
//  MikoGB
//
//  Created on 5/10/20.
//

#include "BitOpInstructions.hpp"

using namespace MikoGB;

int CPUInstructions::bitReadFromRegister(const uint8_t *opcode, CPUCore &core) {
    // Pull the bit index and register code from the instruction (index 1, extended)
    // Low 3 bits are the register code, next 3 are the bit idx
    uint8_t bitIdx = (opcode[1] & 0x38) >> 3;
    uint8_t reg = (opcode[1] & 0x7);
    
    // Check if the indicated bit is set and set flags appropriately
    const uint8_t mask = 1 << bitIdx;
    const bool bitSet = (core.registers[reg] & mask) == mask;
    core.setFlag(FlagBit::Zero, bitSet);
    // H and N flag values always set via this instruction for some reason
    core.setFlag(FlagBit::H, true);
    core.setFlag(FlagBit::N, false);
    
    return 2;
}

int CPUInstructions::bitReadFromPtrHL(const uint8_t *opcode, CPUCore &core) {
    // Pull the bit index from the instruction (index 1, extended)
    // Low 3 bits are register code 110, next 3 are the bit idx
    uint8_t bitIdx = (opcode[1] & 0x38) >> 3;
    
    // Check if the indicated bit is set and set flags appropriately
    const uint8_t mask = 1 << bitIdx;
    const bool bitSet = (core.mainMemory[core.getHLptr()] & mask) == mask;
    core.setFlag(FlagBit::Zero, bitSet);
    // H and N flag values always set via this instruction for some reason
    core.setFlag(FlagBit::H, true);
    core.setFlag(FlagBit::N, false);
    return 3;
}

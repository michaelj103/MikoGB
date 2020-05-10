//
//  JumpInstructions.cpp
//  MikoGB
//
//  Created on 5/10/20.
//

#include "JumpInstructions.hpp"
#include <stdexcept>

using namespace MikoGB;

int CPUInstructions::jumpUnconditionalAbsolute16(const uint8_t *opcode , CPUCore &core) {
    // Set program counter to 16-bit address in immediate operands
    uint8_t lo = opcode[1];
    uint8_t hi = opcode[2];
    uint16_t dest = word16(lo, hi);
    core.programCounter = dest;
    return 4;
}

int CPUInstructions::jumpConditionalAbsolute16(const uint8_t *opcode , CPUCore &core) {
    uint8_t condition = (opcode[0] & 0x18) >> 3;
    bool jump = false;
    switch (condition) {
        case 0:
            // NZ (Z flag == 0)
            jump = !core.getFlag(FlagBit::Zero);
            break;
        case 1:
            // Z flag == 1
            jump = core.getFlag(FlagBit::Zero);
            break;
        case 2:
            // NC (C flag == 0)
            jump = !core.getFlag(FlagBit::Carry);
            break;
        case 3:
            // C flag == 1
            jump = core.getFlag(FlagBit::Carry);
            break;
        default:
            //TODO: Debug assert?
            throw std::runtime_error("Unreachable condition error: JP cc, nn");
            break;
    }
    
    if (jump) {
        // Set program counter to 16-bit address in immediate operands
        uint8_t lo = opcode[1];
        uint8_t hi = opcode[2];
        uint16_t dest = word16(lo, hi);
        core.programCounter = dest;
        return 4;
    } else {
        return 3;
    }
}

int CPUInstructions::jumpUnconditionalRelative8(const uint8_t *opcode , CPUCore &core) {
    //TODO: fill in
    return 0;
}

int CPUInstructions::jumpConditionalRelative8(const uint8_t *opcode , CPUCore &core) {
    //TODO: fill in
    return 0;
}

int CPUInstructions::jumpUnconditionalHL(const uint8_t *opcode , CPUCore &core) {
    core.programCounter = core.getHLptr();
    return 1;
}


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
        // Note: variable timing depending on condition
        return 3;
    }
}

// NOTE: for relative 8, the Game Boy Programming manual claims the range is -127 to 129.
// This doesn't seem right unless 0 is handled specially, but watch out for issues here
int CPUInstructions::jumpUnconditionalRelative8(const uint8_t *opcode , CPUCore &core) {
    // Offset program counter by 8-bit operand, treated as signed
    int8_t relative = (int8_t)opcode[1];
    core.programCounter += relative;
    return 3;
}

// NOTE: for relative 8, the Game Boy Programming manual claims the range is -127 to 129.
// This doesn't seem right unless 0 is handled specially, but watch out for issues here
int CPUInstructions::jumpConditionalRelative8(const uint8_t *opcode , CPUCore &core) {
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
            throw std::runtime_error("Unreachable condition error: JR cc, nn");
            break;
    }
    
    if (jump) {
        // Offset program counter by 8-bit operand, treated as signed
        int8_t relative = (int8_t)opcode[1];
        core.programCounter += relative;
        return 3;
    } else {
        // Note: variable timing depending on condition
        return 2;
    }
}

int CPUInstructions::jumpUnconditionalHL(const uint8_t *opcode , CPUCore &core) {
    core.programCounter = core.getHLptr();
    return 1;
}


//
//  CallAndReturnInstructions.cpp
//  MikoGB
//
//  Created on 5/2/21.
//

#include "CallAndReturnInstructions.hpp"

using namespace MikoGB;

#pragma mark - CALL

static inline void _Call16(uint8_t lo, uint8_t hi, CPUCore &core) {
    core.stackPush(core.programCounter);
    core.programCounter = word16(lo, hi);
}

int CPUInstructions::callImmediate16(const uint8_t *opcode, CPUCore &core) {
    _Call16(opcode[1], opcode[2], core);
    
    return 6;
}

int CPUInstructions::callConditionalImmediate16(const uint8_t *opcode, MikoGB::CPUCore &core) {
    const uint8_t condition = (opcode[0] & 0x18) >> 3;
    bool call = false;
    switch (condition) {
        case 0:
            // NZ (Z flag == 0)
            call = !core.getFlag(FlagBit::Zero);
            break;
        case 1:
            // Z flag == 1
            call = core.getFlag(FlagBit::Zero);
            break;
        case 2:
            // NC (C flag == 0)
            call = !core.getFlag(FlagBit::Carry);
            break;
        case 3:
            // C flag == 1
            call = core.getFlag(FlagBit::Carry);
            break;
    }
    
    if (call) {
        _Call16(opcode[1], opcode[2], core);
        return 6;
    } else {
        // Note: variable timing if condition fails
        return 3;
    }
}

#pragma mark RET

int CPUInstructions::returnSubroutine(const uint8_t *opcode, CPUCore &core) {
    core.programCounter = core.stackPop();
    return 4;
}

int CPUInstructions::returnInterrupt(const uint8_t *opcode, CPUCore &core) {
    core.programCounter = core.stackPop();
    core.interruptsEnabled = true;
    return 4;
}

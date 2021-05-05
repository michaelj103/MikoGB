//
//  SpecialInstructions.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "SpecialInstructions.hpp"
#include <stdexcept>

using namespace MikoGB;

int CPUInstructions::decimalAdjustAccumulator(const uint8_t *opcode, CPUCore &core) {
    // Very weird instruction
    throw std::runtime_error("DAA Instruction not implemented");
    return 1;
}

int CPUInstructions::complementAccumulator(const uint8_t *opcode, CPUCore &core) {
    const uint8_t val = core.registers[REGISTER_A];
    core.registers[REGISTER_A] = ~val;
    core.setFlag(FlagBit::H, true);
    core.setFlag(FlagBit::N, true);
    
    return 1;
}

int CPUInstructions::complementCarryFlag(const uint8_t *opcode, CPUCore &core) {
    const bool carry = core.getFlag(FlagBit::Carry);
    core.setFlag(FlagBit::Carry, !carry);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    
    return 1;
}

int CPUInstructions::setCarryFlag(const uint8_t *opcode, CPUCore &core) {
    core.setFlag(FlagBit::Carry, true);
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::N, false);
    
    return 1;
}

int CPUInstructions::disableInterrupts(const uint8_t *opcode, CPUCore &core) {
    core.interruptsEnabled = false;
    return 1;
}

int CPUInstructions::enableInterrupts(const uint8_t *opcode, CPUCore &core) {
    core.interruptsEnabled = true;
    return 1;
}

int CPUInstructions::haltInstruction(const uint8_t *opcode, CPUCore &core) {
    core.halt();
    return 1;
}

int CPUInstructions::stopInstruction(const uint8_t *opcode, CPUCore &core) {
    // Assert that the next instruction is NOP?
    core.stop();
    return 1;
}

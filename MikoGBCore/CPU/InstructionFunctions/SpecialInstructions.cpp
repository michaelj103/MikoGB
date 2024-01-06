//
//  SpecialInstructions.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "SpecialInstructions.hpp"
#include <stdexcept>
#include <cassert>

using namespace MikoGB;

static uint8_t _DAA_Add(CPUCore &core) {
    const bool carry = core.getFlag(FlagBit::Carry);
    const bool halfCarry = core.getFlag(FlagBit::H);
    const uint16_t accumulator = core.registers[REGISTER_A];
    const uint8_t highNibble = (accumulator & 0xF0) >> 4;
    const uint8_t lowNibble = (accumulator & 0x0F);
    
    if (!carry && !halfCarry) {
        if (highNibble <= 0x9 && lowNibble <= 0x9) {
            core.setFlag(FlagBit::Carry, false);
            return accumulator & 0xFF;
        } else if (highNibble <= 0x8 && lowNibble >= 0xA) {
            core.setFlag(FlagBit::Carry, false);
            return (accumulator + 0x06) & 0xFF;
        } else if (highNibble >= 0xA && lowNibble <= 0x9) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x60) & 0xFF;
        } else if  (highNibble >= 0x9 && lowNibble >= 0xA) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x66) & 0xFF;
        }
        
    } else if (!carry && halfCarry) {
        if (highNibble <= 0x9 && lowNibble <= 0x3) {
            core.setFlag(FlagBit::Carry, false);
            return (accumulator + 0x06) & 0xFF;
        } else if (highNibble >= 0xA && lowNibble <= 0x3) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x66) & 0xFF;
        }
        // Other cases are undefined because it means the addition was not between 2 valid BCD numbers
        // E.G ADC 0x_9 + 0x_9 with C=1 is the max and it is 0x_3
        
    } else if (carry && !halfCarry) {
        if (highNibble <= 0x2 && lowNibble <= 0x9) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x60) & 0xFF;
        } else if (highNibble <= 0x2 && lowNibble >= 0xA) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x66) & 0xFF;
        }
        // cannot get higher than 0x2 in the high nibble via valid addtion of BCD numbers because
        // 0x9 + 0x9 gives 0x2 and C=1. H is known 0 so 0x3 isn't possible
        
    } else if (carry && halfCarry) {
        if (highNibble <= 0x3 && lowNibble <= 0x3) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x66) & 0xFF;
        }
        // same logic applies as the previous 2 cases. higher nibbles imply invalid BCD addition
    }
    
    // Undefined behavior. Implies the addition operation that preceded the DAA was not between valid BCD numbers
    // No-op with debug assert
#if DEBUG
    assert(false);
#endif
    return accumulator & 0xFF;
}

static uint8_t _DAA_Sub(CPUCore &core) {
    const bool carry = core.getFlag(FlagBit::Carry);
    const bool halfCarry = core.getFlag(FlagBit::H);
    const uint16_t accumulator = core.registers[REGISTER_A];
    const uint8_t highNibble = (accumulator & 0xF0) >> 4;
    const uint8_t lowNibble = (accumulator & 0x0F);
    
    if (!carry && !halfCarry) {
        if (highNibble <= 0x9 && lowNibble <= 0x9) {
            core.setFlag(FlagBit::Carry, false);
            return accumulator & 0xFF;
        }
        
    } else if (!carry && halfCarry) {
        if (highNibble <= 0x8 && lowNibble >= 0x6) {
            core.setFlag(FlagBit::Carry, false);
            return (accumulator + 0xFA) & 0xFF; //equivalent to -0x06
        }
        
    } else if (carry && !halfCarry) {
        if (highNibble >= 0x7 && lowNibble <= 0x9) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0xA0) & 0xFF;
        }
        
    } else if (carry && halfCarry) {
        if (highNibble >= 0x6 && lowNibble >= 0x6) {
            core.setFlag(FlagBit::Carry, true);
            return (accumulator + 0x9A) & 0xFF;
        }
        
    }
    
    // Undefined behavior. Implies the addition operation that preceded the DAA was not between valid BCD numbers
    // No-op with debug assert
#if DEBUG
    assert(false);
#endif
    return accumulator & 0xFF;
}

int CPUInstructions::decimalAdjustAccumulator(const uint8_t *opcode, CPUCore &core) {
    // DAA is a very weird instruction by modern standards
    // The goal is basically to make the result of an addition or subtraction into a BCD
    // which is a decimal number represented in binary. e.g. 0x72 represents the number 72
    // In theory it's also only valid after an addition or subtraction of 2 BCD numbers, so
    // there are cases with undefined behavior. e.g. for addition the result 0x_4 through 0x_F with H=1
    // is technically impossible because the highest second-digit in a BCD is 0x_9 + 0x_9 with C = 1
    // so ADC would give 0x_3 with H=1
    
    uint8_t aVal;
    if (core.getFlag(FlagBit::N)) {
        // Last instruction was a subtraction
        aVal = _DAA_Sub(core);
    } else {
        // Last instruction was an addition
        aVal = _DAA_Add(core);
    }
    
    core.setFlag(FlagBit::H, false);
    core.setFlag(FlagBit::Zero, aVal == 0);
    core.registers[REGISTER_A] = aVal;
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
    core.interruptState = CPUCore::InterruptState::Disabled;
    return 1;
}

int CPUInstructions::enableInterrupts(const uint8_t *opcode, CPUCore &core) {
    core.interruptState = CPUCore::InterruptState::Scheduled;
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

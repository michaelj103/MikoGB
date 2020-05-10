//
//  ArithmeticInstructions8.cpp
//  MikoGB
//
//  Created on 5/9/20.
//

#include "ArithmeticInstructions8.hpp"

using namespace MikoGB;

int CPUInstructions::xorAccWithRegister(const uint8_t *opcode, CPUCore &core) {
    //Register code is low 3 bits
    const uint8_t reg = opcode[0] & 0x7;
    const uint8_t result = core.registers[REGISTER_A] ^ core.registers[reg];
    core.registers[REGISTER_A] = result;
    return 1;
}


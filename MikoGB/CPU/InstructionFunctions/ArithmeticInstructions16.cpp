//
//  ArithmeticInstructions16.cpp
//  MikoGB
//
//  Created on 5/1/21.
//

#include "ArithmeticInstructions16.hpp"

using namespace MikoGB;

#pragma mark - ADD

static uint16_t _Add16BitOperands(int a, int b, CPUCore &core) {
    const int sum = a + b;
    //each bit of carriedBits reflects whether there was carry out from the previous bit
    const int carriedBits = a ^ b ^ sum;
    const bool halfCarry = (carriedBits & 0x1000) == 0x1000; //bit 12 set? Means carry out of bit 11
    const bool carry = (carriedBits & 0x10000) == 0x10000; //bit 16 set? Means carry out of bit 15
    
    const uint16_t result = (0xFFFF & sum);
    core.setFlag(FlagBit::H, halfCarry);
    core.setFlag(FlagBit::N, false);
    core.setFlag(FlagBit::Carry, carry);
    return result;
}

int CPUInstructions::addHLWithRegisterPair(const uint8_t *opcode, CPUCore &core) {
    const uint16_t a = core.getHLptr();
    uint16_t b = 0;
    
    // register pair specified by 2 bits ss in opcode
    uint8_t ss = (opcode[0] & 0x30) >> 4;
    //TODO: Debug assert that ss <= 3?
    switch (ss) {
        case 0:
            // add BC
            b = core.getBCptr();
            break;
        case 1:
            // add DE
            b = core.getDEptr();
            break;
        case 2:
            // add HL to itself
            b = a;
            break;
        case 3:
            // add SP
            b = core.stackPointer;
            break;
    }
    
    const uint16_t result = _Add16BitOperands(a, b, core);
    uint8_t hi = 0, lo = 0;
    splitWord16(result, lo, hi);
    core.registers[REGISTER_H] = hi;
    core.registers[REGISTER_L] = lo;
    return 2;
}

// TODO: what this does isn't specified very clearly in the GB manual, so verify it
int CPUInstructions::addSPWithImmediate8Signed(const uint8_t *opcode, CPUCore &core) {
    const int8_t e = (int8_t)(opcode[1]); //treat as signed
    const uint16_t sp = core.stackPointer;
    core.stackPointer = _Add16BitOperands(sp, e, core);
    core.setFlag(FlagBit::Zero, false);
    return 4;
}

#pragma mark INC & DEC

int CPUInstructions::incRegisterPair(const uint8_t *opcode, CPUCore &core) {
    // register pair specified by 2 bits ss in opcode
    uint8_t ss = (opcode[0] & 0x30) >> 4;
    //TODO: Debug assert that ss <= 3?
    switch (ss) {
        case 0: {
            // inc BC
            const uint16_t val = core.getBCptr() + 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_B] = hi;
            core.registers[REGISTER_C] = lo;
        }
            break;
        case 1: {
            // inc DE
            const uint16_t val = core.getDEptr() + 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_D] = hi;
            core.registers[REGISTER_E] = lo;
        }
            break;
        case 2: {
            // inc HL
            const uint16_t val = core.getHLptr() + 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_H] = hi;
            core.registers[REGISTER_L] = lo;
        }
            break;
        case 3:
            // inc SP
            core.stackPointer += 1;
            break;
    }
    
    return 2;
}

int CPUInstructions::decRegisterPair(const uint8_t *opcode, CPUCore &core) {
    // register pair specified by 2 bits ss in opcode
    uint8_t ss = (opcode[0] & 0x30) >> 4;
    //TODO: Debug assert that ss <= 3?
    switch (ss) {
        case 0: {
            // inc BC
            const uint16_t val = core.getBCptr() - 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_B] = hi;
            core.registers[REGISTER_C] = lo;
        }
            break;
        case 1: {
            // inc DE
            const uint16_t val = core.getDEptr() - 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_D] = hi;
            core.registers[REGISTER_E] = lo;
        }
            break;
        case 2: {
            // inc HL
            const uint16_t val = core.getHLptr() - 1;
            uint8_t lo = 0, hi = 0;
            splitWord16(val, lo, hi);
            core.registers[REGISTER_H] = hi;
            core.registers[REGISTER_L] = lo;
        }
            break;
        case 3:
            // inc SP
            core.stackPointer -= 1;
            break;
    }
    
    return 2;
}

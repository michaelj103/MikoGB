//
//  LoadInstructions16.cpp
//  MikoGB
//
//  Created on 5/6/20.
//

#include "LoadInstructions16.hpp"
#include "BitTwiddlingUtil.h"

using namespace MikoGB;

int CPUInstructions::loadRegisterPairFromImmediate16(const uint8_t *opcode, CPUCore &core) {
    //High 2 bits must be 00
    //Next 3 are dd0, where dd is one of 4 codes indicating destination register pair
    //Bottom 3 must be 001
    
    uint8_t lo = opcode[1];
    uint8_t hi = opcode[2];
    uint8_t dd = (opcode[0] & 0x30) >> 4;
    //TODO: Debug assert that dd <= 3?
    switch (dd) {
        case 0:
            //destination is BC
            core.registers[REGISTER_B] = hi;
            core.registers[REGISTER_C] = lo;
            break;
        case 1:
            //destination is DE
            core.registers[REGISTER_D] = hi;
            core.registers[REGISTER_E] = lo;
            break;
        case 2:
            //destination is HL
            core.registers[REGISTER_H] = hi;
            core.registers[REGISTER_L] = lo;
            break;
        case 3:
            //destination is SP
            core.stackPointer = word16(lo, hi);
            break;
    }
    
    return 3;
}

int CPUInstructions::loadStackPtrFromHL(const uint8_t *opcode, CPUCore &core) {
    //bits must be 11111001
    core.stackPointer = core.getHLptr();
    return 2;
}

int CPUInstructions::pushQQ(const uint8_t *opcode, CPUCore &core) {
    uint8_t qq = (opcode[0] & 0x30) >> 4;
    //TODO: Debug assert that qq <= 3?
    switch (qq) {
        case 0:
            // PUSH BC
            core.stackPush(core.registers[REGISTER_B], core.registers[REGISTER_C]);
            break;
        case 1:
            // PUSH DE
            core.stackPush(core.registers[REGISTER_D], core.registers[REGISTER_E]);
            break;
        case 2:
            // PUSH HL
            core.stackPush(core.registers[REGISTER_H], core.registers[REGISTER_L]);
            break;
        case 3:
            // PUSH AF
            core.stackPush(core.registers[REGISTER_A], core.registers[REGISTER_F]);
            break;
    }
    
    return 4;
}

int CPUInstructions::popQQ(const uint8_t *opcode, CPUCore &core) {
    uint8_t qq = (opcode[0] & 0x30) >> 4;
    uint8_t hi = 0, lo = 0;
    //TODO: Debug assert that qq <= 3?
    switch (qq) {
        case 0:
            // POP BC
            core.stackPop(hi, lo);
            core.registers[REGISTER_B] = hi;
            core.registers[REGISTER_C] = lo;
            break;
        case 1:
            // POP DE
            core.stackPop(hi, lo);
            core.registers[REGISTER_D] = hi;
            core.registers[REGISTER_E] = lo;
            break;
        case 2:
            // POP HL
            core.stackPop(hi, lo);
            core.registers[REGISTER_H] = hi;
            core.registers[REGISTER_L] = lo;
            break;
        case 3:
            // POP AF
            core.stackPop(hi, lo);
            core.registers[REGISTER_A] = hi;
            core.registers[REGISTER_F] = lo & 0xF0;
            // The F register has a special format on intel 8080 derived processors
            // Specifically, when on the stack it has the format [ S, Z, 0, AC, 0, P, 1, C ]
            // S = Sign, Z = zero, AC = aux carry, P = parity, C = carry
            // In theory, a program could modify the stack memory and break this format
            // Game Boy apparently changes this behavior: There's no parity or zero bit, instead there's an N
            // It also doesn't follow this format, instead, it's [ Z, N, H, CY, X, X, X, X ]
            // X behavior isn't specified, so treat low 4 bits as always zero via the "& 0xF0"
            break;
    }
    
    return 3;
}

int CPUInstructions::ldhl(const uint8_t *opcode, CPUCore &core) {
    int8_t e = (int8_t)(opcode[1]); //treat as signed
    int result = core.stackPointer + e; //May be subtraction
    uint8_t lo = 0;
    uint8_t hi = 0;
    splitWord16(result, lo, hi);
    core.registers[REGISTER_H] = hi;
    core.registers[REGISTER_L] = lo;
    
    // Per Game Boy Manual, half is carry out of bit 11 and full is 15
    // Not sure if subtraction is supposed to be different, but this is based
    // on 2's comp addition
    int carriedBits = ((int)core.stackPointer ^ (int)e ^ result);
    bool carried11 = (carriedBits & 0x1000) == 0x1000;
    bool carried15 = (carriedBits & 0x10000) == 0x10000;
    
    core.setFlag(FlagBit::Carry, carried15);
    core.setFlag(FlagBit::H, carried11);
    core.setFlag(FlagBit::Zero, false);
    core.setFlag(FlagBit::N, false);
    return 3;
}

int CPUInstructions::loadPtrImmediate16FromSP(const uint8_t *opcode, CPUCore &core) {
    //Construct the destination address from the immediate operands
    uint8_t lo = opcode[1];
    uint8_t hi = opcode[2];
    uint16_t addr = word16(lo, hi);
    
    //Split the stack pointer into 2 bytes and store each at addr. Little-endian
    uint8_t spLo = 0, spHi = 0;
    splitWord16(core.stackPointer, spLo, spHi);
    core.mainMemory[addr] = spLo;
    core.mainMemory[addr+1] = spHi;
    return 5;
}


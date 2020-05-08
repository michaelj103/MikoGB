//
//  LoadInstructions16.cpp
//  MikoGB
//
//  Created on 5/6/20.
//

#include "LoadInstructions16.hpp"
#include <stdexcept>

using namespace MikoGB;

int CPUInstructions::loadRegisterPairFromImmediate16(const uint8_t *opcode, CPUCore &core) {
    //High 2 bits must be 00
    //Next 3 are dd0, where dd is one of 4 codes indicating destination register pair
    //Bottom 3 must be 001
    
    uint8_t lo = opcode[1];
    uint8_t hi = opcode[2];
    uint8_t dd = (opcode[0] & 0x30) >> 4;
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
        default:
            //TODO: Debug assert?
            throw std::runtime_error("Unreachable condition error: LD dd, nn");
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
            
        default:
            //TODO: Debug assert?
            throw std::runtime_error("Unreachable condition error: PUSH qq");
            break;
    }
    
    return 4;
}

int CPUInstructions::popQQ(const uint8_t *opcode, CPUCore &core) {
    uint8_t qq = (opcode[0] & 0x30) >> 4;
    uint8_t hi = 0, lo = 0;
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
            core.registers[REGISTER_F] = lo;
            // TODO: the F register is a little special, maybe validate it now?
            // Specifically, when on the stack it has the format [ S, Z, 0, AC, 0, P, 1, C ]
            // S = Sign, Z = zero, AC = aux carry, P = parity, C = carry
            // In theory, a program could modify the stack memory and break this format
            break;
            
        default:
            //TODO: Debug assert?
            throw std::runtime_error("Unreachable condition error: POP qq");
            break;
    }
    
    return 3;
}


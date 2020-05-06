//
//  CPUInstruction.cpp
//  MikoGB
//
//  Created on 5/3/20.
//

#include "CPUInstruction.hpp"
#include <exception>
#include <string>
#include "BitTwiddlingUtil.h"

#include "LoadInstructions.hpp"

using namespace std;
using namespace MikoGB;
using namespace CPUInstructions;

int CPUInstruction::UnrecognizedInstruction(uint8_t *opcode, CPUCore &core) {
    throw runtime_error("Unrecognized Instruction: " + to_string((int)(*opcode)));
}

const CPUInstruction &CPUInstruction::LookupInstruction(uint8_t *memoryPtr) {
    size_t idx = memoryPtr[0];
    if (idx == 0xCB) {
        //Z80 Extended instruction set
        //Index in the table is 0x1SS where SS is the extended opcode
        idx = memoryPtr[1] | 0x100;
    }
    
    return InstructionTable[idx];
}

#pragma mark - Instruction Definitions

static int LoadStackPointer(uint8_t *opcode, CPUCore &core) {
    uint8_t low = opcode[1];
    uint8_t hi = opcode[2];
    core.stackPointer = word16(low, hi);
    return 3;
}

static int NoOp(uint8_t *opcode, CPUCore &core) {
    return 1;
}

CPUInstruction *CPUInstruction::InstructionTable = nullptr;
void CPUInstruction::InitializeInstructionTable() {
    if (CPUInstruction::InstructionTable != nullptr) {
        return; //already initialized
    }
    
    InstructionTable = new CPUInstruction[512]();
    
    InstructionTable[0x00] = { 1, NoOp };
    InstructionTable[0x31] = { 3, LoadStackPointer };
    
    // LD r, n
    InstructionTable[0x06] = { 2, loadRegisterImmediate8 }; // LD B, n
    InstructionTable[0x0E] = { 2, loadRegisterImmediate8 }; // LD C, n
    InstructionTable[0x16] = { 2, loadRegisterImmediate8 }; // LD D, n
    InstructionTable[0x1E] = { 2, loadRegisterImmediate8 }; // LD E, n
    InstructionTable[0x26] = { 2, loadRegisterImmediate8 }; // LD H, n
    InstructionTable[0x2E] = { 2, loadRegisterImmediate8 }; // LD L, n
    InstructionTable[0x36] = { 2, loadMemoryImmediate8 }; // LD (HL), n
    InstructionTable[0x3E] = { 2, loadRegisterImmediate8 }; // LD A, n
    
    // LD B, r
    InstructionTable[0x40] = { 1, loadRegisterFromRegister }; // LD B, B -> redundant?
    InstructionTable[0x41] = { 1, loadRegisterFromRegister }; // LD B, C
    InstructionTable[0x42] = { 1, loadRegisterFromRegister }; // LD B, D
    InstructionTable[0x43] = { 1, loadRegisterFromRegister }; // LD B, E
    InstructionTable[0x44] = { 1, loadRegisterFromRegister }; // LD B, H
    InstructionTable[0x45] = { 1, loadRegisterFromRegister }; // LD B, L
    InstructionTable[0x46] = { 1, loadRegisterFromMemory }; // LD B, (HL)
    InstructionTable[0x47] = { 1, loadRegisterFromRegister }; // LD B, A
    
    // LD C, r
    InstructionTable[0x48] = { 1, loadRegisterFromRegister }; // LD C, B
    InstructionTable[0x49] = { 1, loadRegisterFromRegister }; // LD C, C -> redundant?
    InstructionTable[0x4A] = { 1, loadRegisterFromRegister }; // LD C, D
    InstructionTable[0x4B] = { 1, loadRegisterFromRegister }; // LD C, E
    InstructionTable[0x4C] = { 1, loadRegisterFromRegister }; // LD C, H
    InstructionTable[0x4D] = { 1, loadRegisterFromRegister }; // LD C, L
    InstructionTable[0x4E] = { 1, loadRegisterFromMemory }; // LD C, (HL)
    InstructionTable[0x4F] = { 1, loadRegisterFromRegister }; // LD C, A
    
    // LD D, r
    InstructionTable[0x50] = { 1, loadRegisterFromRegister }; // LD D, B
    InstructionTable[0x51] = { 1, loadRegisterFromRegister }; // LD D, C
    InstructionTable[0x52] = { 1, loadRegisterFromRegister }; // LD D, D -> redundant?
    InstructionTable[0x53] = { 1, loadRegisterFromRegister }; // LD D, E
    InstructionTable[0x54] = { 1, loadRegisterFromRegister }; // LD D, H
    InstructionTable[0x55] = { 1, loadRegisterFromRegister }; // LD D, L
    InstructionTable[0x56] = { 1, loadRegisterFromMemory }; // LD D, (HL)
    InstructionTable[0x57] = { 1, loadRegisterFromRegister }; // LD D, A
    
    // LD E, r
    InstructionTable[0x58] = { 1, loadRegisterFromRegister }; // LD E, B
    InstructionTable[0x59] = { 1, loadRegisterFromRegister }; // LD E, C
    InstructionTable[0x5A] = { 1, loadRegisterFromRegister }; // LD E, D
    InstructionTable[0x5B] = { 1, loadRegisterFromRegister }; // LD E, E -> redundant?
    InstructionTable[0x5C] = { 1, loadRegisterFromRegister }; // LD E, H
    InstructionTable[0x5D] = { 1, loadRegisterFromRegister }; // LD E, L
    InstructionTable[0x5E] = { 1, loadRegisterFromMemory }; // LD E, (HL)
    InstructionTable[0x5F] = { 1, loadRegisterFromRegister }; // LD E, A
    
    // LD H, r
    InstructionTable[0x60] = { 1, loadRegisterFromRegister }; // LD H, B
    InstructionTable[0x61] = { 1, loadRegisterFromRegister }; // LD H, C
    InstructionTable[0x62] = { 1, loadRegisterFromRegister }; // LD H, D
    InstructionTable[0x63] = { 1, loadRegisterFromRegister }; // LD H, E
    InstructionTable[0x64] = { 1, loadRegisterFromRegister }; // LD H, H -> redundant?
    InstructionTable[0x65] = { 1, loadRegisterFromRegister }; // LD H, L
    InstructionTable[0x66] = { 1, loadRegisterFromMemory }; // LD H, (HL)
    InstructionTable[0x67] = { 1, loadRegisterFromRegister }; // LD H, A
    
    // LD L, r
    InstructionTable[0x68] = { 1, loadRegisterFromRegister }; // LD L, B
    InstructionTable[0x69] = { 1, loadRegisterFromRegister }; // LD L, C
    InstructionTable[0x6A] = { 1, loadRegisterFromRegister }; // LD L, D
    InstructionTable[0x6B] = { 1, loadRegisterFromRegister }; // LD L, E
    InstructionTable[0x6C] = { 1, loadRegisterFromRegister }; // LD L, H
    InstructionTable[0x6D] = { 1, loadRegisterFromRegister }; // LD L, L -> redundant?
    InstructionTable[0x6E] = { 1, loadRegisterFromMemory }; // LD L, (HL)
    InstructionTable[0x6F] = { 1, loadRegisterFromRegister }; // LD L, A
    
    // LD (HL), r
    InstructionTable[0x70] = { 1, loadMemoryFromRegister }; // LD (HL), B
    InstructionTable[0x71] = { 1, loadMemoryFromRegister }; // LD (HL), C
    InstructionTable[0x72] = { 1, loadMemoryFromRegister }; // LD (HL), D
    InstructionTable[0x73] = { 1, loadMemoryFromRegister }; // LD (HL), E
    InstructionTable[0x74] = { 1, loadMemoryFromRegister }; // LD (HL), H
    InstructionTable[0x75] = { 1, loadMemoryFromRegister }; // LD (HL), L
    //TODO: MJB: what is this instruction?
//    InstructionTable[0x76] = { 1, ??? }; // LD (HL), (HL)
    InstructionTable[0x77] = { 1, loadMemoryFromRegister }; // LD (HL), A
    
    // LD A, r
    InstructionTable[0x78] = { 1, loadRegisterFromRegister }; // LD A, B
    InstructionTable[0x79] = { 1, loadRegisterFromRegister }; // LD A, C
    InstructionTable[0x7A] = { 1, loadRegisterFromRegister }; // LD A, D
    InstructionTable[0x7B] = { 1, loadRegisterFromRegister }; // LD A, E
    InstructionTable[0x7C] = { 1, loadRegisterFromRegister }; // LD A, H
    InstructionTable[0x7D] = { 1, loadRegisterFromRegister }; // LD A, L
    InstructionTable[0x7E] = { 1, loadRegisterFromMemory }; // LD A, (HL)
    InstructionTable[0x7F] = { 1, loadRegisterFromRegister }; // LD A, A -> redundant?
}


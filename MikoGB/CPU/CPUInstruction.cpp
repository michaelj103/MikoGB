//
//  CPUInstruction.cpp
//  MikoGB
//
//  Created on 5/3/20.
//

#include "CPUInstruction.hpp"
#include <exception>
#include <string>
#include <iostream>
#include "BitTwiddlingUtil.h"

#include "LoadInstructions8.hpp"
#include "LoadInstructions16.hpp"

using namespace std;
using namespace MikoGB;
using namespace CPUInstructions;

int CPUInstruction::UnrecognizedInstruction(const uint8_t *opcode, CPUCore &core) {
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

static int NoOp(const uint8_t *opcode, CPUCore &core) {
    return 1;
}

CPUInstruction *CPUInstruction::InstructionTable = nullptr;
void CPUInstruction::InitializeInstructionTable() {
    if (CPUInstruction::InstructionTable != nullptr) {
        return; //already initialized
    }
    
    // Technically, there can be 512 instructions but they aren't all used
    // 256 possible with a single slot. 256 possible with the extender 0xCB and the second byte
    // Single-byte instructions are indexed by their value
    // Two-byte instructions must have 0xCB as the first byte and are indexed by 0x1NN
    // So all feasible codes are 0x00 - 0x1FF (0-511), but there are gaps
    // Default initialized so that gaps are automatically UnrecognizedInstruction
    InstructionTable = new CPUInstruction[512]();
    
    InstructionTable[0x00] = { 1, NoOp };
    
    // LD dd, nn
    InstructionTable[0x01] = { 3, loadRegisterPairFromImmediate16 }; // LD BC, nn
    InstructionTable[0x11] = { 3, loadRegisterPairFromImmediate16 }; // LD DE, nn
    InstructionTable[0x21] = { 3, loadRegisterPairFromImmediate16 }; // LD HL, nn
    InstructionTable[0x31] = { 3, loadRegisterPairFromImmediate16 }; // LD SP, nn
    
    // LD r, n
    InstructionTable[0x06] = { 2, loadRegisterFromImmediate8 }; // LD B, n
    InstructionTable[0x0E] = { 2, loadRegisterFromImmediate8 }; // LD C, n
    InstructionTable[0x16] = { 2, loadRegisterFromImmediate8 }; // LD D, n
    InstructionTable[0x1E] = { 2, loadRegisterFromImmediate8 }; // LD E, n
    InstructionTable[0x26] = { 2, loadRegisterFromImmediate8 }; // LD H, n
    InstructionTable[0x2E] = { 2, loadRegisterFromImmediate8 }; // LD L, n
    InstructionTable[0x36] = { 2, loadPtrHLFromImmediate8 }; // LD (HL), n
    InstructionTable[0x3E] = { 2, loadRegisterFromImmediate8 }; // LD A, n
    
    // LD B, r
    InstructionTable[0x40] = { 1, loadRegisterFromRegister }; // LD B, B -> redundant?
    InstructionTable[0x41] = { 1, loadRegisterFromRegister }; // LD B, C
    InstructionTable[0x42] = { 1, loadRegisterFromRegister }; // LD B, D
    InstructionTable[0x43] = { 1, loadRegisterFromRegister }; // LD B, E
    InstructionTable[0x44] = { 1, loadRegisterFromRegister }; // LD B, H
    InstructionTable[0x45] = { 1, loadRegisterFromRegister }; // LD B, L
    InstructionTable[0x46] = { 1, loadRegisterFromPtrHL }; // LD B, (HL)
    InstructionTable[0x47] = { 1, loadRegisterFromRegister }; // LD B, A
    
    // LD C, r
    InstructionTable[0x48] = { 1, loadRegisterFromRegister }; // LD C, B
    InstructionTable[0x49] = { 1, loadRegisterFromRegister }; // LD C, C -> redundant?
    InstructionTable[0x4A] = { 1, loadRegisterFromRegister }; // LD C, D
    InstructionTable[0x4B] = { 1, loadRegisterFromRegister }; // LD C, E
    InstructionTable[0x4C] = { 1, loadRegisterFromRegister }; // LD C, H
    InstructionTable[0x4D] = { 1, loadRegisterFromRegister }; // LD C, L
    InstructionTable[0x4E] = { 1, loadRegisterFromPtrHL }; // LD C, (HL)
    InstructionTable[0x4F] = { 1, loadRegisterFromRegister }; // LD C, A
    
    // LD D, r
    InstructionTable[0x50] = { 1, loadRegisterFromRegister }; // LD D, B
    InstructionTable[0x51] = { 1, loadRegisterFromRegister }; // LD D, C
    InstructionTable[0x52] = { 1, loadRegisterFromRegister }; // LD D, D -> redundant?
    InstructionTable[0x53] = { 1, loadRegisterFromRegister }; // LD D, E
    InstructionTable[0x54] = { 1, loadRegisterFromRegister }; // LD D, H
    InstructionTable[0x55] = { 1, loadRegisterFromRegister }; // LD D, L
    InstructionTable[0x56] = { 1, loadRegisterFromPtrHL }; // LD D, (HL)
    InstructionTable[0x57] = { 1, loadRegisterFromRegister }; // LD D, A
    
    // LD E, r
    InstructionTable[0x58] = { 1, loadRegisterFromRegister }; // LD E, B
    InstructionTable[0x59] = { 1, loadRegisterFromRegister }; // LD E, C
    InstructionTable[0x5A] = { 1, loadRegisterFromRegister }; // LD E, D
    InstructionTable[0x5B] = { 1, loadRegisterFromRegister }; // LD E, E -> redundant?
    InstructionTable[0x5C] = { 1, loadRegisterFromRegister }; // LD E, H
    InstructionTable[0x5D] = { 1, loadRegisterFromRegister }; // LD E, L
    InstructionTable[0x5E] = { 1, loadRegisterFromPtrHL }; // LD E, (HL)
    InstructionTable[0x5F] = { 1, loadRegisterFromRegister }; // LD E, A
    
    // LD H, r
    InstructionTable[0x60] = { 1, loadRegisterFromRegister }; // LD H, B
    InstructionTable[0x61] = { 1, loadRegisterFromRegister }; // LD H, C
    InstructionTable[0x62] = { 1, loadRegisterFromRegister }; // LD H, D
    InstructionTable[0x63] = { 1, loadRegisterFromRegister }; // LD H, E
    InstructionTable[0x64] = { 1, loadRegisterFromRegister }; // LD H, H -> redundant?
    InstructionTable[0x65] = { 1, loadRegisterFromRegister }; // LD H, L
    InstructionTable[0x66] = { 1, loadRegisterFromPtrHL }; // LD H, (HL)
    InstructionTable[0x67] = { 1, loadRegisterFromRegister }; // LD H, A
    
    // LD L, r
    InstructionTable[0x68] = { 1, loadRegisterFromRegister }; // LD L, B
    InstructionTable[0x69] = { 1, loadRegisterFromRegister }; // LD L, C
    InstructionTable[0x6A] = { 1, loadRegisterFromRegister }; // LD L, D
    InstructionTable[0x6B] = { 1, loadRegisterFromRegister }; // LD L, E
    InstructionTable[0x6C] = { 1, loadRegisterFromRegister }; // LD L, H
    InstructionTable[0x6D] = { 1, loadRegisterFromRegister }; // LD L, L -> redundant?
    InstructionTable[0x6E] = { 1, loadRegisterFromPtrHL }; // LD L, (HL)
    InstructionTable[0x6F] = { 1, loadRegisterFromRegister }; // LD L, A
    
    // LD (HL), r
    InstructionTable[0x70] = { 1, loadPtrHLFromRegister }; // LD (HL), B
    InstructionTable[0x71] = { 1, loadPtrHLFromRegister }; // LD (HL), C
    InstructionTable[0x72] = { 1, loadPtrHLFromRegister }; // LD (HL), D
    InstructionTable[0x73] = { 1, loadPtrHLFromRegister }; // LD (HL), E
    InstructionTable[0x74] = { 1, loadPtrHLFromRegister }; // LD (HL), H
    InstructionTable[0x75] = { 1, loadPtrHLFromRegister }; // LD (HL), L
    //TODO: what is this instruction?
//    InstructionTable[0x76] = { 1, ??? }; // LD (HL), (HL)
    InstructionTable[0x77] = { 1, loadPtrHLFromRegister }; // LD (HL), A
    
    // LD A, r
    InstructionTable[0x78] = { 1, loadRegisterFromRegister }; // LD A, B
    InstructionTable[0x79] = { 1, loadRegisterFromRegister }; // LD A, C
    InstructionTable[0x7A] = { 1, loadRegisterFromRegister }; // LD A, D
    InstructionTable[0x7B] = { 1, loadRegisterFromRegister }; // LD A, E
    InstructionTable[0x7C] = { 1, loadRegisterFromRegister }; // LD A, H
    InstructionTable[0x7D] = { 1, loadRegisterFromRegister }; // LD A, L
    InstructionTable[0x7E] = { 1, loadRegisterFromPtrHL }; // LD A, (HL)
    InstructionTable[0x7F] = { 1, loadRegisterFromRegister }; // LD A, A -> redundant?
    
    // LD with accumulator and other register-pair pointers
    InstructionTable[0x02] = { 1, loadPtrBCFromAccumulator }; // LD (BC), A
    InstructionTable[0x12] = { 1, loadPtrDEFromAccumulator }; // LD (DE), A
    InstructionTable[0x0A] = { 1, loadAccumulatorFromPtrBC }; // LD A, (BC)
    InstructionTable[0x1A] = { 1, loadAccumulatorFromPtrDE }; // LD A, (DE)
    
    // LD with (C)
    InstructionTable[0xE2] = { 1, loadPtrCFromAccumulator }; // LD (C), A
    InstructionTable[0xF2] = { 1, loadAccumulatorFromPtrC }; // LD A, (C)
    
    // LD with accumulator and immediate pointers
    InstructionTable[0xE0] = { 2, loadPtrImmediate8FromAccumulator }; // LD (n), A
    InstructionTable[0xEA] = { 3, loadPtrImmediate16FromAccumulator }; // LD (nn), A
    InstructionTable[0xF0] = { 2, loadAccumulatorFromPtrImmediate8 }; // LD A, (n)
    InstructionTable[0xFA] = { 3, loadAccumulatorFromPtrImmediate16 }; // LD A, (nn)
    
    // LD A <-> HL with increment or decrement
    InstructionTable[0x22] = { 1, loadPtrHLIncrementFromAccumulator }; // LD (HLI), A
    InstructionTable[0x2A] = { 1, loadAccumulatorFromPtrHLIncrement }; // LD A, (HLI)
    InstructionTable[0x32] = { 1, loadPtrHLDecrementFromAccumulator }; // LD (HLD), A
    InstructionTable[0x3A] = { 1, loadAccumulatorFromPtrHLDecrement }; // LD A, (HLD)
    
    // PUSH qq
//    InstructionTable[0xC5] = { 1, ??? }; // PUSH BC
//    InstructionTable[0xD5] = { 1, ??? }; // PUSH DE
//    InstructionTable[0xE5] = { 1, ??? }; // PUSH HL
//    InstructionTable[0xF5] = { 1, ??? }; // PUSH AF
    
    // POP qq
//    InstructionTable[0xC1] = { 1, ??? }; // POP BC
//    InstructionTable[0xD1] = { 1, ??? }; // POP DE
//    InstructionTable[0xE1] = { 1, ??? }; // POP HL
//    InstructionTable[0xF1] = { 1, ??? }; // POP AF
    
    InstructionTable[0xF9] = { 1, loadStackPtrFromHL }; // LD SP, HL
    
    size_t instCount = 0;
    for (size_t i = 0; i < 512; ++i) {
        CPUInstruction &inst = InstructionTable[i];
        if (inst.size > 0) {
            instCount++;
        }
    }
    cout << "Loaded " << instCount << " Instructions" << endl;
}


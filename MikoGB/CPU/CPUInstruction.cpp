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
#include "ArithmeticInstructions8.hpp"
#include "BitOpInstructions.hpp"
#include "JumpInstructions.hpp"

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
    //TODO: HALT instruction
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
    InstructionTable[0x22] = { 1, loadPtrHLIncrementFromAccumulator }; // LD (HL+), A
    InstructionTable[0x2A] = { 1, loadAccumulatorFromPtrHLIncrement }; // LD A, (HL+)
    InstructionTable[0x32] = { 1, loadPtrHLDecrementFromAccumulator }; // LD (HL-), A
    InstructionTable[0x3A] = { 1, loadAccumulatorFromPtrHLDecrement }; // LD A, (HL-)
    
    // PUSH qq
    InstructionTable[0xC5] = { 1, pushQQ }; // PUSH BC
    InstructionTable[0xD5] = { 1, pushQQ }; // PUSH DE
    InstructionTable[0xE5] = { 1, pushQQ }; // PUSH HL
    InstructionTable[0xF5] = { 1, pushQQ }; // PUSH AF
    
    // POP qq
    InstructionTable[0xC1] = { 1, popQQ }; // POP BC
    InstructionTable[0xD1] = { 1, popQQ }; // POP DE
    InstructionTable[0xE1] = { 1, popQQ }; // POP HL
    InstructionTable[0xF1] = { 1, popQQ }; // POP AF
    
    // Stack pointer
    InstructionTable[0x08] = { 3, loadPtrImmediate16FromSP }; // LD (nn), SP
    InstructionTable[0xF8] = { 2, ldhl }; // LDHL SP, e
    InstructionTable[0xF9] = { 1, loadStackPtrFromHL }; // LD SP, HL
    
    // XOR Instructions
    InstructionTable[0xA8] = { 1, xorAccWithRegister }; // XOR B
    InstructionTable[0xA9] = { 1, xorAccWithRegister }; // XOR C
    InstructionTable[0xAA] = { 1, xorAccWithRegister }; // XOR D
    InstructionTable[0xAB] = { 1, xorAccWithRegister }; // XOR E
    InstructionTable[0xAC] = { 1, xorAccWithRegister }; // XOR H
    InstructionTable[0xAD] = { 1, xorAccWithRegister }; // XOR L
    InstructionTable[0xAE] = { 1, xorAccWithPtrHL }; // XOR (HL)
    InstructionTable[0xAF] = { 1, xorAccWithRegister }; // XOR A
    InstructionTable[0xEE] = { 2, xorAccWithImmediate8 }; // XOR n
    
    // Jump Instructions (relative)
    InstructionTable[0x18] = { 2, jumpUnconditionalRelative8 }; // JR e
    InstructionTable[0x20] = { 2, jumpConditionalRelative8 }; // JR NZ, e
    InstructionTable[0x28] = { 2, jumpConditionalRelative8 }; // JR Z, e
    InstructionTable[0x30] = { 2, jumpConditionalRelative8 }; // JR NC, e
    InstructionTable[0x38] = { 2, jumpConditionalRelative8 }; // JR C, e
    
    // Jump Instructions (absolute)
    InstructionTable[0xC2] = { 3, jumpConditionalAbsolute16 }; // JP NZ, nn
    InstructionTable[0xC3] = { 3, jumpUnconditionalAbsolute16 }; // JP nn
    InstructionTable[0xCA] = { 3, jumpConditionalAbsolute16 }; // JP Z, nn
    InstructionTable[0xD2] = { 3, jumpConditionalAbsolute16 }; // JP NC, nn
    InstructionTable[0xDA] = { 3, jumpConditionalAbsolute16 }; // JP C, nn
    InstructionTable[0xE9] = { 1, jumpUnconditionalHL }; // JP (HL)
    
    // 8-bit Add instructions
    InstructionTable[0x80] = { 1, addAccWithRegister }; // ADD A, B
    InstructionTable[0x81] = { 1, addAccWithRegister }; // ADD A, C
    InstructionTable[0x82] = { 1, addAccWithRegister }; // ADD A, D
    InstructionTable[0x83] = { 1, addAccWithRegister }; // ADD A, E
    InstructionTable[0x84] = { 1, addAccWithRegister }; // ADD A, H
    InstructionTable[0x85] = { 1, addAccWithRegister }; // ADD A, L
    InstructionTable[0x86] = { 1, addAccWithPtrHL }; // ADD A, (HL)
    InstructionTable[0x87] = { 1, addAccWithRegister }; // ADD A, A
    InstructionTable[0xC6] = { 2, addAccWithImmediate8 }; // ADD A, n
    
    // 8-bit Add with carry instructions
    InstructionTable[0x88] = { 1, addAccWithRegisterAndCarry }; // ADC A, B
    InstructionTable[0x89] = { 1, addAccWithRegisterAndCarry }; // ADC A, C
    InstructionTable[0x8A] = { 1, addAccWithRegisterAndCarry }; // ADC A, D
    InstructionTable[0x8B] = { 1, addAccWithRegisterAndCarry }; // ADC A, E
    InstructionTable[0x8C] = { 1, addAccWithRegisterAndCarry }; // ADC A, H
    InstructionTable[0x8D] = { 1, addAccWithRegisterAndCarry }; // ADC A, L
    InstructionTable[0x8E] = { 1, addAccWithPtrHLAndCarry }; // ADC A, (HL)
    InstructionTable[0x8F] = { 1, addAccWithRegisterAndCarry }; // ADC A, A
    InstructionTable[0xCE] = { 2, addAccWithImmediate8AndCarry }; // ADC A, n
    
    // 8-bit Sub instructions
    InstructionTable[0x90] = { 1, subAccWithRegister }; // SUB A, B
    InstructionTable[0x91] = { 1, subAccWithRegister }; // SUB A, C
    InstructionTable[0x92] = { 1, subAccWithRegister }; // SUB A, D
    InstructionTable[0x93] = { 1, subAccWithRegister }; // SUB A, E
    InstructionTable[0x94] = { 1, subAccWithRegister }; // SUB A, H
    InstructionTable[0x95] = { 1, subAccWithRegister }; // SUB A, L
    InstructionTable[0x96] = { 1, subAccWithPtrHL }; // SUB A, (HL)
    InstructionTable[0x97] = { 1, subAccWithRegister }; // SUB A, A
    InstructionTable[0xD6] = { 2, subAccWithImmediate8 }; // SUB A, n
    
    // 8-bit Sub with carry instructions
    InstructionTable[0x98] = { 1, subAccWithRegisterAndCarry }; // SBC A, B
    InstructionTable[0x99] = { 1, subAccWithRegisterAndCarry }; // SBC A, C
    InstructionTable[0x9A] = { 1, subAccWithRegisterAndCarry }; // SBC A, D
    InstructionTable[0x9B] = { 1, subAccWithRegisterAndCarry }; // SBC A, E
    InstructionTable[0x9C] = { 1, subAccWithRegisterAndCarry }; // SBC A, H
    InstructionTable[0x9D] = { 1, subAccWithRegisterAndCarry }; // SBC A, L
    InstructionTable[0x9E] = { 1, subAccWithPtrHLAndCarry }; // SBC A, (HL)
    InstructionTable[0x9F] = { 1, subAccWithRegisterAndCarry }; // SBC A, A
    InstructionTable[0xDE] = { 2, subAccWithImmediate8AndCarry }; // SBC A, n
    
    // =====================================
    // Extended Opcodes, prefixed with 0xCB
    // =====================================
    
    // bit read instructions
    // BIT 0
    InstructionTable[0x140] = { 2, bitReadFromRegister }; // BIT 0, B
    InstructionTable[0x141] = { 2, bitReadFromRegister }; // BIT 0, C
    InstructionTable[0x142] = { 2, bitReadFromRegister }; // BIT 0, D
    InstructionTable[0x143] = { 2, bitReadFromRegister }; // BIT 0, E
    InstructionTable[0x144] = { 2, bitReadFromRegister }; // BIT 0, H
    InstructionTable[0x145] = { 2, bitReadFromRegister }; // BIT 0, L
    InstructionTable[0x146] = { 2, bitReadFromPtrHL }; // BIT 0, (HL)
    InstructionTable[0x147] = { 2, bitReadFromRegister }; // BIT 0, A
    // BIT 1
    InstructionTable[0x148] = { 2, bitReadFromRegister }; // BIT 1, B
    InstructionTable[0x149] = { 2, bitReadFromRegister }; // BIT 1, C
    InstructionTable[0x14A] = { 2, bitReadFromRegister }; // BIT 1, D
    InstructionTable[0x14B] = { 2, bitReadFromRegister }; // BIT 1, E
    InstructionTable[0x14C] = { 2, bitReadFromRegister }; // BIT 1, H
    InstructionTable[0x14D] = { 2, bitReadFromRegister }; // BIT 1, L
    InstructionTable[0x14E] = { 2, bitReadFromPtrHL }; // BIT 1, (HL)
    InstructionTable[0x14F] = { 2, bitReadFromRegister }; // BIT 1, A
    // BIT 2
    InstructionTable[0x150] = { 2, bitReadFromRegister }; // BIT 2, B
    InstructionTable[0x151] = { 2, bitReadFromRegister }; // BIT 2, C
    InstructionTable[0x152] = { 2, bitReadFromRegister }; // BIT 2, D
    InstructionTable[0x153] = { 2, bitReadFromRegister }; // BIT 2, E
    InstructionTable[0x154] = { 2, bitReadFromRegister }; // BIT 2, H
    InstructionTable[0x155] = { 2, bitReadFromRegister }; // BIT 2, L
    InstructionTable[0x156] = { 2, bitReadFromPtrHL }; // BIT 2, (HL)
    InstructionTable[0x157] = { 2, bitReadFromRegister }; // BIT 2, A
    // BIT 3
    InstructionTable[0x158] = { 2, bitReadFromRegister }; // BIT 3, B
    InstructionTable[0x159] = { 2, bitReadFromRegister }; // BIT 3, C
    InstructionTable[0x15A] = { 2, bitReadFromRegister }; // BIT 3, D
    InstructionTable[0x15B] = { 2, bitReadFromRegister }; // BIT 3, E
    InstructionTable[0x15C] = { 2, bitReadFromRegister }; // BIT 3, H
    InstructionTable[0x15D] = { 2, bitReadFromRegister }; // BIT 3, L
    InstructionTable[0x15E] = { 2, bitReadFromPtrHL }; // BIT 3, (HL)
    InstructionTable[0x15F] = { 2, bitReadFromRegister }; // BIT 3, A
    // BIT 4
    InstructionTable[0x160] = { 2, bitReadFromRegister }; // BIT 4, B
    InstructionTable[0x161] = { 2, bitReadFromRegister }; // BIT 4, C
    InstructionTable[0x162] = { 2, bitReadFromRegister }; // BIT 4, D
    InstructionTable[0x163] = { 2, bitReadFromRegister }; // BIT 4, E
    InstructionTable[0x164] = { 2, bitReadFromRegister }; // BIT 4, H
    InstructionTable[0x165] = { 2, bitReadFromRegister }; // BIT 4, L
    InstructionTable[0x166] = { 2, bitReadFromPtrHL }; // BIT 4, (HL)
    InstructionTable[0x167] = { 2, bitReadFromRegister }; // BIT 4, A
    // BIT 5
    InstructionTable[0x168] = { 2, bitReadFromRegister }; // BIT 5, B
    InstructionTable[0x169] = { 2, bitReadFromRegister }; // BIT 5, C
    InstructionTable[0x16A] = { 2, bitReadFromRegister }; // BIT 5, D
    InstructionTable[0x16B] = { 2, bitReadFromRegister }; // BIT 5, E
    InstructionTable[0x16C] = { 2, bitReadFromRegister }; // BIT 5, H
    InstructionTable[0x16D] = { 2, bitReadFromRegister }; // BIT 5, L
    InstructionTable[0x16E] = { 2, bitReadFromPtrHL }; // BIT 5, (HL)
    InstructionTable[0x16F] = { 2, bitReadFromRegister }; // BIT 5, A
    // BIT 6
    InstructionTable[0x170] = { 2, bitReadFromRegister }; // BIT 6, B
    InstructionTable[0x171] = { 2, bitReadFromRegister }; // BIT 6, C
    InstructionTable[0x172] = { 2, bitReadFromRegister }; // BIT 6, D
    InstructionTable[0x173] = { 2, bitReadFromRegister }; // BIT 6, E
    InstructionTable[0x174] = { 2, bitReadFromRegister }; // BIT 6, H
    InstructionTable[0x175] = { 2, bitReadFromRegister }; // BIT 6, L
    InstructionTable[0x176] = { 2, bitReadFromPtrHL }; // BIT 6, (HL)
    InstructionTable[0x177] = { 2, bitReadFromRegister }; // BIT 6, A
    // BIT 7
    InstructionTable[0x178] = { 2, bitReadFromRegister }; // BIT 7, B
    InstructionTable[0x179] = { 2, bitReadFromRegister }; // BIT 7, C
    InstructionTable[0x17A] = { 2, bitReadFromRegister }; // BIT 7, D
    InstructionTable[0x17B] = { 2, bitReadFromRegister }; // BIT 7, E
    InstructionTable[0x17C] = { 2, bitReadFromRegister }; // BIT 7, H
    InstructionTable[0x17D] = { 2, bitReadFromRegister }; // BIT 7, L
    InstructionTable[0x17E] = { 2, bitReadFromPtrHL }; // BIT 7, (HL)
    InstructionTable[0x17F] = { 2, bitReadFromRegister }; // BIT 7, A
    
    // bit reset instructions
    // RES 0
    InstructionTable[0x180] = { 2, bitResetRegister }; // RES 0, B
    InstructionTable[0x181] = { 2, bitResetRegister }; // RES 0, C
    InstructionTable[0x182] = { 2, bitResetRegister }; // RES 0, D
    InstructionTable[0x183] = { 2, bitResetRegister }; // RES 0, E
    InstructionTable[0x184] = { 2, bitResetRegister }; // RES 0, H
    InstructionTable[0x185] = { 2, bitResetRegister }; // RES 0, L
    InstructionTable[0x186] = { 2, bitResetPtrHL }; // RES 0, (HL)
    InstructionTable[0x187] = { 2, bitResetRegister }; // RES 0, A
    // RES 1
    InstructionTable[0x188] = { 2, bitResetRegister }; // RES 1, B
    InstructionTable[0x189] = { 2, bitResetRegister }; // RES 1, C
    InstructionTable[0x18A] = { 2, bitResetRegister }; // RES 1, D
    InstructionTable[0x18B] = { 2, bitResetRegister }; // RES 1, E
    InstructionTable[0x18C] = { 2, bitResetRegister }; // RES 1, H
    InstructionTable[0x18D] = { 2, bitResetRegister }; // RES 1, L
    InstructionTable[0x18E] = { 2, bitResetPtrHL }; // RES 1, (HL)
    InstructionTable[0x18F] = { 2, bitResetRegister }; // RES 1, A
    // RES 2
    InstructionTable[0x190] = { 2, bitResetRegister }; // RES 2, B
    InstructionTable[0x191] = { 2, bitResetRegister }; // RES 2, C
    InstructionTable[0x192] = { 2, bitResetRegister }; // RES 2, D
    InstructionTable[0x193] = { 2, bitResetRegister }; // RES 2, E
    InstructionTable[0x194] = { 2, bitResetRegister }; // RES 2, H
    InstructionTable[0x195] = { 2, bitResetRegister }; // RES 2, L
    InstructionTable[0x196] = { 2, bitResetPtrHL }; // RES 2, (HL)
    InstructionTable[0x197] = { 2, bitResetRegister }; // RES 2, A
    // RES 3
    InstructionTable[0x198] = { 2, bitResetRegister }; // RES 3, B
    InstructionTable[0x199] = { 2, bitResetRegister }; // RES 3, C
    InstructionTable[0x19A] = { 2, bitResetRegister }; // RES 3, D
    InstructionTable[0x19B] = { 2, bitResetRegister }; // RES 3, E
    InstructionTable[0x19C] = { 2, bitResetRegister }; // RES 3, H
    InstructionTable[0x19D] = { 2, bitResetRegister }; // RES 3, L
    InstructionTable[0x19E] = { 2, bitResetPtrHL }; // RES 3, (HL)
    InstructionTable[0x19F] = { 2, bitResetRegister }; // RES 3, A
    // RES 4
    InstructionTable[0x1A0] = { 2, bitResetRegister }; // RES 4, B
    InstructionTable[0x1A1] = { 2, bitResetRegister }; // RES 4, C
    InstructionTable[0x1A2] = { 2, bitResetRegister }; // RES 4, D
    InstructionTable[0x1A3] = { 2, bitResetRegister }; // RES 4, E
    InstructionTable[0x1A4] = { 2, bitResetRegister }; // RES 4, H
    InstructionTable[0x1A5] = { 2, bitResetRegister }; // RES 4, L
    InstructionTable[0x1A6] = { 2, bitResetPtrHL }; // RES 4, (HL)
    InstructionTable[0x1A7] = { 2, bitResetRegister }; // RES 4, A
    // RES 5
    InstructionTable[0x1A8] = { 2, bitResetRegister }; // RES 5, B
    InstructionTable[0x1A9] = { 2, bitResetRegister }; // RES 5, C
    InstructionTable[0x1AA] = { 2, bitResetRegister }; // RES 5, D
    InstructionTable[0x1AB] = { 2, bitResetRegister }; // RES 5, E
    InstructionTable[0x1AC] = { 2, bitResetRegister }; // RES 5, H
    InstructionTable[0x1AD] = { 2, bitResetRegister }; // RES 5, L
    InstructionTable[0x1AE] = { 2, bitResetPtrHL }; // RES 5, (HL)
    InstructionTable[0x1AF] = { 2, bitResetRegister }; // RES 5, A
    // RES 6
    InstructionTable[0x1B0] = { 2, bitResetRegister }; // RES 6, B
    InstructionTable[0x1B1] = { 2, bitResetRegister }; // RES 6, C
    InstructionTable[0x1B2] = { 2, bitResetRegister }; // RES 6, D
    InstructionTable[0x1B3] = { 2, bitResetRegister }; // RES 6, E
    InstructionTable[0x1B4] = { 2, bitResetRegister }; // RES 6, H
    InstructionTable[0x1B5] = { 2, bitResetRegister }; // RES 6, L
    InstructionTable[0x1B6] = { 2, bitResetPtrHL }; // RES 6, (HL)
    InstructionTable[0x1B7] = { 2, bitResetRegister }; // RES 6, A
    // RES 7
    InstructionTable[0x1B8] = { 2, bitResetRegister }; // RES 7, B
    InstructionTable[0x1B9] = { 2, bitResetRegister }; // RES 7, C
    InstructionTable[0x1BA] = { 2, bitResetRegister }; // RES 7, D
    InstructionTable[0x1BB] = { 2, bitResetRegister }; // RES 7, E
    InstructionTable[0x1BC] = { 2, bitResetRegister }; // RES 7, H
    InstructionTable[0x1BD] = { 2, bitResetRegister }; // RES 7, L
    InstructionTable[0x1BE] = { 2, bitResetPtrHL }; // RES 7, (HL)
    InstructionTable[0x1BF] = { 2, bitResetRegister }; // RES 7, A
    
    // bit set instructions
    // SET 0
    InstructionTable[0x1C0] = { 2, bitSetRegister }; // SET 0, B
    InstructionTable[0x1C1] = { 2, bitSetRegister }; // SET 0, C
    InstructionTable[0x1C2] = { 2, bitSetRegister }; // SET 0, D
    InstructionTable[0x1C3] = { 2, bitSetRegister }; // SET 0, E
    InstructionTable[0x1C4] = { 2, bitSetRegister }; // SET 0, H
    InstructionTable[0x1C5] = { 2, bitSetRegister }; // SET 0, L
    InstructionTable[0x1C6] = { 2, bitSetPtrHL }; // SET 0, (HL)
    InstructionTable[0x1C7] = { 2, bitSetRegister }; // SET 0, A
    // SET 1
    InstructionTable[0x1C8] = { 2, bitSetRegister }; // SET 1, B
    InstructionTable[0x1C9] = { 2, bitSetRegister }; // SET 1, C
    InstructionTable[0x1CA] = { 2, bitSetRegister }; // SET 1, D
    InstructionTable[0x1CB] = { 2, bitSetRegister }; // SET 1, E
    InstructionTable[0x1CC] = { 2, bitSetRegister }; // SET 1, H
    InstructionTable[0x1CD] = { 2, bitSetRegister }; // SET 1, L
    InstructionTable[0x1CE] = { 2, bitSetPtrHL }; // SET 1, (HL)
    InstructionTable[0x1CF] = { 2, bitSetRegister }; // SET 1, A
    // SET 2
    InstructionTable[0x1D0] = { 2, bitSetRegister }; // SET 2, B
    InstructionTable[0x1D1] = { 2, bitSetRegister }; // SET 2, C
    InstructionTable[0x1D2] = { 2, bitSetRegister }; // SET 2, D
    InstructionTable[0x1D3] = { 2, bitSetRegister }; // SET 2, E
    InstructionTable[0x1D4] = { 2, bitSetRegister }; // SET 2, H
    InstructionTable[0x1D5] = { 2, bitSetRegister }; // SET 2, L
    InstructionTable[0x1D6] = { 2, bitSetPtrHL }; // SET 2, (HL)
    InstructionTable[0x1D7] = { 2, bitSetRegister }; // SET 2, A
    // SET 3
    InstructionTable[0x1D8] = { 2, bitSetRegister }; // SET 3, B
    InstructionTable[0x1D9] = { 2, bitSetRegister }; // SET 3, C
    InstructionTable[0x1DA] = { 2, bitSetRegister }; // SET 3, D
    InstructionTable[0x1DB] = { 2, bitSetRegister }; // SET 3, E
    InstructionTable[0x1DC] = { 2, bitSetRegister }; // SET 3, H
    InstructionTable[0x1DD] = { 2, bitSetRegister }; // SET 3, L
    InstructionTable[0x1DE] = { 2, bitSetPtrHL }; // SET 3, (HL)
    InstructionTable[0x1DF] = { 2, bitSetRegister }; // SET 3, A
    // SET 4
    InstructionTable[0x1E0] = { 2, bitSetRegister }; // SET 4, B
    InstructionTable[0x1E1] = { 2, bitSetRegister }; // SET 4, C
    InstructionTable[0x1E2] = { 2, bitSetRegister }; // SET 4, D
    InstructionTable[0x1E3] = { 2, bitSetRegister }; // SET 4, E
    InstructionTable[0x1E4] = { 2, bitSetRegister }; // SET 4, H
    InstructionTable[0x1E5] = { 2, bitSetRegister }; // SET 4, L
    InstructionTable[0x1E6] = { 2, bitSetPtrHL }; // SET 4, (HL)
    InstructionTable[0x1E7] = { 2, bitSetRegister }; // SET 4, A
    // SET 5
    InstructionTable[0x1E8] = { 2, bitSetRegister }; // SET 5, B
    InstructionTable[0x1E9] = { 2, bitSetRegister }; // SET 5, C
    InstructionTable[0x1EA] = { 2, bitSetRegister }; // SET 5, D
    InstructionTable[0x1EB] = { 2, bitSetRegister }; // SET 5, E
    InstructionTable[0x1EC] = { 2, bitSetRegister }; // SET 5, H
    InstructionTable[0x1ED] = { 2, bitSetRegister }; // SET 5, L
    InstructionTable[0x1EE] = { 2, bitSetPtrHL }; // SET 5, (HL)
    InstructionTable[0x1EF] = { 2, bitSetRegister }; // SET 5, A
    // SET 6
    InstructionTable[0x1F0] = { 2, bitSetRegister }; // SET 6, B
    InstructionTable[0x1F1] = { 2, bitSetRegister }; // SET 6, C
    InstructionTable[0x1F2] = { 2, bitSetRegister }; // SET 6, D
    InstructionTable[0x1F3] = { 2, bitSetRegister }; // SET 6, E
    InstructionTable[0x1F4] = { 2, bitSetRegister }; // SET 6, H
    InstructionTable[0x1F5] = { 2, bitSetRegister }; // SET 6, L
    InstructionTable[0x1F6] = { 2, bitSetPtrHL }; // SET 6, (HL)
    InstructionTable[0x1F7] = { 2, bitSetRegister }; // SET 6, A
    // SET 7
    InstructionTable[0x1F8] = { 2, bitSetRegister }; // SET 7, B
    InstructionTable[0x1F9] = { 2, bitSetRegister }; // SET 7, C
    InstructionTable[0x1FA] = { 2, bitSetRegister }; // SET 7, D
    InstructionTable[0x1FB] = { 2, bitSetRegister }; // SET 7, E
    InstructionTable[0x1FC] = { 2, bitSetRegister }; // SET 7, H
    InstructionTable[0x1FD] = { 2, bitSetRegister }; // SET 7, L
    InstructionTable[0x1FE] = { 2, bitSetPtrHL }; // SET 7, (HL)
    InstructionTable[0x1FF] = { 2, bitSetRegister }; // SET 7, A
    
    size_t instCount = 0;
    for (size_t i = 0; i < 512; ++i) {
        CPUInstruction &inst = InstructionTable[i];
        if (inst.size > 0) {
            instCount++;
        }
    }
    cout << "Loaded " << instCount << " Instructions" << endl;
}


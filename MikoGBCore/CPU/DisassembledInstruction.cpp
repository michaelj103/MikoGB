//
//  DisassembledInstruction.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "DisassembledInstruction.hpp"
#include <vector>

using namespace std;
using namespace MikoGB;

enum class ArgumentType {
    Immediate8          = 0,
    Immediate16         = 1,
    Immediate8Ptr       = 2,
    Immediate16Ptr      = 3,
    Immediate8Signed    = 4,
};

struct InstructionDescription {
    uint16_t size = 0;
    string description;
    vector<ArgumentType> argumentTypes;
};

InstructionDescription *DescriptionTable = nullptr;
void Disassembler::InitializeDisasseblyTable() {
    if (DescriptionTable != nullptr) {
        return; //already initialized
    }
    
    // technically 511 possible codes. See comment in CPUInstruction
    DescriptionTable = new InstructionDescription[512]();
    
    DescriptionTable[0x00] = { 1, "NoOp", {} };
    
    // LD dd, nn
    DescriptionTable[0x01] = { 3, "LD BC, %", {ArgumentType::Immediate16} }; // LD BC, nn
    DescriptionTable[0x11] = { 3, "LD DE, %", {ArgumentType::Immediate16} }; // LD DE, nn
    DescriptionTable[0x21] = { 3, "LD HL, %", {ArgumentType::Immediate16} }; // LD HL, nn
    DescriptionTable[0x31] = { 3, "LD SP, %", {ArgumentType::Immediate16} }; // LD SP, nn
    
    // LD r, n
    DescriptionTable[0x06] = { 2, "LD B, %", {ArgumentType::Immediate8} }; // LD B, n
    DescriptionTable[0x0E] = { 2, "LD C, %", {ArgumentType::Immediate8} }; // LD C, n
    DescriptionTable[0x16] = { 2, "LD D, %", {ArgumentType::Immediate8} }; // LD D, n
    DescriptionTable[0x1E] = { 2, "LD E, %", {ArgumentType::Immediate8} }; // LD E, n
    DescriptionTable[0x26] = { 2, "LD H, %", {ArgumentType::Immediate8} }; // LD H, n
    DescriptionTable[0x2E] = { 2, "LD L, %", {ArgumentType::Immediate8} }; // LD L, n
    DescriptionTable[0x36] = { 2, "LD (HL), %", {ArgumentType::Immediate8} }; // LD (HL), n
    DescriptionTable[0x3E] = { 2, "LD A, %", {ArgumentType::Immediate8} }; // LD A, n
    
    // LD B, r
    DescriptionTable[0x40] = { 1, "LD B, B", {} }; // LD B, B -> redundant?
    DescriptionTable[0x41] = { 1, "LD B, C", {} }; // LD B, C
    DescriptionTable[0x42] = { 1, "LD B, D", {} }; // LD B, D
    DescriptionTable[0x43] = { 1, "LD B, E", {} }; // LD B, E
    DescriptionTable[0x44] = { 1, "LD B, H", {} }; // LD B, H
    DescriptionTable[0x45] = { 1, "LD B, L", {} }; // LD B, L
    DescriptionTable[0x46] = { 1, "LD B, (HL)", {} }; // LD B, (HL)
    DescriptionTable[0x47] = { 1, "LD B, A", {} }; // LD B, A
    
    // LD C, r
    DescriptionTable[0x48] = { 1, "LD C, B", {} }; // LD C, B
    DescriptionTable[0x49] = { 1, "LD C, C", {} }; // LD C, C -> redundant?
    DescriptionTable[0x4A] = { 1, "LD C, D", {} }; // LD C, D
    DescriptionTable[0x4B] = { 1, "LD C, E", {} }; // LD C, E
    DescriptionTable[0x4C] = { 1, "LD C, H", {} }; // LD C, H
    DescriptionTable[0x4D] = { 1, "LD C, L", {} }; // LD C, L
    DescriptionTable[0x4E] = { 1, "LD C, (HL)", {} }; // LD C, (HL)
    DescriptionTable[0x4F] = { 1, "LD C, A", {} }; // LD C, A
    
    // LD D, r
    DescriptionTable[0x50] = { 1, "LD D, B", {} }; // LD D, B
    DescriptionTable[0x51] = { 1, "LD D, C", {} }; // LD D, C
    DescriptionTable[0x52] = { 1, "LD D, D", {} }; // LD D, D -> redundant?
    DescriptionTable[0x53] = { 1, "LD D, E", {} }; // LD D, E
    DescriptionTable[0x54] = { 1, "LD D, H", {} }; // LD D, H
    DescriptionTable[0x55] = { 1, "LD D, L", {} }; // LD D, L
    DescriptionTable[0x56] = { 1, "LD D, (HL)", {} }; // LD D, (HL)
    DescriptionTable[0x57] = { 1, "LD D, A", {} }; // LD D, A
    
    // LD E, r
    DescriptionTable[0x58] = { 1, "LD E, B", {} }; // LD E, B
    DescriptionTable[0x59] = { 1, "LD E, C", {} }; // LD E, C
    DescriptionTable[0x5A] = { 1, "LD E, D", {} }; // LD E, D
    DescriptionTable[0x5B] = { 1, "LD E, E", {} }; // LD E, E -> redundant?
    DescriptionTable[0x5C] = { 1, "LD E, H", {} }; // LD E, H
    DescriptionTable[0x5D] = { 1, "LD E, L", {} }; // LD E, L
    DescriptionTable[0x5E] = { 1, "LD E, (HL)", {} }; // LD E, (HL)
    DescriptionTable[0x5F] = { 1, "LD E, A", {} }; // LD E, A
    
    // LD H, r
    DescriptionTable[0x60] = { 1, "LD H, B", {} }; // LD H, B
    DescriptionTable[0x61] = { 1, "LD H, C", {} }; // LD H, C
    DescriptionTable[0x62] = { 1, "LD H, D", {} }; // LD H, D
    DescriptionTable[0x63] = { 1, "LD H, E", {} }; // LD H, E
    DescriptionTable[0x64] = { 1, "LD H, H", {} }; // LD H, H -> redundant?
    DescriptionTable[0x65] = { 1, "LD H, L", {} }; // LD H, L
    DescriptionTable[0x66] = { 1, "LD H, (HL)", {} }; // LD H, (HL)
    DescriptionTable[0x67] = { 1, "LD H, A", {} }; // LD H, A
    
    // LD L, r
    DescriptionTable[0x68] = { 1, "LD L, B", {} }; // LD L, B
    DescriptionTable[0x69] = { 1, "LD L, C", {} }; // LD L, C
    DescriptionTable[0x6A] = { 1, "LD L, D", {} }; // LD L, D
    DescriptionTable[0x6B] = { 1, "LD L, E", {} }; // LD L, E
    DescriptionTable[0x6C] = { 1, "LD L, H", {} }; // LD L, H
    DescriptionTable[0x6D] = { 1, "LD L, L", {} }; // LD L, L -> redundant?
    DescriptionTable[0x6E] = { 1, "LD L, (HL)", {} }; // LD L, (HL)
    DescriptionTable[0x6F] = { 1, "LD L, A", {} }; // LD L, A
    
    // LD (HL), r
    DescriptionTable[0x70] = { 1, "LD (HL), B", {} }; // LD (HL), B
    DescriptionTable[0x71] = { 1, "LD (HL), C", {} }; // LD (HL), C
    DescriptionTable[0x72] = { 1, "LD (HL), D", {} }; // LD (HL), D
    DescriptionTable[0x73] = { 1, "LD (HL), E", {} }; // LD (HL), E
    DescriptionTable[0x74] = { 1, "LD (HL), H", {} }; // LD (HL), H
    DescriptionTable[0x75] = { 1, "LD (HL), L", {} }; // LD (HL), L
    // 0x76 is HALT. LD (HL), (HL) wouldn't do anything
    DescriptionTable[0x77] = { 1, "LD (HL), A", {} }; // LD (HL), A
    
    // LD A, r
    DescriptionTable[0x78] = { 1, "LD A, B", {} }; // LD A, B
    DescriptionTable[0x79] = { 1, "LD A, C", {} }; // LD A, C
    DescriptionTable[0x7A] = { 1, "LD A, D", {} }; // LD A, D
    DescriptionTable[0x7B] = { 1, "LD A, E", {} }; // LD A, E
    DescriptionTable[0x7C] = { 1, "LD A, H", {} }; // LD A, H
    DescriptionTable[0x7D] = { 1, "LD A, L", {} }; // LD A, L
    DescriptionTable[0x7E] = { 1, "LD A, (HL)", {} }; // LD A, (HL)
    DescriptionTable[0x7F] = { 1, "LD A, A", {} }; // LD A, A -> redundant?
    
    // LD with accumulator and other register-pair pointers
    DescriptionTable[0x02] = { 1, "LD (BC), A", {} }; // LD (BC), A
    DescriptionTable[0x12] = { 1, "LD (DE), A", {} }; // LD (DE), A
    DescriptionTable[0x0A] = { 1, "LD A, (BC)", {} }; // LD A, (BC)
    DescriptionTable[0x1A] = { 1, "LD A, (DE)", {} }; // LD A, (DE)
    
    // LD with (C)
    DescriptionTable[0xE2] = { 1, "LD (C), A" }; // LD (C), A
    DescriptionTable[0xF2] = { 1, "LD A, (C)" }; // LD A, (C)
    
    // LD with accumulator and immediate pointers
    DescriptionTable[0xE0] = { 2, "LD (%), A", {ArgumentType::Immediate8Ptr} }; // LD (n), A
    DescriptionTable[0xEA] = { 3, "LD (%), A", {ArgumentType::Immediate16Ptr} }; // LD (nn), A
    DescriptionTable[0xF0] = { 2, "LD A, (%)", {ArgumentType::Immediate8Ptr} }; // LD A, (n)
    DescriptionTable[0xFA] = { 3, "LD A, (%)", {ArgumentType::Immediate16Ptr} }; // LD A, (nn)
    
    // LD A <-> HL with increment or decrement
    DescriptionTable[0x22] = { 1, "LD (HL+), A", {} }; // LD (HL+), A
    DescriptionTable[0x2A] = { 1, "LD A, (HL+)", {} }; // LD A, (HL+)
    DescriptionTable[0x32] = { 1, "LD (HL-), A", {} }; // LD (HL-), A
    DescriptionTable[0x3A] = { 1, "LD A, (HL-)", {} }; // LD A, (HL-)
    
    // PUSH qq
    DescriptionTable[0xC5] = { 1, "PUSH BC", {} }; // PUSH BC
    DescriptionTable[0xD5] = { 1, "PUSH DE", {} }; // PUSH DE
    DescriptionTable[0xE5] = { 1, "PUSH HL", {} }; // PUSH HL
    DescriptionTable[0xF5] = { 1, "PUSH AF", {} }; // PUSH AF
    
    // POP qq
    DescriptionTable[0xC1] = { 1, "POP BC", {} }; // POP BC
    DescriptionTable[0xD1] = { 1, "POP DE", {} }; // POP DE
    DescriptionTable[0xE1] = { 1, "POP HL", {} }; // POP HL
    DescriptionTable[0xF1] = { 1, "POP AF", {} }; // POP AF
    
    // Stack pointer
    DescriptionTable[0x08] = { 3, "LD (%), SP", {} }; // LD (nn), SP
    DescriptionTable[0xF8] = { 2, "LDHL SP, %", {ArgumentType::Immediate8Signed} }; // LDHL SP, e
    DescriptionTable[0xF9] = { 1, "LD SP, HL", {} }; // LD SP, HL
    
    // Jump Instructions (relative)
    DescriptionTable[0x18] = { 2, "JR %", {ArgumentType::Immediate8Signed} }; // JR e
    DescriptionTable[0x20] = { 2, "JR NZ, %", {ArgumentType::Immediate8Signed} }; // JR NZ, e
    DescriptionTable[0x28] = { 2, "JR Z, %", {ArgumentType::Immediate8Signed} }; // JR Z, e
    DescriptionTable[0x30] = { 2, "JR NC, %", {ArgumentType::Immediate8Signed} }; // JR NC, e
    DescriptionTable[0x38] = { 2, "JR C, %", {ArgumentType::Immediate8Signed} }; // JR C, e
    
    // Jump Instructions (absolute)
    DescriptionTable[0xC2] = { 3, "JP NZ, %", {ArgumentType::Immediate16} }; // JP NZ, nn
    DescriptionTable[0xC3] = { 3, "JP %", {ArgumentType::Immediate16} }; // JP nn
    DescriptionTable[0xCA] = { 3, "JP Z, %", {ArgumentType::Immediate16} }; // JP Z, nn
    DescriptionTable[0xD2] = { 3, "JP NC, %", {ArgumentType::Immediate16} }; // JP NC, nn
    DescriptionTable[0xDA] = { 3, "JP C, %", {ArgumentType::Immediate16} }; // JP C, nn
    DescriptionTable[0xE9] = { 1, "JP (HL)", {} }; // JP (HL)
    
    // 8-bit Add instructions
    DescriptionTable[0x80] = { 1, "ADD A, B", {} }; // ADD A, B
    DescriptionTable[0x81] = { 1, "ADD A, C", {} }; // ADD A, C
    DescriptionTable[0x82] = { 1, "ADD A, D", {} }; // ADD A, D
    DescriptionTable[0x83] = { 1, "ADD A, E", {} }; // ADD A, E
    DescriptionTable[0x84] = { 1, "ADD A, H", {} }; // ADD A, H
    DescriptionTable[0x85] = { 1, "ADD A, L", {} }; // ADD A, L
    DescriptionTable[0x86] = { 1, "ADD A, (HL)", {} }; // ADD A, (HL)
    DescriptionTable[0x87] = { 1, "ADD A, A", {} }; // ADD A, A
    DescriptionTable[0xC6] = { 2, "ADD A, %", {ArgumentType::Immediate8} }; // ADD A, n
    
    // 8-bit Add with carry instructions
    DescriptionTable[0x88] = { 1, "ADC A, B", {} }; // ADC A, B
    DescriptionTable[0x89] = { 1, "ADC A, C", {} }; // ADC A, C
    DescriptionTable[0x8A] = { 1, "ADC A, D", {} }; // ADC A, D
    DescriptionTable[0x8B] = { 1, "ADC A, E", {} }; // ADC A, E
    DescriptionTable[0x8C] = { 1, "ADC A, H", {} }; // ADC A, H
    DescriptionTable[0x8D] = { 1, "ADC A, L", {} }; // ADC A, L
    DescriptionTable[0x8E] = { 1, "ADC A, (HL)", {} }; // ADC A, (HL)
    DescriptionTable[0x8F] = { 1, "ADC A, A", {} }; // ADC A, A
    DescriptionTable[0xCE] = { 2, "ADC A, %", {ArgumentType::Immediate8} }; // ADC A, n
    
    // 8-bit Sub instructions
    DescriptionTable[0x90] = { 1, "SUB A, B", {} }; // SUB A, B
    DescriptionTable[0x91] = { 1, "SUB A, C", {} }; // SUB A, C
    DescriptionTable[0x92] = { 1, "SUB A, D", {} }; // SUB A, D
    DescriptionTable[0x93] = { 1, "SUB A, E", {} }; // SUB A, E
    DescriptionTable[0x94] = { 1, "SUB A, H", {} }; // SUB A, H
    DescriptionTable[0x95] = { 1, "SUB A, L", {} }; // SUB A, L
    DescriptionTable[0x96] = { 1, "SUB A, (HL)", {} }; // SUB A, (HL)
    DescriptionTable[0x97] = { 1, "SUB A, A", {} }; // SUB A, A
    DescriptionTable[0xD6] = { 2, "SUB A, %", {ArgumentType::Immediate8} }; // SUB A, n
    
    // 8-bit Sub with carry instructions
    DescriptionTable[0x98] = { 1, "SBC A, B", {} }; // SBC A, B
    DescriptionTable[0x99] = { 1, "SBC A, C", {} }; // SBC A, C
    DescriptionTable[0x9A] = { 1, "SBC A, D", {} }; // SBC A, D
    DescriptionTable[0x9B] = { 1, "SBC A, E", {} }; // SBC A, E
    DescriptionTable[0x9C] = { 1, "SBC A, H", {} }; // SBC A, H
    DescriptionTable[0x9D] = { 1, "SBC A, L", {} }; // SBC A, L
    DescriptionTable[0x9E] = { 1, "SBC A, (HL)", {} }; // SBC A, (HL)
    DescriptionTable[0x9F] = { 1, "SBC A, A", {} }; // SBC A, A
    DescriptionTable[0xDE] = { 2, "SBC A, %", {ArgumentType::Immediate8} }; // SBC A, n
    
    // 8-bit AND instructions
    DescriptionTable[0xA0] = { 1, "AND B", {} }; // AND B
    DescriptionTable[0xA1] = { 1, "AND C", {} }; // AND C
    DescriptionTable[0xA2] = { 1, "AND D", {} }; // AND D
    DescriptionTable[0xA3] = { 1, "AND E", {} }; // AND E
    DescriptionTable[0xA4] = { 1, "AND H", {} }; // AND H
    DescriptionTable[0xA5] = { 1, "AND L", {} }; // AND L
    DescriptionTable[0xA6] = { 1, "AND (HL)", {} }; // AND (HL)
    DescriptionTable[0xA7] = { 1, "AND A", {} }; // AND A
    DescriptionTable[0xE6] = { 2, "AND %", {ArgumentType::Immediate8} }; // AND n
    
    // 8-bit OR instructions
    DescriptionTable[0xB0] = { 1, "OR B", {} }; // OR B
    DescriptionTable[0xB1] = { 1, "OR C", {} }; // OR C
    DescriptionTable[0xB2] = { 1, "OR D", {} }; // OR D
    DescriptionTable[0xB3] = { 1, "OR E", {} }; // OR E
    DescriptionTable[0xB4] = { 1, "OR H", {} }; // OR H
    DescriptionTable[0xB5] = { 1, "OR L", {} }; // OR L
    DescriptionTable[0xB6] = { 1, "OR (HL)", {} }; // OR (HL)
    DescriptionTable[0xB7] = { 1, "OR A", {} }; // OR A
    DescriptionTable[0xF6] = { 2, "OR %", {ArgumentType::Immediate8} }; // OR n
    
    // 8-bit XOR Instructions
    DescriptionTable[0xA8] = { 1, "XOR B", {} }; // XOR B
    DescriptionTable[0xA9] = { 1, "XOR C", {} }; // XOR C
    DescriptionTable[0xAA] = { 1, "XOR D", {} }; // XOR D
    DescriptionTable[0xAB] = { 1, "XOR E", {} }; // XOR E
    DescriptionTable[0xAC] = { 1, "XOR H", {} }; // XOR H
    DescriptionTable[0xAD] = { 1, "XOR L", {} }; // XOR L
    DescriptionTable[0xAE] = { 1, "XOR (HL)", {} }; // XOR (HL)
    DescriptionTable[0xAF] = { 1, "XOR A", {} }; // XOR A
    DescriptionTable[0xEE] = { 2, "XOR %", {ArgumentType::Immediate8} }; // XOR n
    
    // 8-bit CP Instructions
    DescriptionTable[0xB8] = { 1, "CP B", {} }; // CP B
    DescriptionTable[0xB9] = { 1, "CP C", {} }; // CP C
    DescriptionTable[0xBA] = { 1, "CP D", {} }; // CP D
    DescriptionTable[0xBB] = { 1, "CP E", {} }; // CP E
    DescriptionTable[0xBC] = { 1, "CP H", {} }; // CP H
    DescriptionTable[0xBD] = { 1, "CP L", {} }; // CP L
    DescriptionTable[0xBE] = { 1, "CP (HL)", {} }; // CP (HL)
    DescriptionTable[0xBF] = { 1, "CP A", {} }; // CP A
    DescriptionTable[0xFE] = { 2, "CP %", {ArgumentType::Immediate8} }; // CP n
    
    // 8-bit INC instructions
    DescriptionTable[0x04] = { 1, "INC B", {} }; // INC B
    DescriptionTable[0x0C] = { 1, "INC C", {} }; // INC C
    DescriptionTable[0x14] = { 1, "INC D", {} }; // INC D
    DescriptionTable[0x1C] = { 1, "INC E", {} }; // INC E
    DescriptionTable[0x24] = { 1, "INC H", {} }; // INC H
    DescriptionTable[0x2C] = { 1, "INC L", {} }; // INC L
    DescriptionTable[0x34] = { 1, "INC (HL)", {} }; // INC (HL)
    DescriptionTable[0x3C] = { 1, "INC A", {} }; // INC A
    
    // 8-bit DEC instructions
    DescriptionTable[0x05] = { 1, "DEC B", {} }; // DEC B
    DescriptionTable[0x0D] = { 1, "DEC C", {} }; // DEC C
    DescriptionTable[0x15] = { 1, "DEC D", {} }; // DEC D
    DescriptionTable[0x1D] = { 1, "DEC E", {} }; // DEC E
    DescriptionTable[0x25] = { 1, "DEC H", {} }; // DEC H
    DescriptionTable[0x2D] = { 1, "DEC L", {} }; // DEC L
    DescriptionTable[0x35] = { 1, "DEC (HL)", {} }; // DEC (HL)
    DescriptionTable[0x3D] = { 1, "DEC A", {} }; // DEC A
    
    // 16-bit ADD instructions
    DescriptionTable[0x09] = { 1, "ADD HL, BC", {} }; // ADD HL, BC
    DescriptionTable[0x19] = { 1, "ADD HL, DE", {} }; // ADD HL, DE
    DescriptionTable[0x29] = { 1, "ADD HL, HL", {} }; // ADD HL, HL
    DescriptionTable[0x39] = { 1, "ADD HL, SP", {} }; // ADD HL, SP
    DescriptionTable[0xE8] = { 2, "ADD SP, %", {ArgumentType::Immediate8Signed} }; // ADD SP, e
    
    // 16-bit INC and DEC instructions
    DescriptionTable[0x03] = { 1, "INC BC", {} }; // INC BC
    DescriptionTable[0x13] = { 1, "INC DE", {} }; // INC DE
    DescriptionTable[0x23] = { 1, "INC HL", {} }; // INC HL
    DescriptionTable[0x33] = { 1, "INC SP", {} }; // INC SP
    DescriptionTable[0x0B] = { 1, "DEC BC", {} }; // DEC BC
    DescriptionTable[0x1B] = { 1, "DEC DE", {} }; // DEC DE
    DescriptionTable[0x2B] = { 1, "DEC HL", {} }; // DEC HL
    DescriptionTable[0x3B] = { 1, "DEC SP", {} }; // DEC SP
    
    // CALL and RET instructions
    DescriptionTable[0xC0] = { 1, "RET NZ", {} }; // RET NZ
    DescriptionTable[0xC4] = { 3, "CALL NZ %", {ArgumentType::Immediate16} }; // CALL NZ nn
    DescriptionTable[0xC8] = { 1, "RET Z", {} }; // RET Z
    DescriptionTable[0xC9] = { 1, "RET", {} }; // RET
    DescriptionTable[0xCC] = { 3, "CALL Z %", {ArgumentType::Immediate16} }; // CALL Z nn
    DescriptionTable[0xCD] = { 3, "CALL %", {ArgumentType::Immediate16} }; // CALL nn
    DescriptionTable[0xD0] = { 1, "RET NC" }; // RET NC
    DescriptionTable[0xD4] = { 3, "CALL NC %", {ArgumentType::Immediate16} }; // CALL NC nn
    DescriptionTable[0xD8] = { 1, "RET C", {} }; // RET C
    DescriptionTable[0xD9] = { 1, "RETI", {} }; // RETI
    DescriptionTable[0xDC] = { 3, "CALL C %", {ArgumentType::Immediate16} }; // CALL C nn
    
    // RST instruction. A bit of a weird one
    DescriptionTable[0xC7] = { 1, "RST 0", {} }; // RST 0
    DescriptionTable[0xCF] = { 1, "RST 1", {} }; // RST 1
    DescriptionTable[0xD7] = { 1, "RST 2", {} }; // RST 2
    DescriptionTable[0xDF] = { 1, "RST 3", {} }; // RST 3
    DescriptionTable[0xE7] = { 1, "RST 4", {} }; // RST 4
    DescriptionTable[0xEF] = { 1, "RST 5", {} }; // RST 5
    DescriptionTable[0xF7] = { 1, "RST 6", {} }; // RST 6
    DescriptionTable[0xFF] = { 1, "RST 7", {} }; // RST 7
    
    // Rotate instructions
    DescriptionTable[0x07] = { 1, "RLCA", {} }; // RLCA
    DescriptionTable[0x17] = { 1, "RLA", {} }; // RLA
    DescriptionTable[0x0F] = { 1, "RRCA", {} }; // RRCA
    DescriptionTable[0x1F] = { 1, "RRA", {} }; // RRA
    
    // Special instructions
    DescriptionTable[0x27] = { 1, "DAA", {} }; // DAA
    DescriptionTable[0x2F] = { 1, "CPL", {} }; // CPL
    DescriptionTable[0x37] = { 1, "SCF", {} }; // SCF
    DescriptionTable[0x3F] = { 1, "CCF", {} }; // CCF
    DescriptionTable[0xF3] = { 1, "DI", {} }; // DI
    DescriptionTable[0xFB] = { 1, "EI", {} }; // EI
    DescriptionTable[0x76] = { 1, "HALT", {} }; // HALT
    DescriptionTable[0x10] = { 2, "STOP", {} }; // STOP. Technically 2 bytes, the second is expected to be 0x00
    
    // =====================================
    // Extended Opcodes, prefixed with 0xCB
    // =====================================
    
    // bit read instructions
    // BIT 0
    DescriptionTable[0x140] = { 2, "BIT 0, B", {} }; // BIT 0, B
    DescriptionTable[0x141] = { 2, "BIT 0, C", {} }; // BIT 0, C
    DescriptionTable[0x142] = { 2, "BIT 0, D", {} }; // BIT 0, D
    DescriptionTable[0x143] = { 2, "BIT 0, E", {} }; // BIT 0, E
    DescriptionTable[0x144] = { 2, "BIT 0, H", {} }; // BIT 0, H
    DescriptionTable[0x145] = { 2, "BIT 0, L", {} }; // BIT 0, L
    DescriptionTable[0x146] = { 2, "BIT 0, (HL)", {} }; // BIT 0, (HL)
    DescriptionTable[0x147] = { 2, "BIT 0, A", {} }; // BIT 0, A
    // BIT 1
    DescriptionTable[0x148] = { 2, "BIT 1, B", {} }; // BIT 1, B
    DescriptionTable[0x149] = { 2, "BIT 1, C", {} }; // BIT 1, C
    DescriptionTable[0x14A] = { 2, "BIT 1, D", {} }; // BIT 1, D
    DescriptionTable[0x14B] = { 2, "BIT 1, E", {} }; // BIT 1, E
    DescriptionTable[0x14C] = { 2, "BIT 1, H", {} }; // BIT 1, H
    DescriptionTable[0x14D] = { 2, "BIT 1, L", {} }; // BIT 1, L
    DescriptionTable[0x14E] = { 2, "BIT 1, (HL)", {} }; // BIT 1, (HL)
    DescriptionTable[0x14F] = { 2, "BIT 1, A", {} }; // BIT 1, A
    // BIT 2
    DescriptionTable[0x150] = { 2, "BIT 2, B", {} }; // BIT 2, B
    DescriptionTable[0x151] = { 2, "BIT 2, C", {} }; // BIT 2, C
    DescriptionTable[0x152] = { 2, "BIT 2, D", {} }; // BIT 2, D
    DescriptionTable[0x153] = { 2, "BIT 2, E", {} }; // BIT 2, E
    DescriptionTable[0x154] = { 2, "BIT 2, H", {} }; // BIT 2, H
    DescriptionTable[0x155] = { 2, "BIT 2, L", {} }; // BIT 2, L
    DescriptionTable[0x156] = { 2, "BIT 2, (HL)", {} }; // BIT 2, (HL)
    DescriptionTable[0x157] = { 2, "BIT 2, A", {} }; // BIT 2, A
    // BIT 3
    DescriptionTable[0x158] = { 2, "BIT 3, B", {} }; // BIT 3, B
    DescriptionTable[0x159] = { 2, "BIT 3, C", {} }; // BIT 3, C
    DescriptionTable[0x15A] = { 2, "BIT 3, D", {} }; // BIT 3, D
    DescriptionTable[0x15B] = { 2, "BIT 3, E", {} }; // BIT 3, E
    DescriptionTable[0x15C] = { 2, "BIT 3, H", {} }; // BIT 3, H
    DescriptionTable[0x15D] = { 2, "BIT 3, L", {} }; // BIT 3, L
    DescriptionTable[0x15E] = { 2, "BIT 3, (HL)", {} }; // BIT 3, (HL)
    DescriptionTable[0x15F] = { 2, "BIT 3, A", {} }; // BIT 3, A
    // BIT 4
    DescriptionTable[0x160] = { 2, "BIT 4, B", {} }; // BIT 4, B
    DescriptionTable[0x161] = { 2, "BIT 4, C", {} }; // BIT 4, C
    DescriptionTable[0x162] = { 2, "BIT 4, D", {} }; // BIT 4, D
    DescriptionTable[0x163] = { 2, "BIT 4, E", {} }; // BIT 4, E
    DescriptionTable[0x164] = { 2, "BIT 4, H", {} }; // BIT 4, H
    DescriptionTable[0x165] = { 2, "BIT 4, L", {} }; // BIT 4, L
    DescriptionTable[0x166] = { 2, "BIT 4, (HL)", {} }; // BIT 4, (HL)
    DescriptionTable[0x167] = { 2, "BIT 4, A", {} }; // BIT 4, A
    // BIT 5
    DescriptionTable[0x168] = { 2, "BIT 5, B", {} }; // BIT 5, B
    DescriptionTable[0x169] = { 2, "BIT 5, C", {} }; // BIT 5, C
    DescriptionTable[0x16A] = { 2, "BIT 5, D", {} }; // BIT 5, D
    DescriptionTable[0x16B] = { 2, "BIT 5, E", {} }; // BIT 5, E
    DescriptionTable[0x16C] = { 2, "BIT 5, H", {} }; // BIT 5, H
    DescriptionTable[0x16D] = { 2, "BIT 5, L", {} }; // BIT 5, L
    DescriptionTable[0x16E] = { 2, "BIT 5, (HL)", {} }; // BIT 5, (HL)
    DescriptionTable[0x16F] = { 2, "BIT 5, A", {} }; // BIT 5, A
    // BIT 6
    DescriptionTable[0x170] = { 2, "BIT 6, B", {} }; // BIT 6, B
    DescriptionTable[0x171] = { 2, "BIT 6, C", {} }; // BIT 6, C
    DescriptionTable[0x172] = { 2, "BIT 6, D", {} }; // BIT 6, D
    DescriptionTable[0x173] = { 2, "BIT 6, E", {} }; // BIT 6, E
    DescriptionTable[0x174] = { 2, "BIT 6, H", {} }; // BIT 6, H
    DescriptionTable[0x175] = { 2, "BIT 6, L", {} }; // BIT 6, L
    DescriptionTable[0x176] = { 2, "BIT 6, (HL)", {} }; // BIT 6, (HL)
    DescriptionTable[0x177] = { 2, "BIT 6, A", {} }; // BIT 6, A
    // BIT 7
    DescriptionTable[0x178] = { 2, "BIT 7, B", {} }; // BIT 7, B
    DescriptionTable[0x179] = { 2, "BIT 7, C", {} }; // BIT 7, C
    DescriptionTable[0x17A] = { 2, "BIT 7, D", {} }; // BIT 7, D
    DescriptionTable[0x17B] = { 2, "BIT 7, E", {} }; // BIT 7, E
    DescriptionTable[0x17C] = { 2, "BIT 7, H", {} }; // BIT 7, H
    DescriptionTable[0x17D] = { 2, "BIT 7, L", {} }; // BIT 7, L
    DescriptionTable[0x17E] = { 2, "BIT 7, (HL)", {} }; // BIT 7, (HL)
    DescriptionTable[0x17F] = { 2, "BIT 7, A", {} }; // BIT 7, A
    
    // bit reset instructions
    // RES 0
    DescriptionTable[0x180] = { 2, "RES 0, B", {} }; // RES 0, B
    DescriptionTable[0x181] = { 2, "RES 0, C", {} }; // RES 0, C
    DescriptionTable[0x182] = { 2, "RES 0, D", {} }; // RES 0, D
    DescriptionTable[0x183] = { 2, "RES 0, E", {} }; // RES 0, E
    DescriptionTable[0x184] = { 2, "RES 0, H", {} }; // RES 0, H
    DescriptionTable[0x185] = { 2, "RES 0, L", {} }; // RES 0, L
    DescriptionTable[0x186] = { 2, "RES 0, (HL)", {} }; // RES 0, (HL)
    DescriptionTable[0x187] = { 2, "RES 0, A", {} }; // RES 0, A
    // RES 1
    DescriptionTable[0x188] = { 2, "RES 1, B", {} }; // RES 1, B
    DescriptionTable[0x189] = { 2, "RES 1, C", {} }; // RES 1, C
    DescriptionTable[0x18A] = { 2, "RES 1, D", {} }; // RES 1, D
    DescriptionTable[0x18B] = { 2, "RES 1, E", {} }; // RES 1, E
    DescriptionTable[0x18C] = { 2, "RES 1, H", {} }; // RES 1, H
    DescriptionTable[0x18D] = { 2, "RES 1, L", {} }; // RES 1, L
    DescriptionTable[0x18E] = { 2, "RES 1, (HL)", {} }; // RES 1, (HL)
    DescriptionTable[0x18F] = { 2, "RES 1, A", {} }; // RES 1, A
    // RES 2
    DescriptionTable[0x190] = { 2, "RES 2, B", {} }; // RES 2, B
    DescriptionTable[0x191] = { 2, "RES 2, C", {} }; // RES 2, C
    DescriptionTable[0x192] = { 2, "RES 2, D", {} }; // RES 2, D
    DescriptionTable[0x193] = { 2, "RES 2, E", {} }; // RES 2, E
    DescriptionTable[0x194] = { 2, "RES 2, H", {} }; // RES 2, H
    DescriptionTable[0x195] = { 2, "RES 2, L", {} }; // RES 2, L
    DescriptionTable[0x196] = { 2, "RES 2, (HL)", {} }; // RES 2, (HL)
    DescriptionTable[0x197] = { 2, "RES 2, A", {} }; // RES 2, A
    // RES 3
    DescriptionTable[0x198] = { 2, "RES 3, B", {} }; // RES 3, B
    DescriptionTable[0x199] = { 2, "RES 3, C", {} }; // RES 3, C
    DescriptionTable[0x19A] = { 2, "RES 3, D", {} }; // RES 3, D
    DescriptionTable[0x19B] = { 2, "RES 3, E", {} }; // RES 3, E
    DescriptionTable[0x19C] = { 2, "RES 3, H", {} }; // RES 3, H
    DescriptionTable[0x19D] = { 2, "RES 3, L", {} }; // RES 3, L
    DescriptionTable[0x19E] = { 2, "RES 3, (HL)", {} }; // RES 3, (HL)
    DescriptionTable[0x19F] = { 2, "RES 3, A", {} }; // RES 3, A
    // RES 4
    DescriptionTable[0x1A0] = { 2, "RES 4, B", {} }; // RES 4, B
    DescriptionTable[0x1A1] = { 2, "RES 4, C", {} }; // RES 4, C
    DescriptionTable[0x1A2] = { 2, "RES 4, D", {} }; // RES 4, D
    DescriptionTable[0x1A3] = { 2, "RES 4, E", {} }; // RES 4, E
    DescriptionTable[0x1A4] = { 2, "RES 4, H", {} }; // RES 4, H
    DescriptionTable[0x1A5] = { 2, "RES 4, L", {} }; // RES 4, L
    DescriptionTable[0x1A6] = { 2, "RES 4, (HL)", {} }; // RES 4, (HL)
    DescriptionTable[0x1A7] = { 2, "RES 4, A", {} }; // RES 4, A
    // RES 5
    DescriptionTable[0x1A8] = { 2, "RES 5, B", {} }; // RES 5, B
    DescriptionTable[0x1A9] = { 2, "RES 5, C", {} }; // RES 5, C
    DescriptionTable[0x1AA] = { 2, "RES 5, D", {} }; // RES 5, D
    DescriptionTable[0x1AB] = { 2, "RES 5, E", {} }; // RES 5, E
    DescriptionTable[0x1AC] = { 2, "RES 5, H", {} }; // RES 5, H
    DescriptionTable[0x1AD] = { 2, "RES 5, L", {} }; // RES 5, L
    DescriptionTable[0x1AE] = { 2, "RES 5, (HL)", {} }; // RES 5, (HL)
    DescriptionTable[0x1AF] = { 2, "RES 5, A", {} }; // RES 5, A
    // RES 6
    DescriptionTable[0x1B0] = { 2, "RES 6, B", {} }; // RES 6, B
    DescriptionTable[0x1B1] = { 2, "RES 6, C", {} }; // RES 6, C
    DescriptionTable[0x1B2] = { 2, "RES 6, D", {} }; // RES 6, D
    DescriptionTable[0x1B3] = { 2, "RES 6, E", {} }; // RES 6, E
    DescriptionTable[0x1B4] = { 2, "RES 6, H", {} }; // RES 6, H
    DescriptionTable[0x1B5] = { 2, "RES 6, L", {} }; // RES 6, L
    DescriptionTable[0x1B6] = { 2, "RES 6, (HL)", {} }; // RES 6, (HL)
    DescriptionTable[0x1B7] = { 2, "RES 6, A", {} }; // RES 6, A
    // RES 7
    DescriptionTable[0x1B8] = { 2, "RES 7, B", {} }; // RES 7, B
    DescriptionTable[0x1B9] = { 2, "RES 7, C", {} }; // RES 7, C
    DescriptionTable[0x1BA] = { 2, "RES 7, D", {} }; // RES 7, D
    DescriptionTable[0x1BB] = { 2, "RES 7, E", {} }; // RES 7, E
    DescriptionTable[0x1BC] = { 2, "RES 7, H", {} }; // RES 7, H
    DescriptionTable[0x1BD] = { 2, "RES 7, L", {} }; // RES 7, L
    DescriptionTable[0x1BE] = { 2, "RES 7, (HL)", {} }; // RES 7, (HL)
    DescriptionTable[0x1BF] = { 2, "RES 7, A", {} }; // RES 7, A
    
    // bit set instructions
    // SET 0
    DescriptionTable[0x1C0] = { 2, "SET 0, B", {} }; // SET 0, B
    DescriptionTable[0x1C1] = { 2, "SET 0, C", {} }; // SET 0, C
    DescriptionTable[0x1C2] = { 2, "SET 0, D", {} }; // SET 0, D
    DescriptionTable[0x1C3] = { 2, "SET 0, E", {} }; // SET 0, E
    DescriptionTable[0x1C4] = { 2, "SET 0, H", {} }; // SET 0, H
    DescriptionTable[0x1C5] = { 2, "SET 0, L", {} }; // SET 0, L
    DescriptionTable[0x1C6] = { 2, "SET 0, (HL)", {} }; // SET 0, (HL)
    DescriptionTable[0x1C7] = { 2, "SET 0, A", {} }; // SET 0, A
    // SET 1
    DescriptionTable[0x1C8] = { 2, "SET 1, B", {} }; // SET 1, B
    DescriptionTable[0x1C9] = { 2, "SET 1, C", {} }; // SET 1, C
    DescriptionTable[0x1CA] = { 2, "SET 1, D", {} }; // SET 1, D
    DescriptionTable[0x1CB] = { 2, "SET 1, E", {} }; // SET 1, E
    DescriptionTable[0x1CC] = { 2, "SET 1, H", {} }; // SET 1, H
    DescriptionTable[0x1CD] = { 2, "SET 1, L", {} }; // SET 1, L
    DescriptionTable[0x1CE] = { 2, "SET 1, (HL)", {} }; // SET 1, (HL)
    DescriptionTable[0x1CF] = { 2, "SET 1, A", {} }; // SET 1, A
    // SET 2
    DescriptionTable[0x1D0] = { 2, "SET 2, B", {} }; // SET 2, B
    DescriptionTable[0x1D1] = { 2, "SET 2, C", {} }; // SET 2, C
    DescriptionTable[0x1D2] = { 2, "SET 2, D", {} }; // SET 2, D
    DescriptionTable[0x1D3] = { 2, "SET 2, E", {} }; // SET 2, E
    DescriptionTable[0x1D4] = { 2, "SET 2, H", {} }; // SET 2, H
    DescriptionTable[0x1D5] = { 2, "SET 2, L", {} }; // SET 2, L
    DescriptionTable[0x1D6] = { 2, "SET 2, (HL)", {} }; // SET 2, (HL)
    DescriptionTable[0x1D7] = { 2, "SET 2, A", {} }; // SET 2, A
    // SET 3
    DescriptionTable[0x1D8] = { 2, "SET 3, B", {} }; // SET 3, B
    DescriptionTable[0x1D9] = { 2, "SET 3, C", {} }; // SET 3, C
    DescriptionTable[0x1DA] = { 2, "SET 3, D", {} }; // SET 3, D
    DescriptionTable[0x1DB] = { 2, "SET 3, E", {} }; // SET 3, E
    DescriptionTable[0x1DC] = { 2, "SET 3, H", {} }; // SET 3, H
    DescriptionTable[0x1DD] = { 2, "SET 3, L", {} }; // SET 3, L
    DescriptionTable[0x1DE] = { 2, "SET 3, (HL)", {} }; // SET 3, (HL)
    DescriptionTable[0x1DF] = { 2, "SET 3, A", {} }; // SET 3, A
    // SET 4
    DescriptionTable[0x1E0] = { 2, "SET 4, B", {} }; // SET 4, B
    DescriptionTable[0x1E1] = { 2, "SET 4, C", {} }; // SET 4, C
    DescriptionTable[0x1E2] = { 2, "SET 4, D", {} }; // SET 4, D
    DescriptionTable[0x1E3] = { 2, "SET 4, E", {} }; // SET 4, E
    DescriptionTable[0x1E4] = { 2, "SET 4, H", {} }; // SET 4, H
    DescriptionTable[0x1E5] = { 2, "SET 4, L", {} }; // SET 4, L
    DescriptionTable[0x1E6] = { 2, "SET 4, (HL)", {} }; // SET 4, (HL)
    DescriptionTable[0x1E7] = { 2, "SET 4, A", {} }; // SET 4, A
    // SET 5
    DescriptionTable[0x1E8] = { 2, "SET 5, B", {} }; // SET 5, B
    DescriptionTable[0x1E9] = { 2, "SET 5, C", {} }; // SET 5, C
    DescriptionTable[0x1EA] = { 2, "SET 5, D", {} }; // SET 5, D
    DescriptionTable[0x1EB] = { 2, "SET 5, E", {} }; // SET 5, E
    DescriptionTable[0x1EC] = { 2, "SET 5, H", {} }; // SET 5, H
    DescriptionTable[0x1ED] = { 2, "SET 5, L", {} }; // SET 5, L
    DescriptionTable[0x1EE] = { 2, "SET 5, (HL)", {} }; // SET 5, (HL)
    DescriptionTable[0x1EF] = { 2, "SET 5, A", {} }; // SET 5, A
    // SET 6
    DescriptionTable[0x1F0] = { 2, "SET 6, B", {} }; // SET 6, B
    DescriptionTable[0x1F1] = { 2, "SET 6, C", {} }; // SET 6, C
    DescriptionTable[0x1F2] = { 2, "SET 6, D", {} }; // SET 6, D
    DescriptionTable[0x1F3] = { 2, "SET 6, E", {} }; // SET 6, E
    DescriptionTable[0x1F4] = { 2, "SET 6, H", {} }; // SET 6, H
    DescriptionTable[0x1F5] = { 2, "SET 6, L", {} }; // SET 6, L
    DescriptionTable[0x1F6] = { 2, "SET 6, (HL)", {} }; // SET 6, (HL)
    DescriptionTable[0x1F7] = { 2, "SET 6, A", {} }; // SET 6, A
    // SET 7
    DescriptionTable[0x1F8] = { 2, "SET 7, B", {} }; // SET 7, B
    DescriptionTable[0x1F9] = { 2, "SET 7, C", {} }; // SET 7, C
    DescriptionTable[0x1FA] = { 2, "SET 7, D", {} }; // SET 7, D
    DescriptionTable[0x1FB] = { 2, "SET 7, E", {} }; // SET 7, E
    DescriptionTable[0x1FC] = { 2, "SET 7, H", {} }; // SET 7, H
    DescriptionTable[0x1FD] = { 2, "SET 7, L", {} }; // SET 7, L
    DescriptionTable[0x1FE] = { 2, "SET 7, (HL)", {} }; // SET 7, (HL)
    DescriptionTable[0x1FF] = { 2, "SET 7, A", {} }; // SET 7, A
    
    // RLC instructions
    DescriptionTable[0x100] = { 2, "RLC B", {} }; // RLC B
    DescriptionTable[0x101] = { 2, "RLC C", {} }; // RLC C
    DescriptionTable[0x102] = { 2, "RLC D", {} }; // RLC D
    DescriptionTable[0x103] = { 2, "RLC E", {} }; // RLC E
    DescriptionTable[0x104] = { 2, "RLC H", {} }; // RLC H
    DescriptionTable[0x105] = { 2, "RLC L", {} }; // RLC L
    DescriptionTable[0x106] = { 2, "RLC (HL)", {} }; // RLC (HL)
    DescriptionTable[0x107] = { 2, "RLC A", {} }; // RLC A
    
    // RL instructions
    DescriptionTable[0x110] = { 2, "RL B", {} }; // RL B
    DescriptionTable[0x111] = { 2, "RL C", {} }; // RL C
    DescriptionTable[0x112] = { 2, "RL D", {} }; // RL D
    DescriptionTable[0x113] = { 2, "RL E", {} }; // RL E
    DescriptionTable[0x114] = { 2, "RL H", {} }; // RL H
    DescriptionTable[0x115] = { 2, "RL L", {} }; // RL L
    DescriptionTable[0x116] = { 2, "RL (HL)", {} }; // RL (HL)
    DescriptionTable[0x117] = { 2, "RL A", {} }; // RL A
    
    // RRC instructions
    DescriptionTable[0x108] = { 2, "RRC B", {} }; // RRC B
    DescriptionTable[0x109] = { 2, "RRC C", {} }; // RRC C
    DescriptionTable[0x10A] = { 2, "RRC D", {} }; // RRC D
    DescriptionTable[0x10B] = { 2, "RRC E", {} }; // RRC E
    DescriptionTable[0x10C] = { 2, "RRC H", {} }; // RRC H
    DescriptionTable[0x10D] = { 2, "RRC L", {} }; // RRC L
    DescriptionTable[0x10E] = { 2, "RRC (HL)", {} }; // RRC (HL)
    DescriptionTable[0x10F] = { 2, "RRC A", {} }; // RRC A
    
    // RR instructions
    DescriptionTable[0x118] = { 2, "RR B", {} }; // RR B
    DescriptionTable[0x119] = { 2, "RR C", {} }; // RR C
    DescriptionTable[0x11A] = { 2, "RR D", {} }; // RR D
    DescriptionTable[0x11B] = { 2, "RR E", {} }; // RR E
    DescriptionTable[0x11C] = { 2, "RR H", {} }; // RR H
    DescriptionTable[0x11D] = { 2, "RR L", {} }; // RR L
    DescriptionTable[0x11E] = { 2, "RR (HL)", {} }; // RR (HL)
    DescriptionTable[0x11F] = { 2, "RR A", {} }; // RR A
    
    // SLA instructions
    DescriptionTable[0x120] = { 2, "SLA B", {} }; // SLA B
    DescriptionTable[0x121] = { 2, "SLA C", {} }; // SLA C
    DescriptionTable[0x122] = { 2, "SLA D", {} }; // SLA D
    DescriptionTable[0x123] = { 2, "SLA E", {} }; // SLA E
    DescriptionTable[0x124] = { 2, "SLA H", {} }; // SLA H
    DescriptionTable[0x125] = { 2, "SLA L", {} }; // SLA L
    DescriptionTable[0x126] = { 2, "SLA (HL)", {} }; // SLA (HL)
    DescriptionTable[0x127] = { 2, "SLA A", {} }; // SLA A
    
    // SRL instructions
    DescriptionTable[0x138] = { 2, "SRL B", {} }; // SRL B
    DescriptionTable[0x139] = { 2, "SRL C", {} }; // SRL C
    DescriptionTable[0x13A] = { 2, "SRL D", {} }; // SRL D
    DescriptionTable[0x13B] = { 2, "SRL E", {} }; // SRL E
    DescriptionTable[0x13C] = { 2, "SRL H", {} }; // SRL H
    DescriptionTable[0x13D] = { 2, "SRL L", {} }; // SRL L
    DescriptionTable[0x13E] = { 2, "SRL (HL)", {} }; // SRL (HL)
    DescriptionTable[0x13F] = { 2, "SRL A", {} }; // SRL A
    
    // SRA instructions
    DescriptionTable[0x128] = { 2, "SRA B", {} }; // SRA B
    DescriptionTable[0x129] = { 2, "SRA C", {} }; // SRA C
    DescriptionTable[0x12A] = { 2, "SRA D", {} }; // SRA D
    DescriptionTable[0x12B] = { 2, "SRA E", {} }; // SRA E
    DescriptionTable[0x12C] = { 2, "SRA H", {} }; // SRA H
    DescriptionTable[0x12D] = { 2, "SRA L", {} }; // SRA L
    DescriptionTable[0x12E] = { 2, "SRA (HL)", {} }; // SRA (HL)
    DescriptionTable[0x12F] = { 2, "SRA A", {} }; // SRA A
    
    // SWAP instructions
    DescriptionTable[0x130] = { 2, "SWAP B", {} }; // SWAP B
    DescriptionTable[0x131] = { 2, "SWAP C", {} }; // SWAP C
    DescriptionTable[0x132] = { 2, "SWAP D", {} }; // SWAP D
    DescriptionTable[0x133] = { 2, "SWAP E", {} }; // SWAP E
    DescriptionTable[0x134] = { 2, "SWAP H", {} }; // SWAP H
    DescriptionTable[0x135] = { 2, "SWAP L", {} }; // SWAP L
    DescriptionTable[0x136] = { 2, "SWAP (HL)", {} }; // SWAP (HL)
    DescriptionTable[0x137] = { 2, "SWAP A", {} }; // SWAP A
}

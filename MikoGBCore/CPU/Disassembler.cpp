//
//  Disassembler.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "Disassembler.hpp"
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "BitTwiddlingUtil.h"

using namespace std;
using namespace MikoGB;

enum class ArgumentType {
    None                = 0,
    Immediate8          = 1,
    Immediate16         = 2,
    Immediate8Ptr       = 3,
    Immediate8Signed    = 4,
};

struct InstructionDescription {
    uint16_t size = 0;
    string description = "Unrecognized Instruction";
    ArgumentType argumentType = ArgumentType::None;
};
static InstructionDescription *DescriptionTable = nullptr;

static string InsertArgument(string &base, string &arg) {
    size_t pos = base.find('%');
    assert(pos < base.size()); // must be found or there's a transcription error in the table below
    
    string firstPart = base.substr(0, pos);
    string endPart = base.substr(pos+1, base.size());
    
    return firstPart + arg + endPart;
}

static string DescriptionImmediate8(uint8_t arg) {
    ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)arg;
    return oss.str();
}

static string DescriptionImmediate16(uint8_t lo, uint8_t hi) {
    uint16_t val = word16(lo, hi);
    ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)val;
    return oss.str();
}

static string DescriptionImmediate8Signed(uint8_t arg) {
    int8_t signedArg = (int8_t)arg;
    ostringstream oss;
    oss << (int)signedArg;
    return oss.str();
}

static string lookupInstruction(uint16_t pc, const MemoryController::Ptr &mem, uint16_t &outSize) {
    size_t idx = mem->readByte(pc);
    uint16_t offset = 1;
    if (idx == 0xCB) {
        //Z80 Extended instruction set
        //Index in the table is 0x1SS where SS is the extended opcode
        idx = mem->readByte((pc + 1)) | 0x100;
        offset++;
    }
    InstructionDescription &instruction = DescriptionTable[idx];
    outSize = instruction.size;
    
    switch (instruction.argumentType) {
        case ArgumentType::None:
            return instruction.description;
        case ArgumentType::Immediate8: {
            uint8_t immediateArg = mem->readByte(pc + offset);
            string strArg = DescriptionImmediate8(immediateArg);
            return InsertArgument(instruction.description, strArg);
        }
        case ArgumentType::Immediate16: {
            uint8_t arg0 = mem->readByte(pc + offset);
            uint8_t arg1 = mem->readByte(pc + offset + 1);
            string strArg = DescriptionImmediate16(arg0, arg1);
            return InsertArgument(instruction.description, strArg);
        }
        case ArgumentType::Immediate8Ptr: {
            uint8_t immediateArg = mem->readByte(pc + offset);
            string strArg = DescriptionImmediate16(immediateArg, 0xFF);
            return InsertArgument(instruction.description, strArg);
        }
        case ArgumentType::Immediate8Signed: {
            uint8_t immediateArg = mem->readByte(pc + offset);
            string strArg = DescriptionImmediate8Signed(immediateArg);
            return InsertArgument(instruction.description, strArg);
        }
    }
    
    // unreachable
    assert(0);
}

std::vector<DisassembledInstruction> Disassembler::disassembleInstructions(uint16_t pc, int maxCount, const MemoryController::Ptr &mem) const {
    vector<DisassembledInstruction> instructions;
    assert(pc < 0x8000 || (pc >= 0xFF80 && pc < 0xFFFF));
    
    // end if we exceed the boundary of the initial ROM bank
    uint16_t bankBoundary;
    int romBank;
    if (pc >= 0xFF80) {
        bankBoundary = 0xFFFF;
        romBank = -1;
    } else if (pc >= 0x4000) {
        bankBoundary = 0x8000;
        romBank = mem->currentROMBank();
    } else {
        bankBoundary = 0x4000;
        romBank = 0;
    }
    uint16_t currentPC = pc;
    
    for (int i = 0; i < maxCount; ++i) {
        if (currentPC >= bankBoundary) {
            DisassembledInstruction instruction = { romBank, currentPC, "Memory boundary" };
            instructions.push_back(instruction);
            break;
        }
        uint16_t size = 0;
        string description = lookupInstruction(currentPC, mem, size);
        DisassembledInstruction instruction = { romBank, currentPC, description };
        instructions.push_back(instruction);
        if (size == 0) {
            // failed to find the instruction in the table, so stop
            break;
        }
        currentPC += size;
    }
    
    return instructions;
}

std::vector<DisassembledInstruction> Disassembler::precedingDisassembledInstructions(uint16_t pc, int maxCount, const MemoryController::Ptr &mem, const CPUCore::Ptr &cpu) const {
    vector<DisassembledInstruction> instructions;
    assert(pc < 0x8000 || (pc >= 0xFF80 && pc < 0xFFFF));
    
    // end if we exceed the boundary of the initial ROM bank
    int romBank;
    if (pc >= 0xFF80) {
        // no cache for RAM instructions since they're writable and may have changed
        return instructions;
    } else if (pc >= 0x4000) {
        romBank = mem->currentROMBank();
    } else {
        romBank = 0;
    }
    uint16_t size = 0;
    (void)lookupInstruction(pc, mem, size);
    if (size == 0) {
        // The current instruction is unreadable so we can't walk back
        return instructions;
    }
    
    set<KnownInstruction> knownInstructions = cpu->_previousInstructions.uniqueInstructions();
    if (knownInstructions.empty()) {
        return instructions;
    }
    KnownInstruction instructionKey = { romBank, pc, size };
    // find the first item *not less than* the target instruction (aka >= instructionKey)
    // therefore every entry ahead of it is an instruction ahead of instructionKey
    auto cacheIterator = knownInstructions.lower_bound(instructionKey);
    
    // walk back and add instructions as long as the previous instruction is immediately before
    uint16_t currentAddress = pc;
    while (instructions.size() < maxCount) {
        if (cacheIterator == knownInstructions.begin()) {
            // nothing left in front
            break;
        }
        cacheIterator--;
        const KnownInstruction &prev = *cacheIterator;
        if (prev.romBank != romBank) {
            // jumped banks, not preceding
            break;
        }
        if (prev.addr + prev.size != currentAddress) {
            // the previous instruction doesn't directly precede the last
            break;
        }
        // the previous instruction *does* directly precede the last. So add it
        uint16_t size = 0;
        string description = lookupInstruction(prev.addr, mem, size);
        DisassembledInstruction instruction = { romBank, prev.addr, description };
        instructions.push_back(instruction);
        currentAddress = prev.addr;
    }
    
    // we pushed in reverse order, so reverse the list before returning
    std::reverse(instructions.begin(), instructions.end());
    return instructions;
}

std::vector<DisassembledInstruction> Disassembler::lastExecutedInstructions(int maxCount, const MemoryController::Ptr &mem, const CPUCore::Ptr &cpu) const {
    const int currentBank = mem->currentROMBank();
    vector<KnownInstruction> knownInstructions = cpu->_previousInstructions.previousInstructions(maxCount);
    vector<DisassembledInstruction> disassembled;
    for (const auto &prev : knownInstructions) {
        if (prev.romBank != 0 && prev.romBank != currentBank) {
            // The last instruction was in a ROM bank that is no longer in memory
            // Could look this up in theory if needed, would need to pipe a debug function through the memory controller
            break;
        }
        uint16_t size = 0;
        string description = lookupInstruction(prev.addr, mem, size);
        DisassembledInstruction instruction = { prev.romBank, prev.addr, description };
        disassembled.push_back(instruction);
    }
    return disassembled;
}

Disassembler::Disassembler() {
    Disassembler::InitializeDisassemblyTable();
}

void Disassembler::InitializeDisassemblyTable() {
    if (DescriptionTable != nullptr) {
        return; //already initialized
    }
    
    // technically 511 possible codes. 500 are used See comment in CPUInstruction
    DescriptionTable = new InstructionDescription[512]();
    
    DescriptionTable[0x00] = { 1, "NoOp", ArgumentType::None };
    
    // LD dd, nn
    DescriptionTable[0x01] = { 3, "LD BC, %", ArgumentType::Immediate16 }; // LD BC, nn
    DescriptionTable[0x11] = { 3, "LD DE, %", ArgumentType::Immediate16 }; // LD DE, nn
    DescriptionTable[0x21] = { 3, "LD HL, %", ArgumentType::Immediate16 }; // LD HL, nn
    DescriptionTable[0x31] = { 3, "LD SP, %", ArgumentType::Immediate16 }; // LD SP, nn
    
    // LD r, n
    DescriptionTable[0x06] = { 2, "LD B, %", ArgumentType::Immediate8 }; // LD B, n
    DescriptionTable[0x0E] = { 2, "LD C, %", ArgumentType::Immediate8 }; // LD C, n
    DescriptionTable[0x16] = { 2, "LD D, %", ArgumentType::Immediate8 }; // LD D, n
    DescriptionTable[0x1E] = { 2, "LD E, %", ArgumentType::Immediate8 }; // LD E, n
    DescriptionTable[0x26] = { 2, "LD H, %", ArgumentType::Immediate8 }; // LD H, n
    DescriptionTable[0x2E] = { 2, "LD L, %", ArgumentType::Immediate8 }; // LD L, n
    DescriptionTable[0x36] = { 2, "LD (HL), %", ArgumentType::Immediate8 }; // LD (HL), n
    DescriptionTable[0x3E] = { 2, "LD A, %", ArgumentType::Immediate8 }; // LD A, n
    
    // LD B, r
    DescriptionTable[0x40] = { 1, "LD B, B", ArgumentType::None }; // LD B, B -> redundant?
    DescriptionTable[0x41] = { 1, "LD B, C", ArgumentType::None }; // LD B, C
    DescriptionTable[0x42] = { 1, "LD B, D", ArgumentType::None }; // LD B, D
    DescriptionTable[0x43] = { 1, "LD B, E", ArgumentType::None }; // LD B, E
    DescriptionTable[0x44] = { 1, "LD B, H", ArgumentType::None }; // LD B, H
    DescriptionTable[0x45] = { 1, "LD B, L", ArgumentType::None }; // LD B, L
    DescriptionTable[0x46] = { 1, "LD B, (HL)", ArgumentType::None }; // LD B, (HL)
    DescriptionTable[0x47] = { 1, "LD B, A", ArgumentType::None }; // LD B, A
    
    // LD C, r
    DescriptionTable[0x48] = { 1, "LD C, B", ArgumentType::None }; // LD C, B
    DescriptionTable[0x49] = { 1, "LD C, C", ArgumentType::None }; // LD C, C -> redundant?
    DescriptionTable[0x4A] = { 1, "LD C, D", ArgumentType::None }; // LD C, D
    DescriptionTable[0x4B] = { 1, "LD C, E", ArgumentType::None }; // LD C, E
    DescriptionTable[0x4C] = { 1, "LD C, H", ArgumentType::None }; // LD C, H
    DescriptionTable[0x4D] = { 1, "LD C, L", ArgumentType::None }; // LD C, L
    DescriptionTable[0x4E] = { 1, "LD C, (HL)", ArgumentType::None }; // LD C, (HL)
    DescriptionTable[0x4F] = { 1, "LD C, A", ArgumentType::None }; // LD C, A
    
    // LD D, r
    DescriptionTable[0x50] = { 1, "LD D, B", ArgumentType::None }; // LD D, B
    DescriptionTable[0x51] = { 1, "LD D, C", ArgumentType::None }; // LD D, C
    DescriptionTable[0x52] = { 1, "LD D, D", ArgumentType::None }; // LD D, D -> redundant?
    DescriptionTable[0x53] = { 1, "LD D, E", ArgumentType::None }; // LD D, E
    DescriptionTable[0x54] = { 1, "LD D, H", ArgumentType::None }; // LD D, H
    DescriptionTable[0x55] = { 1, "LD D, L", ArgumentType::None }; // LD D, L
    DescriptionTable[0x56] = { 1, "LD D, (HL)", ArgumentType::None }; // LD D, (HL)
    DescriptionTable[0x57] = { 1, "LD D, A", ArgumentType::None }; // LD D, A
    
    // LD E, r
    DescriptionTable[0x58] = { 1, "LD E, B", ArgumentType::None }; // LD E, B
    DescriptionTable[0x59] = { 1, "LD E, C", ArgumentType::None }; // LD E, C
    DescriptionTable[0x5A] = { 1, "LD E, D", ArgumentType::None }; // LD E, D
    DescriptionTable[0x5B] = { 1, "LD E, E", ArgumentType::None }; // LD E, E -> redundant?
    DescriptionTable[0x5C] = { 1, "LD E, H", ArgumentType::None }; // LD E, H
    DescriptionTable[0x5D] = { 1, "LD E, L", ArgumentType::None }; // LD E, L
    DescriptionTable[0x5E] = { 1, "LD E, (HL)", ArgumentType::None }; // LD E, (HL)
    DescriptionTable[0x5F] = { 1, "LD E, A", ArgumentType::None }; // LD E, A
    
    // LD H, r
    DescriptionTable[0x60] = { 1, "LD H, B", ArgumentType::None }; // LD H, B
    DescriptionTable[0x61] = { 1, "LD H, C", ArgumentType::None }; // LD H, C
    DescriptionTable[0x62] = { 1, "LD H, D", ArgumentType::None }; // LD H, D
    DescriptionTable[0x63] = { 1, "LD H, E", ArgumentType::None }; // LD H, E
    DescriptionTable[0x64] = { 1, "LD H, H", ArgumentType::None }; // LD H, H -> redundant?
    DescriptionTable[0x65] = { 1, "LD H, L", ArgumentType::None }; // LD H, L
    DescriptionTable[0x66] = { 1, "LD H, (HL)", ArgumentType::None }; // LD H, (HL)
    DescriptionTable[0x67] = { 1, "LD H, A", ArgumentType::None }; // LD H, A
    
    // LD L, r
    DescriptionTable[0x68] = { 1, "LD L, B", ArgumentType::None }; // LD L, B
    DescriptionTable[0x69] = { 1, "LD L, C", ArgumentType::None }; // LD L, C
    DescriptionTable[0x6A] = { 1, "LD L, D", ArgumentType::None }; // LD L, D
    DescriptionTable[0x6B] = { 1, "LD L, E", ArgumentType::None }; // LD L, E
    DescriptionTable[0x6C] = { 1, "LD L, H", ArgumentType::None }; // LD L, H
    DescriptionTable[0x6D] = { 1, "LD L, L", ArgumentType::None }; // LD L, L -> redundant?
    DescriptionTable[0x6E] = { 1, "LD L, (HL)", ArgumentType::None }; // LD L, (HL)
    DescriptionTable[0x6F] = { 1, "LD L, A", ArgumentType::None }; // LD L, A
    
    // LD (HL), r
    DescriptionTable[0x70] = { 1, "LD (HL), B", ArgumentType::None }; // LD (HL), B
    DescriptionTable[0x71] = { 1, "LD (HL), C", ArgumentType::None }; // LD (HL), C
    DescriptionTable[0x72] = { 1, "LD (HL), D", ArgumentType::None }; // LD (HL), D
    DescriptionTable[0x73] = { 1, "LD (HL), E", ArgumentType::None }; // LD (HL), E
    DescriptionTable[0x74] = { 1, "LD (HL), H", ArgumentType::None }; // LD (HL), H
    DescriptionTable[0x75] = { 1, "LD (HL), L", ArgumentType::None }; // LD (HL), L
    // 0x76 is HALT. LD (HL), (HL) wouldn't do anything
    DescriptionTable[0x77] = { 1, "LD (HL), A", ArgumentType::None }; // LD (HL), A
    
    // LD A, r
    DescriptionTable[0x78] = { 1, "LD A, B", ArgumentType::None }; // LD A, B
    DescriptionTable[0x79] = { 1, "LD A, C", ArgumentType::None }; // LD A, C
    DescriptionTable[0x7A] = { 1, "LD A, D", ArgumentType::None }; // LD A, D
    DescriptionTable[0x7B] = { 1, "LD A, E", ArgumentType::None }; // LD A, E
    DescriptionTable[0x7C] = { 1, "LD A, H", ArgumentType::None }; // LD A, H
    DescriptionTable[0x7D] = { 1, "LD A, L", ArgumentType::None }; // LD A, L
    DescriptionTable[0x7E] = { 1, "LD A, (HL)", ArgumentType::None }; // LD A, (HL)
    DescriptionTable[0x7F] = { 1, "LD A, A", ArgumentType::None }; // LD A, A -> redundant?
    
    // LD with accumulator and other register-pair pointers
    DescriptionTable[0x02] = { 1, "LD (BC), A", ArgumentType::None }; // LD (BC), A
    DescriptionTable[0x12] = { 1, "LD (DE), A", ArgumentType::None }; // LD (DE), A
    DescriptionTable[0x0A] = { 1, "LD A, (BC)", ArgumentType::None }; // LD A, (BC)
    DescriptionTable[0x1A] = { 1, "LD A, (DE)", ArgumentType::None }; // LD A, (DE)
    
    // LD with (C)
    DescriptionTable[0xE2] = { 1, "LD (C), A" }; // LD (C), A
    DescriptionTable[0xF2] = { 1, "LD A, (C)" }; // LD A, (C)
    
    // LD with accumulator and immediate pointers
    DescriptionTable[0xE0] = { 2, "LD (%), A", ArgumentType::Immediate8Ptr }; // LD (n), A
    DescriptionTable[0xEA] = { 3, "LD (%), A", ArgumentType::Immediate16 }; // LD (nn), A
    DescriptionTable[0xF0] = { 2, "LD A, (%)", ArgumentType::Immediate8Ptr }; // LD A, (n)
    DescriptionTable[0xFA] = { 3, "LD A, (%)", ArgumentType::Immediate16 }; // LD A, (nn)
    
    // LD A <-> HL with increment or decrement
    DescriptionTable[0x22] = { 1, "LD (HL+), A", ArgumentType::None }; // LD (HL+), A
    DescriptionTable[0x2A] = { 1, "LD A, (HL+)", ArgumentType::None }; // LD A, (HL+)
    DescriptionTable[0x32] = { 1, "LD (HL-), A", ArgumentType::None }; // LD (HL-), A
    DescriptionTable[0x3A] = { 1, "LD A, (HL-)", ArgumentType::None }; // LD A, (HL-)
    
    // PUSH qq
    DescriptionTable[0xC5] = { 1, "PUSH BC", ArgumentType::None }; // PUSH BC
    DescriptionTable[0xD5] = { 1, "PUSH DE", ArgumentType::None }; // PUSH DE
    DescriptionTable[0xE5] = { 1, "PUSH HL", ArgumentType::None }; // PUSH HL
    DescriptionTable[0xF5] = { 1, "PUSH AF", ArgumentType::None }; // PUSH AF
    
    // POP qq
    DescriptionTable[0xC1] = { 1, "POP BC", ArgumentType::None }; // POP BC
    DescriptionTable[0xD1] = { 1, "POP DE", ArgumentType::None }; // POP DE
    DescriptionTable[0xE1] = { 1, "POP HL", ArgumentType::None }; // POP HL
    DescriptionTable[0xF1] = { 1, "POP AF", ArgumentType::None }; // POP AF
    
    // Stack pointer
    DescriptionTable[0x08] = { 3, "LD (%), SP", ArgumentType::None }; // LD (nn), SP
    DescriptionTable[0xF8] = { 2, "LDHL SP, %", ArgumentType::Immediate8Signed }; // LDHL SP, e
    DescriptionTable[0xF9] = { 1, "LD SP, HL", ArgumentType::None }; // LD SP, HL
    
    // Jump Instructions (relative)
    DescriptionTable[0x18] = { 2, "JR %", ArgumentType::Immediate8Signed }; // JR e
    DescriptionTable[0x20] = { 2, "JR NZ, %", ArgumentType::Immediate8Signed }; // JR NZ, e
    DescriptionTable[0x28] = { 2, "JR Z, %", ArgumentType::Immediate8Signed }; // JR Z, e
    DescriptionTable[0x30] = { 2, "JR NC, %", ArgumentType::Immediate8Signed }; // JR NC, e
    DescriptionTable[0x38] = { 2, "JR C, %", ArgumentType::Immediate8Signed }; // JR C, e
    
    // Jump Instructions (absolute)
    DescriptionTable[0xC2] = { 3, "JP NZ, %", ArgumentType::Immediate16 }; // JP NZ, nn
    DescriptionTable[0xC3] = { 3, "JP %", ArgumentType::Immediate16 }; // JP nn
    DescriptionTable[0xCA] = { 3, "JP Z, %", ArgumentType::Immediate16 }; // JP Z, nn
    DescriptionTable[0xD2] = { 3, "JP NC, %", ArgumentType::Immediate16 }; // JP NC, nn
    DescriptionTable[0xDA] = { 3, "JP C, %", ArgumentType::Immediate16 }; // JP C, nn
    DescriptionTable[0xE9] = { 1, "JP (HL)", ArgumentType::None }; // JP (HL)
    
    // 8-bit Add instructions
    DescriptionTable[0x80] = { 1, "ADD A, B", ArgumentType::None }; // ADD A, B
    DescriptionTable[0x81] = { 1, "ADD A, C", ArgumentType::None }; // ADD A, C
    DescriptionTable[0x82] = { 1, "ADD A, D", ArgumentType::None }; // ADD A, D
    DescriptionTable[0x83] = { 1, "ADD A, E", ArgumentType::None }; // ADD A, E
    DescriptionTable[0x84] = { 1, "ADD A, H", ArgumentType::None }; // ADD A, H
    DescriptionTable[0x85] = { 1, "ADD A, L", ArgumentType::None }; // ADD A, L
    DescriptionTable[0x86] = { 1, "ADD A, (HL)", ArgumentType::None }; // ADD A, (HL)
    DescriptionTable[0x87] = { 1, "ADD A, A", ArgumentType::None }; // ADD A, A
    DescriptionTable[0xC6] = { 2, "ADD A, %", ArgumentType::Immediate8 }; // ADD A, n
    
    // 8-bit Add with carry instructions
    DescriptionTable[0x88] = { 1, "ADC A, B", ArgumentType::None }; // ADC A, B
    DescriptionTable[0x89] = { 1, "ADC A, C", ArgumentType::None }; // ADC A, C
    DescriptionTable[0x8A] = { 1, "ADC A, D", ArgumentType::None }; // ADC A, D
    DescriptionTable[0x8B] = { 1, "ADC A, E", ArgumentType::None }; // ADC A, E
    DescriptionTable[0x8C] = { 1, "ADC A, H", ArgumentType::None }; // ADC A, H
    DescriptionTable[0x8D] = { 1, "ADC A, L", ArgumentType::None }; // ADC A, L
    DescriptionTable[0x8E] = { 1, "ADC A, (HL)", ArgumentType::None }; // ADC A, (HL)
    DescriptionTable[0x8F] = { 1, "ADC A, A", ArgumentType::None }; // ADC A, A
    DescriptionTable[0xCE] = { 2, "ADC A, %", ArgumentType::Immediate8 }; // ADC A, n
    
    // 8-bit Sub instructions
    DescriptionTable[0x90] = { 1, "SUB A, B", ArgumentType::None }; // SUB A, B
    DescriptionTable[0x91] = { 1, "SUB A, C", ArgumentType::None }; // SUB A, C
    DescriptionTable[0x92] = { 1, "SUB A, D", ArgumentType::None }; // SUB A, D
    DescriptionTable[0x93] = { 1, "SUB A, E", ArgumentType::None }; // SUB A, E
    DescriptionTable[0x94] = { 1, "SUB A, H", ArgumentType::None }; // SUB A, H
    DescriptionTable[0x95] = { 1, "SUB A, L", ArgumentType::None }; // SUB A, L
    DescriptionTable[0x96] = { 1, "SUB A, (HL)", ArgumentType::None }; // SUB A, (HL)
    DescriptionTable[0x97] = { 1, "SUB A, A", ArgumentType::None }; // SUB A, A
    DescriptionTable[0xD6] = { 2, "SUB A, %", ArgumentType::Immediate8 }; // SUB A, n
    
    // 8-bit Sub with carry instructions
    DescriptionTable[0x98] = { 1, "SBC A, B", ArgumentType::None }; // SBC A, B
    DescriptionTable[0x99] = { 1, "SBC A, C", ArgumentType::None }; // SBC A, C
    DescriptionTable[0x9A] = { 1, "SBC A, D", ArgumentType::None }; // SBC A, D
    DescriptionTable[0x9B] = { 1, "SBC A, E", ArgumentType::None }; // SBC A, E
    DescriptionTable[0x9C] = { 1, "SBC A, H", ArgumentType::None }; // SBC A, H
    DescriptionTable[0x9D] = { 1, "SBC A, L", ArgumentType::None }; // SBC A, L
    DescriptionTable[0x9E] = { 1, "SBC A, (HL)", ArgumentType::None }; // SBC A, (HL)
    DescriptionTable[0x9F] = { 1, "SBC A, A", ArgumentType::None }; // SBC A, A
    DescriptionTable[0xDE] = { 2, "SBC A, %", ArgumentType::Immediate8 }; // SBC A, n
    
    // 8-bit AND instructions
    DescriptionTable[0xA0] = { 1, "AND B", ArgumentType::None }; // AND B
    DescriptionTable[0xA1] = { 1, "AND C", ArgumentType::None }; // AND C
    DescriptionTable[0xA2] = { 1, "AND D", ArgumentType::None }; // AND D
    DescriptionTable[0xA3] = { 1, "AND E", ArgumentType::None }; // AND E
    DescriptionTable[0xA4] = { 1, "AND H", ArgumentType::None }; // AND H
    DescriptionTable[0xA5] = { 1, "AND L", ArgumentType::None }; // AND L
    DescriptionTable[0xA6] = { 1, "AND (HL)", ArgumentType::None }; // AND (HL)
    DescriptionTable[0xA7] = { 1, "AND A", ArgumentType::None }; // AND A
    DescriptionTable[0xE6] = { 2, "AND %", ArgumentType::Immediate8 }; // AND n
    
    // 8-bit OR instructions
    DescriptionTable[0xB0] = { 1, "OR B", ArgumentType::None }; // OR B
    DescriptionTable[0xB1] = { 1, "OR C", ArgumentType::None }; // OR C
    DescriptionTable[0xB2] = { 1, "OR D", ArgumentType::None }; // OR D
    DescriptionTable[0xB3] = { 1, "OR E", ArgumentType::None }; // OR E
    DescriptionTable[0xB4] = { 1, "OR H", ArgumentType::None }; // OR H
    DescriptionTable[0xB5] = { 1, "OR L", ArgumentType::None }; // OR L
    DescriptionTable[0xB6] = { 1, "OR (HL)", ArgumentType::None }; // OR (HL)
    DescriptionTable[0xB7] = { 1, "OR A", ArgumentType::None }; // OR A
    DescriptionTable[0xF6] = { 2, "OR %", ArgumentType::Immediate8 }; // OR n
    
    // 8-bit XOR Instructions
    DescriptionTable[0xA8] = { 1, "XOR B", ArgumentType::None }; // XOR B
    DescriptionTable[0xA9] = { 1, "XOR C", ArgumentType::None }; // XOR C
    DescriptionTable[0xAA] = { 1, "XOR D", ArgumentType::None }; // XOR D
    DescriptionTable[0xAB] = { 1, "XOR E", ArgumentType::None }; // XOR E
    DescriptionTable[0xAC] = { 1, "XOR H", ArgumentType::None }; // XOR H
    DescriptionTable[0xAD] = { 1, "XOR L", ArgumentType::None }; // XOR L
    DescriptionTable[0xAE] = { 1, "XOR (HL)", ArgumentType::None }; // XOR (HL)
    DescriptionTable[0xAF] = { 1, "XOR A", ArgumentType::None }; // XOR A
    DescriptionTable[0xEE] = { 2, "XOR %", ArgumentType::Immediate8 }; // XOR n
    
    // 8-bit CP Instructions
    DescriptionTable[0xB8] = { 1, "CP B", ArgumentType::None }; // CP B
    DescriptionTable[0xB9] = { 1, "CP C", ArgumentType::None }; // CP C
    DescriptionTable[0xBA] = { 1, "CP D", ArgumentType::None }; // CP D
    DescriptionTable[0xBB] = { 1, "CP E", ArgumentType::None }; // CP E
    DescriptionTable[0xBC] = { 1, "CP H", ArgumentType::None }; // CP H
    DescriptionTable[0xBD] = { 1, "CP L", ArgumentType::None }; // CP L
    DescriptionTable[0xBE] = { 1, "CP (HL)", ArgumentType::None }; // CP (HL)
    DescriptionTable[0xBF] = { 1, "CP A", ArgumentType::None }; // CP A
    DescriptionTable[0xFE] = { 2, "CP %", ArgumentType::Immediate8 }; // CP n
    
    // 8-bit INC instructions
    DescriptionTable[0x04] = { 1, "INC B", ArgumentType::None }; // INC B
    DescriptionTable[0x0C] = { 1, "INC C", ArgumentType::None }; // INC C
    DescriptionTable[0x14] = { 1, "INC D", ArgumentType::None }; // INC D
    DescriptionTable[0x1C] = { 1, "INC E", ArgumentType::None }; // INC E
    DescriptionTable[0x24] = { 1, "INC H", ArgumentType::None }; // INC H
    DescriptionTable[0x2C] = { 1, "INC L", ArgumentType::None }; // INC L
    DescriptionTable[0x34] = { 1, "INC (HL)", ArgumentType::None }; // INC (HL)
    DescriptionTable[0x3C] = { 1, "INC A", ArgumentType::None }; // INC A
    
    // 8-bit DEC instructions
    DescriptionTable[0x05] = { 1, "DEC B", ArgumentType::None }; // DEC B
    DescriptionTable[0x0D] = { 1, "DEC C", ArgumentType::None }; // DEC C
    DescriptionTable[0x15] = { 1, "DEC D", ArgumentType::None }; // DEC D
    DescriptionTable[0x1D] = { 1, "DEC E", ArgumentType::None }; // DEC E
    DescriptionTable[0x25] = { 1, "DEC H", ArgumentType::None }; // DEC H
    DescriptionTable[0x2D] = { 1, "DEC L", ArgumentType::None }; // DEC L
    DescriptionTable[0x35] = { 1, "DEC (HL)", ArgumentType::None }; // DEC (HL)
    DescriptionTable[0x3D] = { 1, "DEC A", ArgumentType::None }; // DEC A
    
    // 16-bit ADD instructions
    DescriptionTable[0x09] = { 1, "ADD HL, BC", ArgumentType::None }; // ADD HL, BC
    DescriptionTable[0x19] = { 1, "ADD HL, DE", ArgumentType::None }; // ADD HL, DE
    DescriptionTable[0x29] = { 1, "ADD HL, HL", ArgumentType::None }; // ADD HL, HL
    DescriptionTable[0x39] = { 1, "ADD HL, SP", ArgumentType::None }; // ADD HL, SP
    DescriptionTable[0xE8] = { 2, "ADD SP, %", ArgumentType::Immediate8Signed }; // ADD SP, e
    
    // 16-bit INC and DEC instructions
    DescriptionTable[0x03] = { 1, "INC BC", ArgumentType::None }; // INC BC
    DescriptionTable[0x13] = { 1, "INC DE", ArgumentType::None }; // INC DE
    DescriptionTable[0x23] = { 1, "INC HL", ArgumentType::None }; // INC HL
    DescriptionTable[0x33] = { 1, "INC SP", ArgumentType::None }; // INC SP
    DescriptionTable[0x0B] = { 1, "DEC BC", ArgumentType::None }; // DEC BC
    DescriptionTable[0x1B] = { 1, "DEC DE", ArgumentType::None }; // DEC DE
    DescriptionTable[0x2B] = { 1, "DEC HL", ArgumentType::None }; // DEC HL
    DescriptionTable[0x3B] = { 1, "DEC SP", ArgumentType::None }; // DEC SP
    
    // CALL and RET instructions
    DescriptionTable[0xC0] = { 1, "RET NZ", ArgumentType::None }; // RET NZ
    DescriptionTable[0xC4] = { 3, "CALL NZ %", ArgumentType::Immediate16 }; // CALL NZ nn
    DescriptionTable[0xC8] = { 1, "RET Z", ArgumentType::None }; // RET Z
    DescriptionTable[0xC9] = { 1, "RET", ArgumentType::None }; // RET
    DescriptionTable[0xCC] = { 3, "CALL Z %", ArgumentType::Immediate16 }; // CALL Z nn
    DescriptionTable[0xCD] = { 3, "CALL %", ArgumentType::Immediate16 }; // CALL nn
    DescriptionTable[0xD0] = { 1, "RET NC" }; // RET NC
    DescriptionTable[0xD4] = { 3, "CALL NC %", ArgumentType::Immediate16 }; // CALL NC nn
    DescriptionTable[0xD8] = { 1, "RET C", ArgumentType::None }; // RET C
    DescriptionTable[0xD9] = { 1, "RETI", ArgumentType::None }; // RETI
    DescriptionTable[0xDC] = { 3, "CALL C %", ArgumentType::Immediate16 }; // CALL C nn
    
    // RST instruction. A bit of a weird one
    DescriptionTable[0xC7] = { 1, "RST 0", ArgumentType::None }; // RST 0
    DescriptionTable[0xCF] = { 1, "RST 1", ArgumentType::None }; // RST 1
    DescriptionTable[0xD7] = { 1, "RST 2", ArgumentType::None }; // RST 2
    DescriptionTable[0xDF] = { 1, "RST 3", ArgumentType::None }; // RST 3
    DescriptionTable[0xE7] = { 1, "RST 4", ArgumentType::None }; // RST 4
    DescriptionTable[0xEF] = { 1, "RST 5", ArgumentType::None }; // RST 5
    DescriptionTable[0xF7] = { 1, "RST 6", ArgumentType::None }; // RST 6
    DescriptionTable[0xFF] = { 1, "RST 7", ArgumentType::None }; // RST 7
    
    // Rotate instructions
    DescriptionTable[0x07] = { 1, "RLCA", ArgumentType::None }; // RLCA
    DescriptionTable[0x17] = { 1, "RLA", ArgumentType::None }; // RLA
    DescriptionTable[0x0F] = { 1, "RRCA", ArgumentType::None }; // RRCA
    DescriptionTable[0x1F] = { 1, "RRA", ArgumentType::None }; // RRA
    
    // Special instructions
    DescriptionTable[0x27] = { 1, "DAA", ArgumentType::None }; // DAA
    DescriptionTable[0x2F] = { 1, "CPL", ArgumentType::None }; // CPL
    DescriptionTable[0x37] = { 1, "SCF", ArgumentType::None }; // SCF
    DescriptionTable[0x3F] = { 1, "CCF", ArgumentType::None }; // CCF
    DescriptionTable[0xF3] = { 1, "DI", ArgumentType::None }; // DI
    DescriptionTable[0xFB] = { 1, "EI", ArgumentType::None }; // EI
    DescriptionTable[0x76] = { 1, "HALT", ArgumentType::None }; // HALT
    DescriptionTable[0x10] = { 2, "STOP", ArgumentType::None }; // STOP. Technically 2 bytes, the second is expected to be 0x00
    
    // =====================================
    // Extended Opcodes, prefixed with 0xCB
    // =====================================
    
    // bit read instructions
    // BIT 0
    DescriptionTable[0x140] = { 2, "BIT 0, B", ArgumentType::None }; // BIT 0, B
    DescriptionTable[0x141] = { 2, "BIT 0, C", ArgumentType::None }; // BIT 0, C
    DescriptionTable[0x142] = { 2, "BIT 0, D", ArgumentType::None }; // BIT 0, D
    DescriptionTable[0x143] = { 2, "BIT 0, E", ArgumentType::None }; // BIT 0, E
    DescriptionTable[0x144] = { 2, "BIT 0, H", ArgumentType::None }; // BIT 0, H
    DescriptionTable[0x145] = { 2, "BIT 0, L", ArgumentType::None }; // BIT 0, L
    DescriptionTable[0x146] = { 2, "BIT 0, (HL)", ArgumentType::None }; // BIT 0, (HL)
    DescriptionTable[0x147] = { 2, "BIT 0, A", ArgumentType::None }; // BIT 0, A
    // BIT 1
    DescriptionTable[0x148] = { 2, "BIT 1, B", ArgumentType::None }; // BIT 1, B
    DescriptionTable[0x149] = { 2, "BIT 1, C", ArgumentType::None }; // BIT 1, C
    DescriptionTable[0x14A] = { 2, "BIT 1, D", ArgumentType::None }; // BIT 1, D
    DescriptionTable[0x14B] = { 2, "BIT 1, E", ArgumentType::None }; // BIT 1, E
    DescriptionTable[0x14C] = { 2, "BIT 1, H", ArgumentType::None }; // BIT 1, H
    DescriptionTable[0x14D] = { 2, "BIT 1, L", ArgumentType::None }; // BIT 1, L
    DescriptionTable[0x14E] = { 2, "BIT 1, (HL)", ArgumentType::None }; // BIT 1, (HL)
    DescriptionTable[0x14F] = { 2, "BIT 1, A", ArgumentType::None }; // BIT 1, A
    // BIT 2
    DescriptionTable[0x150] = { 2, "BIT 2, B", ArgumentType::None }; // BIT 2, B
    DescriptionTable[0x151] = { 2, "BIT 2, C", ArgumentType::None }; // BIT 2, C
    DescriptionTable[0x152] = { 2, "BIT 2, D", ArgumentType::None }; // BIT 2, D
    DescriptionTable[0x153] = { 2, "BIT 2, E", ArgumentType::None }; // BIT 2, E
    DescriptionTable[0x154] = { 2, "BIT 2, H", ArgumentType::None }; // BIT 2, H
    DescriptionTable[0x155] = { 2, "BIT 2, L", ArgumentType::None }; // BIT 2, L
    DescriptionTable[0x156] = { 2, "BIT 2, (HL)", ArgumentType::None }; // BIT 2, (HL)
    DescriptionTable[0x157] = { 2, "BIT 2, A", ArgumentType::None }; // BIT 2, A
    // BIT 3
    DescriptionTable[0x158] = { 2, "BIT 3, B", ArgumentType::None }; // BIT 3, B
    DescriptionTable[0x159] = { 2, "BIT 3, C", ArgumentType::None }; // BIT 3, C
    DescriptionTable[0x15A] = { 2, "BIT 3, D", ArgumentType::None }; // BIT 3, D
    DescriptionTable[0x15B] = { 2, "BIT 3, E", ArgumentType::None }; // BIT 3, E
    DescriptionTable[0x15C] = { 2, "BIT 3, H", ArgumentType::None }; // BIT 3, H
    DescriptionTable[0x15D] = { 2, "BIT 3, L", ArgumentType::None }; // BIT 3, L
    DescriptionTable[0x15E] = { 2, "BIT 3, (HL)", ArgumentType::None }; // BIT 3, (HL)
    DescriptionTable[0x15F] = { 2, "BIT 3, A", ArgumentType::None }; // BIT 3, A
    // BIT 4
    DescriptionTable[0x160] = { 2, "BIT 4, B", ArgumentType::None }; // BIT 4, B
    DescriptionTable[0x161] = { 2, "BIT 4, C", ArgumentType::None }; // BIT 4, C
    DescriptionTable[0x162] = { 2, "BIT 4, D", ArgumentType::None }; // BIT 4, D
    DescriptionTable[0x163] = { 2, "BIT 4, E", ArgumentType::None }; // BIT 4, E
    DescriptionTable[0x164] = { 2, "BIT 4, H", ArgumentType::None }; // BIT 4, H
    DescriptionTable[0x165] = { 2, "BIT 4, L", ArgumentType::None }; // BIT 4, L
    DescriptionTable[0x166] = { 2, "BIT 4, (HL)", ArgumentType::None }; // BIT 4, (HL)
    DescriptionTable[0x167] = { 2, "BIT 4, A", ArgumentType::None }; // BIT 4, A
    // BIT 5
    DescriptionTable[0x168] = { 2, "BIT 5, B", ArgumentType::None }; // BIT 5, B
    DescriptionTable[0x169] = { 2, "BIT 5, C", ArgumentType::None }; // BIT 5, C
    DescriptionTable[0x16A] = { 2, "BIT 5, D", ArgumentType::None }; // BIT 5, D
    DescriptionTable[0x16B] = { 2, "BIT 5, E", ArgumentType::None }; // BIT 5, E
    DescriptionTable[0x16C] = { 2, "BIT 5, H", ArgumentType::None }; // BIT 5, H
    DescriptionTable[0x16D] = { 2, "BIT 5, L", ArgumentType::None }; // BIT 5, L
    DescriptionTable[0x16E] = { 2, "BIT 5, (HL)", ArgumentType::None }; // BIT 5, (HL)
    DescriptionTable[0x16F] = { 2, "BIT 5, A", ArgumentType::None }; // BIT 5, A
    // BIT 6
    DescriptionTable[0x170] = { 2, "BIT 6, B", ArgumentType::None }; // BIT 6, B
    DescriptionTable[0x171] = { 2, "BIT 6, C", ArgumentType::None }; // BIT 6, C
    DescriptionTable[0x172] = { 2, "BIT 6, D", ArgumentType::None }; // BIT 6, D
    DescriptionTable[0x173] = { 2, "BIT 6, E", ArgumentType::None }; // BIT 6, E
    DescriptionTable[0x174] = { 2, "BIT 6, H", ArgumentType::None }; // BIT 6, H
    DescriptionTable[0x175] = { 2, "BIT 6, L", ArgumentType::None }; // BIT 6, L
    DescriptionTable[0x176] = { 2, "BIT 6, (HL)", ArgumentType::None }; // BIT 6, (HL)
    DescriptionTable[0x177] = { 2, "BIT 6, A", ArgumentType::None }; // BIT 6, A
    // BIT 7
    DescriptionTable[0x178] = { 2, "BIT 7, B", ArgumentType::None }; // BIT 7, B
    DescriptionTable[0x179] = { 2, "BIT 7, C", ArgumentType::None }; // BIT 7, C
    DescriptionTable[0x17A] = { 2, "BIT 7, D", ArgumentType::None }; // BIT 7, D
    DescriptionTable[0x17B] = { 2, "BIT 7, E", ArgumentType::None }; // BIT 7, E
    DescriptionTable[0x17C] = { 2, "BIT 7, H", ArgumentType::None }; // BIT 7, H
    DescriptionTable[0x17D] = { 2, "BIT 7, L", ArgumentType::None }; // BIT 7, L
    DescriptionTable[0x17E] = { 2, "BIT 7, (HL)", ArgumentType::None }; // BIT 7, (HL)
    DescriptionTable[0x17F] = { 2, "BIT 7, A", ArgumentType::None }; // BIT 7, A
    
    // bit reset instructions
    // RES 0
    DescriptionTable[0x180] = { 2, "RES 0, B", ArgumentType::None }; // RES 0, B
    DescriptionTable[0x181] = { 2, "RES 0, C", ArgumentType::None }; // RES 0, C
    DescriptionTable[0x182] = { 2, "RES 0, D", ArgumentType::None }; // RES 0, D
    DescriptionTable[0x183] = { 2, "RES 0, E", ArgumentType::None }; // RES 0, E
    DescriptionTable[0x184] = { 2, "RES 0, H", ArgumentType::None }; // RES 0, H
    DescriptionTable[0x185] = { 2, "RES 0, L", ArgumentType::None }; // RES 0, L
    DescriptionTable[0x186] = { 2, "RES 0, (HL)", ArgumentType::None }; // RES 0, (HL)
    DescriptionTable[0x187] = { 2, "RES 0, A", ArgumentType::None }; // RES 0, A
    // RES 1
    DescriptionTable[0x188] = { 2, "RES 1, B", ArgumentType::None }; // RES 1, B
    DescriptionTable[0x189] = { 2, "RES 1, C", ArgumentType::None }; // RES 1, C
    DescriptionTable[0x18A] = { 2, "RES 1, D", ArgumentType::None }; // RES 1, D
    DescriptionTable[0x18B] = { 2, "RES 1, E", ArgumentType::None }; // RES 1, E
    DescriptionTable[0x18C] = { 2, "RES 1, H", ArgumentType::None }; // RES 1, H
    DescriptionTable[0x18D] = { 2, "RES 1, L", ArgumentType::None }; // RES 1, L
    DescriptionTable[0x18E] = { 2, "RES 1, (HL)", ArgumentType::None }; // RES 1, (HL)
    DescriptionTable[0x18F] = { 2, "RES 1, A", ArgumentType::None }; // RES 1, A
    // RES 2
    DescriptionTable[0x190] = { 2, "RES 2, B", ArgumentType::None }; // RES 2, B
    DescriptionTable[0x191] = { 2, "RES 2, C", ArgumentType::None }; // RES 2, C
    DescriptionTable[0x192] = { 2, "RES 2, D", ArgumentType::None }; // RES 2, D
    DescriptionTable[0x193] = { 2, "RES 2, E", ArgumentType::None }; // RES 2, E
    DescriptionTable[0x194] = { 2, "RES 2, H", ArgumentType::None }; // RES 2, H
    DescriptionTable[0x195] = { 2, "RES 2, L", ArgumentType::None }; // RES 2, L
    DescriptionTable[0x196] = { 2, "RES 2, (HL)", ArgumentType::None }; // RES 2, (HL)
    DescriptionTable[0x197] = { 2, "RES 2, A", ArgumentType::None }; // RES 2, A
    // RES 3
    DescriptionTable[0x198] = { 2, "RES 3, B", ArgumentType::None }; // RES 3, B
    DescriptionTable[0x199] = { 2, "RES 3, C", ArgumentType::None }; // RES 3, C
    DescriptionTable[0x19A] = { 2, "RES 3, D", ArgumentType::None }; // RES 3, D
    DescriptionTable[0x19B] = { 2, "RES 3, E", ArgumentType::None }; // RES 3, E
    DescriptionTable[0x19C] = { 2, "RES 3, H", ArgumentType::None }; // RES 3, H
    DescriptionTable[0x19D] = { 2, "RES 3, L", ArgumentType::None }; // RES 3, L
    DescriptionTable[0x19E] = { 2, "RES 3, (HL)", ArgumentType::None }; // RES 3, (HL)
    DescriptionTable[0x19F] = { 2, "RES 3, A", ArgumentType::None }; // RES 3, A
    // RES 4
    DescriptionTable[0x1A0] = { 2, "RES 4, B", ArgumentType::None }; // RES 4, B
    DescriptionTable[0x1A1] = { 2, "RES 4, C", ArgumentType::None }; // RES 4, C
    DescriptionTable[0x1A2] = { 2, "RES 4, D", ArgumentType::None }; // RES 4, D
    DescriptionTable[0x1A3] = { 2, "RES 4, E", ArgumentType::None }; // RES 4, E
    DescriptionTable[0x1A4] = { 2, "RES 4, H", ArgumentType::None }; // RES 4, H
    DescriptionTable[0x1A5] = { 2, "RES 4, L", ArgumentType::None }; // RES 4, L
    DescriptionTable[0x1A6] = { 2, "RES 4, (HL)", ArgumentType::None }; // RES 4, (HL)
    DescriptionTable[0x1A7] = { 2, "RES 4, A", ArgumentType::None }; // RES 4, A
    // RES 5
    DescriptionTable[0x1A8] = { 2, "RES 5, B", ArgumentType::None }; // RES 5, B
    DescriptionTable[0x1A9] = { 2, "RES 5, C", ArgumentType::None }; // RES 5, C
    DescriptionTable[0x1AA] = { 2, "RES 5, D", ArgumentType::None }; // RES 5, D
    DescriptionTable[0x1AB] = { 2, "RES 5, E", ArgumentType::None }; // RES 5, E
    DescriptionTable[0x1AC] = { 2, "RES 5, H", ArgumentType::None }; // RES 5, H
    DescriptionTable[0x1AD] = { 2, "RES 5, L", ArgumentType::None }; // RES 5, L
    DescriptionTable[0x1AE] = { 2, "RES 5, (HL)", ArgumentType::None }; // RES 5, (HL)
    DescriptionTable[0x1AF] = { 2, "RES 5, A", ArgumentType::None }; // RES 5, A
    // RES 6
    DescriptionTable[0x1B0] = { 2, "RES 6, B", ArgumentType::None }; // RES 6, B
    DescriptionTable[0x1B1] = { 2, "RES 6, C", ArgumentType::None }; // RES 6, C
    DescriptionTable[0x1B2] = { 2, "RES 6, D", ArgumentType::None }; // RES 6, D
    DescriptionTable[0x1B3] = { 2, "RES 6, E", ArgumentType::None }; // RES 6, E
    DescriptionTable[0x1B4] = { 2, "RES 6, H", ArgumentType::None }; // RES 6, H
    DescriptionTable[0x1B5] = { 2, "RES 6, L", ArgumentType::None }; // RES 6, L
    DescriptionTable[0x1B6] = { 2, "RES 6, (HL)", ArgumentType::None }; // RES 6, (HL)
    DescriptionTable[0x1B7] = { 2, "RES 6, A", ArgumentType::None }; // RES 6, A
    // RES 7
    DescriptionTable[0x1B8] = { 2, "RES 7, B", ArgumentType::None }; // RES 7, B
    DescriptionTable[0x1B9] = { 2, "RES 7, C", ArgumentType::None }; // RES 7, C
    DescriptionTable[0x1BA] = { 2, "RES 7, D", ArgumentType::None }; // RES 7, D
    DescriptionTable[0x1BB] = { 2, "RES 7, E", ArgumentType::None }; // RES 7, E
    DescriptionTable[0x1BC] = { 2, "RES 7, H", ArgumentType::None }; // RES 7, H
    DescriptionTable[0x1BD] = { 2, "RES 7, L", ArgumentType::None }; // RES 7, L
    DescriptionTable[0x1BE] = { 2, "RES 7, (HL)", ArgumentType::None }; // RES 7, (HL)
    DescriptionTable[0x1BF] = { 2, "RES 7, A", ArgumentType::None }; // RES 7, A
    
    // bit set instructions
    // SET 0
    DescriptionTable[0x1C0] = { 2, "SET 0, B", ArgumentType::None }; // SET 0, B
    DescriptionTable[0x1C1] = { 2, "SET 0, C", ArgumentType::None }; // SET 0, C
    DescriptionTable[0x1C2] = { 2, "SET 0, D", ArgumentType::None }; // SET 0, D
    DescriptionTable[0x1C3] = { 2, "SET 0, E", ArgumentType::None }; // SET 0, E
    DescriptionTable[0x1C4] = { 2, "SET 0, H", ArgumentType::None }; // SET 0, H
    DescriptionTable[0x1C5] = { 2, "SET 0, L", ArgumentType::None }; // SET 0, L
    DescriptionTable[0x1C6] = { 2, "SET 0, (HL)", ArgumentType::None }; // SET 0, (HL)
    DescriptionTable[0x1C7] = { 2, "SET 0, A", ArgumentType::None }; // SET 0, A
    // SET 1
    DescriptionTable[0x1C8] = { 2, "SET 1, B", ArgumentType::None }; // SET 1, B
    DescriptionTable[0x1C9] = { 2, "SET 1, C", ArgumentType::None }; // SET 1, C
    DescriptionTable[0x1CA] = { 2, "SET 1, D", ArgumentType::None }; // SET 1, D
    DescriptionTable[0x1CB] = { 2, "SET 1, E", ArgumentType::None }; // SET 1, E
    DescriptionTable[0x1CC] = { 2, "SET 1, H", ArgumentType::None }; // SET 1, H
    DescriptionTable[0x1CD] = { 2, "SET 1, L", ArgumentType::None }; // SET 1, L
    DescriptionTable[0x1CE] = { 2, "SET 1, (HL)", ArgumentType::None }; // SET 1, (HL)
    DescriptionTable[0x1CF] = { 2, "SET 1, A", ArgumentType::None }; // SET 1, A
    // SET 2
    DescriptionTable[0x1D0] = { 2, "SET 2, B", ArgumentType::None }; // SET 2, B
    DescriptionTable[0x1D1] = { 2, "SET 2, C", ArgumentType::None }; // SET 2, C
    DescriptionTable[0x1D2] = { 2, "SET 2, D", ArgumentType::None }; // SET 2, D
    DescriptionTable[0x1D3] = { 2, "SET 2, E", ArgumentType::None }; // SET 2, E
    DescriptionTable[0x1D4] = { 2, "SET 2, H", ArgumentType::None }; // SET 2, H
    DescriptionTable[0x1D5] = { 2, "SET 2, L", ArgumentType::None }; // SET 2, L
    DescriptionTable[0x1D6] = { 2, "SET 2, (HL)", ArgumentType::None }; // SET 2, (HL)
    DescriptionTable[0x1D7] = { 2, "SET 2, A", ArgumentType::None }; // SET 2, A
    // SET 3
    DescriptionTable[0x1D8] = { 2, "SET 3, B", ArgumentType::None }; // SET 3, B
    DescriptionTable[0x1D9] = { 2, "SET 3, C", ArgumentType::None }; // SET 3, C
    DescriptionTable[0x1DA] = { 2, "SET 3, D", ArgumentType::None }; // SET 3, D
    DescriptionTable[0x1DB] = { 2, "SET 3, E", ArgumentType::None }; // SET 3, E
    DescriptionTable[0x1DC] = { 2, "SET 3, H", ArgumentType::None }; // SET 3, H
    DescriptionTable[0x1DD] = { 2, "SET 3, L", ArgumentType::None }; // SET 3, L
    DescriptionTable[0x1DE] = { 2, "SET 3, (HL)", ArgumentType::None }; // SET 3, (HL)
    DescriptionTable[0x1DF] = { 2, "SET 3, A", ArgumentType::None }; // SET 3, A
    // SET 4
    DescriptionTable[0x1E0] = { 2, "SET 4, B", ArgumentType::None }; // SET 4, B
    DescriptionTable[0x1E1] = { 2, "SET 4, C", ArgumentType::None }; // SET 4, C
    DescriptionTable[0x1E2] = { 2, "SET 4, D", ArgumentType::None }; // SET 4, D
    DescriptionTable[0x1E3] = { 2, "SET 4, E", ArgumentType::None }; // SET 4, E
    DescriptionTable[0x1E4] = { 2, "SET 4, H", ArgumentType::None }; // SET 4, H
    DescriptionTable[0x1E5] = { 2, "SET 4, L", ArgumentType::None }; // SET 4, L
    DescriptionTable[0x1E6] = { 2, "SET 4, (HL)", ArgumentType::None }; // SET 4, (HL)
    DescriptionTable[0x1E7] = { 2, "SET 4, A", ArgumentType::None }; // SET 4, A
    // SET 5
    DescriptionTable[0x1E8] = { 2, "SET 5, B", ArgumentType::None }; // SET 5, B
    DescriptionTable[0x1E9] = { 2, "SET 5, C", ArgumentType::None }; // SET 5, C
    DescriptionTable[0x1EA] = { 2, "SET 5, D", ArgumentType::None }; // SET 5, D
    DescriptionTable[0x1EB] = { 2, "SET 5, E", ArgumentType::None }; // SET 5, E
    DescriptionTable[0x1EC] = { 2, "SET 5, H", ArgumentType::None }; // SET 5, H
    DescriptionTable[0x1ED] = { 2, "SET 5, L", ArgumentType::None }; // SET 5, L
    DescriptionTable[0x1EE] = { 2, "SET 5, (HL)", ArgumentType::None }; // SET 5, (HL)
    DescriptionTable[0x1EF] = { 2, "SET 5, A", ArgumentType::None }; // SET 5, A
    // SET 6
    DescriptionTable[0x1F0] = { 2, "SET 6, B", ArgumentType::None }; // SET 6, B
    DescriptionTable[0x1F1] = { 2, "SET 6, C", ArgumentType::None }; // SET 6, C
    DescriptionTable[0x1F2] = { 2, "SET 6, D", ArgumentType::None }; // SET 6, D
    DescriptionTable[0x1F3] = { 2, "SET 6, E", ArgumentType::None }; // SET 6, E
    DescriptionTable[0x1F4] = { 2, "SET 6, H", ArgumentType::None }; // SET 6, H
    DescriptionTable[0x1F5] = { 2, "SET 6, L", ArgumentType::None }; // SET 6, L
    DescriptionTable[0x1F6] = { 2, "SET 6, (HL)", ArgumentType::None }; // SET 6, (HL)
    DescriptionTable[0x1F7] = { 2, "SET 6, A", ArgumentType::None }; // SET 6, A
    // SET 7
    DescriptionTable[0x1F8] = { 2, "SET 7, B", ArgumentType::None }; // SET 7, B
    DescriptionTable[0x1F9] = { 2, "SET 7, C", ArgumentType::None }; // SET 7, C
    DescriptionTable[0x1FA] = { 2, "SET 7, D", ArgumentType::None }; // SET 7, D
    DescriptionTable[0x1FB] = { 2, "SET 7, E", ArgumentType::None }; // SET 7, E
    DescriptionTable[0x1FC] = { 2, "SET 7, H", ArgumentType::None }; // SET 7, H
    DescriptionTable[0x1FD] = { 2, "SET 7, L", ArgumentType::None }; // SET 7, L
    DescriptionTable[0x1FE] = { 2, "SET 7, (HL)", ArgumentType::None }; // SET 7, (HL)
    DescriptionTable[0x1FF] = { 2, "SET 7, A", ArgumentType::None }; // SET 7, A
    
    // RLC instructions
    DescriptionTable[0x100] = { 2, "RLC B", ArgumentType::None }; // RLC B
    DescriptionTable[0x101] = { 2, "RLC C", ArgumentType::None }; // RLC C
    DescriptionTable[0x102] = { 2, "RLC D", ArgumentType::None }; // RLC D
    DescriptionTable[0x103] = { 2, "RLC E", ArgumentType::None }; // RLC E
    DescriptionTable[0x104] = { 2, "RLC H", ArgumentType::None }; // RLC H
    DescriptionTable[0x105] = { 2, "RLC L", ArgumentType::None }; // RLC L
    DescriptionTable[0x106] = { 2, "RLC (HL)", ArgumentType::None }; // RLC (HL)
    DescriptionTable[0x107] = { 2, "RLC A", ArgumentType::None }; // RLC A
    
    // RL instructions
    DescriptionTable[0x110] = { 2, "RL B", ArgumentType::None }; // RL B
    DescriptionTable[0x111] = { 2, "RL C", ArgumentType::None }; // RL C
    DescriptionTable[0x112] = { 2, "RL D", ArgumentType::None }; // RL D
    DescriptionTable[0x113] = { 2, "RL E", ArgumentType::None }; // RL E
    DescriptionTable[0x114] = { 2, "RL H", ArgumentType::None }; // RL H
    DescriptionTable[0x115] = { 2, "RL L", ArgumentType::None }; // RL L
    DescriptionTable[0x116] = { 2, "RL (HL)", ArgumentType::None }; // RL (HL)
    DescriptionTable[0x117] = { 2, "RL A", ArgumentType::None }; // RL A
    
    // RRC instructions
    DescriptionTable[0x108] = { 2, "RRC B", ArgumentType::None }; // RRC B
    DescriptionTable[0x109] = { 2, "RRC C", ArgumentType::None }; // RRC C
    DescriptionTable[0x10A] = { 2, "RRC D", ArgumentType::None }; // RRC D
    DescriptionTable[0x10B] = { 2, "RRC E", ArgumentType::None }; // RRC E
    DescriptionTable[0x10C] = { 2, "RRC H", ArgumentType::None }; // RRC H
    DescriptionTable[0x10D] = { 2, "RRC L", ArgumentType::None }; // RRC L
    DescriptionTable[0x10E] = { 2, "RRC (HL)", ArgumentType::None }; // RRC (HL)
    DescriptionTable[0x10F] = { 2, "RRC A", ArgumentType::None }; // RRC A
    
    // RR instructions
    DescriptionTable[0x118] = { 2, "RR B", ArgumentType::None }; // RR B
    DescriptionTable[0x119] = { 2, "RR C", ArgumentType::None }; // RR C
    DescriptionTable[0x11A] = { 2, "RR D", ArgumentType::None }; // RR D
    DescriptionTable[0x11B] = { 2, "RR E", ArgumentType::None }; // RR E
    DescriptionTable[0x11C] = { 2, "RR H", ArgumentType::None }; // RR H
    DescriptionTable[0x11D] = { 2, "RR L", ArgumentType::None }; // RR L
    DescriptionTable[0x11E] = { 2, "RR (HL)", ArgumentType::None }; // RR (HL)
    DescriptionTable[0x11F] = { 2, "RR A", ArgumentType::None }; // RR A
    
    // SLA instructions
    DescriptionTable[0x120] = { 2, "SLA B", ArgumentType::None }; // SLA B
    DescriptionTable[0x121] = { 2, "SLA C", ArgumentType::None }; // SLA C
    DescriptionTable[0x122] = { 2, "SLA D", ArgumentType::None }; // SLA D
    DescriptionTable[0x123] = { 2, "SLA E", ArgumentType::None }; // SLA E
    DescriptionTable[0x124] = { 2, "SLA H", ArgumentType::None }; // SLA H
    DescriptionTable[0x125] = { 2, "SLA L", ArgumentType::None }; // SLA L
    DescriptionTable[0x126] = { 2, "SLA (HL)", ArgumentType::None }; // SLA (HL)
    DescriptionTable[0x127] = { 2, "SLA A", ArgumentType::None }; // SLA A
    
    // SRL instructions
    DescriptionTable[0x138] = { 2, "SRL B", ArgumentType::None }; // SRL B
    DescriptionTable[0x139] = { 2, "SRL C", ArgumentType::None }; // SRL C
    DescriptionTable[0x13A] = { 2, "SRL D", ArgumentType::None }; // SRL D
    DescriptionTable[0x13B] = { 2, "SRL E", ArgumentType::None }; // SRL E
    DescriptionTable[0x13C] = { 2, "SRL H", ArgumentType::None }; // SRL H
    DescriptionTable[0x13D] = { 2, "SRL L", ArgumentType::None }; // SRL L
    DescriptionTable[0x13E] = { 2, "SRL (HL)", ArgumentType::None }; // SRL (HL)
    DescriptionTable[0x13F] = { 2, "SRL A", ArgumentType::None }; // SRL A
    
    // SRA instructions
    DescriptionTable[0x128] = { 2, "SRA B", ArgumentType::None }; // SRA B
    DescriptionTable[0x129] = { 2, "SRA C", ArgumentType::None }; // SRA C
    DescriptionTable[0x12A] = { 2, "SRA D", ArgumentType::None }; // SRA D
    DescriptionTable[0x12B] = { 2, "SRA E", ArgumentType::None }; // SRA E
    DescriptionTable[0x12C] = { 2, "SRA H", ArgumentType::None }; // SRA H
    DescriptionTable[0x12D] = { 2, "SRA L", ArgumentType::None }; // SRA L
    DescriptionTable[0x12E] = { 2, "SRA (HL)", ArgumentType::None }; // SRA (HL)
    DescriptionTable[0x12F] = { 2, "SRA A", ArgumentType::None }; // SRA A
    
    // SWAP instructions
    DescriptionTable[0x130] = { 2, "SWAP B", ArgumentType::None }; // SWAP B
    DescriptionTable[0x131] = { 2, "SWAP C", ArgumentType::None }; // SWAP C
    DescriptionTable[0x132] = { 2, "SWAP D", ArgumentType::None }; // SWAP D
    DescriptionTable[0x133] = { 2, "SWAP E", ArgumentType::None }; // SWAP E
    DescriptionTable[0x134] = { 2, "SWAP H", ArgumentType::None }; // SWAP H
    DescriptionTable[0x135] = { 2, "SWAP L", ArgumentType::None }; // SWAP L
    DescriptionTable[0x136] = { 2, "SWAP (HL)", ArgumentType::None }; // SWAP (HL)
    DescriptionTable[0x137] = { 2, "SWAP A", ArgumentType::None }; // SWAP A
}

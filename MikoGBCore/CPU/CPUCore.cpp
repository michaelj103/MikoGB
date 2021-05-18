//
//  CPUCore.cpp
//  MikoGB
//
//  Created on 5/3/20.
//

#include "CPUCore.hpp"
#include "CPUInstruction.hpp"
#include "MemoryController.hpp"
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace MikoGB;

CPUCore::CPUCore(MemoryController *memCon) {
    memoryController = memCon;
    reset();
    CPUInstruction::InitializeInstructionTable();
}

#if BUILD_FOR_TESTING
static const size_t MainMemorySize = 1024 * 64; // 64 KiB
CPUCore::CPUCore(uint8_t *memory, size_t len) {
    mainMemory = new uint8_t[MainMemorySize]();
    if (memory) {
        memcpy(mainMemory, memory, std::min(len, MainMemorySize));
    }
    reset();
    CPUInstruction::InitializeInstructionTable();
}

CPUCore::~CPUCore() {
    delete [] mainMemory;
}

int CPUCore::step() {
    uint8_t *basePtr = mainMemory + programCounter;
    const CPUInstruction &instruction = CPUInstruction::LookupInstruction(basePtr);
    programCounter += instruction.size;
    return instruction.func(basePtr, *this);
}

#else

int CPUCore::step() {
    const uint16_t originalPC = programCounter;
    const CPUInstruction &instruction = CPUInstruction::LookupInstruction(memoryController, programCounter);
    programCounter += instruction.size;
    uint8_t basePtr[3]; // max instruction size
    for (int i = 0; i < instruction.size; ++i) {
        basePtr[i] = memoryController->readByte(originalPC + i);
    }
    int steps = instruction.func(basePtr, *this);
#if DEBUG
    // Detect an overflow of the PC into the address space just above the ROM area
    // Note that >= 0x8000 isn't adequate because technically it's valid to run instructions from HRAM
    bool pcOverflow = programCounter >= 0x8000 && programCounter < 0xA000;
    assert(!pcOverflow);
#endif
    return steps;
}

#endif

void CPUCore::reset() {
    for (int i = 0; i < REGISTER_COUNT; ++i) {
        registers[i] = 0;
    }
    programCounter = 0;
    stackPointer = 0;
}

void CPUCore::halt() {
    throw runtime_error("HALT mode not implemented");
}

void CPUCore::stop() {
    // From Pan docs:
    // "No licensed rom makes use of STOP outside of CGB speed switching"
    throw runtime_error("STOP mode not implemented");
}


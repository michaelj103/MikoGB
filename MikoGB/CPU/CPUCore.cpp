//
//  CPUCore.cpp
//  MikoGB
//
//  Created on 5/3/20.
//

#include "CPUCore.hpp"
#include "CPUInstruction.hpp"
#include <algorithm>

using namespace std;
using namespace MikoGB;

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

void CPUCore::reset() {
    for (int i = 0; i < REGISTER_COUNT; ++i) {
        registers[i] = 0;
    }
    programCounter = 0;
    stackPointer = 0;
}


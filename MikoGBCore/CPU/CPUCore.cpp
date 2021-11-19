//
//  CPUCore.cpp
//  MikoGB
//
//  Created on 5/3/20.
//

#include "CPUCore.hpp"
#include "CPUInstruction.hpp"
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace MikoGB;

CPUCore::CPUCore(MemoryController::Ptr &memCon): memoryController(memCon) {
    reset();
    CPUInstruction::InitializeInstructionTable();
}

#if BUILD_FOR_TESTING
static const size_t MainMemorySize = 1024 * 64; // 64 KiB
static shared_ptr<MemoryController> testMemoryController = make_shared<MemoryController>();
CPUCore::CPUCore(uint8_t *memory, size_t len): memoryController(testMemoryController) {
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
    if (handleInterruptsIfNeeded()) {
        // There has been an interrupt. The handler puts the CPU into a state so that the next step will
        // start the interrupt processing routine
        // Per Pandocs and Z80 sheet, this should take about 5 cycles
        return 5;
    }
    
    if (_isHalted) {
        // Could return 1 but don't necessarily want to loop too tightly
        // Might turn a "low power" mode into a "high power" busy loop
        return 4;
    }
    
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
    _isHalted = false;
}

bool CPUCore::handleInterruptsIfNeeded() {
    bool wasNotEnabled = interruptState != InterruptState::Enabled;
    if (wasNotEnabled) {
        if (interruptState == InterruptState::Scheduled) {
            interruptState = InterruptState::Enabled;
        }
        // If interrupts are not enabled, bail unless we're in HALT mode
        // HALT is exited when an enabled interrupt is triggered regardless
        // of the global interrupt enabled state
        if (!_isHalted) {
            return false;
        }
    }
    
    const uint8_t IE = memoryController->readByte(MemoryController::IERegister) & 0x1F; // Enabled interrupts
    const uint8_t IF = memoryController->readByte(MemoryController::IFRegister) & 0x1F; // Requested interrupts
    const uint8_t interruptsToProcess = IE & IF; // Mask requested interrupts with enabled
    if (interruptsToProcess == 0) {
        // No requested interrupts are enabled
        return false;
    }
    
    // There's an interrupt. If in HALT, mode get out and double check if we want to jump or continue
    if (_isHalted) {
        _isHalted = false;
        if (wasNotEnabled) {
            return false;
        }
    }
    
    // There's an interrupt and we want to jump. So, figure out where to jump
    uint16_t targetPC = 0;
    uint8_t updatedIF = IF;
    if (isMaskSet(interruptsToProcess, MemoryController::VBlank)) {
        targetPC = 0x0040;
        updatedIF ^= MemoryController::VBlank;
    } else if (isMaskSet(interruptsToProcess, MemoryController::LCDStat)) {
        targetPC = 0x0048;
        updatedIF ^= MemoryController::LCDStat;
    } else if (isMaskSet(interruptsToProcess, MemoryController::TIMA)) {
        targetPC = 0x0050;
        updatedIF ^= MemoryController::TIMA;
    } else if (isMaskSet(interruptsToProcess, MemoryController::Serial)) {
        targetPC = 0x0058;
        updatedIF ^= MemoryController::Serial;
    } else if (isMaskSet(interruptsToProcess, MemoryController::Input)) {
        targetPC = 0x0060;
        updatedIF ^= MemoryController::Input;
    } else {
        throw runtime_error("Interrupt processing error");
    }
    
    //... And start the interrupt
    setMemory(MemoryController::IFRegister, updatedIF);
    interruptState = InterruptState::Disabled;
    stackPush(programCounter);
    programCounter = targetPC;
    return true;
}

void CPUCore::halt() {
    _isHalted = true;
}

void CPUCore::stop() {
    // From Pan docs:
    // "No licensed rom makes use of STOP outside of CGB speed switching"
    throw runtime_error("STOP mode not implemented");
}


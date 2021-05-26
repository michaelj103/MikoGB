//
//  CPUCore.hpp
//  MikoGB
//
//  Created on 5/3/20.
//

#ifndef CPUCore_hpp
#define CPUCore_hpp

// Note: Typo on register D code in gameboy programming manual: it's 010.
// It can't be the same as L. 2 is consistent with ordering and matches the 8080 manual
#define REGISTER_B 0
#define REGISTER_C 1
#define REGISTER_D 2
#define REGISTER_E 3
#define REGISTER_H 4
#define REGISTER_L 5

#define REGISTER_A 7
// Register code 6 means load from memory pointed to by HL, or something else contextually
// Use it for register F
#define REGISTER_F 6
#define REGISTER_COUNT 8

#include <cstdlib>
#include "BitTwiddlingUtil.h"
#include "MemoryController.hpp"

namespace MikoGB {

class MemoryController;

enum FlagBit : uint8_t {
    Zero        = 1 << 7,
    N           = 1 << 6,
    H           = 1 << 5,
    Carry       = 1 << 4,
};

class CPUCore {
public:
    CPUCore(MemoryController *memoryController);
#if BUILD_FOR_TESTING
    CPUCore(uint8_t *memory, size_t len);
    ~CPUCore();
#endif
    
    /// Step one instruction. Returns elapsed cycle count
    int step();
    
    /// Reset the CPU state to initial
    void reset();
    
    // State
    
    uint8_t registers[REGISTER_COUNT];
    uint16_t programCounter;
    uint16_t stackPointer;
    
    MemoryController *memoryController;
#if BUILD_FOR_TESTING
    uint8_t *mainMemory;
#endif
    void setMemory(uint16_t address, uint8_t val);
    
    uint8_t getMemory(uint16_t address) const;
    
    uint16_t getHLptr() const;
    
    void incrementHLptr();
    
    void decrementHLptr();
    
    uint16_t getBCptr() const;
    
    uint16_t getDEptr() const;
    
    /// "C" pointer is the memory address at 0xFF00 + register C
    uint16_t getCptr() const;
    
    /// Convenience for pushing two 8-bit values onto the stack
    /// hi is stored at stackPointer-1, lo at SP-2. SP points at lo afterwards
    void stackPush(uint8_t hi, uint8_t lo);
    
    /// Convenince, splits and calls stackPush(uint8_t, uint8_t)
    void stackPush(uint16_t val);
    
    /// Convenience for popping 2 8-bit values from the stack
    void stackPop(uint8_t &hi, uint8_t &lo);
    
    /// Convenience, calls stackPop(uint8_t, uint8_t) and returns combined
    uint16_t stackPop();
    
    void halt();
    void stop();
    
    // Flags
    
    bool getFlag(FlagBit) const;
    void setFlag(FlagBit, bool);
    
    // Interrupts
    enum InterruptState {
        Disabled,
        Scheduled, // The EI command takes one extra cycle to "activate"
        Enabled,
    };
    InterruptState interruptState;
    
private:
    bool handleInterruptsIfNeeded();
};



// Inline convenience member function

inline uint16_t CPUCore::getHLptr() const  {
    return word16(registers[REGISTER_L], registers[REGISTER_H]);
}

inline void CPUCore::incrementHLptr() {
    uint16_t ptr = getHLptr();
    ptr += 1;
    uint8_t lo, hi;
    splitWord16(ptr, lo, hi);
    registers[REGISTER_L] = lo;
    registers[REGISTER_H] = hi;
}

inline void CPUCore::decrementHLptr() {
    uint16_t ptr = getHLptr();
    ptr -= 1;
    uint8_t lo, hi;
    splitWord16(ptr, lo, hi);
    registers[REGISTER_L] = lo;
    registers[REGISTER_H] = hi;
}

inline uint16_t CPUCore::getBCptr() const {
    return word16(registers[REGISTER_C], registers[REGISTER_B]);
}

inline uint16_t CPUCore::getDEptr() const {
    return word16(registers[REGISTER_E], registers[REGISTER_D]);
}

inline uint16_t CPUCore::getCptr() const {
    return 0xFF00 + registers[REGISTER_C];
}

inline void CPUCore::stackPush(uint8_t hi, uint8_t lo) {
    setMemory(--stackPointer, hi);
    setMemory(--stackPointer, lo);
}

inline void CPUCore::stackPush(uint16_t val) {
    uint8_t hi = 0, lo = 0;
    splitWord16(val, lo, hi);
    stackPush(hi, lo);
}

inline uint16_t CPUCore::stackPop() {
    uint8_t hi = 0, lo = 0;
    stackPop(hi, lo);
    return word16(lo, hi);
}

inline void CPUCore::stackPop(uint8_t &hi, uint8_t &lo) {
    lo = memoryController->readByte(stackPointer++);
    hi = memoryController->readByte(stackPointer++);
}

inline bool CPUCore::getFlag(FlagBit bit) const {
    return (registers[REGISTER_F] & bit) == bit;
}

inline void CPUCore::setFlag(FlagBit bit, bool isSet) {
    if (isSet) {
        // & with 0xF0 to disallow setting of low 4 bits
        registers[REGISTER_F] |= (bit & 0xF0);
    } else {
        registers[REGISTER_F] &= ~(bit);
    }
}

inline void CPUCore::setMemory(uint16_t address, uint8_t val) {
#if BUILD_FOR_TESTING
    mainMemory[address] = val;
#else
    memoryController->setByte(address, val);
#endif
}

inline uint8_t CPUCore::getMemory(uint16_t address) const {
#if BUILD_FOR_TESTING
    return mainMemory[address];
#else
    return memoryController->readByte(address);
#endif
}

}

#endif /* CPUCore_hpp */

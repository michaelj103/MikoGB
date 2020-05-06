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
// Register code 6 means load from memory pointed to by HL
#define REGISTER_A 7
#define REGISTER_F 8
#define REGISTER_COUNT 9

#include <cstdlib>
#include "BitTwiddlingUtil.h"

namespace MikoGB {

class CPUCore {
public:
    CPUCore(uint8_t *memory, size_t len);
    ~CPUCore();
    
    /// Step one instruction. Returns elapsed cycle count
    int step();
    
    /// Reset the CPU state to initial
    void reset();
    
    // State
    
    uint8_t registers[REGISTER_COUNT];
    uint16_t programCounter;
    uint16_t stackPointer;
    
    uint16_t getHL() {
        return word16(registers[REGISTER_L], registers[REGISTER_H]);
    }
    
    uint16_t getBC() {
        return word16(registers[REGISTER_C], registers[REGISTER_B]);
    }
    
    uint16_t getDE() {
        return word16(registers[REGISTER_E], registers[REGISTER_D]);
    }
    
    //TODO: should this be a class? "MemoryController"
    uint8_t *mainMemory;
};

}

#endif /* CPUCore_hpp */

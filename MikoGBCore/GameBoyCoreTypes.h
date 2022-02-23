//
//  GameBoyCoreTypes.h
//  MikoGB
//
//  Created on 5/26/21.
//

#ifndef GameBoyCoreTypes_h
#define GameBoyCoreTypes_h

#include <string>

namespace MikoGB {

enum class JoypadButton {
    Right = 0,
    Left = 1,
    Up = 2,
    Down = 3,
    A = 4,
    B = 5,
    Select = 6,
    Start = 7
};

using RunnableChangedCallback = std::function<void(bool isRunnable)>;

struct DisassembledInstruction {
    int romBank = 0; // -1 indicates HRAM, e.g. not in ROM but in writable instruction area
    uint16_t addr = 0;
    std::string description;
};

struct RegisterState {
    // registers
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint8_t A;
    
    // flags
    bool ZFlag;
    bool NFlag;
    bool HFlag;
    bool CFlag;
};

}

// toggle to enable debugger features that have a memory and performan impact on the CPU emulation
// These include:
// - breakpoints
// - instruction location cache for disassembly
#define ENABLE_DEBUGGER 1

#endif /* GameBoyCoreTypes_h */

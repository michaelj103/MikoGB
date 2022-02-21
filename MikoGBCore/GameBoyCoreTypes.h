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

}

#endif /* GameBoyCoreTypes_h */

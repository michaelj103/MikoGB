//
//  Disassembler.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef Disassembler_hpp
#define Disassembler_hpp

#include <string>
#include <vector>
#include <set>
#include "MemoryController.hpp"
#include "GameboyCoreTypes.h"

namespace MikoGB {

class Disassembler {
public:
    Disassembler();
    using Ptr = std::shared_ptr<Disassembler>;
    
    std::vector<DisassembledInstruction> disassembleInstructions(uint16_t, int, const MemoryController::Ptr &);
    std::vector<DisassembledInstruction> precedingDisassembledInstructions(uint16_t, int, const MemoryController::Ptr &);
    
private:
    static void InitializeDisassemblyTable();
    struct KnownInstruction {
        int romBank;
        uint16_t addr;
        uint16_t size;
    };
    
    std::set<KnownInstruction> _knownInstructions;
    friend bool operator<(const KnownInstruction &a, const KnownInstruction &b);
};

inline bool operator<(const Disassembler::KnownInstruction &a, const Disassembler::KnownInstruction &b) {
    if (a.romBank == b.romBank) {
        return a.addr < b.addr;
    }
    return a.romBank < b.romBank;
}

}

#endif /* Disassembler_hpp */

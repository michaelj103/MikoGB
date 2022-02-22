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
#include "CPUCore.hpp"
#include "GameboyCoreTypes.h"

namespace MikoGB {

class Disassembler {
public:
    Disassembler();
    using Ptr = std::shared_ptr<Disassembler>;
    
    std::vector<DisassembledInstruction> disassembleInstructions(uint16_t, int, const MemoryController::Ptr &);
    std::vector<DisassembledInstruction> precedingDisassembledInstructions(uint16_t, int, const MemoryController::Ptr &, const CPUCore::Ptr &);
    
private:
    static void InitializeDisassemblyTable();
};

}

#endif /* Disassembler_hpp */

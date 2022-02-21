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
#include "MemoryController.hpp"
#include <vector>
#include "GameboyCoreTypes.h"

namespace MikoGB {

class Disassembler {
public:
    Disassembler();
    using Ptr = std::shared_ptr<Disassembler>;
    
    std::vector<DisassembledInstruction> disassembleInstructions(uint16_t, int, const MemoryController::Ptr &);
    
private:
    static void InitializeDisassemblyTable();
};

}

#endif /* Disassembler_hpp */

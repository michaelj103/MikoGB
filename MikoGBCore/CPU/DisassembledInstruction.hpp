//
//  DisassembledInstruction.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef DisassembledInstruction_hpp
#define DisassembledInstruction_hpp

#include <string>
#include <map>
#include "MemoryController.hpp"

namespace MikoGB {

struct DisassembledInstruction {
    const uint16_t size = 0;
    const std::string description;
};

class Disassembler {
public:
    const DisassembledInstruction disassembleInstruction(uint16_t, const MemoryController::Ptr &);
    
    static void InitializeDisasseblyTable();
    
private:
    using BankMap = std::map<uint16_t, DisassembledInstruction>;
    std::map<uint16_t, BankMap> cacheByBank;
};

}

#endif /* DisassembledInstruction_hpp */

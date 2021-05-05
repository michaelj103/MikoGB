//
//  CPUInstruction.hpp
//  MikoGB
//
//  Created on 5/3/20.
//

#ifndef CPUInstruction_hpp
#define CPUInstruction_hpp

#include <functional>
#include "CPUCore.hpp"

namespace MikoGB {

struct CPUInstruction {
    uint16_t size = 0;
    std::function<int(const uint8_t *, CPUCore &)> func = UnrecognizedInstruction;
    
    static void InitializeInstructionTable();
    static const CPUInstruction &LookupInstruction(uint8_t *);
    
private:
    static CPUInstruction *InstructionTable;
    static int UnrecognizedInstruction(const uint8_t *, CPUCore &);
};

}

#endif /* CPUInstruction_hpp */

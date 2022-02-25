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
    uint16_t size = 3; // default to 3 so that UnrecognizedInstruction exception can work
    std::function<int(const uint8_t *, CPUCore &)> func = UnrecognizedInstruction;
    
    static void InitializeInstructionTable();
#if BUILD_FOR_TESTING
    static const CPUInstruction &LookupInstruction(uint8_t *);
#else
    static const CPUInstruction &LookupInstruction(const MemoryController::Ptr &, uint16_t);
#endif
    
private:
    static CPUInstruction *InstructionTable;
    static int UnrecognizedInstruction(const uint8_t *, CPUCore &);
};

}

#endif /* CPUInstruction_hpp */

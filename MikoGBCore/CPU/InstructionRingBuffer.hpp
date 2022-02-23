//
//  InstructionRingBuffer.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef InstructionRingBuffer_hpp
#define InstructionRingBuffer_hpp

#include <vector>
#include <set>

namespace MikoGB {

struct KnownInstruction {
    int romBank = 0;
    uint16_t addr = 0;
    uint16_t size = 0;
};

class InstructionRingBuffer {
public:
    InstructionRingBuffer(size_t bufferSize);
    
    void append(const KnownInstruction &i);
    
    std::set<KnownInstruction> uniqueInstructions() const;
    
private:
    std::vector<KnownInstruction> _buffer;
    size_t _startPosition = 0;
    size_t _count = 0;
};

inline bool operator<(const KnownInstruction &a, const KnownInstruction &b) {
    if (a.romBank == b.romBank) {
        return a.addr < b.addr;
    } else {
        return a.romBank < b.romBank;
    }
}

}

#endif /* InstructionRingBuffer_hpp */

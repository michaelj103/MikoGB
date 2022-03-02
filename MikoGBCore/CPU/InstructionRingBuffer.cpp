//
//  InstructionRingBuffer.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "InstructionRingBuffer.hpp"
#include <algorithm>

using namespace std;
using namespace MikoGB;

InstructionRingBuffer::InstructionRingBuffer(size_t desiredSize) {
    KnownInstruction empty;
    _buffer = vector<KnownInstruction>(desiredSize, empty);
}

void InstructionRingBuffer::append(const KnownInstruction &i) {
    const size_t bufferSpace = _buffer.size();
    const size_t nextPos = (_startPosition + _count) % _buffer.size();
    _buffer[nextPos] = i;
    if (_count == bufferSpace) {
        // we were full, so we've wrapped. Move the start position
        _startPosition = (_startPosition + 1) % _buffer.size();
    } else {
        // not full yet, so just bump the count up
        _count += 1;
    }
    _uniqueInstructions.insert(i);
}

std::set<KnownInstruction> InstructionRingBuffer::uniqueInstructions() const {
    // screw it, just track every instruction that's been executed. Requires precompiler flag anyway
    return _uniqueInstructions;
}

std::vector<KnownInstruction> InstructionRingBuffer::previousInstructions(size_t maxCount) const {
    vector<KnownInstruction> instructions;
    const size_t readSize = std::min(maxCount, _count);
    size_t currentPosition = _startPosition;
    for (size_t i = 0; i < readSize; ++i) {
        if (currentPosition == 0) {
            currentPosition = _buffer.size() - 1;
        } else {
            currentPosition -= 1;
        }
        
        instructions.push_back(_buffer[currentPosition]);
    }
    
    return instructions;
}

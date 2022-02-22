//
//  InstructionRingBuffer.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "InstructionRingBuffer.hpp"

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
}

std::set<KnownInstruction> InstructionRingBuffer::uniqueInstructions() const {
    set<KnownInstruction> instructionSet;
    const size_t bufferSpace = _buffer.size();
    for (size_t i = 0; i < _count; ++i) {
        size_t idx = (_startPosition + i) % bufferSpace;
        instructionSet.insert(_buffer[idx]);
    }
    return instructionSet;
}

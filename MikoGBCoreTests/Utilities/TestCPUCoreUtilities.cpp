//
//  TestCPUCoreUtilities.cpp
//  MikoGB
//
//  Created on 5/26/20.
//

#include "TestCPUCoreUtilities.hpp"

std::vector<uint8_t> createGBMemory(const std::vector<uint8_t> &mem, const std::map<uint16_t, uint8_t> &otherValues) {
    //Get largest address
    uint16_t lastAddr = 0;
    if (otherValues.size() > 0) {
        auto endIt = otherValues.rbegin();
        lastAddr = (*endIt).first;
    }
    
    //Create appropriately sized buffer with the initial data
    const size_t size = std::max(mem.size(), (size_t)lastAddr + 1);
    std::vector<uint8_t> outBuffer(size, 0);
    uint8_t *dest = outBuffer.data();
    memcpy(dest, mem.data(), mem.size());
    
    //set the other random point values
    for (auto &x : otherValues) {
        outBuffer[x.first] = x.second;
    }
    
    return outBuffer;
}

//
//  TestCPUCoreUtilities.cpp
//  MikoGB
//
//  Created on 5/26/20.
//

#include "TestCPUCoreUtilities.hpp"

std::unique_ptr<uint8_t[]> createGBMemory(const std::vector<uint8_t> &mem) {
    std::unique_ptr<uint8_t[]> ptr(new uint8_t[mem.size()]);
    uint8_t *dest = ptr.get();
    memcpy(dest, mem.data(), mem.size());
    return ptr;
}

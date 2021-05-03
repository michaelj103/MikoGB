//
//  TestCPUCoreUtilities.hpp
//  MikoGB
//
//  Created on 5/26/20.
//

#ifndef TestCPUCoreUtilities_hpp
#define TestCPUCoreUtilities_hpp

#include <memory>
#include <vector>
#include <map>

std::vector<uint8_t> createGBMemory(const std::vector<uint8_t> &mem, const std::map<uint16_t, uint8_t> &otherValues);
std::vector<uint8_t> createGBMemory(const std::vector<uint8_t> &mem, const std::map<uint16_t, std::vector<uint8_t>> &otherValues);

#endif /* TestCPUCoreUtilities_hpp */

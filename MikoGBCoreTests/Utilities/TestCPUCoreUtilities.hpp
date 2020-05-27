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

std::unique_ptr<uint8_t[]> createGBMemory(const std::vector<uint8_t> &mem);

#endif /* TestCPUCoreUtilities_hpp */

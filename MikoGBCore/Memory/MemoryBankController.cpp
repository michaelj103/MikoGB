//
//  MemoryBankController.cpp
//  MikoGB
//
//  Created on 5/17/21.
//

#include "MemoryBankController.hpp"
#include "NoMBC.hpp"
#include "MBC1.hpp"
#include "MBC3.hpp"

#include <iostream>

using namespace std;
using namespace MikoGB;

MemoryBankController *MemoryBankController::CreateMBC(const CartridgeHeader &header) {
    MemoryBankController *mbc = nullptr;
    switch (header.getType()) {
        case CartridgeType::ROM_Only:
            mbc = new NoMBC(header);
            break;
        case CartridgeType::MBC1:
        case CartridgeType::MBC1_RAM:
        case CartridgeType::MBC1_RAM_BATT:
            mbc = new MBC1(header);
            break;
        case CartridgeType::MBC3_RAM_BATT:
            mbc = new MBC3(header);
            break;
            
        default:
            break;
    }
    
    return mbc;
}

bool MemoryBankController::configureWithROMData(const void *romData, size_t size) {
    if (_romData != nullptr) {
        cerr << "MBC may not be configured multiple times\n";
        return false;
    }
    
    _romData = new uint8_t[size]();
    memcpy(_romData, romData, size);
    return true;
}

void MemoryBankController::updateClock(size_t cpuCycles) {
    // no-op for most MBCs
}

MemoryBankController::~MemoryBankController() {
    delete [] _romData;
}

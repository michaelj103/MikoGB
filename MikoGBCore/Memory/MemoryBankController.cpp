//
//  MemoryBankController.cpp
//  MikoGB
//
//  Created on 5/17/21.
//

#include "MemoryBankController.hpp"
#include "NoMBC.hpp"
#include "MBC1.hpp"

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
            mbc = new MBC1(header);
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
    
    _romData = new uint8_t[size];
    memcpy(_romData, romData, size);
    return true;
}

MemoryBankController::~MemoryBankController() {
    delete [] _romData;
}

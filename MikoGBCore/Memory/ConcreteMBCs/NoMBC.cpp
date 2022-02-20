//
//  NoMBC.cpp
//  MikoGB
//
//  Created on 5/17/21.
//

#include "NoMBC.hpp"
#include <iostream>

using namespace std;
using namespace MikoGB;

static size_t ExpectedDataSize = 1024 * 32; // 32 KiB
static size_t RAMBankSize = 1024 * 8; // 8 KiB
static uint16_t RAMBase = 0xA000;
static uint16_t RAMMax = 0xC000;

NoMBC::NoMBC(const CartridgeHeader &header) {
    switch (header.getRAMSize()) {
        case CartridgeRAMSize::RAM0:
            _ramType = RAMType::None;
            break;
        case CartridgeRAMSize::RAM8KB:
            _ramType = RAMType::SingleBank;
            break;
        default:
            _ramType = RAMType::Invalid;
            break;
    }
}

NoMBC::~NoMBC() {
    delete [] _ramData;
}

bool NoMBC::configureWithROMData(const void *romData, size_t size) {
    if (size != ExpectedDataSize) {
        cerr << "Unexpected ROM data size for no MBC: " << size << endl;
        return false;
    }
    if (_ramType == RAMType::Invalid) {
        cerr << "Unexpected RAM size for no MBC" << endl;
        return false;
    }
    
    _ramData = new uint8_t[RAMBankSize];
    return MemoryBankController::configureWithROMData(romData, size);
}

uint8_t NoMBC::readROM(uint16_t addr) const {
    return _romData[addr];
}

uint8_t NoMBC::readRAM(uint16_t addr) const {
    if (_ramType == RAMType::SingleBank) {
        assert(addr >= RAMBase && addr < RAMMax);
        return _ramData[addr - RAMBase];
    } else {
        throw runtime_error("Read from external RAM but cartridge specifies no RAM");
    }
}

void NoMBC::writeRAM(uint16_t addr, uint8_t val) {
    if (_ramType == RAMType::SingleBank) {
        assert(addr >= RAMBase && addr < RAMMax);
        _ramData[addr - RAMBase] = val;
    } else {
        throw runtime_error("Write to external RAM but cartridge specifies no RAM");
    }
}

void NoMBC::writeControlCode(uint16_t addr, uint8_t val) {
    // Some ROMs write them anyway. Might be a relic from when the game was under development?
//    assert(false); // control codes aren't valid for NoMBC
}

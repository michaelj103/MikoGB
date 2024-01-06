//
//  MBC5.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 9/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "MBC5.hpp"
#include <cassert>

using namespace std;
using namespace MikoGB;

static size_t SwitchableROMBaseAddr = 0x4000;
static size_t SwitchableRAMBaseAddr = 0xA000;
static size_t ROMBankSize = 1024 * 16; // 16 KiB
static size_t RAMBankSize = 1024 * 8; // 8 KiB

MBC5::MBC5(const CartridgeHeader &header) {
    switch (header.getROMSize()) {
        case CartridgeROMSize::BANKS_2:
            _romBankCount = 2;
        case CartridgeROMSize::BANKS_4:
            _romBankCount = 4;
            break;
        case CartridgeROMSize::BANKS_8:
            _romBankCount = 8;
            break;
        case CartridgeROMSize::BANKS_16:
            _romBankCount = 16;
            break;
        case CartridgeROMSize::BANKS_32:
            _romBankCount = 32;
            break;
        case CartridgeROMSize::BANKS_64:
            _romBankCount = 64;
            break;
        case CartridgeROMSize::BANKS_128:
            _romBankCount = 128;
            break;
        case CartridgeROMSize::BANKS_256:
            _romBankCount = 256;
            break;
        case CartridgeROMSize::BANKS_512:
            _romBankCount = 512;
            break;
        case CartridgeROMSize::Unsupported:
            _romBankCount = -1;
            break;
    }
    
    bool hasRumble = header.hasRumble();
    _hasRumble = hasRumble;
    
    switch (header.getRAMSize()) {
        case CartridgeRAMSize::RAM0:
            _ramBankCount = 0;
            break;
        case CartridgeRAMSize::RAM2KB: // For convenience, treat 2KB as a single bank of 8KB
        case CartridgeRAMSize::RAM8KB:
            _ramBankCount = 1;
            break;
        case CartridgeRAMSize::RAM32KB:
            _ramBankCount = 4;
            break;
        case CartridgeRAMSize::RAM64KB:
            if (hasRumble) {
                // Rumble only allows up to 4 banks of RAM
                _ramBankCount = -1;
            } else {
                _ramBankCount = 8;
            }
            break;
        case CartridgeRAMSize::RAM128KB:
            if (hasRumble) {
                // Rumble only allows up to 4 banks of RAM
                _ramBankCount = -1;
            } else {
                _ramBankCount = 16;
            }
            break;
        case CartridgeRAMSize::Unsupported:
            _ramBankCount = -1;
            break;
    }
    
    _hasBatteryBackup = header.hasBatteryBackup();
}

MBC5::~MBC5() {
    delete [] _ramData;
}

bool MBC5::configureWithROMData(const void *romData, size_t size) {
    if (_romBankCount == -1 || _ramBankCount == -1) {
        cerr << "Unexpected ROM/RAM configuration for MBC5" << endl;
        return false;
    }
    
    size_t expectedROMSize = ROMBankSize * _romBankCount;
    if (size != expectedROMSize) {
        cerr << "Unexpected ROM data size for MBC5: " << size << endl;
        return false;
    }
    
    if (_ramBankCount > 0) {
        size_t ramSize = RAMBankSize * _ramBankCount;
        _ramData = new uint8_t[ramSize]();
    }
    
    return MemoryBankController::configureWithROMData(romData, size);
}

void MBC5::_updateBankNumbers() {
    // determine ROM bank
    int romBankUpper = ((int)(_romBankUpper & 0x1)) << 8;
    _romBank = ((int)_romBankLower) | romBankUpper;
    
    uint8_t ramBankMask = _hasRumble ? 0x3 : 0xF; // with rumble, only lower 2 bits are usable
    // Mask to the real usable bits. Counts are always powers of 2. Some games write illegal values otherwise
    // I can't find documentation that this behavior is correct, but it is the documented behavior for
    // earlier MBCs and otherwise games would try to write to unsupported banks
    uint8_t ramBankUsableMask = (_ramBankCount - 1);
    _ramBank = (int)(_ramBankRegister & ramBankMask & ramBankUsableMask);
    
#if DEBUG
    assert(_romBank < _romBankCount && _romBank >= 0);
    assert(_ramBank < _ramBankCount || _ramBank == 0);
#endif
}

void MBC5::writeControlCode(uint16_t addr, uint8_t val) {
    if (addr < 0x2000) {
        // Writing to the RAM Enable area
        // if the low 4 bits are 0x0A, ram is enabled
        _ramEnabled = ((val & 0x0F) == 0x0A);
    } else if (addr < 0x3000) {
        // Writing to the lower ROM bank number
        _romBankLower = val;
        _updateBankNumbers();
    } else if (addr < 0x4000) {
        // Writing to the upper ROM or RAM bank number
        _romBankUpper = val;
        _updateBankNumbers();
    } else if (addr < 0x6000) {
        _ramBankRegister = val;
        _updateBankNumbers();
    }
    
    // Other values are ignored, for compatibility purposes with older MBCs
}

uint8_t MBC5::readROM(uint16_t addr) const {
    size_t baseIdx = _romBank * ROMBankSize;
    size_t romIdx = baseIdx + (addr - SwitchableROMBaseAddr);
    return _romData[romIdx];
}

static inline size_t _RAMDataIndex(uint16_t addr, int bankNum) {
    const size_t baseIdx = bankNum * RAMBankSize;
    const size_t ramIdx = baseIdx + (addr - SwitchableRAMBaseAddr);
    return ramIdx;
}

uint8_t MBC5::readRAM(uint16_t addr) const {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return 0xFF;
    }
    const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
    return _ramData[ramIdx];
}

void MBC5::writeRAM(uint16_t addr, uint8_t val) {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return;
    }
    const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
    if (val != _ramData[ramIdx]) {
        _ramData[ramIdx] = val;
        _isPersistenceStale = _hasBatteryBackup;
    }
}

int MBC5::currentROMBank() const {
    return _romBank;
}

size_t MBC5::saveDataSize() const {
    if (_hasBatteryBackup) {
        return _ramBankCount * RAMBankSize;
    } else {
        return 0;
    }
}

void *MBC5::getSaveData() const {
    if (_hasBatteryBackup) {
        return _ramData;
    } else {
        return nullptr;
    }
}

bool MBC5::loadSaveData(const void *saveData, size_t size) {
    if (_hasBatteryBackup) {
        size_t expectedSize = saveDataSize();
        if (expectedSize != size) {
            return false;
        }
        
        memcpy(_ramData, saveData, size);
        return true;
    } else {
        return false;
    }
}

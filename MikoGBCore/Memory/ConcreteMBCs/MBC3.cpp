//
//  MBC3.cpp
//  MikoGB
//
//  Created on 6/22/21.
//

#include "MBC3.hpp"

using namespace std;
using namespace MikoGB;

static size_t SwitchableROMBaseAddr = 0x4000;
static size_t SwitchableRAMBaseAddr = 0xA000;
static size_t ROMBankSize = 1024 * 16; // 16 KiB
static size_t RAMBankSize = 1024 * 8; // 8 KiB

MBC3::MBC3(const CartridgeHeader &header) {
    switch (header.getROMSize()) {
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
        default:
            // Invalid. MBC3 supports from 64KiB to 2MiB
            _romBankCount = -1;
            break;
    }
    
    switch (header.getRAMSize()) {
        case CartridgeRAMSize::RAM32KB:
            _ramBankCount = 4;
            break;
        default:
            // Invalid. Per GB manual, MBC3 always has 4 RAM banks
            _ramBankCount = -1;
            break;
    }
}

MBC3::~MBC3() {
    delete [] _ramData;
}

bool MBC3::configureWithROMData(const void *romData, size_t size) {
    if (_romBankCount == -1 || _ramBankCount == -1) {
        cerr << "Unexpected ROM/RAM configuration for MBC3" << endl;
        return false;
    }
    
    size_t expectedROMSize = ROMBankSize * _romBankCount;
    if (size != expectedROMSize) {
        cerr << "Unexpected ROM data size for MBC3: " << size << endl;
        return false;
    }
    
    if (_ramBankCount > 0) {
        size_t ramSize = RAMBankSize * _ramBankCount;
        _ramData = new uint8_t[ramSize];
    }
    
    return MemoryBankController::configureWithROMData(romData, size);
}

void MBC3::_updateBankNumbers() {
    // determine ROM bank
    uint8_t mask = _romBankCount - 1; // register is masked to the number of bits required
    uint8_t bankNum = _romBankCode & mask;
    _romBank = bankNum == 0 ? 1 : (int)bankNum;
#if DEBUG
        assert(_romBank < _romBankCount && _romBank > 0);
#endif
    
    // determine RAM bank
    if (_ramBankCode < 4) {
        _ramBank = (int)_ramBankCode;
    } else {
        // Reading/writing to a clock register
        // TODO: support MBC3 clock
        assert(false);
    }
}

void MBC3::writeControlCode(uint16_t addr, uint8_t val) {
    if (addr < 0x2000) {
        // Writing to the RAM and Clock Enable area
        // if the low 4 bits are 0x0A, ram is enabled
        _ramEnabled = ((val & 0x0F) == 0x0A);
    } else if (addr < 0x4000) {
        // ROM bank code. Simply a 1-byte number masked to the valid range
        _romBankCode = val;
        _updateBankNumbers();
    } else if (addr < 0x6000) {
        // Writing the RAM bank code or clock register code. Valid range is 0-3 for RAM and 0x08 - 0x0C for clock
        _ramBankCode = val;
        _updateBankNumbers();
    } else if (addr < 0x8000) {
        // Writing to clock latch. Only 0 and 1 are valid. Changing from 0->1 "latches" the clock values
        _latchVal = val;
#if DEBUG
        //TODO: MBC3 built-in clock
        assert(false);
#endif
    } else {
        // Should be unreachable
        assert(false);
    }
}

uint8_t MBC3::readROM(uint16_t addr) const {
    size_t baseIdx = _romBank * ROMBankSize;
    size_t romIdx = baseIdx + (addr - SwitchableROMBaseAddr);
    return _romData[romIdx];
}

static inline size_t _RAMDataIndex(uint16_t addr, int bankNum) {
    const size_t baseIdx = bankNum * ROMBankSize;
    const size_t ramIdx = baseIdx + (addr - SwitchableRAMBaseAddr);
    return ramIdx;
}

uint8_t MBC3::readRAM(uint16_t addr) const {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return 0xFF;
    }
    const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
    return _ramData[ramIdx];
}

void MBC3::writeRAM(uint16_t addr, uint8_t val) {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return;
    }
    const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
    _ramData[ramIdx] = val;
}

int MBC3::currentROMBank() const {
    return _romBank;
}

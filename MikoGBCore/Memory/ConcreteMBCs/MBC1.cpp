//
//  MBC1.cpp
//  MikoGB
//
//  Created on 6/11/21.
//

#include "MBC1.hpp"

using namespace std;
using namespace MikoGB;

static size_t SwitchableROMBaseAddr = 0x4000;
static size_t ROMBankSize = 1024 * 16; // 16 KiB
static size_t RAMBankSize = 1024 * 8; // 8 KiB

MBC1::MBC1(const CartridgeHeader &header) {
    bool ramRestricted = false;
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
            ramRestricted = true; // with 2MiB of ROM, RAM must be 8KiB
            _romBankCount = 128;
            break;
        default:
            // invalid
            _romBankCount = -1;
            break;
    }
    
    switch (header.getRAMSize()) {
        case CartridgeRAMSize::RAM0:
            _ramBankCount = 0;
            break;
        case CartridgeRAMSize::RAM2KB: // For convenience, treat 2KB as a single bank of 8KB
        case CartridgeRAMSize::RAM8KB:
            _ramBankCount = 1;
            break;
        case CartridgeRAMSize::RAM32KB:
            if (ramRestricted) {
                // Per GB manual, MBC1 is restricted to 8KiB RAM when using 2MiB of ROM
                _ramBankCount = -1;
            } else {
                _ramBankCount = 4;
            }
            break;
        default:
            _ramBankCount = -1;
            break;
    }
}

MBC1::~MBC1() {
    delete [] _ramData;
}

bool MBC1::configureWithROMData(const void *romData, size_t size) {
    if (_romBankCount == -1 || _ramBankCount == -1) {
        cerr << "Unexpected ROM/RAM configuration for MBC1" << endl;
        return false;
    }
    
    size_t expectedROMSize = ROMBankSize * _romBankCount;
    if (size != expectedROMSize) {
        cerr << "Unexpected ROM data size for MBC1: " << size << endl;
        return false;
    }
    
    if (_ramBankCount > 0) {
        size_t ramSize = RAMBankSize * _ramBankCount;
        _ramData = new uint8_t[ramSize];
    }
    
    return MemoryBankController::configureWithROMData(romData, size);
}

void MBC1::_updateBankNumbers() {
    // determine ROM bank
    if (_romBankCount <= 32) {
        // "small" ROM. Bank can be specified in 5 bits
        uint8_t mask = _romBankCount - 1; // register is masked to the number of bits required
        uint8_t bankNum = _romBankLower & mask;
        _romBank = bankNum == 0 ? 1 : (int)bankNum;
    } else {
        // "large" ROM. We need the upper bits
        uint8_t bankUpper = (_bankNumberUpper & 0x03) << 5; // only 2 bits
        uint8_t bankNum = bankUpper | (_romBankLower & 0x1F);
        _romBank = bankNum == 0 ? 1 : (int)bankNum;
    }
#if DEBUG
        assert(_romBank < _romBankCount && _romBank > 0);
#endif
    
    // determine RAM bank
    bool isRamSwitchMode = (_bankingMode & 0x01) == 0x01;
    if (isRamSwitchMode && _ramBankCount > 1) {
        _ramBank = (_bankNumberUpper & 0x03);
    } else {
        _ramBank = 0;
    }
}

void MBC1::writeControlCode(uint16_t addr, uint8_t val) {
    if (addr < 0x2000) {
        // Writing to the RAM Enable area
        // if the low 4 bits are 0x0A, ram is enabled
        _ramEnabled = ((val & 0x0F) == 0x0A);
    } else if (addr < 0x4000) {
        // Writing to the lower ROM bank number
        _romBankLower = val;
        _updateBankNumbers();
    } else if (addr < 0x6000) {
        // Writing to the upper ROM or RAM bank number
        _bankNumberUpper = val;
        _updateBankNumbers();
    } else if (addr < 0x8000) {
        _bankingMode = val;
        _updateBankNumbers();
    } else {
        // Should be unreachable
        assert(false);
    }
}

uint8_t MBC1::readROM(uint16_t addr) const {
    size_t baseIdx = _romBank * ROMBankSize;
    size_t romIdx = baseIdx + (addr - SwitchableROMBaseAddr);
    return _romData[romIdx];
}

uint8_t MBC1::readRAM(uint16_t addr) const {
    //TODO: RAM reading
    return 0xFF;
}

void MBC1::writeRAM(uint16_t addr, uint8_t val) {
    //TODO: RAM writing
}

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
    
    _batteryBackup = header.hasBatteryBackup();
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
        _ramData = new uint8_t[ramSize]();
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
    _ramBank = (int)_ramBankCode;
    
    // it's ok for this to be invalid. Pokemon red/blue may write invalid ram bank codes when
    // viewing the town map for some reason. They don't read from RAM with invalid values. Assertions there hold
//#if DEBUG
//    assert(_ramBank <= 3 || (_ramBank >= 0x08 && _ramBank <= 0x0C));
//#endif
}

void MBC3::_latchClockRegisters() {
    const size_t cpuCyclesPerSecond = 1 << 22; // 4.2MHz (2^22)
    const size_t totalSeconds = _clockCount / cpuCyclesPerSecond;
    _clockRegisters[MBC3Clock::RTC_S] = totalSeconds % 60;
    const size_t totalMinutes = totalSeconds / 60;
    _clockRegisters[MBC3Clock::RTC_M] = totalMinutes % 60;
    const size_t totalHours = totalMinutes / 60;
    _clockRegisters[MBC3Clock::RTC_H] = totalHours % 24;
    const size_t totalDays = totalHours / 24;
    _clockRegisters[MBC3Clock::RTC_DL] = (totalDays & 0xFF);
    size_t daysMask = (totalDays & 0x100) >> 8;
    if (totalDays > 0x1FF) {
        // carry bit always stays set until explicitly reset
        daysMask |= 0x80;
    }
    
    _clockRegisters[MBC3Clock::RTC_DH] |= daysMask;
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
        if (_latchVal != val) {
            _latchVal = val;
            if (_latchVal == 1) {
                // we latched the clock. copy the current values
                _latchClockRegisters();
            }
        }
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
    const size_t baseIdx = bankNum * RAMBankSize;
    const size_t ramIdx = baseIdx + (addr - SwitchableRAMBaseAddr);
    return ramIdx;
}

uint8_t MBC3::readRAM(uint16_t addr) const {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return 0xFF;
    }
    if (_ramBank <= 0x03) {
        // one of 4 true ram banks
        const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
        return _ramData[ramIdx];
    } else {
        // clock register
        assert(_ramBank >= 0x08 && _ramBank <= 0x0C);
        int idx = _ramBank - 0x08;
        return _clockRegisters[idx];
    }
}

void MBC3::writeRAM(uint16_t addr, uint8_t val) {
    if (!_ramEnabled || _ramBankCount <= 0) {
        return;
    }
    if (_ramBank <= 0x03) {
        // one of 4 true ram banks
        const size_t ramIdx = _RAMDataIndex(addr, _ramBank);
        if (val != _ramData[ramIdx]) {
            _ramData[ramIdx] = val;
            _isPersistenceStale = _batteryBackup;
        }
    } else {
        // clock register
        assert(_ramBank >= 0x08 && _ramBank <= 0x0C);
        MBC3Clock clockRegister = static_cast<MBC3Clock>(_ramBank - 0x08);
        _writeClockRegister(clockRegister, val);
    }
}

void MBC3::updateClock(size_t cpuCycles) {
    bool running = ((_clockRegisters[MBC3Clock::RTC_DH - MBC3Clock::RTC_S] & 0x40) == 0);
    if (running) {
        _clockCount += cpuCycles;
    }
}

void MBC3::_writeClockRegister(MBC3Clock reg, uint8_t val) {
//    switch (reg) {
//        case MBC3Clock::RTC_S:
//
//    }
    
    // TODO: clock writing. clock writing is weird.
    assert(false);
}

int MBC3::currentROMBank() const {
    return _romBank;
}

size_t MBC3::saveDataSize() const {
    if (_batteryBackup) {
        return _ramBankCount * RAMBankSize;
    } else {
        return 0;
    }
}

void *MBC3::getSaveData() const {
    if (_batteryBackup) {
        return _ramData;
    } else {
        return nullptr;
    }
}

bool MBC3::loadSaveData(const void *saveData, size_t size) {
    if (_batteryBackup) {
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

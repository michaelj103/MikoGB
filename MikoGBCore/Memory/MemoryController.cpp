//
//  MemoryController.cpp
//  MikoGB
//
//  Created on 5/16/21.
//

#include "MemoryController.hpp"
#include "CartridgeHeader.hpp"
#include "MemoryBankController.hpp"
#include <iostream>

using namespace std;
using namespace MikoGB;

static const size_t PermanentROMSize = 1024 * 16;        // 16 KiB from 0x0000 - 0x3FFF
static const uint16_t SwitchableROMBaseAddr = 0x4000;
                                                         // 16 KiB of bank switchable ROM from 0x4000 - 0x7FFF
static const uint16_t VRAMBaseAddr = 0x8000;
static const size_t VRAMSize = 1024 * 8;                 // 8 KiB from 0x8000 - 0x9FFF
static const uint16_t SwitchableRAMBaseAddr = 0xA000;
                                                         // 8 KiB of bank switchable external RAM from 0xA000 - 0xBFFF
static const uint16_t HighRangeMemoryBaseAddr = 0xC000;
static const size_t HighRangeMemorySize = 1024 * 16;     // 16 KiB of internal memory for various uses from 0xC000 - 0xFFFF
//TODO: In CGB mode, there is additional bank switchable RAM in the high range

static void _LogMemoryControllerErr(const string &msg) {
    cerr << "MemoryController Err: " << msg << "\n";
}

bool MemoryController::configureWithROMData(void *romData, size_t size) {
    if (size < PermanentROMSize) {
        _LogMemoryControllerErr("Data is too small to be a valid ROM");
        return false;
    }
    
    if (_permanentROM != nullptr || _videoRAM != nullptr || _highRangeMemory != nullptr || _mbc != nullptr) {
        _LogMemoryControllerErr("Controller should not be reused");
        return false;
    }
    
    // Map the permanent ROM and read the header data from it
    _permanentROM = new uint8_t[PermanentROMSize];
    memcpy(_permanentROM, romData, PermanentROMSize);
    _header.readHeaderData(_permanentROM);
    
    _mbc = MemoryBankController::CreateMBC(_header);
    if (!_mbc) {
        _LogMemoryControllerErr("Unable to create MBC from header data");
        return false;
    }
    
    _videoRAM = new uint8_t[VRAMSize];
    _highRangeMemory = new uint8_t[HighRangeMemorySize];
    
    bool success = _mbc->configureWithROMData(romData, size);
    return success;
}

MemoryController::~MemoryController() {
    delete [] _permanentROM;
    delete [] _videoRAM;
    delete [] _highRangeMemory;
    delete _mbc;
}

uint8_t MemoryController::readByte(uint16_t addr) const {
    if (addr < SwitchableROMBaseAddr) {
        // Read from permanent ROM
        return _permanentROM[addr];
    } else if (addr < VRAMBaseAddr) {
        // Ask MBC to read from switchable ROM
        return _mbc->readROM(addr);
    } else if (addr < SwitchableRAMBaseAddr) {
        // Read from VRAM
        return _videoRAM[addr - VRAMBaseAddr];
    } else if (addr < HighRangeMemoryBaseAddr) {
        // Ask the MBC to read from switchable RAM
        return _mbc->readROM(addr);
    } else {
        // Read from the high range memory
        return _highRangeMemory[addr - HighRangeMemoryBaseAddr];
    }
}

void MemoryController::setByte(uint16_t addr, uint8_t val) {
    if (addr < VRAMBaseAddr) {
        // Write to ROM area means potentially an MBC control code
        _mbc->writeControlCode(addr, val);
    } else if (addr < SwitchableRAMBaseAddr) {
        // Write to VRAM
        _videoRAM[addr - VRAMBaseAddr] = val;
    } else if (addr < HighRangeMemoryBaseAddr) {
        _mbc->writeRAM(addr, val);
    } else {
        // Write to high range memory
        _highRangeMemory[addr - HighRangeMemoryBaseAddr] = val;
    }
}

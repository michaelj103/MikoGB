//
//  MemoryController.cpp
//  MikoGB
//
//  Created on 5/16/21.
//

#include "MemoryController.hpp"
#include "CartridgeHeader.hpp"
#include "MemoryBankController.hpp"
#include "Joypad.hpp"
#include <iostream>

using namespace std;
using namespace MikoGB;

static const size_t BootROMSize = 256;
static const uint8_t BootROM[] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20, 0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

// 48 bytes
static const uint8_t LogoData[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

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

// Relevant registers
static const uint16_t OAMBase = 0xFE00;

// Relevant I/O registers. Writing triggers events
static const uint16_t DMATransferRegister = 0xFF46; //TODO: In CGB, new DMA transfer registers
static const uint16_t BootROMDisableRegister = 0xFF50;
static const uint16_t ControllerDataRegister = 0xFF00;
static const uint16_t DIVRegister = 0xFF04; // Div is basically the CPU cycle count
static const uint16_t TIMARegister = 0xFF05; // Timer counts according to TAC
static const uint16_t TMARegister = 0xFF06; // Timer modulo replaces TIMA when it overflows
static const uint16_t TACRegister = 0xFF07; // Timer control register
static const uint16_t AudioRegisterBegin = 0xFF10; // NR10, lowest audio control register
static const uint16_t AudioRegisterEnd = 0xFF3F; // end of wave pattern RAM. Highest audio control register

static void _LogMemoryControllerErr(const string &msg) {
    cerr << "MemoryController Err: " << msg << "\n";
}

bool MemoryController::configureWithROMData(const void *romData, size_t size) {
    if (size < PermanentROMSize) {
        _LogMemoryControllerErr("Data is too small to be a valid ROM");
        return false;
    }
    
    if (_permanentROM != nullptr || _videoRAM != nullptr || _highRangeMemory != nullptr || _mbc != nullptr) {
        _LogMemoryControllerErr("Controller should not be reused");
        return false;
    }
    
    // Map the permanent ROM and read the header data from it
    _permanentROM = new uint8_t[PermanentROMSize]();
    memcpy(_permanentROM, romData, PermanentROMSize);
    _header.readHeaderData(_permanentROM);
    
    _mbc = MemoryBankController::CreateMBC(_header);
    if (!_mbc) {
        _LogMemoryControllerErr("Unable to create MBC from header data");
        return false;
    }
    
    _videoRAM = new uint8_t[VRAMSize]();
    _highRangeMemory = new uint8_t[HighRangeMemorySize]();
    
    bool success = _mbc->configureWithROMData(romData, size);
    return success;
}

bool MemoryController::configureWithEmptyData() {
    assert(_permanentROM == nullptr && _videoRAM == nullptr && _highRangeMemory == nullptr && _mbc == nullptr);
    _permanentROM = new uint8_t[PermanentROMSize]();
    _videoRAM = new uint8_t[VRAMSize]();
    _highRangeMemory = new uint8_t[HighRangeMemorySize]();
    
    const size_t ptr = 0x104;
    for (size_t i = 0; i < 48; ++i) {
        _permanentROM[ptr + i] = LogoData[i];
    }
    
    return true;
}

MemoryController::~MemoryController() {
    delete [] _permanentROM;
    delete [] _videoRAM;
    delete [] _highRangeMemory;
    delete _mbc;
}

uint8_t MemoryController::readByte(uint16_t addr) const {
    if (addr < SwitchableROMBaseAddr) {
        if (_bootROMEnabled && addr < BootROMSize) {
            return BootROM[addr];
        }
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
        return _mbc->readRAM(addr);
    } else {
        
        if (addr == ControllerDataRegister) {
            if (joypad) {
                return joypad->readJoypadRegister();
            } else {
                // no buttons pressed
                return 0x0F;
            }
        } else if (addr == DIVRegister) {
            return _timer.getDiv();
        } else if (addr == TIMARegister) {
            return _timer.getTIMA();
        } else if (addr >= AudioRegisterBegin && addr <= AudioRegisterEnd) {
            // read from audio controller
            return _audioController.readAudioRegister(addr);
        }
        
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
        
        // Several special events are triggered when writing to the I/O registers in high range memory
        
        if (addr == DMATransferRegister) {
            _dmaTransfer(val);
        } else if (addr == BootROMDisableRegister) {
            _bootROMEnabled = val == 0;
        } else if (addr == DIVRegister) {
            _timer.resetDiv();
            return;
        } else if (addr == TIMARegister) {
            // TODO: what happens when there's a write to TIMA is not specified
            _timer.setTIMA(val);
            return;
        } else if (addr == TMARegister) {
            _timer.setTMA(val);
        } else if (addr == TACRegister) {
            _timer.setTAC(val);
        } else if (addr >= AudioRegisterBegin && addr <= AudioRegisterEnd) {
            // write to audio controller
            _audioController.writeAudioRegister(addr, val);
        }
        
        // Write to high range memory
        _highRangeMemory[addr - HighRangeMemoryBaseAddr] = val;
    }
}

void MemoryController::updateWithCPUCycles(size_t cpuCycles) {
    bool interrupt = _timer.updateWithCPUCycles(cpuCycles);
    if (interrupt) {
        requestInterrupt(TIMA);
    }
    _mbc->updateClock(cpuCycles);
    _audioController.updateWithCPUCycles((int)cpuCycles);
}

void MemoryController::requestInterrupt(InterruptFlag flag) {
    const uint8_t currentRequests = readByte(IFRegister);
    setByte(IFRegister, currentRequests | flag);
}

MemoryController::InputMask MemoryController::selectedInputMask() const {
    uint16_t idx = ControllerDataRegister - HighRangeMemoryBaseAddr;
    uint8_t regVal = _highRangeMemory[idx] & 0x30;
    return static_cast<InputMask>(regVal);
}

void MemoryController::setAudioSampleCallback(AudioSampleCallback callback) {
    _audioController.setSampleCallback(callback);
}

bool MemoryController::isPersistenceStale() const {
    if (_mbc) {
        return _mbc->isPersistenceStale();
    } else {
        return false;
    }
}

void MemoryController::resetPersistence() {
    if (_mbc) {
        _mbc->resetPersistence();
    }
}

int MemoryController::currentROMBank() const {
    return _mbc->currentROMBank();
}

void MemoryController::_dmaTransfer(uint8_t byte) {
    // DMA transfer is a special procedure to write chunks of data to OAM
    // Bytes can be specified from 0x00 - 0xDF (e.g. 0xYY) and a transfer will be performed
    // from 0xYY00 - 0xYY9F -> 0xFE00 - 0xFE9F, the OAM area
    assert(byte <= 0xDF);
    const uint16_t sourceBase = ((uint16_t)byte) << 8;
    const uint16_t toTransfer = 0xA0; // 160 bytes
    
    for (uint16_t i = 0; i < toTransfer; ++i) {
        const uint16_t src = sourceBase + i;
        const uint16_t dst = OAMBase + i;
        setByte(dst, readByte(src));
    }
}

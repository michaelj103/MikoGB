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
#include "SerialController.hpp"
#include "GPUCore.hpp"
#include "BitTwiddlingUtil.h"
#include <iostream>
#include <cassert>

using namespace std;
using namespace MikoGB;

static const size_t BootROMSize = 256;
static const uint8_t BootROM[] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20, 0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};
// boot ROM base is the first 256 bytes and the header is the next 256
// For CGB, the boot ROM is 2048 bytes in total with the header in the middle
static const uint16_t CartridgeHeaderEndAddr = 512;
static const size_t ColorBootROMSize = 2048 + 256;

// 48 bytes
static const uint8_t LogoData[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

static const size_t PermanentROMSize = 1024 * 16;        // 16 KiB from 0x0000 - 0x3FFF
static const uint16_t SwitchableROMBaseAddr = 0x4000;
                                                         // 16 KiB of bank switchable ROM from 0x4000 - 0x7FFF
static const uint16_t VRAMBaseAddr = 0x8000;
static const size_t VRAMSize = 1024 * 8;                 // 8 KiB from 0x8000 - 0x9FFF
static const uint16_t SwitchableRAMBaseAddr = 0xA000;    // 8 KiB of bank switchable external RAM from 0xA000 - 0xBFFF

// 32 KiB of bank switchable internal working ram. Only switchable on CGB
// 0xC000 - 0xCFFF is 4KiB bank 0, always mapped. 0xD000 - 0xDFFF is switchable bank 1-7
static const uint16_t WorkingRAMBaseAddr = 0xC000;
static const uint16_t SwitchableWorkingRAMBaseAddr = 0xD000;
static const uint16_t WorkingRAMSize = 1024 * 32;

static const uint16_t HighRangeMemoryBaseAddr = 0xE000;
static const size_t HighRangeMemorySize = 1024 * 8;     // 8 KiB of internal memory for various uses from 0xE000 - 0xFFFF

// Relevant registers
static const uint16_t OAMBase = 0xFE00;

// Relevant I/O registers. Writing triggers events
static const uint16_t VRAMBankRegister = 0xFF4F; // VRAM bank switch register (CGB only)
static const uint16_t DMATransferRegister = 0xFF46; // DMG DMA control register
static const uint16_t HDMA1Register = 0xFF51; // HDMA source high-order byte
static const uint16_t HDMA2Register = 0xFF52; // HDMA source low-order byte, masked by 0xF0
static const uint16_t HDMA3Register = 0xFF53; // HDMA destination high-order byte, top 3 bits always 0b100
static const uint16_t HDMA4Register = 0xFF54; // HDMA destination low-order byte, masked by 0xF0
static const uint16_t HDMATransferRegister = 0xFF55; // CGB DMA control register
static const uint16_t DoubleSpeedRegister = 0xFF4D;
static const uint16_t WRAMBankRegister = 0xFF70; // WRAM bank switch register (CGB only)
static const uint16_t BootROMDisableRegister = 0xFF50;
static const uint16_t ControllerDataRegister = 0xFF00;
static const uint16_t DIVRegister = 0xFF04; // Div is basically the CPU cycle count
static const uint16_t TIMARegister = 0xFF05; // Timer counts according to TAC
static const uint16_t TMARegister = 0xFF06; // Timer modulo replaces TIMA when it overflows
static const uint16_t TACRegister = 0xFF07; // Timer control register
static const uint16_t AudioRegisterBegin = 0xFF10; // NR10, lowest audio control register
static const uint16_t AudioRegisterEnd = 0xFF3F; // end of wave pattern RAM. Highest audio control register
static const uint16_t SerialDataRegister = 0xFF01; // Byte queued for serial data Rx/Tx
static const uint16_t SerialControlRegister = 0xFF02; // Control bits for serial transfer
static const uint16_t ColorPaletteRegisterBegin = 0xFF68; // BCPS, lowest color palette I/O register
static const uint16_t ColorPaletteRegisterEnd = 0xFF6B; // OCPD, highest color palette I/O register
static const uint16_t ColorCompatibilityRegister = 0xFF4C; // KEY0, color compatibility


static void _LogMemoryControllerErr(const string &msg) {
    cerr << "MemoryController Err: " << msg << "\n";
}

static uint16_t _switchableWorkingRAMAdjustedAddr(uint16_t addr, uint8_t bank) {
    // input: global address within the switchable working RAM area (0xD000 - 0xDFFF)
    // output: offset within the _workingRAM buffer
    // offset by 1024 per bank
    const uint16_t baseOffset = addr - SwitchableWorkingRAMBaseAddr; // (0x0000 - 0x0FFF)
    const uint16_t bankOffset = ((uint16_t)(bank) * (4 * 1024));
    return baseOffset + bankOffset;
}

bool MemoryController::configureWithROMData(const void *romData, size_t size) {
    if (size < PermanentROMSize) {
        _LogMemoryControllerErr("Data is too small to be a valid ROM");
        return false;
    }
    
    if (_permanentROM != nullptr || _videoRAMBank0 != nullptr || _videoRAMBank1 != nullptr || _workingRAM != nullptr || _highRangeMemory != nullptr || _mbc != nullptr || _colorBootROM != nullptr) {
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
    
    _videoRAMBank0 = new uint8_t[VRAMSize]();
    _videoRAMBank1 = new uint8_t[VRAMSize]();
    _videoRAMCurrentBank = _videoRAMBank0;
    _workingRAM = new uint8_t[WorkingRAMSize]();
    _highRangeMemory = new uint8_t[HighRangeMemorySize]();
    
    bool success = _mbc->configureWithROMData(romData, size);
    return success;
}

bool MemoryController::configureWithColorBootROM(const void *bootROMData, size_t size) {
    if (size != ColorBootROMSize) {
        _LogMemoryControllerErr("Color boot ROM is the wrong size");
        return false;
    }
    
    if (_colorBootROM != nullptr) {
        _LogMemoryControllerErr("Initialized color boot ROM multiple times");
        return false;
    }
    
    _colorBootROM = new uint8_t[ColorBootROMSize];
    memcpy(_colorBootROM, bootROMData, ColorBootROMSize);
    _bootROMEnabled = false;
    _colorBootROMEnabled = true;
    
    return true;
}

bool MemoryController::configureWithEmptyData() {
    assert(_permanentROM == nullptr && _videoRAMBank0 == nullptr && _videoRAMBank1 == nullptr && _workingRAM == nullptr && _highRangeMemory == nullptr && _mbc == nullptr && _colorBootROM == nullptr);
    _permanentROM = new uint8_t[PermanentROMSize]();
    _videoRAMBank0 = new uint8_t[VRAMSize]();
    _videoRAMBank1 = new uint8_t[VRAMSize]();
    _videoRAMCurrentBank = _videoRAMBank0;
    _workingRAM = new uint8_t[WorkingRAMSize]();
    _highRangeMemory = new uint8_t[HighRangeMemorySize]();
    
    const size_t ptr = 0x104;
    for (size_t i = 0; i < 48; ++i) {
        _permanentROM[ptr + i] = LogoData[i];
    }
    
    return true;
}

MemoryController::~MemoryController() {
    delete [] _permanentROM;
    delete [] _videoRAMBank0;
    delete [] _videoRAMBank1;
    delete [] _workingRAM;
    delete [] _highRangeMemory;
    delete _mbc;
}

uint8_t MemoryController::readByte(uint16_t addr) const {
    if (addr < SwitchableROMBaseAddr) {
        if (_bootROMEnabled) {
            if (addr < BootROMSize) {
                return BootROM[addr];
            }
        } else if (_colorBootROMEnabled) {
            if (addr < BootROMSize) {
                return _colorBootROM[addr];
            } else if (addr >= CartridgeHeaderEndAddr && addr < ColorBootROMSize) {
                return _colorBootROM[addr];
            }
        }
        // Read from permanent ROM
        return _permanentROM[addr];
    } else if (addr < VRAMBaseAddr) {
        // Ask MBC to read from switchable ROM
        return _mbc->readROM(addr);
    } else if (addr < SwitchableRAMBaseAddr) {
        // Read from VRAM
        return _videoRAMCurrentBank[addr - VRAMBaseAddr];
    } else if (addr < WorkingRAMBaseAddr) {
        // Ask the MBC to read from switchable RAM
        return _mbc->readRAM(addr);
    } else if (addr < SwitchableWorkingRAMBaseAddr) {
        // Read from bank 0 of WRAM
        return _workingRAM[addr - WorkingRAMBaseAddr];
    } else if (addr < HighRangeMemoryBaseAddr) {
        // Read from switchable bank of WRAM
        const uint16_t workingRAMAddr = _switchableWorkingRAMAdjustedAddr(addr, _switchableWRAMBank);
        return _workingRAM[workingRAMAddr];
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
        } else if (addr >= ColorPaletteRegisterBegin && addr <= ColorPaletteRegisterEnd) {
            return gpu->colorPaletteRegisterRead(addr);
        } else if (addr == DoubleSpeedRegister) {
            // high bit is if we're in double-speed mode. Low bit is if a switch has been "prepared"
            const uint8_t speedMask = _doubleSpeedModeEnabled ? 0x80 : 0x00;
            const uint8_t pendingMask = _doubleSpeedModeTogglePending ? 0x01 : 0x00;
            return speedMask | pendingMask;
        }
        
        // Read from the high range memory
        return _highRangeMemory[addr - HighRangeMemoryBaseAddr];
    }
}

uint8_t MemoryController::readVRAMByte(uint16_t addr, int bank) const {
    assert(addr >= VRAMBaseAddr && addr < SwitchableRAMBaseAddr);
    assert(bank == 0 || bank == 1);
    
    if (bank == 0) {
        return _videoRAMBank0[addr - VRAMBaseAddr];
    } else if (bank == 1) {
        return _videoRAMBank1[addr - VRAMBaseAddr];
    }
    
    // Unreachable except by client error
    assert(false);
}

void MemoryController::setByte(uint16_t addr, uint8_t val) {
    if (addr < VRAMBaseAddr) {
        // Write to ROM area means potentially an MBC control code
        _mbc->writeControlCode(addr, val);
    } else if (addr < SwitchableRAMBaseAddr) {
        // Write to VRAM
        _videoRAMCurrentBank[addr - VRAMBaseAddr] = val;
    } else if (addr < WorkingRAMBaseAddr) {
        // Write to switchable external RAM
        _mbc->writeRAM(addr, val);
    } else if (addr < SwitchableWorkingRAMBaseAddr) {
        // Write to bank 0 of working RAM
        _workingRAM[addr - WorkingRAMBaseAddr] = val;
    } else if (addr < HighRangeMemoryBaseAddr) {
        // Write to switchable bank of working RAM
        const uint16_t workingRAMAddr = _switchableWorkingRAMAdjustedAddr(addr, _switchableWRAMBank);
        _workingRAM[workingRAMAddr] = val;
    } else {
        
        // Several special events are triggered when writing to the I/O registers in high range memory
        uint8_t toWrite = val;
        
        if (addr == DMATransferRegister) {
            _dmaTransfer(val);
        } else if (addr == HDMATransferRegister) {
            // Write to HDMA transfer is either a general purpose or H-blank transfer depending on high bit
            if ((val & 0x80) == 0x80) {
                // high bit == 1 starts an H-blank DMA transfer
                _startHBlankDMATransfer();
            } else {
                // high bit == 0 either terminates an in-progress H-Blank DMA transfer or starts a general purpose one
                if (_isHBlankTransferActive) {
                    _isHBlankTransferActive = false;
                } else {
                    _generalPurposeDMATransfer(val);
                    // on completion of DMA transfer, the transfer register becomes 0xFF;
                    toWrite = 0xFF;
                }
            }
        } else if (addr >= HDMA1Register && addr <= HDMA4Register) {
            if (_isHBlankTransferActive) {
                // TODO: This is to verify that this doesn't happen. If it does, I'll need to handle it
                // If it doesn't, the section can be removed
                printf("Modified DMA transfer destinations while in progress\n");
            }
        } else if (addr == VRAMBankRegister) {
            // switch VRAM banks
            if ((val & 0x01) == 0) {
                _videoRAMCurrentBank = _videoRAMBank0;
            } else {
                _videoRAMCurrentBank = _videoRAMBank1;
            }
            toWrite = 0xFE | val; // top 7 bits are 1 when read
        } else if (addr == WRAMBankRegister) {
            // switch WRAM banks
            _switchableWRAMBank = (val & 0x7);
            if (_switchableWRAMBank == 0) {
                // writing 0 selects bank 1
                _switchableWRAMBank = 1;
            }
        } else if (addr == BootROMDisableRegister) {
            const bool enabled = val == 0;
            _bootROMEnabled = enabled;
            _colorBootROMEnabled = enabled;
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
        } else if (addr == SerialDataRegister) {
            serialController->serialDataWillWrite(val);
        } else if (addr == SerialControlRegister) {
            serialController->serialControlWillWrite(val);
        } else if (addr == DoubleSpeedRegister) {
            if (isMaskSet(val, 0x01)) {
                _doubleSpeedModeTogglePending = true;
            }
        } else if (addr >= ColorPaletteRegisterBegin && addr <= ColorPaletteRegisterEnd) {
            gpu->colorPaletteRegisterWrite(addr, val);
        } else if (addr == ColorCompatibilityRegister) {
            gpu->colorModeRegisterWrite(val);
        }
        
        // Write to high range memory
        _directSetHighRange(addr, toWrite);
    }
}

void MemoryController::_directSetHighRange(uint16_t addr, uint8_t val) {
    _highRangeMemory[addr - HighRangeMemoryBaseAddr] = val;
}

void MemoryController::updateWithCPUCycles(size_t cpuCycles) {
    bool interrupt = _timer.updateWithCPUCycles(cpuCycles);
    if (interrupt) {
        requestInterrupt(TIMA);
    }
    const size_t audioCycles = _doubleSpeedModeEnabled ? cpuCycles : cpuCycles * 2;
    _audioController.updateWithCPUCycles((int)audioCycles);
    serialController->updateWithCPUCycles((int)cpuCycles);
}

void MemoryController::updateWithRealTimeSeconds(size_t secondsElapsed) {
    _mbc->updateClock(secondsElapsed);
}

bool MemoryController::toggleDoubleSpeedModeIfNecessary() {
    if (!_doubleSpeedModeTogglePending) {
        return false;
    }
    _doubleSpeedModeTogglePending = false;
    _doubleSpeedModeEnabled = !_doubleSpeedModeEnabled;
    return true;
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

bool MemoryController::isClockPersistenceStale() const {
    if (_mbc) {
        return _mbc->isClockPersistenceStale();
    } else {
        return false;
    }
}

void MemoryController::resetClockPersistence() {
    if (_mbc) {
        _mbc->resetClockPersistence();
    }
}

int MemoryController::currentROMBank() const {
    return _mbc->currentROMBank();
}

// DMG general-purpose DMA transfer
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

// CGB general-purpose DMA transfer
void MemoryController::_generalPurposeDMATransfer(uint8_t byte) {
    // In CGB, there's a new general purpose DMA transfer mechanism supported that allows more precision
    // and allows transfer from ROM (not sure if anybody uses the previous mechanism to transfer from ROM?)
    const uint8_t sourceHigh = readByte(HDMA1Register);
    const uint8_t sourceLow = readByte(HDMA2Register) & 0xF0;
    
    // destination bits are masked so they are in the range 0x8000 - 0x9FF0
    const uint8_t dstHigh = (readByte(HDMA3Register) & 0x1F) | 0x80; // top 3 bits replaced by 0b100
    const uint8_t dstLow = readByte(HDMA4Register) & 0xF0;
    
    const uint16_t sourceBase = (((uint16_t)sourceHigh) << 8) | sourceLow;
    const uint16_t dstBase = (((uint16_t)dstHigh) << 8) | dstLow;
    
    // amount to transfer is low-7 bits in the HDMA control register plus 1 times 16.
    // Result is in the range of 16 - 2048
    const uint16_t toTransfer = (((uint16_t)(byte & 0x7F)) + 1) << 4;
    for (uint16_t i = 0; i < toTransfer; ++i) {
        const uint16_t src = sourceBase + i;
        const uint16_t dst = dstBase + i;
        setByte(dst, readByte(src));
    }
}

void MemoryController::_startHBlankDMATransfer() {
    _isHBlankTransferActive = true;
    const uint8_t sourceHigh = readByte(HDMA1Register);
    const uint8_t sourceLow = readByte(HDMA2Register) & 0xF0;
    
    // destination bits are masked so they are in the range 0x8000 - 0x9FF0
    const uint8_t dstHigh = (readByte(HDMA3Register) & 0x1F) | 0x80; // top 3 bits replaced by 0b100
    const uint8_t dstLow = readByte(HDMA4Register) & 0xF0;
    
    const uint16_t sourceBase = (((uint16_t)sourceHigh) << 8) | sourceLow;
    const uint16_t dstBase = (((uint16_t)dstHigh) << 8) | dstLow;
    _hBlankTransferSource = sourceBase;
    _hBlankTransferDst = dstBase;
}

// CGB H-blank DMA transfer. Executes one step (16-byte transfer)
void MemoryController::hBlankDMATransferStep() {
    // In CGB, there's an H-blank DMA transfer mechanism that allows very fast transfers during each HBlank
    // until a the counter in the control register underflows, or transfer is cancelled
    if (!_isHBlankTransferActive) {
        return;
    }
    
    const uint16_t sourceBase = _hBlankTransferSource;
    const uint16_t dstBase = _hBlankTransferDst;
    for (uint16_t i = 0; i < 16; ++i) {
        const uint16_t src = sourceBase + i;
        const uint16_t dst = dstBase + i;
        setByte(dst, readByte(src));
    }
    
    // Next step will transfer 16 bytes to/from the next 16 byte window
    // TODO: Is this the correct approach or do programs move the pointers themselves?
    _hBlankTransferSource += 16;
    _hBlankTransferDst += 16;
    
    const uint8_t remainingCount = readByte(HDMATransferRegister) & 0x7F;
    if (remainingCount > 0) {
        // decrement the remaining step count
        const uint8_t newRemainingCount = remainingCount - 1;
        _directSetHighRange(HDMATransferRegister, (0x80 | newRemainingCount));
    } else {
        // the transfer is complete
        _isHBlankTransferActive = false;
        _directSetHighRange(HDMATransferRegister, 0xFF);
    }
}

size_t MemoryController::saveDataSize() const {
    if (_mbc) {
        return _mbc->saveDataSize();
    } else {
        return 0;
    }
}

size_t MemoryController::copySaveData(void *buffer, size_t size) const {
    if (!_mbc) {
        return 0;
    }
    
    size_t dataSize = _mbc->saveDataSize();
    if (size < dataSize) {
        // don't copy if the buffer is too small
        return 0;
    }
    
    void *saveData = _mbc->getSaveData();
    memcpy(buffer, saveData, dataSize);
    return dataSize;
}

bool MemoryController::loadSaveData(const void *saveData, size_t size) {
    if (!_mbc) {
        return false;
    }
    
    return _mbc->loadSaveData(saveData, size);
}

size_t MemoryController::clockDataSize() const {
    if (_mbc) {
        return _mbc->clockDataSize();
    } else {
        return 0;
    }
}

size_t MemoryController::copyClockData(void *buffer, size_t size) const {
    if (_mbc) {
        return _mbc->copyClockData(buffer, size);
    } else {
        return 0;
    }
}

bool MemoryController::loadClockData(const void *saveData, size_t size) {
    if (!_mbc) {
        return false;
    }
    
    return _mbc->loadClockData(saveData, size);
}

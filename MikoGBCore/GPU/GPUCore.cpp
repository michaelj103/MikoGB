//
//  GPUCore.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GPUCore.hpp"
#include "CPUCore.hpp"

using namespace MikoGB;

// A note on display timing
// For now, assume a constant 456 cycles per scanline as measured in TCAGBD and other sources
// It's likely close, and hopefully not many games (if any) have super strict timing assumptions.
// Especially since SGB runs a little differently
// What is documented in the official manual is that the CPU speed is 1.05MHz and that an instruction cycle is 0.954 µs with
// a source oscillation of 4.1943MHz.
// It's also documented in a diagram (Chapter 2, section 1.5) that the LCD driver spends 108.7µs per line
// and that V-blank lasts 1.09ms (10 lines). The first matches well to 456 oscillations per line (108.7µs and change)
// And obviously 10x that is ~1.09ms. All together that means that 154 lines (0-153) would finish 59.7 times per second
// This is the documented refresh rate of the screen
static const size_t CPUCyclesPerScanline = 456;
static const size_t LCDScanlineCount = 154; // 0-153. 144-153 are V-Blank
static const size_t VBlankScanline = 144;

// Scanlines are broken up into 4 modes: 0 - H-Blank, 1 - V-Blank, 2 - Searching OAM, 3 - Transferring to LCD
// Each normal scanline cycles through 2, 3, 0 at some regular cadence (undocumented in the manual)
// Lines 144-153 are in V-Blank (1) the whole time. Timings measured in TCAGBD are:
// 84 cycles in OAM (2)
// 364 cycles in Transfer (3)
// 8 cycles in H-Blank (0)
// Starting there, but we may have to fudge the H-Blank timing to be a longer portion since that's the point where
// VRAM may be accessed and games may wait and try to squeeze work in there. If it's actually so short, probably not. To test
static const size_t OAMCycles = 84;
static const size_t LCDTransferCycles = 364;
static const size_t HBlankCycles = 8;

// Important memory locations
static const uint16_t LCDCRegister = 0xFF40; // LCD Control
static const uint16_t LCDStatRegister = 0xFF41; // LCD Status
static const uint16_t SCYRegister = 0xFF42; // BG scroll Y coordinate
static const uint16_t SCXRegister = 0xFF43; // BG scroll X coordinate
static const uint16_t LYRegister = 0xFF44; // Current scanline y
static const uint16_t LYCRegister = 0xFF45; // LY Compare register (for interrupts)

static const uint16_t BGPRegister = 0xFF47; // BG Palette data
static const uint16_t OBP0Register = 0xFF48; // OBJ Palette 0 data
static const uint16_t OBP1Register = 0xFF49; // OBJ Palette 1 data

static inline bool _IsLCDOn(const CPUCore *core) {
    bool isOn = (core->getMemory(LCDCRegister) & 0x80) == 0x80;
    return isOn;
}

// Clear all state as needed when the LCD is disabled
void GPUCore::_turnOff() {
    _cycleCount = 0;
    _currentScanline = 0;
    _currentMode = HBlank;
    _cpu->setMemory(LYRegister, 0);
    uint8_t stat = _cpu->getMemory(LCDStatRegister);
    _cpu->setMemory(LCDStatRegister, stat & 0xF8); // clear low 3 bits of STAT
}

void GPUCore::_incrementScanline() {
    _currentScanline = (_currentScanline + 1) % LCDScanlineCount;
    _cpu->setMemory(LYRegister, _currentScanline);
    
    //TODO: LYC interrupt if enabled
//    if (_currentScanline == _cpu->getMemory(LYCRegister)) {
//
//    }
}

void GPUCore::_setMode(LCDMode mode) {
    if (_currentMode == mode) {
        return;
    }
    _currentMode = mode;
    
    // Update the STAT register to reflect the new mode
    const uint8_t currentStat = _cpu->getMemory(LCDStatRegister);
    uint8_t updatedStat = (currentStat & 0xFC) | mode;
    _cpu->setMemory(LCDStatRegister, updatedStat);
    
    if (mode == HBlank) {
        // When we hit HBlank actually do the work of figuring out the line and notifying client
        _processScanline(_currentScanline);
    }
    
    //TODO: interrupts for vblank and stat modes
}

void GPUCore::updateWithCPUCycles(size_t cpuCycles) {
    bool isOn = _IsLCDOn(_cpu);
    if (!isOn) {
        if (_wasOn) {
            _turnOff();
        }
    } else {
        if (!_wasOn) {
            // start mode
            _setMode(OAMScan);
        }
        _cycleCount += cpuCycles;
        
        bool done = false;
        while (!done) {
            switch (_currentMode) {
                case HBlank:
                    if (_cycleCount >= HBlankCycles) {
                        _cycleCount -= HBlankCycles;
                        // go to next line and transition to VBlank if 
                        _incrementScanline();
                        if (_currentScanline == VBlankScanline) {
                            _setMode(VBlank);
                        } else {
                            _setMode(OAMScan);
                        }
                    } else {
                        done = true;
                    }
                    break;
                case OAMScan:
                    if (_cycleCount >= OAMCycles) {
                        _cycleCount -= OAMCycles;
                        _setMode(LCDTransfer);
                    } else {
                        done = true;
                    }
                    break;
                case LCDTransfer:
                    if (_cycleCount >= LCDTransferCycles) {
                        _cycleCount -= LCDTransferCycles;
                        _setMode(HBlank);
                    } else {
                        done = true;
                    }
                    break;
                case VBlank:
                    if (_cycleCount >= CPUCyclesPerScanline) {
                        _cycleCount -= CPUCyclesPerScanline;
                        _incrementScanline();
                        if (_currentScanline == 0) {
                            // V-Blank is over. Start over for scanline 0
                            _setMode(OAMScan);
                        }
                    } else {
                        done = true;
                    }
                    break;
            }
        }
    }
    
    _wasOn = isOn;
}

void GPUCore::_processScanline(uint8_t line) {
    //TODO: Figure out the pixels for the current scanline
}

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

static inline Pixel _PixelForCode(uint8_t code) {
    switch (code) {
        case 0:
            // White
            return Pixel(0xFF);
        case 1:
            // Light Gray
            return Pixel(0xBF);
        case 2:
            // Dark Gray
            return Pixel(0x40);
        case 3:
            // Black
            return Pixel(0x00);
        default:
            return Pixel();
    }
}

static void _ReadBGPalette(Pixel *palette, const CPUCore *cpu) {
    const uint8_t paletteVal = cpu->getMemory(BGPRegister);
    palette[0] = _PixelForCode(paletteVal & 0x03);
    palette[1] = _PixelForCode((paletteVal & 0x0C) >> 2);
    palette[2] = _PixelForCode((paletteVal & 0x30) >> 4);
    palette[3] = _PixelForCode((paletteVal & 0xC0) >> 6);
}

static void _ReadBGTile(uint16_t addr, const CPUCore *cpu, const Pixel *bgPalette, PixelBuffer &dest) {
    assert(dest.width == 8 && dest.height == 8);
    for (uint16_t y = 0; y < 16; y += 2) {
        uint8_t byte0 = cpu->getMemory(addr + y);
        uint8_t byte1 = cpu->getMemory(addr + y + 1);
        for (int x = 0; x < 8; ++x) {
            int shift = 8 - x - 1;
            uint8_t code = ((byte0 >> shift) & 0x1) | ((byte1 >> (shift-1)) & 0x2);
            const Pixel &px = bgPalette[code];
            size_t idx = dest.indexOf(x, y/2);
            dest.pixels[idx] = px;
        }
    }
}

static void _DrawPixelBufferToBuffer(const PixelBuffer &source, PixelBuffer &dest, size_t x, size_t y) {
    //TODO: No-op and make this a debug assertion?
    assert(x < dest.width && y < dest.height);
    
    for (size_t sy = 0; sy < source.height; ++sy) {
        const size_t dy = y + sy;
        if (dy >= dest.height) {
            break;
        }
        
        for (size_t sx = 0; sx < source.width; ++sx) {
            const size_t dx = x + sx;
            if (dx >= dest.width) {
                break;
            }
            
            const size_t sIdx = source.indexOf(sx, sy);
            const size_t dIdx = dest.indexOf(dx, dy);
            dest.pixels[dIdx] = source.pixels[sIdx];
        }
    }
}

void GPUCore::getTileMap(PixelBufferImageCallback callback) {
    // For now, just support the 8x8 BG tile map, so 256 tiles. Draw into 16x16 square
    const size_t tilesPerRow = 16;
    const size_t tileWidth = 8;
    const size_t pixelWidth = (tilesPerRow * tileWidth) + (tilesPerRow - 1);
    PixelBuffer tileMap(pixelWidth, pixelWidth);
    
    const uint8_t lcdc = _cpu->getMemory(LCDCRegister);
    
    // Range of tiles is either 0x9000 with codes being signed offsets (0x8800-0x97FF)
    // or they start at 0x8000 with codes being unsigned offsets (0x8000-0x8FFF)
    int32_t bgBase = 0x9000;
    bool signedMode = true;
    if (isMaskSet(lcdc, 0x10)) {
        bgBase = 0x8000;
    }
    
    Pixel bgPalette[4];
    _ReadBGPalette(bgPalette, _cpu);
    
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i <= 0xFF; ++i) {
        const uint8_t code = i & 0xFF;
        uint16_t addr;
        if (signedMode) {
            addr = (uint16_t)(bgBase + ((int8_t)code * 16));
        } else {
            addr = (uint16_t)(bgBase + (code * 16));
        }
        _ReadBGTile(addr, _cpu, bgPalette, tileBuffer);
        
        const size_t tileX = i % tilesPerRow;
        const size_t tileY = i / tilesPerRow;
        const size_t pixelX = tileX * (tileWidth + 1);
        const size_t pixelY = tileY * (tileWidth + 1);
        _DrawPixelBufferToBuffer(tileBuffer, tileMap, pixelX, pixelY);
    }
    
    callback(tileMap);
}

void GPUCore::getBackground(PixelBufferImageCallback callback) {
    const size_t bgCanvasSize = 256;
    PixelBuffer background(bgCanvasSize, bgCanvasSize);
    const uint8_t lcdc = _cpu->getMemory(LCDCRegister);
    
    // Range of tiles is either 0x9000 with codes being signed offsets (0x8800-0x97FF)
    // or they start at 0x8000 with codes being unsigned offsets (0x8000-0x8FFF)
    int32_t bgBase = 0x9000;
    bool signedMode = true;
    if (isMaskSet(lcdc, 0x10)) {
        bgBase = 0x8000;
    }
    
    uint16_t bgCodeArea = 0x9800;
    if (isMaskSet(lcdc, 0x8)) {
        bgCodeArea = 0x9C00;
    }
    
    Pixel bgPalette[4];
    _ReadBGPalette(bgPalette, _cpu);
    
    const uint16_t tileSide = 8;
    const uint16_t TilesPerRow = 32;
    const uint16_t NumberOfBGCodes = 1024; //1024: 32x32 tiles form the background
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i < NumberOfBGCodes; ++i) {
        const uint8_t code = _cpu->getMemory(bgCodeArea + i);
        uint16_t tileBaseAddress;
        if (signedMode) {
            tileBaseAddress = (uint16_t)(bgBase + ((int8_t)code * 16));
        } else {
            tileBaseAddress = (uint16_t)(bgBase + (code * 16));
            
        }
        _ReadBGTile(tileBaseAddress, _cpu, bgPalette, tileBuffer);
        const size_t tileX = i % TilesPerRow;
        const size_t tileY = i / TilesPerRow;
        const size_t pixelX = tileX * tileSide;
        const size_t pixelY = tileY * tileSide;
        _DrawPixelBufferToBuffer(tileBuffer, background, pixelX, pixelY);
    }
    
    callback(background);
}

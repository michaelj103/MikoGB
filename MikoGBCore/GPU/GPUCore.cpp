//
//  GPUCore.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GPUCore.hpp"
#include "MemoryController.hpp"
#include "BitTwiddlingUtil.h"

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
// According to The Ultimate Game Boy Talk, the counts are a bit different:
// 20 clocks (80 cycles) in OAM (2)
// 43+ clocks (172 cycles) in transfer (3)
// 51- clocks (204 cycles) in H-blank (0)
// Which makes more sense since H-blank needs to be long enough to do meaningful computation
// Transfer can take longer if there's window and/or sprites on the line, but longer may be better for emulation? to test
static const size_t OAMCycles = 80;
static const size_t LCDTransferCycles = 172;
static const size_t HBlankCycles = 204;

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

static const size_t ScreenWidth = 160; // screen is 160x144
static const size_t BackgroundCanvasSize = 256; // 256x256;
static const uint8_t BackgroundTileSize = 8; // BG tiles are always 8x8
static const uint16_t BackgroundTilesPerRow = 32; // BG canvas is 32x32 tiles for 256x256 px
static const uint16_t BackgroundTileBytes = 16; // BG tiles are 16 bytes, 2bpp

static inline bool _IsLCDOn(const MemoryController *mem) {
    bool isOn = (mem->readByte(LCDCRegister) & 0x80) == 0x80;
    return isOn;
}

GPUCore::GPUCore(MemoryController *mem): _memoryController(mem), _scanline(ScreenWidth, 1) {}

// Clear all state as needed when the LCD is disabled
void GPUCore::_turnOff() {
    _cycleCount = 0;
    _currentScanline = 0;
    _currentMode = HBlank;
    _memoryController->setByte(LYRegister, 0);
    uint8_t stat = _memoryController->readByte(LCDStatRegister);
    _memoryController->setByte(LCDStatRegister, stat & 0xF8); // clear low 3 bits of STAT
}

void GPUCore::_incrementScanline() {
    _currentScanline = (_currentScanline + 1) % LCDScanlineCount;
    _memoryController->setByte(LYRegister, _currentScanline);
    
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
    const uint8_t currentStat = _memoryController->readByte(LCDStatRegister);
    uint8_t updatedStat = (currentStat & 0xFC) | mode;
    _memoryController->setByte(LCDStatRegister, updatedStat);
    
    if (mode == HBlank) {
        // When we hit HBlank actually do the work of rendering the line and notifying client
        _renderScanline(_currentScanline);
    }
    
    //TODO: interrupts for vblank and stat modes
}

void GPUCore::updateWithCPUCycles(size_t cpuCycles) {
    bool isOn = _IsLCDOn(_memoryController);
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

static void _ReadBGPalette(Pixel *palette, const MemoryController *mem) {
    const uint8_t paletteVal = mem->readByte(BGPRegister);
    palette[0] = _PixelForCode(paletteVal & 0x03);
    palette[1] = _PixelForCode((paletteVal & 0x0C) >> 2);
    palette[2] = _PixelForCode((paletteVal & 0x30) >> 4);
    palette[3] = _PixelForCode((paletteVal & 0xC0) >> 6);
}

static void _GetBGTileMapInfo(int32_t &baseAddr, bool &signedMode, uint16_t &codeArea, const MemoryController *mem) {
    const uint8_t lcdc = mem->readByte(LCDCRegister);
    // Range of background tiles is either 0x9000 with codes being signed offsets (0x8800-0x97FF)
    // or they start at 0x8000 with codes being unsigned offsets (0x8000-0x8FFF)
    baseAddr = 0x9000;
    signedMode = true;
    if (isMaskSet(lcdc, 0x10)) {
        baseAddr = 0x8000;
        signedMode = false;
    }
    
    // Codes are 1024 bytes starting at one of 2 addresses
    codeArea = 0x9800;
    if (isMaskSet(lcdc, 0x8)) {
        codeArea = 0x9C00;
    }
}

static uint16_t _GetBGTileBaseAddress(int32_t bgTileMapBase, uint8_t tileIdx, bool signedMode) {
    if (signedMode) {
        const uint16_t tileBase = (uint16_t)(bgTileMapBase + ((int8_t)tileIdx * BackgroundTileBytes));
        return tileBase;
    } else {
        const uint16_t tileBase = (uint16_t)(bgTileMapBase + (tileIdx * BackgroundTileBytes));
        return tileBase;
    }
}

static void _ReadBGTile(uint16_t addr, const MemoryController *mem, const Pixel *bgPalette, PixelBuffer &dest) {
    assert(dest.width == 8 && dest.height == 8);
    for (uint16_t y = 0; y < 16; y += 2) {
        uint8_t byte0 = mem->readByte(addr + y);
        uint8_t byte1 = mem->readByte(addr + y + 1);
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
    const size_t pixelWidth = (tilesPerRow * BackgroundTileSize) + (tilesPerRow - 1);
    PixelBuffer tileMap(pixelWidth, pixelWidth);
        
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t bgCodeArea;
    _GetBGTileMapInfo(bgTileMapBase, signedMode, bgCodeArea, _memoryController);
    
    Pixel bgPalette[4];
    _ReadBGPalette(bgPalette, _memoryController);
    
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i <= 0xFF; ++i) {
        const uint8_t code = i & 0xFF;
        const uint16_t addr = _GetBGTileBaseAddress(bgTileMapBase, code, signedMode);
        _ReadBGTile(addr, _memoryController, bgPalette, tileBuffer);
        
        const size_t tileX = i % tilesPerRow;
        const size_t tileY = i / tilesPerRow;
        const size_t pixelX = tileX * (BackgroundTileSize + 1);
        const size_t pixelY = tileY * (BackgroundTileSize + 1);
        _DrawPixelBufferToBuffer(tileBuffer, tileMap, pixelX, pixelY);
    }
    
    callback(tileMap);
}

void GPUCore::getBackground(PixelBufferImageCallback callback) {
    PixelBuffer background(BackgroundCanvasSize, BackgroundCanvasSize);
    
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t bgCodeArea;
    _GetBGTileMapInfo(bgTileMapBase, signedMode, bgCodeArea, _memoryController);
    
    Pixel bgPalette[4];
    _ReadBGPalette(bgPalette, _memoryController);
    
    const uint16_t NumberOfBGCodes = 1024; //1024: 32x32 tiles form the background
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i < NumberOfBGCodes; ++i) {
        const uint8_t code = _memoryController->readByte(bgCodeArea + i);
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, code, signedMode);
        _ReadBGTile(tileBaseAddress, _memoryController, bgPalette, tileBuffer);
        const size_t tileX = i % BackgroundTilesPerRow;
        const size_t tileY = i / BackgroundTilesPerRow;
        const size_t pixelX = tileX * BackgroundTileSize;
        const size_t pixelY = tileY * BackgroundTileSize;
        _DrawPixelBufferToBuffer(tileBuffer, background, pixelX, pixelY);
    }
    
    callback(background);
}

static uint8_t _DrawTileRowToScanline(uint16_t tileAddress, uint8_t tileRow, uint8_t tileCol, uint8_t scanlinePos, PixelBuffer &scanline, const MemoryController *mem, const Pixel *bgPalette) {
    // the 2 bytes representing the given row in the tile
    const uint16_t tileRowOffset = tileRow * 2; // 2 bytes per row
    const uint8_t byte0 = mem->readByte(tileAddress + tileRowOffset);
    const uint8_t byte1 = mem->readByte(tileAddress + tileRowOffset + 1);
    uint8_t currentIdx = scanlinePos;
    for (int x = tileCol; x < 8; ++x) {
        int shift = 8 - x - 1;
        uint8_t code = ((byte0 >> shift) & 0x1) | ((byte1 >> (shift-1)) & 0x2);
        const Pixel &px = bgPalette[code];
        scanline.pixels[currentIdx] = px;
        currentIdx++;
    }
    
    return currentIdx - scanlinePos;
}

void GPUCore::_renderBackgroundToScanline(size_t lineNum, PixelBuffer &scanline) {
    // 1. Read relevant info for drawing the background of the current line
    const uint8_t scx = _memoryController->readByte(SCXRegister);
    const uint8_t scy = _memoryController->readByte(SCYRegister);
    
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t bgCodeArea;
    _GetBGTileMapInfo(bgTileMapBase, signedMode, bgCodeArea, _memoryController);
    
    Pixel bgPalette[4];
    _ReadBGPalette(bgPalette, _memoryController);
    
    // 2. Figure out what row of tile codes we need to draw and which row of those tiles is relevant
    const uint8_t bgY = (lineNum + scy) & 0xFF; // wrap around
    const uint8_t bgTileY = bgY / 8;
    const uint8_t tileRow = bgY % 8; // the row in the 8x8 tile that is on this line
    
    // 3. Main loop, draw background tiles progressively to the scanline
    uint8_t pixelsDrawn = 0;
    while (pixelsDrawn < ScreenWidth) {
        // 3a. Figure out the next tile to draw, determine it's code from the code area, then it's address in the map
        const uint8_t bgX = (pixelsDrawn + scx) & 0xFF;
        const uint8_t bgTileX = bgX / 8;
        const uint16_t tileCodeAddress = bgCodeArea + (bgTileY * BackgroundTilesPerRow) + bgTileX;
        const uint8_t tileCode = _memoryController->readByte(tileCodeAddress);
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, tileCode, signedMode);
        
        // 3b. Now draw the line from the tile to the scanline using the helper
        const uint8_t tileCol = bgX % 8; // for all but the first tile, this should be 0
        pixelsDrawn += _DrawTileRowToScanline(tileBaseAddress, tileRow, tileCol, pixelsDrawn, scanline, _memoryController, bgPalette);
    }
}

void GPUCore::_renderScanline(size_t lineNum) {
    _renderBackgroundToScanline(lineNum, _scanline);
    
    if (_scanlineCallback) {
        _scanlineCallback(_scanline, lineNum);
    }
}

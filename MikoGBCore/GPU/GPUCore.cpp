//
//  GPUCore.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GPUCore.hpp"
#include "BitTwiddlingUtil.h"
#include "MonochromePalette.hpp"
#include "ColorPalette.hpp"
#include "GPUTypes.hpp"
#include <array>
#include <cassert>

using namespace MikoGB;
using namespace std;

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
// Finally, total cycles are doubled. In normal-speed mode, input cycles are multiplied by 2, so the x2 cancels
// In double-speed mode, input is not multiplied. This means that twice as many CPU cycles must elapse in double-speed mode
// which counteracts the fact that the CPU would be running twice as fast and keeps the framerate at "real" time
static const size_t CPUCyclesPerScanline = 456 * 2;
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
// Finally, total cycles are doubled. See double-speed note above
static const size_t OAMCycles = 80 * 2;
static const size_t LCDTransferCycles = 172 * 2;
static const size_t HBlankCycles = 204 * 2;

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
static const uint16_t WYRegister = 0xFF4A; // Window origin Y
static const uint16_t WXRegister = 0xFF4B; // Window origin X
static const uint16_t OAMBase = 0xFE00; // base address of the 40 4-byte OAM codes
static const uint16_t TileMapBase = 0x8000; // Base address of tile map

static const uint16_t BCPSRegister = 0xFF68; // BG palette I/O control register
static const uint16_t BCPDRegister = 0xFF69; // BG palette data register
static const uint16_t OCPSRegister = 0xFF6A; // BG palette I/O register
static const uint16_t OCPDRegister = 0xFF6B; // BG palette data register

static const size_t ScreenWidth = 160; // screen is 160x144
static const size_t ScreenHeight = 144;
static const size_t BackgroundCanvasSize = 256; // 256x256;
static const uint8_t BackgroundTileSize = 8; // BG tiles are always 8x8
static const uint16_t BackgroundTilesPerRow = 32; // BG canvas is 32x32 tiles for 256x256 px
static const uint16_t BackgroundTileBytes = 16; // BG tiles are 16 bytes, 2bpp

static inline bool _IsLCDOn(const MemoryController::Ptr &mem) {
    bool isOn = (mem->readByte(LCDCRegister) & 0x80) == 0x80;
    return isOn;
}

GPUCore::GPUCore(MemoryController::Ptr &mem): _memoryController(mem), _scanline(ScreenWidth) {
    // ensure color palettes are default initialized
    for (auto &palette : _colorPaletteBG) {
        palette = ColorPalette();
    }
    for (auto &palette : _colorPaletteOBJ) {
        palette = ColorPalette();
    }
}

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
    
    bool doesMatchLYC = _currentScanline == _memoryController->readByte(LYCRegister);
    const uint8_t currentStat = _memoryController->readByte(LCDStatRegister);
    const uint8_t matchFlagMask = 0x04;
    bool didMatchLYC = isMaskSet(currentStat, matchFlagMask);
    if (doesMatchLYC && !didMatchLYC) {
        // New match, set the match flag and trigger interrupt if enabled
        uint8_t updatedStat = currentStat | matchFlagMask;
        _memoryController->setByte(LCDStatRegister, updatedStat);
        
        bool LYCIntEnabled = isMaskSet(currentStat, 0x40);
        if (LYCIntEnabled) {
            _memoryController->requestInterrupt(MemoryController::LCDStat);
        }
    } else if (!doesMatchLYC && didMatchLYC) {
        // No longer a match. Reset the match flag
        uint8_t updatedStat = currentStat & ~(matchFlagMask);
        _memoryController->setByte(LCDStatRegister, updatedStat);
    }
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
    
    switch (mode) {
        case HBlank:
            _renderScanline(_currentScanline);
            _memoryController->hBlankDMATransferStep();
            if (isMaskSet(updatedStat, 0x08)) {
                _memoryController->requestInterrupt(MemoryController::LCDStat);
            }
            break;
        case VBlank:
            _memoryController->requestInterrupt(MemoryController::VBlank);
            if (isMaskSet(updatedStat, 0x10)) {
                _memoryController->requestInterrupt(MemoryController::LCDStat);
            }
            break;
        case OAMScan:
            if (isMaskSet(updatedStat, 0x20)) {
                _memoryController->requestInterrupt(MemoryController::LCDStat);
            }
            break;
        case LCDTransfer:
            break;
    }
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

#pragma mark - BG Utilities

static void _GetBGTileMapInfo(int32_t &baseAddr, bool &signedMode, uint16_t &codeArea, const MemoryController::Ptr &mem) {
    const uint8_t lcdc = mem->readByte(LCDCRegister);
    // Range of background tiles is either 0x9000 with codes being signed offsets (0x8800-0x97FF)
    // or they start at 0x8000 with codes being unsigned offsets (0x8000-0x8FFF)
    baseAddr = 0x9000;
    signedMode = true;
    if (isMaskSet(lcdc, 0x10)) {
        baseAddr = TileMapBase;
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

static const Palette _GetBGPalette(const TileAttributes attrs, const MonochromePalette &monoPalette, const ColorPalette *colorPalettes, GPUCore::ColorRenderingMode renderingMode) {
    switch (renderingMode) {
        case GPUCore::ColorRenderingMode::DMGOnly:
            return monoPalette;
        case GPUCore::ColorRenderingMode::CGBMode:
            return colorPalettes[attrs.colorPaletteIndex];
        case GPUCore::ColorRenderingMode::DMGCompatibility:
            return ColorPalette(colorPalettes[0], monoPalette.paletteByte);
    }
    
    // Unreachable
    assert(false);
}

static const Palette _GetOBJPalette(const TileAttributes attrs, const MonochromePalette *monoPalettes, const ColorPalette *colorPalettes, GPUCore::ColorRenderingMode renderingMode) {
    switch (renderingMode) {
        case GPUCore::ColorRenderingMode::DMGOnly:
            return monoPalettes[attrs.dmgPaletteIndex];
        case GPUCore::ColorRenderingMode::CGBMode:
            return colorPalettes[attrs.colorPaletteIndex];
        case GPUCore::ColorRenderingMode::DMGCompatibility:
            return ColorPalette(colorPalettes[attrs.dmgPaletteIndex], monoPalettes[attrs.dmgPaletteIndex].paletteByte);
    }
    
    // Unreachable
    assert(false);
}

static inline uint8_t _GetPaletteCode(uint8_t byte0, uint8_t byte1, int x) {
    int shift = 8 - x - 1;
    const uint8_t lowBit = (byte0 >> shift) & 0x01;
    const uint8_t highBit = (byte1 >> shift) & 0x01;
    const uint8_t code = (highBit << 1) | lowBit;
    return code;
}

static void _ReadBGTile(uint16_t addr, const MemoryController::Ptr &mem, const Palette &bgPalette, const TileAttributes &attr, PixelBuffer &dest) {
    assert(dest.width == 8 && dest.height == 8);
    for (uint16_t y = 0; y < 16; y += 2) {
        const uint8_t byte0 = mem->readVRAMByte(addr + y, attr.characterBank);
        const uint8_t byte1 = mem->readVRAMByte(addr + y + 1, attr.characterBank);
        for (int x = 0; x < 8; ++x) {
            const uint8_t code = _GetPaletteCode(byte0, byte1, x);
            const Pixel &px = bgPalette.pixelForCode(code);
            if ((px.red != 0xFF && px.red != 0x00) || (px.blue != 0xFF && px.blue != 0x00) || (px.green != 0xFF && px.green != 0x00)) {
                printf("Non white\n");
            }
            const size_t idx = dest.indexOf(x, y/2);
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
    const uint8_t bgPaletteByte = _memoryController->readByte(BGPRegister);
    MonochromePalette bgPalette = MonochromePalette(bgPaletteByte);
    
    const TileAttributes attr = TileAttributes(0);
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i <= 0xFF; ++i) {
        const uint8_t code = i & 0xFF;
        const uint16_t addr = _GetBGTileBaseAddress(bgTileMapBase, code, signedMode);
        _ReadBGTile(addr, _memoryController, bgPalette, attr, tileBuffer);
        
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
    const uint8_t bgPaletteByte = _memoryController->readByte(BGPRegister);
    MonochromePalette monoPalette = MonochromePalette(bgPaletteByte);
    
    const bool isCGBRendering = _renderingMode == ColorRenderingMode::CGBMode;
    const uint16_t NumberOfBGCodes = 1024; //1024: 32x32 tiles form the background
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i < NumberOfBGCodes; ++i) {
        const uint16_t tileCodeAddr = bgCodeArea + i;
        const uint8_t code = _memoryController->readVRAMByte(tileCodeAddr, 0);
        const uint8_t attrByte = isCGBRendering ? _memoryController->readVRAMByte(tileCodeAddr, 1) : 0;
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, code, signedMode);
        const TileAttributes bgAttributes = TileAttributes(attrByte);
        const Palette finalPalette = _GetBGPalette(bgAttributes, monoPalette, _colorPaletteBG, _renderingMode);
        const size_t tileX = i % BackgroundTilesPerRow;
        const size_t tileY = i / BackgroundTilesPerRow;
        const size_t pixelX = tileX * BackgroundTileSize;
        const size_t pixelY = tileY * BackgroundTileSize;
        _ReadBGTile(tileBaseAddress, _memoryController, finalPalette, bgAttributes, tileBuffer);
        _DrawPixelBufferToBuffer(tileBuffer, background, pixelX, pixelY);
    }
    
    callback(background);
}

static uint8_t _DrawTileRowToScanline(uint16_t tileAddress, uint8_t tileRow, uint8_t tileCol, const TileAttributes &attributes, LCDScanline::WriteType writeType, uint8_t scanlinePos, LCDScanline &scanline, const MemoryController::Ptr &mem, const Palette &palette) {
    // the 2 bytes representing the given row in the tile
    const uint16_t tileRowOffset = tileRow * 2; // 2 bytes per row
    const uint8_t byte0 = mem->readVRAMByte(tileAddress + tileRowOffset, attributes.characterBank);
    const uint8_t byte1 = mem->readVRAMByte(tileAddress + tileRowOffset + 1, attributes.characterBank);
    int x = tileCol;
    uint8_t currentIdx = scanlinePos;
    // Draw until the end of the tile or the end of the scanline
    const size_t width = scanline.getWidth();
    while (currentIdx < width && x < BackgroundTileSize) {
        const int adjustedX = attributes.flipX ? BackgroundTileSize - x - 1 : x;
        const uint8_t code = _GetPaletteCode(byte0, byte1, adjustedX);
        scanline.writePixel(currentIdx, code, palette, writeType);
        ++currentIdx;
        ++x;
    }
    
    return currentIdx - scanlinePos;
}

void GPUCore::_renderBackgroundToScanline(size_t lineNum, LCDScanline &scanline) {
    const bool isCGBRendering = _renderingMode == ColorRenderingMode::CGBMode;
    const uint8_t lcdc = _memoryController->readByte(LCDCRegister);
    if (!isCGBRendering && !isMaskSet(lcdc, 0x01)) {
        // BG off is only valid for DMG mode. Behavior is white but sprites can't be layered under it, so transparent
        scanline.writeBlankBG();
        return;
    }
    
    // 1. Read relevant info for drawing the background of the current line
    const uint8_t scx = _memoryController->readByte(SCXRegister);
    const uint8_t scy = _memoryController->readByte(SCYRegister);
    
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t bgCodeArea;
    _GetBGTileMapInfo(bgTileMapBase, signedMode, bgCodeArea, _memoryController);
    const uint8_t bgPaletteByte = _memoryController->readByte(BGPRegister);
    const MonochromePalette bgPalette = MonochromePalette(bgPaletteByte);
    
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
        const uint8_t tileCode = _memoryController->readVRAMByte(tileCodeAddress, 0);
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, tileCode, signedMode);
        
        const uint8_t tileAttr = isCGBRendering ? _memoryController->readVRAMByte(tileCodeAddress, 1) : 0;
        const TileAttributes bgAttributes = TileAttributes(tileAttr);
        const uint8_t adjustedRow = bgAttributes.flipY ? BackgroundTileSize - tileRow - 1 : tileRow;
        const LCDScanline::WriteType writeType = bgAttributes.priorityToBG ? LCDScanline::WriteType::BackgroundPrioritizeBG : LCDScanline::WriteType::BackgroundDeferToObj;
        const Palette finalPalette = _GetBGPalette(bgAttributes, bgPalette, _colorPaletteBG, _renderingMode);
        
        // 3b. Now draw the line from the tile to the scanline using the helper
        const uint8_t tileCol = bgX % 8; // for all but the first tile, this should be 0
        pixelsDrawn += _DrawTileRowToScanline(tileBaseAddress, adjustedRow, tileCol, bgAttributes, writeType, pixelsDrawn, scanline, _memoryController, finalPalette);
    }
#if DEBUG
    assert(pixelsDrawn == 160);
#endif
}

#pragma mark - Window Utilities

static bool _windowStatus(int32_t &baseAddr, bool &signedMode, uint16_t &codeArea, const MemoryController::Ptr &mem) {
    const uint8_t lcdc = mem->readByte(LCDCRegister);
    bool windowEnabled = isMaskSet(lcdc, 0x20);
    // Range of background tiles is either 0x9000 with codes being signed offsets (0x8800-0x97FF)
    // or they start at 0x8000 with codes being unsigned offsets (0x8000-0x8FFF)
    baseAddr = 0x9000;
    signedMode = true;
    if (isMaskSet(lcdc, 0x10)) {
        baseAddr = TileMapBase;
        signedMode = false;
    }
    
    // Codes are 1024 bytes starting at one of 2 addresses
    // note window is specified in bit 6 (0x40) while background was specified at bit 3 (0x8)
    codeArea = 0x9800;
    if (isMaskSet(lcdc, 0x40)) {
        codeArea = 0x9C00;
    }
    return windowEnabled;
}

void GPUCore::_renderWindowToScanline(size_t lineNum, LCDScanline &scanline) {
    // 1. read relevant info
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t winCodeArea;
    bool windowEnabled = _windowStatus(bgTileMapBase, signedMode, winCodeArea, _memoryController);
    if (!windowEnabled) {
        // not enabled, nothing to do
        return;
    }
    const uint8_t wx = _memoryController->readByte(WXRegister);
    const uint8_t wy = _memoryController->readByte(WYRegister);
    if (wy > lineNum || wx >= ScreenWidth + 7) {
        // window doesn't start until after this scanline. nothing to do
        return;
    }
    const uint8_t bgPaletteByte = _memoryController->readByte(BGPRegister);
    const MonochromePalette bgPalette = MonochromePalette(bgPaletteByte);
    
    // 2. Figure out what row of tile codes we need to draw and which row of those tiles is relevant
    const uint8_t winY = lineNum - wy;
    const uint8_t bgTileY = winY / 8; // y index of the bg tile in the tilemap
    const uint8_t tileRow = winY % 8; // the row in the 8x8 tile that is on this line
    
    // 3. Main loop, draw background tiles progressively to the scanline
    // window can potentially draw off the screen to the left by 7 px if wx < 7, so add 7 to the width for drawing purposes
    uint8_t screenPosition = wx >= 7 ? wx - 7 : 0;
    uint8_t windowPosition = wx < 7 ? 7 - wx : 0;
    
    const bool isCGBRendering = _renderingMode == ColorRenderingMode::CGBMode;
    while (screenPosition < ScreenWidth) {
        // 3a. Figure out the next tile to draw, determine its code from the code area, then its address in the map
        const uint8_t winX = windowPosition;
        const uint8_t bgTileX = winX / 8;
        const uint16_t tileCodeAddress = winCodeArea + (bgTileY * BackgroundTilesPerRow) + bgTileX;
        const uint8_t tileCode = _memoryController->readVRAMByte(tileCodeAddress, 0);
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, tileCode, signedMode);
        
        const uint8_t tileAttr = isCGBRendering ? _memoryController->readVRAMByte(tileCodeAddress, 1) : 0;
        const TileAttributes winAttributes = TileAttributes(tileAttr);
        const uint8_t adjustedRow = winAttributes.flipY ? BackgroundTileSize - tileRow - 1 : tileRow;
        const LCDScanline::WriteType writeType = winAttributes.priorityToBG ? LCDScanline::WriteType::WindowPrioritizeBG : LCDScanline::WriteType::WindowDeferToObj;
        const Palette finalPalette = _GetBGPalette(winAttributes, bgPalette, _colorPaletteBG, _renderingMode);
        
        // 3b. Now draw the line from the tile to the scanline using the helper
        const uint8_t tileCol = windowPosition % 8;
        const uint8_t pixelsDrawn = _DrawTileRowToScanline(tileBaseAddress, adjustedRow, tileCol, winAttributes, writeType, screenPosition, scanline, _memoryController, finalPalette);
        screenPosition += pixelsDrawn;
        windowPosition += pixelsDrawn;
    }
#if DEBUG
    assert(screenPosition == 160);
#endif
}

void GPUCore::getWindow(PixelBufferImageCallback callback) {
    // Show the window at screen size with + 14 on the width and + 7 on the height because:
    // on width, the window can start 7px before the first pixel
    // on both, a tile overflow up to 7px (8x8 px and only 1px needs to be on screen
    PixelBuffer window(ScreenWidth + 14, ScreenHeight + 7);
    const Pixel uninitializedPixel = Pixel();
    for (size_t y = 0; y < window.height; ++y) {
        for (size_t x = 0; x < window.width; ++x) {
            const size_t idx = (y * window.width) + x;
            window.pixels[idx] = uninitializedPixel;
        }
    }
    
    // 1. read relevant info
    int32_t bgTileMapBase;
    bool signedMode;
    uint16_t winCodeArea;
    bool windowEnabled = _windowStatus(bgTileMapBase, signedMode, winCodeArea, _memoryController);
    if (!windowEnabled) {
        // not enabled, nothing to do
        callback(window);
        return;
    }
    MonochromePalette monoPalette = MonochromePalette(_memoryController->readByte(BGPRegister));
    const uint8_t wx = _memoryController->readByte(WXRegister);
    const uint8_t wy = _memoryController->readByte(WYRegister);
    
    const bool isCGBRendering = _renderingMode == ColorRenderingMode::CGBMode;
    const uint16_t NumberOfWindowCodes = 1024; //1024: 32x32 tiles form the window
    PixelBuffer tileBuffer(8, 8);
    for (uint16_t i = 0; i < NumberOfWindowCodes; ++i) {
        const uint16_t tileCodeAddr = winCodeArea + i;
        const uint8_t code = _memoryController->readVRAMByte(tileCodeAddr, 0);
        const uint8_t attrByte = isCGBRendering ? _memoryController->readVRAMByte(tileCodeAddr, 1) : 0;
        const uint16_t tileBaseAddress = _GetBGTileBaseAddress(bgTileMapBase, code, signedMode);
        const TileAttributes bgAttributes = TileAttributes(attrByte);
        const Palette finalPalette = _GetBGPalette(bgAttributes, monoPalette, _colorPaletteBG, _renderingMode);
        const size_t tileX = i % BackgroundTilesPerRow;
        const size_t tileY = i / BackgroundTilesPerRow;
        const size_t pixelX = wx + tileX * BackgroundTileSize;
        const size_t pixelY = wy + tileY * BackgroundTileSize;
        if (pixelX >= ScreenWidth || pixelY >= ScreenHeight) {
            // off screen, don't bother
            continue;
        }
        _ReadBGTile(tileBaseAddress, _memoryController, finalPalette, bgAttributes, tileBuffer);
        _DrawPixelBufferToBuffer(tileBuffer, window, pixelX, pixelY);
    }
    
    callback(window);
}

#pragma mark - Sprite Utilities

static bool _IsSpriteOnLine(size_t line, size_t spriteY, size_t spriteHeight) {
    bool onLine = (spriteY <= line && (spriteY + spriteHeight) > line);
    return onLine;
}

void GPUCore::_renderSpritesToScanline(size_t line, LCDScanline &scanline) {
    // 1. Read relevant display info for drawing sprites
    const uint8_t lcdc = _memoryController->readByte(LCDCRegister);
    if (!isMaskSet(lcdc, 0x02)) {
        // OBJ off
        return;
    }
    const size_t spriteWidth = BackgroundTileSize;
    const bool doubleHeightMode = isMaskSet(lcdc, 0x04);
    const size_t spriteHeight = doubleHeightMode ? BackgroundTileSize * 2 : BackgroundTileSize;
    
    // 2. Z-order priority. In DMG mode it's lowest X-pos with OAM code as the tiebreaker
    // In CGB mode it's just lowest OAM code. Always using CGB Z-order mechanism for simplicity
    // TODO: DMG compatibility priority based on OPRI register?
    // For both, only 10 sprites are drawn per line
    array<int, 10> oamCodesOnLine;
    const size_t currentSpriteLine = line + 16; // sprite y-coords are offset by 16 so they can be hidden above the screen
    int numSpritesOnLine = 0;
    for (int i = 0; i < 40; ++i) {
        const uint16_t codeBase = OAMBase + (i * 4);
        // first byte is y-coord
        uint8_t spriteY = _memoryController->readByte(codeBase);
        if (_IsSpriteOnLine(currentSpriteLine, spriteY, spriteHeight)) {
            oamCodesOnLine[numSpritesOnLine] = i;
            numSpritesOnLine++;
            if (numSpritesOnLine >= 10) {
                break;
            }
        }
    }
    
    // No sprites with pixels on this line, nothing else to do
    if (numSpritesOnLine == 0) {
        return;
    }
    
    // 3. Get palettes
    MonochromePalette monoPalettes[2] = {
        MonochromePalette(_memoryController->readByte(OBP0Register)),
        MonochromePalette(_memoryController->readByte(OBP1Register))
    };
    
    // 4. In reverse z-order, draw the sprites
    const uint8_t chrCodeMask = doubleHeightMode ? 0xFE : 0xFF; // in double-height, ignore least significant bit
    for (int i = numSpritesOnLine - 1; i >= 0; --i) {
        const int oamCode = oamCodesOnLine[i];
        const uint16_t codeBase = OAMBase + (oamCode * 4);
        const uint8_t spriteX = _memoryController->readByte(codeBase + 1);
        if (spriteX == 0 || spriteX >= 168) {
            // off screen sprite
            continue;
        }
        const uint8_t spriteY = _memoryController->readByte(codeBase);
        const uint8_t chrCode = _memoryController->readByte(codeBase + 2) & chrCodeMask;
        const uint8_t attrByte = _memoryController->readByte(codeBase + 3);
        const TileAttributes spriteAttr = TileAttributes(attrByte);
        const LCDScanline::WriteType writeType = spriteAttr.priorityToBG ? LCDScanline::WriteType::ObjectLow : LCDScanline::WriteType::ObjectHigh;

        const uint16_t tileBaseAddr = TileMapBase + (chrCode * BackgroundTileBytes);
        const uint8_t tileRow = currentSpriteLine - spriteY;
        const uint8_t adjustedRow = spriteAttr.flipY ? spriteHeight - tileRow - 1 : tileRow;
        const uint8_t tileCol = spriteX < spriteWidth ? spriteWidth - spriteX : 0;
        const uint8_t scanlinePos = spriteX >= spriteWidth ? spriteX - spriteWidth : 0;
        const Palette finalPalette = _GetOBJPalette(spriteAttr, monoPalettes, _colorPaletteOBJ, _renderingMode);
        _DrawTileRowToScanline(tileBaseAddr, adjustedRow, tileCol, spriteAttr, writeType, scanlinePos, scanline, _memoryController, finalPalette);
    }
    
}

void GPUCore::_renderScanline(size_t lineNum) {
    _scanline.clear();
    _renderBackgroundToScanline(lineNum, _scanline);
    _renderWindowToScanline(lineNum, _scanline);
    _renderSpritesToScanline(lineNum, _scanline);
    
    if (_scanlineCallback) {
        _scanlineCallback(_scanline.getCompositedPixelData(), lineNum);
    }
}

#pragma mark - Color Palette Management

void GPUCore::colorModeRegisterWrite(uint8_t val) {
    if (val == 0x04) {
        _renderingMode = ColorRenderingMode::DMGCompatibility;
    }
    // TODO: Handle other values?
}

// returns the palette index to read from or write to based on the control value.
// Updates the control value if it's a write
static int _paletteControlIndex(const uint8_t controlValue) {
    const int index = (controlValue & 0x38) >> 3;
    return index;
}

static uint8_t _incrementedPaletteControlRegister(const uint8_t controlValue) {
    uint8_t outValue = controlValue;
    if (isMaskSet(controlValue, 0x80)) {
        outValue = (controlValue + 1) & 0xBF; // mask out bit 6. It will "overflow" when the bottom 6 bits wrap
    }
    return outValue;
}

void GPUCore::colorPaletteRegisterWrite(uint16_t addr, uint8_t val) {
    if (addr == BCPSRegister) {
        _bgPaletteControl = val & 0xBF; // mask out bit 6 so it's always 0
    } else if (addr == BCPDRegister) {
        const int index = _paletteControlIndex(_bgPaletteControl);
        _colorPaletteBG[index].paletteDataWrite(_bgPaletteControl, val);
        _bgPaletteControl = _incrementedPaletteControlRegister(_bgPaletteControl);
    } else if (addr == OCPSRegister) {
        _objPaletteControl = val; // mask out bit 6 so it's always 0
    } else if (addr == OCPDRegister) {
        const int index = _paletteControlIndex(_objPaletteControl);
        _colorPaletteOBJ[index].paletteDataWrite(_objPaletteControl, val);
        _objPaletteControl = _incrementedPaletteControlRegister(_objPaletteControl);
    } else {
        // Should be unreachable except by client error
        assert(false);
    }
}

uint8_t GPUCore::colorPaletteRegisterRead(uint16_t addr) const {
    if (addr == BCPSRegister) {
        return _bgPaletteControl;
    } else if (addr == BCPDRegister) {
        const int index = _paletteControlIndex(_bgPaletteControl);
        return _colorPaletteBG[index].paletteDataRead(_bgPaletteControl);
    } else if (addr == OCPSRegister) {
        return _objPaletteControl;
    } else if (addr == OCPDRegister) {
        const int index = _paletteControlIndex(_objPaletteControl);
        return _colorPaletteOBJ[index].paletteDataRead(_objPaletteControl);
    } else {
        // Should be unreachable except by client error
        assert(false);
        return 0xFF;
    }
}

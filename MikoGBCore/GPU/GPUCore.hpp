//
//  GPUCore.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GPUCore_hpp
#define GPUCore_hpp

#include <cstdlib>
#include "PixelBuffer.hpp"
#include "MemoryController.hpp"
#include "LCDScanline.hpp"
#include "ColorPalette.hpp"

#define ColorPaletteCount 8

namespace MikoGB {

class GPUCore {
public:
    GPUCore(MemoryController::Ptr &);
    using Ptr = std::shared_ptr<GPUCore>;
    
    /// Expects CPU oscillation cycles (~4.2MHz, 4 per instruction cycle)
    void updateWithCPUCycles(size_t cpuCycles);
    
    void setScanlineCallback(PixelBufferScanlineCallback callback) {
        _scanlineCallback = callback;
    }
    
    uint8_t getCurrentScanline() {
        return _currentScanline;
    }
    
    void enableCGBRendering() { _renderingMode = ColorRenderingMode::CGBMode; }
    void colorModeRegisterWrite(uint8_t val); /// Writes to the KEY0 register indicating color mode
    void colorPaletteRegisterWrite(uint16_t addr, uint8_t val);
    uint8_t colorPaletteRegisterRead(uint16_t addr) const;
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    void getBackground(PixelBufferImageCallback callback);
    
private:
    enum LCDMode : uint8_t {
        HBlank = 0,
        VBlank = 1,
        OAMScan = 2,
        LCDTransfer = 3,
    };
    
    MemoryController::Ptr &_memoryController;
    size_t _cycleCount = 0;
    uint8_t _currentScanline = 0;
    void _incrementScanline();
    
    LCDMode _currentMode = OAMScan;
    void _setMode(LCDMode mode);
    
    bool _wasOn = false;
    void _turnOff();
    
    LCDScanline _scanline;
    void _renderScanline(size_t line);
    void _renderBackgroundToScanline(size_t line, LCDScanline &scanline);
    void _renderWindowToScanline(size_t line, LCDScanline &scanline);
    void _renderSpritesToScanline(size_t line, LCDScanline &scanline);
    PixelBufferScanlineCallback _scanlineCallback;
    
    // Color palettes
    uint8_t _bgPaletteControl = 0;
    uint8_t _objPaletteControl = 0;
    ColorPalette _colorPaletteBG[ColorPaletteCount];
    ColorPalette _colorPaletteOBJ[ColorPaletteCount];
    
    enum class ColorRenderingMode {
        DMGOnly,
        DMGCompatibility,
        CGBMode,
    };
    ColorRenderingMode _renderingMode = ColorRenderingMode::DMGOnly;
};

}

#endif /* GPUCore_hpp */

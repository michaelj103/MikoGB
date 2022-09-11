//
//  LCDScanline.hpp
//  MikoGB
//
//  Created on 6/17/21.
//

#ifndef LCDScanline_hpp
#define LCDScanline_hpp

#include "PixelBuffer.hpp"
#include "Palette.hpp"
#include <vector>

namespace MikoGB {

struct LCDScanline {
    LCDScanline(size_t width) : _pixelData(width, 1), _bgPixelData(width, 1), _bgPriority(width, InternalPriority::Undefined), _objPixelData(width, 1), _objPriority(width, InternalPriority::Undefined) {}
    
    size_t getWidth() const { return _pixelData.width; }
    
    void clear() {
        Pixel px = Pixel();
        for (int i = 0; i < _pixelData.width; ++i) {
            _pixelData.pixels[i] = px;
            _bgPixelData.pixels[i] = px;
            _bgPriority[i] = InternalPriority::Undefined;
            _objPixelData.pixels[i] = px;
            _objPriority[i] = InternalPriority::Undefined;
        }
    }
    
    enum class WriteType {
        BackgroundDeferToObj,
        BackgroundPrioritizeBG,
        WindowDeferToObj,
        WindowPrioritizeBG,
        ObjectLow,
        ObjectHigh,
    };
    
    void writeBlankBG() {
        const Pixel px = Pixel(0xFF, 0xFF, 0xFF);
        for (size_t i = 0; i < getWidth(); ++i) {
            _bgPixelData.pixels[i] = px;
            _bgPriority[i] = InternalPriority::Transparent;
        }
    }
    
    void writePixel(size_t idx, uint8_t code, const Palette &palette, WriteType writeType) {
        const Pixel &px = palette.pixelForCode(code);
        const bool isTransparentPixel = (code & 0x3) == 0;
        switch (writeType) {
            case WriteType::BackgroundDeferToObj:
                _bgPixelData.pixels[idx] = px;
                _bgPriority[idx] = isTransparentPixel ? InternalPriority::Transparent : InternalPriority::Low;
                break;
            case WriteType::BackgroundPrioritizeBG:
                _bgPixelData.pixels[idx] = px;
                _bgPriority[idx] = isTransparentPixel ? InternalPriority::Transparent : InternalPriority::High;
                break;
            case WriteType::WindowDeferToObj:
                _bgPixelData.pixels[idx] = px;
                _bgPriority[idx] = InternalPriority::Low;
                break;
            case WriteType::WindowPrioritizeBG:
                _bgPixelData.pixels[idx] = px;
                _bgPriority[idx] = InternalPriority::High;
                break;
            case WriteType::ObjectLow:
                if (_objPriority[idx] == InternalPriority::Undefined || !isTransparentPixel) {
                    _objPixelData.pixels[idx] = px;
                    _objPriority[idx] = isTransparentPixel ? InternalPriority::Transparent : InternalPriority::Low;
                }
                break;
            case WriteType::ObjectHigh:
                if (_objPriority[idx] == InternalPriority::Undefined || !isTransparentPixel) {
                    _objPixelData.pixels[idx] = px;
                    _objPriority[idx] = isTransparentPixel ? InternalPriority::Transparent : InternalPriority::High;
                }
                break;
        }
    }
    
    // See section 2.4 in the GB programmer manual for details on compositing BG and OBJ pixels
    const PixelBuffer &getCompositedPixelData() {
        for (int i = 0; i < _pixelData.width; ++i) {
            InternalPriority objPriority = _objPriority[i];
            InternalPriority bgPriority = _bgPriority[i];
            if (objPriority == InternalPriority::Undefined) {
                _pixelData.pixels[i] = _bgPixelData.pixels[i];
            } else if (bgPriority == InternalPriority::Undefined) {
                _pixelData.pixels[i] = _objPixelData.pixels[i];
            } else if (objPriority == InternalPriority::Transparent) {
                // if OBJ is transparent, BG always wins, even if transparent
                _pixelData.pixels[i] = _bgPixelData.pixels[i];
            } else {
                if (bgPriority == InternalPriority::Transparent) {
                    // If the OBJ is non-transparent and the BG is transparent, then OBJ always wins
                    _pixelData.pixels[i] = _objPixelData.pixels[i];
                } else if (bgPriority == InternalPriority::High) {
                    // BG takes priority always if it did not defer to OBJ and neither is transparent
                    _pixelData.pixels[i] = _bgPixelData.pixels[i];
                } else {
                    // Neither is transparent and BG deferred, so use the OBJ priority
                    if (objPriority == InternalPriority::High) {
                        _pixelData.pixels[i] = _objPixelData.pixels[i];
                    } else {
                        _pixelData.pixels[i] = _bgPixelData.pixels[i];
                    }
                }
            }
        }
        
        return _pixelData;
    }
    
private:
    enum class InternalPriority {
        Undefined,
        Transparent,
        Low,
        High
    };
    
    PixelBuffer _pixelData;
    PixelBuffer _bgPixelData;
    std::vector<InternalPriority> _bgPriority;
    PixelBuffer _objPixelData;
    std::vector<InternalPriority> _objPriority;
};

}

#endif /* LCDScanline_hpp */

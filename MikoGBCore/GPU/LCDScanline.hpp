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
    LCDScanline(size_t width) : _pixelData(width, 1), _bgPixelData(width, 1), _bgPriority(width, InternalPriority::Transparent), _objPixelData(width, 1), _objPriority(width, InternalPriority::Transparent) {}
    
    size_t getWidth() const { return _pixelData.width; }
    
    void clear() {
        Pixel px = Pixel();
        for (int i = 0; i < _pixelData.width; ++i) {
            _pixelData.pixels[i] = px;
            _bgPixelData.pixels[i] = px;
            _bgPriority[i] = InternalPriority::Transparent;
            _objPixelData.pixels[i] = px;
            _objPriority[i] = InternalPriority::Transparent;
        }
    }
    
    enum class WriteType {
        BackgroundDeferToObj,
        BackgroundPrioritizeBG,
        ObjectLow,
        ObjectHigh,
    };
        
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
            case WriteType::ObjectLow:
                if (!isTransparentPixel) {
                    _objPixelData.pixels[idx] = px;
                    _objPriority[idx] = isTransparentPixel ? InternalPriority::Transparent : InternalPriority::Low;
                }
                break;
            case WriteType::ObjectHigh:
                if (!isTransparentPixel) {
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
            if (objPriority == InternalPriority::Transparent) {
                // if OBJ is transparent, BG always wins, even if transparent
                _pixelData.pixels[i] = _bgPixelData.pixels[i];
            } else {
                InternalPriority bgPriority = _bgPriority[i];
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

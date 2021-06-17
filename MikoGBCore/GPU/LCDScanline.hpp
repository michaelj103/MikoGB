//
//  LCDScanline.hpp
//  MikoGB
//
//  Created on 6/17/21.
//

#ifndef LCDScanline_hpp
#define LCDScanline_hpp

#include "PixelBuffer.hpp"
#include "MonochromePalette.hpp"
#include <vector>

namespace MikoGB {

struct LCDScanline {
    LCDScanline(size_t width) : _lowPriorityBG(width, false), _pixelData(width, 1) {}
    
    const PixelBuffer &getPixelData() const { return _pixelData; }
    size_t getWidth() const { return _pixelData.width; }
    
    enum class WriteType {
        Background,
        SpriteLowPriority,
        SpriteHighPriority,
    };
    
    void writePixel(size_t idx, uint8_t code, const MonochromePalette &palette, WriteType writeType) {
        if (!palette.isTransparent(code)) {
            bool shouldWrite = true;
            switch (writeType) {
                case WriteType::Background:
                    _lowPriorityBG[idx] = (code & 0x3) == 0;
                    break;
                case WriteType::SpriteLowPriority:
                    // low priority sprites only draw over low priority background
                    shouldWrite = _lowPriorityBG[idx];
                    break;
                case WriteType::SpriteHighPriority:
                    break;
            }
            if (shouldWrite) {
                const Pixel &px = palette.pixelForCode(code);
                _pixelData.pixels[idx] = px;
            }
        }
    }
    
private:
    /// If a pixel is low priority BG, then sprites go over it whether their low or high priority
    std::vector<bool> _lowPriorityBG; // if true, low priority sprites go over it. Otherwise they don't
    //TODO: Indicate that the BG tile here is always over OBJs (CGB feature)
    //std::vector<bool> _highPriorityBG;
    PixelBuffer _pixelData;
};

}

#endif /* LCDScanline_hpp */

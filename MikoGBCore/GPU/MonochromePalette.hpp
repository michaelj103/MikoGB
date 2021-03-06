//
//  MonochromePalette.hpp
//  MikoGB
//
//  Created on 5/26/21.
//

#ifndef MonochromePalette_hpp
#define MonochromePalette_hpp

#include "PixelBuffer.hpp"

namespace MikoGB {

struct MonochromePalette {
    MonochromePalette(uint8_t p, bool allowTransparent): _transparent(allowTransparent) {
        _palette[0] = allowTransparent ? MikoGB::Pixel() : _PixelForCode(p & 0x03);
        _palette[1] = _PixelForCode((p & 0x0C) >> 2);
        _palette[2] = _PixelForCode((p & 0x30) >> 4);
        _palette[3] = _PixelForCode((p & 0xC0) >> 6);
    }
    
    const MikoGB::Pixel &pixelForCode(uint8_t code) const {
        const uint8_t maskedCode = code & 0x3;
        return _palette[maskedCode];
    }
    
    bool isTransparent(uint8_t code) const {
        return _transparent && (code & 0x3) == 0;
    }
    
private:
    bool _transparent;
    MikoGB::Pixel _palette[4];
    
    static MikoGB::Pixel _PixelForCode(uint8_t code) {
        switch (code) {
            case 0:
                // White
                return MikoGB::Pixel(0xFF);
            case 1:
                // Light Gray
                return MikoGB::Pixel(0xBF);
            case 2:
                // Dark Gray
                return MikoGB::Pixel(0x40);
            case 3:
                // Black
                return MikoGB::Pixel(0x00);
            default:
                return MikoGB::Pixel();
        }
    }
};

}

#endif /* MonochromePalette_hpp */

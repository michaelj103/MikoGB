//
//  MonochromePalette.hpp
//  MikoGB
//
//  Created on 5/26/21.
//

#ifndef MonochromePalette_hpp
#define MonochromePalette_hpp

#include "Palette.hpp"

namespace MikoGB {

struct MonochromePalette : public Palette {
    MonochromePalette(uint8_t p): paletteByte(p) {
        _palette[0] = _PixelForCode(p & 0x03);
        _palette[1] = _PixelForCode((p & 0x0C) >> 2);
        _palette[2] = _PixelForCode((p & 0x30) >> 4);
        _palette[3] = _PixelForCode((p & 0xC0) >> 6);
        
        // No code translation for monochrome
        _translation = { 0, 1, 2, 3 };
    }
    
    const uint8_t paletteByte;
    
private:
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

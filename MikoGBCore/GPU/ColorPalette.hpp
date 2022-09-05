//
//  ColorPalette.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 9/4/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef ColorPalette_hpp
#define ColorPalette_hpp

#include "Palette.hpp"

namespace MikoGB {

struct ColorPalette : public Palette {
    ColorPalette() {
        _transparent = false;
        _palette[0] = Pixel();
        _palette[1] = Pixel();
        _palette[2] = Pixel();
        _palette[3] = Pixel();
    }
    
    ColorPalette(const ColorPalette &other, bool wantsTransparent) {
        _transparent = wantsTransparent;
        _palette[0] = wantsTransparent ? Pixel() : other._palette[0];
        _palette[1] = other._palette[1];
        _palette[2] = other._palette[2];
        _palette[3] = other._palette[3];
    }
    
    void paletteDataWrite(uint8_t control, uint8_t data) {
        // bottom 3 bits of control indicate which pixel and which byte (high or low)
        // bit 0 is high(1) byte or low (0) byte
        // bit 1-2 is pixel index
        const uint8_t pixelIndex = (control & 0x7) >> 1;
        const bool isHigh = (control & 0x1) == 0x1;
        const uint16_t pixelData = _pixelData[pixelIndex];
        if (isHigh) {
            _pixelData[pixelIndex] = (pixelData & 0x00FF) | (((uint16_t)data) << 8);
        } else {
            _pixelData[pixelIndex] = (pixelData & 0xFF00) | ((uint16_t)data);
        }
        
        _updatePixelForIndex(pixelIndex);
    }
    
    uint8_t paletteDataRead(uint8_t control) const {
        // bottom 3 bits of control indicate which pixel and which byte (high or low)
        // bit 0 is high(1) byte or low (0) byte
        // bit 1-2 is pixel index
        const uint8_t pixelIndex = (control & 0x7) >> 1;
        const bool isHigh = (control & 0x1) == 0x1;
        const uint16_t pixelData = _pixelData[pixelIndex];
        if (isHigh) {
            return (pixelData & 0xFF00) >> 8;
        } else {
            return (pixelData & 0x00FF);
        }
    }
    
private:
    uint16_t _pixelData[4];
    
    void _updatePixelForIndex(uint8_t pixelIndex) {
        const uint16_t pixelData = _pixelData[pixelIndex];
        
        // Red is the low 5 bits
        const int red5 = (pixelData & 0x1F);
        const uint8_t red = ((red5 * 255) / 31) & 0xFF;
        
        // Green is the middle 5 bits
        const int green5 = (pixelData & 0x3E0) >> 5;
        const uint8_t green = ((green5 * 255) / 31) & 0xFF;
        
        // Blue is the upper 5 bits (highest bit ignored)
        const int blue5 = (pixelData & 0x7C00) >> 10;
        const uint8_t blue = ((blue5 * 255) / 31) & 0xFF;
        
        _palette[pixelIndex] = Pixel(red, green, blue);
    }
};

}

#endif /* ColorPalette_hpp */

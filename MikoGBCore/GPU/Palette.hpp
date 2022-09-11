//
//  Palette.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 9/4/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef Palette_hpp
#define Palette_hpp

#include "PixelBuffer.hpp"
#include <array>

namespace MikoGB {

struct Palette {
    const MikoGB::Pixel &pixelForCode(uint8_t code) const {
        const uint8_t maskedCode = code & 0x3;
        const uint8_t translatedCode = _translation[maskedCode];
        return _palette[translatedCode];
    }
    
protected:
    MikoGB::Pixel _palette[4];
    std::array<uint8_t, 4> _translation;
    
};

}

#endif /* Palette_hpp */

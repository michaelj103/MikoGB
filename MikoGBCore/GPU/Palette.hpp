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

namespace MikoGB {

struct Palette {
    const MikoGB::Pixel &pixelForCode(uint8_t code) const {
        const uint8_t maskedCode = code & 0x3;
        return _palette[maskedCode];
    }
    
    bool isTransparent(uint8_t code) const {
        return _transparent && (code & 0x3) == 0;
    }
    
protected:
    bool _transparent;
    MikoGB::Pixel _palette[4];
};

}

#endif /* Palette_hpp */

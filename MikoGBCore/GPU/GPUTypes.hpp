//
//  GPUTypes.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 9/10/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef GPUTypes_hpp
#define GPUTypes_hpp

#include "BitTwiddlingUtil.h"

namespace MikoGB {

struct TileAttributes {
    uint8_t colorPaletteIndex; // value 0-7 from bits 0-2
    uint8_t characterBank; // 0 or 1, bit 3
    uint8_t dmgPaletteIndex; // 0 or 1, bit 4. Ignored for BG tiles
    bool flipX; // bit 5
    bool flipY; // bit 6
    bool priorityToBG; // bit 7
    
    TileAttributes(uint8_t attr) {
        colorPaletteIndex = (attr & 0x07);
        characterBank = isMaskSet(attr, 0x08) ? 1 : 0;
        dmgPaletteIndex = isMaskSet(attr, 0x10) ? 1 : 0;
        flipX = isMaskSet(attr, 0x20);
        flipY = isMaskSet(attr, 0x40);
        priorityToBG = isMaskSet(attr, 0x80);
    }
};

}

#endif /* GPUTypes_hpp */

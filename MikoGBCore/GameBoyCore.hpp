//
//  GameBoyCore.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCore_hpp
#define GameBoyCore_hpp

#include <cstdlib>
#include "PixelBuffer.hpp"
#include "GameBoyCoreTypes.h"

namespace MikoGB {

class GameBoyCoreImp;

class GameBoyCore {
public:
    GameBoyCore();
    ~GameBoyCore();
    
    bool loadROMData(const void *romData, size_t size);
    void prepTestROM();
    
    /// Step a single CPU instruction
    void step();
    
    /// Emulate at least 1 full frame. If a frame is partially rendered when called, that frame will be finished first and
    /// then the next one will be emulated to completion
    void emulateFrame();
    
    void setButtonPressed(JoypadButton, bool);
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    
    uint16_t getPC() const;
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    
    void getBackground(PixelBufferImageCallback callback);
    
private:
    GameBoyCoreImp *_imp;
};

}

#endif /* GameBoyCore_hpp */

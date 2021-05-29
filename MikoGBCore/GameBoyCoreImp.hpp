//
//  GameBoyCoreImp.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCoreImp_hpp
#define GameBoyCoreImp_hpp

#include <memory>
#include "GameBoyCore.hpp"
#include "CPUCore.hpp"
#include "GPUCore.hpp"
#include "MemoryController.hpp"
#include "Joypad.hpp"

namespace MikoGB {

class GameBoyCoreImp {
public:
    GameBoyCoreImp();
    
    bool loadROMData(const void *romData, size_t size);
    void prepTestROM();
    
    void step();
    
    void emulateFrame();
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    
    void setButtonPressed(JoypadButton, bool);
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    void getBackground(PixelBufferImageCallback callback);
    
private:
    CPUCore::Ptr _cpu;
    GPUCore::Ptr _gpu;
    MemoryController::Ptr _memoryController;
    Joypad::Ptr _joypad;
    
    friend class GameBoyCore;
};

}

#endif /* GameBoyCoreImp_hpp */

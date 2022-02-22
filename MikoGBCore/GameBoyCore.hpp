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
    
    /// Runnability represents whether frame emulation can proceed normally. External runnable represents whether a client wants
    /// emulation to proceed normally. Runnability also takes into account internal signals, mainly for debugging (e.g. breakpoints)
    /// When runnable, emulateFrame() and step() function normally
    /// When not runnable, emulateFrame() returns immediately, step() is available for a client to implement debug functionality
    void setExternallyRunnable(bool);
    bool isExternallyRunnable() const;
    bool isRunnable() const;
    
    /// Set so clients are notified of changes to runnability, e.g. hitting a breakpoint
    void setRunnableChangedCallback(RunnableChangedCallback callback);
    
    void setButtonPressed(JoypadButton, bool);
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    
    uint16_t getPC() const;
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    
    void getBackground(PixelBufferImageCallback callback);
    
    std::vector<DisassembledInstruction> getDisassembledInstructions(int lookAheadCount);
    
    RegisterState getRegisterState();
    
    uint8_t readMem(uint16_t);
    
private:
    GameBoyCoreImp *_imp;
};

}

#endif /* GameBoyCore_hpp */

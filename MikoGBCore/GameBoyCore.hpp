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
    size_t saveDataSize() const;
    size_t copySaveData(void *buffer, size_t size) const;
    
    /// Step a single CPU instruction
    void step();
    
    /// Emulate at least 1 full frame. If a frame is partially rendered when called, that frame will be finished first and
    /// then the next one will be emulated to completion
    void emulateFrame();
    
    /// Runnability represents whether frame emulation can proceed normally. External runnable represents whether a client wants
    /// emulation to proceed normally. Runnability also takes into account internal signals, mainly for debugging (e.g. breakpoints)
    /// When runnable, emulateFrame() and step() function normally
    /// When not runnable, emulateFrame() returns immediately, step() is available for a client to implement debug functionality
    void setRunnable(bool);
    bool isRunnable() const;
    
    /// Set so clients are notified of changes to runnability, e.g. hitting a breakpoint
    void setRunnableChangedCallback(RunnableChangedCallback callback);
    
    void setButtonPressed(JoypadButton, bool);
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    void setAudioSampleCallback(AudioSampleCallback callback);
    
    /// Save state management
    bool isPersistenceStale() const;
    void resetPersistence();
    
    uint16_t getPC() const;
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    
    void getBackground(PixelBufferImageCallback callback);
    
    /// returns the disassembled instructions surrounding the current instruction
    std::vector<DisassembledInstruction> getDisassembledInstructions(int lookAheadCount, int lookBehindCount, size_t *currentIdx) const;
    
    /// returns `count` instructions that were executed before the current instruction. Not super useful if in a long running loop, but useful to roll back jumps/calls
    std::vector<DisassembledInstruction> getDisassembledPreviousInstructions(int count) const;
    
    RegisterState getRegisterState() const;
    
    uint8_t readMem(uint16_t) const;
    
    bool setLineBreakpoint(int romBank, uint16_t addr);
    
private:
    GameBoyCoreImp *_imp;
};

}

#endif /* GameBoyCore_hpp */

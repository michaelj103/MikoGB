//
//  GameBoyCoreImp.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCoreImp_hpp
#define GameBoyCoreImp_hpp

#include <memory>
#include <vector>
#include "GameBoyCore.hpp"
#include "CPUCore.hpp"
#include "GPUCore.hpp"
#include "MemoryController.hpp"
#include "Joypad.hpp"
#include "Disassembler.hpp"

namespace MikoGB {

class GameBoyCoreImp {
public:
    GameBoyCoreImp();
    
    bool loadROMData(const void *romData, size_t size);
    void prepTestROM();
    
    void step();
    
    void emulateFrame();
    
    void setExternallyRunnable(bool);
    bool isExternallyRunnable() const { return _isExternallyRunnable; }
    bool isRunnable() const { return _isExternallyRunnable; }
    void setRunnableChangedCallback(RunnableChangedCallback callback) { _runnableChangedCallback = callback; }
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    
    void setButtonPressed(JoypadButton, bool);
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    void getBackground(PixelBufferImageCallback callback);
    std::vector<DisassembledInstruction> getDisassembledInstructions(int lookAheadCount);
    
private:
    CPUCore::Ptr _cpu;
    GPUCore::Ptr _gpu;
    MemoryController::Ptr _memoryController;
    Joypad::Ptr _joypad;
    Disassembler::Ptr _disassembler;
    Disassembler::Ptr _accessDisassembler();
    
    //TODO: runnable should probably be a CPU notion once there are internal triggers?
    bool _isExternallyRunnable = false;
    RunnableChangedCallback _runnableChangedCallback;
    
    friend class GameBoyCore;
};

}

#endif /* GameBoyCoreImp_hpp */

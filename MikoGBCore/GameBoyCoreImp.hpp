//
//  GameBoyCoreImp.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCoreImp_hpp
#define GameBoyCoreImp_hpp

#include "GameBoyCore.hpp"
#include "CPUCore.hpp"
#include "GPUCore.hpp"
#include "MemoryController.hpp"

namespace MikoGB {

class GameBoyCoreImp {
public:
    GameBoyCoreImp();
    ~GameBoyCoreImp();
    
    bool loadROMData(void *romData, size_t size);
    void prepTestROM();
    
    void step();
    
    void emulateFrame();
    
    void setScanlineCallback(PixelBufferScanlineCallback callback);
    
    /// Debug utilities
    void getTileMap(PixelBufferImageCallback callback);
    void getBackground(PixelBufferImageCallback callback);
    
private:
    CPUCore *_cpu;
    GPUCore *_gpu;
    MemoryController *_memoryController;
    
    friend class GameBoyCore;
};

}

#endif /* GameBoyCoreImp_hpp */

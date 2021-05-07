//
//  GPUCore.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GPUCore_hpp
#define GPUCore_hpp

#include <cstdlib>



namespace MikoGB {

class CPUCore;

enum LCDMode : uint8_t {
    HBlank = 0,
    VBlank = 1,
    OAMScan = 2,
    LCDTransfer = 3,
};

class GPUCore {
public:
    GPUCore(CPUCore *cpu): _cpu(cpu) {};
    
    /// Expects CPU oscillation cycles (~4.2MHz, 4 per instruction cycle)
    void updateWithCPUCycles(size_t cpuCycles);
    
private:
    CPUCore *_cpu = nullptr;
    size_t _cycleCount = 0;
    uint8_t _currentScanline = 0;
    void _incrementScanline();
    
    LCDMode _currentMode = OAMScan;
    void _setMode(LCDMode mode);
    
    bool _wasOn = false;
    void _turnOff();
    
    void _processScanline(uint8_t line);
};

}

#endif /* GPUCore_hpp */

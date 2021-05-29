//
//  Timer.hpp
//  MikoGB
//
//  Created on 5/28/21.
//

#ifndef Timer_hpp
#define Timer_hpp

#include <cstdlib>

namespace MikoGB {

class Timer {
public:
    /// Expects CPU oscillation cycles (~4.2MHz (2^22), 4 per instruction cycle)
    /// Returns whether or not there was a TIMA overflow which triggers an interrupt
    bool updateWithCPUCycles(size_t cpuCycles);
    
    uint8_t getDiv() const;
    void resetDiv();
    
    uint8_t getTIMA() const;
    void setTIMA(uint8_t);
    void setTMA(uint8_t);
    
    void setTAC(uint8_t);
    
private:
    uint32_t _divRegister = 0;
    uint16_t _tima = 0;
    uint8_t _tma = 0;
    size_t _timaClock = 0;
    size_t _timaClockIncRate = 0;
    bool _timaEnabled = false;
};

}

#endif /* Timer_hpp */

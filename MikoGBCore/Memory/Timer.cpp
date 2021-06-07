//
//  Timer.cpp
//  MikoGB
//
//  Created on 5/28/21.
//

#include "Timer.hpp"
#include "BitTwiddlingUtil.h"
#include <cassert>

using namespace MikoGB;

static const uint32_t DivMax = 0xFFFF;

bool Timer::updateWithCPUCycles(size_t cpuCycles) {
    // update DIV
    _divRegister += cpuCycles;
    if (_divRegister > DivMax) {
        _divRegister -= DivMax;
    }
    
    // update TIMA
    bool overflowed = false;
    if (_timaEnabled) {
        _timaClock += cpuCycles;
        while (_timaClock >= _timaClockIncRate) {
            _timaClock -= _timaClockIncRate;
            _tima += 1;
            
            if (_tima > 0xFF) {
                _tima = _tma;
                overflowed = true;
            }
        }
    }
    return overflowed;
}

uint8_t Timer::getDiv() const {
    // Div internally is a 16-bit counter, reading gives the most-significant 8 bits
    uint8_t divVal = (_divRegister & 0xFF00) >> 8;
    return divVal;
}

void Timer::resetDiv() {
    _divRegister = 0;
}

uint8_t Timer::getTIMA() const {
    uint8_t val = (_tima & 0xFF);
    return val;
}

void Timer::setTIMA(uint8_t val) {
    _tima = val;
}

void Timer::setTMA(uint8_t val) {
    _tma = val;
}

void Timer::setTAC(uint8_t tac) {
    _timaEnabled = isMaskSet(tac, 0x04);
    const uint8_t freq = tac & 0x03;
    switch (freq) {
        case 0:
            _timaClockIncRate = 1024; // 4096Hz, every 1024 CPU ticks
            break;
        case 1:
            _timaClockIncRate = 16; // 262144Hz, every 16 CPU ticks
            break;
        case 2:
            _timaClockIncRate = 64; // 65536Hz, every 64 CPU ticks
            break;
        case 3:
            _timaClockIncRate = 256; // 16384Hz, every 256 CPU ticks
            break;
#if DEBUG
        default:
            assert(false);
            break;
#endif
    }
}

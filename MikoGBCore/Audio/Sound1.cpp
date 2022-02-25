//
//  Sound1.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "Sound1.hpp"
#include "BitTwiddlingUtil.h"
#include <cassert>
#include <algorithm>

using namespace std;
using namespace MikoGB;

static const int SweepTimeCycles = 1 << 15; // 128Hz with 4.2MHz CPU: 2^22 / 2^7 = 1 << 15
static const int DurationTimeCycles = 1 << 14; // 256Hz with 4.2MHz CPU: 2^22 / 2^8 = 1 << 14
static const int EnvelopeTimeCycles = 1 << 16; // 64Hz with 4.2MHz CPU: 2^22 / 2^6 = 1 << 16

void Sound1::updateWithCycles(int cycles) {
    if (!_isRunning) {
        return;
    }
    
    // sweep, if enabled
    if (_sweepTime > 0) {
        _sweepCounter -= cycles;
        while (_sweepCounter <= 0) {
            // we need to sweep at least once (realistically, max once)
            _sweepCounter += _sweepTime;
            int nextFreq = _freq + ((_freq >> _sweepShift) * _sweepSign);
            
            // check boundary conditions
            if (nextFreq >= 2048) {
                // exceeding the frequency max immediately stops the sound
                _isRunning = false;
                return;
            }
            // if frequency sweeps to negative, it just stays put
            if (nextFreq >= 0) {
                _freq = nextFreq;
                _updateFreqCounter();
            }
        }
    }
    
    // envelope, if enabled
    if (_envelopeStepTime > 0) {
        _envelopeStepCounter -= cycles;
        while (_envelopeStepCounter <= 0) {
            // we need to make at least one envelope step (realistically, max once)
            _envelopeStepCounter += _envelopeStepTime;
            _envelopeVolume = std::min(std::max(_envelopeVolume + _envelopeSign, 0), 15);
        }
    }
    
    // duration
    if (_durationEnabled) {
        _durationCounter -= cycles;
        if (_durationCounter <= 0) {
            _isRunning = false;
            return;
        }
    }
    
    // duty
    _freqCounter -= cycles;
    while (_freqCounter <= 0) {
        // we need to update the current duty period. Might happen a couple times per instruction
        // for very high frequency sounds
        _freqCounter += _freqCycles;
        _waveDutyPeriod = (_waveDutyPeriod + 1) % 8;
    }
}

uint8_t Sound1::sound1Write(uint16_t offset, uint8_t val) {
    switch (offset) {
        case 0:
            // 0xFF10, NR10: Sound 1 sweep register
            _resetSweep(val);
            return val;
        case 1:
            // 0xFF11, NR11: Sound 1 duty and duration register
            _resetDutyAndDuration(val);
            return val & 0xC0; // only top 2 bits are readable
        case 2:
            // 0xFF12, NR12: Sound 1 envelope register
            _resetEnvelope(val);
            return val;
        case 3:
            // 0xFF13, NR13: Sound 1 frequency low register
            _resetFreqLow(val);
            return 0; // not readable
        case 4:
            // 0xFF14, NR14: Sound 1 frequency hi and control
            _resetFreqHigh(val);
            return val & 0x40; // only bit 6 is readable
        default:
            assert(false);
    }
    return 0;
}

int Sound1::getVolume() const {
    if (_isRunning) {
        switch (_duty) {
            case 0:
                // 12.5% aka 1/8
                return _waveDutyPeriod == 0 ? _envelopeVolume : 0;
            case 1:
                // 25% aka 2/8
                return _waveDutyPeriod < 2 ? _envelopeVolume : 0;
            case 2:
                // 50% aka 4/8 aka normal
                return _waveDutyPeriod < 4 ? _envelopeVolume : 0;
            case 3:
                // 75% aka 6/8
                return _waveDutyPeriod < 6 ? _envelopeVolume : 0;
            default:
                assert(false);
        }
    }
    return 0;
}

void Sound1::_resetSweep(uint8_t val) {
    int sweepTime = (val & 0x70 >> 4); // bits 4-6 indicate time in multiples of 128Hz
    _sweepTime = sweepTime * SweepTimeCycles;
    _sweepSign = isMaskSet(val, 0x8) ? -1 : 1;
    _sweepShift = (val & 0x7); // bits 0-2 indicate shift per sweep
    _sweepCounter = _sweepTime;
}

void Sound1::_resetDutyAndDuration(uint8_t val) {
    _duty = (val & 0xC0) >> 6; // bits 7-8 represent duty
    // bits 0-6 are duration count. sound lasts (64-count) increments of 1/256
    int durationCounts = (val & 0x3F);
    _durationTime = (64 - durationCounts) * DurationTimeCycles;
    _durationCounter = _durationTime;
}

void Sound1::_resetEnvelope(uint8_t val) {
    _envelopeInitialVolume = (val & 0xF0) >> 4; // bits 4-7 are initial envelope volume
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeSign = isMaskSet(val, 0x8) ? 1 : -1; // bit 3 is attenuate/amplify
    // bits 0-2 are envelope step time. Each step is *count* increments of 1/64 second
    int envelopeCounts = (val & 0x7);
    _envelopeStepTime = envelopeCounts * EnvelopeTimeCycles;
    _envelopeStepCounter = _envelopeStepTime;
}

void Sound1::_resetFreqLow(uint8_t val) {
    int freqOld = _freq;
    // freq is 11 bits. keep top 3 and add in the low 8
    _freq = (_freq & 700) | val;
    if (freqOld != _freq) {
        _updateFreqCounter();
    }
}

void Sound1::_resetFreqHigh(uint8_t val) {
    int freqOld = _freq;
    // freq is 11 bits, keep bottom 8 and add the top 3
    int freqUpdate = val & 7;
    _freq = (_freq & 0xFF) | (freqUpdate << 8);
    if (freqOld != _freq) {
        _updateFreqCounter();
    }
    // duration enable is here for whatever reason
    _durationEnabled = isMaskSet(val, 0x40);
    
    // restart the sound if the initialize bit is set
    if (isMaskSet(val, 0x80)) {
        _initialize();
    }
}

void Sound1::_updateFreqCounter() {
    // frequency in Hz is (2^17 / (2048 - _freq)), call it X
    // so 2^22 / X is the number of cycles per wave
    // therefore, cycles per wave is 2^5 * (2048 - _freq)
    // duty cycles can be specified in 8ths, so further divide by 8 to get
    // cycles per duty update
    _freqCycles = 4 * (2048 - _freq);
    _freqCounter = _freqCycles;
    _waveDutyPeriod = 0;
}

void Sound1::_initialize() {
    // reset sweep
    _sweepCounter = _sweepTime;
    // reset envelope
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeStepCounter = _envelopeStepTime;
    // reset duration
    _durationCounter = _durationTime;
    // reset duty period
    _freqCounter = _freqCycles;
    _waveDutyPeriod = 0;
    
    // ...and start
    _isRunning = true;
}

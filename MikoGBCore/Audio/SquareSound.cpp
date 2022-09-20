//
//  SquareSound.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "SquareSound.hpp"
#include "BitTwiddlingUtil.h"
#include <cassert>
#include <algorithm>
#include <array>

using namespace std;
using namespace MikoGB;

// Note on double-speed support. All of the cycle counts are doubled so that we can avoid fractional cycles
// In normal speed mode, cycles are multipled by 2 before being handed to the audio controller, so the timing cancels to 1x
// In double speed mode, cycles are not multiplied by 2, so it takes ~2x as many instructions before audio events occur
// This keeps the audio controller running at real time relative to external driver
static const int SweepTimeCycles = 1 << 16; // 128Hz with 4.2MHz CPU: 2^22 / 2^7 = 1 << 15 (x2 for double speed support)
static const int DurationTimeCycles = 1 << 15; // 256Hz with 4.2MHz CPU: 2^22 / 2^8 = 1 << 14 (x2 for double speed support)
static const int EnvelopeTimeCycles = 1 << 17; // 64Hz with 4.2MHz CPU: 2^22 / 2^6 = 1 << 16 (x2 for double speed support

static const int DutyPatternLength = 8;
// Duty patterns from pan docs. They don't really make a difference though vs idx <= count
static const array<array<double, DutyPatternLength>, 4> DutyPatterns =
{{
    { 0., 0., 0., 0., 0., 0., 0., 1. },
    { 1., 0., 0., 0., 0., 0., 0., 1. },
    { 1., 0., 0., 0., 0., 1., 1., 1. },
    { 0., 1., 1., 1., 1., 1., 1., 0. },
}};

void SquareSound::updateWithCycles(int cycles) {
    if (!_isRunning) {
        return;
    }
    
    // sweep, if enabled
    if (_hasSweep && _sweepTime > 0 && _sweepShift > 0) {
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
        _waveDutyPeriod = (_waveDutyPeriod + 1) % DutyPatternLength;
    }
}

uint8_t SquareSound::soundWrite(uint16_t offset, uint8_t val) {
    // square circuit with sweep is 5 registers, first is sweep. Other is 4 in the same order
    uint16_t trueOffset = _hasSweep ? offset : offset + 1;
    switch (trueOffset) {
        case 0:
            // NR10: Sound 1 sweep register
            _resetSweep(val);
            return val;
        case 1:
            // NR11/NR21: Sound 1/2 duty and duration register
            _resetDutyAndDuration(val);
            return val & 0xC0; // only top 2 bits are readable
        case 2:
            // NR12/NR22: Sound 1/2 envelope register
            _resetEnvelope(val);
            return val;
        case 3:
            // NR13/NR23: Sound 1/2 frequency low register
            _resetFreqLow(val);
            return 0; // not readable
        case 4:
            // NR14/NR24: Sound 1/2 frequency hi and control
            _resetFreqHigh(val);
            return val & 0x40; // only bit 6 is readable
        default:
            assert(false);
    }
    return 0;
}

double SquareSound::getSample() const {
    if (_isRunning) {
        double env = (double)_envelopeVolume / 15.0;
        double output = 0.0;
        switch (_duty) {
            case 0:
                // 12.5% aka 1/8
                output = DutyPatterns[0][_waveDutyPeriod] * env;
                break;
            case 1:
                // 25% aka 2/8
                output = DutyPatterns[1][_waveDutyPeriod] * env;
                break;
            case 2:
                // 50% aka 4/8 aka normal
                output = DutyPatterns[2][_waveDutyPeriod] * env;
                break;
            case 3:
                // 75% aka 6/8
                output = DutyPatterns[3][_waveDutyPeriod] * env;
                break;
            default:
                assert(false);
        }
        
        // linearly translate from [0.0, 1.0] to [-1.0, 1.0]
        double analog = (output * 2.0) - 1.0;
        return analog;
    }
    return 0.0;
}

void SquareSound::_resetSweep(uint8_t val) {
    int sweepTime = (val & 0x70 >> 4); // bits 4-6 indicate time in multiples of 128Hz
    _sweepTime = sweepTime * SweepTimeCycles;
    _sweepSign = isMaskSet(val, 0x8) ? -1 : 1;
    _sweepShift = (val & 0x7); // bits 0-2 indicate shift per sweep
    _sweepCounter = _sweepTime;
}

void SquareSound::_resetDutyAndDuration(uint8_t val) {
    _duty = (val & 0xC0) >> 6; // bits 6-7 represent duty
    // bits 0-5 are duration count. sound lasts (64-count) increments of 1/256
    int durationCounts = (val & 0x3F);
    _durationTime = (64 - durationCounts) * DurationTimeCycles;
    _durationCounter = _durationTime;
}

void SquareSound::_resetEnvelope(uint8_t val) {
    _envelopeInitialVolume = (val & 0xF0) >> 4; // bits 4-7 are initial envelope volume
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeSign = isMaskSet(val, 0x8) ? 1 : -1; // bit 3 is attenuate/amplify
    // bits 0-2 are envelope step time. Each step is *count* increments of 1/64 second
    int envelopeCounts = (val & 0x7);
    _envelopeStepTime = envelopeCounts * EnvelopeTimeCycles;
    _envelopeStepCounter = _envelopeStepTime;
}

void SquareSound::_resetFreqLow(uint8_t val) {
    // freq is 11 bits. keep top 3 and add in the low 8
    _freq = (_freq & 0x700) | val;
}

void SquareSound::_resetFreqHigh(uint8_t val) {
    // freq is 11 bits, keep bottom 8 and add the top 3
    int freqUpdate = val & 7;
    _freq = (_freq & 0xFF) | (freqUpdate << 8);
    // duration enable is here for whatever reason
    _durationEnabled = isMaskSet(val, 0x40);
    
    // restart the sound if the initialize bit is set
    if (isMaskSet(val, 0x80)) {
        _initialize();
    }
}

void SquareSound::_updateFreqCounter() {
    // frequency in Hz is (2^17 / (2048 - _freq)), call it X
    // so 2^22 / X is the number of cycles per wave
    // therefore, cycles per wave is 2^5 * (2048 - _freq)
    // duty cycles can be specified in 8ths, so further divide by 8 to get
    // cycles per duty update
    // Then multiply by 2 for double-speed support. See note at the top of the file
    _freqCycles = 4 * (2048 - _freq) * 2;
    _freqCounter = _freqCycles;
    _waveDutyPeriod = 0;
}

void SquareSound::_initialize() {
    // reset sweep
    _sweepCounter = _sweepTime;
    // reset envelope
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeStepCounter = _envelopeStepTime;
    // reset duration
    _durationCounter = _durationTime;
    // reset duty period and frequency calculations
    _updateFreqCounter();
    
    // ...and start
    _isRunning = true;
}

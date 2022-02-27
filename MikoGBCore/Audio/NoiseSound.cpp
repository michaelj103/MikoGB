//
//  NoiseSound.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/26/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "NoiseSound.hpp"
#include <cassert>
#include "BitTwiddlingUtil.h"
#include <array>

using namespace std;
using namespace MikoGB;

static const int DurationTimeCycles = 1 << 14; // 256Hz with 4.2MHz CPU: 2^22 / 2^8 = 1 << 14
static const int EnvelopeTimeCycles = 1 << 16; // 64Hz with 4.2MHz CPU: 2^22 / 2^6 = 1 << 16
static const int BaseFrequency = 1 << 19; // 4.2MHz / 8 per docs: 2^22 / 8 = 1 << 19

void NoiseSound::updateWithCycles(int cycles) {
    if (!_isRunning) {
        return;
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
    
    // sample
    if (_freqCycles > 0) {
        _freqCounter -= cycles;
        while (_freqCounter <= 0) {
            // we need to shift the LFSR register each frequency "tick"
            _freqCounter += _freqCycles;
            _lfsrShift();
        }
    }
}

double NoiseSound::getSample() const {
    if (!_isRunning) {
        return 0.0;
    }
    double level = (_lfsrRegister & 0x1) == 0 ? 1.0 : 0.0;
    double volume = (double)_envelopeVolume / 15.0;
    double sample = level * volume;
    
    // Adjust the analog sample from [0.0, 1.0] -> [-1.0, 1.0]
    double adjustedSample = (sample * 2.0) - 1.0;
    return adjustedSample;
}

uint8_t NoiseSound::soundWrite(uint16_t offset, uint8_t val) {
    // Noise circuit has 4 registers
    switch (offset) {
        case 0:
            // 0xFF20, NR41: Sound 4 duration register
            _resetDuration(val);
            return val & 0x3F; // top 2 not readable
        case 1:
            // 0xFF21, NR42: Sound 4 envelop register
            _resetEnvelope(val);
            return val;
        case 2:
            // 0xFF22, NR43: Sound 4 frequency register
            _resetFrequency(val);
            return val;
        case 3:
            // 0xFF23, NR44: Sound 4 init and counter mode register
            _resetInitAndCounter(val);
            return val & 0x40; // only bit 6 is readable
        default:
            assert(false);
    }
    return 0;
}

void NoiseSound::_resetDuration(uint8_t val) {
    // bits 0-5 are duration count. sound lasts (64-count) increments of 1/256
    int durationCounts = (val & 0x3F);
    _durationTime = (64 - durationCounts) * DurationTimeCycles;
    _durationCounter = _durationTime;
}

void NoiseSound::_resetEnvelope(uint8_t val) {
    _envelopeInitialVolume = (val & 0xF0) >> 4; // bits 4-7 are initial envelope volume
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeSign = isMaskSet(val, 0x8) ? 1 : -1; // bit 3 is attenuate/amplify
    // bits 0-2 are envelope step time. Each step is *count* increments of 1/64 second
    int envelopeCounts = (val & 0x7);
    _envelopeStepTime = envelopeCounts * EnvelopeTimeCycles;
    _envelopeStepCounter = _envelopeStepTime;
}

void NoiseSound::_resetFrequency(uint8_t val) {
    // dividers are documented as multiplying the base frequency/8 by 2, 1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7
    // so, only 0 is special (multiply by 2) vs divide by the value
    int divider = (val & 0x07); // low 3 bits are divider
    int freq = divider == 0 ? BaseFrequency * 2 : BaseFrequency / divider;
    int shift = (val & 0xF0) >> 4;
    // technically, the max shift is 13. 14 and 15 are documented to be "prohibited codes"
    freq = freq >> shift;
    // now, freq is the desired frequency in Hz, aka shifts per second
    // Divide into the CPU cycles per second to get the cycles per shift
    _freqCycles = (1 << 22) / freq;
    _freqCounter = _freqCycles;
    
    _lowBitMode = isMaskSet(val, 0x08);
}

void NoiseSound::_resetInitAndCounter(uint8_t val) {
    // duration is enabled/disabled here, but parameters are set in the duration register
    _durationEnabled = isMaskSet(val, 0x40);
    // restart the sound if the initialize bit is set
    if (isMaskSet(val, 0x80)) {
        _initialize();
    }
}

void NoiseSound::_lfsrShift() {
    const uint16_t in = _lfsrRegister;
    const uint16_t xorVal = (in & 0x01) ^ ((in & 0x02) >> 1);
    uint16_t out = in >> 1;
    out |= xorVal << 14; // this should always be 0, so we can OR it
    if (_lowBitMode) {
        // bit 6 might already be 1 or 0 so we need to split cases
        if (xorVal == 1) {
            // we want to ensure bit 6 is set
            out |= (1 << 6);
        } else {
            // we want to ensure bit 6 is reset
            const uint16_t mask = (1 << 6);
            out &= ~mask;
        }
    }
    _lfsrRegister = out;
}

void NoiseSound::_initialize() {
    // reset envelope
    _envelopeVolume = _envelopeInitialVolume;
    _envelopeStepCounter = _envelopeStepTime;
    // reset duration
    _durationCounter = _durationTime;
    // reset frequency counter
    _freqCounter = _freqCycles;
    
    // ...and start
    _isRunning = true;
}


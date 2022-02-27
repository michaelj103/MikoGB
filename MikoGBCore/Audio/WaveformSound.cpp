//
//  WaveformSound.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/26/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "WaveformSound.hpp"
#include "BitTwiddlingUtil.h"

using namespace std;
using namespace MikoGB;

static const int DurationTimeCycles = 1 << 14; // 256Hz with 4.2MHz CPU: 2^22 / 2^8 = 1 << 14

void WaveformSound::updateWithCycles(int cycles) {
    if (!_isRunning) {
        return;
    }
    
    // duration
    if (_durationEnabled) {
        _durationCounter -= cycles;
        if (_durationCounter <= 0) {
            _isRunning = false;
            return;
        }
    }
    
    // sample index
    if (_freqCycles > 0) {
        _freqCounter -= cycles;
        while (_freqCounter <= 0) {
            _freqCounter += _freqCycles;
            // we need to shift the sample index. there are 32 samples
            _waveSampleIndex  = (_waveSampleIndex + 1) % 32;
        }
    }
}

double WaveformSound::getSample() const {
    if (!_enabled || !_isRunning || _outputLevel == 0) {
        return 0.0;
    }
    
    uint8_t sample = _samples[_waveSampleIndex] >> (_outputLevel - 1);
    double analog = (double)sample / 15.0;
    double adjustedAnalog = (analog * 2.0) - 1.0;
    return adjustedAnalog;
}

uint8_t WaveformSound::soundWrite(uint16_t offset, uint8_t val) {
    // waveform circuit has 5 registers
    switch (offset) {
        case 0:
            // 0xFF1A, NR30: Sound 3 on/off register
            _resetEnabled(val);
            return val & 0x80; // only top bit is readable
        case 1:
            // 0xFF1B, NR31: Sound 3 duration register
            _resetDuration(val);
            return val;
        case 2:
            // 0xFF1C, NR32: Sound 3 output level register
            _resetOutputLevel(val);
            return val & 0x60; // only bits 5 and 6
        case 3:
            // 0xFF1D, NR33: Sound 3 low order frequency register
            _resetFreqLow(val);
            return val;
        case 4:
            // 0xFF1E, NR34: Sound 3 high order frequency and control
            _resetFreqHigh(val);
            return val & 0x40; // only bit 6
        default:
            assert(false);
    }
    return 0;
}

void WaveformSound::customSampleWrite(uint16_t offset, uint8_t val) {
    assert(offset < 16);
    
    int sampleIdx = offset * 2;
    uint8_t highSample = (val & 0xF0) >> 4;
    uint8_t lowSample = (val & 0x0F);
    _samples[sampleIdx] = highSample;
    _samples[sampleIdx + 1] = lowSample;
}

void WaveformSound::_resetEnabled(uint8_t val) {
    // whole register for 1 bit
    _enabled = isMaskSet(val, 0x80);
}

void WaveformSound::_resetDuration(uint8_t val) {
    // if enabled, sound lasts (256-count) increments of 1/256
    _durationTime = (64 - (int)val) * DurationTimeCycles;
    _durationCounter = _durationTime;
}

void WaveformSound::_resetOutputLevel(uint8_t val) {
    // 2-bit value in bits 5-6 for whatever reason
    _outputLevel = (val & 0x60) >> 5;
}

void WaveformSound::_resetFreqLow(uint8_t val) {
    // freq is 11 bits. keep top 3 and add in the low 8
    _freq = (_freq & 0x700) | val;
}

void WaveformSound::_resetFreqHigh(uint8_t val) {
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

void WaveformSound::_updateFreqCounter() {
    // frequency in Hz is (2^16 / (2048 - _freq)), call it X
    // so 2^22 / X is the number of cycles per wave
    // therefore, cycles per wave is 2^6 * (2048 - _freq)
    // there are 32 samples in the custom waveform, so further divide by 32 to get
    // 2 * (2048 - _freq) cycles per sample
    _freqCycles = 2 * (2048 - _freq);
    _freqCounter = _freqCycles;
    _waveSampleIndex = 0;
}

void WaveformSound::_initialize() {
    // reset duration
    _durationCounter = _durationTime;
    // reset duty period and frequency calculations
    _updateFreqCounter();
    
    // ...and start
    _isRunning = true;
}

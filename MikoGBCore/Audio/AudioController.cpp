//
//  AudioController.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "AudioController.hpp"
#include "BitTwiddlingUtil.h"

using namespace std;
using namespace MikoGB;

// sound 1 register range
static const uint16_t NR10Register = 0xFF10; // Sound 1 sweep register
static const uint16_t NR14Register = 0xFF14; // Sound 1 frequency hi and control

// sound 2 register range
static const uint16_t NR21Register = 0xFF16; // Sound 2 duty and duration register
static const uint16_t NR24Register = 0xFF19; // Sound 2 frequency hi and control

static const uint16_t NR50Register = 0xFF24; // Channel control
static const uint16_t NR51Register = 0xFF25; // Sound selection register
static const uint16_t NR52Register = 0xFF26; // Sound on/off register
static const uint16_t AudioRegisterBase = NR10Register;

// A note on timing:
// "cycles per sample" is "cycles per second" (1<<22) divided by "samples per second"
// this isn't an integer, so to keep the timing right, work fractionally
// each update(cycles) ticks cycles*SamplesPerSecond off of this number
// we emit a sample when it hits <= 0 and reset to += (1 << 22) to account for fracional drift
static const int SampleCounterBase = 1 << 22;
static const int SamplesPerSecond = 44100;
static const int16_t SampleMaxVolume = INT16_MAX;

AudioController::AudioController() {
    _nextSampleCounter = SampleCounterBase;
}

void AudioController::updateWithCPUCycles(int cycles) {
    _sound1.updateWithCycles(cycles);
    _sound2.updateWithCycles(cycles);
    // TODO: others!
    
    // this isn't technically correct if the input cycles is large (~190 or more)
    // but the input cycles should only be the duration of a cpu instruction so up to ~10
    // if there can be multiple samples per update we'd need to do something more complicated
    _nextSampleCounter -= (cycles * SamplesPerSecond);
    while (_nextSampleCounter <= 0) {
        _nextSampleCounter += SampleCounterBase;
        // emit a sample!
        _emitSample();
    }
}

// get left/right channel volumes as double from 0.0 - 1.0
static void ChannelVolumes(uint8_t val, double &leftChannel, double &rightChannel) {
    int leftVolume = (val & 0x70) >> 4;
    int rightVolume = (val & 0x7);
    leftChannel = (double)leftVolume / 7.0;
    rightChannel = (double)rightVolume / 7.0;
}

void AudioController::writeAudioRegister(uint16_t addr, uint8_t val) {
    uint8_t updatedVal = val;
    if (addr >= NR10Register && addr <= NR14Register) {
        updatedVal = _sound1.soundWrite(addr - NR10Register, val);
    } else if (addr >= NR21Register && addr <= NR24Register) {
        updatedVal = _sound2.soundWrite(addr - NR21Register, val);
    } else if (addr == NR50Register) {
        // channel control
        ChannelVolumes(val, _leftVolume, _rightVolume);
    } else if (addr == NR52Register) {
        _soundOn = isMaskSet(val, 0x80);
    }
    
    _audioRegisters[addr - AudioRegisterBase] = updatedVal;
}

uint8_t AudioController::readAudioRegister(uint16_t addr) const {
    if (addr == NR52Register) {
        uint8_t baseVal = _audioRegisters[addr - AudioRegisterBase] & 0x80;
        baseVal |= (_sound1.isRunning() ? 1 : 0);
        baseVal |= (_sound2.isRunning() ? 1 : 0) << 1;
        // TODO: others!
    }
    return _audioRegisters[addr - AudioRegisterBase];
}

void AudioController::_emitSample() {
    // emit an empty sample if sound is globally off
    if (!_soundOn) {
        _sampleCallback(0, 0);
        return;
    }
    
    // get current individual sound volumes as double 0.0 - 1.0
    double sound1 = (double)_sound1.getVolume() / 15.0;
    double sound2 = (double)_sound2.getVolume() / 15.0;
    // TODO: others!
    
    double leftSample = 0.0;
    double rightSample = 0.0;
    uint8_t selectionValue = _audioRegisters[NR51Register - AudioRegisterBase];
    if (isMaskSet(selectionValue, 0x10)) {
        leftSample += sound1;
    }
    if (isMaskSet(selectionValue, 0x20)) {
        leftSample += sound2;
    }
    if (isMaskSet(selectionValue, 0x1)) {
        rightSample += sound1;
    }
    if (isMaskSet(selectionValue, 0x2)) {
        rightSample += sound2;
    }
    
    leftSample /= 4.0;
    rightSample /= 4.0;
    
    int16_t integerLeftSample = (leftSample * _leftVolume) * SampleMaxVolume;
    int16_t integerRightSample = (rightSample * _rightVolume) * SampleMaxVolume;
    _sampleCallback(integerLeftSample, integerRightSample);
}

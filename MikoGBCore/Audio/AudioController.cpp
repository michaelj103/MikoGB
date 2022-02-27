//
//  AudioController.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "AudioController.hpp"
#include "BitTwiddlingUtil.h"

#include <iostream>

using namespace std;
using namespace MikoGB;

// sound 1 register range
static const uint16_t NR10Register = 0xFF10; // Sound 1 sweep register
static const uint16_t NR14Register = 0xFF14; // Sound 1 frequency high and control

// sound 2 register range
static const uint16_t NR21Register = 0xFF16; // Sound 2 duty and duration register
static const uint16_t NR24Register = 0xFF19; // Sound 2 frequency high and control

// sound 3 register range
static const uint16_t NR30Register = 0xFF1A; // Sound 3 on/off register
static const uint16_t NR34Register = 0xFF1E; // Sound 4 frequency high and control
static const uint16_t WaveRamStart = 0xFF30;
static const uint16_t WaveRamEnd = 0xFF3F;

// sound 4 register range
static const uint16_t NR41Register = 0xFF20; // Sound 4 duration register
static const uint16_t NR44Register = 0xFF23; // Sound 4 control register

static const uint16_t NR50Register = 0xFF24; // Channel control
static const uint16_t NR51Register = 0xFF25; // Sound selection register
static const uint16_t NR52Register = 0xFF26; // Sound on/off register
static const uint16_t AudioRegisterBase = NR10Register;

// A note on timing:
// "cycles per sample" is "cycles per second" (1<<22) divided by "samples per second"
// this isn't an integer, so to keep the timing right, work fractionally
// each update(cycles) ticks cycles*SamplesPerSecond off of this number
// we emit a sample when it hits <= 0 and reset to += (1 << 22) to account for fracional drift
// Instead of the actual clock speed (1<22) as the counter base, use the GPU speed, which is
// 456 cycles per scanline, 154 scanlines per frame, 60 frames per second
static const int SampleCounterBase = 456 * 154 * 60;
static const int SamplesPerSecond = 44100;
static const int16_t SampleMaxVolume = INT16_MAX * 0.9;

AudioController::AudioController(): _sound1(true), _sound2(false) {
    _nextSampleCounter = SampleCounterBase;
}

void AudioController::updateWithCPUCycles(int cycles) {
    _sound1.updateWithCycles(cycles);
    _sound2.updateWithCycles(cycles);
    _sound3.updateWithCycles(cycles);
    _sound4.updateWithCycles(cycles);

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

// Alternative version of the updater that updates per sample and not excessively
//void AudioController::updateWithCPUCycles(int cycles) {
//    int cyclesUpdated = 0;
//    int audioCycles = (cycles * SamplesPerSecond);
//    while (audioCycles > 0) {
//        if (audioCycles >= _nextSampleCounter) {
//            // if we'll hit a cycle, run until then
//            int cpuCycles = _nextSampleCounter / SamplesPerSecond;
//            audioCycles -= _nextSampleCounter;
//            _nextSampleCounter = SampleCounterBase;
//
//            // update until the sample will be emitted and then emit
//            _sound1.updateWithCycles(cpuCycles);
//            _sound2.updateWithCycles(cpuCycles);
//            _sound3.updateWithCycles(cpuCycles);
//            _sound4.updateWithCycles(cpuCycles);
//            cyclesUpdated += cpuCycles;
//
//            _emitSample();
//        } else {
//            int remainingCPUCycles = cycles - cyclesUpdated;
//            _nextSampleCounter -= audioCycles;
//            audioCycles = 0;
//            _sound1.updateWithCycles(remainingCPUCycles);
//            _sound2.updateWithCycles(remainingCPUCycles);
//            _sound3.updateWithCycles(remainingCPUCycles);
//            _sound4.updateWithCycles(remainingCPUCycles);
//        }
//    }
//}

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
    } else if (addr >= NR30Register && addr <= NR34Register) {
        updatedVal = _sound3.soundWrite(addr - NR30Register, val);
    } else if (addr >= NR41Register && addr <= NR44Register) {
        updatedVal = _sound4.soundWrite(addr - NR41Register, val);
    } else if (addr >= WaveRamStart && addr <= WaveRamEnd) {
        _sound3.customSampleWrite(addr - WaveRamStart, val);
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
        baseVal |= (_sound3.isRunning() ? 1 : 0) << 2;
        baseVal |= (_sound4.isRunning() ? 1 : 0) << 3;
    }
    return _audioRegisters[addr - AudioRegisterBase];
}

void AudioController::_emitSample() {
    // emit an empty sample if sound is globally off
    if (!_soundOn) {
        _sampleCallback(0, 0);
        return;
    }
    
    // get current individual sound volumes
    double sound1 = _sound1.getSample();
    double sound2 = _sound2.getSample();
    double sound3 = _sound3.getSample();
    double sound4 = _sound4.getSample();
    
    double leftSample = 0.0;
    double rightSample = 0.0;
    uint8_t selectionValue = _audioRegisters[NR51Register - AudioRegisterBase];
    if (isMaskSet(selectionValue, 0x10)) {
        leftSample += sound1;
    }
    if (isMaskSet(selectionValue, 0x20)) {
        leftSample += sound2;
    }
    if (isMaskSet(selectionValue, 0x40)) {
        leftSample += sound3;
    }
    if (isMaskSet(selectionValue, 0x80)) {
        leftSample += sound4;
    }
    if (isMaskSet(selectionValue, 0x1)) {
        rightSample += sound1;
    }
    if (isMaskSet(selectionValue, 0x2)) {
        rightSample += sound2;
    }
    if (isMaskSet(selectionValue, 0x4)) {
        rightSample += sound3;
    }
    if (isMaskSet(selectionValue, 0x8)) {
        rightSample += sound4;
    }
    
    leftSample /= 4.0;
    rightSample /= 4.0;
    
    int16_t integerLeftSample = (leftSample * _leftVolume) * SampleMaxVolume;
    int16_t integerRightSample = (rightSample * _rightVolume) * SampleMaxVolume;
    _sampleCallback(integerLeftSample, integerRightSample);
}

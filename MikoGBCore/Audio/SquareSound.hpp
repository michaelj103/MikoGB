//
//  SquareSound.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef SquareSound_hpp
#define SquareSound_hpp

#include <cstdlib>

namespace MikoGB {

/// Models the state of a square waveform generator (the GB has two types, one with sweep and one without)
/// Told about writes to relevant memory offsets as they happen (mapped audio registers) and elapsed cycles
/// after every CPU step. Output is a sample which can be requested at any time, expected to be requested at 44100kHz
class SquareSound {
public:
    SquareSound(bool hasSweep): _hasSweep(hasSweep) {}
    
    void updateWithCycles(int cycles);
    
    // returns the value to store for future reads
    uint8_t soundWrite(uint16_t offset, uint8_t val);
    
    // sample is a value from 0.0 - 1.0
    double getSample() const;
    bool isRunning() const { return _isRunning; }
    
private:
    bool _isRunning = false;
    const bool _hasSweep;
    
    // sweep
    int _sweepTimePending = 0;
    int _sweepTime = 0; // CPU cycles per sweep event. 0 means sweep disabled
    int _sweepSignPending = 1;
    int _sweepSign = 1;
    int _sweepShiftPending = 1;
    int _sweepShift = 0;
    int _sweepCounter = 0; // remaining CPU cycles until next sweep event
    void _resetSweep(uint8_t val);
    
    // duty & duration
    int _pendingDuty = 0;
    int _duty = 0;
    int _pendingDurationTime = 0;
    int _durationTime = 0; // initial duration CPU cycles
    int _durationCounter = 0; // remaining CPU cycles until sound ends
    int _pendingDurationEnabled = false;
    bool _durationEnabled = false; // controlled by frequency high register below
    void _resetDutyAndDuration(uint8_t val);
    
    // envelope
    int _pendingEnvelopeVolume = 0; // initial envelope volume (0-15)
    int _envelopeVolume = 0; // current envelope volume (0-15)
    int _pendingEnvelopeSign = 1;
    int _envelopeSign = 1; // +1 = amplify. -1 = attenuate
    int _pendingEnvelopeStepTime = 0;
    int _envelopeStepTime = 0; // CPU cycles per envelope step
    int _envelopeStepCounter = 0; // remaining CPU cycles for the current envelope step
    void _resetEnvelope(uint8_t val);
    
    // frequency
    int _pendingFreq = 0;
    int _freq = 0; // frequency value in control registers. must be transformed
    int _freqCycles = 0; // CPU cycles per wave duty period (1/8th of wave frequency)
    int _freqCounter = 0; // remaining cycles in the current wave duty period
    int _waveDutyPeriod = 0;
    void _resetFreqLow(uint8_t val);
    void _resetFreqHigh(uint8_t val);
    void _updateFreqCounter();
    
    void _initialize();
};

}

#endif /* SquareSound_hpp */

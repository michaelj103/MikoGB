//
//  WaveformSound.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/26/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef WaveformSound_hpp
#define WaveformSound_hpp

#include <cstdlib>
#include <array>

namespace MikoGB {

/// Models the state of the custom waveform generator (#3 of the GB's 4 sound circuits)
/// Told about writes to relevant memory offsets as they happen (mapped audio registers) and elapsed cycles
/// after every CPU step. Output is a sample which can be requested at any time, expected to be requested at 44100kHz
class WaveformSound {
public:
    void updateWithCycles(int cycles);
    
    // returns the value to store for future reads
    // offset is from NR30 (0xFF1A)
    uint8_t soundWrite(uint16_t offset, uint8_t val);
    
    // offset is from beginning of wave pattern ram (0xFF30)
    void customSampleWrite(uint16_t offset, uint8_t val);
    
    // volume is a value from -1.0 - 1.0
    double getSample() const;
    bool isRunning() const { return _isRunning; }
    
private:
    bool _isRunning = false;
    
    bool _enabled = false;
    void _resetEnabled(uint8_t val);
    
    int _durationTime = 0; // initial duration CPU cycles
    int _durationCounter = 0; // remaining CPU cycles until sound ends
    bool _durationEnabled = false; // controlled by frequency high register below
    void _resetDuration(uint8_t val);
    
    int _outputLevel = 0;
    void _resetOutputLevel(uint8_t val);
    
    // frequency
    int _freq = 0; // frequency value in control registers. must be transformed
    int _freqCycles = 0; // CPU cycles per index in the sample (1/32nd of wave frequency)
    int _freqCounter = 0; // remaining cycles in the current sample index
    int _waveSampleIndex = 0;
    void _resetFreqLow(uint8_t val);
    void _resetFreqHigh(uint8_t val);
    void _updateFreqCounter();
    
    void _initialize();
    
    // custom waveform is 32 4-bit samples
    // The are offset by the shift value and converted to doubles in [0.0, 1.0] before output
    std::array<uint8_t, 32> _samples = std::array<uint8_t, 32>();
};

}

#endif /* WaveformSound_hpp */

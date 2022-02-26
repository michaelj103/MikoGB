//
//  AudioController.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef AudioController_hpp
#define AudioController_hpp

#include <array>
#include "SquareSound.hpp"
#include "NoiseSound.hpp"
#include "GameBoyCoreTypes.h"

namespace MikoGB {

class AudioController {
public:
    AudioController();
    
    void writeAudioRegister(uint16_t addr, uint8_t val);
    
    uint8_t readAudioRegister(uint16_t addr) const;
    
    // CPU cycles are 4x instruction cycles. 4.2MHz (2^22)
    void updateWithCPUCycles(int cycles);
    
    void setSampleCallback(AudioSampleCallback callback) {
        _sampleCallback = callback;
    }
    
private:
    // Audio registers range from 0xFF10 - 0xFF3F so there are 0x30 of them (48)
    // Some are unused
    std::array<uint8_t, 0x30> _audioRegisters;
    
    bool _soundOn = false;
    double _leftVolume = 0.0;
    double _rightVolume = 0.0;
    SquareSound _sound1;
    SquareSound _sound2;
    NoiseSound _sound4;
    
    int _nextSampleCounter = 0;
    void _emitSample();
    
    AudioSampleCallback _sampleCallback;
};

}

#endif /* AudioController_hpp */

//
//  Joypad.cpp
//  MikoGB
//
//  Created on 5/26/21.
//

#include "Joypad.hpp"
#include "BitTwiddlingUtil.h"

using namespace std;
using namespace MikoGB;

void Joypad::setButtonPressed(JoypadButton button, bool set) {
    bool wasSet = getButtonPressed(button);
    int bVal = static_cast<int>(button);
    uint8_t mask = 1 << bVal;
    
    if (set && !wasSet) {
        _setButtons |= mask;
        // signal the interrupt if necessary
        // Not super clear across sources, but I believe the interrupt is only set for the selected inputs
        // Note that the selection of inputs is inverted, unset means selected
        MemoryController::InputMask inputMask = _memoryController->selectedInputMask();
        if (bVal <= 3 && !isMaskSet(inputMask, MemoryController::InputMask::Directional)) {
            _memoryController->requestInterrupt(MemoryController::InterruptFlag::Input);
        } else if (bVal > 3 && !isMaskSet(inputMask, MemoryController::InputMask::Button)) {
            _memoryController->requestInterrupt(MemoryController::InterruptFlag::Input);
        }
        
    } else if (!set && wasSet) {
        _setButtons &= ~mask;
    }
}

bool Joypad::getButtonPressed(JoypadButton button) const {
    int bVal = static_cast<int>(button);
    uint8_t mask = 1 << bVal;
    return isMaskSet(_setButtons, mask);
}

uint8_t Joypad::readJoypadRegister() const {
    // Button input is inverted
    const uint8_t inverted = ~_setButtons;
    const uint8_t directional = inverted & 0x0F;
    const uint8_t button = (inverted >> 4) & 0x0F;
    
    // Some assumptions with how this works, no documentation is very clear to me. Basically to read joypad input,
    // Games will write the selection masks and then read the low 4 bits. It's feasible (though silly) that both
    // may be selected at the same time. I'm assuming that directional has priority but maybe that's not true
    uint8_t joypadReg = 0;
    MemoryController::InputMask inputMask = _memoryController->selectedInputMask();
    if (!isMaskSet(inputMask, MemoryController::InputMask::Directional)) {
        joypadReg |= directional;
    } else if (!isMaskSet(inputMask, MemoryController::InputMask::Button)) {
        joypadReg |= button;
    }
    
    return joypadReg;
}

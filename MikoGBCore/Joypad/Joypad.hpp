//
//  Joypad.hpp
//  MikoGB
//
//  Created on 5/26/21.
//

#ifndef Joypad_hpp
#define Joypad_hpp

#include "MemoryController.hpp"
#include "GameBoyCoreTypes.h"

namespace MikoGB {

class Joypad {
public:
    Joypad(MemoryController::Ptr &memoryController): _memoryController(memoryController) {}
    using Ptr = std::shared_ptr<Joypad>;
    
    void setButtonPressed(JoypadButton, bool);
    bool getButtonPressed(JoypadButton) const;
    
    uint8_t readJoypadRegister() const;
    bool wantsStop() const;
    
private:
    MemoryController::Ptr &_memoryController;
    uint8_t _setButtons = 0;
};

}

#endif /* Joypad_hpp */

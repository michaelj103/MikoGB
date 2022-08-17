//
//  SerialController.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef SerialController_hpp
#define SerialController_hpp

#include "MemoryController.hpp"
#include "GameBoyCoreTypes.h"

namespace MikoGB {

class SerialController {
public:
    SerialController(MemoryController::Ptr &memoryController): _memoryController(memoryController) {}
    using Ptr = std::shared_ptr<SerialController>;
    
    // CPU cycles are 4x instruction cycles. 4.2MHz (2^22)
    void updateWithCPUCycles(int cycles);
    
    void serialDataWillWrite(uint8_t dataByte) const;
    void serialControlWillWrite(uint8_t controlByte);
    
private:
    MemoryController::Ptr &_memoryController;
    
    enum class SerialState {
        Idle,
        Presenting,
        Transferring,
    };
    
    SerialState _state = SerialState::Idle;
    void _setState(SerialState state);
    int _transferCounter = 0; // cpu cycles until transfer is expected to complete
    
    void _presentByte(uint8_t byte) const;
    void _pushByte(uint8_t byte) const;
};

}

#endif /* SerialController_hpp */

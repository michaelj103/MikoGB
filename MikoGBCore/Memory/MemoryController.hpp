//
//  MemoryController.hpp
//  MikoGB
//
//  Created on 5/16/21.
//

#ifndef MemoryController_hpp
#define MemoryController_hpp

#include <cstdlib>
#include "CartridgeHeader.hpp"

namespace MikoGB {

class MemoryBankController;

class MemoryController {
public:
    MemoryController() = default;
    ~MemoryController();
    
    bool configureWithROMData(const void *romData, size_t size);
    bool configureWithEmptyData();
        
    uint8_t readByte(uint16_t addr) const;
    void setByte(uint16_t addr, uint8_t val);
        
    const CartridgeHeader &getHeader() const { return _header; }
    
private:
    uint8_t *_permanentROM = nullptr;
    uint8_t *_videoRAM= nullptr;
    uint8_t *_highRangeMemory= nullptr;
    CartridgeHeader _header;
    bool _bootROMEnabled = true;
    
    MemoryBankController *_mbc = nullptr;
};

}

#endif /* MemoryController_hpp */

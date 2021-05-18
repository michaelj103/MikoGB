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
    
    bool configureWithROMData(void *romData, size_t size);
    bool configureWithEmptyData();
        
    uint8_t readByte(uint16_t addr) const;
    void setByte(uint16_t addr, uint8_t val);
        
    const CartridgeHeader &getHeader() const { return _header; }
    
private:
    uint8_t *_permanentROM;
    uint8_t *_videoRAM;
    uint8_t *_highRangeMemory;
    CartridgeHeader _header;
    bool _bootROMEnabled = true;
    
    MemoryBankController *_mbc;
};

}

#endif /* MemoryController_hpp */

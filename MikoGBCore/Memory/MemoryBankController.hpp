//
//  MemoryBankController.hpp
//  MikoGB
//
//  Created on 5/17/21.
//

#ifndef MemoryBankController_hpp
#define MemoryBankController_hpp

#include <cstdlib>
#include "CartridgeHeader.hpp"

namespace MikoGB {

/// Base class for memory bank controllers
class MemoryBankController {
public:
    MemoryBankController();
    virtual ~MemoryBankController();
    
    static MemoryBankController *CreateMBC(const CartridgeHeader &header);
    
    virtual bool configureWithROMData(void *romData, size_t size);
    
    /// Read from currently switched ROM bank
    virtual uint8_t readROM(uint16_t addr) const = 0;
    
    /// Read from currently switched external RAM bank
    virtual uint8_t readRAM(uint16_t addr) const = 0;
    
    /// Write to external RAM, potentially switched
    virtual void writeRAM(uint16_t addr, uint8_t val) const = 0;
    
    /// Write to the ROM area which is a control code (or invalid)
    virtual void writeControlCode(uint16_t addr, uint8_t val) const = 0;
        
protected:
    uint8_t *_romData;
    size_t _romSize;
    
};

}

#endif /* MemoryBankController_hpp */

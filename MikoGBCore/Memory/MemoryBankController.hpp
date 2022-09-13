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
    MemoryBankController() = default;
    virtual ~MemoryBankController();
    
    static MemoryBankController *CreateMBC(const CartridgeHeader &header);
    
    bool isPersistenceStale() const { return _isPersistenceStale; }
    void resetPersistence() { _isPersistenceStale = false; }
    bool isClockPersistenceStale() const { return _isClockPersistenceStale; }
    void resetClockPersistence() { _isClockPersistenceStale = false; }
    
    virtual bool configureWithROMData(const void *romData, size_t size);
    
    /// Read from currently switched ROM bank
    virtual uint8_t readROM(uint16_t addr) const = 0;
    
    /// Read from currently switched external RAM bank
    virtual uint8_t readRAM(uint16_t addr) const = 0;
    
    /// Write to external RAM, potentially switched
    virtual void writeRAM(uint16_t addr, uint8_t val) = 0;
    
    /// Write to the ROM area which is a control code (or invalid)
    virtual void writeControlCode(uint16_t addr, uint8_t val) = 0;
    
    /// Get the current ROM bank number. For debugging or diagnostics
    virtual int currentROMBank() const = 0;
    
    /// Some MBCs maintain a real-time clock. Supply real-time seconds elapsed to increment it
    virtual void updateClock(size_t secondsElapsed);
    
    virtual size_t saveDataSize() const = 0;
    virtual void *getSaveData() const = 0;
    virtual bool loadSaveData(const void *saveData, size_t size) = 0;
    
    virtual size_t clockDataSize() const;
    virtual size_t copyClockData(void *buffer, size_t size) const;
    virtual bool loadClockData(const void *clockData, size_t size);
        
protected:
    uint8_t *_romData = nullptr;
    size_t _romSize = 0;
    bool _isPersistenceStale = false;
    bool _isClockPersistenceStale = false;
};

}

#endif /* MemoryBankController_hpp */

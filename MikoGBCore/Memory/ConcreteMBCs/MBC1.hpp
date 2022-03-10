//
//  MBC1.hpp
//  MikoGB
//
//  Created on 6/11/21.
//

#ifndef MBC1_hpp
#define MBC1_hpp

#include "MemoryBankController.hpp"

namespace MikoGB {

class MBC1 : public MemoryBankController {
public:
    MBC1(const CartridgeHeader &header);
    ~MBC1();
    
    bool configureWithROMData(const void *romData, size_t size) override;
    uint8_t readROM(uint16_t addr) const override;
    uint8_t readRAM(uint16_t addr) const override;
    void writeRAM(uint16_t addr, uint8_t val) override;
    void writeControlCode(uint16_t addr, uint8_t val) override;
    int currentROMBank() const override;
    size_t saveDataSize() const override;
    void *getSaveData() const override;
    bool loadSaveData(const void *saveData, size_t size) override;
    
private:
    int _romBankCount;
    int _ramBankCount;
    bool _ramEnabled = false;
    uint8_t *_ramData = nullptr;
    
    uint8_t _romBankLower = 1;
    uint8_t _bankNumberUpper = 0;
    uint8_t _bankingMode = 0;
    int _romBank = 1;
    int _ramBank = 0;
    bool _batteryBackup = false;
    
    void _updateBankNumbers();
};

}

#endif /* MBC1_hpp */

//
//  MBC3.hpp
//  MikoGB
//
//  Created on 6/22/21.
//

#ifndef MBC3_hpp
#define MBC3_hpp

#include "MemoryBankController.hpp"

namespace MikoGB {

class MBC3 : public MemoryBankController {
public:
    MBC3(const CartridgeHeader &header);
    ~MBC3();
    
    bool configureWithROMData(const void *romData, size_t size) override;
    uint8_t readROM(uint16_t addr) const override;
    uint8_t readRAM(uint16_t addr) const override;
    void writeRAM(uint16_t addr, uint8_t val) override;
    void writeControlCode(uint16_t addr, uint8_t val) override;
    int currentROMBank() const override;
    
private:
    int _romBankCount;
    int _ramBankCount;
    bool _ramEnabled = false;
    uint8_t *_ramData = nullptr;
    
    uint8_t _romBankCode = 0;
    uint8_t _ramBankCode = 0;
    uint8_t _latchVal = 0;
    int _romBank = 1;
    int _ramBank = 0;
    
    void _updateBankNumbers();
};

}

#endif /* MBC3_hpp */

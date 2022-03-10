//
//  NoMBC.hpp
//  MikoGB
//
//  Created on 5/17/21.
//

#ifndef NoMBC_hpp
#define NoMBC_hpp

#include "MemoryBankController.hpp"

namespace MikoGB {

class NoMBC : public MemoryBankController {
    enum class RAMType {
        Invalid,
        None,
        SingleBank,
    };
    RAMType _ramType = RAMType::Invalid;
    uint8_t *_ramData = nullptr;
    
public:
    NoMBC(const CartridgeHeader &header);
    ~NoMBC();
    
    bool configureWithROMData(const void *romData, size_t size) override;
    uint8_t readROM(uint16_t addr) const override;
    uint8_t readRAM(uint16_t addr) const override;
    void writeRAM(uint16_t addr, uint8_t val) override;
    void writeControlCode(uint16_t addr, uint8_t val) override;
    int currentROMBank() const override;
    size_t saveDataSize() const override;
    void *getSaveData() const override;
};

}

#endif /* NoMBC_hpp */

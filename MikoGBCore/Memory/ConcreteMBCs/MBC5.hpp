//
//  MBC5.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 9/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef MBC5_hpp
#define MBC5_hpp

#include "MemoryBankController.hpp"

namespace MikoGB {

class MBC5 : public MemoryBankController {
public:
    MBC5(const CartridgeHeader &header);
    ~MBC5();
    
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
    uint8_t _romBankUpper = 0;
    uint8_t _ramBankRegister = 0;
    int _romBank = 1;
    int _ramBank = 0;
    bool _hasBatteryBackup = false;
    bool _hasRumble = false;

    void _updateBankNumbers();
};

}

#endif /* MBC5_hpp */

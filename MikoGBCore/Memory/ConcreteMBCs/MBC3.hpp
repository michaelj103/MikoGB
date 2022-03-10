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

enum MBC3Clock : int {
    RTC_S = 0,      // seconds register, takes values 0-59
    RTC_M = 1,      // minutes register, takes values 0-59
    RTC_H = 2,      // hours register, takes values 0-23
    RTC_DL = 3,     // days register, takes values 0-255
    RTC_DH = 4,     // days high register, includes 9th bit of day count, carry and halt bits
};

class MBC3 : public MemoryBankController {
public:
    MBC3(const CartridgeHeader &header);
    ~MBC3();
    
    bool configureWithROMData(const void *romData, size_t size) override;
    uint8_t readROM(uint16_t addr) const override;
    uint8_t readRAM(uint16_t addr) const override;
    void writeRAM(uint16_t addr, uint8_t val) override;
    void writeControlCode(uint16_t addr, uint8_t val) override;
    void updateClock(size_t cpuCycles) override;
    int currentROMBank() const override;
    size_t saveDataSize() const override;
    void *getSaveData() const override;
    
private:
    int _romBankCount;
    int _ramBankCount;
    bool _ramEnabled = false;
    uint8_t *_ramData = nullptr;
    
    uint8_t _romBankCode = 0;
    uint8_t _ramBankCode = 0;
    uint8_t _latchVal = 0;
    size_t _clockCount = 0;
    int _romBank = 1;
    int _ramBank = 0;
    bool _batteryBackup = false;
    
    uint8_t _clockRegisters[5];
    
    void _updateBankNumbers();
    void _latchClockRegisters();
    void _writeClockRegister(MBC3Clock reg, uint8_t val);
};

}

#endif /* MBC3_hpp */

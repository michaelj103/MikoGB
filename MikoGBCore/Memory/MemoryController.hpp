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
class Joypad;

class MemoryController {
public:
    MemoryController() = default;
    ~MemoryController();
    using Ptr = std::shared_ptr<MemoryController>;
    
    bool configureWithROMData(const void *romData, size_t size);
    bool configureWithEmptyData();
        
    uint8_t readByte(uint16_t addr) const;
    void setByte(uint16_t addr, uint8_t val);
        
    const CartridgeHeader &getHeader() const { return _header; }
    
    // Interrupts
    enum InterruptFlag : uint8_t {
        VBlank      = 1 << 0,
        LCDStat     = 1 << 1,
        Timer       = 1 << 2,
        Serial      = 1 << 3,
        Input       = 1 << 4,
    };
    void requestInterrupt(InterruptFlag);
    static const uint16_t IFRegister = 0xFF0F; // Interrupt Request
    static const uint16_t IERegister = 0xFFFF; // Interrupt Enable
    
    enum InputMask : uint8_t {
        Button = 0x10, // A, B, Sel, Start
        Directional = 0x20, // D-pad
    };
    InputMask selectedInputMask() const;
    std::shared_ptr<Joypad> joypad;
    
private:
    uint8_t *_permanentROM = nullptr;
    uint8_t *_videoRAM = nullptr;
    uint8_t *_highRangeMemory = nullptr;
    CartridgeHeader _header;
    bool _bootROMEnabled = true;
    
    MemoryBankController *_mbc = nullptr;
    
    void _dmaTransfer(uint8_t);
};

}

#endif /* MemoryController_hpp */

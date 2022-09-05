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
#include "Timer.hpp"
#include "AudioController.hpp"

namespace MikoGB {

class MemoryBankController;
class Joypad;
class SerialController;
class GPUCore;

class MemoryController {
public:
    MemoryController() = default;
    ~MemoryController();
    using Ptr = std::shared_ptr<MemoryController>;
    
    bool configureWithROMData(const void *romData, size_t size);
    bool configureWithEmptyData();
    bool configureWithColorBootROM(const void *romData, size_t size);
        
    uint8_t readByte(uint16_t addr) const;
    void setByte(uint16_t addr, uint8_t val);
    
    void updateWithCPUCycles(size_t cpuCycles);
        
    const CartridgeHeader &getHeader() const { return _header; }
    
    // Interrupts
    enum InterruptFlag : uint8_t {
        VBlank      = 1 << 0,
        LCDStat     = 1 << 1,
        TIMA        = 1 << 2,
        Serial      = 1 << 3,
        Input       = 1 << 4,
    };
    void requestInterrupt(InterruptFlag);
    static const uint16_t IFRegister = 0xFF0F; // Interrupt Request
    static const uint16_t IERegister = 0xFFFF; // Interrupt Enable
    
    // GPU
    std::shared_ptr<GPUCore> gpu;
    
    // Joypad
    enum InputMask : uint8_t {
        Directional = 0x10, // D-pad
        Button = 0x20, // A, B, Sel, Start
    };
    InputMask selectedInputMask() const;
    std::shared_ptr<Joypad> joypad;
    
    // Serial
    std::shared_ptr<SerialController> serialController;
    
    // Audio
    void setAudioSampleCallback(AudioSampleCallback callback);
    
    // Persistence
    bool isPersistenceStale() const;
    void resetPersistence();
    
    // Debugging and introspection
    int currentROMBank() const;
    
    bool bootROMEnabled() const { return _bootROMEnabled; }
    size_t saveDataSize() const;
    size_t copySaveData(void *buffer, size_t size) const;
    bool loadSaveData(const void *saveData, size_t size);
    
    void hBlankDMATransferStep();
    
private:
    uint8_t *_permanentROM = nullptr;
    uint8_t *_videoRAMBank0 = nullptr;
    uint8_t *_videoRAMBank1 = nullptr;
    uint8_t *_videoRAMCurrentBank = nullptr;
    uint8_t *_workingRAM = nullptr;
    uint8_t *_highRangeMemory = nullptr;
    CartridgeHeader _header;
    bool _bootROMEnabled = true;
    bool _colorBootROMEnabled = false;
    uint8_t *_colorBootROM = nullptr;
    size_t _saveDataSize = false;
    uint8_t _switchableWRAMBank = 1;
    
    MemoryBankController *_mbc = nullptr;
    Timer _timer;
    AudioController _audioController;
    
    bool _isHBlankTransferActive = false;
    uint16_t _hBlankTransferSource = 0;
    uint16_t _hBlankTransferDst = 0;
    void _dmaTransfer(uint8_t);
    void _generalPurposeDMATransfer(uint8_t);
    void _startHBlankDMATransfer();
    void _directSetHighRange(uint16_t addr, uint8_t val);
};

}

#endif /* MemoryController_hpp */

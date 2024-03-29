//
//  GameBoyCoreImp.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GameBoyCoreImp.hpp"
#include "GameBoyCoreTypes.h"

using namespace std;
using namespace MikoGB;

GameBoyCoreImp::GameBoyCoreImp() {
    // For now, initialize the CPU core with the bootstrap ROM and valid cartridge logo data
    _memoryController = make_shared<MemoryController>();
    _cpu = make_shared<CPUCore>(_memoryController);
    _gpu = make_shared<GPUCore>(_memoryController);
    _memoryController->gpu = _gpu;
    _joypad = make_shared<Joypad>(_memoryController);
    _memoryController->joypad = _joypad;
    _serialController = make_shared<SerialController>(_memoryController);
    _memoryController->serialController = _serialController;
}

bool GameBoyCoreImp::loadROMData(const void *romData, size_t size, const void *bootRomData, size_t bootRomSize) {
    // TODO: rather than creating everything in the constructor, (re-)create it on load
    if (_cpu->programCounter != 0) {
        // Must not have already started running
        return false;
    }
    bool success = _memoryController->configureWithROMData(romData, size);
    if (bootRomData != nullptr) {
        success = success && _memoryController->configureWithColorBootROM(bootRomData, bootRomSize);
        _gpu->enableCGBRendering();
    }
    return success;
}

void GameBoyCoreImp::prepTestROM() {
    _memoryController->configureWithEmptyData();
}

size_t GameBoyCoreImp::saveDataSize() const {
    return _memoryController->saveDataSize();
}

size_t GameBoyCoreImp::copySaveData(void *buffer, size_t size) const {
    return _memoryController->copySaveData(buffer, size);
}

bool GameBoyCoreImp::loadSaveData(const void *saveData, size_t size) {
    return _memoryController->loadSaveData(saveData, size);
}

size_t GameBoyCoreImp::clockDataSize() const {
    return _memoryController->clockDataSize();
}

size_t GameBoyCoreImp::copyClockData(void *buffer, size_t size) const {
    return _memoryController->copyClockData(buffer, size);
}

bool GameBoyCoreImp::loadClockData(const void *clockData, size_t size) {
    return _memoryController->loadClockData(clockData, size);
}

void GameBoyCoreImp::step() {
    int instructionCycles = _cpu->step();
    size_t cpuCycles = instructionCycles * 4;
    // in double-speed mode, GPU cycles take half as long so that GPU events happen in "real" time
    const bool isDoubleSpeed = _memoryController->isDoubleSpeedModeEnabled();
    const size_t cyclesForGPU = isDoubleSpeed ? cpuCycles : cpuCycles * 2;
    _gpu->updateWithCPUCycles(cyclesForGPU);
    _memoryController->updateWithCPUCycles(cpuCycles);
#if ENABLE_DEBUGGER
    if (_cpu->isStoppedAtBreakpoint()) {
        setRunnable(false);
    }
#endif
}

void GameBoyCoreImp::emulateFrame() {
    auto gpu = _gpu.get();
    // If we're in the middle of a frame, run until the start of the next
    while (gpu->getCurrentScanline() != 0 && _isRunnable) {
        step();
    }
    
    // Run until v-blank at line 144
    while (gpu->getCurrentScanline() < 144 && _isRunnable) {
        step();
    }
}

void GameBoyCoreImp::updateWithRealTimeSeconds(size_t secondsElapsed) {
    _memoryController->updateWithRealTimeSeconds(secondsElapsed);
}

void GameBoyCoreImp::emulateFrameStep() {
    auto gpu = _gpu.get();
    // If we're in the middle of a frame, run until the start of the next
    while (gpu->getCurrentScanline() != 0) {
        step();
    }
    
    // Run until v-blank at line 144
    while (gpu->getCurrentScanline() < 144) {
        step();
    }
}

void GameBoyCoreImp::setRunnable(bool runnable) {
    if (runnable == _isRunnable) {
        return;
    }
    _isRunnable = runnable;
    if (_runnableChangedCallback) {
        _runnableChangedCallback(runnable);
    }
}

void GameBoyCoreImp::setButtonPressed(JoypadButton button, bool set) {
    _joypad->setButtonPressed(button, set);
}

void GameBoyCoreImp::setScanlineCallback(PixelBufferScanlineCallback callback) {
    _gpu->setScanlineCallback(callback);
}

void GameBoyCoreImp::setAudioSampleCallback(AudioSampleCallback callback) {
    _memoryController->setAudioSampleCallback(callback);
}

bool GameBoyCoreImp::isPersistenceStale() const {
    return _memoryController->isPersistenceStale();
}

void GameBoyCoreImp::resetPersistence() {
    _memoryController->resetPersistence();
}

bool GameBoyCoreImp::isClockPersistenceStale() const {
    return _memoryController->isClockPersistenceStale();
}

void GameBoyCoreImp::resetClockPersistence() {
    _memoryController->resetClockPersistence();
}

uint8_t GameBoyCoreImp::currentSerialDataByte() const {
    return _serialController->getCurrentDataByte();
}

void GameBoyCoreImp::handleIncomingSerialRequest(SerialIncoming incoming, uint8_t payload) {
    _serialController->handleIncomingEvent(incoming, payload);
}

void GameBoyCoreImp::setSerialEventCallback(SerialEventCallback callback) {
    _serialController->setEventCallback(callback);
}

void GameBoyCoreImp::getTileMap(PixelBufferImageCallback callback) {
    _gpu->getTileMap(callback);
}

void GameBoyCoreImp::getBackground(PixelBufferImageCallback callback) {
    _gpu->getBackground(callback);
}

void GameBoyCoreImp::getWindow(PixelBufferImageCallback callback) {
    _gpu->getWindow(callback);
}

Disassembler::Ptr GameBoyCoreImp::_accessDisassembler() {
    if (!_disassembler) {
        _disassembler = make_shared<Disassembler>();
    }
    return _disassembler;
}

std::vector<DisassembledInstruction> GameBoyCoreImp::getDisassembledInstructions(int lookAheadCount, int lookBehindCount, size_t *currentIdx) {
    Disassembler::Ptr disassembler = _accessDisassembler();
    uint16_t pc = _cpu->programCounter;
    vector<DisassembledInstruction> forward = disassembler->disassembleInstructions(pc, lookAheadCount, _memoryController);
    vector<DisassembledInstruction> backward = disassembler->precedingDisassembledInstructions(pc, lookBehindCount, _memoryController, _cpu);
    
    if (currentIdx) {
        *currentIdx = backward.size();
    }
    
    vector<DisassembledInstruction> combined;
    combined.reserve(backward.size() + forward.size());
    combined.insert(combined.end(), backward.begin(), backward.end());
    combined.insert(combined.end(), forward.begin(), forward.end());
    
    return combined;
}

std::vector<DisassembledInstruction> GameBoyCoreImp::getDisassembledPreviousInstructions(int count) {
    Disassembler::Ptr disassembler = _accessDisassembler();
    vector<DisassembledInstruction> disassembled = disassembler->lastExecutedInstructions(count, _memoryController, _cpu);
    return disassembled;
}

RegisterState GameBoyCoreImp::getRegisterState() const {
    uint8_t *registers = _cpu->registers;
    RegisterState state;
    state.B = registers[REGISTER_B];
    state.C = registers[REGISTER_C];
    state.D = registers[REGISTER_D];
    state.E = registers[REGISTER_E];
    state.H = registers[REGISTER_H];
    state.L = registers[REGISTER_L];
    state.A = registers[REGISTER_A];
    
    uint8_t flag = registers[REGISTER_F];
    state.ZFlag = (flag & FlagBit::Zero) != 0;
    state.NFlag = (flag & FlagBit::N) != 0;
    state.HFlag = (flag & FlagBit::H) != 0;
    state.CFlag = (flag & FlagBit::Carry) != 0;
    
    return state;
}

uint8_t GameBoyCoreImp::readMem(uint16_t addr) const {
    return _memoryController->readByte(addr);
}

void GameBoyCoreImp::setLineBreakpoint(int romBank, uint16_t addr) {
    _cpu->_breakpointManager.addLineBreakpoint(romBank, addr);
}

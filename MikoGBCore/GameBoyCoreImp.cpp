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
    _joypad = make_shared<Joypad>(_memoryController);
    _memoryController->joypad = _joypad;
}

bool GameBoyCoreImp::loadROMData(const void *romData, size_t size) {
    // TODO: rather than creating everything in the constructor, (re-)create it on load
    return _memoryController->configureWithROMData(romData, size);
}

void GameBoyCoreImp::prepTestROM() {
    _memoryController->configureWithEmptyData();
}

size_t GameBoyCoreImp::saveDataSize() const {
    return _memoryController->saveDataSize();
}

void GameBoyCoreImp::step() {
    int instructionCycles = _cpu->step();
    size_t cpuCycles = instructionCycles * 4;
    _gpu->updateWithCPUCycles(cpuCycles);
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

void GameBoyCoreImp::getTileMap(PixelBufferImageCallback callback) {
    _gpu->getTileMap(callback);
}

void GameBoyCoreImp::getBackground(PixelBufferImageCallback callback) {
    _gpu->getBackground(callback);
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

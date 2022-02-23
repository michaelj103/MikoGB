//
//  GameBoyCore.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GameBoyCore.hpp"
#include "GameBoyCoreImp.hpp"

using namespace MikoGB;

GameBoyCore::GameBoyCore() {
    //TODO: use shared pointers
    _imp = new GameBoyCoreImp();
}

GameBoyCore::~GameBoyCore() {
    delete _imp;
}

bool GameBoyCore::loadROMData(const void *romData, size_t size) {
    return _imp->loadROMData(romData, size);
}

void GameBoyCore::prepTestROM() {
    _imp->prepTestROM();
}

void GameBoyCore::step() {
    _imp->step();
}

void GameBoyCore::emulateFrame() {
    _imp->emulateFrame();
}

void GameBoyCore::setExternallyRunnable(bool runnable) {
    _imp->setExternallyRunnable(runnable);
}

bool GameBoyCore::isExternallyRunnable() const {
    return _imp->isExternallyRunnable();
}

bool GameBoyCore::isRunnable() const {
    return _imp->isRunnable();
}

void GameBoyCore::setRunnableChangedCallback(RunnableChangedCallback callback) {
    _imp->setRunnableChangedCallback(callback);
}

void GameBoyCore::setButtonPressed(JoypadButton button, bool set) {
    _imp->setButtonPressed(button, set);
}

void GameBoyCore::setScanlineCallback(PixelBufferScanlineCallback callback) {
    _imp->setScanlineCallback(callback);
}

uint16_t GameBoyCore::getPC() const {
    return _imp->_cpu->programCounter;
}

void GameBoyCore::getTileMap(PixelBufferImageCallback callback) {
    _imp->getTileMap(callback);
}

void GameBoyCore::getBackground(PixelBufferImageCallback callback) {
    _imp->getBackground(callback);
}

std::vector<DisassembledInstruction> GameBoyCore::getDisassembledInstructions(int lookAheadCount, int lookBehindCount, size_t *currentIdx) {
    return _imp->getDisassembledInstructions(lookAheadCount, lookBehindCount, currentIdx);
}

RegisterState GameBoyCore::getRegisterState() const {
    return _imp->getRegisterState();
}

uint8_t GameBoyCore::readMem(uint16_t addr) const {
    return _imp->readMem(addr);
}

bool GameBoyCore::setLineBreakpoint(int romBank, uint16_t addr) {
#if ENABLE_DEBUGGER
    _imp->setLineBreakpoint(romBank, addr);
    return true;
#else
    // breakpoints are not enabled in this build
    return false;
#endif
}

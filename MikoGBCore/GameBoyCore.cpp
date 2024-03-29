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

bool GameBoyCore::loadROMData(const void *romData, size_t size, const void *colorBootROMData, size_t bootRomSize) {
    return _imp->loadROMData(romData, size, colorBootROMData, bootRomSize);
}

void GameBoyCore::prepTestROM() {
    _imp->prepTestROM();
}

size_t GameBoyCore::saveDataSize() const {
    return _imp->saveDataSize();
}

size_t GameBoyCore::copySaveData(void *buffer, size_t size) const {
    return _imp->copySaveData(buffer, size);
}

bool GameBoyCore::loadSaveData(const void *saveData, size_t size) {
    return _imp->loadSaveData(saveData, size);
}

size_t GameBoyCore::clockDataSize() const {
    return _imp->clockDataSize();
}

size_t GameBoyCore::copyClockData(void *buffer, size_t size) const {
    return _imp->copyClockData(buffer, size);
}

bool GameBoyCore::loadClockData(const void *clockData, size_t size) {
    return _imp->loadClockData(clockData, size);
}

void GameBoyCore::step() {
    _imp->step();
}

void GameBoyCore::emulateFrame() {
    _imp->emulateFrame();
}

void GameBoyCore::updateWithRealTimeSeconds(size_t secondsElapsed) {
    _imp->updateWithRealTimeSeconds(secondsElapsed);
}

void GameBoyCore::emulateFrameStep() {
    _imp->emulateFrameStep();
}

void GameBoyCore::setRunnable(bool runnable) {
    _imp->setRunnable(runnable);
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

void GameBoyCore::setAudioSampleCallback(AudioSampleCallback callback) {
    _imp->setAudioSampleCallback(callback);
}

bool GameBoyCore::isPersistenceStale() const {
    return _imp->isPersistenceStale();
}

void GameBoyCore::resetPersistence() {
    _imp->resetPersistence();
}

bool GameBoyCore::isClockPersistenceStale() const {
    return _imp->isClockPersistenceStale();
}

void GameBoyCore::resetClockPersistence() {
    _imp->resetClockPersistence();
}

uint8_t GameBoyCore::currentSerialDataByte() const {
    return _imp->currentSerialDataByte();
}

void GameBoyCore::handleIncomingSerialRequest(SerialIncoming incoming, uint8_t payload) {
    _imp->handleIncomingSerialRequest(incoming, payload);
}

void GameBoyCore::setSerialEventCallback(SerialEventCallback callback) {
    _imp->setSerialEventCallback(callback);
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

void GameBoyCore::getWindow(PixelBufferImageCallback callback) {
    _imp->getWindow(callback);
}

std::vector<DisassembledInstruction> GameBoyCore::getDisassembledInstructions(int lookAheadCount, int lookBehindCount, size_t *currentIdx) const {
    return _imp->getDisassembledInstructions(lookAheadCount, lookBehindCount, currentIdx);
}

std::vector<DisassembledInstruction> GameBoyCore::getDisassembledPreviousInstructions(int count) const {
    return _imp->getDisassembledPreviousInstructions(count);
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

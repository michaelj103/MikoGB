//
//  GameBoyCoreImp.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GameBoyCoreImp.hpp"

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

void GameBoyCoreImp::step() {
    int instructionCycles = _cpu->step();
    size_t cpuCycles = instructionCycles * 4;
    _gpu->updateWithCPUCycles(cpuCycles);
    _memoryController->updateTimer(cpuCycles);
}

void GameBoyCoreImp::emulateFrame() {
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

void GameBoyCoreImp::setButtonPressed(JoypadButton button, bool set) {
    _joypad->setButtonPressed(button, set);
}

void GameBoyCoreImp::setScanlineCallback(PixelBufferScanlineCallback callback) {
    _gpu->setScanlineCallback(callback);
}

void GameBoyCoreImp::getTileMap(PixelBufferImageCallback callback) {
    _gpu->getTileMap(callback);
}

void GameBoyCoreImp::getBackground(PixelBufferImageCallback callback) {
    _gpu->getBackground(callback);
}

//
//  GameBoyCoreImp.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include "GameBoyCoreImp.hpp"

using namespace MikoGB;

GameBoyCoreImp::GameBoyCoreImp() {
    // For now, initialize the CPU core with the bootstrap ROM and valid cartridge logo data
    // TODO: use shared pointers and get rid of the destructor
    _memoryController = new MemoryController();
    _cpu = new CPUCore(_memoryController);
    _gpu = new GPUCore(_cpu);
}

bool GameBoyCoreImp::loadROMData(const void *romData, size_t size) {
    // TODO: rather than creating everything in the constructor, (re-)create it on load
    return _memoryController->configureWithROMData(romData, size);
}

void GameBoyCoreImp::prepTestROM() {
    _memoryController->configureWithEmptyData();
}

GameBoyCoreImp::~GameBoyCoreImp() {
    delete _cpu;
    delete _gpu;
    delete _memoryController;
}

void GameBoyCoreImp::step() {
    int instructionCycles = _cpu->step();
    size_t cpuCycles = instructionCycles * 4;
    _gpu->updateWithCPUCycles(cpuCycles);
}

void GameBoyCoreImp::emulateFrame() {
    // If we're in the middle of a frame, run until the start of the next
    while (_gpu->getCurrentScanline() != 0) {
        step();
    }
    
    // Run until v-blank at line 144
    while (_gpu->getCurrentScanline() < 144) {
        step();
    }
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

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

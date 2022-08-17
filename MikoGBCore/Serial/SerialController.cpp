//
//  SerialController.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "SerialController.hpp"
#include "BitTwiddlingUtil.h"

using namespace std;
using namespace MikoGB;

static const uint16_t SerialDataRegister = 0xFF01; // Byte queued for serial data Rx/Tx
static const uint16_t SerialControlRegister = 0xFF02; // Control bits for serial transfer

// For GB, the only transfer clock speed is 8192Hz where each cycle is 1 bit transferred
// that means that given the base clock speed of 2^22Hz, it will take 4096 cycles to transfer a byte
static const int CyclesPerTransfer = 4096; // 2^22 base clock speed

void SerialController::serialDataWillWrite(uint8_t dataByte) const {
    uint8_t lastKnownByte = _memoryController->readByte(SerialDataRegister);
    if (lastKnownByte != dataByte) {
        if (_state == SerialState::Presenting) {
            _presentByte(dataByte);
        }
    }
}

void SerialController::serialControlWillWrite(uint8_t controlByte) {
    uint8_t existingControlByte = _memoryController->readByte(SerialControlRegister);
    if (existingControlByte != controlByte) {
        if (isMaskSet(controlByte, 0x80)) {
            // if bit 7 is set, the client is ready for a transfer
            if (isMaskSet(controlByte, 0x01)) {
                // if bit 0 is set, the client is clocking ("pushing") the transfer
                _setState(SerialState::Transferring);
            } else {
                // if bit 0 is not set, the client is presenting a byte for "pull" by the other side
                _setState(SerialState::Presenting);
            }
        } else {
            _setState(SerialState::Idle);
        }
    }
}

void SerialController::_setState(SerialState state) {
    if (state == _state) {
        return;
    }
    _state = state;
    
    uint8_t dataByte = _memoryController->readByte(SerialDataRegister);
    switch (state) {
        case SerialState::Idle:
            break;
        case SerialState::Presenting:
            _presentByte(dataByte);
            break;
        case SerialState::Transferring:
            _pushByte(dataByte);
            _transferCounter = CyclesPerTransfer;
            break;
    }
}

void SerialController::_presentByte(uint8_t byte) const {
    printf("Serial: presenting byte %d\n", (int)byte);
}

void SerialController::_pushByte(uint8_t byte) const {
    printf("Serial: pushing byte %d\n", (int)byte);
}

void SerialController::updateWithCPUCycles(int cycles) {
    if (_state == SerialState::Transferring) {
        if (_transferCounter > cycles) {
            // transfer ongoing
            _transferCounter -= cycles;
        } else {
            // transfer complete
            printf("Serial: push complete\n");
            _transferCounter = 0;
            // 1. clear control byte high bit (this will indirectly enter the idle state)
            uint8_t controlByte = _memoryController->readByte(SerialControlRegister);
            _memoryController->setByte(SerialControlRegister, (controlByte & 0x7F));
            // 2. set received byte
            _memoryController->setByte(SerialDataRegister, 0xFF);
            // 3. fire serial interrupt
            _memoryController->requestInterrupt(MemoryController::Serial);
        }
    }
}

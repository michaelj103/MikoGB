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

uint8_t SerialController::getCurrentDataByte() const {
    uint8_t dataByte = _memoryController->readByte(SerialDataRegister);
    return dataByte;
}

void SerialController::handleIncomingEvent(SerialIncoming incoming, uint8_t payload) {
    switch (incoming) {
        case SerialIncoming::PulledByte:
            // we got a byte in response to an internal transfer. Commit it if ready
            _incomingByteToCommit = payload;
            _hasIncomingByte = true;
            _completeInternalTransferIfNecessary();
            break;
        case SerialIncoming::PushedByte:
            // we got a byte from an external transfer. Commit it if wanted
            _incomingByteToCommit = payload;
            _hasIncomingByte = true;
            _completeExternalTransferIfNecessary();
            break;
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
            // When first entering the transferring state, immediately push and start counting
            _pushByte(dataByte);
            _transferCounter = CyclesPerTransfer;
            break;
    }
}

void SerialController::_presentByte(uint8_t byte) const {
    if (_eventCallback) {
        _eventCallback(SerialOutgoing::PresentByte, byte);
    }
}

void SerialController::_pushByte(uint8_t byte) const {
    if (_eventCallback) {
        _eventCallback(SerialOutgoing::PushByte, byte);
    }
}

void SerialController::_completeTransfer() {
    uint8_t byte = _incomingByteToCommit;
    _hasIncomingByte = false;
    // 1. clear control byte high bit (this will indirectly enter the idle state)
    uint8_t controlByte = _memoryController->readByte(SerialControlRegister);
    _memoryController->setByte(SerialControlRegister, (controlByte & 0x7F));
    // 2. set received byte
    _memoryController->setByte(SerialDataRegister, byte);
    // 3. fire serial interrupt
    _memoryController->requestInterrupt(MemoryController::Serial);
}

void SerialController::_completeInternalTransferIfNecessary() {
    if (_state != SerialState::Transferring) {
        // We've moved on internally, do nothing.
        return;
    }
    
    if (_transferCounter > 0) {
        // Transfer completed early. Wait for the counter
        return;
    }
    
    if (!_hasIncomingByte) {
        // The expected number of cycles have elapsed but the byte isn't here. Wait for it
        return;
    }
    
    _completeTransfer();
}

void SerialController::_completeExternalTransferIfNecessary() {
    if (_state != SerialState::Presenting) {
        // We've moved on internally, do nothing.
        return;
    }
    
    _completeTransfer();
}

void SerialController::updateWithCPUCycles(int cycles) {
    if (_state == SerialState::Transferring) {
        if (_transferCounter > cycles) {
            // transfer ongoing
            _transferCounter -= cycles;
        } else {
            // transfer complete
            _transferCounter = 0;
            _completeInternalTransferIfNecessary();
        }
    }
}

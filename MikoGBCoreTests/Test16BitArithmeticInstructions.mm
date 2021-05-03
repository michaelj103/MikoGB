//
//  Test16BitArithmeticInstructions.mm
//  MikoGB
//
//  Created on 5/1/21.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface Test16BitArithmeticInstructions : XCTestCase

@end

@implementation Test16BitArithmeticInstructions

#pragma mark - ADD HL, ss

- (void)testAddHLWithRegisterPair {
    vector<uint8_t> mem = {
        0x09, // ADD HL, BC
        0x29, // ADD HL, HL
        0x19, // ADD HL, DE
        0x39, // ADD HL, SP
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_H] = 0x8A;
    core.registers[REGISTER_L] = 0x23;
    core.registers[REGISTER_B] = 0x06;
    core.registers[REGISTER_C] = 0x05;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0x9028);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    
    core.registers[REGISTER_H] = 0x8A;
    core.registers[REGISTER_L] = 0x23;
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0x1446);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    
    core.registers[REGISTER_H] = 0x02;
    core.registers[REGISTER_L] = 0xBC;
    core.registers[REGISTER_D] = 0x01;
    core.registers[REGISTER_E] = 0x01;
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0x03BD);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    
    core.registers[REGISTER_H] = 0x83;
    core.registers[REGISTER_L] = 0xBD;
    core.stackPointer = 0xC042;
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0x43FF);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - ADD SP, e

- (void)testAddSPWithImmediate8Signed {
    vector<uint8_t> mem = {
        0xE8, 0x02, // ADD SP, 2
        0xE8, 0xFB, // ADD SP -5
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.stackPointer = 0xFFF8;
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFA);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    
    core.stackPointer = 0x04D2;
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0x04CD);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

#pragma mark - INC ss

- (void)testIncRegisterPair {
    vector<uint8_t> mem = {
        0x03, // INC BC
        0x13, // INC DE
        0x23, // INC HL
        0x33, // INC SP
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_B] = 0xC9;
    core.registers[REGISTER_C] = 0x34;
    core.registers[REGISTER_D] = 0x23;
    core.registers[REGISTER_E] = 0x5F;
    core.registers[REGISTER_H] = 0xFF;
    core.registers[REGISTER_L] = 0xFF;
    core.stackPointer = 0x7610;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getBCptr(), 0xC935);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getDEptr(), 0x2360);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0x0000);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.stackPointer, 0x7611);
}

- (void)testDecRegisterPair {
    vector<uint8_t> mem = {
        0x0B, // DEC BC
        0x1B, // DEC DE
        0x2B, // DEC HL
        0x3B, // DEC SP
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_B] = 0xC9;
    core.registers[REGISTER_C] = 0x34;
    core.registers[REGISTER_D] = 0x23;
    core.registers[REGISTER_E] = 0x5F;
    core.registers[REGISTER_H] = 0x00;
    core.registers[REGISTER_L] = 0x00;
    core.stackPointer = 0x7610;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getBCptr(), 0xC933);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getDEptr(), 0x235E);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0xFFFF);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.stackPointer, 0x760F);
}

@end

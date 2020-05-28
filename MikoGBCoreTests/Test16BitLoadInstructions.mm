//
//  Test16BitLoadInstructions.m
//  MikoGB
//
//  Created on 5/27/20.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface Test16BitLoadInstructions : XCTestCase

@end

@implementation Test16BitLoadInstructions

#pragma mark - LD dd, nn

- (void)testLoadRegisterPairFromImmediate16 {
    vector<uint8_t> mem = {
        0x01, 0x03, 0x57,   //LD BC, $5703
        0x11, 0x71, 0xBE,   //LD DE, $BE71
        0x21, 0x41, 0xC7,   //LD HL, $C741
        0x31, 0x61, 0x8A,   //LD SP, $8A61
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.getBCptr(), 0x5703);
    XCTAssertEqual(core.getDEptr(), 0xBE71);
    XCTAssertEqual(core.getHLptr(), 0xC741);
    XCTAssertEqual(core.stackPointer, 0x8A61);
}

#pragma mark - LD SP, HL

- (void)testLoadStackPtrFromHL {
    vector<uint8_t> mem = {
        0x21, 0x41, 0xC7,   //LD HL, $C741
        0xF9,               //LD SP, HL
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.getHLptr(), 0xC741);
    XCTAssertEqual(core.stackPointer, 0xC741);
}

#pragma mark - PUSH and POP

- (void)testPushAndPop {
    vector<uint8_t> mem = {
        0x31, 0xFE, 0xFF,   //LD SP, $FFFE
        0x01, 0x03, 0x57,   //LD BC, $5703
        0x11, 0x71, 0xBE,   //LD DE, $BE71
        0x21, 0x41, 0xC7,   //LD HL, $C741
        0xC5,               //PUSH BC
        0xD5,               //PUSH DE
        0xC1,               //POP BC
        0xD1,               //POP DE
        0xE5,               //PUSH HL
        0xF5,               //PUSH AF
        0xE1,               //POP HL
        0xF1,               //POP AF
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xCC;
    int cycleCount = 0;
    
    //load the registers
    cycleCount = 0;
    for (size_t i = 0; i < 4; ++i) {
        cycleCount += core.step();
    }
    XCTAssertEqual(cycleCount, 12);
    
    //swap BC and DE via stack
    cycleCount = 0;
    for (size_t i = 0; i < 4; ++i) {
        cycleCount += core.step();
    }
    XCTAssertEqual(cycleCount, 14);
    XCTAssertEqual(core.getBCptr(), 0xBE71);
    XCTAssertEqual(core.getDEptr(), 0x5703);
    
    //swap HL and AF via stack
    cycleCount = 0;
    for (size_t i = 0; i < 4; ++i) {
        cycleCount += core.step();
    }
    XCTAssertEqual(cycleCount, 14);
    XCTAssertEqual(core.getHLptr(), 0xCC00);
    XCTAssertEqual(core.registers[REGISTER_A], 0xC7);
    XCTAssertEqual(core.registers[REGISTER_F], 0x40); //low 4 bits cleared
}

@end

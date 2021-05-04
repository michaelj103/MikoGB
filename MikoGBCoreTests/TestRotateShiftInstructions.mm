//
//  TestRotateShiftInstructions.mm
//  MikoGB
//
//  Created on 5/3/21.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface TestRotateShiftInstructions : XCTestCase

@end

@implementation TestRotateShiftInstructions

#pragma mark - RLCA

- (void)testRotateLeftAccumulatorCarryOut {
    vector<uint8_t> mem = { 0x07, 0x07 }; // RLCA, RLCA
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x85;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x0B);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    
    core.registers[REGISTER_A] = 0xF2;
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0xE5);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - RLA

- (void)testRotateLeftAccumulatorThroughCarry {
    vector<uint8_t> mem = { 0x17, 0x17 }; // RLA, RLA
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x95;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x2B);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    
    core.registers[REGISTER_A] = 0xB5;
    core.setFlag(MikoGB::Carry, false);
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x6A);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - RRCA

- (void)testRotateRightAccumulatorCarryOut {
    vector<uint8_t> mem = { 0x0F, 0x0F }; // RRCA, RRCA
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3B;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x9D);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    
    core.registers[REGISTER_A] = 0xF2;
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x79);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - RRA

- (void)testRotateRightAccumulatorThroughCarry {
    vector<uint8_t> mem = { 0x1F, 0x1F }; // RRA, RRA
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x81;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x40);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    
    core.registers[REGISTER_A] = 0x6A;
    core.setFlag(MikoGB::Carry, true);
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0xB5);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

@end

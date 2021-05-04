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

#pragma mark - RLC m

- (void)testRotateLeftRegisterCarryOut {
    vector<uint8_t> mem = { 0xCB, 0x00 }; // RLC B
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_B] = 0x85;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_B], 0x0B);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

- (void)testRotateLeftPtrHLCarryOut {
    vector<uint8_t> mem = { 0xCB, 0x06 }; // RLC (HL)
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
}

#pragma mark - RL m

- (void)testRotateLeftRegisterThroughCarry {
    vector<uint8_t> mem = {
        0xCB, 0x15, // RL L
        0xCB, 0x12, // RL D
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_L] = 0x80;
    core.registers[REGISTER_D] = 0xAD;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_L], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_D], 0x5B);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

- (void)testRotateLeftPtrHLThroughCarry {
    vector<uint8_t> mem = {
        0xCB, 0x16, // RLC (HL)
        0xCB, 0x16, // RLC (HL)
    };
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x11 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x23);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x46);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

#pragma mark - RRC m

- (void)testRotateRightRegisterCarryOut {
    vector<uint8_t> mem = { 0xCB, 0x09 }; // RRC C
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_C] = 0x01;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_C], 0x80);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

- (void)testRotateRightPtrHLCarryOut {
    vector<uint8_t> mem = { 0xCB, 0x0E }; // RRC (HL)
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
}

#pragma mark - RR m

- (void)testRotateRightRegisterThroughCarry {
    vector<uint8_t> mem = {
        0xCB, 0x1F, // RR A
        0xCB, 0x1B, // RR E
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x01;
    core.registers[REGISTER_E] = 0x21;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_E], 0x90);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

- (void)testRotateRightPtrHLThroughCarry {
    vector<uint8_t> mem = {
        0xCB, 0x1E, // RR (HL)
        0xCB, 0x1E, // RR (HL)
    };
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x8A } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x45);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    
    core.setFlag(MikoGB::Carry, true);
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0xA2);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

#pragma mark - SLA m

- (void)testShiftLeftRegisterFill0 {
    vector<uint8_t> mem = { 0xCB, 0x22 }; // SLA D
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_D] = 0x80;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_D], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
}

- (void)testShiftLeftPtrHLFill0 {
    vector<uint8_t> mem = { 0xCB, 0x26 }; // SLA (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0xFF } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0xFE);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
}

@end

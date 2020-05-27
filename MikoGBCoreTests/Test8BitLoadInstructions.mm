//
//  Test8BitLoadInstructions.m
//  MikoGB
//
//  Created on 5/26/20.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface Test8BitLoadInstructions : XCTestCase

@end

@implementation Test8BitLoadInstructions

#pragma mark - LD r, r' and LD r, (HL)

- (void)_loadFromRegisterHelper:(uint8_t)baseInstruction registerIdx:(int)regIdx {
    vector<uint8_t> mem(8, 0);
    for (size_t i = 0; i < 8; ++i) {
        mem[i] = baseInstruction + i;
    }
    map<uint16_t, uint8_t> otherVals = {
        { 0x3CDE, 0x42 },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x01;
    core.registers[REGISTER_B] = 0x02;
    core.registers[REGISTER_C] = 0x03;
    core.registers[REGISTER_D] = 0x04;
    core.registers[REGISTER_E] = 0x05;
    core.registers[REGISTER_H] = 0x06;
    core.registers[REGISTER_L] = 0x07;
    
    // LD ?, B
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_B ? 0x99 : 0x02;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, C
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_C ? 0x99 : 0x03;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, D
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_D ? 0x99 : 0x04;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, E
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_E ? 0x99 : 0x05;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, H
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_H ? 0x99 : 0x06;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, L
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_L ? 0x99 : 0x07;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD ?, (HL)
    core.registers[REGISTER_H] = 0x3C;
    core.registers[REGISTER_L] = 0xDE;
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[regIdx], 0x42);
    
    // LD ?, A
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_A ? 0x99 : 0x01;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
}

- (void)testLoadRegisterFromRegister {
    [self _loadFromRegisterHelper:0x78 registerIdx:REGISTER_A];
    [self _loadFromRegisterHelper:0x40 registerIdx:REGISTER_B];
    [self _loadFromRegisterHelper:0x48 registerIdx:REGISTER_C];
    [self _loadFromRegisterHelper:0x50 registerIdx:REGISTER_D];
    [self _loadFromRegisterHelper:0x58 registerIdx:REGISTER_E];
    [self _loadFromRegisterHelper:0x60 registerIdx:REGISTER_H];
    [self _loadFromRegisterHelper:0x68 registerIdx:REGISTER_L];
}

@end

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
    
    // LD r, B
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_B ? 0x99 : 0x02;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, C
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_C ? 0x99 : 0x03;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, D
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_D ? 0x99 : 0x04;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, E
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_E ? 0x99 : 0x05;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, H
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_H ? 0x99 : 0x06;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, L
    {
        core.registers[regIdx] = 0x99;
        XCTAssertEqual(core.step(), 1);
        uint8_t targetVal = regIdx == REGISTER_L ? 0x99 : 0x07;
        XCTAssertEqual(core.registers[regIdx], targetVal);
    }
    
    // LD r, (HL)
    core.registers[REGISTER_H] = 0x3C;
    core.registers[REGISTER_L] = 0xDE;
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[regIdx], 0x42);
    
    // LD r, A
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

#pragma mark - LD r, n

- (void)testLoadRegisterFromImmediate8 {
    vector<uint8_t> mem = {
        0x06, 0x11, //LD B, $11
        0x0E, 0x22, //LD C, $22
        0x16, 0x33, //LD D, $33
        0x1E, 0x44, //LD E, $44
        0x26, 0x55, //LD H, $55
        0x2E, 0x66, //LD L, $66
        0x36, 0x77, //LD (HL), $77
        0x3E, 0x88, //LD A, $88
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    for (size_t i = 0; i < 8; ++i) {
        int steps = core.step();
        int targetSteps = i == 6 ? 3 : 2;
        XCTAssertEqual(steps, targetSteps);
    }
    XCTAssertEqual(core.registers[REGISTER_A], 0x88);
    XCTAssertEqual(core.registers[REGISTER_B], 0x11);
    XCTAssertEqual(core.registers[REGISTER_C], 0x22);
    XCTAssertEqual(core.registers[REGISTER_D], 0x33);
    XCTAssertEqual(core.registers[REGISTER_E], 0x44);
    XCTAssertEqual(core.registers[REGISTER_H], 0x55);
    XCTAssertEqual(core.registers[REGISTER_L], 0x66);
    XCTAssertEqual(core.mainMemory[0x5566], 0x77);
}

#pragma mark - LD (HL), r

- (void)testLoadPtrHLFromRegister {
    vector<uint8_t> mem = {
        0x70,   //LD (HL), B
        0x71,   //LD (HL), C
        0x72,   //LD (HL), D
        0x73,   //LD (HL), E
        0x74,   //LD (HL), H
        0x75,   //LD (HL), L
        /*0x76, halt*/
        0x77,   //LD (HL), A
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x11;
    core.registers[REGISTER_B] = 0x22;
    core.registers[REGISTER_C] = 0x33;
    core.registers[REGISTER_D] = 0x44;
    core.registers[REGISTER_E] = 0x55;
    core.registers[REGISTER_H] = 0x66;
    core.registers[REGISTER_L] = 0x77;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x22);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x33);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x44);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x55);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x66);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x77);
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x6677], 0x11);
}

#pragma mark - LD A, (BC) and LD (BC), A

- (void)testLoadAccumulatorFromPtrBC {
    vector<uint8_t> mem = { 0x0A };
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x2F } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_B] = 0xBE;
    core.registers[REGISTER_C] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x2F);
}

- (void)testLoadPtrBCFromAccumulator {
    vector<uint8_t> mem = { 0x02 };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x84;
    core.registers[REGISTER_B] = 0xF5;
    core.registers[REGISTER_C] = 0x01;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0xF501], 0x84);
}

#pragma mark - LD A, (DE) and LD (DE), A

- (void)testLoadAccumulatorFromPtrDE {
    vector<uint8_t> mem = { 0x1A };
    map<uint16_t, uint8_t> otherVals = { { 0xABCD, 0xEF } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_D] = 0xAB;
    core.registers[REGISTER_E] = 0xCD;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0xEF);
}

- (void)testLoadPtrDEFromAccumulator {
    vector<uint8_t> mem = { 0x12 };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xFD;
    core.registers[REGISTER_D] = 0x4F;
    core.registers[REGISTER_E] = 0xA4;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0x4FA4], 0xFD);
}

#pragma mark - LD A, (C) and LD (C), A

- (void)testLoadAccumulatorFromPtrC {
    vector<uint8_t> mem = { 0xF2 };
    map<uint16_t, uint8_t> otherVals = { { 0xFFAA, 0x12 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_C] = 0xAA;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x12);
}

- (void)testLoadPtrCFromAccumulator {
    vector<uint8_t> mem = { 0xE2 };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x33;
    core.registers[REGISTER_C] = 0xBB;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.mainMemory[0xFFBB], 0x33);
}

#pragma mark - LD A, (n) and LD (n), A

- (void)testLoadAccumulatorFromPtrImmediate8 {
    vector<uint8_t> mem = {
        0xF0, 0x76,
    };
    map<uint16_t, uint8_t> otherVals = { { 0xFF76, 0xA1 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.registers[REGISTER_A], 0xA1);
}

- (void)testLoadPtrImmediate8FromAccumulator {
    vector<uint8_t> mem = {
        0xE0, 0x92,
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x5D;
    
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.mainMemory[0xFF92], 0x5D);
}

#pragma mark - LD A, (nn) and LD (nn), A

- (void)testLoadAccumulatorFromPtrImmediate16 {
    vector<uint8_t> mem = {
        0xFA, 0xD8, 0x7B,
    };
    map<uint16_t, uint8_t> otherVals = { { 0x7BD8, 0x91 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.registers[REGISTER_A], 0x91);
}

- (void)testLoadPtrImmediate16FromAccumulator {
    vector<uint8_t> mem = {
        0xEA, 0xF9, 0x5A,
    };
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xD2;
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.mainMemory[0x5AF9], 0xD2);
}

@end

//
//  Test8BitArithmeticInstructions.m
//  MikoGB
//
//  Created on 7/12/20.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface Test8BitArithmeticInstructions : XCTestCase

@end

@implementation Test8BitArithmeticInstructions

#pragma mark - ADD A, r

- (void)testAddAccWithRegister {
    vector<uint8_t> mem = {
        0x3E, 0x3A, //LD A, $3A
        0x06, 0xC6, //LD B, $C6
        0x80,       //ADD A, B
    };
    
    MikoGB::CPUCore core(mem.data(), mem.size());
    for (size_t i = 0; i < 3; ++i) {
        int steps = core.step();
        int targetSteps = i == 2 ? 1 : 2;
        XCTAssertEqual(steps, targetSteps);
    }
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - ADD A, n

- (void)testAddAccWithImmediate8 {
    vector<uint8_t> mem = {
        0x3E, 0x3C, //LD A, $3C
        0xC6, 0xFF, //ADD A, $FF
    };
    
    MikoGB::CPUCore core(mem.data(), mem.size());
    for (size_t i = 0; i < 2; ++i) {
        int steps = core.step();
        XCTAssertEqual(steps, 2);
    }
    XCTAssertEqual(core.registers[REGISTER_A], 0x3B);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - ADD A, (HL)

- (void)testAddAccWithPtrHL {
    vector<uint8_t> mem = { 0x86 }; //ADD A, (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x12 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x3C;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x4E);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - ADC A, r

- (void)testAddAccWithRegisterAndCarry {
    vector<uint8_t> mem = { 0x8B }; //ADC A, E
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xE1;
    core.registers[REGISTER_E] = 0x0F;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0xF1);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - ADC A, n

- (void)testAddAccWithImmediate8AndCarry {
    vector<uint8_t> mem = { 0xCE, 0x3B }; //ADC A, $3B
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xE1;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x1D);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - ADC A, (HL)

- (void)testAddAccWithPtrHLAndCarry {
    vector<uint8_t> mem = { 0x8E }; //ADC A, (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x1E } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0xE1;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - SUB A, r

- (void)testSubAccWithRegister {
    vector<uint8_t> mem = { 0x93 }; //SUB A, E
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3E;
    core.registers[REGISTER_E] = 0x3E;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - SUB A, n

- (void)testSubAccWithImmediate8 {
    vector<uint8_t> mem = { 0xD6, 0x0F }; //SUB A, $0F
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3E;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x2F);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - SUB A, (HL)

- (void)testSubAccWithPtrHL {
    vector<uint8_t> mem = { 0x96 }; //SUB A, (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x40 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x3E;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0xFE);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - SBC A, r

- (void)testSubAccWithRegisterAndCarry {
    vector<uint8_t> mem = { 0x9C }; //SBC A, H
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3B;
    core.registers[REGISTER_H] = 0x2A;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x10);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - SBC A, n

- (void)testSubAccWithImmediate8AndCarry {
    vector<uint8_t> mem = { 0xDE, 0x3A }; //SBC A, $0F
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3B;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - SBC A, (HL)

- (void)testSubAccWithPtrHLAndCarry {
    vector<uint8_t> mem = { 0x9E }; //SBC A, (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x4F } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x3B;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    core.setFlag(MikoGB::Carry, true);
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0xEB);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - AND r

- (void)testAndAccWithRegister {
    vector<uint8_t> mem = { 0xA5 }; //AND L
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x5A;
    core.registers[REGISTER_L] = 0x3F;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x1A);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - AND n

- (void)testAndAccWithImmediate8 {
    vector<uint8_t> mem = { 0xE6, 0x38 }; //AND $38
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x5A;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x18);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - AND (HL)

- (void)testAndAccWithPtrHL {
    vector<uint8_t> mem = { 0xA6 }; //AND (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x00 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x5A;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - OR r

- (void)testOrAccWithRegister {
    vector<uint8_t> mem = { 0xB7 }; //OR A
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x5A;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x5A);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - OR n

- (void)testOrAccWithImmediate8 {
    vector<uint8_t> mem = { 0xF6, 0x03 }; //OR $03
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x5A;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x5B);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - OR (HL)

- (void)testOrAccWithPtrHL {
    vector<uint8_t> mem = { 0xB6 }; //OR (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x0F } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x5A;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x5F);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - XOR r

- (void)testXorAccWithRegister {
    vector<uint8_t> mem = { 0xAF }; //XOR A
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xFF;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - XOR n

- (void)testXorAccWithImmediate8 {
    vector<uint8_t> mem = { 0xEE, 0x0F }; //XOR $0F
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xFF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0xF0);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - XOR (HL)

- (void)testXorAccWithPtrHL {
    vector<uint8_t> mem = { 0xAE }; //XOR (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x8A } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0xFF;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x75);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - CP r

- (void)testCpAccWithRegister {
    vector<uint8_t> mem = { 0xB8 }; //CP B
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3C;
    core.registers[REGISTER_B] = 0x2F;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x3C);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - CP n

- (void)testCpAccWithImmediate8 {
    vector<uint8_t> mem = { 0xFE, 0x3C }; //CP $3C
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0x3C;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x3C);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), false);
}

#pragma mark - CP (HL)

- (void)testCpAccWithPtrHL {
    vector<uint8_t> mem = { 0xBE }; //CP (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x40 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_A] = 0x3C;
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.registers[REGISTER_A], 0x3C);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
    XCTAssertEqual(core.getFlag(MikoGB::Carry), true);
}

#pragma mark - INC r

- (void)testIncRegister {
    vector<uint8_t> mem = { 0x3C }; //INC A
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_A] = 0xFF;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_A], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - INC (HL)

- (void)testIncPtrHL {
    vector<uint8_t> mem = { 0x34 }; //INC (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x50 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0x51);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), false);
}

#pragma mark - DEC r

- (void)testDecRegister {
    vector<uint8_t> mem = { 0x2D }; //DEC L
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.registers[REGISTER_L] = 0x01;
    
    XCTAssertEqual(core.step(), 1);
    XCTAssertEqual(core.registers[REGISTER_L], 0x00);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), true);
    XCTAssertEqual(core.getFlag(MikoGB::H), false);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
}

#pragma mark - DEC (HL)

- (void)testDecPtrHL {
    vector<uint8_t> mem = { 0x35 }; //DEC (HL)
    map<uint16_t, uint8_t> otherVals = { { 0xBEEF, 0x00 } };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.registers[REGISTER_H] = 0xBE;
    core.registers[REGISTER_L] = 0xEF;
    
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.mainMemory[0xBEEF], 0xFF);
    XCTAssertEqual(core.getFlag(MikoGB::Zero), false);
    XCTAssertEqual(core.getFlag(MikoGB::H), true);
    XCTAssertEqual(core.getFlag(MikoGB::N), true);
}

@end

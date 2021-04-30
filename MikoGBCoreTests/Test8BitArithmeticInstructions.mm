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
    // Just some basics of identifying the correct register and storing
    
    vector<uint8_t> mem = {
        0x3E, 0x3A, //LD A, $3A
        0x06, 0xC6, //LD B, $C6
        0x80,       //Add A, B
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
    
}

@end

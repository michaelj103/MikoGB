//
//  TestCPUCoreBasics.m
//  MikoGB
//
//  Created on 5/26/20.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface TestCPUCoreBasics : XCTestCase

@end

@implementation TestCPUCoreBasics

- (void)testBasicStepping {
    vector<uint8_t> mem = {
        0x31, 0xFE, 0xFF,   // LD SP, $FFFE
        0x3E, 0xBD,         // LD A, $0xBD
    };
    
    MikoGB::CPUCore core(mem.data(), mem.size());
    core.step();
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 3);
    core.step();
    XCTAssertEqual(core.registers[REGISTER_A], 0xBD);
    XCTAssertEqual(core.programCounter, 5);
}

- (void)testNoOp {
    MikoGB::CPUCore core(NULL, 0);
    core.step();
    XCTAssertEqual(core.programCounter, 1);
}

@end
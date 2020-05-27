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

- (void)setUp {
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testBasicStepping {
    vector<uint8_t> mem = {
        0x31, 0xFE, 0xFF,   // LD SP, $FFFE
        0x3E, 0xBD,         // LD A, $0xBD
    };
    
    unique_ptr<uint8_t[]> allocatedMemory = createGBMemory(mem);
    MikoGB::CPUCore core(allocatedMemory.get(), mem.size());
    core.step();
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 3);
    core.step();
    XCTAssertEqual(core.registers[REGISTER_A], 0xBD);
    XCTAssertEqual(core.programCounter, 5);
}

@end

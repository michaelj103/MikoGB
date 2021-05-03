//
//  TestCallAndReturnInstructions.mm
//  MikoGB
//
//  Created on 5/3/21.
//

#import <XCTest/XCTest.h>
#include "CPUCore.hpp"
#include "TestCPUCoreUtilities.hpp"
#include <vector>

using namespace std;

@interface TestCallAndReturnInstructions : XCTestCase

@end

@implementation TestCallAndReturnInstructions

#pragma mark - CALL

- (void)testCallImmediate16 {
    vector<uint8_t> mem = { 0x00 };
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x8000, {
            0xCD, 0x34, 0x12, // CALL 0x1234
        }
        },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.stackPointer = 0xFFFE;
    core.programCounter = 0x8000;

    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x80);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x03);
    XCTAssertEqual(core.programCounter, 0x1234);
}

#pragma mark - CALL cc

- (void)testCallConditionalImmediate16 {
    vector<uint8_t> mem = { 0x00 };
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x1234, {
            0xDC, 0xEF, 0xBE, // CALL C  0xBEEF
            0xD4, 0xEF, 0xBE, // CALL NC 0xBEEF
        }
        },
        { 0x7FFD, {
            0xC4, 0x34, 0x12, // CALL NZ 0x1234
            0xCC, 0x34, 0x12, // CALL Z  0x1234
        }
        },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.setFlag(MikoGB::Zero, true);
    core.stackPointer = 0xFFFE;
    core.programCounter = 0x7FFD;

    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x8000);
    
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x80);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x03);
    XCTAssertEqual(core.programCounter, 0x1234);
    
    XCTAssertEqual(core.step(), 3);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.programCounter, 0x1237);
    
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFA);
    XCTAssertEqual(core.mainMemory[0xFFFB], 0x12);
    XCTAssertEqual(core.mainMemory[0xFFFA], 0x3A);
    XCTAssertEqual(core.programCounter, 0xBEEF);
}

#pragma mark - RET

- (void)testReturnSubroutine {
    vector<uint8_t> mem = { 0x00 };
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x4444, {
            0xCD, 0x00, 0x90, // CALL 0x9000
        }
        },
        { 0x9000, {
            0xC9, // RET
        }
        },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.stackPointer = 0xFFFE;
    core.programCounter = 0x4444;
    
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x44);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x47);
    XCTAssertEqual(core.programCounter, 0x9000);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x4447);
}

#pragma mark - RETI

- (void)testReturnInterrupt {
    vector<uint8_t> mem = { 0x00 };
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x4444, {
            0xCD, 0x00, 0x90, // CALL 0x9000
        }
        },
        { 0x9000, {
            0xD9, // RETI
        }
        },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.stackPointer = 0xFFFE;
    core.programCounter = 0x4444;
    core.interruptsEnabled = false;
    
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x44);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x47);
    XCTAssertEqual(core.programCounter, 0x9000);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x4447);
    XCTAssertEqual(core.interruptsEnabled, true);
}

#pragma mark - RET cc

- (void)testReturnSubroutineConditional {
    vector<uint8_t> mem = {
        0xCD, 0x00, 0x10, // CALL 0x1000
        0xCD, 0x00, 0x20, // CALL 0x2000
    };
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x1000, {
            0xC8, // RET Z
            0xC0, // RET NZ
        }
        },
        { 0x2000, {
            0xD0, // RET NC
            0xD8, // RET C
        }
        },
    };
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.setFlag(MikoGB::Carry, true);
    core.stackPointer = 0xFFFE;

    // CALL 0x1000
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x00);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x03);
    XCTAssertEqual(core.programCounter, 0x1000);
    
    // RET Z fails
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.programCounter, 0x1001);
    
    // RET NZ succeeds
    XCTAssertEqual(core.step(), 5);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x0003);
    
    // CALL 0x2000
    XCTAssertEqual(core.step(), 6);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x00);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x06);
    XCTAssertEqual(core.programCounter, 0x2000);
    
    // RET NC fails
    XCTAssertEqual(core.step(), 2);
    XCTAssertEqual(core.programCounter, 0x2001);
    
    // RET C succeeds
    XCTAssertEqual(core.step(), 5);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x0006);
}

#pragma mark - RST

- (void)testResetCall {
    vector<uint8_t> mem = { 0xC9 }; // RET at 0
    map<uint16_t, vector<uint8_t>> otherVals = {
        { 0x1111, {
            0xC7, // RST 0
            0xFF, // RST 7
        }
        },
    };
    
    vector<uint8_t> allocatedMemory = createGBMemory(mem, otherVals);
    MikoGB::CPUCore core(allocatedMemory.data(), allocatedMemory.size());
    core.stackPointer = 0xFFFE;
    core.programCounter = 0x1111;
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x11);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x12);
    XCTAssertEqual(core.programCounter, 0x0000);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFE);
    XCTAssertEqual(core.programCounter, 0x1112);
    
    XCTAssertEqual(core.step(), 4);
    XCTAssertEqual(core.stackPointer, 0xFFFC);
    XCTAssertEqual(core.mainMemory[0xFFFD], 0x11);
    XCTAssertEqual(core.mainMemory[0xFFFC], 0x13);
    XCTAssertEqual(core.programCounter, 0x0038);
}

@end

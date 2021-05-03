//
//  CallAndReturnInstructions.hpp
//  MikoGB
//
//  Created on 5/2/21.
//

#ifndef CallAndReturnInstructions_hpp
#define CallAndReturnInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// ============================
// Call and Return Instructions
// ============================

/// CALL nn
/// Calls the subroutine at address specified by immediate 16 bits. Bits are [ 1, 1, 0, 0, 1, 1, 0, 1 ]
/// No impact on flags. Pushes the PC onto the stack and sets it to the given address. Low 8-bits are first
int callImmediate16(const uint8_t *, MikoGB::CPUCore &);

/// CALL cc nn
/// Calls the subroutine at address specified by immediate 16 bits if condition cc is met. Bits are [ 1, 1, 0, c1, c0, 1, 0, 0 ]
/// Condition codes are same as jump, NZ=0, Z=1, NC=2, C=3. Same behavior as CALL if condition met
int callConditionalImmediate16(const uint8_t *, MikoGB::CPUCore &);

/// RET
/// Return from the current subroutine. Bits are [ 1, 1, 0, 0, 1, 0, 0, 1 ]
/// Pops the PC value from the stack
int returnSubroutine(const uint8_t *, MikoGB::CPUCore &);

/// RETI
/// Return from the current subroutine. Bits are [ 1, 1, 0, 1, 1, 0, 0, 1 ]
/// Pops the PC value from the stack and re-enables interrupts between instructions
int returnInterrupt(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* CallAndReturnInstructions_hpp */

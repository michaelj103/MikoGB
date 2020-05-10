//
//  BitOpInstructions.hpp
//  MikoGB
//
//  Created on 5/10/20.
//

#ifndef BitOpInstructions_hpp
#define BitOpInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// =========================================
// Bit Operations
// =========================================

/// BIT b, r     (Extended opcode 0xCB)
/// Sets Z flag based on the specified bit of register r. Bits are [ 0, 1, b2, b1, b0, r2, r1, r0 ]
/// Able to specify 7 registers A(111), B(000), C(001), D(010), E(011), H(100), L(101).
/// 8th code (110) operates on byte at address (HL), see instruction below
int bitReadFromRegister(const uint8_t *, MikoGB::CPUCore &);

/// BIT b, (HL)     (Extended opcode 0xCB)
/// Sets Z flag based on the specified bit of byte at (HL). Bits are [ 0, 1, b2, b1, b0, 1, 1, 0 ]
int bitReadFromPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// SET b, r     (Extended opcode 0xCB)
/// Sets the specified bit of register r. Bits are [ 1, 1, b2, b1, b0, r2, r1, r0 ]
/// Able to specify 7 registers A(111), B(000), C(001), D(010), E(011), H(100), L(101).
/// 8th code (110) operates on byte at address (HL), see instruction below
int bitSetRegister(const uint8_t *, MikoGB::CPUCore &);

/// SET b, (HL)     (Extended opcode 0xCB)
/// Sets the specified bit of byte at (HL). Bits are [ 1, 1, b2, b1, b0, 1, 1, 0 ]
int bitSetPtrHL(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* BitOpInstructions_hpp */

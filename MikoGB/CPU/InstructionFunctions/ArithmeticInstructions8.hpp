//
//  ArithmeticInstructions8.hpp
//  MikoGB
//
//  Created on 5/9/20.
//

#ifndef ArithmeticInstructions8_hpp
#define ArithmeticInstructions8_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// =========================================
// 8-bit Arithmetic and Logical Instructions
// =========================================

/// XOR r
/// A <- A ^ r for standard register codes. Bits are [ 1, 0, 1, 0, 1, r2, r1, r0 ]
/// Able to specify 7 registers A(111), B(000), C(001), D(010), E(011), H(100), L(101).
/// 8th code (110) is xor with byte at address (HL), see instruction below
int xorAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// XOR (HL)
/// A <- A ^ (HL). XOR with the byte at address (HL). Bits are [ 1, 0, 1, 0, 1, 1, 1, 0 ]
/// "8th" register code of XOR r
int xorAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// XOR n
/// A <- A ^ n. XOR A with immediate byte. Bits are [ 1, 1, 1, 0, 1, 1, 1, 0 ]
int xorAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* ArithmeticInstructions8_hpp */

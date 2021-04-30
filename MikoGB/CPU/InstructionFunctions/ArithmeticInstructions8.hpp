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

/// ADD A, r
/// A <- A + r for standard register codes. Bits are [ 1, 0, 0, 0, 0, r2, r1, r0 ]
/// Special register code 110 indicates adding contents pointed to by HL, see below
/// Z flag set if 0, H flag set if carry from bit 3, N flag reset, carry flag set if carry from bit 7
int addAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// ADD A, (HL)
/// A <- A + (HL). Adds byte pointed to by HL into A. Bits are [ 1, 0, 0, 0, 0, 1, 1, 0 ]
/// Special case of addAccWithRegister()
int addAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// ADD A, n
/// A <- A + n for immediate byte n. Bits are [ 1, 1, 0, 0, 0, 1, 1, 0 ]
/// Z flag set if 0, H flag set if carry from bit 3, N flag reset, carry flag set if carry from bit 7
int addAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// ADC A, r
/// A <- A + r + CY for standard register codes and carry flag. Bits are [ 1, 0, 0, 0, 1, r2, r1, r0 ]
/// Special register code 110 indicates adding contents pointed to by HL, see below
/// Z flag set if 0, H flag set if carry from bit 3, N flag reset, carry flag set if carry from bit 7
int addAccWithRegisterAndCarry(const uint8_t *, MikoGB::CPUCore &);

/// ADC A, (HL)
/// A <- A + (HL) + CY for byte at (HL) and carry flag. Bits are [ 1, 0, 0, 0, 1, 1, 1, 0 ]
/// Special case of addAccWithRegisterAndCarry()
int addAccWithPtrHLAndCarry(const uint8_t *, MikoGB::CPUCore &);

/// ADC A, n
/// A <- A + n + CY for immediate byte n. Bits are [ 1, 1, 0, 0, 1, 1, 1, 0 ]
/// Z flag set if 0, H flag set if carry from bit 3, N flag reset, carry flag set if carry from bit 7
int addAccWithImmediate8AndCarry(const uint8_t *, MikoGB::CPUCore &);

/// SUB A, r
/// A <- A - r for standard register codes. Bits are [ 1, 0, 0, 1, 0, r2, r1, r0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// SUB A, n
/// A <- A - n for immediate byte n. Bits are [ 1, 1, 0, 1, 0, 1, 1, 0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// SUB A, (HL)
/// A <- A - (HL). Subtracts byte pointed to by HL from A. Bits are [ 1, 0, 0, 1, 0, 1, 1, 0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// SBC A, r
/// A <- A - r - CY for standard register codes. Bits are [ 1, 0, 0, 1, 1, r2, r1, r0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithRegisterAndCarry(const uint8_t *, MikoGB::CPUCore &);

/// SBC A, n
/// A <- A - n - CY for immediate byte n. Bits are [ 1, 1, 0, 1, 1, 1, 1, 0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithImmediate8AndCarry(const uint8_t *, MikoGB::CPUCore &);

/// SBC A, (HL)
/// A <- A - (HL) - CY. Subtracts byte pointed to by HL from A. Bits are [ 1, 0, 0, 1, 1, 1, 1, 0 ]
/// Z flag set if 0, H flag set if borrow from bit 4, N is always set, carry set if borrow from (imaginary) bit 8
int subAccWithPtrHLAndCarry(const uint8_t *, MikoGB::CPUCore &);

/// AND r
/// A <- A & r for standard register codes. Bits are [ 1, 0, 1, 0, 0, r2, r1, r0 ]
/// Z flag set if 0, H flag always set, C flag always reset, N flag always reset
int andAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// AND n
/// A <- A & n for immediate byte n. Bits are [ 1, 1, 1, 0, 0, 1, 1, 0 ]
/// Z flag set if 0, H flag always set, C flag always reset, N flag always reset
int andAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// AND (HL)
/// A <- A & (HL) for byte pointed to by HL. Bits are [ 1, 0, 1, 0, 0, 1, 1, 0 ]
/// Z flag set if 0, H flag always set, C flag always reset, N flag always reset
int andAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// OR r
/// A <- A | r for standard register codes. Bits are [ 1, 0, 1, 1, 0, r2, r1, r0 ]
/// Z flag set if 0, Other flags always reset
int orAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// OR n
/// A <- A | n for immediate byte n. Bits are [ 1, 1, 1, 1, 0, 1, 1, 0 ]
/// Z flag set if 0, Other flags always reset
int orAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// OR (HL)
/// A <- A | (HL) for byte pointed to by HL. Bits are [ 1, 0, 1, 1, 0, 1, 1, 0 ]
/// Z flag set if 0, Other flags always reset
int orAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// XOR r
/// A <- A ^ r for standard register codes. Bits are [ 1, 0, 1, 0, 1, r2, r1, r0 ]
/// Able to specify 7 registers A(111), B(000), C(001), D(010), E(011), H(100), L(101).
/// 8th code (110) is xor with byte at address (HL), see instruction below
int xorAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// XOR n
/// A <- A ^ n. XOR A with immediate byte. Bits are [ 1, 1, 1, 0, 1, 1, 1, 0 ]
int xorAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// XOR (HL)
/// A <- A ^ (HL). XOR with the byte at address (HL). Bits are [ 1, 0, 1, 0, 1, 1, 1, 0 ]
/// "8th" register code of XOR r
int xorAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

/// CP r
/// Compares A and r for standard register codes. Bits are [ 1, 0, 1, 1, 1, r2, r1, r0 ]
/// Z <- 1 if equal. H <- 1 if A > r. CY <- 1 if A < r
int cpAccWithRegister(const uint8_t *, MikoGB::CPUCore &);

/// CP n
/// Compares A and n for immediate byte n. Bits are [ 1, 1, 1, 1, 1, 1, 1, 0 ]
/// Z <- 1 if equal. H <- 1 if A > n. CY <- 1 if A < n
int cpAccWithImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// CP (HL)
/// Compares A and (HL) for byte pointed to by HL. Bits are [ 1, 0, 1, 1, 1, 1, 1, 0 ]
/// Z <- 1 if equal. H <- 1 if A > (HL). CY <- 1 if A < (HL)
int cpAccWithPtrHL(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* ArithmeticInstructions8_hpp */

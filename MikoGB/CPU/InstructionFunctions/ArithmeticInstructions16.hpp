//
//  ArithmeticInstructions16.hpp
//  MikoGB
//
//  Created on 5/1/21.
//

#ifndef ArithmeticInstructions16_hpp
#define ArithmeticInstructions16_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// ==========================================
// 16-bit Arithmetic and Logical Instructions
// ==========================================

/// ADD HL, ss
/// HL <- HL + ss for register pair codes. Bits are [ 0, 0, s1, s0, 1, 0, 0, 1 ]
/// N flag reset, Z flag untouched, H set if carry out from bit 11, CY set if carry out from bit 15
int addHLWithRegisterPair(const uint8_t *, MikoGB::CPUCore &);

/// ADD SP, e
/// SP <- SP + e for immediate signed byte e. Bits are [ 1, 1, 1, 0, 1, 0, 0, 0 ]
/// N flag reset, Z flag reset, H and CY set for carry out of bits 11 and 15
int addSPWithImmediate8Signed(const uint8_t *, MikoGB::CPUCore &);

/// INC ss
/// ss <- ss + 1 for register pair ss. Bits are [ 0, 0, s1, s0, 0, 0, 1, 1 ]
/// No flags affected
int incRegisterPair(const uint8_t *, MikoGB::CPUCore &);

/// DEC ss
/// ss <- ss - 1 for register pair ss. Bits are [ 0, 0, s1, s0, 1, 0, 1, 1 ]
/// No flags affected
int decRegisterPair(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* ArithmeticInstructions16_hpp */

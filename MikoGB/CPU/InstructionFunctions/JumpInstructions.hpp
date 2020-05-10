//
//  JumpInstructions.hpp
//  MikoGB
//
//  Created on 5/10/20.
//

#ifndef JumpInstructions_hpp
#define JumpInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// =========================================
// Jump Instructions
// =========================================

/// JP nn
/// Jumps unconditionally to absolute address specified by immediate 16-bits. Bits are [ 1, 1, 0, 0, 0, 0, 1, 1 ]
int jumpUnconditionalAbsolute16(const uint8_t *, MikoGB::CPUCore &);

/// JP cc, nn
/// Jumps conditionally to absolute address specified by immediate 16-bits. Bits are [ 1, 1, 0, c1, c0, 0, 1, 0 ]
/// 2-bit condition codes are NZ(00), Z(01), NC(10), C(11)
int jumpConditionalAbsolute16(const uint8_t *, MikoGB::CPUCore &);

/// JR e
/// Jumps unconditionally to relative address specified by immediate 8-bits. Bits are [ 0, 0, 0, 1, 1, 0, 0, 0 ]
int jumpUnconditionalRelative8(const uint8_t *, MikoGB::CPUCore &);

/// JR cc, e
/// Jumps conditionally to relative address specified by immediate 8-bits. Bits are [ 0, 0, 1, c1, c0, 0, 0, 0 ]
/// 2-bit condition codes are NZ(00), Z(01), NC(10), C(11)
int jumpConditionalRelative8(const uint8_t *, MikoGB::CPUCore &);

/// JP (HL)
/// Jumps unconditionally to the instruction at (HL). Bits are [ 1, 1, 1, 0, 1, 0, 0, 1 ]
/// Effectively a copy of HL -> PC
int jumpUnconditionalHL(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* JumpInstructions_hpp */

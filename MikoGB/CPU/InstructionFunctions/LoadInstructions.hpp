//
//  LoadInstructions.hpp
//  MikoGB
//
//  Created on 5/5/20.
//

#ifndef LoadInstructions_hpp
#define LoadInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// ========================
// 8-bit transfer and I/O instructions
// ========================

/// LD r, r'
/// 8-bit load register r' -> r. Bits are [ 0, 1, r2, r1, r0, r'2, r'1, r'0 ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// Matches 42 of 256 opcodes! 49 if you include no-op versions (should we?)
int loadRegisterFromRegister(uint8_t *, MikoGB::CPUCore &);

/// LD r, (HL)
/// 8-bit load register from memory (HL) -> r. Bits are [ 0, 1, r2, r1, r0, 1, 1, 0 ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// Loads byte pointed to by HL into the specified register
int loadRegisterFromMemory(uint8_t *, MikoGB::CPUCore &);

/// LD r, n
/// 8-bit load register immediate -> r. Bits are [ 0, 0, r2, r1, r0, 1, 1, 0 ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// 2-bytes, loads indicated register with immediate value of the second byte
int loadRegisterImmediate8(uint8_t *, MikoGB::CPUCore &);

/// LD (HL), r
/// 8-bit load memory from register r -> (HL). Bits are [ 0, 1, 1, 1, 0, r2, r1, r0  ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// Loads the value of the specified register into the byte pointed to by HL
int loadMemoryFromRegister(uint8_t *, MikoGB::CPUCore &);

/// LD (HL), n
/// 8-bit load memory immediate -> (HL). Bits are [ 0, 0, 1, 1, 0, 1, 1, 0 ]
/// 2-bytes, loads the memory pointed to by HL with the immediate value of the second byte
int loadMemoryImmediate8(uint8_t *, MikoGB::CPUCore &);

}

#endif /* LoadInstructions_hpp */

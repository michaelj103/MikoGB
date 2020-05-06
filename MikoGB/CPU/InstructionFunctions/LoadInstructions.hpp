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
int loadRegisterFromRegister(const uint8_t *, MikoGB::CPUCore &);

/// LD r, (HL)
/// 8-bit load register from memory (HL) -> r. Bits are [ 0, 1, r2, r1, r0, 1, 1, 0 ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// Loads byte pointed to by HL into the specified register
int loadRegisterFromMemory(const uint8_t *, MikoGB::CPUCore &);

/// LD r, n
/// 8-bit load register immediate -> r. Bits are [ 0, 0, r2, r1, r0, 1, 1, 0 ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// 2-bytes, loads indicated register with immediate value of the second byte
int loadRegisterImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// LD (HL), r
/// 8-bit load memory from register r -> (HL). Bits are [ 0, 1, 1, 1, 0, r2, r1, r0  ]
/// Able to specify 8 registers, but only 7 are valid, A(111), B(000), C(001), D(010), E(011), H(100), L(101)
/// Loads the value of the specified register into the byte pointed to by HL
int loadMemoryFromRegister(const uint8_t *, MikoGB::CPUCore &);

/// LD (HL), n
/// 8-bit load memory immediate -> (HL). Bits are [ 0, 0, 1, 1, 0, 1, 1, 0 ]
/// 2-bytes, loads the memory pointed to by HL with the immediate value of the second byte
int loadMemoryImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (BC)
/// 8-bit load (BC) -> A. Bits are [ 0, 0, 0, 0, 1, 0, 1, 0 ]
/// Loads the value pointed to by the register pair BC into the accumulator
int loadAccumulatorFromBCptr(const uint8_t *, MikoGB::CPUCore &);

/// LD (BC), A
/// 8-bit load A -> (BC). Bits are [ 0, 0, 0, 0, 0, 0, 1, 0 ]
/// Loads the value of the accumulator into the memory pointed to by BC
int loadBCptrFromAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (DE)
/// 8-bit load (DE) -> A. Bits are [ 0, 0, 0, 1, 1, 0, 1, 0 ]
/// Loads the value pointed to by the register pair DE into the accumulator
int loadAccumulatorFromDEptr(const uint8_t *, MikoGB::CPUCore &);

/// LD (DE), A
/// 8-bit load A -> (DE). Bits are [ 0, 0, 0, 1, 0, 0, 1, 0 ]
/// Loads the value of the accumulator into the memory pointed to by DE
int loadDEptrFromAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (C)
/// 8-bit load (0xFF00 + C) -> A. Bits are [ 1, 1, 1, 1, 0, 0, 1, 0 ]
/// Loads the value at address 0xFF00 + C into the accumulator
int loadAccumulatorFromC(const uint8_t *, MikoGB::CPUCore &);

/// LD (C), A
/// 8-bit load A -> (0xFF00 + C). Bits are [ 1, 1, 1, 0, 0, 0, 1, 0 ]
/// Loads the value of the accumulator into address 0xFF00 + C
int loadCFromAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// LD (n), A
/// 8-bit load A -> (0xFF00 + n). Bits are [ 1, 1, 1, 0, 0, 0, 0, 0 ]
/// Loads the value of the accumulator into address 0xFF00 + n, for immediate byte n
int loadPtrImmediate8FromAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// LD (nn), A
/// 8-bit load A -> (nn). Bits are [ 1, 1, 1, 0, 1, 0, 1, 0 ]
/// Loads the value of the accumulator into address (nn)), for 2 immediate bytes nn
int loadPtrImmediate16FromAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (n)
/// 8-bit load (0xFF00 + n) -> A. Bits are [ 1, 1, 1, 1, 0, 0, 0, 0 ]
/// Loads the value at address 0xFF00 + n into the accumulator for immediate byte n
int loadAccumulatorFromPtrImmediate8(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (n)
/// 8-bit load (nn) -> A. Bits are [ 1, 1, 1, 1, 1, 0, 1, 0 ]
/// Loads the value at address (nn) into the accumulator for 2 immediate bytes nn
int loadAccumulatorFromPtrImmediate16(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (HLI)
/// 8-bit load (HL) -> A and increment of HL. Bits are [ 0, 0, 1, 0, 1, 0, 1, 0 ]
/// Loads the value at address (HL) into the accumulator. Then increments HL
int loadAccumulatorFromHLPtrIncrement(const uint8_t *, MikoGB::CPUCore &);

/// LD A, (HLD)
/// 8-bit load (HL) -> A and decrement of HL. Bits are [ 0, 0, 1, 1, 1, 0, 1, 0 ]
/// Loads the value at address (HL) into the accumulator. Then decrements HL
int loadAccumulatorFromHLPtrDecrement(const uint8_t *, MikoGB::CPUCore &);

/// LD (HLI), A
/// 8-bit load A -> (HL) and increment of HL. Bits are [ 0, 0, 1, 0, 0, 0, 1, 0 ]
/// Loads the value of the accumulator into address (HL). Then increments HL
int loadHLPtrFromAccumulatorIncrement(const uint8_t *, MikoGB::CPUCore &);

/// LD (HLD), A
/// 8-bit load A -> (HL) and decrement of HL. Bits are [ 0, 0, 1, 1, 0, 0, 1, 0 ]
/// Loads the value of the accumulator into address (HL). Then decrements HL
int loadHLPtrFromAccumulatorDecrement(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* LoadInstructions_hpp */

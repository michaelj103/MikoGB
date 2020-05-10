//
//  LoadInstructions16.hpp
//  MikoGB
//
//  Created on 5/6/20.
//

#ifndef LoadInstructions16_hpp
#define LoadInstructions16_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

// ====================================
// 16-bit transfer and I/O instructions
// ====================================

/// LD dd, nn
/// 16-bit load immediate data -> dd. Bits are [ 0, 0, d1, d0, 0, 0, 0, 1 ]
/// Able to specify 4 register pairs, BC(00), DE(01), HL(10), SP(11)
int loadRegisterPairFromImmediate16(const uint8_t *, MikoGB::CPUCore &);

/// LD SP, HL
/// 16-bit load HL -> SP. Bits are [ 1, 1, 1, 1, 1, 0, 0, 1 ]
/// Copies value, no memory access
int loadStackPtrFromHL(const uint8_t *, MikoGB::CPUCore &);

/// PUSH qq
/// 16-bit push of the indicated register pair onto the stack. Bits are [ 1, 1, q, q, 0, 1, 0, 1 ]
/// Able to specify 4 register pairs BC(00), DE(01), HL(10), and AF(11)
int pushQQ(const uint8_t *, MikoGB::CPUCore &);

/// POP qq
/// 16-bit pop from the stack into the indicated register pair. Bits are [ 1, 1, q, q, 0, 0, 0, 1 ]
/// Able to specify 4 register pairs BC(00), DE(01), HL(10), and AF(11)
int popQQ(const uint8_t *, MikoGB::CPUCore &);

/// LDHL SP, e
/// 8-bit immediate operand e is added to SP and stored in HL. Bits are [ 1, 1, 1, 1, 1, 0, 0, 0 ]
/// e is treated as a signed 8-bit integer from -128 to +127
int ldhl(const uint8_t *, MikoGB::CPUCore &);

/// LD (nn), SP
/// 16-bit store of the stack pointer to the address nn and nn+1. Bits are [ 0, 0, 0, 0, 1, 0, 0, 0 ]
/// SP is stored with the lo byte at (nn) and the hi byte at (nn+1)
int loadPtrImmediate16FromSP(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* LoadInstructions16_hpp */

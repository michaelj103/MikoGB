//
//  SpecialInstructions.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef SpecialInstructions_hpp
#define SpecialInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

/// DAA
int decimalAdjustAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// CPL
/// Takes the complement of the accumulator. Bits are [ 0, 0, 1, 0, 1, 1, 1, 1 ]
int complementAccumulator(const uint8_t *, MikoGB::CPUCore &);

/// CCF
/// Complement Carry Flag. Bits are [ 0, 0, 1, 1, 1, 1, 1, 1 ]
int complementCarryFlag(const uint8_t *, MikoGB::CPUCore &);

/// SCF
/// Set Carry Flag. Bits are [ 0, 0, 1, 1, 0, 1, 1, 1 ]
int setCarryFlag(const uint8_t *, MikoGB::CPUCore &);

/// DI
/// Disable interrupts. Bits are [ 1, 1, 1, 1, 0, 0, 1, 1 ]
int disableInterrupts(const uint8_t *, MikoGB::CPUCore &);

/// EI
/// Enables interrupts. Bits are [ 1, 1, 1, 1, 1, 0, 1, 1 ]
int enableInterrupts(const uint8_t *, MikoGB::CPUCore &);

/// HALT
/// Enter HALT mode. Bits are [ 0, 1, 1, 1, 0, 1, 1, 0 ]
int haltInstruction(const uint8_t *, MikoGB::CPUCore &);

/// STOP
/// Enter STOP mode. Bits are [ 0, 0, 0, 1, 0, 0, 0, 0] followed by NOP
int stopInstruction(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* SpecialInstructions_hpp */

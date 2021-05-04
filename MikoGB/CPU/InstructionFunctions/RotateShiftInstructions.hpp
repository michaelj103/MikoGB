//
//  RotateShiftInstructions.hpp
//  MikoGB
//
//  Created on 5/3/21.
//

#ifndef RotateShiftInstructions_hpp
#define RotateShiftInstructions_hpp

#include "CPUCore.hpp"

namespace CPUInstructions {

/// RLCA
/// Rotate the accumulator left and carry out the high bit. Bits are [ 0, 0, 0, 0, 0, 1, 1, 1 ]
/// Accumulator is rotated left. CY is set to the former high bit of accumulator and A0 is as well
int rotateLeftAccumulatorCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RLA
/// Rotate the accumulator left through the carry. Bits are [ 0, 0, 0, 1, 0, 1, 1, 1 ]
/// Accumulator is rotated left as if it is 9 bits with the carry at the highest position
int rotateLeftAccumulatorThroughCarry(const uint8_t *, MikoGB::CPUCore &);

/// RRCA
/// Rotate the accumulator right and carry out the low bit. Bits are [ 0, 0, 0, 0, 1, 1, 1, 1 ]
/// Accumulator is rotated right. CY is set to the former low bit of accumulator and A7 is as well
int rotateRightAccumulatorCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RRA
/// Rotate the accumulator right through the carry. Bits are [ 0, 0, 0, 1, 1, 1, 1, 1 ]
/// Accumulator is rotated right as if it is 9 bits with the carry at the lowest position
int rotateRightAccumulatorThroughCarry(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* RotateShiftInstructions_hpp */

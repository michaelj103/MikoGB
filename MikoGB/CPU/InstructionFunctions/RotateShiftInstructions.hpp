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

/// RLC r     (Extended opcode 0xCB)
/// Rotate argument left and carry out the high bit. Bits are [ 0, 0, 0, 0, 0, r2, r1, r0 ]
/// Standard register codes including an HL option
int rotateLeftRegisterCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RLC (HL)     (Extended opcode 0xCB)
/// Rotate the byte at address pointed to by HL pointer. Carry out high. Bits are [ 0, 0, 0, 0, 0, 1, 1, 0 ]
int rotateLeftPtrHLCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RL r     (Extended opcode 0xCB)
/// Rotate argument left through the carry. Bits are [ 0, 0, 0, 1, 0, r2, r1, r0 ]
/// Standard register codes including an HL option
int rotateLeftRegisterThroughCarry(const uint8_t *, MikoGB::CPUCore &);

/// RL (HL)     (Extended opcode 0xCB)
/// Rotate the byte at address pointed to by HL pointer through the carry. Bits are [ 0, 0, 0, 1, 0, 1, 1, 0 ]
int rotateLeftPtrHLThroughCarry(const uint8_t *, MikoGB::CPUCore &);

/// RRC r     (Extended opcode 0xCB)
/// Rotate argument right and carry out the low bit. Bits are [ 0, 0, 0, 0, 1, r2, r1, r0 ]
/// Standard register codes including an HL option
int rotateRightRegisterCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RRC (HL)     (Extended opcode 0xCB)
/// Rotate the byte at address pointed to by HL pointer. Carry out low. Bits are [ 0, 0, 0, 0, 1, 1, 1, 0 ]
int rotateRightPtrHLCarryOut(const uint8_t *, MikoGB::CPUCore &);

/// RR r     (Extended opcode 0xCB)
/// Rotate argument right through the carry. Bits are [ 0, 0, 0, 1, 1, r2, r1, r0 ]
/// Standard register codes including an HL option
int rotateRightRegisterThroughCarry(const uint8_t *, MikoGB::CPUCore &);

/// RR (HL)     (Extended opcode 0xCB)
/// Rotate the byte at address pointed to by HL pointer through the carry. Bits are [ 0, 0, 0, 1, 1, 1, 1, 0 ]
int rotateRightPtrHLThroughCarry(const uint8_t *, MikoGB::CPUCore &);

/// SLA m     (Extended opcode 0xCB)
/// Shift left with carry out for register argument. "Normal" shift. Bits are [ 0, 0, 1, 0, 0, r2, r1, r0 ]
/// Standard register codes including an HL option
int shiftLeftRegisterFill0(const uint8_t *, MikoGB::CPUCore &);

/// SLA (HL)     (Extended opcode 0xCB)
/// Shift left with carry out, contents pointed to by HL. "Normal" shift. Bits are [ 0, 0, 1, 0, 0, r2, r1, r0 ]
/// Standard register codes including an HL option
int shiftLeftPtrHLFill0(const uint8_t *, MikoGB::CPUCore &);

}

#endif /* RotateShiftInstructions_hpp */

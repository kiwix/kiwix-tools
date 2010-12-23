/*-
 * Copyright (c) 2004 - 2010 CTPP Team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the CTPP Team nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      CTPP2VMOpcodes.h
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_OPCODES_H__
#define _CTPP2_VM_OPCODES_H__ 1

#include "CTPP2Types.h"

// Version 3
#define VM_OPCODE_VERSION  0x00000003

/**
  @file CTPP2VMOpcodes.h
  @brief Virtual machine operation codes
*/

// Stop machine ////////////////////////////////////////////////////////////////////////////////////
#define HLT              0xFF000000 // Halt
#define BRK              0xFF010000 // Breakpoint
#define NOP              0xFF020000 // No operation

// Instructions //////// 0x-1-X---- ////////////////////////////////////////////////////////////////
// 0000 000X YYYY YYYY
#define SYSCALL          0x01010000 // System call
#define CALLNAME         0x01020000 // Call procedure by name
#define CALLIND          0x01030000 // Call procedure by name given in regsiter or stack (indirect)

#define CALL             0x01040000 // Procedure call
#define RET              0x01050000 // Return from procedure
#define JMP              0x01060000 // Unconditional jump
#define LOOP             0x01070000 // Loop

#define RCALL            0x01140000 // Procedure call, relational version
#define RJMP             0x01160000 // Unconditional jump, relational version
#define RLOOP            0x01170000 // Loop, relational version

// Stack operations //// 0x-2-X---- ////////////////////////////////////////////////////////////////
#define PUSH             0x02010000 // Push value into stack
#define POP              0x02020000 // Pop value from stack

#define PUSH13           0x02030000 // Push AR, BR, CR, DR registers into stack
#define POP13            0x02040000 // Pop AR, BR, CR, DR registers from stack

#define PUSH47           0x02050000 // Push ER, FR, GR, HR registers into stack
#define POP47            0x02060000 // Pop ER, FR, GR, HR registers from stack

#define PUSHA            0x02070000 // Push ALL registers into stack
#define POPA             0x02080000 // Pop ALL registers from stack

// Arithmetic ops. ///// 0x-3-X---- ////////////////////////////////////////////////////////////////
#define ADD              0x03010000 // Add values
#define SUB              0x03020000 // Substract values
#define MUL              0x03030000 // Multiplicate values
#define DIV              0x03040000 // Divide values
#define INC              0x03050000 // Increment value
#define DEC              0x03060000 // Decrement value
#define IDIV             0x03070000 // Integer division
#define MOD              0x03080000 // Divide two numbers and returns only the remainder
#define NEG              0x03090000 // Negate
#define NOT              0x030A0000 // Logical negation

// String ops. ///////// 0x-3X----- ////////////////////////////////////////////////////////////////
#define CONCAT           0x03100000 // Concatenate strings

// Register ops. /////// 0x-4------ /////////////////////////////////////////////////////////////////
#define MOV              0x04010000 // Move value (register-to-register and stack-to-register)
#define MOVIINT          0x04020000 // Move indirect ARRAY to register
#define MOVISTR          0x04030000 // Move indirect HASH  to register
#define IMOVINT          0x04040000 // Move register to indirect ARRAY
#define IMOVSTR          0x04050000 // Move register to indirect HASH
#define MOVSIZE          0x04060000 // Move size of array or hash to register
#define MOVIREG          0x04070000 // Move indirect ARRAY from REGISTER pointed by REGISTER to another register

// Comparison ops. ///// 0x-5-X---- //////////////////////////////////////////////////////////////////
#define CMP              0x05010000 // Compare values, arithmetic
#define SCMP             0x05020000 // Compare values, strging

// Comparison flags //// 0x000X0000 //////////////////////////////////////////////////////////////////
#define FL_NE            0x00010000 // Not Equal, NO flags
#define FL_EQ            0x00020000 // Equal
#define FL_LT            0x00040000 // Less
#define FL_GT            0x00080000 // Greater
#define FL_PF            0x00100000 // Parity flag
#define FL_NPF           0x00200000 // NOT Parity flag

// Conditional ops.1 /// 0x-6-X---- //////////////////////////////////////////////////////////////////
#define JXX              0x06000000    // Conditional jump, generic
#define JN               (JXX | FL_NE) // Jump if !=
#define JL               (JXX | FL_LT) // Jump if <
#define JG               (JXX | FL_GT) // Jump if >
#define JE               (JXX | FL_EQ) // Jump if ==
#define JLE              (JL | JE)     // Jump if <=
#define JGE              (JG | JE)     // Jump if >=

#define JEVEN            (JXX | FL_PF)  // Jump if Result is Even
#define JODD             (JXX | FL_NPF) // Jump if Result is ODD

// Conditional ops.2 /// 0x-7-X---- //////////////////////////////////////////////////////////////////
#define RJXX             0x07000000     // Conditional jump, relational version
#define RJN              (RJXX | FL_NE) // Jump if !=
#define RJL              (RJXX | FL_LT) // Jump if <
#define RJG              (RJXX | FL_GT) // Jump if >
#define RJE              (RJXX | FL_EQ) // Jump if ==
#define RJLE             (RJL | RJE)    // Jump if <=
#define RJGE             (RJG | RJE)    // Jump if >=

#define RJEVEN           (RJXX | FL_PF)  // Jump if Result is Even
#define RJODD            (RJXX | FL_NPF) // Jump if Result is ODD

// Other ops. ////////// 0x-8-X---- ////////////////////////////////////////////////////////////////
#define CLEAR            0x08010000 // Clear register
#define OUTPUT           0x08020000 // Direct output a value to output collector
#define REPLACE          0x08030000 // Replace variable in stack or register
#define EXIST            0x08040000 // Check existence of operand

// Sources ///////////// 0x-------X //////////////////////////////////////////////////////////////////
#define ARG_SRC_AR       0x00000000 // AR is source register
#define ARG_SRC_BR       0x00000001 // BR is source register
#define ARG_SRC_CR       0x00000002 // CR is source register
#define ARG_SRC_DR       0x00000003 // DR is source register
#define ARG_SRC_ER       0x00000004 // ER is source register
#define ARG_SRC_FR       0x00000005 // FR is source register
#define ARG_SRC_GR       0x00000006 // GR is source register
#define ARG_SRC_HR       0x00000007 // HR is source register

#define ARG_SRC_LASTREG  ARG_SRC_HR // Last regsiter is HR

#define ARG_SRC_STACK    0x00000010 // Stack value is source
#define ARG_SRC_IND_VAL  0x00000011 // Stack value is indirect array value is source
#define ARG_SRC_IND_STR  0x00000012 // Stack value is indirect hash value is source
#define ARG_SRC_INT      0x00000013 // Integer value from static data segment is source
#define ARG_SRC_FLOAT    0x00000014 // Float value from static data segment is source
#define ARG_SRC_STR      0x00000015 // String value from static text segment is source


// Destinations //////// 0x-----X-- /////////////////////////////////////////////////////
#define ARG_DST_AR       0x00000000 // AR is destination register
#define ARG_DST_BR       0x00000100 // BR is destination register
#define ARG_DST_CR       0x00000200 // CR is destination register
#define ARG_DST_DR       0x00000300 // DR is destination register
#define ARG_DST_ER       0x00000400 // ER is destination register
#define ARG_DST_FR       0x00000500 // FR is destination register
#define ARG_DST_GR       0x00000600 // GR is destination register
#define ARG_DST_HR       0x00000700 // HR is destination register

#define ARG_DST_LASTREG  ARG_DST_HR // Last regsiter is HR

#define ARG_DST_STACK    0x00001000 // Top of Stack value is destination
#define ARG_DST_IND_VAL  0x00001100 // Stack value is indirect array value is destination
#define ARG_DST_IND_STR  0x00001200 // Stack value is indirect hash value is destination

#define SYSCALL_PARAMS(x, y) ( (UINT_32(x << 16) & 0xFFFF0000) | (UINT_32(y) & 0x0000FFFF) )

#define SYSCALL_OPCODE(x)    ((UINT_32(x) >> 16) & 0x0000FFFF)

#define SYSCALL_OPCODE_HI(x) ((UINT_32(x) >> 24) & 0x000000FF)
#define SYSCALL_OPCODE_LO(x) ((UINT_32(x) >> 16) & 0x000000FF)

#define SYSCALL_REG_DST(x)   ((UINT_32(x)     ) & 0x0000FF00)
#define SYSCALL_REG_SRC(x)    (UINT_32(x)       & 0x000000FF)

#endif // _CTPP2_VM_OPCODES_H__
// End.

/*-
 * Copyright (c) 2004 - 2009 CTPP Team
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
 *      CTPP2ErrorCodes.h
 *
 * $CAS$
 */
#ifndef _CTPP2_ERROR_CODES_HPP__
#define _CTPP2_ERROR_CODES_HPP__ 1

/**
  @file CTPP2ErrorCodes.h
  @brief CTPP2 error codes
*/

#define CTPP_ERROR_CODE_MASK                0x00FFFFFF
#define CTPP_ERROR_SUBSYSTEM_MASK           0xFF000000

#define CTPP_DATA_ERROR                     0x01000000
#define CTPP_VM_ERROR                       0x02000000
#define CTPP_COMPILER_ERROR                 0x04000000

#define CTPP_CHARSET_RECODE_ERROR           0x00000013
#define CTPP_OPERATORS_MISMATCH_ERROR       0x00000012
#define CTPP_SYNTAX_ERROR                   0x00000011
#define CTPP_PARSER_ERROR                   0x00000010

#define CTPP_ZERO_DIVISION_ERROR            0x0000000F
#define CTPP_EXECUTION_LIMIT_REACHED_ERROR  0x0000000E
#define CTPP_CODE_SEGMENT_OVERRUN_ERROR     0x0000000D
#define CTPP_INVALID_SYSCALL_ERROR          0x0000000C
#define CTPP_ILLEGAL_OPCODE_ERROR           0x0000000B
#define CTPP_STACK_OVERFLOW_ERROR           0x0000000A
#define CTPP_STACK_UNDERFLOW_ERROR          0x00000009
#define CTPP_VM_GENERIC_ERROR               0x00000008
#define CTPP_UNIX_ERROR                     0x00000007

#define CTPP_RANGE_ERROR                    0x00000006
#define CTPP_ACCESS_ERROR                   0x00000005
#define CTPP_TYPE_CAST_ERROR                0x00000004
#define CTPP_LOGIC_ERROR                    0x00000003
#define CTPP_UNKNOWN_ERROR                  0x00000002
#define STL_UNKNOWN_ERROR                   0x00000001

#endif /* _CTPP2_ERROR_CODES_HPP__ */
/* End. */

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
 *      CTPP2VM.cpp
 *
 * $CTPP$
 */

#include "CTPP2VM.hpp"

#include "CTPP2OutputCollector.hpp"
#include "CTPP2SyscallFactory.hpp"
#include "CTPP2VMMemoryCore.hpp"

#include "CTPP2VMDebugInfo.hpp"
#include "CTPP2VMOpcodes.h"
#include "CTPP2VMException.hpp"
#include "CTPP2VMStackException.hpp"
#include "CTPP2VMInstruction.hpp"

#include <string.h>

/*
    Foreground colors
       30    Black
       31    Red
       32    Green
       33    Yellow
       34    Blue
       35    Magenta
       36    Cyan
       37    White

    Background colors
       40    Black
       41    Red
       42    Green
       43    Yellow
       44    Blue
       45    Magenta
       46    Cyan
       47    White
*/
#define BRIGHT 1
#define RED    31
#define YELLOW 33
#define GREEN  32
#define BLUE   34
#define BG_BLACK 40


#ifndef WIN32 /* Windows console is not so funky */
#define HL_CODE(x) fprintf(stderr, "%c[%d;%d;%dm", 0x1B, BRIGHT, (x), BG_BLACK)
#define HL_RST  fprintf(stderr, "%c[%dm", 0x1B, 0)
#else
    #define HL_CODE(x)
    #define HL_RST
#endif

// Aliases for registers
#define AR oRegs[0]
#define BR oRegs[1]
#define CR oRegs[2]
#define DR oRegs[3]
#define ER oRegs[4]
#define FR oRegs[5]
#define GR oRegs[6]
#define HR oRegs[7]

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
VM::VM(SyscallFactory  * pISyscallFactory,
       const UINT_32     iIMaxArgStackSize,
       const UINT_32     iIMaxCodeStackSize,
       const UINT_32     iIMaxSteps,
       const UINT_32     iIDebugLevel): pSyscallFactory(pISyscallFactory),
                                        iMaxArgStackSize(iIMaxArgStackSize),
                                        iMaxCodeStackSize(iIMaxCodeStackSize),
                                        iMaxSteps(iIMaxSteps),
                                        iDebugLevel(iIDebugLevel),
                                        iMaxCalls(0),
                                        iMaxUsedCalls(0),
                                        aCallTranslationMap(NULL),
                                        oVMArgStack(iMaxArgStackSize),
                                        oVMCodeStack(iMaxCodeStackSize)
{
	;;
}

//
// Initialize virtual machine
//
INT_32 VM::Init(const VMMemoryCore  * pMemoryCore,
                OutputCollector     * pOutputCollector,
                Logger              * pLogger)
{
	// Create syscalls translation map
	iMaxUsedCalls = pMemoryCore -> syscalls.GetRecordsNum();
	// Cache
	if (iMaxUsedCalls >= iMaxCalls)
	{
		delete [] aCallTranslationMap;
		aCallTranslationMap = new SyscallHandler *[iMaxUsedCalls];
		iMaxCalls = iMaxUsedCalls;
	}

	// Initialize program
	for (UINT_32 iCallNum = 0; iCallNum < iMaxUsedCalls; ++iCallNum)
	{
		// Ugh
		UINT_32 iCallNameLength = 0;
		// NULL-terminated string, iCallNameLength will not used
		CCHAR_P sCallName = pMemoryCore -> syscalls.GetData(iCallNum, iCallNameLength);

		// Get syscall
		SyscallHandler * pTMP = pSyscallFactory -> GetHandlerByName(sCallName);

		// If this syscall is not present in factory, throw exception
		if (pTMP == NULL)
		{
			UINT_32 iDataSize = 0;
			CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(pMemoryCore -> instructions[0].reserved).GetDescrId(), iDataSize);

			throw InvalidSyscall(sCallName, 0, pMemoryCore -> instructions[0].reserved, szTMP);
		}

		// All OK
		aCallTranslationMap[iCallNum] = pTMP;

		// Initialize syscall handler
		pTMP -> PreExecuteSetup(*pOutputCollector,
		                        DR,
		                        pMemoryCore -> syscalls,
		                        pMemoryCore -> static_data,
		                        pMemoryCore -> static_text,
		                        *pLogger);
	}

return 0;
}

//
// Run program
//
INT_32 VM::Run(const VMMemoryCore  * pMemoryCore,
               OutputCollector     * pOutputCollector,
               UINT_32             & iIP,
               CDT                 & oCDT,
               Logger              * pLogger)
{
	DR = oCDT;
	// Get code segment
	const VMInstruction * aCode = pMemoryCore -> instructions;
	const UINT_32 iCodeLength   = pMemoryCore -> code_size;
	UINT_32 iExecutedSteps      = 0;

	try
	{
		while (iIP < iCodeLength)
		{
			const UINT_32 iOpCode = aCode[iIP].instruction;
			const UINT_32 iOpCodeHi = SYSCALL_OPCODE_HI(iOpCode);
			const UINT_32 iOpCodeLo = SYSCALL_OPCODE_LO(iOpCode);
#ifdef _DEBUG
HL_CODE(BLUE);
fprintf(stderr, "CODE 0x%08X ARG 0x%08X RES 0x%016llX | ", iOpCode, aCode[iIP].argument, (long long)(aCode[iIP].reserved));
HL_RST;
#endif

			switch(iOpCodeHi)
			{
				// Instructions //////// 0x-1-X----
				case 0x01:
					{
						switch(iOpCodeLo)
						{
							// SYSCALL
							case SYSCALL_OPCODE_LO(SYSCALL):
								{
									const UINT_32 iCallNum    = (aCode[iIP].argument & 0xFFFF0000) >> 16;
									const UINT_32 iCallArgNum = (aCode[iIP].argument & 0x0000FFFF);
#ifdef _DEBUG
{
	UINT_32 iCallNameLength = 0;
	// NULL-terminated string, iCallNameLength not need to use
	CCHAR_P sCallName = pMemoryCore -> syscalls.GetData(iCallNum, iCallNameLength);
	HL_CODE(YELLOW);
	fprintf(stderr, "0x%08X SYSCALL   0x%08X %s(ARGS: %d)\n", iIP, iCallNum, sCallName, iCallArgNum);
	HL_RST;
}
#endif
									// Check call number
									if (iCallNum > iMaxUsedCalls)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw InvalidSyscall("*** CORRUPTED ***", iIP, aCode[iIP].reserved, szTMP);
									}

									CDT oResult(CDT::UNDEF);
									// Invoke handler
									if (aCallTranslationMap[iCallNum] -> Handler(oVMArgStack.GetStackFrame(), iCallArgNum, oResult, *pLogger) != 0)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw InvalidSyscall("*** Internal syscall error ***", iIP, aCode[iIP].reserved, szTMP);
									}

									// Clear stack
									oVMArgStack.ClearStack(iCallArgNum);

									// Store execution result into stack
									oVMArgStack.PushElement(oResult);

									++iIP;
								}
								break;

							// CALLNAME
							case SYSCALL_OPCODE_LO(CALLNAME):
								{
									// Get call name
									UINT_32 iDataSize = 0;
									CCHAR_P szCallName = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);

									UINT_32 iNewIP = UINT_32(pMemoryCore -> calls_table.Get(szCallName, iDataSize));
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X CALLNAME  `%s` -> %d\n", iIP, szCallName, iNewIP);
HL_RST;
#endif
									// Call exist?
									if (iNewIP == (UINT_32)-1)
									{
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw InvalidCall(iIP, aCode[iIP].reserved, szCallName, szTMP);
									}

									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									oVMCodeStack.PushAddress(iIP + 1);
									iIP = iNewIP;
								}
								break;

							// CALLIND
							case SYSCALL_OPCODE_LO(CALLIND):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X CALLIND   ", iIP);
#endif
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);

									// Call name
									STLW::string sCallName;

									// From register?
									if (iSrcReg <= ARG_SRC_HR)
									{
#ifdef _DEBUG
fprintf(stderr, "%cR (`%s`)\n", CHAR_8(iSrcReg + 'A'), oRegs[iSrcReg].GetString().c_str());
HL_RST;
#endif
										sCallName = oRegs[iSrcReg].GetString();
									}
									// From Stack?
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d] (`%s`)\n", 0, oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
										sCallName = oVMArgStack.GetTopElement(0).GetString();
										oVMArgStack.ClearStack(1);
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}

									// New IP
									const UINT_32 iNewIP = UINT_32(pMemoryCore -> calls_table.Get(sCallName.c_str(), sCallName.size()));

									// Call exist?
									if (iNewIP == (UINT_32)-1)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw InvalidCall(iIP, aCode[iIP].reserved, sCallName.c_str(), szTMP);
									}

									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									// Store return address
									oVMCodeStack.PushAddress(iIP + 1);
									iIP = iNewIP;
								}
								break;

							// CALL
							case SYSCALL_OPCODE_LO(CALL):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X CALL      0x%08X\n", iIP, aCode[iIP].argument);
#endif
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iNewIP = aCode[iIP].argument;
									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									oVMCodeStack.PushAddress(iIP + 1);
									iIP = iNewIP;
								}
								break;

							// RET
							case SYSCALL_OPCODE_LO(RET):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X RET       %d\n", iIP, aCode[iIP].argument);
HL_RST;
#endif
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									// Clear stack
									oVMArgStack.ClearStack(aCode[iIP].argument);
									// Return
									iIP = oVMCodeStack.PopAddress();
								}
								break;

							// JMP
							case SYSCALL_OPCODE_LO(JMP):
								{
#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X JMP       0x%08X\n", iIP, aCode[iIP].argument);
HL_RST;
#endif
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iNewIP = aCode[iIP].argument;
									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									iIP = iNewIP;
								}
								break;

							// LOOP, cycle
							case SYSCALL_OPCODE_LO(LOOP):
								{
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									if (iDstReg <= ARG_DST_LASTREG && iSrcReg <= ARG_SRC_LASTREG)
									{
										CDT & oLoopReg = oRegs[iSrcReg];

#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X LOOP      0x%08X (%cR = %d, %cR = %d)\n", iIP, aCode[iIP].argument, CHAR_8(iSrcReg + 'A'), INT_32(oLoopReg.GetInt()), CHAR_8((iDstReg >> 8) + 'A'), INT_32(oRegs[iDstReg >> 8].GetInt()));
HL_RST;
#endif
										// Decrease number of iterations
										--oLoopReg;
										// End of cycle?
										if (oLoopReg <= 0) { ++iIP; }
										// New iteration
										else
										{
											const UINT_32 iNewIP = aCode[iIP].argument;
											// New IP is correct?
											if (iNewIP >= iCodeLength)
											{
												UINT_32 iDataSize = 0;
												CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
												throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
											}
											iIP = iNewIP;

											// Iteration counter
											++oRegs[iDstReg >> 8];
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// RCALL
							case SYSCALL_OPCODE_LO(RCALL):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X RCALL     0x%08X\n", iIP, aCode[iIP].argument);
#endif
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iNewIP = iIP + aCode[iIP].argument;
									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									oVMCodeStack.PushAddress(iIP + 1);
									iIP = iNewIP;
								}
								break;

							// RJMP
							case SYSCALL_OPCODE_LO(RJMP):
								{
#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X RJMP      0x%08X\n", iIP, aCode[iIP].argument);
HL_RST;
#endif
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iNewIP = iIP + aCode[iIP].argument;
									// New IP is correct?
									if (iNewIP >= iCodeLength)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
									}

									iIP = iNewIP;
								}
								break;

							// RLOOP, cycle
							case SYSCALL_OPCODE_LO(RLOOP):
								{
									// Check execution limit
									if (iExecutedSteps >= iMaxSteps)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
									}

									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									if (iDstReg <= ARG_DST_LASTREG && iSrcReg <= ARG_SRC_LASTREG)
									{
										CDT & oLoopReg = oRegs[iSrcReg];

#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X RLOOP     0x%08X (%cR = %d, %cR = %d)\n", iIP, aCode[iIP].argument, CHAR_8(iSrcReg + 'A'), INT_32(oLoopReg.GetInt()), CHAR_8((iDstReg >> 8) + 'A'), INT_32(oRegs[iDstReg >> 8].GetInt()));
HL_RST;
#endif

										// Decrease number of iterations
										--oLoopReg;
										// End of cycle?
										if (oLoopReg <= 0) { ++iIP; }
										// New iteration
										else
										{
											const UINT_32 iNewIP = iIP - aCode[iIP].argument;
											// New IP is correct?
											if (iNewIP >= iCodeLength)
											{
												UINT_32 iDataSize = 0;
												CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
												throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
											}
											iIP = iNewIP;

											// Iteration counter
											++oRegs[iDstReg >> 8];
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}

								break;

							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					break;

				// Stack operations //// 0x-2-X----
				case 0x02:
					{
						switch(iOpCodeLo)
						{
							// PUSH
							case SYSCALL_OPCODE_LO(PUSH):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X PUSH      ", iIP);
#endif
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);

									// From register to stack
									if (iSrcReg <= ARG_SRC_LASTREG)
									{
										oVMArgStack.PushElement(oRegs[iSrcReg]);
#ifdef _DEBUG
fprintf(stderr, "%cR\n", CHAR_8(iOpCode + 'A'));
HL_RST;
#endif
									}
									// From static text segment to stack
									else if (iSrcReg == ARG_SRC_STR)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
										oVMArgStack.PushElement(STLW::string(szTMP, iDataSize));
#ifdef _DEBUG
fprintf(stderr, "STRING POS: %d (VAL: `%s`)\n", aCode[iIP].argument, szTMP);
HL_RST;
#endif
									}
									// From static data segment to stack (integer value)
									else if (iSrcReg == ARG_SRC_INT)
									{
										oVMArgStack.PushElement(pMemoryCore -> static_data.GetInt(aCode[iIP].argument));
#ifdef _DEBUG
fprintf(stderr, "INT POS: %d (VAL: %d)\n", aCode[iIP].argument, INT_32(pMemoryCore -> static_data.GetInt(aCode[iIP].argument)));
HL_RST;
#endif
									}
									// From static data segment to stack (float value)
									else if (iSrcReg == ARG_SRC_FLOAT)
									{
										oVMArgStack.PushElement(pMemoryCore -> static_data.GetFloat(aCode[iIP].argument));
#ifdef _DEBUG
fprintf(stderr, "FLOAT POS: %d (VAL: %f)\n", aCode[iIP].argument, pMemoryCore -> static_data.GetFloat(aCode[iIP].argument));
HL_RST;
#endif
									}
									// From indirect ARRAY
									else if (iSrcReg == ARG_SRC_IND_VAL)
									{
										const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
										// Indirect operations works ONLY with registers AR - HR and LR
										if (iDstReg <= ARG_DST_LASTREG)
										{
											oVMArgStack.PushElement(oRegs[iDstReg >> 8].GetCDT(aCode[iIP].argument));
#ifdef _DEBUG
fprintf(stderr, "%cR[%d] (`%s`)\n", CHAR_8((iDstReg >> 8) + 'A'), aCode[iIP].argument, oRegs[iDstReg >> 8].GetCDT(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// From indirect HASH
									else if (iSrcReg == ARG_SRC_IND_STR)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);

										const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
										// Indirect operations works ONLY with registers AR - HR and LR
										if (iDstReg <= ARG_DST_LASTREG)
										{
											bool bCDTExist = false;
#ifdef _DEBUG
fprintf(stderr, "%cR[\"%s\"] ", CHAR_8((iDstReg >> 8) + 'A'), szTMP);
fprintf(stderr, "(`%s`)\n", oRegs[iDstReg >> 8].GetExistedCDT(STLW::string(szTMP, iDataSize), bCDTExist).GetString().c_str());
HL_RST;
#endif
											oVMArgStack.PushElement(oRegs[iDstReg >> 8].GetExistedCDT(STLW::string(szTMP, iDataSize), bCDTExist));

											// Found
											if (bCDTExist) { iFlags = FL_EQ; }
											// Not Found
											else           { iFlags = 0;     }
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// From stack to stack
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d](%s)\n", aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										oVMArgStack.PushElement(oVMArgStack.GetTopElement(aCode[iIP].argument));
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// POP
							case SYSCALL_OPCODE_LO(POP):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X POP       ", iIP);
#endif
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);

									// From register to stack
									if (iSrcReg <= ARG_SRC_LASTREG)
									{
										oRegs[iSrcReg] = oVMArgStack.GetTopElement(0);
										oVMArgStack.ClearStack(1);
#ifdef _DEBUG
fprintf(stderr, "%cR\n", CHAR_8(iOpCode + 'A'));
HL_RST;
#endif
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// PUSH13
							case SYSCALL_OPCODE_LO(PUSH13):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X PUSH13    \n", iIP);
#endif
									for (INT_32 iSrcReg = ARG_SRC_AR; iSrcReg <= ARG_SRC_DR; ++iSrcReg)
									{
										oVMArgStack.PushElement(oRegs[iSrcReg]);
									}
								}
								break;

							// POP13
							case SYSCALL_OPCODE_LO(POP13):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X POP13     \n", iIP);
#endif
									for (INT_32 iSrcReg = ARG_SRC_DR; iSrcReg >= ARG_SRC_AR; --iSrcReg)
									{
										oRegs[iSrcReg] = oVMArgStack.GetTopElement(0);
										oVMArgStack.ClearStack(1);
									}
								}
								break;

							// PUSH47
							case SYSCALL_OPCODE_LO(PUSH47):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X PUSH47    \n", iIP);
#endif

									for (INT_32 iSrcReg = ARG_SRC_ER; iSrcReg <= ARG_SRC_HR; ++iSrcReg)
									{
										oVMArgStack.PushElement(oRegs[iSrcReg]);
									}
								}
								break;

							// POP47
							case SYSCALL_OPCODE_LO(POP47):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X POP47     \n", iIP);
#endif

									for (INT_32 iSrcReg = ARG_SRC_HR; iSrcReg >= ARG_SRC_ER; --iSrcReg)
									{
										oRegs[iSrcReg] = oVMArgStack.GetTopElement(0);
										oVMArgStack.ClearStack(1);
									}
								}
								break;

							// PUSHA
							case SYSCALL_OPCODE_LO(PUSHA):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X PUSHA     \n", iIP);
#endif

									for (INT_32 iSrcReg = 0; iSrcReg <= ARG_SRC_LASTREG; ++iSrcReg)
									{
										oVMArgStack.PushElement(oRegs[iSrcReg]);
									}
								}
								break;

							// POPA
							case SYSCALL_OPCODE_LO(POPA):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X POPA      \n", iIP);
#endif

									for (INT_32 iSrcReg = ARG_SRC_LASTREG; iSrcReg >= 0; --iSrcReg)
									{
										oRegs[iSrcReg] = oVMArgStack.GetTopElement(0);
										oVMArgStack.ClearStack(1);
									}
								}
								break;

							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					++iIP;
					break;

				// Arithmetic ops. ///// 0x-3-X----
				case 0x03:
					{
						const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
						const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);

						switch(iOpCodeLo)
						{
							// ADD, Add values
							case SYSCALL_OPCODE_LO(ADD):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X ADD       ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									oVMArgStack.GetTopElement(1) += oVMArgStack.GetTopElement(0);
									oVMArgStack.ClearStack(1);
								}
								break;

							// SUB, Substract values
							case SYSCALL_OPCODE_LO(SUB):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X SUB       ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									oVMArgStack.GetTopElement(1) -= oVMArgStack.GetTopElement(0);
									oVMArgStack.ClearStack(1);
								}
								break;

							// MUL, Multiplicate values
							case SYSCALL_OPCODE_LO(MUL):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MUL       ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									oVMArgStack.GetTopElement(1) *= oVMArgStack.GetTopElement(0);
									oVMArgStack.ClearStack(1);
								}
								break;

							// DIV, Divide values
							case SYSCALL_OPCODE_LO(DIV):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X DIV       ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									oVMArgStack.GetTopElement(1) /= oVMArgStack.GetTopElement(0);
									oVMArgStack.ClearStack(1);
								}
								break;

							// INC, Increment value
							case SYSCALL_OPCODE_LO(INC):
								{
									if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X INC       %cR (%d)\n", iIP, CHAR_8(iSrcReg + 'A'), INT_32(oRegs[iSrcReg].GetInt()));
HL_RST;
#endif
										++oRegs[iSrcReg];
									}
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X INC       STACK[%d] (%d)\n", iIP, 0, INT_32(oVMArgStack.GetTopElement(0).GetInt()));
HL_RST;
#endif
										++oVMArgStack.GetTopElement(0);
									}
								}
								break;

							// DEC, Decrement value
							case SYSCALL_OPCODE_LO(DEC):
								{
									if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X DEC       %cR (%d)\n", iIP, CHAR_8(iSrcReg + 'A'), INT_32(oRegs[iSrcReg].GetInt()));
HL_RST;
#endif
										--oRegs[iSrcReg];
									}
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X DEC       STACK[%d] (%d)\n", iIP, 0, INT_32(oVMArgStack.GetTopElement(0).GetInt()));
HL_RST;
#endif
										--oVMArgStack.GetTopElement(0);
									}
								}
								break;

							// IDIV, Integer division
							case SYSCALL_OPCODE_LO(IDIV):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X IDIV      ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									const INT_64 iFirst  = oVMArgStack.GetTopElement(1).GetInt();
									const INT_64 iSecond = oVMArgStack.GetTopElement(0).GetInt();

									if (iSecond == 0)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw ZeroDivision(iIP, aCode[iIP].reserved, szTMP);
									}

									oVMArgStack.GetTopElement(1) = iFirst / iSecond;
									oVMArgStack.ClearStack(1);
								}
								break;

							// MOD, Divide two numbers and returns only the remainder
							case SYSCALL_OPCODE_LO(MOD):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);

#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOD       ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									const INT_64 iFirst  = oVMArgStack.GetTopElement(1).GetInt();
									const INT_64 iSecond = oVMArgStack.GetTopElement(0).GetInt();

									oVMArgStack.GetTopElement(1) = iFirst % iSecond;
									oVMArgStack.ClearStack(1);
								}
								break;

							// NEG, Negate
							case SYSCALL_OPCODE_LO(NEG):
								{

									if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X NEG       %cR (%d)\n", iIP, CHAR_8(iSrcReg + 'A'), INT_32(oRegs[iSrcReg].GetInt()));
HL_RST;
#endif
										CDT & oTMP = oRegs[iSrcReg];
										if (oTMP.GetType() <= CDT::REAL_VAL) { oTMP = 0 - oTMP; }
									}
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X NEG       STACK[%d](%s)\n", iIP, aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										CDT & oTMP = oVMArgStack.GetTopElement(aCode[iIP].argument);
										if (oTMP.GetType() <= CDT::REAL_VAL) { oTMP = 0 - oTMP; }
									}
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// NOT, Logical negation
							case SYSCALL_OPCODE_LO(NOT):
								{
									if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "0x%08X NOT       STACK[%d](%c)\n", iIP, aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).Defined() ? 't':'f');
HL_RST;
#endif
										CDT & oTMP = oVMArgStack.GetTopElement(aCode[iIP].argument);

										if (oTMP.Defined()) { oTMP = CDT(CDT::UNDEF); }
										// Undefined
										else                { oTMP = 1;               }
									}
									else if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
fprintf(stderr, "0x%08X NOT       %cR (`%s`)\n", iIP, CHAR_8(iSrcReg + 'A'), oRegs[iSrcReg].GetString().c_str());
HL_RST;
#endif
										// Defined
										if (oRegs[iSrcReg].Defined()) { oRegs[iSrcReg] = CDT(CDT::UNDEF); }
										// Undefined
										else                          { oRegs[iSrcReg] = 1;               }
									}
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// CONCAT, Concatenate strings
							case SYSCALL_OPCODE_LO(CONCAT):
								{
									CheckStackOnlyRegs(iSrcReg, iDstReg, pMemoryCore, iIP);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X CONCAT    ('%s', '%s')\n", iIP, oVMArgStack.GetTopElement(1).GetString().c_str(), oVMArgStack.GetTopElement(0).GetString().c_str());
HL_RST;
#endif
									oVMArgStack.GetTopElement(1).Append(oVMArgStack.GetTopElement(0));
									oVMArgStack.ClearStack(1);
								}
								break;

							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					++iIP;
					break;

				// Register ops. /////// 0x-4------
				case 0x04:
					{
						switch(iOpCodeLo)
						{
							// MOV
							case SYSCALL_OPCODE_LO(MOV):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOV       ", iIP);
#endif
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
#ifdef _DEBUG
fprintf(stderr, "%cR, ", CHAR_8((iDstReg >> 8) + 'A'));
#endif
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
#ifdef _DEBUG
fprintf(stderr, "%cR == `%s`\n", CHAR_8(iSrcReg + 'A'), oRegs[iSrcReg].GetString().c_str());
HL_RST;
#endif
											oRegs[iDstReg >> 8] = oRegs[iSrcReg];
										}
										// Register-to-stack
										else if (iSrcReg <= ARG_SRC_STACK)
										{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d] = `%s`\n", aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
											oRegs[iDstReg >> 8] = oVMArgStack.GetTopElement(aCode[iIP].argument);
										}
										// Integer-to-register
										else if (iSrcReg == ARG_SRC_INT)
										{
#ifdef _DEBUG
fprintf(stderr, "INT POS: %d (VAL: %d)\n", aCode[iIP].argument, INT_32(pMemoryCore -> static_data.GetInt(aCode[iIP].argument)));
HL_RST;
#endif
											oRegs[iDstReg >> 8] = pMemoryCore -> static_data.GetInt(aCode[iIP].argument);
										}
										// Float-to-register
										else if (iSrcReg == ARG_SRC_FLOAT)
										{
#ifdef _DEBUG
fprintf(stderr, "FLOAT POS: %d (VAL: %f)\n", aCode[iIP].argument, pMemoryCore -> static_data.GetFloat(aCode[iIP].argument));
HL_RST;
#endif
											oRegs[iDstReg >> 8] = pMemoryCore -> static_data.GetFloat(aCode[iIP].argument);
										}
										// String-to-register
										else if (iSrcReg == ARG_SRC_STR)
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
#ifdef _DEBUG
fprintf(stderr, "STRING POS: %d (VAL: `%s`)\n", aCode[iIP].argument, szTMP);
HL_RST;
#endif
											oRegs[iDstReg >> 8] = STLW::string(szTMP, iDataSize);
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// MOVIINT, Move indirect ARRAY to register
							case SYSCALL_OPCODE_LO(MOVIINT):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOVIINT   %cR, %cR[%d] (`%s`)", iIP, CHAR_8((iDstReg >> 8) + 'A'), CHAR_8(iSrcReg + 'A'), aCode[iIP].argument, oRegs[iDstReg >> 8].GetString().c_str());
HL_RST;
#endif

											oRegs[iDstReg >> 8] = oRegs[iSrcReg].GetCDT(aCode[iIP].argument);
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// MOVISTR, Move indirect HASH to register
							case SYSCALL_OPCODE_LO(MOVISTR):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);

											bool bCDTExist = false;
											oRegs[iDstReg >> 8] = oRegs[iSrcReg].GetExistedCDT(STLW::string(szTMP, iDataSize), bCDTExist);

											// Found
											if (bCDTExist) { iFlags = FL_EQ; }
											// Not Found
											else           { iFlags = FL_NE; }

#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOVISTR   %cR, %cR[%s] (`%s`)\n", iIP, CHAR_8((iDstReg >> 8)  + 'A'), CHAR_8(iSrcReg + 'A'), szTMP, oRegs[iDstReg >> 8].GetString().c_str());
HL_RST;
#endif
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// IMOVINT, Move register to indirect ARRAY
							case SYSCALL_OPCODE_LO(IMOVINT):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
											oRegs[iDstReg >> 8][aCode[iIP].argument] = oRegs[iSrcReg];
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X IMOVINT   %cR[%d], %cR[%d] (`%s`)\n", iIP, CHAR_8((iDstReg >> 8)+ 'A'), aCode[iIP].argument, CHAR_8(iSrcReg + 'A'), aCode[iIP].argument, oRegs[iSrcReg].GetString().c_str());
HL_RST;
#endif
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// IMOVSTR
							case SYSCALL_OPCODE_LO(IMOVSTR):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
											oRegs[iDstReg >> 8][STLW::string(szTMP, iDataSize)] = oRegs[iSrcReg];
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X IMOVSTR   %cR[%s], %cR (`%s`)", iIP, CHAR_8(iDstReg + 'A'), szTMP, CHAR_8(iSrcReg + 'A'), oRegs[iDstReg].GetString().c_str());
HL_RST;
#endif
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// MOVSIZE, Get object size
							case SYSCALL_OPCODE_LO(MOVSIZE):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);

									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOVSIZE   %cR, %cR.Size() (%d)\n", iIP, CHAR_8((iDstReg >> 8) + 'A'), CHAR_8(iSrcReg + 'A'), UINT_32(oRegs[iSrcReg].Size()));
HL_RST;
#endif
											const UINT_64 iSize = oRegs[iSrcReg].Size();
											oRegs[iDstReg >> 8] = iSize;

											if (iSize == 0) { iFlags = FL_EQ; }
											else            { iFlags = 0;     }
										}
										// Stack-to-register
										else if (iSrcReg <= ARG_SRC_STACK)
										{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOVSIZE   %cR, STACK[%d] (%d)\n", iIP, CHAR_8((iDstReg >> 8) + 'A'), aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).Size());
HL_RST;
#endif
											const UINT_64 iSize = oVMArgStack.GetTopElement(aCode[iIP].argument).Size();
											oRegs[iDstReg >> 8] = iSize;

											if (iSize == 0) { iFlags = FL_EQ; }
											else            { iFlags = 0;     }
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// To stack
									else if (iDstReg <= ARG_DST_STACK)
									{
										// Register-to-stack
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X MOVSIZE   STACK[%d], %cR (%d)\n", iIP, aCode[iIP].argument, CHAR_8(iSrcReg + 'A'), oRegs[iSrcReg].Size());
HL_RST;
#endif
											const UINT_64 iSize = oRegs[iSrcReg].Size();
											oVMArgStack.GetTopElement(aCode[iIP].argument) = iSize;

											if (iSize == 0) { iFlags = FL_EQ; }
											else            { iFlags = 0;     }
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// MOVIREG
							case SYSCALL_OPCODE_LO(MOVIREG):
								{
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);

									// To register
									if (iDstReg <= ARG_DST_LASTREG)
									{
										// Register-to-register
										if (iSrcReg <= ARG_SRC_LASTREG)
										{
											const UINT_32 iArgNum = aCode[iIP].argument;
											// Illegal Opcode?
											if (iArgNum > ARG_SRC_LASTREG)
											{
												UINT_32 iDataSize = 0;
												CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
												throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
											}
#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X MOVIREG   %cR, %cR[%cR] (%d): %s\n", iIP, CHAR_8((iDstReg >> 8) + 'A'), CHAR_8(iSrcReg + 'A'), CHAR_8(iArgNum + 'A'), INT_32(oRegs[iArgNum].GetInt()), oRegs[iSrcReg][oRegs[iArgNum].GetInt()].GetString().c_str());
HL_RST;
#endif
											// AR <- CR[BR]
											oRegs[iDstReg >> 8] = oRegs[iSrcReg][oRegs[iArgNum].GetInt()];
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					++iIP;
					break;

				// Comparison ops. ///// 0x-5-X----
				case 0x05:
					{
						switch(iOpCodeLo)
						{
							// CMP
							case SYSCALL_OPCODE_LO(CMP):
								{
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									W_FLOAT dSrc    = 0.0;
									W_FLOAT dDst    = 0.0;
#ifdef _DEBUG
{
HL_CODE(GREEN);
fprintf(stderr, "0x%08X CMP       ", iIP);

STLW::string sMsg2;
if      (iDstReg <= ARG_DST_LASTREG) { sMsg2.append(1, (iDstReg >> 8) + 'A'); sMsg2.append("R(`"); sMsg2.append(oRegs[iDstReg >> 8].GetString());  sMsg2.append("`)"); }
else if (iDstReg == ARG_DST_STACK) { sMsg2.append("STACK[0](`"); sMsg2.append(oVMArgStack.GetTopElement(0).GetString()); sMsg2.append("`)"); }

STLW::string sMsg1;
if      (iSrcReg <= ARG_SRC_LASTREG) { sMsg1.append(1, iSrcReg + 'A'); sMsg1.append("R(`"); sMsg1.append(oRegs[iSrcReg].GetString());  sMsg1.append("`), "); }
else if (iSrcReg == ARG_SRC_STACK) { sMsg1.append("STACK[1](`"); sMsg1.append(oVMArgStack.GetTopElement(1).GetString()); sMsg1.append("`), "); }

fprintf(stderr, "%s%s\n", sMsg1.c_str(), sMsg2.c_str());
HL_RST;
}
#endif
									if (iDstReg <= ARG_DST_LASTREG)
									{
										dDst = oRegs[iDstReg >> 8].GetFloat();
									}
									else if (iDstReg == ARG_DST_STACK)
									{
										dDst = oVMArgStack.GetTopElement(0).GetFloat();
										oVMArgStack.PopElement();
									}

									if (iSrcReg <= ARG_SRC_LASTREG)
									{
										dSrc = oRegs[iSrcReg].GetFloat();
									}
									else if (iSrcReg == ARG_SRC_STACK)
									{
										dSrc = oVMArgStack.GetTopElement(0).GetFloat();
										oVMArgStack.PopElement();
									}

									W_FLOAT dTMP = dSrc - dDst;
									if      (dTMP < 0.0) { iFlags = FL_LT | FL_NE; }
									else if (dTMP > 0.0) { iFlags = FL_GT | FL_NE; }
									else                 { iFlags = FL_EQ; }

									const UINT_64 iTMP = UINT_64(dSrc);
									if ((iTMP % 2) == 0) { iFlags |= FL_PF;  }
									else                 { iFlags |= FL_NPF; }
								}
								break;

							// SCMP
							case SYSCALL_OPCODE_LO(SCMP):
								{
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									STLW::string sSrc;
									STLW::string sDst;
#ifdef _DEBUG
{
HL_CODE(GREEN);
fprintf(stderr, "0x%08X SCMP      ", iIP);

STLW::string sMsg2;
if      (iDstReg <= ARG_DST_LASTREG) { sMsg2.append(1, (iDstReg >> 8) + 'A'); sMsg2.append("R(`"); sMsg2.append(oRegs[iDstReg >> 8].GetString());  sMsg2.append("`)"); }
else if (iDstReg == ARG_DST_STACK) { sMsg2.append("STACK[0](`"); sMsg2.append(oVMArgStack.GetTopElement(0).GetString()); sMsg2.append("`)"); }

STLW::string sMsg1;
if      (iSrcReg <= ARG_SRC_LASTREG) { sMsg1.append(1, iSrcReg + 'A'); sMsg1.append("R(`"); sMsg1.append(oRegs[iSrcReg].GetString());  sMsg1.append("`), "); }
else if (iSrcReg == ARG_SRC_STACK) { sMsg1.append("STACK[1](`"); sMsg1.append(oVMArgStack.GetTopElement(1).GetString()); sMsg1.append("`), "); }

fprintf(stderr, "%s%s\n", sMsg1.c_str(), sMsg2.c_str());
HL_RST;
}
#endif
									if (iDstReg <= ARG_DST_LASTREG)
									{
										sDst = oRegs[iDstReg >> 8].GetString();
									}
									else if (iDstReg == ARG_DST_STACK)
									{
										sDst = oVMArgStack.GetTopElement(0).GetString();
										oVMArgStack.PopElement();
									}

									if (iSrcReg <= ARG_SRC_LASTREG)
									{
										sSrc = oRegs[iSrcReg].GetString();
									}
									else if (iSrcReg == ARG_SRC_STACK)
									{
										sSrc = oVMArgStack.GetTopElement(0).GetString();
										oVMArgStack.PopElement();
									}

									if      (sSrc < sDst) { iFlags = FL_LT | FL_NE; }
									else if (sSrc > sDst) { iFlags = FL_GT | FL_NE; }
									else                  { iFlags = FL_EQ; }
								}
								break;

							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					++iIP;
					break;

				// Conditional ops.1 /// 0x-6-X----
				case 0x06:
					{
						const UINT_32 iOpCodeFlag = iOpCode & 0x00FF0000;
#ifdef _DEBUG
{
HL_CODE(GREEN);
CHAR_8 sFlags[8];
INT_32 iPos = 0;
for (iPos = 0; iPos < 8; ++iPos)  { sFlags[iPos] = ' '; }
sFlags[7] = '\0'; iPos = 0;
if(iOpCodeFlag & FL_NE)  { sFlags[iPos++] = 'N'; }
if(iOpCodeFlag & FL_LT)  { sFlags[iPos++] = 'L'; }
if(iOpCodeFlag & FL_GT)  { sFlags[iPos++] = 'G'; }
if(iOpCodeFlag & FL_EQ)  { sFlags[iPos++] = 'E'; }
if(iOpCodeFlag & FL_PF)  { sFlags[iPos++] = 'P'; }
if(iOpCodeFlag & FL_NPF) { sFlags[iPos++] = 'N'; sFlags[iPos++] = 'P'; }
fprintf(stderr, "0x%08X J%s  0x%08X\n", iIP, sFlags, aCode[iIP].argument);
HL_RST;
}
#endif
						// Check execution limit
						if (iExecutedSteps >= iMaxSteps)
						{
							UINT_32 iDataSize = 0;
							CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
							throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
						}

						// Check flags
						if (!(iOpCodeFlag & iFlags))
						{
							++iIP;
						}
						else
						{
							const UINT_32 iNewIP = aCode[iIP].argument;
							if (iNewIP >= iCodeLength)
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
							}
							iIP = iNewIP;
						}
					}
					break;

				// Conditional ops.2 /// 0x-7-X----
				case 0x07:
					{
						const UINT_32 iOpCodeFlag = iOpCode & 0x00FF0000;
#ifdef _DEBUG
{
HL_CODE(GREEN);
CHAR_8 sFlags[8];
INT_32 iPos = 0;
for (iPos = 0; iPos < 8; ++iPos)  { sFlags[iPos] = ' '; }
sFlags[7] = '\0'; iPos = 0;
if(iOpCodeFlag & FL_NE)  { sFlags[iPos++] = 'N'; }
if(iOpCodeFlag & FL_LT)  { sFlags[iPos++] = 'L'; }
if(iOpCodeFlag & FL_GT)  { sFlags[iPos++] = 'G'; }
if(iOpCodeFlag & FL_EQ)  { sFlags[iPos++] = 'E'; }
if(iOpCodeFlag & FL_PF)  { sFlags[iPos++] = 'P'; }
if(iOpCodeFlag & FL_NPF) { sFlags[iPos++] = 'N'; sFlags[iPos++] = 'P'; }
fprintf(stderr, "0x%08X RJ%s 0x%08X\n", iIP, sFlags, aCode[iIP].argument);
HL_RST;
}
#endif
						// Check execution limit
						if (iExecutedSteps >= iMaxSteps)
						{
							UINT_32 iDataSize = 0;
							CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
							throw ExecutionLimitReached(iIP, aCode[iIP].reserved, szTMP);
						}

						// Check flags
						if (!(iOpCodeFlag & iFlags))
						{
							++iIP;
						}
						else
						{
							const UINT_32 iNewIP = iIP + aCode[iIP].argument;
							if (iNewIP >= iCodeLength)
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw CodeSegmentOverrun(iIP, aCode[iIP].reserved, szTMP);
							}
							iIP = iNewIP;
						}
					}
					break;
				// Other ops. ////////// 0x-8-X----
				case 0x08:
					{
						switch(iOpCodeLo)
						{
							// CLEAR
							case SYSCALL_OPCODE_LO(CLEAR):
								{
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X CLEAR     ", iIP);
#endif
									// Clear register
									if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
fprintf(stderr, "%cR\n", CHAR_8((iSrcReg >> 8) + 'A'));
HL_RST;
#endif
										const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
										oRegs[iDstReg >> 8] = CDT();
									}
									// Clear stack
									else if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d](%s)\n", aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										oVMArgStack.GetTopElement(aCode[iIP].argument) = CDT(CDT::UNDEF);
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// OUTPUT
							case SYSCALL_OPCODE_LO(OUTPUT):
								{
									// Register
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
#ifdef _DEBUG
HL_CODE(YELLOW);
fprintf(stderr, "0x%08X OUTPUT    ", iIP);
#endif
									// From stack
									if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d](`%s`)\n", aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										const STLW::string sTMP = oVMArgStack.GetTopElement(0).GetString();
										pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
										oVMArgStack.ClearStack(1);
									}
									// From static text segment
									else if (iSrcReg == ARG_SRC_STR)
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
#ifdef _DEBUG
fprintf(stderr, "STRING POS: %d (VAL: `%s`)\n", aCode[iIP].argument, szTMP);
HL_RST;
#endif
										pOutputCollector -> Collect(szTMP, iDataSize);
									}
									// From static data segment (integer value)
									else if (iSrcReg == ARG_SRC_INT)
									{
#ifdef _DEBUG
fprintf(stderr, "INT POS: %d (VAL: %d)\n", aCode[iIP].argument, INT_32(pMemoryCore -> static_data.GetInt(aCode[iIP].argument)));
HL_RST;
#endif
										const CDT oTMP(pMemoryCore -> static_data.GetInt(aCode[iIP].argument));
										const STLW::string sTMP = oTMP.GetString();
										pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
									}
									// From static data segment (float value)
									else if (iSrcReg == ARG_SRC_FLOAT)
									{
#ifdef _DEBUG
fprintf(stderr, "FLOAT POS: %d (VAL: %f)\n", aCode[iIP].argument, pMemoryCore -> static_data.GetFloat(aCode[iIP].argument));
HL_RST;
#endif
										const CDT oTMP(pMemoryCore -> static_data.GetFloat(aCode[iIP].argument));
										STLW::string sTMP = oTMP.GetString();
										pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
									}
									// From register
									else if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
fprintf(stderr, "%cR\n", CHAR_8(iSrcReg + 'A'));
HL_RST;
#endif
										const STLW::string sTMP = oRegs[iSrcReg].GetString();
										pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
									}
									// Indirect operations works ONLY with registers
									else if (iSrcReg == ARG_SRC_IND_VAL)
									{
										const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
#ifdef _DEBUG
fprintf(stderr, "%cR[%d] (`%s`)\n", CHAR_8((iDstReg >> 8) + 'A'), aCode[iIP].argument, oRegs[iDstReg >> 8].GetCDT(aCode[iIP].argument).GetString().c_str());
HL_RST;
#endif
										if (iDstReg <= ARG_DST_LASTREG)
										{
											const STLW::string sTMP = oRegs[iDstReg >> 8].GetCDT(aCode[iIP].argument).GetString();
											pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Indirect operations works ONLY with registers
									else if (iSrcReg == ARG_SRC_IND_STR)
									{
										const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);

										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
#ifdef _DEBUG
fprintf(stderr, "%cR[\"%s\"] (`%s`)\n", CHAR_8((iDstReg >> 8) + 'A'), szTMP, oRegs[iDstReg >> 8].GetCDT(STLW::string(szTMP, iDataSize)).GetString().c_str());
HL_RST;
#endif
										if (iDstReg <= ARG_DST_LASTREG)
										{
											STLW::string sTMP = oRegs[iDstReg >> 8].GetCDT(STLW::string(szTMP, iDataSize)).GetString();
											pOutputCollector -> Collect(sTMP.c_str(), sTMP.size());
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// REPLACE
							case SYSCALL_OPCODE_LO(REPLACE):
								{
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X REPLACE   ", iIP);
#endif
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									const UINT_32 iDstReg = SYSCALL_REG_DST(iOpCode);
									// From static text segment to stack
									if (iSrcReg == ARG_SRC_IND_STR)
									{
										if (iDstReg <= ARG_DST_LASTREG)
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);
#ifdef _DEBUG
fprintf(stderr, "%cR[\"%s\"] (`%s`)\n", CHAR_8((iDstReg >> 8) + 'A'), szTMP, oRegs[iDstReg >> 8].GetCDT(STLW::string(szTMP, iDataSize)).GetString().c_str());
HL_RST;
#endif
											oVMArgStack.GetTopElement(0) = oRegs[iDstReg >> 8].GetCDT(STLW::string(szTMP, iDataSize));
										}
										else if (iDstReg == ARG_DST_STACK)
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(aCode[iIP].argument, iDataSize);

											CDT & oTopStack  = oVMArgStack.GetTopElement(0);
											CDT oTMP = oTopStack.GetCDT(STLW::string(szTMP, iDataSize));
#ifdef _DEBUG
fprintf(stderr, "TOP STACK[\"%s\"] (`%s`)\n", szTMP, oTMP.GetString().c_str());
HL_RST;
#endif
											oTopStack = oTMP;
										}
										// Illegal Opcode?
										else
										{
											UINT_32 iDataSize = 0;
											CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
											throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
										}
									}
									// Illegal operation
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;

							// EXIST, check existence of operand
							case SYSCALL_OPCODE_LO(EXIST):
								{
#ifdef _DEBUG
HL_CODE(GREEN);
fprintf(stderr, "0x%08X EXIST     ", iIP);
#endif
									const UINT_32 iSrcReg = SYSCALL_REG_SRC(iOpCode);
									if (iSrcReg == ARG_SRC_STACK)
									{
#ifdef _DEBUG
fprintf(stderr, "STACK[%d](%c)\n", aCode[iIP].argument, oVMArgStack.GetTopElement(aCode[iIP].argument).Defined() ? 't':'f');
HL_RST;
#endif
										// Defined
										if (oVMArgStack.GetTopElement(aCode[iIP].argument).Defined()) { iFlags = FL_EQ; }
										// Undefined
										else { iFlags = FL_NE; }
									}
									else if (iSrcReg <= ARG_SRC_LASTREG)
									{
#ifdef _DEBUG
fprintf(stderr, "%cR (`%s`)\n", CHAR_8(iSrcReg + 'A'), oRegs[iSrcReg].GetString().c_str());
HL_RST;
#endif
										// Defined
										if (oRegs[iSrcReg].Defined()) { iFlags = FL_EQ; }
										// Undefined
										else                          { iFlags = FL_NE; }
									}
									// Illegal Opcode?
									else
									{
										UINT_32 iDataSize = 0;
										CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
										throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
									}
								}
								break;
							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					++iIP;
					break;

				// HLT
				case 0xFF: // Halt
					{
						switch(iOpCodeLo)
						{
							// HLT
							case SYSCALL_OPCODE_LO(HLT):
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X HLT\n", iIP);
HL_RST;
#endif
								return 0;
							// BRK
							case SYSCALL_OPCODE_LO(BRK):
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X BRK\n", iIP);
HL_RST;
#endif
								if (iDebugLevel > 0) { return 0; }
								++iIP;
								break;
							// NOP
							case SYSCALL_OPCODE_LO(NOP):
#ifdef _DEBUG
HL_CODE(RED);
fprintf(stderr, "0x%08X NOP\n", iIP);
HL_RST;
#endif
								++iIP; // Do nothing
								break;
							// Illegal Opcode?
							default:
							{
								UINT_32 iDataSize = 0;
								CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
								throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
							}
						}
					}
					break;

				// Illegal Opcode?
				default:
					{
						UINT_32 iDataSize = 0;
						CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);
						throw IllegalOpcode(iIP, iOpCode, aCode[iIP].reserved, szTMP);
					}
			} // switch(SYSCALL_OPCODE_HI(iOpCode))
			++iExecutedSteps;
		} // while (iIP < iCodeLength)
	}
	catch (StackOverflow  & e)
	{
		// Avoid MS VC warning "unised variable"; using paragma is not effective solution
		UINT_32 iTMP = e.GetIP() * 0;

		UINT_32 iDataSize = 0;
		CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);

		throw StackOverflow(iTMP + iIP, aCode[iIP].reserved, szTMP);
	}
	catch (StackUnderflow &e)
	{
		// Avoid MS VC warning "unised variable"; using paragma is not effective solution
		UINT_32 iTMP = e.GetIP() * 0;

		UINT_32 iDataSize = 0;
		CCHAR_P szTMP = pMemoryCore -> static_text.GetData(VMDebugInfo(aCode[iIP].reserved).GetDescrId(), iDataSize);

		throw StackUnderflow(iTMP + iIP, aCode[iIP].reserved, szTMP);
	}

return 0;
}

//
// Reset virtual machine state
//
INT_32 VM::Reset()
{
	AR = CDT();
	BR = CDT();
	CR = CDT();
	DR = CDT();
	ER = CDT();
	FR = CDT();
	GR = CDT();
	HR = CDT();

	iFlags = 0;

	oVMArgStack.Reset();
	oVMCodeStack.Reset();

return 0;
}

//
//
//
void VM::CheckStackOnlyRegs(const UINT_32 iSrcReg, const UINT_32 iDstReg, const VMMemoryCore  * pMemoryCore, const UINT_32 iIP)
{
	if (iSrcReg != ARG_SRC_STACK || iDstReg != ARG_DST_STACK)
	{
		UINT_32 iDataSize = 0;
		CCHAR_P szTMP     = pMemoryCore -> static_text.GetData(VMDebugInfo(pMemoryCore -> instructions[iIP].reserved).GetDescrId(), iDataSize);
		throw IllegalOpcode(iIP, pMemoryCore -> instructions[iIP].instruction, pMemoryCore -> instructions[iIP].reserved, szTMP);
	}
}

//
// A destructor
//
VM::~VM() throw()
{
	if (aCallTranslationMap != 0) { delete [] aCallTranslationMap; }

}

} // namespace CTPP
// End.

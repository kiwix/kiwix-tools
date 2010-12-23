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
 *      CTPP2VM.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_HPP__
#define _CTPP2_VM_HPP__ 1

/**
  @file CTPP2VM.hpp
  @brief Virtual Machine
*/

#include "CTPP2VMArgStack.hpp"
#include "CTPP2VMCodeStack.hpp"

namespace CTPP // C++ Template Engine
{

// FWD
class OutputCollector;
class SyscallFactory;
class SyscallHandler;
struct VMMemoryCore;
class Logger;

/**
  @class VM CTPP2VM.hpp <CTPP2VM.hpp>
  @brief Virtual Machine implementation
*/
class CTPP2DECL VM
{
public:
	/**
	  @brief Constructor
	  @param oISyscallFactory - factory with system calls
	  @param iIMaxArgStackSize - max. size of arguments stack
	  @param iIMaxCodeStackSize - max. size of code stack
	  @param iIMaxSteps - max. number of executed steps
	  @param iIDebugLevel - debugging level
	*/
	VM(SyscallFactory  * pSyscallFactory,
	   const UINT_32     iIMaxArgStackSize  = 4096,
	   const UINT_32     iIMaxCodeStackSize = 4096,
	   const UINT_32     iIMaxSteps         = 10240,
	   const UINT_32     iIDebugLevel       = 0);

	/**
	  @brief Initialize virtual machine
	*/
	INT_32 Init(const VMMemoryCore  * pMemoryCore,
	            OutputCollector     * pOutputCollector,
	            Logger              * pLogger);

	/**
	  @brief Run program
	  @param oMemoryCore - ready-to-run core of program
	  @param iIP - instruction pointer
	  @param oCDT - initial data
	  @return stack depth
	*/
	INT_32 Run(const VMMemoryCore  * pMemoryCore,
	           OutputCollector     * pOutputCollector,
	           UINT_32             & iIP,
	           CDT                 & oCDT,
	           Logger              * pLogger);

	/**
	  @brief Reset virtual machine state
	*/
	INT_32 Reset();

	/**
	  @brief A destructor
	*/
	~VM() throw();

private:
	/** System calls factory         */
	SyscallFactory   * pSyscallFactory;
	/** Maximal arguments stack size */
	const UINT_32      iMaxArgStackSize;
	/** Maximal code stack size      */
	const UINT_32      iMaxCodeStackSize;
	/** Maximum program execution
	                           steps */
	const UINT_32      iMaxSteps;
	/** Debug level                  */
	const UINT_32      iDebugLevel;

	/** Number of system calls       */
	UINT_32            iMaxCalls;
	/** Number of system calls       */
	UINT_32            iMaxUsedCalls;
	/** System calls translation map */
	SyscallHandler  ** aCallTranslationMap;

	/** Stack of arguments           */
	VMArgStack         oVMArgStack;
	/** Stack of code return points  */
	VMCodeStack        oVMCodeStack;

	/** Virtual machine registers    */
	CDT                oRegs[8];
	/** Virtual flags                */
	UINT_32            iFlags;

	void CheckStackOnlyRegs(const UINT_32 iSrcReg, const UINT_32 iDstReg, const VMMemoryCore  * pMemoryCore, const UINT_32 iIP);
};

} // namespace CTPP
#endif // _CTPP2_VM_HPP__
// End.

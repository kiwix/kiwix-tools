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
 *      CTPP2VMSyscall.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_SYSCALL_HPP__
#define _CTPP2_VM_SYSCALL_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2VMSyscall.hpp
  @brief Virtual machine system calls
*/

namespace CTPP // C++ Template Engine
{

#define CTPP2_INT_HANDLER_PREFIX "__ctpp2"

// FWD
class CDT;
class OutputCollector;
class ReducedStaticText;
class ReducedStaticData;
class Logger;

/**
  @class SyscallHandler CTPP2VMSyscall.hpp <CTPP2VMSyscall.hpp>
  @brief Base class of CTPP2 functions
*/
class CTPP2DECL SyscallHandler
{
public:
	/**
	  @brief Global Handler initialization
	  @param oCDT - initial data
	  @return 0 - if success, -1 - if any error occured
	*/
	virtual INT_32 InitHandler(CDT & oCDT);

	/**
	  @brief Pre-execution handler setup
	  @param oCollector - output data collector
	  @param oCDT - CTPP2 parameters
	  @param oSyscalls - Syscalls segment
	  @param oStaticData - Static data segment
	  @param oStaticText - Static text segment
	  @return 0 - if success, -1 - if any error occured
	*/
	virtual INT_32 PreExecuteSetup(OutputCollector          & oCollector,
	                               CDT                      & oCDT,
	                               const ReducedStaticText  & oSyscalls,
	                               const ReducedStaticData  & oStaticData,
	                               const ReducedStaticText  & oStaticText,
	                               Logger                   & oLogger);

	/**
	  @brief Handler
	  @param aArguments - list of arguments
	  @param iArgNum - number of arguments
	  @param oCDTRetVal - return value
	*/
	virtual INT_32 Handler(CDT            * aArguments,
	                       const UINT_32    iArgNum,
	                       CDT            & oCDTRetVal,
	                       Logger         & oLogger) = 0;

	/**
	  @brief Get function name
	*/
	virtual CCHAR_P GetName() const = 0;

	/**
	  @brief Get API version
	*/
	virtual INT_32 GetVersion() const;

	/**
	  @brief Handler resources destructor
	  @param oCDT - data, same of in InitHandler
	  @return 0 - if success, -1 - if any erro occured
	*/
	virtual INT_32 DestroyHandler(CDT & oCDT) throw();

	/**
	  @brief A destructor
	*/
	virtual ~SyscallHandler() throw();
};

} // namespace CTPP
#endif // _CTPP2_VM_SYSCALL_HPP__
// End.

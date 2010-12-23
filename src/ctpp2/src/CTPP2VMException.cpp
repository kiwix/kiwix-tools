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
 *      CTPP2VMException.cpp
 *
 * $CTPP$
 */
#include "CTPP2VMException.hpp"

#include <stdlib.h>
#include <string.h>

namespace CTPP // C++ Template Engine
{

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class VMException
//
//

//
// A virtual destructor
//
VMException::~VMException() throw() { ;; }


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class IllegalOpcode
//
//

//
// Constructor
//
IllegalOpcode::IllegalOpcode(const UINT_32  iIIP,
                             const UINT_32  iIOpcode,
                             const UINT_64  iIDebugInfo,
                             CCHAR_P        szISourceName): iIP(iIIP),
                                                            iOpcode(iIOpcode),
                                                            iDebugInfo(iIDebugInfo),
                                                            szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P IllegalOpcode::what() const throw() { return "Illegal opcode called"; }

//
// Get instruction pointer
//
UINT_32 IllegalOpcode::GetIP() const throw() { return iIP; }

//
// Get instruction code
//
UINT_32 IllegalOpcode::GetOpcode() const throw() { return iOpcode; }

//
// Get debug information
//
UINT_64 IllegalOpcode::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P IllegalOpcode::GetSourceName() const throw() { return szSourceName; }

//
// A destructor
//
IllegalOpcode::~IllegalOpcode() throw()
{
	free(szSourceName);
}


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class InvalidSyscall
//
//

//
// Constructor
//
InvalidSyscall::InvalidSyscall(CCHAR_P        szISyscall,
                               const UINT_32  iIIP,
                               const UINT_64  iIDebugInfo,
                               CCHAR_P        szISourceName): szSyscall(szISyscall),
                                                              iIP(iIIP),
                                                              iDebugInfo(iIDebugInfo),
                                                              szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P InvalidSyscall::what() const throw() { return szSyscall; }

//
// Get instruction pointer
//
UINT_32 InvalidSyscall::GetIP() const throw() { return iIP; }

//
// Get debug information
//
UINT_64 InvalidSyscall::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P InvalidSyscall::GetSourceName() const throw() { return szSourceName; }

//
// A destructor
//
InvalidSyscall::~InvalidSyscall() throw()
{
	free(szSourceName);
}


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CodeSegmentOverrun
//
//

//
// Constructor
//
CodeSegmentOverrun::CodeSegmentOverrun(const UINT_32  iIIP,
                                       const UINT_64  iIDebugInfo,
                                       CCHAR_P        szISourceName): iIP(iIIP),
                                                                      iDebugInfo(iIDebugInfo),
                                                                      szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P CodeSegmentOverrun::what() const throw() { return "Instruction pointer out of code segment"; }

//
// Get instruction pointer
//
UINT_32 CodeSegmentOverrun::GetIP() const throw() { return iIP; }

//
// Get debug information
//
UINT_64 CodeSegmentOverrun::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P CodeSegmentOverrun::GetSourceName() const throw() { return szSourceName; }

//
// A destructor
//
CodeSegmentOverrun::~CodeSegmentOverrun() throw()
{
	free(szSourceName);
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ExecutionLimitReached
//

//
// Constructor
//
ExecutionLimitReached::ExecutionLimitReached(const UINT_32  iIIP,
                                             const UINT_64  iIDebugInfo,
                                             CCHAR_P        szISourceName): iIP(iIIP),
                                                                            iDebugInfo(iIDebugInfo),
                                                                            szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P ExecutionLimitReached::what() const throw() { return "Execution limit reached"; }

//
// Get instruction pointer
//
UINT_32 ExecutionLimitReached::GetIP() const throw() { return iIP; }

//
// Get debug information
//
UINT_64 ExecutionLimitReached::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P ExecutionLimitReached::GetSourceName() const throw() { return szSourceName; }

//
// A destructor
//
ExecutionLimitReached::~ExecutionLimitReached() throw()
{
	free(szSourceName);
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ZeroDivision
//

//
// Constructor
//
ZeroDivision::ZeroDivision(const UINT_32  iIIP,
                           const UINT_64  iIDebugInfo,
                           CCHAR_P        szISourceName): iIP(iIIP),
                                                          iDebugInfo(iIDebugInfo),
                                                          szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P ZeroDivision::what() const throw() { return "Division by zero"; }

//
// Get instruction pointer
//
UINT_32 ZeroDivision::GetIP() const throw() { return iIP; }

//
// Get debug information
//
UINT_64 ZeroDivision::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P ZeroDivision::GetSourceName() const throw() { return szSourceName; }

//
// A virtual destructor
//
ZeroDivision::~ZeroDivision() throw()
{
	free(szSourceName);
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class InvalidCall
//

//
// Constructor
//
InvalidCall::InvalidCall(const UINT_32  iIIP,
                         const UINT_64  iIDebugInfo,
                         CCHAR_P        szICallName,
                         CCHAR_P        szISourceName): iIP(iIIP),
                                                        iDebugInfo(iIDebugInfo),
                                                        szCallName(strdup(szICallName)),
                                                        szSourceName(strdup(szISourceName)) { ;; }

//
// Get human-readable error description
//
CCHAR_P InvalidCall::what() const throw() { return szCallName; }

//
// Get instruction pointer
//
UINT_32 InvalidCall::GetIP() const throw() { return iIP; }

//
// Get debug information
//
UINT_64 InvalidCall::GetDebugInfo() const throw() { return iDebugInfo; }

//
// Get name of template
//
CCHAR_P InvalidCall::GetSourceName() const throw() { return szSourceName; }

//
// A destructor
//
InvalidCall::~InvalidCall() throw()
{
	free(szCallName);
	free(szSourceName);
}

} // namespace CTPP
// End.

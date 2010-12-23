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
 *      CTPP2VMStackException.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_STACK_EXCEPTION_HPP__
#define _CTPP2_VM_STACK_EXCEPTION_HPP__ 1

#include "CTPP2VMException.hpp"

/**
  @file VMStackException.hpp
  @brief Virtual machine stack exceptions
*/

namespace CTPP // C++ Template Engine
{

/**
  @class StackOverflow CTPP2VMStackException.hpp <CTPP2VMStackException.hpp>
  @brief Stack overflow error
*/
class StackOverflow:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	inline StackOverflow(const UINT_32    iIIP,
	                     const UINT_64    iIDebugInfo   = 0,
	                     CCHAR_P          szISourceName = NULL): iIP(iIIP),
	                                                             iDebugInfo(iIDebugInfo),
	                                                             szSourceName(NULL)
	{
		if (szISourceName != NULL) { szSourceName = strdup(szISourceName); }
	}


	/** @return A asciz string describing the general cause of error */
	inline CCHAR_P what() const throw() { return "Stack overflow"; }

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	inline UINT_32 GetIP() const throw() { return iIP; }

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	inline UINT_64 GetDebugInfo() const throw() { return iDebugInfo; }

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw() { return szSourceName; }

	/** @brief A destructor */
	inline ~StackOverflow() throw()
	{
		free(szSourceName);
	}
private:
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;
};

/**
  @class StackUnderflow CTPP2VMStackException.hpp <CTPP2VMStackException.hpp>
  @brief Stack underflow; throws when pop method called on empty stack
*/
class StackUnderflow:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	inline StackUnderflow(const UINT_32    iIIP,
	                      const UINT_64    iIDebugInfo   = 0,
	                      CCHAR_P          szISourceName = NULL): iIP(iIIP),
	                                                              iDebugInfo(iIDebugInfo),
	                                                              szSourceName(NULL)
	{
		if (szISourceName != NULL) { szSourceName = strdup(szISourceName); }
	}


	/** @return A asciz string describing the general cause of error */
	inline CCHAR_P what() const throw() { return "Stack underflow"; }

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	inline UINT_32 GetIP() const throw() { return iIP; }

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	inline UINT_64 GetDebugInfo() const throw() { return iDebugInfo; }

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw() { return szSourceName; }

	/** @brief A destructor */
	inline ~StackUnderflow() throw()
	{
		free(szSourceName);
	}

private:
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;
};

} // namespace CTPP
#endif // _CTPP2_VM_STACK_EXCEPTION_HPP__
// End.

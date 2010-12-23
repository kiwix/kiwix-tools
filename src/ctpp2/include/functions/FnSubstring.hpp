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
 *      FnSubstring.hpp
 *
 * $CTPP$
 */
#ifndef _VM_FN_SUBSTRING_HPP__
#define _VM_FN_SUBSTRING_HPP__ 1

#include "CTPP2VMSyscall.hpp"

/**
  @file FnSubstring.hpp
  @brief Virtual machine standard library function, get ubstring of given string and/or replacement of part of string
*/

namespace CTPP // C++ Template Engine
{

class CDT;
class Logger;

/**
  @class FnSubstring FnSubstring.hpp <FnSubstring.hpp>
  @brief Substring of given string and replacement of part of string
*/
class FnSubstring:
  public SyscallHandler
{
	/**
	  @brief A destructor
	*/
	~FnSubstring() throw();

private:
	friend class STDLibInitializer;


	/** Ouput data collector  */
	OutputCollector * pCollector;

	// ///////////////////////////////////////////////////////////////

	/**
	  @brief Constructor
	*/
	FnSubstring();

	/**
	  @brief Copy constructor
	  @param oRhs - object to copy
	*/
	FnSubstring(const FnSubstring & oRhs);

	/**
	  @brief Copy operator =
	  @param oRhs - object to copy
	*/
	FnSubstring & operator =(const FnSubstring & oRhs);

	/**
	  @brief Handler
	  @param aArguments - list of arguments
	  @param iArgNum - number of arguments
	  @param oCDTRetVal - return value
	*/
	INT_32 Handler(CDT            * aArguments,
	               const UINT_32    iArgNum,
	               CDT            & oCDTRetVal,
	               Logger         & oLogger);

	/**
	  @brief Get function name
	*/
	CCHAR_P GetName() const;
};

} // namespace CTPP
#endif // _VM_FN_SUBSTRING_HPP__
// End.

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
 *      FnConcat.hpp
 *
 * $CTPP$
 */
#ifndef _VM_FN_CONCAT_HPP__
#define _VM_FN_CONCAT_HPP__ 1

#include "CTPP2VMSyscall.hpp"

/**
  @file FnConcat.hpp
  @brief Virtual machine standard library function, concatenate strings
*/

namespace CTPP // C++ Template Engine
{

class CDT;
class Logger;

/**
  @class FnConcat FnConcat.hpp <FnConcat.hpp>
  @brief Concatenate two or more strings
*/
class FnConcat:
  public SyscallHandler
{
	/**
	  @brief A destructor
	*/
	~FnConcat() throw();

private:
	friend class STDLibInitializer;


	/** Ouput data collector  */
	OutputCollector * pCollector;

	// ///////////////////////////////////////////////////////////////

	/**
	  @brief Constructor
	*/
	FnConcat();

	/**
	  @brief Copy constructor
	  @param oRhs - object to copy
	*/
	FnConcat(const FnConcat & oRhs);

	/**
	  @brief Copy operator =
	  @param oRhs - object to copy
	*/
	FnConcat & operator =(const FnConcat & oRhs);

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
#endif // _VM_FN_CONCAT_HPP__
// End.

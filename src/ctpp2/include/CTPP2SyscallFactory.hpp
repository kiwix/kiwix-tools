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
 *      CTPP2SyscallFactory.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_SYSCALL_FACTORY_HPP__
#define _CTPP2_SYSCALL_FACTORY_HPP__ 1

#include "CTPP2VMSyscall.hpp"

#include "STLFunctional.hpp"
#include "STLMap.hpp"
#include "STLString.hpp"

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

/**
  @file CTPP2SyscallFactory.hpp
  @brief Virtual machine handlers factory
*/
namespace CTPP // C++ Template Engine
{

/**
  @class SyscallFactory CTPP2SyscallFactory.hpp <CTPP2SyscallFactory.hpp>
  @brief System calls abstract factory
*/
class CTPP2DECL SyscallFactory
{
public:
	/**
	  @brief Constructor
	  @param iIMaxHandlers - max. number of handlers
	*/
	SyscallFactory(const UINT_32  iIMaxHandlers);

	/**
	  @brief Get Handler by ID
	  @param iHandlerId - handler ID
	  @return pointer to handler or NULL if handler does not exist
	*/
	SyscallHandler * GetHandlerById(const UINT_32  iHandlerId) const;

	/**
	  @brief Get Handler by name
	  @param szHandlerName - handler name
	  @return pointer to handler or NULL if handler does not exist
	*/
	SyscallHandler * GetHandlerByName(CCHAR_P szHandlerName) const;

	/**
	  @brief Remove handler from factory
	  @param szHandlerName - handler name
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 RemoveHandler(CCHAR_P szHandlerName);

	/**
	  @brief Register handler
	  @param pHandler - handler to register in factory
	  @return Handler ID
	*/
	INT_32 RegisterHandler(SyscallHandler * pHandler);

	/**
	  @brief A destructor
	*/
	~SyscallFactory() throw();

private:
	/**
	  @struct HandlerRefsSort SyscallFactory.hpp <SyscallFactory.hpp>
	  @brief Case-insensitive sorting functor
	*/
	struct HandlerRefsSort:
	  public STLW::binary_function<STLW::string, STLW::string, bool>
	{
		/**
		  @brief comparison operator
		  @param x - first argument
		  @param y - first argument
		  @return true if x > y
		*/
		inline bool operator() (const STLW::string  & x,
		                        const STLW::string  & y) const
		{
			return (strcasecmp(x.c_str(), y.c_str()) > 0);
		}
	};

	/** Max. allowed number of handlers            */
	const UINT_32                      iMaxHandlers;
	/** Max. used handler ID                       */
	UINT_32                            iCurrHandlers;
	/** List of handlers                           */
	SyscallHandler                  ** aHandlers;
	/** Handler name-to-handler ID translation map */
	STLW::map<STLW::string, UINT_32, HandlerRefsSort>     mHandlerRefs;
};

} // namespace CTPP
#endif // _CTPP2_SYSCALL_FACTORY_HPP__
// End.

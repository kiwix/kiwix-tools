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
 *      CTPP2VMArgStack.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_ARG_STACK_HPP__
#define _CTPP2_VM_ARG_STACK_HPP__ 1

/**
  @file CTPP2VMArgStack.hpp
  @brief Virtual Machine stack of arguments
*/

#include "CDT.hpp"

namespace CTPP // C++ Template Engine
{

// FWD
class VM;

/**
  @class VMArgStack CTPP2VMCodeStack.hpp <CTPP2VMCodeStack.hpp>
  @brief Virtual Machine stack of arguments

<pre>
    +----+  <-- iMaxStackSize
    |void|
    |void|
    |----|  <-- iStackPointer
    |data|
    |data|
    | .. |
    |data|  <-- iBasePointer
    |data|
    | .. |
    |data|
    +----+  <-- 0
</pre>

*/
class CTPP2DECL VMArgStack
{
public:
	/**
	  @brief Constructor
	  @param iIMaxStackSize - maximal stack size
	*/
	VMArgStack(const INT_32  iIMaxStackSize = 4096);

	/**
	  @brief Get stack usage factor
	  @return stack depth
	*/
	INT_32 GetSize() const;

	/**
	  @brief Push element into stack
	  @param oCDT - element to push
	  @return stack depth
	*/
	INT_32 PushElement(const CDT & oCDT);

	/**
	  @brief Remove top stack element
	  @return stack depth
	*/
	INT_32 PopElement();

	/**
	  @brief Get element from specified position
	  @param iPos - element position
	  @return Stack element
	*/
	CDT & GetElement(const INT_32  iPos);

	/**
	  @brief Get top stack element
	  @return Stack element
	*/
	inline CDT & GetTopElement(const INT_32  iPos = 0) { return GetElement(iStackPointer + iPos); }

	/**
	  @brief Clear stack on specified depth
	  @param iDepth - number of elements to clear
	  @return stack depth
	*/
	INT_32 ClearStack(const INT_32  iDepth);

	/**
	  @brief Reset stack of arguments to default state
	*/
	void Reset();

	/**
	  @brief A destructor
	*/
	~VMArgStack() throw();

private:
	friend class VM;

	/**
	  @brief Copy Constructor
	*/
	VMArgStack(const VMArgStack & oRhs);

	/**
	  @brief Copy operator
	*/
	VMArgStack & operator=(const VMArgStack & oRhs);

	/** Maximal stack size */
	const INT_32    iMaxStackSize;
	/** Current stack size */
	INT_32          iStackPointer;
	/* * unused
	INT_32          iBasePointer; */

	/** Stack array        */
	CDT           * aStack;

	/**
	  @brief Get stack frame from top of stack
	  @param iTopOffset - offset of frame
	  @return pointer to start of frame
	*/
	inline CDT * GetStackFrame(const INT_32  iTopOffset = 0) { return &aStack[iStackPointer + iTopOffset]; }

};

} // namespace CTPP
#endif // _CTPP2_VM_ARG_STACK_HPP__
// End.

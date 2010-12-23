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
 *      CTPP2VMDebugInfo.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_DEBUG_INFO_HPP__
#define _CTPP2_VM_DEBUG_INFO_HPP__ 1

#include "CTPP2CharIterator.hpp"

/**
  @file CTPP2VMDebugInfo.hpp
  @brief Virtual machine debug information
*/

namespace CTPP // C++ Template Engine
{

/**
  @class VMDebugInfo CTPP2VMDebugInfo.hpp <CTPP2VMDebugInfo.hpp>
  @brief Virtual machine debug information
<pre>
   SSSSSSSS SSSSSSSS - String description
                     LLLLLLLL LLLLLLLL LLLLLLLL - Line
                                                PPPPPPPP PPPPPPPP PPPPPPPP - Position in line
   00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
</pre>
*/
class CTPP2DECL VMDebugInfo
{
public:

	/**
	  @brief Constructor
	  @param oCCharIterator - stream position
	  @param iIStringDescr - error description
	*/
	VMDebugInfo(const CCharIterator  & oCCharIterator,
	            const UINT_32          iIStringDescr = 0);

	/**
	  @brief Constructor
	  @param iIStringDescr - debug info
	  @param iILine - line in template
	  @param iIPos - line position
	*/
	VMDebugInfo(const UINT_32  iIStringDescr = 0,
	            const UINT_32  iILine        = 0,
	            const UINT_32  iIPos         = 0);

	/**
	  @brief Constructor
	  @param iEncoded - debug info
	*/
	VMDebugInfo(const UINT_64  iEncoded);

	/**
	  @brief Get encoded debug information
	  @return Encoded debug information
	*/
	UINT_64 GetInfo() const;

	/**
	  @brief Get string ID
	  @return Description ID
	*/
	UINT_32 GetDescrId() const;

	/**
	  @brief Get line
	  @return Line in template
	*/
	UINT_32 GetLine() const;

	/**
	  @brief Get line position
	  @return Line position
	*/
	UINT_32 GetLinePos() const;
private:
	/** Description ID       */
	UINT_32      iStringDescr;
	/** Line in template     */
	UINT_32      iLine;
	/** Line position        */
	UINT_32      iPos;
};

} // namespace CTPP
#endif // _CTPP2_VM_DEBUG_INFO_HPP__
// End.

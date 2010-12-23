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
 *      CTPP2StringBuffer.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_STRING_BUFFER_H__
#define _CTPP2_STRING_BUFFER_H__ 1

/**
  @file CTPP2StringBuffer.hpp
  @brief CTPP lazy string implementation
*/

#include "CTPP2Types.h"

#include "STLString.hpp"

#define C_STRING_BUFFER_LEN 1024

namespace CTPP // C++ Template Engine
{

/**
  @struct StringBuffer CTPP2StringBuffer.hpp <CTPP2StringBuffer.hpp>
  @brief CTPP lazy string implementation
*/
class StringBuffer
{
public:
	/**
	  @brief Constructor
	  @param sIResult - Destination string
	*/
	StringBuffer(STLW::string & sIResult);

	/**
	  @brief Append string
	  @param sStart - pointer to start of string
	  @param sEnd - pointer to end of string
	  @return Destination string size
	*/
	UINT_32 Append(CCHAR_P sStart, CCHAR_P sEnd);

	/**
	  @brief Append string
	  @param sStart - pointer to start of string
	  @param iLen - number of characters
	  @return Destination string size
	*/
	UINT_32 Append(CCHAR_P sStart, const UINT_32 iLen);

	/**
	  @brief Append multiple characters
	  @param iLen - number of characters
	  @param cChar - character to add
	  @return Destination string size
	*/
	UINT_32 Append(const UINT_32 iLen, const CHAR_8 cChar);

	/**
	  @brief Flush buffer
	  @return Destination string size
	*/
	UINT_32 Flush();

	/**
	  @brief A destructor
	*/
	~StringBuffer() throw();
private:
	// Does not exist
	StringBuffer(const StringBuffer & oRhs);
	StringBuffer & operator=(const StringBuffer & oRhs);

	/** Destination string */
	STLW::string   & sResult;
	/** Temp buffer        */
	CHAR_8           szBuffer[C_STRING_BUFFER_LEN + 1];
	/** Buffer position    */
	UINT_32          iBufferPos;
};

} // namespace CTPP
#endif // _CTPP2_STRING_BUFFER_H__
// End.

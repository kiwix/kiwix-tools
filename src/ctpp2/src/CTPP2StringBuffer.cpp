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
 *      CTPP2StringBuffer.cpp
 *
 * $CTPP$
 */
#include "CTPP2StringBuffer.hpp"

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
StringBuffer::StringBuffer(STLW::string & sIResult): sResult(sIResult), iBufferPos(0)
{
	;;
}

//
// Append string
//
UINT_32 StringBuffer::Append(CCHAR_P sStart, CCHAR_P sEnd)
{
	const UINT_32 iLen = sEnd - sStart;

return Append(sStart, iLen);
}

//
// Append string
//
UINT_32 StringBuffer::Append(const UINT_32 iLen, const CHAR_8 cChar)
{
	if (iLen > (C_STRING_BUFFER_LEN - iBufferPos))
	{
		Flush();
		sResult.append(iLen, cChar);
		return sResult.size();
	}

	for (UINT_32 iPos = 0; iPos < iLen; ++iPos)
	{
		szBuffer[iBufferPos++] = cChar;
	}

return sResult.size() + iLen;
}

//
// Append string
//
UINT_32 StringBuffer::Append(CCHAR_P sStart, const UINT_32 iLen)
{
	// Does not need to copy, only fhush buffer
	if (iLen > (C_STRING_BUFFER_LEN - iBufferPos))
	{
		Flush();
		sResult.append(sStart, iLen);
		return sResult.size();
	}

	for (UINT_32 iPos = 0; iPos < iLen; ++iPos)
	{
		szBuffer[iBufferPos++] = *sStart++;
	}

return sResult.size() + iBufferPos;
}

//
// Flush buffer
//
UINT_32 StringBuffer::Flush()
{
	sResult.append(szBuffer, iBufferPos);
	iBufferPos = 0;

return sResult.size();
}

//
// Destructor
//
StringBuffer::~StringBuffer() throw()
{
	try
	{
		Flush();
	}
	catch (...) { ;; }
}

} // namespace CTPP
// End.

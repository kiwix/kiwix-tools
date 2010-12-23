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
 *      CTPP2StringIconvOutputCollector.cpp
 *
 * $CTPP$
 */

#include "CTPP2StringIconvOutputCollector.hpp"
#include "CTPP2Exception.hpp"

#include <errno.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
StringIconvOutputCollector::StringIconvOutputCollector(STLW::string        & sIResult,
                                                       const STLW::string  & sISrcEnc,
                                                       const STLW::string  & sIDstEnc,
                                                       const UINT_32         iIFlags): sResult(sIResult),
                                                                                       sSrcEnc(sISrcEnc),
                                                                                       sDstEnc(sIDstEnc),
                                                                                       iFlags(iIFlags)
{
	oIconv = iconv_open(sDstEnc.c_str(), sSrcEnc.c_str());
	// Check error
	if (oIconv == (iconv_t)(-1))
	{
		throw CTPPCharsetRecodeException(sSrcEnc.c_str(), sDstEnc.c_str());
	}

#if (_LIBICONV_VERSION >= 0x0108)
	int iFlag = 1;
	// Discard illegal characters
	if (iFlags & C_ICONV_DISCARD_ILSEQ) { iconvctl(oIconv, ICONV_SET_DISCARD_ILSEQ, &iFlag); }

	// Ånable transliteration in the conver-sion
	if (iFlags & C_ICONV_TRANSLITERATE) { iconvctl(oIconv, ICONV_SET_TRANSLITERATE, &iFlag); }
#endif
}

//
// A destructor
//
StringIconvOutputCollector::~StringIconvOutputCollector() throw()
{
	iconv_close(oIconv);
}

//
// Collect data
//
INT_32 StringIconvOutputCollector::Collect(const void     * vData,
                                           const UINT_32    iDataLength)
{
	// Allocate memory
	size_t iSrcLength     = iDataLength;
	size_t iDstLength     = CTPP_ESCAPE_BUFFER_LEN;

	char         aDstData[CTPP_ESCAPE_BUFFER_LEN];
#if defined(linux) || defined(__APPLE__)
	char       * aSrcData = (char *)vData;
#else
	const char * aSrcData = (const char *)vData;
#endif

	for (;;)
	{
		char * aDstTMP        = aDstData;
		size_t iDstLengthTMP  = iDstLength;
		size_t iResult        = iconv(oIconv, &aSrcData, &iSrcLength, &aDstTMP, &iDstLengthTMP);

		if (aDstTMP - aDstData > 0) { sResult.append(aDstData, aDstTMP - aDstData); }

		// All data converted?
		if (iResult != (size_t)-1) { break; }
		else
		{
			if (errno != E2BIG)
			{
				++aSrcData;
				--iSrcLength;
			}
		}
	}

return 0;
}

} // namespace CTPP
// End.

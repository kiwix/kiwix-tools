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
 *      FnIconv.cpp
 *
 * $CTPP$
 */

#include "CDT.hpp"
#include "CTPP2Logger.hpp"
#include "FnIconv.hpp"

#include <errno.h>

namespace CTPP // C++ Template Engine
{
#ifdef ICONV_SUPPORT

#ifndef C_ICONV_DISCARD_ILSEQ
#define C_ICONV_DISCARD_ILSEQ    0x00000001
#endif

#ifndef C_ICONV_TRANSLITERATE
#define C_ICONV_TRANSLITERATE    0x00000002
#endif

//
// Constructor
//
FnIconv::FnIconv()
{
	;;
}

//
// Handler
//
INT_32 FnIconv::Handler(CDT            * aArguments,
                        const UINT_32    iArgNum,
                        CDT            & oCDTRetVal,
                        Logger         & oLogger)
{
	UINT_32 iMyArgNum  = iArgNum;

	// 3 or 4 arguments need
	if (iMyArgNum != 3 && iMyArgNum != 4)
	{
		oLogger.Emerg("Usage: ICONV(text, src_charset, dst_charset) or ICONV(text, src_charset, dst_charset, '[iItT]')");
		return -1;
	}

	// Arg 3: flags
	// Arg 2: destination charset
	// Arg 1: source charset
	// Arg 0: string to convert

	UINT_32 iFlags = 0;

#ifdef ICONV_DISCARD_ILSEQ
	iFlags |= C_ICONV_DISCARD_ILSEQ;
#endif

#ifdef ICONV_TRANSLITERATE
	iFlags |= C_ICONV_TRANSLITERATE;
#endif

	if (iMyArgNum == 4)
	{
		const STLW::string & sFlags = aArguments[0].GetString();
		for (UINT_32 iPos = 0; iPos < sFlags.size(); ++iPos)
		{
			switch (sFlags[iPos])
			{
				// Discard illegal sequence and continue
				case 'i':
				case 'I':
					iFlags |= C_ICONV_DISCARD_ILSEQ;
					break;

				// Enable transliteration
				case 't':
				case 'T':
					iFlags |= C_ICONV_TRANSLITERATE;
					break;

				default:
					oLogger.Error("Last argument should be 'i', 'I', 't' or 'T', but is `%s`", sFlags.c_str());
					return -1;
			}
		}
	}

	const STLW::string & sTo   = aArguments[--iMyArgNum].GetString();
	const STLW::string & sFrom = aArguments[--iMyArgNum].GetString();
	const STLW::string & sWhat = aArguments[--iMyArgNum].GetString();

	STLW::string sFromTo(sFrom);
	sFromTo.append(sTo);

	iconv_t oIconvConverter = (iconv_t)(-1);

	STLW::map<STLW::string, iconv_t>::iterator itmIconvMap = mIconvMap.find(sFromTo);
	if (itmIconvMap != mIconvMap.end()) { oIconvConverter = itmIconvMap -> second; }
	// Try to open iconv converter
	else
	{
		oIconvConverter = iconv_open(sFrom.c_str(), sTo.c_str());

		if (oIconvConverter != (iconv_t)(-1)) { mIconvMap[sFromTo] = oIconvConverter; }
		else
		{
			if (errno == EINVAL)
			{
				oLogger.Error("The conversion from `%s` to `%s` is not supported by the implementation", sFrom.c_str(), sTo.c_str());
			}
			else
			{
				oLogger.Error("Error(%d) in iconv_open('%s', '%s'): %s", sFrom.c_str(), sTo.c_str(), strerror(errno));
			}
			return -1;
		}
	}

#if (_LIBICONV_VERSION >= 0x0108)
	int iFlag = 1;
	// Discard illegal characters
	if (iFlags & C_ICONV_DISCARD_ILSEQ)
	{
		if (iconvctl(oIconvConverter, ICONV_SET_DISCARD_ILSEQ, &iFlag) == -1)
		{
			oLogger.Error("ICONV_SET_DISCARD_ILSEQ is is not supported by the implementation");
			return -1;
		}
	}

	// Ånable transliteration in the conver-sion
	if (iFlags & C_ICONV_TRANSLITERATE)
	{
		if (iconvctl(oIconvConverter, ICONV_SET_TRANSLITERATE, &iFlag) == -1)
		{
			oLogger.Error("ICONV_SET_TRANSLITERATE is is not supported by the implementation");
			return -1;
		}
	}
#endif

	// Allocate memory
	size_t iSrcLength     = sWhat.size();
	size_t iDstLength     = CTPP_ESCAPE_BUFFER_LEN;

	char         aDstData[CTPP_ESCAPE_BUFFER_LEN];
#if defined(linux) || defined(__APPLE__)
	char       * aSrcData = (char *)sWhat.data();
#else
	const char * aSrcData = (const char *)sWhat.data();
#endif
	STLW::string sResult;
	for (;;)
	{
		char * aDstTMP        = aDstData;
		size_t iDstLengthTMP  = iDstLength;
		size_t iResult        = iconv(oIconvConverter, &aSrcData, &iSrcLength, &aDstTMP, &iDstLengthTMP);

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

	oCDTRetVal = sResult;

return 0;
}

//
// Get function name
//
CCHAR_P FnIconv::GetName() const { return "iconv"; }

//
// A destructor
//
FnIconv::~FnIconv() throw()
{
	STLW::map<STLW::string, iconv_t>::iterator itmIconvMap = mIconvMap.begin();
	while (itmIconvMap != mIconvMap.end())
	{
		iconv_close(itmIconvMap -> second);
		++itmIconvMap;
	}
}

#endif // ICONV_SUPPORT
} // namespace CTPP
// End.

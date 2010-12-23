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
 *      CTPP2StringIconvOutputCollector.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_STRING_ICONV_OUTPUT_COLLECTOR_HPP__
#define _CTPP2_STRING_ICONV_OUTPUT_COLLECTOR_HPP__ 1

#include "CTPP2OutputCollector.hpp"
#include "STLString.hpp"

#ifdef ICONV_SUPPORT
    #include <iconv.h>
#endif // ICONV_SUPPORT

/**
  @file CTPP2StringIconvOutputCollector.hpp
  @brief Virtual machine output data collector with charset recoding
*/

namespace CTPP // C++ Template Engine
{

#ifdef ICONV_SUPPORT

#define C_ICONV_DISCARD_ILSEQ    0x00000001
#define C_ICONV_TRANSLITERATE    0x00000002

/**
  @class StringIconvOutputCollector StringIconvOutputCollector.hpp <StringIconvOutputCollector.hpp>
  @brief Output data collector with charset recoding
*/
class StringIconvOutputCollector:
  public OutputCollector
{
public:
	/**
	  @brief Constructor
	  @param sIResult - Result output string
	  @param sISrcEnc - Source charset
	  @param sIDstEnc - Destination charset
	  @param iIFlags - conversion flags
	*/
	StringIconvOutputCollector(STLW::string        & sIResult,
	                           const STLW::string  & sISrcEnc,
	                           const STLW::string  & sIDstEnc,
	                           const UINT_32         iIFlags = 0);

	/**
	  @brief A destructor
	*/
	~StringIconvOutputCollector() throw();
private:
	/** Data object          */
	STLW::string           & sResult;
	/** Source charset       */
	const STLW::string       sSrcEnc;
	/** Destination charset  */
	const STLW::string       sDstEnc;
	/** Conversion flags     */
	UINT_32                  iFlags;
	/** Iconv converter      */
	iconv_t                  oIconv;

	/**
	  @brief Collect data
	  @param vData - data to store
	  @param iDataLength - data length
	  @return 0 - if success, -1 - if any error occured
	*/
	INT_32 Collect(const void     * vData,
	               const UINT_32    iDataLength);
};
#endif // ICONV_SUPPORT

} // namespace CTPP
#endif // _CTPP2_STRING_ICONV_OUTPUT_COLLECTOR_HPP__
// End.

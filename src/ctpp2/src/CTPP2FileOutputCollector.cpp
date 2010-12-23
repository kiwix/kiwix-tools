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
 *      CTPP2FileOutputCollector.hpp
 *
 * $CTPP$
 */

#include "CTPP2FileOutputCollector.hpp"

#include "CTPP2Exception.hpp"

#include <errno.h>

/**
  @file CTPP2FileOutputCollector.hpp
  @brief Virtual machine file output data collector
*/

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
FileOutputCollector::FileOutputCollector(FILE * oIF): oF(oIF), iSelfOpened(0)
{
	;;
}

//
// Constructor
//
FileOutputCollector::FileOutputCollector(CCHAR_P szFileName, CCHAR_P szMode): iSelfOpened(1)
{
	oF = fopen(szFileName, szMode);
	if (oF == NULL) { throw CTPPUnixException("fopen", errno); }
}

//
// Get unix file number
//
INT_32 FileOutputCollector::GetFileNo() const { return fileno(oF); }

//
// A destructor
//
FileOutputCollector::~FileOutputCollector() throw()
{
	if (iSelfOpened == 1) { fclose(oF); }
}

//
// Collect data
//
INT_32 FileOutputCollector::Collect(const void * vData, const UINT_32  iDataLength)
{
	if (fwrite(vData, iDataLength, 1, oF) != 1) { return -1; }

return 0;
}

} // namespace CTPP
// End.

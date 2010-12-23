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
 *      CTPP2JSONFileParser.cpp
 *
 * $CTPP$
 */

#include "CTPP2JSONFileParser.hpp"
#include "CTPP2Exception.hpp"

#include <sys/stat.h>

#include <errno.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
CTPP2JSONFileParser::CTPP2JSONFileParser(CDT & oICDT): CTPP2JSONParser(oICDT) { ;; }


//
// Parse JSON object
//
INT_32 CTPP2JSONFileParser::Parse(CCHAR_P szFileName)
{
	// Get file size
	struct stat oStat;
	if (stat(szFileName, &oStat) == -1) { throw CTPPUnixException("stat", errno); }
	if (oStat.st_size == 0) { throw CTPPLogicError("Cannot get size of file"); }

	// Load file
	FILE * F = fopen(szFileName, "rb");
	if (F == NULL) { throw CTPPUnixException("fopen", errno); }

	// Allocate memory
	CHAR_P oFile = (CHAR_P)malloc(oStat.st_size);

	// Read from file
	if (fread(oFile, oStat.st_size, 1, F) != 1)
	{
		if (ferror(F) != 0)
		{
			free(oFile);
			fclose(F);
			throw CTPPUnixException("fread", errno);
		}
		else
		{
			free(oFile);
			fclose(F);
			throw CTPPLogicError("Cannot read from file");
		}
	}

	try
	{
		CTPP2JSONParser::Parse(oFile, oFile + oStat.st_size);
	}
	catch(...)
	{
		free(oFile);
		fclose(F);
		throw;
	}

	// All Done
	free(oFile);
	fclose(F);

return 0;
}

//
// Parse JSON object
//
INT_32 CTPP2JSONFileParser::Parse(FILE * F, const UINT_32 & iReadBytes)
{
	// Load file
	if (F == NULL) { throw CTPPUnixException("fopen", errno); }

	// Allocate memory
	CHAR_P oFile = (CHAR_P)malloc(iReadBytes);

	// Read from file
	if (fread(oFile, iReadBytes, 1, F) != 1)
	{
		if (ferror(F) != 0)
		{
			free(oFile);
			fclose(F);
			throw CTPPUnixException("fread", errno);
		}
		else
		{
			free(oFile);
			fclose(F);
			throw CTPPLogicError("Cannot read from file");
		}
	}

	try
	{
		CTPP2JSONParser::Parse(oFile, oFile + iReadBytes);
	}
	catch(...)
	{
		free(oFile);
		fclose(F);
		throw;
	}

	// All Done
	free(oFile);
	fclose(F);
return 0;
}

//
// A destructor
//
CTPP2JSONFileParser::~CTPP2JSONFileParser() throw()
{
	;;
}

} // namespace CTPP
// End.

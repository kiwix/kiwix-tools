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
 *      CTPP2FileSourceLoader.cpp
 *
 * $CTPP$
 */

#include "CTPP2FileSourceLoader.hpp"

#include "CTPP2Exception.hpp"

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
    #include <direct.h> /* getcwd */
    #include <windows.h>
#endif

/**
  @file FileOutputCollector.hpp
  @brief Virtual machine file output data collector
*/

namespace CTPP // C++ Template Engine
{

//
// Get base directory and normalise file path
//

#ifndef WIN32

static STLW::string GetBaseDir(const STLW::string & szTemplateName, STLW::string & sNormalizedFileName)
{
	if (szTemplateName.length() == 0) { return ""; }

	STLW::vector<STLW::string> vCurrentDir;

	CCHAR_P sBegin = szTemplateName.c_str();
	CCHAR_P szEnd  = szTemplateName.c_str() + szTemplateName.length();

	CCHAR_P sIter = sBegin;
	while (sIter != szEnd)
	{
		if (*sIter == '/')
		{
			if (sIter != sBegin)
			{
				STLW::string sTMP(sBegin, sIter);

				if      (sTMP == "/." || sTMP == "/") { ;; }
				else if (sTMP == "/..")
				{
					STLW::vector<STLW::string>::iterator itEnd = vCurrentDir.end();
					if (vCurrentDir.begin() == itEnd) { return ""; }
					vCurrentDir.erase(--itEnd);
				}
				else
				{
					vCurrentDir.push_back(sTMP);
				}
			}
			sBegin = sIter;
		}
		++sIter;
	}

	STLW::string sTMP(sBegin, sIter);
	if (sTMP == "/") { return ""; }

	STLW::string sResult;
	for (UINT_32 iI = 0; iI < vCurrentDir.size(); ++iI) { sResult.append(vCurrentDir[iI]); }

	sNormalizedFileName.assign(sResult);
	sNormalizedFileName.append(sTMP);

	sResult.append("/");

return sResult;
}

#else

static STLW::string GetBaseDir(const STLW::string & szTemplateName, STLW::string & sNormalizedFileName)
{
	STLW::string sResult;
	CHAR_8      szPath[MAX_PATH];
	CHAR_P      szFile = NULL;
	DWORD       dwLen  = ::GetFullPathNameA(szTemplateName.c_str(), MAX_PATH, szPath, &szFile);

	if (szFile == NULL) { return ""; }

	sNormalizedFileName.assign(szPath, dwLen);
	sResult.assign(szPath, szFile);

return sResult;
}

#endif


//
// Clone loader
//
CTPP2SourceLoader * CTPP2FileSourceLoader::Clone()
{
	CTPP2FileSourceLoader * pLoader = new CTPP2FileSourceLoader;

	STLW::vector<STLW::string> vTMP(vIncludeDirs);

	vTMP.push_back(sCurrentDir);

	pLoader -> SetIncludeDirs(vTMP);

return pLoader;
}

//
// Constructor
//
CTPP2FileSourceLoader::CTPP2FileSourceLoader(): sTemplate(NULL), iTemplateSize(0)
{
	// Always search in current directory
	vIncludeDirs.push_back("");
}

//
// Set of directories where included templates should be found
//
void CTPP2FileSourceLoader::SetIncludeDirs(const STLW::vector<STLW::string> & vIIncludeDirs)
{
	{ STLW::vector<STLW::string> vTMP; vIncludeDirs.swap(vTMP); }

	STLW::vector<STLW::string>::const_iterator itvIIncludeDirs = vIIncludeDirs.begin();
	while (itvIIncludeDirs != vIIncludeDirs.end())
	{
		if (itvIIncludeDirs -> length() != 0) { vIncludeDirs.push_back(*itvIIncludeDirs); }
		++itvIIncludeDirs;
	}
	vIncludeDirs.push_back("");
}

//
// Load template with specified name
//
INT_32 CTPP2FileSourceLoader::LoadTemplate(CCHAR_P szTemplateName)
{
	sNormalizedFileName.erase();
	INT_32 iStatCode = 0;
	STLW::vector<STLW::string>::const_iterator itvIncludeDirs = vIncludeDirs.begin();
	while(itvIncludeDirs != vIncludeDirs.end())
	{
		STLW::string sTMP = *itvIncludeDirs;
#ifdef WIN32
		if ( sTMP.length() && sTMP[sTMP.length() - 1] != '/' && sTMP[sTMP.length() - 1] != '\\' ) { sTMP.append("\\", 1); }
#else
		if (sTMP.length() && sTMP[sTMP.length() - 1] != '/') { sTMP.append("/", 1); }
#endif
		sTMP.append(szTemplateName);

		sCurrentDir = GetBaseDir(sTMP, sNormalizedFileName);
		if (sNormalizedFileName.length() == 0)
		{
			STLW::string sError("invalid file name `");
			sError.append(sTMP);
			sError.append("`");
			throw CTPPLogicError(sError.c_str());
		}

		// Get file size
		struct stat oStat;
		iStatCode = stat(sNormalizedFileName.c_str(), &oStat);
		if (iStatCode == 0)
		{
			iTemplateSize = oStat.st_size;
			break;
		}
		++itvIncludeDirs;
	}

	if (iStatCode == -1)
	{
		STLW::string sError("cannot find file in include directories ");
		itvIncludeDirs = vIncludeDirs.begin();
		for (;;)
		{
			sError.append("`");
			if (itvIncludeDirs -> size() != 0) { sError.append(*itvIncludeDirs); }
			else
			{
				CHAR_P szPWD = getcwd(NULL, 0);
				sError.append(szPWD);
				free(szPWD);
			}
			sError.append("`");

			++itvIncludeDirs;
			if (itvIncludeDirs == vIncludeDirs.end()) { break; }

			sError.append(", ");
		}
		throw CTPPLogicError(sError.c_str());
	}

	if (iTemplateSize == 0)
	{
		STLW::string sError("empty file `");
		sError.append(sNormalizedFileName);
		sError.append("` found");
		throw CTPPLogicError(sError.c_str());
	}

	// Load file
	FILE * F = fopen(sNormalizedFileName.c_str(), "rb");
	if (F == NULL) { throw CTPPUnixException("fopen", errno); }

	if (sTemplate != NULL) { free(sTemplate); }

	// Allocate memory
	sTemplate = (CHAR_P)malloc(iTemplateSize);

	// Read from file
	if (fread(sTemplate, iTemplateSize, 1, F) != 1)
	{
		if (ferror(F) != 0)
		{
			free(sTemplate);
			fclose(F);
			throw CTPPUnixException("fread", errno);
		}
		else
		{
			free(sTemplate);
			fclose(F);
			throw CTPPLogicError("Cannot read from file");
		}
	}

	fclose(F);

return 0;
}

//
// Get template file name
//
CCHAR_P CTPP2FileSourceLoader::GetTemplateName() const { return sNormalizedFileName.c_str(); }

//
// Get template
//
CCHAR_P CTPP2FileSourceLoader::GetTemplate(UINT_32 & iITemplateSize)
{
	iITemplateSize = iTemplateSize;
	return sTemplate;
}

//
// A destructor
//
CTPP2FileSourceLoader::~CTPP2FileSourceLoader() throw()
{
	if (sTemplate != NULL) { free(sTemplate); }
}

} // namespace CTPP
// End.

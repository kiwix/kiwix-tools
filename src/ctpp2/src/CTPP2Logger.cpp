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
 *      CTPP2Logger.cpp
 *
 * $CAS$
 */
#include "CTPP2Logger.hpp"

#include <stdio.h>
#include <stdlib.h>

namespace CTPP // C++ Template Engine
{

//
// Constructor
//
Logger::Logger(const UINT_32  iIBasePriority): iBasePriority(iIBasePriority) { ;; }

//
// Change base priority
//
void Logger::SetPriority(const UINT_32  iNewPriority) { iBasePriority = iNewPriority; }

//
// Log message with formatted parameters
//
INT_32 Logger::LogMessage(const UINT_32  iPriority, CCHAR_P szFormat, ...)
{
	va_list aArgList;
	va_start(aArgList, szFormat);
	INT_32 iRC = LogMessage(iPriority, szFormat, aArgList);
	va_end(aArgList);

return iRC;
}

//
// Log message with formatted parameters
//
INT_32 Logger::LogMessage(const UINT_32  iPriority,
                          CCHAR_P        szFormat,
                          va_list        aArgList)
{
	// Message not logged, incorrect priority level
	if (iPriority > iBasePriority) { return -1; }

	{
		CHAR_8 szStaticBuffer[C_START_MESSAGE_LEN];

		// Format value
		const INT_32 iFmtLen = vsnprintf(szStaticBuffer, C_START_MESSAGE_LEN, szFormat, aArgList);
		if (iFmtLen < C_START_MESSAGE_LEN)
		{
			// Write to logger
			WriteLog(iPriority, szStaticBuffer, iFmtLen);
			return 0;
		}
	}

	INT_32 iResultLen = C_START_MESSAGE_LEN;
	CHAR_P szResult   = NULL;
	for(;;)
	{
		// TODO: Improve performance
		szResult = (CHAR_P)malloc(iResultLen);
		// Format value
		const INT_32 iFmtLen = vsnprintf(szResult, iResultLen, szFormat, aArgList);
		if (iFmtLen < iResultLen)
		{
			// Write to logger
			WriteLog(iPriority, szResult, iFmtLen);
			free(szResult);
			return 0;
		}

		iResultLen <<= 1;
		free(szResult);
	}
}

//
//log message with formatted parameters and CTPP2_LOG_EMERG priority
//
INT_32 Logger::Emerg(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_EMERG, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_ALERT priority
//
INT_32 Logger::Alert(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_ALERT, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_CRIT priority
//
INT_32 Logger::Crit(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_CRIT, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_ERR priority
//
INT_32 Logger::Err(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_ERR, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_ERR priority
//
INT_32 Logger::Error(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_ERR, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_WARNING priority
//
INT_32 Logger::Warn(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_WARNING, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_WARNING priority
//
INT_32 Logger::Warning(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_WARNING, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_NOTICE priority
//
INT_32 Logger::Notice(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_NOTICE, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_INFO priority
//
INT_32 Logger::Info(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_INFO, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and CTPP2_LOG_DEBUG priority
//
INT_32 Logger::Debug(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(CTPP2_LOG_DEBUG, szFormat, aTMP); va_end(aTMP); return iRC; }

//
// A destructor
//
Logger::~Logger() throw()
{
	;;
}

} // namespace CTPP
// End.

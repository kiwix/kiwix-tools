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
 *      CTPP2Logger.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_LOGGER_HPP__
#define _CTPP2_LOGGER_HPP__ 1

#include "CTPP2Types.h"

#include <stdarg.h>

/**
  @file CTPP2Logger.hpp
  @brief CTPP2 Logger
*/

#define CTPP2_LOG_EMERG       0       /* system is unusable               */
#define CTPP2_LOG_ALERT       1       /* action must be taken immediately */
#define CTPP2_LOG_CRIT        2       /* critical conditions              */
#define CTPP2_LOG_ERR         3       /* error conditions                 */
#define CTPP2_LOG_WARNING     4       /* warning conditions               */
#define CTPP2_LOG_NOTICE      5       /* normal but significant condition */
#define CTPP2_LOG_INFO        6       /* informational                    */
#define CTPP2_LOG_DEBUG       7       /* debug-level messages             */
#define CTPP2_LOG_PRIMASK     0x07    /* mask to extract priority part
                                                               (internal) */

#define CTPP2_LOG_WARN        CTPP2_LOG_WARNING /* alias for CTPP2_LOG_WARNING */
#define CTPP2_LOG_ERROR       CTPP2_LOG_ERR     /* alias for CTPP2_LOG_ERR     */

#define C_START_MESSAGE_LEN   2048

namespace CTPP // C++ Template Engine
{

/**
  @class Logger CTPP2Logger.hpp <CTPP2Logger.hpp>
  @brief CTPP2 logger subsystem
*/
class Logger
{
protected:
	/** Base priority   */
	UINT_32     iBasePriority;

public:
	/**
	  @brief Constructor
	  @param iIBasePriority - base priority
	*/
	Logger(const UINT_32  iIBasePriority = CTPP2_LOG_WARNING);

	/**
	  @brief Change base priority
	  @param iNewPriority - new base priority
	*/
	void SetPriority(const UINT_32  iNewPriority);

	/**
	  @brief Write message to log file
	  @param iPriority - priority level
	  @param szString - message to store in file
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 WriteLog(const UINT_32  iPriority,
	                        CCHAR_P        szString,
	                        const UINT_32  iStringLen) = 0;

	/**
	  @brief log message with formatted parameters
	  @param iPriority - priority level
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 LogMessage(const UINT_32  iPriority,
	                          CCHAR_P        szFormat, ...);

	/**
	  @brief log message with formatted parameters
	  @param iPriority - priority level
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 LogMessage(const UINT_32  iPriority,
	                          CCHAR_P        szFormat,
	                          va_list        aArgList);

	/**
	  @brief log message with formatted parameters and AS_LOG_EMERG priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Emerg(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ALERT priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Alert(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_CRIT priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Crit(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ERR priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Err(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ERR priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Error(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_WARNING priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Warn(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_WARNING priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Warning(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_NOTICE priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Notice(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_INFO priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Info(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_DEBUG priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Debug(CCHAR_P szFormat, ...);

	/**
	  @brief A destructor
	*/
	virtual ~Logger() throw();
};

} // namespace CTPP
#endif // _CTPP2_LOGGER_HPP__
// End.

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
 *      CDT.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_EXCEPTION_HPP__
#define _CTPP2_EXCEPTION_HPP__ 1

#include "CTPP2Types.h"
#ifdef _USE_STL_AS_BASE_EXCEPTION
    #include "STLException.hpp"
#endif

#include <string.h>
#include <stdlib.h>

namespace CTPP // C++ Template Engine
{

/**
  @class CTPP2Exception CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Base class for all CTPP exceptions.
*/
class CTPP2DECL CTPPException
#ifdef _USE_STL_AS_BASE_EXCEPTION
:
  public STLW::exception
#endif // _USE_STL_AS_BASE_EXCEPTION
{
public:
	/** @return A asciz string describing the general cause of error */
	virtual CCHAR_P what() const throw();
	/** @brief A virtual destructor */
	virtual ~CTPPException() throw();
};

/**
  @class CTPPLogicError CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Logic error
*/
class CTPP2DECL CTPPLogicError:
  public CTPPException
{
public:
	/**
	  @brief Constrcutor
	  @param sIReason - Error description
	*/
	CTPPLogicError(CCHAR_P sIReason) throw();

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** A destructor */
	~CTPPLogicError() throw();
private:
	/** Error description */
	CHAR_P sReason;
};

/**
  @class CDTTypeCastException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Invalid type cast
*/
class CTPP2DECL CDTTypeCastException:
  public CTPPException
{
public:
	/**
	  @brief Constrcutor
	  @param sIReason - Error description
	*/
	CDTTypeCastException(CCHAR_P sIReason) throw();

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** A destructor */
	~CDTTypeCastException() throw();
private:
	/** Error description */
	CHAR_P sReason;
};

/**
  @class CDTAccessException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Access violation exception
*/
class CTPP2DECL CDTAccessException:
  public CTPPException
{
public:

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** A destructor */
	~CDTAccessException() throw();
private:

};

/**
  @class CDTRangeException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Array index is out of bounds
*/
class CTPP2DECL CDTRangeException:
  public CTPPException
{
public:

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** A destructor */
	~CDTRangeException() throw();
private:

};

/**
  @class CTPPUnixException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Unix error code
*/
class CTPP2DECL CTPPUnixException:
  public CTPPException
{
public:
	/**
	  @brief Constrcutor
	  @param sIReason - Error description
	  @param iIErrno - Unix error code
	*/
	CTPPUnixException(CCHAR_P sIReason, INT_32 iIErrno) throw();

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** @return error code */
	INT_32 ErrNo() const throw();

	/** A destructor */
	~CTPPUnixException() throw();
private:
	/** Error description */
	CHAR_P sReason;
	/** Error code        */
	INT_32 iErrno;
};

/**
  @class CTPPNullPointerException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Pointer is NULL
*/
class CTPP2DECL CTPPNullPointerException:
  public CTPPException
{
public:
	/**
	  @brief Constrcutor
	  @param sIReason - Error description
	*/
	CTPPNullPointerException(CCHAR_P sIReason) throw();

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** A destructor */
	~CTPPNullPointerException() throw();
private:
	/** Error description */
	CHAR_P sReason;
};

/**
  @class CTPPCharsetRecodeException CTPP2Exception.hpp <CTPP2Exception.hpp>
  @brief Cannot convert from source to destination charset
*/
class CTPP2DECL CTPPCharsetRecodeException:
  public CTPPException
{
public:
	/**
	  @brief Constrcutor
	  @param sISrc - Source encoding
	  @param sIDst - Destination encoding
	*/
	CTPPCharsetRecodeException(CCHAR_P  sISrc,
	                           CCHAR_P  sIDst) throw();

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/** @return Get source charset */
	CCHAR_P GetSource() const throw();

	/** @return Get destination charset */
	CCHAR_P GetDestination() const throw();

	/** A destructor */
	~CTPPCharsetRecodeException() throw();

private:
	/** Source charset    */
	CHAR_P sSrc;
	/** Destinationset    */
	CHAR_P sDst;
};

} // namespace CTPP
#endif // _CTPP2_EXCEPTION_HPP__
// End.

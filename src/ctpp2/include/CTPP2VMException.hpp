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
 *      CTPP2VMException.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_EXCEPTION_HPP__
#define _CTPP2_VM_EXCEPTION_HPP__ 1

#include "CTPP2Types.h"
#include "CTPP2Exception.hpp"

/**
  @file CTPP2VMException.hpp
  @brief Virtual machine exception
*/

namespace CTPP // C++ Template Engine
{

/**
  @class VMException CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Base class for all virtual machine exceptions.
*/
class CTPP2DECL VMException:
  public CTPPException
{
public:
	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	virtual UINT_32 GetIP() const throw() = 0;

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	virtual UINT_64 GetDebugInfo() const throw() = 0;

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw() = 0;

	/** @brief A virtual destructor */
	~VMException() throw();

private:
	// Nothing
};

/**
  @class IllegalOpcode CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Illegal operation code
*/
class CTPP2DECL IllegalOpcode:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIOpcode - illegal opcode
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	IllegalOpcode(const UINT_32  iIIP,
	              const UINT_32  iIOpcode,
	              const UINT_64  iIDebugInfo,
	              CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get instruction code
	  @return Instruction pointer
	*/
	UINT_32 GetOpcode() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();


	/** @brief A virtual destructor */
	~IllegalOpcode() throw();

private:
	/** Instruction pointer  */
	const UINT_32   iIP;
	/** Instruction pointer  */
	const UINT_32   iOpcode;
	/** Debug information    */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;
};

/**
  @class InvalidSyscall CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Non-existent system call
*/
class CTPP2DECL InvalidSyscall:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param szISyscall - syscall name
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	InvalidSyscall(CCHAR_P        szISyscall,
	               const UINT_32  iIIP,
	               const UINT_64  iIDebugInfo,
	               CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();

	/** @brief A virtual destructor */
	~InvalidSyscall() throw();

private:
	/** System call         */
	CCHAR_P         szSyscall;
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;

};

/**
  @class CodeSegmentOverrun CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Overrun from code segment
*/
class CTPP2DECL CodeSegmentOverrun:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	CodeSegmentOverrun(const UINT_32  iIIP,
	                   const UINT_64  iIDebugInfo,
	                   CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();

	/** @brief A virtual destructor */
	~CodeSegmentOverrun() throw();

private:
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;

};

/**
  @class ExecutionLimitReached CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Execution limit reached (in steps)
*/
class CTPP2DECL ExecutionLimitReached:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	ExecutionLimitReached(const UINT_32  iIIP,
	                      const UINT_64  iIDebugInfo,
	                      CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();

	/** @brief A virtual destructor */
	~ExecutionLimitReached() throw();

private:
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;

};

/**
  @class ZeroDivision CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Execution limit reached (in steps)
*/
class CTPP2DECL ZeroDivision:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szISourceName - source file name
	*/
	ZeroDivision(const UINT_32  iIIP,
	             const UINT_64  iIDebugInfo,
	             CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();

	/** @brief A virtual destructor */
	~ZeroDivision() throw();

private:
	/** Instruction pointer */
	const UINT_32   iIP;
	/** Debug information   */
	const UINT_64   iDebugInfo;
	/** Template source name */
	CHAR_P          szSourceName;

};

/**
  @class InvalidCall CTPP2VMException.hpp <CTPP2VMException.hpp>
  @brief Invalid call name
*/
class CTPP2DECL InvalidCall:
  public VMException
{
public:
	/**
	  @brief Constructor
	  @param iIIP - instruction pointer
	  @param iIDebugInfo - debug information
	  @param szICallName - call name
	  @param szISourceName - source file name
	*/
	InvalidCall(const UINT_32  iIIP,
	            const UINT_64  iIDebugInfo,
	            CCHAR_P        szICallName,
	            CCHAR_P        szISourceName);

	/** @return A asciz string describing the general cause of error */
	CCHAR_P what() const throw();

	/**
	  @brief Get instruction pointer
	  @return Instruction pointer
	*/
	UINT_32 GetIP() const throw();

	/**
	  @brief Get debug information
	  @return Packed VMDebugInfo object
	*/
	UINT_64 GetDebugInfo() const throw();

	/**
	  @brief Get name of template
	  @return Template name, asciz string
	*/
	virtual CCHAR_P GetSourceName() const throw();

	/** @brief A virtual destructor */
	~InvalidCall() throw();

private:
	/** Instruction pointer  */
	const UINT_32   iIP;
	/** Debug information    */
	const UINT_64   iDebugInfo;
	/** Call name            */
	CHAR_P          szCallName;
	/** Template source name */
	CHAR_P          szSourceName;
};

} // namespace CTPP
#endif // _VM_EXCEPTION_HPP__
// End.

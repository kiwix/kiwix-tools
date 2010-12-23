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
 *      CTPP2VMDumper.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_DUMPER_HPP__
#define _CTPP2_VM_DUMPER_HPP__ 1

#include "CTPP2VMMemoryCore.hpp"
#include "CTPP2VMExecutable.hpp"

/**
  @file VMDumper.hpp
  @brief Executable program core constructor
*/

namespace CTPP // C++ Template Engine
{

/**
  @class VMDumper VMDumper.hpp <VMDumper.hpp>
  @brief Construct executable program core from source
*/
class CTPP2DECL VMDumper
{
public:
	/**
	  @brief Constructor
	  @param oMemoryCore - ready-to-run memory core
	*/
	VMDumper(const VMMemoryCore & oMemoryCore);

	/**
	  @brief Constructor
	  @param iCodeSize - Code segment size
	  @param aInstructions - Code segment
	  @param oSyscalls - Syscalls segment
	  @param oStaticData - Static data segment
	  @param oStaticText - Static text segment
	  @param oHashTable - Hash table for "blocks"
	*/
	VMDumper(const UINT_32          iCodeSize,
	         const VMInstruction  * aInstructions,
	         const StaticText     & oSyscalls,
	         const StaticData     & oStaticData,
	         const StaticText     & oStaticText,
	         const HashTable      & oHashTable);

	/**
	  @brief Get constructed executable
	  @param iExecutableSize - size of program [out]
	  @return pointer to program core
	*/
	const VMExecutable * GetExecutable(UINT_32 & iExecutableSize);

	/**
	  @brief A destructor
	*/
	~VMDumper();

private:
	/**
	  @brief Copy constructor
	  @param oRhs - object to copy
	*/
	VMDumper(const VMDumper & oRhs);

	/**
	  @brief Operator =
	  @param oRhs - object to copy
	*/
	VMDumper & operator=(const VMDumper & oRhs);

	/** Size of program                */
	UINT_32         iVMExecutableSize;
	/** Program core                   */
	VMExecutable  * oVMExecutable;
};

} // namespace CTPP
#endif // _CTPP2_VM_LOADER_HPP__
// End.

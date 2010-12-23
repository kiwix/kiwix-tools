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
 *      CTPP2VMOpcodeCollector.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_OPCODE_COLLECTOR_H__
#define _CTPP2_VM_OPCODE_COLLECTOR_H__ 1

#include "CTPP2VMInstruction.hpp"

#include "STLVector.hpp"

/**
  @file CTPP2VMOpcodeCollector.hpp
  @brief Collector of generated operation codes
*/

namespace CTPP // C++ Template Engine
{

/**
  @class VMOpcodeCollector VMOpcodeCollector.hpp <VMOpcodeCollector.hpp>
  @brief Collector of generated operation codes
*/
class CTPP2DECL VMOpcodeCollector
{
public:

	/**
	  @brief Remove instruction from top of code segment
	  @return last instruction position
	*/
	INT_32 Remove();

	/**
	  @brief Insert instruction into code segment
	  @param oInstruction - instruction to insert
	  @return instruction position
	*/
	INT_32 Insert(const VMInstruction & oInstruction);

	/**
	  @brief Get instruction by instruction number
	  @param iIP - instruction number
	  @return pointer to instruction or NULL if instruction does not exist
	*/
	VMInstruction * GetInstruction(const UINT_32 & iIP);

	/**
	  @brief Get last instruction number
	*/
	UINT_32 GetCodeSize() const;

	/**
	  @param iCodeSize - dize of code segment [out]
	  @return code segment
	*/
	const VMInstruction * GetCode(UINT_32 & iCodeSize);

	/**
	  @brief A destructor
	*/
	~VMOpcodeCollector() throw();
private:
	/** Code segment */
	STLW::vector<VMInstruction>  oCodeSeg;
};

} // namespace CTPP
#endif // _VM_OPCODE_COLLECTOR_H__
// End.

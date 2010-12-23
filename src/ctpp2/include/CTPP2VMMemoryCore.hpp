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
 *      CTPP2VMMemoryCore.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_MEMORY_CORE_HPP__
#define _CTPP2_VM_MEMORY_CORE_HPP__ 1

#include "CTPP2HashTable.hpp"
#include "CTPP2StaticData.hpp"
#include "CTPP2StaticText.hpp"
#include "CTPP2BitIndex.hpp"

/**
  @file CTPP2VMMemoryCore.hpp
  @brief Virtual machine ready-to-run memory core of executable file
*/

namespace CTPP // C++ Template Engine
{
// FWD
struct VMExecutable;
struct VMInstruction;

/**
  @struct VMMemoryCore VMMemoryCore.hpp <VMMemoryCore.HPP>
  @brief CTPP ready-to-run memory core
*/
struct CTPP2DECL VMMemoryCore
{
	/**
	  @brief Constructor pVMExecutable - image of program
	*/
	VMMemoryCore(const VMExecutable  * pVMExecutable);

	/** Code segment size                    */
	const UINT_32                code_size;
	/** Code segment                         */
	const VMInstruction        * instructions;
	/** Syscalls segment                     */
	const ReducedStaticText      syscalls;
	/** Static data segment                  */
	const ReducedStaticData      static_data;
	/** Static text segment                  */
	const ReducedStaticText      static_text;
	/** Static data bit index                */
	const ReducedBitIndex        bit_index;
	/** Table with syscalls                  */
	const ReducedHashTable       calls_table;
	/** System calls translation map         */
	INT_32                     * syscall_map;
};

} // namespace CTPP
#endif // _CTPP2_VM_MEMORY_CORE_HPP__
// End.

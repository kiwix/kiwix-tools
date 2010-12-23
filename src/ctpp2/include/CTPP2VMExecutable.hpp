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
 *      CTPP2VMExecutable.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_EXECUTABLE_HPP__
#define _CTPP2_VM_EXECUTABLE_HPP__ 1

#include "CTPP2Types.h"

/**
  @file CTPP2VMExecutable.hpp
  @brief Virtual machine executable file
*/

namespace CTPP // C++ Template Engine
{
// FWD
struct VMInstruction;
union StaticDataVar;
struct TextDataIndex;
struct BitIndexData;
struct HashElement;

/**
  @struct VMExecutable CTPP2VMExecutable.hpp <CTPP2VMExecutable.hpp>
  @brief CTPP Executable file
*/
struct VMExecutable
{
	/** CTPP magic number                    */
	UCHAR_8      magic[4]; // 'CTPP' xor 0xFFFFFFFF

	/** CTPP reserved fields                 */
	CHAR_8       version[8];

	/** Code entry point                     */
	UINT_32      entry_point;
	/** Offset of code segment               */
	UINT_32      code_offset;
	/** Code segment size                    */
	UINT_32      code_size;

	/** Offset of static text segment        */
	UINT_32      syscalls_offset;
	/** Static text segment size             */
	UINT_32      syscalls_data_size;

	/** Offset of static text index segment  */
	UINT_32      syscalls_index_offset;
	/** Static text index segment size       */
	UINT_32      syscalls_index_size;

	/** Offset of static data segment        */
	UINT_32      static_data_offset;
	/** Static data segment size             */
	UINT_32      static_data_data_size;

	/** Offset of static text segment        */
	UINT_32      static_text_offset;
	/** Static text segment size             */
	UINT_32      static_text_data_size;

	/** Offset of static text index segment  */
	UINT_32      static_text_index_offset;
	/** Static text index segment size       */
	UINT_32      static_text_index_size;

	// Version 2.2+
	/** Offset of static data bit index      */
	UINT_32      static_data_bit_index_offset;
	// Version 2.2+
	/** Offset of static data bit index      */
	UINT_32      static_data_bit_index_size;

	/** Platform                             */
	UINT_64      platform;
	/** ieee 754 64-bit floating point value */
	W_FLOAT      ieee754double;
	/** Cyclic Redundancy Check              */
	UINT_32      crc;

	// Version 2.4+
	/** Calls hash table offset              */
	UINT_32      calls_hash_table_offset;
	/** Calls hash table size                */
	UINT_32      calls_hash_table_size;
	/** Number of elements                   */
	UINT_32      calls_hash_table_power;

	/** Fix for alignment                    */
	UINT_32      dummy;

	/**
	  @brief Get start of code segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of code segment
	*/
	static const VMInstruction * GetCodeSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of syscalls segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of data segment
	*/
	static CCHAR_P GetSyscallsSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of syscalls index segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of data segment
	*/
	static const TextDataIndex * GetSyscallsIndexSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of static data segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of data segment
	*/
	static const StaticDataVar * GetStaticDataSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of static data bit index
	  @param oVMExecutable - core of executable file
	  @return pointer to start of text index segment
	*/
	static const BitIndexData * GetStaticDataBitIndex(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of static text segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of text segment
	*/
	static CCHAR_P GetStaticTextSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get start of static text index segment
	  @param oVMExecutable - core of executable file
	  @return pointer to start of text index segment
	*/
	static const TextDataIndex * GetStaticTextIndexSeg(const VMExecutable * oVMExecutable);

	/**
	  @brief Get calls hash table
	  @param oVMExecutable - core of executable file
	  @return pointer to hash table
	*/
	static const HashElement * GetCallsTable(const VMExecutable * oVMExecutable);

	/**
	  @brief Get calls hash table size
	  @param oVMExecutable - core of executable file
	  @return Hash table size (in power of 2)
	*/
	static UINT_32 GetCallsTablePower(const VMExecutable * oVMExecutable);
};

} // namespace CTPP
#endif // _CTPP2_VM_EXECUTABLE_HPP__
// End.

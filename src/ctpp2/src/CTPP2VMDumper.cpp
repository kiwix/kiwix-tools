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
 *      VMDumper.cpp
 *
 * $CTPP$
 */
#include "CTPP2VMDumper.hpp"

#include "CTPP2Util.hpp"
#include "CTPP2VMInstruction.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace CTPP // C++ Template Engine
{

//
// Align segment
//
static INT_32 AlignSegment(const INT_32  iOffset)
{
	INT_32 iAlignedOffset = iOffset % 8;
	if (iAlignedOffset == 0 || iAlignedOffset == 8) { return iOffset; }

return iOffset + 8 - iAlignedOffset;
}

//
// Constructor
//
VMDumper::VMDumper(const VMMemoryCore & oMemoryCore)
{
	INT_32 iSyscallsDataSize   = 0;
	if (oMemoryCore.syscalls.iUsedDataOffsetsSize == 0) { iSyscallsDataSize = 0; }
	else
	{
		const TextDataIndex & oTMP = oMemoryCore.syscalls.aDataOffsets[oMemoryCore.syscalls.iUsedDataOffsetsSize - 1];
		iSyscallsDataSize = oTMP.offset + oTMP.length + 1;
	}

	INT_32 iStaticTextDataSize = 0;
	if (oMemoryCore.static_text.iUsedDataOffsetsSize == 0) { iStaticTextDataSize = 0; }
	else
	{
		const TextDataIndex & oTMP = oMemoryCore.static_text.aDataOffsets[oMemoryCore.static_text.iUsedDataOffsetsSize - 1];
		iStaticTextDataSize    = oTMP.offset + oTMP.length + 1;
	}

	const INT_32 iCodeSize               = sizeof(VMInstruction) * oMemoryCore.code_size;
	const INT_32 iSyscallsIndexSize      = sizeof(TextDataIndex) * oMemoryCore.syscalls.iUsedDataOffsetsSize;
	const INT_32 iStaticDataIndexSize    = sizeof(StaticDataVar) * oMemoryCore.static_data.iUsedDataSize;
	const INT_32 iStaticTextIndexSize    = sizeof(TextDataIndex) * oMemoryCore.static_text.iUsedDataOffsetsSize;

	const INT_32 iStaticDataBitIndexSize = sizeof(UINT_64) + oMemoryCore.bit_index.GetUsedSize();
	const INT_32 iCallsHashTableSize     = sizeof(HashElement) * (1 << oMemoryCore.calls_table.iPower);

	                    // Header
	iVMExecutableSize = AlignSegment(sizeof(VMExecutable)) +

	                    // Code segment
	                    AlignSegment(iCodeSize) +

	                    // Static syscalls segment
	                    AlignSegment(iSyscallsDataSize) +
	                    // Static syscalls index segment
	                    AlignSegment(iSyscallsIndexSize) +

	                    // Static Data segment
	                    AlignSegment(iStaticDataIndexSize) +

	                    // Static Text segment
	                    AlignSegment(iStaticTextDataSize) +
	                    // Static Text index segment
	                    AlignSegment(iStaticTextIndexSize) +

	                    // Bit index
	                    AlignSegment(iStaticDataBitIndexSize) +

	                    // hash table size
	                    AlignSegment(iCallsHashTableSize);

	CHAR_P vRawData = (CHAR_P)malloc(iVMExecutableSize);
	// Make valgind happy
	memset(vRawData, '-', iVMExecutableSize);

	oVMExecutable = (VMExecutable *)vRawData;

	oVMExecutable -> magic[0] = 'C';
	oVMExecutable -> magic[1] = 'T';
	oVMExecutable -> magic[2] = 'P';
	oVMExecutable -> magic[3] = 'P';

	for(UINT_32 iI = 0; iI < 8; ++iI) { oVMExecutable -> version[iI] = 0; }

	oVMExecutable -> version[0] = 2;

	oVMExecutable -> entry_point              = 0;
	oVMExecutable -> code_offset              = AlignSegment(sizeof(VMExecutable));
	oVMExecutable -> code_size                = iCodeSize;

	// Syscalls segment                         // Aligned                     Not yet aligned
	oVMExecutable -> syscalls_offset          = oVMExecutable -> code_offset + AlignSegment(iCodeSize);
	oVMExecutable -> syscalls_data_size       = iSyscallsDataSize;

	//                                          // Aligned                         Not yet aligned
	oVMExecutable -> syscalls_index_offset    = oVMExecutable -> syscalls_offset + AlignSegment(iSyscallsDataSize);
	oVMExecutable -> syscalls_index_size      = iSyscallsIndexSize;

	// Data segment                             // Aligned                               Not yet aligned
	oVMExecutable -> static_data_offset       = oVMExecutable -> syscalls_index_offset + AlignSegment(iSyscallsIndexSize);
	oVMExecutable -> static_data_data_size    = iStaticDataIndexSize;

	// Text segment                             // Aligned                            Not yet aligned
	oVMExecutable -> static_text_offset       = oVMExecutable -> static_data_offset + AlignSegment(iStaticDataIndexSize);
	oVMExecutable -> static_text_data_size    = iStaticTextDataSize;

	//                                          // Aligned                            Not yet aligned
	oVMExecutable -> static_text_index_offset = oVMExecutable -> static_text_offset + AlignSegment(iStaticTextDataSize);
	oVMExecutable -> static_text_index_size   = iStaticTextIndexSize;

	// Version 2.2+
	// Offset of static data bit index
	oVMExecutable -> static_data_bit_index_offset = oVMExecutable -> static_text_index_offset + AlignSegment(iStaticTextIndexSize);
	// Offset of static data bit inde
	oVMExecutable -> static_data_bit_index_size   = iStaticDataBitIndexSize;


	// Platform-dependent data (byte order)
	oVMExecutable -> platform      = 0x4142434445464748ull;
	oVMExecutable -> ieee754double = 15839800103804824402926068484019465486336.0;
	oVMExecutable -> crc = 0;

	// Version 2.4+
	// Calls hash table                      // Aligned                                         Not yet aligned
	oVMExecutable -> calls_hash_table_offset  = oVMExecutable -> static_data_bit_index_offset + AlignSegment(iStaticDataBitIndexSize);
	oVMExecutable -> calls_hash_table_size    = iCallsHashTableSize;
	oVMExecutable -> calls_hash_table_power   = oMemoryCore.calls_table.iPower;

	// Copy code segment
	memcpy(vRawData + oVMExecutable -> code_offset,                      oMemoryCore.instructions,            oVMExecutable -> code_size);

	// Copy syscalls, if need
	if (oVMExecutable -> syscalls_data_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> syscalls_offset,          oMemoryCore.syscalls.sData,          oVMExecutable -> syscalls_data_size);
		// Copy static text index segment
		memcpy(vRawData + oVMExecutable -> syscalls_index_offset,    oMemoryCore.syscalls.aDataOffsets,   oVMExecutable -> syscalls_index_size);
	}

	// Copy static data segment, if need
	if (oVMExecutable -> static_data_data_size != 0)
	{
		// Copy static data segment
		memcpy(vRawData + oVMExecutable -> static_data_offset,       oMemoryCore.static_data.aData,        oVMExecutable -> static_data_data_size);
	}

	// Copy static text segment, if need
	if (oVMExecutable -> static_text_data_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> static_text_offset,       oMemoryCore.static_text.sData,        oVMExecutable -> static_text_data_size);
		// Copy static text index segment
		memcpy(vRawData + oVMExecutable -> static_text_index_offset, oMemoryCore.static_text.aDataOffsets, oVMExecutable -> static_text_index_size);
	}

	// Copy static data bit index, if need
	if (oVMExecutable -> static_data_bit_index_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> static_data_bit_index_offset, oMemoryCore.bit_index.GetIndexData(), oVMExecutable -> static_data_bit_index_size);
	}

	// Version 2.4+
	// Copy Calls hash table, if need
	if (oVMExecutable -> calls_hash_table_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> calls_hash_table_offset, oMemoryCore.calls_table.aElements, oVMExecutable -> calls_hash_table_size);
	}

	// Calculate CRC of file
	oVMExecutable -> crc = crc32((UCCHAR_P)oVMExecutable, iVMExecutableSize);
}

//
// Constructor
//
VMDumper::VMDumper(const UINT_32          iInstructions,
                   const VMInstruction  * aInstructions,
                   const StaticText     & oSyscalls,
                   const StaticData     & oStaticData,
                   const StaticText     & oStaticText,
                   const HashTable      & oHashTable)
{
	INT_32 iSyscallsDataSize   = 0;
	if (oSyscalls.iUsedDataOffsetsSize == 0) { iSyscallsDataSize = 0; }
	else
	{
		const TextDataIndex & oTMP = oSyscalls.aDataOffsets[oSyscalls.iUsedDataOffsetsSize - 1];
		iSyscallsDataSize = oTMP.offset + oTMP.length + 1;
	}

	INT_32 iStaticTextDataSize = 0;
	if (oStaticText.iUsedDataOffsetsSize == 0) { iStaticTextDataSize = 0; }
	else
	{
		const TextDataIndex & oTMP = oStaticText.aDataOffsets[oStaticText.iUsedDataOffsetsSize - 1];
		iStaticTextDataSize    = oTMP.offset + oTMP.length + 1;
	}

	const INT_32 iCodeSize               = sizeof(VMInstruction) * iInstructions;
	const INT_32 iSyscallsIndexSize      = sizeof(TextDataIndex) * oSyscalls.iUsedDataOffsetsSize;
	const INT_32 iStaticDataIndexSize    = sizeof(StaticDataVar) * oStaticData.iUsedDataSize;
	const INT_32 iStaticTextIndexSize    = sizeof(TextDataIndex) * oStaticText.iUsedDataOffsetsSize;
	const INT_32 iStaticDataBitIndexSize = sizeof(UINT_64) + oStaticData.GetBitIndex() -> GetUsedSize();
	const INT_32 iCallsHashTableSize     = sizeof(HashElement) * (1 << oHashTable.iPower);

	                    // Header
	iVMExecutableSize = AlignSegment(sizeof(VMExecutable)) +

	                    // Code segment
	                    AlignSegment(iCodeSize) +

	                    // Static syscalls segment
	                    AlignSegment(iSyscallsDataSize) +
	                    // Static syscalls index segment
	                    AlignSegment(iSyscallsIndexSize) +

	                    // Static Data segment
	                    AlignSegment(iStaticDataIndexSize) +

	                    // Static Text segment
	                    AlignSegment(iStaticTextDataSize) +
	                    // Static Text index segment
	                    AlignSegment(iStaticTextIndexSize) +

	                    // Bit index
	                    AlignSegment(iStaticDataBitIndexSize) +

	                    // Calls segment
	                    AlignSegment(iCallsHashTableSize);

	CHAR_P vRawData = (CHAR_P)malloc(iVMExecutableSize);
	// Make valgind happy
	memset(vRawData, '-', iVMExecutableSize);

	oVMExecutable = (VMExecutable *)vRawData;

	oVMExecutable -> magic[0] = 'C';
	oVMExecutable -> magic[1] = 'T';
	oVMExecutable -> magic[2] = 'P';
	oVMExecutable -> magic[3] = 'P';

	for(UINT_32 iI = 0; iI < 8; ++iI) { oVMExecutable -> version[iI] = 0; }

	oVMExecutable -> version[0] = 2;

	oVMExecutable -> entry_point              = 0;
	oVMExecutable -> code_offset              = AlignSegment(sizeof(VMExecutable));
	oVMExecutable -> code_size                = iCodeSize;

	// Syscalls segment                         // Aligned                     Not yet aligned
	oVMExecutable -> syscalls_offset          = oVMExecutable -> code_offset + AlignSegment(iCodeSize);
	oVMExecutable -> syscalls_data_size       = iSyscallsDataSize;

	//                                          // Aligned                         Not yet aligned
	oVMExecutable -> syscalls_index_offset    = oVMExecutable -> syscalls_offset + AlignSegment(iSyscallsDataSize);
	oVMExecutable -> syscalls_index_size      = iSyscallsIndexSize;

	// Data segment                             // Aligned                               Not yet aligned
	oVMExecutable -> static_data_offset       = oVMExecutable -> syscalls_index_offset + AlignSegment(iSyscallsIndexSize);
	oVMExecutable -> static_data_data_size    = iStaticDataIndexSize;

	// Text segment                             // Aligned                            Not yet aligned
	oVMExecutable -> static_text_offset       = oVMExecutable -> static_data_offset + AlignSegment(iStaticDataIndexSize);
	oVMExecutable -> static_text_data_size    = iStaticTextDataSize;

	//                                          // Aligned                            Not yet aligned
	oVMExecutable -> static_text_index_offset = oVMExecutable -> static_text_offset + AlignSegment(iStaticTextDataSize);
	oVMExecutable -> static_text_index_size   = iStaticTextIndexSize;

	// Version 2.2+
	// Offset of static data bit index
	oVMExecutable -> static_data_bit_index_offset = oVMExecutable -> static_text_index_offset + AlignSegment(iStaticTextIndexSize);
	// Offset of static data bit inde
	oVMExecutable -> static_data_bit_index_size   = iStaticDataBitIndexSize;

	// Platform-dependent data (byte order)
	oVMExecutable -> platform      = 0x4142434445464748ull;
	oVMExecutable -> ieee754double = 15839800103804824402926068484019465486336.0;
	oVMExecutable -> crc = 0;

	// Version 2.4+
	// Calls hash table                      // Aligned                                         Not yet aligned
	oVMExecutable -> calls_hash_table_offset  = oVMExecutable -> static_data_bit_index_offset + AlignSegment(iStaticDataBitIndexSize);
	oVMExecutable -> calls_hash_table_size    = iCallsHashTableSize;
	oVMExecutable -> calls_hash_table_power   = oHashTable.iPower;

	// Copy code segment
	memcpy(vRawData + oVMExecutable -> code_offset,                      aInstructions,            oVMExecutable -> code_size);

	// Copy syscalls, if need
	if (oVMExecutable -> syscalls_data_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> syscalls_offset,          oSyscalls.sData,          oVMExecutable -> syscalls_data_size);
		// Copy static text index segment
		memcpy(vRawData + oVMExecutable -> syscalls_index_offset,    oSyscalls.aDataOffsets,   oVMExecutable -> syscalls_index_size);
	}

	// Copy static data segment, if need
	if (oVMExecutable -> static_data_data_size != 0)
	{
		// Copy static data segment
		memcpy(vRawData + oVMExecutable -> static_data_offset,       oStaticData.aData,        oVMExecutable -> static_data_data_size);
	}

	// Copy static text segment, if need
	if (oVMExecutable -> static_text_data_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> static_text_offset,       oStaticText.sData,        oVMExecutable -> static_text_data_size);
		// Copy static text index segment
		memcpy(vRawData + oVMExecutable -> static_text_index_offset, oStaticText.aDataOffsets, oVMExecutable -> static_text_index_size);
	}

	// Copy static data bit index, if need
	if (oVMExecutable -> static_data_bit_index_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> static_data_bit_index_offset, oStaticData.GetBitIndex() -> GetIndexData(), oVMExecutable -> static_data_bit_index_size);
	}

	// Version 2.4+
	// Copy Calls hash table, if need
	if (oVMExecutable -> calls_hash_table_size != 0)
	{
		memcpy(vRawData + oVMExecutable -> calls_hash_table_offset, oHashTable.aElements, oVMExecutable -> calls_hash_table_size);
	}

	// Calculate CRC of file
	oVMExecutable -> crc = crc32((UCCHAR_P)oVMExecutable, iVMExecutableSize);
}

//
// Get constructed executable
//
const VMExecutable * VMDumper::GetExecutable(UINT_32 & iExecutableSize)
{
	iExecutableSize = iVMExecutableSize;

return oVMExecutable;
}

//
// A destructor
//
VMDumper::~VMDumper()
{
	free(oVMExecutable);
}

} // namespace CTPP
// End.

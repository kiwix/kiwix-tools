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
 *      CTPP2VMFileLoader.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMFileLoader.hpp"

#include "CTPP2Util.hpp"
#include "CTPP2Exception.hpp"
#include "CTPP2VMExecutable.hpp"
#include "CTPP2VMInstruction.hpp"
#include "CTPP2VMMemoryCore.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace CTPP // C++ Template Engine
{

//
// Convert byte order
//
static void ConvertExecutable(VMExecutable * oCore)
{
	// Code entry point
	oCore -> entry_point = Swap32(oCore -> entry_point);
	// Offset of code segment
	oCore -> code_offset = Swap32(oCore -> code_offset);

	// Code segment size
	oCore -> code_size   = Swap32(oCore -> code_size);

	// Offset of static text segment
	oCore -> syscalls_offset    = Swap32(oCore -> syscalls_offset);
	// Static text segment size
	oCore -> syscalls_data_size = Swap32(oCore -> syscalls_data_size);

	// Offset of static text index segment
	oCore -> syscalls_index_offset = Swap32(oCore -> syscalls_index_offset);
	// Static text index segment size
	oCore -> syscalls_index_size   = Swap32(oCore -> syscalls_index_size);

	// Offset of static data segment
	oCore -> static_data_offset    = Swap32(oCore -> static_data_offset);

	// Static data segment size
	oCore -> static_data_data_size = Swap32(oCore -> static_data_data_size);

	// Offset of static text segment
	oCore -> static_text_offset    = Swap32(oCore -> static_text_offset);
	// Static text segment size
	oCore -> static_text_data_size = Swap32(oCore -> static_text_data_size);

	// Offset of static text index segment
	oCore -> static_text_index_offset = Swap32(oCore -> static_text_index_offset);
	// Static text index segment size
	oCore -> static_text_index_size   = Swap32(oCore -> static_text_index_size);

	// Version 2.2+
	// Offset of static data bit index
	oCore -> static_data_bit_index_offset = Swap32(oCore -> static_data_bit_index_offset);
	/// Offset of static data bit index
	oCore -> static_data_bit_index_size = Swap32(oCore -> static_data_bit_index_size);

	// Platform
	oCore -> platform      = Swap64(oCore -> platform);

	// Ugly-jolly hack!
	// ... dereferencing type-punned pointer will break strict-aliasing rules ...
	UINT_64 iTMP;
	memcpy(&iTMP, &(oCore -> ieee754double), sizeof(UINT_64));
	iTMP = Swap64(iTMP);
	memcpy(&(oCore -> ieee754double), &iTMP, sizeof(UINT_64));

	// Cyclic Redundancy Check
	oCore -> crc = 0;

	// Convert data structures

	// Convert code segment
	VMInstruction * pInstructions = const_cast<VMInstruction *>(VMExecutable::GetCodeSeg(oCore));
	UINT_32 iI = 0;
	UINT_32 iSteps = oCore -> code_size / sizeof(VMInstruction);
	for(iI = 0; iI < iSteps; ++iI)
	{
		pInstructions -> instruction = Swap32(pInstructions -> instruction);
		pInstructions -> argument    = Swap32(pInstructions -> argument);
		pInstructions -> reserved    = Swap64(pInstructions -> reserved);
		++pInstructions;
	}

	// Convert syscalls index
	TextDataIndex * pTextIndex = const_cast<TextDataIndex *>(VMExecutable::GetSyscallsIndexSeg(oCore));
	iSteps = oCore -> syscalls_index_size / sizeof(TextDataIndex);
	for(iI = 0; iI < iSteps; ++iI)
	{
		pTextIndex -> offset = Swap32(pTextIndex -> offset);
		pTextIndex -> length = Swap32(pTextIndex -> length);
		++pTextIndex;
	}

	// Convert static text index
	pTextIndex = const_cast<TextDataIndex *>(VMExecutable::GetStaticTextIndexSeg(oCore));
	iSteps = oCore -> static_text_index_size / sizeof(TextDataIndex);
	for(iI = 0; iI < iSteps; ++iI)
	{
		pTextIndex -> offset = Swap32(pTextIndex -> offset);
		pTextIndex -> length = Swap32(pTextIndex -> length);
		++pTextIndex;
	}

	// Convert static data
	StaticDataVar * pStaticDataVar = const_cast<StaticDataVar *>(VMExecutable::GetStaticDataSeg(oCore));
	iSteps = oCore -> static_data_data_size / sizeof(StaticDataVar);
	for(iI = 0; iI < iSteps; ++iI)
	{
		(*pStaticDataVar).i_data = Swap64((*pStaticDataVar).i_data);
		++pStaticDataVar;
	}
}

//
// Constructor
//
VMFileLoader::VMFileLoader(CCHAR_P szFileName)
{
	// Get file size
	struct stat oStat;
	if (stat(szFileName, &oStat) == -1) { throw CTPPUnixException("stat", errno); }
	if (oStat.st_size == 0) { throw CTPPLogicError("Cannot get size of file"); }

	// Load file
	FILE * F = fopen(szFileName, "rb");
	if (F == NULL) { throw CTPPUnixException("fopen", errno); }

	// Allocate memory
	oCore = (VMExecutable *)malloc(oStat.st_size);
	// Read from file
	if (fread(oCore, oStat.st_size, 1, F) != 1)
	{
		fclose(F);
		throw CTPPUnixException("fread", errno);
	}

	// All Done
	fclose(F);

	if (oCore -> magic[0] == 'C' &&
	    oCore -> magic[1] == 'T' &&
	    oCore -> magic[2] == 'P' &&
	    oCore -> magic[3] == 'P')
	{
		// Check version
		if (oCore -> version[0] >= 1)
		{
			// Platform-dependent data (byte order)
			if (oCore -> platform == 0x4142434445464748ull)
			{
#ifdef _DEBUG
				fprintf(stderr, "Big/Little Endian conversion: Nothing to do\n");
#endif

				// Nothing to do, only check crc
				UINT_32 iCRC = oCore -> crc;
				oCore -> crc = 0;

				// Calculate CRC of file
				if (iCRC != crc32((UCCHAR_P)oCore, oStat.st_size))
				{
					free(oCore);
					throw CTPPLogicError("CRC checksum invalid");
				}
			}
			// Platform-dependent data (byte order)
			else if (oCore -> platform == 0x4847464544434241ull)
			{
				// Need to reconvert data
#ifdef _DEBUG
				fprintf(stderr, "Big/Little Endian conversion: Need to reconvert core\n");
#endif
				ConvertExecutable(oCore);
			}
			else
			{
				free(oCore);
				throw CTPPLogicError("Conversion of middle-end architecture does not supported.");
			}

			// Check IEEE 754 format
			if (oCore -> ieee754double != 15839800103804824402926068484019465486336.0)
			{
				free(oCore);
				throw CTPPLogicError("IEEE 754 format is broken, cannot convert file");
			}
		}

		pVMMemoryCore = new VMMemoryCore(oCore);
	}
	else
	{
		free(oCore);
		throw CTPPLogicError("Not an CTPP bytecode file.");
	}
}

//
// Get ready-to-run program
//
const VMMemoryCore * VMFileLoader::GetCore() const { return pVMMemoryCore; }

//
// A destructor
//
VMFileLoader::~VMFileLoader() throw()
{
	delete pVMMemoryCore;
	free(oCore);
}

} // namespace CTPP
// End.

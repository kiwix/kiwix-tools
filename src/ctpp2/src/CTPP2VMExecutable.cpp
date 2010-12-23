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
 *      VMExecutable.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMExecutable.hpp"

#include "CTPP2BitIndex.hpp"
#include "CTPP2HashTable.hpp"
#include "CTPP2StaticData.hpp"
#include "CTPP2StaticText.hpp"
#include "CTPP2VMInstruction.hpp"

namespace CTPP // C++ Template Engine
{

//
// Get start of code segment
//
const VMInstruction * VMExecutable::GetCodeSeg(const VMExecutable * oVMExecutable) { return (VMInstruction *)( (CCHAR_P)oVMExecutable + oVMExecutable -> code_offset ); }

//
// Get start of syscalls segment
//
CCHAR_P VMExecutable::GetSyscallsSeg(const VMExecutable * oVMExecutable) { return ( (CCHAR_P)oVMExecutable + oVMExecutable -> syscalls_offset ); }

//
// Get start of syscalls index segment
//
const TextDataIndex * VMExecutable::GetSyscallsIndexSeg(const VMExecutable * oVMExecutable) { return (TextDataIndex *)( (CCHAR_P)oVMExecutable + oVMExecutable -> syscalls_index_offset ) ; }

//
// Get start of static data segment
//
const StaticDataVar * VMExecutable::GetStaticDataSeg(const VMExecutable * oVMExecutable) { return (StaticDataVar *)( (CCHAR_P)oVMExecutable + oVMExecutable -> static_data_offset ); }

//
// Get start of static data bit index
//
const BitIndexData * VMExecutable::GetStaticDataBitIndex(const VMExecutable * oVMExecutable) { return (BitIndexData *)( (CCHAR_P)oVMExecutable + oVMExecutable -> static_data_bit_index_offset ); }

//
// Get start of static text segment
//
CCHAR_P VMExecutable::GetStaticTextSeg(const VMExecutable * oVMExecutable) { return ( (CCHAR_P)oVMExecutable + oVMExecutable -> static_text_offset ); }

//
// Get start of static text index segment
//
const TextDataIndex * VMExecutable::GetStaticTextIndexSeg(const VMExecutable * oVMExecutable) { return (TextDataIndex *) ( (CCHAR_P)oVMExecutable + oVMExecutable -> static_text_index_offset ); }

//
// Get calls hash table
//
const HashElement * VMExecutable::GetCallsTable(const VMExecutable * oVMExecutable) { return (HashElement *)( (CCHAR_P)oVMExecutable + oVMExecutable -> calls_hash_table_offset); }

//
// Get calls hash table size
//
UINT_32 VMExecutable::GetCallsTablePower(const VMExecutable * oVMExecutable) { return oVMExecutable -> calls_hash_table_power; }

} // namespace CTPP
// End.

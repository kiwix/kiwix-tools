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
 *      CTPP2VMMemoryCore.cpp
 *
 * $CTPP$
 */
#include "CTPP2VMMemoryCore.hpp"

#include "CTPP2VMExecutable.hpp"
#include "CTPP2VMInstruction.hpp"

namespace CTPP // C++ Template Engine
{
//
// Constructor
//
VMMemoryCore::VMMemoryCore(const VMExecutable  * pVMExecutable): code_size(pVMExecutable -> code_size / sizeof(VMInstruction)),
                                                                 // Code segment
                                                                 instructions(VMExecutable::GetCodeSeg(pVMExecutable)),
                                                                 // Syscalls
                                                                 syscalls(VMExecutable::GetSyscallsSeg(pVMExecutable),
                                                                          pVMExecutable -> syscalls_index_size / sizeof(TextDataIndex),
                                                                          VMExecutable::GetSyscallsIndexSeg(pVMExecutable)),
                                                                 // Static data
                                                                 static_data(VMExecutable::GetStaticDataSeg(pVMExecutable),
                                                                             pVMExecutable -> static_data_data_size / sizeof(StaticDataVar)),
                                                                 // Static text
                                                                 static_text(VMExecutable::GetStaticTextSeg(pVMExecutable),
                                                                             pVMExecutable -> static_text_index_size / sizeof(TextDataIndex),
                                                                             VMExecutable::GetStaticTextIndexSeg(pVMExecutable)),
                                                                 // Bit index
                                                                 bit_index(VMExecutable::GetStaticDataBitIndex(pVMExecutable)),
                                                                 // hash table
                                                                 calls_table(VMExecutable::GetCallsTable(pVMExecutable),
                                                                             VMExecutable::GetCallsTablePower(pVMExecutable))
{
	;;
}

} // namespace CTPP
// End.

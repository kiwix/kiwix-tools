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
 *      CTPP2VMOpcodeCollector.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMOpcodeCollector.hpp"

namespace CTPP // C++ Template Engine
{

//
// Remove instruction from top of code segment
//
INT_32 VMOpcodeCollector::Remove()
{
	STLW::vector<VMInstruction>::iterator itvCodeSeg = oCodeSeg.end();
	--itvCodeSeg;
	oCodeSeg.erase(itvCodeSeg);

return oCodeSeg.size() - 1;
}

//
// Insert instruction into code segment
//
INT_32 VMOpcodeCollector::Insert(const VMInstruction & oInstruction)
{
	oCodeSeg.push_back(oInstruction);

return oCodeSeg.size() - 1;
}

//
// Get compiled
//
const VMInstruction * VMOpcodeCollector::GetCode(UINT_32 & iCodeSize)
{
	iCodeSize = oCodeSeg.size();

	if (iCodeSize == 0) { return NULL; }

return &oCodeSeg[0];
}

//
// Get instruction by instruction number
//
VMInstruction * VMOpcodeCollector::GetInstruction(const UINT_32 & iIP)
{
	if (iIP >= oCodeSeg.size()) { return NULL; }

return &oCodeSeg[iIP];
}

//
// Get last instruction number
//
UINT_32 VMOpcodeCollector::GetCodeSize() const { return oCodeSeg.size(); }

//
// A destructor
//
VMOpcodeCollector::~VMOpcodeCollector() throw()
{
	;;
}

} // namespace CTPP
// End.

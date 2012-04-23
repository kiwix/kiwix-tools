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
 *      CTPP2FileSourceLoader.cpp
 *
 * $CTPP$
 */

#include "CTPP2TextLoader.hpp"

#include "CTPP2Exception.hpp"

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/**
  @file FileOutputCollector.hpp
  @brief Virtual machine file output data collector
*/

namespace CTPP // C++ Template Engine
{

//
// Clone loader
//
CTPP2SourceLoader * CTPP2TextLoader::Clone()
{
  return new CTPP2TextLoader;
}

//
// Constructor
//
CTPP2TextLoader::CTPP2TextLoader() {
}

//
// Load template with specified name
//
INT_32 CTPP2TextLoader::LoadTemplate(CCHAR_P szTemplateName)
{
  this->templateText = std::string(szTemplateName);
  return 0;
}

//
// Get template
//
CCHAR_P CTPP2TextLoader::GetTemplate(UINT_32 & iITemplateSize)
{
  iITemplateSize = this->templateText.size();
  return this->templateText.data();
}

//
// A destructor
//
CTPP2TextLoader::~CTPP2TextLoader() throw()
{
}

} // namespace CTPP
// End.

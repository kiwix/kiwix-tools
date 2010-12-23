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
 *      CTPP2SourceLoader.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_FILE_SOURCE_LOADER_H__
#define _CTPP2_FILE_SOURCE_LOADER_H__ 1

#include "CTPP2SourceLoader.hpp"

#include "STLString.hpp"
#include "STLVector.hpp"

/**
  @file CTPP2FileSourceLoader.hpp
  @brief Load template from file
*/

namespace CTPP // C++ Template Engine
{

/**
  @class CTPP2FileSourceLoader CTPP2FileSourceLoader.hpp> <CTPP2FileSourceLoader.hpp>
  @brief Load template from file
*/
class CTPP2DECL CTPP2FileSourceLoader:
  public CTPP2SourceLoader
{
public:
	/**
	  @brief Constructor
	*/
	CTPP2FileSourceLoader();

	/**
	  @brief Set of directories where included templates should be found
	  @param vIncludeDirs - vector with directories
	*/
	void SetIncludeDirs(const STLW::vector<STLW::string> & vIncludeDirs = STLW::vector<STLW::string>());

	/**
	  @brief Load template with specified name
	  @param szTemplateName - template name
	  @return 0 if success, -1 if any error occured
	*/
	INT_32 LoadTemplate(CCHAR_P szTemplateName);

	/**
	  @brief Get template
	  @param iTemplateSize - template size [out]
	  @return pointer to start of template buffer if success, NULL - if any error occured
	*/
	CCHAR_P GetTemplate(UINT_32 & iTemplateSize);

	/**
	  @brief Get template file name
	  @return asciz temaplet filename
	*/
	CCHAR_P GetTemplateName() const;

	/**
	  @brief Clone loader object
	  @return clone to self
	*/
	CTPP2SourceLoader * Clone();

	/**
	  @brief A destructor
	*/
	~CTPP2FileSourceLoader() throw();
private:
	/* List ofi nclude directories */
	STLW::vector<STLW::string> vIncludeDirs;

	/** Template body             */
	CHAR_P        sTemplate;
	/** Template size             */
	UINT_32       iTemplateSize;
	/** Current working directory */
	STLW::string  sCurrentDir;
	/** Current template name     */
	STLW::string  sNormalizedFileName;

};

} // namespace CTPP
#endif // _CTPP2_FILE_SOURCE_LOADER_H__
// End.

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
 *      CTPP2VMSTDLibFunctions.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_VM_STDLIB_FUNCTIONS_HPP__
#define _CTPP2_VM_STDLIB_FUNCTIONS_HPP__ 1

/**
  @file CTPP2VMSTDLibFunctions.hpp
  @brief Virtual machine standard library functions
*/

#include "FnArrayElement.hpp"
#include "FnAvg.hpp"
#include "FnBase64Decode.hpp"
#include "FnBase64Encode.hpp"
#include "FnCast.hpp"
#include "FnConcat.hpp"
#include "FnDateFormat.hpp"
#include "FnDefault.hpp"
#include "FnDefined.hpp"
#include "FnEmitter.hpp"
#include "FnError.hpp"
#include "FnFormParam.hpp"

#ifdef GETTEXT_SUPPORT
#include "FnGetText.hpp"
#include "FnGetText_.hpp"
#endif // GETTEXT_SUPPORT

#ifdef MD5_SUPPORT
#include "FnHMACMD5.hpp"
#endif // MD5_SUPPORT

#include "FnHashElement.hpp"
#include "FnHTMLEscape.hpp"
#include "FnHrefParam.hpp"

#ifdef ICONV_SUPPORT
#include "FnIconv.hpp"
#endif // ICONV_SUPPORT

#include "FnInSet.hpp"
#include "FnJSEscape.hpp"
#include "FnJSON.hpp"
#include "FnListElement.hpp"
#include "FnLog.hpp"
#include "FnMBSize.hpp"
#include "FnMBSubstring.hpp"
#include "FnMBTruncate.hpp"

#ifdef MD5_SUPPORT
#include "FnMD5.hpp"
#endif // MD5_SUPPORT

#include "FnMax.hpp"
#include "FnMin.hpp"
#include "FnNumFormat.hpp"
#include "FnNumeral.hpp"
#include "FnObjDump.hpp"
#include "FnRandom.hpp"
#include "FnSize.hpp"
#include "FnSprintf.hpp"
#include "FnSubstring.hpp"
#include "FnTruncate.hpp"
#include "FnURIEscape.hpp"
#include "FnURLEscape.hpp"
#include "FnVersion.hpp"
#include "FnWMLEscape.hpp"
#include "FnXMLEscape.hpp"

#endif // _CTPP2_VM_STDLIB_FUNCTIONS_HPP__
// End.

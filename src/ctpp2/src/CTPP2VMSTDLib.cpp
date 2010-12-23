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
 *      CTPP2VMSTDLib.cpp
 *
 * $CTPP$
 */

#include "CTPP2VMSTDLib.hpp"

#include "CDT.hpp"
#include "CTPP2Util.hpp"
#include "CTPP2OutputCollector.hpp"
#include "CTPP2StaticData.hpp"
#include "CTPP2StaticText.hpp"
#include "CTPP2SyscallFactory.hpp"


#include "CTPP2VMSTDLibFunctions.hpp"

#include <errno.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <time.h>

#ifdef GETTEXT_SUPPORT
#include <libintl.h>
#endif // GETTEXT_SUPPORT

#if defined(MD5_SUPPORT) && !defined(WIN32)
    #ifdef MD5_WITHOUT_OPENSSL
        #include <md5.h>
        #define MD5_Init    MD5Init
        #define MD5_Update  MD5Update
        #define MD5_Final   MD5Final
    #else
        #include <openssl/md5.h>
    #endif
#endif // defined(MD5_SUPPORT) && !defined(WIN32)

namespace CTPP // C++ Template Engine
{

//
// Initialize all functions in library
//
void STDLibInitializer::InitLibrary(SyscallFactory & oSyscallFactory)
{
	CCHAR_P * aFunctions = STDLibInitializer::GetFnList();
	while (*aFunctions != NULL)
	{
		oSyscallFactory.RegisterHandler(STDLibInitializer::CreateHandler(*aFunctions));
		++aFunctions;
	}
}

//
// Destroy all functions in library
//
void STDLibInitializer::DestroyLibrary(SyscallFactory & oSyscallFactory)
{
	CCHAR_P * aFunctions = STDLibInitializer::GetFnList();
	while (*aFunctions != NULL)
	{
		STDLibInitializer::DestroyHandler(oSyscallFactory.GetHandlerByName(*aFunctions));
		oSyscallFactory.RemoveHandler(*aFunctions);
		++aFunctions;
	}
}

//
// Get list of available functions
//
CCHAR_P * STDLibInitializer::GetFnList() { return aSTDFNList; }

//
// Create handler by name
//
SyscallHandler * STDLibInitializer::CreateHandler(CCHAR_P szHandler)
{
	if      (strcasecmp(CTPP2_INT_HANDLER_PREFIX "_emitter",      szHandler) == 0) { return new FnEmitter();      }
	else if (strcasecmp("avg",                                    szHandler) == 0) { return new FnAvg();          }
	else if (strcasecmp("array_element",                          szHandler) == 0) { return new FnArrayElement(); }
	else if (strcasecmp("base64_encode",                          szHandler) == 0) { return new FnBase64Encode(); }
	else if (strcasecmp("base64_decode",                          szHandler) == 0) { return new FnBase64Decode(); }
	else if (strcasecmp("cast",                                   szHandler) == 0) { return new FnCast();         }
	else if (strcasecmp("concat",                                 szHandler) == 0) { return new FnConcat();       }
	else if (strcasecmp("date_format",                            szHandler) == 0) { return new FnDateFormat();   }
	else if (strcasecmp("default",                                szHandler) == 0) { return new FnDefault();      }
	else if (strcasecmp("defined",                                szHandler) == 0) { return new FnDefined();      }
	else if (strcasecmp("error",                                  szHandler) == 0) { return new FnError();        }
	else if (strcasecmp("form_param",                             szHandler) == 0) { return new FnFormParam();    }
#ifdef GETTEXT_SUPPORT
	else if (strcasecmp("_",                                      szHandler) == 0) { return new FnGetText_();     }
	else if (strcasecmp("gettext",                                szHandler) == 0) { return new FnGetText();      }
#endif
#ifdef MD5_SUPPORT
	else if (strcasecmp("hmac_md5",                               szHandler) == 0) { return new FnHMACMD5();      }
#endif
	else if (strcasecmp("hash_element",                           szHandler) == 0) { return new FnHashElement();  }
	else if (strcasecmp("href_param",                             szHandler) == 0) { return new FnHrefParam();    }
	else if (strcasecmp("htmlescape",                             szHandler) == 0) { return new FnHTMLEscape();   }
#ifdef  ICONV_SUPPORT
	else if (strcasecmp("iconv",                                  szHandler) == 0) { return new FnIconv();        }
#endif
	else if (strcasecmp("in_set",                                 szHandler) == 0) { return new FnInSet();        }
	else if (strcasecmp("json",                                   szHandler) == 0) { return new FnJSON();         }
	else if (strcasecmp("jsescape",                               szHandler) == 0) { return new FnJSEscape();     }
	else if (strcasecmp("list_element",                           szHandler) == 0) { return new FnListElement();  }
	else if (strcasecmp("log",                                    szHandler) == 0) { return new FnLog();          }
#ifdef MD5_SUPPORT
	else if (strcasecmp("md5",                                    szHandler) == 0) { return new FnMD5();          }
#endif
	else if (strcasecmp("min",                                    szHandler) == 0) { return new FnMin();          }
	else if (strcasecmp("max",                                    szHandler) == 0) { return new FnMax();          }
	else if (strcasecmp("mb_size",                                szHandler) == 0) { return new FnMBSize();       }
	else if (strcasecmp("mb_substr",                              szHandler) == 0) { return new FnMBSubstring();  }
	else if (strcasecmp("mb_truncate",                            szHandler) == 0) { return new FnMBTruncate();   }
	else if (strcasecmp("num_format",                             szHandler) == 0) { return new FnNumFormat();    }
	else if (strcasecmp("numeral",                                szHandler) == 0) { return new FnNumeral();      }
	else if (strcasecmp("obj_dump",                               szHandler) == 0) { return new FnObjDump();      }
	else if (strcasecmp("random",                                 szHandler) == 0) { return new FnRandom();       }
	else if (strcasecmp("size",                                   szHandler) == 0) { return new FnSize();         }
	else if (strcasecmp("sprintf",                                szHandler) == 0) { return new FnSprintf();      }
	else if (strcasecmp("substr",                                 szHandler) == 0) { return new FnSubstring();    }
	else if (strcasecmp("truncate",                               szHandler) == 0) { return new FnTruncate();     }
	else if (strcasecmp("uriescape",                              szHandler) == 0) { return new FnURIEscape();    }
	else if (strcasecmp("urlescape",                              szHandler) == 0) { return new FnURLEscape();    }
	else if (strcasecmp("version",                                szHandler) == 0) { return new FnVersion();      }
	else if (strcasecmp("wmlescape",                              szHandler) == 0) { return new FnWMLEscape();    }
	else if (strcasecmp("xmlescape",                              szHandler) == 0) { return new FnXMLEscape();    }

#ifdef  PCRE_SUPPORT
//	else if (strcasecmp("re_m",                                   szHandler) == 0) { return new FnReM();          }
#endif
//	else if (strcasecmp("bb_code",                                szHandler) == 0) { return new FnBBCode();       }

return NULL;
}

//
// Destroy Handler
//
void STDLibInitializer::DestroyHandler(SyscallHandler * pHandler) { delete pHandler; }

//
// List of standard functions
//
CCHAR_P STDLibInitializer::aSTDFNList[] =
{
	CTPP2_INT_HANDLER_PREFIX "_emitter",
	"avg",
	"array_element",
	"base64_encode",
	"base64_decode",
	"cast",
	"concat",
	"date_format",
	"default",
	"defined",
	"error",
	"form_param",
#ifdef GETTEXT_SUPPORT
	"_",
	"gettext",
#endif
	"hash_element",
	"href_param",
#ifdef MD5_SUPPORT
	"hmac_md5",
#endif
	"htmlescape",
#ifdef  ICONV_SUPPORT
	"iconv",
#endif
	"in_set",
	"jsescape",
	"json",
	"list_element",
	"log",
#ifdef MD5_SUPPORT
	"md5",
#endif
	"min",
	"max",
	"mb_size",
	"mb_substr",
	"mb_truncate",
	"num_format",
	"numeral",
	"obj_dump",
	"random",
	"size",
	"sprintf",
	"substr",
	"truncate",
	"uriescape",
	"urlescape",
	"version",
	"xmlescape",
	"wmlescape",

	NULL
};

} // namespace CTPP
// End.

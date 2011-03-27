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
 *      CTPP2SysHeaders.h
 *
 * $CTPP$
 */
#ifndef _CTPP2_SYS_HEADERS_H__
#define _CTPP2_SYS_HEADERS_H__ 1

#define HAVE_SYS_TYPES_H     1

#define HAVE_SYS_TIME_H      1

#define HAVE_SYS_UIO_H       1

#define HAVE_FCNTL_H         1

#define HAVE_MATH_H          1

#define HAVE_STDIO_H         1

#define HAVE_STDLIB_H        1

#define HAVE_STRING_H        1

#define HAVE_STRINGS_H       0

#define HAVE_TIME_H          1

#define HAVE_UNISTD_H        1

#define HAVE_SYSEXITS_H      0

/* #undef DEBUG_MODE */

/* #undef NO_STL_STD_NS_PREFIX */

#define GETTEXT_SUPPORT      1

#undef MD5_SUPPORT         
#define MD5_WITHOUT_OPENSSL 1

#define CTPP_FLOAT_PRECISION   12

#define CTPP_ESCAPE_BUFFER_LEN 1024

#define CTPP_MAX_TEMPLATE_RECURSION_DEPTH 1024

#define ICONV_SUPPORT       0

#define ICONV_DISCARD_ILSEQ 1

#define ICONV_TRANSLITERATE 1

#define CTPP_VERSION         "2.6.7"
#define CTPP_IDENT           "Dzoraget"
#define CTPP_MASTER_SITE_URL "http://ctpp.havoc.ru/"

/* #undef THROW_EXCEPTION_IN_COMPARATORS */

#endif /* _CTPP2_SYS_HEADERS_H__ */
/* End. */

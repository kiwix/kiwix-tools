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
 *      CTPP2Dtoa.hpp
 *
 * $CTPP$
 */
#ifndef _CTPP2_DTOA_HPP__
#define _CTPP2_DTOA_HPP__ 1


#include "CTPP2Types.h"

struct Bigint
{
	struct Bigint  * next;
	INT_32           k;
	INT_32           maxwds;
	INT_32           sign;
	INT_32           wds;
	UINT_32          x[1];
};


struct AllocatedBlock
{
	void           * data;
	AllocatedBlock * prev;
};

char *ctpp_dtoa(AllocatedBlock ** aBlocks, Bigint ** freelist, double d, int mode, int ndigits, int *decpt, int *sign, char **rve);

void freedtoa(AllocatedBlock ** aBlocks);

#define Kmax (sizeof(size_t) << 3)

#define HAVE_PTHREAD_ATTR_GET_NP 1
#define HAVE_PTHREAD_GETATTR_NP 1
#define HAVE_PCREPOSIX 1
#define HAVE_SYS_TIMEB_H 1
#define HAVE_FLOAT_H 1
#define HAVE_IEEEFP_H 1
#define HAVE_MEMCHECK_H 1
#define HAVE_FUNC__FINITE 1
#define HAVE_FUNC_FINITE 1
#define HAVE_FUNC_ISINF 1
#define HAVE_FUNC_ISNAN 1
#define HAVE_FUNC_POSIX_MEMALIGN 1

#endif // _CTPP2_DTOA_HPP__
// End.

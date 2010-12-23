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
 *      CTPP2Dtoa.cpp
 *
 * $CTPP$
 */

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* Please send bug reports to
	David M. Gay
	Bell Laboratories, Room 2C-463
	600 Mountain Avenue
	Murray Hill, NJ 07974-0636
	U.S.A.
	dmg@bell-labs.com
 */

/* On a machine with IEEE extended-precision registers, it is
 * necessary to specify double-precision (53-bit) rounding precision
 * before invoking strtod or dtoa.  If the machine uses (the equivalent
 * of) Intel 80x87 arithmetic, the call
 *	_control87(PC_53, MCW_PC);
 * does this with many compilers.  Whether this or another call is
 * appropriate depends on the compiler; for this to work, it may be
 * necessary to #include "float.h" or another system-dependent header
 * file.
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */
#define Honor_FLT_ROUNDS
#define FLT_ROUNDS 3
/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address.
 * #define INT_32 int on machines with 32-bit ints and 64-bit longs.
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic (D_floating).
 * #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa.
 * #define Honor_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *	and strtod and dtoa should round accordingly.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *	and Honor_FLT_ROUNDS is not #defined.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *	that use extended-precision instructions to compute rounded
 *	products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860.
 * #define NO_LONG_LONG on machines that do not have a "INT_32 INT_32"
 *	integer type (of >= 64 bits).  On such machines, you can
 *	#define Just_16 to store 16 bits per 32-bit INT_32 when doing
 *	high-precision integer arithmetic.  Whether this speeds things
 *	up or slows things down depends on the machine and the number
 *	being converted.  If INT_32 INT_32 is available and the name is
 *	something other than "INT_32 INT_32", #define INT_64 to be the name,
 *	and if "unsigned INT_64" does not work as an unsigned version of
 *	INT_64, #define #UINT_64 to be the corresponding unsigned type.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *	define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *	FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *	if memory is available and otherwise does something you deem
 *	appropriate.  If MALLOC is undefined, malloc will be invoked
 *	directly -- and assumed always to succeed.
 * #define NO_IEEE_Scale to disable new (Feb. 1997) logic in strtod that
 *	avoids underflows on inputs whose result does not underflow.
 *	If you #define NO_IEEE_Scale on a machine that uses IEEE-format
 *	floating-point numbers and flushes underflows to zero rather
 *	than implementing gradual underflow, then you must also #define
 * #define YES_ALIAS to permit aliasing certain double values with
 *	arrays of ULongs.  This leads to slightly better code with
 *	some compilers and was always used prior to 19990916, but it
 *	is not strictly legal and can cause trouble with aggressively
 *	optimizing compilers (e.g., gcc 2.95.1 under -O2).
 * #define USE_LOCALE to use the current locale's decimal_point value.
 * #define NO_ERRNO if strtod should not assign errno = ERANGE when
 *	the result overflows to +-Infinity or underflows to 0.
 */

#include "CTPP2DTOA.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define PLATFORM(x) (BYTE_ORDER == (x))

#if PLATFORM(BIG_ENDIAN)
    #define IEEE_MC68k
#else
    #define IEEE_8087
#endif

#ifdef IEEE_Arith
    #undef IEEE_Arith
#endif

#ifdef Avoid_Underflow
    #undef Avoid_Underflow
#endif

#ifdef IEEE_MC68k
    #define IEEE_Arith
#endif

#ifdef IEEE_8087
    #define IEEE_Arith
#endif

#ifdef Bad_float_h

    #ifdef IEEE_Arith
        #define DBL_DIG        15
        #define DBL_MAX_10_EXP 308
        #define DBL_MAX_EXP    1024
        #define FLT_RADIX      2
    #endif /*IEEE_Arith*/

    #ifdef IBM
        #define DBL_DIG        16
        #define DBL_MAX_10_EXP 75
        #define DBL_MAX_EXP    63
        #define FLT_RADIX      16
        #define DBL_MAX        7.2370055773322621e+75
    #endif

    #ifdef VAX
        #define DBL_DIG        16
        #define DBL_MAX_10_EXP 38
        #define DBL_MAX_EXP    127
        #define FLT_RADIX      2
        #define DBL_MAX        1.7014118346046923e+38
    #endif

    #ifndef LONG_MAX
        #define LONG_MAX 2147483647
    #endif

#else /* ifndef Bad_float_h */
    #include <float.h>
#endif /* Bad_float_h */

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
    #error "Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined."
#endif

typedef union
{
	double  d;
	UINT_32 L[2];
} U;

#define dval(x) (x).d
#ifdef IEEE_8087
    #define word0(x) (x).L[1]
    #define word1(x) (x).L[0]
#else
    #define word0(x) (x).L[0]
    #define word1(x) (x).L[1]
#endif

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 */
#define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#ifdef IEEE_Arith
    #define Exp_shift   20
    #define Exp_shift1  20
    #define Exp_msk1    0x100000
    #define Exp_msk11   0x100000
    #define Exp_mask    0x7ff00000
    #define P           53
    #define Bias        1023
    #define Emin        (-1022)
    #define Exp_1       0x3ff00000
    #define Exp_11      0x3ff00000
    #define Ebits       11
    #define Frac_mask   0xfffff
    #define Frac_mask1  0xfffff
    #define Ten_pmax    22
    #define Bletch      0x10
    #define Bndry_mask  0xfffff
    #define Bndry_mask1 0xfffff
    #define LSB         1
    #define Sign_bit    0x80000000
    #define Log2P       1
    #define Tiny0       0
    #define Tiny1       1
    #define Quick_max   14
    #define Int_max     14
    #ifndef NO_IEEE_Scale
        #define Avoid_Underflow
    #endif

    #ifndef Flt_Rounds
        #ifdef FLT_ROUNDS
            #define Flt_Rounds FLT_ROUNDS
        #else
            #define Flt_Rounds 1
        #endif
    #endif /*Flt_Rounds*/

    #ifdef Honor_FLT_ROUNDS
        #define Rounding rounding
        #undef Check_FLT_ROUNDS
        #define Check_FLT_ROUNDS
    #else
        #define Rounding Flt_Rounds
    #endif

#else /* ifndef IEEE_Arith */
    #undef Check_FLT_ROUNDS
    #undef Honor_FLT_ROUNDS
    #ifdef IBM
        #undef Flt_Rounds
        #define Flt_Rounds  0
        #define Exp_shift   24
        #define Exp_shift1  24
        #define Exp_msk1    0x1000000
        #define Exp_msk11   0x1000000
        #define Exp_mask    0x7f000000
        #define P           14
        #define Bias        65
        #define Exp_1       0x41000000
        #define Exp_11      0x41000000
        #define Ebits       8 /* exponent has 7 bits, but 8 is the right value in b2d */
        #define Frac_mask   0xffffff
        #define Frac_mask1  0xffffff
        #define Bletch      4
        #define Ten_pmax    22
        #define Bndry_mask  0xefffff
        #define Bndry_mask1 0xffffff
        #define LSB         1
        #define Sign_bit    0x80000000
        #define Log2P       4
        #define Tiny0       0x100000
        #define Tiny1       0
        #define Quick_max   14
        #define Int_max     15
    #else /* VAX */
        #undef Flt_Rounds
        #define Flt_Rounds  1
        #define Exp_shift   23
        #define Exp_shift1  7
        #define Exp_msk1    0x80
        #define Exp_msk11   0x800000
        #define Exp_mask    0x7f80
        #define P           56
        #define Bias        129
        #define Exp_1       0x40800000
        #define Exp_11      0x4080
        #define Ebits       8
        #define Frac_mask   0x7fffff
        #define Frac_mask1  0xffff007f
        #define Ten_pmax    24
        #define Bletch      2
        #define Bndry_mask  0xffff007f
        #define Bndry_mask1 0xffff007f
        #define LSB         0x10000
        #define Sign_bit    0x8000
        #define Log2P       1
        #define Tiny0       0x80
        #define Tiny1       0
        #define Quick_max   15
        #define Int_max     15
    #endif /* IBM, VAX */
#endif /* IEEE_Arith */

#ifndef IEEE_Arith
    #define ROUND_BIASED
#endif

#define Big0     (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1     0xffffffff
#define FFFFFFFF 0xffffffffUL

#define ACQUIRE_DTOA_LOCK(n)
#define FREE_DTOA_LOCK(n)

void * safe_malloc(AllocatedBlock ** aBlocks, const UINT_32 iSize)
{
	// Allocate memory block placeholder
	AllocatedBlock * pNewBlock = (AllocatedBlock *)malloc(sizeof(AllocatedBlock));
	// Allocate memory block
	pNewBlock -> data = malloc(iSize);
	pNewBlock -> prev = *aBlocks;

	// Ouch!
	*aBlocks = pNewBlock;
//fprintf(stderr, "safe_malloc: %p(%p, %d)\n", (void*)pNewBlock, (void*)(pNewBlock -> data), iSize);

return pNewBlock -> data;
}

void safe_free(AllocatedBlock ** aBlocks)
{
	// Ouch!
	if (aBlocks == NULL || *aBlocks == NULL) { return; }

	for(;;)
	{
		AllocatedBlock * pPrevBlock = (*aBlocks) -> prev;

//fprintf(stderr, "safe_free: %p(%p)\n", (void*)(*aBlocks), (*aBlocks) -> data);

		free((*aBlocks) -> data);
		free(*aBlocks);

		*aBlocks = pPrevBlock;
		if (pPrevBlock == NULL) { break; }
	}
}

static Bigint * Balloc(AllocatedBlock ** aBlocks, Bigint ** freelist, int k)
{
	int x;
	Bigint *rv;

	ACQUIRE_DTOA_LOCK(0);
	if ((rv = freelist[k]))
	{
		freelist[k] = rv->next;
	}
	else
	{
		x = 1 << k;
		rv = (Bigint *)safe_malloc(aBlocks, sizeof(Bigint) + (x-1)*sizeof(UINT_32));
		rv->k = k;
		rv->maxwds = x;
	}
	FREE_DTOA_LOCK(0);
	rv->sign = rv->wds = 0;

return rv;
}

#define Kmax (sizeof(size_t) << 3)

static void Bfree(Bigint ** freelist, Bigint *v)
{
	if (v)
	{
//		if (v->k > Kmax)
//		{
//			free((void*)v);
//		}
//		else
		{
			ACQUIRE_DTOA_LOCK(0);
			v->next = freelist[v->k];
			freelist[v->k] = v;
			FREE_DTOA_LOCK(0);
		}
	}
}

/*static void Bfree(Bigint ** freelist, Bigint *v)
{
	if (v)
	{
		ACQUIRE_DTOA_LOCK(0);
		v->next = freelist[v->k];
		freelist[v->k] = v;
		FREE_DTOA_LOCK(0);
	}
}
*/
#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, y->wds*sizeof(INT_32) + 2*sizeof(int))

//
// multiply by m and add a
//
static Bigint * multadd(AllocatedBlock ** aBlocks, Bigint ** freelist, Bigint *b, int m, int a)
{
	int i, wds;

	UINT_32 *x;
	UINT_64 carry, y;
	Bigint *b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	carry = a;
	do
	{
		y = *x * (UINT_64)m + carry;
		carry = y >> 32;
		*x++ = (UINT_32)y & FFFFFFFF;
	}
	while(++i < wds);

	if (carry)
	{
		if (wds >= b->maxwds)
		{
			b1 = Balloc(aBlocks, freelist, b->k+1);
			Bcopy(b1, b);
			Bfree(freelist, b);
			b = b1;
		}
		b->x[wds++] = (UINT_32)carry;
		b->wds = wds;
	}

return b;
}


static int hi0bits(register UINT_32 x)
{
	register int k = 0;

	if (!(x & 0xffff0000))
	{
		k = 16;
		x <<= 16;
	}
	if (!(x & 0xff000000))
	{
		k += 8;
		x <<= 8;
	}
	if (!(x & 0xf0000000))
	{
		k += 4;
		x <<= 4;
	}
	if (!(x & 0xc0000000))
	{
		k += 2;
		x <<= 2;
	}
	if (!(x & 0x80000000))
	{
		k++;
		if (!(x & 0x40000000))
		{
			return 32;
		}
	}

return k;
}

static int lo0bits(UINT_32 *y)
{
	register int k;
	register UINT_32 x = *y;

	if (x & 7)
	{
		if (x & 1)
		{
			return 0;
		}
		if (x & 2)
		{
			*y = x >> 1;
			return 1;
		}
		*y = x >> 2;
		return 2;
	}

	k = 0;
	if (!(x & 0xffff))
	{
		k = 16;
		x >>= 16;
	}
	if (!(x & 0xff))
	{
		k += 8;
		x >>= 8;
	}
	if (!(x & 0xf))
	{
		k += 4;
		x >>= 4;
	}
	if (!(x & 0x3))
	{
		k += 2;
		x >>= 2;
	}
	if (!(x & 1))
	{
		k++;
		x >>= 1;
		if (!x & 1)
		{
			return 32;
		}
	}
	*y = x;
return k;
}

static Bigint * i2b(AllocatedBlock ** aBlocks, Bigint ** freelist, int i)
{
	Bigint *b;

	b = Balloc(aBlocks, freelist, 1);
	b->x[0] = i;
	b->wds = 1;

return b;
}

static Bigint * mult(AllocatedBlock ** aBlocks, Bigint ** freelist, Bigint *a, Bigint *b)
{
	Bigint *c;
	int k, wa, wb, wc;
	UINT_32 *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
	UINT_32 y;
	UINT_64 carry, z;

	if (a->wds < b->wds)
	{
		c = a;
		a = b;
		b = c;
	}

	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;
	if (wc > a->maxwds) { k++; }
	c = Balloc(aBlocks, freelist, k);
	for(x = c->x, xa = x + wc; x < xa; x++) { *x = 0; }

	xa  = a->x;
	xae = xa + wa;

	xb  = b->x;
	xbe = xb + wb;

	xc0 = c->x;

	for(; xb < xbe; xc0++)
	{
		if ((y = *xb++))
		{
			x = xa;
			xc = xc0;
			carry = 0;
			do
			{
				z = *x++ * (UINT_64)y + *xc + carry;
				carry = z >> 32;
				*xc++ = (UINT_32)z & FFFFFFFF;
			}
			while(x < xae);
			*xc = (UINT_32)carry;
		}
	}

	for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) { ;; }

	c->wds = wc;

return c;
}

//static Bigint *p5s;

static Bigint * pow5mult(AllocatedBlock ** aBlocks, Bigint ** freelist, Bigint *b, int k)
{
	Bigint *b1, *p5, *p51;
	int i;
	static int p05[3] = { 5, 25, 125 };

	if ((i = k & 3)) { b = multadd(aBlocks, freelist, b, p05[i-1], 0); }

	if (!(k >>= 2)) { return b; }
//	if (!(p5 = p5s))
	{
		/* first time */
		ACQUIRE_DTOA_LOCK(1);
//		if (!(p5 = p5s))
		{
			p5 /* = p5s*/ = i2b(aBlocks, freelist, 625);
			p5->next = 0;
		}
		FREE_DTOA_LOCK(1);
	}

	for(;;)
	{
		if (k & 1)
		{
			b1 = mult(aBlocks, freelist, b, p5);
			Bfree(freelist, b);
			b = b1;
		}
		if (!(k >>= 1)) { break; }

		if (!(p51 = p5->next))
		{
			ACQUIRE_DTOA_LOCK(1);
			if (!(p51 = p5->next))
			{
				p51 = p5->next = mult(aBlocks, freelist, p5,p5);
				p51->next = 0;
			}
			FREE_DTOA_LOCK(1);
		}
		p5 = p51;
	}
return b;
}

static Bigint * lshift(AllocatedBlock ** aBlocks, Bigint ** freelist, Bigint *b, int k)
{
	int i, k1, n, n1;
	Bigint *b1;
	UINT_32 *x, *x1, *xe, z;

	n = k >> 5;
	k1 = b->k;
	n1 = n + b->wds + 1;
	for(i = b->maxwds; n1 > i; i <<= 1) { k1++; }

	b1 = Balloc(aBlocks, freelist, k1);
	x1 = b1->x;
	for(i = 0; i < n; i++) { *x1++ = 0; }

	x = b->x;
	xe = x + b->wds;

	if (k &= 0x1f)
	{
		k1 = 32 - k;
		z = 0;
		do
		{
			*x1++ = *x << k | z;
			z = *x++ >> k1;
		}
		while(x < xe);
		if ((*x1 = z)) { ++n1; }
	}
	else
	{
		do { *x1++ = *x++; } while(x < xe);
	}
	b1->wds = n1 - 1;
	Bfree(freelist, b);

return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
	UINT_32 *xa, *xa0, *xb, *xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
#ifdef DEBUG
	if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
	if (i -= j) { return i; }
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for(;;)
	{
		if (*--xa != *--xb)
		{
			return *xa < *xb ? -1 : 1;
		}
		if (xa <= xa0)
		{
			break;
		}
	}

return 0;
}

static Bigint * diff(AllocatedBlock ** aBlocks, Bigint ** freelist, Bigint *a, Bigint *b)
{
	Bigint *c;
	int i, wa, wb;
	UINT_32 *xa, *xae, *xb, *xbe, *xc;

	UINT_64 borrow, y;

	i = cmp(a,b);
	if (!i) {
		c = Balloc(aBlocks, freelist, 0);
		c->wds = 1;
		c->x[0] = 0;
		return c;
		}
	if (i < 0) {
		c = a;
		a = b;
		b = c;
		i = 1;
		}
	else
		i = 0;
	c = Balloc(aBlocks, freelist, a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;

	do
	{
		y = (UINT_64)*xa++ - *xb++ - borrow;
		borrow = y >> 32 & (UINT_32)1;
		*xc++ = (UINT_32)y & FFFFFFFF;
	}
	while(xb < xbe);
	while(xa < xae)
	{
		y = *xa++ - borrow;
		borrow = y >> 32 & (UINT_32)1;
		*xc++ = (UINT_32)y & FFFFFFFF;
	}

	while(!*--xc)
	{
		wa--;
	}
	c->wds = wa;

return c;
}

static Bigint * d2b(AllocatedBlock ** aBlocks, Bigint ** freelist, double dd, int *e, int *bits)
{
	U d;
	Bigint *b;
	int de, k;
	UINT_32 *x, y, z;
	int i;
#ifdef VAX
	UINT_32 d0, d1;
#endif
	dval(d) = dd;
#ifdef VAX
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
    #define d0 word0(d)
    #define d1 word1(d)
#endif
	b = Balloc(aBlocks, freelist, 1);
	x = b->x;

	z = d0 & Frac_mask;
	d0 &= 0x7fffffff;	/* clear sign bit, which we ignore */

	if ((de = (int)(d0 >> Exp_shift))) { z |= Exp_msk1; }

	if ((y = d1))
	{
		if ((k = lo0bits(&y)))
		{
			x[0] = y | z << 32 - k;
			z >>= k;
		}
		else
		{
			x[0] = y;
		}
		i = b->wds = (x[1] = z) ? 2 : 1;
	}
	else
	{
    #ifdef DEBUG
		if (!z)
			Bug("Zero passed to d2b");
    #endif
		k = lo0bits(&z);
		x[0] = z;
		i = b->wds = 1;
		k += 32;
	}

	if (de)
	{
#ifdef IBM
		*e = (de - Bias - (P-1) << 2) + k;
		*bits = 4*P + 8 - k - hi0bits(word0(d) & Frac_mask);
#else
		*e = de - Bias - (P-1) + k;
		*bits = P - k;
#endif
	}
	else
	{
		*e = de - Bias - (P-1) + 1 + k;

		*bits = 32*i - hi0bits(x[i-1]);
	}

return b;
}

#undef d0
#undef d1


static const double tens[] =
{
	1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
	1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
	1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
#ifdef VAX
		, 1e23, 1e24
#endif
};

static const double
#ifdef IEEE_Arith
	bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
	static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128,
#ifdef Avoid_Underflow
		9007199254740992.*9007199254740992.e-256
		/* = 2^106 * 1e-53 */
#else
		1e-256
#endif
	};
/* The factor of 2^53 in tinytens[4] helps us avoid setting the underflow */
/* flag unnecessarily.  It leads to a song and dance at the end of strtod. */
#define Scale_Bit 0x10
    #define n_bigtens 5
#else
    #ifdef IBM
        bigtens[] = { 1e16, 1e32, 1e64 };
        static const double tinytens[] = { 1e-16, 1e-32, 1e-64 };
        #define n_bigtens 3
    #else
        bigtens[] = { 1e16, 1e32 };
        static const double tinytens[] = { 1e-16, 1e-32 };
        #define n_bigtens 2
    #endif
#endif

static int quorem(Bigint *b, Bigint *S)
{
	int n;
	UINT_32 *bx, *bxe, q, *sx, *sxe;

	UINT_64 borrow, carry, y, ys;

	n = S->wds;
#ifdef DEBUG
	/*debug*/ if (b->wds > n)
	/*debug*/	Bug("oversize b in quorem");
#endif
	if (b->wds < n)
		return 0;
	sx = S->x;
	sxe = sx + --n;
	bx = b->x;
	bxe = bx + n;
	q = *bxe / (*sxe + 1);	/* ensure q <= true quotient */
#ifdef DEBUG
	/*debug*/ if (q > 9)
	/*debug*/	Bug("oversized quotient in quorem");
#endif
	if (q) {
		borrow = 0;
		carry = 0;
		do
		{
			ys = *sx++ * (UINT_64)q + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (UINT_32)1;
			*bx++ = (UINT_32)y & FFFFFFFF;
		}
		while(sx <= sxe);

		if (!*bxe)
		{
			bx = b->x;
			while(--bxe > bx && !*bxe)
				--n;
			b->wds = n;
			}
		}
	if (cmp(b, S) >= 0)
	{
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do
		{
			ys = *sx++ + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (UINT_32)1;
			*bx++ = (UINT_32)y & FFFFFFFF;
		}
		while(sx <= sxe);

		bx = b->x;
		bxe = bx + n;
		if (!*bxe)
		{
			while(--bxe > bx && !*bxe) { --n; }
			b->wds = n;
		}
	}

return q;
}

static char * rv_alloc(AllocatedBlock ** aBlocks, Bigint ** freelist, int i)
{
	int j, k, *r;

	j = sizeof(UINT_32);
	for(k = 0; sizeof(Bigint) - sizeof(UINT_32) - sizeof(int) + j <= (unsigned)i; j <<= 1) { k++; }
	r = (int*)Balloc(aBlocks, freelist, k);
	*r = k;
	return (char *)(r+1);
}

static char * nrv_alloc(AllocatedBlock ** aBlocks, Bigint ** freelist, const char *s, char **rve, int n)
{
	char *rv, *t;

	t = rv = rv_alloc(aBlocks, freelist, n);
	while((*t = *s++)) { t++; }
	if (rve) { *rve = t; }
	return rv;
}

void freedtoa(AllocatedBlock ** aBlocks)
{
	safe_free(aBlocks);
}

/*void freedtoa(Bigint ** freelist, char *s)
{
	Bigint *b = (Bigint *)((int *)s - 1);
	b->maxwds = 1 << (b->k = *(int*)b);
	Bfree(freelist, b);
}
*/
/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *	1. Rather than iterating, we use a simple numeric overestimate
 *	   to determine k = floor(log10(d)).  We scale relevant
 *	   quantities using O(log2(k)) rather than O(k) multiplications.
 *	2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *	   try to generate digits strictly left to right.  Instead, we
 *	   compute with fewer bits and propagate the carry if necessary
 *	   when rounding the final digit up.  This is often faster.
 *	3. Under the assumption that input will be rounded nearest,
 *	   mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *	   That is, we allow equality in stopping tests when the
 *	   round-nearest rule will give the same floating-point value
 *	   as would satisfaction of the stopping test with strict
 *	   inequality.
 *	4. We remove common factors of powers of 2 from relevant
 *	   quantities.
 *	5. When converting floating-point integers less than 1e16,
 *	   we use floating-point arithmetic rather than resorting
 *	   to multiple-precision integers.
 *	6. When asked to produce fewer than 15 digits, we first try
 *	   to get by with floating-point arithmetic; we resort to
 *	   multiple-precision integer arithmetic only if we cannot
 *	   guarantee that the floating-point calculation has given
 *	   the correctly rounded result.  For k requested digits and
 *	   "uniformly" distributed input, the probability is
 *	   something like 10^(k-15) that we must resort to the INT_32
 *	   calculation.
 */

char * ctpp_dtoa(AllocatedBlock ** aBlocks, Bigint ** freelist, double dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
 /*	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4,5 ==> similar to 2 and 3, respectively, but (in
			round-nearest mode) with the tests of mode 0 to
			possibly return a shorter string that rounds to d.
			With IEEE arithmetic and compilation with
			-DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
			as modes 2 and 3 when FLT_ROUNDS != 1.
		6-9 ==> Debugging modes similar to mode - 4:  don't try
			fast floating-point estimate (if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

	INT_32 bbits, b2, b5, be, dig, i, ieps, ilim = 0, ilim0, ilim1 = 0, j, j1, k, k0, k_check, leftright, m2, m5, s2, s5, spec_case, try_quick;
	INT_32 L;

	INT_32 denorm;
	UINT_32 x;

	Bigint *b, *b1, *delta, *mlo = NULL, *mhi, *S;
	U d, d2, eps;
	double ds;
	char *s, *s0;
#ifdef Honor_FLT_ROUNDS
	int rounding;
#endif

	dval(d) = dd;
	if (word0(d) & Sign_bit)
	{
		/* set sign for everything, including 0's and NaNs */
		*sign = 1;
		word0(d) &= ~Sign_bit;	/* clear sign bit */
	}
	else
	{
		*sign = 0;
	}

#if defined(IEEE_Arith) + defined(VAX)
    #ifdef IEEE_Arith
	if ((word0(d) & Exp_mask) == Exp_mask)
    #else
	if (word0(d)  == 0x8000)
    #endif
	{
		/* Infinity or NaN */
		*decpt = 9999;
    #ifdef IEEE_Arith
		if (!word1(d) && !(word0(d) & 0xfffff)) { return nrv_alloc(aBlocks, freelist, "Infinity", rve, 8); }
    #endif
		return nrv_alloc(aBlocks, freelist, "NaN", rve, 3);
	}
#endif

#ifdef IBM
	dval(d) += 0; /* normalize */
#endif
	if (!dval(d))
	{
		*decpt = 1;
		return nrv_alloc(aBlocks, freelist, "0", rve, 1);
	}

#ifdef Honor_FLT_ROUNDS
	if ((rounding = Flt_Rounds) >= 2)
	{
		if (*sign)
		{
			rounding = (rounding == 2) ? 0 : 2;
		}
		else
		{
			if (rounding != 2)
			{
				rounding = 0;
			}
		}
	}
#endif

	b = d2b(aBlocks, freelist, dval(d), &be, &bbits);

	if ((i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1))))
	{
		dval(d2) = dval(d);
		word0(d2) &= Frac_mask1;
		word0(d2) |= Exp_11;
#ifdef IBM
		if (j = 11 - hi0bits(word0(d2) & Frac_mask)) { dval(d2) /= 1 << j; }
#endif

		/* log(x)	~=~ log(1.5) + (x-1.5)/1.5
		 * log10(x)	 =  log(x) / log(10)
		 *		~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
		 * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
		 *
		 * This suggests computing an approximation k to log10(d) by
		 *
		 * k = (i - Bias)*0.301029995663981
		 *	+ ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
		 *
		 * We want k to be too large rather than too small.
		 * The error in the first-order Taylor series approximation
		 * is in our favor, so we just round up the constant enough
		 * to compensate for any error in the multiplication of
		 * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
		 * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
		 * adding 1e-13 to the constant term more than suffices.
		 * Hence we adjust the constant term to 0.1760912590558.
		 * (We could get a more accurate k by invoking log10,
		 *  but this is probably not worthwhile.)
		 */

		i -= Bias;
#ifdef IBM
		i <<= 2;
		i += j;
#endif
		denorm = 0;
	}
	else
	{
		/* d is denormalized */

		i = bbits + be + (Bias + (P-1) - 1);
		x = i > 32  ? word0(d) << 64 - i | word1(d) >> i - 32
			    : word1(d) << 32 - i;
		dval(d2) = x;
		word0(d2) -= 31*Exp_msk1; /* adjust exponent */
		i -= (Bias + (P-1) - 1) + 1;
		denorm = 1;
	}

	ds = (dval(d2)-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
	k = (int)ds;
	if (ds < 0. && ds != k)
		k--;	/* want k = floor(ds) */
	k_check = 1;
	if (k >= 0 && k <= Ten_pmax)
	{
		if (dval(d) < tens[k]) { k--; }
		k_check = 0;
	}
	j = bbits - i - 1;

	if (j >= 0)
	{
		b2 = 0;
		s2 = j;
	}
	else
	{
		b2 = -j;
		s2 = 0;
	}

	if (k >= 0)
	{
		b5 = 0;
		s5 = k;
		s2 += k;
	}
	else
	{
		b2 -= k;
		b5 = -k;
		s5 = 0;
	}

	if (mode < 0 || mode > 9) { mode = 0; }

#ifdef Check_FLT_ROUNDS
	try_quick = Rounding == 1;
#else
	try_quick = 1;
#endif

	if (mode > 5)
	{
		mode -= 4;
		try_quick = 0;
	}
	leftright = 1;
	switch(mode)
	{
		case 0:
		case 1:
			ilim = ilim1 = -1;
			i = 18;
			ndigits = 0;
			break;
		case 2:
			leftright = 0;
			/* no break */
		case 4:
			if (ndigits <= 0)
				ndigits = 1;
			ilim = ilim1 = i = ndigits;
			break;
		case 3:
			leftright = 0;
			/* no break */
		case 5:
			i = ndigits + k + 1;
			ilim = i;
			ilim1 = i - 1;
			if (i <= 0)
				i = 1;
	}
	s = s0 = rv_alloc(aBlocks, freelist, i);

#ifdef Honor_FLT_ROUNDS
	if (mode > 1 && rounding != 1) { leftright = 0; }
#endif

	if (ilim >= 0 && ilim <= Quick_max && try_quick)
	{
		/* Try to get by with floating-point arithmetic. */

		i = 0;
		dval(d2) = dval(d);
		k0 = k;
		ilim0 = ilim;
		ieps = 2; /* conservative */
		if (k > 0)
		{
			ds = tens[k&0xf];
			j = k >> 4;
			if (j & Bletch)
			{
				/* prevent overflows */
				j &= Bletch - 1;
				dval(d) /= bigtens[n_bigtens-1];
				ieps++;
			}

			for(; j; j >>= 1, i++)
			{
				if (j & 1)
				{
					ieps++;
					ds *= bigtens[i];
				}
			}

			dval(d) /= ds;
		}
		else if ((j1 = -k))
		{
			dval(d) *= tens[j1 & 0xf];
			for(j = j1 >> 4; j; j >>= 1, i++)
			{
				if (j & 1)
				{
					ieps++;
					dval(d) *= bigtens[i];
				}
			}
		}
		if (k_check && dval(d) < 1. && ilim > 0)
		{
			if (ilim1 <= 0) { goto fast_failed; }
			ilim = ilim1;
			k--;
			dval(d) *= 10.;
			ieps++;
		}
		dval(eps) = ieps*dval(d) + 7.;
		word0(eps) -= (P-1)*Exp_msk1;
		if (ilim == 0)
		{
			S = mhi = 0;
			dval(d) -= 5.;
			if (dval(d) > dval(eps)) { goto one_digit; }
			if (dval(d) < -dval(eps)) { goto no_digits; }
			goto fast_failed;
		}
#ifndef No_leftright
		if (leftright)
		{
			/* Use Steele & White method of only
			 * generating digits needed.
			 */
			dval(eps) = 0.5/tens[ilim-1] - dval(eps);
			for(i = 0;;) {
				L = (INT_32)dval(d);
				dval(d) -= L;
				*s++ = '0' + (int)L;
				if (dval(d) < dval(eps))
					goto ret1;
				if (1. - dval(d) < dval(eps))
					goto bump_up;
				if (++i >= ilim)
					break;
				dval(eps) *= 10.;
				dval(d) *= 10.;
				}
		}
		else
		{
#endif
			/* Generate ilim digits, then fix them up. */
			dval(eps) *= tens[ilim-1];
			for(i = 1;; i++, dval(d) *= 10.)
			{
				L = (INT_32)(dval(d));
				if (!(dval(d) -= L))
				{
					ilim = i;
				}
				*s++ = '0' + (int)L;
				if (i == ilim)
				{
					if (dval(d) > 0.5 + dval(eps))
					{
						goto bump_up;
					}
					else if (dval(d) < 0.5 - dval(eps))
					{
						while(*--s == '0')
						{
							;
						}
						s++;
						goto ret1;
					}
					break;
				}
			}
#ifndef No_leftright
		}
#endif
fast_failed:
		s = s0;
		dval(d) = dval(d2);
		k = k0;
		ilim = ilim0;
	}

	/* Do we have a "small" integer? */

	if (be >= 0 && k <= Int_max)
	{
		/* Yes. */
		ds = tens[k];
		if (ndigits < 0 && ilim <= 0)
		{
			S = mhi = 0;
			if (ilim < 0 || dval(d) <= 5*ds)
			{
				goto no_digits;
			}
			goto one_digit;
		}
		for(i = 1;; i++, dval(d) *= 10.)
		{
			L = (INT_32)(dval(d) / ds);
			dval(d) -= L*ds;
#ifdef Check_FLT_ROUNDS
			/* If FLT_ROUNDS == 2, L will usually be high by 1 */
			if (dval(d) < 0) {
				L--;
				dval(d) += ds;
				}
#endif
			*s++ = '0' + (int)L;
			if (!dval(d)) {
				break;
				}
			if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
				if (mode > 1)
				switch(rounding) {
				  case 0: goto ret1;
				  case 2: goto bump_up;
				  }
#endif
				dval(d) += dval(d);
				if (dval(d) > ds || dval(d) == ds && L & 1)
				{
bump_up:
					while(*--s == '9')
					{
						if (s == s0)
						{
							k++;
							*s = '0';
							break;
						}
					}
					++*s++;
				}
				break;
			}
		}
		goto ret1;
	}

	m2 = b2;
	m5 = b5;
	mhi = mlo = 0;
	if (leftright)
	{
		i =
			denorm ? be + (Bias + (P-1) - 1 + 1) :
#ifdef IBM
			1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
			1 + P - bbits;
#endif
		b2 += i;
		s2 += i;
		mhi = i2b(aBlocks, freelist, 1);
	}

	if (m2 > 0 && s2 > 0)
	{
		i = m2 < s2 ? m2 : s2;
		b2 -= i;
		m2 -= i;
		s2 -= i;
	}
	if (b5 > 0)
	{
		if (leftright)
		{
			if (m5 > 0)
			{
				mhi = pow5mult(aBlocks, freelist, mhi, m5);
				b1 = mult(aBlocks, freelist, mhi, b);
				Bfree(freelist, b);
				b = b1;
			}
			if ((j = b5 - m5))
			{
				b = pow5mult(aBlocks, freelist, b, j);
			}
		}
		else
		{
			b = pow5mult(aBlocks, freelist, b, b5);
		}
	}
	S = i2b(aBlocks, freelist, 1);
	if (s5 > 0)
	{
		S = pow5mult(aBlocks, freelist, S, s5);
	}

	/* Check for special case that d is a normalized power of 2. */

	spec_case = 0;
	if ((mode < 2 || leftright)
#ifdef Honor_FLT_ROUNDS
			&& rounding == 1
#endif
				)
	{
		if (!word1(d) && !(word0(d) & Bndry_mask) && word0(d) & (Exp_mask & ~Exp_msk1)
				)
		{
			/* The special case */
			b2 += Log2P;
			s2 += Log2P;
			spec_case = 1;
		}
	}

	/* Arrange for convenient computation of quotients:
	 * shift left if necessary so divisor has 4 leading 0 bits.
	 *
	 * Perhaps we should just compute leading 28 bits of S once
	 * and for all and pass them and a shift to quorem, so it
	 * can do shifts and ors to compute the numerator for q.
	 */
	if ((i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0x1f))
	{
		i = 32 - i;
	}

	if (i > 4)
	{
		i -= 4;
		b2 += i;
		m2 += i;
		s2 += i;
	}
	else if (i < 4)
	{
		i += 28;
		b2 += i;
		m2 += i;
		s2 += i;
	}

	if (b2 > 0)
	{
		b = lshift(aBlocks, freelist, b, b2);
	}

	if (s2 > 0)
	{
		S = lshift(aBlocks, freelist, S, s2);
	}

	if (k_check)
	{
		if (cmp(b,S) < 0)
		{
			k--;
			b = multadd(aBlocks, freelist, b, 10, 0);	/* we botched the k estimate */
			if (leftright) { mhi = multadd(aBlocks, freelist, mhi, 10, 0); }
			ilim = ilim1;
		}
	}
	if (ilim <= 0 && (mode == 3 || mode == 5))
	{
		if (ilim < 0 || cmp(b,S = multadd(aBlocks, freelist, S,5,0)) <= 0)
		{
			/* no digits, fcvt style */
 no_digits:
			k = -1 - ndigits;
			goto ret;
		}
 one_digit:
		*s++ = '1';
		k++;
		goto ret;
	}
	if (leftright)
	{
		if (m2 > 0)
		{
			mhi = lshift(aBlocks, freelist, mhi, m2);
		}

		/* Compute mlo -- check for special case
		 * that d is a normalized power of 2.
		 */

		mlo = mhi;
		if (spec_case)
		{
			mhi = Balloc(aBlocks, freelist, mhi->k);
			Bcopy(mhi, mlo);
			mhi = lshift(aBlocks, freelist, mhi, Log2P);
		}

		for(i = 1;;i++)
		{
			dig = quorem(b,S) + '0';
			/* Do we yet have the shortest decimal string
			 * that will round to d?
			 */
			j = cmp(b, mlo);
			delta = diff(aBlocks, freelist, S, mhi);
			j1 = delta->sign ? 1 : cmp(b, delta);
			Bfree(freelist, delta);
#ifndef ROUND_BIASED
			if (j1 == 0 && mode != 1 && !(word1(d) & 1)
    #ifdef Honor_FLT_ROUNDS
				&& rounding >= 1
    #endif
			)
			{
				if (dig == '9')
					goto round_9_up;
				if (j > 0)
					dig++;
				*s++ = dig;
				goto ret;
				}
#endif
			if (j < 0 || j == 0 && mode != 1
#ifndef ROUND_BIASED
							&& !(word1(d) & 1)
#endif
				)
			{
				if (!b->x[0] && b->wds <= 1)
				{
					goto accept_dig;
				}
#ifdef Honor_FLT_ROUNDS
				if (mode > 1)
				{
					switch(rounding)
					{
						case 0: goto accept_dig;
						case 2: goto keep_dig;
					}
				}
#endif /*Honor_FLT_ROUNDS*/
				if (j1 > 0)
				{
					b = lshift(aBlocks, freelist, b, 1);
					j1 = cmp(b, S);
					if ((j1 > 0 || j1 == 0 && dig & 1) && dig++ == '9') { goto round_9_up; }
				}
 accept_dig:
				*s++ = dig;
				goto ret;
			}
			if (j1 > 0)
			{
#ifdef Honor_FLT_ROUNDS
				if (!rounding) { goto accept_dig; }
#endif
				if (dig == '9')
				{
					/* possible if i == 1 */
round_9_up:
					*s++ = '9';
					goto roundoff;
				}

				*s++ = dig + 1;
				goto ret;
			}
#ifdef Honor_FLT_ROUNDS
keep_dig:
#endif
			*s++ = dig;
			if (i == ilim)
			{
				break;
			}

			b = multadd(aBlocks, freelist, b, 10, 0);

			if (mlo == mhi)
			{
				mlo = mhi = multadd(aBlocks, freelist, mhi, 10, 0);
			}
			else
			{
				mlo = multadd(aBlocks, freelist, mlo, 10, 0);
				mhi = multadd(aBlocks, freelist, mhi, 10, 0);
			}
		}
	}
	else
	{
		for(i = 1;; i++)
		{
			*s++ = dig = quorem(b,S) + '0';
			if (!b->x[0] && b->wds <= 1)
			{
				goto ret;
			}
			if (i >= ilim)
			{
				break;
			}
			b = multadd(aBlocks, freelist, b, 10, 0);
		}
	}
	/* Round off last digit */
#ifdef Honor_FLT_ROUNDS
	switch(rounding)
	{
		case 0: goto trimzeros;
		case 2: goto roundoff;
	}
#endif
	b = lshift(aBlocks, freelist, b, 1);
	j = cmp(b, S);
	if (j > 0 || j == 0 && dig & 1)
	{
roundoff:
		while(*--s == '9')
		{
			if (s == s0)
			{
				k++;
				*s++ = '1';
				goto ret;
			}
		}
		++*s++;
	}
	else
	{
#ifdef Honor_FLT_ROUNDS
trimzeros:
#endif
		while(*--s == '0') { ;; }
		s++;
	}
 ret:
	Bfree(freelist, S);
	if (mhi)
	{
		if (mlo && mlo != mhi) { Bfree(freelist, mlo); }
		Bfree(freelist, mhi);
	}
 ret1:
	Bfree(freelist, b);
	*s = 0;
	*decpt = k + 1;
	if (rve) { *rve = s; }
return s0;
}

// End.

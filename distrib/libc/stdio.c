#include <xv6/user.h>
#include <stdio.h>
#include <float.h>
#include <stdarg.h>
#include <stddef.h>

FILE _std_files[3] = {
        {0},
        {1},
        {2},
};

#define        MAXEXP          308
#define        MAXFRACT        39

#define        BUF             (MAXEXP+MAXFRACT+1)     /* + decimal point */
#define        DEFPREC         6

#define        to_digit(c)     ((c) - '0')
#define is_digit(c)    ((unsigned)to_digit(c) <= 9)
#define        to_char(n)      ((n) + '0')

#define        ALT             0x001           /* alternate form */
#define        HEXPREFIX       0x002           /* add 0x or 0X prefix */
#define        LADJUST         0x004           /* left adjustment */
#define        LONGDBL         0x008           /* long double; unimplemented */
#define        LONGINT         0x010           /* long integer */
#define        QUADINT         0x020           /* quad integer */
#define        SHORTINT        0x040           /* short integer */
#define        ZEROPAD         0x080           /* zero (as opposed to blank) pad */
#define FPT            0x100           /* Floating point number */
#define	HUGE_VAL	1.701411834604692294E+38

struct __siov {
    void	*iov_base;
    unsigned int	iov_len;
};
struct __suio {
    struct	__siov *uio_iov;
    int	uio_iovcnt;
    int	uio_resid;
};

int __sfvwrite(register FILE *fp, register struct __suio * uio)
{
    register unsigned int len;
    register char *p;
    register struct __siov *iov;
    register int w;
    iov = uio->uio_iov;
    p = iov->iov_base;
    len = iov->iov_len;
    iov++;
#define GETIOV(extra_work) \
	while (len == 0) { \
		extra_work; \
		p = iov->iov_base; \
		len = iov->iov_len; \
		iov++; \
	}
    do {
        GETIOV(;);
        w = write(fp->fd, p, len);
        if (w <= 0)
            goto err;
        p += w;
        len -= w;
    } while ((uio->uio_resid -= w) != 0);
    return (0);

    err:
    return EOF;
}

static int __sprint(FILE *fp, register struct __suio *uio)
{
    register int err;

    if (uio->uio_resid == 0) {
        uio->uio_iovcnt = 0;
        return (0);
    }
    err = __sfvwrite(fp, uio);
    uio->uio_resid = 0;
    uio->uio_iovcnt = 0;
    return (err);
}

#include <math.h>

#define FLOATING_POINT

#include "float.h"

#ifdef FLOATING_POINT
#define IEEE_8087

#ifndef CONST
#define CONST const
#endif

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= 0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

#ifdef IEEE_8087
#define word0(x) ((unsigned long *)&x)[1]
#define word1(x) ((unsigned long *)&x)[0]
#else
#define word0(x) ((unsigned long *)&x)[0]
#define word1(x) ((unsigned long *)&x)[1]
#endif

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */
#if defined(IEEE_8087) + defined(VAX)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_8087) + defined(IEEE_MC68k)
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Bias 1023
#define IEEE_Arith
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Infinite(x) (word0(x) == 0x7ff00000) /* sufficient test for here */
#else
#undef  Sudden_Underflow
#define Sudden_Underflow
#ifdef IBM
#define Exp_shift  24
#define Exp_shift1 24
#define Exp_msk1   0x1000000
#define Exp_msk11  0x1000000
#define Exp_mask  0x7f000000
#define P 14
#define Bias 65
#define Exp_1  0x41000000
#define Exp_11 0x41000000
#define Ebits 8	/* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask  0xffffff
#define Frac_mask1 0xffffff
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 4
#define Tiny0 0x100000
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
#else /* VAX */
#define Exp_shift  23
#define Exp_shift1 7
#define Exp_msk1    0x80
#define Exp_msk11   0x800000
#define Exp_mask  0x7f80
#define P 56
#define Bias 129
#define Exp_1  0x40800000
#define Exp_11 0x4080
#define Ebits 8
#define Frac_mask  0x7fffff
#define Frac_mask1 0xffff007f
#define Ten_pmax 24
#define Bletch 2
#define Bndry_mask  0xffff007f
#define Bndry_mask1 0xffff007f
#define LSB 0x10000
#define Sign_bit 0x8000
#define Log2P 1
#define Tiny0 0x80
#define Tiny1 0
#define Quick_max 15
#define Int_max 15
#endif
#endif

#ifndef IEEE_Arith
#define ROUND_BIASED
#endif

#ifdef RND_PRODQUOT
#define rounded_product(a,b) a = rnd_prod(a, b)
#define rounded_quotient(a,b) a = rnd_quot(a, b)
#ifdef KR_headers
extern double rnd_prod(), rnd_quot();
#else
extern double rnd_prod(double, double), rnd_quot(double, double);
#endif
#else
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#ifndef Just_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per long.
 */
#ifndef Pack_32
#define Pack_32
#endif
#endif

#define Kmax 15

struct Bigint {
    struct Bigint *next;
    int k, maxwds, sign, wds;
    unsigned long x[1];
};

typedef struct Bigint Bigint;

#define Kmax 15
static Bigint *freelist[Kmax+1];

static Bigint *Balloc(int k)
{
    int x;
    Bigint *rv;

    if (rv = freelist[k]) {
        freelist[k] = rv->next;
    } else {
        x = 1 << k;
        rv = (Bigint *)malloc(sizeof(Bigint) + (x-1)*sizeof(long));
        rv->k = k;
        rv->maxwds = x;
    }
    rv->sign = rv->wds = 0;
    return rv;
}

static void Bfree(Bigint *v)
{
    if (v) {
        v->next = freelist[v->k];
        freelist[v->k] = v;
    }
}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, y->wds*sizeof(long) + 2*sizeof(int))

static Bigint *multadd(Bigint *b, int m, int a)
{
    int i, wds;
    unsigned long *x, y;
#ifdef Pack_32
    unsigned long xi, z;
#endif
    Bigint *b1;

    wds = b->wds;
    x = b->x;
    i = 0;
    do {
#ifdef Pack_32
        xi = *x;
        y = (xi & 0xffff) * m + a;
        z = (xi >> 16) * m + (y >> 16);
        a = (int)(z >> 16);
        *x++ = (z << 16) + (y & 0xffff);
#else
        y = *x * m + a;
        a = (int)(y >> 16);
        *x++ = y & 0xffff;
#endif
    } while (++i < wds);
    if (a) {
        if (wds >= b->maxwds) {
            b1 = Balloc(b->k+1);
            Bcopy(b1, b);
            Bfree(b);
            b = b1;
        }
        b->x[wds++] = a;
        b->wds = wds;
    }
    return b;
}

static Bigint *s2b(CONST char *s, int nd0, int nd, unsigned long y9)
{
    Bigint *b;
    int i, k;
    long x, y;

    x = (nd + 8) / 9;
    for (k = 0, y = 1; x > y; y <<= 1, k++) ;
#ifdef Pack_32
    b = Balloc(k);
    b->x[0] = y9;
    b->wds = 1;
#else
    b = Balloc(k+1);
	b->x[0] = y9 & 0xffff;
	b->wds = (b->x[1] = y9 >> 16) ? 2 : 1;
#endif

    i = 9;
    if (9 < nd0) {
        s += 9;
        do
            b = multadd(b, 10, *s++ - '0');
        while (++i < nd0);
        s++;
    } else
        s += 10;
    for (; i < nd; i++)
        b = multadd(b, 10, *s++ - '0');
    return b;
}

static int hi0bits(register unsigned long x)
{
    register int k = 0;

    if (!(x & 0xffff0000)) {
        k = 16;
        x <<= 16;
    }
    if (!(x & 0xff000000)) {
        k += 8;
        x <<= 8;
    }
    if (!(x & 0xf0000000)) {
        k += 4;
        x <<= 4;
    }
    if (!(x & 0xc0000000)) {
        k += 2;
        x <<= 2;
    }
    if (!(x & 0x80000000)) {
        k++;
        if (!(x & 0x40000000))
            return 32;
    }
    return k;
}

static int lo0bits(unsigned long *y)
{
    register int k;
    register unsigned long x = *y;

    if (x & 7) {
        if (x & 1)
            return 0;
        if (x & 2) {
            *y = x >> 1;
            return 1;
        }
        *y = x >> 2;
        return 2;
    }
    k = 0;
    if (!(x & 0xffff)) {
        k = 16;
        x >>= 16;
    }
    if (!(x & 0xff)) {
        k += 8;
        x >>= 8;
    }
    if (!(x & 0xf)) {
        k += 4;
        x >>= 4;
    }
    if (!(x & 0x3)) {
        k += 2;
        x >>= 2;
    }
    if (!(x & 1)) {
        k++;
        x >>= 1;
        if (!x & 1)
            return 32;
    }
    *y = x;
    return k;
}

static Bigint *i2b(int i)
{
    Bigint *b;

    b = Balloc(1);
    b->x[0] = i;
    b->wds = 1;
    return b;
}

static Bigint *mult(Bigint *a, Bigint *b)
{
    Bigint *c;
    int k, wa, wb, wc;
    unsigned long carry, y, z;
    unsigned long *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
#ifdef Pack_32
    unsigned long z2;
#endif

    if (a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if (wc > a->maxwds)
        k++;
    c = Balloc(k);
    for (x = c->x, xa = x + wc; x < xa; x++)
        *x = 0;
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
#ifdef Pack_32
    for (; xb < xbe; xb++, xc0++) {
        if (y = *xb & 0xffff) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                Storeinc(xc, z2, z);
            } while (x < xae);
            *xc = carry;
        }
        if (y = *xb >> 16) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xffff) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                Storeinc(xc, z, z2);
                z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
                carry = z2 >> 16;
            } while (x < xae);
            *xc = z2;
        }
    }
#else
    for (; xb < xbe; xc0++) {
		if (y = *xb++) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = *x++ * y + *xc + carry;
				carry = z >> 16;
				*xc++ = z & 0xffff;
			} while (x < xae);
			*xc = carry;
		}
	}
#endif
    for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) ;
    c->wds = wc;
    return c;
}

static Bigint *p5s;

static Bigint *pow5mult(Bigint *b, int k)
{
    Bigint *b1, *p5, *p51;
    int i;
    static int p05[3] = { 5, 25, 125 };

    if (i = k & 3)
        b = multadd(b, p05[i-1], 0);

    if (!(k >>= 2))
        return b;
    if (!(p5 = p5s)) {
        /* first time */
        p5 = p5s = i2b(625);
        p5->next = 0;
    }
    for (;;) {
        if (k & 1) {
            b1 = mult(b, p5);
            Bfree(b);
            b = b1;
        }
        if (!(k >>= 1))
            break;
        if (!(p51 = p5->next)) {
            p51 = p5->next = mult(p5,p5);
            p51->next = 0;
        }
        p5 = p51;
    }
    return b;
}

static Bigint *lshift(Bigint *b, int k)
{
    int i, k1, n, n1;
    Bigint *b1;
    unsigned long *x, *x1, *xe, z;

#ifdef Pack_32
    n = k >> 5;
#else
    n = k >> 4;
#endif
    k1 = b->k;
    n1 = n + b->wds + 1;
    for (i = b->maxwds; n1 > i; i <<= 1)
        k1++;
    b1 = Balloc(k1);
    x1 = b1->x;
    for (i = 0; i < n; i++)
        *x1++ = 0;
    x = b->x;
    xe = x + b->wds;
#ifdef Pack_32
    if (k &= 0x1f) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while (x < xe);
        if (*x1 = z)
            ++n1;
    }
#else
        if (k &= 0xf) {
		k1 = 16 - k;
		z = 0;
		do {
			*x1++ = *x << k  & 0xffff | z;
			z = *x++ >> k1;
		} while (x < xe);
		if (*x1 = z)
			++n1;
	}
#endif
    else
        do
            *x1++ = *x++;
        while (x < xe);
    b1->wds = n1 - 1;
    Bfree(b);
    return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
    unsigned long *xa, *xa0, *xb, *xb0;
    int i, j;

    i = a->wds;
    j = b->wds;
#ifdef DEBUG
    if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
    if (i -= j)
        return i;
    xa0 = a->x;
    xa = xa0 + j;
    xb0 = b->x;
    xb = xb0 + j;
    for (;;) {
        if (*--xa != *--xb)
            return *xa < *xb ? -1 : 1;
        if (xa <= xa0)
            break;
    }
    return 0;
}

static Bigint *diff(Bigint *a, Bigint *b)
{
    Bigint *c;
    int i, wa, wb;
    long borrow, y;	/* We need signed shifts here. */
    unsigned long *xa, *xae, *xb, *xbe, *xc;
#ifdef Pack_32
    long z;
#endif

    i = cmp(a,b);
    if (!i) {
        c = Balloc(0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }
    if (i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    } else
        i = 0;
    c = Balloc(a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;
#ifdef Pack_32
    do {
        y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
        borrow = z >> 16;
        Sign_Extend(borrow, z);
        Storeinc(xc, z, y);
    } while (xb < xbe);
    while (xa < xae) {
        y = (*xa & 0xffff) + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        z = (*xa++ >> 16) + borrow;
        borrow = z >> 16;
        Sign_Extend(borrow, z);
        Storeinc(xc, z, y);
    }
#else
    do {
		y = *xa++ - *xb++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
	} while (xb < xbe);
	while (xa < xae) {
		y = *xa++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
	}
#endif
    while (!*--xc)
        wa--;
    c->wds = wa;
    return c;
}

static double ulp(double x)
{
    register long L;
    double a;

    L = (word0(x) & Exp_mask) - (P-1)*Exp_msk1;
#ifndef Sudden_Underflow
    if (L > 0) {
#endif
#ifdef IBM
        L |= Exp_msk1 >> 4;
#endif
        word0(a) = L;
        word1(a) = 0;
#ifndef Sudden_Underflow
    } else {
        L = -L >> Exp_shift;
        if (L < Exp_shift) {
            word0(a) = 0x80000 >> L;
            word1(a) = 0;
        } else {
            word0(a) = 0;
            L -= Exp_shift;
            word1(a) = L >= 31 ? 1 : 1 << 31 - L;
        }
    }
#endif
    return a;
}

static double b2d(Bigint *a, int *e)
{
    unsigned long *xa, *xa0, w, y, z;
    int k;
    double d;
#ifdef VAX
    unsigned long d0, d1;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

    xa0 = a->x;
    xa = xa0 + a->wds;
    y = *--xa;
#ifdef DEBUG
    if (!y) Bug("zero y in b2d");
#endif
    k = hi0bits(y);
    *e = 32 - k;
#ifdef Pack_32
    if (k < Ebits) {
        d0 = Exp_1 | y >> Ebits - k;
        w = xa > xa0 ? *--xa : 0;
        d1 = y << (32-Ebits) + k | w >> Ebits - k;
        goto ret_d;
    }
    z = xa > xa0 ? *--xa : 0;
    if (k -= Ebits) {
        d0 = Exp_1 | y << k | z >> 32 - k;
        y = xa > xa0 ? *--xa : 0;
        d1 = z << k | y >> 32 - k;
    } else {
        d0 = Exp_1 | y;
        d1 = z;
    }
#else
    if (k < Ebits + 16) {
		z = xa > xa0 ? *--xa : 0;
		d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
		w = xa > xa0 ? *--xa : 0;
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
		goto ret_d;
	}
	z = xa > xa0 ? *--xa : 0;
	w = xa > xa0 ? *--xa : 0;
	k -= Ebits + 16;
	d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
	y = xa > xa0 ? *--xa : 0;
	d1 = w << k + 16 | y << k;
#endif
    ret_d:
#ifdef VAX
    word0(d) = d0 >> 16 | d0 << 16;
	word1(d) = d1 >> 16 | d1 << 16;
#else
#undef d0
#undef d1
#endif
    return d;
}

static Bigint *d2b(double d, int *e, int *bits)
{
    Bigint *b;
    int de, i, k;
    unsigned long *x, y, z;
#ifdef VAX
    unsigned long d0, d1;
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

#ifdef Pack_32
    b = Balloc(1);
#else
    b = Balloc(2);
#endif
    x = b->x;

    z = d0 & Frac_mask;
    d0 &= 0x7fffffff;	/* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
    de = (int)(d0 >> Exp_shift);
#ifndef IBM
	z |= Exp_msk11;
#endif
#else
    if (de = (int)(d0 >> Exp_shift))
        z |= Exp_msk1;
#endif
#ifdef Pack_32
    if (y = d1) {
        if (k = lo0bits(&y)) {
            x[0] = y | z << 32 - k;
            z >>= k;
        }
        else
            x[0] = y;
        i = b->wds = (x[1] = z) ? 2 : 1;
    } else {
#ifdef DEBUG
        if (!z)
			Bug("Zero passed to d2b");
#endif
        k = lo0bits(&z);
        x[0] = z;
        i = b->wds = 1;
        k += 32;
    }
#else
    if (y = d1) {
		if (k = lo0bits(&y))
			if (k >= 16) {
				x[0] = y | z << 32 - k & 0xffff;
				x[1] = z >> k - 16 & 0xffff;
				x[2] = z >> k;
				i = 2;
			} else {
				x[0] = y & 0xffff;
				x[1] = y >> 16 | z << 16 - k & 0xffff;
				x[2] = z >> k & 0xffff;
				x[3] = z >> k+16;
				i = 3;
			}
		else {
			x[0] = y & 0xffff;
			x[1] = y >> 16;
			x[2] = z & 0xffff;
			x[3] = z >> 16;
			i = 3;
		}
	} else {
#ifdef DEBUG
		if (!z)
			Bug("Zero passed to d2b");
#endif
		k = lo0bits(&z);
		if (k >= 16) {
			x[0] = z;
			i = 0;
		} else {
			x[0] = z & 0xffff;
			x[1] = z >> 16;
			i = 1;
		}
		k += 32;
	}
	while (!x[i])
		--i;
	b->wds = i + 1;
#endif
#ifndef Sudden_Underflow
    if (de) {
#endif
#ifdef IBM
        *e = (de - Bias - (P-1) << 2) + k;
		*bits = 4*P + 8 - k - hi0bits(word0(d) & Frac_mask);
#else
        *e = de - Bias - (P-1) + k;
        *bits = P - k;
#endif
#ifndef Sudden_Underflow
    } else {
        *e = de - Bias - (P-1) + 1 + k;
#ifdef Pack_32
        *bits = 32*i - hi0bits(x[i-1]);
#else
        *bits = (i+2)*16 - hi0bits(x[i]);
#endif
    }
#endif
    return b;
}
#undef d0
#undef d1

static double ratio(Bigint *a, Bigint *b)
{
    double da, db;
    int k, ka, kb;

    da = b2d(a, &ka);
    db = b2d(b, &kb);
#ifdef Pack_32
    k = ka - kb + 32*(a->wds - b->wds);
#else
    k = ka - kb + 16*(a->wds - b->wds);
#endif
#ifdef IBM
    if (k > 0) {
		word0(da) += (k >> 2)*Exp_msk1;
		if (k &= 3)
			da *= 1 << k;
	} else {
		k = -k;
		word0(db) += (k >> 2)*Exp_msk1;
		if (k &= 3)
			db *= 1 << k;
	}
#else
    if (k > 0)
        word0(da) += k*Exp_msk1;
    else {
        k = -k;
        word0(db) += k*Exp_msk1;
    }
#endif
    return da / db;
}

static double
        tens[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
        1e20, 1e21, 1e22
#ifdef VAX
        , 1e23, 1e24
#endif
};

static double
#ifdef IEEE_Arith
        bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };
#define n_bigtens 5
#else
#ifdef IBM
bigtens[] = { 1e16, 1e32, 1e64 };
static double tinytens[] = { 1e-16, 1e-32, 1e-64 };
#define n_bigtens 3
#else
bigtens[] = { 1e16, 1e32 };
static double tinytens[] = { 1e-16, 1e-32 };
#define n_bigtens 2
#endif
#endif

double strtod(CONST char *s00, char **se)
{
    int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, dsign,
            e, e1, esign, i, j, k, nd, nd0, nf, nz, nz0, sign;
    CONST char *s, *s0, *s1;
    double aadj, aadj1, adj, rv, rv0;
    long L;
    unsigned long y, z;
    Bigint *bb, *bb1, *bd, *bd0, *bs, *delta;
    sign = nz0 = nz = 0;
    rv = 0.;
    for (s = s00;;s++) switch(*s) {
            case '-':
                sign = 1;
                /* no break */
            case '+':
                if (*++s)
                    goto break2;
                /* no break */
            case 0:
                s = s00;
                goto ret;
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ':
                continue;
            default:
                goto break2;
        }
    break2:
    if (*s == '0') {
        nz0 = 1;
        while (*++s == '0') ;
        if (!*s)
            goto ret;
    }
    s0 = s;
    y = z = 0;
    for (nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
        if (nd < 9)
            y = 10*y + c - '0';
        else if (nd < 16)
            z = 10*z + c - '0';
    nd0 = nd;
    if (c == '.') {
        c = *++s;
        if (!nd) {
            for (; c == '0'; c = *++s)
                nz++;
            if (c > '0' && c <= '9') {
                s0 = s;
                nf += nz;
                nz = 0;
                goto have_dig;
            }
            goto dig_done;
        }
        for (; c >= '0' && c <= '9'; c = *++s) {
            have_dig:
            nz++;
            if (c -= '0') {
                nf += nz;
                for (i = 1; i < nz; i++)
                    if (nd++ < 9)
                        y *= 10;
                    else if (nd <= DBL_DIG + 1)
                        z *= 10;
                if (nd++ < 9)
                    y = 10*y + c;
                else if (nd <= DBL_DIG + 1)
                    z = 10*z + c;
                nz = 0;
            }
        }
    }
    dig_done:
    e = 0;
    if (c == 'e' || c == 'E') {
        if (!nd && !nz && !nz0) {
            s = s00;
            goto ret;
        }
        s00 = s;
        esign = 0;
        switch(c = *++s) {
            case '-':
                esign = 1;
            case '+':
                c = *++s;
        }
        if (c >= '0' && c <= '9') {
            while (c == '0')
                c = *++s;
            if (c > '0' && c <= '9') {
                L = c - '0';
                s1 = s;
                while ((c = *++s) >= '0' && c <= '9')
                    L = 10*L + c - '0';
                if (s - s1 > 8 || L > 19999)
                    /* Avoid confusion from exponents
                     * so large that e might overflow.
                     */
                    e = 19999; /* safe for 16 bit ints */
                else
                    e = (int)L;
                if (esign)
                    e = -e;
            } else
                e = 0;
        } else
            s = s00;
    }
    if (!nd) {
        if (!nz && !nz0)
            s = s00;
        goto ret;
    }
    e1 = e -= nf;

    /* Now we have nd0 digits, starting at s0, followed by a
     * decimal point, followed by nd-nd0 digits.  The number we're
     * after is the integer represented by those digits times
     * 10**e */

    if (!nd0)
        nd0 = nd;
    k = nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
    rv = y;
    if (k > 9)
        rv = tens[k - 9] * rv + z;
    if (nd <= DBL_DIG
        #ifndef RND_PRODQUOT
        && FLT_ROUNDS == 1
#endif
            ) {
        if (!e)
            goto ret;
        if (e > 0) {
            if (e <= Ten_pmax) {
#ifdef VAX
                goto vax_ovfl_check;
#else
                /* rv = */ rounded_product(rv, tens[e]);
                goto ret;
#endif
            }
            i = DBL_DIG - nd;
            if (e <= Ten_pmax + i) {
                /* A fancier test would sometimes let us do
                 * this for larger i values.
                 */
                e -= i;
                rv *= tens[i];
#ifdef VAX
                /* VAX exponent range is so narrow we must
				 * worry about overflow here...
				 */
 vax_ovfl_check:
				word0(rv) -= P*Exp_msk1;
				/* rv = */ rounded_product(rv, tens[e]);
				if ((word0(rv) & Exp_mask)
				 > Exp_msk1*(DBL_MAX_EXP+Bias-1-P))
					goto ovfl;
				word0(rv) += P*Exp_msk1;
#else
                /* rv = */ rounded_product(rv, tens[e]);
#endif
                goto ret;
            }
        }
#ifndef Inaccurate_Divide
        else if (e >= -Ten_pmax) {
            /* rv = */ rounded_quotient(rv, tens[-e]);
            goto ret;
        }
#endif
    }
    e1 += nd - k;

    /* Get starting approximation = rv * 10**e1 */

    if (e1 > 0) {
        if (i = e1 & 15)
            rv *= tens[i];
        if (e1 &= ~15) {
            if (e1 > DBL_MAX_10_EXP) {
                ovfl:
#ifdef __STDC__
                rv = HUGE_VAL;
#else
                /* Can't trust HUGE_VAL */
#ifdef IEEE_Arith
				word0(rv) = Exp_mask;
				word1(rv) = 0;
#else
				word0(rv) = Big0;
				word1(rv) = Big1;
#endif
#endif
                goto ret;
            }
            if (e1 >>= 4) {
                for (j = 0; e1 > 1; j++, e1 >>= 1)
                    if (e1 & 1)
                        rv *= bigtens[j];
                /* The last multiplication could overflow. */
                word0(rv) -= P*Exp_msk1;
                rv *= bigtens[j];
                if ((z = word0(rv) & Exp_mask)
                    > Exp_msk1*(DBL_MAX_EXP+Bias-P))
                    goto ovfl;
                if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
                    /* set to largest number */
                    /* (Can't trust DBL_MAX) */
                    word0(rv) = Big0;
                    word1(rv) = Big1;
                }
                else
                    word0(rv) += P*Exp_msk1;
            }
        }
    } else if (e1 < 0) {
        e1 = -e1;
        if (i = e1 & 15)
            rv /= tens[i];
        if (e1 &= ~15) {
            e1 >>= 4;
            for (j = 0; e1 > 1; j++, e1 >>= 1)
                if (e1 & 1)
                    rv *= tinytens[j];
            /* The last multiplication could underflow. */
            rv0 = rv;
            rv *= tinytens[j];
            if (!rv) {
                rv = 2.*rv0;
                rv *= tinytens[j];
                if (!rv) {
                    undfl:
                    rv = 0.;
                    goto ret;
                }
                word0(rv) = Tiny0;
                word1(rv) = Tiny1;
                /* The refinement below will clean
                 * this approximation up.
                 */
            }
        }
    }

    /* Now the hard part -- adjusting rv to the correct value.*/

    /* Put digits into bd: true value = bd * 10^e */

    bd0 = s2b(s0, nd0, nd, y);

    for (;;) {
        bd = Balloc(bd0->k);
        Bcopy(bd, bd0);
        bb = d2b(rv, &bbe, &bbbits);	/* rv = bb * 2^bbe */
        bs = i2b(1);

        if (e >= 0) {
            bb2 = bb5 = 0;
            bd2 = bd5 = e;
        } else {
            bb2 = bb5 = -e;
            bd2 = bd5 = 0;
        }
        if (bbe >= 0)
            bb2 += bbe;
        else
            bd2 -= bbe;
        bs2 = bb2;
#ifdef Sudden_Underflow
        #ifdef IBM
		j = 1 + 4*P - 3 - bbbits + ((bbe + bbbits - 1) & 3);
#else
		j = P + 1 - bbbits;
#endif
#else
        i = bbe + bbbits - 1;	/* logb(rv) */
        if (i < Emin)	/* denormal */
            j = bbe + (P-Emin);
        else
            j = P + 1 - bbbits;
#endif
        bb2 += j;
        bd2 += j;
        i = bb2 < bd2 ? bb2 : bd2;
        if (i > bs2)
            i = bs2;
        if (i > 0) {
            bb2 -= i;
            bd2 -= i;
            bs2 -= i;
        }
        if (bb5 > 0) {
            bs = pow5mult(bs, bb5);
            bb1 = mult(bs, bb);
            Bfree(bb);
            bb = bb1;
        }
        if (bb2 > 0)
            bb = lshift(bb, bb2);
        if (bd5 > 0)
            bd = pow5mult(bd, bd5);
        if (bd2 > 0)
            bd = lshift(bd, bd2);
        if (bs2 > 0)
            bs = lshift(bs, bs2);
        delta = diff(bb, bd);
        dsign = delta->sign;
        delta->sign = 0;
        i = cmp(delta, bs);
        if (i < 0) {
            /* Error is less than half an ulp -- check for
             * special case of mantissa a power of two.
             */
            if (dsign || word1(rv) || word0(rv) & Bndry_mask)
                break;
            delta = lshift(delta,Log2P);
            if (cmp(delta, bs) > 0)
                goto drop_down;
            break;
        }
        if (i == 0) {
            /* exactly half-way between */
            if (dsign) {
                if ((word0(rv) & Bndry_mask1) == Bndry_mask1
                    &&  word1(rv) == 0xffffffff) {
                    /*boundary case -- increment exponent*/
                    word0(rv) = (word0(rv) & Exp_mask)
                                + Exp_msk1
#ifdef IBM
                        | Exp_msk1 >> 4
#endif
                            ;
                    word1(rv) = 0;
                    break;
                }
            } else if (!(word0(rv) & Bndry_mask) && !word1(rv)) {
                drop_down:
                /* boundary case -- decrement exponent */
#ifdef Sudden_Underflow
                L = word0(rv) & Exp_mask;
#ifdef IBM
				if (L <  Exp_msk1)
#else
				if (L <= Exp_msk1)
#endif
					goto undfl;
				L -= Exp_msk1;
#else
                L = (word0(rv) & Exp_mask) - Exp_msk1;
#endif
                word0(rv) = L | Bndry_mask1;
                word1(rv) = 0xffffffff;
#ifdef IBM
                goto cont;
#else
                break;
#endif
            }
#ifndef ROUND_BIASED
            if (!(word1(rv) & LSB))
                break;
#endif
            if (dsign)
                rv += ulp(rv);
#ifndef ROUND_BIASED
            else {
                rv -= ulp(rv);
#ifndef Sudden_Underflow
                if (!rv)
                    goto undfl;
#endif
            }
#endif
            break;
        }
        if ((aadj = ratio(delta, bs)) <= 2.) {
            if (dsign)
                aadj = aadj1 = 1.;
            else if (word1(rv) || word0(rv) & Bndry_mask) {
#ifndef Sudden_Underflow
                if (word1(rv) == Tiny1 && !word0(rv))
                    goto undfl;
#endif
                aadj = 1.;
                aadj1 = -1.;
            } else {
                /* special case -- power of FLT_RADIX to be */
                /* rounded down... */

                if (aadj < 2./FLT_RADIX)
                    aadj = 1./FLT_RADIX;
                else
                    aadj *= 0.5;
                aadj1 = -aadj;
            }
        } else {
            aadj *= 0.5;
            aadj1 = dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
            switch(FLT_ROUNDS) {
				case 2: /* towards +infinity */
					aadj1 -= 0.5;
					break;
				case 0: /* towards 0 */
				case 3: /* towards -infinity */
					aadj1 += 0.5;
			}
#else
            if (FLT_ROUNDS == 0)
                aadj1 += 0.5;
#endif
        }
        y = word0(rv) & Exp_mask;

        /* Check for overflow */

        if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
            rv0 = rv;
            word0(rv) -= P*Exp_msk1;
            adj = aadj1 * ulp(rv);
            rv += adj;
            if ((word0(rv) & Exp_mask) >=
                Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
                if (word0(rv0) == Big0 && word1(rv0) == Big1)
                    goto ovfl;
                word0(rv) = Big0;
                word1(rv) = Big1;
                goto cont;
            } else
                word0(rv) += P*Exp_msk1;
        } else {
#ifdef Sudden_Underflow
            if ((word0(rv) & Exp_mask) <= P*Exp_msk1) {
				rv0 = rv;
				word0(rv) += P*Exp_msk1;
				adj = aadj1 * ulp(rv);
				rv += adj;
#ifdef IBM
				if ((word0(rv) & Exp_mask) <  P*Exp_msk1)
#else
				if ((word0(rv) & Exp_mask) <= P*Exp_msk1)
#endif
				{
					if (word0(rv0) == Tiny0
					 && word1(rv0) == Tiny1)
						goto undfl;
					word0(rv) = Tiny0;
					word1(rv) = Tiny1;
					goto cont;
				} else
					word0(rv) -= P*Exp_msk1;
			} else {
				adj = aadj1 * ulp(rv);
				rv += adj;
			}
#else
            /* Compute adj so that the IEEE rounding rules will
             * correctly round rv + adj in some half-way cases.
             * If rv * ulp(rv) is denormalized (i.e.,
             * y <= (P-1)*Exp_msk1), we must adjust aadj to avoid
             * trouble from bits lost to denormalization;
             * example: 1.2e-307 .
             */
            if (y <= (P-1)*Exp_msk1 && aadj >= 1.) {
                aadj1 = (double)(int)(aadj + 0.5);
                if (!dsign)
                    aadj1 = -aadj1;
            }
            adj = aadj1 * ulp(rv);
            rv += adj;
#endif
        }
        z = word0(rv) & Exp_mask;
        if (y == z) {
            /* Can we stop now? */
            L = aadj;
            aadj -= L;
            /* The tolerances below are conservative. */
            if (dsign || word1(rv) || word0(rv) & Bndry_mask) {
                if (aadj < .4999999 || aadj > .5000001)
                    break;
            } else if (aadj < .4999999/FLT_RADIX)
                break;
        }
        cont:
        Bfree(bb);
        Bfree(bd);
        Bfree(bs);
        Bfree(delta);
    }
    Bfree(bb);
    Bfree(bd);
    Bfree(bs);
    Bfree(bd0);
    Bfree(delta);
    ret:
    if (se)
        *se = (char *)s;
    return sign ? -rv : rv;
}

static int quorem(Bigint *b, Bigint *S)
{
    int n;
    long borrow, y;
    unsigned long carry, q, ys;
    unsigned long *bx, *bxe, *sx, *sxe;
#ifdef Pack_32
    long z;
    unsigned long si, zs;
#endif

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
        do {
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            Sign_Extend(borrow, z);
            Storeinc(bx, z, y);
#else
            ys = *sx++ * q + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
        } while (sx <= sxe);
        if (!*bxe) {
            bx = b->x;
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    if (cmp(b, S) >= 0) {
        q++;
        borrow = 0;
        carry = 0;
        bx = b->x;
        sx = S->x;
        do {
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            Sign_Extend(borrow, z);
            Storeinc(bx, z, y);
#else
            ys = *sx++ + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
        } while (sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (!*bxe) {
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    return q;
}

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
 *	   something like 10^(k-15) that we must resort to the long
 *	   calculation.
 */

char *__dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve)
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
           4-9 should give the same return values as 2-3, i.e.,
               4 <= mode <= 9 ==> same return as mode
               2 + (mode & 1).  These modes are mainly for
               debugging; often they run slower but sometimes
               faster than modes 2-3.
           4,5,8,9 ==> left-to-right digit generation.
           6-9 ==> don't try fast floating-point estimate
               (if applicable).

           Values of mode other than 0-9 are treated as mode 0.

           Sufficient space is allocated to the return value
           to hold the suppressed trailing zeros.
       */

    int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1,
            j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
            spec_case, try_quick;
    long L;
#ifndef Sudden_Underflow
    int denorm;
    unsigned long x;
#endif
    Bigint *b, *b1, *delta, *mlo, *mhi, *S;
    double d2, ds, eps;
    char *s, *s0;
    static Bigint *result;
    static int result_k;

    if (result) {
        result->k = result_k;
        result->maxwds = 1 << result_k;
        Bfree(result);
        result = 0;
    }

    if (word0(d) & Sign_bit) {
        /* set sign for everything, including 0's and NaNs */
        *sign = 1;
        word0(d) &= ~Sign_bit;	/* clear sign bit */
    }
    else
        *sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
    if ((word0(d) & Exp_mask) == Exp_mask)
#else
        if (word0(d)  == 0x8000)
#endif
    {
        /* Infinity or NaN */
        *decpt = 9999;
        s =
#ifdef IEEE_Arith
!word1(d) && !(word0(d) & 0xfffff) ? "Infinity" :
#endif
"NaN";
        if (rve)
            *rve =
#ifdef IEEE_Arith
s[3] ? s + 8 :
#endif
s + 3;
        return s;
    }
#endif
#ifdef IBM
    d += 0; /* normalize */
#endif
    if (!d) {
        *decpt = 1;
        s = "0";
        if (rve)
            *rve = s + 1;
        return s;
    }

    b = d2b(d, &be, &bbits);
#ifdef Sudden_Underflow
    i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#else
    if (i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1))) {
#endif
        d2 = d;
        word0(d2) &= Frac_mask1;
        word0(d2) |= Exp_11;
#ifdef IBM
        if (j = 11 - hi0bits(word0(d2) & Frac_mask))
			d2 /= 1 << j;
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
#ifndef Sudden_Underflow
        denorm = 0;
    } else {
        /* d is denormalized */

        i = bbits + be + (Bias + (P-1) - 1);
        x = i > 32  ? word0(d) << 64 - i | word1(d) >> i - 32
                    : word1(d) << 32 - i;
        d2 = x;
        word0(d2) -= 31*Exp_msk1; /* adjust exponent */
        i -= (Bias + (P-1) - 1) + 1;
        denorm = 1;
    }
#endif
    ds = (d2-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
    k = (int)ds;
    if (ds < 0. && ds != k)
        k--;	/* want k = floor(ds) */
    k_check = 1;
    if (k >= 0 && k <= Ten_pmax) {
        if (d < tens[k])
            k--;
        k_check = 0;
    }
    j = bbits - i - 1;
    if (j >= 0) {
        b2 = 0;
        s2 = j;
    } else {
        b2 = -j;
        s2 = 0;
    }
    if (k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
    } else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
    }
    if (mode < 0 || mode > 9)
        mode = 0;
    try_quick = 1;
    if (mode > 5) {
        mode -= 4;
        try_quick = 0;
    }
    leftright = 1;
    switch(mode) {
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
    j = sizeof(unsigned long);
    for (result_k = 0; sizeof(Bigint) - sizeof(unsigned long) + j < i;
         j <<= 1) result_k++;
    result = Balloc(result_k);
    s = s0 = (char *)result;

    if (ilim >= 0 && ilim <= Quick_max && try_quick) {

        /* Try to get by with floating-point arithmetic. */

        i = 0;
        d2 = d;
        k0 = k;
        ilim0 = ilim;
        ieps = 2; /* conservative */
        if (k > 0) {
            ds = tens[k&0xf];
            j = k >> 4;
            if (j & Bletch) {
                /* prevent overflows */
                j &= Bletch - 1;
                d /= bigtens[n_bigtens-1];
                ieps++;
            }
            for (; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    ds *= bigtens[i];
                }
            d /= ds;
        } else if (j1 = -k) {
            d *= tens[j1 & 0xf];
            for (j = j1 >> 4; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    d *= bigtens[i];
                }
        }
        if (k_check && d < 1. && ilim > 0) {
            if (ilim1 <= 0)
                goto fast_failed;
            ilim = ilim1;
            k--;
            d *= 10.;
            ieps++;
        }
        eps = ieps*d + 7.;
        word0(eps) -= (P-1)*Exp_msk1;
        if (ilim == 0) {
            S = mhi = 0;
            d -= 5.;
            if (d > eps)
                goto one_digit;
            if (d < -eps)
                goto no_digits;
            goto fast_failed;
        }
#ifndef No_leftright
        if (leftright) {
            /* Use Steele & White method of only
             * generating digits needed.
             */
            eps = 0.5/tens[ilim-1] - eps;
            for (i = 0;;) {
                L = d;
                d -= L;
                *s++ = '0' + (int)L;
                if (d < eps)
                    goto ret1;
                if (1. - d < eps)
                    goto bump_up;
                if (++i >= ilim)
                    break;
                eps *= 10.;
                d *= 10.;
            }
        } else {
#endif
            /* Generate ilim digits, then fix them up. */
            eps *= tens[ilim-1];
            for (i = 1;; i++, d *= 10.) {
                L = d;
                d -= L;
                *s++ = '0' + (int)L;
                if (i == ilim) {
                    if (d > 0.5 + eps)
                        goto bump_up;
                    else if (d < 0.5 - eps) {
                        while (*--s == '0');
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
        d = d2;
        k = k0;
        ilim = ilim0;
    }

    /* Do we have a "small" integer? */

    if (be >= 0 && k <= Int_max) {
        /* Yes. */
        ds = tens[k];
        if (ndigits < 0 && ilim <= 0) {
            S = mhi = 0;
            if (ilim < 0 || d <= 5*ds)
                goto no_digits;
            goto one_digit;
        }
        for (i = 1;; i++) {
            L = d / ds;
            d -= L*ds;
#ifdef Check_FLT_ROUNDS
            /* If FLT_ROUNDS == 2, L will usually be high by 1 */
			if (d < 0) {
				L--;
				d += ds;
			}
#endif
            *s++ = '0' + (int)L;
            if (i == ilim) {
                d += d;
                if (d > ds || d == ds && L & 1) {
                    bump_up:
                    while (*--s == '9')
                        if (s == s0) {
                            k++;
                            *s = '0';
                            break;
                        }
                    ++*s++;
                }
                break;
            }
            if (!(d *= 10.))
                break;
        }
        goto ret1;
    }

    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if (leftright) {
        if (mode < 2) {
            i =
#ifndef Sudden_Underflow
denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
1 + P - bbits;
#endif
        } else {
            j = ilim - 1;
            if (m5 >= j)
                m5 -= j;
            else {
                s5 += j -= m5;
                b5 += j;
                m5 = 0;
            }
            if ((i = ilim) < 0) {
                m2 -= i;
                i = 0;
            }
        }
        b2 += i;
        s2 += i;
        mhi = i2b(1);
    }
    if (m2 > 0 && s2 > 0) {
        i = m2 < s2 ? m2 : s2;
        b2 -= i;
        m2 -= i;
        s2 -= i;
    }
    if (b5 > 0) {
        if (leftright) {
            if (m5 > 0) {
                mhi = pow5mult(mhi, m5);
                b1 = mult(mhi, b);
                Bfree(b);
                b = b1;
            }
            if (j = b5 - m5)
                b = pow5mult(b, j);
        } else
            b = pow5mult(b, b5);
    }
    S = i2b(1);
    if (s5 > 0)
        S = pow5mult(S, s5);

    /* Check for special case that d is a normalized power of 2. */

    if (mode < 2) {
        if (!word1(d) && !(word0(d) & Bndry_mask)
            #ifndef Sudden_Underflow
            && word0(d) & Exp_mask
#endif
                ) {
            /* The special case */
            b2 += Log2P;
            s2 += Log2P;
            spec_case = 1;
        } else
            spec_case = 0;
    }

    /* Arrange for convenient computation of quotients:
     * shift left if necessary so divisor has 4 leading 0 bits.
     *
     * Perhaps we should just compute leading 28 bits of S once
     * and for all and pass them and a shift to quorem, so it
     * can do shifts and ors to compute the numerator for q.
     */
#ifdef Pack_32
    if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0x1f)
        i = 32 - i;
#else
    if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0xf)
        i = 16 - i;
#endif
    if (i > 4) {
        i -= 4;
        b2 += i;
        m2 += i;
        s2 += i;
    } else if (i < 4) {
        i += 28;
        b2 += i;
        m2 += i;
        s2 += i;
    }
    if (b2 > 0)
        b = lshift(b, b2);
    if (s2 > 0)
        S = lshift(S, s2);
    if (k_check) {
        if (cmp(b,S) < 0) {
            k--;
            b = multadd(b, 10, 0);	/* we botched the k estimate */
            if (leftright)
                mhi = multadd(mhi, 10, 0);
            ilim = ilim1;
        }
    }
    if (ilim <= 0 && mode > 2) {
        if (ilim < 0 || cmp(b,S = multadd(S,5,0)) <= 0) {
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
    if (leftright) {
        if (m2 > 0)
            mhi = lshift(mhi, m2);

        /* Compute mlo -- check for special case
         * that d is a normalized power of 2.
         */

        mlo = mhi;
        if (spec_case) {
            mhi = Balloc(mhi->k);
            Bcopy(mhi, mlo);
            mhi = lshift(mhi, Log2P);
        }

        for (i = 1;;i++) {
            dig = quorem(b,S) + '0';
            /* Do we yet have the shortest decimal string
             * that will round to d?
             */
            j = cmp(b, mlo);
            delta = diff(S, mhi);
            j1 = delta->sign ? 1 : cmp(b, delta);
            Bfree(delta);
#ifndef ROUND_BIASED
            if (j1 == 0 && !mode && !(word1(d) & 1)) {
                if (dig == '9')
                    goto round_9_up;
                if (j > 0)
                    dig++;
                *s++ = dig;
                goto ret;
            }
#endif
            if (j < 0 || j == 0 && !mode
                         #ifndef ROUND_BIASED
                         && !(word1(d) & 1)
#endif
                    ) {
                if (j1 > 0) {
                    b = lshift(b, 1);
                    j1 = cmp(b, S);
                    if ((j1 > 0 || j1 == 0 && dig & 1)
                        && dig++ == '9')
                        goto round_9_up;
                }
                *s++ = dig;
                goto ret;
            }
            if (j1 > 0) {
                if (dig == '9') { /* possible if i == 1 */
                    round_9_up:
                    *s++ = '9';
                    goto roundoff;
                }
                *s++ = dig + 1;
                goto ret;
            }
            *s++ = dig;
            if (i == ilim)
                break;
            b = multadd(b, 10, 0);
            if (mlo == mhi)
                mlo = mhi = multadd(mhi, 10, 0);
            else {
                mlo = multadd(mlo, 10, 0);
                mhi = multadd(mhi, 10, 0);
            }
        }
    } else
        for (i = 1;; i++) {
            *s++ = dig = quorem(b,S) + '0';
            if (i >= ilim)
                break;
            b = multadd(b, 10, 0);
        }

    /* Round off last digit */

    b = lshift(b, 1);
    j = cmp(b, S);
    if (j > 0 || j == 0 && dig & 1) {
        roundoff:
        while (*--s == '9')
            if (s == s0) {
                k++;
                *s++ = '1';
                goto ret;
            }
        ++*s++;
    } else {
        while (*--s == '0');
        s++;
    }
    ret:
    Bfree(S);
    if (mhi) {
        if (mlo && mlo != mhi)
            Bfree(mlo);
        Bfree(mhi);
    }
    ret1:
    Bfree(b);
    if (s == s0) {	/* don't return empty string */
        *s++ = '0';
        k = 0;
    }
    *s = 0;
    *decpt = k + 1;
    if (rve)
        *rve = s;
    return s0;
}

static char *cvt(double value, int ndigits, int flags, char *sign, int *decpt, int ch, int *length)
{
    int mode, dsgn;
    char *digits, *bp, *rve;

    if (ch == 'f') {
        mode = 3;		/* ndigits after the decimal point */
    } else {
        /* To obtain ndigits after the decimal point for the 'e'
         * and 'E' formats, round to ndigits + 1 significant
         * figures.
         */
        if (ch == 'e' || ch == 'E') {
            ndigits++;
        }
        mode = 2;		/* ndigits significant digits */
    }

    if (value < 0) {
        value = -value;
        *sign = '-';
    } else
        *sign = '\000';
    digits = __dtoa(value, mode, ndigits, decpt, &dsgn, &rve);
    if ((ch != 'g' && ch != 'G') || flags & ALT) {	/* Print trailing zeros */
        bp = digits + ndigits;
        if (ch == 'f') {
            if (*digits == '0' && value)
                *decpt = -ndigits + 1;
            bp += *decpt;
        }
        if (value == 0)	/* kludge for __dtoa irregularity */
            rve = bp;
        while (rve < bp)
            *rve++ = '0';
    }
    *length = rve - digits;
    return (digits);
}

static int exponent(char *p0, int exp, int fmtch)
{
    register char *p, *t;
    char expbuf[MAXEXP];

    p = p0;
    *p++ = fmtch;
    if (exp < 0) {
        exp = -exp;
        *p++ = '-';
    }
    else
        *p++ = '+';
    t = expbuf + MAXEXP;
    if (exp > 9) {
        do {
            *--t = to_char(exp % 10);
        } while ((exp /= 10) > 9);
        *--t = to_char(exp);
        for (; t < expbuf + MAXEXP; *p++ = *t++);
    }
    else {
        *p++ = '0';
        *p++ = to_char(exp);
    }
    return (p - p0);
}

#endif

int vfprintf(FILE *fp, const char *fmt0, va_list ap)
{
    register char *fmt;	/* format string */
    register int ch;	/* character from fmt */
    register int n;		/* handy integer (short term usage) */
    register char *cp;	/* handy char pointer (short term usage) */
    register struct __siov *iovp;/* for PRINT macro */
    register int flags;	/* flags as above */
    int ret;		/* return value accumulator */
    int width;		/* width from format (%8d), or 0 */
    int prec;		/* precision from format (%.3d), or -1 */
    char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef FLOATING_POINT
    char softsign;		/* temporary negative sign for floats */
    double _double;		/* double precision arguments %[eEfgG] */
    int expt;		/* integer value of exponent */
    int expsize;		/* character count for expstr */
    int ndig;		/* actual number of digits returned by cvt */
    char expstr[7];		/* buffer for exponent string */
#endif

//#ifdef __GNUC__			/* gcc has builtin quad type (long long) SOS */
#define	quad_t	  long
#define	u_quad_t  unsigned long
//#endif

    u_quad_t _uquad;	/* integer arguments %[diouxX] */
    enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
    int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
    int fieldsz;		/* field size expanded by sign, etc */
    int realsz;		/* field size expanded by dprec */
    int size;		/* size of converted field or string */
    char *xdigs;		/* digits for [xX] conversion */
#define NIOV 8
    struct __suio uio;	/* output information: summary */
    struct __siov iov[NIOV];/* ... and individual io vectors */
    char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
    char ox[2];		/* space for 0x hex-prefix */

    /*
     * Choose PADSIZE to trade efficiency vs. size.  If larger printf
     * fields occur frequently, increase PADSIZE and make the initialisers
     * below longer.
     */
#define	PADSIZE	16		/* pad chunk size */
    static char blanks[PADSIZE] =
            {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    static char zeroes[PADSIZE] =
            {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

    /*
     * BEWARE, these `goto error' on error, and PAD uses `n'.
     */
#define	PRINT(ptr, len) { \
	iovp->iov_base = (ptr); \
	iovp->iov_len = (len); \
	uio.uio_resid += (len); \
	iovp++; \
	if (++uio.uio_iovcnt >= NIOV) { \
		if (__sprint(fp, &uio)) \
			goto error; \
		iovp = iov; \
	} \
}
#define	PAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			PRINT(with, PADSIZE); \
			n -= PADSIZE; \
		} \
		PRINT(with, n); \
	} \
}
#define	FLUSH() { \
	if (uio.uio_resid && __sprint(fp, &uio)) \
		goto error; \
	uio.uio_iovcnt = 0; \
	iovp = iov; \
}
    /*
     * To extend shorts properly, we need both signed and unsigned
     * argument extraction methods.
     */
#define	SARG() \
	(flags&QUADINT ? va_arg(ap, quad_t) : \
	    flags&LONGINT ? va_arg(ap, long) : \
	    flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
	    (long)va_arg(ap, int))
#define	UARG() \
	(flags&QUADINT ? va_arg(ap, u_quad_t) : \
	    flags&LONGINT ? va_arg(ap, unsigned long) : \
	    flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(ap, int) : \
	    (unsigned long)va_arg(ap, unsigned int))

    fmt = (char *)fmt0;
    uio.uio_iov = iovp = iov;
    uio.uio_resid = 0;
    uio.uio_iovcnt = 0;
    ret = 0;

    /*
     * Scan the format for conversions (`%' character).
     */
    for (;;) {
        for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
            /* void */;
        if ((n = fmt - cp) != 0) {
            PRINT(cp, n);
            ret += n;
        }
        if (ch == '\0')
            goto done;
        fmt++;		/* skip over '%' */

        flags = 0;
        dprec = 0;
        width = 0;
        prec = -1;
        sign = '\0';

        rflag:		ch = *fmt++;
        reswitch:	switch (ch) {
        case ' ':
            /*
             * ``If the space and + flags both appear, the space
             * flag will be ignored.''
             *	-- ANSI X3J11
             */
            if (!sign)
                sign = ' ';
            goto rflag;
        case '#':
            flags |= ALT;
            goto rflag;
        case '*':
            /*
             * ``A negative field width argument is taken as a
             * - flag followed by a positive field width.''
             *	-- ANSI X3J11
             * They don't exclude field widths read from args.
             */
            if ((width = va_arg(ap, int)) >= 0)
                goto rflag;
            width = -width;
            /* FALLTHROUGH */
        case '-':
            flags |= LADJUST;
            goto rflag;
        case '+':
            sign = '+';
            goto rflag;
        case '.':
            if ((ch = *fmt++) == '*') {
                n = va_arg(ap, int);
                prec = n < 0 ? -1 : n;
                goto rflag;
            }
            n = 0;
            while (is_digit(ch)) {
                n = 10 * n + to_digit(ch);
                ch = *fmt++;
            }
            prec = n < 0 ? -1 : n;
            goto reswitch;
        case '0':
            /*
             * ``Note that 0 is taken as a flag, not as the
             * beginning of a field width.''
             *	-- ANSI X3J11
             */
            flags |= ZEROPAD;
            goto rflag;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = 0;
            do {
                n = 10 * n + to_digit(ch);
                ch = *fmt++;
            } while (is_digit(ch));
            width = n;
            goto reswitch;
#ifdef FLOATING_POINT
        case 'L':
            flags |= LONGDBL;
            goto rflag;
#endif
        case 'h':
            flags |= SHORTINT;
            goto rflag;
        case 'l':
            flags |= LONGINT;
            goto rflag;
        case 'q':
            flags |= QUADINT;
            goto rflag;
        case 'c':
            *(cp = buf) = va_arg(ap, int);
            size = 1;
            sign = '\0';
            break;
        case 'D':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'd':
        case 'i':
            _uquad = SARG();
            if ((quad_t)_uquad < 0) {
                _uquad = -_uquad;
                sign = '-';
            }
            base = DEC;
            goto number;
#ifdef FLOATING_POINT
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            if (prec == -1) {
                prec = DEFPREC;
            } else if ((ch == 'g' || ch == 'G') && prec == 0) {
                prec = 1;
            }

            if (flags & LONGDBL) {
                _double = (double) va_arg(ap, long double);
            } else {
                _double = va_arg(ap, double);
            }

            /* do this before tricky precision changes */
            if (isinf(_double)) {
                if (_double < 0)
                    sign = '-';
                cp = "Inf";
                size = 3;
                break;
            }
            if (isnan(_double)) {
                cp = "NaN";
                size = 3;
                break;
            }

            flags |= FPT;
            cp = cvt(_double, prec, flags, &softsign,
                     &expt, ch, &ndig);
            if (ch == 'g' || ch == 'G') {
                if (expt <= -4 || expt > prec)
                    ch = (ch == 'g') ? 'e' : 'E';
                else
                    ch = 'g';
            }
            if (ch <= 'e') {	/* 'e' or 'E' fmt */
                --expt;
                expsize = exponent(expstr, expt, ch);
                size = expsize + ndig;
                if (ndig > 1 || flags & ALT)
                    ++size;
            } else if (ch == 'f') {		/* f fmt */
                if (expt > 0) {
                    size = expt;
                    if (prec || flags & ALT)
                        size += prec + 1;
                } else	/* "0.X" */
                    size = prec + 2;
            } else if (expt >= ndig) {	/* fixed g fmt */
                size = expt;
                if (flags & ALT)
                    ++size;
            } else
                size = ndig + (expt > 0 ?
                               1 : 2 - expt);

            if (softsign)
                sign = '-';
            break;
#endif /* FLOATING_POINT */
        case 'n':
            if (flags & QUADINT)
                *va_arg(ap, quad_t *) = ret;
            else if (flags & LONGINT)
                *va_arg(ap, long *) = ret;
            else if (flags & SHORTINT)
                *va_arg(ap, short *) = ret;
            else
                *va_arg(ap, int *) = ret;
            continue;	/* no output */
        case 'O':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'o':
            _uquad = UARG();
            base = OCT;
            goto nosign;
        case 'p':
            /*
             * ``The argument shall be a pointer to void.  The
             * value of the pointer is converted to a sequence
             * of printable characters, in an implementation-
             * defined manner.''
             *	-- ANSI X3J11
             */
            /* NOSTRICT */
            /* no easy way to tell in cpp how big a type is */
            _uquad = (u_quad_t)(unsigned long)va_arg(ap, void *);
            base = HEX;
            xdigs = "0123456789abcdef";
            flags |= HEXPREFIX;
            ch = 'x';
            goto nosign;
        case 's':
            if ((cp = va_arg(ap, char *)) == NULL)
                cp = "(null)";
            if (prec >= 0) {
                /*
                 * can't use strlen; can only look for the
                 * NUL in the first `prec' characters, and
                 * strlen() will go further.
                 */
                char *p = memchr(cp, 0, prec);

                if (p != NULL) {
                    size = p - cp;
                    if (size > prec)
                        size = prec;
                } else
                    size = prec;
            } else
                size = strlen(cp);
            sign = '\0';
            break;
        case 'U':
            flags |= LONGINT;
            /*FALLTHROUGH*/
        case 'u':
            _uquad = UARG();
            base = DEC;
            goto nosign;
        case 'X':
            xdigs = "0123456789ABCDEF";
            goto hex;
        case 'x':
            xdigs = "0123456789abcdef";
        hex:			_uquad = UARG();
            base = HEX;
            /* leading 0x/X only if non-zero */
            if (flags & ALT && _uquad != 0)
                flags |= HEXPREFIX;

            /* unsigned conversions */
        nosign:			sign = '\0';
            /*
             * ``... diouXx conversions ... if a precision is
             * specified, the 0 flag will be ignored.''
             *	-- ANSI X3J11
             */
        number:			if ((dprec = prec) >= 0)
                flags &= ~ZEROPAD;

            /*
             * ``The result of converting a zero value with an
             * explicit precision of zero is no characters.''
             *	-- ANSI X3J11
             */
            cp = buf + BUF;
            if (_uquad != 0 || prec != 0) {
                /*
                 * Unsigned mod is hard, and unsigned mod
                 * by a constant is easier than that by
                 * a variable; hence this switch.
                 */
                switch (base) {
                    case OCT:
                        do {
                            *--cp = to_char(_uquad & 7);
                            _uquad >>= 3;
                        } while (_uquad);
                        /* handle octal leading 0 */
                        if (flags & ALT && *cp != '0')
                            *--cp = '0';
                        break;

                    case DEC:
                        /* many numbers are 1 digit */
                        while (_uquad >= 10) {
                            *--cp = to_char(_uquad % 10);
                            _uquad /= 10;
                        }
                        *--cp = to_char(_uquad);
                        break;

                    case HEX:
                        do {
                            *--cp = xdigs[_uquad & 15];
                            _uquad >>= 4;
                        } while (_uquad);
                        break;

                    default:
                        cp = "bug in vfprintf: bad base";
                        size = strlen(cp);
                        goto skipsize;
                }
            }
            size = buf + BUF - cp;
        skipsize:
            break;
        default:	/* "%?" prints ?, unless ? is NUL */
            if (ch == '\0')
                goto done;
            /* pretend it was %c with argument ch */
            cp = buf;
            *cp = ch;
            size = 1;
            sign = '\0';
            break;
    }

        /*
         * All reasonable formats wind up here.  At this point, `cp'
         * points to a string which (if not flags&LADJUST) should be
         * padded out to `width' places.  If flags&ZEROPAD, it should
         * first be prefixed by any sign or other prefix; otherwise,
         * it should be blank padded before the prefix is emitted.
         * After any left-hand padding and prefixing, emit zeroes
         * required by a decimal [diouxX] precision, then print the
         * string proper, then emit zeroes required by any leftover
         * floating precision; finally, if LADJUST, pad with blanks.
         *
         * Compute actual size, so we know how much to pad.
         * fieldsz excludes decimal prec; realsz includes it.
         */
        fieldsz = size;
        if (sign)
            fieldsz++;
        else if (flags & HEXPREFIX)
            fieldsz += 2;
        realsz = dprec > fieldsz ? dprec : fieldsz;

        /* right-adjusting blank padding */
        if ((flags & (LADJUST|ZEROPAD)) == 0)
        PAD(width - realsz, blanks);

        /* prefix */
        if (sign) {
            PRINT(&sign, 1);
        } else if (flags & HEXPREFIX) {
            ox[0] = '0';
            ox[1] = ch;
            PRINT(ox, 2);
        }

        /* right-adjusting zero padding */
        if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
        PAD(width - realsz, zeroes);

        /* leading zeroes from decimal precision */
        PAD(dprec - fieldsz, zeroes);

        /* the string or number proper */
#ifdef FLOATING_POINT
        if ((flags & FPT) == 0) {
            PRINT(cp, size);
        } else {	/* glue together f_p fragments */
            if (ch >= 'f') {	/* 'f' or 'g' */
                if (_double == 0) {
                    /* kludge for __dtoa irregularity */
                    if (expt >= ndig && (flags & ALT) == 0) {
                        PRINT("0", 1);
                    } else {
                        PRINT("0.", 2);
                        PAD(ndig - 1, zeroes);
                    }
                } else if (expt <= 0) {
                    PRINT("0.", 2);
                    PAD(-expt, zeroes);
                    PRINT(cp, ndig);
                } else if (expt >= ndig) {
                    PRINT(cp, ndig);
                    PAD(expt - ndig, zeroes);
                    if (flags & ALT)
                    PRINT(".", 1);
                } else {
                    PRINT(cp, expt);
                    cp += expt;
                    PRINT(".", 1);
                    PRINT(cp, ndig-expt);
                }
            } else {	/* 'e' or 'E' */
                if (ndig > 1 || flags & ALT) {
                    ox[0] = *cp++;
                    ox[1] = '.';
                    PRINT(ox, 2);
                    if (_double || flags & ALT == 0) {
                        PRINT(cp, ndig-1);
                    } else	/* 0.[0..] */
                        /* __dtoa irregularity */
                    PAD(ndig - 1, zeroes);
                } else	/* XeYYY */
                PRINT(cp, 1);
                PRINT(expstr, expsize);
            }
        }
#else
        PRINT(cp, size);
#endif
        /* left-adjusting padding (always blank) */
        if (flags & LADJUST)
        PAD(width - realsz, blanks);

        /* finally, adjust ret */
        ret += width > realsz ? width : realsz;

        FLUSH();	/* copy out the I/O vectors */
    }
    done:
    FLUSH();
    error:
    return EOF;
    /* NOTREACHED */
}

int myprintf(char const *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return (ret);
}

int fprintf(FILE *fp, const char *fmt, ...)
{
    int ret;
    va_list ap;

#if __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
#endif
    ret = vfprintf(fp, fmt, ap);
    va_end(ap);
    return (ret);
}

int puts(char const *s)
{
    size_t c = strlen(s);
    struct __suio uio;
    struct __siov iov[2];

    iov[0].iov_base = (void *)s;
    iov[0].iov_len = c;
    iov[1].iov_base = "\n";
    iov[1].iov_len = 1;
    uio.uio_resid = c + 1;
    uio.uio_iov = &iov[0];
    uio.uio_iovcnt = 2;
    return (__sfvwrite(stdout, &uio) ? EOF : '\n');
}

int fputs(const char *s, FILE *fp)
{
    struct __suio uio;
    struct __siov iov;

    iov.iov_base = (void *)s;
    iov.iov_len = uio.uio_resid = strlen(s);
    uio.uio_iov = &iov;
    uio.uio_iovcnt = 1;
    return (__sfvwrite(fp, &uio));
}

#include "xv6/user.h"

int	__srget (FILE *fp)
{
    int i, cc;
    char c;
    cc = read(fp->fd, &c, 1);
    if (cc < 1)
        return EOF;
    return c;
}

int __sputc (int _c, FILE *_p)
{
    write(_p->fd, &_c, 1);
    return 1;
}

char *mygets(char *buf)
{
    register int c;
    register char *s;
    for (s = buf; (c = getchar()) != '\n' && c != '\r';)
        if (c == EOF)
            if (s == buf)
                return (NULL);
            else
                break;
        else
            *s++ = c;
    *s = 0;
    return (buf);
}

char *fgets(char *buf, int max, FILE *fp)
{
    int i, cc;
    char c;
    for (i = 0; i + 1 < max; ) {
        cc = read(fp->fd, &c, 1);
        if (cc < 1)
            break;
        if (c == EOF && i == 0)
            return NULL;
        if (c == '\n' || c == '\r')
            break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return buf;
}

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

int __sflags(register char *mode, int *optr)
{
    register int ret, m, o;
    switch (*mode++) {

        case 'r':	/* open for reading */
            m = O_RDONLY;
            o = 0;
            break;

        case 'w':	/* open for writing */
            m = O_WRONLY;
            o = O_CREATE;
            break;

        default:	/* illegal mode */
            return (0);
    }
    /* [rwa]\+ or [rwa]b\+ means read and write */
    if (*mode == '+') {
        m = O_RDWR;
    }
    *optr = m | o;
    return (1);
}

FILE *fopen(const char *file, const char *mode)
{
    register FILE *fp;
    register int f;
    int flags, oflags;

    if ((flags = __sflags(mode, &oflags)) == 0)
        return (NULL);
    if ((f = open(file, O_RDONLY)) < 0) {
        return (NULL);
    }
    fp->fd = f;
    return (fp);
}



#define	BUF		513	/* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define	LONG		0x01	/* l: long or double */
#define	LONGDBL		0x02	/* L: long double; unimplemented */
#define	SHORT		0x04	/* h: short */
#define	SUPPRESS	0x08	/* suppress assignment */
#define	POINTER		0x10	/* weird %p pointer (`fake hex') */
#define	NOSKIP		0x20	/* do not skip blanks */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define	SIGNOK		0x40	/* +/- is (still) legal */
#define	NDIGITS		0x80	/* no digits detected */

#define	DPTOK		0x100	/* (float) decimal point is still legal */
#define	EXPOK		0x200	/* (float) exponent (e+3, etc) still legal */

#define	PFXOK		0x100	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x200	/* no zero digits detected */

/*
 * Conversion types.
 */
#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., strtol or strtoul */
#define	CT_FLOAT	4	/* floating, i.e., strtod */

#define u_char unsigned char
#define u_long unsigned long

int __svfscanf(fp, fmt0, ap)
        register FILE *fp;
        char const *fmt0;
        va_list ap;
{
    register u_char *fmt = (u_char *)fmt0;
    register int c;		/* character from format, or conversion */
    register size_t width;	/* field width, or 0 */
    register char *p;	/* points into all kinds of strings */
    register int n;		/* handy integer */
    register int flags;	/* flags as defined above */
    register char *p0;	/* saves original value of p when necessary */
    int nassigned;		/* number of fields assigned */
    int nread;		/* number of characters consumed from fp */
    int base;		/* base argument to strtol/strtoul */
    u_long (*ccfn)();	/* conversion function (strtol/strtoul) */
    char ccltab[256];	/* character class table for %[...] */
    char buf[BUF];		/* buffer for numeric conversions */

    /* `basefix' is used to avoid `if' tests in the integer scanner */
    static short basefix[17] =
            { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    nassigned = 0;
    nread = 0;
    base = 0;		/* XXX just to keep gcc happy */
    ccfn = NULL;		/* XXX just to keep gcc happy */
    for (;;) {
        c = *fmt++;
        if (c == 0)
            return (nassigned);
        if (isspace(c)) {
            for (;;) {
                if (fp->_r <= 0 && __srefill(fp))
                    return (nassigned);
                if (!isspace(*fp->_p))
                    break;
                nread++, fp->_r--, fp->_p++;
            }
            continue;
        }
        if (c != '%')
            goto literal;
        width = 0;
        flags = 0;
        /*
         * switch on the format.  continue if done;
         * break once format type is derived.
         */
        again:		c = *fmt++;
        switch (c) {
            case '%':
            literal:
                if (fp->_r <= 0 && __srefill(fp))
                    goto input_failure;
                if (*fp->_p != c)
                    goto match_failure;
                fp->_r--, fp->_p++;
                nread++;
                continue;

            case '*':
                flags |= SUPPRESS;
                goto again;
            case 'l':
                flags |= LONG;
                goto again;
            case 'L':
                flags |= LONGDBL;
                goto again;
            case 'h':
                flags |= SHORT;
                goto again;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                width = width * 10 + c - '0';
                goto again;

                /*
                 * Conversions.
                 * Those marked `compat' are for 4.[123]BSD compatibility.
                 *
                 * (According to ANSI, E and X formats are supposed
                 * to the same as e and x.  Sorry about that.)
                 */
            case 'D':	/* compat */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'd':
                c = CT_INT;
                ccfn = (u_long (*)())strtol;
                base = 10;
                break;

            case 'i':
                c = CT_INT;
                ccfn = (u_long (*)())strtol;
                base = 0;
                break;

            case 'O':	/* compat */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'o':
                c = CT_INT;
                ccfn = strtoul;
                base = 8;
                break;

            case 'u':
                c = CT_INT;
                ccfn = strtoul;
                base = 10;
                break;

            case 'X':	/* compat   XXX */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'x':
                flags |= PFXOK;	/* enable 0x prefixing */
                c = CT_INT;
                ccfn = strtoul;
                base = 16;
                break;

#ifdef FLOATING_POINT
            case 'E':	/* compat   XXX */
            case 'F':	/* compat */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'e': case 'f': case 'g':
                c = CT_FLOAT;
                break;
#endif

            case 's':
                c = CT_STRING;
                break;

            case '[':
                fmt = __sccl(ccltab, fmt);
                flags |= NOSKIP;
                c = CT_CCL;
                break;

            case 'c':
                flags |= NOSKIP;
                c = CT_CHAR;
                break;

            case 'p':	/* pointer format is like hex */
                flags |= POINTER | PFXOK;
                c = CT_INT;
                ccfn = strtoul;
                base = 16;
                break;

            case 'n':
                if (flags & SUPPRESS)	/* ??? */
                    continue;
                if (flags & SHORT)
                    *va_arg(ap, short *) = nread;
                else if (flags & LONG)
                    *va_arg(ap, long *) = nread;
                else
                    *va_arg(ap, int *) = nread;
                continue;

                /*
                 * Disgusting backwards compatibility hacks.	XXX
                 */
            case '\0':	/* compat */
                return (EOF);

            default:	/* compat */
                if (isupper(c))
                    flags |= LONG;
                c = CT_INT;
                ccfn = (u_long (*)())strtol;
                base = 10;
                break;
        }

        /*
         * We have a conversion that requires input.
         */
        if (fp->_r <= 0 && __srefill(fp))
            goto input_failure;

        /*
         * Consume leading white space, except for formats
         * that suppress this.
         */
        if ((flags & NOSKIP) == 0) {
            while (isspace(*fp->_p)) {
                nread++;
                if (--fp->_r > 0)
                    fp->_p++;
                else if (__srefill(fp))
                    goto input_failure;
            }
            /*
             * Note that there is at least one character in
             * the buffer, so conversions that do not set NOSKIP
             * ca no longer result in an input failure.
             */
        }

        /*
         * Do the conversion.
         */
        switch (c) {

            case CT_CHAR:
                /* scan arbitrary characters (sets NOSKIP) */
                if (width == 0)
                    width = 1;
                if (flags & SUPPRESS) {
                    size_t sum = 0;
                    for (;;) {
                        if ((n = fp->_r) < width) {
                            sum += n;
                            width -= n;
                            fp->_p += n;
                            if (__srefill(fp)) {
                                if (sum == 0)
                                    goto input_failure;
                                break;
                            }
                        } else {
                            sum += width;
                            fp->_r -= width;
                            fp->_p += width;
                            break;
                        }
                    }
                    nread += sum;
                } else {
                    size_t r = fread((void *)va_arg(ap, char *), 1,
                                     width, fp);

                    if (r == 0)
                        goto input_failure;
                    nread += r;
                    nassigned++;
                }
                break;

            case CT_CCL:
                /* scan a (nonempty) character class (sets NOSKIP) */
                if (width == 0)
                    width = ~0;	/* `infinity' */
                /* take only those things in the class */
                if (flags & SUPPRESS) {
                    n = 0;
                    while (ccltab[*fp->_p]) {
                        n++, fp->_r--, fp->_p++;
                        if (--width == 0)
                            break;
                        if (fp->_r <= 0 && __srefill(fp)) {
                            if (n == 0)
                                goto input_failure;
                            break;
                        }
                    }
                    if (n == 0)
                        goto match_failure;
                } else {
                    p0 = p = va_arg(ap, char *);
                    while (ccltab[*fp->_p]) {
                        fp->_r--;
                        *p++ = *fp->_p++;
                        if (--width == 0)
                            break;
                        if (fp->_r <= 0 && __srefill(fp)) {
                            if (p == p0)
                                goto input_failure;
                            break;
                        }
                    }
                    n = p - p0;
                    if (n == 0)
                        goto match_failure;
                    *p = 0;
                    nassigned++;
                }
                nread += n;
                break;

            case CT_STRING:
                /* like CCL, but zero-length string OK, & no NOSKIP */
                if (width == 0)
                    width = ~0;
                if (flags & SUPPRESS) {
                    n = 0;
                    while (!isspace(*fp->_p)) {
                        n++, fp->_r--, fp->_p++;
                        if (--width == 0)
                            break;
                        if (fp->_r <= 0 && __srefill(fp))
                            break;
                    }
                    nread += n;
                } else {
                    p0 = p = va_arg(ap, char *);
                    while (!isspace(*fp->_p)) {
                        fp->_r--;
                        *p++ = *fp->_p++;
                        if (--width == 0)
                            break;
                        if (fp->_r <= 0 && __srefill(fp))
                            break;
                    }
                    *p = 0;
                    nread += p - p0;
                    nassigned++;
                }
                continue;

            case CT_INT:
                /* scan an integer as if by strtol/strtoul */
#ifdef hardway
                if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
                /* size_t is unsigned, hence this optimisation */
                if (--width > sizeof(buf) - 2)
                    width = sizeof(buf) - 2;
                width++;
#endif
                flags |= SIGNOK | NDIGITS | NZDIGITS;
                for (p = buf; width; width--) {
                    c = *fp->_p;
                    /*
                     * Switch on the character; `goto ok'
                     * if we accept it as a part of number.
                     */
                    switch (c) {

                        /*
                         * The digit 0 is always legal, but is
                         * special.  For %i conversions, if no
                         * digits (zero or nonzero) have been
                         * scanned (only signs), we will have
                         * base==0.  In that case, we should set
                         * it to 8 and enable 0x prefixing.
                         * Also, if we have not scanned zero digits
                         * before this, do not turn off prefixing
                         * (someone else will turn it off if we
                         * have scanned any nonzero digits).
                         */
                        case '0':
                            if (base == 0) {
                                base = 8;
                                flags |= PFXOK;
                            }
                            if (flags & NZDIGITS)
                                flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
                            else
                                flags &= ~(SIGNOK|PFXOK|NDIGITS);
                            goto ok;

                            /* 1 through 7 always legal */
                        case '1': case '2': case '3':
                        case '4': case '5': case '6': case '7':
                            base = basefix[base];
                            flags &= ~(SIGNOK | PFXOK | NDIGITS);
                            goto ok;

                            /* digits 8 and 9 ok iff decimal or hex */
                        case '8': case '9':
                            base = basefix[base];
                            if (base <= 8)
                                break;	/* not legal here */
                            flags &= ~(SIGNOK | PFXOK | NDIGITS);
                            goto ok;

                            /* letters ok iff hex */
                        case 'A': case 'B': case 'C':
                        case 'D': case 'E': case 'F':
                        case 'a': case 'b': case 'c':
                        case 'd': case 'e': case 'f':
                            /* no need to fix base here */
                            if (base <= 10)
                                break;	/* not legal here */
                            flags &= ~(SIGNOK | PFXOK | NDIGITS);
                            goto ok;

                            /* sign ok only as first character */
                        case '+': case '-':
                            if (flags & SIGNOK) {
                                flags &= ~SIGNOK;
                                goto ok;
                            }
                            break;

                            /* x ok iff flag still set & 2nd char */
                        case 'x': case 'X':
                            if (flags & PFXOK && p == buf + 1) {
                                base = 16;	/* if %i */
                                flags &= ~PFXOK;
                                goto ok;
                            }
                            break;
                    }

                    /*
                     * If we got here, c is not a legal character
                     * for a number.  Stop accumulating digits.
                     */
                    break;
                    ok:
                    /*
                     * c is legal: store it and look at the next.
                     */
                    *p++ = c;
                    if (--fp->_r > 0)
                        fp->_p++;
                    else if (__srefill(fp))
                        break;		/* EOF */
                }
                /*
                 * If we had only a sign, it is no good; push
                 * back the sign.  If the number ends in `x',
                 * it was [sign] '0' 'x', so push back the x
                 * and treat it as [sign] '0'.
                 */
                if (flags & NDIGITS) {
                    if (p > buf)
                        (void) ungetc(*(u_char *)--p, fp);
                    goto match_failure;
                }
                c = ((u_char *)p)[-1];
                if (c == 'x' || c == 'X') {
                    --p;
                    (void) ungetc(c, fp);
                }
                if ((flags & SUPPRESS) == 0) {
                    u_long res;

                    *p = 0;
                    res = (*ccfn)(buf, (char **)NULL, base);
                    if (flags & POINTER)
                        *va_arg(ap, void **) = (void *)res;
                    else if (flags & SHORT)
                        *va_arg(ap, short *) = res;
                    else if (flags & LONG)
                        *va_arg(ap, long *) = res;
                    else
                        *va_arg(ap, int *) = res;
                    nassigned++;
                }
                nread += p - buf;
                break;

#ifdef FLOATING_POINT
            case CT_FLOAT:
                /* scan a floating point number as if by strtod */
#ifdef hardway
                if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
                /* size_t is unsigned, hence this optimisation */
                if (--width > sizeof(buf) - 2)
                    width = sizeof(buf) - 2;
                width++;
#endif
                flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
                for (p = buf; width; width--) {
                    c = *fp->_p;
                    /*
                     * This code mimicks the integer conversion
                     * code, but is much simpler.
                     */
                    switch (c) {

                        case '0': case '1': case '2': case '3':
                        case '4': case '5': case '6': case '7':
                        case '8': case '9':
                            flags &= ~(SIGNOK | NDIGITS);
                            goto fok;

                        case '+': case '-':
                            if (flags & SIGNOK) {
                                flags &= ~SIGNOK;
                                goto fok;
                            }
                            break;
                        case '.':
                            if (flags & DPTOK) {
                                flags &= ~(SIGNOK | DPTOK);
                                goto fok;
                            }
                            break;
                        case 'e': case 'E':
                            /* no exponent without some digits */
                            if ((flags&(NDIGITS|EXPOK)) == EXPOK) {
                                flags =
                                        (flags & ~(EXPOK|DPTOK)) |
                                        SIGNOK | NDIGITS;
                                goto fok;
                            }
                            break;
                    }
                    break;
                    fok:
                    *p++ = c;
                    if (--fp->_r > 0)
                        fp->_p++;
                    else if (__srefill(fp))
                        break;	/* EOF */
                }
                /*
                 * If no digits, might be missing exponent digits
                 * (just give back the exponent) or might be missing
                 * regular digits, but had sign and/or decimal point.
                 */
                if (flags & NDIGITS) {
                    if (flags & EXPOK) {
                        /* no digits at all */
                        while (p > buf)
                            ungetc(*(u_char *)--p, fp);
                        goto match_failure;
                    }
                    /* just a bad exponent (e and maybe sign) */
                    c = *(u_char *)--p;
                    if (c != 'e' && c != 'E') {
                        (void) ungetc(c, fp);/* sign */
                        c = *(u_char *)--p;
                    }
                    (void) ungetc(c, fp);
                }
                if ((flags & SUPPRESS) == 0) {
                    double res;

                    *p = 0;
                    res = strtod(buf,(char **) NULL);
                    if (flags & LONG)
                        *va_arg(ap, double *) = res;
                    else
                        *va_arg(ap, float *) = res;
                    nassigned++;
                }
                nread += p - buf;
                break;
#endif /* FLOATING_POINT */
        }
    }
    input_failure:
    return (nassigned ? nassigned : -1);
    match_failure:
    return (nassigned);
}


int	 scanf(const char *fmt, ...)
{
int ret;
va_list ap;

#if __STDC__
va_start(ap, fmt);
#else
va_start(ap);
#endif
ret = __svfscanf(stdin, fmt, ap);
va_end(ap);
return (ret);
}
#include <xv6/user.h>
#include <stdio.h>
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>

#if 1

#define	BUF		513	/* Maximum length of numeric string. */

#define FLOATING_POINT
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

extern  __srefill(register FILE *fp);

long strtol(const char *nptr, char **endptr, register int base)
{
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        //errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = any ? (char *)s - 1 : (char *)nptr;
    return (acc);
}

unsigned long strtoul(const char *nptr, char **endptr, register int base)
{
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    /*
     * See strtol for comments as to the logic used.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
        //errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = any ? (char *)s - 1 : (char *)nptr;
    return (acc);
}

static u_char *__sccl(register char *tab, register u_char *fmt)
{
    register int c, n, v;

    /* first `clear' the whole table */
    c = *fmt++;		/* first char hat => negated scanset */
    if (c == '^') {
        v = 1;		/* default => accept */
        c = *fmt++;	/* get new first char */
    } else
        v = 0;		/* default => reject */
    /* should probably use memset here */
    for (n = 0; n < 256; n++)
        tab[n] = v;
    if (c == 0)
        return (fmt - 1);/* format ended before closing ] */

    /*
     * Now set the entries corresponding to the actual scanset
     * to the opposite of the above.
     *
     * The first character may be ']' (or '-') without being special;
     * the last character may be '-'.
     */
    v = 1 - v;
    for (;;) {
        tab[c] = v;		/* take character c */
        doswitch:
        n = *fmt++;		/* and examine the next */
        switch (n) {

            case 0:			/* format ended too soon */
                return (fmt - 1);

            case '-':
                /*
                 * A scanset of the form
                 *	[01+-]
                 * is defined as `the digit 0, the digit 1,
                 * the character +, the character -', but
                 * the effect of a scanset such as
                 *	[a-zA-Z0-9]
                 * is implementation defined.  The V7 Unix
                 * scanf treats `a-z' as `the letters a through
                 * z', but treats `a-a' as `the letter a, the
                 * character -, and the letter a'.
                 *
                 * For compatibility, the `-' is not considerd
                 * to define a range if the character following
                 * it is either a close bracket (required by ANSI)
                 * or is not numerically greater than the character
                 * we just stored in the table (c).
                 */
                n = *fmt;
                if (n == ']' || n < c) {
                    c = '-';
                    break;	/* resume the for(;;) */
                }
                fmt++;
                do {		/* fill in the range */
                    tab[++c] = v;
                } while (c < n);
#if 1	/* XXX another disgusting compatibility hack */
                /*
                 * Alas, the V7 Unix scanf also treats formats
                 * such as [a-c-e] as `the letters a through e'.
                 * This too is permitted by the standard....
                 */
                goto doswitch;
#else
            c = *fmt++;
			if (c == 0)
				return (fmt - 1);
			if (c == ']')
				return (fmt);
#endif
                break;

            case ']':		/* end of scanset */
                return (fmt);

            default:		/* just another character */
                c = n;
                break;
        }
    }
    /* NOTREACHED */
}


int fread(void *buf, size_t size, size_t count, register FILE *fp)
{
    register size_t resid;
    register char *p;
    register int r;
    int i, j;
    size_t total;

    if ((resid = count * size) == 0)
        return (resid);
    if (fp->reserve < 0)
    {
        //myprintf("eeeee\n");
        fp->reserve = 0;
    }
    total = resid;
    p = buf;
    while (resid > (r = fp->reserve)) {
        //bcopy(fp->cur, p, (size_t)r);

        for(i = 0; i < r; ++i)
        {
            j = (fp->cur + i) % fp->buffer_size;
            p[i] = fp->buffer[j];
        }

        fp->cur = (fp->cur + r) % fp->buffer_size;
        /* fp->_r = 0 ... done in __srefill */
        p += r;
        resid -= r;
        if (__srefill(fp)) {
            /* no more input: return partial result */
            return ((total - resid) / size);
        }
    }
    //(void) bcopy((void *)fp->cur, (void *)p, resid);
    for(i = 0; i < resid; ++i)
    {
        j = (fp->cur + i) % fp->buffer_size;
        p[i] = fp->buffer[j];
    }
    fp->reserve -= resid;
    fp->cur = (fp->cur + resid) % fp->buffer_size;
    return (count);
}


int __svfscanf(register FILE *fp, char const *fmt0, va_list ap)
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
                if (fp->reserve <= 0 && __srefill(fp))
                    return (nassigned);
                if (!isspace(fp->buffer[fp->cur]))
                    break;
                nread++, fp->reserve--, fp->cur = (fp->cur + 1) % fp->buffer_size;
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
                if (fp->reserve <= 0 && __srefill(fp))
                    goto input_failure;
                if (fp->buffer[fp->cur] != c)
                    goto match_failure;
                fp->reserve--, fp->cur = (fp->cur + 1) % fp->buffer_size;
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
        if (fp->reserve <= 0 && __srefill(fp))
            goto input_failure;

        /*
         * Consume leading white space, except for formats
         * that suppress this.
         */
        if ((flags & NOSKIP) == 0) {
            while (isspace(fp->buffer[fp->cur])) {
                nread++;
                if (--fp->reserve > 0)
                    fp->cur = (fp->cur + 1) % fp->buffer_size;
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
                        if ((n = fp->reserve) < width) {
                            sum += n;
                            width -= n;
                            fp->cur = (fp->cur + n) % fp->buffer_size;
                            if (__srefill(fp)) {
                                if (sum == 0)
                                    goto input_failure;
                                break;
                            }
                        } else {
                            sum += width;
                            fp->reserve -= width;
                            fp->cur = (fp->cur + width) % fp->buffer_size;
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
                    while (ccltab[fp->buffer[fp->cur]]) {
                        n++, fp->reserve--, fp->cur = (fp->cur + 1) % fp->buffer_size;
                        if (--width == 0)
                            break;
                        if (fp->reserve <= 0 && __srefill(fp)) {
                            if (n == 0)
                                goto input_failure;
                            break;
                        }
                    }
                    if (n == 0)
                        goto match_failure;
                } else {
                    p0 = p = va_arg(ap, char *);
                    while (ccltab[fp->buffer[fp->cur]]) {
                        fp->reserve--;
                        *p++ = fp->buffer[fp->cur];
                        fp->cur = (fp->cur + 1) % fp->buffer_size;
                        if (--width == 0)
                            break;
                        if (fp->reserve <= 0 && __srefill(fp)) {
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
                    while (!isspace(fp->buffer[fp->cur])) {
                        n++, fp->reserve--, fp->cur = (fp->cur + 1) % fp->buffer_size;
                        if (--width == 0)
                            break;
                        if (fp->reserve <= 0 && __srefill(fp))
                            break;
                    }
                    nread += n;
                } else {
                    p0 = p = va_arg(ap, char *);
                    while (!isspace(fp->buffer[fp->cur])) {
                        fp->reserve--;
                        *p++ = fp->buffer[fp->cur];
                        fp->cur = (fp->cur + 1) % fp->buffer_size;
                        if (--width == 0)
                            break;
                        if (fp->reserve <= 0 && __srefill(fp))
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
                    c = fp->buffer[fp->cur];
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
                    if (--fp->reserve > 0)
                        fp->cur = (fp->cur + 1) % fp->buffer_size;
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
				c = fp->buffer[fp->cur];
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
				if (--fp->reserve > 0)
                    fp->cur = (fp->cur + 1) % fp->buffer_size;
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

#endif
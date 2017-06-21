#include <stdio.h>
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>

#if 0

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

static u_char *__sccl();

void (*__cleanup)();

int _fwalk(register int (*function)())
{
    register FILE *fp;
    register int n, ret;
    register struct glue *g;

    ret = 0;
    for (g = &__sglue; g != NULL; g = g->next)
        for (fp = g->iobs, n = g->niobs; --n >= 0; fp++)
            if (fp->_flags != 0)
                ret |= (*function)(fp);
    return (ret);
}

void
_cleanup()
{
    /* (void) _fwalk(fclose); */
    (void) _fwalk(__sflush);		/* `cheating' */
}

void __sinit()
{
    /* make sure we clean up on exit */
    __cleanup = _cleanup;		/* conservative */
    __sdidinit = 1;
}

int __srefill(register FILE *fp)
{

    /* make sure stdio is set up */
    if (!__sdidinit)
        __sinit();

    fp->_r = 0;		/* largely a convenience for callers */

    /* SysV does not make this test; take it out for compatibility */
    if (fp->_flags & __SEOF)
        return (EOF);

    /* if not already reading, have to be reading and writing */
    if ((fp->_flags & __SRD) == 0) {
        if ((fp->_flags & __SRW) == 0) {
            errno = EBADF;
            return (EOF);
        }
        /* switch to reading */
        if (fp->_flags & __SWR) {
            if (__sflush(fp))
                return (EOF);
            fp->_flags &= ~__SWR;
            fp->_w = 0;
            fp->_lbfsize = 0;
        }
        fp->_flags |= __SRD;
    } else {
        /*
         * We were reading.  If there is an ungetc buffer,
         * we must have been reading from that.  Drop it,
         * restoring the previous buffer (if any).  If there
         * is anything in that buffer, return.
         */
        if (HASUB(fp)) {
            FREEUB(fp);
            if ((fp->_r = fp->_ur) != 0) {
                fp->_p = fp->_up;
                return (0);
            }
        }
    }

    if (fp->_bf._base == NULL)
        __smakebuf(fp);

    /*
     * Before reading from a line buffered or unbuffered file,
     * flush all line buffered output files, per the ANSI C
     * standard.
     */
    if (fp->_flags & (__SLBF|__SNBF))
        (void) _fwalk(lflush);
    fp->_p = fp->_bf._base;
    fp->_r = (*fp->_read)(fp->_cookie, (char *)fp->_p, fp->_bf._size);
    fp->_flags &= ~__SMOD;	/* buffer contents are again pristine */
    if (fp->_r <= 0) {
        if (fp->_r == 0)
            fp->_flags |= __SEOF;
        else {
            fp->_r = 0;
            fp->_flags |= __SERR;
        }
        return (EOF);
    }
    return (0);
}


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
        errno = ERANGE;
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
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = any ? (char *)s - 1 : (char *)nptr;
    return (acc);
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

#endif
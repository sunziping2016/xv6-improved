#include <stdlib.h>
#include <string.h>
#include <xv6/user.h>

typedef long Align;

union header {
    struct {
        union header *ptr;
        unsigned int size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void free(void *ap)
{
    Header *bp, *p;

    bp = (Header*)ap - 1;
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break;
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

static Header *morecore(unsigned int nu)
{
    char *p;
    Header *hp;

    if (nu < 4096)
        nu = 4096;
    p = sbrk(nu * sizeof(Header));
    if (p == (char*) - 1)
        return 0;
    hp = (Header*)p;
    hp->s.size = nu;
    free((void*)(hp + 1));
    return freep;
}

void *malloc(unsigned int nbytes)
{
    Header *p, *prevp;
    unsigned int nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if ((prevp = freep) == 0) {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }
    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p + 1);
        }
        if (p == freep)
            if ((p = morecore(nunits)) == 0)
                return 0;
    }
}

int abs(int j)
{
    return j > 0 ? j : -j;
}

static unsigned next = 1;

void srand(unsigned int seed)
{
    next = seed;
}

int rand()
{
    return ((next = next * 1103515245 + 12345) % ((unsigned)RAND_MAX + 1));
}

static void quick_sort(void *bot, size_t nmemb, size_t size, int (*compar)(const void*, const void*));
static void insertion_sort(void *bot, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

#define	MTHRESH	6
#define	THRESH	4

void qsort(void *bot, size_t nmemb, size_t size, int (*compar)(const void*, const void*))
{
    if (nmemb <= 1)
        return;
    if (nmemb >= THRESH)
        quick_sort(bot, nmemb, size, compar);
    else
        insertion_sort(bot, nmemb, size, compar);
}

#define	SWAP(a, b) { \
	cnt = size; \
	do { \
		ch = *a; \
		*a++ = *b; \
		*b++ = ch; \
	} while (--cnt); \
}

#define	SORT(bot, n) { \
	if (n > 1) \
		if (n == 2) { \
			t1 = bot + size; \
			if (compar(t1, bot) < 0) \
				SWAP(t1, bot); \
		} else \
			insertion_sort(bot, n, size, compar); \
}

static void quick_sort(void *bot, size_t nmemb, size_t size, int (*compar)(const void*, const void*))
{
    register int cnt;
    register unsigned char ch;
    register char *top, *mid, *t1, *t2;
    register int n1, n2;
    char *bsv;

    /* bot and nmemb must already be set. */
    partition:

    /* find mid and top elements */
    mid = bot + size * (nmemb >> 1);
    top = bot + (nmemb - 1) * size;

    /*
     * Find the median of the first, last and middle element (see Knuth,
     * Vol. 3, page 123, Eq. 28).  This test order gets the equalities
     * right.
     */
    if (nmemb >= MTHRESH) {
        n1 = compar(bot, mid);
        n2 = compar(mid, top);
        if (n1 < 0 && n2 > 0)
            t1 = compar(bot, top) < 0 ? top : bot;
        else if (n1 > 0 && n2 < 0)
            t1 = compar(bot, top) > 0 ? top : bot;
        else
            t1 = mid;

        /* if mid element not selected, swap selection there */
        if (t1 != mid) {
            SWAP(t1, mid);
            mid -= size;
        }
    }

        /* Standard quicksort, Knuth, Vol. 3, page 116, Algorithm Q. */
#define	didswap	n1
#define	newbot	t1
#define	replace	t2
    didswap = 0;
    for (bsv = bot;;) {
        for (; bot < mid && compar(bot, mid) <= 0; bot += size);
        while (top > mid) {
            if (compar(mid, top) <= 0) {
                top -= size;
                continue;
            }
            newbot = bot + size;	/* value of bot after swap */
            if (bot == mid)		/* top <-> mid, mid == top */
                replace = mid = top;
            else {			/* bot <-> top */
                replace = top;
                top -= size;
            }
            goto swap;
        }
        if (bot == mid)
            break;

        /* bot <-> mid, mid == bot */
        replace = mid;
        newbot = mid = bot;		/* value of bot after swap */
        top -= size;

        swap:
        SWAP((char *)bot, replace);
        bot = newbot;
        didswap = 1;
    }
    if (!didswap) {
        insertion_sort(bsv, nmemb, size, compar);
        return;
    }
#define	nlower	n1
#define	nupper	n2
    bot = bsv;
    nlower = (mid - (char *)bot) / size;	/* size of lower partition */
    mid += size;
    nupper = nmemb - nlower - 1;	/* size of upper partition */

    /*
     * If must call recursively, do it on the smaller partition; this
     * bounds the stack to lg N entries.
     */
    if (nlower > nupper) {
        if (nupper >= THRESH)
            quick_sort(mid, nupper, size, compar);
        else {
            SORT(mid, nupper);
            if (nlower < THRESH) {
                SORT((char *)bot, nlower);
                return;
            }
        }
        nmemb = nlower;
    } else {
        if (nlower >= THRESH)
            quick_sort(bot, nlower, size, compar);
        else {
            SORT((char *)bot, nlower);
            if (nupper < THRESH) {
                SORT(mid, nupper);
                return;
            }
        }
        bot = mid;
        nmemb = nupper;
    }
    goto partition;
    /* NOTREACHED */
}

static void insertion_sort(void *bot, size_t nmemb, size_t size, int (*compar)(const void*, const void*))
{
    register int cnt;
    register unsigned char ch;
    register char *s1, *s2, *t1, *t2, *top;

    /*
     * A simple insertion sort (see Knuth, Vol. 3, page 81, Algorithm
     * S).  Insertion sort has the same worst case as most simple sorts
     * (O N^2).  It gets used here because it is (O N) in the case of
     * sorted data.
     */
    top = bot + nmemb * size;
    for (t1 = bot + size; t1 < top;) {
        for (t2 = t1; (t2 -= size) >= bot && compar(t1, t2) < 0;);
        if (t1 != (t2 += size)) {
            /* Bubble bytes up through each element. */
            for (cnt = size; cnt--; ++t1) {
                ch = *t1;
                for (s1 = s2 = t1; (s2 -= size) >= t2; s1 = s2)
                    *s1 = *s2;
                *s1 = ch;
            }
        } else
            t1 += size;
    }
}

void abort()
{
    exit();
}
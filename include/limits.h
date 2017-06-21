//
// Created by freeman on 17-6-16.
//

#ifndef LIMITS_H
#define LIMITS_H

#define	_POSIX_ARG_MAX		4096
#define	_POSIX_CHILD_MAX	6
#define	_POSIX_LINK_MAX		8
#define	_POSIX_MAX_CANON	255
#define	_POSIX_MAX_INPUT	255
#define	_POSIX_NAME_MAX		14
#define	_POSIX_NGROUPS_MAX	0
#define	_POSIX_OPEN_MAX		16
#define	_POSIX_PATH_MAX		255
#define	_POSIX_PIPE_BUF		512

#define	_POSIX2_BC_BASE_MAX	99
#define	_POSIX2_BC_DIM_MAX	2048
#define	_POSIX2_BC_SCALE_MAX	99
#define	_POSIX2_BC_STRING_MAX	1000
#define	_POSIX2_EQUIV_CLASS_MAX	2
#define	_POSIX2_EXPR_NEST_MAX	32
#define	_POSIX2_LINE_MAX	2048
#define	_POSIX2_RE_DUP_MAX	255

#define	CHAR_BIT	8		/* number of bits in a char */
#define	MB_LEN_MAX	6		/* allow 21-bit UTF2 */

#define SCHAR_MIN	(-0x7f-1)	/* max value for a signed char */
#define	SCHAR_MAX	0x7f		/* min value for a signed char */

#define	UCHAR_MAX	0xff		/* max value for an unsigned char */
#define	CHAR_MAX	0x7f		/* max value for a char */
#define	CHAR_MIN	(-0x7f-1)	/* min value for a char */

#define	USHRT_MAX	0xffff		/* max value for an unsigned short */
#define	SHRT_MAX	0x7fff		/* max value for a short */
#define SHRT_MIN        (-0x7fff-1)     /* min value for a short */

#define	UINT_MAX	0xffffffff	/* max value for an unsigned int */
#define	INT_MAX		0x7fffffff	/* max value for an int */
#define	INT_MIN		(-0x7fffffff-1)	/* min value for an int */

#define	ULONG_MAX	0xffffffff	/* max value for an unsigned long */
#define	LONG_MAX	0x7fffffff	/* max value for a long */
#define	LONG_MIN	(-0x7fffffff-1)	/* min value for a long */

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
#define	CLK_TCK		128		/* ticks per second */
#define	UQUAD_MAX	0xffffffffffffffffLL		/* max unsigned quad */
#define	QUAD_MAX	0x7fffffffffffffffLL		/* max signed quad */
#define	QUAD_MIN	(-0x7fffffffffffffffLL-1)	/* min signed quad */

#define	ARG_MAX		20480	/* max bytes for an exec function */
#define	CHILD_MAX	40	/* max simultaneous processes */
#define	LINK_MAX	32767	/* max file link count */
#define	MAX_CANON	255	/* max bytes in terminal canonical input line */
#define	MAX_INPUT	255	/* max bytes in terminal input */
#define	NAME_MAX	255	/* max number of bytes in a file name */
#define	NGROUPS_MAX	16	/* max number of supplemental group id's */
#define	OPEN_MAX	64	/* max open files per process */
#define	PATH_MAX	1024	/* max number of bytes in pathname */
#define	PIPE_BUF	512	/* max number of bytes for atomic pipe writes */

#define	BC_BASE_MAX	99	/* max ibase/obase values allowed by bc(1) */
#define	BC_DIM_MAX	2048	/* max array elements allowed by bc(1) */
#define	BC_SCALE_MAX	99	/* max scale value allowed by bc(1) */
#define	BC_STRING_MAX	1000	/* max const string length allowed by bc(1) */
#define	EQUIV_CLASS_MAX	2	/* max weights for order keyword; see locale */
#define	EXPR_NEST_MAX	32	/* max expressions nested in expr(1) */
#define	LINE_MAX	2048	/* max length in bytes of an input line */
#define	RE_DUP_MAX	255	/* max repeated RE's using interval notation */

#endif

#endif //LIMITS_H

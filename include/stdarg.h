#ifndef STDARG_H
#define STDARG_H

typedef char *va_list;
#define	va_arg(ap, type) ((type *)(ap += sizeof(type)))[-1]
#define	va_end(ap)
#define	__va_promote(type) (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define	va_start(ap, last) (ap = ((char *)&(last) + __va_promote(last)))

typedef unsigned char u_char;
typedef	unsigned long u_long;

#endif

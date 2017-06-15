#ifndef STRING_H
#define STRING_H

#include <stddef.h>

unsigned int strlen(const char *str);
char * strchr(register const char *p, register const char ch);
char *strrchr(register const char *p, register const char *ch);
char *strcat(register char *s, register const char *append);
char *strncat(char *dst, const char *src, register unsigned int n);
int strcmp(register const char *s1, register const char *s2);
int strncmp(register const char *s1, register const char *s2, register unsigned int n);
char* strcpy(register char *to, register const char *from);
char *strncpy(char *dst, const char *src, register unsigned int n);
char *strstr(register const char *s, register const char *find);
void *memcpy(void *dst0, const void *src0, register unsigned int length);
void *memmove(void *vdst, const void *vsrc, register unsigned int n);
void *memccpy(void *t, const void *f, int c, register unsigned int n);
void *memchr(const void *s, register unsigned char c, register unsigned int n);
int memcmp(const void *s1, const void *s2, unsigned int n);
void* memset(void*, int, unsigned int);

#endif
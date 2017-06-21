#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void* malloc(unsigned int);
void free(void*);
int abs(int);

#define RAND_MAX 0x7fffffff

void srand(unsigned);
int rand();

void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
void abort();
#endif
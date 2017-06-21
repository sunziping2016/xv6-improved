#pragma once

#include "etypes.h"

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

#define MIN(a, b) ((a<b)?a:b)

#define MAX(a, b) ((a>b)?a:b)

#ifndef NULL
#define NULL 0
#endif

#define true 1
#define false 0

extern int strcmp(const char* str1, const char* str2);

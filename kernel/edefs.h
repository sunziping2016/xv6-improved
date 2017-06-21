/** @file
 * @brief Kernel definitions
 * @author Qifan Lu
 * @date June 22, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"

//[ Macros ]
/**
 * Array size
 */
#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

/**
 * Minimum value
 */
#define MIN(a, b) ((a<b)?a:b)

/**
 * Maximum value
 */
#define MAX(a, b) ((a>b)?a:b)

//[ Constants ]
/**
 * Null
 */
#ifndef NULL
#define NULL 0
#endif

/**
 * True & false
 */
#define true 1
#define false 0

//[ Functions ]
/**
 * Compare two string
 *
 * @param str1 String I
 * @param str2 String II
 * @return -1 if str1<str2, 0 if str1==str2 or 1 if str1>str2
 */
extern int strcmp(const char* str1, const char* str2);

/** @file
 * @brief Kernel data types
 * @author Qifan Lu
 * @date June 22, 2016
 * @version 1.0.0
 */
#pragma once

//[ Types ]
//GCC
#ifdef __GNUC__
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __SIZE_TYPE__ size_t;
//Other compilers (Not supported)
#else
#error Unsupported compiler: No type definitions available.
#endif

/**
 * Boolean type
 */
typedef _Bool bool;

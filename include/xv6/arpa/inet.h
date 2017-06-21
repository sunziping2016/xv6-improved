/** @file
 * @brief Definitions for internet operations.
 * @author Qifan Lu
 * @date April 11, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Libxv6
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>

//[ Functions ]
/**
 * Convert from local endianness to network endianness (32-bit)
 *
 * @param hostlong 32-bit number to be converted
 * @return 32-bit big endian (Network endianness) number
 */
extern uint32_t htonl(uint32_t hostlong);

/**
 * Convert from local endianness to network endianness (16-bit)
 * @param hostshort 16-bit number to be converted
 * @return 16-bit big endian (Network endianness) number
 */
extern uint16_t htons(uint16_t hostshort);

/**
 * Convert from network endianness to local endianness (32-bit)
 *
 * @param netlong 32-bit number to be converted
 * @return 32-bit little endian (Local endianness) number
 */
extern uint32_t ntohl(uint32_t netlong);

/**
 * Convert from network endianness to local endianness (16-bit)
 *
 * @param netshort 16-bit number to be converted
 * @return 16-bit little endian (Local endianness) number
 */
extern uint16_t ntohs(uint16_t netshort);

/**
 * Convert IPv4 string to 32-bit network endianness IPv4 address.
 *
 * @param addr_str IPv4 address string
 * @return 32-bit IPv4 address on success, INADDR_NONE on failure
 */
extern in_addr_t inet_addr(const char* addr_str);

/**
 * Convert a 32-bit network endianness IPv4 address to a string.
 *
 * Convert a 32-bit network endianness IPv4 address to a string.
 * (The string is returned in a statically allocated buffer, which subsequent calls will overwrite)
 *
 * @param addr 32-bit network endianness network address
 * @return A string representing given network address
 */
extern char* inet_ntoa(in_addr_t addr);

/**
 * Convert numeric internet address to a string
 *
 * @param af Address family
 * @param src Numeric internet address buffer
 * @param dst Result string buffer
 * @param size Size of the result string buffer
 * @return Pointer to buffer on success, or NULL on failure. Errno is set to indicate error.
 */
extern const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);

/**
 * Convert a network address string to a numeric internet address
 *
 * @param af Address family
 * @param src Network address string buffer
 * @param dst Numeric network address buffer
 * @return 1 for success, 0 for invaild address and -1 for error.
 */
extern int inet_pton(int af, const char* src, void* dst);

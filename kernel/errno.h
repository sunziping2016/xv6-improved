/** @file
 * @brief Kernel error numbers
 * @author Qifan Lu
 * @date June 22, 2016
 * @version 1.0.0
 */
#pragma once

//[ Constants ]
//* Kernel error numbers
/**
 * No such file or directory
 */
#define	ENOENT 2
/**
 * Bad file number
 */
#define	EBADF 9
/**
 * Not enough memory
 */
#define ENOMEM 12
/**
 * Bad address
 */
#define	EFAULT 14
/**
 * Invalid argument
 */
#define	EINVAL 22
/**
 * Too many open files in system
 */
#define	ENFILE 23
/**
 * Too many open files
 */
#define	EMFILE 24
/**
 * Operation not supported on transport endpoint
 */
#define EOPNOTSUPP 95
/**
 * No buffer space available
 */
#define ENOBUFS 105
/**
 * Address family not supported by protocol family
 */
#define EAFNOSUPPORT 106
/**
 * Protocol wrong type for socket
 */
#define EPROTOTYPE 107
/**
 * Socket operation on non-socket
 */
#define ENOTSOCK 108
/**
 * Address already in use
 */
#define EADDRINUSE 112
/**
 * Host is unreachable
 */
#define EHOSTUNREACH 118
/**
 * Socket already binded or connected
 */
#define EALREADY 120
/**
 * Destination address required
 */
#define EDESTADDRREQ 121
/**
 * Message too long
 */
#define EMSGSIZE 122
/**
 * Unknown protocol
 */
#define EPROTONOSUPPORT 123
/**
 * Address not available
 */
#define EADDRNOTAVAIL 125
/**
 * Socket is not connected
 */
#define ENOTCONN 128

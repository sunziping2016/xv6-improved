/** @file
 * @brief Unix domain socket definitions
 * @author Qifan Lu
 * @date April 11, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Libxv6
#include <sys/socket.h>

//[ Constants ]
/**
 * Unix domain socket maximum path length
 */
#define XV6_UNIX_SOCK_PATH_MAX 108

//[ Types ]
/**
 * Unix domain socket address type
 */
struct sockaddr_un
{   /**
     * Socket address family (Always "AF_UNIX")
     */
    sa_family_t sun_family;
    /**
     * Socket pathname
     */
    char sun_path[XV6_UNIX_SOCK_PATH_MAX];
};

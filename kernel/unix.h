/** @file
 * @brief Unix domain socket common definitions
 * @author Qifan Lu
 * @date April 21, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Xv6 kernel extra
#include "socket.h"

//[ Constants ]
//Unix domain socket maximum path length
#define UNIX_SOCK_PATH_MAX 108

//[ Types ]
//Unix domain socket address type
struct sockaddr_un
{   //Socket family (Always "AF_UNIX")
    sa_family_t sun_family;
    //Socket path name
    char sun_path[UNIX_SOCK_PATH_MAX];
};

//[ Functions ]
/**
 * Unix domain datagram socket module initialization
 */
extern void unix_dgram_module_init();

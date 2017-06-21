/** @file
 * @brief Internet protocol family declarations
 * @author Qifan Lu
 * @date April 11, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Libxv6
#include <stdint.h>
#include <xv6/sys/socket.h>

//[ Types ]
/**
 * Internet port type
 */
typedef uint16_t in_port_t;
/**
 * Internet address (v4) type
 */
typedef uint32_t in_addr_t;

/**
 * Internet protocol address type
 */
struct in_addr
{   /**
     * IP address (v4)
     */
    in_addr_t s_addr;
};

/**
 * Internet socket address type
 */
struct sockaddr_in
{   /**
     * Socket family (Always "AF_INET")
     */
    sa_family_t sin_family;
    /**
     * Socket port
     */
    in_port_t sin_port;
    /**
     * Internet protocol address
     */
    struct in_addr sin_addr;
};

//[ Constants ]
/**
 * Length of the string form for IPv4
 */
#define INET_ADDRSTRLEN 16

//* Special destinations
/**
 * Broadcast address ("255.255.255.255")
 */
const static struct in_addr INADDR_BROADCAST = {
    .s_addr = 0xffffffff
};

/**
 * Arbitrary address ("0.0.0.0")
 */
const static struct in_addr INADDR_ANY = {
    .s_addr = 0x0
};

/** @file
 * @brief Xv6 kernel internet module definitions
 * @author Qifan Lu
 * @date April 18, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"
#include "socket.h"

//[ Types ]
//Internet port type
typedef uint16_t in_port_t;
//Internet address (v4) type
typedef uint32_t in_addr_t;

//Internet protocol address type
struct in_addr
{   //IP address (v4)
    in_addr_t s_addr;
};

//Internet socket address type
struct sockaddr_in
{   //Socket family (Always "AF_INET")
    sa_family_t sin_family;
    //Socket port
    in_port_t sin_port;
    //Internet protocol address
    struct in_addr sin_addr;
};

/**
 * IP packet header type
 */
typedef struct ip4_hdr
{   /**
     * Version (1-bit) | IP header size (1-bit) (usually 0x45)
     */
    uint8_t ver_ihl;
    /**
     * Usually 0x00 (Ignored)
     */
    uint8_t tos;
    /**
     * IP header plus IP data size
     */
    uint16_t length;
    /**
     * Identification for fragment data
     */
    uint16_t id;
    /**
     * Flags (3-bit) | Fragment offset (13-bit)
     */
    uint16_t flag_fo;
    /**
     * Time to live
     */
    uint8_t ttl;
    /**
     * Network protocol
     */
    uint8_t protocol;
    /**
     * IP packet checksum
     */
    uint16_t checksum;
    /**
     * Source IP address
     */
    uint8_t src[4];
    /**
     * Destionation IP address
     */
    uint8_t dst[4];
} ip4_hdr_t;

/**
 * UDP packet header type
 */
typedef struct udp_hdr
{   /**
     * Source port
     */
    uint16_t source_port;
    /**
     * Destination port
     */
    uint16_t dest_port;
    /**
     * Content length
     */
    uint16_t length;
    /**
     * UDP checksum
     */
    uint16_t checksum;
} udp_hdr_t;

//[ Constants ]
//* Transport layer protocol numbers
/**
 * ICMP
 */
#define IPP_ICMP 1
/**
 * TCP
 */
#define IPP_TCP 6
/**
 * UDP
 */
#define IPP_UDP 17

//[ Macros ]
//* IP address convertion
#define IP_A2I(array) (*((in_addr_t*)array))
#define IP_I2A(addr) ((uint8_t*)(&addr))

//* Data endianness convertion
#define HTONS(x) ((((x) >> 8) & 0xFF) | (((x) & 0xFF) << 8))
#define HTONL(x) \
    ((((x) >> 24) & 0xFFL) | (((x) >> 8) & 0xFF00L) | \
        (((x) << 8) & 0xFF0000L) | (((x) << 24) & 0xFF000000L))
#define NTOHS(x) HTONS(x)
#define NTOHL(x) HTONL(x)

//[ Functions ]
//* Local internet interface module (inet_local_iface.c)
//Send packet to routing module
extern int iface_local_send(ip4_hdr_t* ip_packet);
//Local internet interface module initialization
extern void iface_local_init();

//* UDP socket module (inet_dgram.c)
//UDP IP packet handler
extern int inet_dgram_packet_handler(ip4_hdr_t* ip_packet, unsigned int from_iface);
//UDP module initialization
extern void inet_dgram_module_init();

//* Internet utilities (inet.c)
//Calculate IPv4 header checksum
extern void ip4_checksum(ip4_hdr_t* hdr);
//Calculate UDP header checksum
extern void udp_checksum(ip4_hdr_t* ip, udp_hdr_t* udp, uint16_t* data);

#pragma once

#include "etypes.h"
#include "socket.h"

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr
{
    in_addr_t s_addr;
};

struct sockaddr_in
{
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};

typedef struct ip4_hdr
{
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t length;
    uint16_t id;
    uint16_t flag_fo;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src[4];
    uint8_t dst[4];
} ip4_hdr_t;

typedef struct udp_hdr
{
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} udp_hdr_t;


#define IPP_ICMP 1
#define IPP_TCP 6
#define IPP_UDP 17
#define IP_A2I(array) (*((in_addr_t*)array))
#define IP_I2A(addr) ((uint8_t*)(&addr))
#define HTONS(x) ((((x) >> 8) & 0xFF) | (((x) & 0xFF) << 8))
#define HTONL(x) \
    ((((x) >> 24) & 0xFFL) | (((x) >> 8) & 0xFF00L) | \
        (((x) << 8) & 0xFF0000L) | (((x) << 24) & 0xFF000000L))
#define NTOHS(x) HTONS(x)
#define NTOHL(x) HTONL(x)

extern int iface_local_send(ip4_hdr_t* ip_packet);
extern void iface_local_init();
extern int inet_dgram_packet_handler(ip4_hdr_t* ip_packet, unsigned int from_iface);
extern void inet_dgram_module_init();
extern void ip4_checksum(ip4_hdr_t* hdr);
extern void udp_checksum(ip4_hdr_t* ip, udp_hdr_t* udp, uint16_t* data);

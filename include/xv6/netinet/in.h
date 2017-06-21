#pragma once
#include <stdint.h>
#include <xv6/sys/socket.h>

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

#define INET_ADDRSTRLEN 16

const static struct in_addr INADDR_BROADCAST = {
    .s_addr = 0xffffffff
};

const static struct in_addr INADDR_ANY = {
    .s_addr = 0x0
};

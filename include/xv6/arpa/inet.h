#pragma once

#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>


extern uint32_t htonl(uint32_t hostlong);
extern uint16_t htons(uint16_t hostshort);
extern uint32_t ntohl(uint32_t netlong);
extern uint16_t ntohs(uint16_t netshort);
extern in_addr_t inet_addr(const char* addr_str);
extern char* inet_ntoa(in_addr_t addr);
extern const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
extern int inet_pton(int af, const char* src, void* dst);

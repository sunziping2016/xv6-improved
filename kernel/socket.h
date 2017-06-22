#pragma once

#include "xv6/types.h"
#include "xv6/fs.h"
#include "xv6/file.h"
#include "etypes.h"

#define SOCK_DGRAM 0
#define SOCK_STREAM 1
#define SOCK_RAW 2
#define SOCK_SEQPACKET 3
#define AF_INET 0
#define AF_INET6 1
#define AF_UNIX 2
#define MSG_PEEK 0x1

#define SOCKCALL_CREATE 0
#define SOCKCALL_BIND 1
#define SOCKCALL_SEND 2
#define SOCKCALL_RECV 3
#define SOCKCALL_CLOSE 4
#define SOCKCALL_OPER_MAX 4
#define SOCKADDR_LEN 0

typedef int32_t socklen_t;
typedef uint32_t sa_family_t;

struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[SOCKADDR_LEN];
};

struct sockcall_create_param
{   int domain;
    int type;
    int protocol;
};

struct sockcall_bind_param
{   const struct sockaddr* address;
    socklen_t address_len;
};

struct sockcall_send_param
{   void* message;
    size_t length;
    int flags;
    struct sockaddr* dest_addr;
    socklen_t dest_len;
};

struct sockcall_recv_param
{   void* buffer;
    size_t length;
    int flags;
    struct sockaddr* address;
    socklen_t* address_len;
};

struct socket
{
    int domain;
    int type;
};

typedef int (*sockcall_t)(struct file*, void*);

typedef struct
{ 
    int domain;
    int type;
    sockcall_t* impl;
} sockcall_impl_t;

extern int sockread(struct file* sock_file, char* buffer, int len);
extern int sockwrite(struct file* sock_file, char* data, int len);
extern int sockclose(struct file* sock_file);
extern int sockcall_impl_reg(sockcall_impl_t impl);

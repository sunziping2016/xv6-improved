
#pragma once

//[ Header Files ]
//Xv6 kernel
#include "xv6/types.h"
#include "xv6/fs.h"
#include "xv6/file.h"
//Xv6 kernel extra
#include "etypes.h"

//[ Constants ]
//* Protocols
//Datagram protocol
#define SOCK_DGRAM 0
//Stream protocol
#define SOCK_STREAM 1
//Raw socket
#define SOCK_RAW 2
//Sequenced-packet protocol
#define SOCK_SEQPACKET 3

//* Domains
//Internet domain (IPv4)
#define AF_INET 0
//Internet domain (IPv6)
#define AF_INET6 1
//Unix domain
#define AF_UNIX 2

//* Message control flags
//Leave received data in queue
#define MSG_PEEK 0x1

//* Socket system call ID
//Create
#define SOCKCALL_CREATE 0
//Bind
#define SOCKCALL_BIND 1
//Send
#define SOCKCALL_SEND 2
//Receive
#define SOCKCALL_RECV 3
//Close
#define SOCKCALL_CLOSE 4

//Maximum socket system call number
#define SOCKCALL_OPER_MAX 4

//Socket address field length
#define SOCKADDR_LEN 0

//[ Types ]
//Socket address object length type
typedef int32_t socklen_t;
//Socket address type
typedef uint32_t sa_family_t;

//Abstract socket address type
struct sockaddr
{   //Socket address family
    sa_family_t sa_family;
    //Socket address (Variable-length data)
    char sa_data[SOCKADDR_LEN];
};

//* Socket system call parameter types
//Socket creation parameters
struct sockcall_create_param
{   int domain;
    int type;
    int protocol;
};

//Socket binding parameters
struct sockcall_bind_param
{   const struct sockaddr* address;
    socklen_t address_len;
};

//Socket data sending parameters
struct sockcall_send_param
{   void* message;
    size_t length;
    int flags;
    struct sockaddr* dest_addr;
    socklen_t dest_len;
};

//Socket data receiving parameters
struct sockcall_recv_param
{   void* buffer;
    size_t length;
    int flags;
    struct sockaddr* address;
    socklen_t* address_len;
};

//* Kernel socket module types
/**
 * Socket type
 */
struct socket
{   /**
     * Socket domain
     */
    int domain;
    /**
     * Socket protocol
     */
    int type;
};

/**
 * Socket system call function type
 */
typedef int (*sockcall_t)(struct file*, void*);

/**
 * Socket system call implementation type
 */
typedef struct
{   /**
     * Domain of the socket
     */
    int domain;
    /**
     * Protocol of the socket
     */
    int type;
    /**
     * Implementation functions
     */
    sockcall_t* impl;
} sockcall_impl_t;

//[ Functions ]
/**
 * Receive data from socket with read system call.
 *
 * @param sock_file Socket file descriptor object.
 * @param buffer Buffer to hold received data.
 * @param len Length of the buffer.
 * TODO: @return
 */
extern int sockread(struct file* sock_file, char* buffer, int len);

/**
 * Send data to socket with write system call.
 *
 * @param sock_file Socket file descriptor object.
 * @param data Data to be sent.
 * @param len Length of the data.
 * TODO: @return
 */
extern int sockwrite(struct file* sock_file, char* data, int len);

/**
 * Close given socket.
 *
 * @param sock_file Socket file descriptor object.
 * @return 0 for success, error number for failure.
 */
extern int sockclose(struct file* sock_file);

/**
 * Register socket system call implementation for given socket type.
 *
 * @param impl Socket system call implementation.
 * @return 0 for success, non-zero error number for failure.
 */
extern int sockcall_impl_reg(sockcall_impl_t impl);

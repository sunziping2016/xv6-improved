/** @file
 * @brief Xv6 socket API declaration.
 * @author Qifan Lu
 * @date April 10, 2016
 * @version 1.0.0
 */
#pragma once

//[ Header Files ]
//Libxv6
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

//[ Constants ]
//* Protocols
/**
 * Datagram protocol
 */
#define SOCK_DGRAM 0
/**
 * Stream protocol
 */
#define SOCK_STREAM 1
/**
 * Raw socket
 */
#define SOCK_RAW 2
/**
 * Sequenced-packet protocol
 */
#define SOCK_SEQPACKET 3

//* Domains
/**
 * Internet domain (IPv4)
 */
#define AF_INET 0
/**
 * Internet domain (IPv6)
 */
#define AF_INET6 1
/**
 * Unix domain
 */
#define AF_UNIX 2

//* Message control flags
/**
 * Leave received data in queue
 */
#define MSG_PEEK 0x1

/**
 * Socket address field length
 */
#define XV6_SOCKADDR_LEN 0

//[ Types ]
/**
 * Socket address object length type
 */
typedef int32_t socklen_t;
/**
 * Socket address type
 */
typedef uint32_t sa_family_t;

/**
 * Abstract socket address type
 */
struct sockaddr
{   /**
     * Socket address family
     */
    sa_family_t sa_family;
    /**
     * Socket address (Variable-length data)
     */
    char sa_data[XV6_SOCKADDR_LEN];
};

//[ Functions ]
/**
 * Bind socket to given address.
 *
 * @param socket Socket file descriptor.
 * @param sockaddr Address to be binded to.
 * @param address_len Length of the socket address structure.
 * @return 0 on success and 1 on failure. Error number is set when operation failed.
 */
extern int bind(int socket, const struct sockaddr* address, socklen_t address_len);

/**
 * Receive message from remote socket.
 *
 * Receive message from remote socket.
 * (Usually used with connection-based socket)
 *
 * @param socket Socket file descriptor.
 * @param buffer Buffer for receiving incoming data.
 * @param length Length of the buffer.
 * @param flags Special options for receiving data.
 * @return Length of the data received, or -1 for error. Error number is set when operation failed.
 */
extern ssize_t recv(int socket, void* buffer, size_t length, int flags);

/**
 * Receive data from remote socket, as well as remote address.
 *
 * Receive data from remote socket, as well as remote address.
 * (Usually used with connection-less socket)
 *
 * @param socket Socket file descriptor.
 * @param buffer Buffer for receiving incoming data.
 * @param length Length of the buffer.
 * @param flags Special options for receiving data.
 * @param address Used to receive the remote address.
 * @param address_len Used to receive the length of the remote address.
 * @return Length of the data received, or -1 for error. Error number is set when operation failed.
 */
extern ssize_t recvfrom(int socket, void* buffer, size_t length, int flags, struct sockaddr* address, socklen_t* address_len);

/**
 * Send data to remote socket.
 *
 * Send data to remote socket.
 * (Usually used with connection-based socket)
 *
 * @param socket Socket file descriptor.
 * @param message Message or data to be sent.
 * @param length Length of the message or data to be sent.
 * @param flags Special options for sending data.
 * @return Bytes of the data sent, or -1 for error. Error number is set when operation failed.
 */
extern ssize_t send(int socket, const void* message, size_t length, int flags);

/**
 * Send data to remote socket, with remote address given.
 *
 * Send data to remote socket, with remote address given.
 * (Usually used with connection-less socket)
 *
 * @param socket Socket file descriptor.
 * @param message Message or data to be sent.
 * @param length Length of the message or data to be sent.
 * @param flags Special options for sending data.
 * @param dest_addr Destination address.
 * @param dest_len Destination address object length.
 * @return Bytes of the data sent, or -1 for error. Error number is set when operation failed.
 */
extern ssize_t sendto(int socket, const void* message, size_t length, int flags, const struct sockaddr* dest_addr, socklen_t dest_len);

/**
 * Create a socket of given type, protocol and domain.
 *
 * @param domain Network domain.
 * @param type Network protocol.
 * @param protocol Network sub-protocol.
 * @return File descriptor on success, or -1 on fail. Error number is set when operation failed.
 */
extern int socket(int domain, int type, int protocol);

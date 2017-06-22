#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/fcntl.h"
#include "xv6/user.h"
#include "xv6/x86.h"
#include <xv6/sys/socket.h>
#include <xv6/sys/syscall.h>
//#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <xv6/netinet/in.h>

#define true 1
#define false 0

typedef int ssize_t;

char*
gets(char *buf, int max)
{
    int i, cc;
    char c;

    for (i = 0; i + 1 < max; ) {
        cc = read(0, &c, 1);
        if (cc < 1)
            break;
        buf[i++] = c;
        if (c == '\n' || c == '\r')
            break;
    }
    buf[i] = '\0';
    return buf;
}

int
stat(char *n, struct stat *st)
{
    int fd;
    int r;

    fd = open(n, O_RDONLY);
    if (fd < 0)
        return -1;
    r = fstat(fd, st);
    close(fd);
    return r;
}

int
atoi(const char *s)
{
    int n;

    n = 0;
    while ('0' <= *s && *s <= '9')
        n = n * 10 + *s++ - '0';
    return n;
}

struct __sockcall_create_param
{   int domain;
    int type;
    int protocol;
};

/**
 * Socket bind parameters
 */
struct __sockcall_bind_param
{   const struct sockaddr* address;
    socklen_t address_len;
};

/**
 * Socket send parameters
 */
struct __sockcall_send_param
{   const void* message;
    size_t length;
    int flags;
    const struct sockaddr* dest_addr;
    socklen_t dest_len;
};

/**
 * Socket receive parameters
 */
struct __sockcall_recv_param
{   void* buffer;
    size_t length;
    int flags;
    struct sockaddr* address;
    socklen_t* address_len;
};

//[ Constants ]
//* Socket system call ID
/**
 * Create
 */
#define XV6_SOCKCALL_CREATE 0
/**
 * Bind
 */
#define XV6_SOCKCALL_BIND 1
/**
 * Send
 */
#define XV6_SOCKCALL_SEND 2
/**
 * Receive
 */
#define XV6_SOCKCALL_RECV 3

//[ Functions ]
//Socket: Create socket for network communication
int socket(int domain, int type, int protocol)
{   //Create parameter object
    struct __sockcall_create_param param = {
        .domain = domain,
        .type = type,
        .protocol = protocol
    };
    //Do socket system call
    int result = sockcall(0, XV6_SOCKCALL_CREATE, &param);

    //Success
    if (result>=0)
        return result;
    //Failed
    else
    { 
        return -1;
    }
}

//Bind given socket to given address
int bind(int socket, const struct sockaddr* address, socklen_t address_len)
{   //Create parameter object
    struct __sockcall_bind_param param = {
        .address = address,
        .address_len = address_len
    };
    //Do socket system call
    int result = sockcall(socket, XV6_SOCKCALL_BIND, &param);

    //Success
    if (result==0)
        return 0;
    //Failed
    else
    {  
        return -1;
    }
}

//Send data to remote socket (No remote address given)
ssize_t send(int socket, const void* message, size_t length, int flags)
{   //Forward to "sendto" function
    return sendto(socket, message, length, flags, NULL, 0);
}

//Send data to remote socket, with remote address given
ssize_t sendto(int socket, const void* message, size_t length, int flags, const struct sockaddr* dest_addr, socklen_t dest_len)
{   //Create parameter object
    struct __sockcall_send_param param = {
        .message = message,
        .length = length,
        .flags = flags,
        .dest_addr = dest_addr,
        .dest_len = dest_len
    };
    //Do socket system call
    ssize_t result = sockcall(socket, XV6_SOCKCALL_SEND, &param);

    //Success
    if (result>=0)
        return result;
    //Failed
    else
    {  
        return -1;
    }
}

//Receive data from remote socket, as well as remote address
ssize_t recv(int socket, void* buffer, size_t length, int flags)
{   //Forward to "recvfrom" function
    return recvfrom(socket, buffer, length, flags, NULL, NULL);
}

//Receive data from remote socket, as well as remote address
ssize_t recvfrom(int socket, void* buffer, size_t length, int flags, struct sockaddr* address, socklen_t* address_len)
{   //Create parameter object
    struct __sockcall_recv_param param = {
        .buffer = buffer,
        .length = length,
        .flags = flags,
        .address = address,
        .address_len = address_len
    };
    //Do socket system call
    ssize_t result = sockcall(socket, XV6_SOCKCALL_RECV, &param);

    //Success
    if (result>=0)
        return result;
    //Failed
    else
    {   
        return -1;
    }
}

static char ipv4_str_buffer[INET_ADDRSTRLEN];

//[ Functions ]
/**
 * Swap bytes (16-bit number)
 */
static void swaps(uint16_t* num)
{   char* _num = (char*)num;
    uint8_t tmp;

    tmp = _num[0];
    _num[0] = _num[1];
    _num[1] = tmp;
}

/**
 * Swap bytes (32-bit number)
 */
static void swapl(uint32_t* num)
{   char* _num = (char*)num;
    uint8_t tmp;

    tmp = _num[0];
    _num[0] = _num[3];
    _num[3] = tmp;

    tmp = _num[1];
    _num[1] = _num[2];
    _num[2] = tmp;
}

//Convert from local endianness to network endianness (32-bit)
uint32_t htonl(uint32_t hostlong)
{   swapl(&hostlong);
    return hostlong;
}

//Convert from local endianness to network endianness (16-bit)
uint16_t htons(uint16_t hostshort)
{   swaps(&hostshort);
    return hostshort;
}

//Convert from network endianness to local endianness (32-bit)
uint32_t ntohl(uint32_t netlong)
{   swapl(&netlong);
    return netlong;
}

//Convert from network endianness to local endianness (16-bit)
uint16_t ntohs(uint16_t netshort)
{   swaps(&netshort);
    return netshort;
}

//Convert IPv4 string to 32-bit network endianness IPv4 address (Implementation)
static in_addr_t inet_addr_impl(const char* addr_str, bool* success)
{
    //TODO: Implement this function
    *success = false;
    return 0xffffffff;
}

//Convert IPv4 string to 32-bit network endianness IPv4 address (Wrapper)
in_addr_t inet_addr(const char* addr_str)
{   bool success;
    in_addr_t result = inet_addr_impl(addr_str, &success);

    //Failed
    if (!success)
        result = 0xffffffff;
    return result;
}

//Convert a 32-bit network endianness IPv4 address to a string
char* inet_ntoa(in_addr_t addr)
{   uint8_t* _addr = (uint8_t*)(&addr);
    int j = INET_ADDRSTRLEN-1;

    for (uint8_t i=0;i<4;i++)
    {   uint8_t current_num = _addr[3-i];
        //Print delimiter
        ipv4_str_buffer[j] = (i==0)?'\0':'.';
        j--;
        //Print number
        while (current_num!=0)
        {   ipv4_str_buffer[j] = current_num%10+'0';
            current_num /= 10;
            j--;
        }
    }

    return ipv4_str_buffer+j+1;
}

//Convert numeric internet address to a string
const char* inet_ntop(int af, const void* src, char* dst, socklen_t size)
{   //IPv4
    if (af==AF_INET)
    {   //Buffer too small
        if (size<INET_ADDRSTRLEN)
        {   
            return NULL;
        }

        char* result = inet_ntoa(*((const in_addr_t*)src));
        memmove(dst, result, INET_ADDRSTRLEN);
        return dst;
    }
    //Not implemented; not supported
    else
    {  
        return NULL;
    }
}

//Convert a network address string to a numeric internet address
int inet_pton(int af, const char* src, void* dst)
{   //IPv4
    if (af==AF_INET)
    {   bool success;
        in_addr_t result = inet_addr_impl(src, &success);

        //Failed
        if (!success)
            return 0;
        //Succeeded
        memmove(dst, &result, sizeof(in_addr_t));
        return 1;
    }
    //Not implemented; not supported
    else
    {   
        return -1;
    }
}


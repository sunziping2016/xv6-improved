

//[ Header Files ]
//Xv6 kernel
#include "xv6/types.h"
#include "xv6/defs.h"
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"
#include "mem_pool.h"
#include "errno.h"
#include "inet.h"
#include "socket.h"

//[ Constants ]
/**
 * UDP protocol memory pool size
 */
#define INET_DGRAM_POOL_SIZE 65536
/**
 * Message queue maximum size
 */
#define MSG_QUEUE_MAX_SIZE 4096
/**
 * Maximum message length
 */
#define INET_MAX_MSG_LEN 1024

//[ Types ]
/**
 * UDP datagram type
 */
struct inet_dgram_msg
{   /**
     * Source IP
     */
    in_addr_t src_ip;
    /**
     * Source port
     */
    in_port_t src_port;
    /**
     * Data length
     */
    uint32_t data_len;
    /**
     * Data
     */
    char* data;

    /**
     * Next message
     */
    struct inet_dgram_msg* next_msg;
};

/**
 * UDP socket type
 */
struct inet_dgram_socket
{   int domain;
    int type;

    /**
     * IP
     */
    in_addr_t ip;
    /**
     * Port number
     */
    in_port_t port;
    /**
     * Binded or not
     */
    bool binded;

    /**
     * Message queue begin
     */
    struct inet_dgram_msg* msg_queue_begin;
    /**
     * Message queue end
     */
    struct inet_dgram_msg* msg_queue_end;
    /**
     * Message queue size
     */
    uint32_t msg_queue_size;
    /**
     * Socket lock
     */
    struct spinlock lock;

    /**
     * Next UDP socket
     */
    struct inet_dgram_socket* next_sock;
    /**
     * Previous UDP socket
     */
    struct inet_dgram_socket* prev_sock;
};

//[ Variables ]
/**
 * UDP socket memory pool buffer
 */
static char inet_dgram_mem[INET_DGRAM_POOL_SIZE];
/**
 * UDP socket memory pool
 */
static struct mem_pool inet_dgram_pool;
/**
 * Socket linked list begin
 */
static struct inet_dgram_socket* inet_dgram_begin = NULL;
/**
 * Socket linked list end
 */
static struct inet_dgram_socket* inet_dgram_end = NULL;
/**
 * Socket linked list lock
 */
struct spinlock inet_dgram_lock;

//[ Functions ]
/**
 * Create UDP socket.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param (Dummy)
 * @return Zero for success, or negative error number for failure.
 */
static int inet_dgram_create(struct file* sock_file, void* _param)
{   //Allocate memory for UDP socket object
    struct inet_dgram_socket* sock = pool_alloc(&inet_dgram_pool, sizeof(struct inet_dgram_socket));
    //Memory allocation failed
    if (!sock)
        return -1*ENOMEM;

    //Initialize socket lock
    initlock(&sock->lock, "sock");
    //Initialize socket object
    sock->domain = AF_INET;
    sock->type = SOCK_DGRAM;
    sock->binded = false;
    sock->msg_queue_begin = sock->msg_queue_end = NULL;
    sock->msg_queue_size = 0;
    sock->next_sock = sock->prev_sock = NULL;
    //Insert into socket linked list
    acquire(&inet_dgram_lock);
    if (!inet_dgram_begin)
        inet_dgram_begin = inet_dgram_end = sock;
    else
    {   inet_dgram_end->next_sock = sock;
        sock->prev_sock = inet_dgram_end;
        inet_dgram_end = sock;
    }
    release(&inet_dgram_lock);
    //Set socket object
    sock_file->sock = (struct socket*)sock;

    //Return 0 for success
    return 0;
}

/**
 * Bind UDP socket to given IP and port
 *
 * @param sock_file Socket file descriptor object.
 * @param _param UDP socket binding parameters.
 * @return Zero for success, or negative error number for failure.
 */
static int inet_dgram_bind(struct file* sock_file, void* _param)
{   struct sockcall_bind_param* param = (struct sockcall_bind_param*)_param;
    //Bind address
    struct sockaddr_in* addr = (struct sockaddr_in*)param->address;
    //Socket object
    struct inet_dgram_socket* sock = (struct inet_dgram_socket*)sock_file->sock;

    //Check if the socket is already binded
    acquire(&sock->lock);
    if (sock->binded)
        return -1*EALREADY;

    //Walk up the whole socket linked list and check if the IP and the port is already in use
    acquire(&inet_dgram_lock);
    struct inet_dgram_socket* current_sock = inet_dgram_begin;
    while (current_sock)
    {   if ((current_sock->binded)&&(current_sock->ip==addr->sin_addr.s_addr)&&(current_sock->port==addr->sin_port))
            return -1*EADDRINUSE;
        current_sock = current_sock->next_sock;
    }
    release(&inet_dgram_lock);
    //Set socket IP, port and bind state
    sock->ip = addr->sin_addr.s_addr;
    sock->port = addr->sin_port;
    sock->binded = true;
    release(&sock->lock);

    //Return zero for success
    return 0;
}

/**
 * Send data to given address.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param UDP socket sending parameters.
 * @return Bytes of data sent on success, or negative error number for failure.
 */
static int inet_dgram_send(struct file* sock_file, void* _param)
{   struct sockcall_send_param* param = (struct sockcall_send_param*)_param;
    //Message and length
    const char* msg_content = (const char*)param->message;
    int msg_len = param->length;
    //Address
    struct sockaddr_in* addr = (struct sockaddr_in*)param->dest_addr;
    //Socket object
    struct inet_dgram_socket* sock = (struct inet_dgram_socket*)sock_file->sock;

    //No address specified
    if (!addr)
        return -1*EDESTADDRREQ;
    //Message cannot be accessed
    if (!msg_content)
        return -1*EFAULT;
    //Message length less than zero or too long
    if ((msg_len<=0)||(msg_len>INET_MAX_MSG_LEN))
        return -1*EMSGSIZE;

    //Create message object
    uint16_t packet_size = sizeof(ip4_hdr_t)+sizeof(udp_hdr_t)+msg_len;
    char* udp_msg = pool_alloc(&inet_dgram_pool, packet_size);
    if (!udp_msg)
        return -1*ENOMEM;
    //Pointers to headers and body
    ip4_hdr_t* msg_ip_hdr = (ip4_hdr_t*)udp_msg;
    udp_hdr_t* msg_udp_hdr = (udp_hdr_t*)(udp_msg+sizeof(ip4_hdr_t));
    char* msg_body = udp_msg+sizeof(ip4_hdr_t)+sizeof(udp_hdr_t);

    //Set IP headers
    msg_ip_hdr->ver_ihl = 0x45;
    msg_ip_hdr->tos = 0x0;
    msg_ip_hdr->length = HTONS(packet_size);
    msg_ip_hdr->id = 0;
    msg_ip_hdr->flag_fo = 0;
    msg_ip_hdr->ttl = 0xff;
    msg_ip_hdr->protocol = IPP_UDP;
    msg_ip_hdr->checksum = 0;
    memmove(msg_ip_hdr->src, IP_I2A(sock->ip), sizeof(in_addr_t));
    memmove(msg_ip_hdr->dst, IP_I2A(addr->sin_addr.s_addr), sizeof(in_addr_t));
    //Set UDP headers
    msg_udp_hdr->source_port = HTONS(sock->port);
    msg_udp_hdr->dest_port = HTONS(addr->sin_port);
    msg_udp_hdr->length = HTONS(sizeof(udp_hdr_t)+msg_len);
    msg_udp_hdr->checksum = 0;
    //Copy message data
    memmove(msg_body, msg_content, msg_len);
    //Calculate checksums
    ip4_checksum(msg_ip_hdr);
    udp_checksum(msg_ip_hdr, msg_udp_hdr, (uint16_t*)msg_body);

    //Send UDP packet through local interface
    int status = iface_local_send(msg_ip_hdr);
    //Release packet memory
    pool_free(&inet_dgram_pool, udp_msg);
    //Return bytes of data sent or error number
    return (status<0)?status:msg_len;
}

/**
 * Receive data on UDP socket.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param UDP socket receiving parameters.
 * @return Bytes of data sent on success, or negative error number for failure.
 */
static int inet_dgram_recv(struct file* sock_file, void* _param)
{   struct sockcall_recv_param* param = (struct sockcall_recv_param*)_param;
    //Socket object
    struct inet_dgram_socket* sock = (struct inet_dgram_socket*)sock_file->sock;

    //Not binded
    if (!sock->binded)
        return -1*ENOTCONN;
    //Invaild receive buffer
    if (!param->buffer)
        return -1*EFAULT;
    //Invaild buffer size
    if (param->length<=0)
        return -1*EINVAL;

    acquire(&sock->lock);
    //Fetch a message from queue
    struct inet_dgram_msg* msg = sock->msg_queue_begin;
    //Wait until there is a message coming in
    while (!msg)
    {   release(&sock->lock);
        yield();
        acquire(&sock->lock);
        msg = sock->msg_queue_begin;
    }

    //Pop next message from queue
    if ((param->flags&MSG_PEEK)==0)
    {   sock->msg_queue_begin = msg->next_msg;
        if (!msg->next_msg)
            sock->msg_queue_end = NULL;
        //Update message queue length and memory size
        sock->msg_queue_size -= sizeof(struct inet_dgram_msg)+msg->data_len;
    }
    release(&sock->lock);

    //Write message data into buffer
    int write_data_len = MIN(param->length, msg->data_len);
    memmove(param->buffer, msg->data, write_data_len);
    //Set sender address
    if ((param->address)&&(param->address_len))
    {   //No enough buffer
        if (*(param->address_len)<sizeof(struct sockaddr_in))
            *(param->address_len) = 0;
        else
        {   struct sockaddr_in* address = (struct sockaddr_in*)(param->address);
            //Copy IP and port
            address->sin_port = msg->src_port;
            address->sin_addr.s_addr = msg->src_ip;
            //Set actual address object length
            *(param->address_len) = sizeof(struct sockaddr_in);
        }
    }

    //Destroy message object
    if ((param->flags&MSG_PEEK)==0)
        pool_free(&inet_dgram_pool, msg);
    //Return bytes of data received
    return write_data_len;
}

/**
 * Close UDP socket
 *
 * @return Zero for success, non-zero value for error.
 */
static int inet_dgram_close(struct file* sock_file, void* _param)
{   //Get socket object
    struct inet_dgram_socket* sock = (struct inet_dgram_socket*)sock_file->sock;

    //Release all remaining messages
    acquire(&sock->lock);
    if (sock->msg_queue_begin)
    {   struct inet_dgram_msg* current_msg = sock->msg_queue_begin;
        struct inet_dgram_msg* old_msg;

        //Traverse through the message queue (linked list)
        while (current_msg)
        {   old_msg = current_msg;
            current_msg = current_msg->next_msg;
            //Release message objects
            pool_free(&inet_dgram_pool, old_msg);
        }
    }
    release(&sock->lock);

    acquire(&inet_dgram_lock);
    //Modify socket linked list
    if (sock->prev_sock)
        sock->prev_sock->next_sock = sock->next_sock;
    if (sock->next_sock)
        sock->next_sock->prev_sock = sock->prev_sock;
    if (inet_dgram_begin==sock)
        inet_dgram_begin = sock->next_sock;
    if (inet_dgram_end==sock)
        inet_dgram_end = sock->prev_sock;
    release(&inet_dgram_lock);
    //Release socket object memory
    pool_free(&inet_dgram_pool, sock);

    return 0;
}

/**
 * UDP IP packet handler
 *
 * @param ip_packet IP packet
 * @param from_iface Source internet interface
 */
int inet_dgram_packet_handler(ip4_hdr_t* ip_packet, unsigned int from_iface)
{   //Get packet pointer, UDP header and data
    char* _packet = (char*)ip_packet;
    udp_hdr_t* msg_udp_hdr = (udp_hdr_t*)(_packet+sizeof(ip4_hdr_t));
    char* data = _packet+sizeof(ip4_hdr_t)+sizeof(udp_hdr_t);

    acquire(&inet_dgram_lock);
    //Find socket with given IP and port
    struct inet_dgram_socket* current_sock = inet_dgram_begin;
    while (current_sock)
    {   if ((current_sock->ip==IP_A2I(ip_packet->dst))&&(current_sock->port==NTOHS(msg_udp_hdr->dest_port)))
            break;
        current_sock = current_sock->next_sock;
    }
    release(&inet_dgram_lock);
    //No socket found
    if (!current_sock)
        return -1*EHOSTUNREACH;

    acquire(&current_sock->lock);
    //Check socket buffer size
    uint16_t data_len = NTOHS(msg_udp_hdr->length)-sizeof(udp_hdr_t),
        use_space = data_len+sizeof(struct inet_dgram_msg);
    //No enough buffer available, drop packet
    if (current_sock->msg_queue_size+use_space>MSG_QUEUE_MAX_SIZE)
        return -1*ENOBUFS;

    //Create a new UDP message
    struct inet_dgram_msg* new_msg = pool_alloc(&inet_dgram_pool, use_space);
    //No memory
    if (!new_msg)
        return -1*ENOMEM;

    char* new_msg_data = ((char*)new_msg)+sizeof(struct inet_dgram_msg);
    //Copy UDP packet information
    new_msg->src_ip = IP_A2I(ip_packet->src);
    new_msg->src_port = NTOHS(msg_udp_hdr->source_port);
    new_msg->data_len = data_len;
    new_msg->next_msg = NULL;
    //Copy data
    memmove(new_msg_data, data, data_len);
    new_msg->data = new_msg_data;
    //Update socket information
    current_sock->msg_queue_end->next_msg = new_msg;
    current_sock->msg_queue_end = new_msg;
    if (!current_sock->msg_queue_begin)
        current_sock->msg_queue_begin = new_msg;
    current_sock->msg_queue_size += use_space;
    release(&current_sock->lock);

    return 0;
}

//[ Variables ]
/**
 * UDP socket operation functions
 */
static sockcall_t inet_dgram_funcs[] = {
    [SOCKCALL_CREATE]inet_dgram_create,
    [SOCKCALL_BIND]inet_dgram_bind,
    [SOCKCALL_SEND]inet_dgram_send,
    [SOCKCALL_RECV]inet_dgram_recv,
    [SOCKCALL_CLOSE]inet_dgram_close
};

//UDP module initialization
void inet_dgram_module_init()
{   //Register UDP socket implementation
    sockcall_impl_t inet_dgram_impl = {
        .domain = AF_INET,
        .type = SOCK_DGRAM,
        .impl = inet_dgram_funcs
    };
    sockcall_impl_reg(inet_dgram_impl);
    //Initialize memory pool
    pool_init(&inet_dgram_pool, inet_dgram_mem, sizeof(inet_dgram_mem));
    //Initialize global lock
    initlock(&inet_dgram_lock, "sock-global");
}


//[ Includes ]
//Xv6 kernel extra
#include "edefs.h"
#include "etypes.h"
#include "errno.h"
#include "mem_pool.h"
#include "socket.h"
#include "unix.h"
//Xv6 kernel
#include "xv6/types.h"
#include "xv6/param.h"
#include "xv6/defs.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/stat.h"

//[ Types ]
/**
 * Unix domain datagram message
 */
struct unix_dgram_msg
{   /**
     * Message data
     */
    char* data;
    /**
     * Message length
     */
    uint16_t data_len;

    /**
     * Source PID
     */
    int pid;
    /**
     * Source socket path
     */
    const char* sender_path;

    /**
     * Next message
     */
    struct unix_dgram_msg* next_msg;
};

/**
 * Unix domain datagram socket type
 */
struct unix_dgram_socket
{   int domain;
    int type;

    /**
     * Message queue begin
     */
    struct unix_dgram_msg* msg_queue_begin;
    /**
     * Message queue end
     */
    struct unix_dgram_msg* msg_queue_end;
    /**
     * Message queue size
     */
    uint16_t msg_queue_size;
    /**
     * Socket lock
     */
    struct spinlock lock;

    /**
     * Socket path (Address)
     */
    const char* path;
};

//[ Constants ]
/**
 * Unix domain datagram socket memory pool size
 */
#define UNIX_DGRAM_POOL_SIZE 65536
/**
 * Message queue maximum size
 */
#define MSG_QUEUE_MAX_SIZE 4096
/**
 * Maximum message length
 */
#define UNIX_MAX_MSG_LEN 2048
/**
 * Socket file content length
 */
#define SOCK_FILE_CONTENT_LEN (sizeof(struct socket*)+1)

//[ Variables ]
/**
 * Unix domain datagram socket memory pool buffer
 */
static char unix_dgram_mem[UNIX_DGRAM_POOL_SIZE];
/**
 * Unix domain datagram socket memory pool
 */
static struct mem_pool unix_dgram_pool;

//[ Functions ]
/**
 * Generate socket file content
 *
 * @param buffer Socket file content buffer
 * @param sock Socket pointer
 * @param dead Is the socket dead or not
 * @return Socket file content buffer
 */
static inline char* make_sock_file_content(char* buffer, struct socket* sock, bool alive)
{   memmove(buffer, (char*)(&sock), sizeof(&sock));
    buffer[sizeof(&sock)] = alive;
    return buffer;
}

/**
 * Read socket file content
 *
 * @param buffer Raw socket file data buffer
 * @param alive Used to return whether the socket is still alive
 * @return Socket structure pointer
 */
static inline struct socket* read_sock_file_content(char* buffer, bool* alive)
{   if (alive)
        *alive = buffer[sizeof(struct socket*)];
    return *((struct socket**)buffer);
}

/**
 * Create unix domain datagram socket.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param (Dummy)
 * @return Zero for success, or negative error number for failure.
 */
static int unix_dgram_create(struct file* sock_file, void* _param)
{   //Allocate memory for UNIX domain socket object
    struct unix_dgram_socket* sock = pool_alloc(&unix_dgram_pool, sizeof(struct unix_dgram_socket));
    //Memory allocation failed
    if (!sock)
        return -1*ENOMEM;

    //Initialize socket lock
    initlock(&sock->lock, "sock");
    //Initialize socket object
    sock->domain = AF_UNIX;
    sock->type = SOCK_DGRAM;
    sock->msg_queue_begin = sock->msg_queue_end = NULL;
    sock->msg_queue_size = 0;
    sock->path = NULL;
    //Set socket object
    sock_file->sock = (struct socket*)sock;

    //Return 0 for success
    return 0;
}

/**
 * Bind unix domain datagram socket to given path.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Unix domain datagram socket binding parameters.
 * @return Zero for success, or negative error number for failure.
 */
static int unix_dgram_bind(struct file* sock_file, void* _param)
{   struct sockcall_bind_param* param = (struct sockcall_bind_param*)_param;
    //Bind address
    struct sockaddr_un* addr = (struct sockaddr_un*)param->address;
    //Socket object
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    acquire(&sock->lock);
    //Allocate memory for socket address
    int path_len = strlen(addr->sun_path);
    sock->path = (char*)pool_alloc(&unix_dgram_pool, path_len+1);
    if (!sock->path)
        return -1*ENOMEM;
    //Copy socket path
    memmove(sock->path, addr->sun_path, path_len+1);
    release(&sock->lock);

    //Try to create a socket inode
    begin_op();
    struct inode* sock_inode = inode_find_create(addr->sun_path, T_SOCK, 0, 0);
    //Inode creation failed
    if (!sock_inode)
    {   end_op();
        //Parent folder not exists
        char buffer[50];
        if (nameiparent(addr->sun_path, buffer)==NULL)
            return -1*EADDRNOTAVAIL;
        //File already exists
        else
            return -1*EADDRINUSE;
    }

    //Socket exists at given inode
    char buffer[SOCK_FILE_CONTENT_LEN];
    if (readi(sock_inode, buffer, 0, SOCK_FILE_CONTENT_LEN)>0)
    {   iunlock(sock_inode);
        end_op();
        return -1*EADDRINUSE;
    }

    //Set socket on new inode
    writei(
        sock_inode,
        make_sock_file_content(buffer, (struct socket*)sock, true),
        0,
        SOCK_FILE_CONTENT_LEN
    );
    iupdate(sock_inode);
    iunlock(sock_inode);
    end_op();
    //Set inode on file descriptor
    sock_file->ip = sock_inode;

    //Return zero for success
    return 0;
}

/**
 * Send data to given address.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Unix domain datagram socket sending parameters.
 * @return Bytes of data sent on success, or negative error number for failure.
 */
static int unix_dgram_send(struct file* sock_file, void* _param)
{   struct sockcall_send_param* param = (struct sockcall_send_param*)_param;
    //Message and length
    const char* msg_content = (const char*)param->message;
    int msg_len = param->length;
    //Address
    struct sockaddr_un* addr = (struct sockaddr_un*)param->dest_addr;
    //Socket object
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    //No address specified
    if (!addr)
        return -1*EDESTADDRREQ;
    //Message cannot be accessed
    if (!msg_content)
        return -1*EFAULT;
    //Message length less than zero or too long
    if ((msg_len<=0)||(msg_len>UNIX_MAX_MSG_LEN))
        return -1*EMSGSIZE;

    //Find socket inode by given address
    struct inode* sock_inode = namei(addr->sun_path);
    //INode not found
    if (!sock_inode)
        return -1*ENOENT;

    ilock(sock_inode);
    //INode type incorrect
    if (sock_inode->type!=T_SOCK)
        return -1*ENOTSOCK;
    //Read socket information
    bool alive;
    char buffer[SOCK_FILE_CONTENT_LEN];
    readi(sock_inode, buffer, 0, SOCK_FILE_CONTENT_LEN);
    iunlock(sock_inode);
    struct socket* _dest_sock = read_sock_file_content(buffer, &alive);
    //Socket dead
    if (!alive)
        return -1*EHOSTUNREACH;
    //Protocol type incorrect
    if (_dest_sock->type!=SOCK_DGRAM)
        return -1*EPROTOTYPE;

    //Destination socket
    struct unix_dgram_socket* dest_sock = (struct unix_dgram_socket*)_dest_sock;
    //Allocate memory for message
    int msg_obj_len = sizeof(struct unix_dgram_msg);
    int use_space = msg_len+msg_obj_len;
    char* new_msg_mem = (char*)pool_alloc(&unix_dgram_pool, use_space);
    if (!new_msg_mem)
        return -1*ENOMEM;
    //Copy message content
    memmove(new_msg_mem+msg_obj_len, msg_content, msg_len);

    //New message object
    struct unix_dgram_msg* new_msg = (struct unix_dgram_msg*)new_msg_mem;
    //Set message content
    new_msg->data = new_msg_mem+msg_obj_len;
    new_msg->data_len = msg_len;
    //Set socket path and process PID
    new_msg->sender_path = sock->path;
    new_msg->pid = proc->pid;
    //Next message
    new_msg->next_msg = NULL;

    acquire(&dest_sock->lock);
    //Wait until destination socket read
    while (dest_sock->msg_queue_size>MSG_QUEUE_MAX_SIZE)
    {   release(&dest_sock->lock);
        yield();
        acquire(&dest_sock->lock);
    }

    //Save message to message queue
    if (!dest_sock->msg_queue_begin)
        dest_sock->msg_queue_begin = dest_sock->msg_queue_end = new_msg;
    else
    {   dest_sock->msg_queue_end->next_msg = new_msg;
        dest_sock->msg_queue_end = new_msg;
    }
    //Update sending queue size
    dest_sock->msg_queue_size += use_space;
    release(&dest_sock->lock);

    //Success
    return msg_len;
}

/**
 * Receive data on Unix domain datagram socket.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Unix domain datagram socket receiving parameters.
 * @return Bytes of data sent on success, or negative error number for failure.
 */
static int unix_dgram_recv(struct file* sock_file, void* _param)
{   struct sockcall_recv_param* param = (struct sockcall_recv_param*)_param;
    //Socket object
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    //Not binded to path
    if (!sock->path)
        return -1*ENOTCONN;
    //Invaild receive buffer
    if (!param->buffer)
        return -1*EFAULT;
    //Invaild buffer size
    if (param->length<=sizeof(sa_family_t))
        return -1*EINVAL;

    acquire(&sock->lock);
    //Fetch a message from queue
    struct unix_dgram_msg* msg = sock->msg_queue_begin;
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
        sock->msg_queue_size -= sizeof(struct unix_dgram_msg)+msg->data_len;
    }
    release(&sock->lock);

    //Write message data into buffer
    int write_data_len = MIN(param->length, msg->data_len);
    memmove(param->buffer, msg->data, write_data_len);
    //Set sender address
    if ((param->address)&&(param->address_len)&&(msg->sender_path))
    {   int addr_avail_len = *(param->address_len)-sizeof(sa_family_t);
        int sender_path_len = strlen(msg->sender_path)+1;
        int write_addr_len = MIN(addr_avail_len, sender_path_len);
        struct sockaddr_un* address = (struct sockaddr_un*)(param->address);
        address->sun_family = AF_UNIX;
        //Copy sender path
        memmove(address->sun_path, msg->sender_path, write_addr_len);
        //Set actual address object length
        *(param->address_len) = write_addr_len+sizeof(sa_family_t);
    }

    //Destroy message object
    if ((param->flags&MSG_PEEK)==0)
        pool_free(&unix_dgram_pool, msg);
    //Return bytes of data received
    return write_data_len;
}

/**
 * Close unix domain datagram socket.
 *
 * @return Zero for success, non-zero value for error.
 */
static int unix_dgram_close(struct file* sock_file, void* _param)
{   //Get socket object
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    //Release all remaining messages
    acquire(&sock->lock);
    if (sock->msg_queue_begin)
    {   struct unix_dgram_msg* current_msg = sock->msg_queue_begin;
        struct unix_dgram_msg* old_msg;

        //Traverse through the message queue (linked list)
        while (current_msg)
        {   old_msg = current_msg;
            current_msg = current_msg->next_msg;
            //Release message objects
            pool_free(&unix_dgram_pool, old_msg);
        }
    }
    release(&sock->lock);

    //Release socket path memory
    if (sock->path)
        pool_free(&unix_dgram_pool, sock->path);
    //Release socket object memory
    pool_free(&unix_dgram_pool, sock);

    //Indicate socket is dead in inode
    if (sock_file->ip)
    {   begin_op();
        ilock(sock_file->ip);
        char buffer[SOCK_FILE_CONTENT_LEN];
        writei(
            sock_file->ip,
            make_sock_file_content(buffer, NULL, false),
            0,
            SOCK_FILE_CONTENT_LEN
        );
        iupdate(sock_file->ip);
        iunlock(sock_file->ip);
        end_op();
    }

    return 0;
}

//[ Variables ]
/**
 * Unix domain datagram socket operation functions
 */
static sockcall_t unix_dgram_funcs[] = {
    [SOCKCALL_CREATE]unix_dgram_create,
    [SOCKCALL_BIND]unix_dgram_bind,
    [SOCKCALL_SEND]unix_dgram_send,
    [SOCKCALL_RECV]unix_dgram_recv,
    [SOCKCALL_CLOSE]unix_dgram_close
};

//[ Functions ]
//Unix domain datagram socket module initialization
void unix_dgram_module_init()
{   //Register unix domain datagram socket implementation
    sockcall_impl_t unix_dgram_impl = {
        .domain = AF_UNIX,
        .type = SOCK_DGRAM,
        .impl = unix_dgram_funcs
    };
    sockcall_impl_reg(unix_dgram_impl);
    //Initialize memory pool
    pool_init(&unix_dgram_pool, unix_dgram_mem, sizeof(unix_dgram_mem));
}

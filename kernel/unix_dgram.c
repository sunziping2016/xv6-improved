#include "edefs.h"
#include "etypes.h"
#include "errno.h"
#include "mem_pool.h"
#include "socket.h"
#include "unix.h"
#include "xv6/types.h"
#include "xv6/param.h"
#include "xv6/defs.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/stat.h"

struct unix_dgram_msg
{ 
    char* data;
    uint16_t data_len;
    int pid;
    const char* sender_path;
    struct unix_dgram_msg* next_msg;
};

struct unix_dgram_socket
{   int domain;
    int type;
    struct unix_dgram_msg* msg_queue_begin;
    struct unix_dgram_msg* msg_queue_end;
    uint16_t msg_queue_size;
    struct spinlock lock;
    const char* path;
};

#define UNIX_DGRAM_POOL_SIZE 65536
#define MSG_QUEUE_MAX_SIZE 4096
#define UNIX_MAX_MSG_LEN 2048
#define SOCK_FILE_CONTENT_LEN (sizeof(struct socket*)+1)

static char unix_dgram_mem[UNIX_DGRAM_POOL_SIZE];
static struct mem_pool unix_dgram_pool;
static inline char* make_sock_file_content(char* buffer, struct socket* sock, bool alive)
{   memmove(buffer, (char*)(&sock), sizeof(&sock));
    buffer[sizeof(&sock)] = alive;
    return buffer;
}

static inline struct socket* read_sock_file_content(char* buffer, bool* alive)
{   if (alive)
        *alive = buffer[sizeof(struct socket*)];
    return *((struct socket**)buffer);
}

static int unix_dgram_create(struct file* sock_file, void* _param)
{  
    struct unix_dgram_socket* sock = pool_alloc(&unix_dgram_pool, sizeof(struct unix_dgram_socket));
   
    if (!sock)
        return -1*ENOMEM;

    initlock(&sock->lock, "sock");
    sock->domain = AF_UNIX;
    sock->type = SOCK_DGRAM;
    sock->msg_queue_begin = sock->msg_queue_end = NULL;
    sock->msg_queue_size = 0;
    sock->path = NULL;
    sock_file->sock = (struct socket*)sock;


    return 0;
}


static int unix_dgram_bind(struct file* sock_file, void* _param)
{   
    struct sockcall_bind_param* param = (struct sockcall_bind_param*)_param;
    struct sockaddr_un* addr = (struct sockaddr_un*)param->address;
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    acquire(&sock->lock);
    int path_len = strlen(addr->sun_path);
    sock->path = (char*)pool_alloc(&unix_dgram_pool, path_len+1);
    if (!sock->path)
        return -1*ENOMEM;
    memmove(sock->path, addr->sun_path, path_len+1);
    release(&sock->lock);

    begin_op();
    struct inode* sock_inode = inode_find_create(addr->sun_path, T_SOCK, 0, 0);
    if (!sock_inode)
    {   end_op();
        char buffer[50];
        if (nameiparent(addr->sun_path, buffer)==NULL)
            return -1*EADDRNOTAVAIL;
        else
            return -1*EADDRINUSE;
    }

    char buffer[SOCK_FILE_CONTENT_LEN];
    if (readi(sock_inode, buffer, 0, SOCK_FILE_CONTENT_LEN)>0)
    {   iunlock(sock_inode);
        end_op();
        return -1*EADDRINUSE;
    }

    writei(
        sock_inode,
        make_sock_file_content(buffer, (struct socket*)sock, true),
        0,
        SOCK_FILE_CONTENT_LEN
    );
    iupdate(sock_inode);
    iunlock(sock_inode);
    end_op();
    sock_file->ip = sock_inode;

    return 0;
}

static int unix_dgram_send(struct file* sock_file, void* _param)
{   struct sockcall_send_param* param = (struct sockcall_send_param*)_param;
    const char* msg_content = (const char*)param->message;
    int msg_len = param->length;
    struct sockaddr_un* addr = (struct sockaddr_un*)param->dest_addr;
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;
    if (!addr)
        return -1*EDESTADDRREQ;
    if (!msg_content)
        return -1*EFAULT;
    if ((msg_len<=0)||(msg_len>UNIX_MAX_MSG_LEN))
        return -1*EMSGSIZE;

    struct inode* sock_inode = namei(addr->sun_path);
    if (!sock_inode)
        return -1*ENOENT;

    ilock(sock_inode);
    if (sock_inode->type!=T_SOCK)
        return -1*ENOTSOCK;
    bool alive;
    char buffer[SOCK_FILE_CONTENT_LEN];
    readi(sock_inode, buffer, 0, SOCK_FILE_CONTENT_LEN);
    iunlock(sock_inode);
    struct socket* _dest_sock = read_sock_file_content(buffer, &alive);
    if (!alive)
        return -1*EHOSTUNREACH;
    if (_dest_sock->type!=SOCK_DGRAM)
        return -1*EPROTOTYPE;

    struct unix_dgram_socket* dest_sock = (struct unix_dgram_socket*)_dest_sock;
    int msg_obj_len = sizeof(struct unix_dgram_msg);
    int use_space = msg_len+msg_obj_len;
    char* new_msg_mem = (char*)pool_alloc(&unix_dgram_pool, use_space);
    if (!new_msg_mem)
        return -1*ENOMEM;
    memmove(new_msg_mem+msg_obj_len, msg_content, msg_len);

    struct unix_dgram_msg* new_msg = (struct unix_dgram_msg*)new_msg_mem;
    new_msg->data = new_msg_mem+msg_obj_len;
    new_msg->data_len = msg_len;
    new_msg->sender_path = sock->path;
    new_msg->pid = proc->pid;
    new_msg->next_msg = NULL;

    acquire(&dest_sock->lock);
    while (dest_sock->msg_queue_size>MSG_QUEUE_MAX_SIZE)
    {   release(&dest_sock->lock);
        yield();
        acquire(&dest_sock->lock);
    }

    if (!dest_sock->msg_queue_begin)
        dest_sock->msg_queue_begin = dest_sock->msg_queue_end = new_msg;
    else
    {   dest_sock->msg_queue_end->next_msg = new_msg;
        dest_sock->msg_queue_end = new_msg;
    }
    dest_sock->msg_queue_size += use_space;
    release(&dest_sock->lock);
    return msg_len;
}

static int unix_dgram_recv(struct file* sock_file, void* _param)
{   struct sockcall_recv_param* param = (struct sockcall_recv_param*)_param;
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;

    if (!sock->path)
        return -1*ENOTCONN;
    if (!param->buffer)
        return -1*EFAULT;
    if (param->length<=sizeof(sa_family_t))
        return -1*EINVAL;

    acquire(&sock->lock);
    struct unix_dgram_msg* msg = sock->msg_queue_begin;
    while (!msg)
    {   release(&sock->lock);
        yield();
        acquire(&sock->lock);
        msg = sock->msg_queue_begin;
    }

    if ((param->flags&MSG_PEEK)==0)
    {   sock->msg_queue_begin = msg->next_msg;
        if (!msg->next_msg)
            sock->msg_queue_end = NULL;
        sock->msg_queue_size -= sizeof(struct unix_dgram_msg)+msg->data_len;
    }
    release(&sock->lock);

    int write_data_len = MIN(param->length, msg->data_len);
    memmove(param->buffer, msg->data, write_data_len);
    if ((param->address)&&(param->address_len)&&(msg->sender_path))
    {   int addr_avail_len = *(param->address_len)-sizeof(sa_family_t);
        int sender_path_len = strlen(msg->sender_path)+1;
        int write_addr_len = MIN(addr_avail_len, sender_path_len);
        struct sockaddr_un* address = (struct sockaddr_un*)(param->address);
        address->sun_family = AF_UNIX;
        memmove(address->sun_path, msg->sender_path, write_addr_len);
        *(param->address_len) = write_addr_len+sizeof(sa_family_t);
    }

    if ((param->flags&MSG_PEEK)==0)
        pool_free(&unix_dgram_pool, msg);
    return write_data_len;
}

static int unix_dgram_close(struct file* sock_file, void* _param)
{ 
    struct unix_dgram_socket* sock = (struct unix_dgram_socket*)sock_file->sock;
    acquire(&sock->lock);
    if (sock->msg_queue_begin)
    {   struct unix_dgram_msg* current_msg = sock->msg_queue_begin;
        struct unix_dgram_msg* old_msg;
        while (current_msg)
        {   old_msg = current_msg;
            current_msg = current_msg->next_msg;
            pool_free(&unix_dgram_pool, old_msg);
        }
    }
    release(&sock->lock);
    if (sock->path)
        pool_free(&unix_dgram_pool, sock->path);
    pool_free(&unix_dgram_pool, sock);

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

static sockcall_t unix_dgram_funcs[] = {
    [SOCKCALL_CREATE]unix_dgram_create,
    [SOCKCALL_BIND]unix_dgram_bind,
    [SOCKCALL_SEND]unix_dgram_send,
    [SOCKCALL_RECV]unix_dgram_recv,
    [SOCKCALL_CLOSE]unix_dgram_close
};

void unix_dgram_module_init()
{ 
    sockcall_impl_t unix_dgram_impl = {
        .domain = AF_UNIX,
        .type = SOCK_DGRAM,
        .impl = unix_dgram_funcs
    };
    sockcall_impl_reg(unix_dgram_impl);
    pool_init(&unix_dgram_pool, unix_dgram_mem, sizeof(unix_dgram_mem));
}

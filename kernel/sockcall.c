#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "socket.h"
#include "xv6/param.h"
#include "xv6/defs.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
typedef int ssize_t;

#define SOCKCALL_IMPL_CAPACITY 16

static sockcall_impl_t sockcall_impl_store[SOCKCALL_IMPL_CAPACITY];
static size_t sockcall_impl_amount = 0;
static sockcall_t sockcall_impl(int oper, int domain, int type)
{
    sockcall_impl_t* impl_ref = sockcall_impl_store;
    for (size_t i=0;i<sockcall_impl_amount;i++)
    {
        if ((impl_ref->domain==domain)&&(impl_ref->type==type))
            return impl_ref->impl[oper];
        impl_ref++;
    }
    return NULL;
}

int sockcall_impl_reg(sockcall_impl_t impl)
{
    if (sockcall_impl_amount>=SOCKCALL_IMPL_CAPACITY)
        return ENOMEM;
    else
    {   sockcall_impl_store[sockcall_impl_amount] = impl;
        sockcall_impl_amount++;
        return 0;
    }
}

static int sockcall_create(struct file* sock_file, void* _param)
{
    struct sockcall_create_param* param = (struct sockcall_create_param*)_param;
    int domain = param->domain, type = param->type, protocol = param->protocol;

    if (protocol!=0)
        return -1*EPROTONOSUPPORT;
    sock_file = filealloc();
    if (!sock_file)
        return -1*ENFILE;

    sock_file->type = FD_SOCK;
    sock_file->readable = sock_file->writable = 1;
    sock_file->pipe = NULL;
    sock_file->ip = NULL;
    sock_file->sock = NULL;
    sockcall_t create_func = sockcall_impl(SOCKCALL_CREATE, domain, type);

    if (!create_func)
        return -1*EPROTONOSUPPORT;

    int fd = fdalloc(sock_file);
    if (fd<0)
    {   fileclose(sock_file);
        return -1*EMFILE;
    }

    int create_result = (*create_func)(sock_file, NULL);
    if (create_result<0)
    {   proc->ofile[fd] = NULL;
        fileclose(sock_file);
        return create_result;
    }

    return fd;
}

static int sockcall_bind(struct file* sock_file, void* _param)
{
    struct sockcall_bind_param* param = (struct sockcall_bind_param*)_param;
    const struct sockaddr* addr = param->address;
    struct socket* sock = sock_file->sock;
    if (addr->sa_family!=sock->domain)
        return -1*EAFNOSUPPORT;

    sockcall_t bind_func = sockcall_impl(SOCKCALL_BIND, sock->domain, sock->type);
    if (!bind_func)
        return -1*EOPNOTSUPP;

    return (*bind_func)(sock_file, _param);
}


static ssize_t sockcall_send(struct file* sock_file, void* _param)
{
    struct sockcall_send_param* param = (struct sockcall_send_param*)_param;
    struct sockaddr* dest_addr = param->dest_addr;
    struct socket* sock = sock_file->sock;
    if (dest_addr)
    { 
        if (dest_addr->sa_family!=sock->domain)
            return -1*EAFNOSUPPORT;
    }

    sockcall_t send_func = sockcall_impl(SOCKCALL_SEND, sock->domain, sock->type);
    if (!send_func)
        return -1*EOPNOTSUPP;

    return (*send_func)(sock_file, _param);
}

int sockwrite(struct file* sock_file, char* data, int len)
{ 
    struct sockcall_send_param param = {
        .message = data,
        .length = len,
        .flags = 0,
        .dest_addr = NULL,
        .dest_len = 0
    };
    return sockcall_send(sock_file, &param);
}

static ssize_t sockcall_recv(struct file* sock_file, void* _param)
{
    struct sockcall_recv_param* param = (struct sockcall_recv_param*)_param;
    struct socket* sock = sock_file->sock;
    if (param->address)
    {
        if (param->address->sa_family!=sock->domain)
            return -1*EAFNOSUPPORT;
    }
    sockcall_t recv_func = sockcall_impl(SOCKCALL_RECV, sock->domain, sock->type);
    if (!recv_func)
        return -1*EOPNOTSUPP;
    return (*recv_func)(sock_file, _param);
}

int sockread(struct file* sock_file, char* buffer, int len)
{
    struct sockcall_recv_param param = {
        .buffer = buffer,
        .length = len,
        .flags = 0,
        .address = NULL,
        .address_len = NULL
    };
    return sockcall_recv(sock_file, &param);
}

int sockclose(struct file* sock_file)
{   struct socket* sock = sock_file->sock;
    if (!sock)
        return 0;

    sockcall_t close_func = sockcall_impl(SOCKCALL_CLOSE, sock->domain, sock->type);
    if (!close_func)
        return -1*EPROTONOSUPPORT;
    return (*close_func)(sock_file, NULL);
}

static sockcall_t sockcall_opers[] = {
    [SOCKCALL_CREATE]sockcall_create,
    [SOCKCALL_BIND]sockcall_bind,
    [SOCKCALL_SEND]sockcall_send,
    [SOCKCALL_RECV]sockcall_recv
};

int sys_sockcall(void)
{
    int status;

    int sock_fd;
    status = argint(0, &sock_fd);
    if (status)
        return -1*EFAULT;
    if ((sock_fd<0)||(sock_fd>=NOFILE))
        return -1*EBADF;

    int oper;
    status = argint(1, &oper);
    if (status)
        return -1*EFAULT;
    if ((oper<0)||(oper>SOCKCALL_OPER_MAX))
        return -1*EINVAL;

    struct file* sock_file = NULL;
    if (oper!=SOCKCALL_CREATE)
    {   sock_file = proc->ofile[sock_fd];
        if (sock_file->type!=FD_SOCK)
            return -1*ENOTSOCK;
    }

    char* _param = NULL;
    status = argptr(2, &_param, 0);
    if (status)
        return -1*EFAULT;
    sockcall_t oper_func = sockcall_opers[oper];
    return (*oper_func)(sock_file, _param);
}

/** @file
 * @brief Socket system call kernel implementation
 * @author Qifan Lu
 * @date April 21, 2016
 * @version 1.0.0
 */

//[ Header Files ]
//Xv6 kernel extra
#include "etypes.h"
#include "edefs.h"
#include "errno.h"
#include "socket.h"
//Xv6 kernel
#include "xv6/param.h"
#include "xv6/defs.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"

//[ Types ]
//ssize_t
typedef int ssize_t;

//[ Constants ]
/**
 * Socket system call implementation capacity
 */
#define SOCKCALL_IMPL_CAPACITY 16

//[ Variables ]
/**
 * Socket system call implementation store
 */
static sockcall_impl_t sockcall_impl_store[SOCKCALL_IMPL_CAPACITY];

/**
 * Socket system call implementation store size
 */
static size_t sockcall_impl_amount = 0;

//[ Functions ]
/**
 * Get socket system call implementation for specific socket type.
 *
 * @param oper Socket operation.
 * @param domain Socket domain.
 * @param type Socket protocol.
 * @return Socket system call function for specific socket type, or null for no implementation.
 */
static sockcall_t sockcall_impl(int oper, int domain, int type)
{   //Search in the implementation list
    sockcall_impl_t* impl_ref = sockcall_impl_store;
    for (size_t i=0;i<sockcall_impl_amount;i++)
    {   //Matching implementation found
        if ((impl_ref->domain==domain)&&(impl_ref->type==type))
            return impl_ref->impl[oper];
        impl_ref++;
    }
    //Implementation not found, return NULL
    return NULL;
}

//Register socket system call implementation for given socket type
int sockcall_impl_reg(sockcall_impl_t impl)
{   //No memory to store implementation
    if (sockcall_impl_amount>=SOCKCALL_IMPL_CAPACITY)
        return ENOMEM;
    else
    {   sockcall_impl_store[sockcall_impl_amount] = impl;
        sockcall_impl_amount++;
        return 0;
    }
}

/**
 * Generic socket creation procedure.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Socket creation parameters.
 * @return File descriptor on success, negative error number for failure.
 */
static int sockcall_create(struct file* sock_file, void* _param)
{   //Create socket call parameter
    struct sockcall_create_param* param = (struct sockcall_create_param*)_param;
    //Domain, protocol and sub-protocol
    int domain = param->domain, type = param->type, protocol = param->protocol;

    //Currently sub-protocol other than zero is not supported
    if (protocol!=0)
        return -1*EPROTONOSUPPORT;

    //Allocate file descriptor for socket
    sock_file = filealloc();
    //No more file descriptor available
    if (!sock_file)
        return -1*ENFILE;

    //Initialize file descriptor
    sock_file->type = FD_SOCK;
    sock_file->readable = sock_file->writable = 1;
    sock_file->pipe = NULL;
    sock_file->ip = NULL;
    sock_file->sock = NULL;

    //Initialize socket with given domain and type
    sockcall_t create_func = sockcall_impl(SOCKCALL_CREATE, domain, type);
    //Socket creation not implemented
    if (!create_func)
        return -1*EPROTONOSUPPORT;

    //Add socket to process file descriptor
    int fd = fdalloc(sock_file);
    //Failed, return error code
    if (fd<0)
    {   fileclose(sock_file);
        return -1*EMFILE;
    }

    //Create socket with specific creation function
    int create_result = (*create_func)(sock_file, NULL);
    //Socket initialization failed, close file and return error code
    if (create_result<0)
    {   proc->ofile[fd] = NULL;
        fileclose(sock_file);
        return create_result;
    }

    //Return file descriptor
    return fd;
}

/**
 * Generic socket binding procedure.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Socket binding parameters.
 * @return 0 for success, negative error number for failure.
 */
static int sockcall_bind(struct file* sock_file, void* _param)
{   //Bind operation parameter
    struct sockcall_bind_param* param = (struct sockcall_bind_param*)_param;
    //Socket address object
    const struct sockaddr* addr = param->address;

    //Socket object
    struct socket* sock = sock_file->sock;
    //Address family must correspond with socket domain
    if (addr->sa_family!=sock->domain)
        return -1*EAFNOSUPPORT;

    //Get specific binding function
    sockcall_t bind_func = sockcall_impl(SOCKCALL_BIND, sock->domain, sock->type);
    //Not supported
    if (!bind_func)
        return -1*EOPNOTSUPP;

    //Do bind operation
    return (*bind_func)(sock_file, _param);
}

/**
 * Generic socket data sending procedure.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Socket data sending parameters.
 * @return Bytes of data sent on success, negative error number for failure.
 */
static ssize_t sockcall_send(struct file* sock_file, void* _param)
{   //Send operation parameter
    struct sockcall_send_param* param = (struct sockcall_send_param*)_param;
    //Destination address object
    struct sockaddr* dest_addr = param->dest_addr;

    //Socket object
    struct socket* sock = sock_file->sock;
    //Check address
    if (dest_addr)
    {   //Address family must correspond with socket domain
        if (dest_addr->sa_family!=sock->domain)
            return -1*EAFNOSUPPORT;
    }

    //Try to send data to given address
    sockcall_t send_func = sockcall_impl(SOCKCALL_SEND, sock->domain, sock->type);
    //Send operation not implemented
    if (!send_func)
        return -1*EOPNOTSUPP;

    //Do send operation
    return (*send_func)(sock_file, _param);
}

//Send data to socket using file writing
int sockwrite(struct file* sock_file, char* data, int len)
{   //Create socket send operation parameter object
    struct sockcall_send_param param = {
        .message = data,
        .length = len,
        .flags = 0,
        .dest_addr = NULL,
        .dest_len = 0
    };
    //Call send function
    return sockcall_send(sock_file, &param);
}

/**
 * Generic socket data receiving procedure.
 *
 * @param sock_file Socket file descriptor object.
 * @param _param Socket data receiving parameters.
 * @return Bytes of data received on success, negative error number for failure.
 */
static ssize_t sockcall_recv(struct file* sock_file, void* _param)
{   //Receive operation parameters
    struct sockcall_recv_param* param = (struct sockcall_recv_param*)_param;
    //Socket object
    struct socket* sock = sock_file->sock;

    //Check address
    if (param->address)
    {   //Address family must correspond with socket domain
        if (param->address->sa_family!=sock->domain)
            return -1*EAFNOSUPPORT;
    }

    //Try to send data to given address
    sockcall_t recv_func = sockcall_impl(SOCKCALL_RECV, sock->domain, sock->type);
    //Receive operation not implemented
    if (!recv_func)
        return -1*EOPNOTSUPP;

    //Do receive operation
    return (*recv_func)(sock_file, _param);
}

//Receive data from socket using file reading
int sockread(struct file* sock_file, char* buffer, int len)
{   //Create socket receive operation parameter object
    struct sockcall_recv_param param = {
        .buffer = buffer,
        .length = len,
        .flags = 0,
        .address = NULL,
        .address_len = NULL
    };
    //Call receive function
    return sockcall_recv(sock_file, &param);
}

//Close given socket
int sockclose(struct file* sock_file)
{   struct socket* sock = sock_file->sock;
    //Socket not initialized, skipped
    if (!sock)
        return 0;

    //Get specific socket close function
    sockcall_t close_func = sockcall_impl(SOCKCALL_CLOSE, sock->domain, sock->type);
    if (!close_func)
        return -1*EPROTONOSUPPORT;
    //Close socket
    return (*close_func)(sock_file, NULL);
}

//[ Variables ]
/**
 * Socket operation handlers
 */
static sockcall_t sockcall_opers[] = {
    [SOCKCALL_CREATE]sockcall_create,
    [SOCKCALL_BIND]sockcall_bind,
    [SOCKCALL_SEND]sockcall_send,
    [SOCKCALL_RECV]sockcall_recv
};

//[ Functions ]
/**
 * Socket operation system call entry
 */
int sys_sockcall(void)
{   //Argument fetching status
    int status;

    //Socket file descriptor number
    int sock_fd;
    status = argint(0, &sock_fd);
    if (status)
        return -1*EFAULT;
    //Not a vaild file descriptor
    if ((sock_fd<0)||(sock_fd>=NOFILE))
        return -1*EBADF;

    //Operation number
    int oper;
    status = argint(1, &oper);
    if (status)
        return -1*EFAULT;
    //Check operation number
    if ((oper<0)||(oper>SOCKCALL_OPER_MAX))
        return -1*EINVAL;

    struct file* sock_file = NULL;
    //Check file descriptor type
    if (oper!=SOCKCALL_CREATE)
    {   sock_file = proc->ofile[sock_fd];
        //Not socket file descriptor
        if (sock_file->type!=FD_SOCK)
            return -1*ENOTSOCK;
    }

    //Socket system call parameters
    char* _param = NULL;
    status = argptr(2, &_param, 0);
    if (status)
        return -1*EFAULT;

    //Move to specific operation function
    sockcall_t oper_func = sockcall_opers[oper];
    return (*oper_func)(sock_file, _param);
}

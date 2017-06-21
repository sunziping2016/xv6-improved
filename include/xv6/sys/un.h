#pragma once
#include <sys/socket.h>

#define XV6_UNIX_SOCK_PATH_MAX 108

struct sockaddr_un
{
    sa_family_t sun_family;

    char sun_path[XV6_UNIX_SOCK_PATH_MAX];
};

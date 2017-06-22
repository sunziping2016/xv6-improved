#pragma once

#include "socket.h"

#define UNIX_SOCK_PATH_MAX 108

struct sockaddr_un
{
    sa_family_t sun_family;
    char sun_path[UNIX_SOCK_PATH_MAX];
};

extern void unix_dgram_module_init();

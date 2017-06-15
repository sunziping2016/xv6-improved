#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/fcntl.h"
#include "xv6/user.h"
#include "xv6/x86.h"
#include <stddef.h>

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

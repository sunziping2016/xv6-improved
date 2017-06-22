//
// Created by gigi on 17-6-21.
//

#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fs.h"
#include "xv6/fcntl.h"

#define BUFFER_SIZE 512
#define false 0
#define true 1

int
main(int argc, char *argv[])
{
    int in_fd, out_fd, n_chars;
    char buf[BUFFER_SIZE];
    char input;
    struct stat st;
    if(argc < 3)
    {
        printf(1,"cp : %s source destination\n", argv[0]);
        exit();
    }
    if(-1 == (in_fd = open(argv[1], O_RDONLY)) )
    {
        printf(1,"Open File1 error\n");

        exit();
    }
    out_fd = open(argv[2],O_WRONLY | O_CREATE);

    while((n_chars = read(in_fd, buf, BUFFER_SIZE))>0)
    {
        if(write(out_fd, buf, n_chars) != n_chars)
        {
            printf(1,"Write File error\n");
            exit();
        }
    }
    if(n_chars < 0)
    {
        printf(1,"Read error\n");
        exit();
    }

    close(in_fd);
    close(out_fd);

    exit();
}

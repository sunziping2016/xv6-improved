//
// Created by ma on 17-6-22.
//

#include <stdio.h>
#include <stdlib.h>
#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fs.h"

#define BUFFER_SIZE  100

int
main(int argc, char *argv[])
{
    char *filename1, *filename2 = "-";
    char *buf1 = (char*)malloc(BUFFER_SIZE * sizeof(char));
    char *buf2 = (char*)malloc(BUFFER_SIZE * sizeof(char));
    ++argv;
    filename1 = *argv;
    if (*++argv)
    {
        filename2 = *argv;
    }
    const int ID1 = open(filename1, 0);
    const int ID2 = open(filename2, 0);
    if(ID1 < 0 || ID2 < 0)
    {
        myprintf("ERROR\n");
        exit();
    }
    while(1)
    {
        int size1 = read(ID1, buf1, BUFFER_SIZE * sizeof(char));
        int size2 = read(ID2, buf2, BUFFER_SIZE * sizeof(char));
        if(size1 == 0 && size2 == 0)
        {
            myprintf("0\n");
            exit();
        }
        if(size1 != size2)
        {
            myprintf("1\n");
            exit();

        }
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            if(buf1[i] != buf2[i])
            {
                myprintf("1\n");
                exit();
            }
        }
    }
    close(ID1);
    close(ID2);
    exit();
}
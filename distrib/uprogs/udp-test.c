#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xv6/sys/socket.h>
#include <xv6/netinet/in.h>

typedef int ssize_t;

#define true 1
#define false 0
//Master address ("127.0.0.1:23456")
struct sockaddr_in master_addr = {
    .sin_family = AF_INET,
    .sin_port = 23456,
    .sin_addr = {
        .s_addr = 0x0100007f
    }
};

//Worker address ("127.0.0.1:23457")
struct sockaddr_in worker_addr = {
    .sin_family = AF_INET,
    .sin_port = 23457,
    .sin_addr = {
        .s_addr = 0x0100007f
    }
};

void perror(char* str)
{
   puts(str);
}

void invert_str(char* str, unsigned int length)
{   for (unsigned int i=0;i<length/2;i++)
    {   char tmp = str[i];
        str[i] = str[length-1-i];
        str[length-1-i] = tmp;
    }
}

void master_proc()
{ 
    puts("Waiting for worker...");
    sleep(2);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock==-1)
    {  perror("[udp-test master] Socket creation error");
        exit(1);
    }
    int status = bind(sock, (struct sockaddr*)(&master_addr), sizeof(master_addr));
    if (status==-1)
    {  perror("[udp-test master] Bind error");
        exit(2);
    }

    char buffer[100];
    while (true)
    { 
        mygets(buffer);
        sendto(sock, buffer, strlen(buffer)+1, 0, (struct sockaddr*)(&worker_addr), sizeof(worker_addr));
        if (strcmp(buffer, "exit")==0)
            break;
        recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        puts(buffer);
    }
}

void worker_proc()
{ 
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock==-1)
    {   perror("[udp-test worker] Socket creation error");
        exit(1);
    }
    int status = bind(sock, (struct sockaddr*)(&worker_addr), sizeof(worker_addr));
    if (status==-1)
    {   perror("[udp-test worker] Bind error");
        exit(2);
    }

    char buffer[100];
    while (true)
    { 
        recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (strcmp(buffer, "exit")==0)
            break;
        invert_str(buffer, strlen(buffer));
        sendto(sock, buffer, strlen(buffer)+1, 0, (struct sockaddr*)(&master_addr), sizeof(master_addr));
    }
}

int main(int argc, char** argv)
{
    if (fork()==0)
        worker_proc();
    else
        master_proc();

    return 0;
}

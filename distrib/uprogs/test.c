#include "xv6/types.h"
#include "xv6/user.h"

volatile int count = 0;

int main(int argc, char *argv[])
{
    volatile int pid;
    pid = thread_create();

    if (pid > 0) {
        printf(1, "parent: count = %d, %x\n", count, &count);
        ++count;
        sleep(10);
        printf(1, "parent: count = %d, %x\n", count, &count);
        wait();
        exit();
    } else if (pid == 0) {
        printf(1, "child: count = %d, %x\n", count, &count);
        ++count;
        printf(1, "child: count = %d, %x\n", count, &count);
        exit();
    }
}

#include "xv6/types.h"
#include "xv6/user.h"
#define NTHREAD 5

volatile int count = 0;

int main(int argc, char *argv[])
{
    volatile int pid[NTHREAD + 1];
    volatile int index;

// **************************************************
// This block is used to allocate system resources.
// **************************************************
    usersem sem = semaphore_create(1);
// **************************************************
// Block end.
// **************************************************

    pid[0] = 1;
    for (int i = 1; i <= NTHREAD; i++) {
        pid[i] = thread_create();
        if (pid[i] == 0) {
            memset(pid, 0, sizeof(pid));
            index = i;
            break;
        }
        else if (pid[i] < 0) {
            printf(1, "thread error\n");
            exit();
        }
    }

    if (pid[0]) {

// **************************************************
// This block is the code of parent thread.
// **************************************************
// **************************************************
// Block end.
// **************************************************

        while (thread_wait() != -1)
            ;

// **************************************************
// This block is used to free system resources.
// **************************************************
        semaphore_free(sem);
// **************************************************
// Block end.
// **************************************************

        exit();
    } else {

// **************************************************
// This block is the code of chlid threads.
// **************************************************
        semaphore_acquire(sem);

        int t = count;
        sleep(20);
        count = t + 1;
        printf(1, "child %d: count = %d, %x\n", index, count);

        semaphore_release(sem);
// **************************************************
// Block end.
// **************************************************

        thread_exit();
    }
}

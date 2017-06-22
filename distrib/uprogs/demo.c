#include "xv6/types.h"
#include "xv6/user.h"

#define NWRITER 2
#define NREADER 7
#define NTHREAD (NWRITER + NREADER)

volatile int state[NTHREAD]; // 0: out
                             // 1: waiting
                             // 2: reading or writing
int count = 0;

void printstate()
{
    for (int i = 1; i <= NWRITER; i++) {
        printf(1, "  ");
        if (state[i] == 1) printf(1, "%d", i); else printf(1, " ");
        printf(1, " ");
    }
    printf(1, "  ");
    for (int i = 1; i <= NREADER; i++) {
        if (state[i + NWRITER] == 1) printf(1, "  %d ", i + NWRITER); else printf(1, "    ");
    }
    printf(1, " \n");

    printf(1, " \n");

    for (int i = 1; i <= NWRITER; i++)
        printf(1, "+---");
    printf(1, "+ ");
    for (int i = 1; i <= NREADER; i++)
        printf(1, "+---");
    printf(1, "+\n");

    for (int i = 1; i <= NWRITER; i++)
        printf(1, "|   ");
    printf(1, "| ");
    for (int i = 1; i <= NREADER; i++)
        printf(1, "|   ");
    printf(1, "|\n");

    for (int i = 1; i <= NWRITER; i++) {
        if (state[i] == 2) printf(1, "| %d ", i); else printf(1, "|   ");
    }
    printf(1, "| ");
    for (int i = 1; i <= NREADER; i++) {
        if (state[i + NWRITER] == 2) printf(1, "| %d ", i + NWRITER); else printf(1, "|   ");
    }
    printf(1, "|\n");

    for (int i = 1; i <= NWRITER; i++)
        printf(1, "|   ");
    printf(1, "| ");
    for (int i = 1; i <= NREADER; i++)
        printf(1, "|   ");
    printf(1, "|\n");

    for (int i = 1; i <= NWRITER; i++)
        printf(1, "+---");
    printf(1, "+ ");
    for (int i = 1; i <= NREADER; i++)
        printf(1, "+---");
    printf(1, "+\n");

    for (int i = 0; i < 9; i++)
        printf(1, "\n");

    sleep(10);
}

void updatestate(userlock lk, int index, int x)
{
    lock_acquire(lk);
    state[index] = x;
    printstate();
    lock_release(lk);
}

int main(int argc, char *argv[])
{
    volatile int pid[NTHREAD + 1];
    volatile int index;

// **************************************************
// This block is used to allocate system resources.
// **************************************************
    userlock statelk = lock_create();
    userlock mutex = lock_create();
    userlock rw = lock_create();
    userlock w = lock_create();

    memset(state, 0, sizeof(state));
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
        lock_free(statelk);
        lock_free(mutex);
        lock_free(rw);
        lock_free(w);
// **************************************************
// Block end.
// **************************************************

        exit();
    } else {

// **************************************************
// This block is the code of chlid threads.
// **************************************************
        if (index <= NWRITER) {
            while (1) {
                sleep(400 + 100 * index);

                lock_acquire(w);
                updatestate(statelk, index, 1);
                lock_acquire(rw);

                updatestate(statelk, index, 2);
                sleep(50);

                updatestate(statelk, index, 0);
                lock_release(rw);
                lock_release(w);
            }
        } else if (index <= NWRITER + NREADER) {
            while (1) {
                sleep(100 - index * 3);

                lock_acquire(w);
                updatestate(statelk, index, 1);
                lock_acquire(mutex);
                if (!count)
                    lock_acquire(rw);
                count++;
                lock_release(mutex);
                lock_release(w);

                updatestate(statelk, index, 2);
                sleep(50);

                updatestate(statelk, index, 0);
                lock_acquire(mutex);
                count--;
                if (!count)
                    lock_release(rw);
                lock_release(mutex);
            }
        }
// **************************************************
// Block end.
// **************************************************

        thread_exit();
    }
}

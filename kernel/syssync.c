#include "xv6/types.h"
#include "xv6/x86.h"
#include "xv6/defs.h"
#include "xv6/date.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"

struct {
    struct spinlock lock;
    struct {
        struct sleeplock lk;
        int isused;
    } lk[NLOCK];
} lktable;

userlock
sys_lock_create(void)
{
    acquire(&lktable.lock);

    userlock ulk = -1;
    for (int i = 0; i < NLOCK; i++)
        if (!lktable.lk[i].isused) {
            ulk = i;
            initsleeplock(&lktable.lk[i].lk, "user");
            lktable.lk[i].isused = 1;
            break;
        }

    release(&lktable.lock);

    return ulk;
}

void
sys_lock_acquire(void)
{
    userlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!lktable.lk[ulk].isused)
        panic("visit an unused lock");

    acquiresleep(&lktable.lk[ulk].lk);
}

void
sys_lock_release(void)
{
    userlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!lktable.lk[ulk].isused)
        panic("visit an unused lock");

    releasesleep(&lktable.lk[ulk].lk);
}

int
sys_lock_holding(void)
{
    userlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!lktable.lk[ulk].isused)
        panic("visit an unused lock");

    return holdingsleep(&lktable.lk[ulk].lk);
}

void
sys_lock_free(void)
{
    userlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!lktable.lk[ulk].isused)
        panic("visit an unused lock");

    acquire(&lktable.lock);

    lktable.lk[ulk].isused = 0;

    release(&lktable.lock);
}

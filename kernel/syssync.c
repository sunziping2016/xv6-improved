#include "xv6/types.h"
#include "xv6/x86.h"
#include "xv6/defs.h"
#include "xv6/date.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"
#include "xv6/semaphore.h"
#include "xv6/rwlock.h"

struct {
  struct spinlock lock; // Protect this lock table.
  struct {
    struct sleeplock lk;
    int isused;         // Is the lock used now?
  } lk[NLOCK];
} lktable;

struct {
  struct spinlock lock; // Protect this semaphore table.
  struct {
    struct semaphore sem;
    int isused;         // Is the semaphore used now?
  } sem[NSEM];
} semtable;

struct {
  struct spinlock lock; // Protect this semaphore table.
  struct {
    struct rwlock rw;
    int isused;         // Is the semaphore used now?
  } rw[NRW];
} rwtable;

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
        return -1;

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

usersem
sys_semaphore_create(void)
{
    int initvalue;
    if (argint(0, &initvalue) < 0)
        return -1;

    acquire(&semtable.lock);

    usersem usem = -1;
    for (int i = 0; i < NSEM; i++)
        if (!semtable.sem[i].isused) {
            usem = i;
            initsemaphore(&semtable.sem[i].sem, initvalue, "user");
            semtable.sem[i].isused = 1;
            break;
        }

    release(&semtable.lock);

    return usem;
}

void
sys_semaphore_acquire(void)
{
    usersem usem;
    if (argint(0, &usem) < 0)
        return;

    if (!semtable.sem[usem].isused)
        panic("visit an unused semaphore");

    acquiresemaphore(&semtable.sem[usem].sem);
}

void
sys_semaphore_release(void)
{
    usersem usem;
    if (argint(0, &usem) < 0)
        return;

    if (!semtable.sem[usem].isused)
        panic("visit an unused semaphore");

    releasesemaphore(&semtable.sem[usem].sem);
}

int
sys_semaphore_getcounter(void)
{
    usersem usem;
    if (argint(0, &usem) < 0)
        return -1;

    if (!semtable.sem[usem].isused)
        panic("visit an unused semaphore");

    return getcounter(&semtable.sem[usem].sem);
}

void
sys_semaphore_free(void)
{
    usersem usem;
    if (argint(0, &usem) < 0)
        return;

    if (!semtable.sem[usem].isused)
        panic("visit an unused semaphore");

    acquire(&semtable.lock);

    semtable.sem[usem].isused = 0;

    release(&semtable.lock);
}

userrwlock 
sys_rwlock_create(void)
{
    acquire(&rwtable.lock);

    userrwlock ulk = -1;
    for (int i = 0; i < NRW; i++)
        if (!rwtable.rw[i].isused) {
            ulk = i;
            initrwlock(&rwtable.rw[i].rw, "user");
            rwtable.rw[i].isused = 1;
            break;
        }

    release(&rwtable.lock);

    return ulk;
}

void sys_rwlock_acquire_read(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    acquireread(&rwtable.rw[ulk].rw);
}

void sys_rwlock_acquire_write(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    acquirewrite(&rwtable.rw[ulk].rw);
}

void sys_rwlock_release_read(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    releaseread(&rwtable.rw[ulk].rw);
}

void sys_rwlock_release_write(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    releasewrite(&rwtable.rw[ulk].rw);
}


int sys_rwlock_holding_read(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return -1;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    return holding_read(&rwtable.rw[ulk].rw);
}

int sys_rwlock_holding_write(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return -1;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    return holding_write(&rwtable.rw[ulk].rw);
}


void sys_rwlock_free(void)
{
    userrwlock ulk;
    if (argint(0, &ulk) < 0)
        return;

    if (!rwtable.rw[ulk].isused)
        panic("visit an unused lock");

    acquire(&rwtable.lock);

    rwtable.rw[ulk].isused = 0;

    release(&rwtable.lock);
}

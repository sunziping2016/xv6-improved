// Sleeping locks

#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/x86.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"
#include "xv6/rwlock.h"

void initrwlock(struct rwlock *lk, char *name)
{
    initlock(&lk->lk, "read and write lock");
    lk->name = name;
    lk->read_locked = 0;
    lk->write_locked = 0;
    lk->w_required = 0;
    //lk->pid = 0;
}

void acquireread(struct rwlock *lk)
{
    acquire(&lk->lk);
    while (lk->write_locked || lk->w_required)
        sleep(lk, &lk->lk);
    lk->read_locked += 1;
   // lk->pid = proc->pid;
    release(&lk->lk);
}

void acquirewrite(struct rwlock *lk)
{
    acquire(&lk->lk);
    lk->w_required++;
    while (lk->read_locked || lk->write_locked) 
        //sleep(&(lk->write_lk), &lk->lk);
	sleep(lk, &lk->lk);
    lk->w_required--;
    lk->write_locked = 1;
   // lk->pid = proc->pid;
    release(&lk->lk);
}

void releaseread(struct rwlock *lk)
{
    acquire(&lk->lk);
    lk->read_locked--;
   // lk->pid = 0;
    //wakeup(lk);
    release(&lk->lk);
}

void releasewrite(struct rwlock *lk)
{
    acquire(&lk->lk);
    lk->write_locked = 0;
   // lk->pid = 0;
   // wakeup(&(lk->write_lk));
    wakeup(lk);
    release(&lk->lk);
}

int holding_read(struct rwlock *lk)
{
    int r;

    acquire(&lk->lk);
    r = lk->read_locked;
    release(&lk->lk);
    return r;
}

int holding_write(struct rwlock *lk)
{
    int r;

    acquire(&lk->lk);
    r = lk->write_locked;
    release(&lk->lk);
    return r;
}

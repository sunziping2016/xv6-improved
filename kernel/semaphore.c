// Semaphore

#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/x86.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/spinlock.h"
#include "xv6/semaphore.h"

struct {
  struct spinlock lock; // Protect this table.
  struct {
    struct procnode pnode;
    int isused;         // Is the node used now?
  } pnode[NPNODE]
} pntable;

// Init the queue.
void
initqueue(struct proclist *list)
{
    list->head = list->tail = -1;
}

// Insert a process to the tail.
// This function is unsafe.
void
inqueue(struct proclist *list, struct proc *p)
{
    acquire(&pntable.lock);

    int index = -1;
    for (int i = 0; i < NPNODE; i++)
        if (!pntable.pnode[i].isused) {
            index = i;
            break;
        }

    if (index == -1)
        panic("process node missing");

    pntable.pnode[index].pnode.proc = p;
    pntable.pnode[index].isused = 1;
    if (list->tail == -1) {
        list->head = list->tail = index;
    } else {
        pntable.pnode[list->tail].pnode.next = index;
        list->tail = index;
    }

    release(&pntable.lock);
}

// Delete a process from the head, and return it.
// This function is unsafe.
struct proc*
outqueue(struct proclist *list)
{
    if (list->head == -1)
        return 0;
    acquire(&pntable.lock);

    struct proc *p = pntable.pnode[list->head].pnode.proc;
    pntable.pnode[list->head].isused = 0;
    if (list->head == list->tail) {
        list->head = list->tail = -1;
    } else {
        list->head = pntable.pnode[list->head].pnode.next;
    }

    release(&pntable.lock);
    return p;
}

void
initsemaphore(struct semaphore *sem, int initvalue, char *name)
{
    sem->count = initvalue;
    initqueue(&sem->queue);
    initlock(&sem->lk, "semaphore lock");

    sem->name = name;
}

void
acquiresemaphore(struct semaphore *sem)
{
    acquire(&sem->lk);
    sem->count--;
    if (sem->count < 0) {
        inqueue(&sem->queue, proc);
        sleep(proc, &sem->lk);
    }
    release(&sem->lk);
}

void
releasesemaphore(struct semaphore *sem)
{
    acquire(&sem->lk);
    sem->count++;
    if (sem->count <= 0) {
        struct proc *p = outqueue(&sem->queue);
        wakeup(p);
    }
    release(&sem->lk);
}

int
getcounter(struct semaphore *sem)
{
    int r;

    acquire(&sem->lk);
    r = sem->count;
    release(&sem->lk);
    return r;
}


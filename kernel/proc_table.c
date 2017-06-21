#include "xv6/types.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/x86.h"
#include "xv6/proc.h"

int add_bar_back(struct proc_queue *q, struct proc *p)
{
    if (q->head == NULL) {
        q->head = p;
        q->tail = p;
        p->prev = p->next  = NULL;
        p->which = q;
    }
    else {
        q->tail->next = p;
        p->prev = q->tail;
        q->tail = q->tail->next;
        p->next = NULL;
        p->which = q;
    }
    return 0;
}

int add_bar_front(struct proc_queue *q, struct proc *p)//add p to the front of q
{
    if (q->head == NULL) {
        q->head = p;
        q->tail = p;
        p->prev = p->next = NULL;
        p->which = q;
    }
    else {
        q->head->prev = p;
        p->next = q->head;
        q->head = q->head->prev;
        p->prev = NULL;
        p->which = q;
    }
    return 0;
}

struct proc* erase_bar(struct proc_queue *q, struct proc *p)//erase p from q
{
    if(p == q->head && p == q->tail)
    {
        q->head = q->tail = NULL;
    }
    else if (p == q->head)
    {
        q->head = q->head->next;
        q->head->prev = 0;
        p->next = 0;
    }
    else if (p == q->tail)
    {
        q->tail = q->tail->prev;
        q->tail->next = 0;
        p->prev = 0;
    }
    else
    {
        p->prev->next = p->next;
        p->next->prev = p->prev;
        p->next = p->prev = 0;
    }
    p->which = NULL;
    return p;
}


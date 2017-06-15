//
// Created by parallels on 6/11/17.
//

#include "xv6/types.h"
#include "xv6/x86.h"
#include "xv6/defs.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/spinlock.h"
#include "xv6/sleeplock.h"

static struct {
    struct spinlock lock;
    int locking;
} cons;

static ushort *crt = (ushort*)P2V(0xb8000);
#define CRTPORT 0x3d4

int
sys_getcrtc(void)
{
    //initlock(&cons.lock, "console");
    //cons.locking = 1;
    //acquire(&cons.lock);

    int x, y;
    argint(0, &x);
    argint(1, &y);

    //release(&cons.lock);
    return crt[y * 80 + x];
}

int
sys_setcrtc(void)
{
    //initlock(&cons.lock, "console");
    //cons.locking = 1;
    //acquire(&cons.lock);

    int pos, c;
    argint(0, &pos);
    argint(1, &c);
    crt[pos] = c;
    //release(&cons.lock);
    return c;
}

int
sys_getcurpos(void)
{
    //initlock(&cons.lock, "console");
    //cons.locking = 1;
    //acquire(&cons.lock);

    int pos;
    // Cursor position: col + 80*row.
    outb(CRTPORT, 14);
    pos = inb(CRTPORT + 1) << 8;
    outb(CRTPORT, 15);
    pos |= inb(CRTPORT + 1);

    //release(&cons.lock);

    return pos;
}

int
sys_setcurpos(void)
{
    //initlock(&cons.lock, "console");
    //cons.locking = 1;
    //acquire(&cons.lock);

    int pos;
    argint(0, &pos);
    outb(CRTPORT, 14);
    outb(CRTPORT + 1, pos >> 8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, pos);
    //release(&cons.lock);
    return 1;
}

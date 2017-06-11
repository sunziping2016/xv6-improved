#include "xv6/types.h"
#include "xv6/x86.h"
#include "xv6/defs.h"
#include "xv6/date.h"
#include "xv6/param.h"
#include "xv6/memlayout.h"
#include "xv6/mmu.h"
#include "xv6/proc.h"
#include "xv6/kbd.h"

static ushort *crt = (ushort*)P2V(0xb8000);
#define CRTPORT 0x3d4

int
sys_fork(void)
{
    return fork();
}

int
sys_exit(void)
{
    exit();
    return 0;  // not reached
}

int
sys_wait(void)
{
    return wait();
}

int
sys_kill(void)
{
    int pid;

    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int
sys_getpid(void)
{
    return proc->pid;
}

int
sys_sbrk(void)
{
    int addr;
    int n;

    if (argint(0, &n) < 0)
        return -1;
    addr = proc->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

int
sys_sleep(void)
{
    int n;
    uint ticks0;

    if (argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (proc->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

/*

int
sys_getcrtc(void)
{
    int x, y;
    argint(0, &x);
    argint(1, &y);
    return crt[y * 80 + x];
}

int
sys_setcrtc(void)
{
    int pos, c;
    argint(0, &pos);
    argint(1, &c);
    crt[pos] = c;
    return c;
}

int
sys_getcurpos(void)
{
    int pos;
    // Cursor position: col + 80*row.
    outb(CRTPORT, 14);
    pos = inb(CRTPORT + 1) << 8;
    outb(CRTPORT, 15);
    pos |= inb(CRTPORT + 1);
    return pos;
}

int
sys_setcurpos(void)
{
    int pos;
    argint(0, &pos);
    outb(CRTPORT, 14);
    outb(CRTPORT + 1, pos >> 8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, pos);
    return 1;
}*/
